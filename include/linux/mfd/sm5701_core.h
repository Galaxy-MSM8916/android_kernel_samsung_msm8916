/*
 * SM5701_core.h
 * SiliconMitus SM5701 Charger Header
 *
 * Copyright (C) 2014 SiliconMitus
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _sm5701_SW_H_
#define _sm5701_SW_H_

#include <linux/i2c.h>

#if defined(CONFIG_CHARGER_SM5701)
#include <linux/battery/sec_charger.h>
#endif

#define MFD_DEV_NAME "sm5701"

/* Enable charger Operation Mode */
#define OP_MODE_CHG_ON 0x02
#define OP_MODE_CHG_ON_REV3 0x04

/* Disable charger Operation Mode */
#define OP_MODE_CHG_OFF 0x01


//FLEDCNTL1
#define SM5701_FLEDCNTL1_FLEDEN			0x03
#define SM5701_FLEDCNTL1_ABSTMR			0x0B
#define SM5701_FLEDCNTL1_ENABSTMR		0x10
//FLEDCNTL2
#define SM5701_FLEDCNTL2_ONETIMER		0x0F
#define SM5701_FLEDCNTL2_nONESHOT		0x10
#define SM5701_FLEDCNTL2_SAFET			0x60
#define SM5701_FLEDCNTL2_nENSAFET		0x80
//FLEDCNTL3
#define SM5701_FLEDCNTL3_IFLED			0x1F
//FLEDCNTL4
#define SM5701_FLEDCNTL4_IMLED			0x1F
//FLEDCNTL5
#define SM5701_FLEDCNTL5_LBDHYS			0x03
#define SM5701_FLEDCNTL5_LOWBATT		0x1B
#define SM5701_FLEDCNTL5_LBRSTIMER		0x60
#define SM5701_FLEDCNTL5_ENLOWBATT		0x80
//FLEDCNTL6
#define SM5701_FLEDCNTL6_BSTOUT			0x0F
#define SM5701_BSTOUT_4P5               0x05
#define SM5701_BSTOUT_5P0               0x0A

/**
 * struct sm5701_dev
 * @dev: master device of the chip (can be used to access platform data)
 * @i2c: i2c client private data
 * @iolock: mutex for serializing io access
 * @irq_base: base IRQ number for sm5701, required for IRQs
 * @irq: generic IRQ number for sm5701
 */
struct SM5701_dev {
	struct device *dev;
	struct i2c_client *i2c;
	struct mutex i2c_lock;
	struct mutex irqlock;

	int type;

	int irq;
	int irq_base;
	int irq_gpio;
	int irqf_trigger;
	bool wakeup;
};


struct SM5701_charger_data {
	struct SM5701_dev *SM5701;
	struct power_supply	psy_chg;
	struct delayed_work	isr_work;

	unsigned int	is_charging;
	unsigned int	nchgen;
	unsigned int	charging_type;
	unsigned int	battery_state;
	unsigned int	battery_present;
	unsigned int	cable_type;
	unsigned int	charging_current_max;
	unsigned int	charging_current;
	unsigned int	input_current_limit;
	unsigned int	vbus_state;
	bool is_fullcharged;
	int		aicl_on;
	int     slow_rate_on;
	int		status;
	int siop_level;
	int		input_curr_limit_step;
	int		charging_curr_step;
        int             dev_id;

	sec_battery_platform_data_t	*pdata;
};

struct SM5701_platform_data {
	unsigned int irq;
	unsigned int chgen;
	struct SM5701_charger_data *charger_data;
};

//CNTL
#define SM5701_OPERATIONMODE_SUSPEND                0x0 // 000 : Suspend (charger-OFF) MODE
#define SM5701_OPERATIONMODE_FLASH_ON               0x1 // 001 : Flash LED Driver=ON Ready in Charger & OTG OFF Mode
#define SM5701_OPERATIONMODE_OTG_ON                 0x2 // 010 : OTG=ON in Charger & Flash OFF Mode
#define SM5701_OPERATIONMODE_OTG_ON_FLASH_ON        0x3 // 011 : OTG=ON & Flash LED Driver=ON Ready in charger OFF Mode
#define SM5701_OPERATIONMODE_CHARGER_ON             0x4 // 100 : Charger=ON in OTG & Flash OFF Mode. Same as 0x6(110)
#define SM5701_OPERATIONMODE_CHARGER_ON_FLASH_ON    0x5 // 101 : Charger=ON & Flash LED Driver=ON Ready in OTG OFF Mode. Same as 0x7(111)

enum led_state {
    LED_DISABLE,
	LED_MOVIE,
	LED_FLASH,

};

enum sm5701_flash_mode {
			NONE_MODE = 0,
			MOVIE_MODE,
			FLASH_MODE,
	};

enum sm5701_flash_operation {

		SM5701_FLEDEN_DISABLED = 0,
		SM5701_FLEDEN_ON_MOVIE,
		SM5701_FLEDEN_ON_FLASH,
		SM5701_FLEDEN_EXTERNAL_CONTOL
};

//sm5701_core.c
extern int SM5701_reg_read(struct i2c_client *i2c, u8 reg, u8 *dest);
extern int SM5701_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf);
extern int SM5701_reg_write(struct i2c_client *i2c, u8 reg, u8 value);
extern int SM5701_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf);
extern int SM5701_reg_update(struct i2c_client *i2c, u8 reg, u8 val, u8 mask);
extern void SM5701_test_read(struct i2c_client *client );
extern void SM5701_set_operationmode(int operation_mode);
extern void sm5701_led_ready(int led_status);
extern int SM5701_operation_mode_function_control(void);
extern void SM5701_set_charger_data(void *p);

//leds-sm5701.c
extern void SM5701_set_enabstmr(int abstmr_enable);
extern void SM5701_set_abstmr(int abstmr_sec);
extern void SM5701_set_fleden(int fled_enable);
extern void SM5701_set_nensafet(int nensafet_enable);
extern void SM5701_set_safet(int safet_us);
extern void SM5701_set_noneshot(int noneshot_enable);
extern void SM5701_set_onetimer(int onetimer_ms);
extern void SM5701_set_ifled(int ifled_ma);
extern void SM5701_set_imled(int imled_ma);
extern void SM5701_set_enlowbatt(int enlowbatt_enable);
extern void SM5701_set_lbrstimer(int lbrstimer_us);
extern void SM5701_set_lowbatt(int lowbatt_v);
extern void SM5701_set_lbdhys(int lbdhys_mv);
extern void SM5701_set_bstout(int bstout_mv);
extern void sm5701_set_fleden(int fled_enable);

//sm5701_irq
extern int SM5701_irq_init(struct SM5701_dev *sm5701);
extern void SM5701_irq_exit(struct SM5701_dev *sm5701);

#endif /* __SM5701_CHARGER_H */
