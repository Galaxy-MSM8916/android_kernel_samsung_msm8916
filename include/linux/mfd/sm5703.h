/*
 * include/linux/mfd/sm5703.h
 *
 * Driver to Siliconmitus SM5703
 * Multi function device -- Charger / Battery Gauge / DCDC Converter / LED Flashlight
 *
 * Copyright (C) 2013 Siliconmitus Technology Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef SM5703_H
#define SM5703_H
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

#define SM5703_DRV_VER "0.0.1"

#define SM5703FG_SLAVE_ADDR (0x35)
#define SM5703CF_SLAVE_ADDR   (0x49)//CHARGER,FLASH,REGULATOR

#define SM5703_INT1_IRQ_REGS_NR 1
#define SM5703_INT2_IRQ_REGS_NR 1
#define SM5703_INT3_IRQ_REGS_NR 1
#define SM5703_INT4_IRQ_REGS_NR 1

#define SM5703_IRQ_REGS_NR \
    (SM5703_INT1_IRQ_REGS_NR + \
     SM5703_INT2_IRQ_REGS_NR + \
     SM5703_INT3_IRQ_REGS_NR + \
     SM5703_INT4_IRQ_REGS_NR)

enum {
	SM5703_ID_USBLDO1 = 0,
    SM5703_ID_USBLDO2,
	SM5703_ID_LDO1,
    SM5703_ID_LDO2,
    SM5703_ID_LDO3,    
	SM5703_ID_BUCK,
	SM5703_MAX_REGULATOR,
};

typedef union sm5703_irq_status {
    struct {
        uint8_t int1_irq_status[SM5703_INT1_IRQ_REGS_NR];
        uint8_t int2_irq_status[SM5703_INT2_IRQ_REGS_NR];
        uint8_t int3_irq_status[SM5703_INT3_IRQ_REGS_NR];
        uint8_t int4_irq_status[SM5703_INT4_IRQ_REGS_NR];
    };
    struct {
        uint8_t regs[SM5703_IRQ_REGS_NR];
    };
} sm5703_irq_status_t;

typedef struct sm5703_regulator_platform_data {
	struct regulator_init_data *regulator[SM5703_MAX_REGULATOR];
} sm5703_regulator_platform_data_t;


struct sm5703_fled_platform_data;

typedef struct sm5703_charger_platform_data {
    sec_charging_current_t *charging_current_table;
    int chg_float_voltage;
    int full_check_type;
    int full_check_type_2nd;
    int chg_autostop;
    int chg_autoset;
    int chg_aiclen;
    int chg_aiclth;
    int chg_vbuslimit;
    int fg_vol_val;
    int fg_soc_val;
    int fg_curr_avr_val;
    char *charger_name;
    int chgen_gpio; //nCHGEN Pin
} sm5703_charger_platform_data_t;

struct sm5703_mfd_platform_data {
	sm5703_regulator_platform_data_t *regulator_platform_data;
	struct sm5703_fled_platform_data *fled_platform_data;
	int irq_gpio;   //irq
	int irq_base;
	int mrstb_gpio;
#ifdef CONFIG_CHARGER_SM5703
    sm5703_charger_platform_data_t *charger_platform_data;
#endif
};

#define sm5703_mfd_platform_data_t \
	struct sm5703_mfd_platform_data

struct sm5703_charger_data;

struct sm5703_mfd_chip {
	struct i2c_client *i2c_client;
	struct device *dev;
	sm5703_mfd_platform_data_t *pdata;
	int irq_base;
	struct mutex io_lock;
	struct mutex irq_lock;
	struct mutex suspend_flag_lock;
	struct wake_lock irq_wake_lock;
	/* prev IRQ status and now IRQ_status*/
	sm5703_irq_status_t irq_status[4];
	/* irq_status_index ^= 0x01; after access irq*/
	int irq_status_index;
	int irq;
	uint8_t irq_masks_cache[SM5703_IRQ_REGS_NR];
	bool suspend_flag;
	bool pending_irq;
	struct sm5703_charger_data *charger;
#ifdef CONFIG_FLED_SM5703
	struct sm5703_fled_info *fled_info;
#endif
#ifdef CONFIG_REGULATOR_SM5703
	struct sm5703_regulator_info *regulator_info[SM5703_MAX_REGULATOR];
#endif
};

#define sm5703_mfd_chip_t \
	struct sm5703_mfd_chip

extern int sm5703_block_read_device(struct i2c_client *i2c,
		int reg, int bytes, void *dest);

extern int sm5703_block_write_device(struct i2c_client *i2c,
		int reg, int bytes, const void *src);

extern int sm5703_reg_read(struct i2c_client *i2c, int reg_addr);
extern int sm5703_reg_write(struct i2c_client *i2c, int reg_addr, unsigned char data);
extern int sm5703_assign_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask,
		unsigned char data);
extern int sm5703_set_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask);
extern int sm5703_clr_bits(struct i2c_client *i2c, int reg_addr, unsigned char mask);

typedef enum {
        SM5703_PREV_STATUS = 0,
        SM5703_NOW_STATUS } sm5703_irq_status_sel_t;

extern sm5703_irq_status_t *sm5703_get_irq_status(sm5703_mfd_chip_t *mfd_chip,
		sm5703_irq_status_sel_t sel);

#endif // SM5703_H
