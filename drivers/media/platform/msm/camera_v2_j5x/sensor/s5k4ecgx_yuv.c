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

//#define NO_BURST // Enable no burst mode for Tunning

#if 0
#define CONFIG_LOAD_FILE  // Enable it for Tunning Binary
#define NO_BURST // Enable no burst mode for Tunning
#endif

#include "s5k4ecgx.h"

#if defined (CONFIG_SEC_GTEL_PROJECT) || defined(CONFIG_SEC_GTES_PROJECT)
#include "s5k4ecgx_regs_gte.h"
#elif defined(CONFIG_SEC_J1X_PROJECT)
#include "s5k4ecgx_regs_j1x.h"
#else
#include "s5k4ecgx_regs.h"
#endif

#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_dt_util.h"

#ifdef CONFIG_LOAD_FILE
#define S5K4ECGX_WRITE_LIST(A) \
    s5k4ecgx_regs_from_sd_tunning(A,s_ctrl,#A);

#define S5K4ECGX_WRITE_LIST_BURST(A) \
    s5k4ecgx_regs_from_sd_tunning(A,s_ctrl,#A);

#elif defined(NO_BURST)

#define S5K4ECGX_WRITE_LIST(A) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl( \
            s_ctrl->sensor_i2c_client, A, ARRAY_SIZE(A), \
            MSM_CAMERA_I2C_WORD_DATA); CDBG("REGSEQ *** %s", #A)
#define S5K4ECGX_WRITE_LIST_BURST(A) S5K4ECGX_WRITE_LIST(A)
#else
#define S5K4ECGX_WRITE_LIST(A) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl( \
            s_ctrl->sensor_i2c_client, A, ARRAY_SIZE(A), \
            MSM_CAMERA_I2C_WORD_DATA); CDBG("REGSEQ *** %s", #A)
#define S5K4ECGX_WRITE_LIST_BURST(A) \
    s5k4ecgx_sensor_burst_write(s_ctrl, A, ARRAY_SIZE(A)); \
CDBG("REGSEQ *** BURST %s", #A)
#endif

#define S5K4ECGX_WRITE_ADDR(A,B) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write( \
            s_ctrl->sensor_i2c_client,A, B, MSM_CAMERA_I2C_WORD_DATA)

#define S5K4ECGX_READ_ADDR(A,B) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read( \
            s_ctrl->sensor_i2c_client,A, B, MSM_CAMERA_I2C_WORD_DATA)

#ifdef CONFIG_LOAD_FILE
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

static char *s5k4ecgx_regs_table;
static int s5k4ecgx_regs_table_size;
int s5k4ecgx_regs_from_sd_tunning(struct msm_camera_i2c_reg_conf *settings, struct msm_sensor_ctrl_t *s_ctrl,char * name);
void s5k4ecgx_regs_table_init(char *filename);
void s5k4ecgx_regs_table_exit(void);
#endif

static struct s5k4ecgx_ctrl s5k4ecgx_ctrl;

#if 1
extern uint16_t rear_vendor_id;
#endif

#if !defined(CONFIG_SEC_J1X_PROJECT)
#define set_led_flash(x) do{}while(0)
#endif

int s5k4ecgx_sensor_match_id(struct msm_sensor_ctrl_t *s_ctrl)
{
    int rc = 0;
    uint16_t chipid = 0;
    const char *sensor_name = NULL;
    struct msm_camera_slave_info *slave_info = NULL;
    struct msm_camera_i2c_client *sensor_i2c_client = NULL;
    enum msm_camera_i2c_data_type data_type = MSM_CAMERA_I2C_WORD_DATA;

    sensor_i2c_client = s_ctrl->sensor_i2c_client;
    slave_info = s_ctrl->sensordata->slave_info;
    sensor_name = s_ctrl->sensordata->sensor_name;

    if (!sensor_i2c_client || !slave_info || !sensor_name) {
        pr_err("%s:%d failed: %p %p %p\n",
                __func__, __LINE__, sensor_i2c_client, slave_info,
                sensor_name);
        return -EINVAL;
    }


    printk("%s:%d sensor[%s] sid = 0x%X sensorid = 0x%X DATA TYPE = %d\n E",
            __func__, __LINE__, sensor_name, sensor_i2c_client->cci_client->sid,
            slave_info->sensor_id, data_type);

    S5K4ECGX_WRITE_LIST(s5k4ecgx_vendor_id_read_prep);

    rear_vendor_id = 0;
    rc = sensor_i2c_client->i2c_func_tbl->i2c_read(
            sensor_i2c_client, 0xA006 /*slave_info->sensor_id_reg_addr*/,
            &chipid, data_type);

    if (chipid)
        rear_vendor_id = (chipid & 0xFF00);
    else
        goto out;

    chipid = 0;
    rc = sensor_i2c_client->i2c_func_tbl->i2c_read(
            sensor_i2c_client, 0xA007 /*slave_info->sensor_id_reg_addr*/,
            &chipid, data_type);

    if (chipid)
        rear_vendor_id |= ((chipid & 0xFF00) >> 8);

    if (rear_vendor_id != slave_info->sensor_id) {
        pr_err("%s:%d sensor_id doesnot match Read[0x%4x] Expected[0x%4x]\n",
               __func__, __LINE__, rear_vendor_id, slave_info->sensor_id);
    }

    return 0;
out:
    pr_err("%s:%d sensor_id read error.\n", __func__, __LINE__);
    rear_vendor_id = 0xFFFF;
    return 0;
}

#if 1
int32_t s5k4ecgx_sensor_burst_write(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_conf *reg_settings , int size)
{
    int i;
    int idx = 0;
    int burst_flag = 0;
    int err = -EINVAL;
    uint16_t  subaddr;
    uint16_t  value;
    static uint8_t burst_data[BURST_MODE_BUFFER_MAX_SIZE * 2] = {0};

    // burst write related variables
    struct msm_camera_i2c_burst_reg_array burst_reg_setting = { 0 ,};
    struct msm_camera_i2c_reg_setting conf_array = {
        .reg_setting = (void * ) &burst_reg_setting,
        .size = 1,
        .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
        .data_type = MSM_CAMERA_I2C_BURST_DATA,
    };

    CDBG("E");

    // read the register settings array
    for (i = 0; i < size; ++i) {
        if (idx  >= BURST_MODE_BUFFER_MAX_SIZE) {
            burst_flag = 0;

            burst_reg_setting.reg_burst_data = burst_data;
            burst_reg_setting.reg_data_size = idx * 2;
            idx = 0;

            err = s_ctrl->sensor_i2c_client-> \
                  i2c_func_tbl->i2c_write_burst_table( \
                          s_ctrl->sensor_i2c_client, &conf_array);
            if (err < 0) {
                pr_err("[%s:%d]Burst write fail!\n", __func__, __LINE__);
                goto on_error;
            }
        }

        // the register address
        subaddr = reg_settings[i].reg_addr;

        // the register value
        value = reg_settings[i].reg_data;

        if (burst_flag == 0) {
            if ( (subaddr == BURST_REG) && \
                    (reg_settings[i+1].reg_addr == BURST_REG)) {
                //burst mode start
                burst_flag = 1;
                burst_reg_setting.reg_addr = subaddr;
                /* MSB has to go 1st for I2C write */
                burst_data[idx * 2] = (value & 0xFF00) >> 8; /* MSB */
                burst_data[idx * 2 + 1] = value & 0x00FF; /* LSB */
                idx++;

                continue;
            }

            // write the register setting
            err = s_ctrl->sensor_i2c_client-> \
                  i2c_func_tbl->i2c_write( \
                          s_ctrl->sensor_i2c_client, subaddr, \
                          value, MSM_CAMERA_I2C_WORD_DATA);

        } else if (burst_flag == 1) {
            burst_data[idx * 2] = (value & 0xFF00) >> 8; /* MSB */
            burst_data[idx * 2 + 1] = value & 0x00FF; /* LSB */
            idx++;

            if ((subaddr == BURST_REG) && (i + 1 >= size || \
                        (reg_settings[i+1].reg_addr != BURST_REG))) {
                // burst mode end.
                burst_flag = 0;


                burst_reg_setting.reg_burst_data = burst_data;
                burst_reg_setting.reg_data_size = idx * 2;

                idx = 0;

                err = s_ctrl->sensor_i2c_client-> \
                      i2c_func_tbl->i2c_write_burst_table( \
                              s_ctrl->sensor_i2c_client, &conf_array);
                if (err < 0) {
                    pr_err("[%s:%d]Burst write fail!\n", \
                            __func__, __LINE__);
                    goto on_error;
                }
            }
        }
    }

on_error:
    if (unlikely(err < 0)) {
        pr_err("[%s:%d] register set failed\n", __func__, __LINE__);
    }
    CDBG("X");
    return err;
}
#endif

int32_t s5k4ecgx_set_ae_awb(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    if (mode) {
        if (s5k4ecgx_ctrl.settings.ae_awb_lock == 0) {
            CDBG("AWB_AE_LOCK");
            if (s5k4ecgx_ctrl.settings.wb == CAMERA_WHITE_BALANCE_AUTO \
                    || s5k4ecgx_ctrl.settings.wb == CAMERA_WHITE_BALANCE_OFF) {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_ae_lock);
                S5K4ECGX_WRITE_LIST(s5k4ecgx_awb_lock);
            } else {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_ae_lock);
            }
            s5k4ecgx_ctrl.settings.ae_awb_lock = 1;
        }
    } else {
        if (s5k4ecgx_ctrl.settings.ae_awb_lock == 1) {
            CDBG("AWB_AE_UNLOCK");
            if (s5k4ecgx_ctrl.settings.wb == CAMERA_WHITE_BALANCE_AUTO \
                    || s5k4ecgx_ctrl.settings.wb == CAMERA_WHITE_BALANCE_OFF) {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_ae_unlock);
                S5K4ECGX_WRITE_LIST(s5k4ecgx_awb_unlock);
            } else {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_ae_unlock);
            }
            s5k4ecgx_ctrl.settings.ae_awb_lock = 0;
        }
    }

    return 0;
}


