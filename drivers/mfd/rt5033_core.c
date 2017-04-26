/* drivers/mfd/rt5033_core.c
 * RT5033 Multifunction Device Driver
 * Charger / Buck / LDOs / FlashLED
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/mfd/core.h>
#include <linux/mfd/rt5033.h>
#include <linux/mfd/rt5033_irq.h>
#include <linux/battery/charger/rt5033_charger.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_OF
#if defined(CONFIG_MFD_RT5033_USE_DT) || (LINUX_VERSION_CODE>=KERNEL_VERSION(3,10,0))
#define RT5033_USE_NEW_MFD_DT_API
#endif
#endif

#define RT5033_DECLARE_IRQ(irq) { \
	irq, irq, \
	irq##_NAME, IORESOURCE_IRQ }

#ifdef CONFIG_CHARGER_RT5033
const static struct resource rt5033_charger_res[] = {
	RT5033_DECLARE_IRQ(RT5033_ADPBAD_IRQ),
	RT5033_DECLARE_IRQ(RT5033_PPBATLV_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHTMRFI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_VINOVPI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_TSDI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHMIVRI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHTREGI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_IEOC_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHTERMI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHRCHGI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHBATOVI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_CHRVPI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_BSTLOWVI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_BSTOLI_IRQ),
	RT5033_DECLARE_IRQ(RT5033_BSTVMIDOVP_IRQ),
	RT5033_DECLARE_IRQ(RT5033_OVPR_IRQ),
};

static struct mfd_cell rt5033_charger_devs[] = {
	{
		.name			= "rt5033-charger",
		.num_resources	= ARRAY_SIZE(rt5033_charger_res),
		.id				= -1,
		.resources		= rt5033_charger_res,
#ifdef RT5033_USE_NEW_MFD_DT_API
        .of_compatible = "richtek,rt5033-charger"
#endif /* RT5033_USE_NEW_MFD_DT_API */
	},
};
#endif /*CONFIG_CHARGER_RT5033*/

#ifdef CONFIG_FLED_RT5033
const static struct resource rt5033_fled_res[] = {
	RT5033_DECLARE_IRQ(RT5033_VF_L_IRQ),
	RT5033_DECLARE_IRQ(RT5033_LEDCS2_SHORT_IRQ),
	RT5033_DECLARE_IRQ(RT5033_LEDCS1_SHORT_IRQ),
};
static struct mfd_cell rt5033_fled_devs[] = {
	{
		.name			= "rt5033-fled",
		.num_resources	= ARRAY_SIZE(rt5033_fled_res),
		.id				= -1,
		.resources		= rt5033_fled_res,
#ifdef RT5033_USE_NEW_MFD_DT_API
        .of_compatible = "richtek,rt5033-fled"
#endif /* RT5033_USE_NEW_MFD_DT_API */
	},
};
#endif /*CONFIG_FLED_RT5033*/

#ifdef CONFIG_REGULATOR_RT5033
const static struct resource rt5033_regulator_res_LDO_SAFE[] = {
	RT5033_DECLARE_IRQ(RT5033_SAFE_LDO_LV_IRQ),
};

const static struct resource rt5033_regulator_res_LDO1[] = {
	RT5033_DECLARE_IRQ(RT5033_LDO_LV_IRQ),
};

const static struct resource rt5033_regulator_res_DCDC1[] = {
	RT5033_DECLARE_IRQ(RT5033_BUCK_OCP_IRQ),
	RT5033_DECLARE_IRQ(RT5033_BUCK_LV_IRQ),
	RT5033_DECLARE_IRQ(RT5033_OT_IRQ),
	RT5033_DECLARE_IRQ(RT5033_VDDA_UV_IRQ),
};

#define RT5033_OF_COMPATIBLE_LDO_SAFE "richtek,rt5033-safeldo"
#define RT5033_OF_COMPATIBLE_LDO1 "richtek,rt5033-ldo1"
#define RT5033_OF_COMPATIBLE_DCDC1 "richtek,rt5033-dcdc1"

