/*
 * drivers/battery/rt5033_fuelgauge-impl.h
 *
 * Header of Richtek RT5033 Fuelgauge Driver Implementation
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Richtek Gauge Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef RT5033_FUELGAUGE_IMPL_H
#define RT5033_FUELGAUGE_IMPL_H

/* Definitions of RT5033 Fuelgauge Registers */

#define RT5033_OCV_MSB				0x00
#define RT5033_OCV_LSB				0x01
#define RT5033_VBAT_MSB				0x02
#define RT5033_VBAT_LSB				0x03
#define RT5033_SOC_MSB				0x04
#define RT5033_SOC_LSB				0x05
#define RT5033_CRATE					0x08
#define RT5033_DEVICE_ID			0x09
#define RT5033_AVG_VOLT_MSB		0x0A
#define RT5033_AVG_VOLT_LSB		0x0B
#define RT5033_CONFIG_MSB			0x0C
#define RT5033_CONFIG_LSB			0x0D
#define RT5033_FG_IRQ_CTRL		0x10
#define RT5033_FG_IRQ_FLAG		0x11
#define RT5033_VOLT_ALRT_TH		0x12
#define RT5033_SOC_ALRT_TH		0x13
#define RT5033_VGCOMP1				0x20
#define RT5033_VGCOMP2				0x21
#define RT5033_VGCOMP3				0x22
#define RT5033_VGCOMP4				0x23
#define RT5033_MFA_MSB				0xFE
#define RT5033_MFA_LSB				0xFF

#define EOC_LOCK_TH					99			/* unit : % */
#define VALRT_LOCK_TH			1				/* unit : % */
#define SMOOTH_SOC_STEP		1				/* unit : % */
#define CHG_IR_EXCEPT_SOC		10				/* unit : % */
#define QUICK_ROUTINE_TIME		1			/* unit : sec */
#define NORMAL_ROUTINE_TIME 30			/* unit : sec */
#define MFA_CMD_EOC_SOC		0x8664
#define MFA_CMD_VALRT_SOC		0x8600
#define SOC_ALRT_TOP				12
#define SOC_ALRT_HIGH				6
#define SOC_ALRT_LOW				3
#define SOC_ALRT_BOTTOM			1
#define BAT_REMOVAL_SOC			50
#define ROUNDUP_SOC_TH			5
#define VALRT_LOCK_TH				1
#define MFA_CMD_EXIT_HIB    0x7400
#define MFA_CMD_ENTRY_HIB		0x74AA
#define FULL_SOC						100

#define VOLT_ALRT_TH				0 			//disable		/* unit : 20mV*/
#define VALRT_SOC						0				/* unit : % */


#define GAIN_RANGE1		-20
#define GAIN_RANGE2		5
#define GAIN_RANGE3		25
#define GAIN_RANGE4		45
#define GAIN_TABLE1_INDEX	1
#define GAIN_TABLE2_INDEX	2
#define GAIN_TABLE3_INDEX	3
#define GAIN_TABLE4_INDEX	4

#define OFFS_THRES_DSG_N20	-20
#define OFFS_THRES_CHG_25	25
#define OFFS_THRES_DSG_25	25
#define OFFS_TABLE1_INDEX	1
#define OFFS_TABLE2_INDEX	2
#define OFFS_TABLE3_INDEX	3
#define OFFS_TABLE4_INDEX	4

#define LOOKUP_GAIN	1
#define LOOKUP_OFFS	2

#define	R5_TEMP								10			/* unit : oC */
#define MFA_CMD_R5_DEFAULT		0x8501
#define MFA_CMD_R5_LOW_TEMP		0x8503

#define	ENABLE_SOC_OFFSET_COMP		0
#define	ENABLE_LOCK_SOC				0
#define	ENABLE_SMOOTH_SOC			0
#define	ENABLE_SOC_IRREVERSIBLE	0

#endif // RT5033_FUELGAUGE_IMPL_H