int32_t s5k4ecgx_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- EV is %d", mode);

    switch (mode) {
        case CAMERA_EV_M4:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Minus_4);
            break;

        case CAMERA_EV_M3:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Minus_3);
            break;

        case CAMERA_EV_M2:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Minus_2);
            break;

        case CAMERA_EV_M1:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Minus_1);
            break;

        case CAMERA_EV_DEFAULT:
            CDBG("CAMERA_EV_DEFAULT --> no operation\n");
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Default);
            break;

        case CAMERA_EV_P1:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Plus_1);
            break;

        case CAMERA_EV_P2:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Plus_2);
            break;

        case CAMERA_EV_P3:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Plus_3);
            break;

        case CAMERA_EV_P4:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_EV_Plus_4);
            break;
        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }

    return rc;
}


void s5k4ecgx_set_exposure_camcorder(struct msm_sensor_ctrl_t *s_ctrl,unsigned int mode)
{
    
    CDBG("CAMSET CAMCORDER -- EV is %d", mode);

    switch (mode) {
        case CAMERA_EV_M4:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Minus_4);
            break;

        case CAMERA_EV_M3:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Minus_3);
            break;

        case CAMERA_EV_M2:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Minus_2);
            break;

        case CAMERA_EV_M1:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Minus_1);
            break;

        case CAMERA_EV_DEFAULT:
            CDBG("CAMERA_EV_DEFAULT --> no operation\n");
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Default);
            break;

        case CAMERA_EV_P1:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Plus_1);
            break;

        case CAMERA_EV_P2:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Plus_2);
            break;

        case CAMERA_EV_P3:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Plus_3);
            break;

        case CAMERA_EV_P4:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_camcorder_EV_Plus_4);
            break;
        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            break;
    }
    
}

int32_t s5k4ecgx_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- effect is %d", mode);

    switch (mode) {
        case CAMERA_EFFECT_OFF:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Effect_Normal);
            break;

        case CAMERA_EFFECT_MONO:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Effect_Mono);
            break;

        case CAMERA_EFFECT_NEGATIVE:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Effect_Negative);
            break;

        case CAMERA_EFFECT_SEPIA:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Effect_Sepia);
            break;

        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }

    return rc;
}

int32_t s5k4ecgx_set_scene_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- scene is %d", mode);
    s5k4ecgx_set_ae_awb(s_ctrl, 0);
    S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Default);

    switch (mode) {
        case CAMERA_SCENE_AUTO:
            break;

        case CAMERA_SCENE_LANDSCAPE:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Landscape);
            break;

        case CAMERA_SCENE_DAWN:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Duskdawn);
            break;

        case CAMERA_SCENE_BEACH:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Beach_Snow);
            break;

        case CAMERA_SCENE_SUNSET:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Sunset);
            break;

        case CAMERA_SCENE_NIGHT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Nightshot);
            break;

        case CAMERA_SCENE_PORTRAIT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Portrait);
            break;

        case CAMERA_SCENE_AGAINST_LIGHT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Backlight);
            break;

        case CAMERA_SCENE_SPORT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Sports);
            break;

        case CAMERA_SCENE_FALL:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Fall_Color);
            break;

        case CAMERA_SCENE_TEXT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Text);
            /*S5K4ECGX_WRITE_LIST(s5k4ecgx_AF_Macro_mode_1);
              msleep(100);
              S5K4ECGX_WRITE_LIST(s5k4ecgx_AF_Macro_mode_2);
              msleep(100);
              S5K4ECGX_WRITE_LIST(s5k4ecgx_AF_Macro_mode_3);
              msleep(100);*/
            break;

        case CAMERA_SCENE_CANDLE:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Candle_Light);
            break;

        case CAMERA_SCENE_FIRE:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Fireworks);
            break;

        case CAMERA_SCENE_PARTY:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Scene_Party_Indoor);
            break;

        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }

    return rc;
}