#ifdef RT5033_USE_NEW_MFD_DT_API
#define REG_OF_COMP(_id) .of_compatible = RT5033_OF_COMPATIBLE_##_id,
#else
#define REG_OF_COMP(_id)
#endif /* RT5033_USE_NEW_MFD_DT_API */

#define RT5033_VR_DEVS(_id)             \
{                                       \
	.name		= "rt5033-regulator",	\
		.num_resources = ARRAY_SIZE(rt5033_regulator_res_##_id), \
		.id				= RT5033_ID_##_id,			\
		.resources		= rt5033_regulator_res_##_id, \
		REG_OF_COMP(_id)                   \
}

static struct mfd_cell rt5033_regulator_devs[] = {
	RT5033_VR_DEVS(LDO_SAFE),
	RT5033_VR_DEVS(LDO1),
	RT5033_VR_DEVS(DCDC1),
};
#endif

void rt5033_lock_regulator(struct i2c_client *i2c)
{
     struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
     mutex_lock(&chip->regulator_lock);
}
EXPORT_SYMBOL(rt5033_lock_regulator);

void rt5033_unlock_regulator(struct i2c_client *i2c)
{
     struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
     mutex_unlock(&chip->regulator_lock);
}
EXPORT_SYMBOL(rt5033_unlock_regulator);

#ifdef CONFIG_REGULATOR_RT5033
void rt5033_set_regulator_state(struct i2c_client *i2c, int id, bool en)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	chip->regulator_states[id] = en;
}
EXPORT_SYMBOL(rt5033_set_regulator_state);

bool rt5033_get_pmic_state(struct i2c_client *i2c)
{
	int i;
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	bool en = false;
	for (i = RT5033_ID_LDO1; i < RT5033_MAX_REGULATOR; i++)
	{
		en |= chip->regulator_states[i];
	}
	return en;
}
EXPORT_SYMBOL(rt5033_get_pmic_state);

#endif

inline static int rt5033_read_device(struct i2c_client *i2c,
		int reg, int bytes, void *dest)
{
	int ret;
	if (bytes > 1) {
		ret = i2c_smbus_read_i2c_block_data(i2c, reg, bytes, dest);
	} else {
		ret = i2c_smbus_read_byte_data(i2c, reg);
		if (ret < 0)
			return ret;
		*(unsigned char *)dest = (unsigned char)ret;
	}
	return ret;
}

inline static int rt5033_write_device(struct i2c_client *i2c,
		int reg, int bytes, const void *src)
{
	int ret;
	const uint8_t *data;
	if (bytes > 1)
		ret = i2c_smbus_write_i2c_block_data(i2c, reg, bytes, src);
	else {
		data = src;
		ret = i2c_smbus_write_byte_data(i2c, reg, *data);
	}
	return ret;
}

int rt5033_block_read_device(struct i2c_client *i2c,
		int reg, int bytes, void *dest)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	int ret;
	mutex_lock(&chip->io_lock);
	ret = rt5033_read_device(i2c, reg, bytes, dest);
	mutex_unlock(&chip->io_lock);
	return ret;
}
EXPORT_SYMBOL(rt5033_block_read_device);

int rt5033_block_write_device(struct i2c_client *i2c,
		int reg, int bytes, const void *src)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	int ret;
	mutex_lock(&chip->io_lock);
	ret = rt5033_write_device(i2c, reg, bytes, src);
	mutex_unlock(&chip->io_lock);
	return ret;
}
EXPORT_SYMBOL(rt5033_block_write_device);

int rt5033_reg_read(struct i2c_client *i2c, int reg)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	unsigned char data = 0;
	int ret;

	mutex_lock(&chip->io_lock);
	ret = rt5033_read_device(i2c, reg, 1, &data);
	mutex_unlock(&chip->io_lock);

	if (ret < 0)
		return ret;
	else
		return (int)data;
}
EXPORT_SYMBOL(rt5033_reg_read);

