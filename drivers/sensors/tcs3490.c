/*
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wakelock.h>
#include <linux/sensor/sensors_core.h>

#define VENDOR_NAME			"AMS"
#define MODEL_NAME			"TCS3490"
#define LIGHT_MODULE_NAME		"light_sensor"

#define I2C_M_WR			0 /* for i2c Write */
#define I2c_M_RD			1 /* for i2c Read */

#define EVENT_TYPE_LIGHT_LUX		REL_MISC
#define EVENT_TYPE_LIGHT_CCT		REL_WHEEL

#define CHIP_ID				0x87

/* TCS3490 register offsets */
#define CNTRL				0x00
#define ALS_TIME			0X01
#define WAIT_TIME			0x03
#define ALS_MINTHRESHLO			0X04
#define ALS_MINTHRESHHI			0X05
#define ALS_MAXTHRESHLO			0X06
#define ALS_MAXTHRESHHI			0X07
#define INTERRUPT			0x0C
#define CONFIG				0x0D
#define GAIN				0x0F
#define AUX_CNTRL			0x10
#define REVID				0x11
#define CHIPID				0x12
#define STATUS				0x13
#define CLR_CHAN0LO			0x14
#define CLR_CHAN0HI			0x15
#define RED_CHAN1LO			0x16
#define RED_CHAN1HI			0x17
#define GRN_CHAN1LO			0x18
#define GRN_CHAN1HI			0x19
#define BLU_CHAN1LO			0x1A
#define BLU_CHAN1HI			0x1B

/*TCS3490 cmd reg masks*/
#define CMD_REG				0X80
#define CMD_BYTE_RW			0x00
#define CMD_WORD_BLK_RW			0x20
#define CMD_SPL_FN			0x60
#define CMD_ALS_INTCLR			0X06
#define CMD_TST_REG			0X08
#define CMD_USER_REG			0X09

/* TCS3490 cntrl reg masks */
#define CNTL_REG_CLEAR			0x00
#define CNTL_ALS_INT_ENBL		0X10
#define CNTL_WAIT_TMR_ENBL		0X08
#define CNTL_ADC_ENBL			0x02
#define CNTL_PWRON			0x01
#define CNTL_ALSPON_ENBL		0x03
#define CNTL_INTALSPON_ENBL		0x13
#define CNTL_ENBL			0x0B

/* TCS3490 status reg masks */
#define STA_ADCVALID			0x01
#define STA_ADCINTR			0x10

#define MAX_LUX				150000

#define ALS_TIME_SET			0xEB
#define WAIT_TIME_SET			0xFF
#define INTER_FILTER_SET		0x23
#define CONFIG_SET			0x00
#define ALS_GAIN_SET			0x22

#define LIGHT_LOG_TIME			15 /* 15 sec */
#define LIGHT_SENSOR_DEFAULT_DELAY	200000000LL

enum {
	OFF = 0,
	ON = 1,
};

struct tcs3490_p {
	struct i2c_client *i2c_client;
	struct input_dev *light_input;
	struct device *light_dev;
	struct delayed_work light_work;
	atomic_t light_poll_delay;
	bool enabled;
	u16 clrdata;
	u16 reddata;
	u16 grndata;
	u16 bludata;
	int lux;
	int irdata;
	int atime_ms;
	int dgf;
	int coef_b;
	int coef_c;
	int coef_d;
	int ct_coef;
	int ct_offset;
	int integration_cycle;
	int time_count;
};

static int tcs3490_i2c_read_word(struct tcs3490_p *data, u8 reg_addr, u16 *buf)
{
	int ret;
	struct i2c_msg msg[2];
	u8 r_buf[2];

	msg[0].addr = data->i2c_client->addr;
	msg[0].flags = I2C_M_WR;
	msg[0].len = 1;
	msg[0].buf = &reg_addr;

	msg[1].addr = data->i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = r_buf;

	ret = i2c_transfer(data->i2c_client->adapter, msg, 2);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - i2c read error %d\n", __func__, ret);

		return ret;
	}

	*buf = (u16)r_buf[1];
	*buf = (*buf << 8) | (u16)r_buf[0];

	return 0;
}

