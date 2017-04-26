/*
 * =================================================================
 *
 *
 *	Description:  samsung display panel file
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
//#include "ss_dsi_panel_HX8369B_BV045WVM.h"
//#include "ss_dsi_mdnie_HX8369B_BV045WVM.h"
#include "ss_dsi_panel_S6d77A1A01_VIDEO.h"
#include "ss_dsi_mdnie_S6d77A1A01_VIDEO.h"


static int mdss_panel_on_pre(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	mdss_panel_attach_set(ctrl, true);

	return true;
}

static int mdss_panel_on_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	mdss_samsung_cabc_update();

	return true;
}

static int mdss_panel_revision(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	vdd->panel_revision = 0;

	return true;
}

static struct dsi_panel_cmds * mdss_brightness_tft_pwm(struct mdss_dsi_ctrl_pdata *ctrl, int *level_key)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return NULL;
	}

	vdd->scaled_level = get_scaled_level(vdd, ctrl->ndx);

	pr_info("%s bl_level : %d scaled_level : %d\n", __func__, vdd->bl_level, vdd->scaled_level);

	vdd->dtsi_data[ctrl->ndx].tft_pwm_tx_cmds->cmds->payload[1] = vdd->scaled_level ;

	return &vdd->dtsi_data[ctrl->ndx].tft_pwm_tx_cmds[vdd->panel_revision];
}

static void dsi_update_mdnie_data(void)
{
	/* Update mdnie command */
	mdnie_data.DSI0_COLOR_BLIND_MDNIE_2 = NULL;
	mdnie_data.DSI0_RGB_SENSOR_MDNIE_1 = NULL;
	mdnie_data.DSI0_RGB_SENSOR_MDNIE_2 = NULL;
	mdnie_data.DSI0_UI_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_UI_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_UI_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_VIDEO_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_VIDEO_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_VIDEO_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_CAMERA_MDNIE_2 = NULL;
	mdnie_data.DSI0_CAMERA_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_GALLERY_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_GALLERY_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_GALLERY_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_VT_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_VT_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_VT_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_BROWSER_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_BROWSER_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_BROWSER_AUTO_MDNIE_2 = NULL;
	mdnie_data.DSI0_EBOOK_DYNAMIC_MDNIE_2 = NULL;
	mdnie_data.DSI0_EBOOK_STANDARD_MDNIE_2 = NULL;
	mdnie_data.DSI0_EBOOK_AUTO_MDNIE_2 = NULL;

	mdnie_data.DSI0_BYPASS_MDNIE = NULL;
	mdnie_data.DSI0_NEGATIVE_MDNIE = DSI0_NEGATIVE_MDNIE;
	mdnie_data.DSI0_COLOR_BLIND_MDNIE = NULL;
	mdnie_data.DSI0_HBM_CE_MDNIE = NULL;
	mdnie_data.DSI0_RGB_SENSOR_MDNIE = NULL;
	mdnie_data.DSI0_CURTAIN = NULL;
	mdnie_data.DSI0_UI_DYNAMIC_MDNIE = DSI0_UI_MDNIE;
	mdnie_data.DSI0_UI_STANDARD_MDNIE = DSI0_UI_MDNIE;
	mdnie_data.DSI0_UI_NATURAL_MDNIE = DSI0_UI_MDNIE;
	mdnie_data.DSI0_UI_MOVIE_MDNIE = DSI0_UI_MDNIE;
	mdnie_data.DSI0_UI_AUTO_MDNIE = DSI0_UI_MDNIE;
	mdnie_data.DSI0_VIDEO_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_DYNAMIC_MDNIE = DSI0_VIDEO_MDNIE;
	mdnie_data.DSI0_VIDEO_STANDARD_MDNIE = DSI0_VIDEO_MDNIE;
	mdnie_data.DSI0_VIDEO_NATURAL_MDNIE = DSI0_VIDEO_MDNIE;
	mdnie_data.DSI0_VIDEO_MOVIE_MDNIE = DSI0_VIDEO_MDNIE;
	mdnie_data.DSI0_VIDEO_AUTO_MDNIE = DSI0_VIDEO_MDNIE;
	mdnie_data.DSI0_VIDEO_WARM_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_WARM_MDNIE = DSI0_VIDEO_WARM_MDNIE;
	mdnie_data.DSI0_VIDEO_COLD_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_COLD_MDNIE = DSI0_VIDEO_COLD_MDNIE;
	mdnie_data.DSI0_CAMERA_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_CAMERA_MDNIE = DSI0_CAMERA_MDNIE;
	mdnie_data.DSI0_CAMERA_AUTO_MDNIE = DSI0_CAMERA_MDNIE;
	mdnie_data.DSI0_GALLERY_DYNAMIC_MDNIE = DSI0_GALLERY_MDNIE;
	mdnie_data.DSI0_GALLERY_STANDARD_MDNIE = DSI0_GALLERY_MDNIE;
	mdnie_data.DSI0_GALLERY_NATURAL_MDNIE = DSI0_GALLERY_MDNIE;
	mdnie_data.DSI0_GALLERY_MOVIE_MDNIE = DSI0_GALLERY_MDNIE;
	mdnie_data.DSI0_GALLERY_AUTO_MDNIE = DSI0_GALLERY_MDNIE;
	mdnie_data.DSI0_VT_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_VT_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_VT_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_VT_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_VT_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_BROWSER_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_BROWSER_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_BROWSER_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_BROWSER_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_BROWSER_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_EBOOK_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_EBOOK_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_EBOOK_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_EBOOK_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_EBOOK_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_EMAIL_AUTO_MDNIE = NULL;

	mdnie_data.mdnie_tune_value_dsi0 = mdnie_tune_value_dsi0;

	/* Update MDNIE data related with size, offset or index */
	mdnie_data.dsi0_bypass_mdnie_size = ARRAY_SIZE(DSI0_UI_MDNIE);
	mdnie_data.mdnie_color_blinde_cmd_offset = MDNIE_COLOR_BLINDE_CMD_OFFSET;
	mdnie_data.mdnie_step_index[MDNIE_STEP1] = MDNIE_STEP1_INDEX;
	mdnie_data.mdnie_step_index[MDNIE_STEP2] = MDNIE_STEP2_INDEX;
	mdnie_data.address_scr_white[ADDRESS_SCR_WHITE_RED_OFFSET] = 0;
	mdnie_data.address_scr_white[ADDRESS_SCR_WHITE_GREEN_OFFSET] = 0;
	mdnie_data.address_scr_white[ADDRESS_SCR_WHITE_BLUE_OFFSET] = 0;
	mdnie_data.dsi0_rgb_sensor_mdnie_1_size = 0;
	mdnie_data.dsi0_rgb_sensor_mdnie_2_size = 0;
}