int rt5033_reg_write(struct i2c_client *i2c, int reg,
		unsigned char data)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&chip->io_lock);
	ret = i2c_smbus_write_byte_data(i2c, reg, data);
	mutex_unlock(&chip->io_lock);

	return ret;
}
EXPORT_SYMBOL(rt5033_reg_write);

int rt5033_assign_bits(struct i2c_client *i2c, int reg,
		unsigned char mask, unsigned char data)
{
	struct rt5033_mfd_chip *chip = i2c_get_clientdata(i2c);
	unsigned char value;
	int ret;

	mutex_lock(&chip->io_lock);
	ret = rt5033_read_device(i2c, reg, 1, &value);

	if (ret < 0)
		goto out;

	value &= ~mask;
	value |= data;
	ret = i2c_smbus_write_byte_data(i2c, reg, value);

out:
	mutex_unlock(&chip->io_lock);
	return ret;
}
EXPORT_SYMBOL(rt5033_assign_bits);

int rt5033_set_bits(struct i2c_client *i2c, int reg,
		unsigned char mask)
{
	return rt5033_assign_bits(i2c,reg,mask,mask);
}
EXPORT_SYMBOL(rt5033_set_bits);

int rt5033_clr_bits(struct i2c_client *i2c, int reg,
		unsigned char mask)
{
	return rt5033_assign_bits(i2c,reg,mask,0);
}
EXPORT_SYMBOL(rt5033_clr_bits);

extern int rt5033_init_irq(rt5033_mfd_chip_t *chip);
extern int rt5033_exit_irq(rt5033_mfd_chip_t *chip);

static int rt5033mfd_parse_dt(struct device *dev,
		rt5033_mfd_platform_data_t *pdata)
{
	int ret;
	struct device_node *np = dev->of_node;
	enum of_gpio_flags irq_gpio_flags;

	ret = pdata->irq_gpio = of_get_named_gpio_flags(np, "rt5033,irq-gpio",
			0, &irq_gpio_flags);
	if (ret < 0) {
		dev_err(dev, "%s : can't get irq-gpio\r\n", __FUNCTION__);
		return ret;
	}

	pdata->irq_base = -1;
	ret = of_property_read_u32(np, "rt5033,irq-base", (u32 *)&pdata->irq_base);
	if (ret < 0 || pdata->irq_base == -1) {
		dev_info(dev, "%s : no assignment of irq_base, use irq_alloc_descs()\r\n",
			 __FUNCTION__);
	}
	return 0;
}

static int rt5033_reset(struct i2c_client *i2c)
{
	int ret;
	/* Force to enable OSC (Reg0x1a[5]) and then send CHG reset command */
	rt5033_set_bits(i2c, 0x1a, 0x20);
	ret = rt5033_reg_read(i2c, 0x19);
	/* the default value is supposed to be 0x47 but it could be 0x45 as well with certain model */
	if ((ret & (~0x2)) != 0x45) {
		/* Send RESET command to charger */
		rt5033_set_bits(i2c, 0x08, 0x80);
	}
	/* Send RESET command to FLED */
	rt5033_set_bits(i2c, 0x21, 0x80);
	msleep(1);
	/* Assign Force EN bit = 0 */
	rt5033_clr_bits(i2c, 0x1a, 0x20);
	return 0;
}

void rt5033_read_dump(struct i2c_client *i2c)
{
	u8 d1,d2,d3;

	d1 = rt5033_reg_read(i2c, 0x47);
	d2 = rt5033_reg_read(i2c, 0x41) & 0xF0;
	d3 = rt5033_reg_read(i2c, 0x6b) & 0x01;
	printk("RT5033# RST:0x%2X, LDO:0x%x, OSC:0x%x\n", d1, d2, d3);
}
EXPORT_SYMBOL(rt5033_read_dump);

