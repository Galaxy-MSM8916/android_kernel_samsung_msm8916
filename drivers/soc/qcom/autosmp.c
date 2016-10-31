/*
 * drivers/soc/qcom/autosmp.c
 *
 * automatically hotplug/unplug multiple cpu cores
 * based on cpu load and suspend state
 *
 * based on the msm_mpdecision code by
 * Copyright (c) 2012-2013, Dennis Rassmann <showp1984@gmail.com>
 *
 * Copyright (C) 2013-2014, Rauf Gungor, http://github.com/mrg666
 * rewrite to simplify and optimize, Jul. 2013, http://goo.gl/cdGw6x
 * optimize more, generalize for n cores, Sep. 2013, http://goo.gl/448qBz
 * generalize for all arch, rename as autosmp, Dec. 2013, http://goo.gl/x5oyhy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. For more details, see the GNU
 * General Public License included with the Linux kernel or available
 * at www.gnu.org/licenses
 */

#include <linux/moduleparam.h>
#include <linux/cpufreq.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/input.h>
#ifdef CONFIG_STATE_NOTIFIER
#include <linux/state_notifier.h>
#endif

#define DEBUG 0

#define ASMP_TAG			"AutoSMP:"
#define ASMP_ENABLED			false
#define DEFAULT_BOOST_LOCK_DUR		800 * 1000L
#define DEFAULT_NR_CPUS_BOOSTED		2
#define DEFAULT_UPDATE_RATE		30
#define MIN_INPUT_INTERVAL		150 * 1000L
#define DEFAULT_MIN_BOOST_FREQ		1728000

#if DEBUG
struct asmp_cpudata_t {
	long long unsigned int times_hotplugged;
};
static DEFINE_PER_CPU(struct asmp_cpudata_t, asmp_cpudata);
#endif

static struct delayed_work asmp_work;
static struct workqueue_struct *asmp_workq;
static bool enabled_switch = ASMP_ENABLED;

static struct asmp_param_struct {
	unsigned int delay;
	unsigned int max_cpus;
	unsigned int min_cpus;
	unsigned int cpufreq_up;
	unsigned int cpufreq_down;
	unsigned int cycle_up;
	unsigned int cycle_down;
	unsigned int cpus_boosted;
	unsigned int min_boost_freq;
	bool enabled;
	u64 boost_lock_dur;
#ifdef CONFIG_STATE_NOTIFIER
	struct notifier_block notif;
#endif
} asmp_param = {
	.delay = DEFAULT_UPDATE_RATE,
	.max_cpus = NR_CPUS,
	.min_cpus = 1,
	.cpufreq_up = 95,
	.cpufreq_down = 80,
	.cycle_up = 1,
	.cycle_down = 1,
	.min_boost_freq = DEFAULT_MIN_BOOST_FREQ,
	.cpus_boosted = DEFAULT_NR_CPUS_BOOSTED,
	.enabled = ASMP_ENABLED,
	.boost_lock_dur = DEFAULT_BOOST_LOCK_DUR,
};

static u64 last_boost_time;
static unsigned int cycle = 0;

/*
 * suspend mode, if set = 1 hotplug will sleep,
 * if set = 0, then hoplug will be active all the time.
 */
static unsigned int hotplug_suspend = 1;
module_param_named(hotplug_suspend, hotplug_suspend, uint, 0644);

static void reschedule_hotplug_work(void)
{
	queue_delayed_work(asmp_workq, &asmp_work,
			msecs_to_jiffies(asmp_param.delay));
}

static void max_min_check(void)
{
	asmp_param.max_cpus = max((unsigned int)1, asmp_param.max_cpus);
	asmp_param.min_cpus = max((unsigned int)1, asmp_param.min_cpus);

	if (asmp_param.max_cpus > NR_CPUS)
		asmp_param.max_cpus = NR_CPUS;
	if (asmp_param.min_cpus > asmp_param.max_cpus)
		asmp_param.min_cpus = asmp_param.max_cpus;
}

