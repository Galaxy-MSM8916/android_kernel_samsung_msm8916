/* drivers/battery/sm5703_charger.c
 * SM5703 Charger Driver
 *
 * Copyright (C) 2013 Siliconmitus Technology Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/battery/sec_charger.h>
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/host_notify.h>
#include <linux/usb_notify.h>
#endif
#include <linux/wakelock.h>
#ifdef CONFIG_SM5703_MUIC
#include <linux/i2c/sm5703-muic.h>
#endif

#include <linux/mfd/sm5703.h>

//#ifdef CONFIG_FLED_SM5703
#include <linux/leds/sm5703_fled.h>
#include <linux/leds/smfled.h>
#include <linux/mfd/smfled.h>
#include <linux/mfd/sm5703_fled.h>
//#endif /* CONFIG_FLED_SM5703 */
#include <linux/version.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/of_gpio.h>


#define EN_NOBAT_IRQ	0
#define EN_DONE_IRQ 1
#define EN_TOPOFF_IRQ 1
#define EN_CHGON_IRQ 0
#define EN_OTGFAIL_IRQ 1
#define EN_VBUSLIMIT_IRQ	0
#define EN_AICL_IRQ			1
#define DEFAULT_CHARGING_CURRENT 500

#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC) 
#define ENABLE_AICL 1
#endif

#define MINVAL(a, b) ((a <= b) ? a : b)

#ifndef EN_TEST_READ
#define EN_TEST_READ 1
#endif

#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
#define LIMIT_VOLTAGE_STEP1         4200
#define LIMIT_VOLTAGE_STEP2         4100
#define LIMIT_CURRENT_STEP1         300
#define LIMIT_CURRENT_STEP2         500
#if defined(CONFIG_BATTERY_SWELLING)
#define LIMIT_SWELLING_VOLTAGE      4000
#define LIMIT_SWELLING_CURRENT      300
#endif
#endif

static int sm5703_reg_map[] = {
	SM5703_INTMSK1,
	SM5703_INTMSK2,
	SM5703_INTMSK3,
	SM5703_INTMSK4,
	SM5703_STATUS1,
	SM5703_STATUS2,
	SM5703_STATUS3,
	SM5703_STATUS4,
	SM5703_CNTL,
	SM5703_VBUSCNTL,
	SM5703_CHGCNTL1,
	SM5703_CHGCNTL2,
	SM5703_CHGCNTL3,
	SM5703_CHGCNTL4,
	SM5703_CHGCNTL5,
	SM5703_CHGCNTL6,
	SM5703_OTGCURRENTCNTL,
	SM5703_Q3LIMITCNTL,
	SM5703_STATUS5,
};

typedef struct sm5703_charger_data {
	struct i2c_client	*client;
	sm5703_mfd_chip_t	*sm5703;
	struct power_supply	psy_chg;
	sm5703_charger_platform_data_t *pdata;
	int charging_current;
	struct wake_lock vbuslimit_wake_lock;
	struct delayed_work vbuslimit_work;
	int	current_max;
	bool is_current_reduced;
	int siop_level;
	int cable_type;
	bool is_charging;
	struct mutex io_lock;
	/* register programming */
	int reg_addr;
	int reg_data;
	int nchgen;

	bool full_charged;
	bool ovp;
	struct workqueue_struct *wq;
	int status;
#ifdef CONFIG_FLED_SM5703
	struct sm_fled_info *fled_info;
#endif /* CONFIG_FLED_SM5703 */
} sm5703_charger_data_t;

static enum power_supply_property sec_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
#endif
#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
	POWER_SUPPLY_PROP_INPUT_CURRENT_MAX,
#endif
};

static int sm5703_get_charging_health(
		struct sm5703_charger_data *charger);
static int sm5703_get_charging_status(struct sm5703_charger_data *charger);
static void sm5703_read_regs(struct i2c_client *i2c, char *str)
{
	u8 data = 0;
	int i = 0;
	for (i = SM5703_INTMSK1; i < ARRAY_SIZE(sm5703_reg_map); i++) {
		data = sm5703_reg_read(i2c, sm5703_reg_map[i]);
		sprintf(str+strlen(str), "0x%02x, ", data);
	}
}

static void sm5703_test_read(struct i2c_client *i2c)
{
	char str[1000] = {0,};
	int i;

	/* SM5703 REG: 0x04 ~ 0x13 */
	for (i = SM5703_INTMSK1; i <= SM5703_CHGCNTL6; i++) {
		int data;
		data = sm5703_reg_read(i2c, i);
		sprintf(str+strlen(str), "0x%0x = 0x%02x, ", i, data);
	}

	sprintf(str+strlen(str), "0x%0x = 0x%02x, ",SM5703_OTGCURRENTCNTL,
		sm5703_reg_read(i2c, SM5703_OTGCURRENTCNTL));
	sprintf(str+strlen(str), "0x%0x = 0x%02x, ", SM5703_STATUS5,
		sm5703_reg_read(i2c, SM5703_STATUS5));
	sprintf(str+strlen(str), "0x%0x = 0x%02x, ", SM5703_Q3LIMITCNTL,
		sm5703_reg_read(i2c, SM5703_Q3LIMITCNTL));
	pr_info("%s: %s\n", __func__, str);
}

#define SM5703_FLEDCNTL6			0x19
static void sm5703_charger_otg_control(struct sm5703_charger_data *charger,
		bool enable)
{
	pr_info("%s: called charger otg control : %s\n", __func__,
		enable ? "on" : "off");

	if (!enable) {
		/* turn off OTG */
		sm5703_assign_bits(charger->sm5703->i2c_client,
				   SM5703_FLEDCNTL6, SM5703_BSTOUT_MASK,
				   SM5703_BSTOUT_4P5);

#ifdef CONFIG_FLED_SM5703
		if (charger->fled_info == NULL)
			charger->fled_info = sm_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			sm5703_boost_notification(charger->fled_info, 0);
#else
		sm5703_assign_bits(charger->sm5703->i2c_client,
			SM5703_CNTL, SM5703_OPERATION_MODE_MASK,
			SM5703_OPERATION_MODE_CHARGING_ON);
#endif
	} else {
		sm5703_assign_bits(charger->sm5703->i2c_client,
			SM5703_FLEDCNTL6, SM5703_BSTOUT_MASK,
			SM5703_BSTOUT_5P0);
#ifdef CONFIG_FLED_SM5703
		if (charger->fled_info == NULL)
			charger->fled_info = sm_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			sm5703_boost_notification(charger->fled_info, 1);
#else
		sm5703_assign_bits(charger->sm5703->i2c_client,
			SM5703_CNTL, SM5703_OPERATION_MODE_MASK,
			SM5703_OPERATION_MODE_USB_OTG_MODE);
#endif
		charger->cable_type = POWER_SUPPLY_TYPE_OTG;
	}
}


#ifdef CONFIG_CHARGER_SM5703_SOFT_START_CHARGING
static int sm5703_get_input_current_limit(struct i2c_client *i2c);
#endif

