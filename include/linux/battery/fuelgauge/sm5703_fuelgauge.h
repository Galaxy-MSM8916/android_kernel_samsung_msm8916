/*
 * drivers/battery/sm5703_fuelgauge.h
 *
 * Header of SiliconMitus SM5703 Fuelgauge Driver
 *
 * Copyright (C) 2015 SiliconMitus
 * Author: SW Jung
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef SM5703_FUELGAUGE_H
#define SM5703_FUELGAUGE_H

#include <linux/i2c.h>
#include <linux/mfd/sm5703.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif /* #ifdef CONFIG_DEBUG_FS */

#define FG_DRIVER_VER "0.0.0.1"



struct battery_data_t {
	const int battery_type; /* 4200 or 4350 or 4400*/
	const int battery_table[3][16];
	const int rce_value[3];
	const int dtcd_value;
	const int rs_value[4];
	const int vit_period;
	const int mix_value[2];
	const int topoff_soc[2];
	const int volt_cal;
	const int curr_cal;
	const int temp_std;
	const int temp_offset;
	const int temp_offset_cal;
	const int charge_offset_cal;
};

struct sec_fg_info {
	/* State Of Connect */
	int online;
	/* battery SOC (capacity) */
	int batt_soc;
	/* battery voltage */
	int batt_voltage;
	/* battery AvgVoltage */
	int batt_avgvoltage;
	/* battery OCV */
	int batt_ocv;
	/* Current */
	int batt_current;
	/* battery Avg Current */
	int batt_avgcurrent;

	struct battery_data_t *comp_pdata;

	struct mutex param_lock;
	/* copy from platform data /
	 * DTS or update by shell script */

	struct mutex io_lock;
	struct device *dev;
	int32_t temperature; /* 0.1 deg C*/
	int32_t temp_fg; /* 0.1 deg C*/
	/* register programming */
	int reg_addr;
	u8 reg_data[2];

	int battery_table[3][16];
	int rce_value[3];
	int dtcd_value;
	int rs_value[4]; /*rs mix_factor max min*/
	int vit_period;
	int mix_value[2]; /*mix_rate init_blank*/

	int enable_topoff_soc;
	int topoff_soc;

	int volt_cal;
	int curr_cal;

	int temp_std;
	int temp_offset;
	int temp_offset_cal;
	int charge_offset_cal;
	int en_high_temp_cal;
	int high_temp_cal_denom;
	int high_temp_p_cal_fact;
	int high_temp_n_cal_fact;
	int en_low_temp_cal;
	int low_temp_cal_denom;
	int low_temp_p_cal_fact;
	int low_temp_n_cal_fact;

	int battery_type; /* 4200 or 4350 or 4400*/
	uint32_t soc_alert_flag : 1;  /* 0 : nu-occur, 1: occur */
	uint32_t volt_alert_flag : 1; /* 0 : nu-occur, 1: occur */
	uint32_t flag_full_charge : 1; /* 0 : no , 1 : yes*/
	uint32_t flag_chg_status : 1; /* 0 : discharging, 1: charging*/

	int32_t irq_ctrl;

	uint32_t is_FG_initialised;
	int iocv_error_count;

	int n_tem_poff;
	int n_tem_poff_offset;
	int l_tem_poff;
	int l_tem_poff_offset;

	/* previous battery voltage current*/
	int p_batt_voltage;
	int p_batt_current;
	int min_charge_curr;
};

struct sec_fuelgauge_info {
	struct i2c_client		*client;
	sec_battery_platform_data_t *pdata;
	struct power_supply		psy_fg;
	struct delayed_work isr_work;

	int cable_type;
	bool is_charging;

	/* HW-dedicated fuel guage info structure
	 * used in individual fuel gauge file only
	 * (ex. dummy_fuelgauge.c)
	 */
	struct sec_fg_info	info;

	bool is_fuel_alerted;
	bool volt_alert_flag;
	struct wake_lock fuel_alert_wake_lock;

	unsigned int capacity_old;	/* only for atomic calculation */
	unsigned int capacity_max;	/* only for dynamic calculation */

	bool initial_update_of_soc;
	struct mutex fg_lock;

	/* register programming */
	int reg_addr;
	u8 reg_data[2];

	int fg_irq;
};

bool sec_hal_fg_init(struct i2c_client *);
bool sec_hal_fg_suspend(struct i2c_client *);
bool sec_hal_fg_resume(struct i2c_client *);
bool sec_hal_fg_fuelalert_init(struct i2c_client *, int);
bool sec_hal_fg_is_fuelalerted(struct i2c_client *);
bool sec_hal_fg_fuelalert_process(void *, bool);
bool sec_hal_fg_full_charged(struct i2c_client *);
bool sec_hal_fg_reset(struct i2c_client *);
bool sec_hal_fg_get_property(struct i2c_client *,
				enum power_supply_property,
				union power_supply_propval *);
bool sec_hal_fg_set_property(struct i2c_client *,
				enum power_supply_property,
				const union power_supply_propval *);

ssize_t sec_hal_fg_show_attrs(struct device *dev,
				const ptrdiff_t offset, char *buf);

ssize_t sec_hal_fg_store_attrs(struct device *dev,
				const ptrdiff_t offset,
				const char *buf, size_t count);

ssize_t sec_fg_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf);

ssize_t sec_fg_store_attrs(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count);


#ifdef CONFIG_OF
extern void board_fuelgauge_init(void *fuelgauge);
extern bool sec_bat_check_jig_status(void);
#endif

#define SEC_FG_ATTR(_name)				\
{							\
	.attr = {.name = #_name, .mode = 0664},	\
	.show = sec_fg_show_attrs,			\
	.store = sec_fg_store_attrs,			\
}

enum {
	FG_REG = 0,
	FG_DATA,
	FG_REGS,
};

#endif // SM5703_FUELGAUGE_H
