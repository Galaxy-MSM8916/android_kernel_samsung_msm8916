/*
 *  max77849_charger.c
 *  Samsung max77849 Charger Driver
 *
 *  Copyright (C) 2014 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mfd/max77849.h>
#include <linux/mfd/max77849-private.h>
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/usb_notify.h>
#endif

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static struct dentry    *max77849_dentry;

#define ENABLE 1
#define DISABLE 0


#define RECOVERY_DELAY		3000
#define RECOVERY_CNT		5
#define REDUCE_CURRENT_STEP	100
#define MINIMUM_INPUT_CURRENT	300

#define SIOP_INPUT_LIMIT_CURRENT 1200
#define SIOP_CHARGING_LIMIT_CURRENT 1000
#ifndef CONFIG_SLOW_CHARGING_CURRENT_STANDARD
#define CONFIG_SLOW_CHARGING_CURRENT_STANDARD 1000
#endif

struct max77849_charger_data {
	struct max77849_dev	*max77849;

	struct power_supply	psy_chg;
	struct power_supply	psy_otg;

	struct workqueue_struct *wqueue;
	struct work_struct	chgin_work;
	struct delayed_work	isr_work;
	struct delayed_work	wpc_work;	/*  wpc detect work */
	struct delayed_work	chgin_init_work;	/*  chgin init work */
	struct delayed_work	fg_source_change_work;

	/* mutex */
	struct mutex irq_lock;
	struct mutex ops_lock;

	/* wakelock */
	struct wake_lock wpc_wake_lock;
	struct wake_lock chgin_wake_lock;
	struct wake_lock fg_source_change_wake_lock;

	unsigned int	is_charging;
	unsigned int	charging_type;
	unsigned int	battery_state;
	unsigned int	battery_present;
	unsigned int	cable_type;
	unsigned int	charging_current_max;
	unsigned int	charging_current;
	unsigned int	vbus_state;
	int		aicl_on;
	int		status;
	int		siop_level;

	int		irq_bypass;
	int		irq_batp;
	int		irq_battery;
	int		irq_chg;
	int		irq_chgin;

	/* software regulation */
	bool		soft_reg_state;
	int		soft_reg_current;

	/* unsufficient power */
	bool		reg_loop_deted;


	int pmic_ver;
	int input_curr_limit_step;
	int charging_curr_step;
	/* to set otg current limit(OTG_ILIM flag) */
	int otg_current_limit;

	sec_battery_platform_data_t	*pdata;
	bool is_jig_attached;
};

static enum power_supply_property sec_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
#endif
};

static enum power_supply_property max77849_otg_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static void max77849_charger_initialize(struct max77849_charger_data *charger);
static int max77849_get_vbus_state(struct max77849_charger_data *charger);
static int max77849_get_charger_state(struct max77849_charger_data *charger);
static void max77849_dump_reg(struct max77849_charger_data *charger)
{
	u8 reg_data;
	u32 reg_addr;
	pr_info("%s\n", __func__);

	for (reg_addr = 0xB0; reg_addr <= 0xC5; reg_addr++) {
		max77849_read_reg(charger->max77849->i2c, reg_addr, &reg_data);
		pr_info("max77849: c: 0x%02x(0x%02x)\n", reg_addr, reg_data);
	}
}

static bool max77849_charger_unlock(struct max77849_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77849->i2c;
	u8 reg_data;
	u8 chgprot;
	int retry_cnt = 0;
	bool need_init = false;

	do {
		max77849_read_reg(i2c, MAX77849_CHG_REG_CHG_CNFG_06, &reg_data);
		chgprot = ((reg_data & 0x0C) >> 2);
		if (chgprot != 0x03) {
			pr_err("%s: unlock err, chgprot(0x%x), retry(%d)\n",
					__func__, chgprot, retry_cnt);
			max77849_write_reg(i2c, MAX77849_CHG_REG_CHG_CNFG_06,
				(0x03 << 2));
			need_init = true;
			msleep(20);
		} else {
			pr_debug("%s: unlock success, chgprot(0x%x)\n",
				__func__, chgprot);
			break;
		}
	} while ((chgprot != 0x03) && (++retry_cnt < 10));

	return need_init;
}

static void check_charger_unlock_state(struct max77849_charger_data *chg_data)
{
	bool need_reg_init;
	pr_debug("%s\n", __func__);

	need_reg_init = max77849_charger_unlock(chg_data);
	if (need_reg_init) {
		pr_err("%s: charger locked state, reg init\n", __func__);
		max77849_charger_initialize(chg_data);
	}
}

static int max77849_get_battery_present(struct max77849_charger_data *charger)
{
	u8 reg_data;

	if (max77849_read_reg(charger->max77849->i2c,
			MAX77849_CHG_REG_CHG_INT_OK, &reg_data) < 0) {
		/* Eventhough there is an error,
		   don't do power-off */
		return 1;
	}

	pr_debug("%s: CHG_INT_OK(0x%02x)\n", __func__, reg_data);

	reg_data = ((reg_data & MAX77849_BATP_OK) >> MAX77849_BATP_OK_SHIFT);

	return reg_data;
}

static u8 max77849_get_float_voltage_data(
					int float_voltage)
{
	u8 data = 0x16;

	if(float_voltage >= 3800 && float_voltage <= 4325)
		data = 0x1B - (4325 - float_voltage) / 25;
	else if(float_voltage == 4340)
		data = 0x1C;
	else if(float_voltage > 4340 && float_voltage <= 4400)
		data = 0x1F - (4400 - float_voltage) / 25;
	else if(float_voltage > 4400 && float_voltage <= 4550)
		data = 0x05 - (4550 - float_voltage) / 25;
	else
		data = 0x16;

	return data;
}

#if defined(CONFIG_BATTERY_SWELLING)
static int max77849_get_float_voltage(struct max77849_charger_data *charger)
{
	u8 reg_data;
        int float_voltage, cv_param;

        max77849_read_reg(charger->max77849->i2c,
                        MAX77849_CHG_REG_CHG_CNFG_04, &reg_data);

        cv_param = reg_data & CHG_CNFG_04_CHG_CV_PRM_MASK;

        if(cv_param >= 0x00 && cv_param <= 0x05)
                float_voltage = 4425 + (cv_param * 25);
        else if(cv_param >= 0x06 && cv_param <= 0x1B)
                float_voltage = 3650 + (cv_param * 25);
        else if(cv_param == 0x1C)
                float_voltage = 4340;
        else
                float_voltage = 3625 + (cv_param * 25);

        pr_info("%s: battery cv voltage %d\n", __func__, float_voltage);
        return float_voltage;
}

static void max77849_set_float_voltage(struct max77849_charger_data *charger,
                int float_voltage)
{
        u8 reg_data;

	reg_data = max77849_get_float_voltage_data(float_voltage);
        max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_04,
                        (reg_data << CHG_CNFG_04_CHG_CV_PRM_SHIFT),
                        CHG_CNFG_04_CHG_CV_PRM_MASK);
        max77849_read_reg(charger->max77849->i2c,
                        MAX77849_CHG_REG_CHG_CNFG_04, &reg_data);
        pr_info("%s: battery CV voltage : %d(0x%x)\n", __func__, charger->pdata->chg_float_voltage, reg_data);
}
#endif

static void max77849_set_charger_state(struct max77849_charger_data *charger,
		int enable)
{
        u8 reg_data;

        if (enable) {
                max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
                                CHG_CNFG_00_CHG_MASK, CHG_CNFG_00_CHG_MASK);
        } else {
                max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
                                0, CHG_CNFG_00_CHG_MASK);
        }
        max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00, &reg_data);
        pr_debug("%s : CHG_CNFG_00(0x%02x)\n", __func__, reg_data);
}

