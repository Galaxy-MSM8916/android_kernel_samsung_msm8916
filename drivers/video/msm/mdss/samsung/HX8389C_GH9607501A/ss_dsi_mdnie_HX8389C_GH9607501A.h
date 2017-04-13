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
#ifndef _SS_DSI_MDNIE_HX8389C_GH9607501A_H_
#define _SS_DSI_MDNIE_HX8389C_GH9607501A_H_

#include "../ss_dsi_mdnie_lite_common.h"

#define MDNIE_COLOR_BLINDE_CMD_OFFSET 104

#define MDNIE_STEP1_INDEX 0
#define MDNIE_STEP2_INDEX 0

static char DSI0_NEGATIVE_EXIT[] ={
	0x20,
	0x00,
};

static char DSI0_NEGATIVE_ENTER[] ={
	0x21,
	0x00,
};

static struct dsi_cmd_desc DSI0_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_NEGATIVE_ENTER)}, DSI0_NEGATIVE_ENTER},
};

static struct dsi_cmd_desc DSI0_NEGATIVE_EXIT_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_NEGATIVE_EXIT)}, DSI0_NEGATIVE_EXIT},
};

///////////////////////////////////////////////////////////////////////////////////
static struct dsi_cmd_desc *mdnie_tune_value_dsi0[MAX_APP_MODE][MAX_MODE][MAX_OUTDOOR_MODE] = {
		/*
			DYNAMIC_MODE
			STANDARD_MODE
			NATURAL_MODE
			MOVIE_MODE
			AUTO_MODE
			READING_MODE
		*/
		// UI_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// VIDEO_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// CAMERA_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// NAVI_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// GALLERY_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// VT_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// BROWSER_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// eBOOK_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
		// EMAIL_APP
		{
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
			{DSI0_NEGATIVE_EXIT_MDNIE,NULL},
		},
};

#endif /*_DSI_MDNIE_LITE_DATA_WVGA_HX8369B_H_*/
