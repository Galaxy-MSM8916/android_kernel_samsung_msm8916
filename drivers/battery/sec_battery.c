/*
 *  sec_battery.c
 *  Samsung Mobile Battery Driver
 *
 *  Copyright (C) 2012 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/battery/sec_battery.h>
#if defined(CONFIG_SENSORS_QPNP_ADC_VOLTAGE)
#include <linux/qpnp/qpnp-adc.h>
#endif
#ifdef CONFIG_SAMSUNG_BATTERY_DISALLOW_DEEP_SLEEP
#include <linux/clk.h>
#endif

#if defined(CONFIG_TMM_CHG_CTRL)
#define TUNER_SWITCHED_ON_SIGNAL -1
#define TUNER_SWITCHED_OFF_SIGNAL -2
#define TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE 800
#define TUNER_IS_ON 1
#define TUNER_IS_OFF 0
#endif

#ifdef CONFIG_SAMSUNG_BATTERY_DISALLOW_DEEP_SLEEP
struct clk * xo_chr = NULL;
#endif

#if defined(CONFIG_MACH_KOR_EARJACK_WR)
struct sec_battery_info *g_battery;
#endif

static struct device_attribute sec_battery_attrs[] = {
	SEC_BATTERY_ATTR(batt_reset_soc),
	SEC_BATTERY_ATTR(batt_read_raw_soc),
	SEC_BATTERY_ATTR(batt_read_adj_soc),
	SEC_BATTERY_ATTR(batt_type),
	SEC_BATTERY_ATTR(batt_vfocv),
	SEC_BATTERY_ATTR(batt_vol_adc),
	SEC_BATTERY_ATTR(batt_vol_adc_cal),
	SEC_BATTERY_ATTR(batt_vol_aver),
	SEC_BATTERY_ATTR(batt_vol_adc_aver),
	SEC_BATTERY_ATTR(batt_current_ua_now),
	SEC_BATTERY_ATTR(batt_current_ua_avg),
	SEC_BATTERY_ATTR(batt_temp),
	SEC_BATTERY_ATTR(batt_temp_adc),
	SEC_BATTERY_ATTR(batt_temp_aver),
	SEC_BATTERY_ATTR(batt_temp_adc_aver),
	SEC_BATTERY_ATTR(chg_temp),
	SEC_BATTERY_ATTR(chg_temp_adc),
	SEC_BATTERY_ATTR(batt_vf_adc),
	SEC_BATTERY_ATTR(batt_slate_mode),

	SEC_BATTERY_ATTR(batt_lp_charging),
	SEC_BATTERY_ATTR(siop_activated),
	SEC_BATTERY_ATTR(siop_level),
	SEC_BATTERY_ATTR(batt_charging_source),
	SEC_BATTERY_ATTR(fg_reg_dump),
	SEC_BATTERY_ATTR(fg_reset_cap),
	SEC_BATTERY_ATTR(fg_capacity),
	SEC_BATTERY_ATTR(fg_asoc),
	SEC_BATTERY_ATTR(auth),
	SEC_BATTERY_ATTR(chg_current_adc),
	SEC_BATTERY_ATTR(wc_adc),
	SEC_BATTERY_ATTR(wc_status),
	SEC_BATTERY_ATTR(wc_enable),
	SEC_BATTERY_ATTR(hv_charger_status),
	SEC_BATTERY_ATTR(hv_charger_set),
	SEC_BATTERY_ATTR(factory_mode),
	SEC_BATTERY_ATTR(store_mode),
	SEC_BATTERY_ATTR(update),
	SEC_BATTERY_ATTR(test_mode),

	SEC_BATTERY_ATTR(call),
	SEC_BATTERY_ATTR(2g_call),
	SEC_BATTERY_ATTR(talk_gsm),
	SEC_BATTERY_ATTR(3g_call),
	SEC_BATTERY_ATTR(talk_wcdma),
	SEC_BATTERY_ATTR(music),
	SEC_BATTERY_ATTR(video),
	SEC_BATTERY_ATTR(browser),
	SEC_BATTERY_ATTR(hotspot),
	SEC_BATTERY_ATTR(camera),
	SEC_BATTERY_ATTR(camcorder),
	SEC_BATTERY_ATTR(data_call),
	SEC_BATTERY_ATTR(wifi),
	SEC_BATTERY_ATTR(wibro),
	SEC_BATTERY_ATTR(lte),
	SEC_BATTERY_ATTR(lcd),
	SEC_BATTERY_ATTR(gps),
	SEC_BATTERY_ATTR(event),
	SEC_BATTERY_ATTR(batt_temp_table),
	SEC_BATTERY_ATTR(batt_high_current_usb),
#if defined(CONFIG_SAMSUNG_BATTERY_ENG_TEST)
	SEC_BATTERY_ATTR(test_charge_current),
#endif
	SEC_BATTERY_ATTR(set_stability_test),
	SEC_BATTERY_ATTR(batt_inbat_voltage),
	SEC_BATTERY_ATTR(batt_capacity_max),
	SEC_BATTERY_ATTR(batt_discharging_check),
	SEC_BATTERY_ATTR(batt_discharging_check_adc),
	SEC_BATTERY_ATTR(batt_discharging_ntc),
	SEC_BATTERY_ATTR(batt_discharging_ntc_adc),
	SEC_BATTERY_ATTR(batt_self_discharging_control),
#if defined(CONFIG_SW_SELF_DISCHARGING)
	SEC_BATTERY_ATTR(batt_sw_self_discharging),
#endif
	SEC_BATTERY_ATTR(hmt_ta_connected),
	SEC_BATTERY_ATTR(hmt_ta_charge),
};
#if defined(CONFIG_QPNP_BMS)
static char *pm_batt_supplied_to[] = {
	"bms",
};
#endif


static enum power_supply_property sec_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_AMBIENT,
#if defined(CONFIG_BATTERY_SWELLING)
	POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT,
#endif
};

static enum power_supply_property sec_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property sec_ps_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_ONLINE,
};

static char *supply_list[] = {
	"battery",
};

char *sec_bat_charging_mode_str[] = {
	"None",
	"Normal",
	"Additional",
	"Re-Charging",
	"ABS"
};

char *sec_bat_status_str[] = {
	"Unknown",
	"Charging",
	"Discharging",
	"Not-charging",
	"Full"
};

char *sec_bat_health_str[] = {
	"Unknown",
	"Good",
	"Overheat",
	"Warm",
	"Dead",
	"OverVoltage",
	"UnspecFailure",
	"Cold",
	"Cool",
	"WatchdogTimerExpire",
	"SafetyTimerExpire",
	"UnderVoltage",
	"OverheatLimit"
};

#if defined(CONFIG_TMM_CHG_CTRL)
static int tuner_running_status;
#endif

static int fg_reset = 1;
static int sec_bat_get_fg_reset(char *val)
{
	fg_reset = strncmp(val, "1", 1) ? 0 : 1;
	pr_info("%s, fg_reset:%d\n", __func__, fg_reset);
	return 1;
}
__setup("fg_reset=", sec_bat_get_fg_reset);

int poweroff_charging;
static int sec_bat_is_lpm_check(char *str)
{
	if (strncmp(str, "charger", 7) == 0)
		poweroff_charging = 1;

	pr_info("%s: Low power charging mode: %d\n", __func__, poweroff_charging);

	return poweroff_charging;
}
__setup("androidboot.mode=", sec_bat_is_lpm_check);

static bool sec_bat_is_lpm(struct sec_battery_info *battery)
{
	if (battery->pdata->is_lpm) {
		return battery->pdata->is_lpm();
	} else {
		return (bool)poweroff_charging;
	}
}

static int sec_bat_set_charge(
				struct sec_battery_info *battery,
				bool enable)
{
	union power_supply_propval val;
	ktime_t current_time;
	struct timespec ts;
#if defined(CONFIG_EXTCON)
	if (battery->cable_type == POWER_SUPPLY_TYPE_OTG)
		return 0;
#endif
	val.intval = battery->status;
	psy_do_property(battery->pdata->charger_name, set,
		POWER_SUPPLY_PROP_STATUS, val);
#if defined(ANDROID_ALARM_ACTIVATED)
	current_time = alarm_get_elapsed_realtime();
	ts = ktime_to_timespec(current_time);
#else
	current_time = ktime_get_boottime();
	ts = ktime_to_timespec(current_time);
#endif

	if (enable) {
		val.intval = battery->cable_type;
		/*Reset charging start time only in initial charging start */
		if (battery->charging_start_time == 0) {
			if (ts.tv_sec < 1)
				ts.tv_sec = 1;
			battery->charging_start_time = ts.tv_sec;
			battery->charging_next_time =
				battery->pdata->charging_reset_time;
		}
	} else {
		val.intval = POWER_SUPPLY_TYPE_BATTERY;
		battery->charging_start_time = 0;
		battery->charging_passed_time = 0;
		battery->charging_next_time = 0;
		battery->charging_fullcharged_time = 0;
		battery->full_check_cnt = 0;
	}

	battery->charging_block = !enable;

	battery->temp_highlimit_cnt = 0;
	battery->temp_high_cnt = 0;
	battery->temp_low_cnt = 0;
	battery->temp_recover_cnt = 0;

#if defined(CONFIG_TMM_CHG_CTRL)
	if((tuner_running_status==TUNER_IS_ON) &&
		(battery->pdata->charging_current[val.intval].input_current_limit
		> TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE)) {
			union power_supply_propval value;

			dev_dbg(battery->dev,
				"%s: tmm chg current set!\n", __func__);
			value.intval = TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE;
			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CURRENT_NOW, value);
	} else {
		psy_do_property(battery->pdata->charger_name, set,
		POWER_SUPPLY_PROP_ONLINE, val);
	}
#else
	psy_do_property(battery->pdata->charger_name, set,
		POWER_SUPPLY_PROP_ONLINE, val);
#endif

	psy_do_property(battery->pdata->fuelgauge_name, set,
		POWER_SUPPLY_PROP_ONLINE, val);

	return 0;
}

static int sec_bat_get_adc_data(struct sec_battery_info *battery,
			int adc_ch, int count)
{
	int adc_data;
	int adc_max;
	int adc_min;
	int adc_total;
	int i;

	adc_data = 0;
	adc_max = 0;
	adc_min = 0;
	adc_total = 0;

	for (i = 0; i < count; i++) {
		mutex_lock(&battery->adclock);
#ifdef CONFIG_OF
		adc_data = adc_read(battery, adc_ch);
#else
		adc_data = adc_read(battery->pdata, adc_ch);
#endif
		mutex_unlock(&battery->adclock);

		if (adc_data < 0)
			goto err;

		if (i != 0) {
			if (adc_data > adc_max)
				adc_max = adc_data;
			else if (adc_data < adc_min)
				adc_min = adc_data;
		} else {
			adc_max = adc_data;
			adc_min = adc_data;
		}
		adc_total += adc_data;
	}

	return (adc_total - adc_max - adc_min) / (count - 2);
err:
	return adc_data;
}

/*
static unsigned long calculate_average_adc(
			struct sec_battery_info *battery,
			int channel, int adc)
{
	unsigned int cnt = 0;
	int total_adc = 0;
	int average_adc = 0;
	int index = 0;

	cnt = battery->adc_sample[channel].cnt;
	total_adc = battery->adc_sample[channel].total_adc;

	if (adc < 0) {
		dev_err(battery->dev,
			"%s : Invalid ADC : %d\n", __func__, adc);
		adc = battery->adc_sample[channel].average_adc;
	}

	if (cnt < ADC_SAMPLE_COUNT) {
		battery->adc_sample[channel].adc_arr[cnt] = adc;
		battery->adc_sample[channel].index = cnt;
		battery->adc_sample[channel].cnt = ++cnt;

		total_adc += adc;
		average_adc = total_adc / cnt;
	} else {
		index = battery->adc_sample[channel].index;
		if (++index >= ADC_SAMPLE_COUNT)
			index = 0;

		total_adc = total_adc -
			battery->adc_sample[channel].adc_arr[index] + adc;
		average_adc = total_adc / ADC_SAMPLE_COUNT;

		battery->adc_sample[channel].adc_arr[index] = adc;
		battery->adc_sample[channel].index = index;
	}

	battery->adc_sample[channel].total_adc = total_adc;
	battery->adc_sample[channel].average_adc = average_adc;

	return average_adc;
}
*/
static int sec_bat_get_adc_value(
		struct sec_battery_info *battery, int channel)
{
	int adc;

	adc = sec_bat_get_adc_data(battery, channel,
		battery->pdata->adc_check_count);

	if (adc < 0) {
		dev_err(battery->dev,
			"%s: Error in ADC\n", __func__);
		return adc;
	}

	return adc;
}

static int sec_bat_get_charger_type_adc
				(struct sec_battery_info *battery)
{
	/* It is true something valid is
	connected to the device for charging.
	By default this something is considered to be USB.*/
	int result = POWER_SUPPLY_TYPE_USB;

	int adc = 0;
	int i;

	/* Do NOT check cable type when cable_switch_check() returns false
	 * and keep current cable type
	 */
	if (battery->pdata->cable_switch_check &&
	    !battery->pdata->cable_switch_check())
		return battery->cable_type;

	adc = sec_bat_get_adc_value(battery,
		SEC_BAT_ADC_CHANNEL_CABLE_CHECK);

	/* Do NOT check cable type when cable_switch_normal() returns false
	 * and keep current cable type
	 */
	if (battery->pdata->cable_switch_normal &&
	    !battery->pdata->cable_switch_normal())
		return battery->cable_type;

	for (i = 0; i < SEC_SIZEOF_POWER_SUPPLY_TYPE; i++)
		if ((adc > battery->pdata->cable_adc_value[i].min) &&
			(adc < battery->pdata->cable_adc_value[i].max))
			break;
	if (i >= SEC_SIZEOF_POWER_SUPPLY_TYPE)
		dev_err(battery->dev,
			"%s : default USB\n", __func__);
	else
		result = i;

	dev_dbg(battery->dev, "%s : result(%d), adc(%d)\n",
		__func__, result, adc);

	return result;
}

static bool sec_bat_check_vf_adc(struct sec_battery_info *battery)
{
	int adc;

	adc = sec_bat_get_adc_data(battery,
		SEC_BAT_ADC_CHANNEL_BAT_CHECK,
		battery->pdata->adc_check_count);

	if (adc < 0) {
		dev_err(battery->dev, "%s: VF ADC error\n", __func__);
		adc = battery->check_adc_value;
	} else
		battery->check_adc_value = adc;

	if ((battery->check_adc_value <= battery->pdata->check_adc_max) &&
		(battery->check_adc_value >= battery->pdata->check_adc_min)) {
		return true;
	} else {
		dev_info(battery->dev, "%s: adc (%d)\n", __func__, battery->check_adc_value);
		return false;
	}
}

static bool sec_bat_check_by_psy(struct sec_battery_info *battery)
{
	char *psy_name;
	union power_supply_propval value;
	bool ret;
	ret = true;

	switch (battery->pdata->battery_check_type) {
	case SEC_BATTERY_CHECK_PMIC:
		psy_name = battery->pdata->pmic_name;
		break;
	case SEC_BATTERY_CHECK_FUELGAUGE:
		psy_name = battery->pdata->fuelgauge_name;
		break;
	case SEC_BATTERY_CHECK_CHARGER:
		psy_name = battery->pdata->charger_name;
		break;
	default:
		dev_err(battery->dev,
			"%s: Invalid Battery Check Type\n", __func__);
		ret = false;
		goto battery_check_error;
		break;
	}

	psy_do_property(psy_name, get,
		POWER_SUPPLY_PROP_PRESENT, value);
	ret = (bool)value.intval;

battery_check_error:
	return ret;
}

static bool sec_bat_check(struct sec_battery_info *battery)
{
	bool ret;
	ret = true;

#if defined(CONFIG_MUIC_NOTIFIER)
	if (battery->factory_mode || battery->is_jig_on) {
#else
	if (battery->factory_mode || sec_bat_check_jig_status()) {
#endif
		dev_dbg(battery->dev, "%s: No need to check in factory mode\n",
				__func__);
		return ret;
	}

	if (battery->health != POWER_SUPPLY_HEALTH_GOOD &&
		battery->health != POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
		dev_dbg(battery->dev, "%s: No need to check\n", __func__);
		return ret;
	}

	switch (battery->pdata->battery_check_type) {
	case SEC_BATTERY_CHECK_ADC:
		if(battery->cable_type == POWER_SUPPLY_TYPE_BATTERY)
			ret = battery->present;
		else
			ret = sec_bat_check_vf_adc(battery);
		break;
	case SEC_BATTERY_CHECK_INT:
	case SEC_BATTERY_CHECK_CALLBACK:
		if(battery->cable_type == POWER_SUPPLY_TYPE_BATTERY)
			ret = battery->present;
		else
			ret = sec_bat_check_callback(battery);
		break;
	case SEC_BATTERY_CHECK_PMIC:
	case SEC_BATTERY_CHECK_FUELGAUGE:
	case SEC_BATTERY_CHECK_CHARGER:
		ret = sec_bat_check_by_psy(battery);
		break;
	case SEC_BATTERY_CHECK_NONE:
		dev_dbg(battery->dev, "%s: No Check\n", __func__);
	default:
		break;
	}

	return ret;
}

static bool sec_bat_get_cable_type(
			struct sec_battery_info *battery,
			int cable_source_type)
{
	bool ret;
	int cable_type;

	ret = false;
	cable_type = battery->cable_type;

	if (cable_source_type & SEC_BATTERY_CABLE_SOURCE_CALLBACK) {
		cable_type = sec_bat_check_cable_callback(battery);
	}

	if (cable_source_type & SEC_BATTERY_CABLE_SOURCE_ADC) {
		if (gpio_get_value_cansleep(
			battery->pdata->bat_gpio_ta_nconnected) ^
			battery->pdata->bat_polarity_ta_nconnected)
			cable_type = POWER_SUPPLY_TYPE_BATTERY;
		else
			cable_type =
				sec_bat_get_charger_type_adc(battery);
	}

	if (battery->cable_type == cable_type) {
		dev_dbg(battery->dev,
			"%s: No need to change cable status\n", __func__);
	} else {
		if (cable_type < POWER_SUPPLY_TYPE_BATTERY ||
			cable_type >= SEC_SIZEOF_POWER_SUPPLY_TYPE) {
			dev_err(battery->dev,
				"%s: Invalid cable type\n", __func__);
		} else {
			battery->cable_type = cable_type;
			sec_bat_check_cable_result_callback(battery->dev, battery->cable_type);

			ret = true;

			dev_dbg(battery->dev, "%s: Cable Changed (%d)\n",
				__func__, battery->cable_type);
		}
	}

	return ret;
}

static void sec_bat_set_charging_status(struct sec_battery_info *battery,
		int status) {
	union power_supply_propval value;
	switch (status) {
		case POWER_SUPPLY_STATUS_NOT_CHARGING:
		case POWER_SUPPLY_STATUS_DISCHARGING:
			if((battery->status == POWER_SUPPLY_STATUS_FULL) ||
				(battery->capacity == 100)) {
#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
			value.intval = battery->capacity;
			psy_do_property(battery->pdata->fuelgauge_name, set,
					POWER_SUPPLY_PROP_CHARGE_FULL, value);
#else
			value.intval = POWER_SUPPLY_TYPE_BATTERY;
			psy_do_property(battery->pdata->fuelgauge_name, set,
					POWER_SUPPLY_PROP_CHARGE_FULL, value);
#endif
				/* To get SOC value (NOT raw SOC), need to reset value */
				value.intval = 0;
				psy_do_property(battery->pdata->fuelgauge_name, get,
						POWER_SUPPLY_PROP_CAPACITY, value);
				battery->capacity = value.intval;
			}
			break;
		default:
			break;
	}
	battery->status = status;
}

static bool sec_bat_battery_cable_check(struct sec_battery_info *battery)
{
	if (!sec_bat_check(battery)) {
		if (battery->check_count < battery->pdata->check_count)
			battery->check_count++;
		else {
			dev_err(battery->dev,
				"%s: Battery Disconnected\n", __func__);
			battery->present = false;
			battery->health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;

			if (battery->status !=
				POWER_SUPPLY_STATUS_DISCHARGING) {
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_NOT_CHARGING);
				sec_bat_set_charge(battery, false);
			}

			if (battery->pdata->check_battery_result_callback)
				battery->pdata->
					check_battery_result_callback();
			return false;
		}
	} else
		battery->check_count = 0;

	battery->present = true;

	if (battery->health == POWER_SUPPLY_HEALTH_UNSPEC_FAILURE) {
		battery->health = POWER_SUPPLY_HEALTH_GOOD;

		if (battery->status == POWER_SUPPLY_STATUS_NOT_CHARGING) {
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_CHARGING);
#if defined(CONFIG_BATTERY_SWELLING)
			if (!battery->swelling_mode)
				sec_bat_set_charge(battery, true);
#else
			sec_bat_set_charge(battery, true);
#endif
		}
	}

	dev_info(battery->dev, "%s: Battery Connected\n", __func__);

	if (battery->pdata->cable_check_type &
		SEC_BATTERY_CABLE_CHECK_POLLING) {
		if (sec_bat_get_cable_type(battery,
			battery->pdata->cable_source_type)) {
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
					   &battery->cable_work, 0);
		}
	}
	return true;
}

static int sec_bat_ovp_uvlo_by_psy(struct sec_battery_info *battery)
{
	char *psy_name;
	union power_supply_propval value;

	value.intval = POWER_SUPPLY_HEALTH_GOOD;

	switch (battery->pdata->ovp_uvlo_check_type) {
	case SEC_BATTERY_OVP_UVLO_PMICPOLLING:
		psy_name = battery->pdata->pmic_name;
		break;
	case SEC_BATTERY_OVP_UVLO_CHGPOLLING:
		psy_name = battery->pdata->charger_name;
		break;
	default:
		dev_err(battery->dev,
			"%s: Invalid OVP/UVLO Check Type\n", __func__);
		goto ovp_uvlo_check_error;
		break;
	}

	psy_do_property(psy_name, get,
		POWER_SUPPLY_PROP_HEALTH, value);

ovp_uvlo_check_error:
	return value.intval;
}

static bool sec_bat_ovp_uvlo_result(
		struct sec_battery_info *battery, int health)
{
	if (battery->health != health) {
		battery->health = health;
		switch (health) {
		case POWER_SUPPLY_HEALTH_GOOD:
			dev_info(battery->dev, "%s: Safe voltage\n", __func__);
			dev_info(battery->dev, "%s: is_recharging : %d\n", __func__, battery->is_recharging);
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_CHARGING);
			battery->charging_mode = SEC_BATTERY_CHARGING_1ST;
#if defined(CONFIG_BATTERY_SWELLING)
		if (!battery->swelling_mode)
			sec_bat_set_charge(battery, true);
#else
			sec_bat_set_charge(battery, true);
#endif
			battery->health_check_count = 0;
			break;
		case POWER_SUPPLY_HEALTH_OVERVOLTAGE:
		case POWER_SUPPLY_HEALTH_UNDERVOLTAGE:
			dev_info(battery->dev,
				"%s: Unsafe voltage (%d)\n",
				__func__, health);
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_NOT_CHARGING);
			sec_bat_set_charge(battery, false);
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			battery->is_recharging = false;
			battery->health_check_count = DEFAULT_HEALTH_CHECK_COUNT;
			/* Take the wakelock during 10 seconds
			   when over-voltage status is detected	 */
			wake_lock_timeout(&battery->vbus_wake_lock, HZ * 10);
			break;
		}
		power_supply_changed(&battery->psy_bat);
		return true;
	}

	return false;
}

