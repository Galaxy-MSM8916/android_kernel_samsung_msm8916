/*
 * =================================================================
 *
 *
 *	Description:  samsung display common file
 *
 *	Author: jb09.kim
 *	Company:  Samsung Electronics
 *
 * ================================================================
 */
/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) 2012, Samsung Electronics. All rights reserved.

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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
*/
#ifndef _SAMSUNG_DSI_TCON_MDNIE_LIGHT_H_
#define _SAMSUNG_DSI_TCON_MDNIE_LIGHT_H_

#include "ss_dsi_panel_common.h"

#define NATURAL_MODE_ENABLE

#define NAME_STRING_MAX 30
#define MDNIE_COLOR_BLINDE_CMD_SIZE 18
#define COORDINATE_DATA_SIZE 6

extern char mdnie_app_name[][NAME_STRING_MAX];
extern char mdnie_mode_name[][NAME_STRING_MAX];
extern char outdoor_name[][NAME_STRING_MAX];
extern struct mdnie_lite_tune_data mdnie_data;

#define APP_ID_TDMB (20)	/* for fake_id() */

enum BYPASS {
	BYPASS_DISABLE = 0,
	BYPASS_ENABLE,
};

enum APP {
	UI_APP = 0,
	VIDEO_APP,
	VIDEO_WARM_APP,
	VIDEO_COLD_APP,
	CAMERA_APP,
	NAVI_APP,
	GALLERY_APP,
	VT_APP,
	BROWSER_APP,
	eBOOK_APP,
	EMAIL_APP,
	TDMB_APP,	/* is linked to APP_ID_TDMB */
	MAX_APP_MODE,
};

enum MODE {
	DYNAMIC_MODE = 0,
	STANDARD_MODE,
#if defined(NATURAL_MODE_ENABLE)
	NATURAL_MODE,
#endif
	MOVIE_MODE,
	AUTO_MODE,
	READING_MODE,
	MAX_MODE,
};

enum OUTDOOR {
	OUTDOOR_OFF_MODE = 0,
	OUTDOOR_ON_MODE,
	MAX_OUTDOOR_MODE,
};

enum ACCESSIBILITY {
	ACCESSIBILITY_OFF = 0,
	NEGATIVE,
	COLOR_BLIND,
	CURTAIN,
	GRAYSCALE,
	GRAYSCALE_NEGATIVE,
	ACCESSIBILITY_MAX,
};

enum HMT_GRAY {
	HMT_GRAY_OFF = 0,
	HMT_GRAY_1,
	HMT_GRAY_2,
	HMT_GRAY_3,
	HMT_GRAY_4,
	HMT_GRAY_5,
	HMT_GRAY_MAX,
};

enum HMT_COLOR_TEMPERATURE {
	HMT_COLOR_TEMP_OFF = 0,
	HMT_COLOR_TEMP_3000K, // 3000K
	HMT_COLOR_TEMP_4000K, // 4000K
	HMT_COLOR_TEMP_5000K, // 5000K
	HMT_COLOR_TEMP_6500K, // 6500K + gamma 2.2
	HMT_COLOR_TEMP_7500K, // 7500K + gamma 2.2
	HMT_COLOR_TEMP_MAX
};

struct mdnie_lite_tun_type {
	enum BYPASS mdnie_bypass;
	enum BYPASS cabc_bypass;
	int hbm_enable;

	enum APP mdnie_app;
	enum MODE mdnie_mode;
	enum OUTDOOR outdoor;
	enum ACCESSIBILITY mdnie_accessibility;
	enum HMT_COLOR_TEMPERATURE hmt_color_temperature;

	char scr_white_red;
	char scr_white_green;
	char scr_white_blue;

	int index;
	struct list_head used_list;

	struct samsung_display_driver_data *vdd;
};

extern int config_cabc(int value);

enum {
	MDNIE_STEP1 = 0,
	MDNIE_STEP2,
	MDNIE_STEP_MAX,
};

enum {
	ADDRESS_SCR_WHITE_RED_OFFSET = 0,
	ADDRESS_SCR_WHITE_GREEN_OFFSET,
	ADDRESS_SCR_WHITE_BLUE_OFFSET,
	ADDRESS_SCR_WHITE_MAX,
};

/* COMMON DATA THAT POINT TO MDNIE TUNING DATA */
struct mdnie_lite_tune_data {
/*******************************************
*					DSI0 DATA
********************************************/

