/*
 *  drivers/cpufreq/cpufreq_badass.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2003 Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>.
 *                      Jun Nakajima <jun.nakajima@intel.com>
 *            (C)  2012 Dennis Rassmann <showp1984@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

/*
 * bds is used in this file as a shortform for demandbased switching
 * It helps to keep variable names smaller, simpler
 */

#define DEF_FREQUENCY_DOWN_DIFFERENTIAL		(10)
#define DEF_FREQUENCY_UP_THRESHOLD		(80)
#define DEF_SAMPLING_DOWN_FACTOR		(1)
#define MAX_SAMPLING_DOWN_FACTOR		(100000)
#define MICRO_FREQUENCY_DOWN_DIFFERENTIAL	(3)
#define MICRO_FREQUENCY_UP_THRESHOLD		(95)
#define MICRO_FREQUENCY_MIN_SAMPLE_RATE		(10000)
#define MIN_FREQUENCY_UP_THRESHOLD		(11)
#define MAX_FREQUENCY_UP_THRESHOLD		(100)
#define MIN_FREQUENCY_DOWN_DIFFERENTIAL		(1)

/* Phase configurables */
#define MAX_IDLE_COUNTER			160
#define PHASE_2_PERCENT				80
#define PHASE_3_PERCENT				90
#define SEMI_BUSY_THRESHOLD			14
#define SEMI_BUSY_CLR_THRESHOLD			6
#define BUSY_THRESHOLD				130
#define BUSY_CLR_THRESHOLD			100
#define DECREASE_IDLE_COUNTER			14

/*
 * The polling frequency of this governor depends on the capability of
 * the processor. Default polling frequency is 1000 times the transition
 * latency of the processor. The governor will work on any processor with
 * transition latency <= 10mS, using appropriate sampling
 * rate.
 * For CPUs with transition latency > 10mS (mostly drivers with CPUFREQ_ETERNAL)
 * this governor will not work.
 * All times here are in uS.
 */
#define MIN_SAMPLING_RATE_RATIO			(2)

static unsigned int min_sampling_rate;

#define LATENCY_MULTIPLIER			(1000)
#define MIN_LATENCY_MULTIPLIER			(20)
#define TRANSITION_LATENCY_LIMIT		(10 * 1000 * 1000)

#define POWERSAVE_BIAS_MAXLEVEL			(1000)
#define POWERSAVE_BIAS_MINLEVEL			(-1000)

static void do_bds_timer(struct work_struct *work);
static int cpufreq_governor_bds(struct cpufreq_policy *policy,
				unsigned int event);

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_BADASS
static
#endif
struct cpufreq_governor cpufreq_gov_badass = {
       .name                   = "badass",
       .governor               = cpufreq_governor_bds,
       .max_transition_latency = TRANSITION_LATENCY_LIMIT,
       .owner                  = THIS_MODULE,
};

/* Sampling types */
enum {BDS_NORMAL_SAMPLE, BDS_SUB_SAMPLE};

struct cpu_bds_info_s {
	cputime64_t prev_cpu_idle;
	cputime64_t prev_cpu_iowait;
	cputime64_t prev_cpu_wall;
	cputime64_t prev_cpu_nice;
	struct cpufreq_policy *cur_policy;
	struct delayed_work work;
	struct cpufreq_frequency_table *freq_table;
	unsigned int freq_lo;
	unsigned int freq_lo_jiffies;
	unsigned int freq_hi_jiffies;
	unsigned int rate_mult;
	int cpu;
	unsigned int sample_type:1;
	/*
	 * percpu mutex that serializes governor limit change with
	 * do_bds_timer invocation. We do not want do_bds_timer to run
	 * when user is changing the governor or limits.
	 */
	struct mutex timer_mutex;
};
static DEFINE_PER_CPU(struct cpu_bds_info_s, od_cpu_bds_info);

static inline void bds_timer_init(struct cpu_bds_info_s *bds_info);
static inline void bds_timer_exit(struct cpu_bds_info_s *bds_info);

static unsigned int bds_enable;	/* number of CPUs using this policy */

/*
 * bds_mutex protects bds_enable in governor start/stop.
 */
static DEFINE_MUTEX(bds_mutex);

static struct workqueue_struct *input_wq;

static DEFINE_PER_CPU(struct work_struct, bds_refresh_work);

static struct bds_tuners {
	unsigned int sampling_rate;
	unsigned int up_threshold;
	unsigned int down_differential;
	unsigned int ignore_nice;
	unsigned int sampling_down_factor;
	int          powersave_bias;
	unsigned int io_is_busy;
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
	unsigned int two_phase_freq;
	unsigned int semi_busy_threshold;
	unsigned int semi_busy_clr_threshold;
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
	unsigned int three_phase_freq;
	unsigned int busy_threshold;
	unsigned int busy_clr_threshold;
#endif
} bds_tuners_ins = {
	.up_threshold = DEF_FREQUENCY_UP_THRESHOLD,
	.sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR,
	.down_differential = DEF_FREQUENCY_DOWN_DIFFERENTIAL,
	.ignore_nice = 0,
	.powersave_bias = 0,
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
	.two_phase_freq = 0,
	.semi_busy_threshold = SEMI_BUSY_THRESHOLD,
	.semi_busy_clr_threshold = SEMI_BUSY_CLR_THRESHOLD,
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
	.three_phase_freq = 0,
	.busy_threshold = BUSY_THRESHOLD,
	.busy_clr_threshold = BUSY_CLR_THRESHOLD,
#endif
};

