/*
 * Copyright (c) 2017 The Lineage Project
 *                    Vincent Zvikaramba <zvikovincent@gmail.com>
 *                    Vladimir Bely <vlwwwwww@gmail.com>
 *                    Emery Tanghanwaye <emerytang@gmail.com>
 *                    Sean Hoyt <deadman96385@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <linux/sysfs.h>

#include <linux/input.h>

#include <linux/input/touch_disabler.h>

#include "../base/base.h"

static struct touch_disabler_data *g_data;

static void _touch_disabler_set_tk_status(bool status);
static void _touch_disabler_set_ts_status(bool status);


/*
 * Sets the value of g_data->ts_dev to ts_dev.
 *
 * This function is intended to be called within a touch screen driver.
 *
 */
void touch_disabler_set_ts_dev(struct input_dev *ts_dev)
{
	if (g_data) {
		g_data->ts_dev = ts_dev;
	}
}

/*
 * Sets the value of g_data->tk_dev to tk_dev.
 *
 * This function is intended to be called within a touch key driver.
 *
 */
void touch_disabler_set_tk_dev(struct input_dev *tk_dev)
{
	if (g_data) {
		g_data->tk_dev = tk_dev;
	}
}

/*
 * Prints the string equivalent of the value of g_data->tk_enabled to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_tk_enabled(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->tk_dev && g_data->tk_enabled) {
		return sprintf(buf, "%s\n", "true");
	}
	return sprintf(buf, "%s\n", "false");
}

/*
 * Sets g_data->tk_enabled to the status passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_tk_enabled(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	if (g_data->tk_dev) {
		/* only set the variable if control is set to manual */
		if (g_data->tk_control) {
			if (!strncmp(buf, "true", 4) || !strncmp(buf, "1", 1)) {
				pr_info("%s: touch key is enabled.\n", __func__);
				_touch_disabler_set_tk_status(true);
				return count;
			}
			else if (!strncmp(buf, "false", 5) || !strncmp(buf, "0", 1)) {
				pr_info("%s: touch key is disabled.\n", __func__);
				_touch_disabler_set_tk_status(false);
				return count;
			} else {
				pr_err("%s: Invalid input passed\n", __func__);
				return -EINVAL;
			}
		}
		pr_warn("%s: Input ignored since auto control is enabled.\n", __func__);
	}
	return -EINVAL;
}

struct class_attribute class_attr_tk_enabled = __ATTR(enabled,  S_IRUGO | S_IWUSR,
		touch_disabler_get_tk_enabled, touch_disabler_set_tk_enabled);


/*
 * Prints the string equivalent of the value of g_data->ts_enabled to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_ts_enabled(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->ts_dev && g_data->ts_enabled) {
		return sprintf(buf, "%s\n", "true");
	}
	return sprintf(buf, "%s\n", "false");
}

/*
 * Sets g_data->ts_enabled to the status passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_ts_enabled(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	if (g_data->ts_dev) {
		/* only set the variable if control is set to manual */
		if (g_data->ts_control) {
			if (!strncmp(buf, "true", 4) || !strncmp(buf, "1", 1)) {
				pr_info("%s: touch panel is enabled.\n", __func__);
				_touch_disabler_set_ts_status(true);
				return count;
			}
			else if (!strncmp(buf, "false", 5) || !strncmp(buf, "0", 1)) {
				pr_info("%s: touch panel is disabled.\n", __func__);
				_touch_disabler_set_ts_status(false);
				return count;
			} else {
				pr_err("%s: Invalid input passed\n", __func__);
				return -EINVAL;
			}
		}
		pr_warn("%s: Input ignored since auto control is enabled.\n", __func__);
	}
	return -EINVAL;
}

struct class_attribute class_attr_ts_enabled = __ATTR(enabled,  S_IRUGO | S_IWUSR,
		touch_disabler_get_ts_enabled, touch_disabler_set_ts_enabled);

