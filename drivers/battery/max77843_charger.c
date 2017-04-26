/*
 *  max77843_charger.c
 *  Samsung MAX77843 Charger Driver
 *
 *  Copyright (C) 2012 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define DEBUG

#include <linux/mfd/max77843.h>
#include <linux/mfd/max77843-private.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/usb_notify.h>
#endif
#include <linux/battery/charger/max77843_charger.h>
#include <linux/battery/sec_battery.h>

#define ENABLE 1
#define DISABLE 0

static enum power_supply_property max77843_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_USB_HC,
#if defined(CONFIG_BATTERY_SWELLING)
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
#endif
	POWER_SUPPLY_PROP_CHARGE_NOW,
};
static enum power_supply_property max77843_otg_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

ssize_t max77843_chip_id_check_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf);
#define MAX77843_CHARGER_ATTR(_name)			\
{							\
	.attr = {.name = #_name, .mode = 0444},	\
	.show = max77843_chip_id_check_show_attrs,		\
	.store = NULL,			\
}
static struct device_attribute max77843_charger_attrs[] = {
	MAX77843_CHARGER_ATTR(chip_id),
};

static void max77843_charger_initialize(struct max77843_charger_data *charger);
static int max77843_get_vbus_state(struct max77843_charger_data *charger);
static int max77843_get_charger_state(struct max77843_charger_data *charger);
static void reduce_input_current(struct max77843_charger_data *charger, int cur);
static void max77843_set_input_current(struct max77843_charger_data *charger, int input_current);
static void max77843_set_charger_state(struct max77843_charger_data *charger, int enable);
static bool max77843_charger_unlock(struct max77843_charger_data *charger)
{
	u8 reg_data;
	u8 chgprot;
	int retry_cnt = 0;
	bool need_init = false;

	do {
		max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_06, &reg_data);
		chgprot = ((reg_data & 0x0C) >> 2);
		if (chgprot != 0x03) {
			pr_err("%s: unlock err, chgprot(0x%x), retry(%d)\n",
					__func__, chgprot, retry_cnt);
			max77843_write_reg(charger->i2c, MAX77843_CHG_REG_CNFG_06,
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

static void check_charger_unlock_state(struct max77843_charger_data *charger)
{
	bool need_reg_init;
	pr_debug("%s\n", __func__);

	need_reg_init = max77843_charger_unlock(charger);
	if (need_reg_init) {
		pr_err("%s: charger locked state, reg init\n", __func__);
		max77843_charger_initialize(charger);
	}
}

static void max77843_test_read(struct max77843_charger_data *charger)
{
	u8 data = 0;
	u32 addr = 0;
	for (addr = 0xB0; addr <= 0xC3; addr++) {
		max77843_read_reg(charger->i2c, addr, &data);
		pr_debug("MAX77843 addr : 0x%02x data : 0x%02x\n", addr, data);
	}
}

static int max77843_get_vbus_state(struct max77843_charger_data *charger)
{
	u8 reg_data;

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_DETAILS_00, &reg_data);

	if (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS)
		reg_data = ((reg_data & MAX77843_WCIN_DTLS) >>
			    MAX77843_WCIN_DTLS_SHIFT);
	else
		reg_data = ((reg_data & MAX77843_CHGIN_DTLS) >>
			    MAX77843_CHGIN_DTLS_SHIFT);

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
		pr_info("%s: VBUS is valid. CHGIN < CHGIN_OVLO", __func__);
		break;
	default:
		break;
	}

	return reg_data;
}

static int max77843_get_charger_state(struct max77843_charger_data *charger)
{
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	u8 reg_data;

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_DETAILS_01, &reg_data);

	pr_info("%s : charger status (0x%02x)\n", __func__, reg_data);

	reg_data &= 0x0f;

	switch (reg_data)
	{
	case 0x00:
	case 0x01:
	case 0x02:
		status = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case 0x03:
	case 0x04:
		status = POWER_SUPPLY_STATUS_FULL;
		break;
	case 0x05:
	case 0x06:
	case 0x07:
		status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case 0x08:
	case 0xA:
	case 0xB:
		status = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	default:
		status = POWER_SUPPLY_STATUS_UNKNOWN;
		break;
	}

	return (int)status;
}

static int max77843_get_charging_health(struct max77843_charger_data *charger)
{
	int state;
	int vbus_state;
	int retry_cnt;
	u8 chg_dtls_00, chg_dtls, reg_data;
	u8 chg_cnfg_00, chg_cnfg_01 ,chg_cnfg_02, chg_cnfg_04, chg_cnfg_09, chg_cnfg_12;

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_DETAILS_01, &reg_data);
	reg_data = ((reg_data & MAX77843_BAT_DTLS) >> MAX77843_BAT_DTLS_SHIFT);

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
		vbus_state = max77843_get_vbus_state(charger);
		/* read CHG_DTLS and detecting battery terminal error */
		max77843_read_reg(charger->i2c,
				  MAX77843_CHG_REG_DETAILS_01, &chg_dtls);
		chg_dtls = ((chg_dtls & MAX77843_CHG_DTLS) >>
			    MAX77843_CHG_DTLS_SHIFT);
		max77843_read_reg(charger->i2c,
				  MAX77843_CHG_REG_CNFG_00, &chg_cnfg_00);

		/* print the log at the abnormal case */
		if((charger->is_charging == 1) && (chg_dtls & 0x08)) {
			max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_DETAILS_00, &chg_dtls_00);
			max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_01, &chg_cnfg_01);
			max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_02, &chg_cnfg_02);
			max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_04, &chg_cnfg_04);
			max77843_read_reg(charger->i2c,
					MAX77843_CHG_REG_CNFG_09, &chg_cnfg_09);
			max77843_read_reg(charger->i2c,
					MAX77843_CHG_REG_CNFG_12, &chg_cnfg_12);

			pr_info("%s: CHG_DTLS_00(0x%x), CHG_DTLS_01(0x%x), CHG_CNFG_00(0x%x)\n",
				__func__, chg_dtls_00, chg_dtls, chg_cnfg_00);
			pr_info("%s:  CHG_CNFG_01(0x%x), CHG_CNFG_02(0x%x), CHG_CNFG_04(0x%x)\n",
				__func__, chg_cnfg_01, chg_cnfg_02, chg_cnfg_04);
			pr_info("%s:  CHG_CNFG_09(0x%x), CHG_CNFG_12(0x%x)\n",
				__func__, chg_cnfg_09, chg_cnfg_12);
			max77843_set_charger_state(charger, 0);
			max77843_set_charger_state(charger, 1);
		}

		pr_info("%s: vbus_state : 0x%d, chg_dtls : 0x%d\n", __func__, vbus_state, chg_dtls);
		/*  OVP is higher priority */
		if (vbus_state == 0x02) { /*  CHGIN_OVLO */
			pr_info("%s: vbus ovp\n", __func__);
			state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			if (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
				retry_cnt = 0;
				do {
					msleep(50);
					vbus_state = max77843_get_vbus_state(charger);
				} while((retry_cnt++ < 2) && (vbus_state == 0x02));
				if (vbus_state == 0x02) {
					state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
					pr_info("%s: wpc and over-voltage\n", __func__);
				} else
					state = POWER_SUPPLY_HEALTH_GOOD;
			}
		} else if (((vbus_state == 0x0) || (vbus_state == 0x01)) && (chg_dtls & 0x08) && \
				(chg_cnfg_00 & MAX77843_MODE_BUCK) && \
				(chg_cnfg_00 & MAX77843_MODE_CHGR) && \
				(charger->cable_type != POWER_SUPPLY_TYPE_WIRELESS)) {
			pr_info("%s: vbus is under\n", __func__);
			state = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
		} else if ((value.intval == POWER_SUPPLY_HEALTH_UNDERVOLTAGE) && \
				((vbus_state == 0x0) || (vbus_state == 0x01)) && \
				(charger->cable_type != POWER_SUPPLY_TYPE_WIRELESS)) {
			pr_info("%s: keep under-voltage\n", __func__);
			state = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
		}
	}

	max77843_test_read(charger);

	return (int)state;
}