void rt5033_workaround(rt5033_mfd_chip_t *chip)
{
	static int once = 0;
	struct i2c_client *i2c = chip->i2c_client;

	if ( !once ) {
		rt5033_read_dump(i2c);
		rt5033_lock_regulator(i2c);
		msleep(1);
		if ( chip->rev_id >= 6) {
			/* always enable I2C reset */
			rt5033_set_bits(i2c, 0x47, 0x88);
			pr_info("RT5033#I2C enable\n");

			/* Force to enable OSC (Reg0x6b[0]) and then make SCL_SDA_LOW reset be workable */
			rt5033_set_bits(i2c, 0x6b, 0x01);
			msleep(1);
			pr_info("RT5033#OSC enable\n");

			/* disable SLDO,LDO,BUCK */
			rt5033_clr_bits(i2c, 0x41, 0x40);
			pr_info("RT5033#SLDO disable\n");
		}
		else {
#ifdef CONFIG_MFD_RT5033_SLDO_VBUSDET
			/* enable SLDO */
			rt5033_set_bits(i2c, 0x41, 0x40);
			pr_info("RT5033#SLDO enable\n");
#else
			/* disable SLDO */
			rt5033_clr_bits(i2c, 0x41, 0x40);
			pr_info("RT5033#SLDO disable\n");
#endif
		}
		msleep(1);
		rt5033_unlock_regulator(i2c);
		rt5033_read_dump(i2c);
	}
	once++;
}
EXPORT_SYMBOL(rt5033_workaround);

static int rt5033_mfd_probe(struct i2c_client *i2c,
						const struct i2c_device_id *id)
{
	int ret = 0;
	u8 data = 0;
	struct device_node *of_node = i2c->dev.of_node;
	rt5033_mfd_chip_t *chip;
	rt5033_mfd_platform_data_t *pdata = i2c->dev.platform_data;

	pr_info("%s : RT5033 MFD Driver %s start probing\n", __func__, RT5033_DRV_VER);
	if (of_node) {
		pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata) {
			dev_err(&i2c->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_dt_nomem;
		}
		ret = rt5033mfd_parse_dt(&i2c->dev, pdata);
		if (ret < 0)
            goto err_parse_dt;
    } else {
        pdata = i2c->dev.platform_data;
    }

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL) {
		dev_err(&i2c->dev, "Memory is not enough.\n");
		ret = -ENOMEM;
		goto err_mfd_nomem;
	}
	chip->dev = &i2c->dev;

	ret = i2c_check_functionality(i2c->adapter, I2C_FUNC_SMBUS_BYTE_DATA |
			I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_I2C_BLOCK);
	if (!ret) {
		ret = i2c_get_functionality(i2c->adapter);
		dev_err(chip->dev, "I2C functionality is not supported.\n");
		ret = -ENOSYS;
		goto err_i2cfunc_not_support;
	}

	chip->i2c_client = i2c;
	chip->pdata = pdata;

    pr_info("%s:%s pdata->irq_base = %d\n",
            "rt5033-mfd", __func__, pdata->irq_base);
    /* if board-init had already assigned irq_base (>=0) ,
    no need to allocate it;
    assign -1 to let this driver allocate resource by itself*/
    if (pdata->irq_base < 0)
        pdata->irq_base = irq_alloc_descs(-1, 0, RT5033_IRQS_NR, 0);
	if (pdata->irq_base < 0) {
		pr_err("%s:%s irq_alloc_descs Fail! ret(%d)\n",
				"rt5033-mfd", __func__, pdata->irq_base);
		ret = -EINVAL;
		goto irq_base_err;
	} else {
		chip->irq_base = pdata->irq_base;
		pr_info("%s:%s irq_base = %d\n",
			"rt5033-mfd", __func__, chip->irq_base);
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,4,0))
		irq_domain_add_legacy(of_node, RT5033_IRQS_NR, chip->irq_base, 0,
				      &irq_domain_simple_ops, NULL);