	char *DSI0_COLOR_BLIND_MDNIE_2;
	char *DSI0_RGB_SENSOR_MDNIE_1;
	char *DSI0_RGB_SENSOR_MDNIE_2;
	char *DSI0_UI_DYNAMIC_MDNIE_2;
	char *DSI0_UI_STANDARD_MDNIE_2;
	char *DSI0_UI_AUTO_MDNIE_2;
	char *DSI0_VIDEO_DYNAMIC_MDNIE_2;
	char *DSI0_VIDEO_STANDARD_MDNIE_2;
	char *DSI0_VIDEO_AUTO_MDNIE_2;
	char *DSI0_CAMERA_MDNIE_2;
	char *DSI0_CAMERA_AUTO_MDNIE_2;
	char *DSI0_GALLERY_DYNAMIC_MDNIE_2;
	char *DSI0_GALLERY_STANDARD_MDNIE_2;
	char *DSI0_GALLERY_AUTO_MDNIE_2;
	char *DSI0_VT_DYNAMIC_MDNIE_2;
	char *DSI0_VT_STANDARD_MDNIE_2;
	char *DSI0_VT_AUTO_MDNIE_2;
	char *DSI0_BROWSER_DYNAMIC_MDNIE_2;
	char *DSI0_BROWSER_STANDARD_MDNIE_2;
	char *DSI0_BROWSER_AUTO_MDNIE_2;
	char *DSI0_EBOOK_DYNAMIC_MDNIE_2;
	char *DSI0_EBOOK_STANDARD_MDNIE_2;
	char *DSI0_EBOOK_AUTO_MDNIE_2;
	char *DSI0_TDMB_DYNAMIC_MDNIE_2;
	char *DSI0_TDMB_STANDARD_MDNIE_2;
	char *DSI0_TDMB_AUTO_MDNIE_2;

	struct dsi_cmd_desc *DSI0_BYPASS_MDNIE;
	struct dsi_cmd_desc *DSI0_NEGATIVE_MDNIE;
	struct dsi_cmd_desc *DSI0_COLOR_BLIND_MDNIE;
	struct dsi_cmd_desc *DSI0_HBM_CE_MDNIE;
	struct dsi_cmd_desc *DSI0_HBM_CE_TEXT_MDNIE;
	struct dsi_cmd_desc *DSI0_RGB_SENSOR_MDNIE;
	struct dsi_cmd_desc *DSI0_CURTAIN;
	struct dsi_cmd_desc *DSI0_GRAYSCALE_MDNIE;
	struct dsi_cmd_desc *DSI0_GRAYSCALE_NEGATIVE_MDNIE;
	struct dsi_cmd_desc *DSI0_UI_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_UI_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_UI_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_UI_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_UI_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_WARM_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_WARM_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_COLD_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI0_VIDEO_COLD_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_CAMERA_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_GALLERY_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_GALLERY_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_GALLERY_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_GALLERY_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_GALLERY_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_VT_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_VT_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_VT_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_VT_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_VT_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_BROWSER_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_BROWSER_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_BROWSER_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_BROWSER_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_BROWSER_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_EBOOK_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_EBOOK_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_EBOOK_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_EBOOK_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_EBOOK_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_EMAIL_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI0_TDMB_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI0_TDMB_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI0_TDMB_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI0_TDMB_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI0_TDMB_AUTO_MDNIE;

	struct dsi_cmd_desc *(*mdnie_tune_value_dsi0)[MAX_MODE][MAX_OUTDOOR_MODE];
	struct dsi_cmd_desc **hmt_color_temperature_tune_value_dsi0;

