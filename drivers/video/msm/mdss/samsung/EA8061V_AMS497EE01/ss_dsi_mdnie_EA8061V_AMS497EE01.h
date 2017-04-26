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
#ifndef _SAMSUNG_DSI_MDNIE_EA8061V_AMS497EE01_H_
#define _SAMSUNG_DSI_MDNIE_EA8061V_AMS497EE01_H_

#include "../ss_dsi_mdnie_lite_common.h"

#define MDNIE_COLOR_BLINDE_CMD_OFFSET 18

#define ADDRESS_SCR_WHITE_RED   0x24
#define ADDRESS_SCR_WHITE_GREEN 0x26
#define ADDRESS_SCR_WHITE_BLUE  0x28

#define MDNIE_STEP1_INDEX 2
#define MDNIE_STEP2_INDEX 3

static char level_1_key_on[] = {
	0xF0,
	0x5A, 0x5A
};

static char level_1_key_off[] = {
	0xF0,
	0xA5, 0xA5
};

static char level_2_key_on[] = {
	0xF1,
	0x5A, 0x5A
};

static char level_2_key_off[] = {
	0xF1,
	0xA5, 0xA5
};

static char DSI0_GRAYSCALE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_GRAYSCALE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0xb3, //scr Cr Yb
	0x4c, //scr Rr Bb
	0xb3, //scr Cg Yg
	0x4c, //scr Rg Bg
	0xb3, //scr Cb Yr
	0x4c, //scr Rb Br
	0x69, //scr Mr Mb
	0x96, //scr Gr Gb
	0x69, //scr Mg Mg
	0x96, //scr Gg Gg
	0x69, //scr Mb Mr
	0x96, //scr Gb Gr
	0xe2, //scr Yr Cb
	0x1d, //scr Br Rb
	0xe2, //scr Yg Cg
	0x1d, //scr Bg Rg
	0xe2, //scr Yb Cr
	0x1d, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x4c, //scr Cr Yb
	0xb3, //scr Rr Bb
	0x4c, //scr Cg Yg
	0xb3, //scr Rg Bg
	0x4c, //scr Cb Yr
	0xb3, //scr Rb Br
	0x96, //scr Mr Mb
	0x69, //scr Gr Gb
	0x96, //scr Mg Mg
	0x69, //scr Gg Gg
	0x96, //scr Mb Mr
	0x69, //scr Gb Gr
	0x1d, //scr Yr Cb
	0xe2, //scr Br Rb
	0x1d, //scr Yg Cg
	0xe2, //scr Bg Rg
	0x1d, //scr Yb Cr
	0xe2, //scr Bb Rr
	0x00, //scr Wr Wb
	0xff, //scr Kr Kb
	0x00, //scr Wg Wg
	0xff, //scr Kg Kg
	0x00, //scr Wb Wr
	0xff, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// BYPASS ///////////////////////

static char DSI0_BYPASS_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x00, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_BYPASS_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

/////////////////// SENSOR /////////////////////