static void sm5703_enable_charger_switch(struct sm5703_charger_data *charger,
		int onoff)
{
#ifdef CONFIG_CHARGER_SM5703_SOFT_START_CHARGING
	int get_input_current=0;
	int data=0;
#endif
#ifdef CONFIG_CHARGER_SM5703_DUALPATH
	union power_supply_propval batt_pres;
#endif
	int prev_charging_status = charger->is_charging;

	charger->is_charging = onoff ? true : false;
	if ((onoff > 0) && (prev_charging_status == false)) {
		pr_info("%s: turn on charger\n", __func__);

#ifdef CONFIG_FLED_SM5703
		if (charger->fled_info == NULL)
			charger->fled_info = sm_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			sm5703_charger_notification(charger->fled_info,1);
#endif
#ifdef CONFIG_CHARGER_SM5703_SOFT_START_CHARGING
		get_input_current=sm5703_get_input_current_limit(charger->sm5703->i2c_client);
		mutex_lock(&charger->io_lock);
		data = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_VBUSCNTL);
		data &= ~SM5703_VBUSLIMIT;
		sm5703_reg_write(charger->sm5703->i2c_client, SM5703_VBUSCNTL, data);
#endif
		sm5703_assign_bits(charger->sm5703->i2c_client,
			SM5703_CNTL, SM5703_OPERATION_MODE_MASK,
			SM5703_OPERATION_MODE_CHARGING_ON);

		charger->nchgen = false;
		gpio_direction_output(charger->pdata->chgen_gpio,
			charger->nchgen); //nCHG enable

#ifdef CONFIG_CHARGER_SM5703_SOFT_START_CHARGING
		mdelay(100);
		if (get_input_current > 100) {
			int temp;
			temp = ((get_input_current - 100) / 50) | data;
			sm5703_reg_write(charger->sm5703->i2c_client, SM5703_VBUSCNTL, temp);
		}
		mutex_unlock(&charger->io_lock);
#endif
		pr_info("%s : STATUS OF CHARGER ON(0)/OFF(1): %d\n",
			__func__, charger->nchgen);
	} else if (onoff == 0) {
		charger->full_charged = false;
		pr_info("%s: turn off charger\n", __func__);

		charger->charging_current = DEFAULT_CHARGING_CURRENT;
		charger->nchgen = true;
#ifdef CONFIG_FLED_SM5703
		if (charger->fled_info == NULL)
			charger->fled_info = sm_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			sm5703_charger_notification(charger->fled_info,0);
#endif
		gpio_direction_output(charger->pdata->chgen_gpio,
			charger->nchgen); //nCHG disable
		pr_info("%s : STATUS OF CHARGER ON(0)/OFF(1): %d\n",
			__func__, charger->nchgen);
#ifdef CONFIG_CHARGER_SM5703_DUALPATH
		psy_do_property("battery", get,
					POWER_SUPPLY_PROP_PRESENT, batt_pres);
		if(batt_pres.intval== false){
			sm5703_assign_bits(charger->sm5703->i2c_client,
					SM5703_CNTL, SM5703_OPERATION_MODE_MASK,
					SM5703_OPERATION_MODE_SUSPEND);
			pr_info("%s: DUALPATH set to SM5703_OPERATION_MODE_SUSPEND \n",__func__);
		}
#endif
	} else {
	    pr_info("%s: repeated to set charger switch(%d), prev stat = %d\n",
             __func__, onoff, prev_charging_status ? 1 : 0);
	}
}

static int sm5703_CHG_set_TOPOFF_TMR(struct sm5703_charger_data *charger,
				unsigned char topoff_timer)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;

	sm5703_assign_bits(i2c,
		SM5703_CHGCNTL5, SM5703_TOPOFF_TIMER_MASK,
		((topoff_timer & SM5703_TOPOFF_TIMER) << SM5703_TOPOFF_TIMER_SHIFT));
	pr_info("TOPOFF_TMR set (timer=%d)\n", topoff_timer);

	return 0;
}

static void sm5703_enable_autostop(struct sm5703_charger_data *charger,
		int onoff)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;

	pr_info("%s:[BATT] Autostop set(%d)\n", __func__, onoff);

	mutex_lock(&charger->io_lock);

	if (onoff)
		sm5703_set_bits(i2c, SM5703_CHGCNTL4, SM5703_AUTOSTOP_MASK);
	else
		sm5703_clr_bits(i2c, SM5703_CHGCNTL4, SM5703_AUTOSTOP_MASK);

	mutex_unlock(&charger->io_lock);
}

static void sm5703_enable_autoset(struct sm5703_charger_data *charger,
		int onoff)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	pr_info("%s:[BATT] Autoset set(%d)\n", __func__, onoff);

	mutex_lock(&charger->io_lock);
 	if (onoff)
		sm5703_set_bits(i2c, SM5703_CNTL, SM5703_AUTOSET_MASK);
	else
		sm5703_clr_bits(i2c, SM5703_CNTL, SM5703_AUTOSET_MASK);
	mutex_unlock(&charger->io_lock);
}

static void sm5703_enable_aiclen(struct sm5703_charger_data *charger,
		int onoff)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	pr_info("%s:[BATT] AICLEN set(%d)\n", __func__, onoff);

	mutex_lock(&charger->io_lock);
	if (onoff)
		sm5703_set_bits(i2c, SM5703_CHGCNTL5, SM5703_AICLEN_MASK);
	else
		sm5703_clr_bits(i2c, SM5703_CHGCNTL5, SM5703_AICLEN_MASK);
	mutex_unlock(&charger->io_lock);
}

static void sm5703_set_aiclth(struct sm5703_charger_data *charger,
		int aiclth)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0, temp = 0;

	mutex_lock(&charger->io_lock);
	data = sm5703_reg_read(i2c, SM5703_CHGCNTL5);
	data &= ~SM5703_AICLTH;

	if (aiclth >= 4900)
		aiclth = 4900;

	if(aiclth >= 4300){
		temp = (aiclth - 4300)/100;
		data |= temp;
	}

	sm5703_reg_write(i2c, SM5703_CHGCNTL5, data);

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL5);
	pr_info("%s : SM5703_CHGCNTL5 (AICHTH) : 0x%02x\n",
		__func__, data);
	mutex_unlock(&charger->io_lock);
}

#if EN_AICL_IRQ
static void sm5703_set_aicl_irq(struct sm5703_charger_data *charger,
			int mask)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0;

	mutex_lock(&charger->io_lock);
	data = sm5703_reg_read(i2c, SM5703_INTMSK1);
	data &= 0xFE;

	if (mask)
		data |= 0x01;

	sm5703_reg_write(i2c, SM5703_INTMSK1, data);

	data = sm5703_reg_read(i2c, SM5703_INTMSK1);
	pr_info("%s : SM5703_INTMSK1 (AICH-MASK) : 0x%02x, mask : %d\n",
			__func__, data, mask);
	mutex_unlock(&charger->io_lock);
}
#endif

static void sm5703_set_freqsel(struct sm5703_charger_data *charger,
		int freqsel_hz)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0;

	mutex_lock(&charger->io_lock);
	data = sm5703_reg_read(i2c, SM5703_CHGCNTL6);
	data &= ~SM5703_FREQSEL_MASK;
	data |= (freqsel_hz << SM5703_FREQSEL_SHIFT);

	sm5703_reg_write(i2c, SM5703_CHGCNTL6, data);

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL6);
	pr_info("%s : SM5703_CHGCNTL6 (FREQSEL) : 0x%02x\n",
		__func__, data);
	mutex_unlock(&charger->io_lock);
}

static void sm5703_set_input_current_limit(struct sm5703_charger_data *charger,
		int current_limit)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0, temp = 0;

	mutex_lock(&charger->io_lock);
	data = sm5703_reg_read(i2c, SM5703_VBUSCNTL);
	data &= ~SM5703_VBUSLIMIT;

	if (charger->siop_level < 100 && current_limit >= SIOP_INPUT_LIMIT_CURRENT)
		current_limit = SIOP_INPUT_LIMIT_CURRENT;

	if (current_limit >= 2100)
		current_limit = 2100;

	if (charger->current_max < current_limit && charger->is_current_reduced) {
		pr_info("%s: skip set input current limit(%d <--> %d)\n",
			__func__, charger->current_max, current_limit);
	} else {
		if (current_limit > 100) {
			temp = ((current_limit - 100) / 50) | data;
			sm5703_reg_write(i2c, SM5703_VBUSCNTL, temp);
		}

		data = sm5703_reg_read(i2c, SM5703_VBUSCNTL);
		pr_info("%s : SM5703_VBUSCNTL (Input current limit) : 0x%02x\n",
				__func__, data);
		if (charger->pdata->chg_vbuslimit
#if EN_AICL_IRQ
			/* check aicl state */
			&& (sm5703_reg_read(i2c, SM5703_STATUS1) & 0x01)
#endif
			) {
			/* start vbuslimit work */
			wake_lock(&charger->vbuslimit_wake_lock);
			queue_delayed_work_on(0, charger->wq,
				&charger->vbuslimit_work, msecs_to_jiffies(START_VBUSLIMIT_DELAY));
		}
	}

	mutex_unlock(&charger->io_lock);
}

static int sm5703_get_input_current_limit(struct i2c_client *i2c)
{
	int ret, current_limit = 0;
	ret = sm5703_reg_read(i2c, SM5703_VBUSCNTL);
	if (ret < 0)
		return ret;
	ret&=SM5703_VBUSLIMIT_MASK;
	current_limit = (100 + (ret*50));

	return current_limit;
}

static void sm5703_set_regulation_voltage(struct sm5703_charger_data *charger,
		int float_voltage)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0;

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL3);

	data &= ~SM5703_BATREG_MASK;

	if ((float_voltage) <= 4120)
		data = 0x00;
	else if ((float_voltage) >= 4430)
		data = 0x1f;
	else
		data = ((float_voltage - 4120) / 10);

	mutex_lock(&charger->io_lock);
	sm5703_reg_write(i2c, SM5703_CHGCNTL3, data);
	data = sm5703_reg_read(i2c, SM5703_CHGCNTL3);
	pr_info("%s : SM5703_CHGCNTL3 (Battery regulation voltage) : 0x%02x\n",
		__func__, data);
	mutex_unlock(&charger->io_lock);
}

