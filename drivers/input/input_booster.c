/*
 * input_booster.c - touch booster driver
 *
 * Copyright (C) 2014 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>

#include <linux/input/input_booster.h>

#ifndef CONFIG_CPU_FREQ_LIMIT_USERSPACE
#define DVFS_TOUCH_ID	0
int set_freq_limit(unsigned long id, unsigned int freq)
{
	pr_err("%s is not yet implemented\n", __func__);
	return 0;
}
#endif

static struct input_booster_data *g_data;

/* TSP and Wacom use this */
static void input_booster_change_dvfs_work(struct work_struct *work)
{
	int retval = 0;
	struct input_booster_data *data = g_data;
	struct input_booster *booster =
		container_of(work,
				struct input_booster, work_dvfs_chg.work);

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	mutex_lock(&booster->dvfs_lock);

	switch (booster->dvfs_boost_mode) {
	case DVFS_STAGE_SINGLE:
		if (booster->dvfs_freq != -1) {
			if (data->dbg_level)
				pr_info("%s: cpu clk set: -1\n", __func__);
			retval = set_freq_limit(DVFS_TOUCH_ID, -1);
			if (retval < 0)
				pr_err("%s: booster stop failed(%d).\n",
							__func__, retval);
			booster->dvfs_freq = -1;
		}
#ifdef CONFIG_DEBUG_BUS_VOTER
		if (booster->bimc_freq != 0) {
			if (data->dbg_level)
				pr_info("%s: bimc clk set: 0\n", __func__);
			retval = msm_bus_floor_vote("bimc", 0);
			if (retval < 0)
				pr_err("%s: bimc release failed(%d).\n",
							__func__, retval);
			booster->bimc_freq = 0;
		}
#endif
		break;
	case DVFS_STAGE_DUAL:
#ifdef CONFIG_ARCH_MSM8939
	case DVFS_STAGE_TRIPLE:
#endif
		if (booster->dvfs_freq != data->tail.cpu_freq) {
			if (data->dbg_level)
				pr_info("%s: cpu clk set: %d\n", __func__, data->tail.cpu_freq);
			retval = set_freq_limit(DVFS_TOUCH_ID,
					data->tail.cpu_freq);
			if (retval < 0)
				pr_err("%s: cpu clk %d set failed(%d).\n",
					__func__, data->tail.cpu_freq, retval);
			booster->dvfs_freq = data->tail.cpu_freq;
		}
#ifdef CONFIG_DEBUG_BUS_VOTER
		if (booster->bimc_freq != data->tail.bimc_freq) {
			if (data->dbg_level)
				pr_info("%s: bimc clk set: %d\n", __func__, data->tail.bimc_freq);
			retval = msm_bus_floor_vote("bimc", data->tail.bimc_freq * 1000);
			booster->bimc_freq = data->tail.bimc_freq;
		}
#endif
		break;
	}

	if (retval < 0)
		pr_err("%s: booster change failed(%d).\n",
				__func__, retval);
	mutex_unlock(&booster->dvfs_lock);
}