int32_t s5k4ecgx_set_white_balance(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    uint16_t value = 0;
    CDBG("CAMSET -- WB is %d", mode);

    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x04E6);
    S5K4ECGX_READ_ADDR(0x0F12, &value);

    S5K4ECGX_WRITE_ADDR(0x0028, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002A, 0x04E6);

    switch (mode) {
        case CAMERA_WHITE_BALANCE_OFF:
        case CAMERA_WHITE_BALANCE_AUTO:

            value |= (0x1 << 3);
            S5K4ECGX_WRITE_ADDR(0x0F12, value);

            S5K4ECGX_WRITE_LIST(s5k4ecgx_WB_Auto);
            break;

        case CAMERA_WHITE_BALANCE_INCANDESCENT:
            value &= (~(0x1 << 3));
            S5K4ECGX_WRITE_ADDR(0x0F12, value);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_WB_Tungsten);
            break;

        case CAMERA_WHITE_BALANCE_FLUORESCENT:
            value &= (~(0x1 << 3));
            S5K4ECGX_WRITE_ADDR(0x0F12, value);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_WB_Fluorescent);
            break;

        case CAMERA_WHITE_BALANCE_DAYLIGHT:
            value &= (~(0x1 << 3));
            S5K4ECGX_WRITE_ADDR(0x0F12, value);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_WB_Sunny);
            break;

        case CAMERA_WHITE_BALANCE_CLOUDY_DAYLIGHT:
            value &= (~(0x1 << 3));
            S5K4ECGX_WRITE_ADDR(0x0F12, value);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_WB_Cloudy);
            break;
        default:
            CDBG("%s: Setting %d is invalid", __func__, mode);
            rc = 0;
    }

    return rc;
}

int32_t s5k4ecgx_set_metering(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- metering is %d", mode);

    switch (mode) {
        case CAMERA_CENTER_WEIGHT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Metering_Center);
            break;

        case CAMERA_AVERAGE:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Metering_Matrix);
            break;

        case CAMERA_SPOT:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Metering_Spot);
            break;
        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }

    return rc;
}

int32_t s5k4ecgx_set_iso(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- iso is %d", mode);

    switch (mode) {
        case CAMERA_ISO_MODE_AUTO:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_ISO_Auto);
            break;
        case CAMERA_ISO_MODE_50:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_ISO_50);
            break;
        case CAMERA_ISO_MODE_100:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_ISO_100);
            break;
        case CAMERA_ISO_MODE_200:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_ISO_200);
            break;
        case CAMERA_ISO_MODE_400:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_ISO_400);
            break;
        default:
            CDBG("%s: Setting %d is invalid\n", __func__, mode);
            rc = 0;
    }

    return rc;
}

int32_t s5k4ecgx_get_light_level(struct msm_sensor_ctrl_t *s_ctrl)
{
    unsigned short msb = 0;
    unsigned short lsb = 0;
    unsigned short cur_lux = 0;
    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x2C18);
    S5K4ECGX_READ_ADDR(0x0F12, &lsb);
    S5K4ECGX_READ_ADDR( 0x0F12,&msb);
    cur_lux = (msb << 16) | lsb;
    return cur_lux;
}

int32_t s5k4ecgx_get_flash_status(void)
{
    int flash_status = 0;
    if ( ((s5k4ecgx_ctrl.settings.flash_mode == CAMERA_FLASH_AUTO) && (s5k4ecgx_ctrl.settings.lowLight) )
            || (s5k4ecgx_ctrl.settings.flash_mode == CAMERA_FLASH_ON)){
        flash_status = 1;
    }
    CDBG("flash_status = %d", flash_status);
    return flash_status;
}

void s5k4ecgx_check_ae_stable(struct msm_sensor_ctrl_t *s_ctrl)
{
    unsigned short ae_stable = 0;
    int ae_count = 0;

    while (ae_count++ < 7) {
        S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
        S5K4ECGX_WRITE_ADDR(0x002E, 0x2C74);
        S5K4ECGX_READ_ADDR(0x0F12, &ae_stable);
        if (ae_stable == 0x01) {
            CDBG("ae_stabel count[%d]\n", ae_count);
            break;
        }
        msleep(30);
    }
    if (ae_count > 7) {
        CDBG("AE NOT STABLE");
    }
}

int32_t s5k4ecgx_set_af_mode(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
    int32_t rc = 0;
    CDBG("CAMSET -- focus mode is %d", mode);

    switch (mode) {
        case CAMERA_AF_OCR:
        case CAMERA_AF_MACRO:
            S5K4ECGX_WRITE_LIST(s5k4ecgx_macro_af);
            break;
        case CAMERA_AF_AUTO:
        default:
            CDBG("default mode is auto\n");
            S5K4ECGX_WRITE_LIST(s5k4ecgx_normal_af);
    }
    return rc;
}

