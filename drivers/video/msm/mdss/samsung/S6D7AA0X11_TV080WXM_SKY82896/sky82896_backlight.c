/*
 * sky82896-backlight.c - Platform data for SKY82896 backlight driver
 *
 * Copyright (C) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <asm/unaligned.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/regulator/consumer.h>
#include <linux/device.h>
#include <linux/of_gpio.h>

#include "../../mdss.h"
#include "../../mdss_panel.h"
#include "../../mdss_mdp.h"

#include "../ss_dsi_panel_common.h"

/*#define CONFIG_BLIC_TUNING 1*/

struct tuning_table {
	u8* table;
	u8 table_length;
};
struct backlight_platform_data {
	unsigned	 int gpio_backlight_en;
	unsigned	 int gpio_backlight_pwm;
	u32 en_gpio_flags;

	int gpio_sda;
	u32 sda_gpio_flags;

	int gpio_scl;
	u32 scl_gpio_flags;
};

struct backlight_info {
	struct i2c_client			*client;
	struct backlight_platform_data	*pdata;
	struct tuning_table bl_ic_settings;
	struct tuning_table bl_ic_outdoor_settings;
	struct tuning_table bl_ic_normal_settings;
	struct tuning_table bl_control;
};

static struct backlight_info *bl_info;
static const char *bl_ic_name;

#if 0
static int backlight_i2c_read(struct i2c_client *client,
		u8 reg, u8 *val, unsigned int len)
{

	int err = 0;
	int retry = 3;

	while (retry--) {
		err = i2c_smbus_read_i2c_block_data(client,
				reg, len, val);
		if (err >= 0)
			return err;

		dev_info(&client->dev, "%s: i2c transfer error.\n", __func__);
	}
	return err;

}
#endif

static int backlight_i2c_write(struct i2c_client *client,
		u8 reg,  u8 val, unsigned int len)
{
	int err = 0;
	int retry = 3;
	u8 temp_val = val;

	while (retry--) {
		err = i2c_smbus_write_i2c_block_data(client,
				reg, len, &temp_val);
		if (err >= 0)
			return err;

		dev_info(&client->dev, "%s: i2c transfer error. %d\n", __func__, err);
	}
	return err;
}

static void pwm_backlight_control(int enable)
{
	struct backlight_info *info = bl_info;
	struct samsung_display_driver_data *vdd = samsung_get_vdd();

	if (!info) {
		pr_info("%s error bl_info", __func__);
		return ;
	}
	pr_info("skyworks %s :enable:[%d]\n", __func__,enable);
	if (!mdss_panel_attach_get(vdd->ctrl_dsi[DISPLAY_1]))
	{
		gpio_set_value(info->pdata->gpio_backlight_en,0);
		return;
	}

	if(enable) {
		int i;
		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,1);
		if (gpio_is_valid(info->pdata->gpio_backlight_pwm))
			gpio_set_value(info->pdata->gpio_backlight_pwm,1);
		if (!(info->bl_ic_settings.table_length) || !(info->bl_ic_settings.table)) {
			pr_info("%s : No backlight configuration :-", __func__);
			return ;
		}
		msleep(1);
		for (i = 0; i <info->bl_ic_settings.table_length;i=i+2)
			backlight_i2c_write(info->client, info->bl_ic_settings.table[i], info->bl_ic_settings.table[i+1], 1);
	} else {
		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,0);
		if (gpio_is_valid(info->pdata->gpio_backlight_pwm))
			gpio_set_value(info->pdata->gpio_backlight_pwm,0);
		msleep(1);
	}
}
static void pwm_backlight_outdoor_control(int enable)
{
	int i;
	struct backlight_info *info = bl_info;
	if (!info) {
		pr_info("%s error bl_info", __func__);
		return ;
	}
	pr_info("skyworks %s :enable:[%d]\n", __func__,enable);
	msleep(1);
	if(enable) {
		for (i = 0; i <info->bl_ic_outdoor_settings.table_length;i=i+2)
			backlight_i2c_write(info->client, info->bl_ic_outdoor_settings.table[i], info->bl_ic_outdoor_settings.table[i+1], 1);

	} else {
		for (i = 0; i <info->bl_ic_normal_settings.table_length;i=i+2)
			backlight_i2c_write(info->client, info->bl_ic_normal_settings.table[i], info->bl_ic_normal_settings.table[i+1], 1);

	}
}

