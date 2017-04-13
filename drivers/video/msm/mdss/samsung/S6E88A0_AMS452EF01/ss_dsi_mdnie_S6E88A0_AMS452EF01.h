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
#ifndef _SAMSUNG_DSI_MDNIE_S6E88A0_AMS452EF01_H_
#define _SAMSUNG_DSI_MDNIE_S6E88A0_AMS452EF01_H_

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

static char DSI0_GRAYSCALE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_BYPASS_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_BYPASS_MDNIE_2[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_NEGATIVE_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};
static char DSI0_NEGATIVE_MDNIE_2[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_COLOR_BLIND_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};
static char DSI0_COLOR_BLIND_MDNIE_2[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_RGB_SENSOR_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};

static char DSI0_RGB_SENSOR_MDNIE_2[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_UI_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};

static char DSI0_UI_DYNAMIC_MDNIE_2[] ={
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_UI_NATURAL_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};
static char DSI0_UI_NATURAL_MDNIE_2[] ={
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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_UI_MOVIE_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};
static char DSI0_UI_MOVIE_MDNIE_2[] ={
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
};

static char DSI0_UI_AUTO_MDNIE_1[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_VIDEO_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x03,
	0x0b,
};
static char DSI0_VIDEO_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xff,
	0x00, 0xff, 0x00,
	0xff, 0x00, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x26, 0xbe,
	0x26, 0xbe, 0x26,
	0xbe, 0x26, 0xbe,
	0x0f, 0xb4, 0x05,
	0x2c, 0x48, 0x1b,
	0x6d, 0x14, 0x99,
	0x0d, 0xaf, 0x0a,
	0x00, 0xFF, 0x04,
	0x90, 0x1f, 0x88,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x04, 0x55,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x1f, 0x88,
	0x04, 0xb5,
};

static char DSI0_VIDEO_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x07, //sharpen cc gamma 00 0 0
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

static char DSI0_VIDEO_STANDARD_MDNIE_1[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_VIDEO_NATURAL_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x03, //sharpen cc gamma 00 0 0
};
static char DSI0_VIDEO_NATURAL_MDNIE_2[] ={
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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_VIDEO_MOVIE_MDNIE_1[] ={
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x04, //sharpen cc gamma 00 0 0
};
static char DSI0_VIDEO_MOVIE_MDNIE_2[] ={
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
};

static char DSI0_VIDEO_AUTO_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x07, //sharpen cc gamma 00 0 0
};

static char DSI0_VIDEO_AUTO_MDNIE_2[] ={
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

static char DSI0_VIDEO_WARM_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x33,
	0x0b,
};
static char DSI0_VIDEO_WARM_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xff,
	0x00, 0xf5, 0x00,
	0xe4, 0x00, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x26, 0xbe,
	0x26, 0xbe, 0x26,
	0xbe, 0x26, 0xbe,
	0x0f, 0xb4, 0x05,
	0x2c, 0x48, 0x1b,
	0x6d, 0x14, 0x99,
	0x0d, 0xaf, 0x0a,
	0x00, 0xFF, 0x04,
	0x90, 0x1f, 0x88,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x04, 0x55,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x1f, 0x88,
	0x04, 0xb5,
};

static char DSI0_VIDEO_WARM_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x33,
	0x08,
};
static char DSI0_VIDEO_WARM_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xff,
	0x00, 0xf5, 0x00,
	0xe4, 0x00, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x14, 0xA4,
	0x14, 0xA4, 0x14,
	0xA4, 0x14, 0xA4,
	0x14, 0xA4, 0x05,
	0x20, 0x1B, 0x1D,
	0x00, 0xFF, 0x04,
	0x2b, 0x1f, 0xdc,
	0x1f, 0xf9, 0x1f,
	0xee, 0x04, 0x19,
	0x1f, 0xf9, 0x1f,
	0xee, 0x1f, 0xdc,
	0x04, 0x36,
};