static inline cputime64_t get_cpu_idle_time_jiffy(unsigned int cpu,
							cputime64_t *wall)
{
	cputime64_t idle_time;
	cputime64_t cur_wall_time;
	cputime64_t busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	busy_time  = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];

	idle_time = (cur_wall_time - busy_time);
	if (wall)
		*wall = (cputime64_t)jiffies_to_usecs(cur_wall_time);

	return (cputime64_t)jiffies_to_usecs(idle_time);
}

static inline cputime64_t get_cpu_idle_time(unsigned int cpu, cputime64_t *wall)
{
	u64 idle_time = get_cpu_idle_time_us(cpu, wall);

	if (idle_time == -1ULL)
		return get_cpu_idle_time_jiffy(cpu, wall);

	return idle_time;
}

static inline cputime64_t get_cpu_iowait_time(unsigned int cpu, cputime64_t *wall)
{
	u64 iowait_time = get_cpu_iowait_time_us(cpu, wall);

	if (iowait_time == -1ULL)
		return 0;

	return iowait_time;
}

/*
 * Find right freq to be set now with powersave_bias on.
 * Returns the freq_hi to be used right now and will set freq_hi_jiffies,
 * freq_lo, and freq_lo_jiffies in percpu area for averaging freqs.
 */
static unsigned int powersave_bias_target(struct cpufreq_policy *policy,
					  unsigned int freq_next,
					  unsigned int relation)
{
	unsigned int freq_req, freq_avg;
	unsigned int freq_hi, freq_lo;
	unsigned int index = 0;
	unsigned int jiffies_total, jiffies_hi, jiffies_lo;
	int freq_reduc;
	struct cpu_bds_info_s *bds_info = &per_cpu(od_cpu_bds_info,
						   policy->cpu);

	if (!bds_info->freq_table) {
		bds_info->freq_lo = 0;
		bds_info->freq_lo_jiffies = 0;
		return freq_next;
	}

	cpufreq_frequency_table_target(policy, bds_info->freq_table, freq_next,
			relation, &index);
	freq_req = bds_info->freq_table[index].frequency;
	freq_reduc = freq_req * bds_tuners_ins.powersave_bias / 1000;
	freq_avg = freq_req - freq_reduc;

	/* Find freq bounds for freq_avg in freq_table */
	index = 0;
	cpufreq_frequency_table_target(policy, bds_info->freq_table, freq_avg,
			CPUFREQ_RELATION_H, &index);
	freq_lo = bds_info->freq_table[index].frequency;
	index = 0;
	cpufreq_frequency_table_target(policy, bds_info->freq_table, freq_avg,
			CPUFREQ_RELATION_L, &index);
	freq_hi = bds_info->freq_table[index].frequency;

	/* Find out how long we have to be in hi and lo freqs */
	if (freq_hi == freq_lo) {
		bds_info->freq_lo = 0;
		bds_info->freq_lo_jiffies = 0;
		return freq_lo;
	}
	jiffies_total = usecs_to_jiffies(bds_tuners_ins.sampling_rate);
	jiffies_hi = (freq_avg - freq_lo) * jiffies_total;
	jiffies_hi += ((freq_hi - freq_lo) / 2);
	jiffies_hi /= (freq_hi - freq_lo);
	jiffies_lo = jiffies_total - jiffies_hi;
	bds_info->freq_lo = freq_lo;
	bds_info->freq_lo_jiffies = jiffies_lo;
	bds_info->freq_hi_jiffies = jiffies_hi;
	return freq_hi;
}

static int badass_powersave_bias_setspeed(struct cpufreq_policy *policy,
					    struct cpufreq_policy *altpolicy,
					    int level)
{
	if (level == POWERSAVE_BIAS_MAXLEVEL) {
		/* maximum powersave; set to lowest frequency */
		__cpufreq_driver_target(policy,
			(altpolicy) ? altpolicy->min : policy->min,
			CPUFREQ_RELATION_L);
		return 1;
	} else if (level == POWERSAVE_BIAS_MINLEVEL) {
		/* minimum powersave; set to highest frequency */
		__cpufreq_driver_target(policy,
			(altpolicy) ? altpolicy->max : policy->max,
			CPUFREQ_RELATION_H);
		return 1;
	}
	return 0;
}

