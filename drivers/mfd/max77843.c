/*
 * max77843.c - mfd core driver for the Maxim 77843
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

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/mfd/max77843.h>
#include <linux/mfd/max77843-private.h>
#include <linux/regulator/machine.h>

//#include <mach/sec_debug.h>
#include <linux/mfd/pm8xxx/misc.h>
#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif
#define I2C_ADDR_PMIC	(0xCC >> 1)	/* Top sys, Haptic */
#define I2C_ADDR_MUIC	(0x4A >> 1)
#define I2C_ADDR_LED	(0x94 >> 1)
#define I2C_ADDR_CHG    (0xD2 >> 1)
#define I2C_ADDR_FG     (0x6C >> 1)

static struct mfd_cell max77843_devs[] = {
	{ .name = "max77843-fuelgauge", },
	{ .name = "max77843-charger", },
	{ .name = "max77843-muic", },
	{ .name = "max77843-haptic", },
#if defined(CONFIG_LEDS_MAX77843_RGB)
	{ .name = "leds-max77843-rgb", },
#endif
#ifdef CONFIG_LEDS_MAX77843
	{ .name = "max77843-led", },
#endif
};

#if defined(CONFIG_EXTCON)
	struct max77843_muic_data max77843_muic = {
		.usb_sel = 0,
		.uart_sel = 0,
};
#endif

int max77843_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&max77843->iolock);
	if (ret < 0) {
		dev_err(max77843->dev,
			"%s, reg(0x%x), ret(%d)\n", __func__, reg, ret);
		return ret;
	}

	ret &= 0xff;
	*dest = ret;
	return 0;
}
EXPORT_SYMBOL_GPL(max77843_read_reg);

int max77843_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77843->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77843_bulk_read);

int max77843_read_word(struct i2c_client *i2c, u8 reg)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_read_word_data(i2c, reg);
	mutex_unlock(&max77843->iolock);
	if (ret < 0)
		return ret;

	return ret;
}
EXPORT_SYMBOL_GPL(max77843_read_word);

int max77843_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	if (ret < 0)
		dev_err(max77843->dev,
			"%s, reg(0x%x), ret(%d)\n", __func__, reg, ret);
	mutex_unlock(&max77843->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77843_write_reg);

int max77843_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77843->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77843_bulk_write);

int max77843_write_word(struct i2c_client *i2c, u8 reg, u16 value)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_write_word_data(i2c, reg, value);
	mutex_unlock(&max77843->iolock);
	if (ret < 0)
		return ret;
	return 0;
}
EXPORT_SYMBOL_GPL(max77843_write_word);

int max77843_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77843->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		u8 old_val = ret & 0xff;
		u8 new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&max77843->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77843_update_reg);

static int max77843_i2c_pinctrl_init(struct max77843_dev *max77843)
{
	int retval;

	/* Get pinctrl if target uses pinctrl */
	max77843->max_pinctrl = devm_pinctrl_get(max77843->dev);
	if (IS_ERR_OR_NULL(max77843->max_pinctrl)) {
		dev_dbg(max77843->dev,
			"Target does not use pinctrl\n");
		retval = PTR_ERR(max77843->max_pinctrl);
		max77843->max_pinctrl = NULL;
		return retval;
	}

	max77843->gpio_state_active
		= pinctrl_lookup_state(max77843->max_pinctrl, "max77843_interrupt_pins_active");
	if (IS_ERR_OR_NULL(max77843->gpio_state_active)) {
		dev_dbg(max77843->dev,
			"Can not get ts default pinstate\n");
		retval = PTR_ERR(max77843->gpio_state_active);
		max77843->max_pinctrl = NULL;
		return retval;
	}

	max77843->gpio_state_suspend
		= pinctrl_lookup_state(max77843->max_pinctrl, "max77843_interrupt_pins_suspend");
	if (IS_ERR_OR_NULL(max77843->gpio_state_suspend)) {
		dev_dbg(max77843->dev,
			"Can not get ts sleep pinstate\n");
		retval = PTR_ERR(max77843->gpio_state_suspend);
		max77843->max_pinctrl = NULL;
		return retval;
	}

	return 0;
}

