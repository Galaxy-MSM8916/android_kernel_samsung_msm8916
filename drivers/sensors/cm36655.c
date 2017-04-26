/* driver/sensor/cm36655.c
 * Copyright (c) 2011 SAMSUNG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/wakelock.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include "cm36655.h"
#include <linux/sensor/sensors_core.h>

/* For debugging */
#undef	cm36655_DEBUG

#define	VENDOR		"CAPELLA"
#define	CHIP_ID		"CM36655"

#define I2C_M_WR	0 /* for i2c Write */

/* register addresses */
/* Ambient light sensor */
#define REG_CS_CONF1	0x00
#define REG_ALS_RED_DATA	0x08
#define REG_ALS_GREEN_DATA	0x09 /* ALS */
#define REG_ALS_BLUE_DATA	0x0A
#define REG_WHITE_DATA	0x0B

/* Proximity sensor */
#define REG_PS_CONF1		0x03
#define REG_PS_CONF3		0x04
#define REG_PS_THD		0x05
#define REG_PS_CANC		0x06
#define REG_PS_DATA		0x07

#define ALS_REG_NUM		2
#define PS_REG_NUM		4

#define MSK_L(x)		(x & 0xff)
#define MSK_H(x)		((x & 0xff00) >> 8)

/* Intelligent Cancelation*/
#define CM36655_CANCELATION
#ifdef CM36655_CANCELATION
#define CANCELATION_FILE_PATH	"/efs/FactoryApp/prox_cal"
#define CAL_SKIP_ADC	8
#define CAL_FAIL_ADC	18
#endif

#define PROX_READ_NUM		40

 /* proximity sensor threshold */
#define DEFUALT_HI_THD		0x0011
#define DEFUALT_LOW_THD		0x000D
#define CANCEL_HI_THD		0x000A
#define CANCEL_LOW_THD		0x0007
#define DEFAULT_TRIM		0x0003

 /*lightsnesor log time 6SEC 200mec X 30*/
#define LIGHT_LOG_TIME		30
#define LIGHT_ADD_STARTTIME		300000000
enum {
	LIGHT_ENABLED = BIT(0),
	PROXIMITY_ENABLED = BIT(1),
};

/* register settings */
static u16 als_reg_setting[ALS_REG_NUM][2] = {
	{REG_CS_CONF1, 0x10},	/* enable */
	{REG_CS_CONF1, 0x11},	/* disable */
};

/* Change threshold value on the midas-sensor.c */
enum {
	PS_CONF1 = 0,
	PS_CONF3,
	PS_THD,
	PS_CANCEL,
};
enum {
	REG_ADDR = 0,
	CMD,
};

static u16 ps_reg_init_setting[PS_REG_NUM][2] = {
	{REG_PS_CONF1, 0x7348},	/* REG_PS_CONF1 */ ///
	{REG_PS_CONF3, 0x0000},	/* REG_PS_CONF3 */
	{REG_PS_THD, 0x110D},	/* REG_PS_THD */
	{REG_PS_CANC, DEFAULT_TRIM},	/* REG_PS_CANC */
};

/* driver data */
struct cm36655_data {
	struct i2c_client *i2c_client;
	struct wake_lock prx_wake_lock;
	struct input_dev *proximity_input_dev;
	struct input_dev *light_input_dev;
	struct cm36655_platform_data *pdata;
	struct mutex power_lock;
	struct mutex read_lock;
	struct hrtimer light_timer;
	struct hrtimer prox_timer;
	struct workqueue_struct *light_wq;
	struct workqueue_struct *prox_wq;
	struct work_struct work_light;
	struct work_struct work_prox;
	struct device *proximity_dev;
	struct device *light_dev;
	ktime_t light_poll_delay;
	ktime_t prox_poll_delay;
	int irq;
	u8 power_state;
	int avg[3];
	u16 als_data;
	u16 als_red_data;
	u16 als_green_data;
	u16 als_blue_data;
	u16 white_data;
	int count_log_time;
	unsigned int uProxCalResult;
};

static int prox_regulator_onoff(struct device *dev, bool onoff);

int cm36655_i2c_read_word(struct cm36655_data *cm36655, u8 command, u16 *val)
{
	int err = 0;
	int retry = 3;
	struct i2c_client *client = cm36655->i2c_client;
	struct i2c_msg msg[2];
	unsigned char data[2] = {0,};
	u16 value = 0;

	if ((client == NULL) || (!client->adapter))
		return -ENODEV;

	while (retry--) {
		/* send slave address & command */
		msg[0].addr = client->addr;
		msg[0].flags = I2C_M_WR;
		msg[0].len = 1;
		msg[0].buf = &command;

		/* read word data */
		msg[1].addr = client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = 2;
		msg[1].buf = data;

		err = i2c_transfer(client->adapter, msg, 2);

		if (err >= 0) {
			value = (u16)data[1];
			*val = (value << 8) | (u16)data[0];
			return err;
		}
	}
	pr_err("%s, i2c transfer error ret=%d\n", __func__, err);
	return err;
}

int cm36655_i2c_write_word(struct cm36655_data *cm36655, u8 command,
			   u16 val)
{
	int err = 0;
	struct i2c_client *client = cm36655->i2c_client;
	int retry = 3;

	if ((client == NULL) || (!client->adapter))
		return -ENODEV;

	while (retry--) {
		err = i2c_smbus_write_word_data(client, command, val);
		if (err >= 0)
			return 0;
	}
	pr_err("%s, i2c transfer error(%d)\n", __func__, err);
	return err;
}

static void cm36655_light_enable(struct cm36655_data *cm36655)
{
	/* enable setting */
	cm36655_i2c_write_word(cm36655, REG_CS_CONF1,
		als_reg_setting[0][1]);
	hrtimer_start(&cm36655->light_timer, ns_to_ktime(200 * NSEC_PER_MSEC),
		HRTIMER_MODE_REL);
}

static void cm36655_light_disable(struct cm36655_data *cm36655)
{
	/* disable setting */
	cm36655_i2c_write_word(cm36655, REG_CS_CONF1,
		als_reg_setting[1][1]);
	hrtimer_cancel(&cm36655->light_timer);
	cancel_work_sync(&cm36655->work_light);
}

/* sysfs */
static ssize_t cm36655_poll_delay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%lld\n",
		ktime_to_ns(cm36655->light_poll_delay));
}

static ssize_t cm36655_poll_delay_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	int64_t new_delay;
	int err;

	err = kstrtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	mutex_lock(&cm36655->power_lock);
	if (new_delay != ktime_to_ns(cm36655->light_poll_delay)) {
		cm36655->light_poll_delay = ns_to_ktime(new_delay);
		if (cm36655->power_state & LIGHT_ENABLED) {
			cm36655_light_disable(cm36655);
			cm36655_light_enable(cm36655);
		}
		pr_info("[SENSOR] %s, poll_delay = %lld\n",
			__func__, new_delay);
	}
	mutex_unlock(&cm36655->power_lock);

	return size;
}

