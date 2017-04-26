/* drivers/battery/rt5033_charger.c
 * RT5033 Charger Driver
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/battery/sec_charger.h>
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/usb_notify.h>
#endif

#include <linux/mfd/rt5033.h>

#ifdef CONFIG_FLED_RT5033
#include <linux/leds/rt5033_fled.h>
#include <linux/leds/rtfled.h>
#include <linux/mfd/rtfled.h>
#include <linux/mfd/rt5033_fled.h>
#endif /* CONFIG_FLED_RT5033 */
#include <linux/version.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/of_gpio.h>

#if defined(CONFIG_SEC_E5_PROJECT)
#define ENABLE_MIVR 0
#else
#define ENABLE_MIVR 1
#endif

#define EN_BAD_ADP_IRQ	1
#define EN_OVP_IRQ 1
#define EN_IEOC_IRQ 1
#define EN_RECHG_REQ_IRQ 0
#define EN_TR_IRQ 0
#define EN_MIVR_SW_REGULATION 0
#define EN_BST_IRQ 0
#define MINVAL(a, b) ((a <= b) ? a : b)

#define EOC_DEBOUNCE_CNT 2
#define HEALTH_DEBOUNCE_CNT 3
#define DEFAULT_CHARGING_CURRENT 500

#define EOC_SLEEP 200
/* Timeout setting should be larger than wakelock setting
 * to let CPU have enough time to go sleeping */
#define EOC_TIMEOUT (10 * 1000)
/* 9.9 second */
#define BADADP_RESTART_TIMER	(9900)
#ifndef EN_TEST_READ
#define EN_TEST_READ 1
#endif

static int rt5033_reg_map[] = {
	RT5033_CHG_STAT_CTRL,
	RT5033_CHG_CTRL1,
	RT5033_CHG_CTRL2,
	RT5033_CHG_CTRL3,
	RT5033_CHG_CTRL4,
	RT5033_CHG_CTRL5,
	RT5033_CHG_RESET,
	RT5033_UUG,
};

typedef struct rt5033_charger_data {
	rt5033_mfd_chip_t *rt5033;
	struct power_supply psy_chg;
	rt5033_charger_platform_data_t *pdata;
	int eoc_cnt;
	int charging_current;
	int siop_level;
	int cable_type;
	bool is_charging;
	struct mutex io_lock;
	/* register programming */
	int reg_addr;
	int reg_data;
	int rev_id;
	bool full_charged;
	bool ovp;
	int unhealth_cnt;
	struct workqueue_struct *wq;
	struct delayed_work eoc_timeout_work;
#if EN_BAD_ADP_IRQ
	struct delayed_work activate_bad_adp_work;
	int bad_adp_irq;
#endif /* EN_BAD_ADP_IRQ */
	int status;
#ifdef CONFIG_FLED_RT5033
	struct rt_fled_info *fled_info;
#endif /* CONFIG_FLED_RT5033 */
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	bool is_aicr_changed;
	int	aicr;
#endif
} rt5033_charger_data_t;

static enum power_supply_property sec_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_NOW,
#if defined(CONFIG_BATTERY_SWELLING)
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
#endif
};

static void __rt5033_set_fast_charging_current(struct rt5033_charger_data *charger,
		int charging_current);
static int rt5033_charger_parse_dualized_dt(struct device *dev,
                           struct rt5033_charger_platform_data *pdata);

static int rt5033_get_charging_health(
		struct rt5033_charger_data *charger);
static void rt5033_read_regs(struct i2c_client *i2c, char *str)
{
	u8 data = 0;
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(rt5033_reg_map); i++) {
		data = rt5033_reg_read(i2c, rt5033_reg_map[i]);
		sprintf(str+strlen(str), "0x%02x, ", data);
	}
}

static void rt5033_test_read(struct i2c_client *i2c)
{
	int data;
	char str[1000] = {0,};
	int i;

	for (i = 0x0; i <= 0x8; i++) { /* RT5033 REG: 0x00 ~ 0x08 */
		data = rt5033_reg_read(i2c, i);
		sprintf(str+strlen(str), "0x%x = 0x%02x, ", i, data);
	}
	sprintf(str+strlen(str), "#");
	sprintf(str+strlen(str), "0x18 = 0x%02x, ", rt5033_reg_read(i2c, 0x18));
	sprintf(str+strlen(str), "0x19 = 0x%02x, ", rt5033_reg_read(i2c, 0x19));
	sprintf(str+strlen(str), "0x1A = 0x%02x, ", rt5033_reg_read(i2c, 0x1A));
	sprintf(str+strlen(str), "0x22 = 0x%02x, ", rt5033_reg_read(i2c, 0x22));
	sprintf(str+strlen(str), "#");
	pr_info("%s: %s\n", __func__, str);

	/* for charging fail issue */
	data = rt5033_reg_read(i2c, RT5033_UUG);
	if ((data & (~0x2)) != 0x45) {
		pr_info("%s: RT5033_UUG is incorrect, 0x19 = 0x%x \n", __func__, data);
		rt5033_assign_bits(i2c, RT5033_UUG, ~0x02, 0x45);
	}

	/* for leakage current issue */
	data = rt5033_reg_read(i2c, RT5033_CHG_STAT_CTRL);
	if ((data & 0x82) != 0x02) {
		pr_info("%s: SW_HW_CTRL is incorrect (0x%x)\n", __func__, data);
		rt5033_assign_bits(i2c, RT5033_CHG_STAT_CTRL, 0x82, 0x02);
	}
}

static void rt5033_charger_otg_control(struct rt5033_charger_data *charger,
				       bool enable)
{
	struct i2c_client *i2c = charger->rt5033->i2c_client;

	pr_info("%s: called charger otg control : %s\n", __func__,
		enable ? "on" : "off");

	if (!enable) {
		/* turn off OTG */
		rt5033_clr_bits(i2c, RT5033_CHG_CTRL1, RT5033_OPAMODE_MASK);
#ifdef CONFIG_FLED_RT5033
		if (charger->fled_info == NULL)
			charger->fled_info = rt_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			rt5033_boost_notification(charger->fled_info, 0);
#endif /* CONFIG_FLED_RT5033 */
	} else {
		/* Set OTG boost vout = 5V, turn on OTG */
		rt5033_assign_bits(charger->rt5033->i2c_client,
				   RT5033_CHG_CTRL2, RT5033_VOREG_MASK,
				   0x37 << RT5033_VOREG_SHIFT);
		rt5033_set_bits(charger->rt5033->i2c_client,
				RT5033_CHG_CTRL1, RT5033_OPAMODE_MASK);
		rt5033_clr_bits(charger->rt5033->i2c_client,
				RT5033_CHG_CTRL1, RT5033_HZ_MASK);
		charger->cable_type = POWER_SUPPLY_TYPE_OTG;
#ifdef CONFIG_FLED_RT5033
		if (charger->fled_info == NULL)
			charger->fled_info = rt_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			rt5033_boost_notification(charger->fled_info, 1);
#endif /* CONFIG_FLED_RT5033 */
	}
}