static int max77843_i2c_pinctrl_select(struct max77843_dev *max77843, bool on)
{
	struct pinctrl_state *pins_state;
	int ret;

	pins_state = on ? max77843->gpio_state_active
		: max77843->gpio_state_suspend;
	if (!IS_ERR_OR_NULL(pins_state)) {
		ret = pinctrl_select_state(max77843->max_pinctrl, pins_state);
		if (ret) {
			dev_err(max77843->dev,
				"can not set %s pins\n",
				on ? "max77843_interrupt_pins_active" : "max77843_interrupt_pins_suspend");
			return ret;
		}
	} else
		dev_err(max77843->dev,
			"not a valid '%s' pinstate\n",
				on ? "max77843_interrupt_pins_active" : "max77843_interrupt_pins_suspend");

	return 0;
}


static int of_max77843_dt(struct device *dev, struct max77843_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
#ifdef CONFIG_SS_VIBRATOR
	struct max77843_haptic_platform_data *haptic_data;
#endif
	if(!np)
		return -EINVAL;

#ifdef CONFIG_SS_VIBRATOR
	haptic_data = kzalloc(sizeof(struct max77843_haptic_platform_data), GFP_KERNEL);
	if (haptic_data == NULL)
		return -ENOMEM;
#endif

	pdata->irq_gpio = of_get_named_gpio_flags(np, "max77843,irq-gpio",
				0, &pdata->irq_gpio_flags);
	pdata->wakeup = of_property_read_bool(np, "max77843,wakeup");
	pr_info("%s: irq-gpio: %u \n", __func__, pdata->irq_gpio);
#ifdef CONFIG_SS_VIBRATOR
	if (!of_property_read_u32(np, "haptic,mode", &haptic_data->mode))
		haptic_data->mode = 1;
	if (!of_property_read_u32(np, "haptic,divisor", &haptic_data->divisor))
		haptic_data->divisor = 128;
	pr_info("%s: mode: %d \n", __func__, haptic_data->mode);
	pr_info("%s: divisor: %d \n", __func__, haptic_data->divisor);
	pdata->haptic_data = haptic_data;
#endif
	return 0;
}

static int max77843_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	int retval = 0;
	struct max77843_dev *max77843;
	struct max77843_platform_data *pdata;
	u8 reg_data;
	int ret = 0;
	dev_info(&i2c->dev, "%s\n", __func__);

	max77843 = kzalloc(sizeof(struct max77843_dev), GFP_KERNEL);
	if (max77843 == NULL)
		return -ENOMEM;

	if (i2c->dev.of_node) {
		pdata = devm_kzalloc(&i2c->dev,
				sizeof(struct max77843_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&i2c->dev, "Failed to allocate memory \n");
			ret = -ENOMEM;
			goto err;
		}

		ret = of_max77843_dt(&i2c->dev, pdata);
		if (ret < 0){
			dev_err(&i2c->dev, "Failed to get device of_node \n");
			ret = -ENODEV;
			goto err;
		}
		/*Filling the platform data*/
		pdata->muic_data = &max77843_muic;
#ifdef CONFIG_REGULATOR_MAX77843
		pdata->num_regulators = MAX77843_REG_MAX;
		pdata->regulators = max77843_regulators;
#endif
#ifdef CONFIG_LEDS_MAX77843
		pdata->led_data = &max77843_led_pdata;
#endif
		/*pdata update to other modules*/
		i2c->dev.platform_data = pdata;
	} else
		pdata = i2c->dev.platform_data;

	i2c_set_clientdata(i2c, max77843);
	max77843->dev = &i2c->dev;

	max77843->i2c = i2c;
	max77843->irq = i2c->irq;
	if (pdata) {
		max77843->pdata = pdata;
		max77843->irq_base = irq_alloc_descs(-1, 0, MAX77843_IRQ_NR, -1);
		if (max77843->irq_base < 0) {
			pr_err("%s: irq_alloc_descs Fail ret(%d)\n",
					__func__, max77843->irq_base);
			ret = -EINVAL;
		} else {
			pdata->irq_base = max77843->irq_base;
		}
		max77843->irq_gpio = pdata->irq_gpio;
		max77843->wakeup = pdata->wakeup;
	} else {
		ret = -EINVAL;
		goto err;
	}

	retval = max77843_i2c_pinctrl_init(max77843);
	if (!retval && max77843->max_pinctrl) {
		retval = max77843_i2c_pinctrl_select(max77843, true);
		if (retval < 0)
			goto err;
	}


	mutex_init(&max77843->iolock);

	max77843->muic = i2c_new_dummy(i2c->adapter, I2C_ADDR_MUIC);
	i2c_set_clientdata(max77843->muic, max77843);

	max77843->charger = i2c_new_dummy(i2c->adapter, I2C_ADDR_CHG);
	i2c_set_clientdata(max77843->charger, max77843);

	max77843->fuelgauge = i2c_new_dummy(i2c->adapter, I2C_ADDR_FG);
	i2c_set_clientdata(max77843->fuelgauge, max77843);

	max77843->led = i2c_new_dummy(i2c->adapter, I2C_ADDR_LED);
	i2c_set_clientdata(max77843->led, max77843);

	if (max77843_read_reg(max77843->i2c, MAX77843_PMIC_REG_PMICID, &reg_data) < 0) {
		dev_err(max77843->dev,
			"device not found on this channel (this is not an error)\n");
		ret = -ENODEV;
	} else {
		/* print rev */
		max77843->pmic_rev = (reg_data & 0x7);
		max77843->pmic_ver = ((reg_data & 0xF8) >> 0x3);
		pr_info("%s: device found: rev.0x%x, ver.0x%x\n", __func__,
				max77843->pmic_rev, max77843->pmic_ver);
	}

	ret = max77843_irq_init(max77843);
	if (ret < 0)
		goto err_irq_init;

	ret = mfd_add_devices(max77843->dev, -1, max77843_devs,
			ARRAY_SIZE(max77843_devs), NULL, 0, NULL);
	if (ret < 0)
		goto err_mfd;

	device_init_wakeup(max77843->dev, pdata->wakeup);
