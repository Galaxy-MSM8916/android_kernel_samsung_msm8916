/*
 * =================================================================
 *
 *
 *	Description:  samsung display panel file
 *
 *	Author: prabhu.manoj
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
#include "ss_dsi2lvds_VX5B3D_LTM184HL01.h"
static int first_boot = 1;

#define V5D3BX_VEESTRENGHT		0x00001f07
#define V5D3BX_VEEDEFAULTVAL		0
#define V5D3BX_DEFAULT_STRENGHT		5
#define V5D3BX_DEFAULT_LOW_STRENGHT	8
#define V5D3BX_DEFAULT_HIGH_STRENGHT	10
#define V5D3BX_MAX_STRENGHT		15

#define V5D3BX_CABCBRIGHTNESSRATIO	815
#define V5D3BX_10KHZ_DEFAULT_RATIO	187040
#define AUTOBRIGHTNESS_LIMIT_VALUE	207

#define MIN_BRIGHTNESS			0
#define MAX_BRIGHTNESS_LEVEL		255
#define MID_BRIGHTNESS_LEVEL		195
#define LOW_BRIGHTNESS_LEVEL		29
#define DIM_BRIGHTNESS_LEVEL		19
#define BL_DEFAULT_BRIGHTNESS		MID_BRIGHTNESS_LEVEL

#define V5D3BX_MIN_BRIGHTNESS			0
#define V5D3BX_MAX_BRIGHTNESS_LEVEL_SDC		330
#define V5D3BX_MID_BRIGHTNESS_LEVEL_SDC		185

#define V5D3BX_MAX_BRIGHTNESS_LEVEL_BOE		255
#define V5D3BX_MID_BRIGHTNESS_LEVEL_BOE		195

#define V5D3BX_LOW_BRIGHTNESS_LEVEL		11
#define V5D3BX_DIM_BRIGHTNESS_LEVEL		5

static void mdss_brightness_tft_pwm_lvds(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	int vx5b3d_level = 0;
	u32 vee_strenght = 0;
	static u32 prev_vee_strenght=0;

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	if (!get_lcd_attached("GET"))
		return;

	pr_err("%s bl level(%d)\n", __func__, level);

	/* brightness tuning*/
	if (level > MAX_BRIGHTNESS_LEVEL)
		level = MAX_BRIGHTNESS_LEVEL;

	if (level >= MID_BRIGHTNESS_LEVEL) {
		vx5b3d_level  = (level - MID_BRIGHTNESS_LEVEL) *
		(V5D3BX_MAX_BRIGHTNESS_LEVEL_BOE - V5D3BX_MID_BRIGHTNESS_LEVEL_BOE) / (MAX_BRIGHTNESS_LEVEL-MID_BRIGHTNESS_LEVEL) + V5D3BX_MID_BRIGHTNESS_LEVEL_BOE;
	} else if (level >= LOW_BRIGHTNESS_LEVEL) {
		vx5b3d_level  = (level - LOW_BRIGHTNESS_LEVEL) *
		(V5D3BX_MID_BRIGHTNESS_LEVEL_BOE - V5D3BX_LOW_BRIGHTNESS_LEVEL) / (MID_BRIGHTNESS_LEVEL-LOW_BRIGHTNESS_LEVEL) + V5D3BX_LOW_BRIGHTNESS_LEVEL;
	} else if (level >= DIM_BRIGHTNESS_LEVEL) {
		vx5b3d_level  = (level - DIM_BRIGHTNESS_LEVEL) *
		(V5D3BX_LOW_BRIGHTNESS_LEVEL - V5D3BX_DIM_BRIGHTNESS_LEVEL) / (LOW_BRIGHTNESS_LEVEL-DIM_BRIGHTNESS_LEVEL) + V5D3BX_DIM_BRIGHTNESS_LEVEL;
	} else if (level > 0)
		vx5b3d_level  = V5D3BX_DIM_BRIGHTNESS_LEVEL;
	else {
		vx5b3d_level = 0;
		pr_info("level = [%d]: vx5b3d_level = [%d]\n", level,vx5b3d_level);
	}

	switch (vdd->auto_brightness) {
		case	0 ... 3:
			vee_strenght = V5D3BX_DEFAULT_STRENGHT;
			break;
		case	4 ... 5:
			vee_strenght = V5D3BX_DEFAULT_LOW_STRENGHT;
			break;
		case	6 ... 8:
			vee_strenght = V5D3BX_DEFAULT_HIGH_STRENGHT;
			break;
		default:
			vee_strenght = V5D3BX_DEFAULT_STRENGHT;
	}

	vee_strenght = V5D3BX_VEESTRENGHT | ((vee_strenght) << 27);

	if ((vdd->auto_brightness && vdd->auto_brightness < 5) || vdd->siop_status)
		vx5b3d_level = (vx5b3d_level * V5D3BX_CABCBRIGHTNESSRATIO) / 1000;

	if((vee_strenght != prev_vee_strenght)&& vx5b3d_level) {
		vdd->panel_func.samsung_lvds_write_reg(0x400,0);/*temp*/
		prev_vee_strenght = vee_strenght;
	}

	if (vx5b3d_level != 0) {
		vdd->panel_func.samsung_lvds_write_reg(0x164,(vx5b3d_level*V5D3BX_10KHZ_DEFAULT_RATIO)/255);

		pr_info("[MIPI2LVDS-18INCH]:level=%d vx5b3d_level:%d auto_brightness:%d CABC:%d \n",\
			level,vx5b3d_level,vdd->auto_brightness,vdd->siop_status);
	}

	pr_info("%s bl_level : %d \n", __func__, level);

	vdd->bl_level = level;

	return;
}