static u8 max77843_get_float_voltage_data(int float_voltage)
{
	int voltage = 3650;
	int i;

	for (i = 0; voltage <= 4700; i++) {
		if (float_voltage <= voltage)
			break;
		voltage += 25;
	}

	if (float_voltage <= 4340)
		return i;
	else
		return (i+1);
}

static int max77843_get_input_current(struct max77843_charger_data *charger)
{
	u8 reg_data;
	int get_current = 0;

	if (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
		max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_10, &reg_data);
		/* AND operation for removing the formal 2bit  */
		reg_data = reg_data & 0x3F;

		if (reg_data <= 0x3)
			get_current = 60;
		else
			get_current = reg_data * 20;
	} else {
		max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_09, &reg_data);
		/* AND operation for removing the formal 1bit  */
		reg_data = reg_data & 0x7F;

		if (reg_data <= 0x3) {
			get_current = 100;
		} else if (reg_data >= 0x78) {
			get_current = 4000;
		} else {
			int quotient, remainder;
			quotient = reg_data / 3;
			remainder = reg_data % 3;
			if (remainder == 0)
				get_current = quotient * 100;
			else if (remainder == 1)
				get_current = quotient * 100 + 33;
			else
				get_current = quotient * 100 + 67;
		}
	}

	return get_current;
}

static bool max77843_check_battery(struct max77843_charger_data *charger)
{
	u8 reg_data;
	u8 reg_data2;

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_OK, &reg_data);

	pr_info("%s : CHG_INT_OK(0x%x)\n", __func__, reg_data);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_DETAILS_00, &reg_data2);

	pr_info("%s : CHG_DETAILS00(0x%x)\n", __func__, reg_data2);

	if ((reg_data & MAX77843_BATP_OK) ||
	    !(reg_data2 & MAX77843_BATP_DTLS))
		return true;
	else
		return false;
}

static void max77843_set_buck(struct max77843_charger_data *charger,
		int enable)
{
	u8 reg_data;

	if (enable) {
		max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				CHG_CNFG_00_BUCK_MASK, CHG_CNFG_00_BUCK_MASK);
	} else {
		max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				0, CHG_CNFG_00_BUCK_MASK);
	}
	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00, &reg_data);
	pr_debug("%s : CHG_CNFG_00(0x%02x)\n", __func__, reg_data);
}

static void max77843_check_slow_charging(struct max77843_charger_data *charger,
		int input_current)
{
	/* under 400mA considered as slow charging concept for VZW */
	if (input_current <= SLOW_CHARGING_CURRENT_STANDARD &&
			charger->cable_type != POWER_SUPPLY_TYPE_BATTERY) {
		union power_supply_propval value;

		charger->aicl_on = true;
		pr_info("%s: slow charging on : input current(%dmA), cable type(%d)\n",
			__func__, input_current, charger->cable_type);

		psy_do_property("battery", set,
			POWER_SUPPLY_PROP_CHARGE_TYPE, value);
	}
	else
		charger->aicl_on = false;
}

static void max77843_change_charge_path(struct max77843_charger_data *charger,
		int path)
{
	u8 cnfg12;

	if (path == POWER_SUPPLY_TYPE_WIRELESS) {
		cnfg12 = (0 << CHG_CNFG_12_CHGINSEL_SHIFT);
	} else {
		cnfg12 = (1 << CHG_CNFG_12_CHGINSEL_SHIFT);
	}

	max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_12,
			cnfg12,	CHG_CNFG_12_CHGINSEL_MASK);
}

static void max77843_set_input_current(struct max77843_charger_data *charger,
				       int input_current)
{
	u8 reg_data = 0, set_current_reg;
	int quotient, remainder;

	mutex_lock(&charger->charger_mutex);
	max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, MAX77843_CHGIN_IM, MAX77843_CHGIN_IM);

	/* disable only buck because power onoff test issue */
	if (input_current <= 0) {
		max77843_write_reg(charger->i2c,
				   MAX77843_CHG_REG_CNFG_09, 0x0F);
		max77843_set_buck(charger, DISABLE);
		goto exit;
	} else {
		input_current = (input_current > charger->charging_current_max) ?
			charger->charging_current_max : input_current;
		max77843_change_charge_path(charger, charger->cable_type);
		max77843_set_buck(charger, ENABLE);
	}

	if (charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
		set_current_reg = input_current / charger->input_curr_limit_step;
		max77843_write_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_09, set_current_reg);
		goto exit;
	} else if (charger->cable_type == POWER_SUPPLY_TYPE_HV_MAINS ||
		charger->cable_type == POWER_SUPPLY_TYPE_HV_ERR ||
		charger->cable_type == POWER_SUPPLY_TYPE_HV_UNKNOWN) {
		cancel_delayed_work_sync(&charger->afc_work);

		quotient = input_current / 100;
		remainder = input_current % 100;

		if (remainder >= 67)
			reg_data |= (quotient * 3) + 2;
		else if (remainder >= 33)
			reg_data |= (quotient * 3) + 1;
		else if (remainder < 33)
			reg_data |= quotient * 3;

		max77843_write_reg(charger->i2c,
				   MAX77843_CHG_REG_CNFG_09, reg_data);
		pr_info("[3][HV TA][0x%x]\n", reg_data);
		max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_01, &reg_data);

		if (!(reg_data & 0x08)) {
			reg_data |= 0x08;
			max77843_write_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_01, reg_data);
		}
	} else if (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
		quotient = input_current / 20;
		reg_data |= quotient;
		max77843_write_reg(charger->i2c,
				   MAX77843_CHG_REG_CNFG_10, reg_data);
		max77843_update_reg(charger->i2c,
				MAX77843_CHG_REG_INT_MASK, 0, MAX77843_CHGIN_IM);
		mutex_unlock(&charger->charger_mutex);
		return;
	} else {
		set_current_reg = input_current / charger->input_curr_limit_step;

		pr_info("%s: reg_data(0x%02x), input(%d)\n",
			__func__, set_current_reg, input_current);

		max77843_write_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_09, set_current_reg);
	}

exit:
	/* slow charging check */
	max77843_check_slow_charging(charger, input_current);

	max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, 0, MAX77843_CHGIN_IM);
	mutex_unlock(&charger->charger_mutex);
}

