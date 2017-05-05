#ifndef __SR030PC50_H__
#define __SR030PC50_H__

#include "msm_sensor.h"
#include "msm_sensor_driver.h"

#undef DEBUG_LEVEL_HIGH
#undef CDBG

#define DEBUG_LEVEL_HIGH
#ifdef DEBUG_LEVEL_HIGH
#define CDBG(fmt, args...)	printk("[SR030PC50] %s : %d : " fmt "\n",   __FUNCTION__, __LINE__, ##args)
#endif

int32_t sr030pc50_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int32_t sr030pc50_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp);
int sr030pc50_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl);
#endif	//__SR030PC50_H__