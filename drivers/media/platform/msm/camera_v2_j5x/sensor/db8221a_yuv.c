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

#include "db8221a.h"
#include "db8221a_yuv.h"
#include "msm_sd.h"
#include "camera.h"
#include "msm_cci.h"
#include "msm_camera_dt_util.h"

#if defined CONFIG_SEC_CAMERA_TUNING
#define DB8221A_WRITE_LIST(A) \
    if (front_tune) \
        register_read_from_sdcard(A,s_ctrl,MSM_CAMERA_I2C_BYTE_DATA,#A); \
    else \
        s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl( \
            s_ctrl->sensor_i2c_client, A, ARRAY_SIZE(A), \
            MSM_CAMERA_I2C_BYTE_DATA); CDBG("REGSEQ *** %s", #A)
#else
#define DB8221A_WRITE_LIST(A) \
    s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_conf_tbl( \
        s_ctrl->sensor_i2c_client, A, ARRAY_SIZE(A), \
        MSM_CAMERA_I2C_BYTE_DATA); CDBG("REGSEQ *** %s", #A)
#endif

static struct yuv_ctrl db8221a_ctrl;
static exif_data_t db8221a_exif;

static int32_t streamon = 0;
static int32_t resolution = MSM_SENSOR_RES_FULL;
static int32_t prev_fps = 0;

#if defined CONFIG_SEC_CAMERA_TUNING
#define FILENAME "/data/db8221a_yuv.h"
extern int register_read_from_sdcard (struct msm_camera_i2c_reg_conf *settings,
						struct msm_sensor_ctrl_t *s_ctrl,
						enum msm_camera_i2c_data_type data_type,
						char *name);
extern int register_table_init(char *filename);
extern void register_table_exit(void);
extern int front_tune;
#endif

static int db8221a_exif_shutter_speed(struct msm_sensor_ctrl_t *s_ctrl)
{
	u16 read_value1 = 0;
	u16 read_value2 = 0;
	u16 coarseTime = 0;
	u16 lineLength = 0;
	const int SensorInputClock = 72000000;
	int rc = 0;

	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_write(s_ctrl->sensor_i2c_client, 0xFF, 0xE8, MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x06, &read_value1, MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x07, &read_value2, MSM_CAMERA_I2C_BYTE_DATA);

	coarseTime = ((read_value1&0xFF) << 8) | read_value2;

	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_write(s_ctrl->sensor_i2c_client, 0xFF, 0xE8, MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x0A, &read_value1, MSM_CAMERA_I2C_BYTE_DATA);
	s_ctrl->sensor_i2c_client->i2c_func_tbl->
		i2c_read(s_ctrl->sensor_i2c_client, 0x0B, &read_value2, MSM_CAMERA_I2C_BYTE_DATA);

	lineLength = ((read_value1&0xFF) << 8) | read_value2;
	CDBG("%s::%d coarseTime = %d, lineLength = %d",__func__,__LINE__,coarseTime,lineLength);

	if(coarseTime == 0 || lineLength == 0){
		pr_err("%s : %d I2C data is 0\n", __func__, __LINE__);
		rc = -EFAULT;
	}
	else{
		db8221a_exif.shutterspeed = SensorInputClock/(coarseTime*lineLength*2);
		CDBG("Exposure time = %d", db8221a_exif.shutterspeed);
	}
	return rc;
}

static int db8221a_exif_iso(struct msm_sensor_ctrl_t *s_ctrl)
{
	if (db8221a_exif.shutterspeed <= 10)
		db8221a_exif.iso = 400;
	else if (db8221a_exif.shutterspeed > 10 && db8221a_exif.shutterspeed <= 20)
		db8221a_exif.iso = 200;
	else if (db8221a_exif.shutterspeed > 20 && db8221a_exif.shutterspeed <= 30)
		db8221a_exif.iso = 100;
	else
		db8221a_exif.iso = 50;
	CDBG("ISO = %d", db8221a_exif.iso);
	return 0;
}

static int db8221a_get_exif(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	CDBG("%s : %d E",__func__,__LINE__);
	rc = db8221a_exif_shutter_speed(s_ctrl);
	if( rc < 0 ) return rc;
	rc = db8221a_exif_iso(s_ctrl);
	return rc;
}

int32_t db8221a_get_exif_info(struct ioctl_native_cmd * exif_info)
{
	CDBG("%s : %d E",__func__,__LINE__);
	exif_info->value_1 = 1;	// equals 1 to update the exif value in the user level.
	exif_info->value_2 = db8221a_exif.iso;
	exif_info->value_3 = db8221a_exif.shutterspeed;
	return 0;
}

int32_t db8221a_set_exposure_compensation(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("CAM-SETTING -- EV is %d", mode);
	switch (mode) {
	case CAMERA_EV_M4:
		DB8221A_WRITE_LIST(db8221a_bright_m4);
		break;
	case CAMERA_EV_M3:
		DB8221A_WRITE_LIST(db8221a_bright_m3);
		break;
	case CAMERA_EV_M2:
		DB8221A_WRITE_LIST(db8221a_bright_m2);
		break;
	case CAMERA_EV_M1:
		DB8221A_WRITE_LIST(db8221a_bright_m1);
		break;
	case CAMERA_EV_DEFAULT:
		DB8221A_WRITE_LIST(db8221a_bright_default);
		break;
	case CAMERA_EV_P1:
		DB8221A_WRITE_LIST(db8221a_bright_p1);
		break;
	case CAMERA_EV_P2:
		DB8221A_WRITE_LIST(db8221a_bright_p2);
		break;
	case CAMERA_EV_P3:
		DB8221A_WRITE_LIST(db8221a_bright_p3);
		break;
	case CAMERA_EV_P4:
		DB8221A_WRITE_LIST(db8221a_bright_p4);
		break;
	default:
		pr_err("%s: Setting %d is invalid", __func__, mode);
		rc = -EINVAL;
	}
	return rc;
}

int32_t db8221a_set_white_balance(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("CAM-SETTING -- WB is %d", mode);
	switch (mode) {
	case CAMERA_WHITE_BALANCE_OFF:
	case CAMERA_WHITE_BALANCE_AUTO:
		DB8221A_WRITE_LIST(db8221a_wb_auto);
		break;
	case CAMERA_WHITE_BALANCE_INCANDESCENT:
		DB8221A_WRITE_LIST(db8221a_wb_incandescent);
		break;
	case CAMERA_WHITE_BALANCE_FLUORESCENT:
		DB8221A_WRITE_LIST(db8221a_wb_fluorescent);
		break;
	case CAMERA_WHITE_BALANCE_DAYLIGHT:
		DB8221A_WRITE_LIST(db8221a_wb_daylight);
		break;
	case CAMERA_WHITE_BALANCE_CLOUDY_DAYLIGHT:
		DB8221A_WRITE_LIST(db8221a_wb_cloudy);
		break;
	default:
		pr_err("%s: Setting %d is invalid", __func__, mode);
		rc = -EINVAL;
	}
	return rc;
}

int32_t db8221a_set_resolution(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("CAM-SETTING-- resolution is %d", mode);
	switch (mode) {
	case MSM_SENSOR_RES_FULL:
		if (db8221a_ctrl.fixed_fps_val == 24000) {
			DB8221A_WRITE_LIST(db8221a_Capture_1600_1200_for_24fps);
		}
		else {
			DB8221A_WRITE_LIST(db8221a_Capture_1600_1200);
		}
		break;
	case MSM_SENSOR_RES_QTR:
#if defined(CONFIG_MACH_J3LTE_USA_VZW) || defined(CONFIG_MACH_J3LTE_USA_USC)
		if (db8221a_ctrl.fixed_fps_val == 24000) {
			DB8221A_WRITE_LIST(db8221a_24fps_Camcoder);
		} else {
			if (24000 == prev_fps) {	// from 24fps to auto fps
				CDBG("prev_fps is 24!");
				DB8221A_WRITE_LIST(db8221a_Init_Reg);
				DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
			}
			DB8221A_WRITE_LIST(db8221a_preview_640_480);
		}
#else
		if (db8221a_ctrl.fixed_fps_val == 24000) {
			DB8221A_WRITE_LIST(db8221a_24fps_Camcoder_800_600);
		}
		else {
			DB8221A_WRITE_LIST(db8221a_preview_800_600);
		}
#endif
		break;
	case MSM_SENSOR_RES_2:
		DB8221A_WRITE_LIST(db8221a_preview_640_480);
		break;
	case MSM_SENSOR_RES_3:
		DB8221A_WRITE_LIST(db8221a_preview_352_288);
		break;
	case MSM_SENSOR_RES_4:
		DB8221A_WRITE_LIST(db8221a_preview_320_240);
		break;
	case MSM_SENSOR_RES_5:
		DB8221A_WRITE_LIST(db8221a_preview_176_144);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
		rc = -EINVAL;
	}

	return rc;
}

int32_t db8221a_set_effect(struct msm_sensor_ctrl_t *s_ctrl, int mode)
{
	int32_t rc = 0;
	CDBG("CAM-SETTING-- effect is %d", mode);
	switch (mode) {
	case CAMERA_EFFECT_OFF:
		DB8221A_WRITE_LIST(db8221a_effect_none);
		break;
	case CAMERA_EFFECT_MONO:
		DB8221A_WRITE_LIST(db8221a_effect_gray);
		break;
	case CAMERA_EFFECT_NEGATIVE:
		DB8221A_WRITE_LIST(db8221a_effect_negative);
		break;
	case CAMERA_EFFECT_SEPIA:
		DB8221A_WRITE_LIST(db8221a_effect_sepia);
		break;
	default:
		pr_err("%s: Setting %d is invalid\n", __func__, mode);
		rc = -EINVAL;
	}
	return rc;
}

int32_t db8221a_sensor_config(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	struct sensorb_cfg_data *cdata = (struct sensorb_cfg_data *)argp;
	int32_t rc = 0;
	int32_t i = 0;
	mutex_lock(s_ctrl->msm_sensor_mutex);

	CDBG("ENTER %d", cdata->cfgtype);

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

		cdata->cfg.sensor_info.is_mount_angle_valid =
			s_ctrl->sensordata->sensor_info->is_mount_angle_valid;
		cdata->cfg.sensor_info.sensor_mount_angle =
			s_ctrl->sensordata->sensor_info->sensor_mount_angle;
		cdata->cfg.sensor_info.position =
			s_ctrl->sensordata->sensor_info->position;
		cdata->cfg.sensor_info.modes_supported =
			s_ctrl->sensordata->sensor_info->modes_supported;

		CDBG("sensor name %s", cdata->cfg.sensor_info.sensor_name);
		CDBG("session id %d", cdata->cfg.sensor_info.session_id);
        CDBG("mount angle valid %d value %d", cdata->cfg.sensor_info.is_mount_angle_valid,
			cdata->cfg.sensor_info.sensor_mount_angle);
#if 0
		for (i = 0; i < SUB_MODULE_MAX; i++)
			CDBG("%s:%d subdev_id[%d] %d", __func__, __LINE__, i,
				cdata->cfg.sensor_info.subdev_id[i]);
#endif
		break;
	case CFG_SET_INIT_SETTING:
		CDBG("CFG_SET_INIT_SETTING\n");
		db8221a_ctrl.vtcall_mode = 0;

#ifdef CONFIG_SEC_CAMERA_TUNING
		if (front_tune){
			register_table_init(FILENAME);
			pr_err("%s[%d] %s inside CFG_SET_INIT_SETTING", __func__,	__LINE__ ,FILENAME);
		}
#endif
		break;

	case CFG_SET_RESOLUTION:
		CDBG("CFG_SET_RESOLUTION");
		resolution = *((int32_t  *)cdata->cfg.setting);

		if((db8221a_ctrl.prev_mode == CAMERA_MODE_INIT || db8221a_ctrl.prev_mode == CAMERA_MODE_RECORDING)
			&& (db8221a_ctrl.op_mode == CAMERA_MODE_PREVIEW)){
			if (db8221a_ctrl.vtcall_mode) {
				CDBG("vt seq");
				DB8221A_WRITE_LIST(db8221a_VT_Init_Reg);
				//msleep(10);
				if (cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
					CDBG("50Hz init setting");
					DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_50Hz);
				} else {
					CDBG("60Hz init setting");
					DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
				}
			}
			else{
				CDBG("preview seq");
				DB8221A_WRITE_LIST(db8221a_Init_Reg);
				if (cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
					DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_50Hz);
				} else {
					DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
				}
			}
			db8221a_set_effect(s_ctrl, db8221a_ctrl.settings.effect);
			//db8221a_set_white_balance(s_ctrl, db8221a_ctrl.settings.wb);
			db8221a_set_exposure_compensation(s_ctrl , db8221a_ctrl.settings.exposure);
			msleep(100); //Sensor needs 400~500ms of AWB/AE stable time after video recording
			db8221a_set_resolution(s_ctrl , resolution);
		}
		else if(db8221a_ctrl.op_mode == CAMERA_MODE_PREVIEW){
			db8221a_set_effect(s_ctrl, db8221a_ctrl.settings.effect);
			db8221a_set_exposure_compensation(s_ctrl , db8221a_ctrl.settings.exposure);
			db8221a_set_resolution(s_ctrl, resolution);
		}
		else if((db8221a_ctrl.prev_mode == CAMERA_MODE_PREVIEW)
			&& (db8221a_ctrl.op_mode == CAMERA_MODE_CAPTURE)){
			CDBG("capture seq");
			if( resolution == MSM_SENSOR_RES_FULL ){
				db8221a_set_resolution(s_ctrl , resolution);
				db8221a_get_exif(s_ctrl);
			}
			else{
				pr_err("%s:%d invalid capture size\n", __func__, __LINE__);
				rc = -EINVAL;
			}
		}
		else if(db8221a_ctrl.op_mode == CAMERA_MODE_RECORDING){
			CDBG("recording seq");
			if (db8221a_ctrl.prev_mode == CAMERA_MODE_INIT) {

				if (db8221a_ctrl.vtcall_mode) {
					CDBG("vt seq");
					DB8221A_WRITE_LIST(db8221a_VT_Init_Reg);
					//msleep(10);
					if (cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
						CDBG("50Hz init setting");
						DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_50Hz);
					} else {
						CDBG("60Hz init setting");
						DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
					}
				}
				else{
					CDBG("preview seq");
					DB8221A_WRITE_LIST(db8221a_Init_Reg);
					if (cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
						DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_50Hz);
					} else {
						DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
					}
				}
				db8221a_set_effect(s_ctrl, db8221a_ctrl.settings.effect);
				//db8221a_set_white_balance(s_ctrl, db8221a_ctrl.settings.wb);
				db8221a_set_exposure_compensation(s_ctrl , db8221a_ctrl.settings.exposure);
				db8221a_set_resolution(s_ctrl , resolution);
				msleep(50);

			}
			DB8221A_WRITE_LIST(db8221a_24fps_Camcoder);
			if (cdata->flicker_type == MSM_CAM_FLICKER_50HZ) {
				DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_50Hz);
			} else {
				DB8221A_WRITE_LIST(db8221a_anti_banding_flicker_60Hz);
			}
		}
		else{
			pr_err("%s:%d invalid sequence db8221a_ctrl.prev_mode:%d, db8221a_ctrl.op_mode:%d\n",
				__func__, __LINE__, db8221a_ctrl.prev_mode, db8221a_ctrl.op_mode);
			rc = -EFAULT;
		}
		break;

	case CFG_SET_STOP_STREAM:
		CDBG("CFG_SET_STOP_STREAM");
		if(streamon == 1){
			DB8221A_WRITE_LIST(db8221a_stream_stop);
			rc=0;
			streamon = 0;
		}
		break;
	case CFG_SET_START_STREAM:
		CDBG(" CFG_SET_START_STREAM");
		streamon = 1;
		break;
	case CFG_SET_SLAVE_INFO: {
		struct msm_camera_sensor_slave_info sensor_slave_info;
		struct msm_camera_power_ctrl_t *p_ctrl;
		uint16_t size;

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
		CDBG("sensor id %x", sensor_slave_info.slave_addr);
		CDBG("sensor addr type %d", sensor_slave_info.addr_type);
		CDBG("sensor reg %x", sensor_slave_info.sensor_id_info.sensor_id_reg_addr);
		CDBG("sensor id %x", sensor_slave_info.sensor_id_info.sensor_id);
#if 0
		{
			int slave_index = 0;
			for (slave_index = 0; slave_index <
				p_ctrl->power_setting_size; slave_index++) {
				CDBG("%s i %d power setting %d %d %ld %d", __func__,
					slave_index,
					p_ctrl->power_setting[slave_index].seq_type,
					p_ctrl->power_setting[slave_index].seq_val,
					p_ctrl->power_setting[slave_index].config_val,
					p_ctrl->power_setting[slave_index].delay);
			}
		}
#endif
		break;
	}
	case CFG_WRITE_I2C_ARRAY: {
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
		break;
	}
	case CFG_WRITE_I2C_SEQ_ARRAY: {
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
			i2c_write_seq_table(s_ctrl->sensor_i2c_client,
			&conf_array);
		kfree(reg_setting);
		break;
	}

	case CFG_POWER_UP:
		prev_fps = 0;
		streamon = 0;
		db8221a_ctrl.op_mode = CAMERA_MODE_INIT;
		db8221a_ctrl.prev_mode = CAMERA_MODE_INIT;
		db8221a_ctrl.settings.metering = CAMERA_CENTER_WEIGHT;
		db8221a_ctrl.settings.exposure = CAMERA_EV_DEFAULT;
		db8221a_ctrl.settings.wb = CAMERA_WHITE_BALANCE_AUTO;
		db8221a_ctrl.settings.iso = CAMERA_ISO_MODE_AUTO;
		db8221a_ctrl.settings.effect = CAMERA_EFFECT_OFF;
		db8221a_ctrl.settings.scenemode = CAMERA_SCENE_AUTO;
		if (s_ctrl->func_tbl->sensor_power_up) {
            CDBG("CFG_POWER_UP");
			rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
		}
		else {
			pr_err("%s : %d undefined sensor_power_up",__func__,__LINE__);
			rc = -EFAULT;
		}
		break;

	case CFG_POWER_DOWN:
		 if (s_ctrl->func_tbl->sensor_power_down) {
            CDBG("CFG_POWER_DOWN");
			rc = s_ctrl->func_tbl->sensor_power_down(s_ctrl);
		}
		else{
			pr_err("%s : %d undefined sensor_power_down",__func__,__LINE__);
			rc = -EFAULT;
		}
		break;

	case CFG_SET_STOP_STREAM_SETTING: {
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
		break;
	}
	default:
		pr_err("%s: %d invalid\n", __func__, __LINE__);
		//rc = -EINVAL;
		break;
	}

	mutex_unlock(s_ctrl->msm_sensor_mutex);
	CDBG("EXIT");

	return rc;
}

