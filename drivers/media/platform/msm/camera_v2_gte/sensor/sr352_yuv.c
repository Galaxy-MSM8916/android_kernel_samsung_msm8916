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

#if 0
#define CONFIG_LOAD_FILE  // Enable it for Tunning Binary
#define NO_BURST // Enable no burst mode for Tunning
#endif

//#define NO_BURST // Enable no burst mode for Tunning

//#define FLASH_DEBUG
#ifdef FLASH_DEBUG
#define CDBG_FL(_fmt, args...) printk(_fmt, ##args)
#else
#define CDBG_FL(_fmt, args...)
#endif

#include "sr352.h"
/* *********************** Settings file selection *********************** */
#ifdef CONFIG_MACH_MILLETVEWIFI_EUR_OPEN
#define REG_SET_FILE "sr352_yuv.h"
#else
#define REG_SET_FILE "sr352_yuv_matisse.h"
#endif

#include REG_SET_FILE
/* *********************** End Selection *********************** */

#if 0
#define ANTIBANDING_60HZ 1
//dummy declaration for 50HZ
//for successful compilation
#define sr352_50hz_setting sr352_60hz_setting
#define sr352_HD_50hz_setting sr352_HD_60hz_setting
#define sr352_AEAWB_Lock_50Hz sr352_AEAWB_Lock_60Hz
#define sr352_AEAWB_Unlock_50Hz sr352_AEAWB_Unlock_60Hz
#elif defined(CONFIG_MACH_MILLETWIFI_OPEN)
extern int back_camera_antibanding_get(void); /*add anti-banding code */
#define ANTIBANDING_60HZ (back_camera_antibanding_get() == 60)
#else
#define ANTIBANDING_60HZ 0
//dummy declaration for 60HZ
//for successful compilation
#define sr352_60hz_setting sr352_50hz_setting
#define sr352_HD_60hz_setting sr352_HD_50hz_setting
#define sr352_AEAWB_Lock_60Hz sr352_AEAWB_Lock_50Hz
#define sr352_AEAWB_Unlock_60Hz sr352_AEAWB_Unlock_50Hz
#endif


#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_dt_util.h"

#ifndef NO_BURST
#define BURST_MODE_BUFFER_MAX_SIZE 255
#define BURST_REG 0x0e
#define DELAY_REG 0xff
uint8_t burst_reg_data[BURST_MODE_BUFFER_MAX_SIZE];
static int32_t sr352_sensor_burst_write (struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_conf *reg_settings , int size);
#endif

#ifdef CONFIG_LOAD_FILE
#define SR352_WRITE_LIST(A) \
    sr352_regs_from_sd_tunning(A,s_ctrl,#A);
#define SR352_WRITE_LIST_BURST(A) \
    sr352_regs_from_sd_tunning(A,s_ctrl,#A);
#else
#define SR352_WRITE_LIST(A) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl( \
            s_ctrl->sensor_i2c_client, A, \
            ARRAY_SIZE(A), \
            MSM_CAMERA_I2C_BYTE_DATA); CDBG("REGSEQ ****** %s", #A)
#define SR352_WRITE_LIST_BURST(A) \
    sr352_sensor_burst_write(s_ctrl,A,ARRAY_SIZE(A)); CDBG("REGSEQ ****** BURST %s", #A)
#endif

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static char *sr352_regs_table;
static int sr352_regs_table_size;
int sr352_regs_from_sd_tunning(struct msm_camera_i2c_reg_conf *settings, struct msm_sensor_ctrl_t *s_ctrl,char * name);
void sr352_regs_table_init(char *filename);
void sr352_regs_table_exit(void);
#endif

static struct yuv_ctrl sr352_ctrl;
extern uint16_t rear_vendor_id;

static int32_t cur_scene_mode_chg = 0;
extern unsigned int system_rev;
unsigned int settings_type = 0;

#if defined (AF_FLASH_SUPPORT)
int flash_mode;
int flash_status;
int is_preflash;
int is_touchaf;
int focus_mode;
int need_main_flash;
int is_af_run;
FLASH_t Flash;
static int8_t cur_scene_mode = 0;

int set_led_flash(int);
void sr352_get_sensor_ev_data(struct msm_sensor_ctrl_t *s_ctrl, uint16_t PreFlashEnable);
void sr352_get_ev_data_flash_Off_func1(struct msm_sensor_ctrl_t *s_ctrl);
void sr352_get_ev_data_preflash_func2(struct msm_sensor_ctrl_t *s_ctrl);
void sr352_set_ev_data_mainFlash_func3(struct msm_sensor_ctrl_t *s_ctrl);
void sr352_return_ev_data_func4(struct msm_sensor_ctrl_t *s_ctrl);
void sr352_actuator_softlanding(struct msm_sensor_ctrl_t *s_ctrl);
void sr352_ae_stable_without_af(struct msm_sensor_ctrl_t *s_ctrl);
static int sr352_is_required_flash(struct msm_sensor_ctrl_t *s_ctrl, int flash_mode);
void sr352_set_af_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode);
void sr352_recording_landing(struct msm_sensor_ctrl_t *s_ctrl);
#endif


void sr352_check_hw_revision(void);

void sr352_check_hw_revision()
{
    settings_type = 1;
}

int32_t sr352_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);
    switch (mode) {
        case CAMERA_EV_M4:
            rc = SR352_WRITE_LIST(sr352_brightness_M4);
            break;
        case CAMERA_EV_M3:
            rc = SR352_WRITE_LIST(sr352_brightness_M3);
            break;
        case CAMERA_EV_M2:
            rc = SR352_WRITE_LIST(sr352_brightness_M2);
            break;
        case CAMERA_EV_M1:
            rc = SR352_WRITE_LIST(sr352_brightness_M1);
            break;
        case CAMERA_EV_DEFAULT:
            rc = SR352_WRITE_LIST(sr352_brightness_default);
            break;
        case CAMERA_EV_P1:
            rc = SR352_WRITE_LIST(sr352_brightness_P1);
            break;
        case CAMERA_EV_P2:
            rc = SR352_WRITE_LIST(sr352_brightness_P2);
            break;
        case CAMERA_EV_P3:
            rc = SR352_WRITE_LIST(sr352_brightness_P3);
            break;
        case CAMERA_EV_P4:
            rc = SR352_WRITE_LIST(sr352_brightness_P4);
            break;
        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }
    return rc;
}

int32_t sr352_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);
    switch (mode) {
        case CAMERA_EFFECT_OFF:
            if(settings_type == 1) {
                rc = SR352_WRITE_LIST(sr352_effect_none_01);
            }
            else {
                rc = SR352_WRITE_LIST(sr352_effect_none);
            }
            break;
        case CAMERA_EFFECT_MONO:
            rc = SR352_WRITE_LIST(sr352_effect_gray);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            rc = SR352_WRITE_LIST(sr352_effect_negative);
            break;
        case CAMERA_EFFECT_SEPIA:
            rc = SR352_WRITE_LIST(sr352_effect_sepia);
            break;
        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }
    return rc;
}

int32_t sr352_set_scene_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("Scene mode E = %d", mode);
    if(cur_scene_mode_chg == 0) {
        CDBG("Scene mode Not Set = %d", mode);
        return rc;
    }

    if(sr352_ctrl.prev_mode == CAMERA_MODE_INIT
            && mode == CAMERA_SCENE_AUTO) {
        CDBG("Scene mode X Not Set = %d", mode);
        cur_scene_mode_chg = 0;
        return rc;
    }

#if defined (AF_FLASH_SUPPORT)
    cur_scene_mode = mode;
#endif

    switch (mode) {
        case CAMERA_SCENE_AUTO:
            rc = SR352_WRITE_LIST(sr352_Scene_Off);
            break;
        case CAMERA_SCENE_LANDSCAPE:
            rc = SR352_WRITE_LIST(sr352_scene_Landscape);
            break;
        case CAMERA_SCENE_SPORT:
            rc = SR352_WRITE_LIST(sr352_scene_Sports);
            break;
        case CAMERA_SCENE_PARTY:
            rc = SR352_WRITE_LIST(sr352_scene_Party);
            break;
        case CAMERA_SCENE_BEACH:
            rc = SR352_WRITE_LIST(sr352_scene_Beach);
            break;
        case CAMERA_SCENE_SUNSET:
            rc = SR352_WRITE_LIST(sr352_scene_Sunset);
            break;
        case CAMERA_SCENE_DAWN:
            rc = SR352_WRITE_LIST(sr352_scene_Dawn);
            break;
        case CAMERA_SCENE_FALL:
            rc = SR352_WRITE_LIST(sr352_scene_Fall);
            break;
        case CAMERA_SCENE_CANDLE:
            rc = SR352_WRITE_LIST(sr352_scene_Candle);
            break;
        case CAMERA_SCENE_FIRE:
            rc = SR352_WRITE_LIST(sr352_scene_Firework);
            break;
        case CAMERA_SCENE_AGAINST_LIGHT:
            rc = SR352_WRITE_LIST(sr352_scene_Backlight);
            break;
        case CAMERA_SCENE_NIGHT:
            rc = SR352_WRITE_LIST(sr352_scene_Nightshot);
            break;

        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }
    cur_scene_mode_chg = 0;
    CDBG("Scene mode X = %d", mode);
    return rc;
}


int32_t sr352_set_ae_awb_lock(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);

    if(ANTIBANDING_60HZ) {
        if(mode) {
            rc = SR352_WRITE_LIST(sr352_AEAWB_Lock_60Hz);
        } else {
            rc = SR352_WRITE_LIST(sr352_AEAWB_Unlock_60Hz);
        }
    } else {
        if(mode) {
            rc = SR352_WRITE_LIST(sr352_AEAWB_Lock_50Hz);
        } else {
            rc = SR352_WRITE_LIST(sr352_AEAWB_Unlock_50Hz);
        }
    }
    return rc;
}

int32_t sr352_set_white_balance(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);
    switch (mode) {
        case CAMERA_WHITE_BALANCE_OFF:
        case CAMERA_WHITE_BALANCE_AUTO:
            rc = SR352_WRITE_LIST(sr352_wb_auto);
            break;
        case CAMERA_WHITE_BALANCE_INCANDESCENT:
            rc = SR352_WRITE_LIST(sr352_wb_incandescent);
            break;
        case CAMERA_WHITE_BALANCE_FLUORESCENT:
            rc = SR352_WRITE_LIST(sr352_wb_fluorescent);
            break;
        case CAMERA_WHITE_BALANCE_DAYLIGHT:
            rc = SR352_WRITE_LIST(sr352_wb_sunny);
            break;
        case CAMERA_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            rc = SR352_WRITE_LIST(sr352_wb_cloudy);
            break;
        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }
    return rc;
}

int32_t sr352_set_metering(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);
    switch (mode) {
        case CAMERA_AVERAGE:
            rc = SR352_WRITE_LIST(sr352_metering_matrix);
            break;
        case CAMERA_CENTER_WEIGHT:
            rc = SR352_WRITE_LIST(sr352_metering_center);
            break;
        case CAMERA_SPOT:
            rc = SR352_WRITE_LIST(sr352_metering_spot);
            break;
        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }
    return rc;
}

