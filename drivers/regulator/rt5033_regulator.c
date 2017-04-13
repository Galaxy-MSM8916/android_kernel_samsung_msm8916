/* drivers/regulator/rt5033_regulator.c
 * RT5033 Regulator / Buck Driver
 * Copyright (C) 2013
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/rt5033.h>
#include <linux/mfd/rt5033_irq.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/regulator/of_regulator.h>

#define ALIAS_NAME "rt5033-regulator"

#define RT5033A_REV	6
#define EN_DCDC_FORCE_PWM 1
#define EN_BUCK_IRQ 1
#define EN_VDDA_UV_IRQ 0
#define EN_LDO_IRQ 0
#define EN_SLDO_IRQ 0

struct rt5033_regulator_info {
	struct regulator_desc desc;
	struct i2c_client *i2c;
	struct rt5033_mfd_chip	*chip;
	int	min_uV;
	int	max_uV;
	int	vol_reg;
	int	vol_shift;
	int vol_mask;
	int	enable_bit;
	int	enable_reg;
	unsigned int const *output_list;
	unsigned int output_list_count;
};

#define RT5033_REGULATOR_REG_LDO_SAFE   (0x43)
#define RT5033_REGULATOR_SHIFT_LDO_SAFE (6)
#define RT5033_REGULATOR_MASK_LDO_SAFE  (7<<6)
#define RT5033_REGULATOR_REG_LDO1       (0x43)
#define RT5033_REGULATOR_SHIFT_LDO1     (0)
#define RT5033_REGULATOR_MASK_LDO1      (0x1f<<0)
#define RT5033_REGULATOR_REG_DCDC1      (0x42)
#define RT5033_REGULATOR_SHIFT_DCDC1    (0)
#define RT5033_REGULATOR_MASK_DCDC1     (0x1f<<0)

#define RT5033_REGULATOR_REG_OUTPUT_EN (0x41)
#define RT5033_REGULATOR_EN_MASK_LDO_SAFE (1<<6)
#define RT5033_REGULATOR_EN_MASK_LDO1 (1<<5)
#define RT5033_REGULATOR_EN_MASK_DCDC1 (1<<4)

static const unsigned int rt5033_dcdc_output_list[] = {
	1000*1000,
	1100*1000,
	1200*1000,
	1300*1000,
	1400*1000,
	1500*1000,
	1600*1000,
	1700*1000,
	1800*1000,
	1900*1000,
	2000*1000,
	2100*1000,
	2200*1000,
	2300*1000,
	2400*1000,
	2500*1000,
	2600*1000,
	2700*1000,
	2800*1000,
	2900*1000,
	3000*1000,
};

static const unsigned int rt5033_ldo_output_list[] = {
	1200*1000,
	1300*1000,
	1400*1000,
	1500*1000,
	1600*1000,
	1700*1000,
	1800*1000,
	1900*1000,
	2000*1000,
	2100*1000,
	2200*1000,
	2300*1000,
	2400*1000,
	2500*1000,
	2600*1000,
	2700*1000,
	2800*1000,
	2900*1000,
	3000*1000,
};

static const unsigned int rt5033_safe_ldo_output_list[] = {
	3300*1000,
	4850*1000,
	4900*1000,
	4950*1000,
};

static int chip_rev;

#define RT5033_REGULATOR_DECL(_id, min, max,out_list)   \
{								                        \
	.desc	= {						                    \
		.name	= "RT5033_REGULATOR_" #_id,				\
		.ops	= &rt5033_regulator_ldo_dcdc_ops,		\
		.type	= REGULATOR_VOLTAGE,			        \
		.id	= RT5033_ID_##_id,			                \
		.owner	= THIS_MODULE,				            \
		.n_voltages = ARRAY_SIZE(out_list),             \
	},							                        \
	.min_uV		= min * 1000,				            \
	.max_uV		= max * 1000,				            \
	.vol_reg	= RT5033_REGULATOR_REG_##_id,           \
	.vol_shift	= RT5033_REGULATOR_SHIFT_##_id,         \
	.vol_mask	= RT5033_REGULATOR_MASK_##_id,          \
	.enable_reg	= RT5033_REGULATOR_REG_OUTPUT_EN,		\
	.enable_bit	= RT5033_REGULATOR_EN_MASK_##_id,		\
	.output_list = out_list,                            \
	.output_list_count = ARRAY_SIZE(out_list),          \
}

static inline int rt5033_regulator_check_range(struct rt5033_regulator_info *info,
		int min_uV, int max_uV)
{
	if (min_uV < info->min_uV || max_uV > info->max_uV)
		return -EINVAL;

	return 0;
}

static int rt5033_regulator_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);

	return (index>=info->output_list_count)?
		-EINVAL: info->output_list[index];
}


#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,39))
int rt5033_regulator_set_voltage_sel(struct regulator_dev *rdev, unsigned selector)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;
	int ret;

	pr_info("%s select = %d, output list count = %d\n",
			ALIAS_NAME, selector, info->output_list_count);
	if (selector>=info->output_list_count)
		return -EINVAL;
	pr_info("%s Vout = %d\n", ALIAS_NAME, info->output_list[selector]);
	data = (unsigned char)selector;
	data <<= info->vol_shift;
	ret = rt5033_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);

	pr_info("%s %s %s ret (%d)", ALIAS_NAME, rdev->desc->name, __func__, ret);

	return ret;
}
#endif

static int rt5033_regulator_get_voltage_sel(struct regulator_dev *rdev)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	ret = rt5033_reg_read(info->i2c, info->vol_reg);
	if (ret < 0)
		return ret;
	return (ret & info->vol_mask)  >> info->vol_shift;
}

#if (LINUX_VERSION_CODE<KERNEL_VERSION(2,6,39))
static int rt5033_regulator_find_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV)
{
	int i=0;
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	const int count = info->output_list_count;
	for (i=0;i<count;i++)
	{
		if ((info->output_list[i]>=min_uV)
				&& (info->output_list[i]<=max_uV))
		{
			pr_info("%s Found V = %d , min_uV = %d,max_uV = %d\n",
					ALIAS_NAME, info->output_list[i], min_uV, max_uV);
			return i;
		}

	}
	pr_err("%s Not found min_uV = %d, max_uV = %d\n",
			ALIAS_NAME, min_uV, max_uV);
	return -EINVAL;
}
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))
static int rt5033_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,unsigned *selector)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;

	if (rt5033_regulator_check_range(info, min_uV, max_uV)) {
		pr_err("%s %s invalid voltage range (%d, %d) uV\n",
				ALIAS_NAME, rdev->desc->name, min_uV, max_uV);
		return -EINVAL;
	}
	*selector = rt5033_regulator_find_voltage(rdev,min_uV,max_uV);
	data = *selector << info->vol_shift;

	return rt5033_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);
}

#else
static int rt5033_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,int *selector)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;
	int ret;

	if (rt5033_regulator_check_range(info, min_uV, max_uV)) {
		pr_err("%s %s invalid voltage range (%d, %d) uV\n",
				ALIAS_NAME, rdev->desc->name, min_uV, max_uV);
		return -EINVAL;
	}
	data = rt5033_regulator_find_voltage(rdev,min_uV,max_uV);
	data <<= info->vol_shift;
	ret = rt5033_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);

	pr_info("%s %s ret (%d)", ALIAS_NAME, __func__, ret);

	return ret;
}
#endif //(LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))

static int rt5033_regulator_get_voltage(struct regulator_dev *rdev)
{
	int ret;
	ret = rt5033_regulator_get_voltage_sel(rdev);
	if (ret < 0)
		return ret;
	return rt5033_regulator_list_voltage(rdev, ret);
}
#endif //(LINUX_VERSION_CODE<KERNEL_VERSION(2,6,39))


static int rt5033_regulator_enable(struct regulator_dev *rdev)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	bool prev_pmic_state, pmic_state;

	if ( info->desc.id == RT5033_ID_LDO_SAFE ) {
		pr_info("RT5033#SLDO enable skip\n");
		rt5033_read_dump(info->i2c);
		return 0;
	}

	pr_info("%s Enable regulator %s\n", ALIAS_NAME, rdev->desc->name);

	rt5033_lock_regulator(info->i2c);
	prev_pmic_state = rt5033_get_pmic_state(info->i2c);
	rt5033_set_regulator_state(info->i2c, info->desc.id, true);
	pmic_state = rt5033_get_pmic_state(info->i2c);
	if (chip_rev >= RT5033A_REV && prev_pmic_state == false && pmic_state == true)
		rt5033_clr_bits(info->i2c, 0x6b, 0x01);
	mdelay(1);

#if EN_DCDC_FORCE_PWM
	/* Enable Force PWM for Buck */
	if (info->desc.id == RT5033_ID_DCDC1)
		rt5033_set_bits(info->i2c, 0x41, 0x01);