int32_t db8221a_sensor_native_control(struct msm_sensor_ctrl_t *s_ctrl,
	void __user *argp)
{
	int32_t rc = 0;
	struct ioctl_native_cmd *cam_info = (struct ioctl_native_cmd *)argp;
	mutex_lock(s_ctrl->msm_sensor_mutex);

	switch (cam_info->mode) {
	case EXT_CAM_EV:
		db8221a_ctrl.settings.exposure = cam_info->value_1;
		if(streamon == 1)
			db8221a_set_exposure_compensation(s_ctrl, db8221a_ctrl.settings.exposure);
		break;
	case EXT_CAM_WB:
		db8221a_ctrl.settings.wb = cam_info->value_1;
		if(streamon == 1)
			db8221a_set_white_balance(s_ctrl, db8221a_ctrl.settings.wb);
		break;
	case EXT_CAM_EFFECT:
		db8221a_ctrl.settings.effect = cam_info->value_1;
		if(streamon == 1)
			db8221a_set_effect(s_ctrl, db8221a_ctrl.settings.effect);
		break;
	case EXT_CAM_SENSOR_MODE:
		db8221a_ctrl.prev_mode = db8221a_ctrl.op_mode;
		db8221a_ctrl.op_mode = cam_info->value_1;
		break;
	case EXT_CAM_EXIF:
		db8221a_get_exif_info(cam_info);
		if (!copy_to_user((void *)argp,
			(const void *)&cam_info,
			sizeof(cam_info)))
		CDBG("copy failed");
		break;
	case EXT_CAM_VT_MODE:
		db8221a_ctrl.vtcall_mode = cam_info->value_1;
		break;
	case EXT_CAM_FPS_RANGE:
		CDBG("Prev_fps %d EXT_CAM_FPS_RANGE %d", db8221a_ctrl.fixed_fps_val, cam_info->value_1);
		prev_fps = db8221a_ctrl.fixed_fps_val;
		db8221a_ctrl.fixed_fps_val = cam_info->value_1;
		break;
	default:
		pr_err("%s: %d unsupport mode : %d\n", __func__, __LINE__, cam_info->mode);
		//rc = -EINVAL;
		break;
	}
	mutex_unlock(s_ctrl->msm_sensor_mutex);
    CDBG("%s : %d EXIT", __func__, __LINE__);
	return rc;
}