int32_t sr352_set_resolution(struct msm_sensor_ctrl_t *s_ctrl, int mode, int flicker_type)
{
    int32_t rc = 0;
    CDBG("mode = %d", mode);
    switch (mode) {
        case MSM_SENSOR_RES_FULL:
            if(settings_type == 1) {
#if defined (AF_FLASH_SUPPORT)
                if (cur_scene_mode == CAMERA_SCENE_NIGHT) {
                    SR352_WRITE_LIST(sr352_Night_Capture_2048_1536_01);
                } else {
                    SR352_WRITE_LIST(sr352_Capture_2048_1536_01);
                }
#else
                SR352_WRITE_LIST(sr352_Capture_2048_1536_01);
#endif
            }
            else {
                SR352_WRITE_LIST(sr352_Capture_2048_1536);
            }
            break;
        case MSM_SENSOR_RES_QTR:
        case MSM_SENSOR_RES_7:
            if(settings_type == 1) {
                SR352_WRITE_LIST(sr352_Enterpreview_1024x768_01);
            }
            else {
                SR352_WRITE_LIST(sr352_Enterpreview_1024x768);
            }
            break;
        case MSM_SENSOR_RES_2:
            if(settings_type == 1) {
                SR352_WRITE_LIST(sr352_Enterpreview_1024x576_01);
            }
            else {
                SR352_WRITE_LIST(sr352_Enterpreview_1024x576);
            }
            break;
        case MSM_SENSOR_RES_3:
            if(settings_type == 1) {
                rc = SR352_WRITE_LIST_BURST(sr352_recording_50Hz_HD_01);
            }
            else {
                rc = SR352_WRITE_LIST_BURST(sr352_recording_50Hz_HD);
            }
            if(flicker_type == MSM_CAM_FLICKER_50HZ) {
                rc = SR352_WRITE_LIST(sr352_HD_50hz_setting);
            } else {
                rc = SR352_WRITE_LIST(sr352_HD_60hz_setting);
            }
#if defined (AF_FLASH_SUPPORT)
            SR352_WRITE_LIST(sr352_HD_AF_Init_Reg);
#endif
            break;
        case MSM_SENSOR_RES_4:
            rc = SR352_WRITE_LIST(sr352_preview_800_480);
            break;
        case MSM_SENSOR_RES_5:
            if(settings_type == 1) {
                SR352_WRITE_LIST(sr352_Enterpreview_640x480_01);
            }
            else {
                SR352_WRITE_LIST(sr352_Enterpreview_640x480);
            }
            break;
        case MSM_SENSOR_RES_6:
            rc = SR352_WRITE_LIST(sr352_preview_320_240);
            break;
        default:
            pr_err("%s: Setting %d is invalid\n", __func__, mode);
    }
    return rc;
}

void sr352_init_camera(struct msm_sensor_ctrl_t *s_ctrl, int flicker_type)
{
    int32_t rc = 0;
    if(settings_type == 1) {
        rc = SR352_WRITE_LIST_BURST(sr352_Init_Reg_01);
    } else {
        rc = SR352_WRITE_LIST_BURST(sr352_Init_Reg);
    }
    if(rc < 0) {
        pr_err("%s:%d error writing initsettings failed\n", __func__, __LINE__);
    }
#if defined (AF_FLASH_SUPPORT)
    SR352_WRITE_LIST(sr352_AF_Init_Reg);
#endif
    if(flicker_type == MSM_CAM_FLICKER_50HZ) {
        rc = SR352_WRITE_LIST(sr352_50hz_setting);
    } else {
        rc = SR352_WRITE_LIST(sr352_60hz_setting);
    }
    if(rc <0) {
        pr_err("%s:%d error writing 50hz failed\n", __func__, __LINE__);
    }
}

int32_t sr352_get_exif(struct ioctl_native_cmd * exif_info)
{
#if defined (AF_FLASH_SUPPORT)
    exif_info->value_1 = flash_status;
#else
    exif_info->value_1 = 1; // equals 1 to update the exif value in the user level.
#endif
    exif_info->value_2 = sr352_ctrl.exif_iso;
    exif_info->value_3 = sr352_ctrl.exif_shutterspeed;
    return 0;
}

int32_t sr352_set_exif(struct msm_sensor_ctrl_t *s_ctrl )
{
    int32_t rc = 0;
    uint16_t read_value0 = 0;
    uint16_t read_value1 = 0;
    uint16_t read_value2 = 0;
    uint16_t read_value3 = 0;
    uint16_t read_value4 = 0;

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
            s_ctrl->sensor_i2c_client,
            0x03,
            0x20,
            MSM_CAMERA_I2C_BYTE_DATA);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
            s_ctrl->sensor_i2c_client, 0xa0,
            &read_value0,
            MSM_CAMERA_I2C_BYTE_DATA);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
            s_ctrl->sensor_i2c_client, 0xa1,
            &read_value1,
            MSM_CAMERA_I2C_BYTE_DATA);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
            s_ctrl->sensor_i2c_client, 0xa2,
            &read_value2,
            MSM_CAMERA_I2C_BYTE_DATA);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
            s_ctrl->sensor_i2c_client, 0xa3,
            &read_value3,
            MSM_CAMERA_I2C_BYTE_DATA);

    sr352_ctrl.exif_shutterspeed = 27323100 / ((read_value0 << 24)
            + (read_value1 << 16) + (read_value2 << 8) + read_value3);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
            s_ctrl->sensor_i2c_client,
            0x03,
            0x20,
            MSM_CAMERA_I2C_BYTE_DATA);

    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(
            s_ctrl->sensor_i2c_client, 0x50,
            &read_value4,
            MSM_CAMERA_I2C_BYTE_DATA);

    if (read_value4 < 0x26)
        sr352_ctrl.exif_iso= 50;
    else if (read_value4 < 0x5C)
        sr352_ctrl.exif_iso = 100;
    else if (read_value4 < 0x83)
        sr352_ctrl.exif_iso = 200;
    else
        sr352_ctrl.exif_iso = 400;

    pr_debug("sr352_set_exif: ISO = %d shutter speed = %d",
            sr352_ctrl.exif_iso,sr352_ctrl.exif_shutterspeed);
    return rc;
}