static void max77849_set_buck(struct max77849_charger_data *charger,
		int enable)
{
        u8 reg_data;

        if (enable) {
                max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
                                CHG_CNFG_00_BUCK_MASK, CHG_CNFG_00_BUCK_MASK);
        } else {
                max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
                                0, CHG_CNFG_00_BUCK_MASK);
        }
        max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00, &reg_data);
        pr_debug("%s : CHG_CNFG_00(0x%02x)\n", __func__, reg_data);
}

static void max77849_check_slow_charging(struct max77849_charger_data *charger, int set_current_reg)
{
	/* under 500mA, slow rate */
	if (set_current_reg <= (CONFIG_SLOW_CHARGING_CURRENT_STANDARD / charger->input_curr_limit_step) &&
			(charger->cable_type != POWER_SUPPLY_TYPE_BATTERY)) {
		charger->aicl_on = true;
		pr_info("%s: slow charging on : set_current_reg(0x%02x), cable type(%d)\n", __func__, set_current_reg, charger->cable_type);
	}
	else
		charger->aicl_on = false;
}

static void max77849_set_input_current(struct max77849_charger_data *charger,
		int cur)
{
	int set_current_reg, now_current_reg;
	int vbus_state, curr_step, delay;
	u8 set_reg, reg_data;
	int chg_state;

	mutex_lock(&charger->ops_lock);
	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, &reg_data);
	reg_data |= (0x1 << 6);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, reg_data);
	set_reg = MAX77849_CHG_REG_CHG_CNFG_09;
	charger->input_curr_limit_step = 25;

	if (cur <= 0) {
		/* disable only buck because power onoff test issue */
		max77849_write_reg(charger->max77849->i2c,
			set_reg, 0x19);
		max77849_set_buck(charger, DISABLE);
		goto exit;
	} else
		max77849_set_buck(charger, ENABLE);

	set_current_reg = cur / charger->input_curr_limit_step;
	if (charger->cable_type == POWER_SUPPLY_TYPE_BATTERY)
		goto set_input_current;

	max77849_read_reg(charger->max77849->i2c,
		set_reg, &reg_data);
	if (reg_data == set_current_reg) {
		/* check uvlo  */
		while((set_current_reg > (MINIMUM_INPUT_CURRENT / charger->input_curr_limit_step)) && (set_current_reg < 255)) {
			vbus_state = max77849_get_vbus_state(charger);
			if ((vbus_state == 0x00) || (vbus_state == 0x01)) {
				/* UVLO */
				set_current_reg -= 5;
				if (set_current_reg < (MINIMUM_INPUT_CURRENT / charger->input_curr_limit_step))
					set_current_reg = (MINIMUM_INPUT_CURRENT / charger->input_curr_limit_step);
				max77849_write_reg(charger->max77849->i2c,
						set_reg, set_current_reg);
				pr_info("%s: set_current_reg(0x%02x)\n", __func__, set_current_reg);
				chg_state = max77849_get_charger_state(charger);
				if ((chg_state != POWER_SUPPLY_STATUS_CHARGING) &&
						(chg_state != POWER_SUPPLY_STATUS_FULL))
					break;
				msleep(50);
			} else
				break;
		}
		goto exit;
	}

	if (reg_data == 0) {
		now_current_reg = SOFT_CHG_START_CURR / charger->input_curr_limit_step;
		max77849_write_reg(charger->max77849->i2c,
			set_reg, now_current_reg);
		msleep(SOFT_CHG_START_DUR);
	} else
		now_current_reg = reg_data;

	if (cur < 1000) {
		curr_step = 1;
		delay = 50;
	} else {
		curr_step = SOFT_CHG_CURR_STEP / charger->input_curr_limit_step;
		delay = SOFT_CHG_STEP_DUR;
	}
	now_current_reg += (curr_step);

	while (now_current_reg < set_current_reg &&
			charger->cable_type != POWER_SUPPLY_TYPE_BATTERY)
	{
		now_current_reg = min(now_current_reg, set_current_reg);
		max77849_write_reg(charger->max77849->i2c,
			set_reg, now_current_reg);
		msleep(delay);

		vbus_state = max77849_get_vbus_state(charger);
		if ((vbus_state == 0x00) || (vbus_state == 0x01)) {
			/* UVLO */
			if (now_current_reg > (curr_step * 3))
				now_current_reg -= (curr_step * 3);
			/* current limit 300mA */
			if (now_current_reg < (MINIMUM_INPUT_CURRENT / charger->input_curr_limit_step))
				now_current_reg = (MINIMUM_INPUT_CURRENT / charger->input_curr_limit_step);
			curr_step /= 2;
			max77849_write_reg(charger->max77849->i2c,
					set_reg, now_current_reg);
			pr_info("%s: now_current_reg(0x%02x)\n", __func__, now_current_reg);
			chg_state = max77849_get_charger_state(charger);
			if ((chg_state != POWER_SUPPLY_STATUS_CHARGING) &&
					(chg_state != POWER_SUPPLY_STATUS_FULL))
				goto exit;
			if (curr_step < 2) {
				goto exit;
			}
			msleep(50);
		} else
			now_current_reg += (curr_step);
	}

set_input_current:
	pr_info("%s: reg_data(0x%02x), input(%d)\n",
		__func__, set_current_reg, cur);
	max77849_write_reg(charger->max77849->i2c,
		set_reg, set_current_reg);
exit:
	/* slow charging check */
	max77849_read_reg(charger->max77849->i2c,
		set_reg, &reg_data);
	max77849_check_slow_charging(charger, reg_data);

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, &reg_data);
	reg_data &= ~(0x1 << 6);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, reg_data);
	mutex_unlock(&charger->ops_lock);
}

static int max77849_get_input_current(struct max77849_charger_data *charger)
{
	u8 reg_data;
	int get_current = 0;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_09, &reg_data);
	pr_info("%s: CHG_CNFG_09(0x%02x)\n", __func__, reg_data);
	charger->input_curr_limit_step = 25;

	get_current = reg_data * charger->input_curr_limit_step;

	pr_debug("%s: get input current: %dmA\n", __func__, get_current);
	return get_current;
}

static void max77849_set_topoff_current(struct max77849_charger_data *charger,
		int cur, int timeout)
{
	u8 reg_data;

	if (cur >= 350)
		reg_data = 0x07;
	else if (cur >= 300)
		reg_data = 0x06;
	else if (cur >= 250)
		reg_data = 0x05;
	else if (cur >= 200)
		reg_data = 0x04;
	else if (cur >= 175)
		reg_data = 0x03;
	else if (cur >= 150)
		reg_data = 0x02;
	else if (cur >= 125)
		reg_data = 0x01;
	else
		reg_data = 0x00;

	/* always set 70min  */
	reg_data |= (0x7 << 3);
	pr_info("%s: reg_data(0x%02x), topoff(%d)\n", __func__, reg_data, cur);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_03, reg_data);
}

static void max77849_set_charge_current(struct max77849_charger_data *charger,
		int cur)
{
	u8 reg_data = 0;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_02, &reg_data);
	reg_data &= ~MAX77849_CHG_CC;

	if (!cur) {
		/* No charger */
		max77849_write_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_02, reg_data);
	} else {
		reg_data |= ((cur * 10 / charger->charging_curr_step) << 0);
		max77849_write_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_02, reg_data);
	}
	pr_info("%s: reg_data(0x%02x), charge(%d)\n",
			__func__, reg_data, cur);
}

/*
static int max77849_get_charge_current(struct max77849_charger_data *charger)
{
	u8 reg_data;
	int get_current;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_02, &reg_data);
	pr_debug("%s: CHG_CNFG_02(0x%02x)\n", __func__, reg_data);

	reg_data &= MAX77849_CHG_CC;
	get_current = reg_data * charger->charging_curr_step / 10;

	pr_debug("%s: get charge current: %dmA\n", __func__, get_current);
	return get_current;
}
*/

