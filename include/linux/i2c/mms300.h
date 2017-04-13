/*
 * mms_ts.h - Platform data for Melfas MMS-series touch driver
 *
 * Copyright (C) 2011 Google Inc.
 * Author: Dima Zavin <dima@android.com>
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#ifndef _LINUX_MMS_TOUCH_H
#define _LINUX_MMS_TOUCH_H
#define MELFAS_TS_NAME			"mms300-ts"
#define MELFAS_CHIP_NAME_345		"mms345"
#define MELFAS_CHIP_NAME_345L		"mms345L"

#undef TOUCHKEY	

#include <linux/input/tsp_ta_callback.h>

#if defined(CONFIG_SEC_A5_PROJECT) || defined(CONFIG_SEC_E5_PROJECT)
#define TSP_RAWDATA_DUMP
#endif

#if defined(CONFIG_SEC_KLEOS_PROJECT)
#define BOOT_VERSION 0x10
#define CORE_VERSION 0x12
#define FW_VERSION 0x09

#elif defined(CONFIG_SEC_E5_PROJECT)
#define BOOT_VERSION 0x06
#define CORE_VERSION 0x32
#define FW_VERSION 0x15

#elif defined(CONFIG_SEC_FORTUNA_PROJECT)
#define BOOT_VERSION 0x06
#define CORE_VERSION 0x26
#define FW_VERSION 0x04

#elif defined(CONFIG_SEC_A5_PROJECT) || defined(CONFIG_SEC_A5_EUR_PROJECT) || defined(CONFIG_SEC_A53G_EUR_PROJECT) || defined(CONFIG_SEC_E7_EUR_PROJECT)
#define BOOT_VERSION 0x06
#define CORE_VERSION 0x26
#define FW_VERSION 0x26

#else
/* hestia */
#define BOOT_VERSION 0x03
#define CORE_VERSION 0x03
#define FW_VERSION 0x04
#endif

struct melfas_tsi_platform_data {
	int max_x;
	int max_y;

	bool invert_x;
	bool invert_y;
	bool i2c_pull_up;
	int gpio_int;
	u32 irq_gpio_flags;
	int gpio_sda;
	u32 sda_gpio_flags;
	int gpio_scl;
	u32 scl_gpio_flags;
	int vdd_en;
	int vdd_en2;
	int tsp_vendor1;
	int tsp_vendor2;
	int tkey_led_en;
	int key1;

	int gpio_seperated;
	int gpio_temp;
	int use_isp;
	int use_isc;

	/*int (*mux_fw_flash) (bool to_gpios);
	int (*is_vdd_on) (void);
	int (*power) (bool on);*/
	const u8 *config_fw_version;
	const char *fw_name;
	bool use_touchkey;
	const u8 *touchkey_keycode;
	void (*input_event) (void *data);

#if defined(USE_TSP_TA_CALLBACKS)
	void (*register_cb) (struct tsp_callbacks *);
	struct tsp_callbacks callbacks;
#endif
	u8 panel;
};
extern struct class *sec_class;

#endif /* _LINUX_MMS_TOUCH_H */