static void badass_powersave_bias_init_cpu(int cpu)
{
	struct cpu_bds_info_s *bds_info = &per_cpu(od_cpu_bds_info, cpu);
	bds_info->freq_table = cpufreq_frequency_get_table(cpu);
	bds_info->freq_lo = 0;
}

static void badass_powersave_bias_init(void)
{
	int i;
	for_each_online_cpu(i) {
		badass_powersave_bias_init_cpu(i);
	}
}

/************************** sysfs interface ************************/

static ssize_t show_sampling_rate_min(struct kobject *kobj,
				      struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", min_sampling_rate);
}

define_one_global_ro(sampling_rate_min);

/* cpufreq_badass Governor Tunables */
#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct attribute *attr, char *buf)              \
{									\
	return sprintf(buf, "%u\n", bds_tuners_ins.object);		\
}
show_one(sampling_rate, sampling_rate);
show_one(io_is_busy, io_is_busy);
show_one(up_threshold, up_threshold);
show_one(down_differential, down_differential);
show_one(sampling_down_factor, sampling_down_factor);
show_one(ignore_nice_load, ignore_nice);

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
show_one(two_phase_freq, two_phase_freq);
show_one(semi_busy_threshold, semi_busy_threshold);
show_one(semi_busy_clr_threshold, semi_busy_clr_threshold);
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
show_one(three_phase_freq, three_phase_freq);
show_one(busy_threshold, busy_threshold);
show_one(busy_clr_threshold, busy_clr_threshold);
#endif

static ssize_t show_powersave_bias
(struct kobject *kobj, struct attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", bds_tuners_ins.powersave_bias);
}

static ssize_t store_sampling_rate(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	bds_tuners_ins.sampling_rate = max(input, min_sampling_rate);
	return count;
}

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
static ssize_t store_two_phase_freq(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	bds_tuners_ins.two_phase_freq = input;

	return count;
}
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
static ssize_t store_three_phase_freq(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	bds_tuners_ins.three_phase_freq = input;

	return count;
}
#endif

static ssize_t store_io_is_busy(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	bds_tuners_ins.io_is_busy = !!input;
	return count;
}

static ssize_t store_up_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_FREQUENCY_UP_THRESHOLD ||
			input < MIN_FREQUENCY_UP_THRESHOLD) {
		return -EINVAL;
	}
	bds_tuners_ins.up_threshold = input;
	return count;
}

static ssize_t store_down_differential(struct kobject *a, struct attribute *b,
		const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input >= bds_tuners_ins.up_threshold ||
			input < MIN_FREQUENCY_DOWN_DIFFERENTIAL) {
		return -EINVAL;
	}

	bds_tuners_ins.down_differential = input;

	return count;
}

static ssize_t store_sampling_down_factor(struct kobject *a,
			struct attribute *b, const char *buf, size_t count)
{
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_FACTOR || input < 1)
		return -EINVAL;
	bds_tuners_ins.sampling_down_factor = input;

	/* Reset down sampling multiplier in case it was active */
	for_each_online_cpu(j) {
		struct cpu_bds_info_s *bds_info;
		bds_info = &per_cpu(od_cpu_bds_info, j);
		bds_info->rate_mult = 1;
	}
	return count;
}

static ssize_t store_ignore_nice_load(struct kobject *a, struct attribute *b,
				      const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	unsigned int j;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	if (input > 1)
		input = 1;

	if (input == bds_tuners_ins.ignore_nice) { /* nothing to do */
		return count;
	}
	bds_tuners_ins.ignore_nice = input;

	/* we need to re-evaluate prev_cpu_idle */
	for_each_online_cpu(j) {
		struct cpu_bds_info_s *bds_info;
		bds_info = &per_cpu(od_cpu_bds_info, j);
		bds_info->prev_cpu_idle = get_cpu_idle_time(j,
						&bds_info->prev_cpu_wall);
		if (bds_tuners_ins.ignore_nice)
			bds_info->prev_cpu_nice = kcpustat_cpu(j).cpustat[CPUTIME_NICE];

	}
	return count;
}