static char DSI0_RGB_SENSOR_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_RGB_SENSOR_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0x00, //scr Rr Bb
	0x00, //scr Cg Yg
	0x00, //scr Rg Bg
	0x00, //scr Cb Yr
	0x00, //scr Rb Br
	0x00, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0x00, //scr Gg Gg
	0x00, //scr Mb Mr
	0x00, //scr Gb Gr
	0x00, //scr Yr Cb
	0x00, //scr Br Rb
	0x00, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0x00, //scr Bb Rr
	0x00, //scr Wr Wb
	0x00, //scr Kr Kb
	0x00, //scr Wg Wg
	0x00, //scr Kg Kg
	0x00, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x00, //curve 1 a
	0x00, //curve 2 b
	0x00, //curve 2 a
	0x00, //curve 3 b
	0x00, //curve 3 a
	0x00, //curve 4 b
	0x00, //curve 4 a
	0x00, //curve 5 b
	0x00, //curve 5 a
	0x00, //curve 6 b
	0x00, //curve 6 a
	0x00, //curve 7 b
	0x00, //curve 7 a
	0x00, //curve 8 b
	0x00, //curve 8 a
	0x00, //curve 9 b
	0x00, //curve 9 a
	0x00, //curve10 b
	0x00, //curve10 a
	0x00, //curve11 b
	0x00, //curve11 a
	0x00, //curve12 b
	0x00, //curve12 a
	0x00, //curve13 b
	0x00, //curve13 a
	0x00, //curve14 b
	0x00, //curve14 a
	0x00, //curve15 b
	0x00, //curve15 a
	0x00, //curve16 b
	0x00, //curve16 a
	0x00, //curve17 b
	0x00, //curve17 a
	0x00, //curve18 b
	0x00, //curve18 a
	0x00, //curve19 b
	0x00, //curve19 a
	0x00, //curve20 b
	0x00, //curve20 a
	0x00, //curve21 b
	0x00, //curve21 a
	0x00, //curve22 b
	0x00, //curve22 a
	0x00, //curve23 b
	0x00, //curve23 a
	0x00, //curve24 b
	0x00, //curve24 a
	0x00, //cs gain
	0x00,
	//end
};
////////////////// SCREEN CURTAIN//////////////////

static char DSI0_CURTAIN_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_CURTAIN_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0x00, //scr Rr Bb
	0x00, //scr Cg Yg
	0x00, //scr Rg Bg
	0x00, //scr Cb Yr
	0x00, //scr Rb Br
	0x00, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0x00, //scr Gg Gg
	0x00, //scr Mb Mr
	0x00, //scr Gb Gr
	0x00, //scr Yr Cb
	0x00, //scr Br Rb
	0x00, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0x00, //scr Bb Rr
	0x00, //scr Wr Wb
	0x00, //scr Kr Kb
	0x00, //scr Wg Wg
	0x00, //scr Kg Kg
	0x00, //scr Wb Wr
	0x00, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// UI /// /////////////////////

static char DSI0_UI_STANDARD_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_UI_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_UI_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_UI_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_UI_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_UI_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_UI_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
};

static char DSI0_UI_MOVIE_MDNIE_2[] = {
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_UI_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_UI_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// GALLERY /////////////////////
static char DSI0_GALLERY_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_GALLERY_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_GALLERY_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_GALLERY_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_GALLERY_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_GALLERY_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_GALLERY_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_GALLERY_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_GALLERY_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// VIDEO /////////////////////

static char DSI0_VIDEO_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_VIDEO_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_VIDEO_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_VIDEO_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_VIDEO_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_VIDEO_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_VIDEO_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
};