static void mdss_panel_init(struct samsung_display_driver_data *vdd)
{
	pr_info("%s : %s", __func__, vdd->panel_name);

	vdd->support_panel_max = HX8369B_BV045WVM_SUPPORT_PANEL_COUNT;
	vdd->support_mdnie_lite = true;
	vdd->mdnie_tune_size1 = 113;
	vdd->mdnie_tune_size2 = 0;
	vdd->manufacture_id_dsi[vdd->support_panel_max - 1] = get_lcd_attached("GET");

	vdd->support_cabc = true;
	/* ON/OFF */
	vdd->panel_func.samsung_panel_on_pre = mdss_panel_on_pre;
	vdd->panel_func.samsung_panel_on_post = mdss_panel_on_post;
	vdd->panel_func.samsung_panel_off_pre = NULL;
	vdd->panel_func.samsung_panel_off_post = NULL;

	/* DDI RX */
	vdd->panel_func.samsung_panel_revision = mdss_panel_revision;
	vdd->panel_func.samsung_manufacture_date_read = NULL;
	vdd->panel_func.samsung_ddi_id_read = NULL;
	vdd->panel_func.samsung_hbm_read = NULL;
	vdd->panel_func.samsung_mdnie_read = NULL;
	vdd->panel_func.samsung_smart_dimming_init = NULL;

	/* Brightness */
	vdd->panel_func.samsung_brightness_tft_pwm_ldi = mdss_brightness_tft_pwm;
	vdd->panel_func.samsung_brightness_hbm_off = NULL;
	vdd->panel_func.samsung_brightness_aid = NULL;
	vdd->panel_func.samsung_brightness_acl_on = NULL;
	vdd->panel_func.samsung_brightness_acl_percent = NULL;
	vdd->panel_func.samsung_brightness_acl_off = NULL;
	vdd->panel_func.samsung_brightness_elvss = NULL;
	vdd->panel_func.samsung_brightness_elvss_temperature1 = NULL;
	vdd->panel_func.samsung_brightness_elvss_temperature2 = NULL;
	vdd->panel_func.samsung_brightness_vint = NULL;
	vdd->panel_func.samsung_brightness_gamma = NULL;
	vdd->brightness[0].brightness_packet_tx_cmds_dsi.link_state = DSI_HS_MODE;

	dsi_update_mdnie_data();
	mdss_panel_attach_set(vdd->ctrl_dsi[DISPLAY_1], true);
}

static int __init samsung_panel_init(void)
{
	struct samsung_display_driver_data *vdd = samsung_get_vdd();
//	char panel_string[] = "ss_dsi_panel_S6D77A1A01_WVGA";
/*please rework: let this alive*/

	vdd->panel_name = mdss_mdp_panel + 8;
	pr_info("%s : %s\n", __func__, vdd->panel_name);

/*please rework: let this alive*/
//	if (!strncmp(vdd->panel_name, panel_string, strlen(panel_string)))
		vdd->panel_func.samsung_panel_init = mdss_panel_init;

	return 0;
}
early_initcall(samsung_panel_init);
