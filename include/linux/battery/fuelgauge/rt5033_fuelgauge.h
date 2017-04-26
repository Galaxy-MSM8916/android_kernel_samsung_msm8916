/*
 * drivers/battery/rt5033_fuelgauge.h
 *
 * Header of Richtek RT5033 Fuelgauge Driver
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef RT5033_FUELGAUGE_H
#define RT5033_FUELGAUGE_H

#include <linux/i2c.h>
#include <linux/mfd/rt5033.h>
#include <linux/battery/sec_battery.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif /* #ifdef CONFIG_DEBUG_FS */

#define FG_DRIVER_VER "2.0.2S"

/* Added by Patrick */
#define VG_COMP_NR 4

typedef struct {
	int data[VG_COMP_NR];
} vg_comp_data_t;

typedef struct {
	union {
		int x;
		int voltage;
		int soc;
	};
	union {
		int y;
		int temperature;
	};
	union {
		int data[VG_COMP_NR];
		vg_comp_data_t vg_comp;
		int z;
		int offset;
	};
} data_point_t;

typedef struct {
    int voltNR;
    int tempNR;
    data_point_t *vg_comp_data;
} vg_comp_table_t;

typedef struct {
    int soc_voltNR;
    int tempNR;
    data_point_t *soc_offset_data;
} soc_offset_table_t;


enum {
    OFFSET_CHARGING = 0,
    OFFSET_DISCHARGING,
    OFFSET_SPECIAL,
    OFFSET_LOW_POWER,
    OFFSET_NR,
};

enum {
    VGCOMP_NORMAL = 0,
    VGCOMP_IDLE,
    VGCOMP_NR,
};


struct battery_data_t {
	const int offset_interpolation_order[2];
	const int vg_comp_interpolation_order[2];
	const vg_comp_table_t vg_comp[VGCOMP_NR];
	const soc_offset_table_t soc_offset[OFFSET_NR];
	/* crate idle threshold*/
	const int crate_idle_thres;
	const int battery_type; /* 4200 or 4350 or 4400 */
};

#ifdef CONFIG_DEBUG_FS
enum {
	RT5033FG_OFFSET_CHARGING_SIZE = 0,
	RT5033FG_OFFSET_CHARGING_DATA,
	RT5033FG_OFFSET_DISCHARGING_SIZE,
	RT5033FG_OFFSET_DISCHARGING_DATA,
	RT5033FG_OFFSET_SPECIAL_SIZE,
	RT5033FG_OFFSET_SPECIAL_DATA,
	RT5033FG_OFFSET_LOW_POWER_SIZE,
	RT5033FG_OFFSET_LOW_POWER_DATA,
	RT5033FG_VGCOMP_NORMAL_SIZE,
	RT5033FG_VGCOMP_NORMAL_DATA,
	RT5033FG_VGCOMP_IDLE_SIZE,
	RT5033FG_VGCOMP_IDLE_DATA,
	RT5033FG_PARAM_LOCK,
	RT5033FG_VGCOMP_IP_ORDER,
	RT5033FG_OFFSET_IP_ORDER,
	RT5033FG_CRATE_IDLE_THRES,
	RT5033FG_DENTRY_NR,
};

#define RT5033_DBG_OUT_BUF_SIZE 2048

#endif /* CONFIG_DEBUG_FS */

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
	/* battery temperatue */
	u32 current_temp_adc;
	/* crate idle threshold*/
	int crate_idle_thres;

	struct battery_data_t *comp_pdata;

	struct mutex param_lock;
	/* copy from platform data /
	 * DTS or update by shell script */
	int vg_comp_interpolation_order[2];
	int offset_interpolation_order[2];
	vg_comp_table_t vg_comp[VGCOMP_NR];
	soc_offset_table_t soc_offset[OFFSET_NR];

	struct mutex io_lock;
	struct device *dev;
	int32_t temperature;; /* 0.1 deg C*/
	/* register programming */
	int reg_addr;
	u8 reg_data[2];
	uint32_t pre_soc;
	uint32_t move_avg_offset_cnt;
	uint32_t flag_full_charge : 1; /* 0 : no , 1 : yes*/
	uint32_t flag_chg_status : 1; /* 0 : discharging, 1: charging*/
	uint32_t offs_speci_case : 1;
	uint32_t init_once : 1;
	uint32_t volt_alert_flag : 1; /* 0 : nu-occur, 1: occur */
	uint32_t soc_alert_flag : 1;  /* 0 : nu-occur, 1: occur */
	uint32_t bat_pres_flag : 1; /* 0 : removed, 1: inserted */
	uint32_t flag_once_full_soc : 1;
	uint32_t entry_suspend_flag : 1;
	int32_t irq_ctrl;
	int battery_type; /* 4200 or 4350 or 4400*/

#ifdef CONFIG_DEBUG_FS
	struct dentry *dir_dentry;
	struct dentry *file_dentries[RT5033FG_DENTRY_NR];
	char dbg_out_buffer[RT5033_DBG_OUT_BUF_SIZE];
#endif /* #ifdef CONFIG_DEBUG_FS */
};

struct sec_fuelgauge_reg_data {
	u8 reg_addr;
	u8 reg_data1;
	u8 reg_data2;
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

#endif // RT5033_FUELGAUGE_H