static void max77843_set_charge_current(struct max77843_charger_data *charger,
					int fast_charging_current)
{
	int curr_step = 50;
	u8 reg_data;

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_CNFG_02, &reg_data);
	reg_data &= ~MAX77843_CHG_CC;

	if (!fast_charging_current) {
		max77843_write_reg(charger->i2c,
				   MAX77843_CHG_REG_CNFG_02, reg_data);
	} else {
		reg_data |= (fast_charging_current / curr_step);

		max77843_write_reg(charger->i2c,MAX77843_CHG_REG_CNFG_02, reg_data);
	}

	pr_info("[%s] REG(0x%02x) DATA(0x%02x), CURRENT(%d)\n",
		__func__, MAX77843_CHG_REG_CNFG_02,
		reg_data, fast_charging_current);
}

static int max77843_check_aicl_state(struct max77843_charger_data *charger)
{
	u8 aicl_state;
	if (!max77843_read_reg(charger->i2c, MAX77843_CHG_REG_INT_OK, &aicl_state)) {
		return !(aicl_state & 0x80);
	}
	return 0;
}

static void max77843_set_current(struct max77843_charger_data *charger)
{
	int current_now = charger->charging_current,
		current_max = charger->charging_current_max;
	int usb_current = charger->pdata->charging_current[
		POWER_SUPPLY_TYPE_USB].fast_charging_current;

	if (charger->is_charging) {
		/* decrease the charging current according to siop level */
		current_now = current_now * charger->siop_level / 100;

		/* do forced set charging current */
		if (current_now > 0 && current_now < usb_current)
			current_now = usb_current;

		if (charger->siop_level < 100) {
			if (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
				current_max = SIOP_WIRELESS_INPUT_LIMIT_CURRENT;
				if (current_now > SIOP_WIRELESS_CHARGING_LIMIT_CURRENT)
					current_now = SIOP_WIRELESS_CHARGING_LIMIT_CURRENT;
			} else {
				if (current_max > SIOP_INPUT_LIMIT_CURRENT)
					current_max = SIOP_INPUT_LIMIT_CURRENT;
				if (current_now > SIOP_CHARGING_LIMIT_CURRENT)
					current_now = SIOP_CHARGING_LIMIT_CURRENT;
			}
		}
	}

	if (max77843_check_aicl_state(charger)) {
		wake_lock(&charger->aicl_wake_lock);
		queue_delayed_work(charger->wqueue, &charger->aicl_work,
				msecs_to_jiffies(50));
	}
	max77843_set_input_current(charger, current_max);
	max77843_set_charge_current(charger, current_now);

	max77843_test_read(charger);
}

static void afc_detect_work(struct work_struct *work)
{
	struct max77843_charger_data *charger = container_of(work,
							     struct max77843_charger_data,
							     afc_work.work);

	if (charger->is_charging && charger->afc_detect) {
		charger->afc_detect = false;

		if (charger->charging_current_max >= INPUT_CURRENT_TA) {
			charger->charging_current_max = charger->pdata->charging_current[
					POWER_SUPPLY_TYPE_MAINS].input_current_limit;
		}

		pr_info("%s: current_max(%d)\n", __func__, charger->charging_current_max);
		max77843_set_current(charger);
	}
}

static void max77843_set_topoff_current(struct max77843_charger_data *charger,
					int termination_current,
					int termination_time)
{
	int curr_base, curr_step;
	u8 reg_data;

	if (charger->pmic_ver >= 0x2) {
		curr_base = 125;
		curr_step = 75;
		if (termination_current < curr_base)
			termination_current = curr_base;
		else if (termination_current > 650)
			termination_current = 650;
	} else {
		curr_base = 100;
		curr_step = 50;
		if (termination_current < curr_base)
			termination_current = curr_base;
		else if (termination_current > 450)
			termination_current = 450;
	}
	reg_data = (termination_current - curr_base) / curr_step;
	max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_03,
		reg_data, 0x7);

	pr_info("%s: reg_data(0x%02x), topoff(%d)\n",
		__func__, reg_data, termination_current);
}

static void max77843_set_charger_state(struct max77843_charger_data *charger,
	int enable)
{
	u8 reg_data;

	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00, &reg_data);

	if (enable) {
		max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				CHG_CNFG_00_CHG_MASK, CHG_CNFG_00_CHG_MASK);
	} else {
		max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				0, CHG_CNFG_00_CHG_MASK);
	}
	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00, &reg_data);
	pr_debug("%s : CHG_CNFG_00(0x%02x)\n", __func__, reg_data);
}

static void reduce_input_current(struct max77843_charger_data *charger, int cur)
{
	u8 set_reg;
	u8 set_value;
	unsigned int min_input_current = 0;

	set_reg = MAX77843_CHG_REG_CNFG_09;
	min_input_current = MINIMUM_INPUT_CURRENT;

	if (!max77843_read_reg(charger->i2c, set_reg, &set_value)) {
		if ((set_value <= (min_input_current / charger->input_curr_limit_step)) ||
		    (set_value <= (cur / charger->input_curr_limit_step)))
			return;
		set_value -= (cur / charger->input_curr_limit_step);
		set_value = (set_value < (min_input_current / charger->input_curr_limit_step)) ?
			(min_input_current / charger->input_curr_limit_step) : set_value;
		max77843_write_reg(charger->i2c, set_reg, set_value);
		charger->charging_current_max = max77843_get_input_current(charger);
		pr_info("%s: set current: reg:(0x%x), val:(0x%x), input_current(%d)\n",
			__func__, set_reg, set_value, charger->charging_current_max);
	}
}

static void max77843_charger_function_control(
				struct max77843_charger_data *charger)
{
	u8 reg_data;
	union power_supply_propval value;
	union power_supply_propval chg_mode;

	psy_do_property("battery", get, POWER_SUPPLY_PROP_HEALTH, value);

	pr_info("####%s####\n", __func__);

