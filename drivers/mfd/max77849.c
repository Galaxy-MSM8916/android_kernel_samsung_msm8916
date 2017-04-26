/*
 * max77849.c - mfd core driver for the Maxim 77849
 *
 * Copyright (C) 2011 Samsung Electronics
 * SangYoung Son <hello.son@smasung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This driver is based on max8997.c
 */

#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/mfd/core.h>
#include <linux/mfd/max77849.h>
#include <linux/mfd/max77849-private.h>
#include <linux/regulator/machine.h>
#include <linux/delay.h>
#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif

#define I2C_ADDR_PMIC	(0xCC >> 1)	/* Charger, Flash LED */
#define I2C_ADDR_MUIC	(0x4A >> 1)
#define I2C_ADDR_HAPTIC	(0x90 >> 1)

#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
int muic_reset_pin = 0;
EXPORT_SYMBOL_GPL(muic_reset_pin);
#endif

static struct mfd_cell max77849_devs[] = {
	{ .name = "max77849-charger", },
	{ .name = "max77849-muic", },
	{ .name = "max77849-safeout", },
};

#if defined(CONFIG_EXTCON)
struct max77849_muic_data max77849_muic = {
	.usb_sel = 0,
	.uart_sel = 0,
};
#endif
int max77849_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77849->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&max77849->iolock);
	if (ret < 0)
		return ret;

	ret &= 0xff;
	*dest = ret;
	return 0;
}
EXPORT_SYMBOL_GPL(max77849_read_reg);

int max77849_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77849->iolock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77849->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77849_bulk_read);

int max77849_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77849->iolock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	mutex_unlock(&max77849->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77849_write_reg);

int max77849_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77849->iolock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77849->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77849_bulk_write);

int max77849_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77849->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		u8 old_val = ret & 0xff;
		u8 new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&max77849->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77849_update_reg);

static int of_max77849_dt(struct device *dev, struct max77849_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	if(!np) {
		return -EINVAL;
	}

	pdata->irq_gpio = of_get_named_gpio(np, "max77849,irq-gpio", 0);
	if (pdata->irq_gpio < 0) {
		pr_err("%s: failed get max77849 irq-gpio : %d\n",
			__func__, pdata->irq_gpio);
		pdata->irq_gpio = 0;
	}
	pdata->irq_base = irq_alloc_descs(-1, 0, MAX77849_IRQ_NR, -1);
	if (pdata->irq_base < 0) {
		pr_info("%s irq_alloc_descs is failed! irq_base:%d\n", __func__, pdata->irq_base);
		/* getting a predefined irq_base on dt file	*/
		of_property_read_u32(np, "max77849,irq-base", &pdata->irq_base);
	}

#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
	pdata->irq_reset_gpio = of_get_named_gpio(np, "max77849,irq-reset-gpio", 0);
	if (pdata->irq_reset_gpio < 0) {
		pr_err("%s: failed get max77849 irq-reset-gpio : %d\n",
			__func__, pdata->irq_gpio);
		pdata->irq_reset_gpio = -1;
		muic_reset_pin = 0;
	}
	else
		muic_reset_pin = 1;

#endif

	pr_info("%s: irq-gpio: %u \n", __func__, pdata->irq_gpio);
#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
	pr_info("%s: irq-reset-gpio: %u \n", __func__, pdata->irq_reset_gpio);
#endif
	return 0;
}