#define RT5033_REGULATOR_REG_OUTPUT_EN (0x41)
#define RT5033_REGULATOR_EN_MASK_LDO_SAFE (1<<6)
/* this function will work well on CHIP_REV = 3 or later */
static void rt5033_enable_charger_switch(struct rt5033_charger_data *charger,
		int onoff)
{
	int prev_charging_status;
	struct i2c_client *iic;
	union power_supply_propval val;

	mutex_lock(&charger->io_lock);
	prev_charging_status = charger->is_charging;
	iic = charger->rt5033->i2c_client;

	pr_info("%s rev_id = %d \n",__func__, charger->rev_id);
	charger->is_charging = onoff ? true : false;
	if ((onoff > 0) && (prev_charging_status == false)) {
		charger->full_charged = false;
		pr_info("%s: turn on charger\n", __func__);

#ifdef CONFIG_FLED_RT5033
		if (charger->fled_info == NULL)
			charger->fled_info = rt_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			rt5033_charger_notification(charger->fled_info, 1);
#endif /* CONFIG_FLED_RT5033 */
		rt5033_clr_bits(iic, RT5033_CHG_CTRL1, RT5033_HZ_MASK);
		rt5033_set_bits(iic, RT5033_CHG_CTRL3, RT5033_COF_EN_MASK);
		/* Disable Timer function (Charging timeout fault) */
		rt5033_clr_bits(iic, RT5033_CHG_CTRL3, RT5033_TIMEREN_MASK);

		/* Reset EOC loop, and make it re-detect */
		charger->eoc_cnt = 0;
		rt5033_clr_bits(iic, RT5033_CHG_STAT_CTRL, RT5033_CHGENB_MASK);
		rt5033_set_bits(iic, RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
		rt5033_clr_bits(iic, RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
	} else if (onoff == 0) {
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_STATUS, val);
		if (val.intval != POWER_SUPPLY_STATUS_FULL)
			charger->full_charged = false;
		pr_info("%s: turn off charger\n", __func__);
		charger->charging_current = DEFAULT_CHARGING_CURRENT;
		charger->eoc_cnt = 0;
		/* Disable EOC */
		rt5033_clr_bits(iic, RT5033_CHG_CTRL4, RT5033_IEOC_MASK);
#ifdef CONFIG_FLED_RT5033
		if (charger->fled_info == NULL)
			charger->fled_info = rt_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			rt5033_charger_notification(charger->fled_info, 0);
#endif /* CONFIG_FLED_RT5033 */

		if ((val.intval == POWER_SUPPLY_STATUS_NOT_CHARGING) || (val.intval == POWER_SUPPLY_STATUS_FULL) || (val.intval == POWER_SUPPLY_STATUS_CHARGING))
			rt5033_set_bits(iic, RT5033_CHG_STAT_CTRL, RT5033_CHGENB_MASK);
		else {
			rt5033_clr_bits(iic, RT5033_CHG_STAT_CTRL, RT5033_CHGENB_MASK);
			__rt5033_set_fast_charging_current(charger, 500);
		}
	} else {
	    pr_info("%s: repeated to set charger switch(%d), prev stat = %d\n",
             __func__, onoff, prev_charging_status ? 1 : 0);
	}
	mutex_unlock(&charger->io_lock);
}

static void rt5033_enable_charging_termination(struct i2c_client *i2c,
		int onoff)
{
	pr_info("%s:[BATT] Do termination set(%d)\n", __func__, onoff);
	if (onoff)
		rt5033_set_bits(i2c, RT5033_CHG_CTRL1, RT5033_TEEN_MASK);
	else
		rt5033_clr_bits(i2c, RT5033_CHG_CTRL1, RT5033_TEEN_MASK);
}

static int rt5033_input_current_limit[] = {
	100,
	500,
	700,
	900,
	1000,
	1500,
	2000,
};

static int __r5033_current_limit_to_setting(int current_limit)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(rt5033_input_current_limit); i++) {
		if (current_limit <= rt5033_input_current_limit[i])
			return i+1;
	}

	return -EINVAL;
}

#if defined(CONFIG_MACH_FORTUNA_SPR) || defined(CONFIG_MACH_FORTUNA_TMO) || defined(CONFIG_MACH_FORTUNA_ACG)
extern int poweroff_charging;
#endif

static void rt5033_set_input_current_limit(struct rt5033_charger_data *charger,
		int current_limit)
{
	struct i2c_client *i2c = charger->rt5033->i2c_client;
	int data = __r5033_current_limit_to_setting(current_limit);

	if (data < 0)
		data = 0;
	mutex_lock(&charger->io_lock);

#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	charger->aicr = current_limit;
#endif
	if (charger->pdata->dualized_charging_current && charger->rev_id < 6){
		if(charger->cable_type == POWER_SUPPLY_TYPE_USB){
			rt5033_assign_bits(i2c, RT5033_CHG_CTRL1, RT5033_AICR_LIMIT_MASK,
				(data) << RT5033_AICR_LIMIT_SHIFT);
		} else {
			/* disable AICR */
			pr_info("%s: AICR Disabled", __func__);
			rt5033_clr_bits(i2c, RT5033_CHG_CTRL1, RT5033_AICR_LIMIT_MASK);
		}
	}
	else{
#if defined(CONFIG_MACH_FORTUNA_SPR) || defined(CONFIG_MACH_FORTUNA_TMO) || defined(CONFIG_MACH_FORTUNA_ACG)
		/*Soft Start Charging*/
		if((charger->cable_type != POWER_SUPPLY_TYPE_BATTERY) && !poweroff_charging){
			rt5033_assign_bits(i2c, RT5033_CHG_CTRL1, RT5033_AICR_LIMIT_MASK,
				(1) << RT5033_AICR_LIMIT_SHIFT);
			pr_info("%s: Soft Start Charging\n", __func__);
			msleep(100);
		}
#endif
		rt5033_assign_bits(i2c, RT5033_CHG_CTRL1, RT5033_AICR_LIMIT_MASK,
		(data) << RT5033_AICR_LIMIT_SHIFT);
		pr_info("%s: AICR Enabled", __func__);
	}

	mutex_unlock(&charger->io_lock);
}

static int rt5033_get_input_current_limit(struct i2c_client *i2c)
{
    int ret;
    ret = rt5033_reg_read(i2c, RT5033_CHG_CTRL1);
    if (ret < 0)
        return ret;
    ret&=RT5033_AICR_LIMIT_MASK;
    ret>>=RT5033_AICR_LIMIT_SHIFT;
    if (ret == 0)
        return 2000 + 1; // no limitation
    return rt5033_input_current_limit[ret - 1];
}

static void rt5033_set_regulation_voltage(struct rt5033_charger_data *charger,
		int float_voltage)
{
    struct i2c_client *i2c = charger->rt5033->i2c_client;
	int data;

	if (float_voltage < 3650)
		data = 0;
	else if (float_voltage >= 3650 && float_voltage <= 4400)
		data = (float_voltage - 3650) / 25;
	else
		data = 0x3f;
    mutex_lock(&charger->io_lock);
	rt5033_assign_bits(i2c, RT5033_CHG_CTRL2, RT5033_VOREG_MASK,
			data << RT5033_VOREG_SHIFT);
    mutex_unlock(&charger->io_lock);
}

