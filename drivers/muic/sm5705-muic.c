/*
 * sm5705-muic.c - SM5705 micro USB switch device driver
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

#include <linux/muic/muic.h>
#include <linux/muic/sm5705-muic.h>

#if defined(CONFIG_MUIC_NOTIFIER)
#include <linux/muic/muic_notifier.h>
#endif /* CONFIG_MUIC_NOTIFIER */

#if defined(CONFIG_VBUS_NOTIFIER)
#include <linux/vbus_notifier.h>
#endif /* CONFIG_VBUS_NOTIFIER */

#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif /* CONFIG_OF */

extern struct muic_platform_data muic_pdata;

#define I2C_RW_RETRY_MAX    10
#define I2C_RW_RETRY_DELAY  20

int otg_en;

int sm5705_i2c_read_byte(const struct i2c_client *client, u8 command)
{
	int ret;
	int retry = 0;

	ret = i2c_smbus_read_byte_data(client, command);

	while (ret < 0) {
		printk(KERN_ERR "[muic] %s reg(0x%x), retrying ...\n",
			__func__, command);
		if(retry > I2C_RW_RETRY_MAX) {
			printk(KERN_ERR "[muic] %s: retry count > 10 : failed !!\n", __func__);
			break;
		}
		msleep(I2C_RW_RETRY_DELAY);
		ret = i2c_smbus_read_byte_data(client, command);
		retry ++;
	}

	return ret;
}
int sm5705_i2c_write_byte(const struct i2c_client *client,
			u8 command, u8 value)
{
	int ret;
	int retry = 0;
	int written = 0;

	ret = i2c_smbus_write_byte_data(client, command, value);

	while (ret < 0) {
		written = i2c_smbus_read_byte_data(client, command);
		if (written < 0)
			printk(KERN_ERR "[muic] %s reg(0x%x)\n",
				__func__, command);
		if(retry > I2C_RW_RETRY_MAX) {
			printk(KERN_ERR "[muic] %s: retry count > 10 : failed !!\n", __func__);
			break;
		}
		msleep(I2C_RW_RETRY_DELAY);
		ret = i2c_smbus_write_byte_data(client, command, value);
		retry ++;
	}

	return ret;
}
int sm5705_i2c_guaranteed_wbyte(const struct i2c_client *client,
			u8 command, u8 value)
{
	int ret;
	int retry = 0;
	int written;

	ret = sm5705_i2c_write_byte(client, command, value);
	written = sm5705_i2c_read_byte(client, command);

	while (written != value) {
		printk(KERN_ERR "[muic] %s reg(0x%x)\n",
			__func__, command);
		if (retry > I2C_RW_RETRY_MAX) {
			printk(KERN_ERR "[muic] %s: retry count > 10 : failed !!\n", __func__);
			break;
		}
		msleep(I2C_RW_RETRY_DELAY);
		retry ++;
		ret = sm5705_i2c_write_byte(client, command, value);
		written = sm5705_i2c_read_byte(client, command);
	}
	return ret;
}

static int sm5705_set_gpio_uart_sel(int uart_sel)
{
	//const char *mode;
	//int uart_sel_gpio = muic_pdata.gpio_uart_sel;
	int ret = 0;
    
    pr_info("%s:%s\n", MUIC_DEV_NAME, __func__);

	printk(KERN_ERR "[muic] %s (0x%x)\n",
			__func__, uart_sel);
/*
	switch (uart_sel) {
	case MUIC_PATH_UART_AP:
		mode = "AP_UART";
		ret = gpio_direction_output(uart_sel_gpio, 1);
		if (ret)
			printk(KERN_ERR "[muic] failed to gpio_request GPIO_UART_SEL\n");
		break;
	case MUIC_PATH_UART_CP:
		mode = "CP_UART";
		ret = gpio_direction_output(uart_sel_gpio, 0);
		if (ret)
			printk(KERN_ERR "[muic] failed to gpio_request GPIO_UART_SEL\n");
		break;
	default:
		mode = "Error";
		break;
	}
*/
	return ret;
}

static ssize_t sm5705_muic_show_uart_en(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);

	if (!muic_data->is_rustproof) {
		printk(KERN_DEBUG "[muic] UART ENABLE\n");
		return sprintf(buf, "1\n");
	}
	printk(KERN_DEBUG "[muic] UART DISABLE\n");
	return sprintf(buf, "0\n");
}

static ssize_t sm5705_muic_set_uart_en(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);

	if (!strncmp(buf, "1", 1))
		muic_data->is_rustproof = false;
	else if (!strncmp(buf, "0", 1))
		muic_data->is_rustproof = true;
	else
		printk(KERN_DEBUG "[muic] %s invalid value\n", __func__);

	printk(KERN_DEBUG "[muic] uart_en(%d)\n", !muic_data->is_rustproof);

	return count;
}

static ssize_t sm5705_muic_show_uart_sel(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = muic_data->pdata;

	switch (pdata->uart_path) {
	case MUIC_PATH_UART_AP:
		printk(KERN_DEBUG "[muic] %s AP\n", __func__);
		return sprintf(buf, "AP\n");
	case MUIC_PATH_UART_CP:
		printk(KERN_DEBUG "[muic] %s CP\n", __func__);
		return sprintf(buf, "CP\n");
	default:
		break;
	}

	printk(KERN_DEBUG "[muic] %s UNKNOWN\n", __func__);
	return sprintf(buf, "UNKNOWN\n");
}

static int switch_to_ap_uart(struct sm5705_muic_data *muic_data);
static int switch_to_cp_uart(struct sm5705_muic_data *muic_data);

static ssize_t sm5705_muic_set_uart_sel(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = muic_data->pdata;

	if (!strncasecmp(buf, "AP", 2)) {
		pdata->uart_path = MUIC_PATH_UART_AP;
		switch_to_ap_uart(muic_data);
	} else if (!strncasecmp(buf, "CP", 2)) {
		pdata->uart_path = MUIC_PATH_UART_CP;
		switch_to_cp_uart(muic_data);
	} else {
		printk(KERN_DEBUG "[muic] %s invalid value\n", __func__);
	}

	printk(KERN_DEBUG "[muic] %s uart_path(%d)\n", __func__,
			pdata->uart_path);

	return count;
}

static ssize_t sm5705_muic_show_usb_sel(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = muic_data->pdata;

	switch (pdata->usb_path) {
	case MUIC_PATH_USB_AP:
		return sprintf(buf, "PDA\n");
	case MUIC_PATH_USB_CP:
		return sprintf(buf, "MODEM\n");
	default:
		break;
	}

	printk(KERN_DEBUG "[muic] %s UNKNOWN\n", __func__);
	return sprintf(buf, "UNKNOWN\n");
}

static ssize_t sm5705_muic_set_usb_sel(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct muic_platform_data *pdata = muic_data->pdata;

	if (!strncasecmp(buf, "PDA", 3))
		pdata->usb_path = MUIC_PATH_USB_AP;
	else if (!strncasecmp(buf, "MODEM", 5))
		pdata->usb_path = MUIC_PATH_USB_CP;
	else
		printk(KERN_DEBUG "[muic] %s invalid value\n", __func__);

	printk(KERN_DEBUG "[muic] %s usb_path(%d)\n", __func__,
			pdata->usb_path);

	return count;
}