static char DSI0_VIDEO_MOVIE_MDNIE_2[] = {
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_VIDEO_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_VIDEO_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

////////////////// VT /////////////////////

static char DSI0_VT_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_VT_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_VT_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_VT_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_VT_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_VT_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_VT_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_VT_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_VT_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// CAMERA /////////////////////

static char DSI0_CAMERA_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_CAMERA_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_CAMERA_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_CAMERA_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_NEGATIVE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_NEGATIVE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0xff, //scr Cr Yb
	0x00, //scr Rr Bb
	0x00, //scr Cg Yg
	0xff, //scr Rg Bg
	0x00, //scr Cb Yr
	0xff, //scr Rb Br
	0x00, //scr Mr Mb
	0xff, //scr Gr Gb
	0xff, //scr Mg Mg
	0x00, //scr Gg Gg
	0x00, //scr Mb Mr
	0xff, //scr Gb Gr
	0x00, //scr Yr Cb
	0xff, //scr Br Rb
	0x00, //scr Yg Cg
	0xff, //scr Bg Rg
	0xff, //scr Yb Cr
	0x00, //scr Bb Rr
	0x00, //scr Wr Wb
	0xff, //scr Kr Kb
	0x00, //scr Wg Wg
	0xff, //scr Kg Kg
	0x00, //scr Wb Wr
	0xff, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_COLOR_BLIND_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_COLOR_BLIND_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// BROWSER /////////////////////

static char DSI0_BROWSER_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_BROWSER_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_BROWSER_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_BROWSER_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_BROWSER_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_BROWSER_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_BROWSER_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_BROWSER_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_BROWSER_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// eBOOK /////////////////////

static char DSI0_EBOOK_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_EBOOK_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf3, //scr Wg Wg
	0x00, //scr Kg Kg
	0xe4, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_EBOOK_STANDARD_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_EBOOK_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_EBOOK_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_EBOOK_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_EBOOK_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_EBOOK_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_EBOOK_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
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
	0x87, //scr Cr Yb
	0xe7, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1e, //scr Rg Bg
	0xf2, //scr Cb Yr
	0x1e, //scr Rb Br
	0xf5, //scr Mr Mb
	0x73, //scr Gr Gb
	0x2a, //scr Mg Mg
	0xf7, //scr Gg Gg
	0xf0, //scr Mb Mr
	0x37, //scr Gb Gr
	0xfa, //scr Yr Cb
	0x2e, //scr Br Rb
	0xf5, //scr Yg Cg
	0x14, //scr Bg Rg
	0x3c, //scr Yb Cr
	0xed, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf6, //scr Wg Wg
	0x00, //scr Kg Kg
	0xee, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve_1_b
	0x20, //curve_1_a
	0x00, //curve_2_b
	0x20, //curve_2_a
	0x00, //curve_3_b
	0x20, //curve_3_a
	0x00, //curve_4_b
	0x20, //curve_4_a
	0x02, //curve_5_b
	0x1b, //curve_5_a
	0x02, //curve_6_b
	0x1b, //curve_6_a
	0x02, //curve_7_b
	0x1b, //curve_7_a
	0x02, //curve_8_b
	0x1b, //curve_8_a
	0x09, //curve_9_b
	0xa6, //curve_9_a
	0x09, //curve10_b
	0xa6, //curve10_a
	0x09, //curve11_b
	0xa6, //curve11_a
	0x09, //curve12_b
	0xa6, //curve12_a
	0x00, //curve13_b
	0x20, //curve13_a
	0x00, //curve14_b
	0x20, //curve14_a
	0x00, //curve15_b
	0x20, //curve15_a
	0x00, //curve16_b
	0x20, //curve16_a
	0x00, //curve17_b
	0x20, //curve17_a
	0x00, //curve18_b
	0x20, //curve18_a
	0x00, //curve19_b
	0x20, //curve19_a
	0x00, //curve20_b
	0x20, //curve20_a
	0x00, //curve21_b
	0x20, //curve21_a
	0x00, //curve22_b
	0x20, //curve22_a
	0x00, //curve23_b
	0x20, //curve23_a
	0x00, //curve24_b
	0xFF, //curve24_a
	0x01, //cs gain
	0x20,
};

static char DSI0_EMAIL_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_EMAIL_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x00, //scr Rg Bg
	0xff, //scr Cb Yr
	0x00, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xec, //scr Wb Wr
	0x00, //scr Kb Kr
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
	0x01, //cs gain
	0x00,
	//end
};

////////////////// TDMB /// /////////////////////

static char DSI0_TDMB_STANDARD_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_TDMB_STANDARD_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xfb, //scr Rr Bb
	0xef, //scr Cg Yg
	0x10, //scr Rg Bg
	0xe4, //scr Cb Yr
	0x10, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x20, //scr Mg Mg
	0xe2, //scr Gg Gg
	0xec, //scr Mb Mr
	0x00, //scr Gb Gr
	0xed, //scr Yr Cb
	0x1c, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1a, //scr Bg Rg
	0x2a, //scr Yb Cr
	0xf4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