/* TSP and Wacom use this */
static void input_booster_set_dvfs_lock(struct input_booster *booster, int on)
{
	struct input_booster_data *data = g_data;
	int retval = 0;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	booster->dvfs_boost_mode = 1 << data->level;

	if (booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		pr_debug("%s: DVFS stage is none(%d)\n",
				__func__, booster->dvfs_boost_mode);
		return;
	}

	mutex_lock(&booster->dvfs_lock);

	if (on == 0) {
		if (booster->dvfs_lock_status) {
			switch (booster->dvfs_boost_mode) {
			case DVFS_STAGE_SINGLE:
			case DVFS_STAGE_DUAL:
#ifdef CONFIG_ARCH_MSM8939
			case DVFS_STAGE_TRIPLE:
#endif
				schedule_delayed_work(&booster->work_dvfs_off,
					msecs_to_jiffies(data->tail.time));
				break;
			}
		}
	} else if (on > 0) {
		cancel_delayed_work(&booster->work_dvfs_off);

		if ((!booster->dvfs_lock_status) || (booster->dvfs_old_stauts < on)) {
			cancel_delayed_work(&booster->work_dvfs_chg);

			switch (booster->dvfs_boost_mode) {
			case DVFS_STAGE_SINGLE:
#ifdef CONFIG_ARCH_MSM8939
			case DVFS_STAGE_TRIPLE:
				if (booster->dvfs_freq != data->tail.cpu_freq) {
					if (data->dbg_level)
						pr_info("%s: cpu clk set: %d\n", __func__, data->tail.cpu_freq);
					retval = set_freq_limit(DVFS_TOUCH_ID,
							data->tail.cpu_freq);
					booster->dvfs_freq = data->tail.cpu_freq;
				}
#ifdef CONFIG_DEBUG_BUS_VOTER
				if (booster->bimc_freq != data->tail.bimc_freq) {
					if (data->dbg_level)
						pr_info("%s: bimc clk set: %d\n", __func__, data->tail.bimc_freq);
					msm_bus_floor_vote("bimc", data->tail.bimc_freq * 1000);
					booster->bimc_freq = data->tail.bimc_freq;
				}
#endif
				schedule_delayed_work(&booster->work_dvfs_chg,
					msecs_to_jiffies(data->head.time));
				break;
#endif
			case DVFS_STAGE_DUAL:
				if (booster->dvfs_freq != data->head.cpu_freq) {
					if (data->dbg_level)
						pr_info("%s: cpu clk set: %d\n", __func__, data->head.cpu_freq);
					retval = set_freq_limit(DVFS_TOUCH_ID,
							data->head.cpu_freq);
					booster->dvfs_freq = data->head.cpu_freq;
				}
#ifdef CONFIG_DEBUG_BUS_VOTER
				if (booster->bimc_freq != data->head.bimc_freq) {
					if (data->dbg_level)
						pr_info("%s: bimc clk set: %d\n", __func__, data->head.bimc_freq);
					msm_bus_floor_vote("bimc", data->head.bimc_freq * 1000);
					booster->bimc_freq = data->head.bimc_freq;
				}
#endif
				schedule_delayed_work(&booster->work_dvfs_chg,
					msecs_to_jiffies(data->head.time));
				break;
			}

			if (retval < 0)
				pr_err("%s: cpu first lock failed(%d)\n",
						__func__, retval);

			booster->dvfs_lock_status = true;
		}

	} else if (on < 0) {
		if (booster->dvfs_lock_status) {
			cancel_delayed_work(&booster->work_dvfs_off);
			cancel_delayed_work(&booster->work_dvfs_chg);
			schedule_work(&booster->work_dvfs_off.work);
		}
	}

	booster->dvfs_old_stauts = on;
	mutex_unlock(&booster->dvfs_lock);
}

/* Tkey use this */
static void input_booster_change_dvfs_tkey_work(struct work_struct *work)
{
	struct input_booster_data *data = g_data;
	int retval = 0;
	struct input_booster *booster =
		container_of(work,
				struct input_booster, work_dvfs_chg.work);

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	mutex_lock(&booster->dvfs_lock);

	if (data->dbg_level)
		pr_info("%s: cpu clk set: %d\n", __func__, booster->dvfs_freq);
	retval = set_freq_limit(DVFS_TOUCH_ID, booster->dvfs_freq);
	if (retval < 0)
		pr_info("%s: booster change failed(%d).\n",
			__func__, retval);

	booster->dvfs_lock_status = false;
	mutex_unlock(&booster->dvfs_lock);
}

/* Tkey use this */
static void input_booster_set_dvfs_tkey_lock(struct input_booster *booster, int on)
{
	struct input_booster_data *data = g_data;
	int retval = 0;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	if (booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		pr_debug("%s: DVFS stage is none(%d)\n",
				__func__, booster->dvfs_boost_mode);
		return;
	}

	mutex_lock(&booster->dvfs_lock);
	if (on == 0) {
		cancel_delayed_work(&booster->work_dvfs_chg);

		if (booster->dvfs_lock_status) {
			if (data->dbg_level)
				pr_info("%s: cpu clk set: %d\n", __func__, booster->dvfs_freq);
			retval = set_freq_limit(DVFS_TOUCH_ID, booster->dvfs_freq);

			if (retval < 0)
				pr_info("%s: cpu first lock failed(%d)\n", __func__, retval);
			booster->dvfs_lock_status = false;
		}

		schedule_delayed_work(&booster->work_dvfs_off,
			msecs_to_jiffies(INPUT_BOOSTER_OFF_TIME_TKEY));

	} else if (on == 1) {
		cancel_delayed_work(&booster->work_dvfs_off);
		schedule_delayed_work(&booster->work_dvfs_chg,
			msecs_to_jiffies(INPUT_BOOSTER_CHG_TIME_TKEY));

	} else if (on == 2) {
		if (booster->dvfs_lock_status) {
			cancel_delayed_work(&booster->work_dvfs_off);
			cancel_delayed_work(&booster->work_dvfs_chg);
			schedule_work(&booster->work_dvfs_off.work);
		}
	}
	mutex_unlock(&booster->dvfs_lock);
}