static int tcs3490_i2c_write(struct tcs3490_p *data, u8 reg_addr, u8 buf)
{
	int ret = 0;
	struct i2c_msg msg;
	u8 w_buf[2];

	w_buf[0] = reg_addr;
	w_buf[1] = buf;

	/* send slave address & command */
	msg.addr = data->i2c_client->addr;
	msg.flags = I2C_M_WR;
	msg.len = 2;
	msg.buf = (char *)w_buf;

	ret = i2c_transfer(data->i2c_client->adapter, &msg, 1);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - error %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int tcs3490_chip_enable(struct tcs3490_p *data, int enable)
{
	u8 buf;
	int ret;

	if (enable == ON)
		buf = CNTL_ENBL;
	else
		buf = CNTL_REG_CLEAR;

	ret = tcs3490_i2c_write(data, CMD_REG | CNTRL, buf);
	if (ret < 0)
		pr_err("[SENSOR]: %s - fail! %d\n", __func__, ret);

	return ret;
}

static int tcs3490_get_cct(struct tcs3490_p *data)
{
	int bp1 = (int)data->bludata - data->irdata;
	int rp1 = (int)data->reddata - data->irdata;
	int cct = 0;

	if (rp1 != 0)
		cct = data->ct_coef * bp1 / rp1 + data->ct_offset;

	return cct;
}

static int tcs3490_get_lux(struct tcs3490_p *data)
{
	u16 gain_reg = 0x0;
	int rp1, gp1, bp1;
	int lux = 0;
	int gain = 1;
	int ret = 0;

	ret = tcs3490_i2c_read_word(data, CMD_REG | GAIN, &gain_reg);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - fail! %d\n", __func__, ret);
		return 0;
	}

	tcs3490_i2c_read_word(data, CMD_REG | CLR_CHAN0LO, &data->clrdata);
	tcs3490_i2c_read_word(data, CMD_REG | RED_CHAN1LO, &data->reddata);
	tcs3490_i2c_read_word(data, CMD_REG | GRN_CHAN1LO, &data->grndata);
	tcs3490_i2c_read_word(data, CMD_REG | BLU_CHAN1LO, &data->bludata);

	switch (gain_reg & 0x03) {
	case 0x00:
		gain = 1;
		break;
	case 0x01:
		gain = 4;
		break;
	case 0x02:
		gain = 16;
		break;
#if 0
	case 0x03:
		gain = 64;
		break;
#endif
	default:
		break;
	}

	if (gain == 1 && data->clrdata < 25) {
		tcs3490_i2c_write(data, CMD_REG | GAIN, 0x22);
		return data->lux;
	} else if (gain == 16 && data->clrdata > 15000) {
		tcs3490_i2c_write(data, CMD_REG | GAIN, 0x20);
		return data->lux;
	}

	if ((data->clrdata >= 18500) && (gain == 1)) {
		lux = MAX_LUX;
		return lux;
	}

	/* calculate lux */
	data->irdata = ((int)data->reddata + (int)data->grndata
			+ (int)data->bludata - (int)data->clrdata) / 2;

	/* remove ir from counts */
	rp1 = (int)data->reddata - data->irdata;
	gp1 = (int)data->grndata - data->irdata;
	bp1 = (int)data->bludata - data->irdata;

	lux = rp1 * data->coef_b + gp1 * data->coef_c + bp1 * data->coef_d;
	lux /= 1000;

	if (lux < 0) {
		lux = 0;
	} else {
		/* divide by CPL, CPL = (ATIME_MS * ALS_GAIN / DGF) */
		lux = lux * data->dgf;
		/* ATIME_MS */
		lux *= 10;
		lux /= data->atime_ms;
		/* Again */
		lux /= gain;
	}

	data->lux = lux;
	return lux;
}

static void tcs3490_light_enable(struct tcs3490_p *data)
{
	schedule_delayed_work(&data->light_work,
		nsecs_to_jiffies(atomic_read(&data->light_poll_delay)));
}

static void tcs3490_light_disable(struct tcs3490_p *data)
{
	cancel_delayed_work_sync(&data->light_work);
}