/*
 * Prints the string equivalent of the value of g_data->tk_control to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_tk_control(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->tk_dev && g_data->tk_control) {
		return sprintf(buf, "%s\n", CONTROL_MANUAL);
	}
	return sprintf(buf, "%s\n", CONTROL_AUTO);
}

/*
 * Sets g_data->tk_control to the control passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_tk_control(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	if (g_data->tk_dev) {
		if (!strncmp(buf, CONTROL_MANUAL, strlen(CONTROL_MANUAL)) ||
				!strncmp(buf, "1", 1)) {
			pr_info("%s: manual touch key control is enabled.\n", __func__);
			g_data->tk_control = 1;
			return count;
		}
		else if (!strncmp(buf, CONTROL_AUTO, strlen(CONTROL_AUTO)) ||
				!strncmp(buf, "0", 1)) {
			pr_info("%s: auto touch key control is enabled.\n", __func__);
			g_data->tk_control = 0;
			return count;
		}
		pr_err("%s: Invalid input passed\n", __func__);
	}
	return -EINVAL;
}

struct class_attribute class_attr_tk_control = __ATTR(control,  S_IRUGO | S_IWUSR,
		touch_disabler_get_tk_control, touch_disabler_set_tk_control);


/*
 * Prints the string equivalent of the value of g_data->ts_control to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_ts_control(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->ts_dev && g_data->ts_control) {
		return sprintf(buf, "%s\n", CONTROL_MANUAL);
	}
	return sprintf(buf, "%s\n", CONTROL_AUTO);
}

/*
 * Sets g_data->ts_control to the control passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_ts_control(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	if (g_data->ts_dev) {
		if (!strncmp(buf, CONTROL_MANUAL, strlen(CONTROL_MANUAL)) ||
				!strncmp(buf, "1", 1)) {
			pr_info("%s: manual touch panel control is enabled.\n", __func__);
			g_data->ts_control = 1;
			return count;
		}
		else if (!strncmp(buf, CONTROL_AUTO, strlen(CONTROL_AUTO)) ||
				!strncmp(buf, "0", 1)) {
			pr_info("%s: auto touch panel control is enabled.\n", __func__);
			g_data->ts_control = 0;
			return count;
		}
		pr_err("%s: Invalid input passed\n", __func__);
	}
	return -EINVAL;
}

struct class_attribute class_attr_ts_control = __ATTR(control,  S_IRUGO | S_IWUSR,
		touch_disabler_get_ts_control, touch_disabler_set_ts_control);

/*
 *
 * Returns "available" if a touch key device is registered with
 * the driver, or "unavailable" otherwise.
 *
 */
static ssize_t touch_disabler_get_tk_status(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->tk_dev) {
		return sprintf(buf, "%s\n", "available");
	}
	return sprintf(buf, "%s\n", "unavailable");
}

struct class_attribute class_attr_tk_status = __ATTR(status, S_IRUGO,
		touch_disabler_get_tk_status, NULL);


/*
 *
 * Returns "true" if a touch screen device is registered with
 * the driver, or "false" otherwise.
 *
 */
static ssize_t touch_disabler_get_ts_status(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->ts_dev) {
		return sprintf(buf, "%s\n", "available");
	}
	return sprintf(buf, "%s\n", "unavailable");
}

struct class_attribute class_attr_ts_status = __ATTR(status, S_IRUGO,
		touch_disabler_get_ts_status, NULL);


/*
 * Enables or disables the touch key/panel depending on value of status.
 *
 * This function is called by mdss_dsi_post_panel_on/mdss_dsi_panel_off.
 * When the panel blanks or unblanks, touch devices will enabled or disabled.
 *
 */
void touch_disabler_set_touch_status(bool status)
{
	/* let mdss trigger the enaling/disabling */
	if (g_data) {
		if (g_data->tk_dev && !g_data->tk_control) {
			_touch_disabler_set_tk_status(status);
		}

		if (g_data->ts_dev && !g_data->ts_control) {
			_touch_disabler_set_ts_status(status);
		}
	}
}

/*
 * Enables or disables the touch key depending on value of status.
 *
 * This function is called internally.
 *
 */
static void _touch_disabler_set_tk_status(bool status)
{
	/* set the tk_enabled variable */
	g_data->tk_enabled = status;

	if (status) {
		pr_info("%s: Enabling %s touch keys...\n", __func__,
				g_data->tk_dev->name);
		g_data->tk_dev->open(g_data->tk_dev);
	} else {
		pr_info("%s: Disabling %s touch keys...\n", __func__,
				g_data->tk_dev->name);
		g_data->tk_dev->close(g_data->tk_dev);
	}
}

/*
 * Enables or disables the touch panel depending on value of status.
 *
 * This function is called internally.
 *
 */
static void _touch_disabler_set_ts_status(bool status)
{
	/* set the ts_enabled variable */
	g_data->ts_enabled = status;

	if (status) {
		pr_info("%s: Enabling %s touch panel...\n", __func__,
				g_data->ts_dev->name);
		g_data->ts_dev->open(g_data->ts_dev);
	} else {
		pr_info("%s: Disabling %s touch panel...\n", __func__,
				g_data->ts_dev->name);
		g_data->ts_dev->close(g_data->ts_dev);
	}
}

/*
 * Initialises sysfs interfaces specified (tk_enabled and control).
 *
 */
