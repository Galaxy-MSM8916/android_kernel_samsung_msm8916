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
 */
#ifndef MSM_OIS_H
#define MSM_OIS_H

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <soc/qcom/camera2.h>
#include <media/v4l2-subdev.h>
#include <media/msmb_camera.h>
#include <media/msm_cam_sensor.h>
#include "msm_camera_i2c.h"
#include "msm_camera_dt_util.h"
#include "msm_camera_io_util.h"

#define DEFINE_MSM_MUTEX(mutexname) \
    static struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#define MSM_OIS_MAX_VREGS (10)
#define NUM_OIS_VERSION_STR (6)

enum msm_ois_state_t {
    OIS_POWER_UP,
    OIS_POWER_DOWN,
};


struct msm_ois_vreg {
    struct camera_vreg_t *cam_vreg;
    void *data[MSM_OIS_MAX_VREGS];
    int num_vreg;
};

struct msm_ois_ver_t {
    uint8_t  core_ver;
    uint8_t  gyro_sensor;
    uint8_t  driver_ic;
    uint8_t  year;
    uint8_t  month;
    uint8_t  iteration_1;
    uint8_t  iteration_0;
};

struct msm_ois_debug_t {
    int   err_reg;
    int   status_reg;
    char  phone_ver[NUM_OIS_VERSION_STR+1];
    char  module_ver[NUM_OIS_VERSION_STR+1];
    char  cal_ver[NUM_OIS_VERSION_STR+1];
};

enum msm_ois_modes{
    OIS_MODE_OFF       = 1,
    OIS_MODE_ON        = 2,
    OIS_MODE_ON_STILL  = 3,
    OIS_MODE_ON_ZOOM   = 4,
    OIS_MODE_ON_VIDEO  = 5,
    OIS_MODE_SINE_X    = 6,
    OIS_MODE_SINE_Y    = 7,
    OIS_MODE_CENTERING = 8,
    OIS_MODE_MAX,
};

typedef struct __msm_ois_fw_info {
    bool is_loaded;
    bool force_phone;
    bool force_test;
    u8 *phone_fw;
    u8 *module_fw;
    u8 *load_fw;

    u16 shift_offset;
    u16 shift_data_size;
    u16 shift_chksum_offset;

    u16 fw_offset;    //Set F/W Offset
    u16 fw_data_size;
    u16 fw_chksum_offset;

    u16 cal_offset;
    u16 cal_data_size;
    u16 cal_chksum_offset;

    u16 fa_fw_offset; //Factory F/W Offset
    u16 fa_fw_data_size;
    u16 fa_fw_chksum_offset;


    struct msm_ois_ver_t phone_ver;
    struct msm_ois_ver_t module_ver;
    struct msm_ois_cal_info_t cal_info;
    bool is_phone_fw;
    char load_fw_name[256];
} msm_ois_fw_info_t;

struct msm_ois_ctrl_t {
    struct i2c_driver *i2c_driver;
    struct platform_driver *pdriver;
    struct device *dev;
    struct platform_device *pdev;
    struct msm_camera_i2c_client i2c_client;
    enum msm_camera_device_type_t ois_device_type;
    struct msm_sd_subdev msm_sd;
    enum af_camera_name cam_name;
    struct mutex *ois_mutex;
    struct v4l2_subdev sdev;
    struct v4l2_subdev_ops *ois_v4l2_subdev_ops;
    enum cci_i2c_master_t cci_master;
    uint32_t subdev_id;
    enum msm_ois_state_t ois_state;
    enum msm_camera_i2c_data_type i2c_data_type;
    enum msm_ois_modes ois_mode;
    struct msm_ois_vreg vreg_cfg;
    struct msm_camera_gpio_conf *gpio_conf;
    msm_ois_fw_info_t fw_info;
    bool is_camera_run;
    bool is_set_debug_info;
    struct msm_ois_debug_t debug_info;
    int ois_en;
    int ois_reset;
    struct msm_pinctrl_info pinctrl_info;
    uint8_t pinctrl_status;
};


#define OIS_PINCTRL_STATE_DEFAULT               "ois_default"
#define OIS_PINCTRL_STATE_SLEEP                 "ois_suspend"

/*SAMSUNG ELECTRONICS FIRMWARE*/
#define MSM_OIS_SE_FW_PATH                      "/system/etc/firmware/ois_SE_BU24219.bin"
/*SELF PRODUCTION FIRMWARE*/
#define MSM_OIS_SP_FW_PATH                      "/system/etc/firmware/ois_SP_BU24219.bin"

#define MSM_MAX_OIS_SIZE                        (64 * 1024)

