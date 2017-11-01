/*
 * CORERIVER TOUCHCORE 360L touchkey driver
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 * Author: Taeyoon Yoon <tyoony.yoon@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __LINUX_TC360_H
#define __LINUX_TC360_H

#define TC360_DEVICE	"sec_touchkey"

#if defined(CONFIG_SEC_SERRANOVE_PROJECT)
#define	SUPPORT_MULTI_PCB	1
#else
#define	SUPPORT_MULTI_PCB	0
#endif

extern int touch_is_pressed;
#if defined(CONFIG_MACH_GOLDEN)
extern unsigned int system_rev;
#else
extern int system_rev;
#endif

enum {
	TC360_SUSPEND_WITH_POWER_OFF = 0,
	TC360_SUSPEND_WITH_SLEEP_CMD,
};

struct tc360_platform_data {
	u8	enable;
	u32	gpio_scl;
	u32	gpio_sda;
	u32	gpio_int;
	u32 gpio_en;
	u32 irq_gpio_flags;
	u32 sda_gpio_flags;
	u32 scl_gpio_flags;
	u32 vcc_gpio_flags;
	u32 gpio_2p8_en;	
	u32 vcc_gpio2p8_flags;	
	int	udelay;
	int	num_key;
	int	*keycodes;
	u8	suspend_type;
	u8	exit_flag;
	bool firmup;
	const char *vcc_en_ldo_name;
	const char *vdd_led_ldo_name;
};

#define SEC_FAC_TK
#if defined(SEC_FAC_TK)
extern struct class *sec_class;
#endif

#endif /* __LINUX_TC360_H */