#if defined(NATURAL_MODE_ENABLE)
static char DSI0_TDMB_NATURAL_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_TDMB_NATURAL_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x8c, //scr Cr Yb
	0xd5, //scr Rr Bb
	0xf2, //scr Cg Yg
	0x29, //scr Rg Bg
	0xe6, //scr Cb Yr
	0x20, //scr Rb Br
	0xdd, //scr Mr Mb
	0x68, //scr Gr Gb
	0x3a, //scr Mg Mg
	0xed, //scr Gg Gg
	0xed, //scr Mb Mr
	0x38, //scr Gb Gr
	0xef, //scr Yr Cb
	0x2a, //scr Br Rb
	0xf1, //scr Yg Cg
	0x1f, //scr Bg Rg
	0x56, //scr Yb Cr
	0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
	0xed, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
#endif

static char DSI0_TDMB_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x07, //sharpen cc gamma 00 0 0
};

static char DSI0_TDMB_DYNAMIC_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x20, //scr Rg Bg
	0xff, //scr Cb Yr
	0x20, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x14, //curve 1 a
	0x00, //curve 2 b
	0x14, //curve 2 a
	0x00, //curve 3 b
	0x14, //curve 3 a
	0x00, //curve 4 b
	0x14, //curve 4 a
	0x03, //curve 5 b
	0x9a, //curve 5 a
	0x03, //curve 6 b
	0x9a, //curve 6 a
	0x03, //curve 7 b
	0x9a, //curve 7 a
	0x03, //curve 8 b
	0x9a, //curve 8 a
	0x07, //curve 9 b
	0x9e, //curve 9 a
	0x07, //curve10 b
	0x9e, //curve10 a
	0x07, //curve11 b
	0x9e, //curve11 a
	0x07, //curve12 b
	0x9e, //curve12 a
	0x0a, //curve13 b
	0xa0, //curve13 a
	0x0a, //curve14 b
	0xa0, //curve14 a
	0x0a, //curve15 b
	0xa0, //curve15 a
	0x0a, //curve16 b
	0xa0, //curve16 a
	0x16, //curve17 b
	0xa6, //curve17 a
	0x16, //curve18 b
	0xa6, //curve18 a
	0x16, //curve19 b
	0xa6, //curve19 a
	0x16, //curve20 b
	0xa6, //curve20 a
	0x05, //curve21 b
	0x21, //curve21 a
	0x0b, //curve22 b
	0x20, //curve22 a
	0x87, //curve23 b
	0x0f, //curve23 a
	0x00, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};

static char DSI0_TDMB_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //mask 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //data_width sharpen cs gamma 00 00 0 0
};

static char DSI0_TDMB_MOVIE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
        0x00, //roi0 x start
	0x00,
        0x00, //roi0 x end
	0x00,
        0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
        0x00, //roi1 x strat
	0x00,
        0x00, //roi1 x end
	0x00,
        0x00, //roi1 y start
	0x00,
        0x00, //roi1 y end
        0x00,
        0x8c, //scr Cr Yb
        0xd5, //scr Rr Bb
        0xf2, //scr Cg Yg
        0x29, //scr Rg Bg
        0xe6, //scr Cb Yr
        0x20, //scr Rb Br
        0xdd, //scr Mr Mb
        0x68, //scr Gr Gb
        0x3a, //scr Mg Mg
        0xed, //scr Gg Gg
        0xed, //scr Mb Mr
        0x38, //scr Gb Gr
        0xef, //scr Yr Cb
        0x2a, //scr Br Rb
        0xf1, //scr Yg Cg
        0x1f, //scr Bg Rg
        0x56, //scr Yb Cr
        0xe4, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
        0xf7, //scr Wg Wg
	0x00, //scr Kg Kg
        0xed, //scr Wb Wr
	0x00, //scr Kb Kr
        0x00, //curve 1 b
        0x20, //curve 1 a
        0x00, //curve 2 b
        0x20, //curve 2 a
        0x00, //curve 3 b
        0x20, //curve 3 a
        0x00, //curve 4 b
        0x20, //curve 4 a
        0x02, //curve 5 b
        0x1b, //curve 5 a
        0x02, //curve 6 b
        0x1b, //curve 6 a
        0x02, //curve 7 b
        0x1b, //curve 7 a
        0x02, //curve 8 b
        0x1b, //curve 8 a
        0x09, //curve 9 b
        0xa6, //curve 9 a
        0x09, //curve10 b
        0xa6, //curve10 a
        0x09, //curve11 b
        0xa6, //curve11 a
        0x09, //curve12 b
        0xa6, //curve12 a
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
	0x01, //cs gain
        0x40,
        //end
};

