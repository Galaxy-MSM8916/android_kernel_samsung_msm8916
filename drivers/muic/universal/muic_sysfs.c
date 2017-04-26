/*
 * muic_sysfs.c
 *
 * Copyright (C) 2014 Samsung Electronics
 * Thomas Ryu <smilesr.ryu@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/host_notify.h>
#include <linux/string.h>

#include <linux/muic/muic.h>

#if defined(CONFIG_MUIC_NOTIFIER)
#include <linux/muic/muic_notifier.h>
#endif /* CONFIG_MUIC_NOTIFIER */

#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif /* CONFIG_OF */

#include "muic-internal.h"
#include "muic_i2c.h"
#include "muic_debug.h"
#include "muic_apis.h"
#include "muic_regmap.h"

static ssize_t muic_show_uart_en(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);

	if (!pmuic->is_rustproof) {
		pr_info("%s:%s UART ENABLE\n", MUIC_DEV_NAME, __func__);
		return sprintf(buf, "1\n");
	}
	pr_info("%s:%s UART DISABLE\n", MUIC_DEV_NAME, __func__);
	return sprintf(buf, "0\n");
}

static ssize_t muic_set_uart_en(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);

	if (!strncmp(buf, "1", 1)) {
		pmuic->is_rustproof = false;
	} else if (!strncmp(buf, "0", 1)) {
		pmuic->is_rustproof = true;
	} else {
		pr_warn("%s:%s invalid value\n", MUIC_DEV_NAME, __func__);
	}

	pr_info("%s:%s uart_en(%d)\n", MUIC_DEV_NAME, __func__,
			!pmuic->is_rustproof);

	return count;
}

static ssize_t muic_show_uart_sel(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = pmuic->pdata;

	switch (pdata->uart_path) {
	case MUIC_PATH_UART_AP:
		pr_info("%s:%s AP\n", MUIC_DEV_NAME, __func__);
		return sprintf(buf, "AP\n");
	case MUIC_PATH_UART_CP:
		pr_info("%s:%s CP\n", MUIC_DEV_NAME, __func__);
		return sprintf(buf, "CP\n");
	default:
		break;
	}

	pr_info("%s:%s UNKNOWN\n", MUIC_DEV_NAME, __func__);
	return sprintf(buf, "UNKNOWN\n");
}

static ssize_t muic_set_uart_sel(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = pmuic->pdata;

	if (!strncasecmp(buf, "AP", 2)) {
		pdata->uart_path = MUIC_PATH_UART_AP;
		switch_to_ap_uart(pmuic);
	} else if (!strncasecmp(buf, "CP", 2)) {
		pdata->uart_path = MUIC_PATH_UART_CP;
		switch_to_cp_uart(pmuic);
	} else {
		pr_warn("%s:%s invalid value\n", MUIC_DEV_NAME, __func__);
	}

	pr_info("%s:%s uart_path(%d)\n", MUIC_DEV_NAME, __func__,
			pdata->uart_path);

	return count;
}

static ssize_t muic_show_usb_sel(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = pmuic->pdata;

	switch (pdata->usb_path) {
	case MUIC_PATH_USB_AP:
		return sprintf(buf, "PDA\n");
	case MUIC_PATH_USB_CP:
		return sprintf(buf, "MODEM\n");
	default:
		break;
	}

	pr_info("%s:%s UNKNOWN\n", MUIC_DEV_NAME, __func__);
	return sprintf(buf, "UNKNOWN\n");
}

static ssize_t muic_set_usb_sel(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = pmuic->pdata;

	if (!strncasecmp(buf, "PDA", 3)) {
		pdata->usb_path = MUIC_PATH_USB_AP;
	} else if (!strncasecmp(buf, "MODEM", 5)) {
		pdata->usb_path = MUIC_PATH_USB_CP;
	} else {
		pr_warn("%s:%s invalid value\n", MUIC_DEV_NAME, __func__);
	}

	pr_info("%s:%s usb_path(%d)\n", MUIC_DEV_NAME, __func__,
			pdata->usb_path);

	return count;
}

static ssize_t muic_show_adc(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	int ret;

	mutex_lock(&pmuic->muic_mutex);
	ret = get_adc(pmuic);
	mutex_unlock(&pmuic->muic_mutex);
	if (ret < 0) {
		pr_err("%s:%s err read adc reg(%d)\n", MUIC_DEV_NAME, __func__,
				ret);
		return sprintf(buf, "UNKNOWN\n");
	}

	return sprintf(buf, "%x\n", ret);
}

static ssize_t muic_show_usb_state(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);

	switch (pmuic->attached_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		return sprintf(buf, "USB_STATE_CONFIGURED\n");
	default:
		break;
	}

	return 0;
}