static ssize_t store_powersave_bias(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	int input  = 0;
	int bypass = 0;
	int ret, cpu, reenable_timer, j;
	struct cpu_bds_info_s *bds_info;

	struct cpumask cpus_timer_done;
	cpumask_clear(&cpus_timer_done);

	ret = sscanf(buf, "%d", &input);

	if (ret != 1)
		return -EINVAL;

	if (input >= POWERSAVE_BIAS_MAXLEVEL) {
		input  = POWERSAVE_BIAS_MAXLEVEL;
		bypass = 1;
	} else if (input <= POWERSAVE_BIAS_MINLEVEL) {
		input  = POWERSAVE_BIAS_MINLEVEL;
		bypass = 1;
	}

	if (input == bds_tuners_ins.powersave_bias) {
		/* no change */
		return count;
	}

	reenable_timer = ((bds_tuners_ins.powersave_bias ==
				POWERSAVE_BIAS_MAXLEVEL) ||
				(bds_tuners_ins.powersave_bias ==
				POWERSAVE_BIAS_MINLEVEL));

	bds_tuners_ins.powersave_bias = input;
	if (!bypass) {
		if (reenable_timer) {
			/* reinstate bds timer */
			for_each_online_cpu(cpu) {
				if (lock_policy_rwsem_write(cpu) < 0)
					continue;

				bds_info = &per_cpu(od_cpu_bds_info, cpu);

				for_each_cpu(j, &cpus_timer_done) {
					if (!bds_info->cur_policy) {
						printk(KERN_ERR
						"%s Dbs policy is NULL\n",
						 __func__);
						goto skip_this_cpu;
					}
					if (cpumask_test_cpu(j, bds_info->
							cur_policy->cpus))
						goto skip_this_cpu;
				}

				cpumask_set_cpu(cpu, &cpus_timer_done);
				if (bds_info->cur_policy) {
					/* restart bds timer */
					bds_timer_init(bds_info);
				}
skip_this_cpu:
				unlock_policy_rwsem_write(cpu);
			}
		}
		badass_powersave_bias_init();
	} else {
		/* running at maximum or minimum frequencies; cancel
		   bds timer as periodic load sampling is not necessary */
		for_each_online_cpu(cpu) {
			if (lock_policy_rwsem_write(cpu) < 0)
				continue;

			bds_info = &per_cpu(od_cpu_bds_info, cpu);

			for_each_cpu(j, &cpus_timer_done) {
				if (!bds_info->cur_policy) {
					printk(KERN_ERR
					"%s Dbs policy is NULL\n",
					 __func__);
					goto skip_this_cpu_bypass;
				}
				if (cpumask_test_cpu(j, bds_info->
							cur_policy->cpus))
					goto skip_this_cpu_bypass;
			}

			cpumask_set_cpu(cpu, &cpus_timer_done);

			if (bds_info->cur_policy) {
				/* cpu using badass, cancel bds timer */
				mutex_lock(&bds_info->timer_mutex);
				bds_timer_exit(bds_info);

				badass_powersave_bias_setspeed(
					bds_info->cur_policy,
					NULL,
					input);

				mutex_unlock(&bds_info->timer_mutex);
			}
skip_this_cpu_bypass:
			unlock_policy_rwsem_write(cpu);
		}
	}

	return count;
}

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
static ssize_t store_semi_busy_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > bds_tuners_ins.busy_threshold ||
			input <= 0 || input > bds_tuners_ins.busy_clr_threshold) {
		return -EINVAL;
	}
	bds_tuners_ins.semi_busy_threshold = input;
	return count;
}
static ssize_t store_semi_busy_clr_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > bds_tuners_ins.busy_clr_threshold ||
			input < 0 || input > bds_tuners_ins.semi_busy_threshold) {
		return -EINVAL;
	}
	bds_tuners_ins.semi_busy_clr_threshold = input;
	return count;
}
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
static ssize_t store_busy_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_IDLE_COUNTER ||
			input <= 0 || input < bds_tuners_ins.semi_busy_threshold ||
			input < bds_tuners_ins.busy_clr_threshold) {
		return -EINVAL;
	}
	bds_tuners_ins.busy_threshold = input;
	return count;
}
static ssize_t store_busy_clr_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > bds_tuners_ins.busy_threshold ||
			input <= 0 || input < bds_tuners_ins.semi_busy_clr_threshold) {
		return -EINVAL;
	}
	bds_tuners_ins.busy_clr_threshold = input;
	return count;
}
#endif

define_one_global_rw(sampling_rate);
define_one_global_rw(io_is_busy);
define_one_global_rw(up_threshold);
define_one_global_rw(down_differential);
define_one_global_rw(sampling_down_factor);
define_one_global_rw(ignore_nice_load);
define_one_global_rw(powersave_bias);
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
define_one_global_rw(two_phase_freq);
define_one_global_rw(semi_busy_threshold);
define_one_global_rw(semi_busy_clr_threshold);
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
define_one_global_rw(three_phase_freq);
define_one_global_rw(busy_threshold);
define_one_global_rw(busy_clr_threshold);
#endif

static struct attribute *bds_attributes[] = {
	&sampling_rate_min.attr,
	&sampling_rate.attr,
	&up_threshold.attr,
	&down_differential.attr,
	&sampling_down_factor.attr,
	&ignore_nice_load.attr,
	&powersave_bias.attr,
	&io_is_busy.attr,
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
	&two_phase_freq.attr,
	&semi_busy_threshold.attr,
	&semi_busy_clr_threshold.attr,
#endif
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
	&three_phase_freq.attr,
	&busy_threshold.attr,
	&busy_clr_threshold.attr,
#endif
	NULL
};

static struct attribute_group bds_attr_group = {
	.attrs = bds_attributes,
	.name = "badass",
};

/************************** sysfs end ************************/

