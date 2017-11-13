/*
 * Copyright (c) 2012 SAMSUNG
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
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <linux/wakelock.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/i2c.h>
#include <linux/fs.h>

/* iio headers */
#include <linux/iio/buffer.h>
#include <linux/iio/iio.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>

#include <linux/sensor/sensors_core.h>
#include "gp2ap030.h"

#define THR_REG_LSB(data, reg) { reg = (u8)(data & 0xff); }
#define THR_REG_MSB(data, reg) { reg = (u8)(data >> 8); }

#define CAL_PATH		"/efs/prox_cal_data"

#define GP2A_VENDOR		"SHARP"
#define GP2A_CHIP_ID		"GP2AP030"
#define PROX_READ_NUM		10
#define GP2A_PROX_MAX		1023
#define GP2A_PROX_MIN		0
#define MIN_DELAY		10
#define MAX_DELAY		200
#define SENSOR_ENABLE		1
#define SENSOR_DISABLE		0
#define LUX_MAX_VALUE		65535
#define RAWDATA_THRESHOLD	16000

#define GP2A_PROX_NAME	"gp2a_prox"
#define GP2A_LIGHT_NAME	"gp2a_light"

#define GP2A_PROX_INFO_SHARED_MASK	(BIT(IIO_CHAN_INFO_SCALE))
#define GP2A_PROX_INFO_SEPARATE_MASK	(BIT(IIO_CHAN_INFO_RAW))

#define GP2A_LIGHT_INFO_SHARED_MASK	(BIT(IIO_CHAN_INFO_SCALE))
#define GP2A_LIGHT_INFO_SEPARATE_MASK	(BIT(IIO_CHAN_INFO_RAW))

#define GP2A_ADC_SKIP	7
#define GP2A_ADC_PASS	13

#define GP2A_PROX_CHANNEL()					\
{								\
	.type = IIO_PROXIMITY,					\
	.modified = 1,						\
	.channel2 = IIO_MOD_PROXIMITY,				\
	.info_mask_separate = GP2A_PROX_INFO_SEPARATE_MASK,	\
	.info_mask_shared_by_type = GP2A_PROX_INFO_SHARED_MASK,	\
	.scan_index = GP2A_SCAN_PROX_CH,			\
	.scan_type = IIO_ST('s', 32, 32, 0)			\
}
#define GP2A_LIGHT_CHANNEL()					\
{								\
	.type = IIO_LIGHT,					\
	.modified = 1,						\
	.channel2 = IIO_MOD_LIGHT,					\
	.info_mask_separate = GP2A_LIGHT_INFO_SEPARATE_MASK,	\
	.info_mask_shared_by_type = GP2A_LIGHT_INFO_SHARED_MASK,\
	.scan_index = GP2A_SCAN_LIGHT_CH,			\
	.scan_type = IIO_ST('s', 32, 32, 0)			\
}

enum {
	GP2A_SCAN_PROX_CH,
	GP2A_SCAN_PROX_TIMESTAMP,
};

enum {
	GP2A_SCAN_LIGHT_CH,
	GP2A_SCAN_LIGHT_TIMESTAMP,
};

static bool shutdown;
static bool iio_light_enable;
static bool iio_prox_enable;
static bool iio_proximity_detection;

struct gp2a_data {
	struct i2c_client *client;
	struct input_dev *light_input_dev;
	struct delayed_work light_work;
	struct mutex data_mutex;
	struct mutex light_mutex;
	struct mutex prox_mutex;

	u8 light_enabled;
	u8 prox_enabled;
	u8 proximity_detection;
	u8 lightsensor_mode;

	int light_delay;
	int lux;

	struct input_dev *prox_input_dev;
	struct wake_lock prx_wake_lock;
	struct work_struct proximity_work;

	int irq;

	int gpio;
	int power_en;

	int offset_value;
	int cal_result;
	uint16_t threshold_high;
	bool offset_cal_high;
	struct device *light_sensor_device;
	struct device *prox_sensor_device;
	struct delayed_work prox_avg_work;

	int prox_delay;
	int avg[3];

	struct regulator *vio_1p8;

	/* iio variables */
	struct iio_trigger *trig;
	int16_t sampling_frequency_prox;
	int16_t sampling_frequency_light;
	atomic_t pseudo_irq_enable_prox;
	atomic_t pseudo_irq_enable_light;
	struct mutex lock;
	spinlock_t spin_lock;
};

/* initial value for sensor register */
#define COL 8
static u8 gp2a_reg[COL][2] = {
	/*  {Regster, Value} */
	{0x01, 0x63},	/*PRST :01(4 cycle at Detection/Non-detection),
			ALSresolution :16bit, range *128
			//0x1F -> 5F by sharp */
/*	{0x02, 0x1A},*/	/*ALC : 0, INTTYPE : 1,
			PS mode resolution : 12bit, range*1 */
	{0x02, 0x5A},	/*ALC : 0, INTTYPE : 1,
			PS mode resolution : 12bit, range*1 */
	{0x03, 0x3C},	/*LED drive current 110mA,
			Detection/Non-detection judgment output */
	{0x08, 0x0A},	/*PS mode LTH(Loff):  (??mm) */
	{0x09, 0x00},	/*PS mode LTH(Loff) : */
	{0x0A, 0x0C},	/*PS mode HTH(Lon) : (??mm) */
	{0x0B, 0x00},	/*PS mode HTH(Lon) : */
	{0x00, 0xC0}	/*alternating mode (PS+ALS), TYPE=1
			(0:externel 1:auto calculated mode)*/
};

irqreturn_t gp2a_iio_pollfunc_store_boottime(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	pf->timestamp = gp2a_iio_get_boottime_ns();
	return IRQ_WAKE_THREAD;
}

static int gp2a_light_data_rdy_trig_poll(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	unsigned long flags;
	spin_lock_irqsave(&st->spin_lock, flags);
	iio_trigger_poll(st->trig, gp2a_iio_get_boottime_ns());
	spin_unlock_irqrestore(&st->spin_lock, flags);
	return 0;
}

static int gp2a_prox_data_rdy_trig_poll(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	unsigned long flags;

	spin_lock_irqsave(&st->spin_lock, flags);
	iio_trigger_poll(st->trig, gp2a_iio_get_boottime_ns());
	spin_unlock_irqrestore(&st->spin_lock, flags);
	return 0;
}

static int gp2a_i2c_read(struct gp2a_data *gp2a,
	u8 reg, unsigned char *rbuf, int len)
{
	int ret = -1;
	struct i2c_msg msg[2];
	struct i2c_client *client = gp2a->client;

	if (unlikely((client == NULL) || (!client->adapter)))
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = I2C_M_WR;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2c_M_RD;
	msg[1].len = len;
	msg[1].buf = rbuf;

	ret = i2c_transfer(client->adapter, &msg[0], 2);

	if (unlikely(ret < 0))
		pr_err("i2c transfer error ret=%d\n", ret);

	return ret;
}

static int gp2a_i2c_write(struct gp2a_data *gp2a,
	u8 reg, u8 *val)
{
	int err = 0;
	struct i2c_msg msg[1];
	unsigned char data[2];
	int retry = 2;
	struct i2c_client *client = gp2a->client;

	if (unlikely((client == NULL) || (!client->adapter)))
		return -ENODEV;

	do {
		data[0] = reg;
		data[1] = *val;

		msg->addr = client->addr;
		msg->flags = I2C_M_WR;
		msg->len = 2;
		msg->buf = data;

		err = i2c_transfer(client->adapter, msg, 1);

		if (err >= 0)
			return 0;
	} while (--retry > 0);
	pr_err("%s i2c transfer error(%d)\n",__func__ , err);
	return err;
}

static void sensor_power_on_vdd(struct gp2a_data *info, int onoff)
{
	int ret;
	if (info->vio_1p8 == NULL) {
		info->vio_1p8 = regulator_get(&info->client->dev, "gp2a-vio");
		if (IS_ERR(info->vio_1p8)) {
			pr_err("%s: regulator_get failed for gp2a-vio\n",
				__func__);
			return;
		}
		ret = regulator_set_voltage(info->vio_1p8, 1800000, 1800000);
		if (ret)
			pr_err("%s: err vsensor_1p8 setting voltage ret=%d\n",
				__func__, ret);
	}

	if (onoff == 1) {
		ret = regulator_enable(info->vio_1p8);
		if (ret)
			pr_err("%s: err enablinig info->vio_1p8\n", __func__);
	} else if (onoff == 0) {
		if (regulator_is_enabled(info->vio_1p8)) {
			ret = regulator_disable(info->vio_1p8);
			if (ret)
				pr_err("%s: err vio_1p8 disable\n", __func__);
		}
	}
	msleep(30);
	return;
}

static int gp2a_lightsensor_onoff(u8 onoff, struct gp2a_data *data)
{
	u8 value;

	pr_info("%s : light_sensor onoff = %d\n", __func__, onoff);

	if (onoff) {
		/*in calling, must turn on proximity sensor */
		if (iio_prox_enable == 0) {
			value = 0x01;
			gp2a_i2c_write(data, COMMAND4, &value);
			value = 0x63;
			gp2a_i2c_write(data, COMMAND2, &value);
			/*OP3 : 1(operating mode)
			  OP2 : 1(coutinuous operating mode)
			  OP1 : 01(ALS mode) TYPE=0(auto)*/
			value = 0xD0;
			gp2a_i2c_write(data, COMMAND1, &value);
			/* other setting have defualt value. */
		} else
			pr_info("%s : light on prox on\n", __func__);
		msleep(20);
	} else {
		/*in calling, must turn on proximity sensor */
		if (iio_prox_enable == 0) {
			value = 0x00;	/*shutdown mode */
			gp2a_i2c_write(data, COMMAND1, &value);
		} else
			pr_info("%s : light off prox on\n", __func__);
	}

	return 0;
}