#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
static int sm5703_get_regulation_voltage(struct sm5703_charger_data *charger)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0;

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL3);
	data &= SM5703_BATREG_MASK;

	return (4120 + (data * 10));
}
#endif

static void __sm5703_set_fast_charging_current(struct i2c_client *i2c,
		int charging_current)
{
	int data = 0;

	if(charging_current <= 100)
		charging_current = 100;
	else if (charging_current >= 2500)
		charging_current = 2500;

	data = (charging_current - 100) / 50;

	sm5703_reg_write(i2c, SM5703_CHGCNTL2, data);

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL2);
	pr_info("%s : SM5703_CHGCNTL2 (fastchg current) : 0x%02x\n",
			__func__, data);
}

static int sm5703_get_fast_charging_current(struct i2c_client *i2c)
{
	int data = sm5703_reg_read(i2c, SM5703_CHGCNTL2);
	int charging_current = 0;

	if (data < 0)
		return data;

	data &= SM5703_FASTCHG_MASK;
	charging_current = (100 + (data*50));

	return charging_current;
}

static int sm5703_get_current_topoff_setting(struct sm5703_charger_data *charger)
{
	int ret, data = 0, topoff_current = 0;
	mutex_lock(&charger->io_lock);
	ret = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_CHGCNTL4);
	mutex_unlock(&charger->io_lock);
	if (ret < 0) {
		pr_info("%s: warning --> fail to read i2c register(%d)\n", __func__, ret);
		return ret;
	}

	data = ((ret & SM5703_TOPOFF_MASK) >> SM5703_TOPOFF_SHIFT);
	topoff_current = (100 + (data*25));

	return topoff_current;
}

static void __sm5703_set_termination_current_limit(struct i2c_client *i2c,
		int current_limit)
{
	int data = 0, temp = 0;

	pr_info("%s : Set Termination\n", __func__);

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL4);
	data &= ~SM5703_TOPOFF_MASK;

	if(current_limit <= 100)
		current_limit = 100;
	else if (current_limit >= 475)
		current_limit = 475;

	temp = (current_limit - 100) / 25;
	data |= (temp << SM5703_TOPOFF_SHIFT);

	sm5703_reg_write(i2c, SM5703_CHGCNTL4, data);

	data = sm5703_reg_read(i2c, SM5703_CHGCNTL4);
	pr_info("%s : SM5703_CHGCNTL4 (Top-off current threshold) : 0x%02x\n",
		__func__, data);
}

static void sm5703_set_charging_current(struct sm5703_charger_data *charger, int topoff)
{
	int adj_current = 0;
#ifndef CONFIG_DISABLE_MINIMUM_SIOP_CHARGING
	const int usb_charging_current = charger->pdata->charging_current_table[
			POWER_SUPPLY_TYPE_USB].fast_charging_current;
#endif

	adj_current = charger->charging_current * charger->siop_level / 100;

#ifndef CONFIG_DISABLE_MINIMUM_SIOP_CHARGING
	if (adj_current > 0 && adj_current < usb_charging_current)
		adj_current = usb_charging_current;
#endif

#if CONFIG_SIOP_CHARGING_LIMIT_CURRENT
	if(charger->siop_level < 100 && adj_current > CONFIG_SIOP_CHARGING_LIMIT_CURRENT)
		adj_current = CONFIG_SIOP_CHARGING_LIMIT_CURRENT;
#endif
	pr_info("%s adj_current = %dmA charger->siop_level = %d\n",__func__, adj_current,charger->siop_level);
	mutex_lock(&charger->io_lock);
	__sm5703_set_fast_charging_current(charger->sm5703->i2c_client,
					   adj_current);

	__sm5703_set_termination_current_limit(
			charger->sm5703->i2c_client, topoff);
	mutex_unlock(&charger->io_lock);
}

static void sm5703_set_otgcurrent(struct sm5703_charger_data *charger,
		int otg_current)
{
	struct i2c_client *i2c = charger->sm5703->i2c_client;
	int data = 0;

	data = sm5703_reg_read(i2c, SM5703_OTGCURRENTCNTL);
	data &= ~SM5703_OTGCURRENT_MASK;

	if (otg_current <= 500)
		data = 0x00;
	else if (otg_current <= 700)
		data = 0x01;
	else if (otg_current <= 900)
		data = 0x02;
	else
		data = 0x3;

	mutex_lock(&charger->io_lock);
	sm5703_reg_write(i2c, SM5703_OTGCURRENTCNTL, data);
	data = sm5703_reg_read(i2c, SM5703_OTGCURRENTCNTL);
	pr_info("%s : SM5703_OTGCURRENTCNTL (OTG current) : 0x%02x\n",
			__func__, data);
	mutex_unlock(&charger->io_lock);
}

static void sm5703_set_bst_iq3limit(struct sm5703_charger_data *charger,
		int iq3limit)
{
	int data = 0;

	mutex_lock(&charger->io_lock);
	data = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_Q3LIMITCNTL);
	data &= ~SM5703_BST_IQ3LIMIT_MASK;
    data |= (iq3limit << SM5703_BST_IQ3LIMIT_SHIFT);

	sm5703_reg_write(charger->sm5703->i2c_client, SM5703_Q3LIMITCNTL, data);

	data = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_Q3LIMITCNTL);
	pr_info("%s : SM5703_Q3LIMITCNTL (BST_IQ3LIMIT) : 0x%02x\n",
		__func__, data);
	mutex_unlock(&charger->io_lock);
}

#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
enum {
	SM5703_AICL_4300MV = 0,
	SM5703_AICL_4400MV,
	SM5703_AICL_4500MV,
	SM5703_AICL_4600MV,
	SM5703_AICL_4700MV,
	SM5703_AICL_4800MV,
	SM5703_AICL_4900MV,
};

#if ENABLE_AICL
/* Dedicated charger (non-USB) device
 * will use lower AICL level to get better performance
 */
static void sm5703_set_aicl_level(struct sm5703_charger_data *charger)
{
	int aicl;
	switch(charger->cable_type) {
	case POWER_SUPPLY_TYPE_USB ... POWER_SUPPLY_TYPE_USB_ACA:
		aicl = SM5703_AICL_4500MV;
		break;
	default:
		aicl = SM5703_AICL_4500MV;
	}
	mutex_lock(&charger->io_lock);
	sm5703_assign_bits(charger->sm5703->i2c_client,
			SM5703_CHGCNTL5, SM5703_AICLTH_MASK, aicl);
	mutex_unlock(&charger->io_lock);
}
#endif /*ENABLE_AICL*/

static void sm5703_configure_charger(struct sm5703_charger_data *charger)
{
	int topoff;
	union power_supply_propval val;
	union power_supply_propval soc_val;
	int ldo_val, led_mode = 0;
	int float_val;

	pr_info("%s : Set config charging\n", __func__);
	if (charger->charging_current < 0) {
		pr_info("%s : OTG is activated. Ignore command!\n", __func__);
		return;
	}

	ldo_val = sm5703_reg_read(charger->sm5703->i2c_client,SM5703_LDOOUT1CNTL);
	float_val = sm5703_get_regulation_voltage(charger);

#if ENABLE_AICL
	sm5703_set_aicl_level(charger);
#endif /*DISABLE_AICL*/
	psy_do_property("battery", get,
			POWER_SUPPLY_PROP_CHARGE_NOW, val);

	/* Input current limit */
	pr_info("%s : input current (%dmA)\n",
			__func__, charger->pdata->charging_current_table
				[charger->cable_type].input_current_limit);

	soc_val.intval = SEC_BATTEY_VOLTAGE_OCV;
	psy_do_property("sm5703-fuelgauge", get,POWER_SUPPLY_PROP_VOLTAGE_AVG, soc_val);//4200mV : 88~89% : 500mA
#ifdef CONFIG_FLED_SM5703
	if (charger->fled_info == NULL)
		charger->fled_info = sm_fled_get_info_by_name(NULL);
	if (charger->fled_info)
		led_mode = charger->fled_info->flashlight_dev->props.mode;

	pr_info("%s : led_mode = %d\n", __func__, led_mode);
#endif /* CONFIG_FLED_SM5703 */

	pr_info("%s : inval = %d\n", __func__, soc_val.intval);
	if (float_val > 0x08) // swelling_mode == false : Over 4.2V BATREG
	{
		if ((soc_val.intval >= LIMIT_VOLTAGE_STEP1) && (ldo_val & (1<<3)))//Over 4200mV : ENLDOOUT1 enable
		{
			if (led_mode == FLASHLIGHT_MODE_TORCH)
			{
				sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP1+200);
				pr_info("%s : input current (%dmA) -> (%dmA)\n",
						__func__, charger->pdata->charging_current_table
							[charger->cable_type].input_current_limit, LIMIT_CURRENT_STEP1+200);
			}
			else
			{
			sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP1);
			pr_info("%s : input current (%dmA) -> (%dmA)\n",
					__func__, charger->pdata->charging_current_table
						[charger->cable_type].input_current_limit, LIMIT_CURRENT_STEP1);
			}
		}
		else if ((soc_val.intval >= LIMIT_VOLTAGE_STEP2) && (ldo_val & (1<<3)))//Over 4100mV : ENLDOOUT1 enable
		{
			sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP2);
			pr_info("%s : input current (%dmA) -> (%dmA)\n",
					__func__, charger->pdata->charging_current_table
						[charger->cable_type].input_current_limit, LIMIT_CURRENT_STEP2);
		}
		else {
			sm5703_set_input_current_limit(charger,
					charger->pdata->charging_current_table
						[charger->cable_type].input_current_limit);
		}
	}