static int touch_disabler_init_sysfs(void)
{
	struct touch_disabler_data *data;
	int ret = 0;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		pr_err("%s: Failed to alloc data\n", __func__);
		ret = -ENOMEM;
		goto err_alloc_data;
	}

	/* initialise data variables */
	data->ts_control = 0;
	data->ts_dev = NULL;
	data->ts_enabled = 0;

	data->tk_control = 0;
	data->tk_dev = NULL;
	data->tk_enabled = 0;

	g_data = data;

	data->disabler_class = class_create(THIS_MODULE, TOUCH_DISABLER_NAME);
	if (IS_ERR(data->disabler_class)) {
		pr_err("%s: Failed to create class\n", __func__);
		ret = IS_ERR(data->disabler_class);
		goto err_create_main_class;
	}

	data->touch_screen_class = class_create(THIS_MODULE, "touch_screen");
	if (IS_ERR(data->touch_screen_class)) {
		pr_err("%s: Failed to create class\n", __func__);
		ret = IS_ERR(data->touch_screen_class);
		goto err_create_ts_class;
	}

	data->touch_key_class = class_create(THIS_MODULE, "touch_key");
	if (IS_ERR(data->touch_key_class)) {
		pr_err("%s: Failed to create class\n", __func__);
		ret = IS_ERR(data->touch_key_class);
		goto err_create_tk_class;
	}

	ret = class_create_file(data->touch_key_class, &class_attr_tk_enabled);
	if (ret) {
		pr_err("%s: Failed to create tk_enabled\n", __func__);
		goto err_create_tk_enabled;
	}

	ret = class_create_file(data->touch_screen_class, &class_attr_ts_enabled);
	if (ret) {
		pr_err("%s: Failed to create ts_enabled\n", __func__);
		goto err_create_ts_enabled;
	}

	ret = class_create_file(data->touch_key_class, &class_attr_tk_control);
	if (ret) {
		pr_err("%s: Failed to create tk_control\n", __func__);
		goto err_create_tk_control;
	}

	ret = class_create_file(data->touch_screen_class, &class_attr_ts_control);
	if (ret) {
		pr_err("%s: Failed to create ts_control\n", __func__);
		goto err_create_ts_control;
	}

	ret = class_create_file(data->touch_key_class, &class_attr_tk_status);
	if (ret) {
		pr_err("%s: Failed to create tk status\n", __func__);
		goto err_create_tk_status;
	}

	ret = class_create_file(data->touch_screen_class, &class_attr_ts_status);
	if (ret) {
		pr_err("%s: Failed to create ts status\n", __func__);
		goto err_create_ts_status;
	}

	ret = sysfs_create_link(&data->disabler_class->p->subsys.kobj,
			&data->touch_key_class->p->subsys.kobj, "touch_key");
	if (ret) {
		pr_err("%s: Failed to create touch_key symlink\n", __func__);
		goto err_create_tk_symlink;
	}

	ret = sysfs_create_link(&data->disabler_class->p->subsys.kobj,
			&data->touch_screen_class->p->subsys.kobj, "touch_screen");
	if (ret) {
		pr_err("%s: Failed to create touch_screen symlink\n", __func__);
		goto err_create_ts_symlink;
	}

	pr_debug("%s: Initialised sysfs interface.\n", __func__);
	return 0;

err_create_ts_symlink:
	sysfs_remove_link(&data->disabler_class->p->subsys.kobj, "touch_key");
err_create_tk_symlink:
	class_remove_file(data->touch_screen_class, &class_attr_ts_status);
err_create_ts_status:
	class_remove_file(data->touch_key_class, &class_attr_tk_status);
err_create_tk_status:
	class_remove_file(data->touch_screen_class, &class_attr_ts_control);
err_create_ts_control:
	class_remove_file(data->touch_key_class, &class_attr_tk_control);
err_create_tk_control:
	class_remove_file(data->touch_screen_class, &class_attr_ts_enabled);
err_create_ts_enabled:
	class_remove_file(data->touch_key_class, &class_attr_tk_enabled);
err_create_tk_enabled:
	class_destroy(data->touch_key_class);
err_create_tk_class:
	class_destroy(data->touch_screen_class);
err_create_ts_class:
	class_destroy(data->disabler_class);
err_create_main_class:
	kfree(data);
	g_data = NULL;
err_alloc_data:
	pr_err("%s: Failed to initialise sysfs interface.\n", __func__);
	return ret;
}

/*
 * Releases the sysfs structures initialised above.
 *
 */
static void touch_disabler_free_sysfs(void)
{
	struct touch_disabler_data *data = g_data;

	class_remove_file(data->touch_key_class, &class_attr_tk_enabled);
	class_remove_file(data->touch_key_class, &class_attr_tk_control);
	class_remove_file(data->touch_key_class, &class_attr_tk_status);

	class_remove_file(data->touch_screen_class, &class_attr_ts_enabled);
	class_remove_file(data->touch_screen_class, &class_attr_ts_control);
	class_remove_file(data->touch_screen_class, &class_attr_ts_status);

	sysfs_remove_link(&data->disabler_class->p->subsys.kobj, "touch_key");
	sysfs_remove_link(&data->disabler_class->p->subsys.kobj, "touch_screen");

	class_destroy(data->touch_key_class);
	class_destroy(data->touch_screen_class);
	class_destroy(data->disabler_class);

	kfree(data);
	g_data = NULL;
}

static int __init touch_disabler_init(void)
{
	return touch_disabler_init_sysfs();
}

static void __exit touch_disabler_exit (void)
{
	touch_disabler_free_sysfs();
}

module_init(touch_disabler_init);
module_exit(touch_disabler_exit);

MODULE_DESCRIPTION("Touch key/panel disabler");
MODULE_AUTHOR("Vincent Zvikaramba <zvikovincent@gmail.com>, Vladimir Bely <vlwwwwww@gmail.com>");
MODULE_LICENSE("GPL");