int gp2a_get_lux(struct gp2a_data *data)
{
	unsigned char get_data[4] = { 0, };
	int d0_raw_data;
	int d1_raw_data;
	int d0_data;
	int d1_data;
	int lx = 0;
	u8 value;
	int light_alpha = 0;
	int light_beta = 0;
	static int lx_prev;
	int ret;

	mutex_lock(&data->data_mutex);
	ret = gp2a_i2c_read(data, DATA0_LSB, get_data, sizeof(get_data));
	mutex_unlock(&data->data_mutex);
	if (ret < 0)
		return lx_prev;
	d0_raw_data = (get_data[1] << 8) | get_data[0]; /* clear */
	d1_raw_data = (get_data[3] << 8) | get_data[2]; /* IR */

	if (100 * d1_raw_data <= 45 * d0_raw_data) {
		light_alpha = 685;
		light_beta = 0;
	} else if (100 * d1_raw_data <= 70 * d0_raw_data) {
		light_alpha = 1748;
		light_beta = 2364;
	} else if (100 * d1_raw_data <= 91 * d0_raw_data) {
		if (d0_raw_data < 200) {
			light_alpha = 410;
			light_beta = 450;
		} else if (d0_raw_data < 800) {
			light_alpha = 437;
			light_beta = 480;
		} else if (d0_raw_data < 2000) {
			light_alpha = 370;
			light_beta = 407;
		} else if (d0_raw_data < 3000) {
			light_alpha = 420;
			light_beta = 462;
		} else {
			light_alpha = 516;
			light_beta = 562;
		}
	} else {
		light_alpha = 0;
		light_beta = 0;
	}

	if (data->lightsensor_mode) {	/* HIGH_MODE */
		d0_data = d0_raw_data * 16;
		d1_data = d1_raw_data * 16;
	} else {		/* LOW_MODE */
		d0_data = d0_raw_data;
		d1_data = d1_raw_data;
	}

	if (d0_data < 2) {
		lx = 0;
	} else if (data->lightsensor_mode == 0
		&& (d0_raw_data >= RAWDATA_THRESHOLD ||
		d1_raw_data >= RAWDATA_THRESHOLD)) {
		lx = LUX_MAX_VALUE;
	} else if (100 * d1_data > 91 * d0_data) {
		if (d0_raw_data >= RAWDATA_THRESHOLD ||
			d1_raw_data >= RAWDATA_THRESHOLD)
			lx_prev = LUX_MAX_VALUE;
		lx = lx_prev;
		return lx;
	} else {
		lx = (int)((light_alpha * d0_data)
			- (light_beta * d1_data)) * 33 / 10000;
	}
	if (lx >= LUX_MAX_VALUE)
		lx = LUX_MAX_VALUE;

	if (data->lightsensor_mode) {
		/* HIGH MODE */
		if (d0_raw_data < 1000) {
			pr_info("%s: change to LOW_MODE detection=%d\n",
				__func__, iio_proximity_detection);
			data->lightsensor_mode = 0;	/* change to LOW MODE */

			value = 0x0C;
			gp2a_i2c_write(data, COMMAND1, &value);

			if (iio_proximity_detection)
				value = 0x23;
			else
				value = 0x63;
			gp2a_i2c_write(data, COMMAND2, &value);

			if (iio_prox_enable)
				value = 0xCC;
			else
				value = 0xDC;
			gp2a_i2c_write(data, COMMAND1, &value);
			lx = lx_prev;
		}
	} else {
		/* LOW MODE */
		if (d0_raw_data > RAWDATA_THRESHOLD ||
			d1_raw_data > RAWDATA_THRESHOLD) {
			pr_info("%s: change to HIGH_MODE detection=%d\n",
				__func__, iio_proximity_detection);

			/* change to HIGH MODE */
			data->lightsensor_mode = 1;

			value = 0x0C;
			gp2a_i2c_write(data, COMMAND1, &value);

			if (iio_proximity_detection)
				value = 0x27;
			else
				value = 0x67;
			gp2a_i2c_write(data, COMMAND2, &value);

			if (iio_prox_enable)
				value = 0xCC;
			else
				value = 0xDC;
			gp2a_i2c_write(data, COMMAND1, &value);
			lx = lx_prev;
		}
	}
	lx_prev = lx;
	return lx;
}

static void gp2a_work_func_light(struct work_struct *work)
{
	struct gp2a_data *data = container_of((struct delayed_work *)work,
		struct gp2a_data, light_work);
	struct iio_dev *indio_dev = iio_priv_to_dev(data);
	static int count;

	data->lux = gp2a_get_lux(data);

	/* detecting 0 after 3.6sec, set the register again. */
	if (data->lux == 0) {
		count++;
		if (count * data->light_delay > 3600) {
			gp2a_lightsensor_onoff(1, data);
			count = 0;
			pr_info("%s: register reset\n", __func__);
		}
	} else
		count = 0;

	gp2a_light_data_rdy_trig_poll(indio_dev);

	if (data->light_enabled)
		schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(data->light_delay));
	return;
}

static ssize_t gp2a_light_delay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->light_delay);
}

static ssize_t gp2a_light_delay_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	int delay;
	int err = 0;

	if (shutdown == true) {
		pr_err("%s already shutdown.", __func__);
		goto done;
	}

	err = kstrtoint(buf, 10, &delay);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	if (delay < 0)
		goto done;

	delay = delay / 1000000;	/* ns to msec */
	if (delay > MAX_DELAY)
		delay = MAX_DELAY;
	if (delay < MIN_DELAY)
		delay = MIN_DELAY;

	pr_info("%s new_delay = %d, old_delay = %d", __func__, delay,
		data->light_delay);

	data->light_delay = delay;

	mutex_lock(&data->light_mutex);

	if (data->light_enabled) {
		cancel_delayed_work_sync(&data->light_work);
		schedule_delayed_work(&data->light_work,
			msecs_to_jiffies(delay));
	}

	mutex_unlock(&data->light_mutex);
done:
	return count;
}


static ssize_t gp2a_light_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->light_enabled);
}

static ssize_t gp2a_light_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	int value;
	int err = 0;

	if (shutdown == true) {
		pr_err("%s already shutdown.", __func__);
		goto done;
	}

	err = kstrtoint(buf, 10, &value);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	pr_info("%s %d value = %d\n", __func__, __LINE__, value);

	if (value != SENSOR_DISABLE && value != SENSOR_ENABLE)
		goto done;

	if (value) {
		if (data->light_enabled == SENSOR_DISABLE) {
			gp2a_lightsensor_onoff(1, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(100));
			data->light_enabled = SENSOR_ENABLE;
		} else {
			pr_err("%s already enabled\n", __func__);
		}
	} else {
		if (data->light_enabled == SENSOR_ENABLE) {
			gp2a_lightsensor_onoff(0, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(data->light_delay));
			data->light_enabled = SENSOR_ENABLE;
		} else {
			pr_err("%s already disabled\n", __func__);
		}
	}

done:
	return count;
}
/*
static void gp2a_light_enable_set(struct gp2a_data *st, int onoff)
{
	struct gp2a_data *data = st;

	int err = 0;
	int value = onoff;
	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}

	if (value != SENSOR_DISABLE && value != SENSOR_ENABLE)
		goto done;

	if (value) {
		if (data->light_enabled == SENSOR_DISABLE) {
			lightsensor_onoff(1, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(100));
			data->light_enabled = SENSOR_ENABLE;
		} else {
			pr_err("%s already enabled\n", __func__);
		}
	} else {
		if (data->light_enabled == SENSOR_ENABLE) {
			lightsensor_onoff(0, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(data->light_delay));
			data->light_enabled = SENSOR_DISABLE;
		} else {
			pr_err("%s already disabled\n", __func__);
		}
	}
done:
	return;
}
*/
static ssize_t gp2a_light_flush_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	int value;
	int err = 0;

	err = kstrtoint(buf, 10, &value);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	pr_info("%s %d value = %d\n", __func__, __LINE__, value);

	if (value) {
		mutex_lock(&data->light_mutex);
		input_report_rel(data->light_input_dev, REL_MAX, 1);
		input_sync(data->light_input_dev);
		mutex_unlock(&data->light_mutex);
	}

done:
	return count;
}

static struct device_attribute dev_attr_light_enable =
	__ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP, gp2a_light_enable_show,
	gp2a_light_enable_store);
static struct device_attribute dev_attr_light_delay =
	__ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP, gp2a_light_delay_show,
	gp2a_light_delay_store);
static struct device_attribute dev_attr_light_flush =
	__ATTR(flush, S_IWUSR|S_IWGRP, NULL, gp2a_light_flush_store);

static struct attribute *gp2a_light_attributes[] = {
	&dev_attr_light_enable.attr,
	&dev_attr_light_delay.attr,
	&dev_attr_light_flush.attr,
	NULL
};

static struct attribute_group gp2a_light_attribute_group = {
	.attrs = gp2a_light_attributes,
};



static ssize_t gp2a_light_raw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	unsigned char get_data[4] = { 0, };
	int d0_raw_data = 0;
	int d1_raw_data = 0;
	int ret = 0;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	mutex_lock(&data->data_mutex);
	ret = gp2a_i2c_read(data, DATA0_LSB, get_data, sizeof(get_data));
	mutex_unlock(&data->data_mutex);

	if (ret < 0)
		pr_err("%s i2c err: %d\n", __func__, ret);

	d0_raw_data = (get_data[1] << 8) | get_data[0];	/* clear */
	d1_raw_data = (get_data[3] << 8) | get_data[2];	/* IR */
