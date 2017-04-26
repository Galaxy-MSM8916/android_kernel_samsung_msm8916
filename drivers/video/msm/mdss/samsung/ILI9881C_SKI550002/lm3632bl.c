/*
 * rt8555-backlight.c - Platform data for RT8555 backlight driver
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
#include <mach/board.h>
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
#include <asm/mach-types.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/mfd/lm3632.h>
#include <ss_dsi_panel_ILI9881C_SKI550002.h>

#include "../../mdss.h"
#include "../../mdss_panel.h"
#include "../../mdss_mdp.h"

/*#define CONFIG_BLIC_TUNING 1*/
#define ILI9881C_SKI550002_LCD_ID 0x0bc050
#define S6D7AA0X62_BV050HDM_LCD_ID 0x59b810
extern int get_lcd_attached(char *);

struct lm3632_bl {
	struct device *dev;
	struct backlight_device *bl_dev;
	struct lm3632 *lm3632;
	struct lm3632_backlight_platform_data *pdata;
	struct pwm_device *pwm;
};
struct tuning_table {
	u8* table;
	u8 table_length;
};

struct backlight_info {
	struct i2c_client			*client;
	struct lm3632_backlight_platform_data	*pdata;
	struct tuning_table bl_ic_settings;
	struct tuning_table bl_ic_outdoor_settings;
	struct tuning_table bl_ic_normal_settings;
	struct tuning_table bl_control;
	struct lm3632 *lm3632;
};

static struct backlight_info *bl_info;
static const char *bl_ic_name;

static void mdss_backlight_ic_power_on(int enable)
{
	struct backlight_info *info = bl_info;
	int i;
	struct samsung_display_driver_data *vdd = samsung_get_vdd();
	if (!info) {
		pr_info("%s error bl_info", __func__);
		return ;
	}
	pr_info("%s :enable:[%d]\n", __func__,enable);
	if(enable) {
		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,1);
		mdelay(5);
		for (i = 0; i <info->bl_ic_settings.table_length;i=i+2)
			lm3632_write_byte(info->lm3632, info->bl_ic_settings.table[i], info->bl_ic_settings.table[i+1]);
		/* In case of PBA booting, turn bl_out off */
		if (!mdss_panel_attach_get(vdd->ctrl_dsi[DISPLAY_1]))
			lm3632_write_byte(info->lm3632, 0x0A, 0x00);
		if (gpio_is_valid(info->pdata->gpio_backlight_panel_enp))
			gpio_set_value(info->pdata->gpio_backlight_panel_enp,1);
		mdelay(3);
		if (gpio_is_valid(info->pdata->gpio_backlight_panel_enn))
			gpio_set_value(info->pdata->gpio_backlight_panel_enn,1);
		mdelay(8);
	} else {
		mdelay(1);
		if(get_lcd_attached("GET") == S6D7AA0X62_BV050HDM_LCD_ID)
		{
			if (gpio_is_valid(info->pdata->gpio_backlight_panel_enn))
				gpio_set_value(info->pdata->gpio_backlight_panel_enn,0);
			mdelay(2);
			if (gpio_is_valid(info->pdata->gpio_backlight_panel_enp))
				gpio_set_value(info->pdata->gpio_backlight_panel_enp,0);
		}else{
			if (gpio_is_valid(info->pdata->gpio_backlight_panel_enp))
				gpio_set_value(info->pdata->gpio_backlight_panel_enp,0);
			mdelay(2);
			if (gpio_is_valid(info->pdata->gpio_backlight_panel_enn))
				gpio_set_value(info->pdata->gpio_backlight_panel_enn,0);
		}
		mdelay(1);
		if (gpio_is_valid(info->pdata->gpio_backlight_en))
			gpio_set_value(info->pdata->gpio_backlight_en,0);
		mdelay(1);
	}
}