static void __cpuinit asmp_work_fn(struct work_struct *work)
{
	unsigned int cpu = 0, slow_cpu = 0;
	unsigned int rate, cpu0_rate, slow_rate = UINT_MAX, fast_rate;
	unsigned int max_rate, up_rate, down_rate;
	unsigned int nr_cpu_online;
	unsigned int min_boost_freq = asmp_param.min_boost_freq;
	u64 now;
	
	if (!asmp_param.enabled)
		return;

	/* get maximum possible freq for cpu0 and
	   calculate up/down limits */
	max_rate  = cpufreq_quick_get_max(cpu);
	up_rate   = (max_rate / 100) * asmp_param.cpufreq_up;
	down_rate = (max_rate / 100) * asmp_param.cpufreq_down;

	/* find current max and min cpu freq to estimate load */
	nr_cpu_online = num_online_cpus();
	cpu0_rate = cpufreq_quick_get(cpu);
	fast_rate = cpu0_rate;

	for_each_online_cpu(cpu) {
		if (cpu) {
			rate = cpufreq_quick_get(cpu);
			if (rate <= slow_rate) {
				slow_cpu = cpu;
				slow_rate = rate;
			} else if (rate > fast_rate)
				fast_rate = rate;
		}
	}

	if (cpu0_rate < slow_rate)
		slow_rate = cpu0_rate;

	if (max_rate <= asmp_param.min_boost_freq)
		min_boost_freq = max_rate;

	now = ktime_to_us(ktime_get());
	/* hotplug one core if all online cores are over up_rate limit */
	if (slow_rate > up_rate && fast_rate >= min_boost_freq) {
		if (nr_cpu_online < asmp_param.max_cpus &&
				cycle >= asmp_param.cycle_up) {
			cpu = cpumask_next_zero(0, cpu_online_mask);
			if (cpu_is_offline(cpu))
				cpu_up(cpu);
			cycle = 0;
#if DEBUG
			pr_info(ASMP_TAG"CPU [%d] On  | Mask [%d%d%d%d]\n",
				cpu, cpu_online(0), cpu_online(1), cpu_online(2), cpu_online(3));
#endif
		}
	/* check if boost required */
	} else if (nr_cpu_online <= asmp_param.cpus_boosted &&
			now - last_boost_time <= asmp_param.boost_lock_dur) {
		if (nr_cpu_online < asmp_param.cpus_boosted &&
			nr_cpu_online < asmp_param.max_cpus) {
			cpu = cpumask_next_zero(0, cpu_online_mask);
			if (cpu_is_offline(cpu))
				cpu_up(cpu);
			// cycle = 0;
		}
	/* unplug slowest core if all online cores are under down_rate limit */
	} else if (slow_cpu && (fast_rate < down_rate)) {
		if (nr_cpu_online > asmp_param.min_cpus &&
				cycle >= asmp_param.cycle_down) {

			if (cpu_online(slow_cpu))
	 			cpu_down(slow_cpu);
			cycle = 0;
#if DEBUG
			pr_info(ASMP_TAG"CPU [%d] Off | Mask [%d%d%d%d]\n",
				slow_cpu, cpu_online(0), cpu_online(1), cpu_online(2), cpu_online(3));
			per_cpu(asmp_cpudata, cpu).times_hotplugged += 1;
#endif
		}
	} /* else do nothing */

	cycle++;
	reschedule_hotplug_work();
}

#ifdef CONFIG_STATE_NOTIFIER
static void asmp_suspend(void)
{
	unsigned int cpu;

	/* Flush hotplug workqueue */
	flush_workqueue(asmp_workq);
	cancel_delayed_work_sync(&asmp_work);

	/* unplug online cpu cores */
	for_each_possible_cpu(cpu)
		if (cpu != 0 && cpu_online(cpu))
			cpu_down(cpu);

	pr_info(ASMP_TAG"Screen -> Off. Suspended.\n");
}

