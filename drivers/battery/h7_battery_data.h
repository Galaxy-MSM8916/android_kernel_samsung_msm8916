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

static sec_bat_adc_table_data_t temp_table[] = {
	{25954, 900},
	{26005, 890},
	{26052, 880},
	{26105, 870},
	{26151, 860},
	{26207, 850},
	{26253, 840},
	{26302, 830},
	{26354, 820},
	{26405, 810},
	{26454, 800},
	{26503, 790},
	{26554, 780},
	{26602, 770},
	{26657, 760},
	{26691, 750},
	{26757, 740},
	{26823, 730},
	{26889, 720},
	{26955, 710},
	{27020, 700},
	{27081, 690},
	{27142, 680},
	{27203, 670},
	{27264, 660},
	{27327, 650},
	{28040, 600},
	{28644, 550},
	{29297, 500},
	{30124, 450},
	{30891, 400},
	{31706, 350},
	{32818, 300},
	{33873, 250},
	{34934, 200},
	{36036, 150},
	{37098, 100},
	{38080, 50},
	{38670, 0},
	{39848, -50},
	{40541, -100},
	{46563, -150},
	{46689, -200},
};

#if defined(CONFIG_BATTERY_SWELLING)
#define BATT_SWELLING_HIGH_TEMP_BLOCK		500
#define BATT_SWELLING_HIGH_TEMP_RECOV		450
#define BATT_SWELLING_LOW_TEMP_BLOCK		100
#define BATT_SWELLING_LOW_TEMP_RECOV		150
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
