/* drivers/regulator/sm5703_regulator.c
 * SM5703 Regulator / Buck Driver
 * Copyright (C) 2013
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
#include <linux/mfd/sm5703.h>
#include <linux/mfd/sm5703_irq.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/regulator/of_regulator.h>
#include <linux/battery/charger/sm5703_charger.h>
#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
#include <linux/battery/sec_charger.h>
#endif

#define ALIAS_NAME "sm5703-regulator"

#define EN_BUCK_IRQ 0
#define EN_VDDA_UV_IRQ 0
#define EN_LDO_IRQ 0
#define EN_SLDO_IRQ 0

struct sm5703_regulator_info {
	struct regulator_desc desc;
	struct i2c_client *i2c;
	struct sm5703_mfd_chip	*chip;
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

#define SM5703_REGULATOR_REG_USBLDO1   SM5703_USBLDO1CNTL
//#define SM5703_REGULATOR_SHIFT_USBLDO1 (0)
//#define SM5703_REGULATOR_MASK_USBLDO1  (0x07<<0)
#define SM5703_REGULATOR_EN_REG_USBLDO1   SM5703_USBLDO1CNTL
#define SM5703_REGULATOR_EN_MASK_USBLDO1 (1<<6)

#define SM5703_REGULATOR_REG_USBLDO2   SM5703_USBLDO2CNTL
//#define SM5703_REGULATOR_SHIFT_USBLDO2 (0)
//#define SM5703_REGULATOR_MASK_USBLDO2  (0x07<<0)
#define SM5703_REGULATOR_EN_REG_USBLDO2   SM5703_USBLDO2CNTL
#define SM5703_REGULATOR_EN_MASK_USBLDO2 (1<<7)

#define SM5703_REGULATOR_REG_LDO1   SM5703_LDOOUT1CNTL
#define SM5703_REGULATOR_SHIFT_LDO1 (0)
#define SM5703_REGULATOR_MASK_LDO1  (0x07<<0)
#define SM5703_REGULATOR_EN_REG_LDO1   SM5703_LDOOUT1CNTL
#define SM5703_REGULATOR_EN_MASK_LDO1 (1<<3)

#define SM5703_REGULATOR_REG_LDO2   SM5703_LDOOUT2CNTL
#define SM5703_REGULATOR_SHIFT_LDO2 (0)
#define SM5703_REGULATOR_MASK_LDO2  (0x07<<0)
#define SM5703_REGULATOR_EN_REG_LDO2   SM5703_LDOOUT2CNTL
#define SM5703_REGULATOR_EN_MASK_LDO2 (1<<3)

#define SM5703_REGULATOR_REG_LDO3   SM5703_LDOOUT3CNTL
#define SM5703_REGULATOR_SHIFT_LDO3 (0)
#define SM5703_REGULATOR_MASK_LDO3  (0x07<<0)
#define SM5703_REGULATOR_EN_REG_LDO3   SM5703_LDOOUT3CNTL
#define SM5703_REGULATOR_EN_MASK_LDO3 (1<<3)

#define SM5703_REGULATOR_REG_BUCK   SM5703_BUCKCNTL
#define SM5703_REGULATOR_SHIFT_BUCK (0)
#define SM5703_REGULATOR_MASK_BUCK  (0x1F<<0)
#define SM5703_REGULATOR_EN_REG_BUCK   SM5703_BUCKCNTL
#define SM5703_REGULATOR_EN_MASK_BUCK (1<<6)

static const unsigned int sm5703_usbldo_output_list [] = {
	4800*1000,
};

static const unsigned int sm5703_ldo_output_list [] = {
	1500*1000,
	1800*1000,
	2600*1000,
	2800*1000,
	3000*1000,
	3300*1000,
	3300*1000,
	3300*1000,
};

static const unsigned int sm5703_dcdc_output_list[] = {
	1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,
    1000*1000,    
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
    3000*1000,	
};


#define SM5703_REGULATOR_DECL(_id, min, max,out_list)   \
{								                        \
	.desc	= {						                    \
		.name	= "SM5703_REGULATOR" #_id,				\
		.ops	= &sm5703_regulator_ldo_dcdc_ops,		\
		.type	= REGULATOR_VOLTAGE,			        \
		.id	= SM5703_ID_##_id,			                \
		.owner	= THIS_MODULE,				            \
		.n_voltages = ARRAY_SIZE(out_list),             \
	},							                        \
	.min_uV		= min * 1000,				            \
	.max_uV		= max * 1000,				            \
	.vol_reg	= SM5703_REGULATOR_REG_##_id,           \
	.vol_shift	= SM5703_REGULATOR_SHIFT_##_id,         \
	.vol_mask	= SM5703_REGULATOR_MASK_##_id,          \
	.enable_reg	= SM5703_REGULATOR_EN_REG_##_id,		\
	.enable_bit	= SM5703_REGULATOR_EN_MASK_##_id,		\
	.output_list = out_list,                            \
	.output_list_count = ARRAY_SIZE(out_list),          \
}

#define SM5703_REGULATOR_DECL_USBLDO(_id, min, max,out_list)   \
{								                        \
	.desc	= {						                    \
		.name	= "SM5703_REGULATOR" #_id,				\
		.ops	= &sm5703_regulator_usbldo_ops,		    \
		.type	= REGULATOR_VOLTAGE,			        \
		.id	= SM5703_ID_##_id,			                \
		.owner	= THIS_MODULE,				            \
		.n_voltages = ARRAY_SIZE(out_list),             \
	},							                        \
	.min_uV		= min * 1000,				            \
	.max_uV		= max * 1000,				            \
	.enable_reg	= SM5703_REGULATOR_EN_REG_##_id,		\
	.enable_bit	= SM5703_REGULATOR_EN_MASK_##_id,		\
	.output_list = out_list,                            \
	.output_list_count = ARRAY_SIZE(out_list),          \
}


static inline int sm5703_regulator_check_range(struct sm5703_regulator_info *info,
		int min_uV, int max_uV)
{
	if (min_uV < info->min_uV || max_uV > info->max_uV)
		return -EINVAL;

	return 0;
}

static int sm5703_regulator_list_voltage(struct regulator_dev *rdev, unsigned index)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);

	return (index>=info->output_list_count)?
		-EINVAL: info->output_list[index];
}


#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,39))
int sm5703_regulator_set_voltage_sel(struct regulator_dev *rdev, unsigned selector)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;
	int ret;

	pr_info("%s select = %d, output list count = %d\n",
			ALIAS_NAME, selector, info->output_list_count);
	if (selector>=info->output_list_count)
		return -EINVAL;
	pr_info("%s Vout = %d\n", ALIAS_NAME, info->output_list[selector]);
	data = (unsigned char)selector;
	data <<= info->vol_shift;
	ret = sm5703_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);

	pr_info("%s %s %s ret (%d)", ALIAS_NAME, rdev->desc->name, __func__, ret);

	return ret;
}

int sm5703_usbldo_regulator_set_voltage_sel(struct regulator_dev *rdev, unsigned selector)
{
	pr_info("%s %s %s", ALIAS_NAME, rdev->desc->name, __func__);

	return 0;

}

#endif

static int sm5703_regulator_get_voltage_sel(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	ret = sm5703_reg_read(info->i2c, info->vol_reg);
	if (ret < 0)
		return ret;
	return (ret & info->vol_mask)  >> info->vol_shift;
}

static int sm5703_usbldo_regulator_get_voltage_sel(struct regulator_dev *rdev)
{
    pr_info("%s %s %s", ALIAS_NAME, rdev->desc->name, __func__);

    return 0;
}

#if (LINUX_VERSION_CODE<KERNEL_VERSION(2,6,39))
static int sm5703_regulator_find_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV)
{
	int i=0;
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
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
static int sm5703_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,unsigned *selector)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;

	if (sm5703_regulator_check_range(info, min_uV, max_uV)) {
		pr_err("%s %s invalid voltage range (%d, %d) uV\n",
				ALIAS_NAME, rdev->desc->name, min_uV, max_uV);
		return -EINVAL;
	}
	*selector = sm5703_regulator_find_voltage(rdev,min_uV,max_uV);
	data = *selector << info->vol_shift;

	return sm5703_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);
}

static int sm5703_usbldo_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,unsigned *selector)
{
    pr_info("%s %s %s", ALIAS_NAME, rdev->desc->name, __func__);

    return 0;
}

#else
static int sm5703_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,int *selector)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	unsigned char data;
	int ret;

	if (sm5703_regulator_check_range(info, min_uV, max_uV)) {
		pr_err("%s %s invalid voltage range (%d, %d) uV\n",
				ALIAS_NAME, rdev->desc->name, min_uV, max_uV);
		return -EINVAL;
	}
	data = sm5703_regulator_find_voltage(rdev,min_uV,max_uV);
	data <<= info->vol_shift;
	ret = sm5703_assign_bits(info->i2c, info->vol_reg, info->vol_mask, data);

	pr_info("%s %s ret (%d)", ALIAS_NAME, __func__, ret);

	return ret;
}

static int sm5703_usbldo_regulator_set_voltage(struct regulator_dev *rdev,
		int min_uV, int max_uV,int *selector)
{
    pr_info("%s %s %s", ALIAS_NAME, rdev->desc->name, __func__);

    return 0;
}

#endif //(LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))

static int sm5703_regulator_get_voltage(struct regulator_dev *rdev)
{
	int ret;
	ret = sm5703_regulator_get_voltage_sel(rdev);
	if (ret < 0)
		return ret;
	return sm5703_regulator_list_voltage(rdev, ret);
}

static int sm5703_usbldo_regulator_get_voltage(struct regulator_dev *rdev)
{
    pr_info("%s %s %s", ALIAS_NAME, rdev->desc->name, __func__);

    return sm5703_regulator_list_voltage(rdev, 0);
}

#endif //(LINUX_VERSION_CODE<KERNEL_VERSION(2,6,39))

#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
static int sm5703_regulator_enable(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	union power_supply_propval value;

	pr_info("%s Enable regulator %s\n", ALIAS_NAME, rdev->desc->name);
	pr_info("%s desc.id = %d\n", __func__,info->desc.id);

	ret = sm5703_set_bits(info->i2c, info->enable_reg, info->enable_bit);

	if (info->desc.id == SM5703_ID_LDO1)
	{
		value.intval = 5;
		psy_do_property("sm5703-charger", set,
				POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		pr_info("%s %s input current",rdev->desc->name, __func__);
	}
	pr_info("%s %s %s ret (%d)", ALIAS_NAME, rdev->desc->name, __func__, ret);

	return ret;
}

static int sm5703_regulator_disable(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	union power_supply_propval value;

	pr_info("%s Disable regulator %s\n", ALIAS_NAME, rdev->desc->name);
	pr_info("%s desc.id = %d\n", __func__,info->desc.id);

	ret = sm5703_clr_bits(info->i2c, info->enable_reg, info->enable_bit);

	if (info->desc.id == SM5703_ID_LDO1)
	{
		value.intval = 6;
		psy_do_property("sm5703-charger", set,
				POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		pr_info("%s %s input current",rdev->desc->name, __func__);
	}
	pr_info("%s %s ret (%d)", ALIAS_NAME, __func__, ret);

	return ret;
}
#else
static int sm5703_regulator_enable(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;
	pr_info("%s Enable regulator %s\n", ALIAS_NAME, rdev->desc->name);
	ret = sm5703_set_bits(info->i2c, info->enable_reg,
			info->enable_bit);
	pr_info("%s %s %s ret (%d)", ALIAS_NAME, rdev->desc->name, __func__, ret);

	return ret;
}

static int sm5703_regulator_disable(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;

	pr_info("%s Disable regulator %s\n", ALIAS_NAME, rdev->desc->name);
	ret = sm5703_clr_bits(info->i2c, info->enable_reg,
			info->enable_bit);
	pr_info("%s %s ret (%d)", ALIAS_NAME, __func__, ret);

	return ret;
}
#endif
static int sm5703_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct sm5703_regulator_info *info = rdev_get_drvdata(rdev);
	int ret;

	ret = sm5703_reg_read(info->i2c, info->enable_reg);
	if (ret < 0)
		return ret;

	ret = (ret & (info->enable_bit))?1:0;
	pr_info("%s %s %s ret (%d)", ALIAS_NAME, rdev->desc->name, __func__, ret);
	return ret;
}

static struct regulator_ops sm5703_regulator_ldo_dcdc_ops = {
	.list_voltage		= sm5703_regulator_list_voltage,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,39))
	.get_voltage_sel	= sm5703_regulator_get_voltage_sel,
	.set_voltage_sel	= sm5703_regulator_set_voltage_sel,
#else
	.set_voltage		= sm5703_regulator_set_voltage,
	.get_voltage		= sm5703_regulator_get_voltage,
#endif
	.enable			= sm5703_regulator_enable,
	.disable		= sm5703_regulator_disable,
	.is_enabled		= sm5703_regulator_is_enabled,
};

static struct regulator_ops sm5703_regulator_usbldo_ops = {
	.list_voltage		= sm5703_regulator_list_voltage,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,39))
    .get_voltage_sel    = sm5703_usbldo_regulator_get_voltage_sel,
    .set_voltage_sel    = sm5703_usbldo_regulator_set_voltage_sel,
#else
    .set_voltage        = sm5703_usbldo_regulator_set_voltage,
    .get_voltage        = sm5703_usbldo_regulator_get_voltage,
#endif

	.enable			= sm5703_regulator_enable,
	.disable		= sm5703_regulator_disable,
	.is_enabled		= sm5703_regulator_is_enabled,
};

static struct sm5703_regulator_info sm5703_regulator_infos[] = {
    SM5703_REGULATOR_DECL_USBLDO(USBLDO1, 4800, 4800, sm5703_usbldo_output_list),
    SM5703_REGULATOR_DECL_USBLDO(USBLDO2, 4800, 4800, sm5703_usbldo_output_list),
	SM5703_REGULATOR_DECL(LDO1, 1500, 3300, sm5703_ldo_output_list),
	SM5703_REGULATOR_DECL(LDO2, 1500, 3300, sm5703_ldo_output_list),
	SM5703_REGULATOR_DECL(LDO3, 1500, 3300, sm5703_ldo_output_list),	
	SM5703_REGULATOR_DECL(BUCK, 1000, 3000, sm5703_dcdc_output_list),
};

static struct sm5703_regulator_info * find_regulator_info(int id)
{
	struct sm5703_regulator_info *ri;
	int i;

	for (i = 0; i < ARRAY_SIZE(sm5703_regulator_infos); i++) {
		ri = &sm5703_regulator_infos[i];
		if (ri->desc.id == id)
			return ri;
	}
	return NULL;
}

inline struct regulator_dev* sm5703_regulator_register(struct regulator_desc *regulator_desc,
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

static int sm5703_regulator_init_regs(struct regulator_dev* rdev)
{
	return 0;
}

static struct regulator_consumer_supply default_sm5703_usbldo1_consumers[] = {
	REGULATOR_SUPPLY("sm5703_usbldo1",NULL),
};
static struct regulator_consumer_supply default_sm5703_usbldo2_consumers[] = {
	REGULATOR_SUPPLY("sm5703_usbldo2",NULL),
};
static struct regulator_consumer_supply default_sm5703_ldo1_consumers[] = {
	REGULATOR_SUPPLY("sm5703_ldo1",NULL),
};
static struct regulator_consumer_supply default_sm5703_ldo2_consumers[] = {
	REGULATOR_SUPPLY("sm5703_ldo2",NULL),
};
static struct regulator_consumer_supply default_sm5703_ldo3_consumers[] = {
	REGULATOR_SUPPLY("sm5703_ldo3",NULL),
};
static struct regulator_consumer_supply default_sm5703_buck_consumers[] = {
	REGULATOR_SUPPLY("sm5703_buck",NULL),
};

static struct regulator_init_data default_sm5703_usbldo1_data = {
	.constraints = {
		.min_uV = 4800000,
		.max_uV = 4800000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_usbldo1_consumers),
	.consumer_supplies = default_sm5703_usbldo1_consumers,
};
static struct regulator_init_data default_sm5703_usbldo2_data = {
	.constraints = {
		.min_uV = 4800000,
		.max_uV = 4800000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_usbldo2_consumers),
	.consumer_supplies = default_sm5703_usbldo2_consumers,
};
static struct regulator_init_data default_sm5703_ldo1_data = {
	.constraints = {
		.min_uV = 1500000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_ldo1_consumers),
	.consumer_supplies = default_sm5703_ldo1_consumers,
};
static struct regulator_init_data default_sm5703_ldo2_data = {
	.constraints = {
		.min_uV = 1500000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_ldo2_consumers),
	.consumer_supplies = default_sm5703_ldo2_consumers,
};
static struct regulator_init_data default_sm5703_ldo3_data = {
	.constraints = {
		.min_uV = 1500000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_ldo3_consumers),
	.consumer_supplies = default_sm5703_ldo3_consumers,
};
static struct regulator_init_data default_sm5703_buck_data = {
	.constraints = {
		.min_uV = 1000000,
		.max_uV = 3000000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL | REGULATOR_MODE_IDLE,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(default_sm5703_buck_consumers),
	.consumer_supplies = default_sm5703_buck_consumers,
};

static struct sm5703_regulator_platform_data default_rv_pdata = {
	.regulator = {
        [SM5703_ID_USBLDO1] = &default_sm5703_usbldo1_data,
        [SM5703_ID_USBLDO2] = &default_sm5703_usbldo2_data,
		[SM5703_ID_LDO1] = &default_sm5703_ldo1_data,
		[SM5703_ID_LDO2] = &default_sm5703_ldo2_data,
		[SM5703_ID_LDO3] = &default_sm5703_ldo3_data,		
		[SM5703_ID_BUCK] = &default_sm5703_buck_data,
	},
};


#ifdef CONFIG_OF
static struct of_device_id sm5703_regulator_match_table[] = {
	{ .compatible = "siliconmitus,sm5703-usbldo1",},
    { .compatible = "siliconmitus,sm5703-usbldo2",},
	{ .compatible = "siliconmitus,sm5703-ldo1",},
	{ .compatible = "siliconmitus,sm5703-ldo2",},
	{ .compatible = "siliconmitus,sm5703-ldo3",},	
	{ .compatible = "siliconmitus,sm5703-dcdc",},
	{},
};
#else
#define sm5703_regulator_match_table NULL
#endif

static int sm5703_regulator_probe(struct platform_device *pdev)
{
	struct sm5703_mfd_chip *chip = dev_get_drvdata(pdev->dev.parent);
	struct sm5703_mfd_platform_data *mfd_pdata = chip->dev->platform_data;
	const struct sm5703_regulator_platform_data* pdata;
	//const struct sm5703_pmic_irq_handler *irq_handler = NULL;
	struct sm5703_regulator_info *ri;
	struct regulator_dev *rdev;
	struct regulator_init_data* init_data;
	int ret;
//	dev_info(&pdev->dev, "Siliconmitus SM5703 regulator driver probing (id = %d)...\n", pdev->id);
	if (pdev->dev.of_node) {
//		dev_info(&pdev->dev, "Use DT...\n");
		init_data = of_get_regulator_init_data(&pdev->dev, pdev->dev.of_node);
		if (init_data == NULL) {
			dev_info(&pdev->dev, "Cannot find DTS data...\n");
			init_data = default_rv_pdata.regulator[pdev->id];
		}
	} else {
		BUG_ON(mfd_pdata == NULL);
		if (mfd_pdata->regulator_platform_data == NULL)
			mfd_pdata->regulator_platform_data = &default_rv_pdata;
		pdata = mfd_pdata->regulator_platform_data;
		init_data = pdata->regulator[pdev->id];
	}

	if (init_data == NULL) {
		dev_err(&pdev->dev, "no initializing data\n");
		return -EINVAL;
	}
	ri = find_regulator_info(pdev->id);
	if (ri == NULL) {
		dev_err(&pdev->dev, "invalid regulator ID specified\n");
		return -EINVAL;
	}

	ri->desc.name = init_data->constraints.name;
	ri->i2c = chip->i2c_client;
	ri->chip = chip;
	chip->regulator_info[pdev->id] = ri;

	rdev = sm5703_regulator_register(&ri->desc, &pdev->dev,
				  init_data, ri);
	if (IS_ERR(rdev)) {
		dev_err(&pdev->dev, "failed to register regulator %s\n",
				ri->desc.name);
		return PTR_ERR(rdev);
	}
	platform_set_drvdata(pdev, rdev);
    ret = sm5703_regulator_init_regs(rdev);
    if (ret<0)
        goto err_init_device;
//	dev_dvg(&pdev->dev, "SM5703 Regulator %s driver loaded successfully...\n", rdev->desc->name); 

	return 0;
//err_register_irq:
err_init_device:
	dev_info(&pdev->dev, "SM5703 Regulator %s unregistered...\n",
			rdev->desc->name);
    regulator_unregister(rdev);
    return ret;
}

static int sm5703_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = platform_get_drvdata(pdev);
	//const struct sm5703_pmic_irq_handler *irq_handler = NULL;
	//int irq_handler_size = 0;
	dev_info(&pdev->dev, "SM5703 Regulator %s unregistered...\n",
			rdev->desc->name);

	platform_set_drvdata(pdev, NULL);
	regulator_unregister(rdev);

	return 0;
}

static struct platform_driver sm5703_regulator_driver = {
	.driver		= {
		.name	= "sm5703-regulator",
		.owner	= THIS_MODULE,
		.of_match_table = sm5703_regulator_match_table,
	},
	.probe		= sm5703_regulator_probe,
	.remove		= sm5703_regulator_remove,
};

static int __init sm5703_regulator_init(void)
{
	return platform_driver_register(&sm5703_regulator_driver);
}
subsys_initcall(sm5703_regulator_init);

static void __exit sm5703_regulator_exit(void)
{
	platform_driver_unregister(&sm5703_regulator_driver);
}
module_exit(sm5703_regulator_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION(SM5703_DRV_VER);
MODULE_DESCRIPTION("Regulator driver for SM5703");
MODULE_ALIAS("platform:sm5703-regulator");