static ssize_t sm5705_muic_show_adc(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	int ret;

	mutex_lock(&muic_data->muic_mutex);
	ret = sm5705_i2c_read_byte(muic_data->i2c, SM5705_MUIC_REG_ADC);
	mutex_unlock(&muic_data->muic_mutex);
	if (ret < 0) {
		printk(KERN_ERR "[muic] %s err read adc reg(%d)\n",
			__func__, ret);
		return sprintf(buf, "UNKNOWN\n");
	}

	return sprintf(buf, "%x\n", (ret & ADC_ADC_MASK));
}

static ssize_t sm5705_muic_show_usb_state(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);

	switch (muic_data->attached_dev) {
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

#if defined(CONFIG_USB_HOST_NOTIFY)
static ssize_t sm5705_muic_show_otg_test(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	int ret;
	u8 val = 0;

	mutex_lock(&muic_data->muic_mutex);
	ret = sm5705_i2c_read_byte(muic_data->i2c, SM5705_MUIC_REG_INTMASK2);
	mutex_unlock(&muic_data->muic_mutex);

	printk(KERN_DEBUG "[muic] func:%s ret:%d val:%x buf%s\n", __func__, ret, val, buf);

	if (ret < 0) {
		printk(KERN_ERR "[muic] %s: fail to read muic reg\n", __func__);
		return sprintf(buf, "UNKNOWN\n");
	}

	val &= INT2_VBUS_OFF_MASK;
	return sprintf(buf, "%x\n", val);
}

static ssize_t sm5705_muic_set_otg_test(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);

	printk(KERN_DEBUG "[muic] %s : %s\n", __func__, buf);

	/*
	*	The otg_test is set 0 durring the otg test. Not 1 !!!
	*/

	if (!strncmp(buf, "0", 1))
		muic_data->is_otg_test = 1;
	else if (!strncmp(buf, "1", 1))
		muic_data->is_otg_test = 0;
	else {
		printk(KERN_ERR "[muic] %s Wrong command\n", __func__);
		return count;
	}

	return count;
}
#endif

static ssize_t sm5705_muic_show_attached_dev(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);

	printk(KERN_DEBUG "[muic] %s : %d\n",
		__func__, muic_data->attached_dev);

	switch(muic_data->attached_dev) {
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

static ssize_t sm5705_muic_show_audio_path(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	return 0;
}

static ssize_t sm5705_muic_set_audio_path(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	return 0;
}

static ssize_t sm5705_muic_show_apo_factory(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	const char *mode;

	/* true: Factory mode, false: not Factory mode */
	if (muic_data->is_factory_start)
		mode = "FACTORY_MODE";
	else
		mode = "NOT_FACTORY_MODE";

	printk(KERN_DEBUG "[muic] %s = %s\n", __func__, mode);

	return sprintf(buf, "%s\n", mode);
}

static ssize_t sm5705_muic_set_apo_factory(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	const char *mode;

	printk(KERN_DEBUG "[muic] %s : %s\n", __func__, buf);

	/* "FACTORY_START": factory mode */
	if (!strncmp(buf, "FACTORY_START", 13)) {
		muic_data->is_factory_start = true;
		mode = "FACTORY_MODE";
	} else {
		printk(KERN_DEBUG "[muic] Wrong command!!!\n");
		return count;
	}

	printk(KERN_DEBUG "[muic] apo factory=%s\n", mode);

	return count;
}

static DEVICE_ATTR(uart_en, 0664, sm5705_muic_show_uart_en, sm5705_muic_set_uart_en);
static DEVICE_ATTR(uart_sel, 0664, sm5705_muic_show_uart_sel,
		sm5705_muic_set_uart_sel);
static DEVICE_ATTR(usb_sel, 0664,
		sm5705_muic_show_usb_sel, sm5705_muic_set_usb_sel);
static DEVICE_ATTR(adc, 0664, sm5705_muic_show_adc, NULL);
static DEVICE_ATTR(usb_state, 0664, sm5705_muic_show_usb_state, NULL);
#if defined(CONFIG_USB_HOST_NOTIFY)
static DEVICE_ATTR(otg_test, 0664,
		sm5705_muic_show_otg_test, sm5705_muic_set_otg_test);
#endif
static DEVICE_ATTR(attached_dev, 0664, sm5705_muic_show_attached_dev, NULL);
static DEVICE_ATTR(audio_path, 0664,
		sm5705_muic_show_audio_path, sm5705_muic_set_audio_path);
static DEVICE_ATTR(apo_factory, 0664,
		sm5705_muic_show_apo_factory,
		sm5705_muic_set_apo_factory);

static struct attribute *sm5705_muic_attributes[] = {
	&dev_attr_uart_en.attr,
	&dev_attr_uart_sel.attr,
	&dev_attr_usb_sel.attr,
	&dev_attr_adc.attr,
	&dev_attr_usb_state.attr,
#if defined(CONFIG_USB_HOST_NOTIFY)
	&dev_attr_otg_test.attr,
#endif
	&dev_attr_attached_dev.attr,
	&dev_attr_audio_path.attr,
	&dev_attr_apo_factory.attr,
	NULL
};

static const struct attribute_group sm5705_muic_group = {
	.attrs = sm5705_muic_attributes,
};

static int set_ctrl_reg(struct sm5705_muic_data *muic_data, int shift, bool on)
{
	struct i2c_client *i2c = muic_data->i2c;
	u8 reg_val;
	int ret = 0;

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_CTRL);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s(%d)\n", __func__, ret);
	if (on)
		reg_val = ret | (0x1 << shift);
	else
		reg_val = ret & ~(0x1 << shift);

	if (reg_val ^ ret) {
		printk(KERN_DEBUG "[muic] %s reg_val(0x%x)!=CTRL reg(0x%x), update reg\n",
			__func__, reg_val, ret);

		ret = sm5705_i2c_guaranteed_wbyte(i2c, SM5705_MUIC_REG_CTRL,
				reg_val);
		if (ret < 0)
			printk(KERN_ERR "[muic] %s err write CTRL(%d)\n",
					__func__, ret);
	} else {
		printk(KERN_DEBUG "[muic] %s (0x%x), just return\n",
				__func__, ret);
		return 0;
	}

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_CTRL);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s err read CTRL(%d)\n",
			__func__, ret);
	else
		printk(KERN_DEBUG "[muic] %s CTRL reg after change(0x%x)\n",
			__func__, ret);

	return ret;
}

static int set_int_mask(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = CTRL_MASK_INT_SHIFT;
	int ret = 0;

	ret = set_ctrl_reg(muic_data, shift, on);

	return ret;
}

static int set_manual_sw(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = CTRL_MANUAL_SW_SHIFT;
	int ret = 0;

	ret = set_ctrl_reg(muic_data, shift, on);

	return ret;
}

static int set_com_sw(struct sm5705_muic_data *muic_data,
			enum sm5705_reg_manual_sw1_value reg_val)
{
	struct i2c_client *i2c = muic_data->i2c;
	int ret = 0;
	int temp = 0;

	temp = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_MANSW1);
	if (temp < 0)
		printk(KERN_ERR "[muic] %s err read MANSW1(%d)\n",
			__func__, temp);

	if (reg_val != temp) {
		printk(KERN_DEBUG "[muic] %s reg_val(0x%x)!=MANSW1 reg(0x%x), update reg\n",
			__func__, reg_val, temp);

		ret = sm5705_i2c_guaranteed_wbyte(i2c, SM5705_MUIC_REG_MANSW1,
				reg_val);
		if (ret < 0)
			printk(KERN_ERR "[muic] %s err write MANSW1(%d)\n",
				__func__, reg_val);
	} else {
		printk(KERN_DEBUG "[muic] %s reg(0x%x), just return\n",
			__func__, reg_val);
		return 0;
	}

	return ret;
}