#endif /*(LINUX_VERSION_CODE>=KERNEL_VERSION(3,4,0))*/
	}

	i2c_set_clientdata(i2c, chip);
	mutex_init(&chip->io_lock);
	mutex_init(&chip->regulator_lock);
	mutex_init(&chip->suspend_flag_lock);

	wake_lock_init(&(chip->irq_wake_lock), WAKE_LOCK_SUSPEND,
			"rt5033mfd_wakelock");

	/* To disable MRST function should be
	finished before set any reg init-value*/
	data = rt5033_reg_read(i2c, 0x47);
	pr_info("%s : Manual Reset Data = 0x%x\n", __func__, data);
	/* Disable Manual Reset and set debounce time = 3 sec*/
	rt5033_assign_bits(i2c, 0x47, 0x0f, 0);
	rt5033_reset(i2c);

	ret = rt5033_init_irq(chip);

	if (ret < 0) {
		dev_err(chip->dev,
				"Error : can't initialize RT5033 MFD irq\n");
		goto err_init_irq;
	}

	rt5033_set_bits(i2c, 0x6b, 0x01);
	usleep(100); /* delay 100 us to wait for normal read (from e-fuse) */
	chip->rev_id = rt5033_reg_read(i2c, 0x03) & 0x0f;
	rt5033_clr_bits(i2c, 0x6b, 0x01);
	pr_info("%s : rev_id = %d\n", __func__, chip->rev_id);

#ifdef CONFIG_REGULATOR_RT5033
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,6,0))
	ret = mfd_add_devices(chip->dev, 0, &rt5033_regulator_devs[0],
			ARRAY_SIZE(rt5033_regulator_devs),
			NULL, chip->irq_base, NULL);
#else
	ret = mfd_add_devices(chip->dev, 0, &rt5033_regulator_devs[0],
			ARRAY_SIZE(rt5033_regulator_devs),
			NULL, chip->irq_base);
#endif
	if (ret < 0) {
		dev_err(chip->dev,
				"Error : can't add regulator\n");
		goto err_add_regulator_devs;
	}
#endif /*CONFIG_REGULATOR_RT5033*/

#ifdef CONFIG_FLED_RT5033
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,6,0))
	ret = mfd_add_devices(chip->dev, 0, &rt5033_fled_devs[0],
			ARRAY_SIZE(rt5033_fled_devs),
			NULL, chip->irq_base, NULL);
#else
	ret = mfd_add_devices(chip->dev, 0, &rt5033_fled_devs[0],
			ARRAY_SIZE(rt5033_fled_devs),
			NULL, chip->irq_base);
#endif
	if (ret < 0) {
		dev_err(chip->dev, "Failed : add FlashLED devices");
		goto err_add_fled_devs;
	}
#endif /*CONFIG_FLED_RT5033*/

#ifdef CONFIG_CHARGER_RT5033
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,6,0))
	ret = mfd_add_devices(chip->dev, 0, &rt5033_charger_devs[0],
			ARRAY_SIZE(rt5033_charger_devs),
			NULL, chip->irq_base, NULL);
#else
	ret = mfd_add_devices(chip->dev, 0, &rt5033_charger_devs[0],
			ARRAY_SIZE(rt5033_charger_devs),
			NULL, chip->irq_base);
#endif
	if (ret<0) {
		dev_err(chip->dev, "Failed : add charger devices\n");
		goto err_add_chg_devs;
	}
#endif /*CONFIG_CHARGER_RT5033*/

	pr_info("%s : RT5033 MFD Driver Fin probe\n", __func__);
	return ret;

#ifdef CONFIG_CHARGER_RT5033
err_add_chg_devs:
#endif /*CONFIG_CHARGER_RT5033*/

#ifdef CONFIG_FLED_RT5033
err_add_fled_devs:
#endif /*CONFIG_FLED_RT5033*/
	mfd_remove_devices(chip->dev);
#ifdef CONFIG_REGULATOR_RT5033
err_add_regulator_devs:
#endif /*CONFIG_REGULATOR_RT5033*/
err_init_irq:
	wake_lock_destroy(&(chip->irq_wake_lock));
	mutex_destroy(&chip->regulator_lock);
	mutex_destroy(&chip->io_lock);
	kfree(chip);
irq_base_err:
err_mfd_nomem:
err_i2cfunc_not_support:
err_parse_dt:
err_dt_nomem:
	return ret;
}