int32_t s5k4ecgx_set_af_status(struct msm_sensor_ctrl_t *s_ctrl, int status, int initial_pos)
{
    int32_t rc = SENSOR_AF_CANCEL;
    CDBG("CAMSET -- set_af_status %d, init_pos =%d", status, initial_pos);
    if (status == SENSOR_AF_START) {
        unsigned int cur_lux = 0;
        CDBG("S5K4ECGX_AF_START");
        cur_lux = s5k4ecgx_get_light_level(s_ctrl);
        CDBG("AF light level = %d", cur_lux);
        if (cur_lux <= LOW_LIGHT_LEVEL) {
            CDBG("LOW LUX AF ");
            s5k4ecgx_ctrl.settings.lowLight = 1;
        } else {
            CDBG("NORMAL LUX AF ");
            s5k4ecgx_ctrl.settings.lowLight = 0;
        }
        if (s5k4ecgx_get_flash_status()) {
            S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_On);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_On);
            s5k4ecgx_ctrl.settings.is_preflash = 1;
            CDBG("Turn on the pre-flash\n");
            set_led_flash(MSM_CAMERA_LED_LOW);
            rc = SENSOR_AF_PRE_FLASH_ON;
        } else {
            s5k4ecgx_set_ae_awb(s_ctrl, 1);
            S5K4ECGX_WRITE_LIST(s5k4ecgx_af);
            rc = SENSOR_AF_START;
        }

    } else if ( (status == SENSOR_AF_PRE_FLASH_OFF) ) {
        if( s5k4ecgx_ctrl.settings.is_preflash == 1 ) {
            CDBG("Turn off the pre-flash\n");
            S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_Off);
            s5k4ecgx_set_ae_awb(s_ctrl, 0);

            S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_Off);
            set_led_flash(MSM_CAMERA_LED_OFF);
            s5k4ecgx_ctrl.settings.is_preflash = 0;
        }
        else {
            if (s5k4ecgx_ctrl.settings.is_touchaf == 1) {
                CDBG("Check & unlock AE/AWB in Autofocus Finish\n");
#if defined(CONFIG_SEC_J1X_PROJECT)
            	/* This is end. Reset Touch AF */
            	s5k4ecgx_ctrl.settings.is_touchaf = 0;

                if (s5k4ecgx_ctrl.settings.ae_awb_lock == 1)
#endif
		{
                    s5k4ecgx_set_ae_awb(s_ctrl, 0);
		}
            }
        }

        rc = SENSOR_AF_PRE_FLASH_OFF;
    } else if (status == SENSOR_AF_PRE_FLASH_AE_STABLE &&
            s5k4ecgx_ctrl.settings.is_preflash == 1) {

        s5k4ecgx_check_ae_stable(s_ctrl);
        s5k4ecgx_set_ae_awb(s_ctrl, 1);
        S5K4ECGX_WRITE_LIST(s5k4ecgx_af);
        rc = SENSOR_AF_START;

    } else {

        CDBG("S5K4ECGX_AF_ABORT\n");
        s5k4ecgx_set_af_mode(s_ctrl, s5k4ecgx_ctrl.settings.focus_mode);
        s5k4ecgx_set_ae_awb(s_ctrl, 0);
        if( s5k4ecgx_ctrl.settings.is_preflash == 1 ) {
            CDBG("Turn off the pre-flash\n");
            S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_Off);

            S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_Off);
            set_led_flash(MSM_CAMERA_LED_OFF);
            s5k4ecgx_ctrl.settings.is_preflash = 0;
        }
        rc = SENSOR_AF_CANCEL;
    }

    return rc;
}

int32_t s5k4ecgx_get_af_status(struct msm_sensor_ctrl_t *s_ctrl, int is_search_status)
{
    unsigned short af_status = 0;
    CDBG("is_search_status %d", is_search_status);
    switch (is_search_status) {
        case 0:
            S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
            S5K4ECGX_WRITE_ADDR(0x002E, 0x2EEE);
            S5K4ECGX_READ_ADDR(0x0F12, &af_status);
            CDBG("1st AF status : %x\n", af_status);
            break;
        case 1:
            S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
            S5K4ECGX_WRITE_ADDR(0x002E, 0x2207);
            S5K4ECGX_READ_ADDR(0x0F12, &af_status);
            CDBG("2nd AF status : %d", af_status);
            break;
        default:
            CDBG("unexpected mode is coming from HAL");
            break;
    }
    CDBG("return_af_status = %d", af_status);
    return  af_status;
}

int32_t s5k4ecgx_touchaf_set_resolution(struct msm_sensor_ctrl_t *s_ctrl,
        unsigned int addr, unsigned int value)
{
    S5K4ECGX_WRITE_ADDR(0xFCFC, 0xD000);
    S5K4ECGX_WRITE_ADDR(0x0028, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002A, addr);
    S5K4ECGX_WRITE_ADDR(0x0F12, value);

    return 0;
}

int32_t s5k4ecgx_set_touchaf_pos(struct msm_sensor_ctrl_t *s_ctrl,
        struct ioctl_native_cmd *info)
{
    static unsigned short inWindowWidth = 288;
    static unsigned short inWindowHeight = 216;
    static unsigned short outWindowWidth = 640;
    static unsigned short outWindowHeight = 400;
    int x = info->value_1;
    int y = info->value_2;

    unsigned short previewWidth = info->address;
    unsigned short previewHeight = info->value_3;

    CDBG("s5k4ecgx_set_touchaf_pos (%d, %d) previewRes(%ux%u)",x, y, previewWidth, previewHeight);

    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x02A0);
    S5K4ECGX_READ_ADDR(0x0F12, &inWindowWidth);
    S5K4ECGX_READ_ADDR(0x0F12, &inWindowHeight);
    CDBG("[i2c read] inWindowWidth = %d, inWindowHeight = %d\n", inWindowWidth, inWindowHeight);

    inWindowWidth = inWindowWidth * previewWidth / 1024;
    inWindowHeight = inWindowHeight * previewHeight / 1024;

    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x0298);
    S5K4ECGX_READ_ADDR(0x0F12, &outWindowWidth);
    S5K4ECGX_READ_ADDR(0x0F12, &outWindowHeight);
    CDBG("[i2c read] outWindowWidth = %d, outWindowHeight = %d\n", outWindowWidth, outWindowHeight);


    outWindowWidth = outWindowWidth * previewWidth / 1024;
    outWindowHeight = outWindowHeight * previewHeight / 1024;

    if (x < inWindowWidth/2)
        x = inWindowWidth/2+1;
    else if (x > previewWidth - inWindowWidth/2)
        x = previewWidth - inWindowWidth/2 -1;
    if (y < inWindowHeight/2)
        y = inWindowHeight/2+1;
    else if (y > previewHeight - inWindowHeight/2)
        y = previewHeight - inWindowHeight/2 -1;

    s5k4ecgx_touchaf_set_resolution(s_ctrl, 0x029C, (x - inWindowWidth/2) * 1024 / previewWidth);
    s5k4ecgx_touchaf_set_resolution(s_ctrl, 0x029E, (y - inWindowHeight/2) * 1024 / previewHeight);

    if (x < outWindowWidth/2)
        x = outWindowWidth/2+1;
    else if (x > previewWidth - outWindowWidth/2)
        x = previewWidth - outWindowWidth/2 -1;
    if (y < outWindowHeight/2)
        y = outWindowHeight/2+1;
    else if (y > previewHeight - outWindowHeight/2)
        y = previewHeight - outWindowHeight/2 -1;

    s5k4ecgx_touchaf_set_resolution(s_ctrl, 0x0294, (x - outWindowWidth/2) * 1024 / previewWidth);
    s5k4ecgx_touchaf_set_resolution(s_ctrl, 0x0296, (y - outWindowHeight/2) * 1024 / previewHeight);

    s5k4ecgx_touchaf_set_resolution(s_ctrl, 0x02A4, 0x0001);

    return 0;
}