int32_t sr352_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
        void __user *argp)
{
    struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
    int32_t rc = 0;
    int32_t i = 0;
    mutex_lock(s_ctrl->msm_sensor_mutex);

    switch (cdata->cfgtype) {
        case CFG_GET_SENSOR_INFO:
            CDBG(" CFG_GET_SENSOR_INFO");
            memcpy(cdata->cfg.sensor_info.sensor_name,
                    s_ctrl->sensordata->sensor_name,
                    sizeof(cdata->cfg.sensor_info.sensor_name));

            cdata->cfg.sensor_info.session_id =
                s_ctrl->sensordata->sensor_info->session_id;

            for (i = 0; i < SUB_MODULE_MAX; i++)
                cdata->cfg.sensor_info.subdev_id[i] =
                    s_ctrl->sensordata->sensor_info->subdev_id[i];

            CDBG("sensor name %s", cdata->cfg.sensor_info.sensor_name);
            CDBG("session id %d", cdata->cfg.sensor_info.session_id);

            for (i = 0; i < SUB_MODULE_MAX; i++)
                CDBG("subdev_id[%d] %d", i,
                        cdata->cfg.sensor_info.subdev_id[i]);

            break;
        case CFG_SET_INIT_SETTING:
            CDBG("CFG_SET_INIT_SETTING");
#ifdef CONFIG_LOAD_FILE /* this call should be always called first */
            sr352_regs_table_init("/data/"REG_SET_FILE);
            pr_err("/data/"REG_SET_FILE" inside CFG_SET_INIT_SETTING");
#endif
            sr352_init_camera(s_ctrl, cdata->flicker_type);

#if !defined (AF_FLASH_SUPPORT)
            //Stop stream and start in START_STREAM
            CDBG("Stop stream after Init");
            SR352_WRITE_LIST(sr352_stop_stream);
#endif
            break;
        case CFG_SET_RESOLUTION:
            if (sr352_ctrl.prev_mode == CAMERA_MODE_RECORDING &&
                sr352_ctrl.settings.resolution == MSM_SENSOR_RES_3) {

                sr352_init_camera(s_ctrl, cdata->flicker_type);
                //Stop stream and start in START_STREAM
                SR352_WRITE_LIST(sr352_stop_stream);
                CDBG("CFG CFG_SET_RESOLUTION - HD Recording mode off");
                cur_scene_mode_chg = 1;
            }
            else if(sr352_ctrl.prev_mode == CAMERA_MODE_RECORDING &&
                    sr352_ctrl.settings.resolution != MSM_SENSOR_RES_3) {
#if defined (AF_FLASH_SUPPORT)
                sr352_recording_landing(s_ctrl);
#endif

                if(settings_type == 1) {
                    rc = SR352_WRITE_LIST(sr352_recording_50Hz_modeOff_01);
                }
                else {
                    rc = SR352_WRITE_LIST(sr352_recording_50Hz_modeOff);
                }

                rc = SR352_WRITE_LIST(sr352_stop_stream);
                CDBG("CFG CFG_SET_RESOLUTION - Non - HD Recording mode off");
            }
            sr352_ctrl.settings.prev_resolution = sr352_ctrl.settings.resolution;
            sr352_ctrl.settings.resolution = *((int32_t  *)cdata->cfg.setting);
            CDBG("CFG_SET_RESOLUTION  res = %d" , sr352_ctrl.settings.resolution);
            if (sr352_ctrl.streamon == 0) {
                switch(sr352_ctrl.op_mode) {
                    case CAMERA_MODE_PREVIEW:
                    {
                        sr352_set_resolution(s_ctrl , sr352_ctrl.settings.resolution, cdata->flicker_type);
#if !defined(AF_FLASH_SUPPORT)
                        if (sr352_ctrl.prev_mode == CAMERA_MODE_RECORDING &&
                            sr352_ctrl.settings.prev_resolution == MSM_SENSOR_RES_3) {

                            sr352_set_ae_awb_lock(s_ctrl, 0);
                            msleep(100);
                        }

                        if (sr352_ctrl.prev_mode == CAMERA_MODE_INIT) {
                            msleep(100);
                        }
#endif
                    }
                    break;
                    case CAMERA_MODE_CAPTURE:
                    {
#if defined (AF_FLASH_SUPPORT)
                        if (!is_af_run)
                            sr352_ae_stable_without_af(s_ctrl);
                        if (need_main_flash) {
                            SR352_WRITE_LIST(sr352_stop_stream);
                            SR352_WRITE_LIST(sr352_AfterPreFlash_FlashRegTable4_1);
                            sr352_set_ev_data_mainFlash_func3(s_ctrl);
                            set_led_flash(MSM_CAMERA_LED_HIGH);
                            SR352_WRITE_LIST(sr352_StartMainFlash_FlashRegTable5);
                            flash_status = MSM_CAMERA_LED_HIGH;
                        }
#endif
                        sr352_set_resolution(s_ctrl , sr352_ctrl.settings.resolution, cdata->flicker_type);
                    }
                    break;
                    case  CAMERA_MODE_RECORDING:
                    {
#if defined (AF_FLASH_SUPPORT)
                        if(sr352_ctrl.prev_mode != CAMERA_MODE_RECORDING ||
                            sr352_ctrl.settings.resolution != MSM_SENSOR_RES_3) {
                            sr352_recording_landing(s_ctrl);
                        }
#endif
                        if ( sr352_ctrl.settings.resolution == MSM_SENSOR_RES_3 ) {
                            CDBG("CFG writing *** sr352_recording_50Hz_HD");
                            sr352_set_resolution( s_ctrl , sr352_ctrl.settings.resolution, cdata->flicker_type);
                        } else {
                            CDBG("CFG writing *** sr352_recording_50Hz_30fps");
                            sr352_set_resolution( s_ctrl , MSM_SENSOR_RES_5, cdata->flicker_type);
                            if(settings_type == 1) {
                                rc = SR352_WRITE_LIST(sr352_recording_50Hz_30fps_01);
                            }
                            else {
                                rc = SR352_WRITE_LIST(sr352_recording_50Hz_30fps);
                            }
                            //msleep(100);
                        }
                    }
                    break;
                }
            }
        break;
        case CFG_SET_STOP_STREAM:
            if(sr352_ctrl.streamon == 1) {
                CDBG(" CFG_SET_STOP_STREAM writing stop stream registers: sr352_stop_stream");
                sr352_ctrl.streamon = 0;
            }
#if defined (AF_FLASH_SUPPORT)
            switch(sr352_ctrl.op_mode) {
                case  CAMERA_MODE_CAPTURE:
                    if (need_main_flash) {
                        if (flash_status == MSM_CAMERA_LED_HIGH) {
                           set_led_flash(MSM_CAMERA_LED_OFF);
                           flash_status = MSM_CAMERA_LED_OFF;
                           SR352_WRITE_LIST(sr352_AfterMainFlash_FlashRegTable6);
                           sr352_return_ev_data_func4(s_ctrl);
                           SR352_WRITE_LIST(sr352_EndMainFlash_FlashRegTable7);
                           need_main_flash = 0;
                        }
                    }
                    is_af_run = 0;
                    break;
                case  CAMERA_MODE_RECORDING:
                    if (flash_status == MSM_CAMERA_LED_LOW) {
                        set_led_flash(MSM_CAMERA_LED_OFF);
                        flash_status = MSM_CAMERA_LED_OFF;
                    }
                    break;
            }
#endif
        break;
        case CFG_SET_START_STREAM:
        {
            CDBG(" CFG_SET_START_STREAM writing start stream registers: sr352_start_stream start");
            switch(sr352_ctrl.op_mode) {
                case  CAMERA_MODE_PREVIEW:
                {
                    CDBG(" CFG_SET_START_STREAM: Preview");
                    if(sr352_ctrl.prev_mode != CAMERA_MODE_CAPTURE) {
                        sr352_set_scene_mode(s_ctrl, sr352_ctrl.settings.scenemode);
                        sr352_set_exposure_compensation(s_ctrl, sr352_ctrl.settings.exposure);
                        sr352_set_effect(s_ctrl , sr352_ctrl.settings.effect);
                        sr352_set_white_balance(s_ctrl , sr352_ctrl.settings.wb);
                        sr352_set_metering(s_ctrl, sr352_ctrl.settings.metering );
#if defined (AF_FLASH_SUPPORT)
                        sr352_set_af_mode(s_ctrl, focus_mode);
#endif
                    }
                }
                break;
                case CAMERA_MODE_CAPTURE:
                {
                    CDBG("CFG_SET_START_STREAM: Capture");
                    sr352_set_exif(s_ctrl);
                }
                break;
                case CAMERA_MODE_RECORDING:
                {
                    sr352_set_exposure_compensation(s_ctrl, sr352_ctrl.settings.exposure);
                    sr352_set_effect(s_ctrl , sr352_ctrl.settings.effect);
                    sr352_set_white_balance(s_ctrl , sr352_ctrl.settings.wb);
                    sr352_set_metering(s_ctrl, sr352_ctrl.settings.metering );
#if defined (AF_FLASH_SUPPORT)
                    sr352_set_af_mode(s_ctrl, focus_mode);
#endif
                }
                break;
            }
            sr352_ctrl.streamon = 1;
        }
        break;
        case CFG_SET_SLAVE_INFO:
        {
            struct msm_camera_sensor_slave_info sensor_slave_info;
            struct msm_camera_power_ctrl_t *p_ctrl;
            uint16_t size;
            int slave_index = 0;
            CDBG("CFG_SET_SLAVE_INFO");
            if (copy_from_user(&sensor_slave_info,
                (void *)cdata->cfg.setting,
                 sizeof(sensor_slave_info))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -EFAULT;
                break;
            }
            /* Update sensor slave address */
            if (sensor_slave_info.slave_addr) {
                s_ctrl->sensor_i2c_client->cci_client->sid =
                    sensor_slave_info.slave_addr >> 1;
            }

            /* Update sensor address type */
            s_ctrl->sensor_i2c_client->addr_type =
                sensor_slave_info.addr_type;

            /* Update power up / down sequence */
            p_ctrl = &s_ctrl->sensordata->power_info;
            size = sensor_slave_info.power_setting_array.size;
            if (p_ctrl->power_setting_size < size) {
                struct msm_sensor_power_setting *tmp;
                tmp = kmalloc(sizeof(*tmp) * size, GFP_KERNEL);
                if (!tmp) {
                    pr_err("%s: failed to alloc mem\n", __func__);
                    rc = -ENOMEM;
                    break;
                }
                kfree(p_ctrl->power_setting);
                p_ctrl->power_setting = tmp;
            }
            p_ctrl->power_setting_size = size;
            rc = copy_from_user(p_ctrl->power_setting, (void *)
                    sensor_slave_info.power_setting_array.power_setting,
                    size * sizeof(struct msm_sensor_power_setting));
            if (rc) {
                pr_err("%s:%d failed\n", __func__, __LINE__);
                kfree(sensor_slave_info.power_setting_array.
                        power_setting);
                rc = -EFAULT;
                break;
            }
            CDBG("slave_addr = 0x%x, addr_type = %d, sensor_id_reg_addr = 0x%x, sensor_id %x", \
                    sensor_slave_info.slave_addr, sensor_slave_info.addr_type, \
                    sensor_slave_info.sensor_id_info.sensor_id_reg_addr, sensor_slave_info.sensor_id_info.sensor_id);
            for (slave_index = 0; slave_index <
                    p_ctrl->power_setting_size; slave_index++) {
                CDBG("i %d power setting %d %d %ld %d", slave_index,
                        p_ctrl->power_setting[slave_index].seq_type,
                        p_ctrl->power_setting[slave_index].seq_val,
                        p_ctrl->power_setting[slave_index].config_val,
                        p_ctrl->power_setting[slave_index].delay);
            }
            CDBG("CFG_SET_SLAVE_INFO EXIT");
            break;
        }
        case CFG_WRITE_I2C_ARRAY:
        {
            struct msm_camera_i2c_reg_setting conf_array;
            struct msm_camera_i2c_reg_array *reg_setting = NULL;

            CDBG(" CFG_WRITE_I2C_ARRAY");

            if (copy_from_user(&conf_array,
                        (void *)cdata->cfg.setting,
                        sizeof(struct msm_camera_i2c_reg_setting))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -EFAULT;
                break;
            }

            reg_setting = kzalloc(conf_array.size *
                    (sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
            if (!reg_setting) {
                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -ENOMEM;
                break;
            }
            if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
                        conf_array.size *
                        sizeof(struct msm_camera_i2c_reg_array))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                kfree(reg_setting);
                rc = -EFAULT;
                break;
            }

            conf_array.reg_setting = reg_setting;
            rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
                    s_ctrl->sensor_i2c_client, &conf_array);
            kfree(reg_setting);
        }
        break;
        case CFG_WRITE_I2C_SEQ_ARRAY:
        {
            struct msm_camera_i2c_seq_reg_setting conf_array;
            struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

            CDBG("CFG_WRITE_I2C_SEQ_ARRAY");

            if (copy_from_user(&conf_array,
                        (void *)cdata->cfg.setting,
                        sizeof(struct msm_camera_i2c_seq_reg_setting))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -EFAULT;
                break;
            }

            reg_setting = kzalloc(conf_array.size *
                    (sizeof(struct msm_camera_i2c_seq_reg_array)),
                    GFP_KERNEL);
            if (!reg_setting) {
                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -ENOMEM;
                break;
            }
            if (copy_from_user(reg_setting, (void *)conf_array.reg_setting,
                        conf_array.size *
                        sizeof(struct msm_camera_i2c_seq_reg_array))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                kfree(reg_setting);
                rc = -EFAULT;
                break;
            }

            conf_array.reg_setting = reg_setting;
            rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->
                i2c_write_seq_table(s_ctrl->sensor_i2c_client, &conf_array);
            kfree(reg_setting);
        }
        break;
        case CFG_POWER_UP:
        {
            CDBG(" CFG_POWER_UP");
            sr352_ctrl.streamon = 0;
            sr352_ctrl.op_mode = CAMERA_MODE_INIT;
            sr352_ctrl.prev_mode = CAMERA_MODE_INIT;
            sr352_ctrl.settings.prev_resolution = MSM_SENSOR_RES_FULL;
            sr352_ctrl.settings.resolution = MSM_SENSOR_RES_FULL;
#if defined (AF_FLASH_SUPPORT)
            flash_mode = 0;
            flash_status = 0;
            is_preflash = 0;
            is_touchaf = 0;
            focus_mode = 0;
            need_main_flash = 0;
            is_af_run = 0;
            cur_scene_mode = 0;
#endif
            sr352_check_hw_revision();
            if (s_ctrl->func_tbl->sensor_power_up) {
                CDBG("CFG_POWER_UP");
                rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
            } else
                rc = -EFAULT;
        }
        break;
        case CFG_POWER_DOWN:
        {
            CDBG("CFG_POWER_DOWN");
            if (s_ctrl->func_tbl->sensor_power_down) {
                CDBG("CFG_POWER_DOWN");
#if defined (AF_FLASH_SUPPORT)
                if (flash_status)
                    set_led_flash(MSM_CAMERA_LED_OFF);
                sr352_actuator_softlanding(s_ctrl);
#endif
                rc = s_ctrl->func_tbl->sensor_power_down(s_ctrl);
            } else
                 rc = -EFAULT;
#ifdef CONFIG_LOAD_FILE
            sr352_regs_table_exit();
#endif
        }
        break;
        case CFG_SET_STOP_STREAM_SETTING:
        {
            struct msm_camera_i2c_reg_setting *stop_setting =
                &s_ctrl->stop_setting;
            struct msm_camera_i2c_reg_array *reg_setting = NULL;

            CDBG("CFG_SET_STOP_STREAM_SETTING");
            if (copy_from_user(stop_setting, (void *)cdata->cfg.setting,
                        sizeof(struct msm_camera_i2c_reg_setting))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -EFAULT;
                break;
            }

            reg_setting = stop_setting->reg_setting;
            stop_setting->reg_setting = kzalloc(stop_setting->size *
                    (sizeof(struct msm_camera_i2c_reg_array)), GFP_KERNEL);
            if (!stop_setting->reg_setting) {
                pr_err("%s:%d failed\n", __func__, __LINE__);
                rc = -ENOMEM;
                break;
            }
            if (copy_from_user(stop_setting->reg_setting,
                        (void *)reg_setting, stop_setting->size *
                        sizeof(struct msm_camera_i2c_reg_array))) {

                pr_err("%s:%d failed\n", __func__, __LINE__);
                kfree(stop_setting->reg_setting);
                stop_setting->reg_setting = NULL;
                stop_setting->size = 0;
                rc = -EFAULT;
                break;
            }
        }
        break;
        default:
            rc = -EFAULT;
            break;
    }

    mutex_unlock(s_ctrl->msm_sensor_mutex);

    return rc;
}


#if defined (AF_FLASH_SUPPORT)
void sr352_ae_stable_without_af(struct msm_sensor_ctrl_t *s_ctrl)
{
    is_preflash = sr352_is_required_flash(s_ctrl, flash_mode);
    if (is_preflash) {
        memset(&Flash, 0, sizeof(Flash));
        SR352_WRITE_LIST(sr352_StartPreFlash_FlashRegTable1);
        sr352_get_ev_data_flash_Off_func1(s_ctrl);
        SR352_WRITE_LIST(sr352_ReadyPreFlash_FlashRegTable2);
        set_led_flash(MSM_CAMERA_LED_LOW);
        flash_status = MSM_CAMERA_LED_LOW;
        need_main_flash = 1;
        msleep(600); /* AE become stable here */
        SR352_WRITE_LIST(sr352_AfterPreFlash_FlashRegTable3);
        sr352_get_ev_data_preflash_func2(s_ctrl);
        SR352_WRITE_LIST(sr352_EndPreFlash_FlashRegTable4);
        msleep(50);
        set_led_flash(MSM_CAMERA_LED_OFF);
        msleep(300);
    }
}