#endif /* EN_DCDC_FORCE_PWM */
	ret = rt5033_set_bits(info->i2c, info->enable_reg,
			info->enable_bit);
	pr_info("%s %s %s ret (%d)\n", ALIAS_NAME, rdev->desc->name, __func__, ret);
	mdelay(1);

	rt5033_unlock_regulator(info->i2c);

	return ret;
}

static int rt5033_regulator_disable(struct regulator_dev *rdev)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	bool prev_pmic_state, pmic_state;
	rt5033_lock_regulator(info->i2c);

	mdelay(1);
	pr_info("%s Disable regulator %s\n", ALIAS_NAME, rdev->desc->name);
#if EN_DCDC_FORCE_PWM
	/* Disable Force PWM for Buck */
	if (info->desc.id == RT5033_ID_DCDC1) {
		rt5033_clr_bits(info->i2c, 0x41, 0x01);
		udelay(100);
	}
#endif /* EN_DCDC_FORCE_PWM */
	ret = rt5033_clr_bits(info->i2c, info->enable_reg,
			info->enable_bit);
	pr_info("%s %s ret (%d)\n", ALIAS_NAME, __func__, ret);
	udelay(500);
	prev_pmic_state = rt5033_get_pmic_state(info->i2c);
	rt5033_set_regulator_state(info->i2c, info->desc.id, false);
	pmic_state = rt5033_get_pmic_state(info->i2c);
	if (chip_rev >= RT5033A_REV && prev_pmic_state == true && pmic_state == false)
		rt5033_set_bits(info->i2c, 0x6b, 0x01);
	rt5033_unlock_regulator(info->i2c);

	return ret;
}

