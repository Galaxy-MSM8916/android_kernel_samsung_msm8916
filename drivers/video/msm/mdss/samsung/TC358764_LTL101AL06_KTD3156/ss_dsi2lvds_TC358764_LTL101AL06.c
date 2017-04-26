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
#include "ss_dsi2lvds_TC358764_LTL101AL06.h"

#define MAX_BRIGHTNESS_LEVEL		255
#define MID_BRIGHTNESS_LEVEL		143
#define LOW_BRIGHTNESS_LEVEL		10
#define DIM_BRIGHTNESS_LEVEL		20

#define BL_MAX_BRIGHTNESS_LEVEL		106
#define BL_MID_BRIGHTNESS_LEVEL		48
#define BL_MIN_BRIGHTNESS		3
#define BL_LOW_BRIGHTNESS_LEVEL		3
#define BL_DIM_BRIGHTNESS_LEVEL		3

#define BL_MIN_BRIGHTNESS_REV03		1
#define BL_LOW_BRIGHTNESS_LEVEL_REV03	1
#define BL_DIM_BRIGHTNESS_LEVEL_REV03	1
#define BL_DEFAULT_BRIGHTNESS		BL_MID_BRIGHTNESS_LEVEL

#define MMSS_GP3_BASE			0x0180A000
#define MMSS_GP3_SIZE			0x28

extern unsigned int system_rev;

static void bklt_ap_gpio_pwm_clk(int enable)
{
	pr_info("%s %d\n", __func__, enable);
	if(enable){
		HWIO_OUTM(GP0_CMD_RCGR,HWIO_UPDATE_VAL_BMSK,		1 << HWIO_UPDATE_VAL_SHFT);     //UPDATE ACTIVE
		HWIO_OUTM(GP0_CMD_RCGR,HWIO_ROOT_EN_VAL_BMSK,		1 << HWIO_ROOT_EN_VAL_SHFT);    //ROOT_EN
		HWIO_OUTM(CAMSS_GP0_CBCR, HWIO_CLK_ENABLE_VAL_BMSK,	1 << HWIO_CLK_ENABLE_VAL_SHFT); //CLK_ENABLE
	}else {
		HWIO_OUTM(GP0_CMD_RCGR,HWIO_UPDATE_VAL_BMSK,		0 << HWIO_UPDATE_VAL_SHFT);     //UPDATE ACTIVE
		HWIO_OUTM(GP0_CMD_RCGR,HWIO_ROOT_EN_VAL_BMSK,		0 << HWIO_ROOT_EN_VAL_SHFT);    //ROOT_EN
		HWIO_OUTM(CAMSS_GP0_CBCR, HWIO_CLK_ENABLE_VAL_BMSK,	0 << HWIO_CLK_ENABLE_VAL_SHFT); //CLK_ENABLE
	}
}

static void bklt_ap_gpio_pwm(int level)
{
	if(virt_mmss_gp_base == NULL) {
		pr_err("%s:############ I/O remap failed \n", __func__);
		return ;
	}
	pr_info("%s : %d\n", __func__, level);

	/* Put the MND counter in reset mode for programming */
	HWIO_OUTM(GP0_CFG_RCGR, HWIO_GP_SRC_SEL_VAL_BMSK,     1 << HWIO_GP_SRC_SEL_VAL1_SHFT); //SRC_SEL = 001(GPLL0_MAIN.)
	HWIO_OUTM(GP0_CFG_RCGR, HWIO_GP_SRC_DIV_VAL_BMSK,    31 << HWIO_GP_SRC_DIV_VAL_SHFT); //SRC_DIV = 11111 (Div 16)
	HWIO_OUTM(GP0_CFG_RCGR, HWIO_GP_MODE_VAL_BMSK,        2 << HWIO_GP_MODE_VAL_SHFT);    //Mode Select 10

	//M value
	HWIO_OUTM(GP_M_REG, HWIO_GP_MD_REG_M_VAL_BMSK,	GP_CLK_M_DEFAULT << HWIO_GP_MD_REG_M_VAL_SHFT);

	// D value
	HWIO_OUTM(GP_D_REG, HWIO_GP_MD_REG_D_VAL_BMSK,	 (~((int16_t)level << 1)) << HWIO_GP_MD_REG_D_VAL_SHFT);

	//N value
	HWIO_OUTM(GP_NS_REG, HWIO_GP_NS_REG_GP_N_VAL_BMSK, ~(GP_CLK_N_DEFAULT - GP_CLK_M_DEFAULT) << 0);

	bklt_ap_gpio_pwm_clk(1);
}