static void bds_freq_increase(struct cpufreq_policy *p, unsigned int freq)
{
	if (bds_tuners_ins.powersave_bias)
		freq = powersave_bias_target(p, freq, CPUFREQ_RELATION_H);
	else if (p->cur == p->max)
		return;

	__cpufreq_driver_target(p, freq, bds_tuners_ins.powersave_bias ?
			CPUFREQ_RELATION_L : CPUFREQ_RELATION_H);
}

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
int set_two_phase_freq_badass(int cpufreq)
{
	bds_tuners_ins.two_phase_freq = cpufreq;
	return 0;
}
#endif

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
int set_three_phase_freq_badass(int cpufreq)
{
	bds_tuners_ins.three_phase_freq = cpufreq;
	return 0;
}
#endif

static void bds_check_cpu(struct cpu_bds_info_s *this_bds_info)
{
	unsigned int max_load_freq;

	struct cpufreq_policy *policy;
	unsigned int j;
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
	static unsigned int phase = 0;
	static unsigned int counter = 0;
	unsigned int new_phase_max = 0;
#endif

	this_bds_info->freq_lo = 0;
	policy = this_bds_info->cur_policy;

	/*
	 * Every sampling_rate, we check, if current idle time is less
	 * than 20% (default), then we try to increase frequency
	 * Every sampling_rate, we look for a the lowest
	 * frequency which can sustain the load while keeping idle time over
	 * 30%. If such a frequency exist, we try to decrease to this frequency.
	 *
	 * Any frequency increase takes it to the maximum frequency.
	 * Frequency reduction happens at minimum steps of
	 * 5% (default) of current frequency
	 */

	/* Get Absolute Load - in terms of freq */
	max_load_freq = 0;

	for_each_cpu(j, policy->cpus) {
		struct cpu_bds_info_s *j_bds_info;
		cputime64_t cur_wall_time, cur_idle_time, cur_iowait_time;
		unsigned int idle_time, wall_time, iowait_time;
		unsigned int load, load_freq;
		int freq_avg;

		j_bds_info = &per_cpu(od_cpu_bds_info, j);

		cur_idle_time = get_cpu_idle_time(j, &cur_wall_time);
		cur_iowait_time = get_cpu_iowait_time(j, &cur_wall_time);

		wall_time = (unsigned int) (cur_wall_time - j_bds_info->prev_cpu_wall);
		j_bds_info->prev_cpu_wall = cur_wall_time;

		idle_time = (unsigned int) (cur_idle_time - j_bds_info->prev_cpu_idle);
		j_bds_info->prev_cpu_idle = cur_idle_time;

		iowait_time = (unsigned int) (cur_iowait_time - j_bds_info->prev_cpu_iowait);
		j_bds_info->prev_cpu_iowait = cur_iowait_time;

		if (bds_tuners_ins.ignore_nice) {
			cputime64_t cur_nice;
			unsigned long cur_nice_jiffies;

			cur_nice = (kcpustat_cpu(j).cpustat[CPUTIME_NICE] - j_bds_info->prev_cpu_nice);
			/*
			 * Assumption: nice time between sampling periods will
			 * be less than 2^32 jiffies for 32 bit sys
			 */
			cur_nice_jiffies = (unsigned long)
					cputime64_to_jiffies64(cur_nice);

			j_bds_info->prev_cpu_nice = kcpustat_cpu(j).cpustat[CPUTIME_NICE];
			idle_time += jiffies_to_usecs(cur_nice_jiffies);
		}

		/*
		 * For the purpose of badass, waiting for disk IO is an
		 * indication that you're performance critical, and not that
		 * the system is actually idle. So subtract the iowait time
		 * from the cpu idle time.
		 */

		if (bds_tuners_ins.io_is_busy && idle_time >= iowait_time)
			idle_time -= iowait_time;

		if (unlikely(!wall_time || wall_time < idle_time))
			continue;

		load = 100 * (wall_time - idle_time) / wall_time;

		freq_avg = __cpufreq_driver_getavg(policy, j);
		if (freq_avg <= 0)
			freq_avg = policy->cur;

		load_freq = load * freq_avg;
		if (load_freq > max_load_freq)
			max_load_freq = load_freq;
	}

	/* Check for frequency increase */
	if (max_load_freq > bds_tuners_ins.up_threshold * policy->cur) {
		/* If switching to max speed, apply sampling_down_factor */
#ifndef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
		if (policy->cur < policy->max)
			this_bds_info->rate_mult =
				bds_tuners_ins.sampling_down_factor;
		bds_freq_increase(policy, policy->max);
#else
		if (counter < 0)
			counter = 0;

		if (counter < MAX_IDLE_COUNTER) {
			if ((counter < bds_tuners_ins.semi_busy_threshold) && (phase == 0))
				counter += 4;
			else
				counter++;
			if ((counter > bds_tuners_ins.semi_busy_threshold) && (phase < 1)) {
				/* change to semi-busy phase (3) */
				phase = 1;
			}
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
			if ((counter > bds_tuners_ins.busy_threshold) && (phase < 2)) {
				/* change to busy phase (full) */
				phase = 2;
			}
#endif
		}
/*
 * Debug output for cpu control. Still needed for finetuning.
 *
 *		printk(KERN_INFO "badass: cpu_phase: '%i' |	\
 *			 busy_counter: '%i'", phase, counter);
 */

		if ((bds_tuners_ins.two_phase_freq != 0) && (phase == 0)) {
			/* idle phase */
			if (bds_tuners_ins.two_phase_freq > (policy->max*PHASE_2_PERCENT/100)) {
				new_phase_max = (policy->max*PHASE_2_PERCENT/100);
			} else {
				new_phase_max = bds_tuners_ins.two_phase_freq;
			}
			bds_freq_increase(policy, new_phase_max);
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
		} else if ((bds_tuners_ins.three_phase_freq != 0) && (phase == 1)) {
			/* semi-busy phase */
			if (bds_tuners_ins.three_phase_freq > (policy->max*PHASE_3_PERCENT/100)) {
				new_phase_max = (policy->max*PHASE_3_PERCENT/100);
			} else {
				new_phase_max = bds_tuners_ins.three_phase_freq;
			}
			bds_freq_increase(policy, new_phase_max);
#endif
		} else {
			/* busy phase */
			if (policy->cur < policy->max)
				this_bds_info->rate_mult =
					bds_tuners_ins.sampling_down_factor;
			bds_freq_increase(policy, policy->max);
		}
#endif
		return;
	}
#ifdef CONFIG_CPU_FREQ_GOV_BADASS_2_PHASE
	if (counter > 0) {
		if (counter >= DECREASE_IDLE_COUNTER)
			counter -= DECREASE_IDLE_COUNTER;
		if ((counter > 0) && (counter < DECREASE_IDLE_COUNTER))
			counter--;

#ifdef CONFIG_CPU_FREQ_GOV_BADASS_3_PHASE
		if ((counter < bds_tuners_ins.busy_clr_threshold) && (phase > 1)) {
			/* change to semi busy phase */
			phase = 1;
		}
#endif
		if ((counter < bds_tuners_ins.semi_busy_clr_threshold) && (phase > 0)) {
			/* change to idle phase */
			phase = 0;
		}
	}
#endif

	/* Check for frequency decrease */
	/* if we cannot reduce the frequency anymore, break out early */
	if (policy->cur == policy->min)
		return;

	/*
	 * The optimal frequency is the frequency that is the lowest that
	 * can support the current CPU usage without triggering the up
	 * policy. To be safe, we focus 10 points under the threshold.
	 */
	if (max_load_freq <
	    (bds_tuners_ins.up_threshold - bds_tuners_ins.down_differential) *
	     policy->cur) {
		unsigned int freq_next;
		freq_next = max_load_freq /
				(bds_tuners_ins.up_threshold -
				 bds_tuners_ins.down_differential);

		/* No longer fully busy, reset rate_mult */
		this_bds_info->rate_mult = 1;

		if (freq_next < policy->min)
			freq_next = policy->min;

		if (!bds_tuners_ins.powersave_bias) {
			__cpufreq_driver_target(policy, freq_next,
					CPUFREQ_RELATION_L);
		} else {
			int freq = powersave_bias_target(policy, freq_next,
					CPUFREQ_RELATION_L);
			__cpufreq_driver_target(policy, freq,
				CPUFREQ_RELATION_L);
		}
	}
}