done:
	return snprintf(buf, PAGE_SIZE, "%d,%d\n", d0_raw_data, d1_raw_data);
}

static ssize_t gp2a_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", GP2A_VENDOR);
}

static ssize_t gp2a_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", GP2A_CHIP_ID);
}

static struct device_attribute dev_attr_light_raw_data =
	__ATTR(raw_data, S_IRUSR | S_IRGRP, gp2a_light_raw_data_show, NULL);
static struct device_attribute dev_attr_light_lux =
	__ATTR(lux, S_IRUSR | S_IRGRP, gp2a_light_raw_data_show, NULL);
static struct device_attribute dev_attr_light_vendor =
	__ATTR(vendor, S_IRUSR | S_IRGRP, gp2a_vendor_show, NULL);
static struct device_attribute dev_attr_light_name =
	__ATTR(name, S_IRUSR | S_IRGRP, gp2a_name_show, NULL);

static struct device_attribute *light_sensor_attrs[] = {
	&dev_attr_light_raw_data,
	&dev_attr_light_lux,
	&dev_attr_light_vendor,
	&dev_attr_light_name,
	NULL,
};

static int gp2a_prox_open_calibration(struct gp2a_data  *data)
{
	struct file *cal_filp = NULL;
	int err = 0;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CAL_PATH,
		O_RDONLY, S_IRUGO | S_IWUSR | S_IWGRP);
	if (IS_ERR(cal_filp)) {
		pr_err("%s Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		goto done;
	}

	err = cal_filp->f_op->read(cal_filp, (char *)&data->offset_value,
		sizeof(int), &cal_filp->f_pos);
	if (err != sizeof(int)) {
		pr_err("%s Can't read the cal data from file\n", __func__);
		err = -EIO;
	}

	pr_info("%s (%d)\n", __func__, data->offset_value);

	filp_close(cal_filp, current->files);
done:
	set_fs(old_fs);

	return err;
}

static int gp2a_prox_onoff(u8 onoff, struct gp2a_data *data)
{
	u8 value;

	pr_info("%s : proximity turn on/off = %d\n", __func__, onoff);

	/* already on light sensor, so must simultaneously
		turn on light sensor and proximity sensor */
	if (onoff) {
		int i;
//		gpio_set_value(data->vled_gpio, 1);

		for (i = 0; i < COL; i++) {
			int err = gp2a_i2c_write(data, gp2a_reg[i][0],
				&gp2a_reg[i][1]);
			if (err < 0)
				pr_err("%s : turnning on error i = %d, err=%d\n",
					__func__, i, err);
			data->lightsensor_mode = 0;
		}
	} else {
		if (iio_light_enable) {
			if (data->lightsensor_mode)
				value = 0x67;
				/*resolution :16bit, range: *8(HIGH) */
			else
				value = 0x63;
				/* resolution :16bit, range: *128(LOW) */
			gp2a_i2c_write(data, COMMAND2, &value);
			/* OP3 : 1(operating mode)
			   OP2 : 1(coutinuous operating mode)
			   OP1 : 01(ALS mode) */
			value = 0xD0;
			gp2a_i2c_write(data, COMMAND1, &value);
		} else {
			value = 0x00;	/*shutdown mode */
			gp2a_i2c_write(data, (u8) (COMMAND1), &value);
		}
//		gpio_set_value(data->vled_gpio, 0);
	}

	return 0;
}

static ssize_t gp2a_prox_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->light_enabled);
}

static ssize_t gp2a_prox_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	int value;
	int err = 0;

	err = kstrtoint(buf, 10, &value);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	pr_info("%s %d value = %d\n", __func__, __LINE__, value);

	if (value != SENSOR_DISABLE && value != SENSOR_ENABLE)
		goto done;

	if (value) {
		if (data->prox_enabled == SENSOR_DISABLE) {
			uint16_t thrd = 0;
			u8 reg;

			gp2a_prox_onoff(1, data);

			err = gp2a_prox_open_calibration(data);
			if (err < 0 && err != -ENOENT)
				pr_err("%s gp2a_prox_open_offset() failed\n",
					__func__);
			else {
				thrd = gp2a_reg[3][1]+(data->offset_value);
				THR_REG_LSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[3][0], &reg);
				THR_REG_MSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[4][0], &reg);

				thrd = gp2a_reg[5][1]+(data->offset_value);
				THR_REG_LSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[5][0], &reg);
				THR_REG_MSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[6][0], &reg);
			}

			enable_irq_wake(data->irq);
			enable_irq(data->irq);

			input_report_abs(data->prox_input_dev, ABS_DISTANCE, 1);
			input_sync(data->prox_input_dev);

			data->prox_enabled = SENSOR_ENABLE;
		} else {
			pr_err("%s already enabled\n", __func__);
		}
	} else {

		if (data->prox_enabled == SENSOR_ENABLE) {
			disable_irq(data->irq);
			disable_irq_wake(data->irq);
			gp2a_prox_onoff(0, data);
			data->prox_enabled = SENSOR_DISABLE;
		} else {
			pr_err("%s already disabled\n", __func__);
		}
	}

done:
	return count;
}

static void gp2a_iio_prox_enable(struct iio_dev *indio_dev, int enable)
{
	struct gp2a_data *data= iio_priv(indio_dev);

	int *value = 0, len;
	int err = 0;

	pr_info("%s, called new enable = %d enabled = %d\n",
		__func__, enable, data->prox_enabled);

	if (enable != SENSOR_DISABLE && enable != SENSOR_ENABLE) {
		pr_err("%s, enable - invalid data\n", __func__);
		goto done;
	}

	if (enable) {
		if (data->prox_enabled == SENSOR_DISABLE) {
			uint16_t thrd = 0;
			u8 reg;

			gp2a_prox_onoff(1, data);

			err = gp2a_prox_open_calibration(data);
			if (err < 0 && err != -ENOENT)
				pr_err("%s gp2a_prox_open_offset() failed\n",
					__func__);
			else {
				thrd = gp2a_reg[3][1]+(data->offset_value);
				THR_REG_LSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[3][0], &reg);
				THR_REG_MSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[4][0], &reg);

				thrd = gp2a_reg[5][1]+(data->offset_value);
				THR_REG_LSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[5][0], &reg);
				THR_REG_MSB(thrd, reg);
				gp2a_i2c_write(data, gp2a_reg[6][0], &reg);
			}

			enable_irq_wake(data->irq);
			enable_irq(data->irq);

			pr_info("%s, irq enable\n", __func__);

			/* data update */
			value = (int32_t *) kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
			if (value == NULL)
				goto done;
			if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
				/* TODO : data update */
				pr_err("%s, pre update prox = %d\n", __func__, data->proximity_detection);
				*value = 1;
			}
			len = 4;
			/* Guaranteed to be aligned with 8 byte boundary */
			if (indio_dev->scan_timestamp)
				*(s64 *)((u8 *)value + ALIGN(len, sizeof(s64))) = 0;
			err = iio_push_to_buffers(indio_dev, (u8 *)value);
			if (err < 0)
				pr_err("%s, iio_push buffer failed = %d\n",
					__func__, err);
			kfree(value);

			data->prox_enabled = SENSOR_ENABLE;
			iio_prox_enable = SENSOR_ENABLE;
		} else {
			pr_err("%s already enabled\n", __func__);
		}
	} else {

		if (data->prox_enabled == SENSOR_ENABLE) {
			disable_irq(data->irq);
			disable_irq_wake(data->irq);
			gp2a_prox_onoff(0, data);
			data->prox_enabled = SENSOR_DISABLE;
			iio_prox_enable = SENSOR_DISABLE;
		} else {
			pr_err("%s already disabled\n", __func__);
		}
	}

done:
	return;
}

static int gp2a_iio_light_enable(struct gp2a_data *data, int enable)
{
	pr_info("%s called enable = %d\n", __func__, enable);

	if (shutdown == true) {
		pr_err("%s already shutdown.", __func__);
		goto done;
	}

	if (enable != SENSOR_DISABLE && enable != SENSOR_ENABLE)
		goto done;

	if (enable) {
		if (data->light_enabled == SENSOR_DISABLE) {
			gp2a_lightsensor_onoff(1, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(100));
			data->light_enabled = SENSOR_ENABLE;
			iio_light_enable = SENSOR_ENABLE;
		} else {
			pr_err("%s already enabled\n", __func__);
		}
	} else {
		if (data->light_enabled == SENSOR_ENABLE) {
			gp2a_lightsensor_onoff(0, data);
			schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(data->light_delay));
			data->light_enabled = SENSOR_DISABLE;
			iio_light_enable = SENSOR_DISABLE;
		} else {
			pr_err("%s already disabled\n", __func__);
		}
	}
	pr_err("%s done.", __func__);
	return 0;
done:
	return -1;

}

static ssize_t gp2a_prox_flush_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	int value;
	int err = 0;

	err = kstrtoint(buf, 10, &value);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	pr_info("%s %d value = %d\n", __func__, __LINE__, value);

	if (err) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	pr_info("%s %d value = %d\n", __func__, __LINE__, value);

	if (value) {
		mutex_lock(&data->data_mutex);
		input_report_abs(data->prox_input_dev, ABS_MAX, 1);
		input_sync(data->prox_input_dev);
		mutex_unlock(&data->data_mutex);
	}

done:
	return count;
}

static struct device_attribute dev_attr_prox_enable = __ATTR(enable,
	S_IRUGO|S_IWUSR|S_IWGRP, gp2a_prox_enable_show, gp2a_prox_enable_store);

