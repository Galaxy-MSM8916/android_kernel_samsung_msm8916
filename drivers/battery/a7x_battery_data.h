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
	{26007, 900},
	{26223, 850},
	{26475, 800},
	{26803, 750},
	{27160, 700},
	{27555, 650},
	{28065, 600},
	{28626, 550},
	{29276, 500},
	{30019, 450},
	{30860, 400},
	{31783, 350},
	{32781, 300},
	{33828, 250},
	{34885, 200},
	{35957, 150},
	{37004, 100},
	{37989, 50},
	{38875, 0},
	{39641, -50},
	{40473, -100},
	{41082, -150},
	{41576, -200},
};

static sec_bat_adc_table_data_t chg_temp_table[] = {
	{26007, 900},
	{26223, 850},
	{26475, 800},
	{26803, 750},
	{27160, 700},
	{27555, 650},
	{28065, 600},
	{28626, 550},
	{29276, 500},
	{30019, 450},
	{30860, 400},
	{31783, 350},
	{32781, 300},
	{33828, 250},
	{34885, 200},
	{35957, 150},
	{37004, 100},
	{37989, 50},
	{38875, 0},
	{39641, -50},
	{40473, -100},
	{41082, -150},
	{41576, -200},
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

#define TEMP_HIGH_THRESHOLD_EVENT  580
#define TEMP_HIGH_RECOVERY_EVENT   530
#define TEMP_LOW_THRESHOLD_EVENT   (-50)
#define TEMP_LOW_RECOVERY_EVENT    0
#define TEMP_HIGH_THRESHOLD_NORMAL 580
#define TEMP_HIGH_RECOVERY_NORMAL  530
#define TEMP_LOW_THRESHOLD_NORMAL  (-50)
#define TEMP_LOW_RECOVERY_NORMAL   0
#define TEMP_HIGH_THRESHOLD_LPM    580
#define TEMP_HIGH_RECOVERY_LPM     530
#define TEMP_LOW_THRESHOLD_LPM     (-50)
#define TEMP_LOW_RECOVERY_LPM      0

#endif /* __SEC_BATTERY_DATA_H */