#if defined(CONFIG_BLIC_TUNING)
static int atoi(const char *name, int *l)
{
	int val = 0;
	if(*name == '\0' || *name == '\n')
		return -1;
	for (;; name++) {
		(*l)++;
		switch (*name) {
			case '0' ... '9':
					val = 16*val+(*name-'0');
					break;
			case 'A' ... 'F':
				val = 16*val+(*name-'A') + 10;
				break;
			case 'a' ... 'f':
				val = 10*val+(*name-'a') + 10;
				break;
			case ' ':
				pr_debug("%s: val is: %d\n", __func__, val);
				return val;
				break;
			case '\n':
				pr_debug("%s: val is: %d\n", __func__, val);
				return val;
				break;
			default:
				return -1;
		}
	}
}
static int backlight_i2c_read_sys(u8 reg, u8* val)
{
	int err = 0;
	int retry = 3;
	struct backlight_info *info = bl_info;
	struct i2c_client *client=info->client;
	while (retry--) {
		err = i2c_smbus_read_i2c_block_data(client,
				reg, 1, val);
		if (err >= 0)
			return err;
		dev_info(&client->dev, "%s:i2c transfer error.\n", __func__);
	}
	return err;
}
static int backlight_i2c_write_sys(u8 reg,  u8 val)
{
	int err = 0;
	int retry = 3;
	u8 temp_val = val;
	struct backlight_info *info = bl_info;
	struct i2c_client *client=info->client;
	while (retry--) {
		err = i2c_smbus_write_i2c_block_data(client,
				reg, 1, &temp_val);
		if (err >= 0)
			return err;
		dev_info(&client->dev, "%s: i2c transfer error. %d\n", __func__, err);
	}
	return err;
}
static ssize_t backlight_i2c_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	u8 arr[6];
	u32 i, cnt=0;
	u8 val=0;
	for(i=0; i<3; i++)
		arr[i] = atoi(buf+cnt, &cnt);
	for(i=0; i<3; i++) {
		if(arr[i] == -1)
			break;
	}
	if(i<3) {
		pr_err("%s: Invalid value entered\n", __func__);
		return size;
	}
	backlight_i2c_read_sys(arr[0],&val);
	pr_info("%s: Before REG 0x%02X  = 0x%02X\n", __func__,arr[0],val);
	backlight_i2c_write_sys(arr[0],arr[1]);
	pr_info("%s: After REG 0x%02Xh =  0x%02X\n", __func__,arr[0],arr[1]);
	return size;
}
static ssize_t backlight_i2c_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 400;
	char temp[string_size];
	char temp1[40];
	int i;
	u8 val=0;
	snprintf(temp, 34, "BL IC Current register settings\n");
	for(i=0; i<0x20; i++) {
		backlight_i2c_read_sys(i,&val);
		snprintf(temp1, 22, "REG 0x%02Xh = 0x%02X\n", i,val);
		strncat(temp, temp1, 22);
	}
	strlcat(buf, temp, string_size);
	return strnlen(buf, string_size);
}
static ssize_t backlight_ic_name_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	if(bl_ic_name)
		snprintf(buf, 50, "BACKLIGHT IC NAME : %s\n",bl_ic_name);

	return strnlen(buf, 50);
}
static DEVICE_ATTR(bl_ic_tune, 0666,backlight_i2c_show , backlight_i2c_store);
static DEVICE_ATTR(bl_ic_name, S_IRUGO,backlight_ic_name_show , NULL);
#endif
static void backlight_request_gpio(struct backlight_platform_data *pdata)
{
	int ret;
	if (gpio_is_valid(pdata->gpio_backlight_en)) {
		ret = gpio_request(pdata->gpio_backlight_en, "backlight_en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request backlight_en [%d]\n",
				__func__, pdata->gpio_sda);
			return;
		}
	}
}
static int mdss_samsung_parse_backlight_settings(struct device *dev,struct tuning_table* bl_tune, char *keystring)
{
	const char *data;
	int   len = 0;
	struct device_node *np = dev->of_node;
	data = of_get_property(np, keystring, &len);
	if (!data) {
		pr_info("%s:%d, Unable to read table %s ", __func__, __LINE__, keystring);
		return -EINVAL;
	} else
		pr_err("%s:Success to read table %s\n", __func__, keystring);
	if ((len % 2) != 0) {
		pr_err("%s:%d, Incorrect table entries for %s",
					__func__, __LINE__, keystring);
		return -EINVAL;
	}
	bl_tune->table= kzalloc(sizeof(char) * len, GFP_KERNEL);
	bl_tune->table_length = len;
	if (!bl_tune->table)
		return -ENOMEM;
	memcpy(bl_tune->table, data, len);
	return 0;

}

