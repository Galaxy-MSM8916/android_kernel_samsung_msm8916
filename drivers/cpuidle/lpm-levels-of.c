/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/moduleparam.h>
#include "lpm-levels.h"

enum lpm_type {
	IDLE = 0,
	SUSPEND,
	LPM_TYPE_NR
};

struct lpm_type_str {
	enum lpm_type type;
	char *str;
};

static const struct lpm_type_str lpm_types[] = {
	{IDLE, "idle_enabled"},
	{SUSPEND, "suspend_enabled"},
};

static struct lpm_level_avail *cpu_level_available[NR_CPUS];
static struct platform_device *lpm_pdev;

static void *get_avail_val(struct kobject *kobj, struct kobj_attribute *attr)
{
	void *arg = NULL;
	struct lpm_level_avail *avail = NULL;

	if (!strcmp(attr->attr.name, lpm_types[IDLE].str)) {
		avail = container_of(attr, struct lpm_level_avail,
					idle_enabled_attr);
		arg = (void *) &avail->idle_enabled;
	} else if (!strcmp(attr->attr.name, lpm_types[SUSPEND].str)) {
		avail = container_of(attr, struct lpm_level_avail,
					suspend_enabled_attr);
		arg = (void *) &avail->suspend_enabled;
	}

	return arg;
}

#if defined(CONFIG_ARCH_MSM8939)|| defined (CONFIG_ARCH_MSM8929)
static struct lpm_cluster *performance_cluster;
static struct lpm_cluster *power_cluster;

static int cpu_lpm_set_mode(int cpu_no, int power_level, bool on)
{
	int ret = 0, mode=0;
	struct kernel_param kp;
	struct lpm_level_avail *level_list = NULL;
	level_list = cpu_level_available[cpu_no];

	if (power_level == 0) /*  WFI */ {
		mode = MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT;
	} else if (power_level == 1) /*  SPC */ {
		mode = MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE;
	} else if (power_level == 2) /*  PC  */ {
		mode = MSM_PM_SLEEP_MODE_POWER_COLLAPSE;
	} else {
		pr_err("Bad mode for cpu lpm mode!\n");
		return -EINVAL;
	}

	kp.arg = &level_list[mode].idle_enabled;;
	if (on)
		ret = param_set_bool("Y", &kp);
	else
		ret = param_set_bool("N", &kp);

	return ret;
}

int lpm_set_mode(u8 cpu_mask, u32 power_level, bool on)
{
	int cpu = 0, j = 0, k =0;
	int ret = 0;

	for_each_possible_cpu(cpu) {
		if (cpu_mask & (1 << cpu)) {
			for (j=cpu*4, k=0; k<3; j++,k++) {
				if (power_level & (1 << j)) {
					ret = cpu_lpm_set_mode(cpu, k, on);
					if (ret)
						return ret;
				}
			}
		}
	}

	return ret;
}
EXPORT_SYMBOL_GPL(lpm_set_mode);