void sr352_start_af(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t focus_reg = 0;

    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x11, &focus_reg);
    focus_reg &= 0xF7;
    SR352_WRITE_ADDR(0x11, focus_reg);
    SR352_READ_ADDR(0x11, &focus_reg);
    focus_reg |= 0x01;
    SR352_WRITE_ADDR(0x11, focus_reg);
    CDBG("SR352 %s focus_reg[0x%2x]\n", __func__, focus_reg);

}

void sr352_set_af_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    uint16_t focus_reg = 0;
    uint16_t manual_af = 0;

    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x11, &focus_reg);
    SR352_READ_ADDR(0x14, &manual_af);

    switch(mode)
    {
        case CAMERA_AF_OCR:
        case CAMERA_AF_MACRO:
            focus_reg |= 0x40; /* set Macro Scan Mode */

            SR352_READ_ADDR(0x11, &focus_reg);

            manual_af |= 0x10; /* set Macro Lens Position start */
            SR352_WRITE_ADDR(0x11, focus_reg);
            SR352_WRITE_ADDR(0x14, manual_af);
            SR352_WRITE_ADDR(0x03, 0xD0);
            SR352_WRITE_ADDR(0x7C, 0x02);
            SR352_WRITE_ADDR(0x7D, 0xB0);
            msleep(130);
            SR352_WRITE_ADDR(0x03, 0xCF);
            SR352_READ_ADDR(0x14, &manual_af);
            manual_af &= 0xEF;
            SR352_WRITE_ADDR(0x14, manual_af); /* set Macro Lens Position End */
        break;
        case CAMERA_AF_AUTO:
        default:
            focus_reg &= 0xBF; /* set Normal Scan Mode */

            SR352_READ_ADDR(0x11, &focus_reg);

            manual_af |= 0x10; /* set Normal Lens Position start */
            SR352_WRITE_ADDR(0x11, focus_reg);
            SR352_WRITE_ADDR(0x14, manual_af);
            SR352_WRITE_ADDR(0x03, 0xD0);
            SR352_WRITE_ADDR(0x7C, 0x00);
            msleep(130);
            SR352_WRITE_ADDR(0x03, 0xCF);
            SR352_READ_ADDR(0x14, &manual_af);
            manual_af &= 0xEF;
            SR352_WRITE_ADDR(0x14, manual_af); /* set Normal Lens Position End */
        break;
    }

    CDBG("SR352 %s focus_reg[0x%2x]\n", __func__, focus_reg);
}

void sr352_set_touch_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    uint16_t focus_reg = 0;

    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x11, &focus_reg);

    switch(mode)
    {
        case 0:
            /* Center focus */
            focus_reg &= ~(0x02); /* clear 1st bit */
        break;
        case 1:
            /* Touch Focus */
            focus_reg |= 0x02; /* set 1st bit */
        break;
        default:
            /* Center focus */
            focus_reg &= ~(0x02); /* clear 1st bit */
        break;
    }

    CDBG("DAFF %s focus_reg[0x%2x]\n", __func__, focus_reg);
    SR352_WRITE_ADDR(0x11, focus_reg);
}

void sr352_set_touchaf_pos(struct msm_sensor_ctrl_t *s_ctrl, int x, int y)
{
    uint16_t touch_x = x - XAXIS_START;
    uint16_t touch_y = y - YAXIS_START;
    uint16_t lcd_x = LCD_X;
    uint16_t lcd_y = LCD_Y;

    if (x - XAXIS_START < 0 ||  y - YAXIS_START < 0) {
        pr_err("%s Error position x_axis[%d] y_axis[%d]\n",
                __func__, x - XAXIS_START, y - YAXIS_START);
        return;
    }
    if(sr352_ctrl.settings.resolution == MSM_SENSOR_RES_3)
    {
        uint32_t posX, posY;

        posX = ((x * 100) * 92)/10000;
        posY = y;
        if(posX < 200)
            posX = 200;
        else if(posX > 1112)
            posX = 1112;
        if(posY < 200)
            posY = 200;
        else if(posY > 538)
            posY = 538;

        SR352_WRITE_ADDR(0x03, 0xD0);

        SR352_WRITE_ADDR(0xFA, (posY >> 8));
        SR352_WRITE_ADDR(0xFB, (posY & 0xFF));
        SR352_WRITE_ADDR(0xFC, (posX >> 8));
        SR352_WRITE_ADDR(0xFD, (posX & 0xFF));
    }
    else
    {
        SR352_WRITE_ADDR(0x03, 0xCF);
        SR352_WRITE_ADDR(0x15, 0xCA);
        SR352_WRITE_ADDR(0x03, 0xD1);

        SR352_WRITE_ADDR(0x10, (lcd_y >> 8));
        SR352_WRITE_ADDR(0x11, (lcd_y & 0xFF));
        SR352_WRITE_ADDR(0x12, (lcd_x >> 8));
        SR352_WRITE_ADDR(0x13, (lcd_x & 0xFF));

        SR352_WRITE_ADDR(0x03, 0xD0);

        SR352_WRITE_ADDR(0xFA, (touch_y >> 8));
        SR352_WRITE_ADDR(0xFB, (touch_y & 0xFF));
        SR352_WRITE_ADDR(0xFC, (touch_x >> 8));
        SR352_WRITE_ADDR(0xFD, (touch_x & 0xFF));
    }
    return;
}


int sr352_is_required_flash(struct msm_sensor_ctrl_t *s_ctrl, int flash_mode)
{
    int rc = 0;
    uint16_t bpga = 0, bpgath = 0, flashctl = 0;

    if (sr352_ctrl.op_mode == CAMERA_MODE_RECORDING)
        return 0;

    switch(flash_mode) {
        //case CAMERA_FLASH_TORCH:
        case CAMERA_FLASH_ON:
            rc = 1;
        break;
        case CAMERA_FLASH_AUTO:
            SR352_WRITE_ADDR(0x03, ((FLASH_PGARO>>8) & 0xFF));
            SR352_READ_ADDR((FLASH_PGARO & 0xFF), &bpga);

            SR352_WRITE_ADDR(0x03, ((FLASH_PGATH>>8) & 0xFF));
            SR352_READ_ADDR((FLASH_PGATH & 0xFF), &bpgath);

            if (bpga >= bpgath)
                rc = 1;
            else
                rc = 0;
        break;
        default:
            rc = 0;
    }

    SR352_WRITE_ADDR(0x03, ((FLASH_CTL1>>8) & 0xFF));
    SR352_READ_ADDR((FLASH_CTL1 & 0xFF), &flashctl);
    if (flashctl & 0x80)
        rc = 0;

    CDBG("FLASHDBG mode[%2x] bpga[%2x] bpgath[%2x] flashctl[%2x] isFlash[%2x]\n",
            flash_mode, bpga, bpgath, flashctl, rc);

    return rc;
}

void sr352_get_sensor_ev_data(struct msm_sensor_ctrl_t *s_ctrl, uint16_t PreFlashEnable)
{

    //PreFlashEnable = 0: Pre flash off timing
    //PreFlashEnable = 1: Pre flash on timing

    uint16_t    temp;
    uint16_t    wRgain, wGgain, wBgain;
    uint16_t    Pga, Dg, Yavg, Ytgt, Step;
    uint16_t    wTmp1, wTmp2, wTmp3, wTmp4, wTmp5, wTmp6;
    uint32_t    dwTmp1, dwTmp2, dwTmp3, dwTmp4, dwExptime, dwEv;

    SR352_WRITE_ADDR(0x03, 0x20);
    SR352_READ_ADDR(0xA4, &temp);
    dwTmp1 = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xA5, &temp);
    dwTmp2 = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xA6, &temp);
    dwTmp3 = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xA7, &temp);
    dwTmp4 = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xAB, &Pga);
    SR352_READ_ADDR(0xAE, &Dg);

    SR352_WRITE_ADDR(0x03, 0xC7);
    SR352_READ_ADDR(0x7B, &Yavg);//Y Mean
    SR352_READ_ADDR(0x7C, &Ytgt);//Y Target
    SR352_READ_ADDR(0x99, &Step);//BandStep

    dwExptime   = (dwTmp1<<24) + (dwTmp2<<16) + (dwTmp3<<8) + dwTmp4; //Org Exptime
    dwEv        = (uint32_t)((dwExptime * (((uint32_t)Pga*100)/32)*1)/100);
    CDBG_FL("%d FLDBG func0 dwEv[%u]\n", __LINE__, dwEv);

    SR352_WRITE_ADDR(0x03, 0x16);
    SR352_READ_ADDR(0xC0, &temp);
    wTmp1 = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0xC1, &temp);
    wTmp2 = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0xC2, &temp);
    wTmp3 = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0xC3, &temp);
    wTmp4 = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0xC4, &temp);
    wTmp5 = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0xC5, &temp);
    wTmp6 = (uint16_t)temp;
    temp=0;
    wRgain  = (wTmp1<<8) + wTmp2;
    wBgain  = (wTmp3<<8) + wTmp4;
    wGgain  = (wTmp5<<8) + wTmp6;

    if(PreFlashEnable) {
        Flash.dwPreFlashEv  = dwEv;         //Org Ev
        Flash.dwPreExptime  = dwExptime;    //Org Exptime
        Flash.bPrePga       = Pga;
        Flash.bPreDg        = Dg;
        Flash.bPreFlashY    = Yavg;         //Y Mean
        Flash.bPreFlashTgt  = Ytgt;         //Y Target
        Flash.wPreFlashRgain= wRgain;
        Flash.wPreFlashGgain= wGgain;
        Flash.wPreFlashBgain= wBgain;
    } else {
        Flash.dwOrgEv       = dwEv;         //Org Ev
        Flash.dwOrgExptime  = dwExptime;    //Org Exptime
        Flash.bOrgPga       = Pga;
        Flash.bOrgDg        = Dg;
        Flash.bOrgY         = Yavg;         //Y Mean
        Flash.bOrgTgt       = Ytgt;         //Y Target
        Flash.wOrgRgain     = wRgain;
        Flash.wOrgGgain     = wGgain;
        Flash.wOrgBgain     = wBgain;
    }
}

