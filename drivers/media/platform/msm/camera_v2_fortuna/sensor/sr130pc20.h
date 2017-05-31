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
 
 #ifndef __SR130PC20_H__
#define __SR130PC20_H__

#include "msm_sensor.h"
#include "msm_sensor_driver.h"

#undef SR130PC20_DEBUG
#undef CDBG

#define SR130PC20_DEBUG
#ifdef SR130PC20_DEBUG
#define CDBG(fmt, args...)	printk("[SR130PC20] %s : %d : "fmt "\n",   __FUNCTION__, __LINE__, ##args)
#endif

int32_t sr130pc20_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int32_t sr130pc20_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
void sr130pc20_set_default_settings(void);
int sr130pc20_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl);
#endif	//__SR130PC20_H__