#define I2C_RETRY_COUNT                         3

#define MODULE_OIS_TOTAL_SIZE                   0x1800
#define MODULE_OIS_START_ADDR                   0x2000

#define MODULE_OIS_HDR_OFFSET                   0x0000
#define MODULE_OIS_HDR_CHK_SUM_OFFSET           0x00FC

#define MODULE_OIS_CAL_DATA_OFFSET              0x0100
#define MODULE_OIS_CAL_DATA_CHK_SUM_OFFSET      0x019C
#define MODULE_OIS_CAL_DATA_MAP_SIZE            0x0060

#define MODULE_OIS_SHIFT_DATA_OFFSET            0x01A0
#define MODULE_OIS_SHIFT_DATA_CHK_SUM_OFFSET    0x01FC
#define MODULE_OIS_SHIFT_DATA_SIZE              0x0030

#define MODULE_OIS_FW_OFFSET                    0x0200
#define MODULE_OIS_FW_CHK_SUM_OFFSET            0x05FC
#define MODULE_OIS_FW_MAP_SIZE                  0x03FC

#define MODULE_OIS_FW_FACTORY_OFFSET            0x0600
#define MODULE_OIS_FW_FACTORY_CHK_SUM_OFFSET    0x17FC
#define MODULE_OIS_FW_FACTORY_MAP_SIZE          0x11FC


#define PHONE_OIS_TOTAL_SIZE                    0x3000
#define PHONE_OIS_START_ADDR                    0x0000

#define PHONE_OIS_HDR_OFFSET                    0x0000
#define PHONE_OIS_HDR_CHK_SUM_OFFSET            0xFFFF

#define PHONE_OIS_CAL_DATA_OFFSET               0x0080
#define PHONE_OIS_CAL_DATA_CHK_SUM_OFFSET       0xFFFF
#define PHONE_OIS_CAL_DATA_MAP_SIZE             0x0060

#define PHONE_OIS_SHIFT_DATA_OFFSET             0xFFFF
#define PHONE_OIS_SHIFT_DATA_CHK_SUM_OFFSET     0xFFFF
#define PHONE_OIS_SHIFT_DATA_SIZE               0xFFFF

#define PHONE_OIS_FW_OFFSET                     0x0100
#define PHONE_OIS_FW_CHK_SUM_OFFSET             0x04FC
#define PHONE_OIS_FW_MAP_SIZE                   0x03FC

#define PHONE_OIS_FW_FACTORY_OFFSET             0x0500
#define PHONE_OIS_FW_FACTORY_CHK_SUM_OFFSET     0x16FC
#define PHONE_OIS_FW_FACTORY_MAP_SIZE           0x11FC


#define OIS_SET_FW_D1_OFFSET_ADDR_OFFSET        0x04
#define OIS_SET_FW_D1_SIZE_ADDR_OFFSET          0x06
#define OIS_SET_FW_D1_TARGET_ADDR_OFFSET        0x08

#define OIS_SET_FW_D2_OFFSET_ADDR_OFFSET        0x0A
#define OIS_SET_FW_D2_SIZE_ADDR_OFFSET          0x0C
#define OIS_SET_FW_D2_TARGET_ADDR_OFFSET        0x0E

#define OIS_SET_FW_D3_OFFSET_ADDR_OFFSET        0x10
#define OIS_SET_FW_D3_SIZE_ADDR_OFFSET          0x12
#define OIS_SET_FW_D3_TARGET_ADDR_OFFSET        0x14

#define OIS_CAL_DATA_OFFSET_OFFSET              0x04
#define OIS_CAL_DATA_SIZE_OFFSET                0x06
#define OIS_CAL_DATA_TRGT_ADDR_OFFSET           0x08


#define OIS_GYRO_SCALE_FACTOR_V003              262
#define OIS_GYRO_SCALE_FACTOR_V004              131



void msm_is_ois_offset_test(long *raw_data_x, long *raw_data_y, bool is_need_cal);
u8 msm_is_ois_self_test(void);

int msm_ois_i2c_byte_read(struct msm_ois_ctrl_t *a_ctrl, uint32_t addr, uint16_t *data);
int msm_ois_i2c_byte_write(struct msm_ois_ctrl_t *a_ctrl, uint32_t addr, uint16_t data);

static int32_t msm_ois_power_up(struct msm_ois_ctrl_t *a_ctrl);
static int32_t msm_ois_power_down(struct msm_ois_ctrl_t *a_ctrl);
static int msm_ois_disable_reset(struct msm_ois_ctrl_t *a_ctrl);

#endif