void sr352_get_ev_data_flash_Off_func1(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t    temp;
    uint16_t    wTmp1, wTmp2, wTmp3, wTmp4, wTmp5, wTmp6;
    uint32_t    dwTmp1, dwTmp2, dwTmp3, dwTmp4;

    SR352_WRITE_ADDR(0x03, ((FLASH_PRE_FLASHRATE>>8)&0xFF));
    SR352_READ_ADDR(FLASH_PRE_FLASHRATE &0xFF, &(Flash.bPFlashRate ));//PreFlash:20%
    SR352_READ_ADDR(FLASH_MAIN_FLASHRATE&0xFF, &(Flash.bMFlashRate));//MainFlash:80%

    ///////////////////////////////////////////////////////////////////////////////////////
    //For AE
    ///////////////////////////////////////////////////////////////////////////////////////
    SR352_WRITE_ADDR(0x03, 0x20);
    SR352_READ_ADDR(0x51, &(Flash.bAgMax));
    SR352_READ_ADDR(0x52, &(Flash.bAgMin));
    SR352_READ_ADDR(0x71, &(Flash.bDgMax));
    SR352_READ_ADDR(0x72, &(Flash.bDgMin));
    SR352_READ_ADDR(0x24, &temp);
    dwTmp1 = temp;
    temp = 0;

    SR352_READ_ADDR(0x25, &temp);
    dwTmp2 = temp;
    temp = 0;

    SR352_READ_ADDR(0x26, &temp);
    dwTmp3 = temp;
    temp = 0;

    SR352_READ_ADDR(0x27, &temp);
    dwTmp4 = temp;
    temp = 0;

    Flash.dwExpMax  = (dwTmp1<<24) + (dwTmp2<<16) + (dwTmp3<<8) + dwTmp4;

    SR352_WRITE_ADDR(0x03, 0xC7);
    SR352_READ_ADDR(0x38, &(Flash.bDgRatio));
    SR352_READ_ADDR(0x72, &(Flash.bDgCtl));
    SR352_READ_ADDR(0x73, &(Flash.bAntiFlag));
    SR352_READ_ADDR(0x76, &temp);
    dwTmp1  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0x77, &temp);
    dwTmp2  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0x78, &temp);
    dwTmp3  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0x79, &temp);
    dwTmp4  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0x39, &temp);
    wTmp1 = temp;
    temp = 0;
    SR352_READ_ADDR(0x3A, &temp);
    wTmp2 = temp;
    temp = 0;
    Flash.dwExpBand = (dwTmp1<<24) + (dwTmp2<<16) + (dwTmp3<<8) + dwTmp4; //ExpBand
    Flash.wOneLine  = (wTmp1<<8) + wTmp2; //OneLine

    if (Flash.bDgRatio == 0) {
        pr_err("%s:%d ERROR: bDgRatio is Zero Here\n", __func__, __LINE__);
        return;
    }

    Flash.fAgMin    = ((uint32_t)Flash.bAgMin * 100)/32;
    Flash.fDgMin    = ((uint32_t)Flash.bDgMin * 100)/Flash.bDgRatio;
    Flash.fAgMax    = ((uint32_t)Flash.bAgMax * 100)/32;
    Flash.fDgMax    = ((uint32_t)Flash.bDgMax * 100)/Flash.bDgRatio;

    Flash.dwExpMin  = (uint32_t)Flash.wOneLine*4;
    Flash.dwEvMin   = (uint32_t)((Flash.dwExpMin * (Flash.fAgMin * 1)/100));

    //Get Ev data before Pre Flash time
    SR352_WRITE_ADDR(0x03, 0xC7);
    SR352_READ_ADDR(0xA7, &temp);
    dwTmp1  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xA8, &temp);
    dwTmp2  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xA9, &temp);
    dwTmp3  = (uint32_t)temp;
    temp=0;
    SR352_READ_ADDR(0xAA, &temp);
    dwTmp4  = (uint32_t)temp;
    temp=0;
    Flash.dwOrgEvRO =  (dwTmp1<<24) + (dwTmp2<<16) + (dwTmp3<<8) + dwTmp4; //Sensor EV
    ///////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////
    //For AWB
    ///////////////////////////////////////////////////////////////////////////////////////
    SR352_WRITE_ADDR(0x03, 0xCC);
    SR352_READ_ADDR(0x66, &temp);
    wTmp1   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x67, &temp);
    wTmp2   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x68, &temp);
    wTmp3   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x69, &temp);
    wTmp4   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x6E, &temp);
    dwTmp1  = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x6F, &temp);
    dwTmp2  = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x70, &temp);
    dwTmp3  = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x71, &temp);
    dwTmp4  = (uint16_t)temp;
    temp=0;
    Flash.wRgainMin = (uint16_t)((wTmp1 <<8) + wTmp2);
    Flash.wRgainMax = (uint16_t)((wTmp3 <<8) + wTmp4);
    Flash.wBgainMin = (uint16_t)((dwTmp1<<8) + dwTmp2);
    Flash.wBgainMax = (uint16_t)((dwTmp3<<8) + dwTmp4);

    SR352_WRITE_ADDR(0x03, ((FLASH_AWB_YDIFF>>8)&0xFF));
    SR352_READ_ADDR(FLASH_AWB_YDIFF&0xFF, &(Flash.bYdiffTh ));//YDiffTh
    SR352_READ_ADDR((FLASH_AWB_TGTRGAIN&0xFF) , &temp );
    wTmp1   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR((FLASH_AWB_TGTRGAIN&0xFF)+1, &temp);
    wTmp2   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR((FLASH_AWB_TGTGGAIN&0xFF)  , &temp);
    wTmp3   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR((FLASH_AWB_TGTGGAIN&0xFF)+1, &temp);
    wTmp4   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR((FLASH_AWB_TGTBGAIN&0xFF)  , &temp);
    wTmp5   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR((FLASH_AWB_TGTBGAIN&0xFF)+1, &temp);
    wTmp6   = (uint16_t)temp;
    temp=0;
    Flash.wTgtRgain = (uint16_t)((wTmp1 <<8) + wTmp2);
    Flash.wTgtGgain = (uint16_t)((wTmp3 <<8) + wTmp4);
    Flash.wTgtBgain = (uint16_t)((wTmp5 <<8) + wTmp6);

    SR352_WRITE_ADDR(0x03, 0xCC);
    SR352_READ_ADDR(0x5C, &temp);
    wTmp1   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x5D, &temp);
    wTmp2   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x5E, &temp);
    wTmp3   = (uint16_t)temp;
    temp=0;
    SR352_READ_ADDR(0x5F, &temp);
    wTmp4   = (uint16_t)temp;
    temp=0;
    Flash.wOrgRgTgt = (uint16_t)((wTmp1 <<8) + wTmp2);
    Flash.wOrgBgTgt = (uint16_t)((wTmp3 <<8) + wTmp4);
    /////////////////////////////////////////////////////////////////////////////////////////
    //For test code
    //Flash.bYdiffTh    = 10;
    //Flash.wTgtRgain = 0x5C0;
    //Flash.wTgtGgain = 0x400;
    //Flash.wTgtBgain = 0x680;

    sr352_get_sensor_ev_data(s_ctrl,0);
    CDBG_FL("FLDBG1[func:%s] dwOrgEv[0x%x] dwOrgExptime[0x%x] bOrgPga[0x%x] bOrgDg[0x%x] bOrgY[0x%x] bOrgTgt[0x%x] dwOrgEvRO[0x%x]\n",
            __func__, Flash.dwOrgEv, Flash.dwOrgExptime, Flash.bOrgPga, Flash.bOrgDg, Flash.bOrgY, Flash.bOrgTgt, Flash.dwOrgEvRO);
}