static void do_bds_timer(struct work_struct *work)
{
	struct cpu_bds_info_s *bds_info =
		container_of(work, struct cpu_bds_info_s, work.work);
	unsigned int cpu = bds_info->cpu;
	int sample_type = bds_info->sample_type;

	int delay;

	mutex_lock(&bds_info->timer_mutex);

	/* Common NORMAL_SAMPLE setup */
	bds_info->sample_type = BDS_NORMAL_SAMPLE;
	if (!bds_tuners_ins.powersave_bias ||
	    sample_type == BDS_NORMAL_SAMPLE) {
		bds_check_cpu(bds_info);
		if (bds_info->freq_lo) {
			/* Setup timer for SUB_SAMPLE */
			bds_info->sample_type = BDS_SUB_SAMPLE;
			delay = bds_info->freq_hi_jiffies;
		} else {
			/* We want all CPUs to do sampling nearly on
			 * same jiffy
			 */
			delay = usecs_to_jiffies(bds_tuners_ins.sampling_rate
				* bds_info->rate_mult);

			if (num_online_cpus() > 1)
				delay -= jiffies % delay;
		}
	} else {
		__cpufreq_driver_target(bds_info->cur_policy,
			bds_info->freq_lo, CPUFREQ_RELATION_H);
		delay = bds_info->freq_lo_jiffies;
	}
	schedule_delayed_work_on(cpu, &bds_info->work, delay);
	mutex_unlock(&bds_info->timer_mutex);
}