static void __ref asmp_resume(void)
{
	unsigned int cpu;

	/* Fire up all CPUs */
	for_each_possible_cpu(cpu)
		if (cpu_is_offline(cpu))
			cpu_up(cpu);

	last_boost_time = ktime_to_us(ktime_get());

	/* Resume hotplug workqueue */
	INIT_DELAYED_WORK(&asmp_work, asmp_work_fn);
	reschedule_hotplug_work();
	pr_info(ASMP_TAG"Screen -> On. Resumed.\n");
}

static int state_notifier_callback(struct notifier_block *this,
				unsigned long event, void *data)
{
	if (!asmp_param.enabled || !hotplug_suspend)
                return NOTIFY_OK;

	switch (event) {
		case STATE_NOTIFIER_ACTIVE:
			asmp_resume();
			break;
		case STATE_NOTIFIER_SUSPEND:
			asmp_suspend();
			break;
		default:
			break;
	}

	return NOTIFY_OK;
}
#endif

static void autosmp_input_event(struct input_handle *handle, unsigned int type,
				unsigned int code, int value)
{
	u64 now;

	if (!asmp_param.enabled)
		return;

#ifdef CONFIG_STATE_NOTIFIER
	if (state_suspended)
		return;
#endif

	now = ktime_to_us(ktime_get());
	if (now - last_boost_time < MIN_INPUT_INTERVAL)
		return;

	if (asmp_param.cpus_boosted <= asmp_param.min_cpus)
		return;

	last_boost_time = ktime_to_us(ktime_get());
}

static int autosmp_input_connect(struct input_handler *handler,
				 struct input_dev *dev,
				 const struct input_device_id *id)
{
	struct input_handle *handle;
	int err;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = handler->name;

	err = input_register_handle(handle);
	if (err)
		goto err_register;

	err = input_open_device(handle);
	if (err)
		goto err_open;

	return 0;
err_open:
	input_unregister_handle(handle);
err_register:
	kfree(handle);
	return err;
}

static void autosmp_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id autosmp_ids[] = {
	/* multi-touch touchscreen */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.evbit = { BIT_MASK(EV_ABS) },
		.absbit = { [BIT_WORD(ABS_MT_POSITION_X)] =
			BIT_MASK(ABS_MT_POSITION_X) |
			BIT_MASK(ABS_MT_POSITION_Y) },
	},
	/* touchpad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_KEYBIT |
			INPUT_DEVICE_ID_MATCH_ABSBIT,
		.keybit = { [BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) },
		.absbit = { [BIT_WORD(ABS_X)] =
			BIT_MASK(ABS_X) | BIT_MASK(ABS_Y) },
	},
	/* Keypad */
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};

static struct input_handler autosmp_input_handler = {
	.event		= autosmp_input_event,
	.connect	= autosmp_input_connect,
	.disconnect	= autosmp_input_disconnect,
	.name		= ASMP_TAG,
	.id_table	= autosmp_ids,
};

static int hotplug_start(void)
{
	int ret = 0;

	asmp_workq =
		alloc_workqueue("autosmp_wq",
			WQ_HIGHPRI | WQ_FREEZABLE, 0);
	if (!asmp_workq) {
		pr_err("%s: Failed to allocate hotplug workqueue\n",
					ASMP_TAG);
		ret = -ENOMEM;
		goto err_wq;
	}

#ifdef CONFIG_STATE_NOTIFIER
	asmp_param.notif.notifier_call = state_notifier_callback;
	if (state_register_client(&asmp_param.notif)) {
		pr_err("%s: Failed to register State notifier callback\n",
			ASMP_TAG);
		goto err_notif;
	}
#endif

	ret = input_register_handler(&autosmp_input_handler);
	if (ret) {
		pr_err("%s: Failed to register input handler: %d\n",
		       ASMP_TAG, ret);
		goto err;
	}

	INIT_DELAYED_WORK(&asmp_work, asmp_work_fn);
	max_min_check();
	reschedule_hotplug_work();
	return ret;

err:
#ifdef CONFIG_STATE_NOTIFIER
	state_unregister_client(&asmp_param.notif);
err_notif:
	asmp_param.notif.notifier_call = NULL;
#endif
	destroy_workqueue(asmp_workq);
err_wq:
	asmp_param.enabled = false;
	return ret;
}