static struct device_attribute dev_attr_prox_flush =
	__ATTR(flush, S_IWUSR|S_IWGRP, NULL, gp2a_prox_flush_store);

static struct attribute *gp2a_prox_attributes[] = {
	&dev_attr_prox_enable.attr,
	&dev_attr_prox_flush.attr,
	NULL
};

static struct attribute_group gp2a_prox_attribute_group = {
	.attrs = gp2a_prox_attributes,
};

static int gp2a_prox_adc_read(struct gp2a_data *data)
{
	int sum[OFFSET_ARRAY_LENGTH];
	int i = OFFSET_ARRAY_LENGTH-1;
	int avg = 0;
	int min = 0;
	int max = 0;
	int total = 0;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	mutex_lock(&data->data_mutex);
	do {
		unsigned char get_D2_data[2] = {0,};
		int D2_data;
		msleep(50);
		gp2a_i2c_read(data, DATA2_LSB, get_D2_data,
			sizeof(get_D2_data));
		D2_data = (get_D2_data[1] << 8) | get_D2_data[0];
		sum[i] = D2_data;
		if (i == OFFSET_ARRAY_LENGTH - 1) {
			min = sum[i];
			max = sum[i];
		} else {
			if (sum[i] < min)
				min = sum[i];
			else if (sum[i] > max)
				max = sum[i];
		}
		total += sum[i];
	} while (i--);
	mutex_unlock(&data->data_mutex);

	total -= (min + max);
	avg = (int)(total / (OFFSET_ARRAY_LENGTH - 2));
	pr_info("%s offset = %d\n", __func__, avg);
done:
	return avg;
}

static int gp2a_prox_do_calibrate(struct gp2a_data  *data,
	bool do_calib, bool thresh_set)
{
	struct file *cal_filp;
	int err;
	int xtalk_avg = 0;
	int offset_change = 0;
	uint16_t thrd = 0;
	u8 reg;
	mm_segment_t old_fs;

	pr_info("%s calibration is called do_cal = %d thresh_set = %d\n",
		__func__, do_calib, thresh_set);

	if (do_calib) {
		if (thresh_set) {
			/* for gp2a_prox_thresh_store */
			data->offset_value =
				data->threshold_high -
				(gp2a_reg[6][1] << 8 | gp2a_reg[5][1]);
		} else {
			/* tap offset button */
			/* get offset value */
			xtalk_avg = gp2a_prox_adc_read(data);
			offset_change =
				(gp2a_reg[6][1] << 8 | gp2a_reg[5][1])
				- DEFAULT_HI_THR;

			pr_info("%s xtalk_avg = %d \n",__func__, xtalk_avg);

			if (xtalk_avg < GP2A_ADC_SKIP) {
				/* do not need calibration */
				pr_info("%s no need to calibrate"
					"adc = %d offsetchange = %d\n",
					__func__, xtalk_avg, offset_change);
				data->cal_result = 2;
				err = 0;
				goto no_cal;
			} else if (xtalk_avg >= GP2A_ADC_SKIP && 
					xtalk_avg < GP2A_ADC_PASS) {
				pr_info("%s calibration available"
					"adc = %d offsetchange = %d\n",
					__func__, xtalk_avg, offset_change);
				data->cal_result = 1;
			} else if (xtalk_avg >= GP2A_ADC_PASS) {
				pr_info("%s calibration failed"
					"adc = %d offsetchange = %d\n",
					__func__, xtalk_avg, offset_change);
				data->cal_result = 0;
				err = 0;
				goto no_cal;
			}
			data->offset_value = xtalk_avg - offset_change;
		}
		/* update threshold */
		thrd = (gp2a_reg[4][1] << 8 | gp2a_reg[3][1])
			+ (data->offset_value);
		THR_REG_LSB(thrd, reg);
		gp2a_i2c_write(data, gp2a_reg[3][0], &reg);
		THR_REG_MSB(thrd, reg);
		gp2a_i2c_write(data, gp2a_reg[4][0], &reg);

		thrd = (gp2a_reg[4][1] << 8 | gp2a_reg[5][1])
			+(data->offset_value);
		THR_REG_LSB(thrd, reg);
		gp2a_i2c_write(data, gp2a_reg[5][0], &reg);
		THR_REG_MSB(thrd, reg);
		gp2a_i2c_write(data, gp2a_reg[6][0], &reg);

		/* calibration result */
		if (!thresh_set)
			data->cal_result = 1;
	} else {
		/* tap reset button */
		data->offset_value = 0;
		/* update threshold */
		gp2a_i2c_write(data, gp2a_reg[3][0], &gp2a_reg[3][1]);
		gp2a_i2c_write(data, gp2a_reg[4][0], &gp2a_reg[4][1]);
		gp2a_i2c_write(data, gp2a_reg[5][0], &gp2a_reg[5][1]);
		gp2a_i2c_write(data, gp2a_reg[6][0], &gp2a_reg[6][1]);
		/* calibration result */
		data->cal_result = 2;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CAL_PATH, O_CREAT | O_TRUNC | O_WRONLY,
		S_IRUGO | S_IWUSR | S_IWGRP);
	if (IS_ERR(cal_filp)) {
		pr_err("%s Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		goto done;
	}

	err = cal_filp->f_op->write(cal_filp, (char *)&data->offset_value,
		sizeof(int), &cal_filp->f_pos);
	if (err != sizeof(int)) {
		pr_err("%s Can't write the cal data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cal_filp, current->files);
done:
	set_fs(old_fs);
no_cal:
	return err;
}

static ssize_t gp2a_prox_cal_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	int thresh_hi = 0;
	int thresh_low = 0;
	unsigned char get_D2_data[4];

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	msleep(20);
	gp2a_i2c_read(data, PS_LT_LSB, get_D2_data, sizeof(get_D2_data));
	thresh_hi = (get_D2_data[3] << 8) | get_D2_data[2];
	thresh_low = (get_D2_data[1] << 8) | get_D2_data[0];
	data->threshold_high = thresh_hi;
done:
	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n", data->offset_value,
		thresh_hi, thresh_low);
}

static ssize_t gp2a_prox_cal_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	bool do_calib;
	int err = 0;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	if (sysfs_streq(buf, "1")) { /* calibrate cancelation value */
		do_calib = true;
	} else if (sysfs_streq(buf, "0")) { /* reset cancelation value */
		do_calib = false;
	} else {
		pr_err("%s invalid value %d\n", __func__, *buf);
		err = -EINVAL;
		goto done;
	}
	err = gp2a_prox_do_calibrate(data, do_calib, false);
	if (err < 0) {
		pr_err("%s  gp2a_prox_store_offset() failed\n", __func__);
		goto done;
	} else
		err = size;
done:
	return err;
}

static void gp2a_work_avg_prox(struct work_struct *work)
{
	struct gp2a_data *data = container_of((struct delayed_work *)work,
		struct gp2a_data, prox_avg_work);

	int min = 0, max = 0, avg = 0;
	int i = 0;
	unsigned char raw_data[2] = { 0, };

	for (i = 0; i < PROX_READ_NUM; i++) {
		int gp2a_prox_value;
		mutex_lock(&data->data_mutex);
		gp2a_i2c_read(data, 0x10, raw_data, sizeof(raw_data));
		mutex_unlock(&data->data_mutex);
		gp2a_prox_value = (raw_data[1] << 8) | raw_data[0];

		if (gp2a_prox_value > GP2A_PROX_MAX)
			gp2a_prox_value = GP2A_PROX_MAX;
		if (gp2a_prox_value > GP2A_PROX_MIN) {
			avg += gp2a_prox_value;
			if (!i)
				min = gp2a_prox_value;
			else if (gp2a_prox_value < min)
				min = gp2a_prox_value;
			if (gp2a_prox_value > max)
				max = gp2a_prox_value;
		} else {
			gp2a_prox_value = GP2A_PROX_MIN;
		}
		msleep(40);
	}
	avg /= i;
	data->avg[0] = min;
	data->avg[1] = avg;
	data->avg[2] = max;

	if (iio_prox_enable)
		schedule_delayed_work(&data->prox_avg_work,
				msecs_to_jiffies(data->prox_delay));
}

static ssize_t gp2a_prox_avg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		return snprintf(buf, PAGE_SIZE, "0,0,0\n");
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n", data->avg[0],
		data->avg[1], data->avg[2]);
}

static ssize_t gp2a_prox_avg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	int enable = 0;
	int err = 0;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	err = kstrtoint(buf, 10, &enable);
	if (err < 0) {
		pr_err("%s, kstrtoint failed.", __func__);
	} else {
		pr_info("%s, %d\n", __func__, enable);
		if (enable)
			schedule_delayed_work(&data->prox_avg_work,
				msecs_to_jiffies(data->prox_delay));
		else
			cancel_delayed_work_sync(&data->prox_avg_work);
	}
done:
	return size;
}

static ssize_t gp2a_prox_raw_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	int d2_data = 0;
	unsigned char raw_data[2] = { 0, };

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	mutex_lock(&data->data_mutex);
	gp2a_i2c_read(data, 0x10, raw_data, sizeof(raw_data));
	mutex_unlock(&data->data_mutex);
	d2_data = (raw_data[1] << 8) | raw_data[0];
done:
	return snprintf(buf, PAGE_SIZE, "%d\n", d2_data);
}