static void mdss_lvds_data_ql(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	vdd->panel_func.samsung_lvds_write_reg(0x700, 0x18900040);
	vdd->panel_func.samsung_lvds_write_reg(0x704, 0x101D8);
	vdd->panel_func.samsung_lvds_write_reg(0x70C, 0x00004604);
	vdd->panel_func.samsung_lvds_write_reg(0x710, 0x0545100B);
	vdd->panel_func.samsung_lvds_write_reg(0x714, 0x20);
	vdd->panel_func.samsung_lvds_write_reg(0x718, 0x00000102);
	vdd->panel_func.samsung_lvds_write_reg(0x71C, 0xA8002F);
	vdd->panel_func.samsung_lvds_write_reg(0x720, 0x1800);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x00000000);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x80000000);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x700, 0x18900840);
	vdd->panel_func.samsung_lvds_write_reg(0x70C, 0x5E46);
	vdd->panel_func.samsung_lvds_write_reg(0x718, 0x00000202);

	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x00000000);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x80000000);
	usleep_range(1000,1000);

	vdd->panel_func.samsung_lvds_write_reg(0x120, 0x5);
	vdd->panel_func.samsung_lvds_write_reg(0x124, 0x421C780);
	vdd->panel_func.samsung_lvds_write_reg(0x128, 0x102008);
	vdd->panel_func.samsung_lvds_write_reg(0x12C, 0x10);
	vdd->panel_func.samsung_lvds_write_reg(0x130, 0x3C10);
	vdd->panel_func.samsung_lvds_write_reg(0x134, 0x15);
	vdd->panel_func.samsung_lvds_write_reg(0x138, 0xFF0000);
	vdd->panel_func.samsung_lvds_write_reg(0x13C, 0x0);

	vdd->panel_func.samsung_lvds_write_reg(0x114, 0xc6302);/*added for bl pwm*/
	vdd->panel_func.samsung_lvds_write_reg(0x140, 0x10000);

	vdd->panel_func.samsung_lvds_write_reg(0x20C, 0x24);
	vdd->panel_func.samsung_lvds_write_reg(0x21C, 0x780);
	vdd->panel_func.samsung_lvds_write_reg(0x224, 0x7);
	vdd->panel_func.samsung_lvds_write_reg(0x228, 0x50000);
	vdd->panel_func.samsung_lvds_write_reg(0x22C, 0xFF08);
	vdd->panel_func.samsung_lvds_write_reg(0x230, 0x1);
	vdd->panel_func.samsung_lvds_write_reg(0x234, 0xCA033E10);
	vdd->panel_func.samsung_lvds_write_reg(0x238, 0x00000060);
	vdd->panel_func.samsung_lvds_write_reg(0x23C, 0x82E86030);
	vdd->panel_func.samsung_lvds_write_reg(0x240, 0x28616088);
	vdd->panel_func.samsung_lvds_write_reg(0x244, 0x00160285);
	vdd->panel_func.samsung_lvds_write_reg(0x250, 0x600882A8);

	vdd->panel_func.samsung_lvds_write_reg(0x258, 0x80010);
	vdd->panel_func.samsung_lvds_write_reg(0x158, 0x0);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x158, 0x1);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x37C, 0x00001063);
	vdd->panel_func.samsung_lvds_write_reg(0x380, 0x82A86030);
	vdd->panel_func.samsung_lvds_write_reg(0x384, 0x2861408B);
	vdd->panel_func.samsung_lvds_write_reg(0x388, 0x00130285);
	vdd->panel_func.samsung_lvds_write_reg(0x38C, 0x10630009);
	vdd->panel_func.samsung_lvds_write_reg(0x394, 0x400B82A8);
	vdd->panel_func.samsung_lvds_write_reg(0x600, 0x16CC78D);
	vdd->panel_func.samsung_lvds_write_reg(0x608, 0x20F0A);/*20 : delay for skew*/

	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x00000000);
	usleep_range(1000,1000);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x80000000);
	usleep_range(1000,1000);
	/*vee strenght initialization*/
	vdd->panel_func.samsung_lvds_write_reg(0x400, 0x0);
	usleep_range(80, 80);
