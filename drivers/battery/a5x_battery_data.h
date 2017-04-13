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
	{27600, 650},
	{28096, 600},
	{28682, 550},
	{29372, 500},
	{30093, 450},
	{30968, 400},
	{31872, 350},
	{32828, 300},
	{33801, 250},
	{34860, 200},
	{35944, 150},
	{36992, 100},
	{37985, 50},
	{38772, 0},
	{39455, -50},
	{40343, -100},
	{40976, -150},
	{41464, -200},
};

static sec_bat_adc_table_data_t chg_temp_table[] = {
	{26007, 900},
	{26223, 850},
	{26475, 800},
	{26803, 750},
	{27160, 700},
	{27600, 650},
	{28096, 600},
	{28682, 550},
	{29372, 500},
	{30093, 450},
	{30968, 400},
	{31872, 350},
	{32828, 300},
	{33801, 250},
	{34860, 200},
	{35944, 150},
	{36992, 100},
	{37985, 50},
	{38772, 0},
	{39455, -50},
	{40343, -100},
	{40976, -150},
	{41464, -200},
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