static void __ref hotplug_stop(void)
{
	int cpu;

	input_unregister_handler(&autosmp_input_handler);
#ifdef CONFIG_STATE_NOTIFIER
	state_unregister_client(&asmp_param.notif);
	asmp_param.notif.notifier_call = NULL;
#endif
	flush_workqueue(asmp_workq);
	cancel_delayed_work_sync(&asmp_work);
	destroy_workqueue(asmp_workq);

	/* Wake up all the sibling cores */
	for_each_possible_cpu(cpu)
		if (cpu_is_offline(cpu))
			cpu_up(cpu);
}

static int __cpuinit set_enabled(const char *val, const struct kernel_param *kp)
{
	int ret = 0;

	ret = param_set_bool(val, kp);
	if (ret)
		return -EINVAL;

	if (enabled_switch == asmp_param.enabled)
		return ret;

	enabled_switch = asmp_param.enabled;

	if (asmp_param.enabled)
		hotplug_start();
	else
		hotplug_stop();

	return ret;
}

static struct kernel_param_ops module_ops = {
	.set = set_enabled,
	.get = param_get_bool,
};

module_param_cb(enabled, &module_ops, &asmp_param.enabled, 0644);
MODULE_PARM_DESC(enabled, "hotplug/unplug cpu cores based on cpu load");