static int input_booster_set_dvfs_off(struct input_booster *booster)
{
	struct input_booster_data *data = g_data;
	int retval = 0;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return 0;
	}

	mutex_lock(&booster->dvfs_lock);

	if (booster->dvfs_freq != -1) {
		if (data->dbg_level)
			pr_info("%s: cpu clk set: -1\n", __func__);
		retval = set_freq_limit(DVFS_TOUCH_ID, -1);
		if (retval < 0)
			pr_err("%s: booster stop failed(%d).\n",
						__func__, retval);
	}
#ifdef CONFIG_DEBUG_BUS_VOTER
	if (booster->bimc_freq != 0) {
		if (data->dbg_level)
			pr_info("%s: bimc clk set: 0\n", __func__);
		retval = msm_bus_floor_vote("bimc", 0);
		if (retval < 0)
			pr_err("%s: bimc release failed(%d).\n",
						__func__, retval);
	}
#endif

	if (booster->dvfs_id == INPUT_BOOSTER_ID_TKEY) {
		if (retval < 0)
			booster->dvfs_lock_status = false;
		else
			booster->dvfs_lock_status = true;
	} else {
		booster->dvfs_freq = -1;
		booster->bimc_freq = 0;
		booster->dvfs_lock_status = false;
	}

	mutex_unlock(&booster->dvfs_lock);

	return retval;
}

static void input_booster_off_dvfs_work(struct work_struct *work)
{
	struct input_booster *booster =
		container_of(work,
				struct input_booster, work_dvfs_off.work);

	input_booster_set_dvfs_off(booster);
}

static void input_booster_init_dvfs(struct input_booster *booster, int id)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		booster->dvfs_off = NULL;
		booster->dvfs_set = NULL;
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	booster->dvfs_id = id;

	switch (booster->dvfs_id) {
	case INPUT_BOOSTER_ID_TSP:
		if (!data->dt_data->tsp_stage
			|| data->dt_data->tsp_stage == DVFS_STAGE_NONE) {
			booster->dvfs_off = NULL;
			booster->dvfs_set = NULL;
			pr_err("%s: TSP Booster is not enabled\n", __func__);
			return;
		}

		booster->dvfs_set = input_booster_set_dvfs_lock;
		INIT_DELAYED_WORK(&booster->work_dvfs_chg,
				input_booster_change_dvfs_work);
		booster->dvfs_stage = data->dt_data->tsp_stage;
		break;
	case INPUT_BOOSTER_ID_TKEY:
		if (!data->dt_data->tkey_stage
			|| data->dt_data->tkey_stage == DVFS_STAGE_NONE) {
			booster->dvfs_off = NULL;
			booster->dvfs_set = NULL;
			pr_err("%s: Tkey Booster is not enabled\n", __func__);
			return;
		}

		booster->dvfs_set = input_booster_set_dvfs_tkey_lock;
		INIT_DELAYED_WORK(&booster->work_dvfs_chg,
				input_booster_change_dvfs_tkey_work);
		booster->dvfs_stage = data->dt_data->tkey_stage;
		break;
	case INPUT_BOOSTER_ID_WACOM:
		if (!data->dt_data->wacom_stage
			|| data->dt_data->wacom_stage == DVFS_STAGE_NONE) {
			booster->dvfs_off = NULL;
			booster->dvfs_set = NULL;
			pr_err("%s: Wacom Booster is not enabled\n", __func__);
			return;
		}

		booster->dvfs_set = input_booster_set_dvfs_lock;
		INIT_DELAYED_WORK(&booster->work_dvfs_chg,
				input_booster_change_dvfs_work);
		booster->dvfs_stage = data->dt_data->wacom_stage;
		break;
	default:
		booster->dvfs_off = NULL;
		booster->dvfs_set = NULL;
		pr_err("%s: Booster ID is not defined\n", __func__);
		return;
	}

	mutex_init(&booster->dvfs_lock);
	booster->dvfs_off = input_booster_set_dvfs_off;
	INIT_DELAYED_WORK(&booster->work_dvfs_off, input_booster_off_dvfs_work);

	booster->dvfs_boost_mode = 1 << data->level;

	if (booster->dvfs_id == INPUT_BOOSTER_ID_TKEY) {
		booster->dvfs_freq = data->tail.cpu_freq;
		booster->dvfs_lock_status = true;
	} else {
		booster->dvfs_lock_status = false;
	}

	pr_err("%s: [%d] booster stage: 0x%04x, init: 0x%04x\n", __func__,
		booster->dvfs_id, booster->dvfs_stage, booster->dvfs_boost_mode);
}