#if defined(CONFIG_ADC_ONESHOT)
	/* Set oneshot mode */
	max77843_update_reg(max77843->muic, MAX77843_MUIC_REG_CTRL4,
			MAX77843_ADC_ONESHOT<<MAX77843_CTRL4_ADCMODE_SHIFT,
			MAX77843_CTRL4_ADCMODE_MASK);
#else
	/* Set continuous mode */
	max77843_update_reg(max77843->muic, MAX77843_MUIC_REG_CTRL4,
			MAX77843_ADC_ALWAYS<<MAX77843_CTRL4_ADCMODE_SHIFT,
			MAX77843_CTRL4_ADCMODE_MASK);
#endif
	return ret;

err_mfd:
	mfd_remove_devices(max77843->dev);
	max77843_irq_exit(max77843);
err_irq_init:
	i2c_unregister_device(max77843->muic);
	i2c_unregister_device(max77843->led);
err:
	kfree(max77843);
	return ret;
}

static int max77843_i2c_remove(struct i2c_client *i2c)
{
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);

	mfd_remove_devices(max77843->dev);
	i2c_unregister_device(max77843->muic);
	i2c_unregister_device(max77843->led);
	kfree(max77843);

	return 0;
}

static const struct i2c_device_id max77843_i2c_id[] = {
	{ "max77843", TYPE_MAX77843 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max77843_i2c_id);

static struct of_device_id max77843_i2c_match_table[] = {
	{ .compatible = "max77843,i2c", },
	{ },
};
MODULE_DEVICE_TABLE(of, max77843_i2c_match_table);

#ifdef CONFIG_PM
static int max77843_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		enable_irq_wake(max77843->irq);

	return 0;
}

static int max77843_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);

	if (device_may_wakeup(dev))
		disable_irq_wake(max77843->irq);

	return max77843_irq_resume(max77843);
}
#else
#define max77843_suspend	NULL
#define max77843_resume		NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_HIBERNATION
#if 0
u8 max77843_dumpaddr_pmic[] = {
	MAX77843_LED_REG_IFLASH1,
	MAX77843_LED_REG_IFLASH2,
	MAX77843_LED_REG_ITORCH,
	MAX77843_LED_REG_ITORCHTORCHTIMER,
	MAX77843_LED_REG_FLASH_TIMER,
	MAX77843_LED_REG_FLASH_EN,
	MAX77843_LED_REG_MAX_FLASH1,
	MAX77843_LED_REG_MAX_FLASH2,
	MAX77843_LED_REG_VOUT_CNTL,
	MAX77843_LED_REG_VOUT_FLASH1,
	MAX77843_LED_REG_FLASH_INT_STATUS,

	MAX77843_PMIC_REG_TOPSYS_INT_MASK,
	MAX77843_PMIC_REG_MAINCTRL1,
	MAX77843_PMIC_REG_LSCNFG,
};
#endif
u8 max77843_dumpaddr_muic[] = {
	MAX77843_MUIC_REG_INTMASK1,
	MAX77843_MUIC_REG_INTMASK2,
	MAX77843_MUIC_REG_INTMASK3,
	MAX77843_MUIC_REG_CDETCTRL1,
	MAX77843_MUIC_REG_CDETCTRL2,
	MAX77843_MUIC_REG_CTRL1,
	MAX77843_MUIC_REG_CTRL2,
	MAX77843_MUIC_REG_CTRL3,
	MAX77843_MUIC_REG_CTRL4,
};