static char DSI0_TDMB_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x05, //sharpen cc gamma 00 0 0
};

static char DSI0_TDMB_AUTO_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
        0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
        0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x00,
	//end
};

static char DSI0_HBM_CE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_HBM_CE_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xf0, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x79, //curve 1 a
	0x00, //curve 2 b
	0x79, //curve 2 a
	0x0c, //curve 3 b
	0x40, //curve 3 a
	0x0c, //curve 4 b
	0x40, //curve 4 a
	0x10, //curve 5 b
	0x38, //curve 5 a
	0x14, //curve 6 b
	0x30, //curve 6 a
	0x14, //curve 7 b
	0x30, //curve 7 a
	0x14, //curve 8 b
	0x30, //curve 8 a
	0x1c, //curve 9 b
	0x28, //curve 9 a
	0x1c, //curve10 b
	0x28, //curve10 a
	0x1c, //curve11 b
	0x28, //curve11 a
	0x1c, //curve12 b
	0x28, //curve12 a
	0x1c, //curve13 b
	0x28, //curve13 a
	0x27, //curve14 b
	0x21, //curve14 a
	0x27, //curve15 b
	0x21, //curve15 a
	0x27, //curve16 b
	0x21, //curve16 a
	0x27, //curve17 b
	0x21, //curve17 a
	0x27, //curve18 b
	0x21, //curve18 a
	0x34, //curve19 b
	0x1c, //curve19 a
	0x40, //curve20 b
	0x19, //curve20 a
	0x45, //curve21 b
	0x18, //curve21 a
	0x58, //curve22 b
	0x15, //curve22 a
	0x58, //curve23 b
	0x15, //curve23 a
	0x06, //curve24 b
	0xFF, //curve24 a
	0x01, //cs gain
	0x20,
	//end
};
/*
static char DSI0_HBM_CE_TEXT_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_HBM_CE_TEXT_MDNIE_2[] = {
	0xEC,
	0x00, //roi ctrl
	0x00, //roi0 x start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi1 x strat
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 y end
	0x00,
	0x00, //scr Cr Yb
	0xff, //scr Rr Bb
	0xff, //scr Cg Yg
	0x1c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x1c, //scr Rb Br
	0xff, //scr Mr Mb
	0x00, //scr Gr Gb
	0x00, //scr Mg Mg
	0xff, //scr Gg Gg
	0xff, //scr Mb Mr
	0x00, //scr Gb Gr
	0xff, //scr Yr Cb
	0x00, //scr Br Rb
	0xff, //scr Yg Cg
	0x00, //scr Bg Rg
	0x00, //scr Yb Cr
	0xff, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xff, //scr Wg Wg
	0x00, //scr Kg Kg
	0xff, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x20, //curve 1 a
	0x00, //curve 2 b
	0x20, //curve 2 a
	0x00, //curve 3 b
	0x20, //curve 3 a
	0x00, //curve 4 b
	0x20, //curve 4 a
	0x02, //curve 5 b
	0x1b, //curve 5 a
	0x02, //curve 6 b
	0x1b, //curve 6 a
	0x02, //curve 7 b
	0x1b, //curve 7 a
	0x02, //curve 8 b
	0x1b, //curve 8 a
	0x09, //curve 9 b
	0xa6, //curve 9 a
	0x09, //curve10 b
	0xa6, //curve10 a
	0x09, //curve11 b
	0xa6, //curve11 a
	0x09, //curve12 b
	0xa6, //curve12 a
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
	0x01, //cs gain
	0x40,
	//end
};
*/
static struct dsi_cmd_desc DSI0_BYPASS_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_1)}, DSI0_BYPASS_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_2)}, DSI0_BYPASS_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_1)}, DSI0_NEGATIVE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_2)}, DSI0_NEGATIVE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_COLOR_BLIND_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_BLIND_MDNIE_1)}, DSI0_COLOR_BLIND_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_BLIND_MDNIE_2)}, DSI0_COLOR_BLIND_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_HBM_CE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_MDNIE_1)}, DSI0_HBM_CE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_HBM_CE_MDNIE_2)}, DSI0_HBM_CE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_RGB_SENSOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_RGB_SENSOR_MDNIE_1)}, DSI0_RGB_SENSOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_RGB_SENSOR_MDNIE_2)}, DSI0_RGB_SENSOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_CURTAIN[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CURTAIN_1)}, DSI0_CURTAIN_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CURTAIN_2)}, DSI0_CURTAIN_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_GRAYSCALE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_1)}, DSI0_GRAYSCALE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_2)}, DSI0_GRAYSCALE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_GRAYSCALE_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_1)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_2)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};


