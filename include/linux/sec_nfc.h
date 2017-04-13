/*
 * SAMSUNG NFC N2 Controller
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Author: Woonki Lee <woonki84.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 */

#include <linux/platform_device.h>
#ifdef CONFIG_SEC_NFC_CLK_REQ
#include <linux/clk.h>
#endif

#ifdef CONFIG_SEC_NFC_I2C /* support old driver configuration */

    #ifndef CONFIG_SEC_NFC
        #define CONFIG_SEC_NFC
    #endif

    #if !defined(CONFIG_SEC_NFC_IF_I2C) && !defined(CONFIG_SEC_NFC_IF_UART)
        #define CONFIG_SEC_NFC_IF_I2C
    #endif

    #if !defined(CONFIG_SEC_NFC_PRODUCT_N3) && !defined(CONFIG_SEC_NFC_PRODUCT_N5)
        #define CONFIG_SEC_NFC_PRODUCT_N3
    #endif

#endif /* CONFIG_SEC_NFC_I2C */

#define SEC_NFC_DRIVER_NAME		"sec-nfc"

#define SEC_NFC_MAX_BUFFER_SIZE	512

/* ioctl */
#define SEC_NFC_MAGIC	'S'
#define SEC_NFC_GET_MODE		_IOW(SEC_NFC_MAGIC, 0, unsigned int)
#define SEC_NFC_SET_MODE		_IOW(SEC_NFC_MAGIC, 1, unsigned int)
#define SEC_NFC_SLEEP			_IOW(SEC_NFC_MAGIC, 2, unsigned int)
#define SEC_NFC_WAKEUP			_IOW(SEC_NFC_MAGIC, 3, unsigned int)

/* size */
#define SEC_NFC_MSG_MAX_SIZE	(256 + 4)

/* wait for device stable */
#define SEC_NFC_VEN_WAIT_TIME	(100)

/* gpio pin configuration */
struct sec_nfc_platform_data {
	int irq;
	int ven;
	int firm;
	int wake;
	int tvdd;
	int avdd;
#ifdef CONFIG_SEC_NFC_IO_EN
	unsigned int pvdd;
#endif
#ifdef CONFIG_SEC_NFC_CLK_REQ
	unsigned int clk_req;
	unsigned int clk_irq;
#ifdef CONFIG_SEC_NFC_USE_8916_RFCLK2
	struct clk *clk_enable;
#else
	unsigned int clk_enable;
#endif
#endif
#ifdef CONFIG_SEC_NFC_USE_8916_BBCLK2
	struct clk *nfc_clk;
#endif
	void (*cfg_gpio)(void);
	u32 ven_gpio_flags;
	u32 firm_gpio_flags;
	u32 irq_gpio_flags;
#ifdef CONFIG_SEC_NFC_IO_EN
	u32 pvdd_gpio_flags;
#endif
};

enum sec_nfc_mode {
	SEC_NFC_MODE_OFF = 0,
	SEC_NFC_MODE_FIRMWARE,
	SEC_NFC_MODE_BOOTLOADER,
	SEC_NFC_MODE_COUNT,
};

#ifdef CONFIG_SEC_NFC_PRODUCT_N3
enum sec_nfc_power {
	SEC_NFC_PW_OFF = 0,
	SEC_NFC_PW_ON,
};
#elif defined(CONFIG_SEC_NFC_PRODUCT_N5)
enum sec_nfc_power {
	SEC_NFC_PW_ON = 0,
	SEC_NFC_PW_OFF,
};
#endif

enum sec_nfc_firmpin
{
	SEC_NFC_FW_OFF = 0,
	SEC_NFC_FW_ON,
};

enum sec_nfc_wake {
	SEC_NFC_WAKE_SLEEP = 0,
	SEC_NFC_WAKE_UP,
};
#ifdef CONFIG_SEC_NFC_IO_EN
enum sec_nfc_pvddpin
{
	SEC_NFC_PVDD_OFF = 0,
	SEC_NFC_PVDD_ON,
};
#endif
