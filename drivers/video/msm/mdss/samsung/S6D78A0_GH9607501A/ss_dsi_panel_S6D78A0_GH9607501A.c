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
#include "ss_dsi_panel_S6D78A0_GH9607501A.h"
#include "ss_dsi_mdnie_S6D78A0_GH9607501A.h"
static int is_first_boot = 1;
static int lcd_brightness;
static DEFINE_SPINLOCK(bg_gpio_lock);

static void ktd3102_set_led_current(int pulse, struct mdss_dsi_ctrl_pdata *ctrl_pdata)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&bg_gpio_lock, irq_flags);
	for (; pulse > 0; pulse--) {
		gpio_set_value(ctrl_pdata->bklt_en_gpio, 0);
		udelay(3);
		gpio_set_value(ctrl_pdata->bklt_en_gpio, 1);
		udelay(3);
	}
	spin_unlock_irqrestore(&bg_gpio_lock, irq_flags);

	mdelay(1);
}
static void ktd3102_set_brightness(int level, struct mdss_dsi_ctrl_pdata *ctrl)
{
	int pulse;
	int tune_level = 0;
	unsigned long irq_flags;
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	tune_level = level;

	if(level == 0){
		gpio_set_value((vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1]), 0);
		lcd_brightness = tune_level;
		pr_info("level = %d pulling low\n",level);
		return;
	}

	if (unlikely(lcd_brightness < 0)) {
		int val = gpio_get_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1]);
		if (val) {
			lcd_brightness = 0;
			gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1], 0);
			mdelay(3);
			pr_info("LCD Baklight init in boot time on kernel\n");
		}
	}
	if (!lcd_brightness) {
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1], 1);
		udelay(3);
		lcd_brightness = 1;
		pr_info("level = %d !lcd_brightness\n",level);
	}

	pulse = (tune_level - lcd_brightness + MAX_BRIGHTNESS_IN_BLU)
					% MAX_BRIGHTNESS_IN_BLU;

	pr_info("lcd_brightness = %d, tune_level = %d,  pulse = %d\n", lcd_brightness,tune_level,pulse);

	spin_lock_irqsave(&bg_gpio_lock, irq_flags);
	for (; pulse > 0; pulse--) {

		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1], 0);
		udelay(3);
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1], 1);
		udelay(3);
	}
	spin_unlock_irqrestore(&bg_gpio_lock, irq_flags);

	lcd_brightness = tune_level;

	mdelay(1);

}
static int mdss_panel_on_pre(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	mdss_panel_attach_set(ctrl, true);

	return true;
}
static void backlight_tft_late_on(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	if (!mdss_panel_attach_get(ctrl)) {
		pr_err("%s: mdss_panel_attach_get(%d) : %d\n",__func__, ctrl->ndx, mdss_panel_attach_get(ctrl));
		return;
	}

	if (gpio_is_valid(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0])) {
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 1);
		msleep(80);
	}

	if(ctrl->bklt_ctrl == BL_SS_PWM && is_first_boot){
		if (vdd->panel_func.samsung_brightness_tft_pwm)
			vdd->panel_func.samsung_brightness_tft_pwm(ctrl,LCD_DEFAUL_BL_LEVEL);
		is_first_boot = 0;
	}

	pr_info("%s : Backlight is on\n", __func__);

	if(vdd->support_cabc)
		mdss_samsung_cabc_update();

}
static int mdss_panel_on_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	pr_info("%s %d\n", __func__, ctrl->ndx);

	if(ctrl->bklt_ctrl == BL_DCS_CMD)
		ktd3102_set_led_current(5,ctrl);	/* set max current to "6"/19.8mA, only when CABC is enabled */

	if(vdd->support_cabc)
		mdss_samsung_cabc_update();

	if(ctrl->bklt_ctrl == BL_DCS_CMD && is_first_boot){
		mdss_samsung_brightness_dcs(ctrl, LCD_DEFAUL_BL_LEVEL);
		is_first_boot = 0;
	}


	return true;
}
static int mdss_panel_off_post(struct mdss_dsi_ctrl_pdata *ctrl)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);

	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return false;
	}

	if(is_first_boot) {
		mdss_backlight_tft_request_gpios(ctrl);
		if (ctrl->bklt_ctrl == BL_DCS_CMD && vdd->panel_func.samsung_brightness_tft_pwm)
			vdd->panel_func.samsung_brightness_tft_pwm(ctrl, 0);
	}

	if (gpio_is_valid(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0]))
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[0], 0);
	if (gpio_is_valid(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1]))
		gpio_set_value(vdd->dtsi_data[ctrl->ndx].backlight_tft_gpio[1], 0);

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
static void mdss_panel_tft_pwm_control(struct mdss_dsi_ctrl_pdata *ctrl, int level)
{
	struct samsung_display_driver_data *vdd = check_valid_ctrl(ctrl);
	if (IS_ERR_OR_NULL(vdd)) {
		pr_err("%s: Invalid data ctrl : 0x%zx vdd : 0x%zx", __func__, (size_t)ctrl, (size_t)vdd);
		return;
	}

	vdd->bl_level = level;
	vdd->scaled_level = get_scaled_level(vdd, ctrl->ndx);
	ktd3102_set_brightness(vdd->scaled_level, ctrl);

	pr_info("%s bl_level : %d scaled_level : %d\n", __func__, level, vdd->scaled_level);

	return;
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
	mdnie_data.DSI0_GRAYSCALE_MDNIE = DSI0_NEGATIVE_EXIT_MDNIE;
	mdnie_data.DSI0_GRAYSCALE_NEGATIVE_MDNIE = DSI0_NEGATIVE_MDNIE;
	mdnie_data.DSI0_COLOR_BLIND_MDNIE = NULL;
	mdnie_data.DSI0_HBM_CE_MDNIE = NULL;
	mdnie_data.DSI0_RGB_SENSOR_MDNIE = NULL;
	mdnie_data.DSI0_CURTAIN = NULL;
	mdnie_data.DSI0_UI_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_UI_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_UI_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_UI_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_UI_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_WARM_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_WARM_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_COLD_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_VIDEO_COLD_MDNIE = NULL;
	mdnie_data.DSI0_CAMERA_OUTDOOR_MDNIE = NULL;
	mdnie_data.DSI0_CAMERA_MDNIE = NULL;
	mdnie_data.DSI0_CAMERA_AUTO_MDNIE = NULL;
	mdnie_data.DSI0_GALLERY_DYNAMIC_MDNIE = NULL;
	mdnie_data.DSI0_GALLERY_STANDARD_MDNIE = NULL;
	mdnie_data.DSI0_GALLERY_NATURAL_MDNIE = NULL;
	mdnie_data.DSI0_GALLERY_MOVIE_MDNIE = NULL;
	mdnie_data.DSI0_GALLERY_AUTO_MDNIE = NULL;
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
	mdnie_data.dsi0_bypass_mdnie_size = ARRAY_SIZE(DSI0_NEGATIVE_MDNIE);
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

	vdd->support_panel_max = S6D78A0_GH9607501A_SUPPORT_PANEL_COUNT;
	vdd->support_mdnie_lite = true;
	vdd->mdnie_tune_size1 = 2;
	vdd->mdnie_tune_size2 = 0;
	vdd->manufacture_id_dsi[vdd->support_panel_max - 1] = get_lcd_attached("GET");

	if (vdd->ctrl_dsi[DISPLAY_1]->bklt_ctrl == BL_DCS_CMD)
		vdd->support_cabc = true;
	/* ON/OFF */
	vdd->panel_func.samsung_panel_on_pre = mdss_panel_on_pre;
	vdd->panel_func.samsung_panel_on_post = mdss_panel_on_post;
	vdd->panel_func.samsung_panel_off_pre = NULL;
	vdd->panel_func.samsung_panel_off_post = mdss_panel_off_post;
	vdd->panel_func.samsung_backlight_late_on = backlight_tft_late_on;

	/* DDI RX */
	vdd->panel_func.samsung_panel_revision = mdss_panel_revision;
	vdd->panel_func.samsung_manufacture_date_read = NULL;
	vdd->panel_func.samsung_ddi_id_read = NULL;
	vdd->panel_func.samsung_hbm_read = NULL;
	vdd->panel_func.samsung_mdnie_read = NULL;
	vdd->panel_func.samsung_smart_dimming_init = NULL;

	/* Brightness */
	vdd->panel_func.samsung_brightness_tft_pwm_ldi = mdss_brightness_tft_pwm;
	vdd->panel_func.samsung_brightness_tft_pwm = mdss_panel_tft_pwm_control;
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
	char panel_string[] = "ss_dsi_panel_S6D78A0_GH9607501A_QHD";

	vdd->panel_name = mdss_mdp_panel + 8;
	pr_info("%s : %s\n", __func__, vdd->panel_name);

	if (!strncmp(vdd->panel_name, panel_string, strlen(panel_string)))
		vdd->panel_func.samsung_panel_init = mdss_panel_init;

	return 0;
}
early_initcall(samsung_panel_init);
