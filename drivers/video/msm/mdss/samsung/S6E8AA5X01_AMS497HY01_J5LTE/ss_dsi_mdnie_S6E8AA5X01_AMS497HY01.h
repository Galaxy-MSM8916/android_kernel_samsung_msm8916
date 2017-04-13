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
#ifndef _SAMSUNG_DSI_MDNIE_S6E8AA5X01_AMS497HY01_H_
#define _SAMSUNG_DSI_MDNIE_S6E8AA5X01_AMS497HY01_H_

#include "../ss_dsi_mdnie_lite_common.h"

#define MDNIE_COLOR_BLINDE_CMD_OFFSET 18

#define ADDRESS_SCR_WHITE_RED   0x7D
#define ADDRESS_SCR_WHITE_GREEN 0x7F
#define ADDRESS_SCR_WHITE_BLUE  0x81

#define MDNIE_STEP1_INDEX 2
#define MDNIE_STEP2_INDEX 3

static char level_1_key_on[] = {
	0xF0,
	0x5A, 0x5A
};
static char level_2_key_on[] = {
	0xF0,
	0x5A, 0x5A
};

static char level_1_key_off[] = {
	0xF0,
	0xA5, 0xA5
};
static char level_2_key_off[] = {
	0xFC,
	0xA5, 0xA5
};

static char DSI0_BYPASS_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_BYPASS_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_NEGATIVE_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_NEGATIVE_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0xff, // ascr_Cr 
	0x00, // ascr_Rr 
	0x00, // ascr_Cg 
	0xff, // ascr_Rg 
	0x00, // ascr_Cb 
	0xff, // ascr_Rb 
	0x00, // ascr_Mr 
	0xff, // ascr_Gr 
	0xff, // ascr_Mg 
	0x00, // ascr_Gg 
	0x00, // ascr_Mb 
	0xff, // ascr_Gb 
	0x00, // ascr_Yr 
	0xff, // ascr_Br 
	0x00, // ascr_Yg 
	0xff, // ascr_Bg 
	0xff, // ascr_Yb 
	0x00, // ascr_Bb 
	0x00, // ascr_Wr 
	0xff, // ascr_Kr 
	0x00, // ascr_Wg 
	0xff, // ascr_Kg 
	0x00, // ascr_Wb 
	0xff, // ascr_Kb 
	//end
};

static char DSI0_COLOR_BLIND_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_COLOR_BLIND_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_RGB_SENSOR_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_RGB_SENSOR_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_UI_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_UI_DYNAMIC_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_UI_STANDARD_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_UI_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_UI_NATURAL_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_UI_NATURAL_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_UI_MOVIE_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_UI_MOVIE_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_UI_AUTO_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_UI_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VIDEO_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_STANDARD_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VIDEO_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_NATURAL_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_NATURAL_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_MOVIE_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_MOVIE_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_AUTO_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VIDEO_AUTO_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_WARM_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_WARM_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_WARM_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_WARM_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_COLD_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_COLD_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VIDEO_COLD_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VIDEO_COLD_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_CAMERA_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_CAMERA_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_CAMERA_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_CAMERA_MDNIE_2[] ={
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_CAMERA_AUTO_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_CAMERA_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_GALLERY_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_GALLERY_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x00, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_GALLERY_STANDARD_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_GALLERY_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_GALLERY_NATURAL_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_GALLERY_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_GALLERY_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_GALLERY_MOVIE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_GALLERY_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_GALLERY_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VT_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VT_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VT_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VT_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VT_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VT_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VT_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_VT_MOVIE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_VT_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_VT_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_BROWSER_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_BROWSER_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_BROWSER_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_BROWSER_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_BROWSER_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_BROWSER_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_BROWSER_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_BROWSER_MOVIE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_BROWSER_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_BROWSER_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EBOOK_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_EBOOK_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EBOOK_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_EBOOK_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EBOOK_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_EBOOK_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EBOOK_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_EBOOK_MOVIE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EBOOK_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_EBOOK_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_EMAIL_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};
static char DSI0_EMAIL_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_HBM_CE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_HBM_CE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_HBM_CE_TEXT_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_HBM_CE_TEXT_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0xff, // ascr_Rr 
	0xff, // ascr_Cg 
	0x00, // ascr_Rg 
	0xff, // ascr_Cb 
	0x00, // ascr_Rb 
	0xff, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0xff, // ascr_Gg 
	0xff, // ascr_Mb 
	0x00, // ascr_Gb 
	0xff, // ascr_Yr 
	0x00, // ascr_Br 
	0xff, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0xff, // ascr_Bb 
	0xff, // ascr_Wr 
	0x00, // ascr_Kr 
	0xff, // ascr_Wg 
	0x00, // ascr_Kg 
	0xff, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static char DSI0_CURTAIN_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //data_width mask 00 000
};