static bool sec_bat_ovp_uvlo(struct sec_battery_info *battery)
{
	int health;

#if defined(CONFIG_MUIC_NOTIFIER)
	if (battery->factory_mode || battery->is_jig_on) {
#else
	if (battery->factory_mode || sec_bat_check_jig_status()) {
#endif
		dev_dbg(battery->dev, "%s: No need to check in factory mode\n", __func__);
		return false;
	} else if ((battery->status == POWER_SUPPLY_STATUS_FULL) &&
		   (battery->charging_mode == SEC_BATTERY_CHARGING_NONE)) {
		dev_dbg(battery->dev, "%s: No need to check in Full status", __func__);
		return false;
	}

	if (battery->health != POWER_SUPPLY_HEALTH_GOOD &&
		battery->health != POWER_SUPPLY_HEALTH_OVERVOLTAGE &&
		battery->health != POWER_SUPPLY_HEALTH_UNDERVOLTAGE) {
		dev_dbg(battery->dev, "%s: No need to check\n", __func__);
		return false;
	}

	health = battery->health;

	switch (battery->pdata->ovp_uvlo_check_type) {
	case SEC_BATTERY_OVP_UVLO_CALLBACK:
		if (battery->pdata->ovp_uvlo_callback)
			health = battery->pdata->ovp_uvlo_callback();
		break;
	case SEC_BATTERY_OVP_UVLO_PMICPOLLING:
	case SEC_BATTERY_OVP_UVLO_CHGPOLLING:
		health = sec_bat_ovp_uvlo_by_psy(battery);
		break;
	case SEC_BATTERY_OVP_UVLO_PMICINT:
	case SEC_BATTERY_OVP_UVLO_CHGINT:
		/* nothing for interrupt check */
	default:
		break;
	}

	return sec_bat_ovp_uvlo_result(battery, health);
}

static bool sec_bat_check_recharge(struct sec_battery_info *battery)
{
#if defined(CONFIG_BATTERY_SWELLING)
	if (battery->swelling_mode) {
		pr_info("%s: Skip normal recharge check routine for swelling mode\n",
			__func__);
		return false;
	}
#endif
	if ((battery->status == POWER_SUPPLY_STATUS_CHARGING) &&
			(battery->pdata->full_condition_type &
			 SEC_BATTERY_FULL_CONDITION_NOTIMEFULL) &&
			(battery->charging_mode == SEC_BATTERY_CHARGING_NONE)) {
		dev_info(battery->dev,
				"%s: Re-charging by NOTIMEFULL (%d)\n",
				__func__, battery->capacity);
		goto check_recharge_check_count;
	}

	if (battery->status == POWER_SUPPLY_STATUS_FULL &&
			battery->charging_mode == SEC_BATTERY_CHARGING_NONE) {
		if ((battery->pdata->recharge_condition_type &
					SEC_BATTERY_RECHARGE_CONDITION_SOC) &&
				(battery->capacity <=
				 battery->pdata->recharge_condition_soc)) {
			dev_info(battery->dev,
					"%s: Re-charging by SOC (%d)\n",
					__func__, battery->capacity);
			goto check_recharge_check_count;
		}

		if ((battery->pdata->recharge_condition_type &
					SEC_BATTERY_RECHARGE_CONDITION_AVGVCELL) &&
				(battery->voltage_avg <=
				 battery->pdata->recharge_condition_avgvcell)) {
			dev_info(battery->dev,
					"%s: Re-charging by average VCELL (%d)\n",
					__func__, battery->voltage_avg);
			goto check_recharge_check_count;
		}

		if ((battery->pdata->recharge_condition_type &
					SEC_BATTERY_RECHARGE_CONDITION_VCELL) &&
				(battery->voltage_now <=
				 battery->pdata->recharge_condition_vcell)) {
			dev_info(battery->dev,
					"%s: Re-charging by VCELL (%d)\n",
					__func__, battery->voltage_now);
			goto check_recharge_check_count;
		}
	}

	battery->recharge_check_cnt = 0;
	return false;

check_recharge_check_count:
	if (battery->recharge_check_cnt <
		battery->pdata->recharge_check_count)
		battery->recharge_check_cnt++;
	dev_dbg(battery->dev,
		"%s: recharge count = %d\n",
		__func__, battery->recharge_check_cnt);

	if (battery->recharge_check_cnt >=
		battery->pdata->recharge_check_count)
		return true;
	else
		return false;
}

static bool sec_bat_voltage_check(struct sec_battery_info *battery)
{
	union power_supply_propval value;

	if (battery->status == POWER_SUPPLY_STATUS_DISCHARGING) {
		dev_dbg(battery->dev,
			"%s: Charging Disabled\n", __func__);
		return true;
	}

	/* OVP/UVLO check */
	if (sec_bat_ovp_uvlo(battery)) {
		if (battery->pdata->ovp_uvlo_result_callback)
			battery->pdata->
				ovp_uvlo_result_callback(battery->health);
		return false;
	}

	if ((battery->status == POWER_SUPPLY_STATUS_FULL) &&
		(battery->charging_mode == SEC_BATTERY_CHARGING_2ND ||
		 battery->is_recharging)) {
		value.intval = 0;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_CAPACITY, value);
		if (value.intval <
			battery->pdata->full_condition_soc &&
				battery->voltage_now <
				(battery->pdata->recharge_condition_vcell - 50)) {
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_CHARGING);
			battery->voltage_now = 1080;
			battery->voltage_avg = 1080;
			power_supply_changed(&battery->psy_bat);
			dev_info(battery->dev,
				"%s: battery status full -> charging, RepSOC(%d)\n", __func__, value.intval);
			return false;
		}
	}

	/* Re-Charging check */
	if (sec_bat_check_recharge(battery)) {
		if (battery->pdata->full_check_type !=
			SEC_BATTERY_FULLCHARGED_NONE)
			battery->charging_mode = SEC_BATTERY_CHARGING_1ST;
		else
			battery->charging_mode = SEC_BATTERY_CHARGING_2ND;
		battery->is_recharging = true;
#if defined(CONFIG_BATTERY_SWELLING)
		if (!battery->swelling_mode)
			sec_bat_set_charge(battery, true);
#else
		sec_bat_set_charge(battery, true);
#endif
		return false;
	}

	return true;
}

static bool sec_bat_get_temperature_by_adc(
				struct sec_battery_info *battery,
				enum sec_battery_adc_channel channel,
				union power_supply_propval *value)
{
	int temp = 0;
	int temp_adc;
	int low = 0;
	int high = 0;
	int mid = 0;
	const sec_bat_adc_table_data_t *temp_adc_table;
	unsigned int temp_adc_table_size;

	temp_adc = sec_bat_get_adc_value(battery, channel);
	if (temp_adc < 0)
		return true;

	switch (channel) {
	case SEC_BAT_ADC_CHANNEL_TEMP:
		temp_adc_table = battery->pdata->temp_adc_table;
		temp_adc_table_size =
			battery->pdata->temp_adc_table_size;
		battery->temp_adc = temp_adc;
		break;
	case SEC_BAT_ADC_CHANNEL_TEMP_AMBIENT:
		temp_adc_table = battery->pdata->temp_amb_adc_table;
		temp_adc_table_size =
			battery->pdata->temp_amb_adc_table_size;
		battery->temp_ambient_adc = temp_adc;
		break;
	case SEC_BAT_ADC_CHANNEL_CHG_TEMP:
		temp_adc_table = battery->pdata->chg_temp_adc_table;
		temp_adc_table_size =
			battery->pdata->chg_temp_adc_table_size;
		battery->chg_temp_adc = temp_adc;
		break;
	default:
		dev_err(battery->dev,
			"%s: Invalid Property\n", __func__);
		return false;
	}

	if (temp_adc_table[0].adc >= temp_adc) {
		temp = temp_adc_table[0].data;
		goto temp_by_adc_goto;
	} else if (temp_adc_table[temp_adc_table_size-1].adc <= temp_adc) {
		temp = temp_adc_table[temp_adc_table_size-1].data;
		goto temp_by_adc_goto;
	}

	high = temp_adc_table_size - 1;

	while (low <= high) {
		mid = (low + high) / 2;
		if (temp_adc_table[mid].adc > temp_adc)
			high = mid - 1;
		else if (temp_adc_table[mid].adc < temp_adc)
			low = mid + 1;
		else {
			temp = temp_adc_table[mid].data;
			goto temp_by_adc_goto;
		}
	}

	temp = temp_adc_table[high].data;
	temp += ((temp_adc_table[low].data - temp_adc_table[high].data) *
		 (temp_adc - temp_adc_table[high].adc)) /
		(temp_adc_table[low].adc - temp_adc_table[high].adc);

temp_by_adc_goto:
	value->intval = temp;

	dev_info(battery->dev,
		"%s: Temp(%d), Temp-ADC(%d)\n",
		__func__, temp, temp_adc);

	return true;
}

static bool sec_bat_temperature(
				struct sec_battery_info *battery)
{
	bool ret;
	ret = true;

	if (battery->pdata->event_check && battery->event) {
		battery->temp_highlimit_threshold =
			battery->pdata->temp_highlimit_threshold_event;
		battery->temp_highlimit_recovery =
			battery->pdata->temp_highlimit_recovery_event;
		battery->temp_high_threshold =
			battery->pdata->temp_high_threshold_event;
		battery->temp_high_recovery =
			battery->pdata->temp_high_recovery_event;
		battery->temp_low_recovery =
			battery->pdata->temp_low_recovery_event;
		battery->temp_low_threshold =
			battery->pdata->temp_low_threshold_event;
	} else if (sec_bat_is_lpm(battery)) {
		battery->temp_highlimit_threshold =
			battery->pdata->temp_highlimit_threshold_lpm;
		battery->temp_highlimit_recovery =
			battery->pdata->temp_highlimit_recovery_lpm;
		battery->temp_high_threshold =
			battery->pdata->temp_high_threshold_lpm;
		battery->temp_high_recovery =
			battery->pdata->temp_high_recovery_lpm;
		battery->temp_low_recovery =
			battery->pdata->temp_low_recovery_lpm;
		battery->temp_low_threshold =
			battery->pdata->temp_low_threshold_lpm;
	} else {
		battery->temp_highlimit_threshold =
			battery->pdata->temp_highlimit_threshold_normal;
		battery->temp_highlimit_recovery =
			battery->pdata->temp_highlimit_recovery_normal;
		battery->temp_high_threshold =
			battery->pdata->temp_high_threshold_normal;
		battery->temp_high_recovery =
			battery->pdata->temp_high_recovery_normal;
		battery->temp_low_recovery =
			battery->pdata->temp_low_recovery_normal;
		battery->temp_low_threshold =
			battery->pdata->temp_low_threshold_normal;
	}

	dev_info(battery->dev,
		"%s: HLT(%d) HLR(%d) HT(%d), HR(%d), LT(%d), LR(%d)\n",
		__func__, battery->temp_highlimit_threshold,
		battery->temp_highlimit_recovery,
		battery->temp_high_threshold,
		battery->temp_high_recovery,
		battery->temp_low_threshold,
		battery->temp_low_recovery);
	return ret;
}

#if defined(CONFIG_BATTERY_SWELLING)
static void sec_bat_swelling_check(struct sec_battery_info *battery, int temperature)
{
	union power_supply_propval val;
	int swelling_rechg_voltage;

	psy_do_property(battery->pdata->charger_name, get,
			POWER_SUPPLY_PROP_VOLTAGE_MAX, val);

	pr_info("%s: status(%d), swell_mode(%d:%d), cv(0x%02x), temp(%d)\n",
		__func__, battery->status, battery->swelling_mode,
		battery->charging_block, val.intval, temperature);

	/* swelling_mode
		under voltage over voltage, battery missing  */
	if ((battery->status == POWER_SUPPLY_STATUS_DISCHARGING) ||\
		(battery->status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
		pr_info("%s: DISCHARGING or NOT-CHARGING. stop swelling mode\n", __func__);
		battery->swelling_mode = false;
		goto skip_swelling_chek;
	}

	if (!battery->swelling_mode) {
		if (((temperature >= battery->swelling_temp_high_threshold) ||
			(temperature <= battery->swelling_temp_low_threshold)) &&
			battery->pdata->temp_check_type) {

			if (temperature >= battery->swelling_temp_high_threshold &&
				battery->pdata->event_check &&
				!battery->event) {
				pr_info("%s: skip check swelling in high temperature event mode(%d)\n",
						__func__, battery->event);
				return;
			}

			pr_info("%s: swelling mode start. stop charging\n", __func__);
			battery->swelling_mode = true;
			battery->swelling_full_check_cnt = 0;
			sec_bat_set_charge(battery, false);
		}
	}

	if (!battery->voltage_now)
		return;
	if (battery->swelling_mode) {
		if (temperature <= battery->swelling_temp_low_recovery)
			swelling_rechg_voltage = battery->pdata->swelling_low_rechg_voltage;
		else
			swelling_rechg_voltage = battery->pdata->swelling_high_rechg_voltage;

		if ((temperature <= battery->swelling_temp_high_recovery) &&
		    (temperature >= battery->swelling_temp_low_recovery)) {
			pr_info("%s: swelling mode end. restart charging\n", __func__);
			battery->swelling_mode = false;
			battery->charging_mode = SEC_BATTERY_CHARGING_1ST;
			sec_bat_set_charge(battery, true);
			/* restore 4.4V float voltage */
			val.intval = battery->pdata->swelling_normal_float_voltage;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_VOLTAGE_MAX, val);
		} else if (battery->voltage_now < swelling_rechg_voltage &&
			   battery->charging_block) {
			pr_info("%s: swelling mode recharging start. Vbatt(%d)\n",
				__func__, battery->voltage_now);
			battery->charging_mode = SEC_BATTERY_CHARGING_1ST;
			sec_bat_set_charge(battery, true);
			/* change 4.20V float voltage */
			val.intval = battery->pdata->swelling_drop_float_voltage;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_VOLTAGE_MAX, val);
			if ((temperature <= battery->swelling_temp_low_threshold) &&
				(battery->pdata->swelling_chg_current > 0)) {
				pr_info("%s: swelling mode reduce charging current(temp:%d)\n",
					__func__, temperature);
				val.intval = battery->pdata->swelling_chg_current;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CURRENT_AVG, val);
			}

			if ((temperature <= battery->swelling_temp_low_threshold) &&
				(battery->pdata->swelling_low_chg_current > 0)) {
				pr_info("%s: swelling mode reduce charging current(temp:%d)\n",
					__func__, temperature);
				val.intval = battery->pdata->swelling_low_chg_current;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CURRENT_AVG, val);
			} else if ((temperature >= battery->swelling_temp_high_threshold) &&
				(battery->pdata->swelling_high_chg_current > 0)) {
				pr_info("%s: swelling mode reduce charging current(temp:%d)\n",
					__func__, temperature);
				val.intval = battery->pdata->swelling_high_chg_current;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CURRENT_AVG, val);
			}
		}
	}
skip_swelling_chek:
	dev_dbg(battery->dev, "%s end\n", __func__);
}
#endif

static bool sec_bat_temperature_check(
				struct sec_battery_info *battery)
{
	int temp_value;
	int pre_health;

	if (battery->status == POWER_SUPPLY_STATUS_DISCHARGING) {
		dev_dbg(battery->dev,
			"%s: Charging Disabled\n", __func__);
		return true;
	}

	if (battery->health != POWER_SUPPLY_HEALTH_GOOD &&
		battery->health != POWER_SUPPLY_HEALTH_OVERHEAT &&
		battery->health != POWER_SUPPLY_HEALTH_COLD &&
		battery->health != POWER_SUPPLY_HEALTH_OVERHEATLIMIT) {
		dev_dbg(battery->dev, "%s: No need to check\n", __func__);
		return false;
	}

	sec_bat_temperature(battery);

	switch (battery->pdata->temp_check_type) {
	case SEC_BATTERY_TEMP_CHECK_ADC:
		temp_value = battery->temp_adc;
		break;
	case SEC_BATTERY_TEMP_CHECK_TEMP:
		temp_value = battery->temperature;
		break;
	default:
		dev_err(battery->dev,
			"%s: Invalid Temp Check Type\n", __func__);
		return true;
	}
	pre_health = battery->health;

	if (temp_value >= battery->temp_highlimit_threshold) {
		if (battery->health != POWER_SUPPLY_HEALTH_OVERHEATLIMIT) {
			if (battery->temp_highlimit_cnt <
			    battery->pdata->temp_check_count) {
				battery->temp_highlimit_cnt++;
				battery->temp_high_cnt = 0;
				battery->temp_low_cnt = 0;
				battery->temp_recover_cnt = 0;
			}
			dev_dbg(battery->dev,
				"%s: highlimit count = %d\n",
				__func__, battery->temp_highlimit_cnt);
		}
	} else if (temp_value >= battery->temp_high_threshold) {
		if (battery->health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT) {
			if (temp_value <= battery->temp_highlimit_recovery) {
				if (battery->temp_recover_cnt <
				    battery->pdata->temp_check_count) {
					battery->temp_recover_cnt++;
					battery->temp_highlimit_cnt = 0;
					battery->temp_high_cnt = 0;
					battery->temp_low_cnt = 0;
				}
				dev_dbg(battery->dev,
					"%s: recovery count = %d\n",
					__func__, battery->temp_recover_cnt);
			}
		} else if (battery->health != POWER_SUPPLY_HEALTH_OVERHEAT) {
			if (battery->temp_high_cnt <
			    battery->pdata->temp_check_count) {
				battery->temp_high_cnt++;
				battery->temp_highlimit_cnt = 0;
				battery->temp_low_cnt = 0;
				battery->temp_recover_cnt = 0;
			}
			dev_dbg(battery->dev,
				"%s: high count = %d\n",
				__func__, battery->temp_high_cnt);
		}
	} else if ((temp_value <= battery->temp_high_recovery) &&
				(temp_value >= battery->temp_low_recovery)) {
		if (battery->health == POWER_SUPPLY_HEALTH_OVERHEAT ||
			battery->health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT ||
		    battery->health == POWER_SUPPLY_HEALTH_COLD) {
			if (battery->temp_recover_cnt <
			    battery->pdata->temp_check_count) {
				battery->temp_recover_cnt++;
				battery->temp_highlimit_cnt = 0;
				battery->temp_high_cnt = 0;
				battery->temp_low_cnt = 0;
			}
			dev_dbg(battery->dev,
				"%s: recovery count = %d\n",
				__func__, battery->temp_recover_cnt);
		}
	} else if (temp_value <= battery->temp_low_threshold) {
		if (battery->health != POWER_SUPPLY_HEALTH_COLD) {
			if (battery->temp_low_cnt <
			    battery->pdata->temp_check_count) {
				battery->temp_low_cnt++;
				battery->temp_highlimit_cnt = 0;
				battery->temp_high_cnt = 0;
				battery->temp_recover_cnt = 0;
			}
			dev_dbg(battery->dev,
				"%s: low count = %d\n",
				__func__, battery->temp_low_cnt);
		}
	} else {
		battery->temp_highlimit_cnt = 0;
		battery->temp_high_cnt = 0;
		battery->temp_low_cnt = 0;
		battery->temp_recover_cnt = 0;
	}

	if (battery->temp_highlimit_cnt >=
	    battery->pdata->temp_check_count) {
		battery->health = POWER_SUPPLY_HEALTH_OVERHEATLIMIT;
		battery->temp_highlimit_cnt = 0;
	} else if (battery->temp_high_cnt >=
		battery->pdata->temp_check_count) {
		battery->health = POWER_SUPPLY_HEALTH_OVERHEAT;
		battery->temp_high_cnt = 0;
	} else if (battery->temp_low_cnt >=
		battery->pdata->temp_check_count) {
		battery->health = POWER_SUPPLY_HEALTH_COLD;
		battery->temp_low_cnt = 0;
	} else if (battery->temp_recover_cnt >=
		 battery->pdata->temp_check_count) {
		if (battery->health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT) {
			battery->health = POWER_SUPPLY_HEALTH_OVERHEAT;
		} else {
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING) || defined(CONFIG_SW_SELF_DISCHARGING)
			union power_supply_propval value;

			psy_do_property(battery->pdata->charger_name, get,
					POWER_SUPPLY_PROP_VOLTAGE_MAX, value);
			if (value.intval <= battery->pdata->swelling_normal_float_voltage) {
				value.intval = battery->pdata->swelling_normal_float_voltage;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_VOLTAGE_MAX, value);
			}
#endif
			battery->health = POWER_SUPPLY_HEALTH_GOOD;
		}
		battery->temp_recover_cnt = 0;
	}
	if(pre_health != battery->health){
		battery->health_change = true;
		dev_info(battery->dev,"%s: health_change true\n", __func__);
	}
	else
		battery->health_change = false;

	if ((battery->health == POWER_SUPPLY_HEALTH_OVERHEAT) ||
		(battery->health == POWER_SUPPLY_HEALTH_COLD) ||
		(battery->health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) {
		if (battery->status != POWER_SUPPLY_STATUS_NOT_CHARGING) {
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING) || defined(CONFIG_SW_SELF_DISCHARGING)
			if ((battery->health == POWER_SUPPLY_HEALTH_OVERHEAT) ||
				(battery->health == POWER_SUPPLY_HEALTH_OVERHEATLIMIT)) {
				union power_supply_propval val;
				/* change 4.20V float voltage */
				val.intval = battery->pdata->swelling_drop_float_voltage;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_VOLTAGE_MAX, val);
			}
#endif
			dev_info(battery->dev,
				"%s: Unsafe Temperature\n", __func__);
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_NOT_CHARGING);
			/* change charging current to battery (default 0mA) */
			sec_bat_set_charge(battery, false);
			return false;
		}
	} else {
		/* if recovered from not charging */
		if ((battery->health == POWER_SUPPLY_HEALTH_GOOD) &&
			(battery->status ==
			 POWER_SUPPLY_STATUS_NOT_CHARGING)) {
			dev_info(battery->dev,
					"%s: Safe Temperature\n", __func__);
			if (battery->capacity >= 100)
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_FULL);
			else	/* Normal Charging */
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_CHARGING);
#if defined(CONFIG_BATTERY_SWELLING)
			if ((temp_value >= battery->swelling_temp_high_threshold) ||
				(temp_value <= battery->swelling_temp_low_threshold)) {
				pr_info("%s: swelling mode start. stop charging\n", __func__);
				battery->swelling_mode = true;
				battery->swelling_full_check_cnt = 0;
				sec_bat_set_charge(battery, false);
			} else {
				/* turn on charger by cable type */
				sec_bat_set_charge(battery, true);
			}
#else
			/* turn on charger by cable type */
			sec_bat_set_charge(battery, true);
#endif
			return false;
		}
	}
	return true;
}

#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
static void sec_bat_self_discharging_check(struct sec_battery_info *battery)
{
	unsigned int dis_adc;
	union power_supply_propval value;

	switch(battery->pdata->self_discharging_type){
	case SEC_BAT_SELF_DISCHARGING_BY_ADC:
		dis_adc = sec_bat_get_adc_value(battery, SEC_BAT_ADC_CHANNEL_DISCHARGING_CHECK);
		if (dis_adc)
			battery->self_discharging_adc = dis_adc;
		else
			battery->self_discharging_adc = 0;

		if ((dis_adc >= battery->pdata->discharging_adc_min) &&
		    (dis_adc <= battery->pdata->discharging_adc_max))
			battery->self_discharging = true;
		else
			battery->self_discharging = false;

		pr_info("%s : SELF_DISCHARGING(%d) SELF_DISCHARGING_ADC(%d)\n",
			__func__, battery->self_discharging, battery->self_discharging_adc);
		break;
	case SEC_BAT_SELF_DISCHARGING_BY_FG:
		break;
	case SEC_BAT_SELF_DISCHARGING_BY_CHG:
		psy_do_property(battery->pdata->charger_name, get,
			POWER_SUPPLY_PROP_RESISTANCE, value);
		
		battery->self_discharging = value.intval;
		pr_info("%s: SELF_DISCHARGING : %s\n", __func__,
			value.intval ? "Enabled" : "Disabled");
		break;
	default:
		break;
	}
}

static void sec_bat_self_discharging_ntc_check(struct sec_battery_info *battery)
{
	int ntc_adc;

	ntc_adc = sec_bat_get_adc_value(battery, SEC_BAT_ADC_CHANNEL_DISCHARGING_NTC);
	if (ntc_adc)
		battery->discharging_ntc_adc = ntc_adc;
	else
		battery->discharging_ntc_adc = 0;

	if (ntc_adc > battery->pdata->discharging_ntc_limit)
		battery->discharging_ntc = true;
	else
		battery->discharging_ntc = false;

	pr_info("%s : DISCHARGING_NTC(%d) DISCHARGING_NTC_ADC(%d)\n",
		__func__,battery->discharging_ntc, battery->discharging_ntc_adc);
}

static void sec_bat_self_discharging_control(struct sec_battery_info *battery, bool dis_en)
{
	union power_supply_propval value;
	
	switch(battery->pdata->self_discharging_type){
	case SEC_BAT_SELF_DISCHARGING_BY_ADC:
		if (!battery->pdata->factory_discharging) {
			pr_info("Can't control Self Discharging IC (No Factory Discharging Pin).\n");
			return;
		}

		if (dis_en) {
			dev_info(battery->dev,
				 "%s : Self Discharging IC doesn't act until (%d) degree & (%d) voltage. "
				 "Auto Discharging IC ENABLE\n", __func__,
				 battery->temperature, battery->voltage_now);
			gpio_direction_output(battery->pdata->factory_discharging, 1);
			battery->force_discharging = true;
		} else {
			dev_info(battery->dev, "%s : Self Discharging IC disable.\n", __func__);
			gpio_direction_output(battery->pdata->factory_discharging, 0);
			battery->force_discharging = false;
		}
		break;
	case SEC_BAT_SELF_DISCHARGING_BY_FG:
		break;
	case SEC_BAT_SELF_DISCHARGING_BY_CHG:
		value.intval = dis_en;

		if (dis_en)
			battery->force_discharging = true;
		else
			battery->force_discharging = false;
		psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_RESISTANCE, value);
		
		pr_info("%s: SELF_DISCHARGING Set to %s\n", __func__,
			value.intval ? "Enabled" : "Disabled");
		break;
	default:
		break;
	}
}

static void sec_bat_discharging_check(struct sec_battery_info *battery)
{
	if (!battery->pdata->self_discharging_en)
		return;

	sec_bat_self_discharging_check(battery);

	if (!battery->self_discharging &&
	    (battery->temperature >= battery->pdata->force_discharging_limit &&
#if defined(CONFIG_SEC_FACTORY)
	     battery->temperature < 800 &&
#endif
	     battery->voltage_now >= battery->pdata->self_discharging_voltage_limit)) {
		sec_bat_self_discharging_control(battery, true);
	} else if(battery->force_discharging &&
		  (battery->temperature <= battery->pdata->force_discharging_recov ||
		   battery->voltage_now <= battery->pdata->swelling_drop_float_voltage)) {
		sec_bat_self_discharging_control(battery, false);
	}
	dev_info(battery->dev,
		 "%s: Auto_DIS(%d), Force_DIS(%d)\n",
		 __func__, battery->self_discharging, battery->force_discharging);
}
#endif