static int gp2a_prox_manual_offset(struct gp2a_data  *data, u8 change_on)
{
	struct file *cal_filp;
	int err;
	int16_t thrd;
	u8 reg;
	mm_segment_t old_fs;

	data->offset_value = change_on;
	/* update threshold */
	thrd = gp2a_reg[3][1]+(data->offset_value);
	THR_REG_LSB(thrd, reg);
	gp2a_i2c_write(data, gp2a_reg[3][0], &reg);
	THR_REG_MSB(thrd, reg);
	gp2a_i2c_write(data, gp2a_reg[4][0], &reg);

	thrd = gp2a_reg[5][1]+(data->offset_value);
	THR_REG_LSB(thrd, reg);
	gp2a_i2c_write(data, gp2a_reg[5][0], &reg);
	THR_REG_MSB(thrd, reg);
	gp2a_i2c_write(data, gp2a_reg[6][0], &reg);

	/* calibration result */
	data->cal_result = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CAL_PATH, O_CREAT | O_TRUNC | O_WRONLY,
		S_IRUGO | S_IWUSR | S_IWGRP);
	if (IS_ERR(cal_filp)) {
		pr_err("%s Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		goto done;
	}

	err = cal_filp->f_op->write(cal_filp,
		(char *)&data->offset_value, sizeof(int),
			&cal_filp->f_pos);
	if (err != sizeof(int)) {
		pr_err("%s Can't write the cal data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cal_filp, current->files);
done:
	set_fs(old_fs);
	return err;
}

static ssize_t gp2a_prox_cal2_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	u8 change_on;
	int err;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	if (sysfs_streq(buf, "1")) /* change hi threshold by -2 */
		change_on = -2;
	else if (sysfs_streq(buf, "2")) /*change hi threshold by +4 */
		change_on = 4;
	else if (sysfs_streq(buf, "3")) /*change hi threshold by +8 */
		change_on = 8;
	else {
		pr_err("%s invalid value %d\n", __func__, *buf);
		goto done;
	}
	err = gp2a_prox_manual_offset(data, change_on);
	if (err < 0) {
		pr_err("%s gp2a_prox_store_offset() failed\n", __func__);
		goto done;
	}
done:
	return size;
}

static ssize_t gp2a_prox_thresh_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	int thresh_hi = 0, thresh_low = 0;
	unsigned char get_D2_data[4];

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	msleep(20);
	gp2a_i2c_read(data, PS_LT_LSB, get_D2_data,
		sizeof(get_D2_data));
	thresh_hi = (get_D2_data[3] << 8) | get_D2_data[2];
	thresh_low = (get_D2_data[1] << 8) | get_D2_data[0];
	pr_info("%s THRESHOLD = %d\n", __func__, thresh_hi);
done:
	return snprintf(buf, PAGE_SIZE, "%d,%d\n", thresh_hi, thresh_low);
}

static ssize_t gp2a_prox_thresh_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	long thresh_value = 0;
	int err = 0;

	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		goto done;
	}

	err = kstrtol(buf, 10, &thresh_value);
	if (unlikely(err < 0)) {
		pr_err("%s kstrtoint failed.", __func__);
		goto done;
	}
	data->threshold_high = (uint16_t)thresh_value;
	err = gp2a_prox_do_calibrate(data, true, true);
	if (err < 0) {
		pr_err("%s thresh_store failed\n", __func__);
		goto done;
	}
	msleep(20);
done:
	return size;
}


static ssize_t prox_offset_pass_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gp2a_data *data = dev_get_drvdata(dev);
	if (shutdown == true) {
		pr_err("%s shutdown true.", __func__);
		return snprintf(buf, PAGE_SIZE, "0\n");
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", data->cal_result);
}

static struct device_attribute dev_attr_gp2a_prox_sensor_prox_cal2 =
	__ATTR(prox_cal2, S_IWUGO | S_IWUSR | S_IWGRP,
				NULL, gp2a_prox_cal2_store);
static struct device_attribute dev_attr_gp2a_prox_thresh =
	__ATTR(prox_thresh, S_IRUGO | S_IWUSR | S_IWGRP,
				gp2a_prox_thresh_show, gp2a_prox_thresh_store);
static struct device_attribute dev_attr_gp2a_prox_offset_pass =
	__ATTR(prox_offset_pass, S_IRUGO | S_IRUSR | S_IRGRP,
				prox_offset_pass_show, NULL);
static struct device_attribute dev_attr_prox_cal =
	__ATTR(prox_cal, S_IRUGO | S_IWUSR | S_IWGRP,
				gp2a_prox_cal_show, gp2a_prox_cal_store);
static struct device_attribute dev_attr_prox_prox_avg =
	__ATTR(prox_avg, S_IRUGO | S_IWUSR | S_IWGRP,
	gp2a_prox_avg_show, gp2a_prox_avg_store);
static struct device_attribute dev_attr_prox_raw_data =
	__ATTR(raw_data, S_IRUSR | S_IRGRP,
	gp2a_prox_raw_data_show, NULL);
static struct device_attribute dev_attr_prox_state =
	__ATTR(state, S_IRUSR | S_IRGRP,
	gp2a_prox_raw_data_show, NULL);
static struct device_attribute dev_attr_prox_vendor =
	__ATTR(vendor, S_IRUSR | S_IRGRP,
	gp2a_vendor_show, NULL);
static struct device_attribute dev_attr_prox_name =
	__ATTR(name, S_IRUSR | S_IRGRP,
	gp2a_name_show, NULL);

static struct device_attribute *prox_sensor_attrs[] = {
	&dev_attr_prox_state,
	&dev_attr_prox_prox_avg,
	&dev_attr_prox_raw_data,
	&dev_attr_prox_vendor,
	&dev_attr_prox_name,
	&dev_attr_prox_cal,
	&dev_attr_gp2a_prox_thresh,
	&dev_attr_gp2a_prox_offset_pass,
	&dev_attr_gp2a_prox_sensor_prox_cal2,
	NULL,
};

static const struct iio_chan_spec gp2a_prox_channels[] = {
	GP2A_PROX_CHANNEL(),
	IIO_CHAN_SOFT_TIMESTAMP(GP2A_SCAN_PROX_TIMESTAMP)
};

static const struct iio_chan_spec gp2a_light_channels[] = {
	GP2A_LIGHT_CHANNEL(),
	IIO_CHAN_SOFT_TIMESTAMP(GP2A_SCAN_LIGHT_TIMESTAMP)
};

static int gp2a_read_raw_prox(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan,
		int *val,
		int *val2,
		long mask) {
	struct gp2a_data  *st = iio_priv(indio_dev);
	int ret = -EINVAL;

	if (chan->type != IIO_PROXIMITY)
		return -EINVAL;

	pr_info(" %s read_prox.\n", __func__);
	mutex_lock(&st->lock);

	switch (mask) {
	case 0:
//		*val = st->compass_data[chan->channel2 - IIO_MOD_X];
		ret = IIO_VAL_INT;
		break;
	case IIO_CHAN_INFO_SCALE:
		/* Gain : counts / uT = 1000 [nT] */
		/* Scaling factor : 1000000 / Gain = 1000 */
		*val = 0;
		*val2 = 1000;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static int gp2a_read_raw_light(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan,
		int *val,
		int *val2,
		long mask) {
	struct gp2a_data  *st = iio_priv(indio_dev);
	int ret = -EINVAL;

	if (chan->type != IIO_LIGHT)
		return -EINVAL;

	mutex_lock(&st->lock);

	switch (mask) {
	case 0:
		*val = st->lux ;
		ret = IIO_VAL_INT;
		break;
	case IIO_CHAN_INFO_SCALE:
		/* Gain : counts / uT = 1000 [nT] */
		/* Scaling factor : 1000000 / Gain = 1000 */
		*val = 0;
		*val2 = 1000;
		ret = IIO_VAL_INT_PLUS_MICRO;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static ssize_t gp2a_light_sampling_frequency_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct gp2a_data *st = iio_priv(indio_dev);
	return sprintf(buf, "%d\n", (int)st->sampling_frequency_light);
}

static ssize_t gp2a_light_sampling_frequency_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct gp2a_data *st = iio_priv(indio_dev);
	int ret, data;
	int delay;
	ret = kstrtoint(buf, 10, &data);
	if (ret)
		return ret;
	if (data <= 0)
		return -EINVAL;
	mutex_lock(&st->lock);
	st->sampling_frequency_light = data;
	delay = MSEC_PER_SEC / st->sampling_frequency_light;
	// has to be checked about delay
	st->light_delay = delay;
	//st->mag.set_delay(delay);
	mutex_unlock(&st->lock);
	return count;
}

/* iio light sysfs - sampling frequency */
static IIO_DEVICE_ATTR(sampling_frequency, S_IRUSR|S_IWUSR,
		gp2a_light_sampling_frequency_show,
		gp2a_light_sampling_frequency_store, 0);

static struct attribute *gp2a_iio_light_attributes[] = {
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	NULL
};

static const struct attribute_group gp2a_iio_light_attribute_group = {
	.attrs = gp2a_iio_light_attributes,
};

static const struct iio_info gp2a_prox_info= {

	.read_raw = &gp2a_read_raw_prox,
//	.attrs = &gp2a_iio_prox_attribute_group,
	.driver_module = THIS_MODULE,

};

/* iio prox sysfs - sampling frequency */

static const struct iio_info gp2a_light_info= {
	.read_raw = &gp2a_read_raw_light,
	.attrs = &gp2a_iio_light_attribute_group,
	.driver_module = THIS_MODULE,
};

static irqreturn_t gp2a_light_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct gp2a_data *st = iio_priv(indio_dev);
	int len = 0;
	int ret = 0;
	int32_t *data;

	data = (int32_t *) kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
	if (data == NULL)
		goto done;
	if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength))
		*data = st->lux;

	len = 4;
	/* Guaranteed to be aligned with 8 byte boundary */
	if (indio_dev->scan_timestamp)
		*(s64 *)((u8 *)data + ALIGN(len, sizeof(s64))) = pf->timestamp;
	ret = iio_push_to_buffers(indio_dev, (u8 *)data);
	if (ret < 0)
		pr_err("%s, iio_push buffer failed = %d\n",
			__func__, ret);
	kfree(data);