void sr352_get_ev_data_preflash_func2(struct msm_sensor_ctrl_t *s_ctrl)
{
    SR352_WRITE_ADDR(0x03,((FLASH_CTL1>>8)&0xFF));
    SR352_READ_ADDR(FLASH_CTL1&0xFF, &(Flash.bFlashCtl1));

    //--------------------------------------------//
    //Get Ev data after end of Pre Flash time
    sr352_get_sensor_ev_data(s_ctrl, 1);
    //--------------------------------------------//
    //Checking Ev data variation
    if ((Flash.dwOrgEv >= Flash.dwPreFlashEv) && !(Flash.bFlashCtl1 & 0x80)) {

        if (!Flash.bOrgY || !Flash.dwPreFlashEv || !Flash.bPFlashRate) {
            pr_err("%s:%d ERROR: bOrgY[%d] dwPreFlashEv[%d] bPFlashRate[%d] Divide By ZERO\n",
            __func__, __LINE__, Flash.bOrgY, Flash.dwPreFlashEv, Flash.bPFlashRate);
            return;
        }
        Flash.fPreFlash_Ymean   = (uint32_t)(((uint32_t)((Flash.dwOrgEv*100) / Flash.dwPreFlashEv) *\
                        ((uint32_t)((uint32_t)Flash.bPreFlashY*1000) / (uint32_t)Flash.bOrgY) * Flash.bOrgY)/10000);
        CDBG_FL("%d func2 FLDBG2 Flash.fPreFlash_Ymen[%u] = ([%u]/[%u])*([%u]/[%u])*[%u]\n", __LINE__,
            Flash.fPreFlash_Ymean, Flash.dwOrgEv, Flash.dwPreFlashEv, Flash.bPreFlashY, Flash.bOrgY, Flash.bOrgY);

        Flash.fPreFlash_Yinc    = (uint32_t)((Flash.fPreFlash_Ymean * 100)/ Flash.bOrgY);
        CDBG_FL("%d func2 FLDBG2 Flash.fPreFlash_Yinc[%u] = [%u]/[%u]\n", __LINE__,
            Flash.fPreFlash_Yinc, Flash.fPreFlash_Ymean, Flash.bOrgY);
        Flash.fMainFlash_Rate   = (((uint32_t)Flash.bMFlashRate * 1000)/(uint32_t)Flash.bPFlashRate)/1000;
        if (Flash.fPreFlash_Ymean >= ((uint32_t)Flash.bOrgY*10)) {
            Flash.fPreFlash_YDiff   = (Flash.fPreFlash_Ymean - ((uint32_t)Flash.bOrgY*10));
            Flash.fMainFlash_Ymean  = ((Flash.fPreFlash_YDiff * Flash.fMainFlash_Rate) + Flash.fPreFlash_Ymean)/10;
        } else {
            Flash.fPreFlash_YDiff   = (((uint32_t)Flash.bOrgY*10) - Flash.fPreFlash_Ymean);
            Flash.fMainFlash_Ymean  = (Flash.fPreFlash_Ymean - (Flash.fPreFlash_YDiff * Flash.fMainFlash_Rate))/10;
        }

        CDBG_FL("%d func2 FLDBG2 Flash.fPreFlash_YDiff[%u] = [%u] - [%u]\n", __LINE__,
            Flash.fPreFlash_YDiff, Flash.fPreFlash_Ymean, Flash.bOrgY);

        if (Flash.bFlashCtl1 & 0x10) {
            CDBG_FL("%d func2 FLDBG2 NEVER HERE\n", __LINE__);

            SR352_WRITE_ADDR(0x03,((FLASH_CALCRATIOTH>>8)&0xFF));
            SR352_READ_ADDR(FLASH_CALCRATIOTH&0xFF, &(Flash.bFlashCalcRatioTh));
            SR352_READ_ADDR(FLASH_CALCRATIOMAX&0xFF, &(Flash.bFlashCalcRatioMax));

            if (!Flash.bFlashCalcRatioTh) {
                pr_err("%s:%d ERROR bFlashCalcRatioTh[%d] Divide By Zero!\n",
                    __func__, __LINE__, Flash.bFlashCalcRatioTh);
                return;
            }

            if( Flash.fPreFlash_Yinc < 1000) {
                //Flash.fFlashCalcRatio = ((((Flash.fPreFlash_Yinc-1)*100) / Flash.bFlashCalcRatioTh)/1000);

                Flash.fFlashCalcRatio = ((((1000-Flash.fPreFlash_Yinc)*100) / Flash.bFlashCalcRatioTh)/1000);
            } else {
                Flash.fFlashCalcRatio = (((((int)Flash.fPreFlash_Yinc-1000)*100) / Flash.bFlashCalcRatioTh)/1000);
            }

            if(Flash.bFlashCalcRatioMax < Flash.fFlashCalcRatio)
                Flash.fFlashCalcRatio = Flash.bFlashCalcRatioMax;

            if( Flash.fPreFlash_Yinc < 1000) {
                //Flash.fMainFlash_Ymean =  (Flash.fMainFlash_Ymean * (((100-Flash.fFlashCalcRatio) * 1000)/100))/1000;
                Flash.fMainFlash_Ymean =  (Flash.fMainFlash_Ymean * ((100000+Flash.fFlashCalcRatio)/100000));
            } else {
                Flash.fMainFlash_Ymean =  (Flash.fMainFlash_Ymean * ((100000-Flash.fFlashCalcRatio)/100000));
            }
        }

        CDBG_FL("%d func2 FLDBG2 Flash.bPreFlashTgt[%u] >= Flash.fMainFlash_Ymean[%u]\n", __LINE__,
            Flash.bPreFlashTgt, Flash.fMainFlash_Ymean);
        if (Flash.bPreFlashTgt >= Flash.fMainFlash_Ymean) {
            Flash.fYcalRate = 100;
            CDBG_FL("%d func2 FLDBG2 Flash.fYcalRate[%u]\n", __LINE__, 1);
        } else {
            if (!Flash.bPreFlashTgt) {
                pr_err("%s:%d ERROR: bPreFlashTgt[%d] Divide By Zero\n",
                    __func__, __LINE__, Flash.bPreFlashTgt);
                return;
            }
            Flash.fYcalRate = ((Flash.fMainFlash_Ymean * 100)/ Flash.bPreFlashTgt);
            CDBG_FL("%d func2 FLDBG2 Flash.fYcalRate[%u] = [%u]/[%u]\n", __LINE__,
                Flash.fYcalRate, Flash.fMainFlash_Ymean, Flash.bPreFlashTgt);
        }

        if (!Flash.fYcalRate) {
            pr_err("%s:%d ERROR: fYcalRate[%d] Divide By Zero\n",
                __func__, __LINE__, Flash.fYcalRate);
            return;
        }

        Flash.dwMainFlashEv = ((Flash.dwOrgEv*100) / Flash.fYcalRate);
        CDBG_FL("%d func2 FLDBG2 Flash.dwMainFlashEv[%u] = [%u]/[%u]\n", __LINE__,
            Flash.dwMainFlashEv, Flash.dwOrgEv, Flash.fYcalRate);

        CDBG_FL("%d func2 FLDBG2 dwMainFlashEv[%u] < dwEvMin[%u]\n", __LINE__,
            Flash.dwMainFlashEv, Flash.dwEvMin);
        if (Flash.dwMainFlashEv < Flash.dwEvMin) {
            Flash.dwMainFlashEv = Flash.dwEvMin;
        }

        CDBG_FL("%d func2 FLDBG2 case1[0x%0x], case2[0x%0x]\n", __LINE__,
            ((uint32_t)(Flash.dwMainFlashEv/(Flash.wOneLine * Flash.fAgMin * Flash.fDgMin))) * Flash.wOneLine,
            ((uint32_t)(Flash.dwMainFlashEv/(Flash.dwExpBand * Flash.fAgMin * Flash.fDgMin))) * Flash.dwExpBand);

        //Checking whether Indoor or outdoor condition. --> //Exptime step calc
        if (((Flash.bAntiFlag & ~0x02)&&(Flash.dwMainFlashEv < (Flash.dwExpBand)))\
            || ((Flash.bAntiFlag & 0x02)&&(Flash.dwMainFlashEv < (Flash.dwExpBand*2)))) {

            //Outdoor condition
            if (!Flash.wOneLine || !Flash.fAgMin) {
                pr_err("%s:%d ERROR: wOneLine[%d], fAgMin[%d] Divide By Zero\n",
                    __func__, __LINE__, Flash.wOneLine, Flash.fAgMin);
                return;
            }

            Flash.dwMainFlashBandStep   = (uint32_t)(((Flash.dwMainFlashEv*10)/((Flash.wOneLine * Flash.fAgMin * 1)/100))/10);
            Flash.dwMainFlashExp        = Flash.dwMainFlashBandStep * Flash.wOneLine;
            CDBG_FL("%d func2 FLDBG2 Outdoor [%u]\n", __LINE__, Flash.dwMainFlashExp);
        } else {

            //Indoor condition
            if (!Flash.dwExpBand|| !Flash.fAgMin) {
                pr_err("%s:%d ERROR: dwExpBand[%d], fAgMin[%d] Divide By Zero\n",
                    __func__, __LINE__, Flash.dwExpBand, Flash.fAgMin);
                return;
            }
            Flash.dwMainFlashBandStep   = (uint32_t)(((Flash.dwMainFlashEv*10)/(Flash.dwExpBand * (Flash.fAgMin * 1/100)))/10);
            Flash.dwMainFlashExp        = (Flash.dwMainFlashBandStep * Flash.dwExpBand);
            CDBG_FL("%d func2 FLDBG2 Indoor[%u]\n", __LINE__, Flash.dwMainFlashExp);
        }

        //Check EV limit
        if(Flash.dwMainFlashExp < Flash.dwExpMin) { Flash.dwMainFlashExp = Flash.dwExpMin;  }
        else if(Flash.dwMainFlashExp > Flash.dwExpMax) {    Flash.dwMainFlashExp = Flash.dwExpMax;  }

        //Calc PGA
        if (!Flash.dwMainFlashExp) {
            pr_err("%s:%d ERROR: dwMainFlashExp[%d] Divide By Zero\n",
                __func__, __LINE__, Flash.dwMainFlashExp);
            return;
        }
        Flash.fMainFlashAg = ((Flash.dwMainFlashEv*10)/(Flash.dwMainFlashExp *1))*100/10;
        if(Flash.fMainFlashAg < Flash.fAgMin){  Flash.fMainFlashAg = Flash.fAgMin;  }
        else if(Flash.fMainFlashAg > Flash.fAgMax){ Flash.fMainFlashAg = Flash.fAgMax;  }

        Flash.bMainFlashAg  = (uint8_t)((Flash.fMainFlashAg * 32)/100);

        //Calc DG
        if(Flash.bDgCtl & 0x01) {
            //DGain On
            Flash.fMainFlashDg = ((Flash.dwMainFlashEv*10) / (Flash.dwMainFlashExp * Flash.fMainFlashAg/100))/10*100;
            if(Flash.fMainFlashDg < Flash.fDgMin){  Flash.fMainFlashDg = Flash.fDgMin;  }
            else if(Flash.fMainFlashDg > Flash.fDgMax){ Flash.fMainFlashDg = Flash.fDgMax;  }
            Flash.bMainFlashDg = (uint8_t)((Flash.fMainFlashDg * Flash.bDgRatio)/100);
        } else {
            Flash.bMainFlashDg = Flash.bDgRatio;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // AWB cal
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(Flash.fPreFlash_Yinc < 1000) {
        Flash.fPreFlash_Yinc = 1000;
    }

    Flash.fYdiffRatio = ((((int)Flash.fPreFlash_Yinc-1000)/((uint32_t)Flash.bYdiffTh))*100)/1000;

    if(Flash.fYdiffRatio > 100){    Flash.fYdiffRatio = 100;    }
    else if(Flash.fYdiffRatio < 0){ Flash.fYdiffRatio = 0;      }

    Flash.wMainFlashRgain = (uint16_t)(((((((int)Flash.wTgtRgain-Flash.wPreFlashRgain)*10)/100) *\
                    (int)Flash.fYdiffRatio )/10 + Flash.wPreFlashRgain ));
    Flash.wMainFlashGgain = (uint16_t)(((((((int)Flash.wTgtGgain-Flash.wPreFlashGgain)*10)/100) *\
                    (int)Flash.fYdiffRatio )/10 + Flash.wPreFlashGgain ));
    Flash.wMainFlashBgain = (uint16_t)(((((((int)Flash.wTgtBgain-Flash.wPreFlashBgain)*10)/100) *\
                    (int)Flash.fYdiffRatio )/10 + Flash.wPreFlashBgain ));

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        Flash.bState = 0x01;
    } else {
        Flash.bState = 0x00;
    }

    CDBG_FL("FLDBG1[func:%s] dwPreFlashEv[0x%x] dwPreExptime[0x%x] bPrePga[0x%x] bPreDg[0x%x] bPreFlashY[0x%x] bPreFlashTgt[0x%x]"
            " dwMainFlashEv[0x%x] dwMainFlashExp[0x%x] bMainFlashAg[0x%x] bMainFlashDg[0x%x]\n",
            __func__, Flash.dwPreFlashEv, Flash.dwPreExptime, Flash.bPrePga, Flash.bPreDg, Flash.bPreFlashY, Flash.bPreFlashTgt,
            Flash.dwMainFlashEv, Flash.dwMainFlashExp, Flash.bMainFlashAg, Flash.bMainFlashDg);
}

void sr352_set_ev_data_mainFlash_func3(struct msm_sensor_ctrl_t *s_ctrl)
{
    //WriteEvDataForMainFlash
    if(Flash.bState) {

        SR352_WRITE_ADDR(0x03,((FLASH_CTL1>>8)&0xFF));
        SR352_READ_ADDR(FLASH_CTL1&0xFF,  &(Flash.bFlashCtl1));

        if(!(Flash.bFlashCtl1 & 0x40)) {

            //////////////////////////////////////////////////////
            //AE Register Update[ ExpTime, Gain ]
            //////////////////////////////////////////////////////
            SR352_WRITE_ADDR(0x03, 0xC7);
            SR352_WRITE_ADDR(0x44, (Flash.dwMainFlashExp>>24)&0xFF);//ExpTime
            SR352_WRITE_ADDR(0x45, (Flash.dwMainFlashExp>>16)&0xFF);
            SR352_WRITE_ADDR(0x46, (Flash.dwMainFlashExp>> 8)&0xFF);
            SR352_WRITE_ADDR(0x47, (Flash.dwMainFlashExp    )&0xFF);
            SR352_WRITE_ADDR(0x40, Flash.bMainFlashAg);//Gain
            SR352_WRITE_ADDR(0x41, Flash.bMainFlashDg);
            //Update New ExpTime & Gain
            SR352_WRITE_ADDR(0x03, 0x20);
            SR352_WRITE_ADDR(0x20, (Flash.dwMainFlashExp>>24)&0xFF);
            SR352_WRITE_ADDR(0x21, (Flash.dwMainFlashExp>>16)&0xFF);
            SR352_WRITE_ADDR(0x22, (Flash.dwMainFlashExp>> 8)&0xFF);
            SR352_WRITE_ADDR(0x23, (Flash.dwMainFlashExp    )&0xFF);
            SR352_WRITE_ADDR(0x50, Flash.bMainFlashAg);
            SR352_WRITE_ADDR(0x70, Flash.bMainFlashDg);
            //Update flag
            SR352_WRITE_ADDR(0x03, 0xc7);
            SR352_WRITE_ADDR(0x76, 0x00);
            SR352_WRITE_ADDR(0x77, 0x00);
            SR352_WRITE_ADDR(0x78, 0x00);
            SR352_WRITE_ADDR(0x79, 0x00);
        }

        if(!(Flash.bFlashCtl1 & 0x20)) {
            //////////////////////////////////////////////////////
            //AWB Register Update
            //////////////////////////////////////////////////////
            SR352_WRITE_ADDR(0x03, 0x16);
            SR352_WRITE_ADDR(0xA2, (Flash.wMainFlashRgain>>8)&0xFF);
            SR352_WRITE_ADDR(0xA3, (Flash.wMainFlashRgain   )&0xFF);
            SR352_WRITE_ADDR(0xA4, (Flash.wMainFlashBgain>>8)&0xFF);
            SR352_WRITE_ADDR(0xA5, (Flash.wMainFlashBgain   )&0xFF);
            SR352_WRITE_ADDR(0xA6, (Flash.wMainFlashGgain>>8)&0xFF);
            SR352_WRITE_ADDR(0xA7, (Flash.wMainFlashGgain   )&0xFF);
        }

        if(Flash.bFlashCtl1 & 0x08) {

            SR352_WRITE_ADDR(0x03, 0x00);
            SR352_WRITE_ADDR(0x60, (Flash.bFlashCtl1&0x07));
        }
    }
}

void sr352_return_ev_data_func4(struct msm_sensor_ctrl_t *s_ctrl)
{

    uint16_t i, bTmp1, bTmp2;

    if(Flash.bState) {
        //////////////////////////////////////////////////////
        //AE Register Update[ ExpTime, Gain ]
        //////////////////////////////////////////////////////
        SR352_WRITE_ADDR(0x03, 0x20);
        SR352_WRITE_ADDR(0x20, (Flash.dwOrgExptime>>24)&0xFF);  //HW ExpTime
        SR352_WRITE_ADDR(0x21, (Flash.dwOrgExptime>>16)&0xFF);
        SR352_WRITE_ADDR(0x22, (Flash.dwOrgExptime>> 8)&0xFF);
        SR352_WRITE_ADDR(0x23, (Flash.dwOrgExptime    )&0xFF);
        SR352_WRITE_ADDR(0x50, Flash.bOrgPga);                  //HW pga
        SR352_WRITE_ADDR(0x70, Flash.bOrgDg);
        //Update EV
        SR352_WRITE_ADDR(0x03, 0xC7);
        SR352_WRITE_ADDR(0xA7, (Flash.dwOrgEvRO>>24)&0xFF); //EV
        SR352_WRITE_ADDR(0xA8, (Flash.dwOrgEvRO>>16)&0xFF);
        SR352_WRITE_ADDR(0xA9, (Flash.dwOrgEvRO>> 8)&0xFF);
        SR352_WRITE_ADDR(0xAA, (Flash.dwOrgEvRO    )&0xFF);
        //Update Exp
        SR352_WRITE_ADDR(0x03, 0xC7);
        SR352_WRITE_ADDR(0x44, (Flash.dwOrgExptime>>24)&0xFF);  //SW ExpTime
        SR352_WRITE_ADDR(0x45, (Flash.dwOrgExptime>>16)&0xFF);
        SR352_WRITE_ADDR(0x46, (Flash.dwOrgExptime>> 8)&0xFF);
        SR352_WRITE_ADDR(0x47, (Flash.dwOrgExptime    )&0xFF);
        SR352_WRITE_ADDR(0x40, Flash.bOrgPga);                  //SW pga
        SR352_WRITE_ADDR(0x41, Flash.bOrgDg);
        SR352_WRITE_ADDR(0x7b, Flash.bOrgY);
        //////////////////////////////////////////////////////
        //AWB Register Update
        //////////////////////////////////////////////////////
        SR352_WRITE_ADDR(0x03, 0x16);
        SR352_WRITE_ADDR(0xA2, (Flash.wOrgRgain>>8)&0xFF);
        SR352_WRITE_ADDR(0xA3, (Flash.wOrgRgain   )&0xFF);
        SR352_WRITE_ADDR(0xA4, (Flash.wOrgBgain>>8)&0xFF);
        SR352_WRITE_ADDR(0xA5, (Flash.wOrgBgain   )&0xFF);
        SR352_WRITE_ADDR(0xA6, (Flash.wOrgGgain>>8)&0xFF);
        SR352_WRITE_ADDR(0xA7, (Flash.wOrgGgain   )&0xFF);
        //AWB state reset
        SR352_WRITE_ADDR(0x03, 0xCA);
        SR352_WRITE_ADDR(0xA1, 0x00);
        SR352_WRITE_ADDR(0xA3, 0x00);
        SR352_WRITE_ADDR(0x03, 0xCC);
        SR352_WRITE_ADDR(0x74, 0x00);
        //AWB Tgt initial
        SR352_WRITE_ADDR(0x03, 0x26);
        SR352_READ_ADDR(0xAA, &bTmp1);
        SR352_READ_ADDR(0xAB, &bTmp2);
        SR352_WRITE_ADDR(0xAA, 0x0B);
        SR352_WRITE_ADDR(0xAB, 0xE7);
#if 0
        Flash.wOrgRgTgt = 0x800;
        Flash.wOrgBgTgt = 0x800;
#endif
        SR352_WRITE_ADDR(0x03, 0xCD);
        for(i=0; i<15; i++) {
            SR352_WRITE_ADDR((0x10+(i*2)  ), ((Flash.wOrgRgTgt>>8)&0xFF));
            SR352_WRITE_ADDR((0x10+(i*2)+1), ((Flash.wOrgRgTgt   )&0xFF));
            SR352_WRITE_ADDR(((0x2E)+(i*2)  ), ((Flash.wOrgBgTgt>>8)&0xFF));
            SR352_WRITE_ADDR(((0x2E)+(i*2)+1), ((Flash.wOrgBgTgt   )&0xFF));
        }

        SR352_WRITE_ADDR(0x03, 0x26);
        SR352_WRITE_ADDR(0xAA, bTmp1);
        SR352_WRITE_ADDR(0xAB, bTmp2);

        Flash.bState = 0x00;
    }
}

void sr352_recording_landing(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t Recording_Pos = 0;
    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x14, &Recording_Pos);
    Recording_Pos |= 0x10;
    SR352_WRITE_ADDR(0x14, Recording_Pos);
    SR352_WRITE_ADDR(0x03, 0xD0);
    SR352_WRITE_ADDR(0x7c, 0x00);
    msleep(130);
    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x14, & Recording_Pos);
    Recording_Pos &= ~(0x10);
}
void sr352_actuator_softlanding(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t landing_reg = 0;

    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x14, &landing_reg);
    landing_reg |= 0x10;
    SR352_WRITE_ADDR(0x14, landing_reg);
    SR352_WRITE_ADDR(0x03, 0xD0);
    SR352_WRITE_ADDR(0x7c, 0x00);
    msleep(130);
    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x14, &landing_reg);
    landing_reg &= ~(0x10);
    SR352_WRITE_ADDR(0x14, landing_reg);
}