#ifdef DEBUG_MUIC
static ssize_t muic_show_registers(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	char mesg[256] = "";

	mutex_lock(&pmuic->muic_mutex);
	muic_read_reg_dump(pmuic, mesg);
	mutex_unlock(&pmuic->muic_mutex);
	pr_info("%s:%s\n", __func__, mesg);

	return sprintf(buf, "%s\n", mesg);
}

static char reg_dump_buf[256];
static ssize_t muic_show_reg_sel(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	pr_info("%s:%s\n", __func__, reg_dump_buf);

	return sprintf(buf, "%s\n", reg_dump_buf);
}

static ssize_t muic_set_reg_sel(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	int len = strlen(buf);
	unsigned int reg_base = 0, reg_num = 0;
	int ret = -EINVAL, i = 0;

#if 0 /* For the compatibility in 64 bit */
	pr_info("%s:%s -> %s(%d, %d)\n", MUIC_DEV_NAME, __func__,
		buf, count, len);
#endif
	if (len < 6) {
		ret = kstrtoint(buf, 0, &reg_base);
		if (ret) {
			pr_err("%s: Undefined Regs\n", __func__);
			goto err;
		}
		reg_num = 1;
	} else if (len < 10) {
		char *ptr;
		char reg_buf[8];

		strcpy(reg_buf, buf);
		ptr = strstr(reg_buf, "++");
		*ptr = 0x00;
		ret = kstrtoint(reg_buf, 0, &reg_base);
		if (ret) {
			pr_err("%s: Undefined Regs\n", __func__);
			goto err;
		}
		ret = kstrtoint(ptr + 2, 0, &reg_num);
		if (ret) {
			pr_err("%s: Undefined Regs\n", __func__);
			goto err;
		}
	} else {
		pr_err("%s: Undefined Regs\n", __func__);
		goto err;
	}

	pr_info(" (reg_base,reg_num) = (0x%02x,%d)\n", reg_base, reg_num);

	memset(reg_dump_buf, 0x00, sizeof(reg_dump_buf));

	while (reg_num--) {
		mutex_lock(&pmuic->muic_mutex);
		ret = muic_i2c_read_byte(pmuic->i2c, reg_base + i);
		mutex_unlock(&pmuic->muic_mutex);
		if (ret < 0) {
			pr_err("%s:%s err read %d\n", MUIC_DEV_NAME, __func__,
					reg_base);
			goto err;
		}
		pr_info(" [%02x] : %02x\n", reg_base, ret);

		sprintf(reg_dump_buf + strlen(reg_dump_buf),
			" [%02x] : %02x\n", reg_base + i++, ret);
	}

err:
	return count;
}
#endif

#if defined(CONFIG_USB_HOST_NOTIFY)
static ssize_t muic_show_otg_test(struct device *dev,
					   struct device_attribute *pattr,
					   char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct regmap_ops *pops = pmuic->regmapdesc->regmapops;
	int uattr;
	u8 val = 0;

	mutex_lock(&pmuic->muic_mutex);
	pops->ioctl(pmuic->regmapdesc, GET_OTG_STATUS, NULL, &uattr);
	val = regmap_read_value(pmuic->regmapdesc, uattr);
	mutex_unlock(&pmuic->muic_mutex);

	pr_info("%s val:%x buf%s\n", __func__, val, buf);

	if (val < 0) {
		pr_err("%s: fail to read muic reg\n", __func__);
		return sprintf(buf, "UNKNOWN\n");
	}

	return sprintf(buf, "%x\n", val);
}

static ssize_t muic_set_otg_test(struct device *dev,
		struct device_attribute *pattr,
		const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	struct regmap_ops *pops = pmuic->regmapdesc->regmapops;
	struct reg_attr attr;
	int uattr;
	u8 val;

	pr_info("%s buf:%s\n", __func__, buf);
	if (!strncmp(buf, "0", 1)) {
		val = 0;
		pmuic->is_otg_test = true;
	} else if (!strncmp(buf, "1", 1)) {
		val = 1;
		pmuic->is_otg_test = false;
	} else {
		pr_warn("%s:%s Wrong command\n", MUIC_DEV_NAME, __func__);
		return count;
	}

	mutex_lock(&pmuic->muic_mutex);
	pops->ioctl(pmuic->regmapdesc, GET_OTG_STATUS, NULL, &uattr);
	_REG_ATTR(&attr, uattr);

	val = muic_i2c_read_byte(pmuic->i2c, attr.addr);
	val |= attr.mask << attr.bitn;
	val |= _ATTR_OVERWRITE_M;
	val = regmap_write_value(pmuic->regmapdesc, uattr, val);
	mutex_unlock(&pmuic->muic_mutex);

	if (val < 0) {
		pr_err("%s err writing %s reg(%d)\n", __func__,
			regmap_to_name(pmuic->regmapdesc, attr.addr), val);
	}

	val = 0;
	val = muic_i2c_read_byte(pmuic->i2c, attr.addr);
	val &= (attr.mask << attr.bitn);
	pr_info("%s: %s(0x%02x)\n", __func__,
		regmap_to_name(pmuic->regmapdesc, attr.addr), val);

	return count;
}
#endif