#if !defined(CONFIG_SEC_FACTORY)
static void sec_bat_chg_temperature_check(
	struct sec_battery_info *battery)
{
	if (battery->skip_chg_temp_check)
		return;
	if (battery->siop_level >= 100 &&
			((battery->cable_type == POWER_SUPPLY_TYPE_HV_MAINS) ||
			 (battery->cable_type == POWER_SUPPLY_TYPE_HV_ERR))) {
		union power_supply_propval value;
		if ((battery->chg_limit == SEC_BATTERY_CHG_TEMP_NONE) &&
				(battery->chg_temp > battery->pdata->chg_high_temp_1st)) {
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_HIGH_1ST;
			value.intval = battery->pdata->chg_charging_limit_current;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CURRENT_MAX, value);
			dev_info(battery->dev,"%s: Chg current is reduced by Temp: %d\n",
					__func__, battery->chg_temp);
		} else if ((battery->chg_limit == SEC_BATTERY_CHG_TEMP_HIGH_1ST) &&
				(battery->pre_chg_temp < battery->pdata->chg_high_temp_2nd) &&
				(battery->chg_temp > battery->pdata->chg_high_temp_2nd)) {
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_HIGH_2ND;
			value.intval = battery->pdata->chg_charging_limit_current_2nd;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CURRENT_MAX, value);
			dev_info(battery->dev,"%s: Chg current 2nd is reduced by Temp: %d\n",
					__func__, battery->chg_temp);
		} else if ((battery->chg_limit != SEC_BATTERY_CHG_TEMP_NONE) &&
				(battery->chg_temp < battery->pdata->chg_high_temp_recovery)) {
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_NONE;
			value.intval = battery->pdata->charging_current
				[battery->cable_type].input_current_limit;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CURRENT_MAX, value);
			dev_info(battery->dev,"%s: Chg current is recovered by Temp: %d\n",
					__func__, battery->chg_temp);
		}
	} else if (battery->siop_level >= 100 &&
			(battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS) && battery->pdata->wpc_temp_check) {
		union power_supply_propval value;
		if ((battery->chg_limit == SEC_BATTERY_CHG_TEMP_NONE) &&
				(battery->chg_temp > battery->pdata->wpc_high_temp)) {
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_HIGH_1ST;
			value.intval = battery->pdata->wpc_charging_limit_current;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CURRENT_MAX, value);
			dev_info(battery->dev,"%s: WPC Chg current is reduced by Temp: %d\n",
					__func__, battery->chg_temp);
		} else if ((battery->chg_limit != SEC_BATTERY_CHG_TEMP_NONE) &&
				(battery->chg_temp < battery->pdata->wpc_high_temp_recovery)) {
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_NONE;
			value.intval = battery->pdata->charging_current
				[battery->cable_type].input_current_limit;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CURRENT_MAX, value);
			dev_info(battery->dev,"%s: WPC Chg current is recovered by Temp: %d\n",
					__func__, battery->chg_temp);
		}
	} else if (battery->chg_limit != SEC_BATTERY_CHG_TEMP_NONE) {
		battery->chg_limit = SEC_BATTERY_CHG_TEMP_NONE;
	}
}
#endif

static void  sec_bat_event_program_alarm(
	struct sec_battery_info *battery, int seconds)
{
#if defined(ANDROID_ALARM_ACTIVATED)
	ktime_t low_interval = ktime_set(seconds - 10, 0);
	ktime_t slack = ktime_set(20, 0);
	ktime_t next;

	next = ktime_add(battery->last_event_time, low_interval);
	alarm_start_range(&battery->event_termination_alarm,
		next, ktime_add(next, slack));
#else
	alarm_start(&battery->event_termination_alarm,
		ktime_add(battery->last_event_time, ktime_set(seconds - 10, 0)));
#endif
}

#if defined(ANDROID_ALARM_ACTIVATED)
static void sec_bat_event_expired_timer_func(struct alarm *alarm)
#else
static enum alarmtimer_restart sec_bat_event_expired_timer_func(
	struct alarm *alarm, ktime_t now)
#endif
{
	struct sec_battery_info *battery =
		container_of(alarm, struct sec_battery_info,
			event_termination_alarm);

	battery->event &= (~battery->event_wait);
	dev_info(battery->dev,
		"%s: event expired (0x%x)\n", __func__, battery->event);

#if !defined(ANDROID_ALARM_ACTIVATED)
	return ALARMTIMER_NORESTART;
#endif
}

static void sec_bat_event_set(
	struct sec_battery_info *battery, int event, int enable)
{
	if (!battery->pdata->event_check)
		return;

	/* ignore duplicated deactivation of same event
	 * only if the event is one last event
	 */
	if (!enable && (battery->event == battery->event_wait)) {
		dev_info(battery->dev,
			"%s: ignore duplicated deactivation of same event\n",
			__func__);
		return;
	}

	alarm_cancel(&battery->event_termination_alarm);
	battery->event &= (~battery->event_wait);

	if (enable) {
		battery->event_wait = 0;
		battery->event |= event;

		dev_info(battery->dev,
			"%s: event set (0x%x)\n", __func__, battery->event);
	} else {
		if (battery->event == 0) {
			dev_dbg(battery->dev,
				"%s: nothing to clear\n", __func__);
			return;	/* nothing to clear */
		}
		battery->event_wait = event;
#if defined(ANDROID_ALARM_ACTIVATED)
		battery->last_event_time = alarm_get_elapsed_realtime();
#else
		battery->last_event_time = ktime_get_boottime();
#endif
		sec_bat_event_program_alarm(battery,
			battery->pdata->event_waiting_time);
		dev_info(battery->dev,
			"%s: start timer (curr 0x%x, wait 0x%x)\n",
			__func__, battery->event, battery->event_wait);
	}
}

static bool sec_bat_check_fullcharged_condition(
					struct sec_battery_info *battery)
{
	int full_check_type;

	if (battery->charging_mode == SEC_BATTERY_CHARGING_1ST)
		full_check_type = battery->pdata->full_check_type;
	else
		full_check_type = battery->pdata->full_check_type_2nd;

	switch (full_check_type) {
	case SEC_BATTERY_FULLCHARGED_ADC:
	case SEC_BATTERY_FULLCHARGED_FG_CURRENT:
	case SEC_BATTERY_FULLCHARGED_SOC:
	case SEC_BATTERY_FULLCHARGED_CHGGPIO:
	case SEC_BATTERY_FULLCHARGED_CHGPSY:
		break;

	/* If these is NOT full check type or NONE full check type,
	 * it is full-charged
	 */
	case SEC_BATTERY_FULLCHARGED_CHGINT:
	case SEC_BATTERY_FULLCHARGED_TIME:
	case SEC_BATTERY_FULLCHARGED_NONE:
	default:
		return true;
		break;
	}

	if (battery->pdata->full_condition_type &
		SEC_BATTERY_FULL_CONDITION_SOC) {
		if (battery->capacity <
			battery->pdata->full_condition_soc) {
			dev_dbg(battery->dev,
				"%s: Not enough SOC (%d%%)\n",
				__func__, battery->capacity);
			return false;
		}
	}

	if (battery->pdata->full_condition_type &
		SEC_BATTERY_FULL_CONDITION_VCELL) {
		if (battery->voltage_now <
			battery->pdata->full_condition_vcell) {
			dev_dbg(battery->dev,
				"%s: Not enough VCELL (%dmV)\n",
				__func__, battery->voltage_now);
			return false;
		}
	}

	if (battery->pdata->full_condition_type &
		SEC_BATTERY_FULL_CONDITION_AVGVCELL) {
		if (battery->voltage_avg <
			battery->pdata->full_condition_avgvcell) {
			dev_dbg(battery->dev,
				"%s: Not enough AVGVCELL (%dmV)\n",
				__func__, battery->voltage_avg);
			return false;
		}
	}

	if (battery->pdata->full_condition_type &
		SEC_BATTERY_FULL_CONDITION_OCV) {
		if (battery->voltage_ocv <
			battery->pdata->full_condition_ocv) {
			dev_dbg(battery->dev,
				"%s: Not enough OCV (%dmV)\n",
				__func__, battery->voltage_ocv);
			return false;
		}
	}

	return true;
}

static void sec_bat_do_test_function(
		struct sec_battery_info *battery)
{
	union power_supply_propval value;

	switch (battery->test_mode) {
		case 1:
			if (battery->status == POWER_SUPPLY_STATUS_CHARGING) {
				sec_bat_set_charge(battery, false);
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_DISCHARGING);
			}
			break;
		case 2:
			if(battery->status == POWER_SUPPLY_STATUS_DISCHARGING) {
				sec_bat_set_charge(battery, true);
				psy_do_property(battery->pdata->charger_name, get,
						POWER_SUPPLY_PROP_STATUS, value);
				sec_bat_set_charging_status(battery, value.intval);
			}
			battery->test_mode = 0;
			break;
		case 3: // clear temp block
			battery->health = POWER_SUPPLY_HEALTH_GOOD;
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_DISCHARGING);
			break;
		case 4:
			if(battery->status == POWER_SUPPLY_STATUS_DISCHARGING) {
				sec_bat_set_charge(battery, true);
				psy_do_property(battery->pdata->charger_name, get,
						POWER_SUPPLY_PROP_STATUS, value);
				sec_bat_set_charging_status(battery, value.intval);
			}
			break;
		default:
			pr_info("%s: error test: unknown state\n", __func__);
			break;
	}
}

static bool sec_bat_time_management(
				struct sec_battery_info *battery)
{
	unsigned long charging_time;
	struct timespec ts;
#if defined(ANDROID_ALARM_ACTIVATED)
	ktime_t current_time;

	current_time = alarm_get_elapsed_realtime();
	ts = ktime_to_timespec(current_time);
#else
	get_monotonic_boottime(&ts);
#endif

	if (battery->charging_start_time == 0) {
		dev_dbg(battery->dev,
			"%s: Charging Disabled\n", __func__);
		return true;
	}

	if (ts.tv_sec >= battery->charging_start_time)
		charging_time = ts.tv_sec - battery->charging_start_time;
	else
		charging_time = 0xFFFFFFFF - battery->charging_start_time
		    + ts.tv_sec;

	battery->charging_passed_time = charging_time;

	dev_info(battery->dev,
		"%s: Charging Time : %ld secs\n", __func__,
		battery->charging_passed_time);

	if (battery->pdata->chg_temp_check && battery->skip_chg_temp_check) {
		if ((battery->cable_type == POWER_SUPPLY_TYPE_HV_MAINS ||
			battery->cable_type == POWER_SUPPLY_TYPE_HV_ERR ||
			battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS) &&
			battery->charging_passed_time >= battery->pdata->chg_skip_check_time) {
				battery->skip_chg_temp_check = false;
				dev_info(battery->dev,
					"%s: skip_chg_temp_check(%d), Charging Time : %ld secs\n",
					__func__,
					battery->skip_chg_temp_check,
					battery->charging_passed_time);
		}
	}
	switch (battery->status) {
	case POWER_SUPPLY_STATUS_FULL:
		if (battery->is_recharging && (charging_time >
			battery->pdata->recharging_total_time)) {
			dev_info(battery->dev,
				"%s: Recharging Timer Expired\n", __func__);
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			battery->is_recharging = false;
			if (sec_bat_set_charge(battery, false)) {
				dev_err(battery->dev,
					"%s: Fail to Set Charger\n", __func__);
				return true;
			}

			return false;
		}
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		if ((battery->pdata->full_condition_type &
			SEC_BATTERY_FULL_CONDITION_NOTIMEFULL) &&
			(battery->is_recharging && (charging_time >
			battery->pdata->recharging_total_time))) {
			dev_info(battery->dev,
			"%s: Recharging Timer Expired\n", __func__);
			if (battery->capacity >= 100)
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_FULL);
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			battery->is_recharging = false;
			if (sec_bat_set_charge(battery, false)) {
				dev_err(battery->dev,
					"%s: Fail to Set Charger\n", __func__);
				return true;
			}
			return false;
		} else if (!battery->is_recharging &&
			(charging_time > battery->pdata->charging_total_time)) {
			dev_info(battery->dev,
				"%s: Charging Timer Expired\n", __func__);
			if (battery->pdata->full_condition_type &
				SEC_BATTERY_FULL_CONDITION_NOTIMEFULL) {
				if (battery->capacity >= 100)
					sec_bat_set_charging_status(battery,
							POWER_SUPPLY_STATUS_FULL);
			} else
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_FULL);
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			if (sec_bat_set_charge(battery, false)) {
				dev_err(battery->dev,
					"%s: Fail to Set Charger\n", __func__);
				return true;
			}

			return false;
		}
		if (battery->pdata->charging_reset_time) {
			if (charging_time > battery->charging_next_time) {
				/*reset current in charging status */
				battery->charging_next_time =
					battery->charging_passed_time +
					(battery->pdata->charging_reset_time);

				dev_dbg(battery->dev,
					"%s: Reset charging current\n",
					__func__);
#if defined(CONFIG_BATTERY_SWELLING)
				if (!battery->swelling_mode) {
					if (sec_bat_set_charge(battery, true)) {
						dev_err(battery->dev,
							"%s: Fail to Set Charger\n",
							__func__);
						return true;
					}
				}
#else
				if (sec_bat_set_charge(battery, true)) {
					dev_err(battery->dev,
						"%s: Fail to Set Charger\n",
						__func__);
					return true;
				}
#endif
			}
		}
		break;
	default:
		dev_err(battery->dev,
			"%s: Undefine Battery Status\n", __func__);
		return true;
	}

	return true;
}

static bool sec_bat_check_fullcharged(
				struct sec_battery_info *battery)
{
	union power_supply_propval value;
	int current_adc;
	int full_check_type;
	bool ret;
	int err;

	ret = false;

	if (!sec_bat_check_fullcharged_condition(battery))
		goto not_full_charged;

	if (battery->charging_mode == SEC_BATTERY_CHARGING_1ST)
		full_check_type = battery->pdata->full_check_type;
	else
		full_check_type = battery->pdata->full_check_type_2nd;

	switch (full_check_type) {
	case SEC_BATTERY_FULLCHARGED_ADC:
		current_adc =
			sec_bat_get_adc_value(battery,
			SEC_BAT_ADC_CHANNEL_FULL_CHECK);

		dev_dbg(battery->dev,
			"%s: Current ADC (%d)\n",
			__func__, current_adc);

		if (current_adc < 0)
			break;
		battery->current_adc = current_adc;

		if (battery->current_adc <
			(battery->charging_mode ==
			SEC_BATTERY_CHARGING_1ST ?
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_1st :
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_2nd)) {
			battery->full_check_cnt++;
			dev_dbg(battery->dev,
				"%s: Full Check ADC (%d)\n",
				__func__,
				battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		break;

	case SEC_BATTERY_FULLCHARGED_FG_CURRENT:
		if ((battery->current_now > 0 && battery->current_now <
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_1st) &&
			(battery->current_avg > 0 && battery->current_avg <
			(battery->charging_mode ==
			SEC_BATTERY_CHARGING_1ST ?
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_1st :
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_2nd))) {
				battery->full_check_cnt++;
				dev_dbg(battery->dev,
				"%s: Full Check Current (%d)\n",
				__func__,
				battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		break;

	case SEC_BATTERY_FULLCHARGED_TIME:
		if ((battery->charging_mode ==
			SEC_BATTERY_CHARGING_2ND ?
			(battery->charging_passed_time -
			battery->charging_fullcharged_time) :
			battery->charging_passed_time) >
			(battery->charging_mode ==
			SEC_BATTERY_CHARGING_1ST ?
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_1st :
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_2nd)) {
			battery->full_check_cnt++;
			dev_dbg(battery->dev,
				"%s: Full Check Time (%d)\n",
				__func__,
				battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		break;

	case SEC_BATTERY_FULLCHARGED_SOC:
		if (battery->capacity <=
			(battery->charging_mode ==
			SEC_BATTERY_CHARGING_1ST ?
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_1st :
			battery->pdata->charging_current[
			battery->cable_type].full_check_current_2nd)) {
			battery->full_check_cnt++;
			dev_dbg(battery->dev,
				"%s: Full Check SOC (%d)\n",
				__func__,
				battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		break;

	case SEC_BATTERY_FULLCHARGED_CHGGPIO:
		err = gpio_request(
			battery->pdata->chg_gpio_full_check,
			"GPIO_CHG_FULL");
		if (err) {
			dev_err(battery->dev,
				"%s: Error in Request of GPIO\n", __func__);
			break;
		}
		if (!(gpio_get_value_cansleep(
			battery->pdata->chg_gpio_full_check) ^
			!battery->pdata->chg_polarity_full_check)) {
			battery->full_check_cnt++;
			dev_dbg(battery->dev,
				"%s: Full Check GPIO (%d)\n",
				__func__, battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		gpio_free(battery->pdata->chg_gpio_full_check);
		break;

	case SEC_BATTERY_FULLCHARGED_CHGINT:
	case SEC_BATTERY_FULLCHARGED_CHGPSY:
		psy_do_property(battery->pdata->charger_name, get,
			POWER_SUPPLY_PROP_STATUS, value);

		if (value.intval == POWER_SUPPLY_STATUS_FULL) {
			battery->full_check_cnt++;
			dev_info(battery->dev,
				"%s: Full Check Charger (%d)\n",
				__func__, battery->full_check_cnt);
		} else
			battery->full_check_cnt = 0;
		break;

	/* If these is NOT full check type or NONE full check type,
	 * it is full-charged
	 */
	case SEC_BATTERY_FULLCHARGED_NONE:
		battery->full_check_cnt = 0;
		ret = true;
		break;
	default:
		dev_err(battery->dev,
			"%s: Invalid Full Check\n", __func__);
		break;
	}

	if (battery->full_check_cnt >=
		battery->pdata->full_check_count) {
		battery->full_check_cnt = 0;
		ret = true;
	}

not_full_charged:
	return ret;
}

static void sec_bat_do_fullcharged(
				struct sec_battery_info *battery)
{
	union power_supply_propval value;

	/* To let charger/fuel gauge know the full status,
	 * set status before calling sec_bat_set_charge()
	 */
	sec_bat_set_charging_status(battery,
			POWER_SUPPLY_STATUS_FULL);

#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
	if (battery->charging_mode == SEC_BATTERY_CHARGING_1ST &&
		battery->pdata->full_check_type_2nd != SEC_BATTERY_FULLCHARGED_NONE) {
		battery->charging_mode = SEC_BATTERY_CHARGING_2ND;
		battery->charging_fullcharged_time =
			battery->charging_passed_time;
		sec_bat_set_charge(battery, true);
	} else if (battery->capacity == 100) {
		battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
		battery->is_recharging = false;
		sec_bat_set_charge(battery, false);

		value.intval = POWER_SUPPLY_STATUS_FULL;
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_STATUS, value);
	} else {
		battery->full_check_cnt = battery->pdata->full_check_count; 
	}
#else	
	if (battery->charging_mode == SEC_BATTERY_CHARGING_1ST &&
		battery->pdata->full_check_type_2nd != SEC_BATTERY_FULLCHARGED_NONE) {
		battery->charging_mode = SEC_BATTERY_CHARGING_2ND;
		battery->charging_fullcharged_time =
			battery->charging_passed_time;
		sec_bat_set_charge(battery, true);
	} else {
		battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
		battery->is_recharging = false;
		sec_bat_set_charge(battery, false);

		value.intval = POWER_SUPPLY_STATUS_FULL;
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_STATUS, value);
	}
#endif

#if !defined(CONFIG_DISABLE_SAVE_CAPACITY_MAX)
#if defined(CONFIG_PREVENT_SOC_JUMP)
	value.intval = battery->capacity;
#else
	value.intval = POWER_SUPPLY_TYPE_BATTERY;
#endif
	psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_CHARGE_FULL, value);
#endif

	/* platform can NOT get information of battery
	 * because wakeup time is too short to check uevent
	 * To make sure that target is wakeup if full-charged,
	 * activated wake lock in a few seconds
	 */
	if (battery->pdata->polling_type == SEC_BATTERY_MONITOR_ALARM)
		wake_lock_timeout(&battery->vbus_wake_lock, HZ * 10);
}

static bool sec_bat_fullcharged_check(
				struct sec_battery_info *battery)
{
	if ((battery->charging_mode == SEC_BATTERY_CHARGING_NONE) ||
		(battery->status == POWER_SUPPLY_STATUS_NOT_CHARGING)) {
		dev_dbg(battery->dev,
			"%s: No Need to Check Full-Charged\n", __func__);
		return true;
	}

	if (sec_bat_check_fullcharged(battery))
		sec_bat_do_fullcharged(battery);

	dev_info(battery->dev,
		"%s: Charging Mode : %s\n", __func__,
		battery->is_recharging ?
		sec_bat_charging_mode_str[SEC_BATTERY_CHARGING_RECHARGING] :
		sec_bat_charging_mode_str[battery->charging_mode]);

	return true;
}

#if defined(CONFIG_MACH_KOR_EARJACK_WR)
extern bool sec_jack_onoff(void);
extern bool primary_sound_onoff(void);
static bool sec_bat_check_earjack_state(struct sec_battery_info *battery)
{
	if (!battery->earjack_wr_enable ||
		(battery->status != POWER_SUPPLY_STATUS_CHARGING &&
		battery->status != POWER_SUPPLY_STATUS_FULL)) {
		return false;
	}
	return true;
}

void set_earjack_state(void)
{
	if (g_battery && sec_bat_check_earjack_state(g_battery)) {
		wake_lock(&g_battery->monitor_wake_lock);
		queue_delayed_work(g_battery->monitor_wqueue, &g_battery->monitor_work, 0);
	}
}

void set_soundpath_state(void)
{
	if (g_battery && sec_jack_onoff() &&
		sec_bat_check_earjack_state(g_battery)) {
		wake_lock(&g_battery->monitor_wake_lock);
		queue_delayed_work(g_battery->monitor_wqueue, &g_battery->monitor_work, 0);
	}
}

static void sec_bat_earjack_lcd_state(struct sec_battery_info *battery, int lcd_state)
{
	int prev_lcd_state = (battery->earjack_wr_state & EARJACK_WR_LCD) ? 1 : 0;
	if (prev_lcd_state != lcd_state) {
		battery->earjack_wr_state = (lcd_state) ?
				(battery->earjack_wr_state | EARJACK_WR_LCD) :
				(battery->earjack_wr_state & ~EARJACK_WR_LCD);

		if (sec_bat_check_earjack_state(battery)) {
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work(battery->monitor_wqueue, &battery->monitor_work, 0);
		}
	}
}

static int sec_bat_get_earjack_input_current(struct sec_battery_info *battery)
{
	int earjack_wr_state = EARJACK_WR_NONE;
	int input_current_limit = battery->pdata->charging_current
						[battery->cable_type].input_current_limit;
	int ret_val = input_current_limit;

	if (sec_bat_check_earjack_state(battery)) {
		earjack_wr_state |= sec_jack_onoff() ? EARJACK_WR_EARJACK : EARJACK_WR_NONE;
		earjack_wr_state |= primary_sound_onoff() ? EARJACK_WR_SOUNDPATH : EARJACK_WR_NONE;
		earjack_wr_state |= battery->earjack_wr_state & EARJACK_WR_LCD;
		battery->earjack_wr_state = earjack_wr_state;
		
		if (battery->earjack_wr_state & EARJACK_WR_EARJACK) {
			if (battery->capacity >= battery->earjack_wr_soc_2nd) {
				if (!(battery->earjack_wr_state & EARJACK_WR_LCD) &&
					!(battery->earjack_wr_state & EARJACK_WR_SOUNDPATH)) {
					input_current_limit = battery->earjack_wr_input_current_1st;
				} else {
					input_current_limit = battery->earjack_wr_input_current_2nd;
				}
			} else if (battery->capacity >= battery->earjack_wr_soc_1st) {
				input_current_limit = battery->earjack_wr_input_current_1st;
			}
		}
		ret_val = (input_current_limit <= ret_val) ?
			input_current_limit : ret_val;
	}

	pr_info("%s: state=0x%x,capacity=%d, ret_val=%d, input_current=%d\n",
		__func__, battery->earjack_wr_state, battery->capacity,
		ret_val, input_current_limit);

	return ret_val;
}
#endif

static void sec_bat_get_temperature_info(
				struct sec_battery_info *battery)
{
	union power_supply_propval value;

	switch (battery->pdata->thermal_source) {
	case SEC_BATTERY_THERMAL_SOURCE_FG:
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_TEMP, value);
		battery->temperature = value.intval;

		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_TEMP_AMBIENT, value);
		battery->temper_amb = value.intval;
		break;
	case SEC_BATTERY_THERMAL_SOURCE_CALLBACK:
		if (battery->pdata->get_temperature_callback) {
			battery->pdata->get_temperature_callback(
				POWER_SUPPLY_PROP_TEMP, &value);
			battery->temperature = value.intval;
				psy_do_property(battery->pdata->fuelgauge_name, set,
				POWER_SUPPLY_PROP_TEMP, value);

			battery->pdata->get_temperature_callback(
				POWER_SUPPLY_PROP_TEMP_AMBIENT, &value);
			battery->temper_amb = value.intval;
				psy_do_property(battery->pdata->fuelgauge_name, set,
				POWER_SUPPLY_PROP_TEMP_AMBIENT, value);
		}
		break;
	case SEC_BATTERY_THERMAL_SOURCE_ADC:
		sec_bat_get_temperature_by_adc(battery,
			SEC_BAT_ADC_CHANNEL_TEMP, &value);
		battery->temperature = value.intval;
#if !defined(CONFIG_QPNP_BMS)
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_TEMP, value);
#endif
		sec_bat_get_temperature_by_adc(battery,
			SEC_BAT_ADC_CHANNEL_TEMP_AMBIENT, &value);
		battery->temper_amb = value.intval;
#if !defined(CONFIG_QPNP_BMS)
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_TEMP_AMBIENT, value);
#endif
		break;
	default:
		break;
	}

	if (battery->pdata->chg_temp_check) {
		sec_bat_get_temperature_by_adc(battery,
					       SEC_BAT_ADC_CHANNEL_CHG_TEMP,
					       &value);
		if (battery->pre_chg_temp == 0) {
			battery->pre_chg_temp = value.intval;
			battery->chg_temp = value.intval;
		} else {
			battery->pre_chg_temp = battery->chg_temp;
			battery->chg_temp = value.intval;
		}
	}
}

static void sec_bat_get_battery_info(
				struct sec_battery_info *battery)
{
	union power_supply_propval value;
#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
	static struct timespec old_ts;
	struct timespec c_ts;
#if defined(ANDROID_ALARM_ACTIVATED)
	c_ts = ktime_to_timespec(alarm_get_elapsed_realtime());
#else
	c_ts = ktime_to_timespec(ktime_get_boottime());
#endif
#endif

	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, value);
	battery->voltage_now = value.intval;

	value.intval = SEC_BATTEY_VOLTAGE_AVERAGE;
	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_VOLTAGE_AVG, value);
	battery->voltage_avg = value.intval;

	value.intval = SEC_BATTEY_VOLTAGE_OCV;
	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_VOLTAGE_AVG, value);
	battery->voltage_ocv = value.intval;