	if (charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
		charger->is_charging = false;
		charger->afc_detect = false;
		charger->aicl_on = false;
#if defined(CONFIG_MUIC_SUPPORT_MULTIMEDIA_DOCK)
		charger->is_mdock = false;
#endif
		charger->charging_current = 0;
		charger->charging_current_max =
			((value.intval == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) || \
				(value.intval == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) ?
			0 : charger->pdata->charging_current[POWER_SUPPLY_TYPE_BATTERY].input_current_limit;

		max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
			CHG_CNFG_00_BUCK_MASK, CHG_CNFG_00_CHG_MASK | CHG_CNFG_00_BUCK_MASK | CHG_CNFG_00_OTG_CTRL);

		max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_01, &reg_data);
		if (reg_data & 0x08) {
			reg_data &= ~(0x08);
			max77843_write_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_01, reg_data);
		}
	} else {
		charger->is_charging = true;
		charger->afc_detect = false;
		charger->charging_current_max =
			charger->pdata->charging_current
			[charger->cable_type].input_current_limit;
		charger->charging_current =
			charger->pdata->charging_current
			[charger->cable_type].fast_charging_current;
#if defined(CONFIG_MUIC_SUPPORT_MULTIMEDIA_DOCK)
		if (charger->is_mdock) { /* if mdock was already inserted, then check OTG, or NOTG state */
			if (charger->cable_type == POWER_SUPPLY_TYPE_SMART_NOTG) {
				charger->charging_current =
					charger->pdata->charging_current
					[POWER_SUPPLY_TYPE_MDOCK_TA].fast_charging_current;
				charger->charging_current_max =
					charger->pdata->charging_current
					[POWER_SUPPLY_TYPE_MDOCK_TA].input_current_limit;
			} else if (charger->cable_type == POWER_SUPPLY_TYPE_SMART_OTG) {
				charger->charging_current =
					charger->pdata->charging_current
					[POWER_SUPPLY_TYPE_MDOCK_TA].fast_charging_current - 500;
				charger->charging_current_max =
					charger->pdata->charging_current
					[POWER_SUPPLY_TYPE_MDOCK_TA].input_current_limit - 500;
			}
		} else { /* if mdock wasn't inserted, then check mdock state */
			if (charger->cable_type == POWER_SUPPLY_TYPE_MDOCK_TA)
				charger->is_mdock = true;
		}
#endif
		if (charger->cable_type == POWER_SUPPLY_TYPE_MAINS) {
			charger->afc_detect = true;
			charger->charging_current_max = INPUT_CURRENT_TA;
			queue_delayed_work(charger->wqueue, &charger->afc_work, msecs_to_jiffies(2000));
			wake_lock_timeout(&charger->afc_wake_lock, HZ * 3);
		}
	}

	if (charger->pdata->full_check_type_2nd == SEC_BATTERY_FULLCHARGED_CHGPSY) {
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_CHARGE_NOW,
				chg_mode);

		if (chg_mode.intval == SEC_BATTERY_CHARGING_2ND) {
			max77843_set_charger_state(charger, 0);
			max77843_set_topoff_current(charger,
						    charger->pdata->charging_current[
							    charger->cable_type].full_check_current_2nd,
						    (70 * 60));
		} else {
			max77843_set_topoff_current(charger,
						    charger->pdata->charging_current[
							    charger->cable_type].full_check_current_1st,
						    (70 * 60));
		}
	} else {
		max77843_set_topoff_current(charger,
					    charger->pdata->charging_current[
						    charger->cable_type].full_check_current_1st,
					    charger->pdata->charging_current[
						    charger->cable_type].full_check_current_2nd);
	}

	max77843_set_charger_state(charger, charger->is_charging);

	pr_info("charging = %d, fc = %d, il = %d, t1 = %d, t2 = %d, cable = %d\n",
		charger->is_charging,
		charger->charging_current,
		charger->charging_current_max,
		charger->pdata->charging_current[charger->cable_type].full_check_current_1st,
		charger->pdata->charging_current[charger->cable_type].full_check_current_2nd,
		charger->cable_type);
}

static void max77843_charger_initialize(struct max77843_charger_data *charger)
{
	u8 reg_data;
	pr_info("%s\n", __func__);

	/* unmasked: CHGIN_I, WCIN_I, BATP_I, BYP_I	*/
	/*max77843_write_reg(charger->i2c, MAX77843_CHG_REG_INT_MASK, 0x9a);*/

	/* unlock charger setting protect */
	reg_data = (0x03 << 2);
	max77843_write_reg(charger->i2c, MAX77843_CHG_REG_CNFG_06, reg_data);

	/*
	 * fast charge timer disable
	 * restart threshold disable
	 * pre-qual charge enable(default)
	 */
	reg_data = (0x03 << 4);
	max77843_write_reg(charger->i2c, MAX77843_CHG_REG_CNFG_01, reg_data);

	/*
	 * charge current 466mA(default)
	 * otg current limit 1200mA
	 */
	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_02, &reg_data);
	reg_data |= (1 << 7);
	max77843_write_reg(charger->i2c, MAX77843_CHG_REG_CNFG_02, reg_data);

	/*
	 * top off current 100mA
	 * top off timer 70min
	 */
	reg_data = 0x38;
	max77843_write_reg(charger->i2c, MAX77843_CHG_REG_CNFG_03, reg_data);

#if defined(CONFIG_CHARGER_SMB1357)
	/* Disable WCIN */
	reg_data = 0x27;
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_12, reg_data);
#endif

	/* Disable OCP */
	max77843_update_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_12, 0, CHG_CNFG_12_B2SOVRC_MASK);

	/*
	 * cv voltage 4.2V or 4.35V
	 * MINVSYS 3.6V(default)
	 */
	reg_data = max77843_get_float_voltage_data(charger->pdata->chg_float_voltage);
	max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_04,
			(reg_data << CHG_CNFG_04_CHG_CV_PRM_SHIFT),
			CHG_CNFG_04_CHG_CV_PRM_MASK);
	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_04, &reg_data);
	pr_info("%s: battery cv voltage 0x%x\n", __func__, reg_data);

	max77843_test_read(charger);
}

#if defined(CONFIG_BATTERY_SWELLING)
static void max77843_set_float_voltage(struct max77843_charger_data *charger, int float_voltage)
{
	u8 reg_data = 0;

	reg_data = max77843_get_float_voltage_data(float_voltage);
	max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_04,
			(reg_data << CHG_CNFG_04_CHG_CV_PRM_SHIFT),
			CHG_CNFG_04_CHG_CV_PRM_MASK);
	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_04, &reg_data);
	pr_info("%s: battery cv voltage 0x%x\n", __func__, reg_data);
}

static u8 max77843_get_float_voltage(struct max77843_charger_data *charger)
{
	u8 reg_data = 0;

	max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_04, &reg_data);
	reg_data &= 0x3F;
	pr_info("%s: battery cv voltage 0x%x\n", __func__, reg_data);
	return reg_data;
}

#endif

