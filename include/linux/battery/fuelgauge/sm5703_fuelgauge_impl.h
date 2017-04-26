/*
 * drivers/battery/sm5703_fuelgauge-impl.h
 *
 * Header of SiliconMitus SM5703 Fuelgauge Driver Implementation
 *
 * Copyright (C) 2015 SiliconMitus
 * Author: SW Jung
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef SM5703_FUELGAUGE_IMPL_H
#define SM5703_FUELGAUGE_IMPL_H

/* Definitions of SM5703 Fuelgauge Registers */
// I2C Register
#define SM5703_REG_DEVICE_ID                 0x00
#define SM5703_REG_CNTL                      0x01
#define SM5703_REG_INTFG                     0x02
#define SM5703_REG_INTFG_MASK                0x03
#define SM5703_REG_STATUS                    0x04
#define SM5703_REG_SOC                       0x05
#define SM5703_REG_OCV                       0x06
#define SM5703_REG_VOLTAGE                   0x07
#define SM5703_REG_CURRENT                   0x08
#define SM5703_REG_TEMPERATURE               0x09
#define SM5703_REG_FG_OP_STATUS              0x10
#define SM5703_REG_PARAM_RUN_UPDATE          0x14
#define SM5703_REG_IOCV_MAN                  0x2E
#define SM5703_REG_END_V_IDX                 0x2F

#define SM5703_REG_V_ALARM                   0x0C
#define SM5703_REG_T_ALARM                   0x0D
#define SM5703_REG_SOC_ALARM                 0x0E

#define SM5703_REG_TOPOFFSOC                 0x12
#define SM5703_REG_PARAM_CTRL                0x13
#define SM5703_REG_PARAM_RUN_UPDATE          0x14
#define SM5703_REG_VIT_PERIOD                0x1A
#define SM5703_REG_MIX_RATE                  0x1B
#define SM5703_REG_MIX_INIT_BLANK            0x1C

//for cal
#define SM5703_REG_VOLT_CAL         0x2B
#define SM5703_REG_CURR_CAL         0x2C

#define SM5703_REG_RCE0             0x20
#define SM5703_REG_RCE1             0x21
#define SM5703_REG_RCE2             0x22
#define SM5703_REG_DTCD             0x23
#define SM5703_REG_RS               0x24

#define SM5703_REG_RS_MIX_FACTOR    0x25
#define SM5703_REG_RS_MAX           0x26
#define SM5703_REG_RS_MIN           0x27
#define SM5703_REG_RS_MAN           0x29

#define SM5703_REG_RS_TUNE          0x28
#define SM5703_REG_CURRENT_EST      0x85
#define SM5703_REG_CURRENT_ERR      0x86
#define SM5703_REG_Q_EST            0x87

//start reg addr for table
#define SM5703_REG_TABLE_START      0xA0

#define SM5703_REG_IOCV_B_L_MIN		0x30
#define SM5703_REG_IOCV_B_L_MAX		0x37
#define SM5703_REG_IOCV_B_S_MIN		0x40
#define SM5703_REG_IOCV_B_S_MAX		0x43

#define SM5703_REG_MISC			0x90

#define SM5703_FG_PARAM_UNLOCK_CODE     0x3700
#define SM5703_FG_PARAM_LOCK_CODE     0x0000
#define SM5703_FG_TABLE_LEN    0xF//real table length -1


#define SW_RESET_CODE		0x0008
#define RS_MAN_CNTL             0x0800

// control register value
#define ENABLE_MIX_MODE         0x8000
#define ENABLE_TEMP_MEASURE     0x4000
#define ENABLE_TOPOFF_SOC       0x2000
#define ENABLE_MANUAL_OCV       0x0400
#define ENABLE_RS_MAN_MODE      0x0800

#define ENABLE_SOC_ALARM        0x0008
#define ENABLE_T_H_ALARM        0x0004
#define ENABLE_T_L_ALARM        0x0002
#define ENABLE_V_ALARM          0x0001

#define CNTL_REG_DEFAULT_VALUE  0x2008
#define INIT_CHECK_MASK         0x00FF
#define DISABLE_RE_INIT         0x0007

#define TOPOFF_SOC_100   0x111
#define TOPOFF_SOC_99    0x110
#define TOPOFF_SOC_98    0x101
#define TOPOFF_SOC_97    0x100
#define TOPOFF_SOC_96    0x011
#define TOPOFF_SOC_95    0x010
#define TOPOFF_SOC_94    0x001
#define TOPOFF_SOC_93    0x000

#define MASK_L_SOC_INT  0x0008
#define MASK_H_TEM_INT  0x0004
#define MASK_L_TEM_INT  0x0002
#define MASK_L_VOL_INT  0x0001

#define FULL_SOC						100

#endif // SM5703_FUELGAUGE_IMPL_H
