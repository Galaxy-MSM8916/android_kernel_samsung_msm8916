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

static struct touch_disabler_data *g_data;

static void _touch_disabler_set_touch_status(bool status);


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
 * Prints the string equivalent of the value of g_data->enabled to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_enabled(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->enabled) {
		return sprintf(buf, "%s\n", "true");
	}
	return sprintf(buf, "%s\n", "false");
}

/*
 * Sets g_data->enabled to the status passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_enabled(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	/* only set the variable if control is set to manual */
	if (g_data->control) {
		if (!strncmp(buf, "true", 4) || !strncmp(buf, "1", 1)) {
			pr_info("%s: touch devices are enabled.\n", __func__);
			_touch_disabler_set_touch_status(true);
			return count;
		}
		else if (!strncmp(buf, "false", 5) || !strncmp(buf, "0", 1)) {
			pr_info("%s: touch devices are disabled.\n", __func__);
			_touch_disabler_set_touch_status(false);
			return count;
		} else {
			pr_err("%s: Invalid input passed\n", __func__);
			return -EINVAL;
		}
	}
	pr_warn("%s: Input ignored since auto control is enabled.\n", __func__);
	return -EINVAL;
}

static CLASS_ATTR(enabled, S_IRUGO | S_IWUSR, touch_disabler_get_enabled, touch_disabler_set_enabled);


/*
 * Prints the string equivalent of the value of g_data->control to buf.
 *
 * Returns the number of bytes printed to buf.
 *
 */
static ssize_t touch_disabler_get_control(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	if (g_data->control) {
		return sprintf(buf, "%s\n", CONTROL_MANUAL);
	}
	return sprintf(buf, "%s\n", CONTROL_AUTO);
}

/*
 * Sets g_data->control to the control passed in buf.
 *
 * Returns the number of bytes read from buf, or -EINVAL if
 * invalid input is passed.
 *
 */
static ssize_t touch_disabler_set_control(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	if (!strncmp(buf, CONTROL_MANUAL, strlen(CONTROL_MANUAL)) ||
			!strncmp(buf, "1", 1)) {
		pr_info("%s: manual control is enabled.\n", __func__);
		g_data->control = 1;
		return count;
	}
	else if (!strncmp(buf, CONTROL_AUTO, strlen(CONTROL_AUTO)) ||
			!strncmp(buf, "0", 1)) {
		pr_info("%s: auto control is enabled.\n", __func__);
		g_data->control = 0;
		return count;
	}
	pr_err("%s: Invalid input passed\n", __func__);
	return -EINVAL;
}

static CLASS_ATTR(control, S_IRUGO | S_IWUSR, touch_disabler_get_control, touch_disabler_set_control);


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
	if (g_data && !g_data->control) {
		_touch_disabler_set_touch_status(status);
	}
}

/*
 * Enables or disables the touch key/panel depending on value of status.
 *
 * This function is called internally.
 *
 */
static void _touch_disabler_set_touch_status(bool status)
{
	/* set the enabled variable */
	if (g_data) {
		g_data->enabled = status;
	}

	/* check if the struct has been initialised by the touch driver */
	if (g_data && g_data->ts_dev) {
		if (status) {
			pr_info("%s: Enabling %s touch panel...\n", __func__,
					g_data->ts_dev->name);
			g_data->ts_dev->open(g_data->ts_dev);
		} else {
			pr_info("%s: Disabling %s touch panel...\n", __func__,
					g_data->ts_dev->name);
			g_data->ts_dev->close(g_data->ts_dev);
		}
	} else {
		pr_warn("%s: Touch panel data struct is uninitialised!\n", __func__);
	}

	if (g_data && g_data->tk_dev) {
		if (status) {
			pr_info("%s: Enabling %s touch keys...\n", __func__,
					g_data->tk_dev->name);
			g_data->tk_dev->open(g_data->tk_dev);
		} else {
			pr_info("%s: Disabling %s touch keys...\n", __func__,
					g_data->tk_dev->name);
			g_data->tk_dev->close(g_data->tk_dev);
		}
	} else {
		pr_warn("%s: Touch key data struct is uninitialised!\n", __func__);
	}
}

/*
 * Initialises sysfs interfaces specified (enabled and control).
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
	data->enabled = 0;
	data->control = 0;
	data->ts_dev = NULL;
	data->tk_dev = NULL;
	g_data = data;

	data->disabler_class = class_create(THIS_MODULE, TOUCH_DISABLER_NAME);
	if (IS_ERR(data->disabler_class)) {
		pr_err("%s: Failed to create class\n", __func__);
		ret = IS_ERR(data->disabler_class);
		goto err_create_class;
	}

	ret = class_create_file(data->disabler_class, &class_attr_enabled);
	if (ret) {
		pr_err("%s: Failed to create enabled\n", __func__);
		goto err_create_enabled;
	}

	ret = class_create_file(data->disabler_class, &class_attr_control);
	if (ret) {
		pr_err("%s: Failed to create control\n", __func__);
		goto err_create_control;
	}

	pr_debug("%s: Initialised sysfs interface.\n", __func__);
	return 0;

err_create_control:
	class_remove_file(data->disabler_class, &class_attr_enabled);
err_create_enabled:
	class_destroy(data->disabler_class);
err_create_class:
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

	class_remove_file(data->disabler_class, &class_attr_enabled);
	class_remove_file(data->disabler_class, &class_attr_control);
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