int32_t s5k4ecgx_wait_preview_stable(struct msm_sensor_ctrl_t *s_ctrl)
{
    int try = 0;
    unsigned short is_captured = 0;

    do {
        if (try)
            mdelay(10);

        S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
        //S5K4ECGX_WRITE_ADDR(0x002E, 0x215F);
        S5K4ECGX_WRITE_ADDR(0x002E, 0x215E);
        S5K4ECGX_READ_ADDR(0x0F12, &is_captured);
        try++;
    } while(is_captured == 0x0100 && try <= 50);


    if (is_captured == 0x0100)
        pr_err("%s:%d maximum tried!\n", __func__, __LINE__);

    CDBG("try[%d]   is_captured(0x%x)\n", try, is_captured);

    return 0;
}

int32_t s5k4ecgx_wait_capture_stable(struct msm_sensor_ctrl_t *s_ctrl)
{
    int try = 0;
    unsigned short is_captured = 0;

    do {
        if (try)
            mdelay(10);

        S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
        //S5K4ECGX_WRITE_ADDR(0x002E, 0x215F);
        S5K4ECGX_WRITE_ADDR(0x002E, 0x215E);
        S5K4ECGX_READ_ADDR(0x0F12, &is_captured);
        try++;
    } while(is_captured != 0x0100 && try <= 50);


    if (is_captured != 0x0100)
        pr_err("%s:%d maximum tried!\n", __func__, __LINE__);
    CDBG("try[%d]\n", try);

    return 0;
}

int32_t s5k4ecgx_set_exif(struct msm_sensor_ctrl_t *s_ctrl )
{
    unsigned short msb = 0;
    unsigned short lsb = 0;
    unsigned short analog_gain;
    unsigned short digital_gain;
    unsigned int iso_value;
    unsigned int temp;

    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x2BC0);
    S5K4ECGX_READ_ADDR(0x0F12, &lsb);
    S5K4ECGX_READ_ADDR(0x0F12, &msb);

    temp = ((msb << 16) + lsb);
    if ( temp != 0)
        s5k4ecgx_ctrl.exif_shutterspeed = 400000 / temp;
    else
        s5k4ecgx_ctrl.exif_shutterspeed = 0;

    S5K4ECGX_WRITE_ADDR(0x002C, 0x7000);
    S5K4ECGX_WRITE_ADDR(0x002E, 0x2BC4);
    S5K4ECGX_READ_ADDR(0x0F12, &analog_gain);
    S5K4ECGX_READ_ADDR(0x0F12, &digital_gain);

    iso_value =(unsigned int) ( analog_gain * digital_gain ) / 256 / 2;
    if (iso_value < 0xD0)
        s5k4ecgx_ctrl.exif_iso = 50;
    else if(iso_value < 0x1A0)
        s5k4ecgx_ctrl.exif_iso = 100;
    else if(iso_value < 0x374)
        s5k4ecgx_ctrl.exif_iso = 200;
    else
        s5k4ecgx_ctrl.exif_iso = 400;

    CDBG("shutter_speed = %d, ISO = %d, iso_value = 0x%x",
            s5k4ecgx_ctrl.exif_shutterspeed, s5k4ecgx_ctrl.exif_iso, iso_value);
    return 0;
}


int32_t s5k4ecgx_get_exif(struct ioctl_native_cmd * exif_info)
{
    exif_info->value_1 = 1;	// equals 1 to update the exif value in the user level.
    exif_info->value_2 = s5k4ecgx_ctrl.exif_iso;
    exif_info->value_3 = s5k4ecgx_ctrl.exif_shutterspeed;
    return 0;
}