done:
	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
}

static irqreturn_t gp2a_prox_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct gp2a_data *st = iio_priv(indio_dev);
	int len = 0;
	int ret = 0;
	int32_t *data;

	data = (int32_t *) kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
	pr_err("%s, data = %x\n", __func__, (int)data);
	if (data == NULL)
		goto done;

	pr_err("%s, data = %d\n", __func__, bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength));
	if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
		/* TODO : data update */
		pr_err("%s, prox = %d\n", __func__, st->proximity_detection);
		if (!st->proximity_detection) {
			*data = 0;
		}
		else {
			*data = 1;//st->proximity_detection;
		}
	}
	len = 4;
	/* Guaranteed to be aligned with 8 byte boundary */
	pr_err("%s, data = %d\n", __func__, indio_dev->scan_timestamp);
	if (indio_dev->scan_timestamp)
		*(s64 *)((u8 *)data + ALIGN(len, sizeof(s64))) = pf->timestamp;
	ret = iio_push_to_buffers(indio_dev, (u8 *)data);
	if (ret < 0)
		pr_err("%s, iio_push buffer failed = %d\n",
			__func__, ret);
	kfree(data);
done:
	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
}

#if 0
static void gp2a_work_func_prox(struct work_struct *work)
{
	struct gp2a_data *data = container_of((struct work_struct *)work,
		struct gp2a_data, proximity_work);

	unsigned char value;
	int ret;

	ret = gp2a_i2c_read(data, COMMAND1, &value, sizeof(value));
	if (ret < 0) {
		pr_info("%s, read data error\n", __func__);
	} else {
		pr_info("%s, read data %d, %d\n", __func__,
			value & 0x08, !(value & 0x08));
		data->proximity_detection = !(value & 0x08);
	}

	if (!(value & 0x08)) {
		if (data->lightsensor_mode == 0)
			value = 0x63;
		else
			value = 0x67;
		gp2a_i2c_write(data, COMMAND2, &value);
	} else {
		if (data->lightsensor_mode == 0)
			value = 0x23;
		else
			value = 0x27;
		gp2a_i2c_write(data, COMMAND2, &value);
	}

	value = 0xCC;
	gp2a_i2c_write(data, COMMAND1, &value);

	ret = gp2a_i2c_read(data, COMMAND1, &value, sizeof(value));
	if (ret < 0)
		pr_info("%s, read data error\n", __func__);

	pr_info("%s, detection=%d, mode=%d\n", __func__,
		data->proximity_detection, data->lightsensor_mode);

	if (!data->proximity_detection)
		input_report_abs(data->prox_input_dev, ABS_DISTANCE, 1);
	else
		input_report_abs(data->prox_input_dev, ABS_DISTANCE,
			data->proximity_detection);
	input_sync(data->prox_input_dev);
}
#endif

static void gp2a_work_func_iio_prox(struct work_struct *work)
{
	struct gp2a_data *data = container_of((struct work_struct *)work,
		struct gp2a_data, proximity_work);
	struct iio_dev *indio_dev = iio_priv_to_dev(data);
	unsigned char value;
	int ret;

	ret = gp2a_i2c_read(data, COMMAND1, &value, sizeof(value));
	if (ret < 0) {
		pr_info("%s, read data error\n", __func__);
	} else {
		pr_info("%s, read data %d, %d\n", __func__,
			value & 0x08, !(value & 0x08));
		data->proximity_detection = !(value & 0x08);
		iio_proximity_detection = !(value & 0x08);
	}

	if (!(value & 0x08)) {
		if (data->lightsensor_mode == 0)
			value = 0x63;
		else
			value = 0x67;
		gp2a_i2c_write(data, COMMAND2, &value);
	} else {
		if (data->lightsensor_mode == 0)
			value = 0x23;
		else
			value = 0x27;
		gp2a_i2c_write(data, COMMAND2, &value);
	}

	value = 0xCC;
	gp2a_i2c_write(data, COMMAND1, &value);

	ret = gp2a_i2c_read(data, COMMAND1, &value, sizeof(value));
	if (ret < 0)
		pr_info("%s, read data error\n", __func__);

	pr_info("%s, detection=%d, mode=%d\n", __func__,
		data->proximity_detection, data->lightsensor_mode);

	gp2a_prox_data_rdy_trig_poll(indio_dev);
}



static irqreturn_t gp2a_irq_handler(int irq, void *dev_id)
{
	struct gp2a_data *gp2a = dev_id;
	wake_lock_timeout(&gp2a->prx_wake_lock, 3 * HZ);

	schedule_work(&gp2a->proximity_work);

	pr_info("%s IRQ_HANDLED.\n", __func__);
	return IRQ_HANDLED;
}

static int gp2a_setup_irq(struct gp2a_data *data)
{
	int rc;

	pr_err("%s, start\n", __func__);

	rc = gpio_request(data->gpio, "gpio_gp2a_prox_out");
	if (unlikely(rc < 0)) {
		pr_err("%s: gpio_gp2a_prox_out gpio %d request failed (%d)\n",
			__func__, data->gpio, rc);
		goto err_gpio_direction_output;
	}

	rc = gpio_direction_input(data->gpio);
	if (unlikely(rc < 0)) {
		pr_err("%s:gpio_gp2a_prox_out  failed to set gpio %d as input (%d)\n",
			__func__, data->gpio, rc);
		goto err_gpio_direction_input;
	}
	data->irq = gpio_to_irq(data->gpio);

	rc = request_irq(data->irq, gp2a_irq_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"proximity_int", data);
	if (unlikely(rc < 0)) {
		pr_err("%s: request_threaded_irq  request_irq(%d) failed for gpio %d (%d)\n",
			__func__, data->irq, data->gpio, rc);
		goto err_request_irq;
	}

	disable_irq(data->irq);

	pr_info("%s, success\n", __func__);

	goto done;

err_request_irq:
err_gpio_direction_input:
	gpio_free(data->gpio);
err_gpio_direction_output:

done:
	return rc;
}

static int gp2a_parse_dt(struct gp2a_data *data, struct device *dev)
{
	struct device_node *this_node = dev->of_node;
	enum of_gpio_flags flags;
	int ret;

	if (this_node == NULL)
		return -ENODEV;

	data->gpio = of_get_named_gpio_flags(this_node,
		"gp2a030a,irq_gpio", 0, &flags);
	if (data->gpio < 0) {
		pr_err("%s : get irq_gpio(%d) error\n", __func__, data->gpio);
		return -ENODEV;
	}

	data->power_en = of_get_named_gpio_flags(this_node,
		"gp2a030a,en_gpio", 0, &flags);
	if (data->power_en < 0) {
		pr_err("%s : get power_en(%d) error\n", __func__,
			data->power_en);
		return -ENODEV;
	}

	ret = gpio_request(data->power_en, "prox_en");
	if(ret) {
		pr_err("%s: gpio request fail\n",__func__);
		return ret;
	}

	ret = gpio_direction_output(data->power_en, 1);
	if (ret) {
		pr_err("%s: unable to set_direction [%d]\n",__func__, data->power_en);
		return ret;
	}

	return 0;
}

static const struct iio_buffer_setup_ops gp2a_prox_buffer_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};

static const struct iio_buffer_setup_ops gp2a_light_buffer_setup_ops = {
	.preenable = &iio_sw_buffer_preenable,
	.postenable = &iio_triggered_buffer_postenable,
	.predisable = &iio_triggered_buffer_predisable,
};


static int gp2a_prox_probe_buffer(struct iio_dev *indio_dev)
{
	int ret;
	struct iio_buffer *buffer;

	buffer = iio_kfifo_allocate(indio_dev);

	if (!buffer) {
		ret = -ENOMEM;
		goto error_ret;
	}

	buffer->scan_timestamp = true;
	indio_dev->buffer = buffer;
	indio_dev->setup_ops = &gp2a_prox_buffer_setup_ops;
	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;

	ret = iio_buffer_register(indio_dev, indio_dev->channels,
			indio_dev->num_channels);
	if (ret)
		goto error_free_buf;
	iio_scan_mask_set(indio_dev, indio_dev->buffer, GP2A_SCAN_PROX_CH);
	return 0;

error_free_buf:
	iio_kfifo_free(indio_dev->buffer);
error_ret:
	return ret;
}

static int gp2a_light_probe_buffer(struct iio_dev *indio_dev)
{
	int ret;
	struct iio_buffer *buffer;

	buffer = iio_kfifo_allocate(indio_dev);
	if (!buffer) {
		ret = -ENOMEM;
		goto error_ret;
	}

	buffer->scan_timestamp = true;
	indio_dev->buffer = buffer;
	indio_dev->setup_ops = &gp2a_light_buffer_setup_ops;
	indio_dev->modes |= INDIO_BUFFER_TRIGGERED;

	ret = iio_buffer_register(indio_dev, indio_dev->channels,
			indio_dev->num_channels);
	if (ret)
		goto error_free_buf;
	iio_scan_mask_set(indio_dev, indio_dev->buffer, GP2A_SCAN_LIGHT_CH);
	pr_err("%s, successed \n", __func__);
	return 0;

error_free_buf:
	pr_err("%s, failed \n", __func__);
	iio_kfifo_free(indio_dev->buffer);
error_ret:
	return ret;
}

