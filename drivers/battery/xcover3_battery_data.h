/*
 * battery_data.h
 * Samsung Mobile Battery data Header
 *
 *
 * Copyright (C) 2014 Samsung Electronics, Inc.
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
#ifndef __SEC_BATTERY_DATA_H
#define __SEC_BATTERY_DATA_H __FILE__

#define CAPACITY_MAX			1000
#define CAPACITY_MAX_MARGIN		70
#define CAPACITY_MIN			0

static sec_bat_adc_table_data_t temp_table[] = {
	{26514, 800},
	{26820, 750},
	{27182, 700},
	{27610, 650},
	{28088, 600},
	{28661, 550},
	{29317, 500},
	{30072, 450},
	{30921, 400},
	{31852, 350},
	{32856, 300},
	{33903, 250},
	{34998, 200},
	{36081, 150},
	{37148, 100},
	{38134, 50},
	{39043, 0},
	{39854, -50},
	{40547, -100},
	{41129, -150},
	{41604, -200},
};

#if defined(CONFIG_BATTERY_SWELLING)
#define BATT_SWELLING_HIGH_TEMP_BLOCK		500
#define BATT_SWELLING_HIGH_TEMP_RECOV		450
#define BATT_SWELLING_LOW_TEMP_BLOCK		50
#define BATT_SWELLING_LOW_TEMP_RECOV		100
#define BATT_SWELLING_RECHG_VOLTAGE		4150
#endif

#define TEMP_HIGHLIMIT_THRESHOLD	800
#define TEMP_HIGHLIMIT_RECOVERY		700

#define TEMP_HIGH_THRESHOLD_EVENT  610
#define TEMP_HIGH_RECOVERY_EVENT   550
#define TEMP_LOW_THRESHOLD_EVENT   (-50)
#define TEMP_LOW_RECOVERY_EVENT    0
#define TEMP_HIGH_THRESHOLD_NORMAL 610
#define TEMP_HIGH_RECOVERY_NORMAL  550
#define TEMP_LOW_THRESHOLD_NORMAL  (-50)
#define TEMP_LOW_RECOVERY_NORMAL   0
#define TEMP_HIGH_THRESHOLD_LPM    610
#define TEMP_HIGH_RECOVERY_LPM     550
#define TEMP_LOW_THRESHOLD_LPM     (-50)
#define TEMP_LOW_RECOVERY_LPM      0

#endif /* __SEC_BATTERY_DATA_H */