	int dsi0_bypass_mdnie_size;
	int mdnie_color_blinde_cmd_offset;
	int mdnie_step_index[MDNIE_STEP_MAX];
	int address_scr_white[ADDRESS_SCR_WHITE_MAX];
	int dsi0_rgb_sensor_mdnie_1_size;
	int dsi0_rgb_sensor_mdnie_2_size;

/*******************************************
*					DSI1 DATA
********************************************/
	char *DSI1_COLOR_BLIND_MDNIE_2;
	char *DSI1_RGB_SENSOR_MDNIE_1;
	char *DSI1_RGB_SENSOR_MDNIE_2;
	char *DSI1_UI_DYNAMIC_MDNIE_2;
	char *DSI1_UI_STANDARD_MDNIE_2;
	char *DSI1_UI_AUTO_MDNIE_2;
	char *DSI1_VIDEO_DYNAMIC_MDNIE_2;
	char *DSI1_VIDEO_STANDARD_MDNIE_2;
	char *DSI1_VIDEO_AUTO_MDNIE_2;
	char *DSI1_CAMERA_MDNIE_2;
	char *DSI1_CAMERA_AUTO_MDNIE_2;
	char *DSI1_GALLERY_DYNAMIC_MDNIE_2;
	char *DSI1_GALLERY_STANDARD_MDNIE_2;
	char *DSI1_GALLERY_AUTO_MDNIE_2;
	char *DSI1_VT_DYNAMIC_MDNIE_2;
	char *DSI1_VT_STANDARD_MDNIE_2;
	char *DSI1_VT_AUTO_MDNIE_2;
	char *DSI1_BROWSER_DYNAMIC_MDNIE_2;
	char *DSI1_BROWSER_STANDARD_MDNIE_2;
	char *DSI1_BROWSER_AUTO_MDNIE_2;
	char *DSI1_EBOOK_DYNAMIC_MDNIE_2;
	char *DSI1_EBOOK_STANDARD_MDNIE_2;
	char *DSI1_EBOOK_AUTO_MDNIE_2;
	char *DSI1_TDMB_DYNAMIC_MDNIE_2;
	char *DSI1_TDMB_STANDARD_MDNIE_2;
	char *DSI1_TDMB_AUTO_MDNIE_2;

	struct dsi_cmd_desc *DSI1_BYPASS_MDNIE;
	struct dsi_cmd_desc *DSI1_NEGATIVE_MDNIE;
	struct dsi_cmd_desc *DSI1_COLOR_BLIND_MDNIE;
	struct dsi_cmd_desc *DSI1_HBM_CE_MDNIE;
	struct dsi_cmd_desc *DSI1_HBM_CE_TEXT_MDNIE;
	struct dsi_cmd_desc *DSI1_RGB_SENSOR_MDNIE;
	struct dsi_cmd_desc *DSI1_CURTAIN;
	struct dsi_cmd_desc *DSI1_GRAYSCALE_MDNIE;
	struct dsi_cmd_desc *DSI1_GRAYSCALE_NEGATIVE_MDNIE;
	struct dsi_cmd_desc *DSI1_UI_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_UI_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_UI_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_UI_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_UI_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_WARM_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_WARM_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_COLD_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI1_VIDEO_COLD_MDNIE;
	struct dsi_cmd_desc *DSI1_CAMERA_OUTDOOR_MDNIE;
	struct dsi_cmd_desc *DSI1_CAMERA_MDNIE;
	struct dsi_cmd_desc *DSI1_CAMERA_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_GALLERY_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_GALLERY_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_GALLERY_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_GALLERY_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_GALLERY_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_VT_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_VT_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_VT_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_VT_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_VT_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_BROWSER_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_BROWSER_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_BROWSER_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_BROWSER_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_BROWSER_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_EBOOK_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_EBOOK_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_EBOOK_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_EBOOK_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_EBOOK_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_EMAIL_AUTO_MDNIE;
	struct dsi_cmd_desc *DSI1_TDMB_DYNAMIC_MDNIE;
	struct dsi_cmd_desc *DSI1_TDMB_STANDARD_MDNIE;
	struct dsi_cmd_desc *DSI1_TDMB_NATURAL_MDNIE;
	struct dsi_cmd_desc *DSI1_TDMB_MOVIE_MDNIE;
	struct dsi_cmd_desc *DSI1_TDMB_AUTO_MDNIE;

	struct dsi_cmd_desc *(*mdnie_tune_value_dsi1)[MAX_MODE][MAX_OUTDOOR_MODE];
	struct dsi_cmd_desc **hmt_color_temperature_tune_value_dsi1;

	int dsi1_bypass_mdnie_size;
	//int mdnie_color_blinde_cmd_offset;
	//int mdnie_step_index[MDNIE_STEP_MAX];
	//int address_scr_white[ADDRESS_SCR_WHITE_MAX];
	int dsi1_rgb_sensor_mdnie_1_size;
	int dsi1_rgb_sensor_mdnie_2_size;
};

/* COMMON FUNCTION*/
struct mdnie_lite_tun_type* init_dsi_tcon_mdnie_class(int index, struct samsung_display_driver_data *vdd_data);
int update_dsi_tcon_mdnie_register(struct samsung_display_driver_data *vdd);
void coordinate_tunning(int index, char *coordinate_data, int scr_wr_addr, int data_size);
void coordinate_tunning_multi(int index, char (*coordinate_data_multi[MAX_MODE])[COORDINATE_DATA_SIZE], int mdnie_tune_index, int scr_wr_addr, int data_size);
/* COMMON FUNCTION END*/

#endif /*_DSI_TCON_MDNIE_H_*/