	value.intval = SEC_BATTEY_CURRENT_MA;
	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_CURRENT_NOW, value);
#if defined(CONFIG_QPNP_BMS)
	battery->current_now = value.intval / 1000;
	battery->current_avg = value.intval / 1000;
#else
	battery->current_now = value.intval;

	value.intval = SEC_BATTEY_CURRENT_MA;
	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_CURRENT_AVG, value);
	battery->current_avg = value.intval;
#endif
	/* input current limit in charger */
	psy_do_property(battery->pdata->charger_name, get,
		POWER_SUPPLY_PROP_CURRENT_MAX, value);
	battery->current_max = value.intval;

	sec_bat_get_temperature_info(battery);

	/* To get SOC value (NOT raw SOC), need to reset value */
	value.intval = 0;
	psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_CAPACITY, value);
#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
	/* if the battery status was full, and SOC wasn't 100% yet,
		then ignore FG SOC, and report (previous SOC +1)% */
	if (battery->status != POWER_SUPPLY_STATUS_FULL) {
		battery->capacity = value.intval;
	} else if ((c_ts.tv_sec - old_ts.tv_sec) >= 30) {
		if (battery->capacity != 100) {
			battery->capacity++;
			pr_info("%s : forced full-charged sequence for the capacity(%d)\n",
				__func__, battery->capacity);
		}

		if (value.intval >= battery->pdata->full_condition_soc &&
			battery->voltage_now >= (battery->pdata->recharge_condition_vcell - 50)) {
			/* update capacity max */
			value.intval = battery->capacity;
			psy_do_property(battery->pdata->fuelgauge_name, set,
				POWER_SUPPLY_PROP_CHARGE_FULL, value);
		}
		old_ts = c_ts;
	}
#else
	battery->capacity = value.intval;
#endif

#if !defined(CONFIG_SEC_FACTORY)
#if defined(CONFIG_SW_SELF_DISCHARGING)
	if (battery->temperature >= battery->pdata->self_discharging_temp_block &&
			battery->voltage_now >= battery->pdata->self_discharging_volt_block) {
		battery->sw_self_discharging = true;
		wake_lock(&battery->self_discharging_wake_lock);
	} else if (battery->temperature <= battery->pdata->self_discharging_temp_recov ||
			battery->voltage_now <= battery->pdata->swelling_drop_float_voltage) {
		battery->sw_self_discharging = false;
		wake_unlock(&battery->self_discharging_wake_lock);
	}
	pr_info("%s : sw_self_discharging (%d)\n",__func__, battery->sw_self_discharging);
#endif
#endif

#if !defined(CONFIG_SEC_FACTORY)
	if (battery->capacity > 5 && battery->ignore_store_mode) {
		battery->ignore_store_mode = false;
		value.intval = battery->store_mode;
		psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX, value);
	}
#endif

#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	value.intval = sec_bat_get_earjack_input_current(battery);
	psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
#endif
	dev_info(battery->dev,
		"%s:Vnow(%dmV),Inow(%dmA),Imax(%dmA),SOC(%d%%),Tbat(%d),Tchg(%d),is_hc_usb(%d)\n",
		__func__,
		battery->voltage_now, battery->current_now,
		battery->current_max, battery->capacity,
		battery->temperature, battery->chg_temp,
		battery->is_hc_usb);
	dev_dbg(battery->dev,
		"%s,Vavg(%dmV),Vocv(%dmV),Tamb(%d),"
		"Iavg(%dmA),Iadc(%d)\n",
		battery->present ? "Connected" : "Disconnected",
		battery->voltage_avg, battery->voltage_ocv,
		battery->temper_amb,
		battery->current_avg, battery->current_adc);
}

static void sec_bat_polling_work(struct work_struct *work)
{
	struct sec_battery_info *battery = container_of(
		work, struct sec_battery_info, polling_work.work);

	wake_lock(&battery->monitor_wake_lock);
	queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
	dev_dbg(battery->dev, "%s: Activated\n", __func__);
}

static void sec_bat_program_alarm(
				struct sec_battery_info *battery, int seconds)
{
#if defined(ANDROID_ALARM_ACTIVATED)
	ktime_t low_interval = ktime_set(seconds, 0);
	ktime_t slack = ktime_set(10, 0);
	ktime_t next;

	next = ktime_add(battery->last_poll_time, low_interval);
	alarm_start_range(&battery->polling_alarm,
		next, ktime_add(next, slack));
#else
	alarm_start(&battery->polling_alarm,
		    ktime_add(battery->last_poll_time, ktime_set(seconds, 0)));
#endif
}

static unsigned int sec_bat_get_polling_time(
	struct sec_battery_info *battery)
{
	if (battery->status ==
		POWER_SUPPLY_STATUS_FULL)
		battery->polling_time =
			battery->pdata->polling_time[
			POWER_SUPPLY_STATUS_CHARGING];
	else
		battery->polling_time =
			battery->pdata->polling_time[
			battery->status];

	battery->polling_short = true;

	switch (battery->status) {
	case POWER_SUPPLY_STATUS_CHARGING:
		if (battery->polling_in_sleep)
			battery->polling_short = false;
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
		if (battery->polling_in_sleep && (battery->ps_enable != true)){
#if defined(CONFIG_SW_SELF_DISCHARGING)
			if (battery->voltage_now > battery->pdata->self_discharging_volt_block) {
				if(battery->temperature > battery->pdata->self_discharging_temp_pollingtime)
					battery->polling_time = 300;
				else
					battery->polling_time = 600;
			}
			else
#endif
			battery->polling_time =
				battery->pdata->polling_time[
				SEC_BATTERY_POLLING_TIME_SLEEP];
		} else
			battery->polling_time =
				battery->pdata->polling_time[
				battery->status];
		battery->polling_short = false;
		break;
	case POWER_SUPPLY_STATUS_FULL:
		if (battery->polling_in_sleep) {
			if (!(battery->pdata->full_condition_type &
				SEC_BATTERY_FULL_CONDITION_NOSLEEPINFULL) &&
				battery->charging_mode ==
				SEC_BATTERY_CHARGING_NONE){
#if defined(CONFIG_SW_SELF_DISCHARGING)
			if (battery->voltage_now > battery->pdata->self_discharging_volt_block) {
				if(battery->temperature > battery->pdata->self_discharging_temp_pollingtime)
					battery->polling_time = 300;
				else
					battery->polling_time = 600;
			}
			else
#endif
				battery->polling_time =
					battery->pdata->polling_time[
					SEC_BATTERY_POLLING_TIME_SLEEP];
			}
			battery->polling_short = false;
		} else {
			if (battery->charging_mode ==
				SEC_BATTERY_CHARGING_NONE)
				battery->polling_short = false;
		}
		break;
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		if ((battery->health == POWER_SUPPLY_HEALTH_OVERVOLTAGE ||
			battery->health == POWER_SUPPLY_HEALTH_UNDERVOLTAGE) &&
			battery->health_check_count-- > 0) {
			battery->polling_time = 1;
			battery->polling_short = false;
		}
		break;
	}

	if (battery->polling_short)
		return battery->pdata->polling_time[
			SEC_BATTERY_POLLING_TIME_BASIC];
	/* set polling time to 46s to reduce current noise on wc */
	else if (battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS &&
			battery->status == POWER_SUPPLY_STATUS_CHARGING)
		battery->polling_time = 46;

	return battery->polling_time;
}

static bool sec_bat_is_short_polling(
	struct sec_battery_info *battery)
{
	/* Change the full and short monitoring sequence
	 * Originally, full monitoring was the last time of polling_count
	 * But change full monitoring to first time
	 * because temperature check is too late
	 */
	if (!battery->polling_short || battery->polling_count == 1)
		return false;
	else
		return true;
}

static void sec_bat_update_polling_count(
	struct sec_battery_info *battery)
{
	/* do NOT change polling count in sleep
	 * even though it is short polling
	 * to keep polling count along sleep/wakeup
	 */
	if (battery->polling_short && battery->polling_in_sleep)
		return;

	if (battery->polling_short &&
		((battery->polling_time /
		battery->pdata->polling_time[
		SEC_BATTERY_POLLING_TIME_BASIC])
		> battery->polling_count))
		battery->polling_count++;
	else
		battery->polling_count = 1;	/* initial value = 1 */
}

static void sec_bat_set_polling(
	struct sec_battery_info *battery)
{
	unsigned int polling_time_temp;

	dev_dbg(battery->dev, "%s: Start\n", __func__);

	polling_time_temp = sec_bat_get_polling_time(battery);

	dev_dbg(battery->dev,
		"%s: Status:%s, Sleep:%s, Charging:%s, Short Poll:%s\n",
		__func__, sec_bat_status_str[battery->status],
		battery->polling_in_sleep ? "Yes" : "No",
		(battery->charging_mode ==
		SEC_BATTERY_CHARGING_NONE) ? "No" : "Yes",
		battery->polling_short ? "Yes" : "No");
	dev_dbg(battery->dev,
		"%s: Polling time %d/%d sec.\n", __func__,
		battery->polling_short ?
		(polling_time_temp * battery->polling_count) :
		polling_time_temp, battery->polling_time);

	/* To sync with log above,
	 * change polling count after log is displayed
	 * Do NOT update polling count in initial monitor
	 */
	if (!battery->pdata->monitor_initial_count)
		sec_bat_update_polling_count(battery);
	else
		dev_dbg(battery->dev,
			"%s: Initial monitor %d times left.\n", __func__,
			battery->pdata->monitor_initial_count);

	switch (battery->pdata->polling_type) {
	case SEC_BATTERY_MONITOR_WORKQUEUE:
		if (battery->pdata->monitor_initial_count) {
			battery->pdata->monitor_initial_count--;
			schedule_delayed_work(&battery->polling_work, HZ);
		} else
			schedule_delayed_work(&battery->polling_work,
				polling_time_temp * HZ);
		break;
	case SEC_BATTERY_MONITOR_ALARM:
#if defined(ANDROID_ALARM_ACTIVATED)
		battery->last_poll_time = alarm_get_elapsed_realtime();
#else
		battery->last_poll_time = ktime_get_boottime();
#endif

		if (battery->pdata->monitor_initial_count) {
			battery->pdata->monitor_initial_count--;
			sec_bat_program_alarm(battery, 1);
		} else
			sec_bat_program_alarm(battery, polling_time_temp);
		break;
	case SEC_BATTERY_MONITOR_TIMER:
		break;
	default:
		break;
	}
	dev_dbg(battery->dev, "%s: End\n", __func__);
}
#if defined(CONFIG_BATTERY_SWELLING)
static void sec_bat_swelling_fullcharged_check(struct sec_battery_info *battery)
{
	union power_supply_propval value;

	switch (battery->pdata->full_check_type_2nd) {
	case SEC_BATTERY_FULLCHARGED_FG_CURRENT:
		if (battery->pdata->swelling_full_check_current_2nd > 0) {
			if ((battery->current_now > 0 && battery->current_now <
				battery->pdata->charging_current[
				battery->cable_type].full_check_current_1st) &&
				(battery->current_avg > 0 && battery->current_avg <
				battery->pdata->swelling_full_check_current_2nd)) {
				value.intval = POWER_SUPPLY_STATUS_FULL;
			}
		} else {
			if ((battery->current_now > 0 && battery->current_now <
				battery->pdata->charging_current[
				battery->cable_type].full_check_current_1st) &&
				(battery->current_avg > 0 && battery->current_avg <
				battery->pdata->charging_current[
				battery->cable_type].full_check_current_2nd)) {
				value.intval = POWER_SUPPLY_STATUS_FULL;
			}
		}
		break;
	default:
		psy_do_property(battery->pdata->charger_name, get,
			POWER_SUPPLY_PROP_STATUS, value);
		break;
	}

	if (value.intval == POWER_SUPPLY_STATUS_FULL) {
		battery->swelling_full_check_cnt++;
		pr_info("%s: Swelling mode full-charged check (%d)\n",
			__func__, battery->swelling_full_check_cnt);
	} else
		battery->swelling_full_check_cnt = 0;

	if (battery->swelling_full_check_cnt >=
		battery->pdata->full_check_count) {
		battery->swelling_full_check_cnt = 0;
		battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
		battery->is_recharging = false;
		sec_bat_set_charge(battery, false);
	}
}
#endif
static void sec_bat_monitor_work(
				struct work_struct *work)
{
	struct sec_battery_info *battery =
		container_of(work, struct sec_battery_info,
		monitor_work.work);
	static struct timespec old_ts;
	struct timespec c_ts;

	dev_dbg(battery->dev, "%s: Start\n", __func__);
#if defined(ANDROID_ALARM_ACTIVATED)
	c_ts = ktime_to_timespec(alarm_get_elapsed_realtime());
#else
	c_ts = ktime_to_timespec(ktime_get_boottime());
#endif

	/* monitor once after wakeup */
	if (battery->polling_in_sleep) {
		battery->polling_in_sleep = false;
		if ((battery->status == POWER_SUPPLY_STATUS_DISCHARGING) &&
			(battery->ps_enable != true)) {
			if ((unsigned long)(c_ts.tv_sec - old_ts.tv_sec) < 10 * 60) {
					union power_supply_propval value;

					psy_do_property(battery->pdata->fuelgauge_name, get,
						POWER_SUPPLY_PROP_VOLTAGE_NOW, value);
					battery->voltage_now = value.intval;
					sec_bat_get_temperature_info(battery);
					power_supply_changed(&battery->psy_bat);
					pr_info("Skip monitor work(%ld, Vnow:%dmV, Tbat:%d)\n",
						c_ts.tv_sec - old_ts.tv_sec, battery->voltage_now, battery->temperature);

				goto skip_monitor;
			}
		}
	}
	/* update last monitor time */
	old_ts = c_ts;

	sec_bat_get_battery_info(battery);

	/* 0. test mode */
	if (battery->test_mode) {
		dev_err(battery->dev, "%s: Test Mode\n", __func__);
		sec_bat_do_test_function(battery);
		if (battery->test_mode != 0)
			goto continue_monitor;
	}

	/* 1. battery check */
	if (!sec_bat_battery_cable_check(battery))
		goto continue_monitor;

	/* 2. voltage check */
	if (!sec_bat_voltage_check(battery))
		goto continue_monitor;

	/* monitor short routine in initial monitor */
	if (battery->pdata->monitor_initial_count ||
		sec_bat_is_short_polling(battery))
		goto continue_monitor;

	/* 3. time management */
	if (!sec_bat_time_management(battery))
		goto continue_monitor;

	/* 4. temperature check */
	if (!sec_bat_temperature_check(battery))
		goto continue_monitor;

#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	sec_bat_discharging_check(battery);
#endif

#if defined(CONFIG_BATTERY_SWELLING)
	sec_bat_swelling_check(battery, battery->temperature);

	if (battery->swelling_mode && !battery->charging_block)
		sec_bat_swelling_fullcharged_check(battery);
	else
		sec_bat_fullcharged_check(battery);
#else
	/* 5. full charging check */
	sec_bat_fullcharged_check(battery);
#endif /* CONFIG_BATTERY_SWELLING */

	/* 6. additional check */
	if (battery->pdata->monitor_additional_check)
		battery->pdata->monitor_additional_check();

#if !defined(CONFIG_SEC_FACTORY)
	/* 7. charger temperature check */
	if (battery->pdata->chg_temp_check)
		sec_bat_chg_temperature_check(battery);
#endif

continue_monitor:
#if defined(CONFIG_AFC_CHARGER_MODE)
	dev_info(battery->dev,
		 "%s: Status(%s), mode(%s), Health(%s), Cable(%d), level(%d%%), HV(%s)\n",
		 __func__,
		 sec_bat_status_str[battery->status],
		 sec_bat_charging_mode_str[battery->charging_mode],
		 sec_bat_health_str[battery->health],
		 battery->cable_type, battery->siop_level,
		 battery->hv_chg_name);
#else
	dev_info(battery->dev,
		"%s: Status(%s), Mode(%s), Health(%s), Cable(%d), Vendor(%s), level(%d%%)\n",
		__func__,
		sec_bat_status_str[battery->status],
		sec_bat_charging_mode_str[battery->charging_mode],
		sec_bat_health_str[battery->health],
		battery->cable_type, battery->pdata->vendor, battery->siop_level);
#endif
#if defined(CONFIG_SAMSUNG_BATTERY_ENG_TEST)
	dev_info(battery->dev,
			"%s: battery->stability_test(%d), battery->eng_not_full_status(%d)\n",
			__func__, battery->stability_test, battery->eng_not_full_status);
#endif
	if (battery->store_mode && !poweroff_charging && (battery->cable_type != POWER_SUPPLY_TYPE_BATTERY)) {

		dev_info(battery->dev,
			"%s: @battery->capacity = (%d), battery->status= (%d), battery->store_mode=(%d)\n",
			__func__, battery->capacity, battery->status, battery->store_mode);

		if ((battery->capacity >= STORE_MODE_CHARGING_MAX) && (battery->status == POWER_SUPPLY_STATUS_CHARGING)) {
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_DISCHARGING);
			sec_bat_set_charge(battery, false);
		}
		if ((battery->capacity <= STORE_MODE_CHARGING_MIN) && (battery->status == POWER_SUPPLY_STATUS_DISCHARGING)) {
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_CHARGING);
			sec_bat_set_charge(battery, true);
		}
	}
	power_supply_changed(&battery->psy_bat);

skip_monitor:
	sec_bat_set_polling(battery);

	if (battery->capacity <= 0 || battery->health_change)
		wake_lock_timeout(&battery->monitor_wake_lock, HZ * 5);
	else
		wake_unlock(&battery->monitor_wake_lock);

	dev_dbg(battery->dev, "%s: End\n", __func__);

	return;
}

#if defined(ANDROID_ALARM_ACTIVATED)
static void sec_bat_alarm(struct alarm *alarm)
#else
static enum alarmtimer_restart sec_bat_alarm(
	struct alarm *alarm, ktime_t now)

#endif
{
	struct sec_battery_info *battery = container_of(alarm,
				struct sec_battery_info, polling_alarm);

	dev_dbg(battery->dev,
			"%s\n", __func__);

	/* In wake up, monitor work will be queued in complete function
	 * To avoid duplicated queuing of monitor work,
	 * do NOT queue monitor work in wake up by polling alarm
	 */
	if (!battery->polling_in_sleep) {
		wake_lock(&battery->monitor_wake_lock);
		queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		dev_dbg(battery->dev, "%s: Activated\n", __func__);
	}
#if !defined(ANDROID_ALARM_ACTIVATED)
	return ALARMTIMER_NORESTART;
#endif
}


static void sec_bat_cable_work(struct work_struct *work)
{
	struct sec_battery_info *battery = container_of(work,
				struct sec_battery_info, cable_work.work);
	union power_supply_propval val;
	int wl_cur, wr_cur, current_cable_type;
	int sleep_check_count;

	dev_dbg(battery->dev, "%s: Start\n", __func__);

	/* check fuelgauge in sleep for i2c */
	sleep_check_count = 10;
	while ((battery->fuelgauge_in_sleep == true) && (sleep_check_count > 0)) {
		dev_info(battery->dev, "%s in suspend status count (%d)\n",
			__func__, sleep_check_count);
		sleep_check_count--;
		msleep(50);
	}

	wl_cur = battery->pdata->charging_current[
		POWER_SUPPLY_TYPE_WIRELESS].input_current_limit;
	wr_cur = battery->pdata->charging_current[
		battery->wire_status].input_current_limit;
	if (battery->wc_status && battery->wc_enable &&
			(wl_cur > wr_cur))
		current_cable_type = POWER_SUPPLY_TYPE_WIRELESS;
	else
		current_cable_type = battery->wire_status;

	if ((current_cable_type == battery->cable_type) && !battery->slate_mode) {
		dev_dbg(battery->dev,
				"%s: Cable is NOT Changed(%d)\n",
				__func__, battery->cable_type);
		/* Do NOT activate cable work for NOT changed */
		goto end_of_cable_work;
	}

#if defined(CONFIG_BATTERY_SWELLING)
	battery->swelling_mode = false;
	/* restore 4.4V float voltage */
	val.intval = battery->pdata->swelling_normal_float_voltage;
	psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_VOLTAGE_MAX, val);
#endif

	battery->cable_type = current_cable_type;
	sec_bat_check_cable_result_callback(battery->dev, battery->cable_type);

#ifdef CONFIG_SAMSUNG_BATTERY_DISALLOW_DEEP_SLEEP
	if (battery->pdata->charging_current[battery->cable_type].fast_charging_current != 0) {
		pr_info("QMCK: block xo shutdown\n");
		if (!xo_chr)
			xo_chr = clk_get_sys("charger", "xo_chr"); // Disable xo shutdown
		clk_prepare_enable(xo_chr);
		clk_set_rate(xo_chr, 19200000);
	} else {
		pr_info("QMCK: Enable xo shutdown\n");
		if (xo_chr) {
			clk_disable_unprepare(xo_chr);
			clk_put(xo_chr);
		}
	}
#endif

	/* platform can NOT get information of cable connection
	 * because wakeup time is too short to check uevent
	 * To make sure that target is wakeup
	 * if cable is connected and disconnected,
	 * activated wake lock in a few seconds
	 */
	wake_lock_timeout(&battery->vbus_wake_lock, HZ * 10);

	if (battery->cable_type == POWER_SUPPLY_TYPE_BATTERY ||
		((battery->pdata->cable_check_type &
		SEC_BATTERY_CABLE_CHECK_NOINCOMPATIBLECHARGE) &&
		battery->cable_type == POWER_SUPPLY_TYPE_UNKNOWN)) {

		battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
		battery->is_recharging = false;
		sec_bat_set_charging_status(battery,
				POWER_SUPPLY_STATUS_DISCHARGING);
		battery->health = POWER_SUPPLY_HEALTH_GOOD;

		if (sec_bat_set_charge(battery, false))
			goto end_of_cable_work;
	} else if (battery->slate_mode == true) {
		sec_bat_set_charging_status(battery,
				POWER_SUPPLY_STATUS_DISCHARGING);
		battery->cable_type = POWER_SUPPLY_TYPE_BATTERY;

		val.intval = 0;
		psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_CURRENT_NOW, val);

		dev_info(battery->dev,
			"%s:slate mode on\n",__func__);
	} else {
#if defined(CONFIG_EN_OOPS)
		val.intval = battery->cable_type;
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, val);
#endif
		/* Do NOT display the charging icon when OTG is enabled */
		if (battery->cable_type == POWER_SUPPLY_TYPE_OTG) {
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			sec_bat_set_charging_status(battery, POWER_SUPPLY_STATUS_DISCHARGING);
		} else if (battery->cable_type == POWER_SUPPLY_TYPE_HV_PREPARE_MAINS) {
			val.intval = battery->cable_type;
			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_ONLINE, val);
			dev_info(battery->dev,
				"%s: Prepare AFC cable plugin\n", __func__);
			goto end_of_cable_work;
		} else {
			if (battery->pdata->full_check_type !=
				SEC_BATTERY_FULLCHARGED_NONE)
				battery->charging_mode =
					SEC_BATTERY_CHARGING_1ST;
			else
				battery->charging_mode =
					SEC_BATTERY_CHARGING_2ND;

			if (battery->status == POWER_SUPPLY_STATUS_FULL)
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_FULL);
			else
				sec_bat_set_charging_status(battery,
						POWER_SUPPLY_STATUS_CHARGING);