#if 0
static void pwm_backlight_control_i2c(int scaled_level)
{
	struct backlight_info *info = bl_info;
	int data, ret;
	if (!info) {
		pr_info("%s error pinfo", __func__);
		return ;
	}
	pr_info("%s :BL Level :%d\n", __func__,scaled_level);
	if (!(info->bl_control.table)) {
		pr_info("%s : No backlight configuration :-", __func__);
		return ;
	}

   /* Following taken from lm3632_bl_set_brightness provided by TI for i2c based control*/
	data = scaled_level & LM3632_BRT_LSB_MASK;
	ret = lm3632_update_bits(info->lm3632, info->bl_control.table[0],
				 LM3632_BRT_LSB_MASK, data);
	if (ret)
		return;

	data = (scaled_level >> LM3632_BRT_MSB_SHIFT) & 0xFF;

	lm3632_write_byte(info->lm3632, info->bl_control.table[2], data);
}
#endif
static void pwm_backlight_outdoor_control(int enable)
{
	int i;
	struct backlight_info *info = bl_info;
	if (!info) {
		pr_info("%s error bl_info", __func__);
		return ;
	}
	pr_info("%s :enable:[%d]\n", __func__,enable);
	msleep(1);
	if(enable) {
		for (i = 0; i <info->bl_ic_outdoor_settings.table_length;i=i+2)
			lm3632_write_byte(info->lm3632, info->bl_ic_outdoor_settings.table[i], info->bl_ic_outdoor_settings.table[i+1]);
	} else {
		for (i = 0; i <info->bl_ic_normal_settings.table_length;i=i+2)
			lm3632_write_byte(info->lm3632, info->bl_ic_normal_settings.table[i], info->bl_ic_normal_settings.table[i+1]);
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
int backlight_i2c_read_sys(u8 reg, u8* val)
{
	int err = 0;
	struct backlight_info *info = bl_info;
    err = lm3632_read_byte(info->lm3632, reg, val);
	return err;
}
int backlight_i2c_write_sys(u8 reg,  u8 val)
{
	int err = 0;
	struct backlight_info *info = bl_info;
    err = lm3632_write_byte(info->lm3632, reg, val);
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

static void backlight_request_gpio(struct lm3632_backlight_platform_data *pdata)
{
	int ret;
	if (gpio_is_valid(pdata->gpio_backlight_en)) {
		ret = gpio_request(pdata->gpio_backlight_en, "backlight_en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request backlight_en [%d]\n",
				__func__, pdata->gpio_backlight_en);
			return;
		}
	}
	if (gpio_is_valid(pdata->gpio_backlight_panel_enp)) {
		ret = gpio_request(pdata->gpio_backlight_panel_enp, "backlight_en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_backlight_panel_enp [%d]\n",
				__func__, pdata->gpio_backlight_panel_enp);
			return;
		}
	}
	if (gpio_is_valid(pdata->gpio_backlight_panel_enn)) {
		ret = gpio_request(pdata->gpio_backlight_panel_enn, "backlight_en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_backlight_panel_enn [%d]\n",
				__func__, pdata->gpio_backlight_panel_enn);
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
			struct lm3632_backlight_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->gpio_backlight_en = of_get_named_gpio_flags(np, "backlight-en-gpio",
				0, &pdata->en_gpio_flags);
	pdata->gpio_backlight_panel_enp= of_get_named_gpio_flags(np, "backlight-panel-enp-gpio",
				0, &pdata->panel_enp_gpio_flags);
	pdata->gpio_backlight_panel_enn= of_get_named_gpio_flags(np, "backlight-panel-enn-gpio",
				0, &pdata->panel_enn_gpio_flags);

	bl_ic_name = of_get_property(np, "backlight-ic-name", NULL);
	if (!bl_ic_name) {
		pr_info("%s:%d, backlight IC name not specified\n",__func__, __LINE__);
	} else {
		pr_info("%s: Backlight IC Name = %s\n", __func__,bl_ic_name);
	}

	pr_info("%s gpio en : %d panel_enp:%d panel_enn : %d \n",__func__,
		pdata->gpio_backlight_en,pdata->gpio_backlight_panel_enp,pdata->gpio_backlight_panel_enn);
	return 0;
}

static int lm3632_bl_probe(struct platform_device *pdev)
{
	struct lm3632 *lm3632 = dev_get_drvdata(pdev->dev.parent);
	struct lm3632_backlight_platform_data *pdata = lm3632->pdata->bl_pdata;
	static struct lm3632_bl *lm3632_bl;
	struct backlight_info *info;
	struct samsung_display_driver_data *vdd = samsung_get_vdd();

	int error = 0;
#if defined(CONFIG_BLIC_TUNING)
	struct class *backlight_class;
	struct device *backlight_dev;
#endif
	dev_info(&pdev->dev, "SKY panel %s", __func__);

	lm3632_bl = devm_kzalloc(&pdev->dev, sizeof(*lm3632_bl), GFP_KERNEL);
	if (!lm3632_bl)
		return -ENOMEM;

	lm3632_bl->dev = &pdev->dev;
	lm3632_bl->lm3632 = lm3632;
	lm3632_bl->pdata = pdata;

	if (!lm3632_bl->pdata) {
		pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct lm3632_backlight_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_info(&pdev->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}
		if (IS_ENABLED(CONFIG_OF))
			error = backlight_parse_dt(&pdev->dev, pdata);
		else
			return -ENODEV;

		if (error)
			return error;
	}

	backlight_request_gpio(pdata);

	bl_info = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_info(&pdev->dev, "%s: fail to memory allocation.\n", __func__);
		return -ENOMEM;
	}

	info->pdata = pdata;
	info->lm3632 = lm3632;
	mdss_samsung_parse_backlight_settings(&pdev->dev,&info->bl_ic_settings,"backlight-ic-tuning");
	mdss_samsung_parse_backlight_settings(&pdev->dev,&info->bl_ic_outdoor_settings,"backlight-ic-tuning-outdoor");
	mdss_samsung_parse_backlight_settings(&pdev->dev,&info->bl_ic_normal_settings,"backlight-ic-tuning-normal");
	mdss_samsung_parse_backlight_settings(&pdev->dev,&info->bl_control,"backlight-i2c-bl-control");
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
	  vdd->panel_func.samsung_backlight_ic_power_on = mdss_backlight_ic_power_on;
	  vdd->panel_func.samsung_bl_ic_outdoor = pwm_backlight_outdoor_control;


      return error;
}

static int lm3632_bl_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id lm3632_bl_of_match[] = {
	{ .compatible = "ti,lm3632-backlight-sky", },
	{ }
};
MODULE_DEVICE_TABLE(of, lm3632_bl_of_match);
#endif

static struct platform_driver lm3632_bl_driver = {
	.probe = lm3632_bl_probe,
	.remove = lm3632_bl_remove,
	.driver = {
		.name = "lm3632-backlight-sky",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lm3632_bl_of_match),
	},
};
module_platform_driver(lm3632_bl_driver);

MODULE_DESCRIPTION("TI LM3632 Backlight Driver");
MODULE_AUTHOR("Milo Kim");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:lm3632-backlight");