/***************************** SYSFS START *****************************/
#define define_one_global_ro(_name)					\
static struct global_attr _name =					\
__ATTR(_name, 0444, show_##_name, NULL)

#define define_one_global_rw(_name)					\
static struct global_attr _name =					\
__ATTR(_name, 0644, show_##_name, store_##_name)

struct kobject *asmp_kobject;

#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct attribute *attr, char *buf)		\
{									\
	return sprintf(buf, "%u\n", asmp_param.object);			\
}
show_one(delay, delay);
show_one(min_cpus, min_cpus);
show_one(max_cpus, max_cpus);
show_one(cpufreq_up, cpufreq_up);
show_one(cpufreq_down, cpufreq_down);
show_one(cycle_up, cycle_up);
show_one(cycle_down, cycle_down);

#define store_one(file_name, object)					\
static ssize_t store_##file_name					\
(struct kobject *a, struct attribute *b, const char *buf, size_t count)	\
{									\
	unsigned int input;						\
	int ret;							\
	ret = sscanf(buf, "%u", &input);				\
	if (ret != 1)							\
		return -EINVAL;						\
	asmp_param.object = input;					\
	max_min_check();						\
	if (asmp_param.enabled)						\
		reschedule_hotplug_work();				\
	return count;							\
}									\
define_one_global_rw(file_name);
store_one(delay, delay);
store_one(min_cpus, min_cpus);
store_one(max_cpus, max_cpus);
store_one(cpufreq_up, cpufreq_up);
store_one(cpufreq_down, cpufreq_down);
store_one(cycle_up, cycle_up);
store_one(cycle_down, cycle_down);

static ssize_t show_boost_lock_duration(struct device *dev,
				        struct device_attribute
				        *asmp_attributes, char *buf)
{
	return sprintf(buf, "%llu\n", div_u64(asmp_param.boost_lock_dur, 1000));
}

static ssize_t store_boost_lock_duration(struct device *dev,
					 struct device_attribute
					 *asmp_attributes, const char *buf,
					 size_t count)
{
	int ret;
	u64 val;

	ret = sscanf(buf, "%llu", &val);
	if (ret != 1)
		return -EINVAL;

	asmp_param.boost_lock_dur = val * 1000;

	return count;
}

static ssize_t show_cpus_boosted(struct device *dev,
				 struct device_attribute *asmp_attributes,
				 char *buf)
{
	return sprintf(buf, "%u\n", asmp_param.cpus_boosted);
}

static ssize_t store_cpus_boosted(struct device *dev,
				  struct device_attribute *asmp_attributes,
				  const char *buf, size_t count)
{
	int ret;
	unsigned int val;

	ret = sscanf(buf, "%u", &val);
	if (ret != 1 || val < 1 || val > CONFIG_NR_CPUS)
		return -EINVAL;

	asmp_param.cpus_boosted = val;

	return count;
}


static ssize_t show_min_boost_freq(struct device *dev,
				   struct device_attribute *asmp_attributes,
				   char *buf)
{
	return sprintf(buf, "%u\n", asmp_param.min_boost_freq);
}

static ssize_t store_min_boost_freq(struct device *dev,
				    struct device_attribute *asmp_attributes,
				    const char *buf, size_t count)
{
	int ret;
	unsigned int val;

	ret = sscanf(buf, "%u", &val);
	if (ret != 1)
		return -EINVAL;

	asmp_param.min_boost_freq = val;

	return count;
}

static DEVICE_ATTR(boost_lock_duration, 644, show_boost_lock_duration,
		   store_boost_lock_duration);
static DEVICE_ATTR(cpus_boosted, 644, show_cpus_boosted, store_cpus_boosted);
static DEVICE_ATTR(min_boost_freq, 644, show_min_boost_freq,
		   store_min_boost_freq);

static struct attribute *asmp_attributes[] = {
	&delay.attr,
	&min_cpus.attr,
	&max_cpus.attr,
	&cpufreq_up.attr,
	&cpufreq_down.attr,
	&cycle_up.attr,
	&cycle_down.attr,
	&dev_attr_boost_lock_duration.attr,
	&dev_attr_cpus_boosted.attr,
	&dev_attr_min_boost_freq.attr,
	NULL
};

static struct attribute_group asmp_attr_group = {
	.attrs = asmp_attributes,
	.name = "conf",
};

#if DEBUG
static ssize_t show_times_hotplugged(struct kobject *a,
					struct attribute *b, char *buf)
{
	ssize_t len = 0;
	int cpu;

	for_each_possible_cpu(cpu) {
		len += sprintf(buf + len, "%i %llu\n", cpu,
			per_cpu(asmp_cpudata, cpu).times_hotplugged);
	}
	return len;
}
define_one_global_ro(times_hotplugged);

static struct attribute *asmp_stats_attributes[] = {
	&times_hotplugged.attr,
	NULL
};

static struct attribute_group asmp_stats_attr_group = {
	.attrs = asmp_stats_attributes,
	.name = "stats",
};
#endif
/****************************** SYSFS END ******************************/

static int __init asmp_init(void)
{
	int ret = 0;

	asmp_param.max_cpus = NR_CPUS;
#if DEBUG
	for_each_possible_cpu(cpu)
		per_cpu(asmp_cpudata, cpu).times_hotplugged = 0;
#endif

	asmp_kobject = kobject_create_and_add("autosmp", kernel_kobj);
	if (asmp_kobject) {
		ret = sysfs_create_group(asmp_kobject, &asmp_attr_group);
		if (ret) {
			pr_warn(ASMP_TAG "sysfs: ERROR, create sysfs group.");
			goto err_dev;
		}
#if DEBUG
		ret = sysfs_create_group(asmp_kobject, &asmp_stats_attr_group);
		if (ret) {
			pr_warn(ASMP_TAG "sysfs: ERROR, create sysfs stats group.");
			goto err_dev;
		}
#endif
	} else {
		pr_warn(ASMP_TAG "sysfs: ERROR, create sysfs kobj");
		goto err_dev;
	}

	if (asmp_param.enabled)
		hotplug_start();

	pr_info(ASMP_TAG "Init complete.\n");
	return ret;

err_dev:
	asmp_param.enabled = false;
	return ret;
}

late_initcall(asmp_init);