static void reduce_input_current(struct max77849_charger_data *charger, int cur)
{
	u8 set_reg;
	u8 set_value;
	unsigned int min_input_current = 0;

	if ((!charger->is_charging) || mutex_is_locked(&charger->ops_lock)) 
		return;
	set_reg = MAX77849_CHG_REG_CHG_CNFG_09;
	min_input_current = MINIMUM_INPUT_CURRENT;
	charger->input_curr_limit_step = 25;

	if (!max77849_read_reg(charger->max77849->i2c,
				set_reg, &set_value)) {
		if ((set_value <= (min_input_current / charger->input_curr_limit_step)) ||
		    (set_value <= (cur / charger->input_curr_limit_step)))
			return;
		set_value -= (cur / charger->input_curr_limit_step);
		set_value = (set_value < (min_input_current / charger->input_curr_limit_step)) ?
			(min_input_current / charger->input_curr_limit_step) : set_value;
		max77849_write_reg(charger->max77849->i2c,
				set_reg, set_value);
		pr_info("%s: set current: reg:(0x%x), val:(0x%x)\n",
				__func__, set_reg, set_value);
	}
}

static int max77849_get_vbus_state(struct max77849_charger_data *charger)
{
	u8 reg_data;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_DTLS_00, &reg_data);
	reg_data = ((reg_data & MAX77849_CHGIN_DTLS) >>
		MAX77849_CHGIN_DTLS_SHIFT);

	switch (reg_data) {
	case 0x00:
		pr_info("%s: VBUS is invalid. CHGIN < CHGIN_UVLO\n",
			__func__);
		break;
	case 0x01:
		pr_info("%s: VBUS is invalid. CHGIN < MBAT+CHGIN2SYS" \
			"and CHGIN > CHGIN_UVLO\n", __func__);
		break;
	case 0x02:
		pr_info("%s: VBUS is invalid. CHGIN > CHGIN_OVLO",
			__func__);
		break;
	case 0x03:
		break;
	default:
		break;
	}

	return reg_data;
}

static int max77849_get_charger_state(struct max77849_charger_data *charger)
{
	int state;
	u8 reg_data;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_DTLS_01, &reg_data);
	reg_data = ((reg_data & MAX77849_CHG_DTLS) >> MAX77849_CHG_DTLS_SHIFT);
	pr_info("%s: CHG_DTLS : 0x%2x\n", __func__, reg_data);

	switch (reg_data) {
	case 0x0:
	case 0x1:
	case 0x2:
		state = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case 0x3:
	case 0x4:
		state = POWER_SUPPLY_STATUS_FULL;
		break;
	case 0x5:
	case 0x6:
	case 0x7:
		state = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case 0x8:
	case 0xA:
	case 0xB:
		state = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	default:
		state = POWER_SUPPLY_STATUS_UNKNOWN;
		break;
	}

	return state;
}

static int max77849_get_health_state(struct max77849_charger_data *charger)
{
	int state;
	int vbus_state;
	u8 chg_dtls_00, chg_dtls, reg_data;
	u8 chg_cnfg_00, chg_cnfg_01 ,chg_cnfg_02, chg_cnfg_04, chg_cnfg_09, chg_cnfg_12;

	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_DTLS_01, &reg_data);
	reg_data = ((reg_data & MAX77849_BAT_DTLS) >> MAX77849_BAT_DTLS_SHIFT);

	pr_info("%s: reg_data(0x%x)\n", __func__, reg_data);
	switch (reg_data) {
	case 0x00:
		pr_info("%s: No battery and the charger is suspended\n",
			__func__);
		state = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
		break;
	case 0x01:
		pr_info("%s: battery is okay "
			"but its voltage is low(~VPQLB)\n", __func__);
		state = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case 0x02:
		pr_info("%s: battery dead\n", __func__);
		state = POWER_SUPPLY_HEALTH_DEAD;
		break;
	case 0x03:
		state = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case 0x04:
		pr_info("%s: battery is okay" \
			"but its voltage is low\n", __func__);
		state = POWER_SUPPLY_HEALTH_GOOD;
		break;
	case 0x05:
		pr_info("%s: battery ovp\n", __func__);
		state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		break;
	default:
		pr_info("%s: battery unknown : 0x%d\n", __func__, reg_data);
		state = POWER_SUPPLY_HEALTH_UNKNOWN;
		break;
	}

	if (state == POWER_SUPPLY_HEALTH_GOOD) {
		union power_supply_propval value;
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_HEALTH, value);
		/* VBUS OVP state return battery OVP state */
		vbus_state = max77849_get_vbus_state(charger);
		/* read CHG_DTLS and detecting battery terminal error */
		max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_DTLS_01, &chg_dtls);
		chg_dtls = ((chg_dtls & MAX77849_CHG_DTLS) >>
				MAX77849_CHG_DTLS_SHIFT);
		max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_00, &chg_cnfg_00);

		/* print the log at the abnormal case */
		if((charger->is_charging == 1) && (chg_dtls & 0x08)) {
			max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_DTLS_00, &chg_dtls_00);
			max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_01, &chg_cnfg_01);
			max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_02, &chg_cnfg_02);
		max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_CNFG_04, &chg_cnfg_04);
			max77849_read_reg(charger->max77849->i2c,
					MAX77849_CHG_REG_CHG_CNFG_09, &chg_cnfg_09);
			max77849_read_reg(charger->max77849->i2c,
					MAX77849_CHG_REG_CHG_CNFG_12, &chg_cnfg_12);

			pr_info("%s: CHG_DTLS_00(0x%x), CHG_DTLS_01(0x%x), CHG_CNFG_00(0x%x)\n",
					__func__, chg_dtls_00, chg_dtls, chg_cnfg_00);
			pr_info("%s:  CHG_CNFG_01(0x%x), CHG_CNFG_02(0x%x), CHG_CNFG_04(0x%x)\n",
					__func__, chg_cnfg_01, chg_cnfg_02, chg_cnfg_04);
			pr_info("%s:  CHG_CNFG_09(0x%x), CHG_CNFG_12(0x%x)\n",
					__func__, chg_cnfg_09, chg_cnfg_12);
		}

		pr_info("%s: vbus_state : 0x%d, chg_dtls : 0x%d\n", __func__, vbus_state, chg_dtls);
		/*  OVP is higher priority */
		if (vbus_state == 0x02) { /*  CHGIN_OVLO */
			pr_info("%s: vbus ovp\n", __func__);
			state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		} else if (((vbus_state == 0x0) || (vbus_state == 0x01)) &&(chg_dtls & 0x08) && \
				(chg_cnfg_00 & MAX77849_MODE_BUCK) && \
				(chg_cnfg_00 & MAX77849_MODE_CHGR)) { 
			pr_info("%s: vbus is under\n", __func__);
			state = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
		} else if((value.intval == POWER_SUPPLY_HEALTH_UNDERVOLTAGE) && \
				!((vbus_state == 0x0) || (vbus_state == 0x01))){
			max77849_set_input_current(charger,
					charger->charging_current_max);
		}
	}

	return state;
}

static void max77849_fg_source_change_work(struct work_struct *work)
{
	struct max77849_charger_data *charger =
		container_of(work, struct max77849_charger_data, fg_source_change_work.work);
	u8 reg_data;
	u8 val = 0;

	val = charger->is_jig_attached;
	max77849_read_reg(charger->max77849->i2c,
		MAX77849_PMIC_REG_MAINCTRL2, &reg_data);
	pr_info("%s: PMIC_REG_MAINCTRL2 : %d(0x%x)\n", __func__, val, reg_data);
	reg_data = charger->is_jig_attached;
	max77849_update_reg(charger->max77849->i2c, MAX77849_PMIC_REG_MAINCTRL2,
			(reg_data << MAX77849_PMIC_REG_MAINCTRL2_SHIFT),
			MAX77849_PMIC_REG_MAINCTRL2_MASK);
	max77849_read_reg(charger->max77849->i2c,
		MAX77849_PMIC_REG_MAINCTRL2, &reg_data);
	pr_info("%s: After PMIC_REG_MAINCTRL2 : %d(0x%x)\n", __func__, val, reg_data);
	wake_unlock(&charger->fg_source_change_wake_lock);
}