#if defined(CONFIG_MUIC_NOTIFIER)
			if (sec_bat_set_charge(battery, true))
				goto end_of_cable_work;
		}
#else
		}
		if (sec_bat_set_charge(battery, true))
				goto end_of_cable_work;
#endif
#if defined(ANDROID_ALARM_ACTIVATED)
		/* No need for wakelock in Alarm */
		if (battery->pdata->polling_type != SEC_BATTERY_MONITOR_ALARM)
			wake_lock(&battery->vbus_wake_lock);
#endif
		if (battery->pdata->chg_temp_check &&
			(battery->cable_type == POWER_SUPPLY_TYPE_HV_MAINS ||
			battery->cable_type == POWER_SUPPLY_TYPE_HV_ERR ||
			battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS ) &&
			battery->capacity <= battery->pdata->chg_skip_check_capacity) {
				battery->skip_chg_temp_check = true;
				dev_info(battery->dev,
					"%s: skip_chg_temp_check(%d), Charging Time : %ld secs, soc(%d)\n",
					__func__,
					battery->skip_chg_temp_check,
					battery->charging_passed_time,
					battery->capacity);
		}
	}

	/* polling time should be reset when cable is changed
	 * polling_in_sleep should be reset also
	 * before polling time is re-calculated
	 * to prevent from counting 1 for events
	 * right after cable is connected
	 */
	battery->polling_in_sleep = false;
	sec_bat_get_polling_time(battery);

	dev_info(battery->dev,
		"%s: Status:%s, Sleep:%s, Charging:%s, Short Poll:%s\n",
		__func__, sec_bat_status_str[battery->status],
		battery->polling_in_sleep ? "Yes" : "No",
		(battery->charging_mode ==
		SEC_BATTERY_CHARGING_NONE) ? "No" : "Yes",
		battery->polling_short ? "Yes" : "No");
	dev_info(battery->dev,
		"%s: Polling time is reset to %d sec.\n", __func__,
		battery->polling_time);

	battery->polling_count = 1;	/* initial value = 1 */

	wake_lock(&battery->monitor_wake_lock);
	if (battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS)
		queue_delayed_work(battery->monitor_wqueue, &battery->monitor_work,
				msecs_to_jiffies(0));
	else
		queue_delayed_work(battery->monitor_wqueue, &battery->monitor_work,
				msecs_to_jiffies(500));

end_of_cable_work:
	wake_unlock(&battery->cable_wake_lock);
	dev_dbg(battery->dev, "%s: End\n", __func__);
}

static void sec_bat_vbus_detect_work(struct work_struct *work)
{
	struct sec_battery_info *battery = container_of(work,
				struct sec_battery_info, vbus_detect_work.work);

	dev_dbg(battery->dev, "%s\n", __func__);

	sec_bat_check_cable_callback(battery);
	wake_unlock(&battery->vbus_detect_wake_lock);
}

ssize_t sec_bat_show_attrs(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_bat);
	const ptrdiff_t offset = attr - sec_battery_attrs;
	union power_supply_propval value;
	int i = 0;
	int ret = 0;

	switch (offset) {
	case BATT_RESET_SOC:
		break;
	case BATT_READ_RAW_SOC:
		{
			union power_supply_propval value;
 //when quick start pushed, panic  * (int *) 0 = 0;	
			value.intval =
				SEC_FUELGAUGE_CAPACITY_TYPE_RAW;
			psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_CAPACITY, value);

			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				value.intval);
		}
		
		break;
	case BATT_READ_ADJ_SOC:
		break;
	case BATT_TYPE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
			battery->pdata->vendor);
		break;
	case BATT_VFOCV:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->voltage_ocv);
		break;
	case BATT_VOL_ADC:
		break;
	case BATT_VOL_ADC_CAL:
		break;
	case BATT_VOL_AVER:
		break;
	case BATT_VOL_ADC_AVER:
		break;

	case BATT_CURRENT_UA_NOW:
		{
			union power_supply_propval value;

			value.intval = SEC_BATTEY_CURRENT_UA;
			psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_CURRENT_NOW, value);

			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				value.intval);
		}
		break;
	case BATT_CURRENT_UA_AVG:
		{
			union power_supply_propval value;

			value.intval = SEC_BATTEY_CURRENT_UA;
			psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_CURRENT_AVG, value);

			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				value.intval);
		}
		break;

	case BATT_TEMP:
		switch (battery->pdata->thermal_source) {
		case SEC_BATTERY_THERMAL_SOURCE_FG:
			psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_TEMP, value);
			break;
		case SEC_BATTERY_THERMAL_SOURCE_CALLBACK:
			if (battery->pdata->get_temperature_callback) {
			battery->pdata->get_temperature_callback(
				POWER_SUPPLY_PROP_TEMP, &value);
			}
			break;
		case SEC_BATTERY_THERMAL_SOURCE_ADC:
			sec_bat_get_temperature_by_adc(battery,
				SEC_BAT_ADC_CHANNEL_TEMP, &value);
			break;
		default:
			break;
		}
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			value.intval);
		break;
	case BATT_TEMP_ADC:
		/*
			If F/G is used for reading the temperature and
			compensation table is used,
			the raw value that isn't compensated can be read by
			POWER_SUPPLY_PROP_TEMP_AMBIENT
		 */
		switch (battery->pdata->thermal_source) {
		case SEC_BATTERY_THERMAL_SOURCE_FG:
			psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_TEMP_AMBIENT, value);
			battery->temp_adc = value.intval;
			break;
		default:
			break;
		}
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->temp_adc);
		break;
	case BATT_TEMP_AVER:
		break;
	case BATT_TEMP_ADC_AVER:
		break;
	case BATT_CHG_TEMP:
		if (battery->pdata->chg_temp_check) {
			sec_bat_get_temperature_by_adc(battery,
					       SEC_BAT_ADC_CHANNEL_CHG_TEMP, &value);
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				       value.intval);
		} else {
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				       0);
		}
		break;
	case BATT_CHG_TEMP_ADC:
		if (battery->pdata->chg_temp_check) {
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				       battery->chg_temp_adc);
		} else {
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				       0);
		}
		break;
	case BATT_VF_ADC:
		break;
	case BATT_SLATE_MODE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->slate_mode);
		break;

	case BATT_LP_CHARGING:
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			sec_bat_is_lpm(battery) ? 1 : 0);
		break;
	case SIOP_ACTIVATED:
		break;
	case SIOP_LEVEL:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->siop_level);
		break;
	case BATT_CHARGING_SOURCE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->cable_type);
		break;
	case FG_REG_DUMP:
		break;
	case FG_RESET_CAP:
		break;
	case FG_ASOC:
		psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_ENERGY_FULL, value);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				value.intval);
		break;
	case FG_CAPACITY:
	{
		union power_supply_propval value;

		value.intval =
			SEC_BATTEY_CAPACITY_DESIGNED;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_ENERGY_NOW, value);

		i += scnprintf(buf + i, PAGE_SIZE - i, "0x%04x ",
			value.intval);

		value.intval =
			SEC_BATTEY_CAPACITY_ABSOLUTE;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_ENERGY_NOW, value);

		i += scnprintf(buf + i, PAGE_SIZE - i, "0x%04x ",
			value.intval);

		value.intval =
			SEC_BATTEY_CAPACITY_TEMPERARY;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_ENERGY_NOW, value);

		i += scnprintf(buf + i, PAGE_SIZE - i, "0x%04x ",
			value.intval);

		value.intval =
			SEC_BATTEY_CAPACITY_CURRENT;
		psy_do_property(battery->pdata->fuelgauge_name, get,
			POWER_SUPPLY_PROP_ENERGY_NOW, value);

		i += scnprintf(buf + i, PAGE_SIZE - i, "0x%04x\n",
			value.intval);
	}
		break;
	case AUTH:
		break;
	case CHG_CURRENT_ADC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->current_adc);
		break;
	case WC_ADC:
		break;
	case WC_STATUS:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->cable_type == POWER_SUPPLY_TYPE_WIRELESS));
		break;
	case WC_ENABLE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->wc_enable);
		break;
	case HV_CHARGER_STATUS:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			((battery->cable_type == POWER_SUPPLY_TYPE_HV_MAINS) ||
			(battery->cable_type == POWER_SUPPLY_TYPE_HV_ERR)) ? 1 : 0);
		break;
	case HV_CHARGER_SET:
		break;
	case FACTORY_MODE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->factory_mode);
		break;
	case STORE_MODE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->store_mode);
		break;
	case UPDATE:
		break;
	case TEST_MODE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->test_mode);
		break;

	case BATT_EVENT_CALL:
	case BATT_EVENT_2G_CALL:
	case BATT_EVENT_TALK_GSM:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_2G_CALL) ? 1 : 0);
		break;
	case BATT_EVENT_3G_CALL:
	case BATT_EVENT_TALK_WCDMA:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_3G_CALL) ? 1 : 0);
		break;
	case BATT_EVENT_MUSIC:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_MUSIC) ? 1 : 0);
		break;
	case BATT_EVENT_VIDEO:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_VIDEO) ? 1 : 0);
		break;
	case BATT_EVENT_BROWSER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_BROWSER) ? 1 : 0);
		break;
	case BATT_EVENT_HOTSPOT:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_HOTSPOT) ? 1 : 0);
		break;
	case BATT_EVENT_CAMERA:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_CAMERA) ? 1 : 0);
		break;
	case BATT_EVENT_CAMCORDER:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_CAMCORDER) ? 1 : 0);
		break;
	case BATT_EVENT_DATA_CALL:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_DATA_CALL) ? 1 : 0);
		break;
	case BATT_EVENT_WIFI:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_WIFI) ? 1 : 0);
		break;
	case BATT_EVENT_WIBRO:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_WIBRO) ? 1 : 0);
		break;
	case BATT_EVENT_LTE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_LTE) ? 1 : 0);
		break;
	case BATT_EVENT_LCD:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_LCD) ? 1 : 0);
		break;
	case BATT_EVENT_GPS:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->event & EVENT_GPS) ? 1 : 0);
		break;
	case BATT_EVENT:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->event);
		break;
	case BATT_TEMP_TABLE:
		i += scnprintf(buf + i, PAGE_SIZE - i,
			"%d %d %d %d %d %d %d %d %d %d %d %d\n",
			battery->pdata->temp_high_threshold_event,
			battery->pdata->temp_high_recovery_event,
			battery->pdata->temp_low_threshold_event,
			battery->pdata->temp_low_recovery_event,
			battery->pdata->temp_high_threshold_normal,
			battery->pdata->temp_high_recovery_normal,
			battery->pdata->temp_low_threshold_normal,
			battery->pdata->temp_low_recovery_normal,
			battery->pdata->temp_high_threshold_lpm,
			battery->pdata->temp_high_recovery_lpm,
			battery->pdata->temp_low_threshold_lpm,
			battery->pdata->temp_low_recovery_lpm);
		break;
	case BATT_HIGH_CURRENT_USB:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->is_hc_usb);
		break;
#if defined(CONFIG_SAMSUNG_BATTERY_ENG_TEST)
	case BATT_TEST_CHARGE_CURRENT:
		{
			union power_supply_propval value;

			psy_do_property(battery->pdata->charger_name, get,
				POWER_SUPPLY_PROP_CURRENT_NOW, value);
			i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
					value.intval);
		}
		break;
#endif
	case BATT_STABILITY_TEST:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			battery->stability_test);
		break;
	case BATT_INBAT_VOLTAGE:
#if defined(CONFIG_CHARGER_MAX77849)
	{
		union power_supply_propval value;
		value.intval = 0;
		psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CHARGE_TYPE, value);
		mdelay(200);
		psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, value);
		ret = value.intval;
		if(sec_bat_check_jig_status()) {
			mdelay(200);
			value.intval = 1;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CHARGE_TYPE, value);
		}
	}
#else
		ret = sec_bat_get_adc_value(battery, SEC_BAT_ADC_CHANNEL_INBAT_VOLTAGE);
#endif
		dev_info(battery->dev, "in-battery voltage(%d)\n", ret);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
				ret);
		break;
#if !defined(CONFIG_DISABLE_SAVE_CAPACITY_MAX)
	case BATT_CAPACITY_MAX:
		psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN, value);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", value.intval);
		break;
#endif
	case BATT_DISCHARGING_CHECK:
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		sec_bat_self_discharging_check(battery);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       battery->self_discharging);
#endif
		break;
	case BATT_DISCHARGING_CHECK_ADC:
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		sec_bat_self_discharging_check(battery);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       battery->self_discharging_adc);
#endif
		break;
	case BATT_DISCHARGING_NTC:
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		sec_bat_self_discharging_ntc_check(battery);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       battery->discharging_ntc);
#endif
		break;
	case BATT_DISCHARGING_NTC_ADC:
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		sec_bat_self_discharging_ntc_check(battery);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       battery->discharging_ntc_adc);
#endif
		break;
	case BATT_SELF_DISCHARGING_CONTROL:
		break;
#if defined(CONFIG_SW_SELF_DISCHARGING)
	case BATT_SW_SELF_DISCHARGING:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       battery->sw_self_discharging);
		break;
#endif
	case HMT_TA_CONNECTED:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->cable_type == POWER_SUPPLY_TYPE_HMT_CONNECTED) ? 1 : 0);
		break;
	case HMT_TA_CHARGE:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			(battery->cable_type == POWER_SUPPLY_TYPE_HMT_CHARGE) ? 1 : 0);
		break;
	default:
		i = -EINVAL;
	}

	return i;
}
void update_external_temp_table(struct sec_battery_info *battery, int temp[])
{
	battery->pdata->temp_high_threshold_event = temp[0];
	battery->pdata->temp_high_recovery_event = temp[1];
	battery->pdata->temp_low_threshold_event = temp[2];
	battery->pdata->temp_low_recovery_event = temp[3];
	battery->pdata->temp_high_threshold_normal = temp[4];
	battery->pdata->temp_high_recovery_normal = temp[5];
	battery->pdata->temp_low_threshold_normal = temp[6];
	battery->pdata->temp_low_recovery_normal = temp[7];
	battery->pdata->temp_high_threshold_lpm = temp[8];
	battery->pdata->temp_high_recovery_lpm = temp[9];
	battery->pdata->temp_low_threshold_lpm = temp[10];
	battery->pdata->temp_low_recovery_lpm = temp[11];

	if (battery->pdata->temp_high_threshold_event !=
		battery->pdata->temp_high_threshold_normal)
		battery->pdata->event_check = 1;
}

ssize_t sec_bat_store_attrs(
					struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_bat);
	const ptrdiff_t offset = attr - sec_battery_attrs;
	int ret = -EINVAL;
	int x = 0;
	int t[12];
	switch (offset) {
	case BATT_RESET_SOC:
		/* Do NOT reset fuel gauge in charging mode */
		if ((battery->cable_type == POWER_SUPPLY_TYPE_BATTERY) ||
#if defined(CONFIG_MUIC_NOTIFIER)
			battery->is_jig_on) {
#else
			sec_bat_check_jig_status()) {
#endif
#if defined(CONFIG_QPNP_BMS)
			extern void bms_quickstart(void);
			battery->voltage_now = 1234;
			battery->voltage_avg = 1234;
			power_supply_changed(&battery->psy_bat);
			bms_quickstart();
#else
			union power_supply_propval value;
			battery->voltage_now = 1234;
			battery->voltage_avg = 1234;
			power_supply_changed(&battery->psy_bat);

			value.intval =
				SEC_FUELGAUGE_CAPACITY_TYPE_RESET;
			psy_do_property(battery->pdata->fuelgauge_name, set,
					POWER_SUPPLY_PROP_CAPACITY, value);
#endif
			dev_info(battery->dev,"do reset SOC\n");
			/* update battery info */
			sec_bat_get_battery_info(battery);
		}
		ret = count;
		break;
	case BATT_READ_RAW_SOC:
		break;
	case BATT_READ_ADJ_SOC:
		break;
	case BATT_TYPE:
		break;
	case BATT_VFOCV:
		break;
	case BATT_VOL_ADC:
		break;
	case BATT_VOL_ADC_CAL:
		break;
	case BATT_VOL_AVER:
		break;
	case BATT_VOL_ADC_AVER:
		break;
	case BATT_CURRENT_UA_NOW:
		break;
	case BATT_CURRENT_UA_AVG:
		break;
	case BATT_TEMP:
		break;
	case BATT_TEMP_ADC:
		break;
	case BATT_TEMP_AVER:
		break;
	case BATT_TEMP_ADC_AVER:
		break;
	case BATT_CHG_TEMP:
		break;
	case BATT_CHG_TEMP_ADC:
		break;
	case BATT_VF_ADC:
		break;
	case BATT_SLATE_MODE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 1) {
				battery->slate_mode = true;
			} else if (x == 0) {
				battery->slate_mode = false;
			} else {
				dev_info(battery->dev,
					"%s: SLATE MODE unknown command\n",
					__func__);
				return -EINVAL;
			}
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
					   &battery->cable_work, 0);
			ret = count;
		}
		break;

	case BATT_LP_CHARGING:
		break;
	case SIOP_ACTIVATED:
		break;
	case SIOP_LEVEL:

#if defined(CONFIG_TMM_CHG_CTRL)
	if(tuner_running_status==TUNER_IS_OFF) {
		dev_dbg(battery->dev,
			"%s: tmm tuner is off!\n", __func__);
#endif

		if (sscanf(buf, "%d\n", &x) == 1) {
			union power_supply_propval value;
			dev_info(battery->dev,
				"%s: siop level: %d\n", __func__, x);
			battery->chg_limit = SEC_BATTERY_CHG_TEMP_NONE;
			if (battery->capacity <= 5)
				battery->siop_level = 100;
			else if (x >= 0 && x <= 100)
				battery->siop_level = x;
			else
				battery->siop_level = 100;
			value.intval = battery->siop_level;
			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, value);
			ret = count;
		}

#if defined(CONFIG_TMM_CHG_CTRL)
	}
#endif

		break;
	case BATT_CHARGING_SOURCE:
		break;
	case FG_REG_DUMP:
		break;
	case FG_RESET_CAP:
		break;
	case FG_CAPACITY:
		break;
	case AUTH:
		break;
	case CHG_CURRENT_ADC:
		break;
	case WC_ADC:
		break;
	case WC_STATUS:
		break;
	case WC_ENABLE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x == 0) {
				battery->wc_enable = false;
			} else if (x == 1) {
				battery->wc_enable = true;
			} else {
				dev_info(battery->dev,
					"%s: WPC ENABLE unknown command\n",
					__func__);
				return -EINVAL;
			}
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
					&battery->cable_work, 0);
			ret = count;
		}
		break;
	case HV_CHARGER_STATUS:
		break;
	case HV_CHARGER_SET:
		if (sscanf(buf, "%d\n", &x) == 1) {
			dev_info(battery->dev,
				"%s: HV_CHARGER_SET(%d)\n", __func__, x);
			if (x == 1) {
				battery->wire_status = POWER_SUPPLY_TYPE_HV_MAINS;
				wake_lock(&battery->cable_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
			} else {
				battery->wire_status = POWER_SUPPLY_TYPE_BATTERY;
				wake_lock(&battery->cable_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
			}
			ret = count;
		}
		break;
	case FACTORY_MODE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			battery->factory_mode = x ? true : false;
			ret = count;
		}
		break;
	case STORE_MODE:
		if (sscanf(buf, "%d\n", &x) == 1) {
		        if (x)
			        battery->store_mode = true;
			ret = count;
#if !defined(CONFIG_SEC_FACTORY)
			if (battery->store_mode) {
				if (battery->capacity <= 5) {
					battery->ignore_store_mode = true;
				} else {
					union power_supply_propval value;
					value.intval = battery->store_mode;
					psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX, value);
				}
			}
#endif
		}
		break;
	case UPDATE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			/* update battery info */
			sec_bat_get_battery_info(battery);
			ret = count;
		}
		break;
	case TEST_MODE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			battery->test_mode = x;
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
				&battery->monitor_work, 0);
			ret = count;
		}
		break;

	case BATT_EVENT_CALL:
	case BATT_EVENT_2G_CALL:
	case BATT_EVENT_TALK_GSM:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_2G_CALL, x);
			ret = count;
		}
		break;
	case BATT_EVENT_3G_CALL:
	case BATT_EVENT_TALK_WCDMA:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_3G_CALL, x);
			ret = count;
		}
		break;
	case BATT_EVENT_MUSIC:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_MUSIC, x);
			ret = count;
		}
		break;
	case BATT_EVENT_VIDEO:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_VIDEO, x);
			ret = count;
		}
		break;
	case BATT_EVENT_BROWSER:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_BROWSER, x);
			ret = count;
		}
		break;
	case BATT_EVENT_HOTSPOT:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_HOTSPOT, x);
			ret = count;
		}
		break;
	case BATT_EVENT_CAMERA:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_CAMERA, x);
			ret = count;
		}
		break;
	case BATT_EVENT_CAMCORDER:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_CAMCORDER, x);
			ret = count;
		}
		break;
	case BATT_EVENT_DATA_CALL:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_DATA_CALL, x);
			ret = count;
		}
		break;
	case BATT_EVENT_WIFI:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_WIFI, x);
			ret = count;
		}
		break;
	case BATT_EVENT_WIBRO:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_WIBRO, x);
			ret = count;
		}
		break;
	case BATT_EVENT_LTE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_LTE, x);
			ret = count;
		}
		break;
	case BATT_EVENT_LCD:
		if (sscanf(buf, "%d\n", &x) == 1) {
			/* we need to test
			sec_bat_event_set(battery, EVENT_LCD, x);
			*/
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
			sec_bat_earjack_lcd_state(battery, x);
#endif
			ret = count;
		}
		break;
	case BATT_EVENT_GPS:
		if (sscanf(buf, "%d\n", &x) == 1) {
			sec_bat_event_set(battery, EVENT_GPS, x);
			ret = count;
		}
		break;
	case BATT_TEMP_TABLE:
		if (sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d\n",
			&t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6], &t[7], &t[8], &t[9], &t[10], &t[11]) == 12) {
			pr_info("%s: (new) %d %d %d %d %d %d %d %d %d %d %d %d\n",
				__func__, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9], t[10], t[11]);
			pr_info("%s: (default) %d %d %d %d %d %d %d %d %d %d %d %d\n",
				__func__,
				battery->pdata->temp_high_threshold_event,
				battery->pdata->temp_high_recovery_event,
				battery->pdata->temp_low_threshold_event,
				battery->pdata->temp_low_recovery_event,
				battery->pdata->temp_high_threshold_normal,
				battery->pdata->temp_high_recovery_normal,
				battery->pdata->temp_low_threshold_normal,
				battery->pdata->temp_low_recovery_normal,
				battery->pdata->temp_high_threshold_lpm,
				battery->pdata->temp_high_recovery_lpm,
				battery->pdata->temp_low_threshold_lpm,
				battery->pdata->temp_low_recovery_lpm);
			update_external_temp_table(battery, t);
			ret = count;
		}
		break;
	case BATT_HIGH_CURRENT_USB:
		if (sscanf(buf, "%d\n", &x) == 1) {
			union power_supply_propval value;
			battery->is_hc_usb = x ? true : false;
			value.intval = battery->is_hc_usb;

			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_USB_HC, value);

			pr_info("%s: is_hc_usb (%d)\n", __func__, battery->is_hc_usb);
			ret = count;
		}
		break;
#if defined(CONFIG_SAMSUNG_BATTERY_ENG_TEST)
	case BATT_TEST_CHARGE_CURRENT:
		if (sscanf(buf, "%d\n", &x) == 1) {
			if (x >= 0 && x <= 2000) {
				union power_supply_propval value;
				dev_err(battery->dev,
					"%s: BATT_TEST_CHARGE_CURRENT(%d)\n", __func__, x);
				battery->pdata->charging_current[
					POWER_SUPPLY_TYPE_USB].input_current_limit = x;
				battery->pdata->charging_current[
					POWER_SUPPLY_TYPE_USB].fast_charging_current = x;
				if (x > 500) {
					battery->eng_not_full_status = true;
					battery->pdata->temp_check_type =
						SEC_BATTERY_TEMP_CHECK_NONE;
					battery->pdata->charging_total_time =
						10000 * 60 * 60;
				}
				if (battery->cable_type == POWER_SUPPLY_TYPE_USB) {
					value.intval = x;

#if defined(CONFIG_TMM_CHG_CTRL)
				if(tuner_running_status==TUNER_IS_ON) {
					dev_dbg(battery->dev,
						"%s: tmm tuner is on!\n", __func__);

					if(value.intval > TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE) {
						value.intval = TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE;
					}
				}
#endif

					psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CURRENT_NOW,
						value);
				}
			}
			ret = count;
		}
		break;
#endif
	case BATT_STABILITY_TEST:
		if (sscanf(buf, "%d\n", &x) == 1) {
			dev_err(battery->dev,
				"%s: BATT_STABILITY_TEST(%d)\n", __func__, x);
			if (x) {
				battery->stability_test = true;
				battery->eng_not_full_status = true;
			}
			else {
				battery->stability_test = false;
				battery->eng_not_full_status = false;
			}
			ret = count;
		}
		break;
	case BATT_INBAT_VOLTAGE:
		break;