static char DSI0_CURTAIN_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, // nr de cs cc 0000
	0xff, // nr_mask_th
	0x00, // de_gain 10
	0x00,
	0x07, // de_maxplus 11 
	0xff,
	0x07, // de_maxminus 11 
	0xff,
	0x01, // CS Gain 10
	0x83,
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x00, //curve 5 b
	0x20, //curve 5 a
	0x00, //curve 6 b
	0x20, //curve 6 a
	0x00, //curve 7 b
	0x20, //curve 7 a
	0x00, //curve 8 b
	0x20, //curve 8 a
	0x00, //curve 9 b
	0x20, //curve 9 a
	0x00, //curve10 b
	0x20, //curve10 a
	0x00, //curve11 b
	0x20, //curve11 a
	0x00, //curve12 b
	0x20, //curve12 a
	0x00, //curve13 b
	0x20, //curve13 a
	0x00, //curve14 b
	0x20, //curve14 a
	0x00, //curve15 b
	0x20, //curve15 a
	0x00, //curve16 b
	0x20, //curve16 a
	0x00, //curve17 b
	0x20, //curve17 a
	0x00, //curve18 b
	0x20, //curve18 a
	0x00, //curve19 b
	0x20, //curve19 a
	0x00, //curve20 b
	0x20, //curve20 a
	0x00, //curve21 b
	0x20, //curve21 a
	0x00, //curve22 b
	0x20, //curve22 a
	0x00, //curve23 b
	0x20, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x30, // ascr_skin_on strength 0 00000
	0x67, // ascr_skin_cb 
	0xa9, // ascr_skin_cr 
	0x0c, // ascr_dist_up
	0x0c, // ascr_dist_down
	0x0c, // ascr_dist_right
	0x0c, // ascr_dist_left
	0x00, // ascr_div_up 20
	0xaa,
	0xab,
	0x00, // ascr_div_down 
	0xaa,
	0xab,
	0x00, // ascr_div_right
	0xaa,
	0xab,
	0x00, // ascr_div_left
	0xaa,
	0xab,
	0xd5, // ascr_skin_Rr 
	0x2c, // ascr_skin_Rg 
	0x2a, // ascr_skin_Rb 
	0xff, // ascr_skin_Yr 
	0xf5, // ascr_skin_Yg 
	0x63, // ascr_skin_Yb 
	0xfe, // ascr_skin_Mr 
	0x4a, // ascr_skin_Mg 
	0xff, // ascr_skin_Mb 
	0xff, // ascr_skin_Wr 
	0xf9, // ascr_skin_Wg 
	0xf8, // ascr_skin_Wb 
	0x00, // ascr_Cr 
	0x00, // ascr_Rr 
	0x00, // ascr_Cg 
	0x00, // ascr_Rg 
	0x00, // ascr_Cb 
	0x00, // ascr_Rb 
	0x00, // ascr_Mr 
	0x00, // ascr_Gr 
	0x00, // ascr_Mg 
	0x00, // ascr_Gg 
	0x00, // ascr_Mb 
	0x00, // ascr_Gb 
	0x00, // ascr_Yr 
	0x00, // ascr_Br 
	0x00, // ascr_Yg 
	0x00, // ascr_Bg 
	0x00, // ascr_Yb 
	0x00, // ascr_Bb 
	0x00, // ascr_Wr 
	0x00, // ascr_Kr 
	0x00, // ascr_Wg 
	0x00, // ascr_Kg 
	0x00, // ascr_Wb 
	0x00, // ascr_Kb 
	//end
};

static struct dsi_cmd_desc DSI0_BYPASS_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_1)}, DSI0_BYPASS_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_2)}, DSI0_BYPASS_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_1)}, DSI0_NEGATIVE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_2)}, DSI0_NEGATIVE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_COLOR_BLIND_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_BLIND_MDNIE_1)}, DSI0_COLOR_BLIND_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_BLIND_MDNIE_2)}, DSI0_COLOR_BLIND_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_HBM_CE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_MDNIE_1)}, DSI0_HBM_CE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_MDNIE_2)}, DSI0_HBM_CE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_HBM_CE_TEXT_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_TEXT_MDNIE_1)}, DSI0_HBM_CE_TEXT_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_TEXT_MDNIE_2)}, DSI0_HBM_CE_TEXT_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_RGB_SENSOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_RGB_SENSOR_MDNIE_1)}, DSI0_RGB_SENSOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_RGB_SENSOR_MDNIE_2)}, DSI0_RGB_SENSOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_CURTAIN[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CURTAIN_1)}, DSI0_CURTAIN_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CURTAIN_2)}, DSI0_CURTAIN_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