static int sec_chg_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct max77849_charger_data *charger =
		container_of(psy, struct max77849_charger_data, psy_chg);
	u8 reg_data;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = POWER_SUPPLY_TYPE_BATTERY;
		if (max77849_read_reg(charger->max77849->i2c,
			MAX77849_CHG_REG_CHG_INT_OK, &reg_data) == 0) {
			if (reg_data & MAX77849_CHGIN_OK)
				val->intval = POWER_SUPPLY_TYPE_MAINS;
		}
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = max77849_get_charger_state(charger);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = max77849_get_health_state(charger);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = max77849_get_input_current(charger);
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = charger->charging_current;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = max77849_get_input_current(charger);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		if (!charger->is_charging)
			val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
		else if (charger->aicl_on)
		{
			val->intval = POWER_SUPPLY_CHARGE_TYPE_SLOW;
			pr_info("%s: slow-charging mode\n", __func__);
		}
		else
			val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = max77849_get_battery_present(charger);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		break;
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = max77849_get_float_voltage(charger);
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int sec_chg_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct max77849_charger_data *charger =
		container_of(psy, struct max77849_charger_data, psy_chg);
	union power_supply_propval value, chg_now, swelling_state;
	int set_charging_current, set_charging_current_max;
	const int usb_charging_current = charger->pdata->charging_current[
		POWER_SUPPLY_TYPE_USB].fast_charging_current;
	u8 chg_cnfg_00 = 0;
        int full_check_type;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		charger->status = val->intval;
		break;
	/* val->intval : type */
	case POWER_SUPPLY_PROP_ONLINE:
		/* check and unlock */
		check_charger_unlock_state(charger);
		if (val->intval == POWER_SUPPLY_TYPE_POWER_SHARING) {
			u8 reg_mask = 0;
			psy_do_property("ps", get,
					POWER_SUPPLY_PROP_STATUS, value);
			reg_mask = (CHG_CNFG_00_OTG_MASK
					| CHG_CNFG_00_BOOST_MASK
					| CHG_CNFG_00_CHG_MASK
					| CHG_CNFG_00_BUCK_MASK
					| CHG_CNFG_00_DIS_MUIC_CTRL_MASK);
			if (value.intval) {
				chg_cnfg_00 = CHG_CNFG_00_OTG_MASK
					| CHG_CNFG_00_BOOST_MASK
					| CHG_CNFG_00_DIS_MUIC_CTRL_MASK;
				pr_info("%s: ps enable \n", __func__);
			} else {
				chg_cnfg_00 = CHG_CNFG_00_BUCK_MASK;
				pr_info("%s: ps disable\n", __func__);
			}
			max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
					chg_cnfg_00, reg_mask);
			break;
		}
		charger->cable_type = val->intval;
		if (val->intval == POWER_SUPPLY_TYPE_OTG)
			break;
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_HEALTH, value);
		if (val->intval == POWER_SUPPLY_TYPE_BATTERY) {
			charger->is_charging = false;
			charger->aicl_on = false;
			set_charging_current = 0;
			set_charging_current_max =
				charger->charging_current_max;

			if ((charger->status == POWER_SUPPLY_STATUS_DISCHARGING) ||
					(value.intval == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) ||
					(value.intval == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) {
				set_charging_current_max =
					((value.intval == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) || \
					 (value.intval == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) ?
					0 : charger->pdata->charging_current[POWER_SUPPLY_TYPE_USB].input_current_limit;
			}
		} else {
			charger->is_charging = true;
			charger->charging_current_max =
					charger->pdata->charging_current
					[charger->cable_type].input_current_limit;
			charger->charging_current =
					charger->pdata->charging_current
					[charger->cable_type].fast_charging_current;
			/* decrease the charging current according to siop level */
			set_charging_current =
				charger->charging_current * charger->siop_level / 100;
			if (set_charging_current > 0 &&
					set_charging_current < usb_charging_current)
				set_charging_current = usb_charging_current;

			set_charging_current_max =
				charger->charging_current_max;

			if (charger->siop_level < 100 &&
				set_charging_current_max > SIOP_INPUT_LIMIT_CURRENT) {
				set_charging_current_max = SIOP_INPUT_LIMIT_CURRENT;
				if (set_charging_current > SIOP_CHARGING_LIMIT_CURRENT)
					set_charging_current = SIOP_CHARGING_LIMIT_CURRENT;
			}

			psy_do_property("battery", get,
					POWER_SUPPLY_PROP_CHARGE_NOW, chg_now);

			if (chg_now.intval == SEC_BATTERY_CHARGING_1ST)
				full_check_type = charger->pdata->full_check_type;
			else
				full_check_type = charger->pdata->full_check_type_2nd;

			switch (full_check_type) {
			case SEC_BATTERY_FULLCHARGED_CHGPSY:
				/** Stop charging and enable after setting new termination current */
				max77849_set_charger_state(charger, 0);

#if defined(CONFIG_BATTERY_SWELLING)
				psy_do_property("battery", get,
						POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, swelling_state);
#else
				swelling_state.intval = 0;
#endif

				if (chg_now.intval == SEC_BATTERY_CHARGING_1ST && (!swelling_state.intval)) {
					pr_info("%s : termination current (%dmA)\n",
							__func__, charger->pdata->charging_current[
									charger->cable_type].full_check_current_1st);

					/** Setting 1st termination current as charger termination current*/
					max77849_set_topoff_current(charger,
							charger->pdata->charging_current[
									val->intval].full_check_current_1st,0);
				} else {
					pr_info("%s : termination current (%dmA)\n",
							__func__, charger->pdata->charging_current[
									charger->cable_type].full_check_current_2nd);

					/** Setting 2nd termination current as new charger termination current*/
					max77849_set_topoff_current(charger,
							charger->pdata->charging_current[
									val->intval].full_check_current_2nd,0);
				}
				break;
			}
		}
		max77849_set_charger_state(charger, charger->is_charging);
		/* if battery full, only disable charging  */
		if ((charger->status == POWER_SUPPLY_STATUS_CHARGING) ||
				(charger->status == POWER_SUPPLY_STATUS_DISCHARGING) ||
				(value.intval == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)) {
			/* current setting */
			max77849_set_charge_current(charger,
				set_charging_current);
			/* if battery is removed, disable input current and reenable input current
			  *  to enable buck always */
			if (value.intval == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE)
				max77849_set_input_current(charger, 0);
			else
				max77849_set_input_current(charger,
						set_charging_current_max);
			max77849_set_topoff_current(charger,
				charger->pdata->charging_current[
				val->intval].full_check_current_1st,
				charger->pdata->charging_current[
				val->intval].full_check_current_2nd);
		}
		break;
	/* val->intval : input charging current */
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		charger->charging_current_max = val->intval;
		break;
	/*  val->intval : charging current */
	case POWER_SUPPLY_PROP_CURRENT_AVG:
#if defined(CONFIG_BATTERY_SWELLING)
		if (val->intval > charger->pdata->charging_current
			[charger->cable_type].fast_charging_current) {
			break;
		}
#endif
		charger->charging_current = val->intval;
		max77849_set_charge_current(charger,
			val->intval);
		break;
	/* val->intval : charging current */
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		max77849_set_charge_current(charger,
				val->intval);
		max77849_set_input_current(charger,
				val->intval);
		break;
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		pr_info("%s: float voltage(%d)\n", __func__, val->intval);
		max77849_set_float_voltage(charger, val->intval);
		break;
#endif
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		charger->siop_level = val->intval;
		if (charger->is_charging) {
			/* decrease the charging current according to siop level */
			int current_now =
				charger->charging_current * val->intval / 100;

			/* do forced set charging current */
			if (current_now > 0 &&
					current_now < usb_charging_current)
				current_now = usb_charging_current;

			if (charger->cable_type == POWER_SUPPLY_TYPE_MAINS) {
				if (charger->siop_level < 100 ) {
					set_charging_current_max = SIOP_INPUT_LIMIT_CURRENT;
				} else {
					set_charging_current_max =
						charger->charging_current_max;
				}

				if (charger->siop_level < 100 &&
						current_now > SIOP_CHARGING_LIMIT_CURRENT)
					current_now = SIOP_CHARGING_LIMIT_CURRENT;
				max77849_set_input_current(charger,
					set_charging_current_max);
			}

			max77849_set_charge_current(charger, current_now);
		}
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		if(val->intval)
			charger->is_jig_attached = 1;
		else
			charger->is_jig_attached = 0;

		/* if jig attached, change the power source
		   from the VBATFG to the internal VSYS*/
		wake_lock(&charger->fg_source_change_wake_lock);
		queue_delayed_work(charger->wqueue, &charger->fg_source_change_work,
				msecs_to_jiffies(0));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static u8 get_otg_current_limit(int otg_current_limit)
{
	u8 data = 0x0;

	if(otg_current_limit <= 500){
		data = 0x0;
	} else {
		data = 0x1;
	}

	return data;
}

static void max77849_charger_initialize(struct max77849_charger_data *charger)
{
	u8 reg_data;
	pr_debug("%s\n", __func__);

	/* unmasked: CHGIN_I, WCIN_I, BATP_I, BYP_I	*/
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, 0x9a);

	/* unlock charger setting protect */
	reg_data = (0x03 << 2);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_06, reg_data);

	/*
	 * fast charge timer disable
	 * restart threshold disable
	 * pre-qual charge enable(default)
	 */
	reg_data = (0x0 << 0) | (0x03 << 4);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_01, reg_data);

	/*
	 * charge current 466mA(default)
	 * otg current limit
	 */

	if (charger->otg_current_limit){
		reg_data = get_otg_current_limit(charger->otg_current_limit);
	} else {
		reg_data = get_otg_current_limit(1200);
	}

	max77849_update_reg(charger->max77849->i2c,MAX77849_CHG_REG_CHG_CNFG_02,
			(reg_data << MAX77849_OTG_ILIM_SHIFT), MAX77849_OTG_ILIM_MASK);

	/*
	 * top off current 100mA
	 * top off timer 70min
	 */
	reg_data = (0x07 << 3);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_03, reg_data);

	/*
	 * cv voltage 4.2V or 4.35V
	 * MINVSYS 3.6V(default)
	 */
	reg_data = max77849_get_float_voltage_data(charger->pdata->chg_float_voltage);
	max77849_update_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_04,
			(reg_data << CHG_CNFG_04_CHG_CV_PRM_SHIFT),
			CHG_CNFG_04_CHG_CV_PRM_MASK);
	max77849_read_reg(charger->max77849->i2c,
			MAX77849_CHG_REG_CHG_CNFG_04, &reg_data);
	pr_info("%s: battery CV voltage : %d(0x%x)\n", __func__, charger->pdata->chg_float_voltage, reg_data);

	/** Bat to sys overcurrent threshold : 6A */
	reg_data = 0x7;
	max77849_update_reg(charger->max77849->i2c,MAX77849_CHG_REG_CHG_CNFG_12,
			(reg_data << CHG_CNFG_12_CHG_BAT_SYS_SHIFT),
			CHG_CNFG_12_CHG_BAT_SYS_MASK);

	max77849_dump_reg(charger);
}