static void tcs3490_work_func(struct work_struct *work)
{
	struct tcs3490_p *data = container_of((struct delayed_work *)work,
			struct tcs3490_p, light_work);
	unsigned long delay =
			nsecs_to_jiffies(atomic_read(&data->light_poll_delay));
	int lux = tcs3490_get_lux(data);
	int cct = tcs3490_get_cct(data);

	input_report_rel(data->light_input, EVENT_TYPE_LIGHT_LUX, lux + 1);
	input_report_rel(data->light_input, EVENT_TYPE_LIGHT_CCT, cct);
	input_sync(data->light_input);

	if (((int64_t)atomic_read(&data->light_poll_delay)
		* (int64_t)data->time_count)
		>= ((int64_t)LIGHT_LOG_TIME * NSEC_PER_SEC)) {
		pr_info("[SENSOR]: %s - r = %u g = %u b = %u c = %u lux = %d\n",
			__func__, data->reddata, data->grndata,	data->bludata,
			data->clrdata, lux);
		data->time_count = 0;
	} else {
		data->time_count++;
	}

	schedule_delayed_work(&data->light_work, delay);
}

/* sysfs */
static ssize_t tcs3490_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			atomic_read(&data->light_poll_delay));
}

static ssize_t tcs3490_delay_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);
	int64_t new_delay;
	int ret;

	ret = kstrtoll(buf, 10, &new_delay);
	if (ret) {
		pr_err("[SENSOR]: %s - Invalid Argument\n", __func__);
		return ret;
	}

	atomic_set(&data->light_poll_delay, new_delay);
	pr_info("[SENSOR]: %s - poll_delay = %lld\n", __func__, new_delay);

	return size;
}

static ssize_t tcs3490_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	u8 enable;
	int ret;
	struct tcs3490_p *data = dev_get_drvdata(dev);

	ret = kstrtou8(buf, 2, &enable);
	if (ret) {
		pr_err("[SENSOR]: %s - Invalid Argument\n", __func__);
		return size;
	}
	pr_info("[SENSOR]: %s - new_value = %u, old_value = %u\n",
		__func__, enable, data->enabled);

	if (enable == (u8)data->enabled)
		return size;

	if (enable) {
		data->enabled = ON;
		tcs3490_chip_enable(data, ON);
		tcs3490_light_enable(data);
	} else {
		tcs3490_light_disable(data);
		tcs3490_chip_enable(data, OFF);
		data->enabled = OFF;
	}

	return size;
}

static ssize_t tcs3490_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->enabled);
}

static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		tcs3490_delay_show, tcs3490_delay_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
		tcs3490_enable_show, tcs3490_enable_store);

static struct attribute *light_sysfs_attrs[] = {
	&dev_attr_enable.attr,
	&dev_attr_poll_delay.attr,
	NULL,
};

static struct attribute_group light_attribute_group = {
	.attrs = light_sysfs_attrs,
};

/* light sysfs */
static ssize_t tcs3490_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", MODEL_NAME);
}

static ssize_t tcs3490_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", VENDOR_NAME);
}

static ssize_t tcs3490_lux_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u,%u,%u,%u\n",
			data->reddata, data->grndata,
			data->bludata, data->clrdata);
}

static ssize_t tcs3490_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%u,%u,%u,%u\n",
			data->reddata, data->grndata,
			data->bludata, data->clrdata);
}

static DEVICE_ATTR(name, S_IRUGO, tcs3490_name_show, NULL);
static DEVICE_ATTR(vendor, S_IRUGO, tcs3490_vendor_show, NULL);
static DEVICE_ATTR(lux, S_IRUGO, tcs3490_lux_show, NULL);
static DEVICE_ATTR(raw_data, S_IRUGO, tcs3490_data_show, NULL);

static struct device_attribute *light_sensor_attrs[] = {
	&dev_attr_name,
	&dev_attr_vendor,
	&dev_attr_lux,
	&dev_attr_raw_data,
	NULL,
};

