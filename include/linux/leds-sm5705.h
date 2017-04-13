/*
 * Flash-LED device driver for SM5705
 *
 * Copyright (C) 2015 Silicon Mitus 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 */

#ifndef __LEDS_SM5705_H__
#define __LEDS_SM5705_H__

enum {
    SM5705_FLED_0   = 0x0,
    SM5705_FLED_1,
    SM5705_FLED_MAX,
};

struct sm5705_fled_platform_data {
    struct {
        unsigned short flash_current_mA;
        unsigned short torch_current_mA;

        bool used_gpio;
        int flash_en_pin;
        int torch_en_pin;
    }led[SM5705_FLED_MAX];
};

int sm5705_fled_prepare_flash(unsigned char index);
int sm5705_fled_torch_on(unsigned char index);
int sm5705_fled_flash_on(unsigned char index);
int sm5705_fled_led_off(unsigned char index);
int sm5705_fled_close_flash(unsigned char index);

#endif