static void sec_chg_isr_work(struct work_struct *work)
{
	struct max77849_charger_data *charger =
		container_of(work, struct max77849_charger_data, isr_work.work);

	union power_supply_propval val;

	if (charger->pdata->full_check_type ==
			SEC_BATTERY_FULLCHARGED_CHGINT) {

		val.intval = max77849_get_charger_state(charger);

		switch (val.intval) {
		case POWER_SUPPLY_STATUS_DISCHARGING:
			pr_err("%s: Interrupted but Discharging\n", __func__);
			break;

		case POWER_SUPPLY_STATUS_NOT_CHARGING:
			pr_err("%s: Interrupted but NOT Charging\n", __func__);
			break;

		case POWER_SUPPLY_STATUS_FULL:
			pr_info("%s: Interrupted by Full\n", __func__);
			psy_do_property("battery", set,
				POWER_SUPPLY_PROP_STATUS, val);
			break;

		case POWER_SUPPLY_STATUS_CHARGING:
			pr_err("%s: Interrupted but Charging\n", __func__);
			break;

		case POWER_SUPPLY_STATUS_UNKNOWN:
		default:
			pr_err("%s: Invalid Charger Status\n", __func__);
			break;
		}
	}

	if (charger->pdata->ovp_uvlo_check_type ==
			SEC_BATTERY_OVP_UVLO_CHGINT) {

		val.intval = max77849_get_health_state(charger);

		switch (val.intval) {
		case POWER_SUPPLY_HEALTH_OVERHEAT:
		case POWER_SUPPLY_HEALTH_COLD:
			pr_err("%s: Interrupted but Hot/Cold\n", __func__);
			break;

		case POWER_SUPPLY_HEALTH_DEAD:
			pr_err("%s: Interrupted but Dead\n", __func__);
			break;

		case POWER_SUPPLY_HEALTH_OVERVOLTAGE:
		case POWER_SUPPLY_HEALTH_UNDERVOLTAGE:
			pr_info("%s: Interrupted by OVP/UVLO\n", __func__);
			psy_do_property("battery", set,
				POWER_SUPPLY_PROP_HEALTH, val);
			break;

		case POWER_SUPPLY_HEALTH_UNSPEC_FAILURE:
			pr_err("%s: Interrupted but Unspec\n", __func__);
			break;

		case POWER_SUPPLY_HEALTH_GOOD:
			pr_err("%s: Interrupted but Good\n", __func__);
			break;

		case POWER_SUPPLY_HEALTH_UNKNOWN:
		default:
			pr_err("%s: Invalid Charger Health\n", __func__);
			break;
		}
	}
}

static irqreturn_t sec_chg_irq_thread(int irq, void *irq_data)
{
	struct max77849_charger_data *charger = irq_data;

	pr_info("%s: Charger interrupt occured\n", __func__);

	if ((charger->pdata->full_check_type ==
				SEC_BATTERY_FULLCHARGED_CHGINT) ||
			(charger->pdata->ovp_uvlo_check_type ==
			 SEC_BATTERY_OVP_UVLO_CHGINT))
		schedule_delayed_work(&charger->isr_work, 0);

	return IRQ_HANDLED;
}