static int rt5033_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;

	ret = rt5033_reg_read(info->i2c, info->enable_reg);
	if (ret < 0)
		return ret;

	ret = (ret & (info->enable_bit))?1:0;
	pr_info("%s %s %s ret (%d)\n", ALIAS_NAME, rdev->desc->name, __func__, ret);
	return ret;
}

#ifdef CONFIG_MFD_RT5033_RESET_WA
const static uint16_t rt5033_valid_pmic_status_f6[] = {
	0x00,
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
};
const static uint16_t rt5033_valid_pmic_status_f7[] = {
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
};

static bool check_status_is_vaild(uint16_t sta, uint16_t reg)
{
	uint16_t i;
	if (reg == 0xf6){
		for (i = 0; i < ARRAY_SIZE(rt5033_valid_pmic_status_f6); i++) {
			if (rt5033_valid_pmic_status_f6[i] == sta)
			return true;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(rt5033_valid_pmic_status_f7); i++) {
			if (rt5033_valid_pmic_status_f7[i] == sta)
			return true;
		}
	}
	return false;
}

static int rt5033_regulator_get_status(struct regulator_dev *rdev)
{
	struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
	/* REGULATOR_STATUS_OFF, REGULATOR_STATUS_ON, REGULATOR_STATUS_OFF */
	int ret = REGULATOR_STATUS_ERROR; /* 2 */
	int org_regval, dump_reg;
	//int sta1, sta2;
	uint16_t sta1, sta2;
#ifndef CONFIG_MFD_RT5033_RESET_GPIO
	/* if there are no reset solution(MRSTB or I2C),
       skip reset workaround(always return true) */
	if (chip_rev < RT5033A_REV)
		return REGULATOR_STATUS_ON;
#endif
	rt5033_lock_regulator(info->i2c);
	/* First time to check it */
	msleep(2);
	org_regval = rt5033_reg_read(info->i2c, 0xf0);
	rt5033_reg_write(info->i2c, 0xf0, 0x1e);
	rt5033_assign_bits(info->i2c, 0xf3, 0x03 << 6, 0x2 << 6);
	sta1 = rt5033_reg_read(info->i2c, 0xf6) & 0x1f;
	rt5033_assign_bits(info->i2c, 0xf3, 0x03 << 6, 0x3 << 6);
	sta2 = rt5033_reg_read(info->i2c, 0xf7) & 0x1f;
	printk("%s status #1[0x%2x]\n", __func__, (sta2 << 8) | sta1);
	if (check_status_is_vaild(sta1, 0xf6) && check_status_is_vaild(sta2, 0xf7))
		goto rt5033_reg_status_ok;

	/* Failed case, we need to check again */
	msleep(2);
	org_regval = rt5033_reg_read(info->i2c, 0xf0);
	rt5033_reg_write(info->i2c, 0xf0, 0x1e);
	rt5033_assign_bits(info->i2c, 0xf3, 0x03 << 6, 0x2 << 6);
	sta1 = rt5033_reg_read(info->i2c, 0xf6) & 0x1f;
	rt5033_assign_bits(info->i2c, 0xf3, 0x03 << 6, 0x3 << 6);
	sta2 = rt5033_reg_read(info->i2c, 0xf7) & 0x1f;
	printk("%s status #2[0x%2x]\n", __func__, (sta2 << 8) | sta1);
	if (check_status_is_vaild(sta1, 0xf6) && check_status_is_vaild(sta2, 0xf7))
		goto rt5033_reg_status_ok;

	/* Failed case, dump registers */
	dump_reg = rt5033_reg_read(info->i2c, 0x41);
	printk("%s LDO_CTRL:0x%2X\n", __func__, dump_reg);
	dump_reg = rt5033_reg_read(info->i2c, 0x47);
	printk("%s LDO_CTRL:0x%2X\n", __func__, dump_reg);
	dump_reg = rt5033_reg_read(info->i2c, 0x68);
	printk("%s PMIC_IRQ_STAT:0x%2X\n", __func__, dump_reg);
	dump_reg = rt5033_reg_read(info->i2c, 0x69);
	printk("%s PMIC_IRQ_CTRL:0x%2X\n", __func__, dump_reg);
	dump_reg = rt5033_reg_read(info->i2c, 0x6A);
	printk("%s SHDN_CTRL:0x%2X\n", __func__, dump_reg);
	dump_reg = rt5033_reg_read(info->i2c, 0x6B);
	printk("%s SHDN_CTRL:0x%2X\n", __func__, dump_reg);
		goto rt5033_reg_status_exit;

rt5033_reg_status_ok:
	ret = rt5033_regulator_is_enabled(rdev) ?
		REGULATOR_STATUS_ON :  REGULATOR_STATUS_OFF;

rt5033_reg_status_exit:
	rt5033_reg_write(info->i2c, 0xf0, org_regval);
	rt5033_unlock_regulator(info->i2c);
	if (ret==REGULATOR_STATUS_ERROR && chip_rev >= RT5033A_REV)
		ret = REGULATOR_STATUS_UNDEFINED; /* 8 */
	pr_err("%s ret:%d\n", __func__, ret);
	return ret;
}
#endif

