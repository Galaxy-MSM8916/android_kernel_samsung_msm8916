/*
 * TI LM3632 MFD Driver
 *
 * Copyright 2015 Texas Instruments
 *
 * Author: Milo Kim <milo.kim@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/lm3632.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>

#define LM3632_DEV_BL				\
{						\
	.name = "lm3632-backlight",		\
	.of_compatible = "ti,lm3632-backlight",	\
}

#define LM3632_DEV_BL_SKY				\
{						\
	.name = "lm3632-backlight-sky",		\
	.of_compatible = "ti,lm3632-backlight-sky",	\
}

#define LM3632_DEV_FLASH				\
{						\
	.name = "lm3632-flash",		\
	.of_compatible = "ti,lm3632-flash",	\
}

static struct mfd_cell lm3632_devs[] = {
	/* Backlight */
	LM3632_DEV_BL,
	/* Backlight for SKY panel */
	LM3632_DEV_BL_SKY,
	/* Torch Flash */
	LM3632_DEV_FLASH,
};

struct lm3632 *lm3632_global;

static int lm3632_i2c_write(struct i2c_client *client,
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

static int lm3632_i2c_read(struct i2c_client *client,
		u8 reg,  u8* val, unsigned int len)
{
	int err = 0;
	int retry = 3;

	while (retry--) {
		err = i2c_smbus_read_i2c_block_data(client,
				reg, len, val);
		if (err >= 0)
			return err;
		dev_info(&client->dev, "%s:i2c transfer error.\n", __func__);
	}
	return err;
}

int lm3632_read_byte(struct lm3632 *lm3632, u8 reg, u8 *read)
{
	int ret;
	u8 val = 0;

	mutex_lock(&lm3632->lm3632_lock);
	ret = lm3632_i2c_read(lm3632->client, reg, &val, 1);
	mutex_unlock(&lm3632->lm3632_lock);
	if (ret < 0)
		return ret;

	*read = val;
	return 0;
}
EXPORT_SYMBOL_GPL(lm3632_read_byte);

int lm3632_write_byte(struct lm3632 *lm3632, u8 reg, u8 data)
{
      int ret;
      mutex_lock(&lm3632->lm3632_lock);
      ret = lm3632_i2c_write(lm3632->client, reg, data, 1);
      mutex_unlock(&lm3632->lm3632_lock);
      return ret;
}
EXPORT_SYMBOL_GPL(lm3632_write_byte);

int lm3632_update_bits(struct lm3632 *lm3632, u8 reg, u8 mask, u8 data)
{
	int ret;
       u8 tmp, orig=0;

       mutex_lock(&lm3632->lm3632_lock);
       ret = lm3632_i2c_read(lm3632->client, reg, &orig, 1);
       if (ret < 0){
		mutex_unlock(&lm3632->lm3632_lock);
		return ret;
	}

      tmp = orig & ~mask;
      tmp |= data & mask;

      if (tmp != orig) {
              ret = lm3632_i2c_write(lm3632->client, reg, tmp, 1);
        }

      mutex_unlock(&lm3632->lm3632_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(lm3632_update_bits);

static int lm3632_init_device(struct lm3632 *lm3632)
{
   if(gpio_is_valid(lm3632->pdata->en_gpio)){
	return devm_gpio_request_one(lm3632->dev, lm3632->pdata->en_gpio,
				     GPIOF_OUT_INIT_HIGH, "lm3632_hwen");
	}
   return 0;
}

static void lm3632_deinit_device(struct lm3632 *lm3632)
{
   if(gpio_is_valid(lm3632->pdata->en_gpio)){
	gpio_set_value(lm3632->pdata->en_gpio, 0);
      }
   return;
}

static int lm3632_parse_dt(struct device *dev, struct lm3632 *lm3632)
{
	struct device_node *node = dev->of_node;
	struct lm3632_platform_data *pdata;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->en_gpio = of_get_named_gpio(node, "ti,en-gpio", 0);
	lm3632->pdata = pdata;

	return 0;
}

static int lm3632_probe(struct i2c_client *cl, const struct i2c_device_id *id)
{
	struct lm3632 *lm3632;
	struct device *dev = &cl->dev;
	struct lm3632_platform_data *pdata = dev_get_platdata(dev);
	int ret;

	lm3632 = devm_kzalloc(dev, sizeof(struct lm3632), GFP_KERNEL);
	if (!lm3632)
		return -ENOMEM;

      mutex_init(&lm3632->lm3632_lock);

	lm3632->pdata = pdata;
	lm3632->client = cl;
	if (!pdata) {
		if (IS_ENABLED(CONFIG_OF))
			ret = lm3632_parse_dt(dev, lm3632);
		else
			ret = -ENODEV;

		if (ret)
			return ret;
	}

	lm3632->dev = &cl->dev;
	i2c_set_clientdata(cl, lm3632);

	ret = lm3632_init_device(lm3632);
	if (ret)
		return ret;

	pr_info("%s... end\n", __func__);

	return mfd_add_devices(dev, -1, lm3632_devs, ARRAY_SIZE(lm3632_devs),
			       NULL, 0, NULL);
}

static int lm3632_remove(struct i2c_client *cl)
{
	struct lm3632 *lm3632 = i2c_get_clientdata(cl);

	lm3632_deinit_device(lm3632);
	mfd_remove_devices(lm3632->dev);
       mutex_destroy(&lm3632->lm3632_lock);
	return 0;
}

static const struct i2c_device_id lm3632_ids[] = {
	{ "lm3632", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lm3632_ids);

#ifdef CONFIG_OF
static const struct of_device_id lm3632_of_match[] = {
	{ .compatible = "ti,lm3632", },
	{ }
};
MODULE_DEVICE_TABLE(of, lm3632_of_match);
#endif

static struct i2c_driver lm3632_driver = {
	.probe = lm3632_probe,
	.remove = lm3632_remove,
	.driver = {
		.name = "lm3632",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lm3632_of_match),
	},
	.id_table = lm3632_ids,
};
module_i2c_driver(lm3632_driver);

MODULE_DESCRIPTION("TI LM3632 MFD Core");
MODULE_AUTHOR("Milo Kim");
MODULE_LICENSE("GPL");