static irqreturn_t max77849_bypass_irq(int irq, void *data)
{
	struct max77849_charger_data *chg_data = data;
#ifdef CONFIG_USB_HOST_NOTIFY
	struct otg_notify *n = get_otg_notify();
#endif
	u8 dtls_02;
	u8 byp_dtls;
	u8 chg_cnfg_00 = 0;
	u8 vbus_state;

	pr_info("%s: irq(%d)\n", __func__, irq);

	/* check and unlock */
	check_charger_unlock_state(chg_data);

	max77849_read_reg(chg_data->max77849->i2c,
				MAX77849_CHG_REG_CHG_DTLS_02,
				&dtls_02);

	byp_dtls = ((dtls_02 & MAX77849_BYP_DTLS) >>
				MAX77849_BYP_DTLS_SHIFT);
	pr_info("%s: BYP_DTLS(0x%02x)\n", __func__, byp_dtls);
	vbus_state = max77849_get_vbus_state(chg_data);

	if (byp_dtls & 0x1) {
		pr_info("%s: bypass overcurrent limit\n", __func__);
#ifdef CONFIG_USB_HOST_NOTIFY
		send_otg_notify(n, NOTIFY_EVENT_OVERCURRENT, 0);
#endif
		/* disable the register values just related to OTG and
		   keep the values about the charging */
		chg_cnfg_00 &= ~(CHG_CNFG_00_OTG_MASK
				| CHG_CNFG_00_BOOST_MASK
				| CHG_CNFG_00_DIS_MUIC_CTRL_MASK);
		max77849_update_reg(chg_data->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
				chg_cnfg_00, CHG_CNFG_00_OTG_MASK
				| CHG_CNFG_00_BOOST_MASK
				| CHG_CNFG_00_DIS_MUIC_CTRL_MASK);
		max77849_read_reg(chg_data->max77849->i2c,
			MAX77849_CHG_REG_CHG_CNFG_00, &chg_cnfg_00);
		pr_debug("%s : CHG_CNFG_00(0x%02x)\n", __func__, chg_cnfg_00);
	}
	if ((byp_dtls & 0x8) && (vbus_state < 0x03)) {
		reduce_input_current(chg_data, REDUCE_CURRENT_STEP);
	}

	return IRQ_HANDLED;
}

static void max77849_chgin_isr_work(struct work_struct *work)
{
	struct max77849_charger_data *charger = container_of(work,
				struct max77849_charger_data, chgin_work);
	u8 chgin_dtls, chg_dtls, chg_cnfg_00, reg_data;
	u8 prev_chgin_dtls = 0xff;
	int battery_health;
	union power_supply_propval value;
	int stable_count = 0;

	wake_lock(&charger->chgin_wake_lock);
	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, &reg_data);
	reg_data |= (1 << 6);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, reg_data);

	while (1) {
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_HEALTH, value);
		battery_health = value.intval;

		max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_DTLS_00,
				&chgin_dtls);
		chgin_dtls = ((chgin_dtls & MAX77849_CHGIN_DTLS) >>
				MAX77849_CHGIN_DTLS_SHIFT);
		max77849_read_reg(charger->max77849->i2c,
				MAX77849_CHG_REG_CHG_DTLS_01, &chg_dtls);
		chg_dtls = ((chg_dtls & MAX77849_CHG_DTLS) >>
				MAX77849_CHG_DTLS_SHIFT);
		max77849_read_reg(charger->max77849->i2c,
			MAX77849_CHG_REG_CHG_CNFG_00, &chg_cnfg_00);

		if (prev_chgin_dtls == chgin_dtls)
			stable_count++;
		else
			stable_count = 0;
		if (stable_count > 10) {
			pr_info("%s: irq(%d), chgin(0x%x), chg_dtls(0x%x) prev 0x%x\n",
					__func__, charger->irq_chgin,
					chgin_dtls, chg_dtls, prev_chgin_dtls);
			if (charger->is_charging) {
				if ((chgin_dtls == 0x02) && \
					(battery_health != POWER_SUPPLY_HEALTH_OVERVOLTAGE)) {
					pr_info("%s: charger is over voltage\n",
							__func__);
					value.intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
					psy_do_property("battery", set,
						POWER_SUPPLY_PROP_HEALTH, value);
				} else if (((chgin_dtls == 0x0) || (chgin_dtls == 0x01)) &&(chg_dtls & 0x08) && \
						(chg_cnfg_00 & MAX77849_MODE_BUCK) && \
						(chg_cnfg_00 & MAX77849_MODE_CHGR) && \
						(battery_health != POWER_SUPPLY_HEALTH_UNDERVOLTAGE)) { 
					pr_info("%s, vbus_state : 0x%d, chg_state : 0x%d\n", __func__, chgin_dtls, chg_dtls);
					pr_info("%s: vBus is undervoltage\n", __func__);
					value.intval = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
					psy_do_property("battery", set,
							POWER_SUPPLY_PROP_HEALTH, value);
				}
			} else {
				if ((battery_health == \
							POWER_SUPPLY_HEALTH_OVERVOLTAGE) &&
						(chgin_dtls != 0x02)) {
					pr_info("%s: vbus_state : 0x%d, chg_state : 0x%d\n", __func__, chgin_dtls, chg_dtls);
					pr_info("%s: overvoltage->normal\n", __func__);
					value.intval = POWER_SUPPLY_HEALTH_GOOD;
					psy_do_property("battery", set,
							POWER_SUPPLY_PROP_HEALTH, value);
				} else if ((battery_health == \
							POWER_SUPPLY_HEALTH_UNDERVOLTAGE) &&
						!((chgin_dtls == 0x0) || (chgin_dtls == 0x01))){
					pr_info("%s: vbus_state : 0x%d, chg_state : 0x%d\n", __func__, chgin_dtls, chg_dtls);
					pr_info("%s: undervoltage->normal\n", __func__);
					value.intval = POWER_SUPPLY_HEALTH_GOOD;
					psy_do_property("battery", set,
							POWER_SUPPLY_PROP_HEALTH, value);
					max77849_set_input_current(charger,
							charger->charging_current_max);
				}
			}
			break;
		}

		if (charger->is_charging) {
			/* reduce only at CC MODE */
			if (((chgin_dtls == 0x0) || (chgin_dtls == 0x01)) &&
					(chg_dtls == 0x01) && (stable_count > 2))
				reduce_input_current(charger, REDUCE_CURRENT_STEP);
		}
		prev_chgin_dtls = chgin_dtls;
		msleep(100);
	}
	max77849_read_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, &reg_data);
	reg_data &= ~(1 << 6);
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_INT_MASK, reg_data);
	wake_unlock(&charger->chgin_wake_lock);
}

static irqreturn_t max77849_chgin_irq(int irq, void *data)
{
	struct max77849_charger_data *charger = data;
	queue_work(charger->wqueue, &charger->chgin_work);

	return IRQ_HANDLED;
}

static int max77849_otg_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	return 0;
}

#if defined(CONFIG_MUIC_SUPPORT_LANHUB)
extern bool	lanhub_ta_status;
#endif
static int max77849_otg_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct max77849_charger_data *charger =
		container_of(psy, struct max77849_charger_data, psy_otg);

	u8 chg_cnfg_00 = 0, chg_cnfg_06 = 0, chg_cnfg_07 = 0;
#if defined(CONFIG_MUIC_SUPPORT_LANHUB)
	u8 value;
#endif

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			pr_info("%s: OTG %s\n", __func__, val->intval > 0 ? "on" : "off");
			if (val->intval) {
				if(sec_bat_check_jig_status()){      //FMBST mode set when OTG TEST for Factory mode
					chg_cnfg_07 |= CHG_CNFG_07_CHG_FMBST_MASK ;
					max77849_update_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_07,
						chg_cnfg_07, CHG_CNFG_07_CHG_FMBST_MASK );
					pr_info("%s: Boost on - update CHG_CNFG_07(0x%x)\n", __func__, chg_cnfg_07);
					mdelay(1);
				}
				chg_cnfg_00 &= ~(CHG_CNFG_00_CHG_MASK
						| CHG_CNFG_00_BUCK_MASK);
				chg_cnfg_00 |= (CHG_CNFG_00_OTG_MASK
						| CHG_CNFG_00_BOOST_MASK
						| CHG_CNFG_00_DIS_MUIC_CTRL_MASK);