static int rt5033_mfd_remove(struct i2c_client *i2c)
{
	rt5033_mfd_chip_t *chip = i2c_get_clientdata(i2c);

	pr_info("%s : RT5033 MFD Driver remove\n", __func__);
	mfd_remove_devices(chip->dev);
	wake_lock_destroy(&(chip->irq_wake_lock));
	mutex_destroy(&chip->suspend_flag_lock);
	mutex_destroy(&chip->regulator_lock);
	mutex_destroy(&chip->io_lock);
	kfree(chip);

	return 0;
}

#ifdef CONFIG_PM
extern int rt5033_irq_suspend(rt5033_mfd_chip_t *chip);
extern int rt5033_irq_resume(rt5033_mfd_chip_t *chip);
int rt5033_mfd_suspend(struct device *dev)
{

	struct i2c_client *i2c =
		container_of(dev, struct i2c_client, dev);

	rt5033_mfd_chip_t *chip = i2c_get_clientdata(i2c);
	BUG_ON(chip == NULL);
	return rt5033_irq_suspend(chip);
}

int rt5033_mfd_resume(struct device *dev)
{
	struct i2c_client *i2c =
		container_of(dev, struct i2c_client, dev);
	rt5033_mfd_chip_t *chip = i2c_get_clientdata(i2c);
	BUG_ON(chip == NULL);
	return rt5033_irq_resume(chip);
}
#endif /* CONFIG_PM */


const static uint8_t rt5033_chg_group1_default[] = { 0x40};
const static uint8_t rt5033_chg_group2_default[] = {0x41, 0xAB, 0x35};

static void rt5033_mfd_shutdown(struct i2c_client *client)
{
	struct i2c_client *i2c = client;
    /* Force to enable charger & reset charger before shutdown */
    rt5033_clr_bits(i2c, RT5033_CHG_STAT_CTRL, RT5033_CHGENB_MASK);
    /* Set all charger settings to default values */
    rt5033_block_write_device(i2c, RT5033_CHG_CTRL1,
                              sizeof(rt5033_chg_group1_default),
                              rt5033_chg_group1_default);
    rt5033_block_write_device(i2c, RT5033_CHG_CTRL3,
                              sizeof(rt5033_chg_group2_default),
                              rt5033_chg_group2_default);
}


static const struct i2c_device_id rt5033_mfd_id_table[] = {
	{ "rt5033-mfd", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, rt5033_id_table);


#ifdef CONFIG_PM
const struct dev_pm_ops rt5033_pm = {
	.suspend = rt5033_mfd_suspend,
	.resume = rt5033_mfd_resume,
};
#endif

#ifdef CONFIG_OF
static struct of_device_id rt5033_match_table[] = {
	{ .compatible = "richtek,rt5033mfd",},
	{},
};
#else
#define rt5033_match_table NULL
#endif

static struct i2c_driver rt5033_mfd_driver = {
	.driver	= {
		.name	= "rt5033-mfd",
		.owner	= THIS_MODULE,
		.of_match_table = rt5033_match_table,
#ifdef CONFIG_PM
		.pm		= &rt5033_pm,
#endif
	},
	.shutdown = rt5033_mfd_shutdown,
	.probe		= rt5033_mfd_probe,
	.remove		= rt5033_mfd_remove,
	.id_table	= rt5033_mfd_id_table,
};

static int __init rt5033_mfd_i2c_init(void)
{
	int ret;

	pr_info("%s : RT5033 init\n", __func__);
	ret = i2c_add_driver(&rt5033_mfd_driver);
	if (ret != 0)
		pr_info("%s : Failed to register RT5033 MFD I2C driver\n",
		__func__);

	return ret;
}
subsys_initcall(rt5033_mfd_i2c_init);

static void __exit rt5033_mfd_i2c_exit(void)
{
	i2c_del_driver(&rt5033_mfd_driver);
}
module_exit(rt5033_mfd_i2c_exit);

MODULE_DESCRIPTION("Richtek RT5033 MFD I2C Driver");
MODULE_AUTHOR("Patrick Chang <patrick_chang@richtek.com>");
MODULE_VERSION(RT5033_DRV_VER);
MODULE_LICENSE("GPL");