static int set_msw2_reg(struct sm5705_muic_data *muic_data, int shift, bool on)
{
	struct i2c_client *i2c = muic_data->i2c;
	u8 reg_val;
	int ret = 0;

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_MANSW2);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s err read MANSW2(%d)\n",
			__func__, ret);
	if (on)
		reg_val = ret | (0x1 << shift);
	else
		reg_val = ret & ~(0x1 << shift);

	if (reg_val ^ ret) {
		printk(KERN_DEBUG "[muic] %s reg_val(0x%x)!=MANSW2 reg(0x%x), update reg\n",
			__func__, reg_val, ret);

		ret = sm5705_i2c_guaranteed_wbyte(i2c, SM5705_MUIC_REG_MANSW2,
				reg_val);
		if (ret < 0)
			printk(KERN_ERR "[muic] %s err write MANSW2(%d)\n",
				__func__, ret);
	} else {
		printk(KERN_DEBUG "[muic] %s reg(0x%x), just return\n",
			__func__, ret);
		return 0;
	}

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_MANSW2);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s err read MANSW2(%d)\n",
			__func__, ret);
	else
		printk(KERN_DEBUG "[muic] %s MANSW2 reg after change(0x%x)\n",
			__func__, ret);

	return ret;
}

static int enable_periodic_adc_scan(struct sm5705_muic_data *muic_data)
{
	printk(KERN_DEBUG "[muic] %s\n", __func__);
	set_msw2_reg(muic_data, SM5705_MANSW2_SINGLE_MODE_SHIFT, 1);
	set_ctrl_reg(muic_data, CTRL_RAW_DATA_SHIFT , 0);

	return 0;
}

static int disable_periodic_adc_scan(struct sm5705_muic_data *muic_data)
{
	printk(KERN_DEBUG "[muic] %s\n", __func__);
	set_msw2_reg(muic_data, SM5705_MANSW2_SINGLE_MODE_SHIFT, 0);
	set_ctrl_reg(muic_data, CTRL_RAW_DATA_SHIFT , 1);

	return 0;
}


#define com_to_open com_to_open_with_vbus

#ifndef com_to_open
static int com_to_open(struct sm5705_muic_data *muic_data)
{
	enum sm5705_reg_manual_sw1_value reg_val;
	int ret = 0;

	reg_val = MANSW1_OPEN;
	ret = set_com_sw(muic_data, reg_val);
	if (ret)
		printk(KERN_ERR "[muic] %s set_com_sw err\n", __func__);

	return ret;
}
#endif
static int com_to_open_with_vbus(struct sm5705_muic_data *muic_data)
{
	enum sm5705_reg_manual_sw1_value reg_val;
	int ret = 0;

	reg_val = MANSW1_OPEN_WITH_V_BUS;
	ret = set_com_sw(muic_data, reg_val);
	if (ret)
		printk(KERN_ERR "[muic] %s set_com_sw err\n", __func__);

	return ret;
}

static int com_to_usb(struct sm5705_muic_data *muic_data)
{
	enum sm5705_reg_manual_sw1_value reg_val;
	int ret = 0;

	reg_val = MANSW1_USB_AP;
	ret = set_com_sw(muic_data, reg_val);
	if (ret)
		printk(KERN_ERR "[muic] %s set_com_usb err\n", __func__);

	return ret;
}

static int com_to_uart(struct sm5705_muic_data *muic_data)
{
	enum sm5705_reg_manual_sw1_value reg_val;
	int ret = 0;

	if (muic_data->is_rustproof) {
		printk(KERN_DEBUG "[muic] %s rustproof mode : do not set uart path\n",
			__func__);
		return ret;
	}

	reg_val = MANSW1_UART_AP;
	ret = set_com_sw(muic_data, reg_val);
	if (ret)
		printk(KERN_ERR "[muic] %s set_com_uart err\n", __func__);

	return ret;
}

static int com_to_audio(struct sm5705_muic_data *muic_data)
{
	enum sm5705_reg_manual_sw1_value reg_val;
	int ret = 0;

	reg_val = MANSW1_AUDIO;
	ret = set_com_sw(muic_data, reg_val);
	if (ret)
		printk(KERN_ERR "[muic] %s set_com_audio err\n", __func__);

	return ret;
}

static int switch_to_ap_usb(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = com_to_usb(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] %s com->usb set err\n", __func__);
		return ret;
	}

	return ret;
}

static int switch_to_ap_uart(struct sm5705_muic_data *muic_data)
{
	struct muic_platform_data *pdata = muic_data->pdata;
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	if (pdata->set_gpio_uart_sel)
		pdata->set_gpio_uart_sel(MUIC_PATH_UART_AP);

	ret = com_to_uart(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] %s com->uart set err\n", __func__);
		return ret;
	}

	return ret;
}

static int switch_to_cp_uart(struct sm5705_muic_data *muic_data)
{
	struct muic_platform_data *pdata = muic_data->pdata;
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	if (pdata->set_gpio_uart_sel)
		pdata->set_gpio_uart_sel(MUIC_PATH_UART_CP);

	ret = com_to_uart(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] %s com->uart set err\n", __func__);
		return ret;
	}

	return ret;
}

static int attach_usb_util(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev)
{
	int ret = 0;

	muic_data->attached_dev = new_dev;

	ret = switch_to_ap_usb(muic_data);
	return ret;
}

static int attach_usb(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev)
{
	int ret = 0;

	if (muic_data->attached_dev == new_dev) {
		printk(KERN_DEBUG "[muic] %s duplicated(USB)\n", __func__);
		return ret;
	}

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = attach_usb_util(muic_data, new_dev);
	if (ret)
		return ret;

	return ret;
}

static int detach_usb(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s attached_dev type(%d)\n", __func__,
			muic_data->attached_dev);

	ret = com_to_open_with_vbus(muic_data);
	if (ret)
		return ret;

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_otg_usb(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev)
{
	int ret = 0;

	if (muic_data->attached_dev == new_dev) {
		printk(KERN_DEBUG "[muic] %s duplicated(USB)\n", __func__);
		return ret;
	}

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	/* LANHUB doesn't work under AUTO switch mode, so turn it off */
	/* set MANUAL SW mode */
	set_manual_sw(muic_data, 0);

	/* enable RAW DATA mode, only for OTG LANHUB */
	enable_periodic_adc_scan(muic_data);

	ret = switch_to_ap_usb(muic_data);

	muic_data->attached_dev = new_dev;

	return ret;
}

static int detach_otg_usb(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s attached_dev type(%d)\n",
		__func__, muic_data->attached_dev);

	ret = com_to_open_with_vbus(muic_data);
	if (ret)
		return ret;

	/* disable RAW DATA mode */
	disable_periodic_adc_scan(muic_data);

	/* set AUTO SW mode */
	set_manual_sw(muic_data, 1);

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_ps_cable(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s new_dev(%d)\n", __func__, new_dev);
	com_to_open_with_vbus(muic_data);

	muic_data->attached_dev = new_dev;

	return ret;
}