#if defined(CONFIG_BATTERY_SWELLING)
static int rt5033_get_regulation_voltage(struct rt5033_charger_data *charger)
{
	int data;
	data = rt5033_reg_read(charger->rt5033->i2c_client, RT5033_CHG_CTRL2);
	if (data < 0) {
		pr_info("%s: warning --> fail to read i2c register(%d)\n", __func__, data);
		return data;
	}
	data &= RT5033_VOREG_MASK;
	pr_info("%s: battery cv voltage 0x%x\n", __func__, data);

	return data;
}
#endif

static void __rt5033_set_fast_charging_current(struct rt5033_charger_data *charger,
		int charging_current)
{
	int data;

	if (charging_current <= 0) {
		rt5033_set_bits(charger->rt5033->i2c_client, RT5033_CHG_STAT_CTRL, RT5033_CHGENB_MASK);
		charger->is_charging = false;
	} else if (charging_current < 700)
		data = 0;
	else if (charging_current >= 700 && charging_current <= 2000)
		data = (charging_current-700)/100;
	else
		data = 0xd;

	rt5033_assign_bits(charger->rt5033->i2c_client, RT5033_CHG_CTRL5, RT5033_ICHRG_MASK,
			data << RT5033_ICHRG_SHIFT);
}

static int rt5033_eoc_level[] = {
	0,
	150,
	200,
	250,
	300,
	400,
	500,
	600,
};

static int rt5033_get_eoc_level(int eoc_current)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(rt5033_eoc_level); i++) {
		if (eoc_current < rt5033_eoc_level[i]) {
			if (i == 0)
				return 0;
			return i - 1;
		}
	}
	return ARRAY_SIZE(rt5033_eoc_level) - 1;
}

static int rt5033_get_current_eoc_setting(struct rt5033_charger_data *charger)
{
	int ret;
	mutex_lock(&charger->io_lock);
	ret = rt5033_reg_read(charger->rt5033->i2c_client, RT5033_CHG_CTRL4);
	mutex_unlock(&charger->io_lock);
	if (ret < 0) {
		pr_info("%s: warning --> fail to read i2c register(%d)\n", __func__, ret);
		return ret;
	}
	return rt5033_eoc_level[(RT5033_IEOC_MASK & ret) >> RT5033_IEOC_SHIFT];
}

static int rt5033_get_fast_charging_current(struct i2c_client *i2c)
{
	int data = rt5033_reg_read(i2c, RT5033_CHG_CTRL5);
	if (data < 0)
		return data;

	data = (data >> 4) & 0x0f;

	if (data > 0xd)
		data = 0xd;
	return data * 100 + 700;
}

