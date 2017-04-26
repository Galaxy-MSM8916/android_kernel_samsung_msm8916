/*
 * sm5701.c
 *
 * Copyright (c) 2014 Siliconmitus Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/mfd/sm5701_core.h>

#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif /* CONFIG_OF */

/* Slave address should be shifted to the right 1bit.
 * R/W bit should NOT be included.
 */
#define SEC_CHARGER_I2C_SLAVEADDR	(0x92>>1)

static struct mfd_cell SM5701_devs[] = {
	{
		.name = "sm5701-charger",
#ifdef CONFIG_OF
		.of_compatible = "sm,sm5701-charger"
#endif
	},
	{
        .name = "leds_sm5701",
#ifdef CONFIG_OF
        .of_compatible = "sm,leds_sm5701"
#endif
	},
};

static struct i2c_client *SM5701_core_client = NULL;

static int led_ready_state = 0;
int led_state_charger = LED_DISABLE;

int SM5701_reg_read(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&SM5701->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&SM5701->i2c_lock);
	if (ret < 0) {
		pr_info("%s:%s reg(0x%x), ret(%d)\n", MFD_DEV_NAME, __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*dest = ret;
	return 0;
}
EXPORT_SYMBOL_GPL(SM5701_reg_read);

int SM5701_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&SM5701->i2c_lock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&SM5701->i2c_lock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(SM5701_bulk_read);

int SM5701_reg_write(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&SM5701->i2c_lock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	mutex_unlock(&SM5701->i2c_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(SM5701_reg_write);

int SM5701_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&SM5701->i2c_lock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&SM5701->i2c_lock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(SM5701_bulk_write);

int SM5701_reg_update(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&SM5701->i2c_lock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		u8 old_val = ret & 0xff;
		u8 new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&SM5701->i2c_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(SM5701_reg_update);

void SM5701_test_read(struct i2c_client *client)
{
	u8 data = 0,reg = SM5701_INTMASK1;
	char str[1000] = {0,};

	for (reg = SM5701_INTMASK1; reg <= SM5701_FLEDCNTL6; reg++) {
		SM5701_reg_read(client, reg, &data);
		sprintf(str+strlen(str), "0x%x = 0x%02x, ", reg, data);
	}
	pr_info("%s: %s\n", __func__, str);
}
EXPORT_SYMBOL_GPL(SM5701_test_read);

void SM5701_set_operationmode(int operation_mode)
{
	struct i2c_client * client;
	u8 data = 0;

	client = SM5701_core_client;

	if(!client) return;

	pr_info("%s operation_mode = %d\n",__func__,operation_mode);

	SM5701_reg_read(client, SM5701_CNTL, &data);

	data = (data & (~SM5701_CNTL_OPERATIONMODE)) | operation_mode;

	SM5701_reg_write(client,SM5701_CNTL,data);

	pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_CNTL, data);
}
EXPORT_SYMBOL_GPL(SM5701_set_operationmode);

/* core config for SM5701*/
static ssize_t SM5701_core_store(struct device *dev,
										struct device_attribute *attr,
										const char *buf, size_t size)
{
	ssize_t ret;
	int state = 0;

	ret = kstrtouint(buf, 10, &state);
	if (ret)
	        goto out_strtoint;

	if (state == 0)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_SUSPEND);
	else if (state == 1)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
	else if (state == 2)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_OTG_ON);
	else if (state == 3)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_OTG_ON_FLASH_ON);
	else if (state == 4)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);
	else if (state == 5)
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON_FLASH_ON);
        else
	        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);

	//SM5701_dump_register();

	return size;

out_strtoint:
	dev_err(dev, "%s: fail to change str to int\n", __func__);
	return ret;
}

void sm5701_led_ready(int led_status)
{
	struct i2c_client * client;
	u8 data = 0;

	client = SM5701_core_client;

	if(!client) return;

	SM5701_reg_read(client, SM5701_DEVICE_ID, &data);

	printk("sm5701 device id =%d, led_status=%d\n", data, led_status);

    printk("%s led_status = %d\n",__func__,led_status);
    // led_status == 0 : LED_DISABLE
    // led_status == 1 : LED_FLASH
    // led_status == 2 : LED_MOVIE
	led_ready_state = led_status;
	/* SM5701 charger */
	led_state_charger = led_status;

    SM5701_operation_mode_function_control();
}
EXPORT_SYMBOL(sm5701_led_ready);