static struct regulator_ops rt5033_regulator_ldo_dcdc_ops = {
	.list_voltage		= rt5033_regulator_list_voltage,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,39))
	.get_voltage_sel	= rt5033_regulator_get_voltage_sel,
	.set_voltage_sel	= rt5033_regulator_set_voltage_sel,
#else
	.set_voltage		= rt5033_regulator_set_voltage,
	.get_voltage		= rt5033_regulator_get_voltage,
#endif
	.enable			= rt5033_regulator_enable,
	.disable		= rt5033_regulator_disable,
	.is_enabled		= rt5033_regulator_is_enabled,
#ifdef CONFIG_MFD_RT5033_RESET_WA
	.get_status		= rt5033_regulator_get_status,
#endif
};

static struct rt5033_regulator_info rt5033_regulator_infos[] = {
	RT5033_REGULATOR_DECL(LDO_SAFE, 3300, 4950, rt5033_safe_ldo_output_list),
	RT5033_REGULATOR_DECL(LDO1, 1200, 3000, rt5033_ldo_output_list),
	RT5033_REGULATOR_DECL(DCDC1, 1000, 3300, rt5033_dcdc_output_list),
};

static struct rt5033_regulator_info * find_regulator_info(int id)
{
	struct rt5033_regulator_info *ri;
	int i;