///////////////////////////////////////////////////////////////////////////////////

static struct dsi_cmd_desc DSI0_UI_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_DYNAMIC_MDNIE_1)}, DSI0_UI_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_DYNAMIC_MDNIE_2)}, DSI0_UI_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_UI_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_STANDARD_MDNIE_1)}, DSI0_UI_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_STANDARD_MDNIE_2)}, DSI0_UI_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_UI_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_NATURAL_MDNIE_1)}, DSI0_UI_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_NATURAL_MDNIE_2)}, DSI0_UI_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif

static struct dsi_cmd_desc DSI0_UI_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MOVIE_MDNIE_1)}, DSI0_UI_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MOVIE_MDNIE_2)}, DSI0_UI_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_UI_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_AUTO_MDNIE_1)}, DSI0_UI_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_AUTO_MDNIE_2)}, DSI0_UI_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_DYNAMIC_MDNIE_1)}, DSI0_VIDEO_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_DYNAMIC_MDNIE_2)}, DSI0_VIDEO_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_STANDARD_MDNIE_1)}, DSI0_VIDEO_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_STANDARD_MDNIE_2)}, DSI0_VIDEO_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_VIDEO_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_NATURAL_MDNIE_1)}, DSI0_VIDEO_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_NATURAL_MDNIE_2)}, DSI0_VIDEO_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif

static struct dsi_cmd_desc DSI0_VIDEO_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MOVIE_MDNIE_1)}, DSI0_VIDEO_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MOVIE_MDNIE_2)}, DSI0_VIDEO_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VIDEO_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_AUTO_MDNIE_1)}, DSI0_VIDEO_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_AUTO_MDNIE_2)}, DSI0_VIDEO_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_1)}, DSI0_CAMERA_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_2)}, DSI0_CAMERA_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_AUTO_MDNIE_1)}, DSI0_CAMERA_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_AUTO_MDNIE_2)}, DSI0_CAMERA_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_DYNAMIC_MDNIE_1)}, DSI0_GALLERY_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_DYNAMIC_MDNIE_2)}, DSI0_GALLERY_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_STANDARD_MDNIE_1)}, DSI0_GALLERY_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_STANDARD_MDNIE_2)}, DSI0_GALLERY_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_GALLERY_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_NATURAL_MDNIE_1)}, DSI0_GALLERY_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_NATURAL_MDNIE_2)}, DSI0_GALLERY_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif

static struct dsi_cmd_desc DSI0_GALLERY_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MOVIE_MDNIE_1)}, DSI0_GALLERY_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MOVIE_MDNIE_2)}, DSI0_GALLERY_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_AUTO_MDNIE_1)}, DSI0_GALLERY_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_AUTO_MDNIE_2)}, DSI0_GALLERY_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VT_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_DYNAMIC_MDNIE_1)}, DSI0_VT_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_DYNAMIC_MDNIE_2)}, DSI0_VT_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VT_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_STANDARD_MDNIE_1)}, DSI0_VT_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_STANDARD_MDNIE_2)}, DSI0_VT_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_VT_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_NATURAL_MDNIE_1)}, DSI0_VT_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_NATURAL_MDNIE_2)}, DSI0_VT_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif
static struct dsi_cmd_desc DSI0_VT_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MOVIE_MDNIE_1)}, DSI0_VT_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MOVIE_MDNIE_2)}, DSI0_VT_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_VT_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_AUTO_MDNIE_1)}, DSI0_VT_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_AUTO_MDNIE_2)}, DSI0_VT_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_DYNAMIC_MDNIE_1)}, DSI0_BROWSER_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_DYNAMIC_MDNIE_2)}, DSI0_BROWSER_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_STANDARD_MDNIE_1)}, DSI0_BROWSER_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_STANDARD_MDNIE_2)}, DSI0_BROWSER_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_BROWSER_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_NATURAL_MDNIE_1)}, DSI0_BROWSER_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_NATURAL_MDNIE_2)}, DSI0_BROWSER_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif
static struct dsi_cmd_desc DSI0_BROWSER_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MOVIE_MDNIE_1)}, DSI0_BROWSER_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MOVIE_MDNIE_2)}, DSI0_BROWSER_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_BROWSER_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_AUTO_MDNIE_1)}, DSI0_BROWSER_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_AUTO_MDNIE_2)}, DSI0_BROWSER_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_DYNAMIC_MDNIE_1)}, DSI0_EBOOK_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_DYNAMIC_MDNIE_2)}, DSI0_EBOOK_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_STANDARD_MDNIE_1)}, DSI0_EBOOK_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_STANDARD_MDNIE_2)}, DSI0_EBOOK_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_EBOOK_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_NATURAL_MDNIE_1)}, DSI0_EBOOK_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_NATURAL_MDNIE_2)}, DSI0_EBOOK_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif
static struct dsi_cmd_desc DSI0_EBOOK_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MOVIE_MDNIE_1)}, DSI0_EBOOK_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MOVIE_MDNIE_2)}, DSI0_EBOOK_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_EBOOK_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_AUTO_MDNIE_1)}, DSI0_EBOOK_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_AUTO_MDNIE_2)}, DSI0_EBOOK_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_EMAIL_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_AUTO_MDNIE_1)}, DSI0_EMAIL_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_AUTO_MDNIE_2)}, DSI0_EMAIL_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

///////////////////////////////////////////////////////////////////////////////////

static struct dsi_cmd_desc DSI0_TDMB_DYNAMIC_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_DYNAMIC_MDNIE_1)}, DSI0_TDMB_DYNAMIC_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_DYNAMIC_MDNIE_2)}, DSI0_TDMB_DYNAMIC_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_TDMB_STANDARD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_STANDARD_MDNIE_1)}, DSI0_TDMB_STANDARD_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_STANDARD_MDNIE_2)}, DSI0_TDMB_STANDARD_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

#if defined(NATURAL_MODE_ENABLE)
static struct dsi_cmd_desc DSI0_TDMB_NATURAL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_NATURAL_MDNIE_1)}, DSI0_TDMB_NATURAL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_NATURAL_MDNIE_2)}, DSI0_TDMB_NATURAL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};
#endif

static struct dsi_cmd_desc DSI0_TDMB_MOVIE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_MOVIE_MDNIE_1)}, DSI0_TDMB_MOVIE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_MOVIE_MDNIE_2)}, DSI0_TDMB_MOVIE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc DSI0_TDMB_AUTO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_2_key_on)}, level_2_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_AUTO_MDNIE_1)}, DSI0_TDMB_AUTO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_TDMB_AUTO_MDNIE_2)}, DSI0_TDMB_AUTO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_2_key_off)}, level_2_key_off},
};