static int detach_ps_cable(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_deskdock(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	/* Audio-out doesn't work under AUTO switch mode, so turn it off */
	/* set MANUAL SW mode */
	set_manual_sw(muic_data, 0);

	ret = com_to_audio(muic_data);
	if (ret)
		return ret;

	muic_data->attached_dev = new_dev;

	return ret;
}

static int detach_deskdock(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	set_manual_sw(muic_data, 1); /* set AUTO SW mode */

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_audiodock(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev, u8 vbus)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = attach_usb_util(muic_data, new_dev);
	if (ret)
		printk(KERN_DEBUG "[muic] %s attach_usb_util(%d)\n", __func__,ret);

	return ret;
}

static int detach_audiodock(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = com_to_open_with_vbus(muic_data);
	if (ret)
		return ret;

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_jig_uart_boot_off(struct sm5705_muic_data *muic_data, muic_attached_dev_t new_dev,
				u8 vbvolt)
{
	struct muic_platform_data *pdata = muic_data->pdata;

	int ret = 0;

	printk(KERN_DEBUG "[muic] %s vbus(%s)\n",
		__func__, vbvolt ? "ON" : "OFF");

	if (pdata->uart_path == MUIC_PATH_UART_AP)
		ret = switch_to_ap_uart(muic_data);
	else
		ret = switch_to_cp_uart(muic_data);

	muic_data->attached_dev = new_dev;

	return new_dev;
}
static int detach_jig_uart_boot_off(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	if(ret)
		printk(KERN_ERR "[muic] %s err detach_charger(%d)\n", __func__, ret);

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static int attach_jig_usb_boot_off(struct sm5705_muic_data *muic_data,
				u8 vbvolt)
{
	int ret = 0;

	if (muic_data->attached_dev == ATTACHED_DEV_JIG_USB_OFF_MUIC) {
		printk(KERN_DEBUG "[muic] %s duplicated\n",
				__func__);
		return ret;
	}

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = attach_usb_util(muic_data, ATTACHED_DEV_JIG_USB_OFF_MUIC);
	if (ret)
		return ret;

	return ret;
}

static int attach_jig_usb_boot_on(struct sm5705_muic_data *muic_data,
				u8 vbvolt)
{
	int ret = 0;

	if (muic_data->attached_dev == ATTACHED_DEV_JIG_USB_ON_MUIC) {
		printk(KERN_DEBUG "[muic] %s duplicated\n", __func__);
		return ret;
	}

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = attach_usb_util(muic_data, ATTACHED_DEV_JIG_USB_ON_MUIC);
	if (ret)
		return ret;

	return ret;
}

static int attach_mhl(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = com_to_open_with_vbus(muic_data);
	if (ret)
		return ret;

	muic_data->attached_dev = ATTACHED_DEV_MHL_MUIC;

	return ret;
}

static int detach_mhl(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	if(ret)
		printk(KERN_ERR "[muic] %s err detach_charger(%d)\n", __func__, ret);

	muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;

	return ret;
}

static void sm5705_muic_handle_attach(struct sm5705_muic_data *muic_data,
			muic_attached_dev_t new_dev, int adc, u8 vbvolt)
{
	int ret = 0;
	bool noti = (new_dev != muic_data->attached_dev) ? true : false;

	printk(KERN_DEBUG "[muic] %s attached_dev:%d new_dev:%d adc:0x%02x, vbvolt:%02x\n",
		__func__, muic_data->attached_dev, new_dev, adc, vbvolt);

	switch (muic_data->attached_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		if (new_dev != muic_data->attached_dev) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_usb(muic_data);
		}
		break;
	case ATTACHED_DEV_OTG_MUIC:
	/* OTG -> LANHUB, meaning TA is attached to LANHUB(OTG) */
		if (new_dev == ATTACHED_DEV_USB_LANHUB_MUIC)
			break;
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		if (new_dev != muic_data->attached_dev) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_otg_usb(muic_data);
		}
		break;

	case ATTACHED_DEV_AUDIODOCK_MUIC:
		if (new_dev != muic_data->attached_dev) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_audiodock(muic_data);
		}
		break;

	case ATTACHED_DEV_TA_MUIC:
		muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		if (new_dev != ATTACHED_DEV_JIG_UART_OFF_MUIC) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_jig_uart_boot_off(muic_data);
		}
		break;

	case ATTACHED_DEV_JIG_UART_ON_MUIC:
	case ATTACHED_DEV_DESKDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_VB_MUIC:
		if (new_dev != muic_data->attached_dev) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_deskdock(muic_data);
		}
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		if (new_dev != muic_data->attached_dev) {
			printk(KERN_DEBUG "[muic] %s new(%d)!=attached(%d), assume detach\n",
					__func__, new_dev,
					muic_data->attached_dev);
			ret = detach_ps_cable(muic_data);
		}
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
	default:
		break;
	}

	if (noti)
		muic_notifier_detach_attached_dev(muic_data->attached_dev);

	switch (new_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
		ret = attach_usb(muic_data, new_dev);
		break;
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		ret = attach_otg_usb(muic_data, new_dev);
		break;
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		ret = attach_audiodock(muic_data, new_dev, vbvolt);
		break;
	case ATTACHED_DEV_TA_MUIC:
		com_to_open_with_vbus(muic_data);
		mdelay(150);
		muic_data->attached_dev = new_dev;
		break;
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		if (vbvolt) {
			if (muic_data->is_otg_test)
				new_dev = ATTACHED_DEV_JIG_UART_OFF_VB_OTG_MUIC;
			else
				new_dev = ATTACHED_DEV_JIG_UART_OFF_VB_FG_MUIC;
		} else
			new_dev = ATTACHED_DEV_JIG_UART_OFF_MUIC;

		new_dev = attach_jig_uart_boot_off(muic_data, new_dev, vbvolt);
		break;
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		/* call attach_deskdock to wake up the device */
		ret = attach_deskdock(muic_data, new_dev);
		break;
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
		ret = attach_jig_usb_boot_off(muic_data, vbvolt);
		break;
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		ret = attach_jig_usb_boot_on(muic_data, vbvolt);
		break;
	case ATTACHED_DEV_MHL_MUIC:
		ret = attach_mhl(muic_data);
		break;
	case ATTACHED_DEV_DESKDOCK_MUIC:
		if (vbvolt)
			new_dev = ATTACHED_DEV_DESKDOCK_VB_MUIC;
		ret = attach_deskdock(muic_data, new_dev);
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		ret = attach_ps_cable(muic_data, new_dev);
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		com_to_open_with_vbus(muic_data);
		break;
	default:
		printk(KERN_DEBUG "[muic] %s unsupported dev=%d, adc=0x%x, vbus=%c\n",
				__func__, new_dev, adc,
				(vbvolt ? 'O' : 'X'));
		break;
	}

	muic_notifier_attach_attached_dev(new_dev);

	if (ret)
		printk(KERN_DEBUG "[muic] %s something wrong with attaching %d (ERR=%d)\n",
				__func__, new_dev, ret);
}

static void sm5705_muic_handle_detach(struct sm5705_muic_data *muic_data)
{
	int ret = 0;

	ret = com_to_open_with_vbus(muic_data);

	switch (muic_data->attached_dev) {
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
		ret = detach_usb(muic_data);
		break;
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		ret = detach_otg_usb(muic_data);
		break;
	case ATTACHED_DEV_TA_MUIC:
		muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		ret = detach_jig_uart_boot_off(muic_data);
		break;
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
	case ATTACHED_DEV_DESKDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_VB_MUIC:
		ret = detach_deskdock(muic_data);
		break;
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		ret = detach_audiodock(muic_data);
		break;
	case ATTACHED_DEV_MHL_MUIC:
		ret = detach_mhl(muic_data);
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		ret = detach_ps_cable(muic_data);
		break;
	case ATTACHED_DEV_NONE_MUIC:
		printk(KERN_DEBUG "[muic] %s duplicated(NONE)\n", __func__);
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		printk(KERN_DEBUG "[muic] %s UNKNOWN\n", __func__);
		muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
#if defined(CONFIG_MUIC_SM5705_AFC)
    case ATTACHED_DEV_AFC_CHARGER_9V_MUIC:
        printk(KERN_DEBUG "[muic] %s AFC_CHARGER_9V\n", __func__);
        muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;
        break;
#endif    
        
	default:
		printk(KERN_DEBUG "[muic] %s invalid attached_dev type(%d)\n",
			__func__, muic_data->attached_dev);
		muic_data->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	}

	muic_notifier_detach_attached_dev(muic_data->attached_dev);

	if (ret)
		printk(KERN_DEBUG "[muic] %s something wrong with detaching %d (ERR=%d)\n",
				__func__, muic_data->attached_dev, ret);

}