static char DSI0_VIDEO_COLD_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x33,
	0x0b,
};
static char DSI0_VIDEO_COLD_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xee,
	0x00, 0xf3, 0x00,
	0xff, 0x00, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x26, 0xbe,
	0x26, 0xbe, 0x26,
	0xbe, 0x26, 0xbe,
	0x0f, 0xb4, 0x05,
	0x2c, 0x48, 0x1b,
	0x6d, 0x14, 0x99,
	0x0d, 0xaf, 0x0a,
	0x00, 0xFF, 0x04,
	0x90, 0x1f, 0x88,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x04, 0x55,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x1f, 0x88,
	0x04, 0xb5,
};

static char DSI0_VIDEO_COLD_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x33,
	0x08,
};
static char DSI0_VIDEO_COLD_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xee,
	0x00, 0xf3, 0x00,
	0xff, 0x00, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x00, 0x1A,
	0x00, 0x1A, 0x00,
	0x1A, 0x14, 0xA4,
	0x14, 0xA4, 0x14,
	0xA4, 0x14, 0xA4,
	0x14, 0xA4, 0x05,
	0x20, 0x1B, 0x1D,
	0x00, 0xFF, 0x04,
	0x2b, 0x1f, 0xdc,
	0x1f, 0xf9, 0x1f,
	0xee, 0x04, 0x19,
	0x1f, 0xf9, 0x1f,
	0xee, 0x1f, 0xdc,
	0x04, 0x36,
};

static char DSI0_CAMERA_OUTDOOR_MDNIE_1[] ={
	0xEB,
	0x01, 0x00, 0x03,
	0x0b,
};
static char DSI0_CAMERA_OUTDOOR_MDNIE_2[] ={
	0xEC,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0x00,
	0xff, 0x00, 0xff,
	0x00, 0x00, 0xff,
	0xff, 0x00, 0xff,
	0x00, 0xff, 0x00,
	0x00, 0xff, 0xff,
	0x00, 0xff, 0x00,
	0xff, 0x00, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x00, 0x20,
	0x00, 0x20, 0x00,
	0x20, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x0c, 0xae,
	0x0c, 0xae, 0x0c,
	0xae, 0x26, 0xbe,
	0x26, 0xbe, 0x26,
	0xbe, 0x26, 0xbe,
	0x0f, 0xb4, 0x05,
	0x2c, 0x48, 0x1b,
	0x6d, 0x14, 0x99,
	0x0d, 0xaf, 0x0a,
	0x00, 0xFF, 0x04,
	0x90, 0x1f, 0x88,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x04, 0x55,
	0x1f, 0xe8, 0x1f,
	0xc3, 0x1f, 0x88,
	0x04, 0xb5,
};

static char DSI0_CAMERA_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x01, //sharpen cc gamma 00 0 0
};