struct input_booster *input_booster_allocate(int id)
{
	struct input_booster *booster;

	booster = kzalloc(sizeof(struct input_booster), GFP_KERNEL);
	if (!booster) {
		pr_err("%s: Failed to alloc mem for booster\n", __func__);
		return NULL;
	}

	input_booster_init_dvfs(booster, id);

	return booster;
}

void input_booster_free(struct input_booster *booster)
{
	if (!booster)
		return;

	cancel_delayed_work(&booster->work_dvfs_off);
	cancel_delayed_work(&booster->work_dvfs_chg);

	kfree(booster);
}

void input_booster_get_default_setting(const char *flag, struct dvfs *value)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	if (strncmp(flag, "head", 4) == 0) {
		memcpy(value, &data->dt_data->head, sizeof(struct dvfs));
	} else if (strncmp(flag, "tail", 4) == 0) {
		memcpy(value, &data->dt_data->tail, sizeof(struct dvfs));
	} else {
		memset(value, 0x0, sizeof(struct dvfs));
	}
	dev_info(data->dev, "%s: %s %d %d %d\n", __func__, flag,
		value->time, value->cpu_freq, value->bimc_freq);
}

static ssize_t input_booster_get_debug_level(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	dev_info(data->dev, "%s: %d\n", __func__, data->dbg_level);

	return sprintf(buf, "%u\n", data->dbg_level);
}

static ssize_t input_booster_set_debug_level(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct input_booster_data *data = g_data;
	unsigned long val;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	if (kstrtoul(buf, 0, &val) < 0)
		return -EINVAL;

	data->dbg_level = (unsigned int)val;
	dev_info(data->dev, "%s: %d\n", __func__, data->dbg_level);

	return count;
}

static ssize_t input_booster_get_head(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	dev_info(data->dev, "%s: %d %d %d\n", __func__,
		data->head.time, data->head.cpu_freq, data->head.bimc_freq);

	return sprintf(buf, "%d %d %d\n",\
		data->head.time, data->head.cpu_freq, data->head.bimc_freq);
}

static ssize_t input_booster_set_head(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct input_booster_data *data = g_data;
	int time, cpu_freq, bimc_freq;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	if (sscanf(buf, "%d%d%d", &time,&cpu_freq, &bimc_freq) != 3) {
		dev_err(data->dev,
			"%s: format [time cpu_freq bimc_freq] (Ex: 130 1600000 1500000)\n",
			__func__);
		goto out;
	}

	data->head.time = time;
	data->head.cpu_freq = cpu_freq;
	data->head.bimc_freq = bimc_freq;

	dev_info(data->dev, "%s: %d %d %d\n", __func__,
		data->head.time, data->head.cpu_freq, data->head.bimc_freq);

out:
	return count;
}

static ssize_t input_booster_get_tail(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	dev_info(data->dev, "%s: %d %d %d\n", __func__,
		data->tail.time, data->tail.cpu_freq, data->tail.bimc_freq);

	return sprintf(buf, "%d %d %d\n",\
		data->tail.time, data->tail.cpu_freq, data->tail.bimc_freq);
}

static ssize_t input_booster_set_tail(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct input_booster_data *data = g_data;
	int time, cpu_freq, bimc_freq;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	if (sscanf(buf, "%d%d%d", &time,&cpu_freq, &bimc_freq) != 3) {
		dev_err(data->dev,
			"%s: format [time cpu_freq bimc_freq] (Ex: 130 1600000 1500000)\n",
			__func__);
		goto out;
	}

	data->tail.time = time;
	data->tail.cpu_freq = cpu_freq;
	data->tail.bimc_freq = bimc_freq;

	dev_info(data->dev, "%s: %d %d %d\n", __func__,
		data->tail.time, data->tail.cpu_freq, data->tail.bimc_freq);

out:
	return count;
}