///////////////////////////////////////////////////////////////////////////////////

static struct dsi_cmd_desc DSI0_UI_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_DYNAMIC_MDNIE_1)}, DSI0_UI_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_DYNAMIC_MDNIE_2)}, DSI0_UI_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_UI_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_STANDARD_MDNIE_1)}, DSI0_UI_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_STANDARD_MDNIE_2)}, DSI0_UI_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_UI_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_NATURAL_MDNIE_1)}, DSI0_UI_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_NATURAL_MDNIE_2)}, DSI0_UI_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_UI_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MOVIE_MDNIE_1)}, DSI0_UI_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MOVIE_MDNIE_2)}, DSI0_UI_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_UI_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_AUTO_MDNIE_1)}, DSI0_UI_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_AUTO_MDNIE_2)}, DSI0_UI_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_OUTDOOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_OUTDOOR_MDNIE_1)}, DSI0_VIDEO_OUTDOOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_OUTDOOR_MDNIE_2)}, DSI0_VIDEO_OUTDOOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_DYNAMIC_MDNIE_1)}, DSI0_VIDEO_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_DYNAMIC_MDNIE_2)}, DSI0_VIDEO_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_STANDARD_MDNIE_1)}, DSI0_VIDEO_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_STANDARD_MDNIE_2)}, DSI0_VIDEO_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_NATURAL_MDNIE_1)}, DSI0_VIDEO_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_NATURAL_MDNIE_2)}, DSI0_VIDEO_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MOVIE_MDNIE_1)}, DSI0_VIDEO_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MOVIE_MDNIE_2)}, DSI0_VIDEO_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_AUTO_MDNIE_1)}, DSI0_VIDEO_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_AUTO_MDNIE_2)}, DSI0_VIDEO_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_WARM_OUTDOOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_WARM_OUTDOOR_MDNIE_1)}, DSI0_VIDEO_WARM_OUTDOOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_WARM_OUTDOOR_MDNIE_2)}, DSI0_VIDEO_WARM_OUTDOOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_WARM_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_WARM_MDNIE_1)}, DSI0_VIDEO_WARM_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_WARM_MDNIE_2)}, DSI0_VIDEO_WARM_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_COLD_OUTDOOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_COLD_OUTDOOR_MDNIE_1)}, DSI0_VIDEO_COLD_OUTDOOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_COLD_OUTDOOR_MDNIE_2)}, DSI0_VIDEO_COLD_OUTDOOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_COLD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_COLD_MDNIE_1)}, DSI0_VIDEO_COLD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_COLD_MDNIE_2)}, DSI0_VIDEO_COLD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_OUTDOOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_OUTDOOR_MDNIE_1)}, DSI0_CAMERA_OUTDOOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_OUTDOOR_MDNIE_2)}, DSI0_CAMERA_OUTDOOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_1)}, DSI0_CAMERA_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_2)}, DSI0_CAMERA_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_AUTO_MDNIE_1)}, DSI0_CAMERA_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_AUTO_MDNIE_2)}, DSI0_CAMERA_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_DYNAMIC_MDNIE_1)}, DSI0_GALLERY_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_DYNAMIC_MDNIE_2)}, DSI0_GALLERY_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_STANDARD_MDNIE_1)}, DSI0_GALLERY_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_STANDARD_MDNIE_2)}, DSI0_GALLERY_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_NATURAL_MDNIE_1)}, DSI0_GALLERY_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_NATURAL_MDNIE_2)}, DSI0_GALLERY_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MOVIE_MDNIE_1)}, DSI0_GALLERY_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MOVIE_MDNIE_2)}, DSI0_GALLERY_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_AUTO_MDNIE_1)}, DSI0_GALLERY_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_AUTO_MDNIE_2)}, DSI0_GALLERY_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VT_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_DYNAMIC_MDNIE_1)}, DSI0_VT_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_DYNAMIC_MDNIE_2)}, DSI0_VT_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VT_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_STANDARD_MDNIE_1)}, DSI0_VT_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_STANDARD_MDNIE_2)}, DSI0_VT_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VT_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_NATURAL_MDNIE_1)}, DSI0_VT_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_NATURAL_MDNIE_2)}, DSI0_VT_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VT_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MOVIE_MDNIE_1)}, DSI0_VT_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MOVIE_MDNIE_2)}, DSI0_VT_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_VT_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_AUTO_MDNIE_1)}, DSI0_VT_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_AUTO_MDNIE_2)}, DSI0_VT_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_DYNAMIC_MDNIE_1)}, DSI0_BROWSER_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_DYNAMIC_MDNIE_2)}, DSI0_BROWSER_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_STANDARD_MDNIE_1)}, DSI0_BROWSER_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_STANDARD_MDNIE_2)}, DSI0_BROWSER_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_NATURAL_MDNIE_1)}, DSI0_BROWSER_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_NATURAL_MDNIE_2)}, DSI0_BROWSER_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MOVIE_MDNIE_1)}, DSI0_BROWSER_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MOVIE_MDNIE_2)}, DSI0_BROWSER_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_AUTO_MDNIE_1)}, DSI0_BROWSER_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_AUTO_MDNIE_2)}, DSI0_BROWSER_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_DYNAMIC_MDNIE_1)}, DSI0_EBOOK_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_DYNAMIC_MDNIE_2)}, DSI0_EBOOK_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_STANDARD_MDNIE_1)}, DSI0_EBOOK_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_STANDARD_MDNIE_2)}, DSI0_EBOOK_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_NATURAL_MDNIE_1)}, DSI0_EBOOK_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_NATURAL_MDNIE_2)}, DSI0_EBOOK_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MOVIE_MDNIE_1)}, DSI0_EBOOK_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MOVIE_MDNIE_2)}, DSI0_EBOOK_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_AUTO_MDNIE_1)}, DSI0_EBOOK_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_AUTO_MDNIE_2)}, DSI0_EBOOK_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_EMAIL_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_AUTO_MDNIE_1)}, DSI0_EMAIL_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_AUTO_MDNIE_2)}, DSI0_EMAIL_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

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
			{DSI0_UI_DYNAMIC_MDNIE,	NULL},
			{DSI0_UI_STANDARD_MDNIE,	NULL},
			{DSI0_UI_NATURAL_MDNIE,	NULL},
			{DSI0_UI_MOVIE_MDNIE,	NULL},
			{DSI0_UI_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// VIDEO_APP
		{
			{DSI0_VIDEO_DYNAMIC_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_STANDARD_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_NATURAL_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_MOVIE_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_AUTO_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_EBOOK_AUTO_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
		},
		// VIDEO_WARM_APP
		{
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_EBOOK_AUTO_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
		},
		// VIDEO_COLD_APP
		{
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_EBOOK_AUTO_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
		},
		// CAMERA_APP
		{
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_AUTO_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_EBOOK_AUTO_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
		},
		// NAVI_APP
		{
			{NULL,	NULL},
			{NULL,	NULL},
			{NULL,	NULL},
			{NULL,	NULL},
			{NULL,	NULL},
			{NULL,	NULL},
		},
		// GALLERY_APP
		{
			{DSI0_GALLERY_DYNAMIC_MDNIE,	NULL},
			{DSI0_GALLERY_STANDARD_MDNIE,	NULL},
			{DSI0_GALLERY_NATURAL_MDNIE,	NULL},
			{DSI0_GALLERY_MOVIE_MDNIE,	NULL},
			{DSI0_GALLERY_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// VT_APP
		{
			{DSI0_VT_DYNAMIC_MDNIE,	NULL},
			{DSI0_VT_STANDARD_MDNIE,	NULL},
			{DSI0_VT_NATURAL_MDNIE,	NULL},
			{DSI0_VT_MOVIE_MDNIE,	NULL},
			{DSI0_VT_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// BROWSER_APP
		{
			{DSI0_BROWSER_DYNAMIC_MDNIE,	NULL},
			{DSI0_BROWSER_STANDARD_MDNIE,	NULL},
			{DSI0_BROWSER_NATURAL_MDNIE,	NULL},
			{DSI0_BROWSER_MOVIE_MDNIE,	NULL},
			{DSI0_BROWSER_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// eBOOK_APP
		{
			{DSI0_EBOOK_DYNAMIC_MDNIE,	NULL},
			{DSI0_EBOOK_STANDARD_MDNIE,NULL},
			{DSI0_EBOOK_NATURAL_MDNIE,	NULL},
			{DSI0_EBOOK_MOVIE_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// EMAIL_APP
		{
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// TDMB_APP
		{
			{DSI0_UI_DYNAMIC_MDNIE,	NULL},
			{DSI0_UI_STANDARD_MDNIE,	NULL},
			{DSI0_UI_NATURAL_MDNIE,	NULL},
			{DSI0_UI_MOVIE_MDNIE,	NULL},
			{DSI0_UI_AUTO_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
};

#define DSI0_RGB_SENSOR_MDNIE_1_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_1)
#define DSI0_RGB_SENSOR_MDNIE_2_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_2)
#endif /*_DSI_TCON_MDNIE_LITE_DATA_FHD_S6E3FA2_H_*/