int SM5701_operation_mode_function_control(void)
{
        struct i2c_client * client;
        struct SM5701_platform_data *pdata;

        client = SM5701_core_client;
        pdata = client->dev.platform_data;
		//printk("SM5701_operation_mode_function_control\n");
		//printk("client:%d\n", client);
		//printk("client->dev:%d\n", client->dev);
		//printk("pdata = client->dev.platform_data:%d\n", client->dev.platform_data);
		//printk("pdata->charger_data:%d\n", pdata->charger_data);
		//printk("pdata->charger_data->cable_type:%d\n", pdata->charger_data->cable_type);

        if ((pdata->charger_data->cable_type == POWER_SUPPLY_TYPE_MAINS) ||
                        (pdata->charger_data->cable_type == POWER_SUPPLY_TYPE_USB))
        {
                if (led_ready_state == LED_FLASH)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
                }
                else if (led_ready_state == LED_MOVIE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
						if (pdata->charger_data->dev_id == 4)
							SM5701_set_operationmode(
								SM5701_OPERATIONMODE_FLASH_ON);
						else
							SM5701_set_operationmode(
								SM5701_OPERATIONMODE_CHARGER_ON_FLASH_ON);
                }
                else if (led_ready_state == LED_DISABLE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);
                }
                else
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);
                }

        }
        else if ((pdata->charger_data->cable_type == POWER_SUPPLY_TYPE_BATTERY) ||
                        (pdata->charger_data->cable_type == POWER_SUPPLY_TYPE_UNKNOWN))
        {
                if (led_ready_state == LED_FLASH)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
                }
                else if (led_ready_state == LED_MOVIE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
                }
                else if (led_ready_state == LED_DISABLE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);
                }  
                else
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);
                }     
        }
        else if (pdata->charger_data->cable_type == POWER_SUPPLY_TYPE_OTG)
        {
                if (led_ready_state == LED_FLASH)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_4P5);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
                }
                else if (led_ready_state == LED_MOVIE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_5P0);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_FLASH_ON);
                }
                else if (led_ready_state == LED_DISABLE)
                {
                        SM5701_set_bstout(SM5701_BSTOUT_5P0);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_OTG_ON);
                }  
                else
                {
                        SM5701_set_bstout(SM5701_BSTOUT_5P0);
                        SM5701_set_operationmode(SM5701_OPERATIONMODE_OTG_ON);
                }    
        }
        else
        {
                SM5701_set_bstout(SM5701_BSTOUT_4P5);
                SM5701_set_operationmode(SM5701_OPERATIONMODE_CHARGER_ON);

        }

        return 0;
}
EXPORT_SYMBOL(SM5701_operation_mode_function_control);


static DEVICE_ATTR(SM5701_core, S_IWUSR, NULL, SM5701_core_store);

void SM5701_set_charger_data(void *p)
{
        struct SM5701_platform_data *pdata = SM5701_core_client->dev.platform_data;
        pdata->charger_data = p;
}

static int SM5701_parse_dt(struct SM5701_platform_data *pdata)
{
	int ret = 0;
	struct device_node *np = of_find_node_by_name(NULL, "chargermfd");
	struct device_node *np2 = of_find_node_by_name(np, "sm5701_charger");

	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
	} else {
		pdata->irq = of_get_named_gpio(np, "chgirq-gpio", 0);
		pdata->chgen = of_get_named_gpio(np2, "chgen-gpio", 0);
		pr_info("%s: SM5701_parse_dt irq: %d\n", __func__, pdata->irq);
		pr_info("%s: SM5701_parse_dt chgen: %d\n", __func__, pdata->chgen);
		if (pdata->irq < 0) {
			pr_err("%s: of_get_named_gpio failed: %d\n",
					__func__, pdata->irq);
			return pdata->irq;
		}
	}
	return ret;
}