static int max77843_chg_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(max77843_charger_attrs); i++) {
		rc = device_create_file(dev, &max77843_charger_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	return rc;

create_attrs_failed:
	dev_err(dev, "%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &max77843_charger_attrs[i]);
	return rc;
}

ssize_t max77843_chip_id_check_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct max77843_charger_data *charger =
		container_of(psy, struct max77843_charger_data, psy_chg);

	return scnprintf(buf, PAGE_SIZE, "%s\n",
		(charger->pmic_ver >= 0x1 && charger->pmic_ver <= 0x3) ? "MAX77843" : "Unknown");
}

static int max77843_chg_get_property(struct power_supply *psy,
			      enum power_supply_property psp,
			      union power_supply_propval *val)
{
	struct max77843_charger_data *charger =
		container_of(psy, struct max77843_charger_data, psy_chg);
	u8 reg_data;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = POWER_SUPPLY_TYPE_BATTERY;
		if (max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_INT_OK, &reg_data) == 0) {
			if (reg_data & MAX77843_WCIN_OK) {
				val->intval = POWER_SUPPLY_TYPE_WIRELESS;
				charger->wc_w_state = 1;
			} else if (reg_data & MAX77843_CHGIN_OK) {
				val->intval = POWER_SUPPLY_TYPE_MAINS;
			}
		}
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = max77843_check_battery(charger);
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = max77843_get_charger_state(charger);
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
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = max77843_get_charging_health(charger);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = max77843_get_input_current(charger);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = max77843_get_input_current(charger);
		pr_debug("%s : set-current(%dmA), current now(%dmA)\n",
			__func__, charger->charging_current, val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = max77843_get_float_voltage(charger);
		break;
#endif
	case POWER_SUPPLY_PROP_USB_HC:
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		max77843_read_reg(charger->i2c,
				  MAX77843_CHG_REG_DETAILS_01, &reg_data);
		if (reg_data & 0x01)
			val->strval = "CC Mode";
		else if (reg_data & 0x02)
			val->strval = "CV Mode";
		else
			val->strval = "NONE";
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int max77843_chg_set_property(struct power_supply *psy,
			  enum power_supply_property psp,
			  const union power_supply_propval *val)
{
	struct max77843_charger_data *charger =
		container_of(psy, struct max77843_charger_data, psy_chg);
	union power_supply_propval value;

	switch (psp) {
	/* val->intval : type */
	case POWER_SUPPLY_PROP_STATUS:
		charger->status = val->intval;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* check and unlock */
		check_charger_unlock_state(charger);
		if (val->intval == POWER_SUPPLY_TYPE_POWER_SHARING) {
			psy_do_property("ps", get,
				POWER_SUPPLY_PROP_STATUS, value);
			if (value.intval) {
				max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
					CHG_CNFG_00_OTG_CTRL, CHG_CNFG_00_OTG_CTRL);
			} else {
				max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
					0, CHG_CNFG_00_OTG_CTRL);
			}
			break;
		}

		charger->cable_type = val->intval;
		max77843_charger_function_control(charger);
		max77843_set_current(charger);
		break;
	/* val->intval : input charging current */
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		max77843_set_input_current(charger, val->intval);
		break;
	/*  val->intval : charging current */
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		max77843_set_charge_current(charger, val->intval);
		break;
	/* val->intval : charging current */
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		max77843_set_charge_current(charger,
			val->intval);
		max77843_set_input_current(charger,
			val->intval);
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		pr_info("%s: float voltage(%d)\n", __func__, val->intval);
		max77843_set_float_voltage(charger, val->intval);
		break;
#endif
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		charger->siop_level = val->intval;
		max77843_set_current(charger);
		break;
	case POWER_SUPPLY_PROP_USB_HC:
		/* set input/charging current for usb up to TA's current */
		if (val->intval) {
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_USB].fast_charging_current =
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_MAINS].fast_charging_current;
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_USB].input_current_limit =
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_MAINS].input_current_limit;
		/* restore input/charging current for usb */
		} else {
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_USB].fast_charging_current =
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_BATTERY].input_current_limit;
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_USB].input_current_limit =
			charger->pdata->charging_current[
				POWER_SUPPLY_TYPE_BATTERY].input_current_limit;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int max77843_otg_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	return 0;
}

static int max77843_otg_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct max77843_charger_data *charger =
		container_of(psy, struct max77843_charger_data, psy_otg);
	static u8 chg_int_state;
	u8 chg_cnfg_00;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		pr_info("%s: OTG %s\n", __func__, val->intval > 0 ? "on" : "off");

		if (val->intval) {
			max77843_read_reg(charger->i2c, MAX77843_CHG_REG_INT_MASK,
				&chg_int_state);

			/* disable charger interrupt: CHG_I, CHGIN_I */
			/* enable charger interrupt: BYP_I */
			max77843_update_reg(charger->i2c, MAX77843_CHG_REG_INT_MASK,
				MAX77843_CHG_IM | MAX77843_CHGIN_IM,
				MAX77843_CHG_IM | MAX77843_CHGIN_IM | MAX77843_BYP_IM);

			/* OTG on, boost on */
			max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				CHG_CNFG_00_OTG_CTRL, CHG_CNFG_00_OTG_CTRL);

			/* Update CHG_CNFG_11 to 0x50(5V) */
			max77843_write_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_11, 0x50);
		} else {
			/* OTG off, boost off, (buck on) */
			max77843_update_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
				CHG_CNFG_00_BUCK_MASK, CHG_CNFG_00_BUCK_MASK | CHG_CNFG_00_OTG_CTRL);

			/* Update CHG_CNFG_11 to 0x00(3V) */
			max77843_write_reg(charger->i2c,
				MAX77843_CHG_REG_CNFG_11, 0x00);
			mdelay(50);

			/* enable charger interrupt */
			max77843_write_reg(charger->i2c,
				MAX77843_CHG_REG_INT_MASK, chg_int_state);
		}
		max77843_read_reg(charger->i2c, MAX77843_CHG_REG_INT_MASK,
			&chg_int_state);
		max77843_read_reg(charger->i2c, MAX77843_CHG_REG_CNFG_00,
			&chg_cnfg_00);
		pr_debug("%s: INT_MASK(0x%x), CHG_CNFG_00(0x%x)\n",
			__func__, chg_int_state, chg_cnfg_00);
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int max77843_debugfs_show(struct seq_file *s, void *data)
{
	struct max77843_charger_data *charger = s->private;
	u8 reg;
	u8 reg_data;

	seq_printf(s, "MAX77843 CHARGER IC :\n");
	seq_printf(s, "===================\n");
	for (reg = 0xB0; reg <= 0xC3; reg++) {
		max77843_read_reg(charger->i2c, reg, &reg_data);
		seq_printf(s, "0x%02x:\t0x%02x\n", reg, reg_data);
	}

	seq_printf(s, "\n");
	return 0;
}

static int max77843_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, max77843_debugfs_show, inode->i_private);
}