#if defined(CONFIG_BATTERY_SWELLING)
	else // swelling_mode == true
	{
		if ((soc_val.intval >= LIMIT_SWELLING_VOLTAGE) && (ldo_val & (1<<3)))//Over 3980mV : ENLDOOUT1 enable
		{
			if (led_mode == FLASHLIGHT_MODE_TORCH)
			{
				sm5703_set_input_current_limit(charger, LIMIT_SWELLING_CURRENT+200);
				pr_info("%s : input current (%dmA) -> (%dmA)\n",
						__func__, charger->pdata->charging_current_table
							[charger->cable_type].input_current_limit, LIMIT_SWELLING_CURRENT+200);
			}
			else
			{
				sm5703_set_input_current_limit(charger, LIMIT_SWELLING_CURRENT);
				pr_info("%s : input current (%dmA) -> (%dmA)\n",
						__func__, charger->pdata->charging_current_table
							[charger->cable_type].input_current_limit, LIMIT_SWELLING_CURRENT);
			}
		}
		else
		{
			sm5703_set_input_current_limit(charger,
					charger->pdata->charging_current_table
						[charger->cable_type].input_current_limit);
		}
	}
#else
	else {
		sm5703_set_input_current_limit(charger,
				charger->pdata->charging_current_table
					[charger->cable_type].input_current_limit);
	}
#endif
	/* Float voltage */
	pr_info("%s : float voltage (%dmV)\n",
			__func__, charger->pdata->chg_float_voltage);

	sm5703_set_regulation_voltage(charger,
			charger->pdata->chg_float_voltage);

	charger->charging_current = charger->pdata->charging_current_table
			[charger->cable_type].fast_charging_current;
	topoff = charger->pdata->charging_current_table
			[charger->cable_type].full_check_current_1st;
	/* Fast charge and Termination current */
	pr_info("%s : fast charging current (%dmA)\n",
			__func__, charger->charging_current);

	pr_info("%s : termination current (%dmA)\n",
			__func__, topoff);

	sm5703_set_charging_current(charger, topoff);//Fastcharging/Topoff Current
	sm5703_enable_charger_switch(charger, 1); //Charging Enable/Disable.
}
#else
static void sm5703_configure_charger(struct sm5703_charger_data *charger)
{
	int topoff;
	union power_supply_propval val, chg_now;
	int full_check_type;

	pr_info("%s : Set config charging\n", __func__);
	if (charger->charging_current < 0) {
		pr_info("%s : OTG is activated. Ignore command!\n",
				__func__);
		return;
	}

	psy_do_property("battery", get,
			POWER_SUPPLY_PROP_CHARGE_NOW, val);

	/* Input current limit */
	pr_info("%s : input current (%dmA)\n",
			__func__, charger->pdata->charging_current_table
				[charger->cable_type].input_current_limit);

	sm5703_set_input_current_limit(charger,
			charger->pdata->charging_current_table
				[charger->cable_type].input_current_limit);

	/* Float voltage */
	pr_info("%s : float voltage (%dmV)\n",
			__func__, charger->pdata->chg_float_voltage);
	sm5703_set_regulation_voltage(charger,
			charger->pdata->chg_float_voltage);

	/* Fast charge and Termination current */
	charger->charging_current = charger->pdata->charging_current_table
		[charger->cable_type].fast_charging_current;
	topoff = charger->pdata->charging_current_table
		[charger->cable_type].full_check_current_1st;
	psy_do_property("battery", get,
			POWER_SUPPLY_PROP_CHARGE_NOW, chg_now);

	if (chg_now.intval == SEC_BATTERY_CHARGING_1ST)
		full_check_type = charger->pdata->full_check_type;
	else
		full_check_type = charger->pdata->full_check_type_2nd;

	switch (full_check_type) {
		case SEC_BATTERY_FULLCHARGED_CHGPSY:
			if (chg_now.intval == SEC_BATTERY_CHARGING_1ST) {
				pr_info("%s : termination current (%dmA)\n",
						__func__, charger->pdata->charging_current_table[
						charger->cable_type].full_check_current_1st);

				/** Setting 1st termination current as charger termination current*/
				topoff = charger->pdata->charging_current_table
					[charger->cable_type].full_check_current_1st;
			} else {
				pr_info("%s : termination current (%dmA)\n",
						__func__, charger->pdata->charging_current_table[
						charger->cable_type].full_check_current_2nd);

				if (sm5703_get_charging_status(charger) == POWER_SUPPLY_STATUS_FULL) {
					sm5703_enable_charger_switch(charger, 0);
					charger->charging_current = charger->pdata->charging_current_table
						[charger->cable_type].fast_charging_current;
				}
				/** Setting 2nd termination current as new charger termination current*/
				topoff = charger->pdata->charging_current_table
					[charger->cable_type].full_check_current_2nd;
			}
			break;
	}
	pr_info("%s : fast charging current (%dmA), topoff current (%dmA)\n",
			__func__, charger->charging_current, topoff);

	sm5703_set_charging_current(charger, topoff);
	sm5703_enable_charger_switch(charger, 1);

}
#endif

int sm5703_chg_fled_init(struct i2c_client *client)
{
	int ret = 0;//, rev_id;
#if 0
	sm5703_mfd_chip_t *chip = i2c_get_clientdata(client);
	struct sm5703_charger_data *charger = chip->charger;
#endif
	return ret;
}
EXPORT_SYMBOL(sm5703_chg_fled_init);

/* here is set init charger data */
static bool sm5703_chg_init(struct sm5703_charger_data *charger)
{
	sm5703_mfd_chip_t *chip = i2c_get_clientdata(charger->sm5703->i2c_client);
	chip->charger = charger;
	sm5703_chg_fled_init(charger->sm5703->i2c_client);
	//int data = 0;
	charger->full_charged = false;

	/* AUTOSTOP */
	sm5703_enable_autostop(chip->charger, (int)charger->pdata->chg_autostop);
	/* AUTOSET */
	sm5703_enable_autoset(chip->charger, (int)charger->pdata->chg_autoset);
	/* AICLEN */
	sm5703_enable_aiclen(chip->charger, (int)charger->pdata->chg_aiclen);
	/* AICLTH */
	sm5703_set_aiclth(chip->charger, (int)charger->pdata->chg_aiclth);
	/* FREQSEL */
	sm5703_set_freqsel(chip->charger, SM5703_FREQSEL_1P5MHZ);

	/* Auto-Stop configuration for Emergency status */
	__sm5703_set_termination_current_limit(charger->sm5703->i2c_client, 300);
	sm5703_CHG_set_TOPOFF_TMR(charger, SM5703_TOPOFF_TIMER_45m);

	/* MUST set correct regulation voltage first
	 * Before MUIC pass cable type information to charger
	 * charger would be already enabled (default setting)
	 * it might cause EOC event by incorrect regulation voltage */
	sm5703_set_regulation_voltage(charger,
			charger->pdata->chg_float_voltage);

	sm5703_set_otgcurrent(charger, 1200); /* OTGCURRENT : 1.2A */

	sm5703_set_bst_iq3limit(charger, SM5703_BST_IQ3LIMIT_1X);

	sm5703_test_read(charger->sm5703->i2c_client);

	return true;
}