static void sm5705_muic_detect_dev(struct sm5705_muic_data *muic_data)
{
	struct i2c_client *i2c = muic_data->i2c;
	muic_attached_dev_t new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
	int intr = MUIC_INTR_DETACH;
	int vbvolt = 0;
	int val1, val2, val3, adc;

	val1 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_DEV_T1);
	val2 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_DEV_T2);
	val3 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_DEV_T3);
	vbvolt = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_VBUS_VALID);
	adc = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_ADC);

	printk(KERN_DEBUG
		"[muic] %s dev[1:0x%02x, 2:0x%02x, 3:0x%02x], adc:0x%02x, vbvolt:0x%02x\n",
			__func__, val1, val2, val3, adc, vbvolt);

	/* Attached */
	switch (val1) {
	case DEV_TYPE1_CDP:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_CDP_MUIC;
		printk(KERN_DEBUG "[muic] USB_CDP DETECTED\n");
		break;
	case DEV_TYPE1_USB:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_USB_MUIC;
		printk(KERN_DEBUG "[muic] USB DETECTED\n");
		break;
	case DEV_TYPE1_DEDICATED_CHG:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_TA_MUIC;
		printk(KERN_DEBUG "[muic] DEDICATED CHARGER DETECTED\n");
		break;
	case DEV_TYPE1_USB_OTG:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_OTG_MUIC;
		printk(KERN_DEBUG "[muic] USB_OTG DETECTED\n");
		break;
	case DEV_TYPE1_AUDIO_2:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_USB_LANHUB_MUIC;
		printk(KERN_DEBUG "[muic] LANHUB DETECTED\n");
		break;

	default:
		break;
	}

	switch (val2) {
	case DEV_TYPE2_JIG_UART_OFF:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_UART_OFF_MUIC;
		printk(KERN_DEBUG "[muic] JIG_UART_OFF DETECTED\n");
		break;
	case DEV_TYPE2_JIG_USB_OFF:
		if (!vbvolt) break;
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_USB_OFF_MUIC;
		printk(KERN_DEBUG "[muic] JIG_USB_OFF DETECTED\n");
		break;
	case DEV_TYPE2_JIG_USB_ON:
		if (!vbvolt) break;
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_USB_ON_MUIC;
		printk(KERN_DEBUG "[muic] JIG_USB_ON DETECTED\n");
		break;
	default:
		break;
	}

	if (val3 & DEV_TYPE3_CHG_TYPE) {
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_TA_MUIC;
		printk(KERN_DEBUG "[muic] TYPE3_CHARGER DETECTED\n");
	}

	if (val2 & DEV_TYPE2_AV || val3 & DEV_TYPE3_AV_WITH_VBUS) {
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_DESKDOCK_MUIC;
		printk(KERN_DEBUG "[muic] DESKDOCK DETECTED\n");
	}

	if (val3 & DEV_TYPE3_MHL) {
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_MHL_MUIC;
		printk(KERN_DEBUG "[muic] MHL DETECTED\n");
	}

	/* If there is no matching device found using device type registers
		use ADC to find the attached device */
	if(new_dev == ATTACHED_DEV_UNKNOWN_MUIC) {
		switch (adc) {
		case ADC_CEA936ATYPE1_CHG : /*200k ohm */
			intr = MUIC_INTR_ATTACH;
			/* This is workaournd for LG USB cable which has 219k ohm ID */
			new_dev = ATTACHED_DEV_USB_MUIC;
			printk(KERN_DEBUG "[muic] TYPE1 CHARGER DETECTED(USB)\n");
			break;
		case ADC_CEA936ATYPE2_CHG:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_TA_MUIC;
			printk(KERN_DEBUG "[muic] TYPE1/2 CHARGER DETECTED(TA)\n");
			break;
		case ADC_JIG_USB_OFF: /* 255k */
			if (!vbvolt) break;
			if (new_dev != ATTACHED_DEV_JIG_USB_OFF_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_USB_OFF_MUIC;
				printk(KERN_DEBUG "[muic] ADC JIG_USB_OFF DETECTED\n");
			}
			break;
		case ADC_JIG_USB_ON:
			if (!vbvolt) break;
			if (new_dev != ATTACHED_DEV_JIG_USB_ON_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_USB_ON_MUIC;
				printk(KERN_DEBUG "[muic] ADC JIG_USB_ON DETECTED\n");
			}
			break;
		case ADC_JIG_UART_OFF:
			if (new_dev != ATTACHED_DEV_JIG_UART_OFF_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_UART_OFF_MUIC;
				printk(KERN_DEBUG "[muic] ADC JIG_UART_OFF DETECTED\n");
			}
			break;
		case ADC_JIG_UART_ON:
			/* This is the mode to wake up device during factory mode. */
			if (new_dev != ATTACHED_DEV_JIG_UART_ON_MUIC
					&& muic_data->is_factory_start) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_UART_ON_MUIC;
				printk(KERN_DEBUG "[muic] ADC JIG_UART_ON DETECTED\n");
			}
			break;
		case ADC_AUDIODOCK:
#ifdef CONFIG_MUIC_SM5705_SUPPORT_AUDIODOCK
			intr = MUIC_INTR_ATTACH;
#endif
			new_dev = ATTACHED_DEV_AUDIODOCK_MUIC;
			printk(KERN_DEBUG "[muic] ADC AUDIODOCK DETECTED\n");
			break;
		case ADC_CHARGING_CABLE:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_CHARGING_CABLE_MUIC;
			printk(KERN_DEBUG "[muic] PS_CABLE DETECTED\n");
			break;
		case ADC_OPEN:
			/* sometimes muic fails to catch JIG_UART_OFF detaching */
			/* double check with ADC */
			if (new_dev == ATTACHED_DEV_JIG_UART_OFF_MUIC) {
				new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
				intr = MUIC_INTR_DETACH;
				printk(KERN_DEBUG "[muic] ADC OPEN DETECTED\n");
			}
			break;
		default:
			printk(KERN_DEBUG "[muic] %s unsupported ADC(0x%02x)\n",
				__func__, adc);
			if(vbvolt) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_UNDEFINED_CHARGING_MUIC;
				printk(KERN_DEBUG "[muic] UNDEFINED VB DETECTED\n");
			} else
				intr = MUIC_INTR_DETACH;
			break;
		}
	}

	if (intr == MUIC_INTR_ATTACH)
		sm5705_muic_handle_attach(muic_data, new_dev, adc, vbvolt);
	else
		sm5705_muic_handle_detach(muic_data);

#if defined(CONFIG_VBUS_NOTIFIER)
	vbus_notifier_handle(!!vbvolt ? STATUS_VBUS_HIGH : STATUS_VBUS_LOW);
#endif /* CONFIG_VBUS_NOTIFIER */

}