static unsigned char mdss_dsi_panel_pwm_scaling(int level)
{
	unsigned char scaled_level;

	if ( system_rev >=3) {
		if (level >= MAX_BRIGHTNESS_LEVEL)
			scaled_level  = BL_MAX_BRIGHTNESS_LEVEL;
		else if (level >= MID_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - MID_BRIGHTNESS_LEVEL) *
			(BL_MAX_BRIGHTNESS_LEVEL - BL_MID_BRIGHTNESS_LEVEL) / (MAX_BRIGHTNESS_LEVEL-MID_BRIGHTNESS_LEVEL) + BL_MID_BRIGHTNESS_LEVEL;
		} else if (level >= DIM_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - DIM_BRIGHTNESS_LEVEL) *
			(BL_MID_BRIGHTNESS_LEVEL - BL_DIM_BRIGHTNESS_LEVEL_REV03) / (MID_BRIGHTNESS_LEVEL-DIM_BRIGHTNESS_LEVEL) + BL_DIM_BRIGHTNESS_LEVEL_REV03;
		} else if (level >= LOW_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - LOW_BRIGHTNESS_LEVEL) *
			(BL_DIM_BRIGHTNESS_LEVEL_REV03 - BL_LOW_BRIGHTNESS_LEVEL_REV03) / (DIM_BRIGHTNESS_LEVEL-LOW_BRIGHTNESS_LEVEL) + BL_LOW_BRIGHTNESS_LEVEL_REV03;
		}  else{
			scaled_level  = BL_MIN_BRIGHTNESS_REV03;
		}
	}
	else {
		if (level >= MAX_BRIGHTNESS_LEVEL)
			scaled_level  = BL_MAX_BRIGHTNESS_LEVEL;
		else if (level >= MID_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - MID_BRIGHTNESS_LEVEL) *
			(BL_MAX_BRIGHTNESS_LEVEL - BL_MID_BRIGHTNESS_LEVEL) / (MAX_BRIGHTNESS_LEVEL-MID_BRIGHTNESS_LEVEL) + BL_MID_BRIGHTNESS_LEVEL;
		} else if (level >= DIM_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - DIM_BRIGHTNESS_LEVEL) *
			(BL_MID_BRIGHTNESS_LEVEL - BL_DIM_BRIGHTNESS_LEVEL) / (MID_BRIGHTNESS_LEVEL-DIM_BRIGHTNESS_LEVEL) + BL_DIM_BRIGHTNESS_LEVEL;
		} else if (level >= LOW_BRIGHTNESS_LEVEL) {
			scaled_level  = (level - LOW_BRIGHTNESS_LEVEL) *
			(BL_DIM_BRIGHTNESS_LEVEL - BL_LOW_BRIGHTNESS_LEVEL) / (DIM_BRIGHTNESS_LEVEL-LOW_BRIGHTNESS_LEVEL) + BL_LOW_BRIGHTNESS_LEVEL;
		}  else{
			scaled_level  = BL_MIN_BRIGHTNESS;
		}
	}

	pr_info("%s  level = [%d]: scaled_level = [%d] \n",__func__,level,scaled_level);
	return scaled_level;
}

static void mdss_brightness_tft_pwm_gpio(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	vdd->scaled_level = mdss_dsi_panel_pwm_scaling(level);
	bklt_ap_gpio_pwm(vdd->scaled_level);
	pr_debug("%s bl_level : %d scaled_level : %d\n", __func__, level, vdd->scaled_level);

	return;
}

static void kdt3156_pwm_backlight_onoff(struct mdss_dsi_ctrl_pdata *ctrl, int enable)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	pr_err("%s :enable:[%d]\n", __func__,enable);

	if (enable) {
		vdd->scaled_level = mdss_dsi_panel_pwm_scaling(vdd->bl_level);
		bklt_ap_gpio_pwm(vdd->scaled_level);
	}else {
		bklt_ap_gpio_pwm_clk(0);
	}
}

static void backlight_tft_late_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	pr_err("%s : Backlight on sleep\n", __func__);
	msleep(200);	// Add for LCD Timing (T3)
	msleep(300);	// Add for LCD Timing (T3)
	kdt3156_pwm_backlight_onoff(ctrl,1);
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
	kdt3156_pwm_backlight_onoff(ctrl,0);
	msleep(300);	// Add for LCD Timing (T4)

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

static void mdss_panel_blic_init(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	struct mdss_panel_info *pinfo = NULL;

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}
	pinfo = &(ctrl->panel_data.panel_info);
	pinfo->blank_state = MDSS_PANEL_BLANK_UNBLANK;

	vdd->manufacture_id_dsi[ctrl->ndx] = get_lcd_attached("GET");
	pr_info("%s: DSI%d manufacture_id=0x%x\n", __func__, ctrl->ndx, vdd->manufacture_id_dsi[ctrl->ndx]);
}

static void mdss_panel_init(struct samsung_display_driver_data *vdd)
{
	pr_info("%s : %s", __func__, vdd->panel_name);

	virt_mmss_gp_base = ioremap(MMSS_GP3_BASE, MMSS_GP3_SIZE);
	bklt_ap_gpio_pwm(12);/*duty 9%*/

	vdd->support_panel_max = TC358764_BOE_SUPPORT_PANEL_COUNT;
	vdd->support_mdnie_lite = false;
	vdd->support_cabc = false;

	/* ON/OFF */
	vdd->panel_func.samsung_panel_on_pre = mdss_panel_on_pre;
	vdd->panel_func.samsung_panel_on_post = NULL;
	vdd->panel_func.samsung_panel_off_pre = mdss_panel_off_pre;
	vdd->panel_func.samsung_panel_off_post = NULL;
	vdd->panel_func.samsung_backlight_late_on = backlight_tft_late_on;

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

	vdd->panel_func.samsung_brightness_tft_pwm = mdss_brightness_tft_pwm_gpio;
	vdd->panel_func.samsung_brightness_tft_pwm_ldi = NULL;
	vdd->panel_func.samsung_tft_blic_init = mdss_panel_blic_init;

	mdss_panel_attach_set(vdd->ctrl_dsi[DISPLAY_1], true);
}

static int __init samsung_panel_init(void)
{
	struct samsung_display_driver_data *vdd = samsung_get_vdd();
	char panel_string[] = "ss_dsi_panel_TC358764_LTL101A106_WXGA";
	char panel2_string[] = "ss_dsi_panel_TC358764_BP101WX1_WXGA";

	vdd->panel_name = mdss_mdp_panel + 8;

	pr_info("%s : %s\n", __func__, vdd->panel_name);

	if ((!strncmp(vdd->panel_name, panel_string, strlen(panel_string)))|| \
		(!strncmp(vdd->panel_name, panel2_string, strlen(panel2_string))))
		vdd->panel_func.samsung_panel_init = mdss_panel_init;

	return 0;
}
early_initcall(samsung_panel_init);