static const struct file_operations max77843_debugfs_fops = {
	.open           = max77843_debugfs_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static void max77843_chg_isr_work(struct work_struct *work)
{
	struct max77843_charger_data *charger =
		container_of(work, struct max77843_charger_data, isr_work.work);

	union power_supply_propval val;

	if (charger->pdata->full_check_type ==
	    SEC_BATTERY_FULLCHARGED_CHGINT) {

		val.intval = max77843_get_charger_state(charger);

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
		val.intval = max77843_get_charging_health(charger);
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

static irqreturn_t max77843_chg_irq_thread(int irq, void *irq_data)
{
	struct max77843_charger_data *charger = irq_data;

	pr_info("%s: Charger interrupt occured\n", __func__);

	if ((charger->pdata->full_check_type ==
	     SEC_BATTERY_FULLCHARGED_CHGINT) ||
	    (charger->pdata->ovp_uvlo_check_type ==
	     SEC_BATTERY_OVP_UVLO_CHGINT))
		schedule_delayed_work(&charger->isr_work, 0);

	return IRQ_HANDLED;
}

static void wpc_detect_work(struct work_struct *work)
{
	struct max77843_charger_data *charger = container_of(work,
						struct max77843_charger_data,
						wpc_work.work);
	int wc_w_state;
	int retry_cnt;
	union power_supply_propval value;
	u8 reg_data;

	pr_info("%s\n", __func__);

	max77843_update_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, 0, MAX77843_WCIN_IM);

	/* check and unlock */
	check_charger_unlock_state(charger);

	retry_cnt = 0;
	do {
		max77843_read_reg(charger->i2c,
				  MAX77843_CHG_REG_INT_OK, &reg_data);
		wc_w_state = (reg_data & MAX77843_WCIN_OK)
			>> MAX77843_WCIN_OK_SHIFT;
		msleep(50);
	} while((retry_cnt++ < 2) && (wc_w_state == 0));

	if ((charger->wc_w_state == 0) && (wc_w_state == 1)) {
		value.intval = 1;
		psy_do_property("wireless", set,
				POWER_SUPPLY_PROP_ONLINE, value);
		value.intval = POWER_SUPPLY_TYPE_WIRELESS;
		pr_info("%s: wpc activated, set V_INT as PN\n",
				__func__);
	} else if ((charger->wc_w_state == 1) && (wc_w_state == 0)) {
		if (!charger->is_charging)
			max77843_set_charger_state(charger, true);

		retry_cnt = 0;
		do {
			max77843_read_reg(charger->i2c,
					  MAX77843_CHG_REG_DETAILS_01, &reg_data);
			reg_data = ((reg_data & MAX77843_CHG_DTLS)
				    >> MAX77843_CHG_DTLS_SHIFT);
			msleep(50);
		} while((retry_cnt++ < 2) && (reg_data == 0x8));
		pr_info("%s: reg_data: 0x%x, charging: %d\n", __func__,
			reg_data, charger->is_charging);
		if (!charger->is_charging)
			max77843_set_charger_state(charger, false);
		if ((reg_data != 0x08)
		    && (charger->cable_type == POWER_SUPPLY_TYPE_WIRELESS)) {
			pr_info("%s: wpc uvlo, but charging\n", __func__);
			queue_delayed_work(charger->wqueue, &charger->wpc_work,
					   msecs_to_jiffies(500));
			return;
		} else {
			value.intval = 0;
			psy_do_property("wireless", set,
					POWER_SUPPLY_PROP_ONLINE, value);
			pr_info("%s: wpc deactivated, set V_INT as PD\n",
					__func__);
		}
	}
	pr_info("%s: w(%d to %d)\n", __func__,
		charger->wc_w_state, wc_w_state);

	charger->wc_w_state = wc_w_state;

	/* Do unmask again. (for frequent wcin irq problem) */
	max77843_update_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, 0, MAX77843_WCIN_IM);

	wake_unlock(&charger->wpc_wake_lock);
}

static irqreturn_t wpc_charger_irq(int irq, void *data)
{
	struct max77843_charger_data *charger = data;
	unsigned long delay;
	u8 reg_data;

	max77843_read_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, &reg_data);
	reg_data |= (1 << 5);
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, reg_data);

	wake_lock(&charger->wpc_wake_lock);
#ifdef CONFIG_SAMSUNG_BATTERY_FACTORY
	delay = msecs_to_jiffies(0);
#else
	if (charger->wc_w_state)
		delay = msecs_to_jiffies(500);
	else
		delay = msecs_to_jiffies(0);
#endif
	queue_delayed_work(charger->wqueue, &charger->wpc_work,
			delay);
	return IRQ_HANDLED;
}

static irqreturn_t max77843_batp_irq(int irq, void *data)
{
	struct max77843_charger_data *charger = data;
	union power_supply_propval value;
	u8 reg_data;

	pr_info("%s : irq(%d)\n", __func__, irq);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_MASK, &reg_data);
	reg_data |= (1 << 2);
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, reg_data);

	check_charger_unlock_state(charger);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_OK,
			  &reg_data);

	if (!(reg_data & MAX77843_BATP_OK))
		psy_do_property("battery", set, POWER_SUPPLY_PROP_PRESENT, value);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_MASK, &reg_data);
	reg_data &= ~(1 << 2);
	max77843_write_reg(charger->i2c,
			   MAX77843_CHG_REG_INT_MASK, reg_data);

	return IRQ_HANDLED;
}

static irqreturn_t max77843_bypass_irq(int irq, void *data)
{
	struct max77843_charger_data *charger = data;
#ifdef CONFIG_USB_HOST_NOTIFY
	struct otg_notify *n = get_otg_notify();
#endif
	u8 dtls_02;
	u8 byp_dtls;
	u8 chg_cnfg_00;
	u8 vbus_state;

	pr_info("%s: irq(%d)\n", __func__, irq);

	/* check and unlock */
	check_charger_unlock_state(charger);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_DETAILS_02,
			  &dtls_02);

	byp_dtls = ((dtls_02 & MAX77843_BYP_DTLS) >>
				MAX77843_BYP_DTLS_SHIFT);
	pr_info("%s: BYP_DTLS(0x%02x)\n", __func__, byp_dtls);
	vbus_state = max77843_get_vbus_state(charger);

	if (byp_dtls & 0x1) {
		pr_info("%s: bypass overcurrent limit\n", __func__);
#ifdef CONFIG_USB_HOST_NOTIFY
		send_otg_notify(n, NOTIFY_EVENT_OVERCURRENT, 0);
#endif

		/* disable the register values just related to OTG and
		   keep the values about the charging */
		max77843_read_reg(charger->i2c,
				  MAX77843_CHG_REG_CNFG_00, &chg_cnfg_00);
		chg_cnfg_00 &= ~(CHG_CNFG_00_OTG_MASK
				| CHG_CNFG_00_BOOST_MASK);
		max77843_write_reg(charger->i2c,
				   MAX77843_CHG_REG_CNFG_00,
				   chg_cnfg_00);
	}
	return IRQ_HANDLED;
}

static void max77843_aicl_work(struct work_struct *work)
{
	struct max77843_charger_data *charger = container_of(work,
				     struct max77843_charger_data, aicl_work.work);

	charger->afc_detect = false;

	if ((charger->is_charging) &&
		(charger->cable_type != POWER_SUPPLY_TYPE_WIRELESS)) {
		int now_count = 0,
			max_count = charger->charging_current_max / REDUCE_CURRENT_STEP;
		int prev_current_max = charger->charging_current_max;

		mutex_lock(&charger->charger_mutex);

		check_charger_unlock_state(charger);
		while (max77843_check_aicl_state(charger) &&
			(now_count++ < max_count) && (charger->is_charging) &&
			(charger->cable_type != POWER_SUPPLY_TYPE_WIRELESS)) {

			reduce_input_current(charger, REDUCE_CURRENT_STEP);
			msleep(50);
		}
		pr_info("%s: charging_current_max(%d --> %d)\n",
			__func__, prev_current_max, charger->charging_current_max);

		if (prev_current_max > charger->charging_current_max) {
			max77843_check_slow_charging(charger, charger->charging_current_max);
		}

		mutex_unlock(&charger->charger_mutex);
	}

	wake_unlock(&charger->aicl_wake_lock);
}

static irqreturn_t max77843_aicl_irq(int irq, void *data)
{
	struct max77843_charger_data *charger = data;

	pr_info("%s: irq(%d)\n", __func__, irq);

	wake_lock(&charger->aicl_wake_lock);
	queue_delayed_work(charger->wqueue, &charger->aicl_work,
		msecs_to_jiffies(50));

	return IRQ_HANDLED;
}

