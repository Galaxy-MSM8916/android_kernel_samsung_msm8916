/*
 * Copyright (C) 2010 Samsung Electronics
 * Hyoyoung Kim <hyway.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef __SM5705_H__
#define __SM5705_H__

#include <linux/muic/muic.h>

#define MUIC_DEV_NAME   "muic-sm5705"

/* sm5705 muic register read/write related information defines. */

/* Slave addr(8bit) = 0x4A: MUIC */

/* SM5705 I2C registers */
enum sm5705_muic_reg {
    SM5705_MUIC_REG_DEVID             = 0x01,
    SM5705_MUIC_REG_CTRL              = 0x02,
    SM5705_MUIC_REG_INT1              = 0x03,
    SM5705_MUIC_REG_INT2              = 0x04,
    SM5705_MUIC_REG_INT3_AFC          = 0x05,
    SM5705_MUIC_REG_INTMASK1          = 0x06,
    SM5705_MUIC_REG_INTMASK2          = 0x07,
    SM5705_MUIC_REG_INTMASK3_AFC      = 0x08,
    SM5705_MUIC_REG_ADC               = 0x09,
    SM5705_MUIC_REG_DEV_T1            = 0x0A,
    SM5705_MUIC_REG_DEV_T2            = 0x0B,
    SM5705_MUIC_REG_DEV_T3            = 0x0C,
    SM5705_MUIC_REG_TIMING1           = 0x0D,
    SM5705_MUIC_REG_TIMING2           = 0x0E,
    SM5705_MUIC_REG_BUTTON1           = 0x10,
    SM5705_MUIC_REG_BUTTON2           = 0x11,
    SM5705_MUIC_REG_CARKIT_STATUS     = 0x12,
    SM5705_MUIC_REG_MANSW1            = 0x13,
    SM5705_MUIC_REG_MANSW2            = 0x14,
    SM5705_MUIC_REG_VBUS_VALID        = 0x15,
    SM5705_MUIC_REG_RESERVED_ID2      = 0x16,
    SM5705_MUIC_REG_CHG_TYPE          = 0x17,
    SM5705_MUIC_REG_AFC_CTRL          = 0x18,
    SM5705_MUIC_REG_AFC_TXD           = 0x19,
    SM5705_MUIC_REG_AFC_STATUS        = 0x1A,
    SM5705_MUIC_REG_AFC_VBUS_STATUS   = 0x1B,
    SM5705_MUIC_REG_AFC_RXD1          = 0x1C,
    SM5705_MUIC_REG_AFC_RXD2          = 0x1D,
    SM5705_MUIC_REG_AFC_RXD3          = 0x1E,
    SM5705_MUIC_REG_AFC_RXD4          = 0x1F,
    SM5705_MUIC_REG_AFC_RXD5          = 0x20,
    SM5705_MUIC_REG_AFC_RXD6          = 0x21,
    SM5705_MUIC_REG_RESET             = 0x22,    
    SM5705_MUIC_REG_END,
};

/* SM5705 REGISTER ENABLE or DISABLE bit */
#define SM5705_ENABLE_BIT     1
#define SM5705_DISABLE_BIT    0

/* SM5705 Control register */
#define CTRL_SWITCH_OPEN_SHIFT      4
#define CTRL_RAW_DATA_SHIFT         3
#define CTRL_MANUAL_SW_SHIFT        2
#define CTRL_WAIT_SHIFT             1
#define CTRL_MASK_INT_SHIFT         0
#define CTRL_SWITCH_OPEN_MASK      (1 << CTRL_SWITCH_OPEN_SHIFT)
#define CTRL_RAW_DATA_MASK         (1 << CTRL_RAW_DATA_SHIFT)
#define CTRL_MANUAL_SW_MASK        (1 << CTRL_MANUAL_SW_SHIFT)
#define CTRL_WAIT_MASK             (1 << CTRL_WAIT_SHIFT)
#define CTRL_MASK_INT_MASK         (1 << CTRL_MASK_INT_SHIFT)
#define CTRL_MASK                  (CTRL_SWITCH_OPEN_MASK | CTRL_RAW_DATA_MASK | CTRL_MANUAL_SW_MASK | CTRL_WAIT_MASK )

/* SM5705 Interrupt 1 register */
#define INT1_LKR_SHIFT           4
#define INT1_LKP_SHIFT           3
#define INT1_KP_SHIFT            2
#define INT1_DETACH_SHIFT        1
#define INT1_ATTACH_SHIFT        0