static ssize_t light_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		pr_err("[SENSOR] %s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&cm36655->power_lock);
	pr_info("[SENSOR] %s,new_value=%d\n", __func__, new_value);
	if (new_value && !(cm36655->power_state & LIGHT_ENABLED)) {
		cm36655->power_state |= LIGHT_ENABLED;
		cm36655_light_enable(cm36655);
	} else if (!new_value && (cm36655->power_state & LIGHT_ENABLED)) {
		cm36655_light_disable(cm36655);
		cm36655->power_state &= ~LIGHT_ENABLED;
	}
	mutex_unlock(&cm36655->power_lock);
	return size;
}

static ssize_t light_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",
		(cm36655->power_state & LIGHT_ENABLED) ? 1 : 0);
}

#ifdef CM36655_CANCELATION
static int proximity_open_cancelation(struct cm36655_data *data)
{
	struct file *cancel_filp = NULL;
	int err = 0;
	u16 buf = 0;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cancel_filp = filp_open(CANCELATION_FILE_PATH, O_RDONLY, 0);
	if (IS_ERR(cancel_filp)) {
		err = PTR_ERR(cancel_filp);
		if (err != -ENOENT)
			pr_err("[SENSOR] %s: Can't open cancelation file\n",
				__func__);
		set_fs(old_fs);
		return err;
	}

	err = cancel_filp->f_op->read(cancel_filp, (char *)&buf,
		sizeof(u16), &cancel_filp->f_pos);
	if (err != sizeof(u16)) {
		pr_err("%s: Can't read the cancel data from file\n", __func__);
		err = -EIO;
	}

	if (buf < CAL_SKIP_ADC)
		goto exit;

	ps_reg_init_setting[PS_CANCEL][CMD] = buf;
	/*If there is an offset cal data. */
	if ((ps_reg_init_setting[PS_CANCEL][CMD] != data->pdata->default_trim)
		&& (ps_reg_init_setting[PS_CANCEL][CMD] != 0)) {
		ps_reg_init_setting[PS_THD][CMD] =
			((data->pdata->cancel_hi_thd << 8) & 0xff00)
			| (data->pdata->cancel_low_thd & 0xff);
	}

exit:
	pr_info("%s prox_cal[%d] PS_THD[%x]\n", __func__,
		ps_reg_init_setting[PS_CANCEL][CMD], ps_reg_init_setting[PS_THD][CMD]);

	filp_close(cancel_filp, current->files);
	set_fs(old_fs);

	return err;
}

static int proximity_store_cancelation(struct device *dev, bool do_calib)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	struct file *cancel_filp = NULL;
	mm_segment_t old_fs;
	int err = 0;
	u16 ps_data = 0;

	if (do_calib) {
		mutex_lock(&cm36655->read_lock);
		cm36655_i2c_read_word(cm36655, REG_PS_DATA, &ps_data);
		ps_reg_init_setting[PS_CANCEL][CMD] = ps_data;
		mutex_unlock(&cm36655->read_lock);

		if (ps_reg_init_setting[PS_CANCEL][CMD] < CAL_SKIP_ADC) {
			ps_reg_init_setting[PS_CANCEL][CMD] = cm36655->pdata->default_trim;
			pr_info("%s:crosstalk <= %d SKIP!!\n", __func__, CAL_SKIP_ADC);
			cm36655->uProxCalResult = 2;
			err = 1;
		} else if (ps_reg_init_setting[PS_CANCEL][CMD] < CAL_FAIL_ADC) {
			ps_reg_init_setting[PS_CANCEL][CMD] =
					cm36655->pdata->default_trim+ps_data;

			pr_info("%s:crosstalk_offset = %u Canceled", __func__,
				ps_reg_init_setting[PS_CANCEL][CMD]);
			cm36655->uProxCalResult = 1;
			err = 0;
			ps_reg_init_setting[PS_THD][CMD] =
				((cm36655->pdata->cancel_hi_thd<< 8) & 0xff00)
				| (cm36655->pdata->cancel_low_thd & 0xff);
		} else {
			ps_reg_init_setting[PS_CANCEL][CMD] = cm36655->pdata->default_trim;
			pr_info("%s:crosstalk >= %d\n FAILED!!", __func__, CAL_FAIL_ADC);
			ps_reg_init_setting[PS_THD][CMD] =
				((cm36655->pdata->default_hi_thd << 8) & 0xff00)
				| (cm36655->pdata->default_low_thd & 0xff);
			cm36655->uProxCalResult = 0;
			err = 1;
		}
	} else { /* reset */
		ps_reg_init_setting[PS_CANCEL][CMD] = cm36655->pdata->default_trim;
		ps_reg_init_setting[PS_THD][CMD] =
			((cm36655->pdata->default_hi_thd << 8) & 0xff00)
			| (cm36655->pdata->default_low_thd & 0xff);
	}

	err = cm36655_i2c_write_word(cm36655, REG_PS_CANC,
		ps_reg_init_setting[PS_CANCEL][CMD]);
	if (err < 0)
		pr_err("%s: cm36655_ps_canc_reg is failed. %d\n", __func__,
			err);

	usleep_range(2900, 3000);
	err = cm36655_i2c_write_word(cm36655, REG_PS_THD,
		ps_reg_init_setting[PS_THD][CMD]);
	if (err < 0)
		pr_err("%s: cm36655_ps_canc_reg is failed. %d\n", __func__,
			err);
	pr_info("%s CalResult[%d] PS_THD[%x]\n", __func__,
		cm36655->uProxCalResult, ps_reg_init_setting[PS_THD][CMD]);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cancel_filp = filp_open(CANCELATION_FILE_PATH,
			O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0660);
	if (IS_ERR(cancel_filp)) {
		pr_err("[SENSOR] %s: Can't open cancelation file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cancel_filp);
		return err;
	}

	err = cancel_filp->f_op->write(cancel_filp,
		(char *)&ps_reg_init_setting[PS_CANCEL][CMD],
		sizeof(u16), &cancel_filp->f_pos);
	if (err != sizeof(u16)) {
		pr_err("%s: Can't write the cancel data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cancel_filp, current->files);
	set_fs(old_fs);

	if (!do_calib) /* delay for clearing */
		msleep(150);

	return err;
}

static ssize_t proximity_cancel_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	bool do_calib;
	int err;

	if (sysfs_streq(buf, "1")) /* calibrate cancelation value */
		do_calib = true;
	else if (sysfs_streq(buf, "0")) /* reset cancelation value */
		do_calib = false;
	else {
		pr_debug("[SENSOR] %s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	err = proximity_store_cancelation(dev, do_calib);
	if (err < 0) {
		pr_err("[SENSOR] %s: proximity_store_cancelation() failed\n",
			__func__);
		return err;
	}

	return size;
}

static ssize_t proximity_cancel_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	pr_info("cm366 %s = %u,%u,%u\n",__func__, ps_reg_init_setting[PS_CANCEL][CMD],
	ps_reg_init_setting[PS_THD][CMD] >> 8, ps_reg_init_setting[PS_THD][CMD] &0x00ff);

	return snprintf(buf, PAGE_SIZE, "%u,%u,%u\n", ps_reg_init_setting[PS_CANCEL][CMD],
	ps_reg_init_setting[PS_THD][CMD] >> 8, ps_reg_init_setting[PS_THD][CMD] &0x00ff);
}

static ssize_t proximity_cancel_pass_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	pr_info("%s, %u\n", __func__, cm36655->uProxCalResult);
	return snprintf(buf, PAGE_SIZE, "%u\n", cm36655->uProxCalResult);
}
#endif

#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
static int cm36655_setup_leden_gpio(struct cm36655_data *cm36655)
{
	int rc;
	struct cm36655_platform_data *pdata = cm36655->pdata;

	rc = gpio_request(pdata->leden_gpio, "rgb_en");
	if (rc < 0) {
		pr_err("%s: gpio %d request failed (%d)\n",
			__func__, pdata->leden_gpio, rc);
	}
	gpio_direction_output(pdata->leden_gpio, 1);
	pr_info("%s: gpio %d request success\n", __func__, pdata->leden_gpio);
	return rc;
}

static int cm36655_leden_gpio_onoff(struct cm36655_data *cm36655, bool onoff)
{
	struct cm36655_platform_data *pdata = cm36655->pdata;
	gpio_set_value(pdata->leden_gpio, onoff);
	pr_info("%s onoff:%d\n", __func__, onoff);
	if (onoff)
		msleep(20);
	return 0;
}
#endif

#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
static int cm36655_setup_sensoren_gpio(struct cm36655_data *cm36655)
{
	int rc;
	struct cm36655_platform_data *pdata = cm36655->pdata;

	rc = gpio_request(pdata->sensoren_gpio, "sensor_en");
	if (rc < 0) {
		pr_err("%s: gpio %d request failed (%d)\n",
			__func__, pdata->sensoren_gpio, rc);
	}
	gpio_direction_output(pdata->sensoren_gpio, 1);
	pr_info("%s: gpio %d request success\n", __func__, pdata->sensoren_gpio);
	return rc;
}

static int cm36655_sensoren_gpio_onoff(struct cm36655_data *cm36655, bool onoff)
{
	struct cm36655_platform_data *pdata = cm36655->pdata;
	gpio_set_value(pdata->sensoren_gpio, onoff);
	pr_info("%s onoff:%d\n", __func__, onoff);
	if (onoff)
		msleep(20);
	return 0;
}
#endif

static ssize_t proximity_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	bool new_value;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		pr_err("[SENSOR] %s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	mutex_lock(&cm36655->power_lock);
	pr_info("%s, new_value = %d\n", __func__, new_value);
	if (new_value && !(cm36655->power_state & PROXIMITY_ENABLED)) {
		u8 val = 1;
		int i;
		int err = 0;

		cm36655->power_state |= PROXIMITY_ENABLED;
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
		cm36655_leden_gpio_onoff(cm36655, 1);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
		prox_regulator_onoff(&cm36655->i2c_client->dev, 1);
		cm36655_sensoren_gpio_onoff(cm36655, 1);
#endif
#ifdef CM36655_CANCELATION
		/* open cancelation data */
		err = proximity_open_cancelation(cm36655);
		if (err < 0 && err != -ENOENT)
			pr_err("[SENSOR] %s: proximity_open_cancelation() failed\n",
				__func__);
#endif
		/* enable settings */
		for (i = 0; i < PS_REG_NUM; i++) {
			cm36655_i2c_write_word(cm36655,
				ps_reg_init_setting[i][REG_ADDR],
				ps_reg_init_setting[i][CMD]);
		}
		/*send  the far for input update*/
		input_report_abs(cm36655->proximity_input_dev, ABS_DISTANCE,val);
		val = gpio_get_value(cm36655->pdata->irq);
		/* 0 is close, 1 is far */
		input_report_abs(cm36655->proximity_input_dev, ABS_DISTANCE,val);
		input_sync(cm36655->proximity_input_dev);

		enable_irq(cm36655->irq);
		enable_irq_wake(cm36655->irq);
	} else if (!new_value && (cm36655->power_state & PROXIMITY_ENABLED)) {
		cm36655->power_state &= ~PROXIMITY_ENABLED;

		disable_irq_wake(cm36655->irq);
		disable_irq(cm36655->irq);
		/* disable settings */
		cm36655_i2c_write_word(cm36655, REG_PS_CONF1, 0x0001);
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
		cm36655_leden_gpio_onoff(cm36655, 0);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
		prox_regulator_onoff(&cm36655->i2c_client->dev, 0);
		cm36655_sensoren_gpio_onoff(cm36655, 0);
#endif
	}
	mutex_unlock(&cm36655->power_lock);
	return size;
}

static ssize_t proximity_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
		(cm36655->power_state & PROXIMITY_ENABLED) ? 1 : 0);
}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
	cm36655_poll_delay_show, cm36655_poll_delay_store);