static int sm5705_muic_reg_init(struct sm5705_muic_data *muic_data)
{
	struct i2c_client *i2c = muic_data->i2c;
	int ret;

    pr_info("%s:%s\n", MUIC_DEV_NAME, __func__);
	printk(KERN_DEBUG "[muic] %s\n", __func__);

	ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_INTMASK1, REG_INT_MASK1_VALUE);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err mask interrupt1(%d)\n", __func__, ret);

	ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_INTMASK2, REG_INT_MASK2_VALUE);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err mask interrupt2(%d)\n", __func__, ret);

	ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_INTMASK3_AFC, REG_INT_MASK3_AFC_VALUE);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err mask interrupt3(%d)\n", __func__, ret);

	ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_TIMING1, REG_TIMING1_VALUE);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err write timing1(%d)\n", __func__, ret);

	ret = sm5705_i2c_guaranteed_wbyte(i2c, SM5705_MUIC_REG_CTRL, CTRL_MASK);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err write ctrl(%d)\n", __func__, ret);

	/* enable ChargePump */
	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_RESERVED_ID2);
	if (ret < 0)
		printk(KERN_ERR "%s: err %d\n", __func__, ret);
    ret &= 0xDF;  
	ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_RESERVED_ID2, ret);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err write ctrl(%d)\n", __func__, ret);

	return ret;
}

static irqreturn_t sm5705_muic_irq_thread(int irq, void *data)
{
	struct sm5705_muic_data *muic_data = data;
	struct i2c_client *i2c = muic_data->i2c;
	int intr1, intr2;
    int intr3_AFC;

#if defined(CONFIG_MUIC_SM5705_AFC)
    int ret, value;
    int multi_byte[6] = {0,0,0,0,0,0};
    int i;
    int dev3;
    int vbus_status;
#endif    

	printk(KERN_DEBUG "[muic] %s irq(%d) start\n", __func__, irq);

	mutex_lock(&muic_data->muic_mutex);

	/* read and clear interrupt status bits */
	intr1 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_INT1);
	intr2 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_INT2);
    intr3_AFC = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_INT3_AFC);

	if ( (intr1 < 0) || (intr2 < 0) || (intr3_AFC < 0) ) {
		printk(KERN_ERR "[muic] %s: err read interrupt status [1:0x%02x, 2:0x%02x 3:0x%02x]\n",
				__func__, intr1, intr2, intr3_AFC );
		goto skip_detect_dev;
	}

	if (intr1 & INT1_ATTACH_MASK) {
		int intr_tmp = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_INT1);

		if (intr_tmp & INT1_DETACH_MASK) {
			printk(KERN_DEBUG "[muic] %s attach/detach interrupt occurred\n", __func__);
			intr1 &= 0xFE;
		}
		intr1 |= intr_tmp;
	}

	printk(KERN_DEBUG "[muic] %s intr[1:0x%02x, 2:0x%02x 3:0x%02x]\n", __func__,
			intr1, intr2, intr3_AFC);

	/* check for muic reset and recover for every interrupt occurred */
	if ( (intr1 == 0) && (intr2 == 0) && (irq != -1) ) {
		int ctrl = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_CTRL);

		if (ctrl == 0x1F) {
			/* CONTROL register is reset to 1F */
			printk(KERN_ERR "[muic] %s: err muic could have been reseted. Initilize!!\n",
				__func__);
			sm5705_muic_reg_init(muic_data);

			/* MUIC Interrupt On */
			set_int_mask(muic_data, false);
		}

		//if((intr1 & INT1_ATTACH_MASK) == 0)
		//	goto skip_detect_dev;
	}
#if defined(CONFIG_MUIC_SM5705_AFC)
    if ( (intr1 & INT1_ATTACH_MASK) ||
         (intr1 & INT1_DETACH_MASK) ||
         (intr1 & INT1_KP_MASK) ||
         (intr1 & INT1_LKP_MASK) ||
         (intr1 & INT1_LKR_MASK) ||
         (intr2 & INT2_VBUS_OFF_MASK) ||
         (intr2 & INT2_RSRV_ATTACH_MASK) ||
         (intr2 & INT2_ADC_CHANGE_MASK) ||
         (intr2 & INT2_STUCK_KEY_MASK) ||
         (intr2 & INT2_STUCK_KEY_RCV_MASK) ||
         (intr2 & INT2_MHL_MASK) ||
         (intr2 & INT2_RID_CHARGER_MASK) ||
         (intr2 & INT2_VBUSDET_ON_MASK) ) 
    {
         sm5705_muic_detect_dev(muic_data);
    /* AFC TA */
    } else if ( intr3_AFC & INT3_AFC_TA_ATTACHED_MASK ){  /*AFC_TA_ATTACHED*/
        pr_info("%s:%s AFC_TA_ATTACHED \n",MUIC_DEV_NAME, __func__);

        muic_data->attached_dev = ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC;
        muic_notifier_attach_attached_dev(ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC);
        
        // voltage(9.0V) + current(1.65A) setting : 0x
        value = 0x46;
        ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_AFC_TXD, value);
        if (ret < 0)
            printk(KERN_ERR "[muic] %s: err write AFC_TXD(%d)\n", __func__, ret);
        pr_info("%s:%s AFC_TXD [0x%02x]\n",MUIC_DEV_NAME, __func__, value);

        // ENAFC set '1' 
        set_afc_ctrl_enafc(muic_data, 1);
       
    } else if ( intr3_AFC & INT3_AFC_ACCEPTED_MASK ){  /*AFC_ACCEPTED*/
        pr_info("%s:%s AFC_ACCEPTED \n",MUIC_DEV_NAME, __func__);
        
        dev3 = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_DEV_T3);
        pr_info("%s: dev3 [0x%02x]\n",MUIC_DEV_NAME, dev3);
        if (dev3 & DEV_TYPE3_AFC_CHG) {
            // VBUS_READ
            set_afc_vbus_read(muic_data, 1);
            pr_info("%s:%s VBUS READ start\n",MUIC_DEV_NAME, __func__);
        } else {
            // ENAFC set '0' 
            set_afc_ctrl_enafc(muic_data, 0);            
        }
    
    } else if ( intr3_AFC & INT3_AFC_VBUS_UPDATE_MASK ){  /*AFC_VBUS_UPDATE*/
        pr_info("%s:%s AFC_VBUS_UPDATE \n",MUIC_DEV_NAME, __func__);

        vbus_status = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_VBUS_STATUS);
        pr_info("%s: vbus_status [0x%02x]\n",MUIC_DEV_NAME, vbus_status);
        vbus_status = (vbus_status&0x0F);
        value = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_TXD);
        pr_info("%s: AFC_TXD [0x%02x]\n",MUIC_DEV_NAME, value);
        value = (value&0xF0)>>4;
        pr_info("%s:%s AFC_VBUS_STATUS:0x%02x, AFC_TXD:0x%02x \n",MUIC_DEV_NAME, __func__,vbus_status, value);
        if ( vbus_status == value ) { /* AFC DONE */
            // ENAFC set '0' 
            set_afc_ctrl_enafc(muic_data, 0);
            pr_info("%s:%s AFC done \n",MUIC_DEV_NAME, __func__);
            
            muic_data->attached_dev = ATTACHED_DEV_AFC_CHARGER_9V_MUIC;
            muic_notifier_attach_attached_dev(ATTACHED_DEV_AFC_CHARGER_9V_MUIC);
        } else {
            // VBUS_READ
            set_afc_vbus_read(muic_data, 1);
            set_afc_vbus_read(muic_data, 0);
            pr_info("%s:%s VBUS READ retry\n",MUIC_DEV_NAME, __func__);
        }
    } else if ( intr3_AFC & INT3_AFC_MULTI_BYTE_MASK ){  /*AFC_MULTI_BYTE*/
        pr_info("%s:%s AFC_MULTI_BYTE \n",MUIC_DEV_NAME, __func__);
        
        // read AFC_RXD1 ~ RXD6
        for(i = 0 ; i < 6 ; i++ ){
            multi_byte[i] = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_RXD1 + i);
            if (multi_byte[i] < 0)
                printk(KERN_ERR "%s: err read AFC_RXD%d %d\n", __func__,i+1, multi_byte[i]);
            pr_info("%s:%s AFC_RXD%d [0x%02x]\n",MUIC_DEV_NAME, __func__,i+1, multi_byte[i]);
        }

        // voltage(9.0V) + current(1.65A) setting : 0x
        value = multi_byte[0];
        ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_AFC_TXD, value);
        if (ret < 0)
            printk(KERN_ERR "[muic] %s: err write AFC_TXD(%d)\n", __func__, ret);
        pr_info("%s:%s AFC_TXD [0x%02x]\n",MUIC_DEV_NAME, __func__, value);

        // ENAFC set '1' 
        set_afc_ctrl_enafc(muic_data, 1);
        

    } else if ( intr3_AFC & INT3_AFC_ERROR_MASK ){  /*AFC_ERROR*/
        pr_info("%s:%s AFC_ERROR \n",MUIC_DEV_NAME, __func__);
        
        // read AFC_STATUS
        value = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_STATUS);
        if (value < 0)
            printk(KERN_ERR "%s: err read AFC_STATUS %d\n", __func__, value);
        pr_info("%s:%s AFC_STATUS [0x%02x]\n",MUIC_DEV_NAME, __func__, value);

        // read AFC_VBUS_STATUS
        value = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_VBUS_STATUS);
        if (value < 0)
            printk(KERN_ERR "%s: err read AFC_VBUS_STATUS %d\n", __func__, value);
        pr_info("%s:%s AFC_VBUS_STATUS [0x%02x]\n",MUIC_DEV_NAME, __func__, value);

        // ENAFC set '0' 
        set_afc_ctrl_enafc(muic_data, 0);
    } else {
        sm5705_muic_detect_dev(muic_data);
    }
