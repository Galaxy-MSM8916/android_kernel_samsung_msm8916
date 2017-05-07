/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
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
#ifndef MSM_EEPROM_H
#define MSM_EEPROM_H

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <soc/qcom/camera2.h>
#include <media/v4l2-subdev.h>
#include <media/msmb_camera.h>
#include "msm_camera_i2c.h"
#include "msm_camera_spi.h"
#include "msm_camera_io_util.h"
#include "msm_camera_dt_util.h"

struct msm_eeprom_ctrl_t;

#define DEFINE_MSM_MUTEX(mutexname) \
	static struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#define PROPERTY_MAXSIZE 32
#define EEPROM_FW_VERSION_OFFSET 17

struct msm_eeprom_ctrl_t {
	struct platform_device *pdev;
	struct mutex *eeprom_mutex;

	struct v4l2_subdev sdev;
	struct v4l2_subdev_ops *eeprom_v4l2_subdev_ops;
	enum msm_camera_device_type_t eeprom_device_type;
	struct msm_sd_subdev msm_sd;
	enum cci_i2c_master_t cci_master;

	struct msm_camera_i2c_client i2c_client;
	struct msm_eeprom_memory_block_t cal_data;
	uint8_t is_supported;
	struct msm_eeprom_board_info *eboard_info;
	uint32_t subdev_id;
};

static struct msm_camera_i2c_reg_array init_read_s5k5e3yx_otp_reg[] = {
	{0x0A00, 0x04, NULL, 0},
	{0x0A02, 0x02, NULL, 0},
	{0x0A00, 0x01, NULL, 10},
};

struct msm_camera_i2c_reg_setting init_read_otp = {
	init_read_s5k5e3yx_otp_reg, sizeof(init_read_s5k5e3yx_otp_reg)/sizeof(struct msm_camera_i2c_reg_array), MSM_CAMERA_I2C_WORD_ADDR, MSM_CAMERA_I2C_BYTE_DATA, 10
};

static struct msm_camera_i2c_reg_array finish_read_s5k5e3yx_otp_reg[] = {
	{0x0A00, 0x04, NULL, 0},
	{0x0A00, 0x00, NULL, 0},
};

struct msm_camera_i2c_reg_setting finish_read_otp = {
	finish_read_s5k5e3yx_otp_reg, sizeof(finish_read_s5k5e3yx_otp_reg)/sizeof(struct msm_camera_i2c_reg_array), MSM_CAMERA_I2C_WORD_ADDR, MSM_CAMERA_I2C_BYTE_DATA, 10
};

static struct msm_camera_i2c_reg_array init_write_s5k5e3yx_otp_reg[] = {
	{0x3b42, 0x68, NULL, 0},
	{0x3b41, 0x01, NULL, 0},
	{0x3b40, 0x00, NULL, 0},
	{0x3b45, 0x02, NULL, 0},
	{0x0A00, 0x04, NULL, 0},
	{0x0A00, 0x03, NULL, 0},
	{0x3b42, 0x00, NULL, 0},
	{0x0A02, 0x02, NULL, 0},
	{0x0A00, 0x03, NULL, 10},
};

struct msm_camera_i2c_reg_setting init_write_otp = {
	init_write_s5k5e3yx_otp_reg, sizeof(init_write_s5k5e3yx_otp_reg)/sizeof(struct msm_camera_i2c_reg_array), MSM_CAMERA_I2C_WORD_ADDR, MSM_CAMERA_I2C_BYTE_DATA, 10
};

static struct msm_camera_i2c_reg_array finish_write_s5k5e3yx_otp_reg[] = {
	{0x0A00, 0x04, NULL, 0},
	{0x0A00, 0x00, NULL, 0},
	{0x3b40, 0x01, NULL, 10},
};

struct msm_camera_i2c_reg_setting finish_write_otp = {
	finish_write_s5k5e3yx_otp_reg, sizeof(finish_write_s5k5e3yx_otp_reg)/sizeof(struct msm_camera_i2c_reg_array), MSM_CAMERA_I2C_WORD_ADDR, MSM_CAMERA_I2C_BYTE_DATA, 10
};

extern uint8_t* get_eeprom_data_addr(void);
#endif