static inline void bds_timer_init(struct cpu_bds_info_s *bds_info)
{
	/* We want all CPUs to do sampling nearly on same jiffy */
	int delay = usecs_to_jiffies(bds_tuners_ins.sampling_rate);

	if (num_online_cpus() > 1)
		delay -= jiffies % delay;

	bds_info->sample_type = BDS_NORMAL_SAMPLE;
	INIT_DELAYED_WORK_DEFERRABLE(&bds_info->work, do_bds_timer);
	schedule_delayed_work_on(bds_info->cpu, &bds_info->work, delay);
}

static inline void bds_timer_exit(struct cpu_bds_info_s *bds_info)
{
	cancel_delayed_work_sync(&bds_info->work);
}

/*
 * Not all CPUs want IO time to be accounted as busy; this dependson how
 * efficient idling at a higher frequency/voltage is.
 * Pavel Machek says this is not so for various generations of AMD and old
 * Intel systems.
 * Mike Chan (androidlcom) calis this is also not true for ARM.
 * Because of this, whitelist specific known (series) of CPUs by default, and
 * leave all others up to the user.
 */
static int should_io_be_busy(void)
{
#if defined(CONFIG_X86)
	/*
	 * For Intel, Core 2 (model 15) andl later have an efficient idle.
	 */
	if (boot_cpu_data.x86_vendor == X86_VENDOR_INTEL &&
	    boot_cpu_data.x86 == 6 &&
	    boot_cpu_data.x86_model >= 15)
		return 1;
#endif
	return 0;
}

static void bds_refresh_callback(struct work_struct *unused)
{
	struct cpufreq_policy *policy;
	struct cpu_bds_info_s *this_bds_info;
	unsigned int cpu = smp_processor_id();

	if (lock_policy_rwsem_write(cpu) < 0)
		return;

	this_bds_info = &per_cpu(od_cpu_bds_info, cpu);
	policy = this_bds_info->cur_policy;
	if (!policy) {
		/* CPU not using badass governor */
		unlock_policy_rwsem_write(cpu);
		return;
	}

	if (policy->cur < policy->max) {
		policy->cur = policy->max;

		__cpufreq_driver_target(policy, policy->max,
					CPUFREQ_RELATION_L);
		this_bds_info->prev_cpu_idle = get_cpu_idle_time(cpu,
				&this_bds_info->prev_cpu_wall);
	}
	unlock_policy_rwsem_write(cpu);
}

static unsigned int enable_bds_input_event;
static void bds_input_event(struct input_handle *handle, unsigned int type,
		unsigned int code, int value)
{
	int i;

	if (enable_bds_input_event) {

		if ((bds_tuners_ins.powersave_bias == POWERSAVE_BIAS_MAXLEVEL) ||
			(bds_tuners_ins.powersave_bias == POWERSAVE_BIAS_MINLEVEL)) {
			/* nothing to do */
			return;
		}

		for_each_online_cpu(i) {
			queue_work_on(i, input_wq, &per_cpu(bds_refresh_work, i));
		}
	}
}

static int bds_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpufreq";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;

	return 0;
err1:
	input_unregister_handle(handle);
err2:
	kfree(handle);
	return error;
}

static void bds_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id bds_ids[] = {
	{ .driver_info = 1 },
	{ },
};

static struct input_handler bds_input_handler = {
	.event		= bds_input_event,
	.connect	= bds_input_connect,
	.disconnect	= bds_input_disconnect,
	.name		= "cpufreq_bad",
	.id_table	= bds_ids,
};