#else    
	sm5705_muic_detect_dev(muic_data);
#endif /* CONFIG_MUIC_SM5705_AFC */
    


skip_detect_dev:
	mutex_unlock(&muic_data->muic_mutex);
	printk(KERN_DEBUG "[muic] %s irq(%d) end\n", __func__, irq);

	return IRQ_HANDLED;
}

static void sm5705_muic_init_detect(struct work_struct *work)
{
	struct sm5705_muic_data *muic_data =
		container_of(work, struct sm5705_muic_data, init_work.work);

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	/* MUIC Interrupt On */
	set_int_mask(muic_data, false);

	sm5705_muic_irq_thread(-1, muic_data);
}

static int sm5705_init_rev_info(struct sm5705_muic_data *muic_data)
{
	u8 dev_id;
	int ret = 0;

	printk(KERN_DEBUG "[muic] %s\n", __func__);

	dev_id = sm5705_i2c_read_byte(muic_data->i2c, SM5705_MUIC_REG_DEVID);
	if (dev_id < 0) {
		printk(KERN_ERR "[muic] %s i2c io error(%d)\n",
			__func__, ret);
		ret = -ENODEV;
	} else {
		muic_data->muic_vendor = (dev_id & 0x7);
		muic_data->muic_version = ((dev_id & 0xF8) >> 3);
		printk(KERN_DEBUG "[muic] %s device found: vendor=0x%x, ver=0x%x\n",
				__func__, muic_data->muic_vendor,
				muic_data->muic_version);
	}

	return ret;
}

static int sm5705_muic_irq_init(struct sm5705_muic_data *muic_data)
{
	struct i2c_client *i2c = muic_data->i2c;
	struct muic_platform_data *pdata = muic_data->pdata;
	int ret = 0;

    pr_info("%s:%s\n", MUIC_DEV_NAME, __func__);
	printk(KERN_DEBUG "[muic] %s\n", __func__);

	if (!pdata->irq_gpio) {
		printk(KERN_ERR "[muic] %s No interrupt specified\n",
			__func__);
		return -ENXIO;
	}

	i2c->irq = gpio_to_irq(pdata->irq_gpio);

	if (i2c->irq) {
		ret = request_threaded_irq(i2c->irq, NULL,
				sm5705_muic_irq_thread,
				(IRQF_TRIGGER_FALLING | IRQF_ONESHOT),
				"sm5705-muic", muic_data);
		if (ret < 0) {
			printk(KERN_ERR "[muic] %s failed to reqeust IRQ(%d)\n",
					__func__, i2c->irq);
			return ret;
		}

		ret = enable_irq_wake(i2c->irq);
		if (ret < 0)
			printk(KERN_ERR "[muic] %s failed to enable wakeup src\n",
					__func__);
	}

	return ret;
}

#if defined(CONFIG_OF)
static int of_sm5705_dt(struct device *dev, struct muic_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	int gpio = 0, ret = 0;

    pr_info("%s:%s\n", MUIC_DEV_NAME, __func__);

	if(!np)
		return -EINVAL;

	pdata->irq_gpio = of_get_named_gpio(np, "muic-sm5705,irq-gpio", 0);
	printk(KERN_DEBUG "[muic] irq-gpio: %u\n", pdata->irq_gpio);
    pr_info("%s:pdata->irq_gpio=%d\n", __func__ ,pdata->irq_gpio);

	gpio = of_get_named_gpio(np, "muic-sm5705,uart-gpio", 0);
    pr_info("%s:gpio=%d\n", __func__ ,gpio);
	ret = gpio_is_valid(gpio);
    pr_info("%s:gpio_is_valid=%d\n", __func__ ,ret);
	if (!ret)
		printk(KERN_ERR "[muic] GPIO_UART_SEL is not valid!!!\n");
	else {
		ret = gpio_request(gpio, "GPIO_UART_SEL");
		if (ret) {
			pr_err("failed to gpio_request GPIO_UART_SEL\n");
			return ret;
		}

		muic_pdata.gpio_uart_sel = gpio;

		printk(KERN_DEBUG "[muic] %s: uart-gpio : %d\n",
			__func__, gpio);
	}

	return 0;
}
#endif