#define INT1_LKR_MASK            (1 << INT1_LKR_SHIFT)
#define INT1_LKP_MASK            (1 << INT1_LKP_SHIFT)
#define INT1_KP_MASK             (1 << INT1_KP_SHIFT)
#define INT1_DETACH_MASK         (1 << INT1_DETACH_SHIFT)
#define INT1_ATTACH_MASK         (1 << INT1_ATTACH_SHIFT)

/* SM5705 Interrupt 2 register */
#define INT2_VBUSDET_ON_SHIFT        7
#define INT2_RID_CHARGER_SHIFT       6
#define INT2_MHL_SHIFT               5
#define INT2_STUCK_KEY_RCV_SHIFT     4
#define INT2_STUCK_KEY_SHIFT         3
#define INT2_ADC_CHANGE_SHIFT        2
#define INT2_RSRV_ATTACH_SHIFT       1
#define INT2_VBUS_OFF_SHIFT          0

#define INT2_VBUSDET_ON_MASK         (1 << INT2_VBUSDET_ON_SHIFT)
#define INT2_RID_CHARGER_MASK        (1 << INT2_RID_CHARGER_SHIFT)
#define INT2_MHL_MASK                (1 << INT2_MHL_SHIFT)
#define INT2_STUCK_KEY_RCV_MASK      (1 << INT2_STUCK_KEY_RCV_SHIFT)
#define INT2_STUCK_KEY_MASK          (1 << INT2_STUCK_KEY_SHIFT)
#define INT2_ADC_CHANGE_MASK         (1 << INT2_ADC_CHANGE_SHIFT)
#define INT2_RSRV_ATTACH_MASK        (1 << INT2_RSRV_ATTACH_SHIFT)
#define INT2_VBUS_OFF_MASK           (1 << INT2_VBUS_OFF_SHIFT)

/* SM5705 Interrupt 3  AFC register */
#define INT3_AFC_ERROR_SHIFT         5
#define INT3_AFC_STA_CHG_SHIFT       4
#define INT3_AFC_MULTI_BYTE_SHIFT    3
#define INT3_AFC_VBUS_UPDATE_SHIFT   2
#define INT3_AFC_ACCEPTED_SHIFT      1
#define INT3_AFC_TA_ATTACHED_SHIFT   0

#define INT3_AFC_ERROR_MASK          (1 << INT3_AFC_ERROR_SHIFT)
#define INT3_AFC_STA_CHG_MASK        (1 << INT3_AFC_STA_CHG_SHIFT)
#define INT3_AFC_MULTI_BYTE_MASK     (1 << INT3_AFC_MULTI_BYTE_SHIFT)
#define INT3_AFC_VBUS_UPDATE_MASK    (1 << INT3_AFC_VBUS_UPDATE_SHIFT)
#define INT3_AFC_ACCEPTED_MASK       (1 << INT3_AFC_ACCEPTED_SHIFT)
#define INT3_AFC_TA_ATTACHED_MASK    (1 << INT3_AFC_TA_ATTACHED_SHIFT)


/* SM5705 AFC CTRL register */
#define AFC_VBUS_READ_SHIFT    3
#define AFC_DM_RESET_SHIFT     2
#define AFC_DP_RESET_SHIFT     1
#define AFC_ENAFC_SHIFT        0

#define AFC_VBUS_READ_MASK    (1 << AFC_VBUS_READ_SHIFT)
#define AFC_DM_RESET_MASK     (1 << AFC_DM_RESET_SHIFT)
#define AFC_DP_RESET_MASK     (1 << AFC_DP_RESET_SHIFT)
#define AFC_ENAFC_MASK        (1 << AFC_ENAFC_SHIFT)


/* SM5705 ADC register */
#define ADC_ADC_SHIFT           0
#define ADC_ADC_MASK            (0x1f << ADC_ADC_SHIFT)


/* SM5705 Device Type 1 register */
#define DEV_TYPE1_USB_OTG       (1 << 7)
#define DEV_TYPE1_DEDICATED_CHG (1 << 6)
#define DEV_TYPE1_CDP           (1 << 5)
#define DEV_TYPE1_CARKIT_CHG    (1 << 4)
#define DEV_TYPE1_UART          (1 << 3)
#define DEV_TYPE1_USB           (1 << 2)
#define DEV_TYPE1_AUDIO_2       (1 << 1)
#define DEV_TYPE1_AUDIO_1       (1 << 0)
#define DEV_TYPE1_USB_TYPES     (DEV_TYPE1_USB_OTG | DEV_TYPE1_CDP | DEV_TYPE1_USB)