static ssize_t input_booster_get_level(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	struct input_booster_data *data = g_data;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	dev_info(data->dev, "%s: %d\n", __func__, data->level);

	return sprintf(buf, "%d\n", data->level);
}

static ssize_t input_booster_set_level(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	struct input_booster_data *data = g_data;
	unsigned int val;
	int stage;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return -ENODEV;
	}

	if (kstrtoint(buf, 0, &val) < 0)
		return -EINVAL;

	stage = 1 << val;
	if (!(data->dt_data->tsp_stage & stage)) {
		dev_err(data->dev,
				"%s: %d is not supported(%04x != %04x).\n",
				__func__, val, stage, data->dt_data->tsp_stage);
		return count;
	}

	data->level = (unsigned int)val;
	dev_info(data->dev, "%s: %d\n", __func__, data->level);

	return count;
}

/* Only use tsp/wacom */
void input_booster_set_level_change(int val)
{
	struct input_booster_data *data = g_data;
	int stage;

	if (!data) {
		pr_err("%s: data is NULL, return", __func__);
		return;
	}

	stage = 1 << val;
	if (!(data->dt_data->tsp_stage & stage)) {
		dev_err(data->dev,
				"%s: %d is not supported(%04x != %04x).\n",
				__func__, val, stage, data->dt_data->tsp_stage);
		return;
	}

	data->level = (unsigned int)val;
	dev_info(data->dev, "%s: %d\n", __func__, data->level);
}

static CLASS_ATTR(debug_level, S_IRUGO | S_IWUSR, input_booster_get_debug_level, input_booster_set_debug_level);
static CLASS_ATTR(head, S_IRUGO | S_IWUSR, input_booster_get_head, input_booster_set_head);
static CLASS_ATTR(tail, S_IRUGO | S_IWUSR, input_booster_get_tail, input_booster_set_tail);
static CLASS_ATTR(level, S_IRUGO | S_IWUSR, input_booster_get_level, input_booster_set_level);

static void input_booster_free_sysfs(struct input_booster_data *data)
{
	class_remove_file(data->booster_class, &class_attr_debug_level);
	class_remove_file(data->booster_class, &class_attr_head);
	class_remove_file(data->booster_class, &class_attr_tail);
	class_remove_file(data->booster_class, &class_attr_level);
	class_destroy(data->booster_class);
}

static int input_booster_init_sysfs(struct input_booster_data *data)
{
	int ret;

	data->booster_class = class_create(THIS_MODULE, "input_booster");
	if (IS_ERR(data->booster_class)) {
		dev_err(data->dev, "%s: Failed to create class\n", __func__);
		ret = IS_ERR(data->booster_class);
		goto err_create_class;
	}

	ret = class_create_file(data->booster_class, &class_attr_debug_level);
	if (ret) {
		dev_err(data->dev, "%s: Failed to create debug_level\n", __func__);
		goto err_create_dbg_level;
	}

	ret = class_create_file(data->booster_class, &class_attr_head);
	if (ret) {
		dev_err(data->dev, "%s: Failed to create head\n", __func__);
		goto err_create_head;
	}

	ret = class_create_file(data->booster_class, &class_attr_tail);
	if (ret) {
		dev_err(data->dev, "%s: Failed to create tail\n", __func__);
		goto err_create_tail;
	}

	ret = class_create_file(data->booster_class, &class_attr_level);
	if (ret) {
		dev_err(data->dev, "%s: Failed to create level\n", __func__);
		goto err_create_level;
	}

	return 0;

	class_remove_file(data->booster_class, &class_attr_level);
err_create_level:
	class_remove_file(data->booster_class, &class_attr_tail);
err_create_tail:
	class_remove_file(data->booster_class, &class_attr_head);
err_create_head:
	class_remove_file(data->booster_class, &class_attr_debug_level);
err_create_dbg_level:
	class_destroy(data->booster_class);
err_create_class:
	return ret;
}

#ifdef CONFIG_OF
static int input_booster_parse_dt(struct device *dev,
			struct input_booster_dt_data *dt_data)
{
	struct device_node *np = dev->of_node;
	int rc;
	u32 head[3];
	u32 tail[3];

	rc = of_property_read_u32(np, "booster,tsp_stage", &dt_data->tsp_stage);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,tsp_stage[%d]\n", __func__, rc);