static struct device_attribute dev_attr_light_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	light_enable_show, light_enable_store);

static struct device_attribute dev_attr_proximity_enable =
	__ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_enable_show, proximity_enable_store);

static struct attribute *light_sysfs_attrs[] = {
	&dev_attr_light_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static struct attribute_group light_attribute_group = {
	.attrs = light_sysfs_attrs,
};

static struct attribute *proximity_sysfs_attrs[] = {
	&dev_attr_proximity_enable.attr,
	NULL
};

static struct attribute_group proximity_attribute_group = {
	.attrs = proximity_sysfs_attrs,
};

/* sysfs for vendor & name */
static ssize_t cm36655_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", VENDOR);
}

static ssize_t cm36655_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", CHIP_ID);
}
static struct device_attribute dev_attr_prox_sensor_vendor =
	__ATTR(vendor, S_IRUSR | S_IRGRP, cm36655_vendor_show, NULL);
static struct device_attribute dev_attr_light_sensor_vendor =
	__ATTR(vendor, S_IRUSR | S_IRGRP, cm36655_vendor_show, NULL);
static struct device_attribute dev_attr_prox_sensor_name =
	__ATTR(name, S_IRUSR | S_IRGRP, cm36655_name_show, NULL);
static struct device_attribute dev_attr_light_sensor_name =
	__ATTR(name, S_IRUSR | S_IRGRP, cm36655_name_show, NULL);

/* proximity sysfs */
static ssize_t proximity_trim_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u\n", cm36655->pdata->default_trim);
}
static ssize_t proximity_avg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n", cm36655->avg[0],
		cm36655->avg[1], cm36655->avg[2]);
}

