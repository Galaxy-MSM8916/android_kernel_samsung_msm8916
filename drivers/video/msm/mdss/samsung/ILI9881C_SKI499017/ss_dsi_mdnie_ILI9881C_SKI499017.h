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


#ifndef _SS_DSI_MDNIE_ILI9881C_SKI499017_
#define _SS_DSI_MDNIE_ILI9881C_SKI499017_

/* 2015.04.08 */

#include "../ss_dsi_mdnie_lite_common.h"

#define MDNIE_COLOR_BLINDE_CMD_OFFSET 1

#define ADDRESS_SCR_WHITE_RED   0x7A
#define ADDRESS_SCR_WHITE_GREEN 0x7C
#define ADDRESS_SCR_WHITE_BLUE  0x7E

#define MDNIE_STEP1_INDEX 1
#define MDNIE_STEP2_INDEX 2

static char level_1_key_on[] = {
	0xF0,
	0x5A, 0x5A
};

static char level_1_key_off[] = {
	0xF0,
	0xA5, 0xA5
};

static char DSI0_BYPASS_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_BYPASS_MDNIE_2[] ={
0xE9,
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
};

static char DSI0_BYPASS_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_BYPASS_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_BYPASS_MDNIE_5[] ={
0xEC,
0x04, //cc r1
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

static char DSI0_BYPASS_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x00, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_NEGATIVE_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_NEGATIVE_MDNIE_2[] ={
0xE9,
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
};

static char DSI0_NEGATIVE_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_NEGATIVE_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_NEGATIVE_MDNIE_5[] ={
0xEC,
0x04, //cc r1
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

static char DSI0_NEGATIVE_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0, 
0x05, //VSIZE
0x00, 
0x00, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_GRAYSCALE_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_GRAYSCALE_MDNIE_2[] ={
0xE9,
0xe2, //scr Cr Yb
0x1d, //scr Rr Bb
0xe2, //scr Cg Yg
0x1d, //scr Rg Bg
0xe2, //scr Cb Yr
0x1d, //scr Rb Br
0x69, //scr Mr Mb
0x96, //scr Gr Gb
0x69, //scr Mg Mg
0x96, //scr Gg Gg
0x69, //scr Mb Mr
0x96, //scr Gb Gr
0xb3, //scr Yr Cb
0x4c, //scr Br Rb
0xb3, //scr Yg Cg
0x4c, //scr Bg Rg
0xb3, //scr Yb Cr
0x4c, //scr Bb Rr
0xff, //scr Wr Wb
0x00, //scr Kr Kb
0xff, //scr Wg Wg
0x00, //scr Kg Kg
0xff, //scr Wb Wr
0x00, //scr Kb Kr
};

static char DSI0_GRAYSCALE_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_GRAYSCALE_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_GRAYSCALE_MDNIE_5[] ={
0xEC,
0x04, //cc r1
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

static char DSI0_GRAYSCALE_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_2[] ={
0xE9,
0x1d, //scr Cr Yb
0xe2, //scr Rr Bb
0x1d, //scr Cg Yg
0xe2, //scr Rg Bg
0x1d, //scr Cb Yr
0xe2, //scr Rb Br
0x96, //scr Mr Mb
0x69, //scr Gr Gb
0x96, //scr Mg Mg
0x69, //scr Gg Gg
0x96, //scr Mb Mr
0x69, //scr Gb Gr
0x4c, //scr Yr Cb
0xb3, //scr Br Rb
0x4c, //scr Yg Cg
0xb3, //scr Bg Rg
0x4c, //scr Yb Cr
0xb3, //scr Bb Rr
0x00, //scr Wr Wb
0xff, //scr Kr Kb
0x00, //scr Wg Wg
0xff, //scr Kg Kg
0x00, //scr Wb Wr
0xff, //scr Kb Kr
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_5[] ={
0xEC,
0x04, //cc r1
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

static char DSI0_GRAYSCALE_NEGATIVE_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};
static char DSI0_COLOR_ADJUSTMENT_MDNIE_1[] ={
//start
0xE8,
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
};
static char DSI0_COLOR_ADJUSTMENT_MDNIE_2[] ={
0xE9,
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
};
static char DSI0_COLOR_ADJUSTMENT_MDNIE_3[] ={
0xEA,
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
};
static char DSI0_COLOR_ADJUSTMENT_MDNIE_4[] ={
0xEB,
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
};
static char DSI0_COLOR_ADJUSTMENT_MDNIE_5[] ={
0xEC,
0x04, //cc r1
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
static char DSI0_COLOR_ADJUSTMENT_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_UI_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_UI_MDNIE_2[] ={
0xE9,
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
};

static char DSI0_UI_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_UI_MDNIE_4[] = {
0xEB,
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
};

static char DSI0_UI_MDNIE_5[] ={
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};
static char DSI0_UI_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x02, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_OUTDOOR_MDNIE_1[] ={
//start
0xE8,
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
};
static char DSI0_OUTDOOR_MDNIE_2[] ={
0xE9,
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
};

static char DSI0_OUTDOOR_MDNIE_3[] ={
0xEA,
0x00, //curve 1 b
0x7b, //curve 1 a
0x03, //curve 2 b
0x48, //curve 2 a
0x08, //curve 3 b
0x32, //curve 3 a
0x08, //curve 4 b
0x32, //curve 4 a
0x08, //curve 5 b
0x32, //curve 5 a
0x08, //curve 6 b
0x32, //curve 6 a
0x08, //curve 7 b
0x32, //curve 7 a
0x10, //curve 8 b
0x28, //curve 8 a
0x10, //curve 9 b
0x28, //curve 9 a
0x10, //curve10 b
0x28, //curve10 a
0x10, //curve11 b
0x28, //curve11 a
0x10, //curve12 b
0x28, //curve12 a
};
static char DSI0_OUTDOOR_MDNIE_4[] ={
0xEB,
0x19, //curve13 b
0x22, //curve13 a
0x19, //curve14 b
0x22, //curve14 a
0x19, //curve15 b
0x22, //curve15 a
0x19, //curve16 b
0x22, //curve16 a
0x19, //curve17 b
0x22, //curve17 a
0x19, //curve18 b
0x22, //curve18 a
0x23, //curve19 b
0x1e, //curve19 a
0x2e, //curve20 b
0x1b, //curve20 a
0x33, //curve21 b
0x1a, //curve21 a
0x40, //curve22 b
0x18, //curve22 a
0x48, //curve23 b
0x17, //curve23 a
0x00, //curve24 b
0xFF, //curve24 a
};

static char DSI0_OUTDOOR_MDNIE_5[] ={
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};
static char DSI0_OUTDOOR_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x03, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_VIDEO_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_VIDEO_MDNIE_2[] = {
0xE9,
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
};

static char DSI0_VIDEO_MDNIE_3[] ={
0xEA,
0x00, //curve 1 b
0x1c, //curve 1 a
0x00, //curve 2 b
0x1c, //curve 2 a
0x00, //curve 3 b
0x1c, //curve 3 a
0x00, //curve 4 b
0x1c, //curve 4 a
0x00, //curve 5 b
0x1c, //curve 5 a
0x00, //curve 6 b
0x1c, //curve 6 a
0x00, //curve 7 b
0x1c, //curve 7 a
0x00, //curve 8 b
0x1c, //curve 8 a
0x00, //curve 9 b
0x1c, //curve 9 a
0x00, //curve10 b
0x1c, //curve10 a
0x00, //curve11 b
0x1c, //curve11 a
0x00, //curve12 b
0x1c, //curve12 a
};

static char DSI0_VIDEO_MDNIE_4[] = {
0xEB,
0x00, //curve13 b
0x1c, //curve13 a
0x0d, //curve14 b
0xa4, //curve14 a
0x0d, //curve15 b
0xa4, //curve15 a
0x0d, //curve16 b
0xa4, //curve16 a
0x0d, //curve17 b
0xa4, //curve17 a
0x0d, //curve18 b
0xa4, //curve18 a
0x0d, //curve19 b
0xa4, //curve19 a
0x0d, //curve20 b
0xa4, //curve20 a
0x0d, //curve21 b
0xa4, //curve21 a
0x25, //curve22 b
0x1c, //curve22 a
0x4a, //curve23 b
0x17, //curve23 a
0x00, //curve24 b
0xFF, //curve24 a
};

static char DSI0_VIDEO_MDNIE_5[] ={
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};

static char DSI0_VIDEO_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x07, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_CAMERA_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_CAMERA_MDNIE_2[] ={
0xE9,
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
};

static char DSI0_CAMERA_MDNIE_3[] ={
0xEA,
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
};

static char DSI0_CAMERA_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_CAMERA_MDNIE_5[] ={
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};

static char DSI0_CAMERA_MDNIE_6[] ={
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x02, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_GALLERY_MDNIE_1[] ={
//start
0xE8,
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
};

static char DSI0_GALLERY_MDNIE_2[] = {
0xE9,
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
};

static char DSI0_GALLERY_MDNIE_3[] = {
0xEA,
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
};

static char DSI0_GALLERY_MDNIE_4[] ={
0xEB,
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
};

static char DSI0_GALLERY_MDNIE_5[] = {
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};

static char DSI0_GALLERY_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x02, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_VT_MDNIE_1[] = {
//start
0xE8,
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
};

static char DSI0_VT_MDNIE_2[] = {
0xE9,
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
};
static char DSI0_VT_MDNIE_3[] = {
0xEA,
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
};

static char DSI0_VT_MDNIE_4[] = {
0xEB,
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
};
static char DSI0_VT_MDNIE_5[] = {
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};

static char DSI0_VT_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x03, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x06, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_BROWSER_MDNIE_1[] = {
//start
0xE8,
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
};
static char DSI0_BROWSER_MDNIE_2[] = {
0xE9,
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
};

static char DSI0_BROWSER_MDNIE_3[] = {
0xEA,
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
};
static char DSI0_BROWSER_MDNIE_4[] = {
0xEB,
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
};

static char DSI0_BROWSER_MDNIE_5[] = {
0xEC,
0x04, //cc r1 0.15
0x88,
0x1f, //cc r2
0xa6,
0x1f, //cc r3
0xd2,
0x1f, //cc g1
0xee,
0x04, //cc g2
0x3f,
0x1f, //cc g3
0xd2,
0x1f, //cc b1
0xee,
0x1f, //cc b2
0xa6,
0x04, //cc b3
0x6c,
};
static char DSI0_BROWSER_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x33, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x02, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_EBOOK_MDNIE_1[] = {
//start
0xE8,
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
};

static char DSI0_EBOOK_MDNIE_2[] = {
0xE9,
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
0xd6, //scr Wr Wb
0x00, //scr Kr Kb
0xea, //scr Wg Wg
0x00, //scr Kg Kg
0xff, //scr Wb Wr
0x00, //scr Kb Kr
};
static char DSI0_EBOOK_MDNIE_3[] = {
0xEA,
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
};

static char DSI0_EBOOK_MDNIE_4[] = {
0xEB,
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

};
static char DSI0_EBOOK_MDNIE_5[] = {
0xEC,
0x04, //cc r1
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

static char DSI0_EBOOK_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};

static char DSI0_EMAIL_MDNIE_1[] = {
//start
0xE8,
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
};
static char DSI0_EMAIL_MDNIE_2[] = {
0xE9,
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
0xf7, //scr Wb Wr
0x00, //scr Kb Kr
};

static char DSI0_EMAIL_MDNIE_3[] = {
0xEA,
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
};
static char DSI0_EMAIL_MDNIE_4[] = {
0xEB,
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
};
static char DSI0_EMAIL_MDNIE_5[] = {
0xEC,
0x04, //cc r1
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
static char DSI0_EMAIL_MDNIE_6[] = {
0xE7,
0x08, //roi_ctrl rgb_if_type mdnie_en mask 00 00 0 000
0x30, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
0x02, //HSIZE
0xD0,
0x05, //VSIZE
0x00,
0x00, //sharpen cc gamma 00 0 0
//end
};

static struct dsi_cmd_desc DSI0_BYPASS_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_1)}, DSI0_BYPASS_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_2)}, DSI0_BYPASS_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_3)}, DSI0_BYPASS_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_4)}, DSI0_BYPASS_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_5)}, DSI0_BYPASS_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BYPASS_MDNIE_6)}, DSI0_BYPASS_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_CAMERA_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_1)}, DSI0_CAMERA_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_2)}, DSI0_CAMERA_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_3)}, DSI0_CAMERA_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_4)}, DSI0_CAMERA_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_5)}, DSI0_CAMERA_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_6)}, DSI0_CAMERA_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_COLOR_ADJUSTMENT_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_1)}, DSI0_COLOR_ADJUSTMENT_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_2)}, DSI0_COLOR_ADJUSTMENT_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_3)}, DSI0_COLOR_ADJUSTMENT_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_4)}, DSI0_COLOR_ADJUSTMENT_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_5)}, DSI0_COLOR_ADJUSTMENT_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_COLOR_ADJUSTMENT_MDNIE_6)}, DSI0_COLOR_ADJUSTMENT_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_EBOOK_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_1)}, DSI0_EBOOK_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_2)}, DSI0_EBOOK_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_3)}, DSI0_EBOOK_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_4)}, DSI0_EBOOK_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_5)}, DSI0_EBOOK_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EBOOK_MDNIE_6)}, DSI0_EBOOK_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_EMAIL_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_1)}, DSI0_EMAIL_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_2)}, DSI0_EMAIL_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_3)}, DSI0_EMAIL_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_4)}, DSI0_EMAIL_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_5)}, DSI0_EMAIL_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_EMAIL_MDNIE_6)}, DSI0_EMAIL_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};

