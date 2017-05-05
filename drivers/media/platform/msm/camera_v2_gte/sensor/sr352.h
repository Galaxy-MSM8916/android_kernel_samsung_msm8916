/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
 #ifndef __SR352_H__
#define __SR352_H__

#include "msm_sensor.h"
#include "msm_sensor_driver.h"

#undef SR352_DEBUG
#undef CDBG

#define SR352_DEBUG
#ifdef SR352_DEBUG
#define CDBG(fmt, args...)	printk("[SR352] %s : %d : "fmt "\n",   __FUNCTION__, __LINE__, ##args)
#endif

int32_t sr352_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int32_t sr352_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
void sr352_set_default_settings(void);
int sr352_sensor_match_id(struct msm_camera_i2c_client *sensor_i2c_client,
	struct msm_camera_slave_info *slave_info,
	const char *sensor_name);
#endif	//__sr352_H__