static int max77849_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	struct max77849_dev *max77849;
	struct max77849_platform_data *pdata;
	int ret = 0;
	struct pinctrl *muic_pinctrl;
	dev_info(&i2c->dev, "%s\n", __func__);

	max77849 = kzalloc(sizeof(struct max77849_dev), GFP_KERNEL);
	if (max77849 == NULL)
		return -ENOMEM;

	if (i2c->dev.of_node) {
		pdata = devm_kzalloc(&i2c->dev,
				sizeof(struct max77849_platform_data),
				GFP_KERNEL);

		if (!pdata) {
			dev_err(&i2c->dev, "Failed to allocate memory \n");
			ret = -ENOMEM;
			goto err;
		}

		ret = of_max77849_dt(&i2c->dev, pdata);
		if (ret < 0){
			dev_err(&i2c->dev, "Failed to get device of_node \n");
			return ret;
		}

		/*Filling the platform data*/
		pdata->muic_data = &max77849_muic;
#if defined(CONFIG_REGULATOR_MAX77849)
		pdata->num_regulators = MAX77849_REG_MAX;
		pdata->regulators = max77849_regulators,
#endif
		/*pdata update to other modules*/
		i2c->dev.platform_data = pdata;
		muic_pinctrl = devm_pinctrl_get_select(&i2c->dev,
			"muic_i2c_active");
		if (IS_ERR(muic_pinctrl)) {
			if (PTR_ERR(muic_pinctrl) == -EPROBE_DEFER)
				return -EPROBE_DEFER;

			pr_debug("Target does not use pinctrl\n");
			muic_pinctrl = NULL;
		}
	} else {
		pdata = i2c->dev.platform_data;
	}

	i2c_set_clientdata(i2c, max77849);
	max77849->dev = &i2c->dev;
	max77849->i2c = i2c;
	max77849->irq = i2c->irq;
	if (pdata) {
		max77849->irq_base = pdata->irq_base;
		max77849->irq_gpio = pdata->irq_gpio;
#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
		if (muic_reset_pin)
		{
			max77849->irq_reset_gpio = pdata->irq_reset_gpio;
			gpio_tlmm_config(GPIO_CFG(max77849->irq_reset_gpio,  0, GPIO_CFG_INPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_DISABLE);
		}
#endif

	} else {
		goto err;
	}

	mutex_init(&max77849->iolock);

	/* No active discharge on safeout ldo 1,2 */
	max77849_update_reg(i2c, MAX77849_CHG_REG_SAFEOUT_CTRL, 0x00, 0x30);
	pr_info("%s: i2c->name=%s irq=%d   !!!\n",__func__, i2c->name, i2c->irq);

	max77849->muic = i2c_new_dummy(i2c->adapter, I2C_ADDR_MUIC);
	i2c_set_clientdata(max77849->muic, max77849);

	ret = max77849_irq_init(max77849);
	if (ret < 0)
		goto err_irq_init;

	ret = mfd_add_devices(max77849->dev, -1, max77849_devs,
			ARRAY_SIZE(max77849_devs), NULL, 0, NULL);
	if (ret < 0)
		goto err_mfd;

	device_init_wakeup(max77849->dev, 1);

#if defined(CONFIG_ADC_ONESHOT)
	/* Set oneshot mode */
	max77849_update_reg(max77849->muic, MAX77849_MUIC_REG_CTRL4,
			ADC_ONESHOT<<CTRL4_ADCMODE_SHIFT, CTRL4_ADCMODE_MASK);
#else
	/* Set continuous mode */
	max77849_update_reg(max77849->muic, MAX77849_MUIC_REG_CTRL4,
			ADC_ALWAYS<<CTRL4_ADCMODE_SHIFT, CTRL4_ADCMODE_MASK);
#endif
	return ret;

err_mfd:
	mfd_remove_devices(max77849->dev);
	max77849_irq_exit(max77849);
	pr_info("%s1\n", __func__);
err_irq_init:
	i2c_unregister_device(max77849->muic);
	pr_info("%s2\n", __func__);
err:
	kfree(max77849);
	pr_info("%s3\n", __func__);
	return ret;
}

static int max77849_i2c_remove(struct i2c_client *i2c)
{
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);

	mfd_remove_devices(max77849->dev);
	i2c_unregister_device(max77849->muic);
	kfree(max77849);

	return 0;
}

static const struct i2c_device_id max77849_i2c_id[] = {
	{ "max77849", TYPE_MAX77849 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max77849_i2c_id);
static struct of_device_id max77849_i2c_match_table[] = {
    { .compatible = "max77849,i2c", },
    { },
};
MODULE_DEVICE_TABLE(of, max77849_i2c_match_table);


#ifdef CONFIG_PM
static int max77849_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		enable_irq_wake(max77849->irq);

	return 0;
}

static int max77849_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		disable_irq_wake(max77849->irq);

	return max77849_irq_resume(max77849);
}
#else
#define max77849_suspend	NULL
#define max77849_resume		NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_HIBERNATION