int32_t s5k4ecgx_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
        void __user *argp)
{
    struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
    int32_t rc = 0;
    int32_t i = 0;
    mutex_lock(s_ctrl->msm_sensor_mutex);

    switch (cdata->cfgtype) {
        case CFG_GET_SENSOR_INFO:
            CDBG(" CFG_GET_SENSOR_INFO \n");
            memcpy(cdata->cfg.sensor_info.sensor_name,
                    s_ctrl->sensordata->sensor_name,
                    sizeof(cdata->cfg.sensor_info.sensor_name));
            cdata->cfg.sensor_info.session_id =
                s_ctrl->sensordata->sensor_info->session_id;
            for (i = 0; i < SUB_MODULE_MAX; i++)
                cdata->cfg.sensor_info.subdev_id[i] =
                    s_ctrl->sensordata->sensor_info->subdev_id[i];

            cdata->cfg.sensor_info.is_mount_angle_valid =
                s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
            cdata->cfg.sensor_info.sensor_mount_angle =
                s_ctrl->sensordata->sensor_info->sensor_mount_angle;
            cdata->cfg.sensor_info.position =
                s_ctrl->sensordata->sensor_info->position;
            cdata->cfg.sensor_info.modes_supported =
                s_ctrl->sensordata->sensor_info->modes_supported;

            CDBG("%s:%d sensor name %s\n", __func__, __LINE__,
                    cdata->cfg.sensor_info.sensor_name);
            CDBG("%s:%d session id %d\n", __func__, __LINE__,
                    cdata->cfg.sensor_info.session_id);
            for (i = 0; i < SUB_MODULE_MAX; i++)
                CDBG("%s:%d subdev_id[%d] %d\n", __func__, __LINE__, i,
                        cdata->cfg.sensor_info.subdev_id[i]);

            break;
        case CFG_SET_INIT_SETTING:
            CDBG("CFG_SET_INIT_SETTING\n");
#ifdef CONFIG_LOAD_FILE
            s5k4ecgx_regs_table_init("/data/s5k4ecgx_regs.h");
            pr_err("/data/s5k4ecgx_yuv.h inside CFG_SET_INIT_SETTING\n");
#endif
            rc = S5K4ECGX_WRITE_LIST_BURST(s5k4ecgx_init_regs);
            break;
        case CFG_SET_RESOLUTION:
            s5k4ecgx_ctrl.settings.resolution = *((int32_t  *)cdata->cfg.setting);
            CDBG("CFG_SET_RESOLUTION  res = %d\n " , s5k4ecgx_ctrl.settings.resolution);
            if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_CAPTURE ){
                if (s5k4ecgx_get_flash_status()) {
                    // Allow actuator and pre-flash to settle down
                    msleep(500);
                    set_led_flash(MSM_CAMERA_LED_HIGH);
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_Main_Flash_On);
                } else if ((s5k4ecgx_ctrl.settings.flash_mode == CAMERA_FLASH_AUTO)
                        && (!s5k4ecgx_ctrl.settings.lowLight)) {
                    unsigned short cur_lux;
                    cur_lux = s5k4ecgx_get_light_level(s_ctrl);
                    CDBG("light level = %d\n", cur_lux);
                    if (cur_lux <= LOW_LIGHT_LEVEL) {
                        CDBG("LOW LUX\n");
                        s5k4ecgx_ctrl.settings.lowLight = 1;
                        set_led_flash(MSM_CAMERA_LED_HIGH);
                        S5K4ECGX_WRITE_LIST(s5k4ecgx_Main_Flash_On);
                    }
                }
                S5K4ECGX_WRITE_LIST(s5k4ecgx_snapshot_regs);
                s5k4ecgx_wait_capture_stable(s_ctrl);
                s5k4ecgx_set_exif(s_ctrl);
            } else if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_PREVIEW) {
                if (s5k4ecgx_ctrl.prev_mode == CAMERA_MODE_CAPTURE){
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_preview_regs);
                }
                s5k4ecgx_wait_preview_stable(s_ctrl);
            }

            break;
        case CFG_SET_STOP_STREAM:
            CDBG("CFG_SET_STOP_STREAM\n");
            if(s5k4ecgx_get_flash_status() ){
                if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_CAPTURE ){
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_Main_Flash_Off);
                    set_led_flash(MSM_CAMERA_LED_OFF);
                    s5k4ecgx_ctrl.settings.lowLight = 0;
                } else if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_RECORDING) {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_Off);
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_Off);
                    set_led_flash(MSM_CAMERA_LED_OFF);
                    s5k4ecgx_ctrl.settings.lowLight = 0;
                }
            }

            if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_PREVIEW) {
                if (s5k4ecgx_ctrl.settings.is_preflash == 1) {
                    CDBG("Turn off the pre-flash\n");
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_Off);
                    if(s5k4ecgx_ctrl.settings.scenemode == CAMERA_SCENE_AUTO){
                        s5k4ecgx_set_ae_awb(s_ctrl, 0);
                    }
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_Off);
                    set_led_flash(MSM_CAMERA_LED_OFF);
                    s5k4ecgx_ctrl.settings.is_preflash = 0;
                }
            }

            if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_RECORDING) {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_preview_1280_960);
                    msleep(50);
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_stop_stream);
            }

            break;
        case CFG_SET_START_STREAM:
            CDBG("CFG_SET_START_STREAM");
            if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_RECORDING) {
                if (s5k4ecgx_get_flash_status()) {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_FAST_AE_On);
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_Pre_Flash_On);
                    set_led_flash(MSM_CAMERA_LED_LOW);
                    msleep(380);
                    s5k4ecgx_check_ae_stable(s_ctrl);
                }
                switch (s5k4ecgx_ctrl.fixed_fps_val)
                {
                    case 15000:
                        S5K4ECGX_WRITE_LIST(s5k4ecgx_fps_15);
                        break;
                    case 30000:
                        S5K4ECGX_WRITE_LIST(s5k4ecgx_fps_30);
                        break;
                    default:
                        S5K4ECGX_WRITE_LIST(s5k4ecgx_fps_auto);
                }
                S5K4ECGX_WRITE_LIST_BURST(s5k4ecgx_camcorder);
                s5k4ecgx_set_exposure_camcorder(s_ctrl,s5k4ecgx_ctrl.settings.exposure);

                if (s5k4ecgx_ctrl.settings.resolution == MSM_SENSOR_RES_5) {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_preview_640_480);
                } else {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_preview_1280_720);
                }

            } else if (s5k4ecgx_ctrl.op_mode == CAMERA_MODE_PREVIEW) {
                CDBG("prev_mode = %d\n", s5k4ecgx_ctrl.prev_mode);
                s5k4ecgx_set_ae_awb(s_ctrl, 0);
                if ((s5k4ecgx_ctrl.prev_mode == CAMERA_MODE_RECORDING)) {
                    S5K4ECGX_WRITE_LIST(s5k4ecgx_fps_auto);
                    S5K4ECGX_WRITE_LIST_BURST(s5k4ecgx_camcorder_disable);
                    s5k4ecgx_set_exposure_compensation(s_ctrl,
                            s5k4ecgx_ctrl.settings.exposure);
                }

                if (s5k4ecgx_ctrl.streamon == 0) {
                    s5k4ecgx_set_exposure_compensation(s_ctrl,
                            s5k4ecgx_ctrl.settings.exposure);

                    s5k4ecgx_set_white_balance(s_ctrl,
                            s5k4ecgx_ctrl.settings.wb);

                    s5k4ecgx_set_iso(s_ctrl, s5k4ecgx_ctrl.settings.iso);

            if(cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_anti_banding_50hz_auto);
            } else if(cdata->flicker_type == MSM_CAM_FLICKER_60HZ) {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_anti_banding_60hz_auto);
            }
                }

                s5k4ecgx_ctrl.streamon = 1;
            }

            break;
        case CFG_SET_SLAVE_INFO: {
                                     struct msm_camera_sensor_slave_info sensor_slave_info;
                                     struct msm_camera_power_ctrl_t *p_ctrl;
                                     uint16_t size;
                                     int slave_index = 0;
                                     CDBG("CFG_SET_SLAVE_INFO  \n");
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
                                     CDBG("%s sensor id %x\n", __func__,
                                             sensor_slave_info.slave_addr);
                                     CDBG("%s sensor addr type %d\n", __func__,
                                             sensor_slave_info.addr_type);
                                     CDBG("%s sensor reg %x\n", __func__,
                                             sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
                                     CDBG("%s sensor id %x\n", __func__,
                                             sensor_slave_info.sensor_id_info.sensor_id);
                                     for (slave_index = 0; slave_index <
                                             p_ctrl->power_setting_size; slave_index++) {
                                         CDBG("%s i %d power setting %d %d %ld %d\n", __func__,
                                                 slave_index,
                                                 p_ctrl->power_setting[slave_index].seq_type,
                                                 p_ctrl->power_setting[slave_index].seq_val,
                                                 p_ctrl->power_setting[slave_index].config_val,
                                                 p_ctrl->power_setting[slave_index].delay);
                                     }
                                     CDBG("CFG_SET_SLAVE_INFO EXIT \n");
                                     break;
                                 }
        case CFG_WRITE_I2C_ARRAY: {
                                      struct msm_camera_i2c_reg_setting conf_array;
                                      struct msm_camera_i2c_reg_array *reg_setting = NULL;

                                      CDBG(" CFG_WRITE_I2C_ARRAY  \n");

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
                                      break;
                                  }
        case CFG_WRITE_I2C_SEQ_ARRAY: {
                                          struct msm_camera_i2c_seq_reg_setting conf_array;
                                          struct msm_camera_i2c_seq_reg_array *reg_setting = NULL;

                                          CDBG("CFG_WRITE_I2C_SEQ_ARRAY  \n");

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
                                              i2c_write_seq_table(s_ctrl->sensor_i2c_client,
                                                      &conf_array);
                                          kfree(reg_setting);
                                          break;
                                      }

        case CFG_POWER_UP:
                                      CDBG(" CFG_POWER_UP  \n");
                                      s5k4ecgx_ctrl.streamon = 0;
                                      s5k4ecgx_ctrl.op_mode = CAMERA_MODE_INIT;
                                      s5k4ecgx_ctrl.prev_mode = CAMERA_MODE_INIT;
                                      s5k4ecgx_ctrl.settings.metering = CAMERA_CENTER_WEIGHT;
                                      s5k4ecgx_ctrl.settings.exposure = CAMERA_EV_DEFAULT;
                                      s5k4ecgx_ctrl.settings.wb = CAMERA_WHITE_BALANCE_AUTO;
                                      s5k4ecgx_ctrl.settings.iso = CAMERA_ISO_MODE_AUTO;
                                      s5k4ecgx_ctrl.settings.effect = CAMERA_EFFECT_OFF;
                                      s5k4ecgx_ctrl.settings.scenemode = CAMERA_SCENE_AUTO;
                                      s5k4ecgx_ctrl.settings.flash_mode = CAMERA_FLASH_OFF;
                                      s5k4ecgx_ctrl.settings.lowLight = 0;
                                      s5k4ecgx_ctrl.settings.is_touchaf = 0;
                                      s5k4ecgx_ctrl.settings.is_preflash = 0;
                                      if (s_ctrl->func_tbl->sensor_power_up) {
                                          CDBG("CFG_POWER_UP");
                                          rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
                                      } else
                                          rc = -EFAULT;
                                      break;

        case CFG_POWER_DOWN:
                                      CDBG("CFG_POWER_DOWN  \n");


                                      S5K4ECGX_WRITE_LIST(s5k4ecgx_preview_regs);
                                      msleep(100);
                                      S5K4ECGX_WRITE_LIST(s5k4ecgx_focus_mode_auto);

                                      /* Actuator softlanding */
                                      if (s5k4ecgx_ctrl.settings.focus_mode == CAMERA_AF_MACRO)
                                      {
                                          /* 250ms sleep is not sufficient for AF to settle */
                                          msleep(400);
                                      }
                                      else
                                      {
                                          msleep(100);
                                      }

                                      /* Required for 3rd Party Tourch Light Mode */
                                      set_led_flash(MSM_CAMERA_LED_OFF);

                                      if (s_ctrl->func_tbl->sensor_power_down) {
                                          CDBG("CFG_POWER_DOWN");
                                          rc = s_ctrl->func_tbl->sensor_power_down(s_ctrl);
                                      } else
                                          rc = -EFAULT;

#ifdef CONFIG_LOAD_FILE
                                      s5k4ecgx_regs_table_exit();
#endif
                                      break;

        case CFG_SET_STOP_STREAM_SETTING: {
                                              struct msm_camera_i2c_reg_setting *stop_setting =
                                                  &s_ctrl->stop_setting;
                                              struct msm_camera_i2c_reg_array *reg_setting = NULL;

                                              CDBG("CFG_SET_STOP_STREAM_SETTING  \n");

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
                                              break;
                                          }
        default:
                                          rc = -EFAULT;
                                          break;
    }

    mutex_unlock(s_ctrl->msm_sensor_mutex);

    return rc;
}