	for (i = 0; i < ARRAY_SIZE(rt5033_regulator_infos); i++) {
		ri = &rt5033_regulator_infos[i];
		if (ri->desc.id == id)
			return ri;
	}
	return NULL;
}

inline struct regulator_dev* rt5033_regulator_register(struct regulator_desc *regulator_desc,
		struct device *dev, struct regulator_init_data *init_data,
		void *driver_data)
{
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,5,0))
	struct regulator_config config = {
		.dev = dev,
		.init_data = init_data,
		.driver_data = driver_data,
		.of_node = dev->of_node,
	};
	return regulator_register(regulator_desc, &config);
#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0))
	return regulator_register(regulator_desc, dev,
			init_data, driver_data, dev->of_node);
#else
	return regulator_register(regulator_desc, dev,
			init_data, driver_data);
#endif
}

static int rt5033_regulator_init_regs(struct regulator_dev* rdev)
{
	return 0;
}

static struct regulator_consumer_supply default_rt5033_safe_ldo_consumers[] = {
	REGULATOR_SUPPLY("rt5033_safe_ldo",NULL),
};
static struct regulator_consumer_supply default_rt5033_ldo_consumers[] = {
	REGULATOR_SUPPLY("rt5033_ldo",NULL),
};
static struct regulator_consumer_supply default_rt5033_buck_consumers[] = {
	REGULATOR_SUPPLY("rt5033_buck",NULL),
};

static struct regulator_init_data default_rt5033_safe_ldo_data = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 4950000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_rt5033_safe_ldo_consumers),
	.consumer_supplies = default_rt5033_safe_ldo_consumers,
};
static struct regulator_init_data default_rt5033_ldo_data = {
	.constraints = {
		.min_uV = 1200000,
		.max_uV = 3000000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_rt5033_ldo_consumers),
	.consumer_supplies = default_rt5033_ldo_consumers,
};
static struct regulator_init_data default_rt5033_buck_data = {
	.constraints = {
		.min_uV = 1000000,
		.max_uV = 3000000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_rt5033_buck_consumers),
	.consumer_supplies = default_rt5033_buck_consumers,
};

static struct rt5033_regulator_platform_data default_rv_pdata = {
	.regulator = {
		[RT5033_ID_LDO_SAFE] = &default_rt5033_safe_ldo_data,
		[RT5033_ID_LDO1] = &default_rt5033_ldo_data,
		[RT5033_ID_DCDC1] = &default_rt5033_buck_data,
	},
};