u8 max77849_dumpaddr_pmic[] = {
	MAX77849_PMIC_REG_TOPSYS_INT_MASK,
	MAX77849_PMIC_REG_MAINCTRL1,
	MAX77849_PMIC_REG_LSCNFG,
	MAX77849_CHG_REG_CHG_INT_MASK,
	MAX77849_CHG_REG_CHG_CNFG_00,
	MAX77849_CHG_REG_CHG_CNFG_01,
	MAX77849_CHG_REG_CHG_CNFG_02,
	MAX77849_CHG_REG_CHG_CNFG_03,
	MAX77849_CHG_REG_CHG_CNFG_04,
	MAX77849_CHG_REG_CHG_CNFG_05,
	MAX77849_CHG_REG_CHG_CNFG_06,
	MAX77849_CHG_REG_CHG_CNFG_07,
	MAX77849_CHG_REG_CHG_CNFG_08,
	MAX77849_CHG_REG_CHG_CNFG_09,
	MAX77849_CHG_REG_CHG_CNFG_10,
	MAX77849_CHG_REG_CHG_CNFG_11,
	MAX77849_CHG_REG_CHG_CNFG_12,
	MAX77849_CHG_REG_CHG_CNFG_13,
	MAX77849_CHG_REG_CHG_CNFG_14,
	MAX77849_CHG_REG_SAFEOUT_CTRL,
};

u8 max77849_dumpaddr_muic[] = {
	MAX77849_MUIC_REG_INTMASK1,
	MAX77849_MUIC_REG_INTMASK2,
	MAX77849_MUIC_REG_INTMASK3,
	MAX77849_MUIC_REG_CDETCTRL1,
	MAX77849_MUIC_REG_CDETCTRL2,
	MAX77849_MUIC_REG_CTRL1,
	MAX77849_MUIC_REG_CTRL2,
	MAX77849_MUIC_REG_CTRL3,
};

static int max77849_freeze(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int i;

	for (i = 0; i < ARRAY_SIZE(max77849_dumpaddr_pmic); i++)
		max77849_read_reg(i2c, max77849_dumpaddr_pmic[i],
				&max77849->reg_pmic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77849_dumpaddr_muic); i++)
		max77849_read_reg(i2c, max77849_dumpaddr_muic[i],
				&max77849->reg_muic_dump[i]);

	disable_irq(max77849->irq);
#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
	if (muic_reset_pin)
		disable_irq(max77849->irq_reset);
#endif
	return 0;
}

static int max77849_restore(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77849_dev *max77849 = i2c_get_clientdata(i2c);
	int i;

	enable_irq(max77849->irq);
#ifdef CONFIG_MUIC_RESET_PIN_ENABLE
	if (muic_reset_pin)
		enable_irq(max77849->irq_reset);
#endif
	for (i = 0; i < ARRAY_SIZE(max77849_dumpaddr_pmic); i++)
		max77849_write_reg(i2c, max77849_dumpaddr_pmic[i],
				max77849->reg_pmic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77849_dumpaddr_muic); i++)
		max77849_write_reg(i2c, max77849_dumpaddr_muic[i],
				max77849->reg_muic_dump[i]);

	return 0;
}
#endif


const struct dev_pm_ops max77849_pm = {
	.suspend = max77849_suspend,
	.resume = max77849_resume,
#ifdef CONFIG_HIBERNATION
	.freeze =  max77849_freeze,
	.thaw = max77849_restore,
	.restore = max77849_restore,
#endif
};

static struct i2c_driver max77849_i2c_driver = {
	.probe = max77849_i2c_probe,
	.remove = max77849_i2c_remove,
	.driver = {
		.name = "max77849",
		.owner = THIS_MODULE,
	        .of_match_table = max77849_i2c_match_table,
		.pm = &max77849_pm,
	},
	.id_table = max77849_i2c_id,
};

static int __init max77849_i2c_init(void)
{
	int data;
	pr_info("%s: START\n", __func__);
	data =  i2c_add_driver(&max77849_i2c_driver);
	return data;
}
/* init early so consumer devices can complete system boot */
subsys_initcall(max77849_i2c_init);

static void __exit max77849_i2c_exit(void)
{
	i2c_del_driver(&max77849_i2c_driver);
}
module_exit(max77849_i2c_exit);

MODULE_DESCRIPTION("MAXIM 77849 multi-function core driver");
MODULE_AUTHOR("SangYoung, Son <hello.son@samsung.com>");
MODULE_LICENSE("GPL");