static int SM5701_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct device_node *of_node = i2c->dev.of_node;
	struct SM5701_platform_data *pdata = i2c->dev.platform_data;
	struct SM5701_dev *SM5701;
	int ret = 0;

	pr_info("%s: SM5701 MFD Probe Start !!!!!\n", __func__);
	if (of_node) {
		pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata) {
			dev_err(&i2c->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_dt_nomem;
		}
		ret = SM5701_parse_dt(pdata);
		if (ret < 0)
			goto err_parse_dt;
		i2c->dev.platform_data = pdata;
	} else
		pdata = i2c->dev.platform_data;

	SM5701 = kzalloc(sizeof(struct SM5701_dev), GFP_KERNEL);
	if (!SM5701) {
		dev_err(&i2c->dev, "Failed to allocate memory for SM5701 \n");
		ret = -ENOMEM;
		goto err_parse_dt;
	}

	i2c_set_clientdata(i2c, SM5701);

	SM5701->dev = &i2c->dev;
	SM5701->i2c = i2c;
	SM5701->irq = i2c->irq;

    SM5701_core_client = i2c;

	if (!pdata)
		goto err_pdata_null;

	mutex_init(&SM5701->i2c_lock);

//	ret = SM5701_irq_init(SM5701);
//	if (ret < 0)
//		goto err_irq_init;

	ret = mfd_add_devices(SM5701->dev, -1,
			SM5701_devs, ARRAY_SIZE(SM5701_devs),
			NULL, 0, NULL);

	if (ret < 0)
		goto err;

	ret = device_create_file(SM5701->dev, &dev_attr_SM5701_core);
	if (ret < 0) {
		dev_err(SM5701->dev, "failed to create flash file\n");
		goto err_create_core_file;
	}

	dev_info(SM5701->dev ,"SM5701 MFD probe done!!! \n");
	return ret;

err:
	pr_info("%s: err error \n", __func__);
	mfd_remove_devices(SM5701->dev);
//err_irq_init:
//	SM5701_irq_exit(SM5701);

err_create_core_file:
	device_remove_file(SM5701->dev, &dev_attr_SM5701_core);
err_pdata_null:
	 i2c_set_clientdata(i2c, NULL);
	 kfree(SM5701);
err_parse_dt:
	if (of_node) {
		devm_kfree(&i2c->dev, pdata);
	}
	pr_info("%s: parse_dt error \n", __func__);

err_dt_nomem:
	pr_info("%s: dt_nomem error \n", __func__);
	return ret;
}

static int SM5701_i2c_remove(struct i2c_client *i2c)
{
	struct SM5701_dev *SM5701 = i2c_get_clientdata(i2c);

	mfd_remove_devices(SM5701->dev);
//	SM5701_irq_exit(SM5701);
	kfree(SM5701);

	return 0;
}

static const struct i2c_device_id SM5701_i2c_id[] = {
	{ "sm5701-i2c", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, SM5701_i2c_id);

#ifdef CONFIG_OF
static struct of_device_id SM5701_match_table[] = {
	{ .compatible = "sm,sm5701",},
	{},
};
#else
#define SM5701_match_table NULL
#endif

static struct i2c_driver SM5701_i2c_driver = {
	.driver = {
		.name = "sm5701-i2c",
		.owner = THIS_MODULE,
		.of_match_table = SM5701_match_table,
	},
	.probe = SM5701_i2c_probe,
	.remove = SM5701_i2c_remove,
	.id_table = SM5701_i2c_id,
};

static int __init SM5701_i2c_init(void)
{
	pr_info("%s:%s\n", MFD_DEV_NAME, __func__);
	return i2c_add_driver(&SM5701_i2c_driver);
}

subsys_initcall(SM5701_i2c_init);

static void __exit SM5701_i2c_exit(void)
{
	i2c_del_driver(&SM5701_i2c_driver);
}
module_exit(SM5701_i2c_exit);

MODULE_DESCRIPTION("Core support for the SM5701 MFD");
MODULE_LICENSE("GPL");