#if defined(CONFIG_MUIC_SUPPORT_LANHUB)
				if(lanhub_ta_status == false) {
					pr_info("%s: lanhub_ta_status = %d \n",__func__, lanhub_ta_status);
					max77849_update_reg(charger->max77849->i2c,
							MAX77849_CHG_REG_CHG_CNFG_00,
							chg_cnfg_00,
							(CHG_CNFG_00_CHG_MASK
							 | CHG_CNFG_00_OTG_MASK
							 | CHG_CNFG_00_BUCK_MASK
							 | CHG_CNFG_00_BOOST_MASK
							 | CHG_CNFG_00_DIS_MUIC_CTRL_MASK));
				} else if (lanhub_ta_status == true) {
					pr_info("%s: lanhub_ta_status = %d \n",__func__, lanhub_ta_status);
					max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00, &value);
					value &= 0x0F;
					max77849_write_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00, value);
				}
#else
				max77849_update_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_00,
						chg_cnfg_00,
						(CHG_CNFG_00_CHG_MASK
						 | CHG_CNFG_00_OTG_MASK
						 | CHG_CNFG_00_BUCK_MASK
						 | CHG_CNFG_00_BOOST_MASK
						 | CHG_CNFG_00_DIS_MUIC_CTRL_MASK));
#endif
				/* Update CHG_CNFG_11 to 0x50(5V) */
				max77849_write_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_11, 0x50);
			} else {
				if(sec_bat_check_jig_status()){
					chg_cnfg_07 &= ~(CHG_CNFG_07_CHG_FMBST_MASK) ;
					max77849_update_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_07,
						chg_cnfg_07, CHG_CNFG_07_CHG_FMBST_MASK );
					pr_info("%s: Boost off - update CHG_CNFG_07(0x%x)\n", __func__, chg_cnfg_07);
					mdelay(1);
				}
				chg_cnfg_00 = ~(CHG_CNFG_00_OTG_MASK
						| CHG_CNFG_00_BOOST_MASK
						| CHG_CNFG_00_DIS_MUIC_CTRL_MASK);
				chg_cnfg_00 |= CHG_CNFG_00_BUCK_MASK;
				max77849_update_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_00,
						chg_cnfg_00,
						(CHG_CNFG_00_OTG_MASK
						 | CHG_CNFG_00_BUCK_MASK
						 | CHG_CNFG_00_BOOST_MASK
						 | CHG_CNFG_00_DIS_MUIC_CTRL_MASK));
				/* Update CHG_CNFG_11 to 0x00(3V) */
				max77849_write_reg(charger->max77849->i2c,
						MAX77849_CHG_REG_CHG_CNFG_11, 0x00);
				mdelay(50);
			}
			max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_00,
					&chg_cnfg_00);
			pr_info("%s: CHG_CNFG_00(0x%x)\n", __func__, chg_cnfg_00);

			max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_06,
					&chg_cnfg_06);
			pr_info("%s: CHG_CNFG_06(0x%x)\n", __func__, chg_cnfg_06);

			max77849_read_reg(charger->max77849->i2c, MAX77849_CHG_REG_CHG_CNFG_07,
					&chg_cnfg_07);
			pr_info("%s: CHG_CNFG_07(0x%x)\n", __func__, chg_cnfg_07);


			
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int max77849_debugfs_show(struct seq_file *s, void *data)
{
	struct max77849_charger_data *charger = s->private;
	u8 reg;
	u8 reg_data;

	seq_printf(s, "MAX77849 CHARGER IC :\n");
	seq_printf(s, "==================\n");
	for (reg = 0xB0; reg <= 0xC5; reg++) {
		max77849_read_reg(charger->max77849->i2c, reg, &reg_data);
		seq_printf(s, "0x%02x:\t0x%02x\n", reg, reg_data);
	}

	seq_printf(s, "\n");

	return 0;
}

static int max77849_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, max77849_debugfs_show, inode->i_private);
}

static const struct file_operations max77849_debugfs_fops = {
	.open           = max77849_debugfs_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

/* register chgin isr after sec_battery_probe */
static void max77849_chgin_init_work(struct work_struct *work)
{
	struct max77849_charger_data *charger = container_of(work,
						struct max77849_charger_data,
						chgin_init_work.work);
	int ret;

	pr_info("%s \n", __func__);
	ret = request_threaded_irq(charger->irq_chgin, NULL,
			max77849_chgin_irq, 0, "chgin-irq", charger);
	if (ret < 0) {
		pr_err("%s: fail to request chgin IRQ: %d: %d\n",
				__func__, charger->irq_chgin, ret);
	}
}

#ifdef CONFIG_OF
static int sec_charger_read_u32_index_dt(const struct device_node *np,
				       const char *propname,
				       u32 index, u32 *out_value)
{
	struct property *prop = of_find_property(np, propname, NULL);
	u32 len = (index + 1) * sizeof(*out_value);

	if (!prop)
		return (-EINVAL);
	if (!prop->value)
		return (-ENODATA);
	if (len > prop->length)
		return (-EOVERFLOW);

	*out_value = be32_to_cpup(((__be32 *)prop->value) + index);

	return 0;
}

static int sec_charger_parse_dt(struct max77849_charger_data *charger)
{
	struct device_node *np = of_find_node_by_name(NULL, "charger");
	sec_battery_platform_data_t *pdata = charger->pdata;
	int ret = 0;
	int len;

	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
	} else {
		int i;

		ret = of_property_read_u32(np, "battery,chg_float_voltage",
				&pdata->chg_float_voltage);
		if (ret < 0)
			pr_err("%s error reading battery,chg_float_voltage %d\n", __func__, ret);

		ret = of_property_read_u32(np, "battery,ovp_uvlo_check_type",
				&pdata->ovp_uvlo_check_type);
		if (ret < 0)
			pr_err("%s error reading battery,ovp_uvlo_check_type %d\n", __func__, ret);

		ret = of_property_read_u32(np, "battery,full_check_type",
				&pdata->full_check_type);
		if (ret < 0)
			pr_err("%s error reading battery,full_check_type %d\n", __func__, ret);

		ret = of_property_read_u32(np, "battery,full_check_type_2nd",
				&pdata->full_check_type_2nd);
		if (ret < 0)
			pr_err("%s error reading battery,full_check_type_2nd %d\n", __func__, ret);

		ret = of_property_read_u32(np, "battery,otg_current_limit",
				&charger->otg_current_limit);
		if (ret < 0)
			pr_err("%s error reading battery,otg_current_limit %d\n", __func__, ret);

		of_get_property(np, "battery,input_current_limit", &len);

		len = len / sizeof(u32);
		pdata->charging_current = kzalloc(sizeof(sec_charging_current_t) * len,
				GFP_KERNEL);

		for(i = 0; i < len; i++) {
			ret = sec_charger_read_u32_index_dt(np,
					"battery,input_current_limit", i,
					&pdata->charging_current[i].input_current_limit);
			if (ret < 0)
				pr_err("%s error reading battery,input_current_limit %d\n", __func__, ret);

			ret = sec_charger_read_u32_index_dt(np,
					"battery,fast_charging_current", i,
					&pdata->charging_current[i].fast_charging_current);
			if (ret < 0)
				pr_err("%s error reading battery,fast_charging_current %d\n", __func__, ret);

			ret = sec_charger_read_u32_index_dt(np,
					"battery,full_check_current_1st", i,
					&pdata->charging_current[i].full_check_current_1st);
			if (ret < 0)
				pr_err("%s error reading battery,full_check_current_1st %d\n", __func__, ret);

			ret = sec_charger_read_u32_index_dt(np,
					"battery,full_check_current_2nd", i,
					&pdata->charging_current[i].full_check_current_2nd);
			if (ret < 0)
				pr_err("%s error reading battery,full_check_current_2nd %d\n", __func__, ret);

		}

#if defined(CONFIG_MACH_MONDRIAN)
		/* check battery company*/
		if(sec_bat_check_battery_company() == BATT_TYPE_SDI) {
			for(i = 0; i < len; i++) {
				ret = sec_charger_read_u32_index_dt(np,
						"battery,full_check_current_1st_sdi", i,
						&pdata->charging_current[i].full_check_current_1st);
				if (ret < 0)
					pr_err("%s error reading battery,full_check_current_1st %d\n", __func__, ret);
			}
		}
#endif
	}

	return ret;
}
#endif