static ssize_t proximity_avg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	bool new_value = false;

	if (sysfs_streq(buf, "1"))
		new_value = true;
	else if (sysfs_streq(buf, "0"))
		new_value = false;
	else {
		pr_err("[SENSOR] %s, invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	pr_info("[SENSOR] %s, average enable = %d\n", __func__, new_value);
	mutex_lock(&cm36655->power_lock);
	if (new_value) {
		if (!(cm36655->power_state & PROXIMITY_ENABLED)) {
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
			cm36655_leden_gpio_onoff(cm36655, 1);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
			prox_regulator_onoff(&cm36655->i2c_client->dev, 1);
			cm36655_sensoren_gpio_onoff(cm36655, 1);
#endif
			cm36655_i2c_write_word(cm36655, REG_PS_CONF1,
				ps_reg_init_setting[PS_CONF1][CMD]);
		}
		hrtimer_start(&cm36655->prox_timer, cm36655->prox_poll_delay,
			HRTIMER_MODE_REL);
	} else if (!new_value) {
		hrtimer_cancel(&cm36655->prox_timer);
		cancel_work_sync(&cm36655->work_prox);
		if (!(cm36655->power_state & PROXIMITY_ENABLED)) {
			cm36655_i2c_write_word(cm36655, REG_PS_CONF1,
				0x0001);
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
			cm36655_leden_gpio_onoff(cm36655, 0);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
			prox_regulator_onoff(&cm36655->i2c_client->dev, 0);
			cm36655_sensoren_gpio_onoff(cm36655, 0);
#endif
		}
	}
	mutex_unlock(&cm36655->power_lock);

	return size;
}

static ssize_t proximity_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	u16 ps_data;

	mutex_lock(&cm36655->power_lock);
	if (!(cm36655->power_state & PROXIMITY_ENABLED)) {
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
		cm36655_leden_gpio_onoff(cm36655, 1);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
		prox_regulator_onoff(&cm36655->i2c_client->dev, 1);
		cm36655_sensoren_gpio_onoff(cm36655, 1);
#endif
		pr_info("%s prox_led_on\n", __func__);
		cm36655_i2c_write_word(cm36655, REG_PS_CONF1,
			ps_reg_init_setting[PS_CONF1][CMD]);
	}

	mutex_lock(&cm36655->read_lock);
	cm36655_i2c_read_word(cm36655, REG_PS_DATA, &ps_data);
	mutex_unlock(&cm36655->read_lock);

	if (!(cm36655->power_state & PROXIMITY_ENABLED)) {
		cm36655_i2c_write_word(cm36655, REG_PS_CONF1, 0x0001);
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
		cm36655_leden_gpio_onoff(cm36655, 0);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
		prox_regulator_onoff(&cm36655->i2c_client->dev, 0);
		cm36655_sensoren_gpio_onoff(cm36655, 0);
#endif
	}
	mutex_unlock(&cm36655->power_lock);

	return snprintf(buf, PAGE_SIZE, "%u\n", ps_data);
}

static ssize_t proximity_thresh_high_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int high, low;

	high = ps_reg_init_setting[PS_THD][CMD] >> 8;
	low = ps_reg_init_setting[PS_THD][CMD] &0x00ff;
	pr_info("cm366 %s = %u,%u\n", __func__, high, low);
	return snprintf(buf, PAGE_SIZE, "%u,%u\n", high, low);
}

static ssize_t proximity_thresh_high_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	u16 thresh_value;
	int err;

	err = kstrtou16(buf, 10, &thresh_value);
	if (err < 0)
		pr_err("%s, kstrtoint failed.\n", __func__);
	pr_info("%s, thresh_value:%u\n", __func__, thresh_value);

	if (thresh_value > 2) {
		ps_reg_init_setting[PS_THD][CMD] =
			(ps_reg_init_setting[PS_THD][CMD] & 0xff) | ((thresh_value << 8) & 0xff00);
		err = cm36655_i2c_write_word(cm36655, REG_PS_THD,
			ps_reg_init_setting[PS_THD][CMD]);
		if (err < 0)
			pr_err("%s: cm36655_ps_high_reg is failed. %d\n",
				__func__, err);
		pr_info("%s, new high threshold = 0x%x\n",
			__func__, thresh_value);
		msleep(150);
	} else
		pr_err("%s, wrong high threshold value(0x%x)!!\n",
			__func__, thresh_value);

	return size;
}

static ssize_t proximity_thresh_low_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int high, low;

	high = ps_reg_init_setting[PS_THD][CMD] >> 8;
	low = ps_reg_init_setting[PS_THD][CMD] &0x00ff;
	pr_info("%s = %u,%u\n", __func__, high, low);

	return snprintf(buf, PAGE_SIZE, "%u,%u\n", high, low);
}

static ssize_t proximity_thresh_low_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
	u16 thresh_value;
	int err;

	err = kstrtou16(buf, 10, &thresh_value);
	if (err < 0)
		pr_err("%s, kstrtoint failed.", __func__);
	pr_info("%s, thresh_value:%u\n", __func__, thresh_value);

	if (thresh_value > 2) {
		ps_reg_init_setting[PS_THD][CMD] =
			(ps_reg_init_setting[PS_THD][CMD] & 0xff00) | (thresh_value & 0x00ff) ;
		err = cm36655_i2c_write_word(cm36655, REG_PS_THD,
			ps_reg_init_setting[PS_THD][CMD]);
		if (err < 0)
			pr_err("%s: cm36655_ps_low_reg is failed. %d\n",
				__func__, err);
		pr_info("%s, new low threshold = 0x%x\n",
			__func__, thresh_value);
		msleep(150);
	} else
		pr_err("%s, wrong low threshold value(0x%x)!!\n",
			__func__, thresh_value);

	return size;
}

#ifdef CM36655_CANCELATION
static DEVICE_ATTR(prox_cal, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_cancel_show, proximity_cancel_store);
static DEVICE_ATTR(prox_offset_pass, S_IRUGO, proximity_cancel_pass_show, NULL);
#endif
static DEVICE_ATTR(prox_avg, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_avg_show, proximity_avg_store);
static DEVICE_ATTR(state, S_IRUGO, proximity_state_show, NULL);
static struct device_attribute dev_attr_prox_raw = __ATTR(raw_data,
	S_IRUGO, proximity_state_show, NULL);
static DEVICE_ATTR(thresh_high, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_thresh_high_show, proximity_thresh_high_store);
static DEVICE_ATTR(thresh_low, S_IRUGO | S_IWUSR | S_IWGRP,
	proximity_thresh_low_show, proximity_thresh_low_store);
static DEVICE_ATTR(prox_trim, S_IRUSR | S_IRGRP,
	proximity_trim_show, NULL);

static struct device_attribute *prox_sensor_attrs[] = {
	&dev_attr_prox_sensor_vendor,
	&dev_attr_prox_sensor_name,
	&dev_attr_prox_cal,
	&dev_attr_prox_offset_pass,
	&dev_attr_prox_avg,
	&dev_attr_state,
	&dev_attr_thresh_high,
	&dev_attr_thresh_low,
	&dev_attr_prox_raw,
	&dev_attr_prox_trim,
	NULL,
};

/* light sysfs */
static ssize_t light_lux_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u,%u\n", cm36655->als_data,
		cm36655->white_data);
}

static ssize_t light_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);
#ifdef cm36655_DEBUG
	pr_info("%s = %u,%u,%u,%u\n", __func__, cm36655->als_red_data,
		cm36655->als_green_data, cm36655->als_blue_data, cm36655->white_data);
#endif
	return snprintf(buf, PAGE_SIZE, "%u,%u,%u,%u\n", cm36655->als_red_data,
		cm36655->als_green_data, cm36655->als_blue_data, cm36655->white_data);
}

static DEVICE_ATTR(lux, S_IRUGO, light_lux_show, NULL);
static DEVICE_ATTR(raw_data, S_IRUGO, light_data_show, NULL);

static struct device_attribute *light_sensor_attrs[] = {
	&dev_attr_light_sensor_vendor,
	&dev_attr_light_sensor_name,
	&dev_attr_lux,
	&dev_attr_raw_data,
	NULL,
};

