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

#ifndef __db8221a_H__
#define __db8221a_H__

#include "msm_sensor.h"
#include "msm_sensor_driver.h"

#undef db8221a_DEBUG
#undef CDBG
#define db8221a_DEBUG
#ifdef db8221a_DEBUG
#define CDBG(fmt, args...)	pr_err("%s : %d : "fmt "\n",   __FUNCTION__, __LINE__, ##args)
#else
#define CDBG(fmt, args...)	pr_debug("%s : %d : "fmt "\n",   __FUNCTION__, __LINE__, ##args)
#endif

int32_t db8221a_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int32_t db8221a_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
#endif	//__sr200pc20_H__