static char DSI0_CAMERA_MDNIE_2[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_CAMERA_AUTO_MDNIE_1[] ={
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
	0x2c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x2c, //scr Rb Br
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_GALLERY_DYNAMIC_MDNIE_1[] ={
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x07, //sharpen cc gamma 00 0 0
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

static char DSI0_GALLERY_STANDARD_MDNIE_1[] ={
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_GALLERY_NATURAL_MDNIE_1[] ={
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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_GALLERY_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x04, //sharpen cc gamma 00 0 0
};
static char DSI0_GALLERY_MOVIE_MDNIE_2[] = {
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
};

static char DSI0_GALLERY_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x05, //sharpen cc gamma 00 0 0
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
	0x2c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x2c, //scr Rb Br
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_VT_DYNAMIC_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x07, //sharpen cc gamma 00 0 0
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_VT_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x04, //sharpen cc gamma 00 0 0
};
static char DSI0_VT_MOVIE_MDNIE_2[] = {
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
};

static char DSI0_VT_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x05, //sharpen cc gamma 00 0 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_BROWSER_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};
static char DSI0_BROWSER_MOVIE_MDNIE_2[] = {
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
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
	0x2c, //scr Rg Bg
	0xff, //scr Cb Yr
	0x2c, //scr Rb Br
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
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
	0x24, //scr Rg Bg
	0xff, //scr Cb Yr
	0x24, //scr Rb Br
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
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

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
	0x04, //cc r1 0.15
	0x6c,
	0x1f, //cc r2
	0xa6,
	0x1f, //cc r3
	0xee,
	0x1f, //cc g1
	0xd2,
	0x04, //cc g2
	0x40,
	0x1f, //cc g3
	0xee,
	0x1f, //cc b1
	0xd2,
	0x1f, //cc b2
	0xa6,
	0x04, //cc b3
	0x88,
	//end
};

static char DSI0_EBOOK_MOVIE_MDNIE_1[] = {
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //sharpen cc gamma 00 0 0
};
static char DSI0_EBOOK_MOVIE_MDNIE_2[] = {
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
	0x97, //scr Cr Yb
	0xd2, //scr Rr Bb
	0xf4, //scr Cg Yg
	0x19, //scr Rg Bg
	0xe7, //scr Cb Yr
	0x16, //scr Rb Br
	0xdb, //scr Mr Mb
	0x70, //scr Gr Gb
	0x24, //scr Mg Mg
	0xf0, //scr Gg Gg
	0xe5, //scr Mb Mr
	0x22, //scr Gb Gr
	0xf0, //scr Yr Cb
	0x22, //scr Br Rb
	0xf2, //scr Yg Cg
	0x11, //scr Bg Rg
	0x40, //scr Yb Cr
	0xe1, //scr Bb Rr
	0xff, //scr Wr Wb
	0x00, //scr Kr Kb
	0xf9, //scr Wg Wg
	0x00, //scr Kg Kg
	0xf0, //scr Wb Wr
	0x00, //scr Kb Kr
	0x00, //curve 1 b
	0x0f, //curve 1 a
	0x00, //curve 2 b
	0x0f, //curve 2 a
	0x00, //curve 3 b
	0x0f, //curve 3 a
	0x00, //curve 4 b
	0x0f, //curve 4 a
	0x10, //curve 5 b
	0xb0, //curve 5 a
	0x10, //curve 6 b
	0xb0, //curve 6 a
	0x10, //curve 7 b
	0xb0, //curve 7 a
	0x10, //curve 8 b
	0xb0, //curve 8 a
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
	0x04, //cc r1 0.2
	0x90,
	0x1f, //cc r2
	0x88,
	0x1f, //cc r3
	0xe8,
	0x1f, //cc g1
	0xc3,
	0x04, //cc g2
	0x55,
	0x1f, //cc g3
	0xe8,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc b2
	0x88,
	0x04, //cc b3
	0xb5,
};

static char DSI0_EBOOK_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
	//end
};

static char DSI0_EMAIL_AUTO_MDNIE_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
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
	0x00, //curve_1_b
	0x7f, //curve_1_a
	0x00, //curve_2_b
	0x7f, //curve_2_a
	0x11, //curve_3_b
	0x32, //curve_3_a
	0x11, //curve_4_b
	0x32, //curve_4_a
	0x11, //curve_5_b
	0x32, //curve_5_a
	0x11, //curve_6_b
	0x32, //curve_6_a
	0x11, //curve_7_b
	0x32, //curve_7_a
	0x19, //curve_8_b
	0x28, //curve_8_a
	0x19, //curve_9_b
	0x28, //curve_9_a
	0x19, //curve10_b
	0x28, //curve10_a
	0x19, //curve11_b
	0x28, //curve11_a
	0x19, //curve12_b
	0x28, //curve12_a
	0x19, //curve13_b
	0x28, //curve13_a
	0x19, //curve14_b
	0x28, //curve14_a
	0x19, //curve15_b
	0x28, //curve15_a
	0x19, //curve16_b
	0x28, //curve16_a
	0x19, //curve17_b
	0x28, //curve17_a
	0x19, //curve18_b
	0x28, //curve18_a
	0x28, //curve19_b
	0x22, //curve19_a
	0x53, //curve20_b
	0x17, //curve20_a
	0x63, //curve21_b
	0x14, //curve21_a
	0x68, //curve22_b
	0x13, //curve22_a
	0x68, //curve23_b
	0x13, //curve23_a
	0x00, //curve24_b
	0xff, //curve24_a
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

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
	0xff, //curve24 a
	0x04, //cc r1 0.06
	0x2b,
	0x1f, //cc r2
	0xdc,
	0x1f, //cc r3
	0xf9,
	0x1f, //cc g1
	0xee,
	0x04, //cc g2
	0x19,
	0x1f, //cc g3
	0xf9,
	0x1f, //cc b1
	0xee,
	0x1f, //cc b2
	0xdc,
	0x04, //cc b3
	0x36,
	//end
};

static char DSI0_CURTAIN_1[] = {
	//start
	0xEB,
	0x01, //mdnie_en
	0x00, //data_width mask 00 000
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
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
	0x04, //cc r1 x
	0x00,
	0x00, //cc r2
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc g1
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g3
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc b2
	0x00,
	0x04, //cc b3
	0x00,
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
		*/
		// UI_APP
		{
			{DSI0_UI_DYNAMIC_MDNIE,	NULL},
			{DSI0_UI_STANDARD_MDNIE,	NULL},
			{DSI0_UI_NATURAL_MDNIE,	NULL},
			{DSI0_UI_MOVIE_MDNIE,	NULL},
			{DSI0_UI_AUTO_MDNIE,	NULL},
		},
		// VIDEO_APP
		{
			{DSI0_VIDEO_DYNAMIC_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_STANDARD_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_NATURAL_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_MOVIE_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
			{DSI0_VIDEO_AUTO_MDNIE,	DSI0_VIDEO_OUTDOOR_MDNIE},
		},
		// VIDEO_WARM_APP
		{
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
			{DSI0_VIDEO_WARM_MDNIE,	DSI0_VIDEO_WARM_OUTDOOR_MDNIE},
		},
		// VIDEO_COLD_APP
		{
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
			{DSI0_VIDEO_COLD_MDNIE,	DSI0_VIDEO_COLD_OUTDOOR_MDNIE},
		},
		// CAMERA_APP
		{
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
			{DSI0_CAMERA_AUTO_MDNIE,	DSI0_CAMERA_OUTDOOR_MDNIE},
		},
		// NAVI_APP
		{
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
		},
		// VT_APP
		{
			{DSI0_VT_DYNAMIC_MDNIE,	NULL},
			{DSI0_VT_STANDARD_MDNIE,	NULL},
			{DSI0_VT_NATURAL_MDNIE,	NULL},
			{DSI0_VT_MOVIE_MDNIE,	NULL},
			{DSI0_VT_AUTO_MDNIE,	NULL},
		},
		// BROWSER_APP
		{
			{DSI0_BROWSER_DYNAMIC_MDNIE,	NULL},
			{DSI0_BROWSER_STANDARD_MDNIE,	NULL},
			{DSI0_BROWSER_NATURAL_MDNIE,	NULL},
			{DSI0_BROWSER_MOVIE_MDNIE,	NULL},
			{DSI0_BROWSER_AUTO_MDNIE,	NULL},
		},
		// eBOOK_APP
		{
			{DSI0_EBOOK_DYNAMIC_MDNIE,	NULL},
			{DSI0_EBOOK_STANDARD_MDNIE,NULL},
			{DSI0_EBOOK_NATURAL_MDNIE,	NULL},
			{DSI0_EBOOK_MOVIE_MDNIE,	NULL},
			{DSI0_EBOOK_AUTO_MDNIE,	NULL},
		},
		// EMAIL_APP
		{
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
			{DSI0_EMAIL_AUTO_MDNIE,	NULL},
		},
		// TDMB_APP
		{
			{DSI0_UI_DYNAMIC_MDNIE,	NULL},
			{DSI0_UI_STANDARD_MDNIE,	NULL},
			{DSI0_UI_NATURAL_MDNIE,	NULL},
			{DSI0_UI_MOVIE_MDNIE,	NULL},
			{DSI0_UI_AUTO_MDNIE,	NULL},
		},
};

#define DSI0_RGB_SENSOR_MDNIE_1_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_1)
#define DSI0_RGB_SENSOR_MDNIE_2_SIZE ARRAY_SIZE(DSI0_RGB_SENSOR_MDNIE_2)
#endif /*_DSI_TCON_MDNIE_LITE_DATA_FHD_S6E3FA2_H_*/
