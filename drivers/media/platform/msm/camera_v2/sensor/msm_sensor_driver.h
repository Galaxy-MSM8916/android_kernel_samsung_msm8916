/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MSM_SENSOR_DRIVER_H
#define MSM_SENSOR_DRIVER_H

#include "msm_sensor.h"

int32_t msm_sensor_driver_probe(void *setting);
int32_t msm_sensor_remove_dev_node_for_eeprom(int id,int remove);
struct yuv_userset {
    unsigned int metering;
    unsigned int exposure;
    unsigned int wb;
    unsigned int iso;
    unsigned int effect;
    unsigned int scenemode;
    unsigned int aeawblock;
    unsigned int resolution;
    unsigned int prev_resolution;
};

struct yuv_ctrl {
    struct yuv_userset settings;
    int op_mode;
    int prev_mode;
    int streamon;
    int vtcall_mode;
    int exif_iso;
    int exif_shutterspeed;
    int fixed_fps_val;
};


#endif