static int tcs3490_input_init(struct tcs3490_p *data)
{
	int ret = 0;
	struct input_dev *dev;

	/* allocate lightsensor input_device */
	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;

	dev->name = LIGHT_MODULE_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_REL, EVENT_TYPE_LIGHT_LUX);
	input_set_capability(dev, EV_REL, EVENT_TYPE_LIGHT_CCT);
	input_set_drvdata(dev, data);

	ret = input_register_device(dev);
	if (ret < 0) {
		input_free_device(dev);
		return ret;
	}

	ret = sensors_create_symlink(&dev->dev.kobj, dev->name);
	if (ret < 0) {
		input_unregister_device(dev);
		return ret;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &light_attribute_group);
	if (ret < 0) {
		sensors_remove_symlink(&dev->dev.kobj, dev->name);
		input_unregister_device(dev);
		return ret;
	}

	data->light_input = dev;
	return 0;
}

static int tcs3490_parse_dt(struct tcs3490_p *data, struct device *dev)
{
	struct device_node *dNode = dev->of_node;
	int ret;
	u32 temp;

	if (dNode == NULL)
		return -ENODEV;

	ret = of_property_read_u32(dNode, "ams,atime_ms", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid atime_ms value %u\n",
			__func__, ret);
		data->atime_ms = 504;
	} else {
		data->atime_ms = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,dgf", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid dgf value %u\n",
			__func__, ret);
		data->dgf = 550;
	} else {
		data->dgf = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,coef_b", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid coef_b value %u\n",
			__func__, ret);
		data->coef_b = 535;
	} else {
		data->coef_b = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,coef_c", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid coef_c value %u\n",
			__func__, ret);
		data->coef_c = 1000;
	} else {
		data->coef_c = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,coef_d", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid coef_d value %u\n",
			__func__, ret);
		data->coef_d = 795;
	} else {
		data->coef_d = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,ct_coef", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid ct_coef value %u\n",
			__func__, ret);
		data->ct_coef = 2855;
	} else {
		data->ct_coef = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,ct_offset", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid ct_offset value %u\n",
			__func__, ret);
		data->ct_offset = 1973;
	} else {
		data->ct_offset = (int)temp;
	}

	ret = of_property_read_u32(dNode, "ams,integration_cycle", &temp);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - invalid integration_cycle value %u\n",
			__func__, ret);
		data->integration_cycle = 240;
	} else {
		data->integration_cycle = (int)temp;
	}

	return 0;
}

static int tcs3490_initailize_register(struct tcs3490_p *data)
{
	int ret;

	ret = tcs3490_i2c_write(data, CMD_REG | CNTRL, CNTL_PWRON);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | ALS_TIME, ALS_TIME_SET);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | WAIT_TIME, WAIT_TIME_SET);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | INTERRUPT, INTER_FILTER_SET);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | CONFIG, CONFIG_SET);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | GAIN, ALS_GAIN_SET);
	if (ret < 0)
		goto exit_i2c_fail;

	ret = tcs3490_i2c_write(data, CMD_REG | CNTRL, CNTL_REG_CLEAR);
	if (ret < 0)
		goto exit_i2c_fail;

	goto exit;

exit_i2c_fail:
	pr_err("[SENSOR]: %s - fail! %d\n", __func__, ret);
exit:
	return ret;
}

static int tcs3490_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret = -ENODEV;
	struct tcs3490_p *data = NULL;

	pr_info("[SENSOR]: %s - Probe Start!\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("[SENSOR]: %s - i2c_check_functionality error\n",
			__func__);
		goto exit;
	}

	data = kzalloc(sizeof(struct tcs3490_p), GFP_KERNEL);
	if (data == NULL) {
		pr_err("[SENSOR]: %s - kzalloc error\n", __func__);
		ret = -ENOMEM;
		goto exit_kzalloc;
	}

	ret = tcs3490_parse_dt(data, &client->dev);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - of_node error\n", __func__);
		ret = -ENODEV;
		goto exit_of_node;
	}

	data->i2c_client = client;
	i2c_set_clientdata(client, data);

	/* ID Check */
	ret = i2c_smbus_read_byte_data(client, CMD_REG | CHIPID);
	if (ret != CHIP_ID) {
		pr_err("[SENSOR]: %s - chipid error [%d]\n", __func__, ret);
		goto exit_chip_id;
	}

	ret = tcs3490_initailize_register(data);
	if (ret < 0) {
		pr_err("[SENSOR]: %s - init reg error [%d]\n", __func__, ret);
		goto exit_initailize_register;
	}

	/* input device init */
	ret = tcs3490_input_init(data);
	if (ret < 0)
		goto exit_light_input_init;

	atomic_set(&data->light_poll_delay, LIGHT_SENSOR_DEFAULT_DELAY);
	data->time_count = 0;
	data->enabled = 0;
	data->lux = 0;

	INIT_DELAYED_WORK(&data->light_work, tcs3490_work_func);

	/* set sysfs for light sensor */
	ret = sensors_register(data->light_dev, data, light_sensor_attrs,
			LIGHT_MODULE_NAME);
	if (ret < 0)
		goto exit_light_sensors_register;

	pr_info("[SENSOR]: %s - Probe done!\n", __func__);

	return 0;