#if !defined(CONFIG_DISABLE_SAVE_CAPACITY_MAX)
	case BATT_CAPACITY_MAX:
		if (sscanf(buf, "%d\n", &x) == 1) {
			union power_supply_propval value;
			dev_err(battery->dev,
					"%s: BATT_CAPACITY_MAX(%d), fg_reset(%d)\n", __func__, x, fg_reset);
			if (x  > 800 && x < 1200 && !fg_reset) {
				value.intval = x;
				psy_do_property(battery->pdata->fuelgauge_name, set,
						POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN, value);
				/* To get SOC value (NOT raw SOC), need to reset value */
				value.intval = 0;
				psy_do_property(battery->pdata->fuelgauge_name, get,
						POWER_SUPPLY_PROP_CAPACITY, value);
				dev_err(battery->dev,
					"%s: soc(%d)->soc(%d), BATT_CAPACITY_MAX(%d)\n",
					__func__, battery->capacity, value.intval, x);
				battery->capacity = value.intval;
				wake_lock(&battery->monitor_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue,
						   &battery->monitor_work, 0);
			}
			ret = count;
		}
		break;
#endif
	case BATT_DISCHARGING_CHECK:
		break;
	case BATT_DISCHARGING_CHECK_ADC:
		break;
	case BATT_DISCHARGING_NTC:
		break;
	case BATT_DISCHARGING_NTC_ADC:
		break;
	case BATT_SELF_DISCHARGING_CONTROL:
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		if (sscanf(buf, "%d\n", &x) == 1) {
			dev_err(battery->dev,
				"%s: BATT_SELF_DISCHARGING_CONTROL(%d)\n", __func__, x);
			if (x) {
				pr_info("SELF DISCHARGING IC ENABLE\n");
				sec_bat_self_discharging_control(battery, true);
			} else {
				pr_info("SELF DISCHARGING IC DISABLE\n");
				sec_bat_self_discharging_control(battery, false);
			}
			ret = count;
		}
#endif
		break;
#if defined(CONFIG_SW_SELF_DISCHARGING)
	case BATT_SW_SELF_DISCHARGING:
		break;
#endif
	case HMT_TA_CONNECTED:
		if (sscanf(buf, "%d\n", &x) == 1) {
			union power_supply_propval value;
			dev_info(battery->dev,
					"%s: HMT_TA_CONNECTED(%d)\n", __func__, x);
			if (x) {
				value.intval = false;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL,
						value);
				dev_info(battery->dev,
						"%s: changed to OTG cable detached\n", __func__);
				battery->wire_status = POWER_SUPPLY_TYPE_HMT_CONNECTED;
				wake_lock(&battery->cable_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
			} else {
				value.intval = true;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL,
						value);
				dev_info(battery->dev,
						"%s: changed to OTG cable attached\n", __func__);
				battery->wire_status = POWER_SUPPLY_TYPE_OTG;
				wake_lock(&battery->cable_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
			}
			ret = count;
		}
		break;
	case HMT_TA_CHARGE:
		if (sscanf(buf, "%d\n", &x) == 1) {
			union power_supply_propval value;
			dev_info(battery->dev,
					"%s: HMT_TA_CHARGE(%d)\n", __func__, x);
			psy_do_property(battery->pdata->charger_name, get,
					POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL, value);
			if (value.intval) {
				dev_info(battery->dev,
					"%s: ignore HMT_TA_CHARGE(%d)\n", __func__, x);
			} else {
				if (x) {
					value.intval = false;
					psy_do_property(battery->pdata->charger_name, set,
							POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL,
							value);
					dev_info(battery->dev,
						"%s: changed to OTG cable detached\n", __func__);
					battery->wire_status = POWER_SUPPLY_TYPE_HMT_CHARGE;
					wake_lock(&battery->cable_wake_lock);
					queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
				} else {
					value.intval = false;
					psy_do_property(battery->pdata->charger_name, set,
							POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL,
							value);
					dev_info(battery->dev,
							"%s: changed to OTG cable detached\n", __func__);
					battery->wire_status = POWER_SUPPLY_TYPE_HMT_CONNECTED;
					wake_lock(&battery->cable_wake_lock);
					queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
				}
			}
			ret = count;
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int sec_bat_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(sec_battery_attrs); i++) {
		rc = device_create_file(dev, &sec_battery_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	while (i--)
		device_remove_file(dev, &sec_battery_attrs[i]);
create_attrs_succeed:
	return rc;
}

static int sec_bat_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_bat);
	int current_cable_type;
	int full_check_type;

	dev_dbg(battery->dev,
		"%s: (%d,%d)\n", __func__, psp, val->intval);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (battery->charging_mode == SEC_BATTERY_CHARGING_1ST)
			full_check_type = battery->pdata->full_check_type;
		else
			full_check_type = battery->pdata->full_check_type_2nd;
		if ((full_check_type == SEC_BATTERY_FULLCHARGED_CHGINT) &&
			(val->intval == POWER_SUPPLY_STATUS_FULL))
			sec_bat_do_fullcharged(battery);
		sec_bat_set_charging_status(battery, val->intval);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		sec_bat_ovp_uvlo_result(battery, val->intval);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		current_cable_type = val->intval;

#if defined(CONFIG_CHARGER_RT5033) && defined(CONFIG_SM5504_MUIC)
                if(current_cable_type != POWER_SUPPLY_TYPE_UNKNOWN &&
                        current_cable_type != POWER_SUPPLY_TYPE_BATTERY)
			msleep(150);
#endif
		if (current_cable_type < 0) {
			dev_info(battery->dev,
					"%s: ignore event(%d)\n",
					__func__, current_cable_type);
#if defined(CONFIG_MUIC_NOTIFIER)
		} else if (current_cable_type == POWER_SUPPLY_TYPE_OTG) {
			battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
			battery->is_recharging = false;
			sec_bat_set_charging_status(battery,
					POWER_SUPPLY_STATUS_DISCHARGING);
			battery->cable_type = current_cable_type;
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
					   &battery->monitor_work, 0);
			break;
#endif
		} else {
			battery->wire_status = current_cable_type;
			if ((battery->wire_status == POWER_SUPPLY_TYPE_BATTERY)
					&& battery->wc_status)
				current_cable_type = POWER_SUPPLY_TYPE_WIRELESS;
		}
		dev_info(battery->dev,
				"%s: current_cable(%d), wc_status(%d), wire_status(%d)\n",
				__func__, current_cable_type, battery->wc_status,
				battery->wire_status);

		/* cable is attached or detached
		 * if current_cable_type is minus value,
		 * check cable by sec_bat_get_cable_type()
		 * although SEC_BATTERY_CABLE_SOURCE_EXTERNAL is set
		 * (0 is POWER_SUPPLY_TYPE_UNKNOWN)
		 */
		if ((current_cable_type >= 0) &&
			(current_cable_type < SEC_SIZEOF_POWER_SUPPLY_TYPE) &&
			(battery->pdata->cable_source_type &
			SEC_BATTERY_CABLE_SOURCE_EXTERNAL)) {

			wake_lock(&battery->cable_wake_lock);
				queue_delayed_work_on(0, battery->monitor_wqueue,
					&battery->cable_work,0);
		} else {
			if (sec_bat_get_cable_type(battery,
						battery->pdata->cable_source_type)) {
				wake_lock(&battery->cable_wake_lock);
					queue_delayed_work_on(0, battery->monitor_wqueue,
						&battery->cable_work,0);
			}
		}
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		battery->capacity = val->intval;
		power_supply_changed(&battery->psy_bat);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		/* If JIG is attached, the voltage is set as 1079 */
		pr_info("%s : set to the battery history : (%d)\n",__func__, val->intval);
		if(val->intval == 1079)	{
			battery->voltage_now = 1079;
			battery->voltage_avg = 1079;
			power_supply_changed(&battery->psy_bat);
		}
		break;

#if defined(CONFIG_TMM_CHG_CTRL)
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		if((val->intval)==TUNER_SWITCHED_ON_SIGNAL) {
			dev_dbg(battery->dev,
				"%s: tmm switched on!\n", __func__);
			if((battery->cable_type != POWER_SUPPLY_TYPE_UNKNOWN) &&
				(battery->cable_type != POWER_SUPPLY_TYPE_BATTERY)) {

				union power_supply_propval value_Check_CurrentNow;
				int input_curr_limit;

				psy_do_property(battery->pdata->charger_name, get,
					POWER_SUPPLY_PROP_CURRENT_NOW, value_Check_CurrentNow);

				input_curr_limit = value_Check_CurrentNow.intval;

				if(input_curr_limit > TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE) {
					union power_supply_propval value;
					value.intval = TMM_CHG_CTRL_INPUT_LIMIT_CURRENT_VALUE;
					psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_CURRENT_NOW, value);
				}
			}
			tuner_running_status=TUNER_IS_ON;
		}else if((val->intval)==TUNER_SWITCHED_OFF_SIGNAL) {
			union power_supply_propval value;

			dev_dbg(battery->dev,
				"%s: tmm switched off!\n", __func__);
			value.intval = battery->cable_type;
			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_ONLINE, value);
			tuner_running_status=TUNER_IS_OFF;
		}
		break;
#endif
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		battery->present = val->intval;

		wake_lock(&battery->monitor_wake_lock);
		queue_delayed_work_on(0, battery->monitor_wqueue,
				   &battery->monitor_work, 0);
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

static int sec_bat_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_bat);
	union power_supply_propval value;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if ((battery->health == POWER_SUPPLY_HEALTH_OVERVOLTAGE) ||
			(battery->health == POWER_SUPPLY_HEALTH_UNDERVOLTAGE)) {
				val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		} else {
			if ((battery->pdata->cable_check_type &
					SEC_BATTERY_CABLE_CHECK_NOUSBCHARGE) &&
					!sec_bat_is_lpm(battery)) {
				switch (battery->cable_type) {
				case POWER_SUPPLY_TYPE_USB:
				case POWER_SUPPLY_TYPE_USB_DCP:
				case POWER_SUPPLY_TYPE_USB_CDP:
				case POWER_SUPPLY_TYPE_USB_ACA:
					val->intval =
						POWER_SUPPLY_STATUS_DISCHARGING;
					return 0;
				}
			}
#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
			if (battery->status == POWER_SUPPLY_STATUS_FULL &&
				battery->capacity != 100) {
				val->intval = POWER_SUPPLY_STATUS_CHARGING;
				pr_info("%s: forced full-charged sequence progressing\n", __func__);
			} else
#endif
				val->intval = battery->status;
		}
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		if (battery->cable_type == POWER_SUPPLY_TYPE_BATTERY ||
			battery->cable_type == POWER_SUPPLY_TYPE_MHL_USB_100) {
			val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
		} else {
			psy_do_property(battery->pdata->charger_name, get,
				POWER_SUPPLY_PROP_CHARGE_TYPE, value);
			if (value.intval == POWER_SUPPLY_CHARGE_TYPE_UNKNOWN)
				/* if error in CHARGE_TYPE of charger
				 * set CHARGE_TYPE as NONE
				 */
				val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
			else
				val->intval = value.intval;
		}
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = battery->health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = battery->present;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = battery->cable_type;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = battery->pdata->technology;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
#ifdef CONFIG_SEC_FACTORY
		psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_VOLTAGE_NOW, value);
		battery->voltage_now = value.intval;
		dev_err(battery->dev,
			"%s: voltage now(%d)\n", __func__, battery->voltage_now);
#endif
		/* voltage value should be in uV */
		val->intval = battery->voltage_now * 1000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
#ifdef CONFIG_SEC_FACTORY
		value.intval = SEC_BATTEY_VOLTAGE_AVERAGE;
		psy_do_property(battery->pdata->fuelgauge_name, get,
				POWER_SUPPLY_PROP_VOLTAGE_AVG, value);
		battery->voltage_avg = value.intval;
		dev_err(battery->dev,
			"%s: voltage avg(%d)\n", __func__, battery->voltage_avg);
#endif
		/* voltage value should be in uV */
		val->intval = battery->voltage_avg * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = battery->current_now;
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		val->intval = battery->current_avg;
		break;
	/* charging mode (differ from power supply) */
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = battery->charging_mode;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
#if defined(CONFIG_SAMSUNG_BATTERY_ENG_TEST)
		if (battery->status == POWER_SUPPLY_STATUS_FULL) {
			if(battery->eng_not_full_status)
				val->intval = battery->capacity;
			else
				val->intval = 100;
		} else {
			val->intval = battery->capacity;
		}
#else
#if defined(CONFIG_AFC_CHARGER_MODE) || defined(CONFIG_PREVENT_SOC_JUMP)
		val->intval = battery->capacity;
#else
		/* In full-charged status, SOC is always 100% */
		if (battery->status == POWER_SUPPLY_STATUS_FULL)
			val->intval = 100;
		else
			val->intval = battery->capacity;
#endif
#endif
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = battery->temperature;
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		val->intval = battery->temper_amb;
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT:
		if (battery->swelling_mode)
			val->intval = 1;
		else
			val->intval = 0;
		break;
#endif
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	case POWER_SUPPLY_PROP_INPUT_CURRENT_MAX:
		val->intval = sec_bat_get_earjack_input_current(battery);
		break;
#endif
	default:
		return -EINVAL;
	}
	return 0;
}

static int sec_usb_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_usb);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	if ((battery->health == POWER_SUPPLY_HEALTH_OVERVOLTAGE) ||
		(battery->health == POWER_SUPPLY_HEALTH_UNDERVOLTAGE)) {
		val->intval = 0;
		return 0;
	}
	/* Set enable=1 only if the USB charger is connected */
	switch (battery->wire_status) {
	case POWER_SUPPLY_TYPE_USB:
	case POWER_SUPPLY_TYPE_USB_DCP:
	case POWER_SUPPLY_TYPE_USB_CDP:
	case POWER_SUPPLY_TYPE_USB_ACA:
	case POWER_SUPPLY_TYPE_MHL_USB:
	case POWER_SUPPLY_TYPE_MHL_USB_100:
		val->intval = 1;
		break;
	default:
		val->intval = 0;
		break;
	}

	if (battery->slate_mode)
		val->intval = 0;
	return 0;
}

static int sec_ac_get_property(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_ac);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	if ((battery->health == POWER_SUPPLY_HEALTH_OVERVOLTAGE) ||
		(battery->health == POWER_SUPPLY_HEALTH_UNDERVOLTAGE)) {
			val->intval = 0;
			return 0;
	}

	/* Set enable=1 only if the AC charger is connected */
	switch (battery->cable_type) {
	case POWER_SUPPLY_TYPE_MAINS:
	case POWER_SUPPLY_TYPE_MISC:
	case POWER_SUPPLY_TYPE_CARDOCK:
	case POWER_SUPPLY_TYPE_UARTOFF:
	case POWER_SUPPLY_TYPE_LAN_HUB:
	case POWER_SUPPLY_TYPE_UNKNOWN:
	case POWER_SUPPLY_TYPE_MHL_500:
	case POWER_SUPPLY_TYPE_MHL_900:
	case POWER_SUPPLY_TYPE_MHL_1500:
	case POWER_SUPPLY_TYPE_MHL_2000:
	case POWER_SUPPLY_TYPE_SMART_OTG:
	case POWER_SUPPLY_TYPE_SMART_NOTG:
	case POWER_SUPPLY_TYPE_HV_PREPARE_MAINS:
	case POWER_SUPPLY_TYPE_HV_ERR:
	case POWER_SUPPLY_TYPE_HV_UNKNOWN:
	case POWER_SUPPLY_TYPE_HV_MAINS:
#if defined(CONFIG_MUIC_SUPPORT_MULTIMEDIA_DOCK)
	case POWER_SUPPLY_TYPE_MDOCK_TA:
#endif
	case POWER_SUPPLY_TYPE_HMT_CONNECTED:
	case POWER_SUPPLY_TYPE_HMT_CHARGE:
		val->intval = 1;
		break;
	default:
		val->intval = 0;
		break;
	}

	return 0;
}

static int sec_wireless_get_property(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_wireless);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	if (battery->wc_status)
		val->intval = 1;
	else
		val->intval = 0;

	return 0;
}

static int sec_wireless_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_wireless);

	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	battery->wc_status = val->intval;

	wake_lock(&battery->cable_wake_lock);
	queue_delayed_work(battery->monitor_wqueue,
		&battery->cable_work, 0);

	return 0;
}

static int sec_ps_set_property(struct power_supply *psy,
				enum power_supply_property psp,
				const union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_ps);
	union power_supply_propval value;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
	if (val->intval == 0) {
		if (battery->ps_enable == true) {
			battery->ps_enable = val->intval;
				dev_info(battery->dev,
					"%s: power sharing cable set (%d)\n", __func__, battery->ps_enable);
			value.intval = POWER_SUPPLY_TYPE_POWER_SHARING;
			psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_ONLINE, value);
		}
	} else if ((val->intval == 1) && (battery->ps_status == true)) {
		battery->ps_enable = val->intval;
			dev_info(battery->dev,
				"%s: power sharing cable set (%d)\n", __func__, battery->ps_enable);
		value.intval = POWER_SUPPLY_TYPE_POWER_SHARING;
		psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_ONLINE, value);
	} else {
		dev_err(battery->dev,
			"%s: invalid setting (%d) ps_status (%d)\n",
			__func__, val->intval, battery->ps_status);
	}
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		if (val->intval == POWER_SUPPLY_TYPE_POWER_SHARING) {
			battery->ps_status = true;
			battery->ps_enable = true;
			battery->ps_changed = true;
			dev_info(battery->dev,
				"%s: power sharing cable plugin (%d)\n", __func__, battery->ps_status);
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		} else {
			battery->ps_status = false;
			dev_info(battery->dev,
				"%s: power sharing cable plugout (%d)\n", __func__, battery->ps_status);
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sec_ps_get_property(struct power_supply *psy,
			       enum power_supply_property psp,
			       union power_supply_propval *val)
{
	struct sec_battery_info *battery =
		container_of(psy, struct sec_battery_info, psy_ps);
	union power_supply_propval value;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (battery->ps_enable)
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		if (battery->ps_status) {
			if ((battery->ps_enable == true) && (battery->ps_changed == true)) {
				battery->ps_changed = false;

				value.intval = POWER_SUPPLY_TYPE_POWER_SHARING;
				psy_do_property(battery->pdata->charger_name, set,
						POWER_SUPPLY_PROP_ONLINE, value);
			}
			val->intval = 1;
		} else {
			if (battery->ps_enable == true) {
				battery->ps_enable = false;
				dev_info(battery->dev,
					"%s: power sharing cable disconnected! ps disable (%d)\n",
					__func__, battery->ps_enable);

				value.intval = POWER_SUPPLY_TYPE_POWER_SHARING;
				psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_ONLINE, value);
			}
			val->intval = 0;
		}
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static irqreturn_t sec_bat_irq_thread(int irq, void *irq_data)
{
	struct sec_battery_info *battery = irq_data;

	if ((battery->pdata->battery_check_type == SEC_BATTERY_CHECK_INT)
		|| (battery->pdata->battery_check_type == SEC_BATTERY_CHECK_FUELGAUGE)
		|| (battery->pdata->battery_check_type == SEC_BATTERY_CHECK_ADC)) {
		if (battery->pdata->check_battery_callback)
		battery->present = battery->pdata->check_battery_callback();

		wake_lock(&battery->monitor_wake_lock);
		queue_delayed_work(battery->monitor_wqueue, &battery->monitor_work, 0);
	}

	return IRQ_HANDLED;
}

static irqreturn_t sec_ta_irq_thread(int irq, void *irq_data)
{
	struct sec_battery_info *battery = irq_data;

	if (battery->pdata->cable_source_type & SEC_BATTERY_CABLE_SOURCE_CALLBACK) {
		wake_lock(&battery->vbus_detect_wake_lock);
		queue_delayed_work(battery->monitor_wqueue,
			&battery->vbus_detect_work, msecs_to_jiffies(750));
	}

	return IRQ_HANDLED;
}

#if defined(CONFIG_EXTCON)
static int sec_bat_cable_check(struct sec_battery_info *battery,
				enum extcon_cable_name attached_dev)
{
	int current_cable_type = -1;
	bool is_jig_attached = false;

	switch (attached_dev)
	{
	case EXTCON_JIG_UARTOFF:
	case EXTCON_JIG_UARTON:
		is_jig_attached = true;
	case EXTCON_DESKDOCK:
	case EXTCON_SMARTDOCK:
		current_cable_type = POWER_SUPPLY_TYPE_BATTERY;
		break;
	case EXTCON_USB_HOST:
		current_cable_type = POWER_SUPPLY_TYPE_OTG;
		break;
	case EXTCON_JIG_USBOFF:
	case EXTCON_JIG_USBON:
		is_jig_attached = true;
	case EXTCON_USB:
	case EXTCON_SMARTDOCK_USB:
		current_cable_type = POWER_SUPPLY_TYPE_USB;
		break;
	case EXTCON_JIG_UARTOFF_VB:
		current_cable_type = POWER_SUPPLY_TYPE_UARTOFF;
		break;
	case EXTCON_TA:
	case EXTCON_DESKDOCK_VB:
	case EXTCON_SMARTDOCK_TA:
	case EXTCON_UNDEFINED_CHARGER:
#if defined(CONFIG_MUIC_SUPPORT_LANHUB)
	case EXTCON_LANHUB_TA:
#endif
		current_cable_type = POWER_SUPPLY_TYPE_MAINS;
		break;
	case EXTCON_CHARGE_DOWNSTREAM:
		current_cable_type = POWER_SUPPLY_TYPE_USB_CDP;
		break;
	case EXTCON_INCOMPATIBLE:
		current_cable_type = POWER_SUPPLY_TYPE_UNKNOWN;
		break;
	case EXTCON_AUDIODOCK:
		current_cable_type = POWER_SUPPLY_TYPE_MISC;
		break;
	case EXTCON_CHARGING_CABLE:
		current_cable_type = POWER_SUPPLY_TYPE_POWER_SHARING;
		break;
	case EXTCON_HV_PREPARE:
		current_cable_type = POWER_SUPPLY_TYPE_HV_PREPARE_MAINS;
		break;
	case EXTCON_HV_TA_ERR:
		current_cable_type = POWER_SUPPLY_TYPE_HV_ERR;
		break;
	case EXTCON_HV_TA_1A:
		current_cable_type = POWER_SUPPLY_TYPE_HV_UNKNOWN;
		break;
	case EXTCON_HV_TA:
		current_cable_type = POWER_SUPPLY_TYPE_HV_MAINS;
		break;
	case EXTCON_CARDOCK:
		is_jig_attached = true;
		break;
	default:
		pr_err("%s: invalid type for charger:%d\n",
			__func__, attached_dev);
	}

#if defined(CONFIG_CHARGER_MAX77849)
{
	union power_supply_propval value;

	if (is_jig_attached) {
		value.intval = 1;
		psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CHARGE_TYPE, value);
		psy_do_property(battery->pdata->fuelgauge_name, set,
				POWER_SUPPLY_PROP_CHARGE_TYPE, value);
	}
}
#endif
	return current_cable_type;
}

