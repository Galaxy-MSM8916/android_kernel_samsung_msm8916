/*
 * drivers/battery/rt5033_charger.h
 *
 * Header of Richtek RT5033 Fuelgauge Driver
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef RT5033_CHARGER_H
#define RT5033_CHARGER_H
#include <linux/mfd/rt5033.h>
#include <linux/mfd/rt5033_irq.h>

#define RT5033_CHG_STAT_CTRL	0x00
#define RT5033_CHG_CTRL1			0x01
#define RT5033_CHG_CTRL2			0x02
#define RT5033_CHG_CTRL3			0x04
#define RT5033_CHG_CTRL4			0x05
#define RT5033_CHG_CTRL5			0x06
#define RT5033_EOC_CTRL				0x07
#define RT5033_CHG_RESET			0x08
#define RT5033_UUG					0x19
#define RT5033_CONFIG_H				0x0C
#define RT5033_CONFIG_L				0x0D


#define RT5033_CHGENB_MASK  (1 << 0)
#define RT5033_OPAMODE_MASK (1 << 0)
#define RT5033_COF_EN_MASK  (1 << 6)
#define RT5033_TIMEREN_MASK (1 << 0)
#define RT5033_SEL_SWFREQ_MASK   (1 << 2)
#define RT5033_EOC_RESET_MASK   (1<<4)
#define RT5033_TR_HIGH_GM   (1<<6)
#define RT5033_TEEN_MASK    (1 << 3)
#define RT5033_AICR_LIMIT_MASK (0x7 << 5)
#define RT5033_AICR_LIMIT_SHIFT 5
#define RT5033_MIVR_MASK (0x7 << 5)
#define RT5033_MIVR_SHIFT 5
#define RT5033_HZ_MASK  (1 << 1)
#define RT5033_VOREG_MASK (0x3f << 2)
#define RT5033_VOREG_SHIFT 2
#define RT5033_IEOC_MASK 0x07
#define RT5033_IEOC_SHIFT 0
#define RT5033_ICHRG_MASK 0xf0
#define RT5033_ICHRG_SHIFT 4

#define RT5033_CHG_IRQ1				0x60
#define RT5033_CHG_IRQ2				0x61
#define RT5033_CHG_IRQ3				0x62

#define RT5033_CHG_IRQ_CTRL1		0x63
#define RT5033_CHG_IRQ_CTRL2		0x64
#define RT5033_CHG_IRQ_CTRL3		0x65

/* RT5033_CHG_STAT */
#define RT5033_EXT_PMOS_CTRL				0x1
#define RT5033_EXT_PMOS_CTRL_SHIFT			7
#define RT5033SW_HW_CTRL					0x1
#define RT5033SW_HW_CTRL_SHIFT				2
#define RT5033_OTG_SS_DISABLE				0x1
#define RT5033_OTG_SS_DISABLE_SHIFT			1

/* RT5033_CHR_CTRL1 */
#define RT5033_IAICR						0x101
#define RT5033_IAICR_SHIFT					5
#define RT5033_HIGHER_OCP					0x1
#define RT5033_HIGHER_OCP_SHIFT				4
#define RT5033_TERMINATION_EN				0x0
#define RT5033_TERMINATION_EN_SHIFT			3
#define RT5033_SEL_SWFREQ					0x1
#define RT5033_SEL_SWFREQ_SHIFT				2
#define RT5033_HIGH_IMPEDANCE				0
#define RT5033_HIGH_IMPEDANCE_SHIFT			1
#define RT5033_OPA_MODE						2
#define RT5033_OPA_MODE_SHIFT				0

/* RT5033_CHG_CTRL2 */
#define RT5033_REG_VOLTAGE					0x011100
#define RT5033_REG_VOLTAGE_SHIFT			2
#define RT5033_TDEG_EOC						0x0
#define RT5033_TDEG_EOC_SHIFT				0

/* RT5033_CHG_CTRL3 */
#define RT5033_PPC_TE						0x0
#define RT5033_PPC_TE_SHIFT					7
#define RT5033_CHG_EN						0x1
#define RT5033_CHG_EN_SHIFT					6



extern sec_battery_platform_data_t sec_battery_pdata;


#endif /*RT5033_CHARGER_H*/