#if 0
	/*backlight duty ration control when device is first bring up.*/
	vdd->panel_func.samsung_lvds_write_reg(0x160, 0x4B4/*0xff*/);
	vdd->panel_func.samsung_lvds_write_reg(0x138, 0x3fff0000);
	vdd->panel_func.samsung_lvds_write_reg(0x15c, 0x5);
	vdd->panel_func.samsung_lvds_write_reg(0x164, 0xcf);
	/*LVDS Enable*/
	vdd->panel_func.samsung_lvds_write_reg(0x604, 0x3FFFFFE0);
#endif
}

static void mdss_lvds_data_ql2(struct samsung_display_driver_data *vdd)
{
	pr_err("%s\n", __func__);

	msleep(5);
        /*pwm freq.=33.66M /(0x2DAA1+1)= 180hz;  */
	vdd->panel_func.samsung_lvds_write_reg(0x160, 0x2DAA1);
	vdd->panel_func.samsung_lvds_write_reg(0x604, 0x3FFFFD08);
	msleep(200);

	vdd->panel_func.samsung_lvds_write_reg(0x138, 0x3fff0000);
	vdd->panel_func.samsung_lvds_write_reg(0x15c, 0x5);/*selelct pwm*/
	msleep(1);/*after init -3*/

#ifdef __RGB_OUT__
	pr_info(" <delay for RGB out> !!! ================================ \n");
	msleep(500);
	msleep(500);
	pr_info(" <making RGB out> !!! =================================== \n");
	vdd->panel_func.samsung_lvds_write_reg(0x70C, 0x5E76);
	vdd->panel_func.samsung_lvds_write_reg(0x710, 0x54D004F);
	vdd->panel_func.samsung_lvds_write_reg(0x134, 0x05);
	mdelay(1);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x00000000);
	mdelay(1);
	vdd->panel_func.samsung_lvds_write_reg(0x154, 0x80000000);
	mdelay(1);
	pr_info(" <ending RGB out> !!! =================================== \n");
#endif
}

static void backlight_tft_late_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return ;
	}

	pr_err("%s \n", __func__);
	mdss_lvds_data_ql2(vdd);

	if(vdd->bl_level)
		mdss_brightness_tft_pwm_lvds(ctrl, vdd->bl_level);

	if(first_boot) {
		mdss_brightness_tft_pwm_lvds(ctrl, 200);
		first_boot = 0;
	}

	if (gpio_is_valid(ctrl->bklt_en_gpio)){
		gpio_set_value((ctrl->bklt_en_gpio), 1);
		pr_err("%s : bklt_en_gpio on\n", __func__);
	}

}

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

static int mdss_panel_off_pre(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	if (gpio_is_valid(ctrl->bklt_en_gpio)){
		gpio_set_value((ctrl->bklt_en_gpio), 0);
		pr_err("%s : bklt_en_gpio off\n", __func__);
	}

	msleep(100);//after off, make delay 1s T6

	return true;
}

static int mdss_panel_off_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);
	/*joann_test*/
	msleep(30);//after off, make delay 1s T3

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

static void mdss_panel_init(struct samsung_display_driver_data *vdd)
{
	pr_info("%s : %s", __func__, vdd->panel_name);

	vdd->support_panel_max = VX5B3D_LTM184HL01_SUPPORT_PANEL_COUNT;
	vdd->manufacture_id_dsi[vdd->support_panel_max - 1] = get_lcd_attached("GET");
	vdd->support_mdnie_lite = false;
	vdd->support_cabc = false;

	/* ON/OFF */
	vdd->panel_func.samsung_panel_on_pre = mdss_panel_on_pre;
	vdd->panel_func.samsung_panel_on_post = NULL;
	vdd->panel_func.samsung_panel_off_pre = mdss_panel_off_pre;
	vdd->panel_func.samsung_panel_off_post = mdss_panel_off_post;
	vdd->panel_func.samsung_backlight_late_on = backlight_tft_late_on;
	vdd->panel_func.samsung_ql_lvds_register_set = mdss_lvds_data_ql;

	/* DDI RX */
	vdd->panel_func.samsung_panel_revision = mdss_panel_revision;
	vdd->panel_func.samsung_manufacture_date_read = NULL;
	vdd->panel_func.samsung_ddi_id_read = NULL;
	vdd->panel_func.samsung_hbm_read = NULL;
	vdd->panel_func.samsung_mdnie_read = NULL;
	vdd->panel_func.samsung_smart_dimming_init = NULL;

	/* Brightness */
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
	vdd->panel_func.samsung_brightness_tft_pwm = mdss_brightness_tft_pwm_lvds;

	mdss_panel_attach_set(vdd->ctrl_dsi[DISPLAY_1], true);
}

static int __init samsung_panel_init(void)
{
	struct samsung_display_driver_data *vdd = samsung_get_vdd();
	char panel_string[] = "ss_dsi_panel_VX5B3D_LTM184HL01_FHD";

	vdd->panel_name = mdss_mdp_panel + 8;

	pr_info("%s : %s\n", __func__, vdd->panel_name);

	if (!strncmp(vdd->panel_name, panel_string, strlen(panel_string)))
		vdd->panel_func.samsung_panel_init = mdss_panel_init;

	return 0;
}
early_initcall(samsung_panel_init);