static int backlight_parse_dt(struct device *dev,
			struct backlight_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	/* reset, irq gpio info */
	pdata->gpio_scl = of_get_named_gpio_flags(np, "backlight,scl-gpio",
				0, &pdata->scl_gpio_flags);
	pdata->gpio_sda = of_get_named_gpio_flags(np, "backlight,sda-gpio",
				0, &pdata->sda_gpio_flags);
	pdata->gpio_backlight_en = of_get_named_gpio_flags(np, "backlight-en-gpio",
				0, &pdata->en_gpio_flags);
	pdata->gpio_backlight_pwm= of_get_named_gpio_flags(np, "backlight-pwm-gpio",
				0, &pdata->en_gpio_flags);



	bl_ic_name = of_get_property(np, "backlight-ic-name", NULL);

	if (!bl_ic_name) {
		pr_info("%s:%d, backlight IC name not specified\n",__func__, __LINE__);

	} else {
		pr_info("%s: Backlight IC Name = %s\n", __func__,bl_ic_name);
	}

	pr_err("%s gpio_scl : %d , gpio_sda : %d en : %d pwm:%d\n", __func__, pdata->gpio_scl, pdata->gpio_sda, pdata->gpio_backlight_en, pdata->gpio_backlight_en);
	return 0;
}


static int backlight_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct backlight_platform_data *pdata;
	struct backlight_info *info;
	int error = 0;
#if defined(CONFIG_BLIC_TUNING)
	struct class *backlight_class;
	struct device *backlight_dev;
#endif
	struct samsung_display_driver_data *vdd = samsung_get_vdd();

	pr_info("%s", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
		pr_err("[BACKLIGHT] failed to check i2c functionality!\n");
		return -EIO;
	}

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct backlight_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_info(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		error = backlight_parse_dt(&client->dev, pdata);
		if (error)
			return error;
	} else
		pdata = client->dev.platform_data;

	backlight_request_gpio(pdata);

	bl_info = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_info(&client->dev, "%s: fail to memory allocation.\n", __func__);
		return -ENOMEM;
	}

	info->client = client;
	info->pdata = pdata;
	mdss_samsung_parse_backlight_settings(&client->dev,&info->bl_ic_settings,"backlight-ic-tuning");
	mdss_samsung_parse_backlight_settings(&client->dev,&info->bl_ic_outdoor_settings,"backlight-ic-tuning-outdoor");
	mdss_samsung_parse_backlight_settings(&client->dev,&info->bl_ic_normal_settings,"backlight-ic-tuning-normal");
	mdss_samsung_parse_backlight_settings(&client->dev,&info->bl_control,"backlight-i2c-bl-control");
#if defined(CONFIG_BLIC_TUNING)
	backlight_class = class_create(THIS_MODULE, "bl-dbg");
	backlight_dev = device_create(backlight_class, NULL, 0, NULL,  "ic-tuning");
	if (IS_ERR(backlight_dev))
		pr_err("Failed to create device(backlight_dev)!\n");
	else {
		if (device_create_file(backlight_dev, &dev_attr_bl_ic_tune) < 0)
			pr_err("Failed to create device file(%s)!\n", dev_attr_bl_ic_tune.attr.name);
		if (device_create_file(backlight_dev, &dev_attr_bl_ic_name) < 0)
			pr_err("Failed to create device file(%s)!\n", dev_attr_bl_ic_name.attr.name);
	}
#endif
	i2c_set_clientdata(client, info);

	vdd->panel_func.samsung_bl_ic_pwm_en = pwm_backlight_control;
	vdd->panel_func.samsung_bl_ic_outdoor = pwm_backlight_outdoor_control;

	return error;
}

static int backlight_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id backlight_id[] = {
	{"backlight_ic", 0},
	{}
};


static struct of_device_id backlight_match_table[] = {
	{ .compatible = "backlight-controller",},
	{ },
};



static struct i2c_driver backlight_driver = {
	.driver = {
		.name = "backlight_ic",
		.of_match_table = backlight_match_table,
		   },
	.id_table = backlight_id,
	.probe = backlight_probe,
	.remove = backlight_remove,
};

static int __init backlight_init(void)
{

	int ret = 0;

	pr_info("%s", __func__);

	ret = i2c_add_driver(&backlight_driver);
	if (ret) {
		printk(KERN_ERR "backlight_init registration failed. ret= %d\n",
			ret);
	}

	return ret;
}

static void __exit backlight_exit(void)
{

	i2c_del_driver(&backlight_driver);
}

module_init(backlight_init);
module_exit(backlight_exit);

MODULE_AUTHOR("rb.bankapur@samsung.com");
MODULE_DESCRIPTION("SKY82896 backlight driver");
MODULE_LICENSE("GPL");