static int gp2a_prox_pseudo_irq_enable(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	if (!atomic_cmpxchg(&st->pseudo_irq_enable_prox, 0, 1)) {
		mutex_lock(&st->lock);
		gp2a_iio_prox_enable(indio_dev, 1);
		mutex_unlock(&st->lock);
		//schedule_delayed_work(&st->prox_work, 0);
	}

	return 0;
}

static int gp2a_prox_pseudo_irq_disable(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	if (atomic_cmpxchg(&st->pseudo_irq_enable_prox, 1, 0)) {
		//cancel_delayed_work_sync(&st->prox_work);
		mutex_lock(&st->lock);
		gp2a_iio_prox_enable(indio_dev, 0);
		mutex_unlock(&st->lock);
	}
	return 0;
}

static int gp2a_light_pseudo_irq_enable(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	int err = 0;
	pr_err(" %s : START\n", __func__);

	if (!atomic_cmpxchg(&st->pseudo_irq_enable_light, 0, 1)) {
		pr_err("%s, enable routine\n", __func__);
		mutex_lock(&st->lock);
		err = gp2a_iio_light_enable(st, 1);
		if (err < 0)
			pr_err(" %s : enable failed\n", __func__);
		mutex_unlock(&st->lock);
		schedule_delayed_work(&st->light_work, 0);
	}
	return 0;
}

static int gp2a_light_pseudo_irq_disable(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	if (atomic_cmpxchg(&st->pseudo_irq_enable_light, 1, 0)) {
		cancel_delayed_work_sync(&st->light_work);
		mutex_lock(&st->lock);
#if 0
		st->acc.set_enable(0);
#endif
		mutex_unlock(&st->lock);
	}
	return 0;
}

static int gp2a_prox_set_pseudo_irq(struct iio_dev *indio_dev, int enable)
{
	if (enable)
		gp2a_prox_pseudo_irq_enable(indio_dev);
	else
		gp2a_prox_pseudo_irq_disable(indio_dev);
	return 0;
}

static int gp2a_light_set_pseudo_irq(struct iio_dev *indio_dev, int enable)
{
	if (enable)
		gp2a_light_pseudo_irq_enable(indio_dev);
	else
		gp2a_light_pseudo_irq_disable(indio_dev);
	return 0;
}

static int gp2a_data_prox_rdy_trigger_set_state(struct iio_trigger *trig,
		bool state)
{
	struct iio_dev *indio_dev = iio_trigger_get_drvdata(trig);
	gp2a_prox_set_pseudo_irq(indio_dev, state);
	return 0;
}

static int gp2a_data_light_rdy_trigger_set_state(struct iio_trigger *trig,
		bool state)
{
	struct iio_dev *indio_dev = iio_trigger_get_drvdata(trig);
	pr_info("%s, called state = %d\n", __func__, state);
	gp2a_light_set_pseudo_irq(indio_dev, state);
	return 0;
}

static const struct iio_trigger_ops gp2a_prox_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = &gp2a_data_prox_rdy_trigger_set_state,
};
static const struct iio_trigger_ops gp2a_light_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = &gp2a_data_light_rdy_trigger_set_state,
};


static int gp2a_prox_probe_trigger(struct iio_dev *indio_dev)
{
	int ret;
	struct gp2a_data *st = iio_priv(indio_dev);
	indio_dev->pollfunc = iio_alloc_pollfunc(&gp2a_iio_pollfunc_store_boottime,
			&gp2a_prox_trigger_handler, IRQF_ONESHOT, indio_dev,
			"%s_consumer%d", indio_dev->name, indio_dev->id);
	pr_info("%s is called\n", __func__);
	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	st->trig = iio_trigger_alloc("%s-dev%d",
			indio_dev->name,
			indio_dev->id);
	if (!st->trig) {
		ret = -ENOMEM;
		goto error_dealloc_pollfunc;
	}
	st->trig->dev.parent = &st->client->dev;
	st->trig->ops = &gp2a_prox_trigger_ops;
	iio_trigger_set_drvdata(st->trig, indio_dev);
	ret = iio_trigger_register(st->trig);
	if (ret)
		goto error_free_trig;
	return 0;

error_free_trig:
	iio_trigger_free(st->trig);
error_dealloc_pollfunc:
	iio_dealloc_pollfunc(indio_dev->pollfunc);
error_ret:
	pr_info("%s is failed\n", __func__);
	return ret;
}