static int batt_handle_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	const char *cmd;
	int cable_type;
	struct sec_battery_extcon_cable *obj =
		container_of(nb, struct sec_battery_extcon_cable, batt_nb);
	enum extcon_cable_name attached_dev = obj->cable_index;
	struct sec_battery_info *battery =
		container_of(nb, struct sec_battery_info,
			extcon_cable_list[attached_dev].batt_nb);

	if (action) {
		cmd = "ATTACH";
		cable_type = sec_bat_cable_check(battery, attached_dev);
	} else {
		cmd = "DETACH";
		cable_type = POWER_SUPPLY_TYPE_BATTERY;
	}

	if (cable_type < 0) {
		dev_info(battery->dev, "%s: ignore event(%d)\n",
			__func__, cable_type);
	} else if (cable_type == POWER_SUPPLY_TYPE_POWER_SHARING) {
		battery->ps_status = true;
		battery->ps_enable = true;
		battery->ps_changed = true;
		battery->wire_status = cable_type;
		dev_info(battery->dev,
			"%s: power sharing cable plugin (%d)\n", __func__, battery->ps_status);
	} else if (cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
		battery->wc_status = true;
	} else {
		battery->wire_status = cable_type;
		if ((battery->wire_status == POWER_SUPPLY_TYPE_BATTERY)
				&& battery->wc_status && !battery->ps_status)
			cable_type = POWER_SUPPLY_TYPE_WIRELESS;
	}
	dev_info(battery->dev,
			"%s: current_cable(%d), wc_status(%d), wire_status(%d)\n",
			__func__, cable_type, battery->wc_status,
			battery->wire_status);

	if ((cable_type >= 0) &&
	    cable_type <= SEC_SIZEOF_POWER_SUPPLY_TYPE) {
		if((cable_type == POWER_SUPPLY_TYPE_BATTERY) && battery->ps_status) {
			battery->ps_status = false;
			dev_info(battery->dev,
				"%s: power sharing cable plugout (%d)\n", __func__, battery->ps_status);
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work(battery->monitor_wqueue, &battery->cable_work, 0);
		} else {
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work(battery->monitor_wqueue,
				   &battery->cable_work, 0);
		}
	}

	pr_info("%s: CMD=%s, attached_dev=%d\n", __func__, cmd, attached_dev);

	return 0;
}
#elif defined(CONFIG_MUIC_NOTIFIER)
static int sec_bat_cable_check(struct sec_battery_info *battery,
				muic_attached_dev_t attached_dev)
{
	int current_cable_type = -1;
	union power_supply_propval val;

	pr_info("[%s]ATTACHED(%d)\n", __func__, attached_dev);

	switch (attached_dev)
	{
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		battery->is_jig_on = true;
		break;
	case ATTACHED_DEV_SMARTDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_BATTERY;
		break;
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_VB_OTG_MUIC:
	case ATTACHED_DEV_HMT_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_OTG;
		break;
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
	case ATTACHED_DEV_SMARTDOCK_USB_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_USB_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_USB;
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_VB_FG_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_UARTOFF;
		break;
	case ATTACHED_DEV_TA_MUIC:
	case ATTACHED_DEV_CARDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_VB_MUIC:
	case ATTACHED_DEV_SMARTDOCK_TA_MUIC:
	case ATTACHED_DEV_AFC_CHARGER_5V_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_TA_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_TA_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_ANY_MUIC:
	case ATTACHED_DEV_QC_CHARGER_5V_MUIC:
	case ATTACHED_DEV_UNSUPPORTED_ID_VB_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_MAINS;
		break;
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_CDP_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_USB_CDP;
		break;
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_LAN_HUB;
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_POWER_SHARING;
		break;
	case ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC:
	case ATTACHED_DEV_QC_CHARGER_PREPARE_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_HV_PREPARE_MAINS;
		break;
	case ATTACHED_DEV_AFC_CHARGER_9V_MUIC:
	case ATTACHED_DEV_QC_CHARGER_9V_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_HV_MAINS;
		break;
	case ATTACHED_DEV_AFC_CHARGER_ERR_V_MUIC:
	case ATTACHED_DEV_QC_CHARGER_ERR_V_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_HV_ERR;
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_MAINS;
		break;
	case ATTACHED_DEV_HV_ID_ERR_UNDEFINED_MUIC:
	case ATTACHED_DEV_HV_ID_ERR_UNSUPPORTED_MUIC:
	case ATTACHED_DEV_HV_ID_ERR_SUPPORTED_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_HV_UNKNOWN;
		break;
	case ATTACHED_DEV_VZW_INCOMPATIBLE_MUIC:
		current_cable_type = POWER_SUPPLY_TYPE_UNKNOWN;
		break;
	default:
		pr_err("%s: invalid type for charger:%d\n",
			__func__, attached_dev);
	}

	if (battery->is_jig_on)
		psy_do_property(battery->pdata->fuelgauge_name, set,
			POWER_SUPPLY_PROP_ENERGY_NOW, val);

	val.intval = battery->is_jig_on;
	psy_do_property(battery->pdata->charger_name, set,
		POWER_SUPPLY_PROP_ENERGY_NOW, val);

	return current_cable_type;

}

static int batt_handle_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	muic_attached_dev_t attached_dev = *(muic_attached_dev_t *)data;
	const char *cmd;
	int cable_type;
	struct sec_battery_info *battery =
		container_of(nb, struct sec_battery_info,
			     batt_nb);
//	union power_supply_propval value;

	battery->is_jig_on = false;

	switch (action) {
	case MUIC_NOTIFY_CMD_DETACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_DETACH:
		cmd = "DETACH";
		cable_type = POWER_SUPPLY_TYPE_BATTERY;
		battery->muic_cable_type = ATTACHED_DEV_NONE_MUIC;
		break;
	case MUIC_NOTIFY_CMD_ATTACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_ATTACH:
		cmd = "ATTACH";
		cable_type = sec_bat_cable_check(battery, attached_dev);
		battery->muic_cable_type = attached_dev;
		break;
	default:
		cmd = "ERROR";
		cable_type = -1;
		battery->muic_cable_type = ATTACHED_DEV_NONE_MUIC;
		break;
	}

	if (attached_dev == ATTACHED_DEV_MHL_MUIC)
		return 0;

	if (cable_type < 0) {
		dev_info(battery->dev, "%s: ignore event(%d)\n",
			__func__, cable_type);
	} else if (cable_type == POWER_SUPPLY_TYPE_POWER_SHARING) {
		battery->ps_status = true;
		battery->ps_enable = true;
		battery->ps_changed = true;

		dev_info(battery->dev,
			"%s: power sharing cable plugin (%d)\n", __func__, battery->ps_status);
	} else if (cable_type == POWER_SUPPLY_TYPE_WIRELESS) {
		battery->wc_status = true;
	} else if ((cable_type == POWER_SUPPLY_TYPE_UNKNOWN) &&
		   (battery->status != POWER_SUPPLY_STATUS_DISCHARGING)) {
		battery->cable_type = cable_type;
		wake_lock(&battery->monitor_wake_lock);
		queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		dev_info(battery->dev,
			"%s: UNKNOWN cable plugin\n", __func__);
		return 0;
	} else {
		battery->wire_status = cable_type;
		if ((battery->wire_status == POWER_SUPPLY_TYPE_BATTERY)
				&& battery->wc_status && !battery->ps_status)
			cable_type = POWER_SUPPLY_TYPE_WIRELESS;
	}
	dev_info(battery->dev,
			"%s: current_cable(%d), wc_status(%d), wire_status(%d)\n",
			__func__, cable_type, battery->wc_status,
			battery->wire_status);
#if 0
	if (attached_dev == ATTACHED_DEV_USB_LANHUB_MUIC) {

		if (!strcmp(cmd, "ATTACH")) {
			value.intval = true;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CHARGE_POWERED_OTG_CONTROL,
					value);
			dev_info(battery->dev,
				"%s: Powered OTG cable attached\n", __func__);
		} else {
			value.intval = false;
			psy_do_property(battery->pdata->charger_name, set,
					POWER_SUPPLY_PROP_CHARGE_POWERED_OTG_CONTROL,
					value);
			dev_info(battery->dev,
				"%s: Powered OTG cable detached\n", __func__);
		}
	}
#endif				// temp
#if defined(CONFIG_AFC_CHARGER_MODE)
	if (!strcmp(cmd, "ATTACH")) {
		if ((battery->muic_cable_type >= ATTACHED_DEV_QC_CHARGER_PREPARE_MUIC) &&
		    (battery->muic_cable_type <= ATTACHED_DEV_QC_CHARGER_9V_MUIC))
			battery->hv_chg_name = "QC";
		else if ((battery->muic_cable_type >= ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC) &&
			 (battery->muic_cable_type <= ATTACHED_DEV_AFC_CHARGER_ERR_V_DUPLI_MUIC))
			battery->hv_chg_name = "AFC";
		else
			battery->hv_chg_name = "NONE";
	} else {
			battery->hv_chg_name = "NONE";
	}

	pr_info("%s : HV_CHARGER_NAME(%s)\n",
		__func__, battery->hv_chg_name);
#endif

	if ((cable_type >= 0) &&
	    cable_type <= SEC_SIZEOF_POWER_SUPPLY_TYPE) {
		if ((cable_type == POWER_SUPPLY_TYPE_POWER_SHARING)
		    || (cable_type == POWER_SUPPLY_TYPE_OTG)) {
			wake_lock(&battery->monitor_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue, &battery->monitor_work, 0);
		} else if((cable_type == POWER_SUPPLY_TYPE_BATTERY)
					&& battery->ps_status) {
			battery->ps_status = false;
			dev_info(battery->dev,
				"%s: power sharing cable plugout (%d)\n", __func__, battery->ps_status);
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
		} else {
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work_on(0, battery->monitor_wqueue,
					   &battery->cable_work, 0);
		}
	}

	pr_info("%s: CMD=%s, attached_dev=%d\n", __func__, cmd, attached_dev);

	return 0;
}

#endif

#if defined(CONFIG_VBUS_NOTIFIER)
static int vbus_handle_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	vbus_status_t vbus_status = *(vbus_status_t *)data;
	struct sec_battery_info *battery =
		container_of(nb, struct sec_battery_info,
			     vbus_nb);
	union power_supply_propval value;
	if (battery->muic_cable_type == ATTACHED_DEV_HMT_MUIC &&
		battery->muic_vbus_status != vbus_status &&
		battery->muic_vbus_status == STATUS_VBUS_HIGH &&
		vbus_status == STATUS_VBUS_LOW) {
		sec_bat_set_charge(battery, false);
		msleep(500);
		value.intval = true;
		psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CHARGE_OTG_CONTROL,
				value);
		dev_info(battery->dev,
			"%s: changed to OTG cable attached\n", __func__);
		battery->wire_status = POWER_SUPPLY_TYPE_OTG;
		wake_lock(&battery->cable_wake_lock);
		queue_delayed_work_on(0, battery->monitor_wqueue, &battery->cable_work, 0);
	}
	pr_info("%s: action=%d, vbus_status=%d\n", __func__, (int)action, vbus_status);
	battery->muic_vbus_status = vbus_status;
	return 0;
}
#endif

#ifdef CONFIG_OF
static int sec_bat_read_u32_index_dt(const struct device_node *np,
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

static int sec_bat_parse_dt(struct device *dev,
		struct sec_battery_info *battery)
{
	struct device_node *np = dev->of_node;
	sec_battery_platform_data_t *pdata = battery->pdata;
	int ret, len;
	unsigned int i;
	const u32 *p;

	if (!np) {
		pr_info("%s: np NULL\n", __func__);
		return 1;
	}

	ret = of_property_read_string(np,
		"battery,vendor", (char const **)&pdata->vendor);
	if (ret)
		pr_info("%s: Vendor is Empty\n", __func__);

#if defined(CONFIG_PM8926_BATTERY_CHECK_INTERRUPT)
        ret = of_property_read_string(np,
                "battery,pmic_name", (char const **)&pdata->pmic_name);
        if (ret)
                pr_info("%s: Vendor is Empty\n", __func__);
#endif

	ret = of_property_read_string(np,
		"battery,charger_name", (char const **)&pdata->charger_name);
	if (ret)
		pr_info("%s: Vendor is Empty\n", __func__);

	ret = of_property_read_string(np,
		"battery,fuelgauge_name", (char const **)&pdata->fuelgauge_name);
	if (ret)
		pr_info("%s: Vendor is Empty\n", __func__);

	ret = of_property_read_string(np,
		"battery,chip_vendor", (char const **)&pdata->chip_vendor);
	if (ret)
		pr_info("%s: Vendor is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,technology",
		&pdata->technology);
	if (ret)
		pr_info("%s: technology is Empty\n", __func__);

	ret = of_get_named_gpio(np, "battery,bat_int", 0);
	if (ret > 0) {
		pdata->bat_irq_gpio = ret;
		pdata->bat_irq = gpio_to_irq(ret);
		pr_info("%s reading bat_int_gpio = %d\n", __func__, ret);
	} else {
		pr_info("%s reading bat_int_gpio is empty\n", __func__);
	}

	ret = of_property_read_u32(np, "battery,bat_irq_attr",
		(unsigned int *)&pdata->bat_irq_attr);
	if (ret)
		pr_info("%s: bat_irq_attr is Empty\n", __func__);

	ret = of_get_named_gpio(np, "battery,ta_int", 0);
	if (ret > 0) {
		pdata->ta_irq_gpio = ret;
		pdata->ta_irq = gpio_to_irq(ret);
		pr_info("%s reading ta_int_gpio = %d\n", __func__, ret);
	}

	ret = of_property_read_u32(np, "battery,ta_irq_attr",
		(unsigned int *)&pdata->ta_irq_attr);
	if (ret)
		pr_info("%s: ta_irq_attr is Empty\n", __func__);

	p = of_get_property(np, "battery,polling_time", &len);

	len = len / sizeof(u32);

	pdata->polling_time = kzalloc(sizeof(*pdata->polling_time) * len, GFP_KERNEL);

	ret = of_property_read_u32_array(np, "battery,polling_time",
					 pdata->polling_time, len);
	if (ret)
		pr_info("%s: polling_time is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,adc_check_count",
		&pdata->adc_check_count);
	if (ret)
		pr_info("%s: adc_check_count is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,temp_adc_type",
		&pdata->temp_adc_type);
	if (ret)
		pr_info("%s: temp_adc_type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,cable_check_type",
		&pdata->cable_check_type);
#if defined(CONFIG_CHARGING_VZWCONCEPT)
	pdata->cable_check_type &= ~SEC_BATTERY_CABLE_CHECK_NOUSBCHARGE;
	pdata->cable_check_type |= SEC_BATTERY_CABLE_CHECK_NOINCOMPATIBLECHARGE;
#endif
	if (ret)
		pr_info("%s: cable_check_type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,cable_source_type",
		&pdata->cable_source_type);
	if (ret)
		pr_info("%s: cable_source_type is Empty\n", __func__);

	pdata->event_check = of_property_read_bool(np,
		"battery,event_check");
	pdata->chg_temp_check = of_property_read_bool(np,
		"battery,chg_temp_check");
	pr_info("%s: chg_temp_check: %d \n", __func__, pdata->chg_temp_check);

	pdata->swelling_mode_skip_in_high_temp = of_property_read_bool(np,
		"battery,swelling_mode_skip_in_high_temp");
	pr_info("%s: swelling_mode_skip: %d \n", __func__, pdata->swelling_mode_skip_in_high_temp);

	if (pdata->chg_temp_check) {
		ret = of_property_read_u32(np, "battery,chg_high_temp_1st",
				&pdata->chg_high_temp_1st);
		if (ret)
			pr_info("%s : chg_high_temp_threshold is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_high_temp_2nd",
				&pdata->chg_high_temp_2nd);
		if (ret)
			pr_info("%s : chg_high_temp_threshold is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_high_temp_recovery",
			&pdata->chg_high_temp_recovery);
		if (ret)
			pr_info("%s : chg_temp_recovery is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_charging_limit_current",
					   &pdata->chg_charging_limit_current);
		if (ret)
			pr_info("%s : chg_charging_limit_current is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_charging_limit_current_2nd",
					   &pdata->chg_charging_limit_current_2nd);
		if (ret)
			pr_info("%s : chg_charging_limit_current_2nd is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_skip_check_time",
					   &pdata->chg_skip_check_time);
		if (ret)
			pr_info("%s : chg_skip_check_time is Empty\n", __func__);

		ret = of_property_read_u32(np, "battery,chg_skip_check_capacity",
					   &pdata->chg_skip_check_capacity);
		if (ret)
			pr_info("%s : chg_skip_check_capacity is Empty\n", __func__);
	}

	ret = of_property_read_u32(np, "battery,event_waiting_time",
		&pdata->event_waiting_time);
	if (ret)
		pr_info("%s : Event waiting time is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,polling_type",
		&pdata->polling_type);
	if (ret)
		pr_info("%s : Polling type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,monitor_initial_count",
		&pdata->monitor_initial_count);
	if (ret)
		pr_info("%s : Monitor initial count is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,battery_check_type",
		&pdata->battery_check_type);
	if (ret)
		pr_info("%s : Battery check type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,check_count",
		&pdata->check_count);
	if (ret)
		pr_info("%s : Check count is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,check_adc_max",
		&pdata->check_adc_max);
	if (ret)
		pr_info("%s : Check adc max is Empty\n", __func__);

        ret = of_property_read_u32(np, "battery,check_adc_min",
                &pdata->check_adc_min);
	if (ret)
		pr_info("%s : Check adc min is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,ovp_uvlo_check_type",
		&pdata->ovp_uvlo_check_type);
	if (ret)
		pr_info("%s : Ovp Uvlo check type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,thermal_source",
		&pdata->thermal_source);
	if (ret)
		pr_info("%s: thermal_source is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,temp_check_type",
		&pdata->temp_check_type);
	if (ret)
		pr_info("%s : Temp check type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,temp_check_count",
		&pdata->temp_check_count);
	if (ret)
		pr_info("%s : Temp check count is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_check_type",
		&pdata->full_check_type);
	if (ret)
		pr_info("%s : Full check type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_check_type_2nd",
		&pdata->full_check_type_2nd);
	if (ret)
		pr_info("%s : Full check type 2nd is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_check_count",
		&pdata->full_check_count);
	if (ret)
		pr_info("%s : Full check count is Empty\n", __func__);

        ret = of_property_read_u32(np, "battery,chg_gpio_full_check",
                &pdata->chg_gpio_full_check);
	if (ret)
		pr_info("%s : Chg gpio full check is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,chg_polarity_full_check",
		&pdata->chg_polarity_full_check);
	if (ret)
		pr_info("%s : Chg polarity full check is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_condition_type",
		&pdata->full_condition_type);
	if (ret)
		pr_info("%s : Full condition type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_condition_soc",
		&pdata->full_condition_soc);
	if (ret)
		pr_info("%s : Full condition soc is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_condition_vcell",
		&pdata->full_condition_vcell);
	if (ret)
		pr_info("%s : Full condition vcell is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,recharge_check_count",
		&pdata->recharge_check_count);
	if (ret)
		pr_info("%s : Recharge check count is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,recharge_condition_type",
		&pdata->recharge_condition_type);
	if (ret)
		pr_info("%s : Recharge condition type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,recharge_condition_soc",
		&pdata->recharge_condition_soc);
	if (ret)
		pr_info("%s : Recharge condition soc is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,recharge_condition_vcell",
		&pdata->recharge_condition_vcell);
	if (ret)
		pr_info("%s : Recharge condition vcell is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,charging_total_time",
		(unsigned int *)&pdata->charging_total_time);
	if (ret)
		pr_info("%s : Charging total time is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,recharging_total_time",
		(unsigned int *)&pdata->recharging_total_time);
	if (ret)
		pr_info("%s : Recharging total time is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,charging_reset_time",
		(unsigned int *)&pdata->charging_reset_time);
	if (ret)
		pr_info("%s : Charging reset time is Empty\n", __func__);

#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	ret = of_property_read_u32(np, "battery,self_discharging_type",
		(unsigned int *)&pdata->self_discharging_type);
	if (ret) {
		pr_info("%s: Self discharging type is Empty, Set default\n", __func__);
		pdata->self_discharging_type = 0;
	}
	
	pdata->factory_discharging = of_get_named_gpio(np, "battery,factory_discharging", 0);
	pr_info("%s : battery,factory_discharging %d \n", __func__,pdata->factory_discharging);
	if (pdata->factory_discharging < 0)
		pdata->factory_discharging = 0;

	pdata->self_discharging_en = of_property_read_bool(np,
							   "battery,self_discharging_en");
	pr_info("%s :battery,self_discharging_en  %d \n", __func__,pdata->self_discharging_en);

	ret = of_property_read_u32(np, "battery,force_discharging_limit",
				   &pdata->force_discharging_limit);
	pr_info("%s : battery,force_discharging_limit %d \n", __func__,pdata->force_discharging_limit);
	if (ret)
		pr_info("%s : Force Discharging limit is Empty", __func__);

	ret = of_property_read_u32(np, "battery,force_discharging_recov",
				   &pdata->force_discharging_recov);
	pr_info("%s :battery,force_discharging_recov  %d \n", __func__,pdata->force_discharging_recov);
	if (ret)
		pr_info("%s : Force Discharging recov is Empty", __func__);

	if (!pdata->self_discharging_type) {

		ret = of_property_read_u32(np, "battery,discharging_adc_min",
					   (unsigned int *)&pdata->discharging_adc_min);
		pr_info("%s : battery,discharging_adc_min %d \n", __func__,pdata->discharging_adc_min);
		if (ret)
			pr_info("%s : Discharging ADC Min is Empty", __func__);

		ret = of_property_read_u32(np, "battery,discharging_adc_max",
					   (unsigned int *)&pdata->discharging_adc_max);;
		pr_info("%s :battery,discharging_adc_max  %d \n", __func__,pdata->discharging_adc_max);
		if (ret)
			pr_info("%s : Discharging ADC Max is Empty", __func__);
	}

	ret = of_property_read_u32(np, "battery,self_discharging_voltage_limit",
				   (unsigned int *)&pdata->self_discharging_voltage_limit);
	pr_info("%s :battery,self_discharging_voltage_limit  %d \n", __func__,pdata->self_discharging_voltage_limit);
	if (ret)
		pr_info("%s : Force Discharging recov is Empty", __func__);

	ret = of_property_read_u32(np, "battery,discharging_ntc_limit",
				   (unsigned int *)&pdata->discharging_ntc_limit);
	pr_info("%s :battery,discharging_ntc_limit  %d \n", __func__,pdata->discharging_ntc_limit);
	if (ret)
		pr_info("%s : Discharging NTC LIMIT is Empty", __func__);
#endif

#if defined(CONFIG_BATTERY_SWELLING)
	ret = of_property_read_u32(np, "battery,swelling_chg_current",
		&pdata->swelling_chg_current);
	if (ret) {
		pr_info("%s: swelling low temp chg current is Empty, Defualt value 1300mA \n", __func__);
		pdata->swelling_chg_current = 0;
	}
	pr_info("%s: swelling_chg_current : %d\n", __func__, pdata->swelling_chg_current);

	ret = of_property_read_u32(np, "battery,swelling_high_chg_current",
		&pdata->swelling_high_chg_current);
	if (ret) {
		pr_info("%s: swelling high temp chg current is Empty, Defualt value 1300mA \n", __func__);
		pdata->swelling_high_chg_current = 0;
	}
	pr_info("%s: swelling_high_chg_current : %d\n", __func__, pdata->swelling_high_chg_current);

	ret = of_property_read_u32(np, "battery,swelling_low_chg_current",
		&pdata->swelling_low_chg_current);
	if (ret) {
		pr_info("%s: swelling low temp chg current is Empty, Defualt value 1300mA \n", __func__);
		pdata->swelling_low_chg_current = 0;
	}
	pr_info("%s: swelling_low_chg_current : %d\n", __func__, pdata->swelling_low_chg_current);

	ret = of_property_read_u32(np, "battery,swelling_full_check_current_2nd",
		&pdata->swelling_full_check_current_2nd);
	if (ret) {
		pr_info("%s: swelling_full_check_current_2nd is Empty\n", __func__);
		pdata->swelling_full_check_current_2nd = 0;
	}
	pr_info("%s: swelling_full_check_current_2nd : %d\n", __func__, pdata->swelling_full_check_current_2nd);

	ret = of_property_read_u32(np, "battery,swelling_drop_float_voltage",
		(unsigned int *)&pdata->swelling_drop_float_voltage);
	if (ret) {
		pr_info("%s: swelling drop float voltage is Empty, Default value 4200mV \n", __func__);
		pdata->swelling_drop_float_voltage = 4200;
	}

	ret = of_property_read_u32(np, "battery,swelling_high_rechg_voltage",
		(unsigned int *)&pdata->swelling_high_rechg_voltage);
	if (ret) {
		pr_info("%s: swelling_high_rechg_voltage is Empty\n", __func__);
		pdata->swelling_high_rechg_voltage = 4150;
	}

	ret = of_property_read_u32(np, "battery,swelling_low_rechg_voltage",
		(unsigned int *)&pdata->swelling_low_rechg_voltage);
	if (ret) {
		pr_info("%s: swelling_low_rechg_voltage is Empty\n", __func__);
		pdata->swelling_low_rechg_voltage = 4050;
	}

	pr_info("%s : SWELLING_CHG_CURRENT(%d) SWELLING_HIGH_CHG_CURRENT(%d)\n"
		"SWELLING_LOW_CHG_CURRENT(%d) SWELLING_FULL_CHECK_CURRENT(%d)\n",
		__func__, pdata->swelling_chg_current, pdata->swelling_high_chg_current,
		pdata->swelling_low_chg_current, pdata->swelling_full_check_current_2nd);
#endif

#if defined(CONFIG_SW_SELF_DISCHARGING)
	ret = of_property_read_u32(np, "battery,self_discharging_temp_block",
			(unsigned int *)&pdata->self_discharging_temp_block);
	if (ret)
		pr_info("%s: self_discharging_temp_block is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,self_discharging_volt_block",
			(unsigned int *)&pdata->self_discharging_volt_block);
	if (ret)
		pr_info("%s: self_discharging_volt_block is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,self_discharging_temp_recov",
			(unsigned int *)&pdata->self_discharging_temp_recov);
	if (ret)
		pr_info("%s: self_discharging_temp_recov is Empty\n", __func__);
	
	ret = of_property_read_u32(np, "battery,self_discharging_temp_pollingtime",
			(unsigned int *)&pdata->self_discharging_temp_pollingtime);
	if (ret)
		pr_info("%s: self_discharging_temp_pollingtime is Empty\n", __func__);
#endif

	np = of_find_node_by_name(NULL, "charger");

	if (!np) {
		pr_info("%s : np NULL\n", __func__);
		return 1;
	}

#if defined(CONFIG_BATTERY_SWELLING)
	ret = of_property_read_u32(np, "battery,chg_float_voltage",
		(unsigned int *)&pdata->swelling_normal_float_voltage);
	if (ret)
		pr_info("%s: chg_float_voltage is Empty\n", __func__);
#endif
	p = of_get_property(np, "battery,input_current_limit", &len);

	len = len / sizeof(u32);

	pdata->charging_current = kzalloc(sizeof(sec_charging_current_t) * len,
					  GFP_KERNEL);

	for(i = 0; i < len; i++) {
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,input_current_limit", i,
				 &pdata->charging_current[i].input_current_limit);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,fast_charging_current", i,
				 &pdata->charging_current[i].fast_charging_current);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,full_check_current_1st", i,
				 &pdata->charging_current[i].full_check_current_1st);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,full_check_current_2nd", i,
				 &pdata->charging_current[i].full_check_current_2nd);
	}

	pr_info("%s: vendor : %s, technology : %d, cable_check_type : %d\n"
		"cable_source_type : %d, event_waiting_time : %d\n"
		"polling_type : %d, initial_count : %d, check_count : %d\n"
		"check_adc_max : %d, check_adc_min : %d\n"
		"ovp_uvlo_check_type : %d, thermal_source : %d\n"
		"temp_check_type : %d, temp_check_count : %d\n",
		__func__,
		pdata->vendor, pdata->technology,pdata->cable_check_type,
		pdata->cable_source_type, pdata->event_waiting_time,
		pdata->polling_type, pdata->monitor_initial_count,
		pdata->check_count, pdata->check_adc_max, pdata->check_adc_min,
		pdata->ovp_uvlo_check_type, pdata->thermal_source,
		pdata->temp_check_type, pdata->temp_check_count);

	return ret;
}
#endif