struct rt5033_pmic_irq_handler {
    char *name;
    int irq_index;
    irqreturn_t (*handler)(int irq, void *data);
};

#if EN_BUCK_IRQ
static irqreturn_t rt5033_pmic_buck_ocp_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("Buck OCP\n");
    return IRQ_HANDLED;
}

static irqreturn_t rt5033_pmic_buck_lv_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("Buck LV\n");
    return IRQ_HANDLED;
}

static irqreturn_t rt5033_pmic_ot_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("PMIC OT\n");
    return IRQ_HANDLED;
}
#endif /* EN_BUCK_IRQ */

#if EN_VDDA_UV_IRQ

static irqreturn_t rt5033_pmic_vdda_uv_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("PMIC VDDA UV\n");
    return IRQ_HANDLED;
}
#endif /* EN_VDDA_UV_IRQ */

#if EN_SLDO_IRQ
static irqreturn_t rt5033_pmic_safeldo_lv_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("Safe LDO LV\n");
    return IRQ_HANDLED;
}
#endif /* EN_SLDO_IRQ */

#if EN_LDO_IRQ
static irqreturn_t rt5033_pmic_ldo_lv_event_handler(int irq, void *data)
{
    struct regulator_dev *rdev = data;
    struct rt5033_regulator_info *info = rdev_get_drvdata(rdev);
    BUG_ON(rdev == NULL);
    BUG_ON(info == NULL);
    pr_info("LDO LV\n");
    return IRQ_HANDLED;
}
#endif /* EN_LDO_IRQ */

const struct rt5033_pmic_irq_handler rt5033_pmic_buck_irq_handlers[] = {
#if EN_BUCK_IRQ
    {
        .name = "BuckOCP",
        .handler = rt5033_pmic_buck_ocp_event_handler,
        .irq_index = RT5033_BUCK_OCP_IRQ,
    },
    {
        .name = "BuckLV",
        .handler = rt5033_pmic_buck_lv_event_handler,
        .irq_index = RT5033_BUCK_LV_IRQ,
    },
    {
        .name = "PMIC OT",
        .handler = rt5033_pmic_ot_event_handler,
        .irq_index = RT5033_OT_IRQ,
    },
#endif /* EN_BUCK_IRQ */
#if EN_VDDA_UV_IRQ
    {
        .name = "PMIC VDDA UV",
        .handler = rt5033_pmic_vdda_uv_event_handler,
        .irq_index = RT5033_VDDA_UV_IRQ,
    },
#endif /* EN_VDDA_UV_IRQ */
};

const struct rt5033_pmic_irq_handler rt5033_pmic_safeldo_irq_handlers[] = {
#if EN_SLDO_IRQ
    {
        .name = "SafeLDO LV",
        .handler = rt5033_pmic_safeldo_lv_event_handler,
        .irq_index = RT5033_SAFE_LDO_LV_IRQ,
    },
#endif
};
const struct rt5033_pmic_irq_handler rt5033_pmic_ldo_irq_handlers[] = {
#if EN_LDO_IRQ
    {
        .name = "LDO LV",
        .handler = rt5033_pmic_ldo_lv_event_handler,
        .irq_index = RT5033_LDO_LV_IRQ,
    },
#endif
};

static int register_irq(struct platform_device *pdev,
                struct regulator_dev *rdev,
                const struct rt5033_pmic_irq_handler *irq_handler,
                int irq_handler_size)
{
    int irq;
    int i, j;
    int ret;
    const char *irq_name;
    for (i = 0; i < irq_handler_size; i++) {
        irq_name = rt5033_get_irq_name_by_index(irq_handler[i].irq_index);
        irq = platform_get_irq_byname(pdev, irq_name);
        ret = request_threaded_irq(irq, NULL, irq_handler[i].handler,
                       IRQF_ONESHOT | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
                       irq_name, rdev);
        if (ret < 0) {
            pr_err("Failed to request IRQ: #%d: %d\n", irq, ret);
            goto err_irq;
        }
    }

    return 0;
err_irq:
    for (j = 0; j < i; j++) {
        irq_name = rt5033_get_irq_name_by_index(irq_handler[j].irq_index);
        irq = platform_get_irq_byname(pdev, irq_name);
        free_irq(irq, rdev);
    }
    return ret;
}