ssize_t lpm_bundle_show(struct kobject *kobj, struct kobj_attribute *attr,
				char *buf)
{
	int i = 0, j = 0;
	int cpu;
	u32 len = 0, size = 0;
	struct lpm_level_avail *level_list = NULL;
	int idle_enabled_list[3] = { 0 , };

	len = snprintf(&buf[size], PAGE_SIZE - size,
					"[CPUIDLE] %s, %s\n",__func__,attr->attr.name);
	size += len;
	if (!performance_cluster || !power_cluster) {
		pr_err("[LPM] Why null????\n");
		return size;
	}

	for (i = 0; i < performance_cluster->nlevels; i++) {
		len = snprintf(&buf[size], PAGE_SIZE - size, "%s idle_enabled : %d\n",
						performance_cluster->levels[i].level_name,
						performance_cluster->levels[i].available.idle_enabled);
		size += len;
	}

	for_each_cpu(cpu, &performance_cluster->child_cpus) {
		level_list = cpu_level_available[cpu];
		idle_enabled_list[0] =
				level_list[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].idle_enabled;
		idle_enabled_list[1] =
				level_list[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].idle_enabled;
		idle_enabled_list[2] =
				level_list[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled;
		for (j = 0; j < performance_cluster->cpu->nlevels; j++) {
			len = snprintf(&buf[size], PAGE_SIZE - size,
					"CPU%d, name:%s idle_enabled:%d\n",
					cpu,performance_cluster->cpu->levels[j].name,
					idle_enabled_list[j]);
			size += len;
		}
	}

	len = snprintf(&buf[size], PAGE_SIZE - size,
			"[LPM] %s Cluster :\n",power_cluster->cluster_name);
	size += len;
	for (i = 0; i < power_cluster->nlevels; i++) {
		len = snprintf(&buf[size], PAGE_SIZE - size, "%s idle_enabled : %d\n",
				power_cluster->levels[i].level_name,
				power_cluster->levels[i].available.idle_enabled);
		size += len;
	}

	for_each_cpu(cpu, &power_cluster->child_cpus) {
		level_list = cpu_level_available[cpu];
		idle_enabled_list[0] =
				level_list[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].idle_enabled;
		idle_enabled_list[1] =
				level_list[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE].idle_enabled;
		idle_enabled_list[2] =
				level_list[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled;
		for (j = 0; j < power_cluster->cpu->nlevels; j++) {
			len = snprintf(&buf[size], PAGE_SIZE - size,
					"CPU%d, name:%s  idle_enabled:%d\n",
					cpu,power_cluster->cpu->levels[j].name,
					idle_enabled_list[j]);
			size += len;
		}
	}


	return size;
}

static struct kobj_attribute lpm_bundle_attribute =
		__ATTR(lpm_bundle, 0440, lpm_bundle_show, NULL);
#endif

ssize_t lpm_enable_show(struct kobject *kobj, struct kobj_attribute *attr,
				char *buf)
{
	int ret = 0;
	struct kernel_param kp;

	kp.arg = get_avail_val(kobj, attr);
	ret = param_get_bool(buf, &kp);
	if (ret > 0) {
		strlcat(buf, "\n", PAGE_SIZE);
		ret++;
	}

	return ret;
}

ssize_t lpm_enable_store(struct kobject *kobj, struct kobj_attribute *attr,
				const char *buf, size_t len)
{
	int ret = 0;
	struct kernel_param kp;

	kp.arg = get_avail_val(kobj, attr);
	ret = param_set_bool(buf, &kp);

	return ret ? ret : len;
}

static int create_lvl_avail_nodes(const char *name,
			struct kobject *parent, struct lpm_level_avail *avail)
{
	struct attribute_group *attr_group = NULL;
	struct attribute **attr = NULL;
	struct kobject *kobj = NULL;
	int ret = 0;

	kobj = kobject_create_and_add(name, parent);
	if (!kobj)
		return -ENOMEM;

	attr_group = devm_kzalloc(&lpm_pdev->dev, sizeof(*attr_group),
					GFP_KERNEL);
	if (!attr_group) {
		ret = -ENOMEM;
		goto failed;
	}

	attr = devm_kzalloc(&lpm_pdev->dev,
		sizeof(*attr) * (LPM_TYPE_NR + 1), GFP_KERNEL);
	if (!attr) {
		ret = -ENOMEM;
		goto failed;
	}

	avail->idle_enabled_attr.attr.name = lpm_types[IDLE].str;
	avail->idle_enabled_attr.attr.mode = 0644;
	avail->idle_enabled_attr.show = lpm_enable_show;
	avail->idle_enabled_attr.store = lpm_enable_store;

	avail->suspend_enabled_attr.attr.name = lpm_types[SUSPEND].str;
	avail->suspend_enabled_attr.attr.mode = 0644;
	avail->suspend_enabled_attr.show = lpm_enable_show;
	avail->suspend_enabled_attr.store = lpm_enable_store;

	attr[0] = &avail->idle_enabled_attr.attr;
	attr[1] = &avail->suspend_enabled_attr.attr;
	attr[2] = NULL;
	attr_group->attrs = attr;

	ret = sysfs_create_group(kobj, attr_group);
	if (ret) {
		ret = -ENOMEM;
		goto failed;
	}

	avail->idle_enabled = true;
	avail->suspend_enabled = true;
	avail->kobj = kobj;

	return ret;

failed:
	kobject_put(kobj);
	return ret;
}

static int create_cpu_lvl_nodes(struct lpm_cluster *p, struct kobject *parent)
{
	int cpu;
	int i, j, cpu_idx;
	struct kobject **cpu_kobj = NULL;
	struct lpm_level_avail *level_list = NULL;
	char cpu_name[20] = {0};
	int ret = 0;

	cpu_kobj = devm_kzalloc(&lpm_pdev->dev, sizeof(*cpu_kobj) *
			cpumask_weight(&p->child_cpus), GFP_KERNEL);
	if (!cpu_kobj)
		return -ENOMEM;

	cpu_idx = 0;
	for_each_cpu(cpu, &p->child_cpus) {
		snprintf(cpu_name, sizeof(cpu_name), "cpu%d", cpu);
		cpu_kobj[cpu_idx] = kobject_create_and_add(cpu_name, parent);
		if (!cpu_kobj[cpu_idx]) {
			ret = -ENOMEM;
			goto release_kobj;
		}

		level_list = devm_kzalloc(&lpm_pdev->dev,
				MSM_PM_SLEEP_MODE_NR * sizeof(*level_list),
				GFP_KERNEL);
		if (!level_list) {
			ret = -ENOMEM;
			goto release_kobj;
		}

		for (i = 0; i < MSM_PM_SLEEP_MODE_NR; i++) {
			for (j = 0; j < p->cpu->nlevels; j++)
				if (p->cpu->levels[j].mode == i)
					break;
			if (j == p->cpu->nlevels) {
				/* Level not defined in DT */
				level_list[i].idle_enabled = false;
				level_list[i].suspend_enabled = false;
				continue;
			}

			ret = create_lvl_avail_nodes(p->cpu->levels[j].name,
					cpu_kobj[cpu_idx], &level_list[i]);
			if (ret)
				goto release_kobj;
		}

		cpu_level_available[cpu] = level_list;
		cpu_idx++;
	}

	return ret;

release_kobj:
	j = cpumask_weight(&p->child_cpus);
	for (i = 0; i < j; i++)
		kobject_put(cpu_kobj[i]);

	return ret;
}

int create_cluster_lvl_nodes(struct lpm_cluster *p, struct kobject *kobj)
{
	int ret = 0;
	struct lpm_cluster *child = NULL;
	int i;
	struct kobject *cluster_kobj = NULL;

	if (!p)
		return -ENODEV;

#if defined(CONFIG_ARCH_MSM8939)|| defined (CONFIG_ARCH_MSM8929)
	printk("[LPM] %s create\n",p->cluster_name);
	if (!strncmp(p->cluster_name, "power", 5))
		power_cluster = p;
	else if (!strncmp(p->cluster_name, "performance", 11))
		performance_cluster = p;
	else if (!strncmp(p->cluster_name, "system", 6))
		ret = sysfs_create_file(kobj, &lpm_bundle_attribute.attr);
#endif

	cluster_kobj = kobject_create_and_add(p->cluster_name, kobj);
	if (!cluster_kobj)
		return -ENOMEM;

	for (i = 0; i < p->nlevels; i++) {
		ret = create_lvl_avail_nodes(p->levels[i].level_name,
				cluster_kobj, &p->levels[i].available);
		if (ret)
			return ret;
	}

	list_for_each_entry(child, &p->child, list) {
		ret = create_cluster_lvl_nodes(child, cluster_kobj);
		if (ret)
			return ret;
	}

	if (p->cpu) {
		ret = create_cpu_lvl_nodes(p, cluster_kobj);
		if (ret)
			return ret;
	}

	return 0;
}

bool lpm_cpu_mode_allow(unsigned int cpu,
		unsigned int mode, bool from_idle)
{
	struct lpm_level_avail *avail = cpu_level_available[cpu];

	if (!lpm_pdev || !avail)
		return !from_idle;

	return !!(from_idle ? avail[mode].idle_enabled :
				avail[mode].suspend_enabled);
}

bool lpm_cluster_mode_allow(struct lpm_cluster *cluster,
		unsigned int mode, bool from_idle)
{
	struct lpm_level_avail *avail = &cluster->levels[mode].available;

	if (!lpm_pdev || !avail)
		return false;

	return !!(from_idle ? avail->idle_enabled :
				avail->suspend_enabled);
}

static int parse_cluster_params(struct device_node *node, struct lpm_cluster *c)
{
	int i;
	char *key;
	int ret;
	struct lpm_match {
		char *devname;
		int (*set_mode)(struct low_power_ops *, int, bool);
	};
	struct lpm_match match_tbl[] = {
		{"l2", set_l2_mode},
		{"cci", set_cci_mode},
	};

	key = "label";
	ret = of_property_read_string(node, key, &c->cluster_name);
	if (ret) {
		pr_err("%s(): Cannot read required param %s\n", __func__, key);
		return ret;
	}

	key = "qcom,spm-device-names";
	c->ndevices = of_property_count_strings(node, key);

	if (c->ndevices < 0) {
		pr_info("%s(): Ignoring cluster params\n", __func__);
		c->no_saw_devices = true;
		c->ndevices = 0;
		return 0;
	}

	c->name = devm_kzalloc(&lpm_pdev->dev, c->ndevices * sizeof(*c->name),
				GFP_KERNEL);
	c->lpm_dev = devm_kzalloc(&lpm_pdev->dev,
				c->ndevices * sizeof(*c->lpm_dev),
				GFP_KERNEL);
	if (!c->name || !c->lpm_dev) {
		ret = -ENOMEM;
		goto failed;
	}

	for (i = 0; i < c->ndevices; i++) {
		char device_name[20];
		int j;

		ret = of_property_read_string_index(node, key, i, &c->name[i]);
		if (ret)
			goto failed;
		snprintf(device_name, sizeof(device_name), "%s-%s",
				c->cluster_name, c->name[i]);

		c->lpm_dev[i].spm = msm_spm_get_device_by_name(device_name);

		if (IS_ERR_OR_NULL(c->lpm_dev[i].spm)) {
			pr_err("Failed to get spm device by name:%s\n",
					device_name);
			ret = PTR_ERR(c->lpm_dev[i].spm);
			goto failed;
		}
		for (j = 0; j < ARRAY_SIZE(match_tbl); j++) {
			if (!strcmp(c->name[i], match_tbl[j].devname))
				c->lpm_dev[i].set_mode = match_tbl[j].set_mode;
		}

		if (!c->lpm_dev[i].set_mode) {
			ret = -ENODEV;
			goto failed;
		}
	}

	key = "qcom,default-level";
	if (of_property_read_u32(node, key, &c->default_level))
		c->default_level = 0;
	return 0;
failed:
	pr_err("%s(): Failed reading %s\n", __func__, key);
	kfree(c->name);
	kfree(c->lpm_dev);
	c->name = NULL;
	c->lpm_dev = NULL;
	return ret;
}

static int parse_lpm_mode(const char *str)
{
	int i;
	struct lpm_lookup_table mode_lookup[] = {
		{MSM_SPM_MODE_POWER_COLLAPSE, "pc"},
		{MSM_SPM_MODE_GDHS, "gdhs"},
		{MSM_SPM_MODE_RETENTION, "retention"},
		{MSM_SPM_MODE_CLOCK_GATING, "wfi"},
		{MSM_SPM_MODE_DISABLED, "active"}
	};

	for (i = 0; i < ARRAY_SIZE(mode_lookup); i++)
		if (!strcmp(str, mode_lookup[i].mode_name))
			return  mode_lookup[i].modes;
	return -EINVAL;
}

static int parse_power_params(struct device_node *node,
		struct power_params *pwr)
{
	char *key;
	int ret;

	key = "qcom,latency-us";
	ret  = of_property_read_u32(node, key, &pwr->latency_us);
	if (ret)
		goto fail;

	key = "qcom,ss-power";
	ret = of_property_read_u32(node, key, &pwr->ss_power);
	if (ret)
		goto fail;

	key = "qcom,energy-overhead";
	ret = of_property_read_u32(node, key, &pwr->energy_overhead);
	if (ret)
		goto fail;

	key = "qcom,time-overhead";
	ret = of_property_read_u32(node, key, &pwr->time_overhead_us);
fail:
	if (ret)
		pr_err("%s(): %s Error reading %s\n", __func__, node->name,
				key);
	return ret;
}

static int parse_cluster_level(struct device_node *node,
		struct lpm_cluster *cluster)
{
	int i = 0;
	struct lpm_cluster_level *level = &cluster->levels[cluster->nlevels];
	int ret = -ENOMEM;
	char key[25] = {0};

	if (!cluster->no_saw_devices) {

		level->mode = devm_kzalloc(&lpm_pdev->dev,
				cluster->ndevices * sizeof(*level->mode),
				GFP_KERNEL);
		if (!level->mode)
			goto failed;

		for (i = 0; i < cluster->ndevices; i++) {
			const char *spm_mode;

			snprintf(key, 25, "qcom,spm-%s-mode", cluster->name[i]);
			ret = of_property_read_string(node, key, &spm_mode);
			if (ret)
				goto failed;

			level->mode[i] = parse_lpm_mode(spm_mode);
			if (level->mode[i] < 0)
				goto failed;
		}
	}

	ret = of_property_read_string(node, "label", &level->level_name);
	if (ret)
		goto failed;

	if (cluster->nlevels != cluster->default_level) {
		ret = of_property_read_u32(node, "qcom,min-child-idx",
				&level->min_child_level);
		if (ret)
			goto failed;

		if (cluster->min_child_level > level->min_child_level)
			cluster->min_child_level = level->min_child_level;
	}

	level->notify_rpm = of_property_read_bool(node, "qcom,notify-rpm");
	level->last_core_only = of_property_read_bool(node,
					"qcom,last-core-only");

	ret = parse_power_params(node, &level->pwr);
	if (ret)
		goto failed;

	cluster->nlevels++;
	return 0;
failed:
	pr_err("Failed %s() ret = %d\n", __func__, ret);
	kfree(level->mode);
	level->mode = NULL;
	return ret;
}

static int parse_cpu_mode(const char *mode_name)
{
	struct lpm_lookup_table pm_sm_lookup[] = {
		{MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
			"wfi"},
		{MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
			"standalone_pc"},
		{MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
			"pc"},
		{MSM_PM_SLEEP_MODE_RETENTION,
			"retention"},
	};
	int i;
	int ret = -EINVAL;

	for (i = 0; i < ARRAY_SIZE(pm_sm_lookup); i++) {
		if (!strcmp(mode_name, pm_sm_lookup[i].mode_name)) {
			ret = pm_sm_lookup[i].modes;
			break;
		}
	}
	return ret;
}

static int get_cpumask_for_node(struct device_node *node, struct cpumask *mask)
{
	struct device_node *cpu_node;
	int cpu;
	int idx = 0;
	bool found = false;

	cpu_node = of_parse_phandle(node, "qcom,cpu", idx++);
	if (!cpu_node) {
		pr_info("%s: No CPU phandle, assuming single cluster\n",
				node->full_name);
		/*
		 * Not all targets have the cpu node populated in the device
		 * tree. If cpu node is not populated assume all possible
		 * nodes belong to this cluster
		 */
		cpumask_copy(mask, cpu_possible_mask);
		return 0;
	}

	while (cpu_node) {
		found = false;
		for_each_possible_cpu(cpu) {
			if (of_get_cpu_node(cpu, NULL) == cpu_node) {
				cpumask_set_cpu(cpu, mask);
				found = true;
				break;
			}
		}
		if (!found)
			pr_crit("Unable to find CPU node for %s\n",
					cpu_node->full_name);

		cpu_node = of_parse_phandle(node, "qcom,cpu", idx++);
	}

	if (!cpumask_empty(mask))
		return 0;
	return -EINVAL;
}

static int parse_cpu_levels(struct device_node *node, struct lpm_cluster *c)
{
	struct device_node *n;
	int ret = -ENOMEM;
	int i;

	c->cpu = devm_kzalloc(&lpm_pdev->dev, sizeof(*c->cpu), GFP_KERNEL);
	if (!c->cpu)
		return ret;

	c->cpu->parent = c;

	for_each_child_of_node(node, n) {
		struct lpm_cpu_level *l = &c->cpu->levels[c->cpu->nlevels];
		char *key =  "qcom,spm-cpu-mode";

		c->cpu->nlevels++;

		ret = of_property_read_string(n, key, &l->name);
		if (ret) {
			pr_info("Failed %s %d\n", n->name,  __LINE__);
			goto failed;
		}

		l->mode = parse_cpu_mode(l->name);
		if (l->mode < 0) {
			pr_info("Failed %s\n", l->name);
			goto failed;
		}

		ret = parse_power_params(n, &l->pwr);
		if (ret)
			goto failed;

		key = "qcom,use-broadcast-timer";
		l->use_bc_timer = of_property_read_bool(n, key);

	}
	return 0;
failed:
	for (i = 0; i < c->cpu->nlevels; i++) {
		kfree(c->cpu->levels[i].name);
		c->cpu->levels[i].name = NULL;
	}
	kfree(c->cpu);
	c->cpu = NULL;
	pr_err("%s(): Failed with error code:%d\n", __func__, ret);
	return ret;
}

void free_cluster_node(struct lpm_cluster *cluster)
{
	int i;
	struct lpm_cluster *cl, *m;

	list_for_each_entry_safe(cl, m, &cluster->child, list) {
		list_del(&cl->list);
		free_cluster_node(cl);
	};

	if (cluster->cpu) {
		for (i = 0; i < cluster->cpu->nlevels; i++) {
			kfree(cluster->cpu->levels[i].name);
			cluster->cpu->levels[i].name = NULL;
		}
	}
	for (i = 0; i < cluster->nlevels; i++) {
		kfree(cluster->levels[i].mode);
		cluster->levels[i].mode = NULL;
	}
	kfree(cluster->cpu);
	kfree(cluster->name);
	kfree(cluster->lpm_dev);
	cluster->cpu = NULL;
	cluster->name = NULL;
	cluster->lpm_dev = NULL;
	cluster->ndevices = 0;
}

/*
 * TODO:
 * Expects a CPU or a cluster only. This ensures that affinity
 * level of a cluster is consistent with reference to its
 * child nodes.
 */
struct lpm_cluster *parse_cluster(struct device_node *node,
		struct lpm_cluster *parent)
{
	struct lpm_cluster *c;
	struct device_node *n;
	char *key;
	int ret = 0;

	c = devm_kzalloc(&lpm_pdev->dev, sizeof(*c), GFP_KERNEL);
	if (!c)
		return ERR_PTR(-ENOMEM);

	ret = parse_cluster_params(node, c);

	if (ret)
		goto failed_parse_params;

	INIT_LIST_HEAD(&c->child);
	c->parent = parent;
	spin_lock_init(&c->sync_lock);
	c->min_child_level = NR_LPM_LEVELS;

	for_each_child_of_node(node, n) {

		if (!n->name)
			continue;
		key = "qcom,pm-cluster-level";
		if (!of_node_cmp(n->name, key)) {
			WARN_ON(c->no_saw_devices);
			if (parse_cluster_level(n, c))
				goto failed_parse_cluster;
			continue;
		}

		key = "qcom,pm-cluster";
		if (!of_node_cmp(n->name, key)) {
			struct lpm_cluster *child;

			WARN_ON(c->no_saw_devices);
			child = parse_cluster(n, c);
			if (!child)
				goto failed_parse_cluster;

			list_add(&child->list, &c->child);
			cpumask_or(&c->child_cpus, &c->child_cpus,
					&child->child_cpus);
			continue;
		}

		key = "qcom,pm-cpu";
		if (!of_node_cmp(n->name, key)) {
			/*
			 * Parse the the cpu node only if a pm-cpu node
			 * is available, though the mask is defined @ the
			 * cluster level
			 */
			if (get_cpumask_for_node(node, &c->child_cpus))
				goto failed_parse_cluster;

			if (parse_cpu_levels(n, c))
				goto failed_parse_cluster;
		}
	}

	c->last_level = c->nlevels-1;

	return c;

failed_parse_cluster:
	pr_err("Failed parse cluster:%s\n", key);
	if (parent)
		list_del(&c->list);
	free_cluster_node(c);
failed_parse_params:
	c->parent = NULL;
	pr_err("Failed parse params\n");
	kfree(c);
	return NULL;
}

struct lpm_cluster *lpm_of_parse_cluster(struct platform_device *pdev)
{
	struct device_node *top = NULL;

	top = of_find_node_by_name(pdev->dev.of_node, "qcom,pm-cluster");
	if (!top)
		return ERR_PTR(-ENODEV);

	lpm_pdev = pdev;
	return parse_cluster(top, NULL);
}

void cluster_dt_walkthrough(struct lpm_cluster *cluster)
{
	struct list_head *list;
	int i, j;
	static int id;
	char str[10] = {0};

	if (!cluster)
		return;

	for (i = 0; i < id; i++)
		snprintf(str+i, 10 - i, "\t");
	pr_info("%d\n", __LINE__);

	for (i = 0; i < cluster->nlevels; i++) {
		struct lpm_cluster_level *l = &cluster->levels[i];
		pr_info("%d ndevices:%d\n", __LINE__, cluster->ndevices);
		for (j = 0; j < cluster->ndevices; j++)
			pr_info("%sDevice: %p id:%p\n", str,
					&cluster->name[j], &l->mode[i]);
	}

	if (cluster->cpu) {
		pr_info("%d\n", __LINE__);
		for (j = 0; j < cluster->cpu->nlevels; j++)
			pr_info("%s\tCPU mode: %s id:%d\n", str,
					cluster->cpu->levels[j].name,
					cluster->cpu->levels[j].mode);
	}

	id++;


	list_for_each(list, &cluster->child) {
		struct lpm_cluster *n;
		pr_info("%d\n", __LINE__);
		n = list_entry(list, typeof(*n), list);
		cluster_dt_walkthrough(n);
	}
	id--;
}