static struct dsi_cmd_desc DSI0_GALLERY_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_1)}, DSI0_GALLERY_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_2)}, DSI0_GALLERY_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_3)}, DSI0_GALLERY_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_4)}, DSI0_GALLERY_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_5)}, DSI0_GALLERY_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_6)}, DSI0_GALLERY_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_1)}, DSI0_NEGATIVE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_2)}, DSI0_NEGATIVE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_3)}, DSI0_NEGATIVE_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_4)}, DSI0_NEGATIVE_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_5)}, DSI0_NEGATIVE_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_6)}, DSI0_NEGATIVE_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_GRAYSCALE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_1)}, DSI0_GRAYSCALE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_2)}, DSI0_GRAYSCALE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_3)}, DSI0_GRAYSCALE_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_4)}, DSI0_GRAYSCALE_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_5)}, DSI0_GRAYSCALE_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_MDNIE_6)}, DSI0_GRAYSCALE_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_GRAYSCALE_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_1)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_2)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_3)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_4)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_5)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_GRAYSCALE_NEGATIVE_MDNIE_6)}, DSI0_GRAYSCALE_NEGATIVE_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_OUTDOOR_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_1)}, DSI0_OUTDOOR_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_2)}, DSI0_OUTDOOR_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_3)}, DSI0_OUTDOOR_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_4)}, DSI0_OUTDOOR_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_5)}, DSI0_OUTDOOR_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_OUTDOOR_MDNIE_6)}, DSI0_OUTDOOR_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_UI_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_1)}, DSI0_UI_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_2)}, DSI0_UI_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_3)}, DSI0_UI_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_4)}, DSI0_UI_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_5)}, DSI0_UI_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_UI_MDNIE_6)}, DSI0_UI_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_VIDEO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_1)}, DSI0_VIDEO_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_2)}, DSI0_VIDEO_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_3)}, DSI0_VIDEO_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_4)}, DSI0_VIDEO_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_5)}, DSI0_VIDEO_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_6)}, DSI0_VIDEO_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_VT_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_1)}, DSI0_VT_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_2)}, DSI0_VT_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_3)}, DSI0_VT_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_4)}, DSI0_VT_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_5)}, DSI0_VT_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_VT_MDNIE_6)}, DSI0_VT_MDNIE_6},
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(level_1_key_off)}, level_1_key_off},
};
static struct dsi_cmd_desc DSI0_BROWSER_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(level_1_key_on)}, level_1_key_on},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_1)}, DSI0_BROWSER_MDNIE_1},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_2)}, DSI0_BROWSER_MDNIE_2},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_3)}, DSI0_BROWSER_MDNIE_3},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_4)}, DSI0_BROWSER_MDNIE_4},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_5)}, DSI0_BROWSER_MDNIE_5},
	{{DTYPE_DCS_LWRITE, 0, 0, 0, 0, sizeof(DSI0_BROWSER_MDNIE_6)}, DSI0_BROWSER_MDNIE_6},
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
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// VIDEO_APP
		{
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
		},
		// VIDEO_WARM_APP
		{
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
		},
		// VIDEO_COLD_APP
		{
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
			{DSI0_VIDEO_MDNIE,	NULL},
		},
		// CAMERA_APP
		{
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
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
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
		},
		// VT_APP
		{
			{DSI0_VT_MDNIE,	NULL},
			{DSI0_VT_MDNIE,	NULL},
			{DSI0_VT_MDNIE,	NULL},
			{DSI0_VT_MDNIE,	NULL},
			{DSI0_VT_MDNIE,	NULL},
		},
		// BROWSER_APP
		{
			{DSI0_BROWSER_MDNIE,	NULL},
			{DSI0_BROWSER_MDNIE,	NULL},
			{DSI0_BROWSER_MDNIE,	NULL},
			{DSI0_BROWSER_MDNIE,	NULL},
			{DSI0_BROWSER_MDNIE,	NULL},
		},
		// eBOOK_APP
		{
			{DSI0_EBOOK_MDNIE,	NULL},
			{DSI0_EBOOK_MDNIE,	NULL},
			{DSI0_EBOOK_MDNIE,	NULL},
			{DSI0_EBOOK_MDNIE,	NULL},
			{DSI0_EBOOK_MDNIE,	NULL},
		},
		// EMAIL_APP
		{
			{DSI0_EMAIL_MDNIE,	NULL},
			{DSI0_EMAIL_MDNIE,	NULL},
			{DSI0_EMAIL_MDNIE,	NULL},
			{DSI0_EMAIL_MDNIE,	NULL},
			{DSI0_EMAIL_MDNIE,	NULL},
		},
};
#endif /*_SAMSUNG_DSI_MDNIE_S6D7AA0X62_BV050HDM_*/