static void max77843_chgin_isr_work(struct work_struct *work)
{
	struct max77843_charger_data *charger = container_of(work,
				     struct max77843_charger_data, chgin_work);
	u8 chgin_dtls, chg_dtls, chg_cnfg_00, reg_data;
	u8 prev_chgin_dtls = 0xff;
	int battery_health;
	union power_supply_propval value;
	int stable_count = 0;

	wake_lock(&charger->chgin_wake_lock);

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_MASK, &reg_data);
	reg_data |= (1 << 6);
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, reg_data);

	while (1) {
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_HEALTH, value);
		battery_health = value.intval;

		max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_DETAILS_00,
				&chgin_dtls);
		chgin_dtls = ((chgin_dtls & MAX77843_CHGIN_DTLS) >>
				MAX77843_CHGIN_DTLS_SHIFT);
		max77843_read_reg(charger->i2c,
				MAX77843_CHG_REG_DETAILS_01, &chg_dtls);
		chg_dtls = ((chg_dtls & MAX77843_CHG_DTLS) >>
				MAX77843_CHG_DTLS_SHIFT);
		max77843_read_reg(charger->i2c,
			MAX77843_CHG_REG_CNFG_00, &chg_cnfg_00);

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
				} else if (((chgin_dtls == 0x0) || (chgin_dtls == 0x01)) && (chg_dtls & 0x08) && \
						(chg_cnfg_00 & MAX77843_MODE_BUCK) && \
						(chg_cnfg_00 & MAX77843_MODE_CHGR) && \
						(battery_health != POWER_SUPPLY_HEALTH_UNDERVOLTAGE) && \
						(charger->cable_type != POWER_SUPPLY_TYPE_WIRELESS)) {
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
					max77843_set_input_current(charger,
							charger->charging_current_max);
				}
			}
			break;
		}
		prev_chgin_dtls = chgin_dtls;
		msleep(100);
	}
	max77843_read_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, &reg_data);
	reg_data &= ~(1 << 6);
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_INT_MASK, reg_data);
	wake_unlock(&charger->chgin_wake_lock);
}

static irqreturn_t max77843_chgin_irq(int irq, void *data)
{
	struct max77843_charger_data *charger = data;
	queue_work(charger->wqueue, &charger->chgin_work);

	return IRQ_HANDLED;
}

/* register chgin isr after sec_battery_probe */
static void max77843_chgin_init_work(struct work_struct *work)
{
	struct max77843_charger_data *charger = container_of(work,
						struct max77843_charger_data,
						chgin_init_work.work);
	int ret;

	pr_info("%s \n", __func__);
	ret = request_threaded_irq(charger->irq_chgin, NULL,
			max77843_chgin_irq, 0, "chgin-irq", charger);
	if (ret < 0) {
		pr_err("%s: fail to request chgin IRQ: %d: %d\n",
				__func__, charger->irq_chgin, ret);
	} else {
		max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, 0, MAX77843_CHGIN_IM);
	}
}

#ifdef CONFIG_OF
static int max77843_charger_parse_dt(struct max77843_charger_data *charger)
{
	struct device_node *np = of_find_node_by_name(NULL, "charger");
	sec_charger_platform_data_t *pdata = charger->pdata;
	int ret = 0;

	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
		return -1;
	} else {
		int i, len;
		const u32 *p;

		ret = of_property_read_u32(np, "battery,chg_float_voltage",
					&pdata->chg_float_voltage);
		ret = of_property_read_u32(np, "battery,ovp_uvlo_check_type",
					&pdata->ovp_uvlo_check_type);
		ret = of_property_read_u32(np, "battery,full_check_type",
					&pdata->full_check_type);
		ret = of_property_read_u32(np, "battery,full_check_type_2nd",
					&pdata->full_check_type_2nd);

		p = of_get_property(np, "battery,input_current_limit", &len);
		len = len / sizeof(u32);
		pdata->charging_current = kzalloc(sizeof(sec_charging_current_t) * len,
						  GFP_KERNEL);

		for(i = 0; i < len; i++) {
			ret = of_property_read_u32_index(np,
				 "battery,input_current_limit", i,
				 &pdata->charging_current[i].input_current_limit);
			ret = of_property_read_u32_index(np,
				 "battery,fast_charging_current", i,
				 &pdata->charging_current[i].fast_charging_current);
			ret = of_property_read_u32_index(np,
				 "battery,full_check_current_1st", i,
				 &pdata->charging_current[i].full_check_current_1st);
			ret = of_property_read_u32_index(np,
				 "battery,full_check_current_2nd", i,
				 &pdata->charging_current[i].full_check_current_2nd);
		}
	}

	return ret;
}
#endif