#if 0
u8 max77843_dumpaddr_haptic[] = {
	MAX77843_HAPTIC_REG_CONFIG1,
	MAX77843_HAPTIC_REG_CONFIG2,
	MAX77843_HAPTIC_REG_CONFIG_CHNL,
	MAX77843_HAPTIC_REG_CONFG_CYC1,
	MAX77843_HAPTIC_REG_CONFG_CYC2,
	MAX77843_HAPTIC_REG_CONFIG_PER1,
	MAX77843_HAPTIC_REG_CONFIG_PER2,
	MAX77843_HAPTIC_REG_CONFIG_PER3,
	MAX77843_HAPTIC_REG_CONFIG_PER4,
	MAX77843_HAPTIC_REG_CONFIG_DUTY1,
	MAX77843_HAPTIC_REG_CONFIG_DUTY2,
	MAX77843_HAPTIC_REG_CONFIG_PWM1,
	MAX77843_HAPTIC_REG_CONFIG_PWM2,
	MAX77843_HAPTIC_REG_CONFIG_PWM3,
	MAX77843_HAPTIC_REG_CONFIG_PWM4,
};
#endif

u8 max77843_dumpaddr_led[] = {
	MAX77843_RGBLED_REG_LEDEN,
	MAX77843_RGBLED_REG_LED0BRT,
	MAX77843_RGBLED_REG_LED1BRT,
	MAX77843_RGBLED_REG_LED2BRT,
	MAX77843_RGBLED_REG_LED3BRT,
	MAX77843_RGBLED_REG_LEDBLNK,
	MAX77843_RGBLED_REG_LEDRMP,
};

static int max77843_freeze(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int i;

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_pmic); i++)
		max77843_read_reg(i2c, max77843_dumpaddr_pmic[i],
				&max77843->reg_pmic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_muic); i++)
		max77843_read_reg(i2c, max77843_dumpaddr_muic[i],
				&max77843->reg_muic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_led); i++)
		max77843_read_reg(i2c, max77843_dumpaddr_led[i],
				&max77843->reg_led_dump[i]);

	disable_irq(max77843->irq);

	return 0;
}

static int max77843_restore(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77843_dev *max77843 = i2c_get_clientdata(i2c);
	int i;

	enable_irq(max77843->irq);

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_pmic); i++)
		max77843_write_reg(i2c, max77843_dumpaddr_pmic[i],
				max77843->reg_pmic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_muic); i++)
		max77843_write_reg(i2c, max77843_dumpaddr_muic[i],
				max77843->reg_muic_dump[i]);

	for (i = 0; i < ARRAY_SIZE(max77843_dumpaddr_led); i++)
		max77843_write_reg(i2c, max77843_dumpaddr_led[i],
				max77843->reg_led_dump[i]);


	return 0;
}
#endif


const struct dev_pm_ops max77843_pm = {
	.suspend = max77843_suspend,
	.resume = max77843_resume,
#ifdef CONFIG_HIBERNATION
	.freeze =  max77843_freeze,
	.thaw = max77843_restore,
	.restore = max77843_restore,
#endif
};

static struct i2c_driver max77843_i2c_driver = {
	.driver = {
		.name = "max77843",
		.owner = THIS_MODULE,
		.pm = &max77843_pm,
		.of_match_table = max77843_i2c_match_table,
	},
	.probe = max77843_i2c_probe,
	.remove = max77843_i2c_remove,
	.id_table = max77843_i2c_id,
};

//module_i2c_driver(max77843_i2c_driver);

static int __init max77843_i2c_init(void)
{
	printk("%s \n",__func__);
	return i2c_add_driver(&max77843_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(max77843_i2c_init);

static void __exit max77843_i2c_exit(void)
{
	i2c_del_driver(&max77843_i2c_driver);
}
module_exit(max77843_i2c_exit);

MODULE_DESCRIPTION("MAXIM 77843 multi-function core driver");
MODULE_AUTHOR("SangYoung, Son <hello.son@samsung.com>");
MODULE_LICENSE("GPL");