/* SM5705 Device Type 2 register */
#define DEV_TYPE2_AV             (1 << 6)
#define DEV_TYPE2_TTY            (1 << 5)
#define DEV_TYPE2_PPD            (1 << 4)
#define DEV_TYPE2_JIG_UART_OFF   (1 << 3)
#define DEV_TYPE2_JIG_UART_ON    (1 << 2)
#define DEV_TYPE2_JIG_USB_OFF    (1 << 1)
#define DEV_TYPE2_JIG_USB_ON     (1 << 0)
#define DEV_TYPE2_JIG_USB_TYPES  (DEV_TYPE2_JIG_USB_OFF | DEV_TYPE2_JIG_USB_ON)
#define DEV_TYPE2_JIG_UART_TYPES (DEV_TYPE2_JIG_UART_OFF)
#define DEV_TYPE2_JIG_TYPES      (DEV_TYPE2_JIG_UART_TYPES | DEV_TYPE2_JIG_USB_TYPES)

/* SM5705 Device Type 3 register */
#define DEV_TYPE3_AFC_CHG            (1 << 7)
#define DEV_TYPE3_U200_CHG           (1 << 6)
#define DEV_TYPE3_LO_CHG             (1 << 5)
#define DEV_TYPE3_AV_WITH_VBUS       (1 << 4)
#define DEV_TYPE3_NON_STANDARD_CHG   (1 << 2)
#define DEV_TYPE3_MHL                (1 << 0)
#define DEV_TYPE3_CHG_TYPE           (DEV_TYPE3_U200_CHG | DEV_TYPE3_LO_CHG)

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open / 001: USB AP / 010: AUDIO / 011: UART AP / 100: USB CP / 101: UART CP
 */
#define SW_DHOST_AP     ((1 << 5) | (1 << 2)) 
#define SW_AUDIO        ((2 << 5) | (2 << 2))
#define SW_UART_AP      ((3 << 5) | (3 << 2))
#define SW_DHOST_CP     ((4 << 5) | (4 << 2)) 
#define SW_UART_CP      ((5 << 5) | (5 << 2))
#define SW_ALL_OPEN     (0)
#define SW_ALL_OPEN_WITH_VBUS      ((0 << 5) | (0 << 2))
#define SW_ALL_OPEN_WITHOUT_VBUS   0x00
 

enum sm5705_reg_manual_sw1_value {
    MANSW1_OPEN =  SW_ALL_OPEN,
    MANSW1_OPEN_WITH_V_BUS = SW_ALL_OPEN,
    MANSW1_USB_AP =  SW_DHOST_AP,
    MANSW1_AUDIO =  SW_AUDIO,
    MANSW1_UART_AP =  SW_UART_AP,
    MANSW1_USB_CP =  SW_DHOST_CP,
    MANSW1_UART_CP =  SW_UART_CP,    
    MANSW1_OPEN_RUSTPROOF = SW_ALL_OPEN,
};

/*
 * Manual Switch2
 *
 */
#define SM5705_MANSW2_SINGLE_MODE_SHIFT (1)

#define REG_INT_MASK1_VALUE        0x1C
#define REG_INT_MASK2_VALUE        0x00
#define REG_INT_MASK3_AFC_VALUE    0x00
#define REG_TIMING1_VALUE          0x03



/* muic chip specific internal data structure
 * that setted at muic-xxxx.c file
 */
struct sm5705_muic_data {

    struct device *dev;
    struct i2c_client *i2c; /* i2c addr: 0x4A; MUIC */
    struct mutex muic_mutex;

    /* model dependant muic platform data */
    struct muic_platform_data *pdata;

    /* muic current attached device */
    muic_attached_dev_t attached_dev;

    /* muic Device ID */
    u8 muic_vendor;         /* Vendor ID */
    u8 muic_version;        /* Version ID */

    bool            is_usb_ready;
    bool            is_factory_start;
    bool            is_rustproof;
    bool            is_otg_test;

    struct delayed_work init_work;
    struct delayed_work usb_work;
};

extern struct device *switch_device;

extern int sm5705_i2c_read_byte(const struct i2c_client *client, u8 command);
extern int sm5705_i2c_write_byte(const struct i2c_client *client, u8 command, u8 value);
extern int sm5705_i2c_guaranteed_wbyte(const struct i2c_client *client, u8 command, u8 value);
                

extern int set_afc_ctrl_reg(struct sm5705_muic_data *muic_data, int shift, bool on);
extern int set_afc_ctrl_enafc(struct sm5705_muic_data *muic_data, bool on);
extern int set_afc_vbus_read(struct sm5705_muic_data *muic_data, bool on);


#endif /* __SM5705_H__ */