static int sec_battery_probe(struct platform_device *pdev)
{
	sec_battery_platform_data_t *pdata = NULL;
	struct sec_battery_info *battery;
	int ret = 0;
#if !defined(CONFIG_OF) || defined(CONFIG_EXTCON)
	int i;
#endif

#if defined(CONFIG_TMM_CHG_CTRL)
	tuner_running_status=TUNER_IS_OFF;
#endif

#if defined(CONFIG_AFC_CHARGER_MODE)
	union power_supply_propval value;
#endif
	dev_dbg(&pdev->dev,
		"%s: SEC Battery Driver Loading\n", __func__);

	battery = kzalloc(sizeof(*battery), GFP_KERNEL);
	if (!battery)
		return -ENOMEM;

	if (pdev->dev.of_node) {
		pdata = devm_kzalloc(&pdev->dev,
				sizeof(sec_battery_platform_data_t),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&pdev->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_bat_free;
		}

		battery->pdata = pdata;
		if (sec_bat_parse_dt(&pdev->dev, battery)) {
			dev_err(&pdev->dev,
				"%s: Failed to get battery dt\n", __func__);
			ret = -EINVAL;
			goto err_bat_free;
		}
	} else {
		pdata = dev_get_platdata(&pdev->dev);
		battery->pdata = pdata;
	}

	platform_set_drvdata(pdev, battery);

	battery->dev = &pdev->dev;

	mutex_init(&battery->adclock);
	dev_dbg(battery->dev, "%s: ADC init\n", __func__);

	battery->pdata->temp_highlimit_threshold_event = TEMP_HIGHLIMIT_DEFAULT;
	battery->pdata->temp_highlimit_recovery_event = TEMP_HIGHLIMIT_DEFAULT;
	battery->pdata->temp_highlimit_threshold_normal = TEMP_HIGHLIMIT_DEFAULT;
	battery->pdata->temp_highlimit_recovery_normal =TEMP_HIGHLIMIT_DEFAULT;
	battery->pdata->temp_highlimit_threshold_lpm = TEMP_HIGHLIMIT_DEFAULT;
	battery->pdata->temp_highlimit_recovery_lpm = TEMP_HIGHLIMIT_DEFAULT;
#ifdef CONFIG_OF
	board_battery_init(pdev, battery);
#else
	for (i = 0; i < SEC_BAT_ADC_CHANNEL_NUM; i++)
		adc_init(pdev, pdata, i);
#endif
	wake_lock_init(&battery->monitor_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-monitor");
	wake_lock_init(&battery->cable_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-cable");
	wake_lock_init(&battery->vbus_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-vbus");
	wake_lock_init(&battery->vbus_detect_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-vbus-detect");

#if defined(CONFIG_SW_SELF_DISCHARGING)
	wake_lock_init(&battery->self_discharging_wake_lock, WAKE_LOCK_SUSPEND,
		       "sec-battery-self-discharging");
#endif

	/* initialization of battery info */
	sec_bat_set_charging_status(battery,
			POWER_SUPPLY_STATUS_DISCHARGING);
	battery->health = POWER_SUPPLY_HEALTH_GOOD;
	battery->present = true;
	battery->is_jig_on = false;

	battery->polling_count = 1;	/* initial value = 1 */
	battery->polling_time = pdata->polling_time[
		SEC_BATTERY_POLLING_TIME_DISCHARGING];
	battery->polling_in_sleep = false;
	battery->polling_short = false;
	battery->fuelgauge_in_sleep = false;

	battery->check_count = 0;
	battery->check_adc_count = 0;
	battery->check_adc_value = 0;

	battery->charging_start_time = 0;
	battery->charging_passed_time = 0;
	battery->charging_next_time = 0;
	battery->charging_fullcharged_time = 0;
	battery->siop_level = 100;
	battery->wc_enable = 1;
	battery->stability_test = 0;
	battery->eng_not_full_status = 0;
	battery->chg_limit = false;

	if (battery->pdata->check_batt_id)
		battery->pdata->check_batt_id();

	battery->wc_status = 0;
	battery->ps_status = 0;
	battery->ps_changed = 0;
	battery->ps_enable = 0;
	battery->wire_status = POWER_SUPPLY_TYPE_BATTERY;

#if defined(CONFIG_BATTERY_SWELLING)
	battery->swelling_mode = false;
#endif
	battery->charging_block = false;
#if defined(ANDROID_ALARM_ACTIVATED)
	alarm_init(&battery->event_termination_alarm,
			ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,
			sec_bat_event_expired_timer_func);
#else
	alarm_init(&battery->event_termination_alarm,
			ALARM_BOOTTIME,
			sec_bat_event_expired_timer_func);
#endif
	battery->temp_highlimit_threshold =
		pdata->temp_highlimit_threshold_normal;
	battery->temp_highlimit_recovery =
		pdata->temp_highlimit_recovery_normal;
	battery->temp_high_threshold =
		pdata->temp_high_threshold_normal;
	battery->temp_high_recovery =
		pdata->temp_high_recovery_normal;
	battery->temp_low_recovery =
		pdata->temp_low_recovery_normal;
	battery->temp_low_threshold =
		pdata->temp_low_threshold_normal;

	battery->charging_mode = SEC_BATTERY_CHARGING_NONE;
	battery->is_recharging = false;
	battery->cable_type = POWER_SUPPLY_TYPE_BATTERY;
	battery->test_mode = 0;
	battery->factory_mode = false;
	battery->store_mode = false;
	battery->ignore_store_mode = false;
	battery->slate_mode = false;
	battery->is_hc_usb = false;
	battery->health_change = false;
	battery->skip_chg_temp_check = false;
#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	battery->self_discharging = false;
	battery->force_discharging = false;
#endif
#if defined(CONFIG_SW_SELF_DISCHARGING)
	battery->sw_self_discharging = false;
#endif

	if (battery->pdata->charger_name == NULL)
		battery->pdata->charger_name = "sec-charger";
	if (battery->pdata->fuelgauge_name == NULL)
		battery->pdata->fuelgauge_name = "sec-fuelgauge";

	battery->psy_bat.name = "battery",
	battery->psy_bat.type = POWER_SUPPLY_TYPE_BATTERY,
	battery->psy_bat.properties = sec_battery_props,
	battery->psy_bat.num_properties = ARRAY_SIZE(sec_battery_props),
	battery->psy_bat.get_property = sec_bat_get_property,
	battery->psy_bat.set_property = sec_bat_set_property,
#if defined(CONFIG_QPNP_BMS)
	battery->psy_bat.supplied_to = pm_batt_supplied_to;
	battery->psy_bat.num_supplicants =
				ARRAY_SIZE(pm_batt_supplied_to);
#endif
	battery->psy_usb.name = "usb",
	battery->psy_usb.type = POWER_SUPPLY_TYPE_USB,
	battery->psy_usb.supplied_to = supply_list,
	battery->psy_usb.num_supplicants = ARRAY_SIZE(supply_list),
	battery->psy_usb.properties = sec_power_props,
	battery->psy_usb.num_properties = ARRAY_SIZE(sec_power_props),
	battery->psy_usb.get_property = sec_usb_get_property,
	battery->psy_ac.name = "ac",
	battery->psy_ac.type = POWER_SUPPLY_TYPE_MAINS,
	battery->psy_ac.supplied_to = supply_list,
	battery->psy_ac.num_supplicants = ARRAY_SIZE(supply_list),
	battery->psy_ac.properties = sec_power_props,
	battery->psy_ac.num_properties = ARRAY_SIZE(sec_power_props),
	battery->psy_ac.get_property = sec_ac_get_property;
	battery->psy_wireless.name = "wireless",
	battery->psy_wireless.type = POWER_SUPPLY_TYPE_WIRELESS,
	battery->psy_wireless.supplied_to = supply_list,
	battery->psy_wireless.num_supplicants = ARRAY_SIZE(supply_list),
	battery->psy_wireless.properties = sec_power_props,
	battery->psy_wireless.num_properties = ARRAY_SIZE(sec_power_props),
	battery->psy_wireless.get_property = sec_wireless_get_property;
	battery->psy_wireless.set_property = sec_wireless_set_property;
	battery->psy_ps.name = "ps",
	battery->psy_ps.type = POWER_SUPPLY_TYPE_POWER_SHARING,
	battery->psy_ps.supplied_to = supply_list,
	battery->psy_ps.num_supplicants = ARRAY_SIZE(supply_list),
	battery->psy_ps.properties = sec_ps_props,
	battery->psy_ps.num_properties = ARRAY_SIZE(sec_ps_props),
	battery->psy_ps.get_property = sec_ps_get_property;
	battery->psy_ps.set_property = sec_ps_set_property;

#if defined (CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	if (battery->pdata->factory_discharging) {
		ret = gpio_request(battery->pdata->factory_discharging, "FACTORY_DISCHARGING");
		if (ret) {
			pr_err("failed to request GPIO %u\n", battery->pdata->factory_discharging);
			goto err_bat_free;
		}
	}
#endif

#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	g_battery = battery;
#endif

	/* create work queue */
	battery->monitor_wqueue =
	    create_singlethread_workqueue(dev_name(&pdev->dev));
	if (!battery->monitor_wqueue) {
		dev_err(battery->dev,
			"%s: Fail to Create Workqueue\n", __func__);
		goto err_wake_lock;
	}

	INIT_DELAYED_WORK(&battery->monitor_work, sec_bat_monitor_work);
	INIT_DELAYED_WORK(&battery->cable_work, sec_bat_cable_work);
	INIT_DELAYED_WORK(&battery->vbus_detect_work, sec_bat_vbus_detect_work);

	switch (pdata->polling_type) {
	case SEC_BATTERY_MONITOR_WORKQUEUE:
		INIT_DELAYED_WORK(&battery->polling_work,
			sec_bat_polling_work);
		break;
	case SEC_BATTERY_MONITOR_ALARM:
#if defined(ANDROID_ALARM_ACTIVATED)
		battery->last_poll_time = alarm_get_elapsed_realtime();
		alarm_init(&battery->polling_alarm,
			ANDROID_ALARM_ELAPSED_REALTIME_WAKEUP,
			sec_bat_alarm);
#else
		battery->last_poll_time = ktime_get_boottime();
		alarm_init(&battery->polling_alarm, ALARM_BOOTTIME,
			sec_bat_alarm);
#endif
		break;

	default:
		break;
	}

	sec_bat_get_battery_info(battery);

	/* init power supplier framework */
	ret = power_supply_register(&pdev->dev, &battery->psy_ps);
	if (ret) {
		dev_err(battery->dev,
			"%s: Failed to Register psy_ps\n", __func__);
		goto err_workqueue;
	}

	ret = power_supply_register(&pdev->dev, &battery->psy_wireless);
	if (ret) {
		dev_err(battery->dev,
			"%s: Failed to Register psy_wireless\n", __func__);
		goto err_supply_unreg_ps;
	}

	ret = power_supply_register(&pdev->dev, &battery->psy_usb);
	if (ret) {
		dev_err(battery->dev,
			"%s: Failed to Register psy_usb\n", __func__);
		goto err_supply_unreg_wireless;
	}

	ret = power_supply_register(&pdev->dev, &battery->psy_ac);
	if (ret) {
		dev_err(battery->dev,
			"%s: Failed to Register psy_ac\n", __func__);
		goto err_supply_unreg_usb;
	}

	ret = power_supply_register(&pdev->dev, &battery->psy_bat);
	if (ret) {
		dev_err(battery->dev,
			"%s: Failed to Register psy_bat\n", __func__);
		goto err_supply_unreg_ac;
	}

	if (battery->pdata->bat_gpio_init && !battery->pdata->bat_gpio_init()) {
		dev_err(battery->dev,
			"%s: Failed to Initialize GPIO\n", __func__);
		goto err_supply_unreg_bat;
	}

	if (battery->pdata->bat_irq) {
		ret = request_threaded_irq(battery->pdata->bat_irq,
				NULL, sec_bat_irq_thread,
				battery->pdata->bat_irq_attr
				| IRQF_ONESHOT,
				"battery-irq", battery);
		if (ret) {
			dev_err(battery->dev,
				"%s: Failed to Request IRQ (bat_int)\n", __func__);
			goto err_supply_unreg_bat;
		}
		if (battery->pdata->bat_irq_gpio) {
			ret = enable_irq_wake(battery->pdata->bat_irq);
			if (ret < 0)
				dev_err(battery->dev,
					"%s: Failed to Enable Wakeup Source(%d)(bat_int)\n",
					__func__, ret);
		}
	}

	if (battery->pdata->ta_irq) {
		ret = request_threaded_irq(battery->pdata->ta_irq,
				NULL, sec_ta_irq_thread,
				battery->pdata->ta_irq_attr
				| IRQF_ONESHOT,
				"ta-irq", battery);
		if (ret) {
			dev_err(battery->dev,
				"%s: Failed to Request IRQ (ta_int)\n", __func__);
			goto err_req_irq;
		}
		if (battery->pdata->ta_irq_gpio) {
			ret = enable_irq_wake(battery->pdata->ta_irq);
			if (ret < 0)
				dev_err(battery->dev,
					"%s: Failed to Enable Wakeup Source(%d)(ta_int)\n",
					__func__, ret);
		}
	}

	ret = sec_bat_create_attrs(battery->psy_bat.dev);
	if (ret) {
		dev_err(battery->dev,
			"%s : Failed to create_attrs\n", __func__);
		goto err_req_ta_irq;
	}

#if defined(CONFIG_EXTCON)
	for (i=0; i < EXTCON_NONE; i++) {
		battery->extcon_cable_list[i].batt_nb.notifier_call = batt_handle_notification;
		battery->extcon_cable_list[i].cable_index = i;
		ret = extcon_register_interest(&battery->extcon_cable_list[i].extcon_nb,
					EXTCON_DEV_NAME,
					extcon_cable_name[i],
					&battery->extcon_cable_list[i].batt_nb);

		if (ret)
			pr_err("%s: fail to register extcon notifier(%s, %d)\n",
				__func__, extcon_cable_name[i], ret);
		else {
			if(extcon_get_cable_state_(battery->extcon_cable_list[i].extcon_nb.edev,i)) {
				battery->wire_status = sec_bat_cable_check(battery, i);
				pr_info("%s: %s(wire_status = %d) attached from extcon\n", __func__,
						extcon_cable_name[i], battery->wire_status);
			}
		}
	}

	if ((battery->wire_status > POWER_SUPPLY_TYPE_BATTERY) &&
		(battery->wire_status < SEC_SIZEOF_POWER_SUPPLY_TYPE) &&
		(battery->pdata->cable_source_type &
		SEC_BATTERY_CABLE_SOURCE_EXTERNAL)) {
			wake_lock(&battery->cable_wake_lock);
				queue_delayed_work(battery->monitor_wqueue,
					&battery->cable_work, 0);
	} else {
		union power_supply_propval value;
		psy_do_property(battery->pdata->charger_name, get,
			POWER_SUPPLY_PROP_ONLINE, value);
		if (value.intval == POWER_SUPPLY_TYPE_WIRELESS) {
			battery->wc_status = 1;
			wake_lock(&battery->cable_wake_lock);
			queue_delayed_work(battery->monitor_wqueue,
				&battery->cable_work, 0);
		}
	}
#elif defined(CONFIG_MUIC_NOTIFIER)
	muic_notifier_register(&battery->batt_nb,
	batt_handle_notification,
	MUIC_NOTIFY_DEV_CHARGER);
#else
	cable_initial_check(battery);
#endif

#if defined(CONFIG_VBUS_NOTIFIER)
	vbus_notifier_register(&battery->vbus_nb,
				vbus_handle_notification,
				VBUS_NOTIFY_DEV_CHARGER);
#endif
#if defined(CONFIG_AFC_CHARGER_MODE)
	value.intval = 1;
	psy_do_property(battery->pdata->charger_name, set,
			POWER_SUPPLY_PROP_AFC_CHARGER_MODE,
			value);
#endif
#if defined(CONFIG_STORE_MODE) && !defined(CONFIG_SEC_FACTORY)
	battery->store_mode = true;
	value.intval = 0;
	psy_do_property(battery->pdata->fuelgauge_name, get,
		POWER_SUPPLY_PROP_CAPACITY, value);
	if (value.intval <= 5) {
		battery->ignore_store_mode = true;
	} else {	
		value.intval = battery->store_mode;
		psy_do_property(battery->pdata->charger_name, set,
				POWER_SUPPLY_PROP_CONSTANT_CHARGE_CURRENT_MAX, value);
	}
#endif
	wake_lock(&battery->monitor_wake_lock);
	queue_delayed_work(battery->monitor_wqueue, &battery->monitor_work, 0);

	sec_bat_check_cable_result_callback(battery->dev, POWER_SUPPLY_TYPE_MAINS);
	battery->present = sec_bat_check(battery);
	sec_bat_check_cable_result_callback(battery->dev, battery->cable_type);

	dev_info(battery->dev,
		"%s: SEC Battery Driver Loaded\n", __func__);
#ifdef CONFIG_SAMSUNG_BATTERY_FACTORY
	/* do not sleep in lpm mode & factory mode */
	if (sec_bat_is_lpm(battery)) {
		wake_lock_init(&battery->lpm_wake_lock, WAKE_LOCK_SUSPEND,
				"sec-lpm-monitor");
		wake_lock(&battery->lpm_wake_lock);
	}
#endif
	return 0;


err_req_ta_irq:
	if (battery->pdata->ta_irq)
		free_irq(battery->pdata->ta_irq, battery);
err_req_irq:
	if (battery->pdata->bat_irq)
		free_irq(battery->pdata->bat_irq, battery);
err_supply_unreg_bat:
	power_supply_unregister(&battery->psy_bat);
err_supply_unreg_ac:
	power_supply_unregister(&battery->psy_ac);
err_supply_unreg_usb:
	power_supply_unregister(&battery->psy_usb);
err_supply_unreg_wireless:
	power_supply_unregister(&battery->psy_wireless);
err_supply_unreg_ps:
	power_supply_unregister(&battery->psy_ps);
err_workqueue:
	destroy_workqueue(battery->monitor_wqueue);
err_wake_lock:
	wake_lock_destroy(&battery->monitor_wake_lock);
	wake_lock_destroy(&battery->cable_wake_lock);
	wake_lock_destroy(&battery->vbus_wake_lock);
	wake_lock_destroy(&battery->vbus_detect_wake_lock);
#if defined(CONFIG_SW_SELF_DISCHARGING)
	wake_lock_destroy(&battery->self_discharging_wake_lock);
#endif	
	mutex_destroy(&battery->adclock);
	kfree(pdata);
err_bat_free:
	kfree(battery);

	return ret;
}

static int sec_battery_remove(struct platform_device *pdev)
{
	struct sec_battery_info *battery = platform_get_drvdata(pdev);
#ifndef CONFIG_OF
	int i;
#endif

	dev_dbg(battery->dev, "%s: Start\n", __func__);

	switch (battery->pdata->polling_type) {
	case SEC_BATTERY_MONITOR_WORKQUEUE:
		cancel_delayed_work(&battery->polling_work);
		break;
	case SEC_BATTERY_MONITOR_ALARM:
		alarm_cancel(&battery->polling_alarm);
		break;
	default:
		break;
	}

	alarm_cancel(&battery->event_termination_alarm);
	flush_workqueue(battery->monitor_wqueue);
	destroy_workqueue(battery->monitor_wqueue);
	wake_lock_destroy(&battery->monitor_wake_lock);
	wake_lock_destroy(&battery->cable_wake_lock);
	wake_lock_destroy(&battery->vbus_wake_lock);
	wake_lock_destroy(&battery->vbus_detect_wake_lock);

#if defined(CONFIG_SW_SELF_DISCHARGING)
	wake_lock_destroy(&battery->self_discharging_wake_lock);
#endif

	mutex_destroy(&battery->adclock);
#ifdef CONFIG_OF
	adc_exit(battery);
#else
	for (i = 0; i < SEC_BAT_ADC_CHANNEL_NUM; i++)
		adc_exit(battery->pdata, i);
#endif
	power_supply_unregister(&battery->psy_ps);
	power_supply_unregister(&battery->psy_wireless);
	power_supply_unregister(&battery->psy_ac);
	power_supply_unregister(&battery->psy_usb);
	power_supply_unregister(&battery->psy_bat);

	dev_dbg(battery->dev, "%s: End\n", __func__);
	kfree(battery);

	return 0;
}

static int sec_battery_prepare(struct device *dev)
{
	struct sec_battery_info *battery
		= dev_get_drvdata(dev);

	dev_dbg(battery->dev, "%s: Start\n", __func__);

	switch (battery->pdata->polling_type) {
	case SEC_BATTERY_MONITOR_WORKQUEUE:
		cancel_delayed_work(&battery->polling_work);
		break;
	case SEC_BATTERY_MONITOR_ALARM:
		alarm_cancel(&battery->polling_alarm);
		break;
	default:
		break;
	}
	cancel_delayed_work_sync(&battery->monitor_work);

	battery->polling_in_sleep = true;

	sec_bat_set_polling(battery);

	/* cancel work for polling
	 * that is set in sec_bat_set_polling()
	 * no need for polling in sleep
	 */
	if (battery->pdata->polling_type ==
		SEC_BATTERY_MONITOR_WORKQUEUE)
		cancel_delayed_work(&battery->polling_work);

	dev_dbg(battery->dev, "%s: End\n", __func__);

	return 0;
}

static int sec_battery_suspend(struct device *dev)
{
	return 0;
}

static int sec_battery_resume(struct device *dev)
{
	return 0;
}

static void sec_battery_complete(struct device *dev)
{
	struct sec_battery_info *battery
		= dev_get_drvdata(dev);

	dev_dbg(battery->dev, "%s: Start\n", __func__);

	/* cancel current alarm and reset after monitor work */
	if (battery->pdata->polling_type == SEC_BATTERY_MONITOR_ALARM)
		alarm_cancel(&battery->polling_alarm);

	wake_lock(&battery->monitor_wake_lock);
	queue_delayed_work_on(0, battery->monitor_wqueue,
		&battery->monitor_work, 0);

	dev_dbg(battery->dev, "%s: End\n", __func__);

	return;
}

static void sec_battery_shutdown(struct device *dev)
{
        struct sec_battery_info *battery
                = dev_get_drvdata(dev);

#if defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	pr_info("%s : FORCE DISCHARGING(%d)\n", __func__, battery->force_discharging);
	if (battery->force_discharging) {
		if (battery->pdata->factory_discharging)
			gpio_direction_output(battery->pdata->factory_discharging, 0);
	}
#endif

	switch (battery->pdata->polling_type) {
	case SEC_BATTERY_MONITOR_WORKQUEUE:
		cancel_delayed_work(&battery->polling_work);
		break;
	case SEC_BATTERY_MONITOR_ALARM:
		alarm_cancel(&battery->polling_alarm);
		break;
	default:
		break;
	}

	return;
}

#ifdef CONFIG_OF
static struct of_device_id sec_battery_dt_ids[] = {
	{ .compatible = "samsung,sec-battery" },
	{ }
};
MODULE_DEVICE_TABLE(of, sec_battery_dt_ids);
#endif /* CONFIG_OF */

static const struct dev_pm_ops sec_battery_pm_ops = {
	.prepare = sec_battery_prepare,
	.suspend = sec_battery_suspend,
	.resume = sec_battery_resume,
	.complete = sec_battery_complete,
};

static struct platform_driver sec_battery_driver = {
	.driver = {
		   .name = "sec-battery",
		   .owner = THIS_MODULE,
		   .pm = &sec_battery_pm_ops,
		   .shutdown = sec_battery_shutdown,
#ifdef CONFIG_OF
		.of_match_table = sec_battery_dt_ids,
#endif
		   },
	.probe = sec_battery_probe,
	.remove = sec_battery_remove,
};

static int __init sec_battery_init(void)
{
	return platform_driver_register(&sec_battery_driver);
}

static void __exit sec_battery_exit(void)
{
	platform_driver_unregister(&sec_battery_driver);
}

late_initcall(sec_battery_init);
module_exit(sec_battery_exit);

MODULE_DESCRIPTION("Samsung Battery Driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
