/*
 * max77843.h - Driver for the Maxim 77843
 *
 *  Copyright (C) 2011 Samsung Electrnoics
 *  SangYoung Son <hello.son@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This driver is based on max8997.h
 *
 * MAX77843 has Charger, Flash LED, Haptic, MUIC devices.
 * The devices share the same I2C bus and included in
 * this mfd driver.
 */

#ifndef __LINUX_MFD_MAX77843_H
#define __LINUX_MFD_MAX77843_H

#include <linux/regulator/consumer.h>
#include <linux/battery/sec_charging_common.h>
#include <linux/battery/charger/max77843_charger.h>

enum {
	MAX77843_MUIC_DETACHED = 0,
	MAX77843_MUIC_ATTACHED
};

enum {
	MAX77843_PATH_OPEN = 0,
	MAX77843_PATH_USB_AP,
	MAX77843_PATH_AUDIO,
	MAX77843_PATH_UART_AP,
	MAX77843_PATH_USB_CP,
	MAX77843_PATH_UART_CP,
};

/* ATTACHED Dock Type */
enum {
	MAX77843_MUIC_DOCK_DETACHED,
	MAX77843_MUIC_DOCK_DESKDOCK,
	MAX77843_MUIC_DOCK_CARDOCK,
	MAX77843_MUIC_DOCK_AUDIODOCK = 7,
	MAX77843_MUIC_DOCK_SMARTDOCK = 8,
};

/* MAX77686 regulator IDs */
enum max77843_regulators {
	MAX77843_ESAFEOUT1 = 0,
	MAX77843_ESAFEOUT2,

	MAX77843_CHARGER,

	MAX77843_REG_MAX,
};

struct max77843_charger_reg_data {
	u8 addr;
	u8 data;
};

struct max77843_charger_platform_data {
	struct max77843_charger_reg_data *init_data;
	int num_init_data;
#if defined(CONFIG_WIRELESS_CHARGING) || defined(CONFIG_CHARGER_MAX77843)
	int wpc_irq_gpio;
	int vbus_irq_gpio;
	bool wc_pwr_det;
#endif
};

#ifdef CONFIG_SS_VIBRATOR
#define DIVIDER_32			(0x00)
#define DIVIDER_64			(0x01)
#define DIVIDER_128		(DIVIDER_64<<1)
#define DIVIDER_256		(0x03)
#define MAX77843_VIB_MOTOR_ERM	0
#define MAX77843_VIB_MOTOR_LRA	1
#define MOTOR_LRA			(MAX77843_VIB_MOTOR_LRA<<7)
#define MOTOR_EN			(1<<6)

struct max77843_haptic_platform_data
{
    int mode;
    int divisor;    /* PWM Frequency Divisor. 32, 64, 128 or 256 */
};

#endif

#ifdef CONFIG_LEDS_MAX77843
struct max77843_led_platform_data;
#endif

struct max77843_regulator_data {
	int id;
	struct regulator_init_data *initdata;
};

struct max77843_platform_data {
	/* IRQ */
	u32 irq_base;
	u32 irq_base_flags;
	int irq_gpio;
	u32 irq_gpio_flags;
	bool wakeup;
	struct max77843_muic_data *muic_data;
	struct max77843_regulator_data *regulators;
	int num_regulators;
#ifdef CONFIG_SS_VIBRATOR
	/* haptic motor data */
	struct max77843_haptic_platform_data *haptic_data;
#endif
#ifdef CONFIG_LEDS_MAX77843
	/* led (flash/torch) data */
	struct max77843_led_platform_data *led_data;
#endif
#if defined(CONFIG_CHARGER_MAX77843)
	sec_charger_platform_data_t *charger_data;
#endif
#if defined(CONFIG_FUELGAUGE_MAX77843)
	sec_fuelgauge_platform_data_t *fuelgauge_data;
#endif
};

enum cable_type_muic;
struct max77843_muic_data {
#if !defined(CONFIG_EXTCON)
	void (*usb_cb) (u8 attached);
	void (*uart_cb) (u8 attached);
	int (*charger_cb) (enum cable_type_muic);
	void (*dock_cb) (int type);
	void (*mhl_cb) (int attached);
	void (*init_cb) (void);
	int (*set_safeout) (int path);
	 bool(*is_mhl_attached) (void);
	int (*cfg_uart_gpio) (void);
	void (*jig_uart_cb) (int path);
	int (*host_notify_cb) (int enable);
	int gpio_usb_sel;
	int sw_path;
	int uart_path;

	void (*jig_state) (int jig_state);
#else
	int usb_sel;
	int uart_sel;
#endif
};

#ifdef CONFIG_LEDS_MAX77843
#define max77843_set_bit(m) ((m) & 0x0F ? ((m) & 0x03 ? ((m) & 0x01 ? 0 : 1) : ((m) & 0x04 ? 2 : 3)) : \
                             ((m) & 0x30 ? ((m) & 0x10 ? 4 : 5) : ((m) & 0x40 ? 6 : 7)))
#endif

//#ifdef CONFIG_MFD_MAX77843
extern struct max77843_muic_data max77843_muic;
extern struct max77843_regulator_data max77843_regulators[];
extern int muic_otg_control(int enable);
extern struct max77843_haptic_platform_data max77843_haptic_pdata;
extern struct max77843_led_platform_data max77843_led_pdata;
extern int max77843_muic_set_safeout(int path);
//#endif
#if defined(CONFIG_SEC_MHL_SUPPORT)
int acc_register_notifier(struct notifier_block *nb);
#endif
#endif				/* __LINUX_MFD_MAX77843_H */
