/*
 * LP8556-backlight.c - Platform data for lp8556 backlight driver
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


struct lp8556_backlight_platform_data {
	unsigned int gpio_backlight_en;
	unsigned int gpio_backlight_pwm;
	u32 en_gpio_flags;

	int gpio_sda;
	u32 sda_gpio_flags;

	int gpio_scl;
	u32 scl_gpio_flags;
};

struct lp8556_backlight_info {
	struct i2c_client	*client;
	struct lp8556_backlight_platform_data	*pdata;
};

struct lp8556_backlight_info *pinfo;

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

static void backlight_request_gpio(struct lp8556_backlight_platform_data *pdata)
{
	int ret;
	if (gpio_is_valid(pdata->gpio_backlight_en)) {
		ret = gpio_request(pdata->gpio_backlight_en, "lp8556_backlight_en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request backlight_en [%d]\n",
				__func__, pdata->gpio_sda);
			return;
		}
	}

}

static int lp8556_backlight_parse_dt(struct device *dev,
			struct lp8556_backlight_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	/* reset, irq gpio info */
	pdata->gpio_scl = of_get_named_gpio_flags(np, "lp8556,scl-gpio",
				0, &pdata->scl_gpio_flags);
	pdata->gpio_sda = of_get_named_gpio_flags(np, "lp8556,sda-gpio",
				0, &pdata->sda_gpio_flags);
	pdata->gpio_backlight_en = of_get_named_gpio_flags(np, "backlight-en-gpio",
				0, &pdata->en_gpio_flags);
	pdata->gpio_backlight_pwm= of_get_named_gpio_flags(np, "backlight-pwm-gpio",
				0, &pdata->en_gpio_flags);

	pr_err("%s gpio_scl : %d , gpio_sda : %d en : %d pwm:%d\n", __func__, pdata->gpio_scl, pdata->gpio_sda, pdata->gpio_backlight_en, pdata->gpio_backlight_en);
	return 0;
}

static u8 channel_setting[][2] ={
	{0x01, 0x85},
	{0x00, 0x00},
};

static void pwm_backlight_control(int enable)
{
	int i;
	struct lp8556_backlight_info *info = pinfo;

	if (!info) {
		pr_info("%s error pinfo", __func__);
		return ;
	}
	pr_info("%s :enable:[%d]\n", __func__,enable);
	if(enable) {
		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,1);
		if (gpio_is_valid(info->pdata->gpio_backlight_pwm))
			gpio_set_value(info->pdata->gpio_backlight_pwm,1);

		msleep(1);
		for (i = 0; i < ARRAY_SIZE(channel_setting) ;i++)
			backlight_i2c_write(info->client, channel_setting[i][0], channel_setting[i][1], 1);
	} else {

		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,0);
		if (gpio_is_valid(info->pdata->gpio_backlight_pwm))
			gpio_set_value(info->pdata->gpio_backlight_pwm,0);
		msleep(1);
	}



}


static u8 channel_backlight_control[][2] ={
	{0x00, 0x00},
};
static void pwm_backlight_control_i2c(int scaled_level)
{
	int i;
	struct lp8556_backlight_info *info = pinfo;
	channel_backlight_control[0][1] =  scaled_level;

	if (!info) {
		pr_info("%s error pinfo", __func__);
		return ;
	}
	pr_info("%s :BL Level :%d\n", __func__,scaled_level);
	for (i = 0; i < ARRAY_SIZE(channel_backlight_control) ;i++)
		backlight_i2c_write(info->client, channel_backlight_control[i][0], channel_backlight_control[i][1], 1);
}

static int lp8556_backlight_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct lp8556_backlight_platform_data *pdata;
	struct lp8556_backlight_info *info;
	int error = 0;
	struct samsung_display_driver_data *vdd = samsung_get_vdd();

	pr_info("%s", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
		pr_err("[BACKLIGHT] failed to check i2c functionality!\n");
		return -EIO;
	}

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct lp8556_backlight_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_info(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		error = lp8556_backlight_parse_dt(&client->dev, pdata);
		if (error)
			return error;
	} else
		pdata = client->dev.platform_data;

	backlight_request_gpio(pdata);

	pinfo = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_info(&client->dev, "%s: fail to memory allocation.\n", __func__);
		return -ENOMEM;
	}

	info->client = client;
	info->pdata = pdata;

	i2c_set_clientdata(client, info);

	vdd->panel_func.samsung_bl_ic_pwm_en = pwm_backlight_control;
	vdd->panel_func.samsung_bl_ic_i2c_ctrl = pwm_backlight_control_i2c;

	return error;
}

static int lp8556_backlight_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id lp8556_backlight_id[] = {
	{"lp8556_backlight", 0},
	{}
};


static struct of_device_id lp8556_backlight_match_table[] = {
	{ .compatible = "lp8556,backlight-control",},
	{ },
};



static struct i2c_driver lp8556_backlight_driver = {
	.driver = {
		.name = "lp8556_backlight",
		.of_match_table = lp8556_backlight_match_table,
		   },
	.id_table = lp8556_backlight_id,
	.probe = lp8556_backlight_probe,
	.remove = lp8556_backlight_remove,
};

static int __init lp8556_backlight_init(void)
{

	int ret = 0;

	pr_info("%s", __func__);

	ret = i2c_add_driver(&lp8556_backlight_driver);
	if (ret) {
		printk(KERN_ERR "lp8556_backlight_init registration failed. ret= %d\n",
			ret);
	}

	return ret;
}

static void __exit lp8556_backlight_exit(void)
{

	i2c_del_driver(&lp8556_backlight_driver);
}

module_init(lp8556_backlight_init);
module_exit(lp8556_backlight_exit);

MODULE_AUTHOR("rb.bankapur@samsung.com");
MODULE_DESCRIPTION("lp8556 backlight driver");
MODULE_LICENSE("GPL");
