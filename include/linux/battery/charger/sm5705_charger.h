/*
 * sm5705_charger.h
 * Samsung SM5705 Charger Header
 *
 * Copyright (C) 2012 Samsung Electronics, Inc.
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

#ifndef __SM5705_CHARGER_H
#define __SM5705_CHARGER_H __FILE__

#include <linux/mfd/core.h>
#include <linux/mfd/sm5705/sm5705.h>
#include <linux/regulator/machine.h>
#include <linux/battery/sec_charging_common.h>

/* CONFIG: Kernel Feature & Target System configuration */
//#define SM5705_SUPPORT_AICL_CONTROL       - New A series dosen't support, It's MUST be disabled
//#define SM5705_SUPPORT_OTG_CONTROL        - New A series dosen't support, It's MUST be disabled

enum {
	CHIP_ID = 0,
};

ssize_t sm5705_chg_show_attrs(struct device *dev, struct device_attribute *attr, char *buf);

ssize_t sm5705_chg_store_attrs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

#define SM5705_CHARGER_ATTR(_name)				\
{							                    \
	.attr = {.name = #_name, .mode = 0664},	    \
	.show = sm5705_chg_show_attrs,			    \
	.store = sm5705_chg_store_attrs,			\
}

enum {
	SM5705_MANUAL_RESET_TIME_7s = 0x1,
	SM5705_MANUAL_RESET_TIME_8s = 0x2,
	SM5705_MANUAL_RESET_TIME_9s = 0x3,
};

enum {
	SM5705_WATCHDOG_RESET_TIME_30s  = 0x0,
	SM5705_WATCHDOG_RESET_TIME_60s  = 0x1,
	SM5705_WATCHDOG_RESET_TIME_90s  = 0x2,
	SM5705_WATCHDOG_RESET_TIME_120s = 0x3,
};

/* for SIOP - default values */
#define SIOP_INPUT_LIMIT_CURRENT                1200
#define SIOP_CHARGING_LIMIT_CURRENT             1000
#define SIOP_HV_INPUT_LIMIT_CURRENT             1200
#define SIOP_HV_CHARGING_LIMIT_CURRENT          1000
#define SIOP_WIRELESS_INPUT_LIMIT_CURRENT       700
#define SIOP_WIRELESS_CHARGING_LIMIT_CURRENT    600
#define SIOP_HV_WIRELESS_INPUT_LIMIT_CURRENT	500
#define SIOP_HV_WIRELESS_CHARGING_LIMIT_CURRENT	1000
/* for VZW support */
#define SLOW_CHARGING_CURRENT_STANDARD          400

/* SM5705 Charger - AICL reduce current configuration */
#define REDUCE_CURRENT_STEP					100
#define MINIMUM_INPUT_CURRENT					300
#define AICL_VALID_CHECK_DELAY_TIME				10

#define SM5705_EN_DISCHG_FORCE_MASK		0x02

struct sm5705_charger_data {
	struct device *dev;
	struct i2c_client *i2c;
	struct sec_charger_platform_data *pdata;

	struct power_supply	psy_chg;
	struct power_supply	psy_otg;
	int	status;

	/* for IRQ-service handling */
	int	irq_aicl;
	int	irq_vbus_pok;
	int	irq_wpcin_pok;
	int irq_topoff;

	/* for Workqueue & wake-lock, mutex process */
	struct mutex charger_mutex;

	struct workqueue_struct *wqueue;
	struct delayed_work	wpc_work;
	struct delayed_work	wc_afc_work;
	struct delayed_work	aicl_work;
	struct delayed_work	topoff_work;
	struct delayed_work soft_start_work;
	struct delayed_work set_current_work;
	// temp for rev2 SW WA
	struct delayed_work op_mode_switch_work;        /* for WA obnormal switch case in JIG cable */

	struct wake_lock wpc_wake_lock;
	struct wake_lock afc_wake_lock;
	struct wake_lock softstart_wake_lock;
	struct wake_lock check_slow_wake_lock;
	struct wake_lock aicl_wake_lock;

	/* for charging operation handling */
	unsigned int cable_type;
	unsigned int charging_current_max;
	unsigned int charging_current;
	int	irq_wpcin_state;
	int	siop_level;
	bool wc_afc_detect;
	bool topoff_pending;
	// temp for rev2 SW WA
	bool is_rev2_wa_done;
};

extern int sm5705_call_fg_device_id(void);

/* SM5705 Charger - RESET condition configuaration */
#define SM5705_I2C_RESET_ACTIVATE
//#define SM5705_MANUAL_RESET_ACTIVATE
//#define SM5705_MANUAL_RESET_TIMER                   SM5705_MANUAL_RESET_TIME_7s
//#define SM5705_WATCHDOG_RESET_ACTIVATE
//#define SM5705_WATCHDOG_RESET_TIMER                   SM5705_WATCHDOG_RESET_TIME_120s
//#define SM5705_SW_SOFT_START      /* default used HW soft-start */

#if defined(SM5705_WATCHDOG_RESET_ACTIVATE)
extern void sm5705_charger_watchdog_timer_keepalive(void);
#endif

#endif /* __SM5705_CHARGER_H */