static int sm5703_get_charging_status(struct sm5703_charger_data *charger)
{
	int status = POWER_SUPPLY_STATUS_UNKNOWN;
	int chg_status3,chg_status5;
	int nCHG = 0;

	chg_status3 = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS3);
	if (chg_status3<0) {
		pr_info("Error : SM5703_STATUS3 can't get charging status (%d)\n", chg_status3);
	}
	pr_info("%s chg_status3 = %d \n",__func__, chg_status3);

	chg_status5 = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS5);
	if (chg_status5<0) {
		pr_info("Error : SM5703_STATUS5 can't get charging status (%d)\n", chg_status5);
	}

	pr_info("%s charger->full_charged = %d, charger->cable_type = %d \n",__func__,charger->full_charged,charger->cable_type);

	nCHG = gpio_get_value(charger->pdata->chgen_gpio);

	if(((chg_status3 & SM5703_STATUS3_DONE) || (chg_status3 & SM5703_STATUS3_TOPOFF))
		&& (chg_status5 & SM5703_STATUS5_VBUSOK)
		&& (charger->cable_type != POWER_SUPPLY_TYPE_OTG)
		&& (charger->cable_type != POWER_SUPPLY_TYPE_POWER_SHARING))
	{
		status = POWER_SUPPLY_STATUS_FULL;
		charger->full_charged = true;
		pr_info("%s : Status, Power Supply Full \n", __func__);
	}else if(chg_status3 & SM5703_STATUS3_CHGON){
		status = POWER_SUPPLY_STATUS_CHARGING;
	}else {
		if (nCHG)
			status = POWER_SUPPLY_STATUS_DISCHARGING;
		else
			status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	}

	/* TEMP_TEST : when OTG is enabled(charging_current -1), handle OTG func. */
	if (charger->charging_current < 0) {
		int chg_status1;
		/* For OTG mode, SM5703 would still report "charging" */
		status = POWER_SUPPLY_STATUS_DISCHARGING;
		chg_status1 = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS1);

		if (chg_status1 & SM5703_STATUS1_OTGFAIL) {
			pr_info("%s: otg overcurrent limit\n", __func__);

			sm5703_charger_otg_control(charger, false);
		}
	}

	return status;
}

static int sm5703_get_charging_health(struct sm5703_charger_data *charger)
{
	int vbus_status = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS5);
	int health = POWER_SUPPLY_HEALTH_GOOD;

	pr_info("%s : charger->is_charging = %d, charger->cable_type = %d, is_current_reduced = %d\n",
		__func__, charger->is_charging, charger->cable_type, charger->is_current_reduced);

	// temp for test
	pr_info("%s : vbus_status = %d\n", __func__, vbus_status);

	if (vbus_status < 0) {
		health = POWER_SUPPLY_HEALTH_UNKNOWN;
		pr_info("%s : Health : %d, vbus_status : %d\n", __func__, health,vbus_status);

		return (int)health;
	}

	if (vbus_status & SM5703_STATUS5_VBUSOK)
		health = POWER_SUPPLY_HEALTH_GOOD;
	else if (vbus_status & SM5703_STATUS5_VBUSOVP)
		health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
	else if (vbus_status & SM5703_STATUS5_VBUSUVLO)
		health = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
	else
		health = POWER_SUPPLY_HEALTH_UNKNOWN;

	pr_info("%s : Health : %d\n", __func__, health);

	return (int)health;
}

static int sec_chg_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int chg_curr, aicr, vbus_status;
	int data = 0;

	struct sm5703_charger_data *charger =
		container_of(psy, struct sm5703_charger_data, psy_chg);

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			vbus_status = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS5);
			if (charger->cable_type != POWER_SUPPLY_TYPE_BATTERY &&
				!(vbus_status & SM5703_STATUS5_VBUSOK))
					charger->cable_type = POWER_SUPPLY_TYPE_BATTERY;

			val->intval = charger->cable_type;
			pr_info("%s: Charger Cable type : %d\n", __func__, charger->cable_type);
			break;
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = sm5703_get_charging_status(charger);
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = sm5703_get_charging_health(charger);
			break;
		case POWER_SUPPLY_PROP_CURRENT_MAX:
			sm5703_test_read(charger->sm5703->i2c_client);
			val->intval = sm5703_get_fast_charging_current(charger->sm5703->i2c_client);
			break;
		case POWER_SUPPLY_PROP_CURRENT_AVG:
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			if (charger->charging_current) {
				aicr = sm5703_get_input_current_limit(charger->sm5703->i2c_client);
				chg_curr = sm5703_get_fast_charging_current(charger->sm5703->i2c_client);
				val->intval = MINVAL(aicr, chg_curr);
			} else
				val->intval = 0;
			break;
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		case POWER_SUPPLY_PROP_VOLTAGE_MAX:
			val->intval = sm5703_get_regulation_voltage(charger);
		break;
