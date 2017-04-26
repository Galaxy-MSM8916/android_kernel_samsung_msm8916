/* 
 * drivers/battery/sm5705_charger_oper.h
 * 
 * SM5705 Charger Operation Mode controller
 *
 * Copyright (C) 2015 Siliconmitus Technology Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 */

#include <linux/init.h>
#include <linux/mfd/sm5705/sm5705.h>

enum {
    SM5705_CHARGER_OP_MODE_SUSPEND             = 0x0,
    SM5705_CHARGER_OP_MODE_FACTORY             = 0x1,
    SM5705_CHARGER_OP_MODE_WPC_OTG_CHG_OFF     = 0x2,
    SM5705_CHARGER_OP_MODE_WPC_OTG_CHG_ON      = 0x3,
    SM5705_CHARGER_OP_MODE_CHG_OFF             = 0x4,
    SM5705_CHARGER_OP_MODE_CHG_ON              = 0x5,
    SM5705_CHARGER_OP_MODE_FLASH_BOOST         = 0x6,
    SM5705_CHARGER_OP_MODE_USB_OTG             = 0x7,
};

enum SM5705_CHARGER_OP_EVENT_TYPE {
    SM5705_CHARGER_OP_EVENT_VBUS                = 0x5,
    SM5705_CHARGER_OP_EVENT_WPC                 = 0x4,
    SM5705_CHARGER_OP_EVENT_FLASH               = 0x3,
    SM5705_CHARGER_OP_EVENT_TORCH               = 0x2,
    SM5705_CHARGER_OP_EVENT_OTG                 = 0x1,
    SM5705_CHARGER_OP_EVENT_PWR_SHAR            = 0x0,
};

#define make_OP_STATUS(vbus,wpc,flash,torch,otg,pwr_shar)   (((vbus & 0x1) << SM5705_CHARGER_OP_EVENT_VBUS)         | \
                                                             ((wpc & 0x1) << SM5705_CHARGER_OP_EVENT_WPC)           | \
                                                             ((flash & 0x1) << SM5705_CHARGER_OP_EVENT_FLASH)       | \
                                                             ((torch & 0x1) << SM5705_CHARGER_OP_EVENT_TORCH)       | \
                                                             ((otg & 0x1) << SM5705_CHARGER_OP_EVENT_OTG)           | \
                                                             ((pwr_shar & 0x1) << SM5705_CHARGER_OP_EVENT_PWR_SHAR))

int sm5705_charger_oper_push_event(int event_type, bool enable);
int sm5705_charger_oper_table_init(struct i2c_client *i2c);

int sm5705_charger_oper_get_current_status(void);
int sm5705_charger_oper_get_current_op_mode(void);