exit_light_sensors_register:
	sensors_remove_symlink(&data->light_input->dev.kobj,
			data->light_input->name);
	sysfs_remove_group(&data->light_input->dev.kobj,
			&light_attribute_group);
	input_unregister_device(data->light_input);
exit_light_input_init:
exit_initailize_register:
exit_chip_id:
exit_of_node:
	kfree(data);
exit_kzalloc:
exit:
	pr_err("[SENSOR]: %s - Probe fail!\n", __func__);
	return ret;
}

static void tcs3490_shutdown(struct i2c_client *client)
{
	struct tcs3490_p *data = i2c_get_clientdata(client);

	pr_info("[SENSOR]: %s\n", __func__);
	if (data->enabled) {
		tcs3490_light_disable(data);
		tcs3490_chip_enable(data, OFF);
		data->enabled = OFF;
	}
}

static int tcs3490_remove(struct i2c_client *client)
{
	struct tcs3490_p *data = i2c_get_clientdata(client);

	/* device off */
	if (data->enabled) {
		tcs3490_light_disable(data);
		tcs3490_chip_enable(data, OFF);
		data->enabled = OFF;
	}

	/* sysfs destroy */
	sensors_unregister(data->light_dev, light_sensor_attrs);

	/* symbolic link destroy */
	sensors_remove_symlink(&data->light_input->dev.kobj,
			data->light_input->name);

	/* input sysfs destroy */
	sysfs_remove_group(&data->light_input->dev.kobj,
			&light_attribute_group);

	/* input device destroy */
	input_unregister_device(data->light_input);
	kfree(data);

	return 0;
}

static int tcs3490_suspend(struct device *dev)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	if (data->enabled) {
		pr_info("[SENSOR]: %s\n", __func__);
		tcs3490_light_disable(data);
		tcs3490_chip_enable(data, OFF);
	}

	return 0;
}

static int tcs3490_resume(struct device *dev)
{
	struct tcs3490_p *data = dev_get_drvdata(dev);

	if (data->enabled) {
		pr_info("[SENSOR]: %s\n", __func__);
		tcs3490_chip_enable(data, ON);
		tcs3490_light_enable(data);
	}

	return 0;
}

static struct of_device_id tcs3490_match_table[] = {
	{ .compatible = "ams,tcs3490",},
	{},
};

static const struct i2c_device_id tcs3490_id[] = {
	{ "tcs3490_match_table", 0 },
	{ }
};

static const struct dev_pm_ops tcs3490_pm_ops = {
	.suspend = tcs3490_suspend,
	.resume = tcs3490_resume
};

static struct i2c_driver tcs3490_i2c_driver = {
	.driver = {
		.name = MODEL_NAME,
		.owner = THIS_MODULE,
		.of_match_table = tcs3490_match_table,
		.pm = &tcs3490_pm_ops
	},
	.probe = tcs3490_probe,
	.shutdown = tcs3490_shutdown,
	.remove = tcs3490_remove,
	.id_table = tcs3490_id,
};

static int __init tcs3490_init(void)
{
	return i2c_add_driver(&tcs3490_i2c_driver);
}

static void __exit tcs3490_exit(void)
{
	i2c_del_driver(&tcs3490_i2c_driver);
}

module_init(tcs3490_init);
module_exit(tcs3490_exit);

MODULE_DESCRIPTION("Light Sensor device driver for tcs3490");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