#endif
		case POWER_SUPPLY_PROP_CHARGE_TYPE:
			if (!charger->is_charging || charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
				val->intval = POWER_SUPPLY_CHARGE_TYPE_NONE;
			} else if (charger->current_max <= SLOW_CHARGING_CURRENT_STANDARD) {
				val->intval = POWER_SUPPLY_CHARGE_TYPE_SLOW;
				pr_info("%s: slow-charging mode\n", __func__);
			} else
				val->intval = POWER_SUPPLY_CHARGE_TYPE_FAST;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = 0;
			data = sm5703_reg_read(charger->sm5703->i2c_client, SM5703_STATUS2);
			if((data & (1 << 4)) == 0x0)
				val->intval = 1;
			pr_info("%s: batt_present : %d\n", __func__, val->intval);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int sec_chg_set_property(struct power_supply *psy,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{
	struct sm5703_charger_data *charger =
		container_of(psy, struct sm5703_charger_data, psy_chg);

	int topoff;
	union power_supply_propval value;
	int previous_cable_type = charger->cable_type;
#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
	union power_supply_propval soc_val;
	struct power_supply *psy_soc;
	int ldo_val, led_mode = 0;
	int float_val;
#endif

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			charger->status = val->intval;
			break;
			/* val->intval : type */
		case POWER_SUPPLY_PROP_ONLINE:
			charger->cable_type = val->intval;
			if (previous_cable_type != charger->cable_type) {
				charger->current_max = charger->pdata->charging_current_table
						[charger->cable_type].input_current_limit;
				charger->charging_current = charger->pdata->charging_current_table
						[charger->cable_type].fast_charging_current;

				charger->is_current_reduced = false;
				if (charger->pdata->chg_vbuslimit) {
					wake_unlock(&charger->vbuslimit_wake_lock);
					cancel_delayed_work(&charger->vbuslimit_work);
				}
#if EN_AICL_IRQ
				sm5703_set_aicl_irq(charger, 0);
#endif
			}

			if (val->intval == POWER_SUPPLY_TYPE_POWER_SHARING) {
				psy_do_property("ps", get,
						POWER_SUPPLY_PROP_STATUS, value);

				sm5703_charger_otg_control(charger, value.intval);
			} else if (charger->cable_type == POWER_SUPPLY_TYPE_BATTERY) {
				pr_info("%s:[BATT] Type Battery\n", __func__);
				/* sm5703_enable_charger_switch(charger, 0); */
				if (previous_cable_type == POWER_SUPPLY_TYPE_OTG)
					sm5703_charger_otg_control(charger, false);
				else
					sm5703_enable_charger_switch(charger, 0);
				/* set default input current */
				charger->current_max = charger->pdata->charging_current_table
						[POWER_SUPPLY_TYPE_USB].input_current_limit;
				sm5703_set_input_current_limit(charger, charger->current_max);
			} else if (charger->cable_type == POWER_SUPPLY_TYPE_OTG) {
				pr_info("%s: OTG mode\n", __func__);
				sm5703_charger_otg_control(charger, true);
				charger->full_charged = false;
			} else {
				pr_info("%s:[BATT] Set charging"
					", Cable type = %d\n", __func__, charger->cable_type);
				/* Enable charger */
				sm5703_configure_charger(charger);
			}
#if EN_TEST_READ
		//msleep(100);
		sm5703_test_read(charger->sm5703->i2c_client);
#endif
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
#if defined(CONFIG_BATTERY_SWELLING)
		if (val->intval > charger->pdata->charging_current_table
			[charger->cable_type].fast_charging_current) {
			break;
		}
#endif
			topoff = sm5703_get_current_topoff_setting(charger);
			pr_info("%s:Set chg current = %d mA, topoff = %d mA\n", __func__,
					val->intval, topoff);
			charger->charging_current = val->intval;
			sm5703_set_charging_current(charger, topoff);
			break;
		case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
			/* decrease the charging current according to siop level */
			charger->siop_level = val->intval;
			pr_info("%s:SIOP level = %d, chg current = %d\n", __func__,
					val->intval, charger->charging_current);
			if(charger->is_charging) {
				sm5703_configure_charger(charger);
			}
			break;
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			/* set charging current */
			if (charger->is_charging) {
				sm5703_configure_charger(charger);
			}
			break;
		case POWER_SUPPLY_PROP_POWER_NOW:
			topoff = sm5703_get_current_topoff_setting(charger);
			pr_info("%s:Set Power Now -> chg current = %d mA, topoff = %d mA\n", __func__,
					val->intval, topoff);
			sm5703_set_charging_current(charger, topoff);
			break;
#if defined(CONFIG_BATTERY_SWELLING) || defined(CONFIG_BATTERY_SWELLING_SELF_DISCHARGING)
		case POWER_SUPPLY_PROP_VOLTAGE_MAX:
			pr_info("%s: float voltage(%d)\n", __func__, val->intval);
			charger->pdata->chg_float_voltage = val->intval;
			sm5703_set_regulation_voltage(charger, val->intval);
			break;
#endif
#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
	case POWER_SUPPLY_PROP_INPUT_CURRENT_MAX:
		psy_soc = get_power_supply_by_name("battery");
		ldo_val = sm5703_reg_read(charger->sm5703->i2c_client,SM5703_LDOOUT1CNTL);
		float_val = sm5703_get_regulation_voltage(charger);

#ifdef CONFIG_FLED_SM5703
		if (charger->fled_info == NULL)
			charger->fled_info = sm_fled_get_info_by_name(NULL);
		if (charger->fled_info)
			led_mode = charger->fled_info->flashlight_dev->props.mode;

		pr_info("%s : led_mode = %d\n", __func__, led_mode);
#endif /* CONFIG_FLED_SM5703 */
		if (psy_soc)
		{
			pr_info("%s : input current psy_soc = !NULL\n", __func__);
			soc_val.intval = SEC_BATTEY_VOLTAGE_OCV;
			psy_do_property("sm5703-fuelgauge", get,
					POWER_SUPPLY_PROP_VOLTAGE_AVG, soc_val);//4200mV : 88~89% : 500mA

			pr_info("%s : soc_val.intval = %d, ldo1 = %d, val->intval = %d\n", __func__, soc_val.intval, (ldo_val & (1<<3)), val->intval);

			if (float_val > 0x08) // swelling_mode == false, 0x08 = 4.2V
			{
				if ((soc_val.intval >= LIMIT_VOLTAGE_STEP1) && (ldo_val & (1<<3)))//OCV Over 4200mV : ENLDOOUT1 enable
				{
					if (led_mode == FLASHLIGHT_MODE_TORCH)
					{
						sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP1+200);
						pr_info("%s : ocv >= 4200 input current (%dmA)\n", __func__, LIMIT_CURRENT_STEP1+200);
					}
					else
				{
					sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP1);
					pr_info("%s : ocv >= 4200 input current (%dmA)\n", __func__, LIMIT_CURRENT_STEP1);
				}
				}
				else if ((soc_val.intval >= LIMIT_VOLTAGE_STEP2) && (ldo_val & (1<<3)))//Over 4100mV : ENLDOOUT1 enable
				{
					sm5703_set_input_current_limit(charger, LIMIT_CURRENT_STEP2);
					pr_info("%s : input current (%dmA) -> (%dmA)\n",
							__func__, charger->pdata->charging_current_table
								[charger->cable_type].input_current_limit, LIMIT_CURRENT_STEP2);
				}
				else
				{
					sm5703_set_input_current_limit(charger,
							charger->pdata->charging_current_table
								[charger->cable_type].input_current_limit);
					pr_info("%s : input current (%dmA)\n", __func__, charger->pdata->charging_current_table[charger->cable_type].input_current_limit);
				}
			}
#if defined(CONFIG_BATTERY_SWELLING)
			else // swelling_mode == true
			{
				if ((soc_val.intval >= LIMIT_SWELLING_VOLTAGE) && (ldo_val & (1<<3)))//ENLDOOUT1 enable : OCV Over 3980mV
				{
					if (led_mode == FLASHLIGHT_MODE_TORCH)
					{
						sm5703_set_input_current_limit(charger, LIMIT_SWELLING_CURRENT+200);
						pr_info("%s : ocv >= 3980 input current (%dmA)\n", __func__, LIMIT_SWELLING_CURRENT+200);
					}
					else
					{
						sm5703_set_input_current_limit(charger, LIMIT_SWELLING_CURRENT);
						pr_info("%s : ocv >= 3980 input current (%dmA)\n", __func__, LIMIT_SWELLING_CURRENT);
					}
				}
				else
				{
					sm5703_set_input_current_limit(charger,
							charger->pdata->charging_current_table
								[charger->cable_type].input_current_limit);
					pr_info("%s : input current (%dmA)\n", __func__, charger->pdata->charging_current_table[charger->cable_type].input_current_limit);
				}
			}
#else
			else
			{
				sm5703_set_input_current_limit(charger,
						charger->pdata->charging_current_table
							[charger->cable_type].input_current_limit);
				pr_info("%s : input current (%dmA)\n", __func__, charger->pdata->charging_current_table[charger->cable_type].input_current_limit);
			}
#endif /* CONFIG_BATTERY_SWELLING */
		}
		else
			pr_info("%s : input current psy_soc = NULL\n", __func__);
		break;
#endif
	case POWER_SUPPLY_PROP_HEALTH:
        //charger->ovp = val->intval;
        break;
	default:
		return -EINVAL;
	}

	return 0;
}