	rc = of_property_read_u32(np, "booster,tkey_stage", &dt_data->tkey_stage);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,tkey_stage[%d]\n", __func__, rc);

	rc = of_property_read_u32(np, "booster,wacom_stage", &dt_data->wacom_stage);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,wacom_stage[%d]\n", __func__, rc);

	dev_info(dev, "%s: stage tsp:0x%04x, tkey:0x%04x, wacom:0x%04x\n", __func__,
		dt_data->tsp_stage, dt_data->tkey_stage, dt_data->wacom_stage);


	rc = of_property_read_u32(np, "booster,level", &dt_data->level);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,level[%d]\n", __func__, rc);

	rc = of_property_read_u32_array(np, "booster,head", head, 3);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,head[%d]\n", __func__, rc);

	dt_data->head.time = head[0];
	dt_data->head.cpu_freq = head[1];
	dt_data->head.bimc_freq = head[2];

	rc = of_property_read_u32_array(np, "booster,tail", tail, 3);
	if (rc < 0)
		dev_info(dev, "%s: Unable to read booster,tail[%d]\n", __func__, rc);

	dt_data->tail.time = tail[0];
	dt_data->tail.cpu_freq = tail[1];
	dt_data->tail.bimc_freq = tail[2];

	dev_info(dev, "%s: level: %d, header: %d %d %d, tail: %d %d %d\n", __func__, dt_data->level,
		dt_data->head.time, dt_data->head.cpu_freq, dt_data->head.bimc_freq,
		dt_data->tail.time, dt_data->tail.cpu_freq, dt_data->tail.bimc_freq);

	return 0;
}

#else
static int input_booster_parse_dt(struct device *dev,
			struct input_booster_dt_data *dt_data)
{
	return -ENODEV;
}
#endif

static int input_booster_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct input_booster_data *data;
	struct input_booster_dt_data *dt_data = NULL;
	int ret = 0;

	dev_info(dev, "%s\n", __func__);

	if (dev->of_node) {
		dt_data = devm_kzalloc(dev,
				sizeof(struct input_booster_dt_data),
				GFP_KERNEL);
		if (!dt_data) {
			dev_err(dev, "%s: failed to allocate memory\n", __func__);
			return -ENOMEM;
		}
		ret = input_booster_parse_dt(dev, dt_data);
		if (ret)
			goto err_alloc_dt_data;
	} else {
		dt_data = dev->platform_data;
		dev_err(dev, "%s: failed to align dtsi\n", __func__);
	}

	if (!dt_data) {
		dev_err(dev, "%s: dt_data is NULL\n", __func__);
		ret = -ENODEV;
		goto err_alloc_dt_data;
	};

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		dev_err(dev, "%s: Failed to alloc data\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_data;
	}

	data->dev = dev;
	data->dt_data = dt_data;
	g_data = data;
	platform_set_drvdata(pdev, data);

	memcpy(&data->head, &dt_data->head, sizeof(data->head));
	memcpy(&data->tail, &dt_data->tail, sizeof(data->tail));
	data->level = dt_data->level;

	ret = input_booster_init_sysfs(data);
	if (ret)
		goto err_init_sysfs;

	return 0;

	input_booster_free_sysfs(data);
err_init_sysfs:
	kfree(data);
	g_data = NULL;
err_alloc_data:
	devm_kfree(dev, dt_data);
err_alloc_dt_data:
	return ret;
}

static int input_booster_remove(struct platform_device *pdev)
{
	struct input_booster_data *data = platform_get_drvdata(pdev);

	input_booster_free_sysfs(data);
	devm_kfree(data->dev, data->dt_data);
	kfree(data);
	g_data = NULL;

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id input_booster_dt_ids[] = {
	{ .compatible = INPUT_BOOSTER_NAME },
	{ }
};
#endif

static struct platform_driver input_booster_driver = {
	.driver = {
		.name = INPUT_BOOSTER_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table	= of_match_ptr(input_booster_dt_ids),
#endif
	},
	.probe = input_booster_probe,
	.remove = input_booster_remove,
};

static int __init input_booster_init(void)
{
	return platform_driver_register(&input_booster_driver);
}

static void __exit input_booster_exit(void)
{
	return platform_driver_unregister(&input_booster_driver);
}

module_init(input_booster_init);
module_exit(input_booster_exit);

MODULE_DESCRIPTION("Input device[Touch, Touchkey, Hardkey] booster 2.1 version");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SAMSUNG Electronics");