/* interrupt happened due to transition/change of near/far proximity state */
irqreturn_t cm36655_irq_thread_fn(int irq, void *data)
{
	struct cm36655_data *cm36655 = data;
	u8 val = 1;
	u16 ps_data = 0;
#ifdef cm36655_DEBUG
	static int count;
	pr_info("%s\n", __func__);
#endif

	val = gpio_get_value(cm36655->pdata->irq);
	cm36655_i2c_read_word(cm36655, REG_PS_DATA, &ps_data);
#ifdef cm36655_DEBUG
	pr_info("[SENSOR] %s: count = %d\n", __func__, count++);
#endif

	if (cm36655->power_state & PROXIMITY_ENABLED) {
		/* 0 is close, 1 is far */
		input_report_abs(cm36655->proximity_input_dev, ABS_DISTANCE,
			val);
		input_sync(cm36655->proximity_input_dev);
	}

	wake_lock_timeout(&cm36655->prx_wake_lock, 3 * HZ);

	pr_info("%s: val = %u, ps_data = %u (close:0, far:1)\n",
		__func__, val, ps_data);

	return IRQ_HANDLED;
}

static int cm36655_setup_reg(struct cm36655_data *cm36655)
{
	int err = 0, i = 0;
	u16 tmp = 0;

	/* ALS initialization */
	err = cm36655_i2c_write_word(cm36655,
			als_reg_setting[0][0],
			als_reg_setting[0][1]);
	if (err < 0) {
		pr_err("[SENSOR] %s: cm36655_als_reg is failed. %d\n", __func__,
			err);
		return err;
	}
	/* PS initialization */
	for (i = 0; i < PS_REG_NUM; i++) {
		err = cm36655_i2c_write_word(cm36655,
			ps_reg_init_setting[i][REG_ADDR],
			ps_reg_init_setting[i][CMD]);
		if (err < 0) {
			pr_err("[SENSOR] %s: cm36655_ps_reg is failed. %d\n",
				__func__, err);
			return err;
		}
	}

	/* printing the inital proximity value with no contact */
	msleep(50);
	mutex_lock(&cm36655->read_lock);
	err = cm36655_i2c_read_word(cm36655, REG_PS_DATA, &tmp);
	mutex_unlock(&cm36655->read_lock);
	if (err < 0) {
		pr_err("[SENSOR] %s: read ps_data failed\n", __func__);
		err = -EIO;
	}
	pr_err("%s: initial proximity value = %d\n",
		__func__, tmp);

	/* turn off */
	cm36655_i2c_write_word(cm36655, REG_CS_CONF1, 0x0001);
	cm36655_i2c_write_word(cm36655, REG_PS_CONF1, 0x0001);
	cm36655_i2c_write_word(cm36655, REG_PS_CONF3, 0x0000);

	pr_info("[SENSOR] %s is success.", __func__);
	return err;
}

static int cm36655_setup_irq(struct cm36655_data *cm36655)
{
	int rc = -EIO;
	struct cm36655_platform_data *pdata = cm36655->pdata;

	rc = gpio_request(pdata->irq, "gpio_proximity_out");
	if (rc < 0) {
		pr_err("%s: gpio %d request failed (%d)\n",
			__func__, pdata->irq, rc);
		return rc;
	}

	rc = gpio_direction_input(pdata->irq);
	if (rc < 0) {
		pr_err("%s: failed to set gpio %d as input (%d)\n",
			__func__, pdata->irq, rc);
		goto err_gpio_direction_input;
	}

	cm36655->irq = gpio_to_irq(pdata->irq);
	rc = request_threaded_irq(cm36655->irq, NULL, cm36655_irq_thread_fn,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"proximity_int", cm36655);
	if (rc < 0) {
		pr_err("%s: irq:%d failed for qpio:%d err:%d\n",
			__func__, cm36655->irq, pdata->irq, rc);
		goto err_request_irq;
	}

	/* start with interrupts disabled */
	disable_irq(cm36655->irq);

	pr_err("[SENSOR] %s, dir out success\n", __func__);

	goto done;

err_request_irq:
err_gpio_direction_input:
	gpio_free(pdata->irq);
done:
	return rc;
}

/* This function is for light sensor.  It operates every a few seconds.
 * It asks for work to be done on a thread because i2c needs a thread
 * context (slow and blocking) and then reschedules the timer to run again.
 */
static enum hrtimer_restart cm36655_light_timer_func(struct hrtimer *timer)
{
	struct cm36655_data *cm36655
		= container_of(timer, struct cm36655_data, light_timer);
	queue_work(cm36655->light_wq, &cm36655->work_light);
	hrtimer_forward_now(&cm36655->light_timer, cm36655->light_poll_delay);
	return HRTIMER_RESTART;
}

static void cm36655_work_func_light(struct work_struct *work)
{
	struct cm36655_data *cm36655 = container_of(work, struct cm36655_data,
						work_light);
	mutex_lock(&cm36655->read_lock);
	cm36655_i2c_read_word(cm36655, REG_ALS_RED_DATA, &cm36655->als_red_data);
	cm36655_i2c_read_word(cm36655, REG_ALS_GREEN_DATA, &cm36655->als_green_data);
	cm36655_i2c_read_word(cm36655, REG_ALS_BLUE_DATA, &cm36655->als_blue_data);
	cm36655_i2c_read_word(cm36655, REG_WHITE_DATA, &cm36655->white_data);
	mutex_unlock(&cm36655->read_lock);

cm36655->als_data = cm36655->als_green_data;
	input_report_rel(cm36655->light_input_dev, REL_HWHEEL,
		cm36655->als_red_data + 1);
	input_report_rel(cm36655->light_input_dev, REL_DIAL,
		cm36655->als_green_data + 1);
	input_report_rel(cm36655->light_input_dev, REL_WHEEL,
		cm36655->als_blue_data + 1);
	input_report_rel(cm36655->light_input_dev, REL_MISC,
		cm36655->white_data + 1);
	input_sync(cm36655->light_input_dev);

	if (cm36655->count_log_time >= LIGHT_LOG_TIME) {
		pr_info("%s, %u,%u\n", __func__,
			cm36655->als_data, cm36655->white_data);
		cm36655->count_log_time = 0;
	} else
		cm36655->count_log_time++;

#ifdef cm36655_DEBUG
	pr_info("%s, %u,%u\n", __func__,
		cm36655->als_data, cm36655->white_data);
#endif
}

static void proxsensor_get_avg_val(struct cm36655_data *cm36655)
{
	int min = 0, max = 0, avg = 0;
	int i;
	u16 ps_data = 0;

	for (i = 0; i < PROX_READ_NUM; i++) {
		msleep(40);
		cm36655_i2c_read_word(cm36655, REG_PS_DATA,
			&ps_data);
		avg += ps_data;

		if (!i)
			min = ps_data;
		else if (ps_data < min)
			min = ps_data;

		if (ps_data > max)
			max = ps_data;
	}
	avg /= PROX_READ_NUM;

	cm36655->avg[0] = min;
	cm36655->avg[1] = avg;
	cm36655->avg[2] = max;
}