ssize_t sm5703_chg_show_attrs(struct device *dev,
		const ptrdiff_t offset, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sm5703_charger_data *charger =
		container_of(psy, struct sm5703_charger_data, psy_chg);
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

		sm5703_read_regs(charger->sm5703->i2c_client, str);
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

ssize_t sm5703_chg_store_attrs(struct device *dev,
		const ptrdiff_t offset,
		const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sm5703_charger_data *charger =
		container_of(psy, struct sm5703_charger_data, psy_chg);

	int ret = 0;
	int x = 0;
	uint8_t data = 0;

	switch (offset) {
	case CHG_REG:
		if (sscanf(buf, "%x\n", &x) == 1) {
			charger->reg_addr = x;
			data = sm5703_reg_read(charger->sm5703->i2c_client,
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
			ret = sm5703_reg_write(charger->sm5703->i2c_client,
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

struct sm5703_chg_irq_handler {
	char *name;
	int irq_index;
	irqreturn_t (*handler)(int irq, void *data);
};
#if EN_NOBAT_IRQ
static irqreturn_t sm5703_chg_nobat_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *info = data;
	struct i2c_client *iic = info->sm5703->i2c_client;

	/* set full charged flag
	 * until TA/USB unplug event / stop charging by PSY
	 */

	pr_info("%s : Nobat\n", __func__);

#if EN_TEST_READ
	sm5703_test_read(iic);
#endif

	return IRQ_HANDLED;
}
#endif /*EN_NOBAT_IRQ*/

#if EN_DONE_IRQ
static irqreturn_t sm5703_chg_done_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *info = data;
	struct i2c_client *iic = info->sm5703->i2c_client;

	/* set full charged flag
	 * until TA/USB unplug event / stop charging by PSY
	 */

	pr_info("%s : Full charged(done)\n", __func__);
	info->full_charged = true;

	/* nCHG pin toggle */
	gpio_direction_output(info->pdata->chgen_gpio, info->is_charging);
	msleep(10);
	gpio_direction_output(info->pdata->chgen_gpio, !(info->is_charging));

#if EN_TEST_READ
	sm5703_test_read(iic);
#endif

	return IRQ_HANDLED;
}
#endif/*EN_DONE_IRQ*/

#if EN_TOPOFF_IRQ
static irqreturn_t sm5703_chg_topoff_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *info = data;
	struct i2c_client *iic = info->sm5703->i2c_client;

	/* set full charged flag
	 * until TA/USB unplug event / stop charging by PSY
	 */

	 pr_info("%s : Full charged(topoff)\n", __func__);
	 info->full_charged = true;

#if EN_TEST_READ
	sm5703_test_read(iic);
#endif

	return IRQ_HANDLED;
}
#endif /*EN_TOPOFF_IRQ*/

#if EN_CHGON_IRQ
static irqreturn_t sm5703_chg_chgon_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *info = data;
	struct i2c_client *iic = info->sm5703->i2c_client;

	 pr_info("%s : Chgon\n", __func__);

#if EN_TEST_READ
	sm5703_test_read(iic);
#endif

	return IRQ_HANDLED;
}
#endif /*EN_CHGON_IRQ*/

#if EN_OTGFAIL_IRQ
static irqreturn_t sm5703_chg_otgfail_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *info = data;
	/* struct i2c_client *iic = info->sm5703->i2c_client; */
	int ret;

#ifdef CONFIG_USB_HOST_NOTIFY
	struct otg_notify *o_notify;

	o_notify = get_otg_notify();
#endif
	pr_info("%s : OTG Failed\n", __func__);

	ret = sm5703_reg_read(info->sm5703->i2c_client, SM5703_STATUS1);
	if (ret & SM5703_STATUS1_OTGFAIL) {
		pr_info("%s: otg overcurrent limit\n", __func__);
#ifdef CONFIG_USB_HOST_NOTIFY
		send_otg_notify(o_notify, NOTIFY_EVENT_OVERCURRENT, 0);
#endif
		sm5703_charger_otg_control(info, false);
	}

	return IRQ_HANDLED;
}
#endif /*EN_CHGON_IRQ*/

static void sm5703_chg_vbuslimit_work(struct work_struct *work)
{
	struct sm5703_charger_data *charger =
			container_of(work, struct sm5703_charger_data, vbuslimit_work.work);
	struct i2c_client *i2c = charger->sm5703->i2c_client;

	if (charger->cable_type != POWER_SUPPLY_TYPE_BATTERY) {
		int vbuslimit_state;

		vbuslimit_state = sm5703_reg_read(i2c, SM5703_STATUS1) & 0x08;
		if (vbuslimit_state || (charger->current_max <= MINIMUM_INPUT_CURRENT)) {
			/* check slow charging */
			if (charger->is_current_reduced &&
				charger->current_max <= SLOW_CHARGING_CURRENT_STANDARD) {
				union power_supply_propval value;
				psy_do_property("battery", set,
					POWER_SUPPLY_PROP_CHARGE_TYPE, value);
				pr_info("%s: slow charging on : input current(%dmA), cable type(%d)\n",
					__func__, charger->current_max, charger->cable_type);
			}
			wake_unlock(&charger->vbuslimit_wake_lock);
		} else {
			/* reduce input current & restart vbuslimit work */
			int reg_data, temp;

			mutex_lock(&charger->io_lock);
			charger->is_current_reduced = true;

			charger->current_max -= REDUCE_CURRENT_STEP;
			reg_data = sm5703_reg_read(i2c, SM5703_VBUSCNTL);
			reg_data &= ~SM5703_VBUSLIMIT;
			temp = ((charger->current_max - 100) / 50) | reg_data;
			sm5703_reg_write(i2c, SM5703_VBUSCNTL, temp);
			pr_info("%s: reduce input current(%d)\n", __func__, charger->current_max);
			mutex_unlock(&charger->io_lock);

			queue_delayed_work_on(0, charger->wq,
				&charger->vbuslimit_work, msecs_to_jiffies(VBUSLIMIT_DELAY));
		}
		pr_info("%s: vbuslimit state(%d)\n", __func__, vbuslimit_state);
	} else {
		wake_unlock(&charger->vbuslimit_wake_lock);
	}
}

#if EN_VBUSLIMIT_IRQ
static irqreturn_t sm5703_chg_vbuslimit_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *charger = data;
	struct i2c_client *i2c = charger->sm5703->i2c_client;

	pr_info("%s: VBUS Limit\n", __func__);

#if EN_TEST_READ
	sm5703_test_read(i2c);
#endif

	return IRQ_HANDLED;
}
#endif /* EN_VBUSLIMIT_IRQ */

#if EN_AICL_IRQ
static irqreturn_t sm5703_chg_aicl_irq_handler(int irq, void *data)
{
	struct sm5703_charger_data *charger = data;
	struct i2c_client *i2c = charger->sm5703->i2c_client;

	pr_info("%s: AICL\n", __func__);

	sm5703_set_aicl_irq(charger, 1);

	if (charger->pdata->chg_vbuslimit &&
		charger->cable_type != POWER_SUPPLY_TYPE_BATTERY) {
		/* start vbuslimit work */
		wake_lock(&charger->vbuslimit_wake_lock);
		queue_delayed_work_on(0, charger->wq,
			&charger->vbuslimit_work, msecs_to_jiffies(START_VBUSLIMIT_DELAY));
	}

#if EN_TEST_READ
	sm5703_test_read(i2c);
#endif

	return IRQ_HANDLED;
}
#endif /* EN_AICL_IRQ */

const struct sm5703_chg_irq_handler sm5703_chg_irq_handlers[] = {
#if EN_NOBAT_IRQ
	{
		.name = "NOBAT",
		.handler = sm5703_chg_nobat_irq_handler,
		.irq_index = SM5703_NOBAT_IRQ,
	},
#endif /*EN_NOBAT_IRQ*/
#if EN_DONE_IRQ
    {
		.name = "DONE",
		.handler = sm5703_chg_done_irq_handler,
		.irq_index = SM5703_DONE_IRQ,
	},
#endif/*EN_DONE_IRQ*/
#if EN_TOPOFF_IRQ
    {
		.name = "TOPOFF",
		.handler = sm5703_chg_topoff_irq_handler,
		.irq_index = SM5703_TOPOFF_IRQ,
	},
#endif /*EN_TOPOFF_IRQ*/
#if EN_CHGON_IRQ
    {
        .name = "CHGON",
        .handler = sm5703_chg_chgon_irq_handler,
        .irq_index = SM5703_CHGON_IRQ,
    },
#endif /*EN_CHGON_IRQ*/
#if EN_OTGFAIL_IRQ
	{
		.name = "OTGFAIL",
		.handler = sm5703_chg_otgfail_irq_handler,
		.irq_index = SM5703_OTGFAIL_IRQ,
	},
#endif /* EN_OTGFAIL_IRQ */
#if EN_VBUSLIMIT_IRQ
	{
		.name = "VBUSLIMIT",
		.handler = sm5703_chg_vbuslimit_irq_handler,
		.irq_index = SM5703_VBUSLIMIT_IRQ,
	},
#endif /* EN_VBUSLIMIT_IRQ */
#if EN_AICL_IRQ
	{
		.name = "AICL",
		.handler = sm5703_chg_aicl_irq_handler,
		.irq_index = SM5703_AICL_IRQ,
	},
#endif /* EN_AICL_IRQ */
};


static int register_irq(struct platform_device *pdev,
		struct sm5703_charger_data *info)
{
	int irq;
	int i, j;
	int ret;
	const struct sm5703_chg_irq_handler *irq_handler = sm5703_chg_irq_handlers;
	const char *irq_name;
	for (i = 0; i < ARRAY_SIZE(sm5703_chg_irq_handlers); i++) {
		irq_name = sm5703_get_irq_name_by_index(irq_handler[i].irq_index);
		irq = platform_get_irq_byname(pdev, irq_name);
		ret = request_threaded_irq(irq, NULL, irq_handler[i].handler,
					   IRQF_ONESHOT | IRQF_TRIGGER_FALLING |
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
		irq_name = sm5703_get_irq_name_by_index(irq_handler[j].irq_index);
		irq = platform_get_irq_byname(pdev, irq_name);
		free_irq(irq, info);
	}

	return ret;
}

static void unregister_irq(struct platform_device *pdev,
		struct sm5703_charger_data *info)
{
	int irq;
	int i;
	const char *irq_name;
	const struct sm5703_chg_irq_handler *irq_handler = sm5703_chg_irq_handlers;