static int sm5705_muic_probe(struct i2c_client *i2c,
				const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(i2c->dev.parent);
	struct muic_platform_data *pdata = &muic_pdata;
	struct sm5705_muic_data *muic_data;
	int ret = 0;

    pr_info("%s:%s start\n", MUIC_DEV_NAME, __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		printk(KERN_ERR "[muic] i2c functionality check error\n");
		ret = -EIO;
		goto err_return;
	}

	muic_data = kzalloc(sizeof(struct sm5705_muic_data), GFP_KERNEL);
	if (!muic_data) {
		printk(KERN_ERR "[muic] failed to allocate driver data\n");
		ret = -ENOMEM;
		goto err_return;
	}

	i2c->dev.platform_data = pdata;

	i2c_set_clientdata(i2c, muic_data);

#if defined(CONFIG_OF)
	ret = of_sm5705_dt(&i2c->dev, pdata);
	if (ret < 0) {
		printk(KERN_ERR "[muic] Failed to get device of_node \n");
		goto err_io;
	}

#endif /* CONFIG_OF */
	if (!pdata) {
		printk(KERN_ERR "[muic] %s: failed to get i2c platform data\n", __func__);
		ret = -ENODEV;
		goto err_io;
	}

	mutex_init(&muic_data->muic_mutex);

	muic_data->pdata = pdata;
	muic_data->i2c = i2c;
	muic_data->is_factory_start = false;
	muic_data->is_otg_test = false;
	muic_data->attached_dev = ATTACHED_DEV_UNKNOWN_MUIC;
	muic_data->is_usb_ready = false;

	if (!pdata->set_gpio_uart_sel)
		pdata->set_gpio_uart_sel = sm5705_set_gpio_uart_sel;

	if (muic_data->pdata->init_gpio_cb) {
		ret = muic_data->pdata->init_gpio_cb(get_switch_sel());
		if (ret) {
			printk(KERN_ERR "[muic] failed to init gpio(%d)\n", ret);
		goto fail_init_gpio;
		}
	}

	if (pdata->init_switch_dev_cb)
		pdata->init_switch_dev_cb();

	/* set switch device's driver_data */
	dev_set_drvdata(switch_device, muic_data);

	/* create sysfs group */
	ret = sysfs_create_group(&switch_device->kobj, &sm5705_muic_group);
	if (ret) {
		printk(KERN_ERR "[muic] failed to create sysfs\n");
		goto fail;
	}

	ret = sm5705_init_rev_info(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] failed to init muic rev register(%d)\n", ret);
		goto fail;
	}

	ret = sm5705_muic_reg_init(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] failed to init muic register(%d)\n", ret);
		goto fail;
	}

	//ret = sm5705_i2c_read_byte(muic_data->i2c, SM5705_MUIC_REG_MANSW1);
	//if (ret < 0)
	//	printk(KERN_ERR "[muic] %s: err mansw1 (%d)\n", __func__, ret);

#if 0
	/* RUSTPROOF : disable UART connection if MANSW1
		from BL is OPEN_RUSTPROOF */
	if (ret == MANSW1_OPEN_RUSTPROOF) {
		muic_data->is_rustproof = true;
		com_to_open_with_vbus(muic_data);
	} else
		muic_data->is_rustproof = false;
#else
	muic_data->is_rustproof = pdata->rustproof_on;
#endif

	ret = sm5705_muic_irq_init(muic_data);
	if (ret) {
		printk(KERN_ERR "[muic] failed to init muic irq(%d)\n", ret);
		goto fail_init_irq;
	}

	/* initial cable detection */
	INIT_DELAYED_WORK(&muic_data->init_work, sm5705_muic_init_detect);
	schedule_delayed_work(&muic_data->init_work, msecs_to_jiffies(300));

    pr_info("%s:%s end \n", MUIC_DEV_NAME, __func__);
	return 0;

fail_init_irq:
	if (i2c->irq)
		free_irq(i2c->irq, muic_data);
fail:
	if (muic_data->pdata->cleanup_switch_dev_cb)
		muic_data->pdata->cleanup_switch_dev_cb();
	sysfs_remove_group(&switch_device->kobj, &sm5705_muic_group);
	mutex_unlock(&muic_data->muic_mutex);
	mutex_destroy(&muic_data->muic_mutex);
fail_init_gpio:
	i2c_set_clientdata(i2c, NULL);
err_io:
	kfree(muic_data);
err_return:
	return ret;
}

int charger_muic_disable(int val)
{
	otg_en = val;
	return 1;
}
EXPORT_SYMBOL(charger_muic_disable);


static int __devexit sm5705_muic_remove(struct i2c_client *i2c)
{
	struct sm5705_muic_data *muic_data = i2c_get_clientdata(i2c);
	sysfs_remove_group(&switch_device->kobj, &sm5705_muic_group);

	if (muic_data) {
		printk(KERN_DEBUG "[muic] %s\n", __func__);
		cancel_delayed_work(&muic_data->init_work);
		cancel_delayed_work(&muic_data->usb_work);
		disable_irq_wake(muic_data->i2c->irq);
		free_irq(muic_data->i2c->irq, muic_data);

		if (muic_data->pdata->cleanup_switch_dev_cb)
			muic_data->pdata->cleanup_switch_dev_cb();

		mutex_destroy(&muic_data->muic_mutex);
		i2c_set_clientdata(muic_data->i2c, NULL);
		kfree(muic_data);
	}
	return 0;
}

static const struct i2c_device_id sm5705_i2c_id[] = {
	{ MUIC_DEV_NAME, 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, sm5705_i2c_id);

#if defined(CONFIG_OF)
static struct of_device_id sm5705_i2c_dt_ids[] = {
	{ .compatible = "sm,sm5705-muic" },
	{ },
};
MODULE_DEVICE_TABLE(of, sm5705_i2c_dt_ids);
#endif /* CONFIG_OF */

static void sm5705_muic_shutdown(struct i2c_client *i2c)
{
	struct sm5705_muic_data *muic_data = i2c_get_clientdata(i2c);
	int ret;

	printk(KERN_DEBUG "[muic] %s\n", __func__);
	if (!muic_data->i2c) {
		printk(KERN_ERR "[muic] %s no muic i2c client\n", __func__);
		return;
	}

	printk(KERN_DEBUG "[muic] %s open D+,D-\n", __func__);
	ret = com_to_open_with_vbus(muic_data);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s fail to open mansw1 reg\n",
			__func__);

	/* set auto sw mode before shutdown to make sure device goes into */
	/* LPM charging when TA or USB is connected during off state */
	printk(KERN_DEBUG "[muic] %s muic auto detection enable\n", __func__);
	ret = set_manual_sw(muic_data, true);
	if (ret < 0) {
		printk(KERN_ERR "[muic] %s fail to update reg\n", __func__);
		return;
	}

	if (muic_data->pdata && muic_data->pdata->cleanup_switch_dev_cb)
		muic_data->pdata->cleanup_switch_dev_cb();

}
#if defined(CONFIG_PM)

static int sm5705_muic_suspend(struct device *dev)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct i2c_client *i2c = muic_data->i2c;

	disable_irq_nosync(i2c->irq);

	return 0;
}

static int sm5705_muic_resume(struct device *dev)
{
	struct sm5705_muic_data *muic_data = dev_get_drvdata(dev);
	struct i2c_client *i2c = muic_data->i2c;

	enable_irq(i2c->irq);

	return 0;
}

const struct dev_pm_ops sm5705_muic_pm = {
	.suspend = sm5705_muic_suspend,
	.resume = sm5705_muic_resume,
};
#endif /* CONFIG_PM */

static struct i2c_driver sm5705_muic_driver = {
	.driver		= {
		.name	= MUIC_DEV_NAME,
#if defined(CONFIG_OF)
		.of_match_table	= sm5705_i2c_dt_ids,
#endif /* CONFIG_OF */
#if defined(CONFIG_PM)
		.pm = &sm5705_muic_pm,
#endif /* CONFIG_PM */
	},
	.probe		= sm5705_muic_probe,
	.remove		= __devexit_p(sm5705_muic_remove),
	.shutdown	= sm5705_muic_shutdown,
	.id_table	= sm5705_i2c_id,
};

static int __init sm5705_muic_init(void)
{
    pr_info("%s:%s\n", MUIC_DEV_NAME, __func__);
    printk(KERN_DEBUG "[muic] %s:%s\n",MUIC_DEV_NAME, __func__);

	return i2c_add_driver(&sm5705_muic_driver);
}
module_init(sm5705_muic_init);

static void __exit sm5705_muic_exit(void)
{
	i2c_del_driver(&sm5705_muic_driver);
}
module_exit(sm5705_muic_exit);

MODULE_DESCRIPTION("SM5705 MUIC driver");
MODULE_LICENSE("GPL");