static void unregister_irq(struct platform_device *pdev,
                struct regulator_dev *rdev,
                const struct rt5033_pmic_irq_handler *irq_handler,
                int irq_handler_size)
{
    int irq;
    int i;
    const char *irq_name;
    for (i = 0; i < irq_handler_size; i++) {
        irq_name = rt5033_get_irq_name_by_index(irq_handler[i].irq_index);
        irq = platform_get_irq_byname(pdev, irq_name);
        free_irq(irq, rdev);
    }
}

#ifdef CONFIG_OF
static struct of_device_id rt5033_regulator_match_table[] = {
	{ .compatible = "richtek,rt5033-safeldo",},
	{ .compatible = "richtek,rt5033-ldo1",},
	{ .compatible = "richtek,rt5033-dcdc1",},
	{},
};
#else
#define rt5033_regulator_match_table NULL
#endif

static int rt5033_regulator_probe(struct platform_device *pdev)
{
	struct rt5033_mfd_chip *chip = dev_get_drvdata(pdev->dev.parent);
	struct rt5033_mfd_platform_data *mfd_pdata = chip->dev->platform_data;
	const struct rt5033_regulator_platform_data* pdata;
	const struct rt5033_pmic_irq_handler *irq_handler = NULL;
	int irq_handler_size = 0;
	struct rt5033_regulator_info *ri;
	struct regulator_dev *rdev;
	struct regulator_init_data* init_data;
	int ret;

	dev_info(&pdev->dev, "Richtek RT5033 regulator driver probing (id = %d)...\n", pdev->id);
	chip_rev = chip->rev_id;
#ifdef CONFIG_OF
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	if (pdev->dev.parent->of_node) {
		pdev->dev.of_node = of_find_compatible_node(
			of_node_get(pdev->dev.parent->of_node), NULL,
			rt5033_regulator_match_table[pdev->id].compatible);
	}
#endif
#endif
	if (pdev->dev.of_node) {
		dev_info(&pdev->dev, "Use DT...\n");
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(3,1,0))
		init_data = of_get_regulator_init_data(&pdev->dev, pdev->dev.of_node);
#else
        init_data = of_get_regulator_init_data(&pdev->dev);
#endif
		if (init_data == NULL) {
			dev_info(&pdev->dev, "Cannot find DTS data...\n");
			init_data = default_rv_pdata.regulator[pdev->id];
		}
	}
	else {
		BUG_ON(mfd_pdata == NULL);
		if (mfd_pdata->regulator_platform_data == NULL)
			mfd_pdata->regulator_platform_data = &default_rv_pdata;
		pdata = mfd_pdata->regulator_platform_data;
		init_data = pdata->regulator[pdev->id];
	}
	ri = find_regulator_info(pdev->id);
	if (ri == NULL) {
		dev_err(&pdev->dev, "invalid regulator ID specified\n");
		return -EINVAL;
	}
	if (init_data == NULL) {
		dev_err(&pdev->dev, "no initializing data\n");
		return -EINVAL;
	}
	ri->i2c = chip->i2c_client;
	ri->chip = chip;
	chip->regulator_info[pdev->id] = ri;

	rdev = rt5033_regulator_register(&ri->desc, &pdev->dev,
				  init_data, ri);
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register regulator %s\n",
				ri->desc.name);
		return PTR_ERR(rdev);
	}
	platform_set_drvdata(pdev, rdev);
    ret = rt5033_regulator_init_regs(rdev);
    if (ret<0)
        goto err_init_device;
	dev_info(&pdev->dev, "RT5033 Regulator %s driver loaded successfully...\n",
			rdev->desc->name);

    switch (pdev->id)
    {
        case RT5033_ID_LDO_SAFE:
            irq_handler = rt5033_pmic_safeldo_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_safeldo_irq_handlers);
            break;
        case RT5033_ID_LDO1:
            irq_handler = rt5033_pmic_ldo_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_ldo_irq_handlers);
            break;
        case RT5033_ID_DCDC1:
            irq_handler = rt5033_pmic_buck_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_buck_irq_handlers);
            break;
        default:
            pr_err("Error : invalid ID\n");
    }
    ret = register_irq(pdev, rdev, irq_handler, irq_handler_size);
    if (ret < 0) {
        pr_err("Error : can't register irq\n");
        goto err_register_irq;
    }
	return 0;