static void cm36655_work_func_prox(struct work_struct *work)
{
	struct cm36655_data *cm36655 = container_of(work, struct cm36655_data,
						  work_prox);
	proxsensor_get_avg_val(cm36655);
}

static enum hrtimer_restart cm36655_prox_timer_func(struct hrtimer *timer)
{
	struct cm36655_data *cm36655
			= container_of(timer, struct cm36655_data, prox_timer);
	queue_work(cm36655->prox_wq, &cm36655->work_prox);
	hrtimer_forward_now(&cm36655->prox_timer, cm36655->prox_poll_delay);
	return HRTIMER_RESTART;
}

#ifdef CONFIG_OF

/* device tree parsing function */
static int cm36655_parse_dt(struct device *dev,
	struct cm36655_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	enum of_gpio_flags flags;
	int ret;

	pdata->irq = of_get_named_gpio_flags(np, "cm36655,irq_gpio", 0, &flags);
	if (pdata->irq < 0) {
		pr_err("[SENSOR]: %s - get prox_int error\n", __func__);
		return -ENODEV;
	}

#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
	pdata->leden_gpio = of_get_named_gpio_flags(np, "cm36655,leden_gpio", 0,
		&flags);
	if (pdata->leden_gpio < 0) {
		pr_err("[SENSOR]: %s - get prox_leden_gpio error\n", __func__);
		return -ENODEV;
	}
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
	pdata->sensoren_gpio = of_get_named_gpio_flags(np, "cm36655,en-gpio", 0,
		&flags);
	if (pdata->sensoren_gpio < 0) {
		pr_err("[SENSOR]: %s - get prox_sensoren_gpio error\n", __func__);
		return -ENODEV;
	}
#endif

	ret = of_property_read_u32(np, "cm36655,default_hi_thd",
	&pdata->default_hi_thd);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - Cannot set default_hi_thd through DTSI error!!\n",
			__func__);
		pdata->default_hi_thd = DEFUALT_HI_THD;
	}

	ret = of_property_read_u32(np, "cm36655,default_low_thd",
		&pdata->default_low_thd);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - Cannot set default_low_thd through DTSI error!!\n",
			__func__);
		pdata->default_low_thd = DEFUALT_LOW_THD;
	}

	ret = of_property_read_u32(np, "cm36655,cancel_hi_thd",
		&pdata->cancel_hi_thd);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - Cannot set cancel_hi_thd through DTSI error!!\n",
			__func__);
		pdata->cancel_hi_thd = CANCEL_HI_THD;
	}

	ret = of_property_read_u32(np, "cm36655,cancel_low_thd",
		&pdata->cancel_low_thd);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - Cannot set cancel_low_thd through DTSI error!!\n",
			__func__);
		pdata->cancel_low_thd = CANCEL_LOW_THD;
	}
	ps_reg_init_setting[PS_THD][CMD] =
		((pdata->default_hi_thd << 8) & 0xff00) | (pdata->default_low_thd & 0xff);

	ret = of_property_read_u32(np, "cm36655,default_trim",
		&pdata->default_trim);
	if (ret < 0) {
		pr_err("[SENSOR] %s, Cannot set default_trim\n", __func__);
		pdata->default_trim = DEFAULT_TRIM;
	}

	pr_info("%s DefaultTHS[%d/%d] CancelTHD[%d/%d] PS_THD[%x] DefaultTRIM[%d]\n", __func__,
		pdata->default_hi_thd, pdata->default_low_thd,
		pdata->cancel_hi_thd, pdata->cancel_low_thd, ps_reg_init_setting[PS_THD][CMD],
		pdata->default_trim);
	return 0;
}
#else
static int cm36655_parse_dt(struct device *dev, struct cm36655_platform_data)
{
	return -ENODEV;
}
#endif

static int prox_regulator_onoff(struct device *dev, bool onoff)
{
	struct regulator *vdd;
	struct regulator *vio;
	int ret = 0;

	pr_info("%s %s\n", __func__, (onoff) ? "on" : "off");

	vdd = devm_regulator_get(dev, "cm36655,vdd");
	if (IS_ERR(vdd)) {
		pr_err("%s: cannot get vdd\n", __func__);
		ret = -ENOMEM;
		goto err_vdd;
	} else if (!regulator_get_voltage(vdd)) {
		ret = regulator_set_voltage(vdd, 3300000, 3300000);
	}

	vio = devm_regulator_get(dev, "cm36655,vio");
	if (IS_ERR(vio)) {
		pr_err("%s: cannot get vio\n", __func__);
		ret = -ENOMEM;
		goto err_vio;
	} else if (!regulator_get_voltage(vio)) {
		ret = regulator_set_voltage(vio, 1800000, 1800000);
	}

	if (onoff) {
		ret = regulator_enable(vdd);
		if (ret) {
			pr_err("%s: Failed to enable vdd.\n", __func__);
		}
		msleep(20);
		ret = regulator_enable(vio);
		if (ret) {
			pr_err("%s: Failed to enable vio.\n", __func__);
		}
		msleep(20);
	} else {
		ret = regulator_disable(vdd);
		if (ret) {
			pr_err("%s: Failed to enable vdd.\n", __func__);
		}
		msleep(20);
		ret = regulator_disable(vio);
		if (ret) {
			pr_err("%s: Failed to enable vio.\n", __func__);
		}
		msleep(20);
	}

	devm_regulator_put(vio);
err_vio:
	devm_regulator_put(vdd);
err_vdd:
	return ret;
}

#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
static int cm36655_pin_ctrl(struct device *dev)
{
	int ret;
	struct pinctrl *pinctrl;
	struct pinctrl_state *suspend;
	struct pinctrl_state *active;

       /* Get pinctrl if target uses pinctrl */
	pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(pinctrl)) {
		pr_info("%s devm_pinctrl_get fail\n", __func__);
		return -EINVAL;
	} else {
		active = pinctrl_lookup_state(pinctrl, "prox_active");
		if (IS_ERR(active)) {
			pr_info("%s fail to active lookup_state\n", __func__);
			ret = -EINVAL;
			goto exit;
		}
		suspend = pinctrl_lookup_state(pinctrl, "prox_suspend");
		if (IS_ERR(suspend)) {
			pr_info("%s fail to suspend lookup_state\n", __func__);
			ret = -EINVAL;
			goto exit;
		}
		ret = pinctrl_select_state(pinctrl, active);
		if (ret != 0) {
			pr_err("%s: fail to select_state active\n", __func__);
			ret = -EIO;
			goto exit;
		}
		ret = pinctrl_select_state(pinctrl, suspend);
		if (ret != 0) {
			pr_err("%s: fail to select_state suspend\n", __func__);
			ret = -EIO;
			goto exit;
		}
	}