static int max77843_charger_probe(struct platform_device *pdev)
{
	struct max77843_dev *max77843 = dev_get_drvdata(pdev->dev.parent);
	struct max77843_platform_data *pdata = dev_get_platdata(max77843->dev);
	struct max77843_charger_data *charger;
	int ret = 0;
	u8 reg_data;

	pr_info("%s: MAX77843 Charger Driver Loading\n", __func__);

	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

	pdata->charger_data = kzalloc(sizeof(sec_charger_platform_data_t), GFP_KERNEL);
	if (!pdata->charger_data) {
		ret = -ENOMEM;
		goto err_free;
	}

	mutex_init(&charger->charger_mutex);
	charger->dev = &pdev->dev;
	charger->i2c = max77843->charger;
	charger->pdata = pdata->charger_data;
	charger->aicl_on = false;
	charger->afc_detect = false;
#if defined(CONFIG_MUIC_SUPPORT_MULTIMEDIA_DOCK)
	charger->is_mdock = false;
#endif
	charger->siop_level = 100;
	charger->max77843_pdata = pdata;
	charger->input_curr_limit_step = 33;

#if defined(CONFIG_OF)
	ret = max77843_charger_parse_dt(charger);
	if (ret < 0) {
		pr_err("%s not found charger dt! ret[%d]\n",
		       __func__, ret);
	}
#endif

	platform_set_drvdata(pdev, charger);

	charger->psy_chg.name		= "max77843-charger";
	charger->psy_chg.type		= POWER_SUPPLY_TYPE_UNKNOWN;
	charger->psy_chg.get_property	= max77843_chg_get_property;
	charger->psy_chg.set_property	= max77843_chg_set_property;
	charger->psy_chg.properties	= max77843_charger_props;
	charger->psy_chg.num_properties	= ARRAY_SIZE(max77843_charger_props);
	charger->psy_otg.name		= "otg";
	charger->psy_otg.type		= POWER_SUPPLY_TYPE_OTG;
	charger->psy_otg.get_property	= max77843_otg_get_property;
	charger->psy_otg.set_property	= max77843_otg_set_property;
	charger->psy_otg.properties		= max77843_otg_props;
	charger->psy_otg.num_properties	= ARRAY_SIZE(max77843_otg_props);

	max77843_charger_initialize(charger);

	if (max77843_read_reg(max77843->i2c, MAX77843_PMIC_REG_PMICREV, &reg_data) < 0) {
		pr_err("device not found on this channel (this is not an error)\n");
		ret = -ENODEV;
		goto err_pdata_free;
	} else {
		charger->pmic_ver = (reg_data & 0x7);
		pr_info("%s: device found: ver.0x%x\n", __func__, charger->pmic_ver);
	}

	(void) debugfs_create_file("max77843-regs",
		S_IRUGO, NULL, (void *)charger, &max77843_debugfs_fops);

	charger->wqueue =
	    create_singlethread_workqueue(dev_name(&pdev->dev));
	if (!charger->wqueue) {
		pr_err("%s: Fail to Create Workqueue\n", __func__);
		ret = -ENOMEM;
		goto err_pdata_free;
	}
	wake_lock_init(&charger->chgin_wake_lock, WAKE_LOCK_SUSPEND,
			            "charger-chgin");
	INIT_WORK(&charger->chgin_work, max77843_chgin_isr_work);
	INIT_DELAYED_WORK(&charger->chgin_init_work, max77843_chgin_init_work);
	wake_lock_init(&charger->wpc_wake_lock, WAKE_LOCK_SUSPEND,
					       "charger-wpc");
	wake_lock_init(&charger->afc_wake_lock, WAKE_LOCK_SUSPEND,
					       "charger-afc");
	INIT_DELAYED_WORK(&charger->wpc_work, wpc_detect_work);
	INIT_DELAYED_WORK(&charger->afc_work, afc_detect_work);

	wake_lock_init(&charger->aicl_wake_lock, WAKE_LOCK_SUSPEND,
					       "charger-aicl");
	INIT_DELAYED_WORK(&charger->aicl_work, max77843_aicl_work);

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
		INIT_DELAYED_WORK(&charger->isr_work, max77843_chg_isr_work);

		ret = request_threaded_irq(charger->pdata->chg_irq,
				NULL, max77843_chg_irq_thread,
				charger->pdata->chg_irq_attr,
				"charger-irq", charger);
		if (ret) {
			pr_err("%s: Failed to Request IRQ\n", __func__);
			goto err_irq;
		}

			ret = enable_irq_wake(charger->pdata->chg_irq);
			if (ret < 0)
				pr_err("%s: Failed to Enable Wakeup Source(%d)\n",
					__func__, ret);
		}

	charger->wc_w_irq = pdata->irq_base + MAX77843_CHG_IRQ_WCIN_I;
	ret = request_threaded_irq(charger->wc_w_irq,
				   NULL, wpc_charger_irq,
				   IRQF_TRIGGER_FALLING,
				   "wpc-int", charger);
	if (ret) {
		pr_err("%s: Failed to Request IRQ\n", __func__);
		goto err_wc_irq;
	}

	max77843_read_reg(charger->i2c,
			  MAX77843_CHG_REG_INT_OK, &reg_data);
	charger->wc_w_state = (reg_data & MAX77843_WCIN_OK)
		>> MAX77843_WCIN_OK_SHIFT;

	charger->irq_chgin = pdata->irq_base + MAX77843_CHG_IRQ_CHGIN_I;
	/* enable chgin irq after sec_battery_probe */
	queue_delayed_work(charger->wqueue, &charger->chgin_init_work,
			msecs_to_jiffies(3000));

	charger->irq_bypass = pdata->irq_base + MAX77843_CHG_IRQ_BYP_I;
	ret = request_threaded_irq(charger->irq_bypass, NULL,
			max77843_bypass_irq, 0, "bypass-irq", charger);
	if (ret < 0) {
		pr_err("%s: fail to request bypass IRQ: %d: %d\n",
				__func__, charger->irq_bypass, ret);
	} else {
		max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, 0, MAX77843_BYP_IM);
	}

	charger->irq_batp = pdata->irq_base + MAX77843_CHG_IRQ_BATP_I;
	ret = request_threaded_irq(charger->irq_batp, NULL,
				   max77843_batp_irq, 0,
				   "batp-irq", charger);
	if (ret < 0) {
		pr_err("%s: fail to request batp IRQ: %d: %d\n",
		       __func__, charger->irq_batp, ret);
	} else {
		max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, 0, MAX77843_BATP_IM);
	}

	charger->irq_aicl = pdata->irq_base + MAX77843_CHG_IRQ_AICL_I;
	ret = request_threaded_irq(charger->irq_aicl, NULL,
			max77843_aicl_irq, 0, "aicl-irq", charger);
	if (ret < 0) {
		pr_err("%s: fail to request aicl IRQ: %d: %d\n",
				__func__, charger->irq_aicl, ret);
	} else {
		max77843_update_reg(charger->i2c,
			MAX77843_CHG_REG_INT_MASK, 0, MAX77843_AICL_IM);
	}

	ret = max77843_chg_create_attrs(charger->psy_chg.dev);
	if (ret) {
		dev_err(charger->dev,
			"%s : Failed to create_attrs\n", __func__);
		goto err_wc_irq;
	}
	pr_info("%s: MAX77843 Charger Driver Loaded\n", __func__);

	return 0;

err_wc_irq:
	if (charger->pdata->chg_irq)
		free_irq(charger->pdata->chg_irq, NULL);
err_irq:
	power_supply_unregister(&charger->psy_chg);
err_psy_unreg_otg:
	power_supply_unregister(&charger->psy_otg);
err_workqueue:
	destroy_workqueue(charger->wqueue);
err_pdata_free:
	kfree(pdata->charger_data);
	mutex_destroy(&charger->charger_mutex);
err_free:
	kfree(charger);

	return ret;
}

static int max77843_charger_remove(struct platform_device *pdev)
{
	struct max77843_charger_data *charger =
		platform_get_drvdata(pdev);

	destroy_workqueue(charger->wqueue);
	free_irq(charger->wc_w_irq, NULL);
	free_irq(charger->pdata->chg_irq, NULL);
	power_supply_unregister(&charger->psy_chg);
	power_supply_unregister(&charger->psy_otg);
	mutex_destroy(&charger->charger_mutex);
	kfree(charger);

	return 0;
}

#if defined CONFIG_PM
static int max77843_charger_suspend(struct device *dev)
{
	return 0;
}

static int max77843_charger_resume(struct device *dev)
{
	return 0;
}
#else
#define max77843_charger_suspend NULL
#define max77843_charger_resume NULL
#endif

static void max77843_charger_shutdown(struct device *dev)
{
	struct max77843_charger_data *charger =
				dev_get_drvdata(dev);
	u8 reg_data;

	pr_info("%s: MAX77843 Charger driver shutdown\n", __func__);
	if (!charger->i2c) {
		pr_err("%s: no max77843 i2c client\n", __func__);
		return;
	}
	reg_data = 0x05;
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_00, reg_data);
	reg_data = 0x0F;
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_09, reg_data);
	reg_data = 0x19;
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_10, reg_data);
	reg_data = 0x60;	/* Disable OCP */
	max77843_write_reg(charger->i2c,
		MAX77843_CHG_REG_CNFG_12, reg_data);
	pr_info("func:%s \n", __func__);
}

static SIMPLE_DEV_PM_OPS(max77843_charger_pm_ops, max77843_charger_suspend,
			 max77843_charger_resume);

static struct platform_driver max77843_charger_driver = {
	.driver = {
		.name = "max77843-charger",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &max77843_charger_pm_ops,
#endif
		.shutdown = max77843_charger_shutdown,
	},
	.probe = max77843_charger_probe,
	.remove = max77843_charger_remove,
};

static int __init max77843_charger_init(void)
{
	pr_info("%s : \n", __func__);
	return platform_driver_register(&max77843_charger_driver);
}

static void __exit max77843_charger_exit(void)
{
	platform_driver_unregister(&max77843_charger_driver);
}

module_init(max77843_charger_init);
module_exit(max77843_charger_exit);

MODULE_DESCRIPTION("Samsung MAX77843 Charger Driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