int32_t s5k4ecgx_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
        void __user *argp)
{
    int32_t rc = 0;
    struct ioctl_native_cmd *cam_info = (struct ioctl_native_cmd *)argp;

    mutex_lock(s_ctrl->msm_sensor_mutex);

    /*pr_err("cam_info values = %d : %d : %d : %d : %d\n", cam_info->mode, cam_info->address, cam_info->value_1, cam_info->value_2 , cam_info->value_3);*/
    switch (cam_info->mode) {
        case EXT_CAM_EV:
            s5k4ecgx_ctrl.settings.exposure = (cam_info->value_1);
            if (s5k4ecgx_ctrl.streamon == 1)
                s5k4ecgx_set_exposure_compensation(s_ctrl, s5k4ecgx_ctrl.settings.exposure);
            break;
        case EXT_CAM_WB:
            s5k4ecgx_ctrl.settings.wb = (cam_info->value_1);
            if (s5k4ecgx_ctrl.streamon == 1)
                s5k4ecgx_set_white_balance(s_ctrl, s5k4ecgx_ctrl.settings.wb);
            break;
        case EXT_CAM_METERING:
            s5k4ecgx_ctrl.settings.metering = (cam_info->value_1);
            s5k4ecgx_set_metering(s_ctrl, s5k4ecgx_ctrl.settings.metering);
            break;
        case EXT_CAM_ISO:
            s5k4ecgx_ctrl.settings.iso = (cam_info->value_1);
            if (s5k4ecgx_ctrl.streamon == 1)
                s5k4ecgx_set_iso(s_ctrl, s5k4ecgx_ctrl.settings.iso);
            break;
        case EXT_CAM_EFFECT:
            s5k4ecgx_ctrl.settings.effect = (cam_info->value_1);
            s5k4ecgx_set_effect(s_ctrl, s5k4ecgx_ctrl.settings.effect);
            break;
        case EXT_CAM_SCENE_MODE:
            s5k4ecgx_ctrl.settings.scenemode = (cam_info->value_1);
            s5k4ecgx_set_scene_mode(s_ctrl, s5k4ecgx_ctrl.settings.scenemode);
#if !defined(CONFIG_SEC_J1X_PROJECT)
            // During Camera Init time two times below setting are Executting
            /* setting scene mode is resetting EV,ISO & WB - so re-applying it again */
            s5k4ecgx_set_exposure_compensation(s_ctrl, s5k4ecgx_ctrl.settings.exposure);
            s5k4ecgx_set_iso(s_ctrl, s5k4ecgx_ctrl.settings.iso);
            s5k4ecgx_set_white_balance(s_ctrl, s5k4ecgx_ctrl.settings.wb);
#endif

            break;
        case EXT_CAM_SENSOR_MODE:
            s5k4ecgx_ctrl.prev_mode = s5k4ecgx_ctrl.op_mode;
            s5k4ecgx_ctrl.op_mode = (cam_info->value_1);
            CDBG("EXT_CAM_SENSOR_MODE = %d", s5k4ecgx_ctrl.op_mode);
            break;
        case EXT_CAM_FOCUS:
            s5k4ecgx_ctrl.settings.focus_mode = (cam_info->value_1);
            /*CDBG("EXT_CAM_FOCUS focus mode = %d\n ", s5k4ecgx_ctrl.settings.focus_mode);*/
            s5k4ecgx_set_af_mode(s_ctrl, s5k4ecgx_ctrl.settings.focus_mode);
            break;
        case EXT_CAM_SET_AF_STATUS:
            /*CDBG("EXT_CAM_SET_AF_STATUS: %d : %d\n", cam_info->value_1, cam_info->value_2);*/
            cam_info->value_1 = s5k4ecgx_set_af_status(s_ctrl, cam_info->value_1,
                    cam_info->value_2);
            if (!copy_to_user((void *)argp, (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");
            break;
        case EXT_CAM_SET_TOUCHAF_POS:
            /*CDBG("EXT_CAM_SET_TOUCHAF_POS: %d : %d", cam_info->value_1, cam_info->value_2);*/
            if(cam_info->value_1 !=0 && cam_info->value_2 != 0) {
                s5k4ecgx_set_touchaf_pos(s_ctrl, cam_info);
                s5k4ecgx_ctrl.settings.is_touchaf = 1;
            } else {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_reset_touchaf);
                s5k4ecgx_ctrl.settings.is_touchaf = 0;
            }
            break;
        case EXT_CAM_GET_AF_STATUS:
            /*CDBG("EXT_CAM_GET_AF_STATUS: %d\n", cam_info->value_1);*/
            cam_info->value_1 = s5k4ecgx_get_af_status(s_ctrl, cam_info->value_1);
            if (!copy_to_user((void *)argp, (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");
            break;
        case EXT_CAM_FLASH_MODE:
            /*CDBG("EXT_CAM_FLASH_MODE  = %d\n", (cam_info->value_1));*/
            s5k4ecgx_ctrl.settings.flash_mode = (cam_info->value_1);
            if((cam_info->value_1) == CAMERA_FLASH_TORCH) {
                set_led_flash(MSM_CAMERA_LED_LOW);
            } else {
                set_led_flash(MSM_CAMERA_LED_OFF);
            }
            break;
        case EXT_CAM_SET_FLASH:
            CDBG("EXT_CAM_SET_FLASH = %d\n", (cam_info->value_1));
            if( cam_info->value_1 && s5k4ecgx_get_flash_status() ){
                set_led_flash(MSM_CAMERA_LED_HIGH);
                S5K4ECGX_WRITE_LIST(s5k4ecgx_Main_Flash_On);
            } else {
                S5K4ECGX_WRITE_LIST(s5k4ecgx_Main_Flash_Off);
                set_led_flash(MSM_CAMERA_LED_OFF);
            }
            break;
        case EXT_CAM_EXIF:
            s5k4ecgx_get_exif(cam_info);
            if (!copy_to_user((void *)argp,
                        (const void *)&cam_info,
                        sizeof(cam_info)))
                pr_err("copy failed");

            break;
        case EXT_CAM_SET_AE_AWB:
            CDBG("EXT_CAM_SET_AE_AWB = %d\n", (cam_info->value_1));
            if (!s5k4ecgx_ctrl.settings.flash_mode)
                s5k4ecgx_set_ae_awb(s_ctrl, cam_info->value_1);
            break;

	case EXT_CAM_CONTRAST:
	     CDBG("EXT_CAM_CONTRAST isn't supported \n");
	     break;
	case EXT_CAM_FPS_RANGE:
            if (cam_info->value_1 > 0)
                s5k4ecgx_ctrl.fixed_fps_val = cam_info->value_1;
            else
                s5k4ecgx_ctrl.fixed_fps_val = 0;
            break;
        default:
            rc = -EFAULT;
            break;
    }

    mutex_unlock(s_ctrl->msm_sensor_mutex);

    return rc;
}

#ifdef CONFIG_LOAD_FILE
int s5k4ecgx_regs_from_sd_tunning(struct msm_camera_i2c_reg_conf *settings, struct msm_sensor_ctrl_t *s_ctrl,char * name) {
    char *start, *end, *reg;
    int addr,rc = 0;
    unsigned int  value;
    char reg_buf[7], data_buf1[7];
    *(reg_buf + 6) = '\0';
    *(data_buf1 + 6) = '\0';
    printk("PHANI.....Inside Modified Tuning s5k4ecgx_regs_from_sd_tunning\n");
    if (settings != NULL){
        pr_err("s5k4ecgx_regs_from_sd_tunning start address %x start data %x",settings->reg_addr,settings->reg_data);
    }
    if(s5k4ecgx_regs_table == NULL) {
        pr_err("s5k4ecgx_regs_table is null ");
        return -1;
    }
    pr_err("@@@ %s \n",name);
    start = strstr(s5k4ecgx_regs_table, name);
    if (start == NULL){
        return -1;
    }
    end = strstr(start, "};");
    while (1) {
        reg = strstr(start, "{0x");
        if ((reg == NULL) || (reg > end))
            break;
        if (reg != NULL) {
            memcpy(reg_buf, (reg + 1), 6);
            memcpy(data_buf1, (reg + 9), 6);
            if(kstrtoint(reg_buf, 16, &addr))
                pr_err("kstrtoint error in reg_buf .Please Align contents of the Header file!!") ;
            if(kstrtoint(data_buf1, 16, &value))
                pr_err("kstrtoint error in data_buf1 .Please Align contents of the Header file!!");
            if (reg)
                start = (reg + 18);
            if (addr == 0xff){
                msleep(value);
                pr_err("delay = %d\n", (int)value);
            }
            else{
                rc=s_ctrl->sensor_i2c_client->i2c_func_tbl->
                    i2c_write(s_ctrl->sensor_i2c_client, addr,
                            value,MSM_CAMERA_I2C_WORD_DATA);
            }
        }
    }
    pr_err("s5k4ecgx_regs_from_sd_tunning end!");
    return rc;
}
void s5k4ecgx_regs_table_exit(void)
{
    printk("%s %d", __func__, __LINE__);
    if (s5k4ecgx_regs_table) {
        vfree(s5k4ecgx_regs_table);
        s5k4ecgx_regs_table = NULL;
    }
}
void s5k4ecgx_regs_table_init(char *filename)
{
    struct file *filp;
    char *dp;
    long lsize;
    loff_t pos;
    int ret;
    mm_segment_t fs = get_fs();
    pr_err("%s %d", __func__, __LINE__);
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
    filp_close(filp, current->files);
    set_fs(fs);
    pr_err("coming to if part of string compare s5k4ecgx_regs_table");
    s5k4ecgx_regs_table = dp;
    s5k4ecgx_regs_table_size = lsize;
    *((s5k4ecgx_regs_table + s5k4ecgx_regs_table_size) - 1) = '\0';
    return;
}
#endif