static struct dsi_cmd_desc *mdnie_tune_value_dsi0[MAX_APP_MODE][MAX_MODE][MAX_OUTDOOR_MODE] = {
		/*
			DYNAMIC_MODE
			STANDARD_MODE
			NATURAL_MODE
			MOVIE_MODE
			AUTO_MODE
		*/
		// UI_APP
		{
			{DSI0_UI_DYNAMIC_MDNIE,	NULL},
			{DSI0_UI_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_UI_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_UI_MOVIE_MDNIE,	NULL},
			{DSI0_UI_AUTO_MDNIE,	NULL},
		},
		// VIDEO_APP
		{
			{DSI0_VIDEO_DYNAMIC_MDNIE,	NULL},
			{DSI0_VIDEO_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_VIDEO_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_VIDEO_MOVIE_MDNIE,	NULL},
			{DSI0_VIDEO_AUTO_MDNIE,	NULL},
		},
		// VIDEO_WARM_APP
		{
			{NULL, NULL},
			{NULL, NULL},
#if defined(NATURAL_MODE_ENABLE)
			{NULL, NULL},
#endif
			{NULL, NULL},
			{NULL, NULL},
		},
		// VIDEO_COLD_APP
		{
			{NULL, NULL},
			{NULL, NULL},
#if defined(NATURAL_MODE_ENABLE)
			{NULL, NULL},
#endif
			{NULL, NULL},
			{NULL, NULL},
		},
		// CAMERA_APP
		{
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_CAMERA_MDNIE,	NULL},
#endif
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_AUTO_MDNIE,	NULL},
		},
		// NAVI_APP
		{
			{NULL,	NULL},
			{NULL,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{NULL,	NULL},
#endif
			{NULL,	NULL},
			{NULL,	NULL},
		},
		// GALLERY_APP
		{
			{DSI0_GALLERY_DYNAMIC_MDNIE,	NULL},
			{DSI0_GALLERY_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_GALLERY_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_GALLERY_MOVIE_MDNIE,	NULL},
			{DSI0_GALLERY_AUTO_MDNIE,	NULL},
		},
		// VT_APP
		{
			{DSI0_VT_DYNAMIC_MDNIE,	NULL},
			{DSI0_VT_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_VT_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_VT_MOVIE_MDNIE,	NULL},
			{DSI0_VT_AUTO_MDNIE,	NULL},
		},
		// BROWSER_APP
		{
			{DSI0_BROWSER_DYNAMIC_MDNIE,	NULL},
			{DSI0_BROWSER_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_BROWSER_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_BROWSER_MOVIE_MDNIE,	NULL},
			{DSI0_BROWSER_AUTO_MDNIE,	NULL},
		},
		// eBOOK_APP
		{
			{DSI0_EBOOK_DYNAMIC_MDNIE,	NULL},
			{DSI0_EBOOK_STANDARD_MDNIE,NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_EBOOK_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_EBOOK_MOVIE_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// EMAIL_APP
		{
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
#endif
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
		},
		// TDMB_APP
		{
			{DSI0_TDMB_DYNAMIC_MDNIE,	NULL},
			{DSI0_TDMB_STANDARD_MDNIE,	NULL},
#if defined(NATURAL_MODE_ENABLE)
			{DSI0_TDMB_NATURAL_MDNIE,	NULL},
#endif
			{DSI0_TDMB_MOVIE_MDNIE,	NULL},
			{DSI0_TDMB_AUTO_MDNIE,	NULL},
		},
};


#define DSI0_RGB_SENSOR_MDNIE_1_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_1)
#define DSI0_RGB_SENSOR_MDNIE_2_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_2)

#endif /*_DSI_TCON_MDNIE_LITE_DATA_HD_EA8061V_AMS497EE01_A5FU_H_*/