static void __rt5033_set_termination_current_limit(struct i2c_client *i2c,
		int current_limit)
{
	int data = rt5033_get_eoc_level(current_limit);
	int ret;

	pr_info("%s : Set Termination\n", __func__);

	ret = rt5033_assign_bits(i2c, RT5033_CHG_CTRL4, RT5033_IEOC_MASK,
			data << RT5033_IEOC_SHIFT);
}
static void rt5033_set_charging_current(struct rt5033_charger_data *charger,
					int eoc, int reset_eoc)
{
	int adj_current = 0;

	adj_current = charger->charging_current * charger->siop_level / 100;
#if CONFIG_SIOP_CHARGING_LIMIT_CURRENT
	if(charger->siop_level < 100 && adj_current > CONFIG_SIOP_CHARGING_LIMIT_CURRENT)
		adj_current = CONFIG_SIOP_CHARGING_LIMIT_CURRENT;
#endif
	pr_info("%s adj_current = %dmA charger->siop_level = %d\n",__func__, adj_current,charger->siop_level);
	mutex_lock(&charger->io_lock);
	__rt5033_set_fast_charging_current(charger,
					   adj_current);
	if (reset_eoc) {
		__rt5033_set_termination_current_limit(
					charger->rt5033->i2c_client, 0);
		/* set EOC RESET */
		rt5033_set_bits(charger->rt5033->i2c_client,
			RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
		/* clear EOC RESET */
		rt5033_clr_bits(charger->rt5033->i2c_client,
			RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
	}

	__rt5033_set_termination_current_limit(
			charger->rt5033->i2c_client, eoc);
	mutex_unlock(&charger->io_lock);
}

enum {
	RT5033_MIVR_DISABLE = 0,
	RT5033_MIVR_4200MV,
	RT5033_MIVR_4300MV,
	RT5033_MIVR_4400MV,
	RT5033_MIVR_4500MV,
	RT5033_MIVR_4600MV,
	RT5033_MIVR_4700MV,
	RT5033_MIVR_4800MV,
};

#if ENABLE_MIVR
/* Dedicated charger (non-USB) device
 * will use lower MIVR level to get better performance
 */
static void rt5033_set_mivr_level(struct rt5033_charger_data *charger)
{
    int mivr;
	switch(charger->cable_type) {
	case POWER_SUPPLY_TYPE_USB ... POWER_SUPPLY_TYPE_USB_ACA:
		mivr = RT5033_MIVR_4600MV;
		break;
	default:
		mivr = RT5033_MIVR_DISABLE;
	}
    mutex_lock(&charger->io_lock);
    rt5033_assign_bits(charger->rt5033->i2c_client,
                       RT5033_CHG_CTRL4, RT5033_MIVR_MASK, mivr << RT5033_MIVR_SHIFT);
    mutex_unlock(&charger->io_lock);
}
#endif /*ENABLE_MIVR*/

static void rt5033_configure_charger(struct rt5033_charger_data *charger)
{
	int eoc;
	int input_current_limit;
	union power_supply_propval val;
	union power_supply_propval chg_mode,swelling_state;

	pr_info("%s : Set config charging\n", __func__);
	if (charger->charging_current < 0) {
		pr_info("%s : OTG is activated. Ignore command!\n",
				__func__);
		return;
	}
	/* Before configure charging setting,
	 * enable bad adaptor detection.
	 * Set Reg0x07[5] = 0 -> enable bad adaptor detection */
	rt5033_clr_bits(charger->rt5033->i2c_client,
			RT5033_EOC_CTRL, (1 << 5));
#if ENABLE_MIVR
	rt5033_set_mivr_level(charger);
#endif /*DISABLE_MIVR*/
	psy_do_property("battery", get,
			POWER_SUPPLY_PROP_CHARGE_NOW, val);
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	psy_do_property("battery", get,
			POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, val);
	input_current_limit = val.intval;
#else
	input_current_limit = charger->pdata->charging_current_table
		[charger->cable_type].input_current_limit;
#endif
	/* Input current limit */
	pr_info("%s : input current (%dmA)\n",
		__func__, input_current_limit);
	rt5033_set_input_current_limit(charger, input_current_limit);
	/*  The MIVR deglitch enable bit is reg0x14[0]
	 * and its deglitch time is around 4us.*/
	rt5033_set_bits(charger->rt5033->i2c_client, 0x14, 0x01);
	/* Float voltage */
	pr_info("%s : float voltage (%dmV)\n",
		__func__, charger->pdata->chg_float_voltage);

	rt5033_set_regulation_voltage(charger,
				      charger->pdata->chg_float_voltage);

	charger->charging_current = charger->pdata->charging_current_table
				    [charger->cable_type].fast_charging_current;

	if (charger->pdata->full_check_type_2nd == SEC_BATTERY_FULLCHARGED_CHGPSY)
	{
#if defined(CONFIG_BATTERY_SWELLING)
		psy_do_property("battery", get,
				POWER_SUPPLY_PROP_CHARGE_CONTROL_LIMIT, swelling_state);
#else
		swelling_state.intval = 0;
#endif
		psy_do_property("battery", get, POWER_SUPPLY_PROP_CHARGE_NOW, chg_mode);

		if (chg_mode.intval == SEC_BATTERY_CHARGING_2ND || swelling_state.intval) {
			eoc = charger->pdata->charging_current_table
					[charger->cable_type].full_check_current_2nd;

			/* change full_charged status */
			charger->full_charged = false;
		} else {
			eoc = charger->pdata->charging_current_table
					[charger->cable_type].full_check_current_1st;
		}
	}
	else {
		eoc = charger->pdata->charging_current_table
				[charger->cable_type].full_check_current_1st;
	}
	/* Fast charge and Termination current */
	pr_info("%s : fast charging current (%dmA)\n",
		__func__, charger->charging_current);

	pr_info("%s : termination current (%dmA)\n",
		__func__, eoc);
	rt5033_set_charging_current(charger, eoc, 1);
}

int rt5033_chg_fled_init(struct i2c_client *client)
{
	int ret;
	rt5033_mfd_chip_t *chip = i2c_get_clientdata(client);
	int is_750k_switching = 0;
	int is_fixed_switching = 0;
	struct rt5033_charger_data *charger = chip->charger;
	/* if charger had loaded, we can get platform data */
	if (charger) {
		is_fixed_switching = charger->pdata->is_fixed_switching;
		is_750k_switching = charger->pdata->is_750kHz_switching;
		pr_info("%s rt5033 chg: is_750k_switching = %d, is_fixed_switching = %d\n",
				__func__, is_750k_switching, is_fixed_switching);
	}
	charger->rev_id = chip->rev_id;
	pr_err("%s : rev_id = %d \n", __func__,charger->rev_id );
	if (charger->rev_id >= 4) {
		if (charger) {

			if (is_fixed_switching)
				ret = rt5033_set_bits(client, 0x22, 0x04);
			else
				ret = rt5033_clr_bits(client, 0x22, 0x04);
			if (ret < 0)
				goto rt5033_chg_fled_init_exit;
			if (is_750k_switching)
				ret = rt5033_set_bits(client, RT5033_CHG_CTRL1,
							RT5033_SEL_SWFREQ_MASK);
			else
				ret = rt5033_clr_bits(client, RT5033_CHG_CTRL1,
							RT5033_SEL_SWFREQ_MASK);
			if (ret < 0)
				goto rt5033_chg_fled_init_exit;
		}
		/* Enable charging CV High-GM, reg0x22 bit 5 */
		ret = rt5033_set_bits(client, 0x22, 0x20);
	} else {
		/* Set switching frequency to 0.75MHz for old revision IC*/
		ret = rt5033_set_bits(client, RT5033_CHG_CTRL1,
				      RT5033_SEL_SWFREQ_MASK);
	}
	rt5033_workaround(chip);

rt5033_chg_fled_init_exit:
	return ret;
}
EXPORT_SYMBOL(rt5033_chg_fled_init);

/* here is set init charger data */
static bool rt5033_chg_init(struct rt5033_charger_data *charger)
{
	rt5033_mfd_chip_t *chip = i2c_get_clientdata(charger->rt5033->i2c_client);
	chip->charger = charger;
	rt5033_chg_fled_init(charger->rt5033->i2c_client);
    /* Disable Timer function (Charging timeout fault) */
    rt5033_clr_bits(charger->rt5033->i2c_client,
                    RT5033_CHG_CTRL3, RT5033_TIMEREN_MASK);
    /* Disable TE */
    rt5033_enable_charging_termination(charger->rt5033->i2c_client, 0);

	/* Enable High-GM */
	rt5033_set_bits(charger->rt5033->i2c_client,
			0x07, 0xC0);

	/* EMI improvement , let reg0x18 bit2~5 be 1100*/
	rt5033_assign_bits(charger->rt5033->i2c_client, 0x18, 0x3C, 0x30);

    /* MUST set correct regulation voltage first
     * Before MUIC pass cable type information to charger
     * charger would be already enabled (default setting)
     * it might cause EOC event by incorrect regulation voltage */
	rt5033_set_regulation_voltage(charger,
				charger->pdata->chg_float_voltage);

#if !(ENABLE_MIVR)
    rt5033_assign_bits(charger->rt5033->i2c_client,
                           RT5033_CHG_CTRL4, RT5033_MIVR_MASK,
                           RT5033_MIVR_DISABLE << RT5033_MIVR_SHIFT);
#endif
	/* Set Reg0x07[5] = 0 -> enable bad adaptor detection */
	rt5033_clr_bits(charger->rt5033->i2c_client,
				RT5033_EOC_CTRL, (1 << 5));
	/* Set Reg0x19[3] = 0 -> enable SafeLDO OVP */
	rt5033_clr_bits(charger->rt5033->i2c_client,
				 0x19, 1 << 3);
	/* set EXT_PMOS_CTRL bit to Enable PMOS for leakage current issue */
	rt5033_assign_bits(charger->rt5033->i2c_client,
		RT5033_CHG_STAT_CTRL, 0x82, 0x02);

	return true;
}

static int rt5033_get_charging_status(struct rt5033_charger_data *charger)
{
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	int ret;

#ifdef CONFIG_USB_HOST_NOTIFY
	struct otg_notify *n = get_otg_notify();
#endif

	ret = rt5033_reg_read(charger->rt5033->i2c_client, RT5033_CHG_STAT_CTRL);
	if (ret<0) {
	    pr_info("Error : can't get charging status (%d)\n", ret);

	}
	pr_info("%s charger->full_charged = %d \n",__func__,charger->full_charged);
	if (charger->full_charged)
		return POWER_SUPPLY_STATUS_FULL;
	switch (ret & 0x30) {
	case 0x00:
		status = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case 0x20:
	case 0x10:
		status = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case 0x30:
		status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	}

	/* TEMP_TEST : when OTG is enabled(charging_current -1), handle OTG func. */
	if (charger->charging_current < 0) {
		/* For OTG mode, RT5033 would still report "charging" */
		status = POWER_SUPPLY_STATUS_DISCHARGING;
		ret = rt5033_reg_read(charger->rt5033->i2c_client, RT5033_CHG_IRQ3);
		if (ret & 0x80) {
			pr_info("%s: otg overcurrent limit\n", __func__);
#ifdef CONFIG_USB_HOST_NOTIFY
			send_otg_notify(n, NOTIFY_EVENT_OVERCURRENT, 0);
#endif
			rt5033_charger_otg_control(charger, false);
		}

	}

	return status;
}

static int rt5033_get_charge_type(struct i2c_client *iic)
{
	int status = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
	int ret;

	ret = rt5033_reg_read(iic, RT5033_CHG_STAT_CTRL);
	if (ret<0)
		dev_err(&iic->dev, "%s fail\n", __func__);

	switch (ret&0x40) {
	case 0x40:
		status = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
	default:
		/* pre-charge mode */
		status = POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
		break;
	}

	return status;
}

static int rt5033_get_charging_health(struct rt5033_charger_data *charger)
{
	int ret = rt5033_reg_read(charger->rt5033->i2c_client, RT5033_CHG_STAT_CTRL);

	if (ret < 0)
		return POWER_SUPPLY_HEALTH_UNKNOWN;

	if (ret & (0x03 << 2)) {
		charger->ovp = false;
		charger->unhealth_cnt = 0;
		return POWER_SUPPLY_HEALTH_GOOD;
	} else if ((ret & (0x04)) == 0) {
		/* No need to disable charger,
		 * H/W will do it automatically */
		charger->unhealth_cnt = HEALTH_DEBOUNCE_CNT;
		charger->ovp = true;
	}
	if (charger->cable_type > POWER_SUPPLY_TYPE_BATTERY)
		charger->unhealth_cnt++;
	if (charger->unhealth_cnt < HEALTH_DEBOUNCE_CNT)
		return POWER_SUPPLY_HEALTH_GOOD;

	charger->unhealth_cnt = HEALTH_DEBOUNCE_CNT;
	if (charger->ovp)
		return POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	return POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
}

static int sec_chg_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{

	int chg_curr,aicr;
	struct rt5033_charger_data *charger =
		container_of(psy, struct rt5033_charger_data, psy_chg);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = charger->charging_current ? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = rt5033_get_charging_status(charger);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = rt5033_get_charging_health(charger);
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		rt5033_test_read(charger->rt5033->i2c_client);
		val->intval =
			rt5033_get_fast_charging_current(
					charger->rt5033->i2c_client);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		mutex_lock(&charger->io_lock);
		if (charger->charging_current) {
			aicr = rt5033_get_input_current_limit(
					charger->rt5033->i2c_client);
			chg_curr = rt5033_get_fast_charging_current(
					charger->rt5033->i2c_client);
			val->intval = MINVAL(aicr, chg_curr);
		} else
			val->intval = 0;
		mutex_unlock(&charger->io_lock);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = rt5033_get_charge_type(
					charger->rt5033->i2c_client);
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = rt5033_get_regulation_voltage(charger);
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
	struct rt5033_charger_data *charger =
	    container_of(psy, struct rt5033_charger_data, psy_chg);

	union power_supply_propval value;
	int eoc;
	int previous_cable_type = charger->cable_type;

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		charger->status = val->intval;
		break;
		/* val->intval : type */
	case POWER_SUPPLY_PROP_ONLINE:
		charger->cable_type = val->intval;
		if (charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
			pr_info("%s:[BATT] Type Battery\n", __func__);
			rt5033_enable_charger_switch(charger, 0);
			if (previous_cable_type == POWER_SUPPLY_TYPE_OTG)
				rt5033_charger_otg_control(charger, false);
		} else if (charger->cable_type == POWER_SUPPLY_TYPE_POWER_SHARING) {
			psy_do_property("ps", get,
					POWER_SUPPLY_PROP_STATUS, value);
			if(value.intval) {
				rt5033_charger_otg_control(charger, true);
				pr_info("%s: Power sharing enable\n", __func__);
			} else {
				rt5033_charger_otg_control(charger, false);
				pr_info("%s: Power sharing disable\n", __func__);
			}
			break;
		} else if (charger->cable_type == POWER_SUPPLY_TYPE_OTG) {
			pr_info("%s: OTG mode\n", __func__);
			rt5033_charger_otg_control(charger, true);
		} else {
			pr_info("%s:[BATT] Set charging"
					", Cable type = %d\n", __func__, charger->cable_type);
			/* Enable charger */
			rt5033_configure_charger(charger);
			rt5033_enable_charger_switch(charger, 1);
		}
#if EN_TEST_READ
		msleep(100);
		rt5033_test_read(charger->rt5033->i2c_client);
#endif
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		/* decrease the charging current according to siop level */
		charger->siop_level = val->intval;
		pr_info("%s:SIOP level = %d, chg current = %d\n", __func__,
				val->intval, charger->charging_current);
		if(charger->is_charging) {
			eoc = rt5033_get_current_eoc_setting(charger);
			rt5033_set_charging_current(charger, eoc, 0);
		}
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		/* If slate mode is enable, set the current as 0mA*/
		__rt5033_set_fast_charging_current(charger, val->intval);
		break;
	case POWER_SUPPLY_PROP_POWER_NOW:
		eoc = rt5033_get_current_eoc_setting(charger);
		pr_info("%s:Set Power Now -> chg current = %d mA, eoc = %d mA\n", __func__,
			val->intval, eoc);
		rt5033_set_charging_current(charger, eoc, 0);
		break;
#if defined(CONFIG_BATTERY_SWELLING)
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		pr_info("%s: float voltage(%d)\n", __func__, val->intval);
		rt5033_set_regulation_voltage(charger, val->intval);
		break;
		
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		charger->charging_current = val->intval;
		__rt5033_set_fast_charging_current(charger, val->intval);
#endif
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	case POWER_SUPPLY_PROP_INPUT_CURRENT_MAX:
		if (val->intval != charger->aicr) {
			if (val->intval > charger->aicr) {
				charger->is_aicr_changed = true;
			} else {
				charger->is_aicr_changed = false;
			}
			pr_info("%s: input current limit(%d, %d)\n",
				__func__, val->intval, charger->is_aicr_changed);
			rt5033_set_input_current_limit(charger, val->intval);
		}
		break;
#endif
	default:
		return -EINVAL;
	}

	return 0;
}

ssize_t rt5033_chg_show_attrs(struct device *dev,
		const ptrdiff_t offset, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct rt5033_charger_data *charger =
		container_of(psy, struct rt5033_charger_data, psy_chg);
	int i = 0;
	char *str = NULL;

	switch (offset) {
	case CHG_REG:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%x\n",
				charger->reg_addr);
		break;
	case CHG_DATA:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%x\n",
				charger->reg_data);
		break;
	case CHG_REGS:
		str = kzalloc(sizeof(char) * 256, GFP_KERNEL);
		if (!str)
			return -ENOMEM;

		rt5033_read_regs(charger->rt5033->i2c_client, str);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
				str);

		kfree(str);
		break;
	default:
		i = -EINVAL;
		break;
	}

	return i;
}