exit:
	devm_pinctrl_put(pinctrl);

	return ret;
}
#endif
static int cm36655_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = -ENODEV;
	int err;
	struct cm36655_data *cm36655 = NULL;
	struct cm36655_platform_data *pdata = NULL;

	pr_info("[SENSOR] %s: Probe Start!\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[SENSOR] %s: i2c functionality check failed!\n",
			__func__);
		return ret;
	}

	cm36655 = kzalloc(sizeof(struct cm36655_data), GFP_KERNEL);
	if (!cm36655) {
		pr_err("[SENSOR] %s: failed to alloc memory for RGB sensor module data\n",
			__func__);
		return -ENOMEM;
	}

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
		sizeof(struct cm36655_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			kfree(cm36655);
			return -ENOMEM;
		}
		err = cm36655_parse_dt(&client->dev, pdata);
		if (err)
			goto err_devicetree;
	} else
		pdata = client->dev.platform_data;

	if (!pdata) {
		pr_err("%s: missing pdata!\n", __func__);
		kfree(cm36655);
		return ret;
	}

	prox_regulator_onoff(&client->dev, 1);

	cm36655->pdata = pdata;
	cm36655->i2c_client = client;
	i2c_set_clientdata(client, cm36655);

	mutex_init(&cm36655->power_lock);
	mutex_init(&cm36655->read_lock);

	/* wake lock init for proximity sensor */
	wake_lock_init(&cm36655->prx_wake_lock, WAKE_LOCK_SUSPEND,
			"prx_wake_lock");

#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
	/* setup leden_gpio */
	ret = cm36655_setup_leden_gpio(cm36655);
	if (ret) {
		pr_err("%s: could not setup leden_gpio\n", __func__);
		goto err_setup_leden_gpio;
	}
	cm36655_leden_gpio_onoff(cm36655, 1);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
	/* setup sensoren_gpio */
	ret = cm36655_setup_sensoren_gpio(cm36655);
	if (ret) {
		pr_err("%s: could not setup sensoren_gpio\n", __func__);
		goto err_setup_sensoren_gpio;
	}
	cm36655_sensoren_gpio_onoff(cm36655, 1);
#endif

	/* Check if the device is there or not. */
	ret = cm36655_i2c_write_word(cm36655, REG_CS_CONF1, 0x0001);
	if (ret < 0) {
		pr_err("[SENSOR] %s: cm36655 is not connected.(%d)\n", __func__,
			ret);
		goto err_setup_reg;
	}

	/* setup initial registers */
	ret = cm36655_setup_reg(cm36655);
	if (ret < 0) {
		pr_err("[SENSOR] %s: could not setup regs\n", __func__);
		goto err_setup_reg;
	}

#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)

	cm36655_pin_ctrl(&client->dev);

	cm36655_leden_gpio_onoff(cm36655, 0);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
	cm36655_sensoren_gpio_onoff(cm36655, 0);