static int cpufreq_governor_bds(struct cpufreq_policy *policy,
				   unsigned int event)
{
	unsigned int cpu = policy->cpu;
	struct cpu_bds_info_s *this_bds_info;
	unsigned int j;
	int rc;

	this_bds_info = &per_cpu(od_cpu_bds_info, cpu);

	switch (event) {
	case CPUFREQ_GOV_START:
		if ((!cpu_online(cpu)) || (!policy->cur))
			return -EINVAL;

		mutex_lock(&bds_mutex);

		bds_enable++;
		for_each_cpu(j, policy->cpus) {
			struct cpu_bds_info_s *j_bds_info;
			j_bds_info = &per_cpu(od_cpu_bds_info, j);
			j_bds_info->cur_policy = policy;

			j_bds_info->prev_cpu_idle = get_cpu_idle_time(j,
						&j_bds_info->prev_cpu_wall);
			if (bds_tuners_ins.ignore_nice) {
				j_bds_info->prev_cpu_nice =
						kcpustat_cpu(j).cpustat[CPUTIME_NICE];
			}
		}
		this_bds_info->cpu = cpu;
		this_bds_info->rate_mult = 1;
		badass_powersave_bias_init_cpu(cpu);
		/*
		 * Start the timerschedule work, when this governor
		 * is used for first time
		 */
		if (bds_enable == 1) {
			unsigned int latency;

			rc = sysfs_create_group(cpufreq_global_kobject,
						&bds_attr_group);
			if (rc) {
				mutex_unlock(&bds_mutex);
				return rc;
			}

			/* policy latency is in nS. Convert it to uS first */
			latency = policy->cpuinfo.transition_latency / 1000;
			if (latency == 0)
				latency = 1;
			/* Bring kernel and HW constraints together */
			min_sampling_rate = max(min_sampling_rate,
					MIN_LATENCY_MULTIPLIER * latency);
			bds_tuners_ins.sampling_rate =
				max(min_sampling_rate,
				    latency * LATENCY_MULTIPLIER);
			bds_tuners_ins.io_is_busy = should_io_be_busy();
		}
		if (!cpu)
			rc = input_register_handler(&bds_input_handler);
		mutex_unlock(&bds_mutex);

		mutex_init(&this_bds_info->timer_mutex);

		if (!badass_powersave_bias_setspeed(
					this_bds_info->cur_policy,
					NULL,
					bds_tuners_ins.powersave_bias))
			bds_timer_init(this_bds_info);
		break;

	case CPUFREQ_GOV_STOP:
		bds_timer_exit(this_bds_info);

		mutex_lock(&bds_mutex);
		mutex_destroy(&this_bds_info->timer_mutex);
		bds_enable--;
		/* If device is being removed, policy is no longer
		 * valid. */
		this_bds_info->cur_policy = NULL;
		if (!cpu)
			input_unregister_handler(&bds_input_handler);
		mutex_unlock(&bds_mutex);
		if (!bds_enable)
			sysfs_remove_group(cpufreq_global_kobject,
					   &bds_attr_group);

		break;

	case CPUFREQ_GOV_LIMITS:
		mutex_lock(&this_bds_info->timer_mutex);
		if (policy->max < this_bds_info->cur_policy->cur)
			__cpufreq_driver_target(this_bds_info->cur_policy,
				policy->max, CPUFREQ_RELATION_H);
		else if (policy->min > this_bds_info->cur_policy->cur)
			__cpufreq_driver_target(this_bds_info->cur_policy,
				policy->min, CPUFREQ_RELATION_L);
		else if (bds_tuners_ins.powersave_bias != 0)
			badass_powersave_bias_setspeed(
				this_bds_info->cur_policy,
				policy,
				bds_tuners_ins.powersave_bias);
		mutex_unlock(&this_bds_info->timer_mutex);
		break;
	}
	return 0;
}

static int __init cpufreq_gov_bds_init(void)
{
	cputime64_t wall;
	u64 idle_time;
	unsigned int i;
	int cpu = get_cpu();

	idle_time = get_cpu_idle_time_us(cpu, &wall);
	put_cpu();
	if (idle_time != -1ULL) {
		/* Idle micro accounting is supported. Use finer thresholds */
		bds_tuners_ins.up_threshold = MICRO_FREQUENCY_UP_THRESHOLD;
		bds_tuners_ins.down_differential =
					MICRO_FREQUENCY_DOWN_DIFFERENTIAL;
		/*
		 * In no_hz/micro accounting case we set the minimum frequency
		 * not depending on HZ, but fixed (very low). The deferred
		 * timer might skip some samples if idle/sleeping as needed.
		*/
		min_sampling_rate = MICRO_FREQUENCY_MIN_SAMPLE_RATE;
	} else {
		/* For correct statistics, we need 10 ticks for each measure */
		min_sampling_rate =
			MIN_SAMPLING_RATE_RATIO * jiffies_to_usecs(10);
	}

	input_wq = create_workqueue("iewq");
	if (!input_wq) {
		printk(KERN_ERR "Failed to create iewq workqueue\n");
		return -EFAULT;
	}
	for_each_possible_cpu(i) {
		INIT_WORK(&per_cpu(bds_refresh_work, i), bds_refresh_callback);
	}

	return cpufreq_register_governor(&cpufreq_gov_badass);
}

static void __exit cpufreq_gov_bds_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_badass);
	destroy_workqueue(input_wq);
}

static int set_enable_bds_input_event_param(const char *val, struct kernel_param *kp)
{
	int ret = 0;

	ret = param_set_uint(val, kp);
	if (ret)
		pr_err("%s: error setting value %d\n", __func__, ret);

	return ret;
}
module_param_call(enable_bds_input_event, set_enable_bds_input_event_param, param_get_uint,
		&enable_bds_input_event, S_IWUSR | S_IRUGO);


MODULE_AUTHOR("Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>");
MODULE_AUTHOR("Alexey Starikovskiy <alexey.y.starikovskiy@intel.com>");
MODULE_AUTHOR("Dennis Rassmann <showp1984@gmail.com>");
MODULE_DESCRIPTION("'cpufreq_badass' - A badass cpufreq governor based on ondemand");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_BADASS
fs_initcall(cpufreq_gov_bds_init);
#else
module_init(cpufreq_gov_bds_init);
#endif
module_exit(cpufreq_gov_bds_exit);