ssize_t rt5033_chg_store_attrs(struct device *dev,
		const ptrdiff_t offset,
		const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct rt5033_charger_data *charger =
		container_of(psy, struct rt5033_charger_data, psy_chg);

	int ret = 0;
	int x = 0;
	uint8_t data = 0;

	switch (offset) {
	case CHG_REG:
		if (sscanf(buf, "%x\n", &x) == 1) {
			charger->reg_addr = x;
			data = rt5033_reg_read(charger->rt5033->i2c_client,
					charger->reg_addr);
			charger->reg_data = data;
			dev_dbg(dev, "%s: (read) addr = 0x%x, data = 0x%x\n",
					__func__, charger->reg_addr, charger->reg_data);
			ret = count;
		}
		break;
	case CHG_DATA:
		if (sscanf(buf, "%x\n", &x) == 1) {
			data = (u8)x;

			dev_dbg(dev, "%s: (write) addr = 0x%x, data = 0x%x\n",
					__func__, charger->reg_addr, data);
			ret = rt5033_reg_write(charger->rt5033->i2c_client,
					charger->reg_addr, data);
			if (ret < 0) {
				dev_dbg(dev, "I2C write fail Reg0x%x = 0x%x\n",
					(int)charger->reg_addr, (int)data);
			}
			ret = count;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

struct rt5033_chg_irq_handler {
	char *name;
	int irq_index;
	irqreturn_t (*handler)(int irq, void *data);
};

#if EN_BAD_ADP_IRQ

static void rt5033chg_activate_bad_adp(
				struct work_struct *work)
{
	rt5033_charger_data_t *charger =
			container_of(to_delayed_work(work),
					rt5033_charger_data_t,
					activate_bad_adp_work);
#if ENABLE_MIVR
	/* Restore MIVR setting */
	rt5033_set_mivr_level(charger);
#endif
	/* Set Reg0x07[5] = 0 -> enable bad adaptor detection */
	rt5033_clr_bits(charger->rt5033->i2c_client,
					RT5033_EOC_CTRL, (1 << 5));
}

static irqreturn_t rt5033_chg_bad_adp_irq_handler(int irq,
								void *data)
{
	struct rt5033_charger_data *info = data;
	cancel_delayed_work(&info->activate_bad_adp_work);
	pr_info("%s : Bad adaptor : enable MIVR temporarily\n",
						__func__);
	pr_info("%s : set MIVR = 4200mV\n", __func__);
	rt5033_assign_bits(info->rt5033->i2c_client,
			RT5033_CHG_CTRL4, RT5033_MIVR_MASK,
			RT5033_MIVR_4200MV << RT5033_MIVR_SHIFT);
	/* Disable bad adaptor detection */
	rt5033_set_bits(info->rt5033->i2c_client,
			RT5033_EOC_CTRL, (1 << 5));
	queue_delayed_work(info->wq, &info->activate_bad_adp_work,
			msecs_to_jiffies(BADADP_RESTART_TIMER));

	return IRQ_HANDLED;
}
#endif /* EN_BAD_ADP_IRQ */
#if EN_OVP_IRQ
static irqreturn_t rt5033_chg_vin_ovp_irq_handler(int irq, void *data)
{
	struct rt5033_charger_data *info = data;
	union power_supply_propval value;
	int status;

	/* Delay 100ms for debounce */
	msleep(100);
	status = rt5033_reg_read(info->rt5033->i2c_client,
			RT5033_CHG_STAT_CTRL);

	/* PWR ready = 0*/
	if ((status & (0x04)) == 0) {
		/* No need to disable charger,
		 * H/W will do it automatically */
		info->unhealth_cnt = HEALTH_DEBOUNCE_CNT;
		info->ovp = true;
		pr_info("%s: OVP triggered\n", __func__);
		value.intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		psy_do_property("battery", set,
				POWER_SUPPLY_PROP_HEALTH, value);
	} else {
		info->unhealth_cnt = 0;
		info->ovp = false;
	}

	return IRQ_HANDLED;
}

static irqreturn_t rt5033_chg_vin_ovpr_irq_handler(int irq, void *data)
{
	struct rt5033_charger_data *info = data;
	union power_supply_propval value;
	int status;
	status = rt5033_reg_read(info->rt5033->i2c_client,
			0x22);
	if ((status & 0x02) == 0) {
		status = rt5033_reg_read(info->rt5033->i2c_client,
			RT5033_CHG_STAT_CTRL);
		/* Check PowerGood bit */
		if (status & (0x01 << 2)) {
			info->unhealth_cnt = 0;
			info->ovp = false;
			pr_info("%s: OVP recover triggered\n", __func__);
			value.intval = POWER_SUPPLY_HEALTH_GOOD;
			psy_do_property("battery", set,
					POWER_SUPPLY_PROP_HEALTH, value);
		}
	}
	return IRQ_HANDLED;
}

#endif /* EN_OVP_IRQ */

#if EN_IEOC_IRQ
static irqreturn_t rt5033_chg_ieoc_irq_handler(int irq, void *data)
{
	struct rt5033_charger_data *info = data;
	struct i2c_client *iic = info->rt5033->i2c_client;
	int eoc_reg;
	cancel_delayed_work(&info->eoc_timeout_work);
	mutex_lock(&info->io_lock);
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	if (info->is_aicr_changed) {
		info->eoc_cnt = 0;
	} else {
		info->eoc_cnt++;
	}
	info->is_aicr_changed = false;
#else
	info->eoc_cnt++;
#endif
	mutex_unlock(&info->io_lock);
	pr_info("%s : EOC CNT = %d / %d\n", __func__,
		info->eoc_cnt, EOC_DEBOUNCE_CNT);
	if (info->eoc_cnt >= EOC_DEBOUNCE_CNT) {
		/* set full charged flag
		 * until TA/USB unplug event / stop charging by PSY
		 * / recharging event
		 */
		pr_info("%s : Full charged\n", __func__);
		info->full_charged = true;
		info->eoc_cnt = 0;
	} else {
		pr_info("%s : Reset EOC detection\n", __func__);
		msleep(10);
		/* Reset EOC loop, and make it re-detect */
		mutex_lock(&info->io_lock);
		eoc_reg = rt5033_reg_read(iic, RT5033_CHG_CTRL4);
		if (eoc_reg < 0)
			pr_err("error : rt5033 i2c read failed...\n");
		rt5033_set_bits(iic, RT5033_UUG, 0x80); // EN sinking
		msleep(10); // 10ms
		rt5033_clr_bits(iic, RT5033_UUG, 0x80); // DIS sinking
        /* Disable EOC function */
        rt5033_reg_write(iic, RT5033_CHG_CTRL4, (~RT5033_IEOC_MASK) & eoc_reg);
        /* set EOC RESET */
        rt5033_set_bits(iic, RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
        /* clear EOC RESET */
        rt5033_clr_bits(iic, RT5033_EOC_CTRL, RT5033_EOC_RESET_MASK);
        /* Restore EOC setting */
        rt5033_reg_write(iic, RT5033_CHG_CTRL4, eoc_reg);
        mutex_unlock(&info->io_lock);
		msleep(EOC_SLEEP);
		queue_delayed_work(info->wq, &info->eoc_timeout_work,
					msecs_to_jiffies(EOC_TIMEOUT));
	}
	return IRQ_HANDLED;
}
#endif /* EN_IEOC_IRQ */

#if EN_RECHG_REQ_IRQ
static irqreturn_t rt5033_chg_rechg_request_irq_handler(int irq, void *data)
{
	struct rt5033_charger_data *info = data;
	pr_info("%s: Recharging requesting\n", __func__);

	info->full_charged = false;
	info->eoc_cnt = 0;

        return IRQ_HANDLED;
}
#endif /* EN_RECHG_REQ_IRQ */

#if EN_TR_IRQ
static irqreturn_t rt5033_chg_otp_tr_irq_handler(int irq, void *data)
{
	pr_info("%s : Over temperature : thermal regulation loop active\n",
			__func__);
	// if needs callback, do it here
	return IRQ_HANDLED;
}
#endif

const struct rt5033_chg_irq_handler rt5033_chg_irq_handlers[] = {
#if EN_BAD_ADP_IRQ
	{
		.name = "BAD ADAPTOR",
		.handler = rt5033_chg_bad_adp_irq_handler,
		.irq_index = RT5033_ADPBAD_IRQ,
	},
#endif
#if EN_OVP_IRQ
	{
		.name = "VIN_OVP",
		.handler = rt5033_chg_vin_ovp_irq_handler,
		.irq_index = RT5033_VINOVPI_IRQ,
	},
	{
		.name = "VIN_OVP_RELEASE",
		.handler = rt5033_chg_vin_ovpr_irq_handler,
		.irq_index = RT5033_OVPR_IRQ,
	},
#endif /* EN_OVP_IRQ */
#if EN_IEOC_IRQ
	{
		.name = "IEOC",
		.handler = rt5033_chg_ieoc_irq_handler,
		.irq_index = RT5033_IEOC_IRQ,
	},
#endif /* EN_IEOC_IRQ */
#if EN_RECHG_REQ_IRQ
	{
		.name = "RECHG_RQUEST",
		.handler = rt5033_chg_rechg_request_irq_handler,
		.irq_index = RT5033_CHRCHGI_IRQ,
	},
#endif /* EN_RECHG_REQ_IRQ*/
#if EN_TR_IRQ
	{
		.name = "OTP ThermalRegulation",
		.handler = rt5033_chg_otp_tr_irq_handler,
		.irq_index = RT5033_CHTREGI_IRQ,
	},
#endif /* EN_TR_IRQ */

#if EN_BST_IRQ
	{
		.name = "OTG Low Batt",
		.handler = rt5033_chg_otg_fail_irq_handler,
		.irq_index = RT5033_BSTLOWVI_IRQ,
	},
	{
		.name = "OTG Over Load",
		.handler = rt5033_chg_otg_fail_irq_handler,
		.irq_index = RT5033_BSTOLI_IRQ,
	},
	{
		.name = "OTG OVP",
		.handler = rt5033_chg_otg_fail_irq_handler,
		.irq_index = RT5033_BSTVMIDOVP_IRQ,
	},
#endif //EN_BST_IRQ
};

static int register_irq(struct platform_device *pdev,
		struct rt5033_charger_data *info)
{
	int irq;
	int i, j;
	int ret;
	const struct rt5033_chg_irq_handler *irq_handler = rt5033_chg_irq_handlers;
	const char *irq_name;

#if EN_BAD_ADP_IRQ
	irq_name = rt5033_get_irq_name_by_index(RT5033_ADPBAD_IRQ);
	irq = platform_get_irq_byname(pdev, irq_name);
	info->bad_adp_irq = irq;
#endif
	for (i = 0; i < ARRAY_SIZE(rt5033_chg_irq_handlers); i++) {
		irq_name = rt5033_get_irq_name_by_index(irq_handler[i].irq_index);
		irq = platform_get_irq_byname(pdev, irq_name);
		ret = request_threaded_irq(irq, NULL, irq_handler[i].handler,
					   IRQF_ONESHOT | IRQF_TRIGGER_RISING |
					   IRQF_NO_SUSPEND, irq_name, info);
		if (ret < 0) {
			pr_err("%s : Failed to request IRQ (%s): #%d: %d\n",
					__func__, irq_name, irq, ret);
			goto err_irq;
		}

		pr_info("%s : Register IRQ%d(%s) successfully\n",
				__func__, irq, irq_name);
	}

	return 0;
err_irq:
	for (j = 0; j < i; j++) {
		irq_name = rt5033_get_irq_name_by_index(irq_handler[j].irq_index);
		irq = platform_get_irq_byname(pdev, irq_name);
		free_irq(irq, info);
	}

	return ret;
}

static void unregister_irq(struct platform_device *pdev,
		struct rt5033_charger_data *info)
{
	int irq;
	int i;
	const char *irq_name;
	const struct rt5033_chg_irq_handler *irq_handler = rt5033_chg_irq_handlers;

	for (i = 0; i < ARRAY_SIZE(rt5033_chg_irq_handlers); i++) {
		irq_name = rt5033_get_irq_name_by_index(irq_handler[i].irq_index);
		irq = platform_get_irq_byname(pdev, irq_name);
		free_irq(irq, info);
	}
}

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
static int rt5033_charger_parse_dualized_dt(struct device *dev,
                           struct rt5033_charger_platform_data *pdata)
{
	struct device_node *np;
	int ret, i, len;

	np = of_find_node_by_name(NULL, "charger");
	if (!np) {
		pr_info("%s : np NULL\n", __func__);
		return -ENODATA;
	}

	of_get_property(np, "battery,fast_charging_current_5033a", &len);
	len = len / sizeof(u32);

	for(i = 0; i < len; i++) {
		ret = sec_bat_read_u32_index_dt(np,
			 "battery,input_current_limit_5033a", i,
			 &pdata->charging_current_table[i].input_current_limit);
		ret = sec_bat_read_u32_index_dt(np,
			 "battery,fast_charging_current_5033a", i,
			 &pdata->charging_current_table[i].fast_charging_current);
	}
	pr_info("%s : length of 5033a : %d, AICR = %d\n", __func__, len, pdata->dualized_charging_current);
	return ret;
}
static int rt5033_charger_parse_dt(struct device *dev,
                           struct rt5033_charger_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	const u32 *p;
	int ret, i, len;

	if (of_find_property(np, "is_750kHz_switching", NULL))
		pdata->is_750kHz_switching = 1;
	if (of_find_property(np, "is_fixed_switching", NULL))
		pdata->is_fixed_switching = 1;
	pr_info("%s : is_750kHz_switching = %d\n", __func__,
		pdata->is_750kHz_switching);
	pr_info("%s : is_fixed_switching = %d\n", __func__,
		pdata->is_fixed_switching);
		
	np = of_find_node_by_name(NULL, "battery");
	if (!np) {
		pr_info("%s : np NULL\n", __func__);
		return -ENODATA;
	}
	ret = of_property_read_u32(np, "battery,full_check_type",
			&pdata->full_check_type);
	if (ret)
		pr_info("%s: full_check_type is Empty\n", __func__);

	ret = of_property_read_u32(np, "battery,full_check_type_2nd",
			&pdata->full_check_type_2nd);
	if (ret)
		pr_info("%s: full_check_type_2nd is Empty\n", __func__);
	
	np = of_find_node_by_name(NULL, "charger");
	if (!np) {
		pr_info("%s : np NULL\n", __func__);
		return -ENODATA;
	}
	ret = of_property_read_u32(np, "battery,chg_float_voltage",
				   &pdata->chg_float_voltage);
	if (ret < 0) {
		pr_info("%s : cannot get chg float voltage\n", __func__);
		return -ENODATA;
	}

	p = of_get_property(np, "battery,input_current_limit", &len);

	len = len / sizeof(u32);

	pdata->charging_current_table =
        kzalloc(sizeof(sec_charging_current_t) * len, GFP_KERNEL);

	for(i = 0; i < len; i++) {
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,input_current_limit", i,
				 &pdata->charging_current_table[i].input_current_limit);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,fast_charging_current", i,
				 &pdata->charging_current_table[i].fast_charging_current);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,full_check_current_1st", i,
				 &pdata->charging_current_table[i].full_check_current_1st);
		ret = sec_bat_read_u32_index_dt(np,
				 "battery,full_check_current_2nd", i,
				 &pdata->charging_current_table[i].full_check_current_2nd);
	}
	dev_info(dev,"rt5033 charger parse dt retval = %d\n", ret);

	/* use for AICR disabled in RT5033 */
	pdata->dualized_charging_current = of_property_read_bool(np,"battery,dualized_charging_current");

	return ret;
}

static struct of_device_id rt5033_charger_match_table[] = {
	{ .compatible = "richtek,rt5033-charger",},
	{},
};
#else
static int rt5033_charger_parse_dt(struct device *dev,
                           struct rt5033_charger_platform_data *pdata)
{
    return -ENOSYS;
}
#define rt5033_charger_match_table NULL
#endif /* CONFIG_OF */

static void rt5033chg_eoc_timeout_reset(
				struct work_struct *work)
{
	rt5033_charger_data_t *charger =
			container_of(to_delayed_work(work),
					rt5033_charger_data_t,
					eoc_timeout_work);
	mutex_lock(&charger->io_lock);
	charger->eoc_cnt = 0;
	mutex_unlock(&charger->io_lock);
}
static int rt5033_charger_probe(struct platform_device *pdev)
{
	rt5033_mfd_chip_t *chip = dev_get_drvdata(pdev->dev.parent);
	struct rt5033_mfd_platform_data *mfd_pdata = dev_get_platdata(chip->dev);
	struct rt5033_charger_data *charger;
	int ret = 0;

	pr_info("%s:[BATT] RT5033 Charger driver probe..0x%x\n", __func__, (unsigned int)mfd_pdata);

	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

#ifdef CONFIG_OF
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	if (pdev->dev.parent->of_node) {
		pdev->dev.of_node = of_find_compatible_node(
			of_node_get(pdev->dev.parent->of_node), NULL,
			rt5033_charger_match_table[0].compatible);
	}
#endif
#endif
	mutex_init(&charger->io_lock);
	charger->wq = create_workqueue("rt5033chg_workqueue");
	INIT_DELAYED_WORK(&charger->eoc_timeout_work,
				rt5033chg_eoc_timeout_reset);
#if EN_BAD_ADP_IRQ
	INIT_DELAYED_WORK(&charger->activate_bad_adp_work,
				rt5033chg_activate_bad_adp);
#endif /* EN_BAD_ADP_IRQ */
	charger->rt5033= chip;
	if (pdev->dev.of_node) {
		charger->pdata = devm_kzalloc(&pdev->dev, sizeof(*(charger->pdata)),
					      GFP_KERNEL);
		if (!charger->pdata) {
			dev_err(&pdev->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_parse_dt_nomem;
		}
		ret = rt5033_charger_parse_dt(&pdev->dev, charger->pdata);
		if (ret < 0)
			goto err_parse_dt;
	} else
		charger->pdata = mfd_pdata->charger_platform_data;

	platform_set_drvdata(pdev, charger);

	charger->psy_chg.name           = "rt5033-charger";
	charger->psy_chg.type           = POWER_SUPPLY_TYPE_UNKNOWN;
	charger->psy_chg.get_property   = sec_chg_get_property;
	charger->psy_chg.set_property   = sec_chg_set_property;
	charger->psy_chg.properties     = sec_charger_props;
	charger->psy_chg.num_properties = ARRAY_SIZE(sec_charger_props);

	charger->siop_level = 100;
	rt5033_chg_init(charger);

	/* parse for dualized dt */
	if(charger->pdata->dualized_charging_current && charger->rev_id >= 6){
		ret = rt5033_charger_parse_dualized_dt(&pdev->dev, charger->pdata);
		if (ret < 0)
			goto err_parse_dt;
	}
	ret = power_supply_register(&pdev->dev, &charger->psy_chg);
	if (ret) {
		pr_err("%s: Failed to Register psy_chg\n", __func__);
		goto err_power_supply_register;
	}
	ret = register_irq(pdev, charger);
	if (ret < 0)
        goto err_reg_irq;

	rt5033_test_read(charger->rt5033->i2c_client);
	pr_info("%s:[BATT] RT5033 charger driver loaded OK\n", __func__);

	return 0;
err_reg_irq:
    power_supply_unregister(&charger->psy_chg);
err_power_supply_register:
err_parse_dt:
err_parse_dt_nomem:
	destroy_workqueue(charger->wq);
	mutex_destroy(&charger->io_lock);
	kfree(charger);
	return ret;
}

static int rt5033_charger_remove(struct platform_device *pdev)
{
	struct rt5033_charger_data *charger =
	platform_get_drvdata(pdev);
	unregister_irq(pdev, charger);
	power_supply_unregister(&charger->psy_chg);
	destroy_workqueue(charger->wq);
	mutex_destroy(&charger->io_lock);
	kfree(charger);
	return 0;
}

#if defined CONFIG_PM
static int rt5033_charger_suspend(struct device *dev)
{
	return 0;
}

static int rt5033_charger_resume(struct device *dev)
{
	return 0;
}
#else
#define rt5033_charger_suspend NULL
#define rt5033_charger_resume NULL
#endif

static void rt5033_charger_shutdown(struct device *dev)
{
	pr_info("%s: RT5033 Charger driver shutdown\n", __func__);
}

static SIMPLE_DEV_PM_OPS(rt5033_charger_pm_ops, rt5033_charger_suspend,
		rt5033_charger_resume);

static struct platform_driver rt5033_charger_driver = {
	.driver		= {
		.name	= "rt5033-charger",
		.owner	= THIS_MODULE,
		.of_match_table = rt5033_charger_match_table,
		.pm 	= &rt5033_charger_pm_ops,
		.shutdown = rt5033_charger_shutdown,
	},
	.probe		= rt5033_charger_probe,
	.remove		= rt5033_charger_remove,
};

static int __init rt5033_charger_init(void)
{
	int ret = 0;

	pr_info("%s \n", __func__);
	ret = platform_driver_register(&rt5033_charger_driver);

	return ret;
}
subsys_initcall(rt5033_charger_init);

static void __exit rt5033_charger_exit(void)
{
	platform_driver_unregister(&rt5033_charger_driver);
}
module_exit(rt5033_charger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick Chang <patrick_chang@richtek.com");
MODULE_DESCRIPTION("Charger driver for RT5033");