static ssize_t muic_show_attached_dev(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);

	pr_info("%s:%s attached_dev:%d\n", MUIC_DEV_NAME, __func__,
			pmuic->attached_dev);

	switch(pmuic->attached_dev) {
	case ATTACHED_DEV_NONE_MUIC:
		return sprintf(buf, "No VPS\n");
	case ATTACHED_DEV_USB_MUIC:
		return sprintf(buf, "USB\n");
	case ATTACHED_DEV_CDP_MUIC:
		return sprintf(buf, "CDP\n");
	case ATTACHED_DEV_OTG_MUIC:
		return sprintf(buf, "OTG\n");
	case ATTACHED_DEV_TA_MUIC:
		return sprintf(buf, "TA\n");
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		return sprintf(buf, "JIG UART OFF\n");
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
		return sprintf(buf, "JIG UART OFF/VB\n");
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		return sprintf(buf, "JIG UART ON\n");
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
		return sprintf(buf, "JIG USB OFF\n");
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		return sprintf(buf, "JIG USB ON\n");
	case ATTACHED_DEV_DESKDOCK_MUIC:
		return sprintf(buf, "DESKDOCK\n");
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		return sprintf(buf, "AUDIODOCK\n");
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		return sprintf(buf, "PS CABLE\n");
	default:
		break;
	}

	return sprintf(buf, "UNKNOWN\n");
}

static ssize_t muic_show_audio_path(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	return 0;
}

static ssize_t muic_set_audio_path(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	return 0;
}

static ssize_t muic_show_apo_factory(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	const char *mode;

	/* true: Factory mode, false: not Factory mode */
	if (pmuic->is_factory_start)
		mode = "FACTORY_MODE";
	else
		mode = "NOT_FACTORY_MODE";

	pr_info("%s:%s apo factory=%s\n", MUIC_DEV_NAME, __func__, mode);

	return sprintf(buf, "%s\n", mode);
}

static ssize_t muic_set_apo_factory(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	muic_data_t *pmuic = dev_get_drvdata(dev);
	const char *mode;

	pr_info("%s:%s buf:%s\n", MUIC_DEV_NAME, __func__, buf);

	/* "FACTORY_START": factory mode */
	if (!strncmp(buf, "FACTORY_START", 13)) {
		pmuic->is_factory_start = true;
		mode = "FACTORY_MODE";
	} else {
		pr_warn("%s:%s Wrong command\n", MUIC_DEV_NAME, __func__);
		return count;
	}

	pr_info("%s:%s apo factory=%s\n", MUIC_DEV_NAME, __func__, mode);

	return count;
}

static DEVICE_ATTR(uart_en, 0664, muic_show_uart_en, muic_set_uart_en);
static DEVICE_ATTR(uart_sel, 0664, muic_show_uart_sel,
		muic_set_uart_sel);
static DEVICE_ATTR(usb_sel, 0664,
		muic_show_usb_sel, muic_set_usb_sel);
static DEVICE_ATTR(adc, 0664, muic_show_adc, NULL);
#ifdef DEBUG_MUIC
static DEVICE_ATTR(reg_dump, 0664, muic_show_registers, NULL);
static DEVICE_ATTR(reg_sel, 0664, muic_show_reg_sel, muic_set_reg_sel);
#endif
static DEVICE_ATTR(usb_state, 0664, muic_show_usb_state, NULL);
#if defined(CONFIG_USB_HOST_NOTIFY)
static DEVICE_ATTR(otg_test, 0664,
		muic_show_otg_test, muic_set_otg_test);
#endif
static DEVICE_ATTR(attached_dev, 0664, muic_show_attached_dev, NULL);
static DEVICE_ATTR(audio_path, 0664,
		muic_show_audio_path, muic_set_audio_path);
static DEVICE_ATTR(apo_factory, 0664,
		muic_show_apo_factory,
		muic_set_apo_factory);

static struct attribute *muic_attributes[] = {
	&dev_attr_uart_en.attr,
	&dev_attr_uart_sel.attr,
	&dev_attr_usb_sel.attr,
	&dev_attr_adc.attr,
#ifdef DEBUG_MUIC
	&dev_attr_reg_dump.attr,
	&dev_attr_reg_sel.attr,
#endif
	&dev_attr_usb_state.attr,
#if defined(CONFIG_USB_HOST_NOTIFY)
	&dev_attr_otg_test.attr,
#endif
	&dev_attr_attached_dev.attr,
	&dev_attr_audio_path.attr,
	&dev_attr_apo_factory.attr,
	NULL
};

const struct attribute_group muic_sysfs_group = {
	.attrs = muic_attributes,
};