void sr352_cancel_af(struct msm_sensor_ctrl_t *s_ctrl)
{
    uint16_t focus_reg = 0;

    SR352_WRITE_ADDR(0x03, 0xCF);
    SR352_READ_ADDR(0x11, &focus_reg);

    focus_reg |= 0x08; /* set 3rd bit */

    CDBG("DAFF %s focus_reg[0x%2x]\n", __func__, focus_reg);
    SR352_WRITE_ADDR(0x11, focus_reg);
}

int32_t sr352_set_af_status(struct msm_sensor_ctrl_t *s_ctrl, int status, int initial_pos)
{
    int rc = -EINVAL;

    switch(status) {
        case SENSOR_AF_START:
            CDBG("SENSOR_AF_START\n");
            is_af_run = 1;
            is_preflash = sr352_is_required_flash(s_ctrl, flash_mode);

            if (is_preflash) {
                memset(&Flash, 0, sizeof(Flash));
                SR352_WRITE_LIST(sr352_StartPreFlash_FlashRegTable1);
                sr352_get_ev_data_flash_Off_func1(s_ctrl);
                SR352_WRITE_LIST(sr352_ReadyPreFlash_FlashRegTable2);
                set_led_flash(MSM_CAMERA_LED_LOW);
                flash_status = MSM_CAMERA_LED_LOW;
                rc = SENSOR_AF_PRE_FLASH_ON;
                need_main_flash = 1;
            } else {
                sr352_start_af(s_ctrl);
                need_main_flash = 0;
                rc = SENSOR_AF_START;
            }
        break;
        case SENSOR_AF_PRE_FLASH_OFF:
            CDBG("SENSOR_AF_PRE_FLASH_OFF\n");
            if (is_preflash) {
                SR352_WRITE_LIST(sr352_AfterPreFlash_FlashRegTable3);
                sr352_get_ev_data_preflash_func2(s_ctrl);
                SR352_WRITE_LIST(sr352_EndPreFlash_FlashRegTable4);
                if (!is_touchaf)
                    msleep(100);

                set_led_flash(MSM_CAMERA_LED_OFF);
                if (is_touchaf) {
                    is_touchaf = 0;
                }
                flash_status = MSM_CAMERA_LED_OFF;
                is_preflash = 0;
            }
            rc = SENSOR_AF_PRE_FLASH_OFF;
        break;
        case SENSOR_AF_PRE_FLASH_AE_STABLE:
            CDBG("SENSOR_AF_PRE_FLASH_AE_STABLE\n");
            if (is_preflash) {

                sr352_start_af(s_ctrl);
            }
            rc = SENSOR_AF_START;
        break;
        case SENSOR_AF_CANCEL:
            CDBG("SENSOR_AF_CANCEL\n");
            sr352_cancel_af(s_ctrl);
            is_af_run = 0;
        break;
        default:
            pr_err("%s:%d Invalid argument\n", __func__, __LINE__);
    }

    return rc;
}
int32_t sr352_get_af_status(struct msm_sensor_ctrl_t *s_ctrl, int is_search_status)
{
    unsigned short af_status = 0;
    CDBG("is_search_status %d\n", is_search_status);
    switch (is_search_status) {
        case 0:
            SR352_WRITE_ADDR(0x03, 0xD2);
            SR352_READ_ADDR(0x11, &af_status);
            CDBG("1st AF status before : %x\n", af_status);
            if (af_status & 0x04)
                af_status = 2; // AF completed.
            else
                af_status = 1; //still 1st search running.
            CDBG("1st AF status after: %x\n", af_status);
        break;
        case 1:
            SR352_WRITE_ADDR(0x03, 0xD2);
            SR352_READ_ADDR(0x11, &af_status);
            if (af_status & 0x02)
                af_status = 0; //success
            else if (af_status & 0x01)
                af_status = 1; //failure
            else
                af_status = -1; // AF status unknown.
            CDBG("2nd AF status : %d\n", af_status);
        break;
        default:
            pr_err("%s:%d unexpected mode is coming from HAL\n", __func__, __LINE__);
    }
    CDBG("return_af_status = %d\n", af_status);
    return  af_status;
}
#endif