#endif

	/* allocate proximity input_device */
	cm36655->proximity_input_dev = input_allocate_device();
	if (!cm36655->proximity_input_dev) {
		pr_err("%s: could not allocate proximity input device\n",
			__func__);
		goto err_input_allocate_device_proximity;
	}

	input_set_drvdata(cm36655->proximity_input_dev, cm36655);
	cm36655->proximity_input_dev->name = "proximity_sensor";
	input_set_capability(cm36655->proximity_input_dev, EV_ABS,
			ABS_DISTANCE);
	input_set_abs_params(cm36655->proximity_input_dev, ABS_DISTANCE, 0, 1,
			0, 0);

	ret = input_register_device(cm36655->proximity_input_dev);
	if (ret < 0) {
		input_free_device(cm36655->proximity_input_dev);
		pr_err("[SENSOR] %s: could not register input device\n",
			__func__);
		goto err_input_register_device_proximity;
	}

	ret = sensors_create_symlink(&cm36655->proximity_input_dev->dev.kobj,
					cm36655->proximity_input_dev->name);
	if (ret < 0) {
		pr_err("[SENSOR] %s: create_symlink error\n", __func__);
		goto err_sensors_create_symlink_prox;
	}

	ret = sysfs_create_group(&cm36655->proximity_input_dev->dev.kobj,
				 &proximity_attribute_group);
	if (ret) {
		pr_err("[SENSOR] %s: could not create sysfs group\n", __func__);
		goto err_sysfs_create_group_proximity;
	}

	/* setup irq */
	ret = cm36655_setup_irq(cm36655);
	if (ret) {
		pr_err("%s: could not setup irq\n", __func__);
		goto err_setup_irq;
	}

	/* For factory test mode, we use timer to get average proximity data. */
	/* prox_timer settings. we poll for light values using a timer. */
	hrtimer_init(&cm36655->prox_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cm36655->prox_poll_delay = ns_to_ktime(2000 * NSEC_PER_MSEC);/*2 sec*/
	cm36655->prox_timer.function = cm36655_prox_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	   to read the i2c (can be slow and blocking). */
	cm36655->prox_wq = create_singlethread_workqueue("cm36655_prox_wq");
	if (!cm36655->prox_wq) {
		ret = -ENOMEM;
		pr_err("[SENSOR] %s: could not create prox workqueue\n",
			__func__);
		goto err_create_prox_workqueue;
	}
	/* this is the thread function we run on the work queue */
	INIT_WORK(&cm36655->work_prox, cm36655_work_func_prox);

	/* allocate lightsensor input_device */
	cm36655->light_input_dev = input_allocate_device();
	if (!cm36655->light_input_dev) {
		pr_err("%s: could not allocate light input device\n", __func__);
		goto err_input_allocate_device_light;
	}

	input_set_drvdata(cm36655->light_input_dev, cm36655);
	cm36655->light_input_dev->name = "light_sensor";
	input_set_capability(cm36655->light_input_dev, EV_REL, REL_HWHEEL);
	input_set_capability(cm36655->light_input_dev, EV_REL, REL_MISC);
	input_set_capability(cm36655->light_input_dev, EV_REL, REL_DIAL);
	input_set_capability(cm36655->light_input_dev, EV_REL, REL_WHEEL);

	ret = input_register_device(cm36655->light_input_dev);
	if (ret < 0) {
		input_free_device(cm36655->light_input_dev);
		pr_err("%s: could not register input device\n", __func__);
		goto err_input_register_device_light;
	}

	ret = sensors_create_symlink(&cm36655->light_input_dev->dev.kobj,
					cm36655->light_input_dev->name);
	if (ret < 0) {
		pr_err("[SENSOR] %s: create_symlink error\n", __func__);
		goto err_sensors_create_symlink_light;
	}

	ret = sysfs_create_group(&cm36655->light_input_dev->dev.kobj,
				 &light_attribute_group);
	if (ret) {
		pr_err("[SENSOR] %s: could not create sysfs group\n", __func__);
		goto err_sysfs_create_group_light;
	}

	/* light_timer settings. we poll for light values using a timer. */
	hrtimer_init(&cm36655->light_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cm36655->light_poll_delay = ns_to_ktime(200 * NSEC_PER_MSEC);
	cm36655->light_timer.function = cm36655_light_timer_func;

	/* the timer just fires off a work queue request.  we need a thread
	   to read the i2c (can be slow and blocking). */
	cm36655->light_wq = create_singlethread_workqueue("cm36655_light_wq");
	if (!cm36655->light_wq) {
		ret = -ENOMEM;
		pr_err("[SENSOR] %s: could not create light workqueue\n",
			__func__);
		goto err_create_light_workqueue;
	}

	/* this is the thread function we run on the work queue */
	INIT_WORK(&cm36655->work_light, cm36655_work_func_light);

	/* set sysfs for proximity sensor */
	ret = sensors_register(cm36655->proximity_dev,
		cm36655, prox_sensor_attrs,
			"proximity_sensor");
	if (ret) {
		pr_err("[SENSOR] %s: cound not register proximity sensor device(%d).\n",
			__func__, ret);
		goto prox_sensor_register_failed;
	}

	/* set sysfs for light sensor */
	ret = sensors_register(cm36655->light_dev,
		cm36655, light_sensor_attrs,
			"light_sensor");
	if (ret) {
		pr_err("[SENSOR] %s: cound not register light sensor device(%d).\n",
			__func__, ret);
		goto light_sensor_register_failed;
	}

	pr_info("[SENSOR] %s is success.\n", __func__);
	goto done;

err_devicetree:
	pr_err("[SENSOR] error in device tree");

/* error, unwind it all */
light_sensor_register_failed:
	sensors_unregister(cm36655->proximity_dev, prox_sensor_attrs);
prox_sensor_register_failed:
	destroy_workqueue(cm36655->light_wq);
err_create_light_workqueue:
	sysfs_remove_group(&cm36655->light_input_dev->dev.kobj,
			   &light_attribute_group);
err_sysfs_create_group_light:
	sensors_remove_symlink(&cm36655->light_input_dev->dev.kobj,
			cm36655->light_input_dev->name);
err_sensors_create_symlink_light:
	input_unregister_device(cm36655->light_input_dev);
err_input_register_device_light:
err_input_allocate_device_light:
	destroy_workqueue(cm36655->prox_wq);
err_create_prox_workqueue:
	free_irq(cm36655->irq, cm36655);
	gpio_free(cm36655->pdata->irq);
err_setup_irq:
	sysfs_remove_group(&cm36655->proximity_input_dev->dev.kobj,
			   &proximity_attribute_group);
err_sysfs_create_group_proximity:
	sensors_remove_symlink(&cm36655->proximity_input_dev->dev.kobj,
			cm36655->proximity_input_dev->name);
err_sensors_create_symlink_prox:
	input_unregister_device(cm36655->proximity_input_dev);
err_input_register_device_proximity:
err_input_allocate_device_proximity:
err_setup_reg:
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
	cm36655_sensoren_gpio_onoff(cm36655, 0);
	gpio_free(cm36655->pdata->sensoren_gpio);
err_setup_sensoren_gpio:
#endif
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
	cm36655_leden_gpio_onoff(cm36655, 0);
	gpio_free(cm36655->pdata->leden_gpio);
err_setup_leden_gpio:
#endif
	wake_lock_destroy(&cm36655->prx_wake_lock);
	mutex_destroy(&cm36655->read_lock);
	mutex_destroy(&cm36655->power_lock);


	kfree(cm36655);
	pr_err("%s is FAILED (%d)\n", __func__, ret);

done:
	return ret;
}

static int cm36655_i2c_remove(struct i2c_client *client)
{
	struct cm36655_data *cm36655 = i2c_get_clientdata(client);

	/* free irq */
	if (cm36655->power_state & PROXIMITY_ENABLED) {
		disable_irq_wake(cm36655->irq);
		disable_irq(cm36655->irq);
	}
	free_irq(cm36655->irq, cm36655);
	gpio_free(cm36655->pdata->irq);

	/* device off */
	if (cm36655->power_state & LIGHT_ENABLED)
		cm36655_light_disable(cm36655);
	if (cm36655->power_state & PROXIMITY_ENABLED) {
		cm36655_i2c_write_word(cm36655, REG_PS_CONF1,
					   0x0001);
	}
	prox_regulator_onoff(&client->dev, 0);

	/* destroy workqueue */
	destroy_workqueue(cm36655->light_wq);
	destroy_workqueue(cm36655->prox_wq);

	/* sysfs destroy */
	sensors_unregister(cm36655->light_dev, light_sensor_attrs);
	sensors_unregister(cm36655->proximity_dev, prox_sensor_attrs);
	sensors_remove_symlink(&cm36655->light_input_dev->dev.kobj,
			cm36655->light_input_dev->name);
	sensors_remove_symlink(&cm36655->proximity_input_dev->dev.kobj,
			cm36655->proximity_input_dev->name);

	/* input device destroy */
	sysfs_remove_group(&cm36655->light_input_dev->dev.kobj,
				&light_attribute_group);
	input_unregister_device(cm36655->light_input_dev);
	sysfs_remove_group(&cm36655->proximity_input_dev->dev.kobj,
				&proximity_attribute_group);
	input_unregister_device(cm36655->proximity_input_dev);
#if defined(CONFIG_SENSORS_CM36655_LEDA_EN_GPIO)
	cm36655_leden_gpio_onoff(cm36655, 0);
	gpio_free(cm36655->pdata->leden_gpio);
#endif
#if defined(CONFIG_SENSORS_CM36655_SENSOR_EN_GPIO)
	cm36655_sensoren_gpio_onoff(cm36655, 0);
	gpio_free(cm36655->pdata->sensoren_gpio);
#endif

	/* lock destroy */
	mutex_destroy(&cm36655->read_lock);
	mutex_destroy(&cm36655->power_lock);
	wake_lock_destroy(&cm36655->prx_wake_lock);

	kfree(cm36655);

	return 0;
}

static int cm36655_suspend(struct device *dev)
{
	/* We disable power only if proximity is disabled.  If proximity
	   is enabled, we leave power on because proximity is allowed
	   to wake up device.  We remove power without changing
	   cm36655->power_state because we use that state in resume.
	 */
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	if (cm36655->power_state & LIGHT_ENABLED)
		cm36655_light_disable(cm36655);
	pr_info("%s is called.\n", __func__);

	return 0;
}

static int cm36655_resume(struct device *dev)
{
	struct cm36655_data *cm36655 = dev_get_drvdata(dev);

	if (cm36655->power_state & LIGHT_ENABLED)
		cm36655_light_enable(cm36655);
	pr_info("%s is called.\n", __func__);

	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id cm36655_match_table[] = {
	{ .compatible = "cm36655",},
	{},
};
#else
#define cm36655_match_table NULL
#endif


static const struct i2c_device_id cm36655_device_id[] = {
	{"cm36655", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cm36655_device_id);

static const struct dev_pm_ops cm36655_pm_ops = {
	.suspend = cm36655_suspend,
	.resume = cm36655_resume
};

static struct i2c_driver cm36655_i2c_driver = {
	.driver = {
		   .name = "cm36655",
		   .owner = THIS_MODULE,
		   .of_match_table = cm36655_match_table,
		   .pm = &cm36655_pm_ops
	},
	.probe = cm36655_i2c_probe,
	.remove = cm36655_i2c_remove,
	.id_table = cm36655_device_id,
};

static int __init cm36655_init(void)
{
	return i2c_add_driver(&cm36655_i2c_driver);
}

static void __exit cm36655_exit(void)
{
	i2c_del_driver(&cm36655_i2c_driver);
}

module_init(cm36655_init);
module_exit(cm36655_exit);

MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("RGB Sensor device driver for cm36655");
MODULE_LICENSE("GPL");