	for (i = 0; i < ARRAY_SIZE(sm5703_chg_irq_handlers); i++) {
		irq_name = sm5703_get_irq_name_by_index(irq_handler[i].irq_index);
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

static int sm5703_charger_parse_dt(struct device *dev,
                           struct sm5703_charger_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	const u32 *p;
	int ret, i, len;

	// chg_autostop
	ret = of_property_read_u32(np, "chg_autostop",
			&pdata->chg_autostop);
	if (ret < 0) {
		pr_info("%s : cannot get chg autostop\n", __func__);
		return -ENODATA;
	}
	// chg_autoset
	ret = of_property_read_u32(np, "chg_autoset",
			&pdata->chg_autoset);
	if (ret < 0) {
		pr_info("%s : cannot get chg autoset\n", __func__);
            return -ENODATA;
	}
	// chg_aiclen
	ret = of_property_read_u32(np, "chg_aiclen",
			&pdata->chg_aiclen);
	if (ret < 0) {
		pr_info("%s : cannot get chg aiclen\n", __func__);
		return -ENODATA;
	}
	// chg_aiclth
	ret = of_property_read_u32(np, "chg_aiclth",
			&pdata->chg_aiclth);
	if (ret < 0) {
		pr_info("%s : cannot get chg aiclth\n", __func__);
		pdata->chg_aiclth = 4500;
		return -ENODATA;
	}

	// fg_vol_val
	ret = of_property_read_u32(np, "fg_vol_val",
			&pdata->fg_vol_val);
	if (ret < 0) {
		pr_info("%s : cannot get fg_vol_val\n", __func__);
		return -ENODATA;
	}

	// fg_soc_val
	ret = of_property_read_u32(np, "fg_soc_val",
			&pdata->fg_soc_val);
	if (ret < 0) {
		pr_info("%s : cannot get fg_soc_val\n", __func__);
		return -ENODATA;
	}

	// fg_curr_avr_val
	ret = of_property_read_u32(np, "fg_curr_avr_val",
			&pdata->fg_curr_avr_val);
	if (ret < 0) {
		pr_info("%s : cannot get fg_curr_avr_val\n", __func__);
		return -ENODATA;
	}

	np = of_find_node_by_name(NULL, "charger");
	if (!np) {
		pr_info("%s : np NULL\n", __func__);
		return -ENODATA;
	}
	// battery,chg_float_voltage
	ret = of_property_read_u32(np, "battery,chg_float_voltage",
			&pdata->chg_float_voltage);
	if (ret < 0) {
		pr_info("%s : cannot get chg float voltage\n", __func__);
		return -ENODATA;
	}
	ret = of_property_read_u32(np, "battery,chg_vbuslimit", &pdata->chg_vbuslimit);
	if (ret < 0) {
		pr_info("%s : cannot get chg vbuslimit\n", __func__);
		pdata->chg_vbuslimit = 0;
	}

	ret = of_property_read_u32(np, "battery,full_check_type",
			&pdata->full_check_type);
	pr_info("%s full_check_type: %d\n", __func__, pdata->full_check_type);
	if (ret < 0)
		pr_err("%s error reading battery,full_check_type %d\n", __func__, ret);

	ret = of_property_read_u32(np, "battery,full_check_type_2nd",
			&pdata->full_check_type_2nd);
	pr_info("%s full_check_type_2nd: %d\n", __func__, pdata->full_check_type_2nd);
	if (ret < 0)
		pr_err("%s error reading battery,full_check_type_2nd %d\n", __func__, ret);

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

	pdata->chgen_gpio = of_get_named_gpio(np, "battery,chg_gpio_en",0); //nCHGEN
	pr_info("%s: SM5703_parse_dt chgen: %d\n", __func__, pdata->chgen_gpio);

	dev_info(dev,"sm5703 charger parse dt retval = %d\n", ret);
	return ret;
}

static struct of_device_id sm5703_charger_match_table[] = {
	{ .compatible = "siliconmitus,sm5703-charger",},
	{},
};
#else
static int sm5703_charger_parse_dt(struct device *dev,
                           struct sm5703_charger_platform_data *pdata)
{
    return -ENOSYS;
}
#define sm5703_charger_match_table NULL
#endif /* CONFIG_OF */

static int sm5703_charger_probe(struct platform_device *pdev)
{
	sm5703_mfd_chip_t *chip = dev_get_drvdata(pdev->dev.parent);
	struct sm5703_mfd_platform_data *mfd_pdata = dev_get_platdata(chip->dev);
	struct sm5703_charger_data *charger;
	int ret = 0;

	pr_info("%s:[BATT] SM5703 Charger driver probe..0x%x\n", __func__, (unsigned int)mfd_pdata);

	charger = kzalloc(sizeof(*charger), GFP_KERNEL);
	if (!charger)
		return -ENOMEM;

#ifdef CONFIG_OF
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	if (pdev->dev.parent->of_node) {
		pdev->dev.of_node = of_find_compatible_node(
			of_node_get(pdev->dev.parent->of_node), NULL,
			sm5703_charger_match_table[0].compatible);
	}
#endif
#endif
	mutex_init(&charger->io_lock);
	charger->wq = create_workqueue("sm5703chg_workqueue");
	charger->sm5703= chip;
	//if (pdev->dev.of_node)
	if(1)
	{
		charger->pdata = devm_kzalloc(&pdev->dev, sizeof(*(charger->pdata)), GFP_KERNEL);
		if (!charger->pdata) {
			dev_err(&pdev->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_parse_dt_nomem;
		}
		ret = sm5703_charger_parse_dt(&pdev->dev, charger->pdata);
		if (ret < 0)
            goto err_parse_dt;
	} else
        charger->pdata = mfd_pdata->charger_platform_data;

	platform_set_drvdata(pdev, charger);

	charger->psy_chg.name           = "sm5703-charger";
	charger->psy_chg.type           = POWER_SUPPLY_TYPE_UNKNOWN;
	charger->psy_chg.get_property   = sec_chg_get_property;
	charger->psy_chg.set_property   = sec_chg_set_property;
	charger->psy_chg.properties     = sec_charger_props;
	charger->psy_chg.num_properties = ARRAY_SIZE(sec_charger_props);

	charger->siop_level = 100;
	charger->ovp = 0;
	sm5703_chg_init(charger);

	if (charger->pdata->chg_vbuslimit) {
		INIT_DELAYED_WORK(&charger->vbuslimit_work, sm5703_chg_vbuslimit_work);
		wake_lock_init(&charger->vbuslimit_wake_lock, WAKE_LOCK_SUSPEND, "sm5703-vbuslimit");
	}

	ret = power_supply_register(&pdev->dev, &charger->psy_chg);
	if (ret) {
		pr_err("%s: Failed to Register psy_chg\n", __func__);
		goto err_power_supply_register;
	}
	ret = register_irq(pdev, charger);
	if (ret < 0)
        goto err_reg_irq;

	ret = gpio_request(charger->pdata->chgen_gpio, "sm5703_nCHGEN");
	if (ret) {
		pr_info("%s : Request GPIO %d failed\n",
		       __func__, (int)charger->pdata->chgen_gpio);
    }

	sm5703_test_read(charger->sm5703->i2c_client);
	pr_info("%s:[BATT] SM5703 charger driver loaded OK\n", __func__);

	return 0;
err_reg_irq:
    power_supply_unregister(&charger->psy_chg);
err_power_supply_register:
	if (charger->pdata->chg_vbuslimit) {
		wake_lock_destroy(&charger->vbuslimit_wake_lock);
	}
err_parse_dt:
err_parse_dt_nomem:
	destroy_workqueue(charger->wq);
	mutex_destroy(&charger->io_lock);
	kfree(charger);
	return ret;
}

static int sm5703_charger_remove(struct platform_device *pdev)
{
	struct sm5703_charger_data *charger =
	platform_get_drvdata(pdev);
	unregister_irq(pdev, charger);
	power_supply_unregister(&charger->psy_chg);
	destroy_workqueue(charger->wq);
	if (charger->pdata->chg_vbuslimit) {
		wake_lock_destroy(&charger->vbuslimit_wake_lock);
	}
	mutex_destroy(&charger->io_lock);
	kfree(charger);
	return 0;
}

#if defined CONFIG_PM
static int sm5703_charger_suspend(struct device *dev)
{
	return 0;
}

static int sm5703_charger_resume(struct device *dev)
{
	return 0;
}
#else
#define sm5703_charger_suspend NULL
#define sm5703_charger_resume NULL
#endif

static void sm5703_charger_shutdown(struct device *dev)
{
	pr_info("%s: SM5703 Charger driver shutdown\n", __func__);
}

static SIMPLE_DEV_PM_OPS(sm5703_charger_pm_ops, sm5703_charger_suspend,
		sm5703_charger_resume);

static struct platform_driver sm5703_charger_driver = {
	.driver		= {
		.name	= "sm5703-charger",
		.owner	= THIS_MODULE,
		.of_match_table = sm5703_charger_match_table,
		.pm 	= &sm5703_charger_pm_ops,
		.shutdown = sm5703_charger_shutdown,
	},
	.probe		= sm5703_charger_probe,
	.remove		= sm5703_charger_remove,
};

static int __init sm5703_charger_init(void)
{
	int ret = 0;

	pr_info("%s \n", __func__);
	ret = platform_driver_register(&sm5703_charger_driver);

	return ret;
}

subsys_initcall(sm5703_charger_init);

static void __exit sm5703_charger_exit(void)
{
	platform_driver_unregister(&sm5703_charger_driver);
}
module_exit(sm5703_charger_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Charger driver for SM5703");
