/*
 * Copyright (C) 2015 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm_qos.h>
#include <linux/sched.h>

enum Cluster {
	CLUSTER_LITTLE = 0,
	CLUSTER_BIG,
	CLUSTER_MAX_VALUE,
};

enum Cmd {
	CMD_USE_CLUSTER_BIG = 0,
	CMD_USE_CLUSTER_DEFAULT,
	CMD_MAX_VALUE,
};

#define CPUS_PER_CLUSTER (NR_CPUS / CLUSTER_MAX_VALUE)

#define set_qos(req, pm_qos_class, value) {				\
	if (value) {							\
		if (pm_qos_request_active(req))				\
			pm_qos_update_request(req, value);		\
		else							\
			pm_qos_add_request(req, pm_qos_class, value);	\
	}								\
	else								\
		if (pm_qos_request_active(req))				\
			pm_qos_remove_request(req);			\
}

static int sw_cpu;
static int nr_big_cluster_requests = 0;
static cpumask_t tzdev_cpu_mask[CLUSTER_MAX_VALUE];
static DEFINE_MUTEX(tzdev_core_migration_lock);

static struct pm_qos_request tz_qos;

static int tzdev_cpu_callback(struct notifier_block *nfb,
			unsigned long action, void *hcpu);

static struct notifier_block tzdev_cpu_notifier = {
	.notifier_call = tzdev_cpu_callback,
};

static int tzdev_get_destination_cpu(void)
{
	int cpu;
	cpumask_t *migration_mask;

	if (nr_big_cluster_requests)
		migration_mask = &tzdev_cpu_mask[CLUSTER_BIG];
	else
		migration_mask = &tzdev_cpu_mask[CLUSTER_LITTLE];

	cpu = cpumask_first_and(migration_mask, cpu_active_mask);
	if (cpu >= nr_cpu_ids) {
		pr_warn("No active CPUs ready for migration, migration failed.\n");
		return -1;
	}
	pr_notice("Found destination CPU:%d.\n", cpu);
	return cpu;
}

static int tzdev_should_migrate(void)
{
	if (smp_processor_id() != sw_cpu)
		return 1;
	if (cpumask_test_cpu(sw_cpu, &tzdev_cpu_mask[CLUSTER_LITTLE]) &&
			nr_big_cluster_requests)
		/* In case secure world is running on one of the
		 * LITTLE cores and we have at least one pending
		 * request to move secure world to big core,
		 * tzdev should do core migration */
		return 1;
	else if (cpumask_test_cpu(sw_cpu, &tzdev_cpu_mask[CLUSTER_BIG]) &&
			!nr_big_cluster_requests)
		/* In case secure world is running on one of the
		 * big cores and we don't have any request to move
		 * secure world to big core, tzdev should do core
		 * migration to move secure world to LITTLE core. */
		return 1;
	return 0;
}

static struct task_struct *tzdev_get_next_thread(cpumask_t *mask)
{
	struct task_struct *child = NULL, *tmp = current;

	read_lock(&tasklist_lock);
	while_each_thread(current, tmp) {
		if (cpumask_equal(tsk_cpus_allowed(tmp), mask))
			continue;
		child = tmp;
		get_task_struct(child);
		break;
	}
	read_unlock(&tasklist_lock);
	return child;
}

static void tzdev_migrate_threads(int cpu)
{
	struct task_struct *thread;
	cpumask_t next_cpumask;

	cpumask_clear(&next_cpumask);
	cpumask_set_cpu(cpu, &next_cpumask);

	while ((thread = tzdev_get_next_thread(&next_cpumask))) {
		pr_notice("Migrate thread pid = %d to cpu = %d\n", thread->pid, cpu);
		/* We shouldn't fail here because of we wrap this code by
		 * get_online_cpus() / put_online_cpus() */
		BUG_ON(set_cpus_allowed(thread, next_cpumask));
		put_task_struct(thread);
	}
	BUG_ON(set_cpus_allowed(current, next_cpumask));
}

