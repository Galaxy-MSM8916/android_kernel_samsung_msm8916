/*
 * include/linux/mfd/rt5033.h
 *
 * Driver to Richtek RT5033
 * Multi function device -- Charger / Battery Gauge / DCDC Converter / LED Flashlight
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef RT5033_H
#define RT5033_H
/* delete below header caused it is only for this module*/
/*#include <linux/rtdefs.h> */

#include <linux/mutex.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/battery/sec_charging_common.h>

#define RT5033_DRV_VER "1.2.2_S"

#ifdef CONFIG_RT5033_SADDR
#define RT5033FG_SLAVE_ADDR_MSB (0x40)
#else
#define RT5033FG_SLAVE_ADDR_MSB (0x00)
#endif

#define RT5033FG_SLAVE_ADDR (0x35|RT5033FG_SLAVE_ADDR_MSB)
#define RT5033_SLAVE_ADDR   (0x34|RT5033FG_SLAVE_ADDR_MSB)

#define RT5033_CHG_IRQ_REGS_NR 3
#define RT5033_LED_IRQ_REGS_NR 1
#define RT5033_PMIC_IRQ_REGS_NR 1

#define RT5033_IRQ_REGS_NR \
    (RT5033_CHG_IRQ_REGS_NR + \
     RT5033_LED_IRQ_REGS_NR + \
     RT5033_PMIC_IRQ_REGS_NR)

enum {
	RT5033_ID_LDO_SAFE = 0,
	RT5033_ID_LDO1,
	RT5033_ID_DCDC1,
	RT5033_MAX_REGULATOR,
};

typedef union rt5033_irq_status {
    struct {
        uint8_t chg_irq_status[RT5033_CHG_IRQ_REGS_NR];
        uint8_t fled_irq_status[RT5033_LED_IRQ_REGS_NR];
        uint8_t pmic_irq_status[RT5033_PMIC_IRQ_REGS_NR];
    };
    struct {
        uint8_t regs[RT5033_IRQ_REGS_NR];
    };
} rt5033_irq_status_t;

typedef union rt5033_pmic_shdn_ctrl {
    struct {
        uint8_t reserved : 2;
        uint8_t buck_ocp_enshdn : 1;
        uint8_t buck_lv_enshdn : 1;
        uint8_t sldo_lv_enshdn : 1;
        uint8_t ldo_lv_enshdn : 1;
        uint8_t ot_enshdn : 1;
        uint8_t vdda_uv_enshdn : 1;
        } shdn_ctrl1;
    uint8_t shdn_ctrl[1];

} rt5033_pmic_shdn_ctrl_t;


typedef struct rt5033_regulator_platform_data {
	struct regulator_init_data *regulator[RT5033_MAX_REGULATOR];
} rt5033_regulator_platform_data_t;


struct rt5033_fled_platform_data;

typedef struct rt5033_charger_platform_data {
    sec_charging_current_t *charging_current_table;
    int chg_float_voltage;
    char *charger_name;
    bool dualized_charging_current;
    /* 1st full check */
    sec_battery_full_charged_t full_check_type;
    /* 2nd full check */
    sec_battery_full_charged_t full_check_type_2nd;
    uint32_t is_750kHz_switching : 1;
    uint32_t is_fixed_switching : 1;
} rt5033_charger_platform_data_t;

struct rt5033_mfd_platform_data {
	rt5033_regulator_platform_data_t *regulator_platform_data;
	struct rt5033_fled_platform_data *fled_platform_data;
	int irq_gpio;
	int irq_base;
#ifdef CONFIG_MFD_RT5033_EN_MRSTB
	/* MRSTB function to reset charger */
	int mrstb_gpio;
#endif /* CONFIG_MFD_RT5033_EN_MRSTB */
#ifdef CONFIG_CHARGER_RT5033
    rt5033_charger_platform_data_t *charger_platform_data;
#endif
};

#define rt5033_mfd_platform_data_t \
	struct rt5033_mfd_platform_data

struct rt5033_charger_data;

struct rt5033_mfd_chip {
	struct i2c_client *i2c_client;
	struct device *dev;
	rt5033_mfd_platform_data_t *pdata;
	int irq_base;
	struct mutex io_lock;
	struct mutex regulator_lock;
	struct mutex irq_lock;
	struct mutex suspend_flag_lock;
	struct wake_lock irq_wake_lock;
	/* prev IRQ status and now IRQ_status*/
	rt5033_irq_status_t irq_status[2];
	/* irq_status_index ^= 0x01; after access irq*/
	int irq_status_index;
	int irq;
	uint8_t irq_masks_cache[RT5033_IRQ_REGS_NR];
	bool suspend_flag;
	bool pending_irq;
	struct rt5033_charger_data *charger;
#ifdef CONFIG_FLED_RT5033
	struct rt5033_fled_info *fled_info;
#endif
#ifdef CONFIG_REGULATOR_RT5033
	bool regulator_states[RT5033_MAX_REGULATOR];
	struct rt5033_regulator_info *regulator_info[RT5033_MAX_REGULATOR];
#endif
	int rev_id;
};

#define rt5033_mfd_chip_t \
	struct rt5033_mfd_chip

extern int rt5033_block_read_device(struct i2c_client *i2c,
		int reg, int bytes, void *dest);

extern int rt5033_block_write_device(struct i2c_client *i2c,
		int reg, int bytes, const void *src);

extern int rt5033_reg_read(struct i2c_client *i2c, int reg_addr);
extern int rt5033_reg_write(struct i2c_client *i2c, int reg_addr, unsigned char data);
extern int rt5033_assign_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask,
		unsigned char data);
extern int rt5033_set_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask);
extern int rt5033_clr_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask);
extern void rt5033_lock_regulator(struct i2c_client *i2c);
extern void rt5033_unlock_regulator(struct i2c_client *i2c);

#ifdef CONFIG_REGULATOR_RT5033
extern void rt5033_set_regulator_state(struct i2c_client *i2c, int id, bool en);
extern bool rt5033_get_pmic_state(struct i2c_client *i2c);
#endif

void rt5033_read_dump(struct i2c_client *i2c);
void rt5033_workaround(rt5033_mfd_chip_t *chip);

typedef enum {
        RT5033_PREV_STATUS = 0,
        RT5033_NOW_STATUS } rt5033_irq_status_sel_t;

extern rt5033_irq_status_t *rt5033_get_irq_status(rt5033_mfd_chip_t *mfd_chip,
		rt5033_irq_status_sel_t sel);

#endif // RT5033_H