static int max77849_charger_probe(struct platform_device *pdev)
{
	struct max77849_dev *iodev = dev_get_drvdata(pdev->dev.parent);
	struct max77849_platform_data *pdata = dev_get_platdata(iodev->dev);
	struct max77849_charger_data *charger;
	int ret = 0;
	u8 reg_data;

	pr_info("%s: MAX77849 Charger driver probe\n", __func__);

	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

	pdata->charger_data = kzalloc(sizeof(sec_battery_platform_data_t), GFP_KERNEL);
	if (!pdata->charger_data)
		goto err_charger;

	charger->max77849 = iodev;
	charger->pdata = pdata->charger_data;
	charger->aicl_on = false;
	charger->siop_level = 100;

#ifdef CONFIG_OF
	if (sec_charger_parse_dt(charger))
		dev_err(&pdev->dev,
			"%s : Failed to get charger int\n", __func__);
#endif
	platform_set_drvdata(pdev, charger);

	charger->psy_chg.name           = "max77849-charger";
	charger->psy_chg.type           = POWER_SUPPLY_TYPE_UNKNOWN;
	charger->psy_chg.get_property   = sec_chg_get_property;
	charger->psy_chg.set_property   = sec_chg_set_property;
	charger->psy_chg.properties     = sec_charger_props;
	charger->psy_chg.num_properties = ARRAY_SIZE(sec_charger_props);
	charger->psy_otg.name		= "otg";
	charger->psy_otg.type		= POWER_SUPPLY_TYPE_OTG;
	charger->psy_otg.get_property	= max77849_otg_get_property;
	charger->psy_otg.set_property	= max77849_otg_set_property;
	charger->psy_otg.properties	= max77849_otg_props;
	charger->psy_otg.num_properties	= ARRAY_SIZE(max77849_otg_props);

	mutex_init(&charger->ops_lock);

	if (charger->pdata->chg_gpio_init) {
		if (!charger->pdata->chg_gpio_init()) {
			pr_err("%s: Failed to Initialize GPIO\n", __func__);
			goto err_charger_data;
		}
	}

	max77849_charger_initialize(charger);

	if (max77849_read_reg(charger->max77849->i2c, MAX77849_PMIC_REG_PMIC_ID1, &reg_data) < 0) {
		pr_err("device not found on this channel (this is not an error)\n");
		ret = -ENODEV;
		goto err_charger_data;
	} else {
		charger->pmic_ver = (reg_data & 0xf);
		pr_info("%s: device found: ver.0x%x\n", __func__,
				charger->pmic_ver);
	}

	charger->input_curr_limit_step = 25;
	charger->charging_curr_step= 400;  // 0.1mA unit

	charger->wqueue =
	    create_singlethread_workqueue(dev_name(&pdev->dev));
	if (!charger->wqueue) {
		pr_err("%s: Fail to Create Workqueue\n", __func__);
		goto err_charger_data;
	}
	wake_lock_init(&charger->chgin_wake_lock, WAKE_LOCK_SUSPEND,
			            "charger-chgin");
	INIT_WORK(&charger->chgin_work, max77849_chgin_isr_work);
	INIT_DELAYED_WORK(&charger->chgin_init_work, max77849_chgin_init_work);
	wake_lock_init(&charger->fg_source_change_wake_lock, WAKE_LOCK_SUSPEND,
			            "charger-fg_source_change");
	INIT_DELAYED_WORK(&charger->fg_source_change_work, max77849_fg_source_change_work);

	ret = power_supply_register(&pdev->dev, &charger->psy_otg);
	if (ret) {
		pr_err("%s: Failed to Register psy_otg\n", __func__);
		goto err_workqueue;
	}

	ret = power_supply_register(&pdev->dev, &charger->psy_chg);
	if (ret) {
		pr_err("%s: Failed to Register psy_chg\n", __func__);
		goto err_psy_unreg_otg;
	}

	if (charger->pdata->chg_irq) {
		INIT_DELAYED_WORK(&charger->isr_work, sec_chg_isr_work);
		ret = request_threaded_irq(charger->pdata->chg_irq,
				NULL, sec_chg_irq_thread,
				charger->pdata->chg_irq_attr,
				"charger-irq", charger);
		if (ret) {
			pr_err("%s: Failed to Reqeust IRQ\n", __func__);
			goto err_irq;
		}
	}

	charger->irq_chgin = pdata->irq_base + MAX77849_CHG_IRQ_CHGIN_I;
	/* enable chgin irq after sec_battery_probe */
	queue_delayed_work(charger->wqueue, &charger->chgin_init_work,
			msecs_to_jiffies(3000));

	charger->irq_bypass = pdata->irq_base + MAX77849_CHG_IRQ_BYP_I;
	ret = request_threaded_irq(charger->irq_bypass, NULL,
			max77849_bypass_irq, 0, "bypass-irq", charger);
	if (ret < 0)
		pr_err("%s: fail to request bypass IRQ: %d: %d\n",
				__func__, charger->irq_bypass, ret);

	max77849_dentry = debugfs_create_file("max77849-regs",
		S_IRUSR, NULL, charger, &max77849_debugfs_fops);

	return 0;
err_irq:
	power_supply_unregister(&charger->psy_chg);
err_psy_unreg_otg:
	power_supply_unregister(&charger->psy_otg);
err_workqueue:
	destroy_workqueue(charger->wqueue);
err_charger_data:
	kfree(pdata->charger_data);
err_charger:
	kfree(charger);

	return ret;

}

static int max77849_charger_remove(struct platform_device *pdev)
{
	struct max77849_charger_data *charger =
				platform_get_drvdata(pdev);

	destroy_workqueue(charger->wqueue);
	free_irq(charger->pdata->chg_irq, NULL);
	power_supply_unregister(&charger->psy_otg);
	power_supply_unregister(&charger->psy_chg);
	kfree(charger);

	return 0;
}

#if defined CONFIG_PM
static int max77849_charger_suspend(struct device *dev)
{
	return 0;
}

static int max77849_charger_resume(struct device *dev)
{
	return 0;
}
#else
#define max77849_charger_suspend NULL
#define max77849_charger_resume NULL
#endif

static void max77849_charger_shutdown(struct device *dev)
{
	struct max77849_charger_data *charger = dev_get_drvdata(dev);
	u8 reg_data;

	pr_info("%s: MAX77849 Charger driver shutdown\n", __func__);
	if (!charger->max77849->i2c) {
		pr_err("%s: no max77849 i2c client\n", __func__);
		return;
	}
	reg_data = 0x05;
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_00, reg_data);
	reg_data = 0x19;
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_09, reg_data);
	reg_data = 0x19;
	max77849_write_reg(charger->max77849->i2c,
		MAX77849_CHG_REG_CHG_CNFG_10, reg_data);
	pr_info("func:%s \n", __func__);
}

static SIMPLE_DEV_PM_OPS(max77849_charger_pm_ops, max77849_charger_suspend,
		max77849_charger_resume);

static struct platform_driver max77849_charger_driver = {
	.driver = {
		.name = "max77849-charger",
		.owner = THIS_MODULE,
		.pm = &max77849_charger_pm_ops,
		.shutdown = max77849_charger_shutdown,
	},
	.probe = max77849_charger_probe,
	.remove = max77849_charger_remove,
};

static int __init max77849_charger_init(void)
{
	pr_info("func:%s\n", __func__);
	return platform_driver_register(&max77849_charger_driver);
}
module_init(max77849_charger_init);

static void __exit max77849_charger_exit(void)
{
	platform_driver_unregister(&max77849_charger_driver);
}

module_exit(max77849_charger_exit);

MODULE_DESCRIPTION("max77849 charger driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