err_register_irq:
err_init_device:
	dev_info(&pdev->dev, "RT5033 Regulator %s unregistered...\n",
			rdev->desc->name);
    regulator_unregister(rdev);
    return ret;
}

static int rt5033_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);
	const struct rt5033_pmic_irq_handler *irq_handler = NULL;
	int irq_handler_size = 0;
	dev_info(&pdev->dev, "RT5033 Regulator %s unregistered...\n",
			rdev->desc->name);
    switch (pdev->id)
    {
        case RT5033_ID_LDO_SAFE:
            irq_handler = rt5033_pmic_safeldo_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_safeldo_irq_handlers);
            break;
        case RT5033_ID_LDO1:
            irq_handler = rt5033_pmic_ldo_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_ldo_irq_handlers);
            break;
        case RT5033_ID_DCDC1:
            irq_handler = rt5033_pmic_buck_irq_handlers;
            irq_handler_size = ARRAY_SIZE(rt5033_pmic_buck_irq_handlers);
            break;
        default:
            pr_err("Error : invalid ID\n");
    }
    unregister_irq(pdev, rdev, irq_handler, irq_handler_size);
	platform_set_drvdata(pdev, NULL);
	regulator_unregister(rdev);
	return 0;
}

static void rt5033_regulator_shutdown(struct device *dev)
{
    struct rt5033_mfd_chip *chip = dev_get_drvdata(dev->parent);
    static int once = ARRAY_SIZE(rt5033_regulator_infos);

    if ( chip_rev >= RT5033A_REV ) {
        once--;
        if ( once==0 ) {
            printk("RT5033#SLDO enable by shutdown\n");
            rt5033_read_dump(chip->i2c_client);
            rt5033_lock_regulator(chip->i2c_client);
            msleep(1);

            // OSC clear, SafeLDO enable
            rt5033_clr_bits(chip->i2c_client, 0x6b, 0x01);
            msleep(1);

            // SafeLDO enable for charger booting
            rt5033_set_bits(chip->i2c_client, RT5033_REGULATOR_REG_OUTPUT_EN, RT5033_REGULATOR_EN_MASK_LDO_SAFE);
            msleep(1);
            rt5033_unlock_regulator(chip->i2c_client);
            rt5033_read_dump(chip->i2c_client);
        }
    }
}

static struct platform_driver rt5033_regulator_driver = {
	.driver		= {
		.name	= "rt5033-regulator",
		.owner	= THIS_MODULE,
		.of_match_table = rt5033_regulator_match_table,
		.shutdown = rt5033_regulator_shutdown,
	},
	.probe		= rt5033_regulator_probe,
	.remove		= rt5033_regulator_remove,
};

static int __init rt5033_regulator_init(void)
{
	return platform_driver_register(&rt5033_regulator_driver);
}
subsys_initcall(rt5033_regulator_init);

static void __exit rt5033_regulator_exit(void)
{
	platform_driver_unregister(&rt5033_regulator_driver);
}
module_exit(rt5033_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick Chang <patrick_chang@richtek.com");
MODULE_VERSION(RT5033_DRV_VER);
MODULE_DESCRIPTION("Regulator driver for RT5033");
MODULE_ALIAS("platform:rt5033-regulator");
