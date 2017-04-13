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

#ifndef __MFD_LM3632_H__
#define __MFD_LM3632_H__

#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/regmap.h>
#include <linux/regulator/machine.h>
#include <linux/i2c.h>

/* Registers */
#define LM3632_REG_CONFIG1		0x02
#define LM3632_OVP_MASK			(BIT(5) | BIT(6) | BIT(7))
#define LM3632_OVP_25V			BIT(6)

#define LM3632_REG_CONFIG2		0x03
#define LM3632_SWFREQ_MASK		BIT(7)
#define LM3632_SWFREQ_1MHZ		BIT(7)

#define LM3632_REG_BRT_LSB		0x04
#define LM3632_BRT_LSB_MASK		(BIT(0) | BIT(1) | BIT(2))
#define LM3632_REG_BRT_MSB		0x05
#define LM3632_BRT_MSB_SHIFT		3

#define LM3632_REG_IO_CTRL		0x09
#define LM3632_PWM_MASK			BIT(6)
#define LM3632_PWM_SHIFT		6

#define LM3632_REG_ENABLE		0x0A
#define LM3632_BL_EN_MASK		BIT(0)
#define LM3632_BL_EN_SHIFT		0
#define LM3632_BL_STRING_MASK		(BIT(3) | BIT(4))
#define LM3632_BL_ONE_STRING		BIT(4)
#define LM3632_BL_TWO_STRINGS		BIT(3)

#define LM3632_REG_BIAS_CONFIG		0x0C
#define LM3632_EXT_EN_MASK		BIT(0)
#define LM3632_EN_VNEG_MASK		BIT(1)
#define LM3632_EN_VPOS_MASK		BIT(2)

#define LM3632_REG_VOUT_BOOST		0x0D
#define LM3632_REG_VOUT_POS		0x0E
#define LM3632_REG_VOUT_NEG		0x0F
#define LM3632_VOUT_MASK		0x3F

#define LM3632_MAX_REGISTERS		0x10

#define LM3632_NUM_REGULATORS		3

/*
 * struct lm3632_backlight_platform_data
 * @name: Backlight driver name
 * @is_full_strings: set true if two strings are used
 * @pwm_period: Platform specific PWM period value. unit is nano
 */
struct lm3632_backlight_platform_data {
	const char *name;
	bool is_full_strings;

	/* Only valid in case of PWM mode */
	unsigned int pwm_period;

	unsigned	 int gpio_backlight_en;
	u32 en_gpio_flags;

	unsigned	 int gpio_backlight_panel_enp;
	u32 panel_enp_gpio_flags;

	unsigned	 int gpio_backlight_panel_enn;
	u32 panel_enn_gpio_flags;
};

/*
 * struct lm3632_flash_platform_data
 * @name: flash driver name
 * @is_full_strings: set true if two strings are used
 */
struct lm3632_flash_platform_data {
	const char *name;
	bool is_full_strings;

	unsigned int gpio_flash_en;
	u32 flash_en_gpio_flags;

	unsigned int gpio_flash_int;
	u32 flash_int_gpio_flags;
	struct pinctrl *fled_pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
};

/*
 * struct lm3632_platform_data
 * @en_gpio: GPIO for chip enable pin
 * @lcm_en1_gpio: GPIO for VPOS LDO
 * @lcm_en2_gpio: GPIO for VNEG LDO
 * @regulator_data: Regulator initial data for LCD bias
 * @bl_pdata: Backlight platform data
 */
struct lm3632_platform_data {
	int en_gpio;
	int lcm_en1_gpio;
	int lcm_en2_gpio;
	int i2c_scl_gpio;
	int i2c_sda_gpio;
	struct regulator_init_data *regulator_data[LM3632_NUM_REGULATORS];
	struct lm3632_backlight_platform_data *bl_pdata;
	struct lm3632_flash_platform_data *fl_pdata;
};

/*
 * struct lm3632
 * @dev: Parent device pointer
 * @regmap: Used for i2c communcation on accessing registers
 * @pdata: LMU platform specific data
 */
struct lm3632 {
	struct device *dev;
	struct regmap *regmap;
	struct lm3632_platform_data *pdata;
	struct i2c_client *client;
	struct mutex lm3632_lock;
};

int lm3632_read_byte(struct lm3632 *lm3632, u8 reg, u8 *read);
int lm3632_write_byte(struct lm3632 *lm3632, u8 reg, u8 data);
int lm3632_update_bits(struct lm3632 *lm3632, u8 reg, u8 mask, u8 data);
void lm3632_gpio_control(int enable)   ;

#endif