static int tzdev_cpu_callback(struct notifier_block *nfb,
			unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int new_cpu;

	switch (action) {
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		get_online_cpus();
		mutex_lock(&tzdev_core_migration_lock);
		if (sw_cpu == cpu) {
			pr_notice("Current CPU = %d is going down.\n", cpu);
			new_cpu = tzdev_get_destination_cpu();
			if (new_cpu < 0 || new_cpu == sw_cpu) {
				new_cpu = cpumask_any(cpu_active_mask);
				BUG_ON(new_cpu >= nr_cpu_ids || new_cpu == sw_cpu);
				pr_notice("No big CPU available for migration,"
					  "move Secure OS to CPU = %d\n", new_cpu);
			}
			sw_cpu = new_cpu;
			mutex_unlock(&tzdev_core_migration_lock);
			/* Migration is necessary, so do it
			 * and return */
			tzdev_migrate_threads(new_cpu);
			put_online_cpus();
			return NOTIFY_OK;
		}
		mutex_unlock(&tzdev_core_migration_lock);
		put_online_cpus();
		return NOTIFY_OK;
	}
	return NOTIFY_OK;
}

void tzdev_init_migration(void)
{
	cpumask_setall(&tzdev_cpu_mask[CLUSTER_BIG]);
	cpumask_clear(&tzdev_cpu_mask[CLUSTER_LITTLE]);

	if (strlen(CONFIG_HMP_FAST_CPU_MASK))
		cpulist_parse(CONFIG_HMP_FAST_CPU_MASK, &tzdev_cpu_mask[CLUSTER_BIG]);
	else
		pr_notice("All CPUs are equal, core migration will do nothing.\n");
	cpumask_andnot(&tzdev_cpu_mask[CLUSTER_LITTLE], cpu_present_mask,
			&tzdev_cpu_mask[CLUSTER_BIG]);
	register_cpu_notifier(&tzdev_cpu_notifier);
}

void tzdev_fini_migration(void)
{
	unregister_cpu_notifier(&tzdev_cpu_notifier);
}

int tzdev_migrate(void)
{
	int cpu, ret = 0;

	get_online_cpus();
	mutex_lock(&tzdev_core_migration_lock);
	if (!tzdev_should_migrate())
		goto exit;
	cpu = tzdev_get_destination_cpu();
	if (cpu < 0) {
		ret = -EFAULT;
		goto exit;
	}
	sw_cpu = cpu;
	mutex_unlock(&tzdev_core_migration_lock);

	tzdev_migrate_threads(cpu);
	put_online_cpus();

	return 0;
exit:
	mutex_unlock(&tzdev_core_migration_lock);
	put_online_cpus();
	return ret;
}

int tzdev_migration_request(unsigned long arg)
{
	int ret = 0;

	mutex_lock(&tzdev_core_migration_lock);
	switch (arg) {
	case CMD_USE_CLUSTER_BIG:
		if (nr_big_cluster_requests == 0) {
			set_qos(&tz_qos, PM_QOS_CLUSTER1_FREQ_MIN, PM_QOS_CLUSTER1_FREQ_MAX_DEFAULT_VALUE);
			pr_notice("Set QoS request: max frequency for big cluster.\n");
		}
		nr_big_cluster_requests++;
		break;
	case CMD_USE_CLUSTER_DEFAULT:
		if (nr_big_cluster_requests == 0) {
			pr_err("Unexpected value of migration requests"
				"quantity while processing CORE_LITTLE migration:"
				"nr_big_cluster_requests = %d\n", nr_big_cluster_requests);
			ret = -EINVAL;
		} else {
			nr_big_cluster_requests--;
			if (nr_big_cluster_requests == 0) {
				pr_notice("Set QoS request: min frequency for big cluster.\n");
				set_qos(&tz_qos, PM_QOS_CLUSTER1_FREQ_MIN, 0);
			}
		}
		break;
	default:
		pr_err("Wrong request: %lu\n", arg);
		ret = -EINVAL;
	}
	mutex_unlock(&tzdev_core_migration_lock);
	return ret;
}