int32_t sr352_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
        void __user *argp)
{
    int32_t rc = 0;
    struct ioctl_native_cmd *cam_info = (struct ioctl_native_cmd *)argp;

    mutex_lock(s_ctrl->msm_sensor_mutex);

    /*CDBG("cam_info values = %d : %d : %d : %d : %d\n", cam_info->mode, cam_info->address, cam_info->value_1, cam_info->value_2 , cam_info->value_3);*/
    switch (cam_info->mode) {
        case EXT_CAM_EV:
            sr352_ctrl.settings.exposure = (cam_info->value_1);
            if(sr352_ctrl.streamon == 1)
                sr352_set_exposure_compensation(s_ctrl, sr352_ctrl.settings.exposure);
        break;
        case EXT_CAM_WB:
            sr352_ctrl.settings.wb = (cam_info->value_1);
            if(sr352_ctrl.streamon == 1)
                sr352_set_white_balance(s_ctrl, sr352_ctrl.settings.wb);
        break;
        case EXT_CAM_METERING:
            sr352_ctrl.settings.metering = (cam_info->value_1);
            if(sr352_ctrl.streamon == 1)
                sr352_set_metering(s_ctrl, sr352_ctrl.settings.metering);
        break;
        case EXT_CAM_EFFECT:
            sr352_ctrl.settings.effect = (cam_info->value_1);
            if(sr352_ctrl.streamon == 1)
                sr352_set_effect(s_ctrl, sr352_ctrl.settings.effect);
        break;
        case EXT_CAM_SCENE_MODE:
            sr352_ctrl.settings.scenemode = (cam_info->value_1);
            cur_scene_mode_chg = 1;
            if(sr352_ctrl.streamon == 1)
                sr352_set_scene_mode(s_ctrl, sr352_ctrl.settings.scenemode);
        break;
        case EXT_CAM_SENSOR_MODE:
            sr352_ctrl.prev_mode =  sr352_ctrl.op_mode;
            sr352_ctrl.op_mode = (cam_info->value_1);
            pr_info("EXT_CAM_SENSOR_MODE = %d", sr352_ctrl.op_mode);
        break;
        case EXT_CAM_EXIF:
            sr352_get_exif(cam_info);
            if (!copy_to_user((void *)argp,
                        (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");

        break;
        case EXT_CAM_SET_AE_AWB:
            CDBG("EXT_CAM_SET_AE_AWB lock[%d]\n", cam_info->value_1);
            sr352_ctrl.settings.aeawblock = cam_info->value_1;
#if defined (AF_FLASH_SUPPORT)
            if(!flash_mode){
                CDBG("EXT_CAM_SET_AE_AWB, !flash_mode");
                sr352_set_ae_awb_lock(s_ctrl, sr352_ctrl.settings.aeawblock);
            }
#else
            sr352_set_ae_awb_lock(s_ctrl, sr352_ctrl.settings.aeawblock);
#endif
        break;
#if defined (AF_FLASH_SUPPORT)
        case EXT_CAM_FOCUS:
            CDBG("DAFF EXT_CAM_FOCUS focus mode[%d]\n", cam_info->value_1);
            focus_mode = (cam_info->value_1);
            if(sr352_ctrl.streamon == 1)
                sr352_set_af_mode(s_ctrl, focus_mode);
        break;
        case EXT_CAM_SET_AF_STATUS:
            CDBG("DAFF EXT_CAM_SET_AF_STATUS: %d : %d\n", cam_info->value_1, cam_info->value_2);

            cam_info->value_1 = sr352_set_af_status(s_ctrl, cam_info->value_1,
                    cam_info->value_2);

            if (!copy_to_user((void *)argp, (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");

        break;
        case EXT_CAM_SET_TOUCHAF_POS:
            CDBG("DAFF EXT_CAM_SET_TOUCHAF_POS: %d : %d", cam_info->value_1, cam_info->value_2);

            if(cam_info->value_1 !=0 && cam_info->value_2 != 0) {
                sr352_set_touchaf_pos(s_ctrl, cam_info->value_1, cam_info->value_2);
                is_touchaf = 1;
            } else {
                is_touchaf = 0;
            }

            sr352_set_touch_mode(s_ctrl, is_touchaf);

        break;
        case EXT_CAM_GET_AF_STATUS:
            CDBG("DAFF EXT_CAM_GET_AF_STATUS: %d\n", cam_info->value_1);

            cam_info->value_1 = sr352_get_af_status(s_ctrl, cam_info->value_1);
            if (!copy_to_user((void *)argp, (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");

        break;
        case EXT_CAM_FLASH_MODE:
            CDBG("DAFF EXT_CAM_FLASH_MODE  = %d\n", (cam_info->value_1));
            flash_mode = cam_info->value_1;
            if((flash_mode) == CAMERA_FLASH_TORCH) {
                set_led_flash(MSM_CAMERA_LED_LOW);
                flash_status = MSM_CAMERA_LED_LOW;
            } else {
                set_led_flash(MSM_CAMERA_LED_OFF);
                flash_status = MSM_CAMERA_LED_OFF;
            }
        break;
        case EXT_CAM_SET_FLASH:
            CDBG("DAFF EXT_CAM_SET_FLASH = %d\n", (cam_info->value_1));
            if (flash_mode != CAMERA_FLASH_TORCH) {
                if (need_main_flash) {
                    if (flash_status == MSM_CAMERA_LED_HIGH) {
                        set_led_flash(MSM_CAMERA_LED_OFF);
                        flash_status = MSM_CAMERA_LED_OFF;
                        SR352_WRITE_LIST(sr352_AfterMainFlash_FlashRegTable6);
                        sr352_return_ev_data_func4(s_ctrl);
                        SR352_WRITE_LIST(sr352_EndMainFlash_FlashRegTable7);
                        need_main_flash = 0;
                    } else {
                        flash_status = cam_info->value_1;
                        set_led_flash(flash_status);
                    }
                }
            }
        break;
#endif
        default:
            rc = 0;
    }

    mutex_unlock(s_ctrl->msm_sensor_mutex);

    return rc;
}

void sr352_set_default_settings(void)
{
    sr352_ctrl.settings.metering = CAMERA_CENTER_WEIGHT;
    sr352_ctrl.settings.exposure = CAMERA_EV_DEFAULT;
    sr352_ctrl.settings.wb = CAMERA_WHITE_BALANCE_AUTO;
    sr352_ctrl.settings.iso = CAMERA_ISO_MODE_AUTO;
    sr352_ctrl.settings.effect = CAMERA_EFFECT_OFF;
    sr352_ctrl.settings.scenemode = CAMERA_SCENE_AUTO;
    sr352_ctrl.settings.aeawblock = 0;
}

#ifndef NO_BURST
int32_t sr352_sensor_burst_write(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_conf *reg_settings , int size)
{
    int i;
    int err;
    int idx = 0;
    int seq_idx = 0;
    int burst_flag = 0;
    int seq_flag = 0;
    unsigned char subaddr;
    unsigned char value;
    static uint8_t burst_data[BURST_MODE_BUFFER_MAX_SIZE] = {0};

    struct msm_camera_i2c_burst_reg_array burst_reg_setting = { 0 ,};
    struct msm_camera_i2c_reg_setting conf_array = {
        .reg_setting = (void * ) &burst_reg_setting,
        .size = 1,
        .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
        .data_type = MSM_CAMERA_I2C_BURST_DATA,
    };

    struct msm_camera_i2c_seq_reg_array *seq_reg_setting  = NULL;
    struct msm_camera_i2c_seq_reg_setting seq_conf_array = {
        .reg_setting = NULL,
        .size = 0,
        .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
        .delay = 0,
    };
#if 0
    seq_reg_setting = kzalloc((size/I2C_SEQ_REG_DATA_MAX + 1) *\
                (sizeof(struct msm_camera_i2c_seq_reg_array)), GFP_KERNEL); // allocate maximum possible
#endif

    seq_reg_setting = kzalloc(4 * (sizeof(struct msm_camera_i2c_seq_reg_array)), GFP_KERNEL); // enough to fit sr352 seq. data
    if (!seq_reg_setting) {
        pr_err("%s:%d failed\n", __func__, __LINE__);
        err = -ENOMEM;
        goto on_error;
    }
    seq_conf_array.reg_setting = seq_reg_setting;

    for (i = 0; i < size; i++) {

        subaddr = reg_settings[i].reg_addr;
        value = reg_settings[i].reg_data;

        if (burst_flag == 0 && seq_flag == 0) {
            if (subaddr == BURST_REG && value != 0x00) {


                idx = 0;
                seq_idx = 0;
                if (reg_settings[i+1].reg_addr == reg_settings[i+2].reg_addr) {
                    burst_flag = 1;
                } else {
                    seq_flag = 1;
                }
            }

            err = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
                        s_ctrl->sensor_i2c_client, subaddr, value, MSM_CAMERA_I2C_BYTE_DATA);
        } else if (burst_flag == 1) {

            if (subaddr == BURST_REG && value == 0x00) {

                burst_flag = 0;
                burst_reg_setting.reg_burst_data = burst_data;
                burst_reg_setting.reg_data_size = idx;
                idx = 0;

                err = s_ctrl->sensor_i2c_client-> \
                      i2c_func_tbl->i2c_write_burst_table( \
                              s_ctrl->sensor_i2c_client, &conf_array);
                if (err < 0) {
                    pr_err("[%s:%d]Burst write fail!\n", \
                            __func__, __LINE__);
                    goto on_error;
                }

                err = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
                            s_ctrl->sensor_i2c_client, subaddr, value, MSM_CAMERA_I2C_BYTE_DATA);



            } else {
                if (idx == 0)
                    burst_reg_setting.reg_addr = subaddr;

                burst_data[idx] = value;
                idx++;
            }

        } else if (seq_flag == 1) {
            if (subaddr == BURST_REG && value == 0x00) {
                seq_flag = 0;
                idx = 0;
                seq_conf_array.size = seq_idx;
                seq_idx = 0;

                s_ctrl->sensor_i2c_client->i2c_func_tbl->\
                      i2c_write_seq_table(s_ctrl->sensor_i2c_client, &seq_conf_array);
                if (err < 0) {
                    pr_err("[%s:%d]Sequential write fail!\n", __func__, __LINE__);
                        goto on_error;
                }

                err = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
                            s_ctrl->sensor_i2c_client, subaddr, value, MSM_CAMERA_I2C_BYTE_DATA);


            } else {
                if (idx % I2C_SEQ_REG_DATA_MAX == 0) {
                    seq_reg_setting[seq_idx].reg_addr = subaddr;
                    seq_reg_setting[seq_idx].reg_data_size = 0;
                    seq_idx ++;
                    if (seq_idx >=4) {
                       pr_err("[%s:%d]error: More memory needed!!\n", __func__, __LINE__);
                       goto on_error;
                    }
                }

                seq_reg_setting[seq_idx - 1].\
                       reg_data[seq_reg_setting[seq_idx - 1].reg_data_size++] = value;

                idx++;
            }
        }
    } // End of for loop

on_error:
   if (seq_reg_setting)
       kfree(seq_reg_setting);

   return 0;
}
#endif

#ifdef CONFIG_LOAD_FILE
int sr352_regs_from_sd_tunning(struct msm_camera_i2c_reg_conf *settings, struct msm_sensor_ctrl_t *s_ctrl,char * name) {
    char *start, *end, *reg;
    int addr,rc = 0;
    unsigned int  value;
    char reg_buf[5], data_buf1[5];

    *(reg_buf + 4) = '\0';
    *(data_buf1 + 4) = '\0';

    if (settings != NULL) {
        pr_err("sr352_regs_from_sd_tunning start address %x start data %x\n",
            settings->reg_addr,settings->reg_data);
    }

    if (sr352_regs_table == NULL) {
        pr_err("sr352_regs_table is null\n");
        return -1;
    }
    pr_err("@@@ %s\n",name);
    start = strstr(sr352_regs_table, name);
    if (start == NULL){
        return -1;
    }
    end = strstr(start, "};");
    while (1) {
        /* Find Address */
        reg = strstr(start, "{0x");
        if ((reg == NULL) || (reg > end))
            break;
        /* Write Value to Address */
        if (reg != NULL) {
            memcpy(reg_buf, (reg + 1), 4);
            memcpy(data_buf1, (reg + 7), 4);

            if(kstrtoint(reg_buf, 16, &addr))
                pr_err("kstrtoint error .Please Align contents of the Header file!!\n") ;

            if(kstrtoint(data_buf1, 16, &value))
                pr_err("kstrtoint error .Please Align contents of the Header file!!\n");

            if (reg)
                start = (reg + 14);

            if (addr == 0xff){
                pr_err("delay = %dms STARTn", (int)value*10);
                msleep(value * 10);
                pr_err("delay END\n");

            }
            else {
                rc=s_ctrl->sensor_i2c_client->i2c_func_tbl->
                    i2c_write(s_ctrl->sensor_i2c_client, addr,
                            value,MSM_CAMERA_I2C_BYTE_DATA);
            }
        }
    }
    pr_err("sr352_regs_from_sd_tunning end!\n");
    return rc;
}

void sr352_regs_table_exit(void)
{
    pr_info("%s:%d\n", __func__, __LINE__);
    if (sr352_regs_table) {
        vfree(sr352_regs_table);
        sr352_regs_table = NULL;
    }

}


void sr352_regs_table_init(char *filename)
{
    struct file *filp;
    char *dp;
    long lsize;
    loff_t pos;
    int ret;

    /*Get the current address space */
    mm_segment_t fs = get_fs();
    pr_err("%s %d", __func__, __LINE__);
    /*Set the current segment to kernel data segment */
    set_fs(get_ds());

    filp = filp_open(filename, O_RDONLY, 0);

    if (IS_ERR_OR_NULL(filp)) {
        pr_err("file open error %ld",(long) filp);
        return;
    }
    lsize = filp->f_path.dentry->d_inode->i_size;
    pr_err("size : %ld", lsize);
    dp = vmalloc(lsize);
    if (dp == NULL) {
        pr_err("Out of Memory");
        filp_close(filp, current->files);
    }

    pos = 0;
    memset(dp, 0, lsize);
    ret = vfs_read(filp, (char __user *)dp, lsize, &pos);
    if (ret != lsize) {
        pr_err("Failed to read file ret = %d\n", ret);
        vfree(dp);
        filp_close(filp, current->files);
    }
    /*close the file*/
    filp_close(filp, current->files);

    /*restore the previous address space*/
    set_fs(fs);

    pr_err("coming to if part of string compare sr352_regs_table");
    sr352_regs_table = dp;
    sr352_regs_table_size = lsize;
    *((sr352_regs_table + sr352_regs_table_size) - 1) = '\0';

    return;
}

#endif