static int gp2a_light_probe_trigger(struct iio_dev *indio_dev)
{
	int ret;
	struct gp2a_data *st = iio_priv(indio_dev);
	indio_dev->pollfunc = iio_alloc_pollfunc(&gp2a_iio_pollfunc_store_boottime,
			&gp2a_light_trigger_handler, IRQF_ONESHOT, indio_dev,
			"%s_consumer%d", indio_dev->name, indio_dev->id);

	pr_err("%s is called\n", __func__);

	if (indio_dev->pollfunc == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	st->trig = iio_trigger_alloc("%s-dev%d",
			indio_dev->name,
			indio_dev->id);
	if (!st->trig) {
		ret = -ENOMEM;
		goto error_dealloc_pollfunc;
	}
	st->trig->dev.parent = &st->client->dev;
	st->trig->ops = &gp2a_light_trigger_ops;
	iio_trigger_set_drvdata(st->trig, indio_dev);
	ret = iio_trigger_register(st->trig);
	if (ret)
		goto error_free_trig;
	pr_err("%s is success\n", __func__);
	return 0;

error_free_trig:
	iio_trigger_free(st->trig);
error_dealloc_pollfunc:
	iio_dealloc_pollfunc(indio_dev->pollfunc);
error_ret:
	return ret;
}

static void gp2a_prox_remove_trigger(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	iio_trigger_unregister(st->trig);
	iio_trigger_free(st->trig);
	iio_dealloc_pollfunc(indio_dev->pollfunc);
}

static void gp2a_light_remove_trigger(struct iio_dev *indio_dev)
{
	struct gp2a_data *st = iio_priv(indio_dev);
	iio_trigger_unregister(st->trig);
	iio_trigger_free(st->trig);
	iio_dealloc_pollfunc(indio_dev->pollfunc);
}

static void gp2a_prox_remove_buffer(struct iio_dev *indio_dev)
{
	iio_buffer_unregister(indio_dev);
	iio_kfifo_free(indio_dev->buffer);
}

static void gp2a_light_remove_buffer(struct iio_dev *indio_dev)
{
	iio_buffer_unregister(indio_dev);
	iio_kfifo_free(indio_dev->buffer);
}

static int gp2a_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int err = 0;
	struct gp2a_data *data;
	struct gp2a_data *data_input;
	struct iio_dev *indio_dev_prox;
	struct iio_dev *indio_dev_light;
	u8 value;

	pr_info("%s, is called\n", __func__);

	if (client == NULL) {
		pr_err("%s, client doesn't exist\n", __func__);
		err = -ENOMEM;
		return err;
	}
/*
	data = kzalloc(sizeof(struct gp2a_data), GFP_KERNEL);
	if (!data) {
		pr_err("%s, kzalloc error\n", __func__);
		err = -ENOMEM;
		return err;
	}
*/
	/* iio device register - light*/
	indio_dev_light = iio_device_alloc(sizeof(*data));
	if (!indio_dev_light) {
		pr_err("%s, iio_dev_light alloc failed\n", __func__);
		goto gp2a_parse_dt_err;
	}
	i2c_set_clientdata(client, indio_dev_light);

	indio_dev_light->name = GP2A_LIGHT_NAME;
	indio_dev_light->dev.parent = &client->dev;
	indio_dev_light->info = &gp2a_light_info;
	indio_dev_light->channels = gp2a_light_channels;
	indio_dev_light->num_channels = ARRAY_SIZE(gp2a_light_channels);
	indio_dev_light->modes = INDIO_DIRECT_MODE;

	data = iio_priv(indio_dev_light);
	data->client = client;
	data->sampling_frequency_light = 5;
	data->light_delay = MSEC_PER_SEC / data->sampling_frequency_light;

	spin_lock_init(&data->spin_lock);
	mutex_init(&data->lock);
	mutex_init(&data->data_mutex);
	mutex_init(&data->light_mutex);
	mutex_init(&data->prox_mutex);

	pr_err("%s, before light probe buffer\n", __func__);
	err = gp2a_light_probe_buffer(indio_dev_light);
	if (err) {
		pr_err("%s, light probe buffer failed\n", __func__);
		goto error_free_light_dev;
	}

	err = gp2a_light_probe_trigger(indio_dev_light);
	if (err) {
		pr_err("%s, light probe trigger failed\n", __func__);
		goto error_remove_light_buffer;
	}
	err = iio_device_register(indio_dev_light);
	if (err) {
		pr_err("%s, light iio register failed\n", __func__);
		goto error_remove_light_trigger;
	}
	INIT_DELAYED_WORK(&data->light_work, gp2a_work_func_light);
	pr_err("%s, iio done\n", __func__);

	data->light_enabled = SENSOR_DISABLE;
	iio_light_enable = SENSOR_DISABLE;

	/* iio device register - prox*/
	indio_dev_prox = iio_device_alloc(sizeof(*data));
	if (!indio_dev_prox) {
		pr_err("%s, iio_dev_prox alloc failed\n", __func__);
		goto gp2a_parse_dt_err;
	}

	i2c_set_clientdata(client, indio_dev_prox);

	indio_dev_prox->name = GP2A_PROX_NAME;
	indio_dev_prox->dev.parent = &client->dev;
	indio_dev_prox->info = &gp2a_prox_info;
	indio_dev_prox->channels = gp2a_prox_channels;
	indio_dev_prox->num_channels = ARRAY_SIZE(gp2a_prox_channels);
	indio_dev_prox->modes = INDIO_DIRECT_MODE;

	data = iio_priv(indio_dev_prox);
	data->client = client;
	data->sampling_frequency_prox = 5;
	data->prox_delay = MSEC_PER_SEC / data->sampling_frequency_prox;

	data->prox_enabled = SENSOR_DISABLE;
	iio_prox_enable = SENSOR_DISABLE;

	spin_lock_init(&data->spin_lock);
	mutex_init(&data->lock);
	mutex_init(&data->data_mutex);
	mutex_init(&data->light_mutex);
	mutex_init(&data->prox_mutex);

	/* probe buffer */
	pr_err("%s, before prox probe buffer\n", __func__);
	err = gp2a_prox_probe_buffer(indio_dev_prox);
	if (err) {
		pr_err("%s, prox probe buffer failed\n", __func__);
		goto error_free_prox_dev;
	}

	/* probe trigger */
	err = gp2a_prox_probe_trigger(indio_dev_prox);
	if (err) {
		pr_err("%s, prox probe trigger failed\n", __func__);
		goto error_remove_prox_buffer;
	}
	/* iio device register */
	err = iio_device_register(indio_dev_prox);
	if (err) {
		pr_err("%s, prox iio register failed\n", __func__);
		goto error_remove_prox_trigger;
	}
	/* end of iio */

	data_input = kzalloc(sizeof(struct gp2a_data), GFP_KERNEL);
	if (!data_input) {
		pr_err("%s, kzalloc error\n", __func__);
		err = -ENOMEM;
		return err;
	}
	err = gp2a_parse_dt(data_input, &client->dev);
	if (err) {
		pr_err("%s, get gpio is failed\n", __func__);
		goto gp2a_parse_dt_err;
	}
	data_input->client = client;
	data_input->light_delay = MAX_DELAY;
	i2c_set_clientdata(client, data_input);
	shutdown = false;

	sensor_power_on_vdd(data_input, 1);
	value = 0x00;
	err = gp2a_i2c_write(data_input, (u8) (COMMAND1), &value);
	if (err < 0) {
		pr_err("%s failed : threre is no such device.\n", __func__);
		goto i2c_fail_err;
	}

	data_input->light_input_dev = input_allocate_device();
	if (!data_input->light_input_dev) {
		pr_err("%s input_allocate_device error\n", __func__);
		err = -ENOMEM;
		goto input_allocate_light_device_err;
	}

	data_input->light_input_dev->name = "light_sensor";
	input_set_capability(data_input->light_input_dev, EV_REL, REL_MAX);
	input_set_capability(data_input->light_input_dev, EV_REL, REL_MISC);
	input_set_drvdata(data_input->light_input_dev, data_input);

	err = input_register_device(data_input->light_input_dev);
	if (err < 0) {
		pr_err("%s input_register_device light error\n", __func__);
		goto input_register_device_err;
	}

	err = sysfs_create_group(&data_input->light_input_dev->dev.kobj,
				&gp2a_light_attribute_group);
	if (err) {
		pr_err("%s sysfs_create_group light error\n", __func__);
		goto sysfs_create_group_light_err;
	}

	//err = gp2a_setup_irq(data_input);
	data->gpio = data_input->gpio;
	err = gp2a_setup_irq(data);
	if (err) {
		pr_err("%s: could not setup irq\n", __func__);
		goto err_setup_irq;
	}

	data_input->prox_input_dev = input_allocate_device();
	if (!data_input->prox_input_dev) {
		pr_err("%s input_allocate_device error\n", __func__);
		err = -ENOMEM;
		goto input_allocate_prox_device_err;
	}

	data_input->prox_input_dev->name = "proximity_sensor";
	input_set_capability(data_input->prox_input_dev, EV_ABS, ABS_DISTANCE);
	input_set_capability(data_input->prox_input_dev, EV_ABS, ABS_MAX);
	input_set_abs_params(data_input->prox_input_dev, ABS_DISTANCE, 0, 1, 0, 0);
	input_set_abs_params(data_input->prox_input_dev, ABS_MAX, 0, 1, 0, 0);
	input_set_drvdata(data_input->prox_input_dev, data);

	err = input_register_device(data_input->prox_input_dev);
	if (err < 0) {
		pr_err("%s input_register_device prox error\n", __func__);
		goto input_register_prox_device_err;
	}

	err = sysfs_create_group(&data_input->prox_input_dev->dev.kobj,
				&gp2a_prox_attribute_group);
	if (err) {
		pr_err("%s sysfs_create_group prox error\n", __func__);
		goto sysfs_create_group_prox_err;
	}

	err = sensors_register(data_input->light_sensor_device,
			data_input, light_sensor_attrs, "light_sensor");
	if (err) {
		pr_err("%s: cound not register prox sensor device(%d).\n",
			__func__, err);
		goto sensors_register_light_err;
	}
	err = sensors_register(data_input->prox_sensor_device,
			data_input, prox_sensor_attrs, "proximity_sensor");
	if (err) {
		pr_err("%s: cound not register prox sensor device(%d).\n",
			__func__, err);
		goto sensors_register_prox_err;
	}

	mutex_init(&data_input->lock);
	mutex_init(&data_input->data_mutex);
	mutex_init(&data_input->light_mutex);
	mutex_init(&data_input->prox_mutex);

	//wake_lock_init(&data_input->prx_wake_lock, WAKE_LOCK_SUSPEND,
	//	"prx_wake_lock");
	//INIT_WORK(&data_input->proximity_work, gp2a_work_func_prox);
//	INIT_WORK(&data_input->proximity_work, gp2a_work_func_iio_prox);
	wake_lock_init(&data->prx_wake_lock, WAKE_LOCK_SUSPEND,
		"prx_wake_lock");
	INIT_WORK(&data->proximity_work, gp2a_work_func_iio_prox);
	INIT_DELAYED_WORK(&data_input->prox_avg_work, gp2a_work_avg_prox);

	pr_info("%s: success\n", __func__);
	return 0;

	mutex_destroy(&data_input->light_mutex);
	mutex_destroy(&data_input->data_mutex);

	sensors_unregister(data_input->prox_sensor_device, prox_sensor_attrs);
sensors_register_prox_err:
	sensors_unregister(data_input->light_sensor_device, light_sensor_attrs);
sensors_register_light_err:

	sysfs_remove_group(&data_input->prox_input_dev->dev.kobj,
			&gp2a_prox_attribute_group);
sysfs_create_group_prox_err:
	input_unregister_device(data_input->prox_input_dev);
input_register_prox_device_err:
	input_free_device(data_input->prox_input_dev);
input_allocate_prox_device_err:
	gpio_free(data_input->gpio);
err_setup_irq:
	sysfs_remove_group(&data_input->light_input_dev->dev.kobj,
			&gp2a_light_attribute_group);
sysfs_create_group_light_err:
	input_unregister_device(data_input->light_input_dev);
input_register_device_err:
	input_free_device(data_input->light_input_dev);

input_allocate_light_device_err:

i2c_fail_err:
	iio_device_unregister(indio_dev_light);
error_remove_light_trigger:
	gp2a_light_remove_trigger(indio_dev_light);
error_remove_light_buffer:
	gp2a_light_remove_buffer(indio_dev_light);
error_free_light_dev:
	iio_device_free(indio_dev_light);

	iio_device_unregister(indio_dev_prox);
error_remove_prox_trigger:
	gp2a_prox_remove_trigger(indio_dev_prox);
error_remove_prox_buffer:
	gp2a_prox_remove_buffer(indio_dev_prox);
error_free_prox_dev:
	iio_device_free(indio_dev_prox);
gp2a_parse_dt_err:
/*
	sensor_power_on_vdd(data, 0);
	bShutdown = true;
*/
	pr_info("%s, is failed\n", __func__);
	return err;
}

static void gp2a_shutdown(struct i2c_client *client)
{
	pr_info("%s, is called\n", __func__);
}

static int gp2a_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct gp2a_data *data = i2c_get_clientdata(client);

	disable_irq(data->irq);

	if (data->light_enabled) {
		cancel_delayed_work_sync(&data->light_work);
		gp2a_lightsensor_onoff(0, data);
	}
	pr_info("%s, is called\n", __func__);
	return 0;
}

static int gp2a_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct gp2a_data *data = i2c_get_clientdata(client);

	if (data->light_enabled) {
		gp2a_lightsensor_onoff(1, data);
		schedule_delayed_work(&data->light_work,
				msecs_to_jiffies(data->light_delay));
	}
	enable_irq(data->irq);

	pr_info("%s, is called\n", __func__);
	return 0;
}

static const u16 normal_i2c[] = { I2C_CLIENT_END };

static struct of_device_id gp2a_match_table[] = {
	{ .compatible = "gp2a030a",},
	{},
};

static const struct i2c_device_id gp2a_device_id[] = {
	{"gp2a030a", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, gp2a_device_id);

static const struct dev_pm_ops gp2a_pm_ops = {
	.suspend = gp2a_suspend,
	.resume = gp2a_resume,
};

static struct i2c_driver gp2a_driver = {
	.driver = {
		   .name = "gp2a030a",
		   .owner = THIS_MODULE,
		   .pm = &gp2a_pm_ops,
		   .of_match_table = gp2a_match_table,
	},
	.probe = gp2a_probe,
	.shutdown = gp2a_shutdown,
	.id_table = gp2a_device_id,
	.address_list = normal_i2c,
};

module_i2c_driver(gp2a_driver);

MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("Optical Sensor driver for GP2AP030A00F");
MODULE_LICENSE("GPL");
