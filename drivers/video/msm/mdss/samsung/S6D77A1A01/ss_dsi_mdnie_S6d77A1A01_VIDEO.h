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
#ifndef _SS_DSI_MDNIE_HX8369B_BV045WVM_H_
#define _SS_DSI_MDNIE_HX8369B_BV045WVM_H_

#include "../ss_dsi_mdnie_lite_common.h"

#define MDNIE_COLOR_BLINDE_CMD_OFFSET 104

#define MDNIE_STEP1_INDEX 1
#define MDNIE_STEP2_INDEX 2

static char DSI0_NEGATIVE_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A, //password 5A
	0x00, //mask 000
	0x00, //data_width
	0x32, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //roi_ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0xff, //scr Kb
	0x00, //scr Wb
	0xff, //scr Kg
	0x00, //scr Wg
	0xff, //scr Kr
	0x00, //scr Wr
	0x00, //scr Bb
	0xff, //scr Yb
	0xff, //scr Bg
	0x00, //scr Yg
	0xff, //scr Br
	0x00, //scr Yr
	0xff, //scr Gb
	0x00, //scr Mb
	0x00, //scr Gg
	0xff, //scr Mg
	0xff, //scr Gr
	0x00, //scr Mr
	0xff, //scr Rb
	0x00, //scr Cb
	0xff, //scr Rg
	0x00, //scr Cg
	0x00, //scr Rr
	0xff, //scr Cr
	0x00, //sharpen_set cc_en gamma_en 00 0 0
	0xff, //curve24 a
	0x00, //curve24 b
	0x20, //curve23 a
	0x00, //curve23 b
	0x20, //curve22 a
	0x00, //curve22 b
	0x20, //curve21 a
	0x00, //curve21 b
	0x20, //curve20 a
	0x00, //curve20 b
	0x20, //curve19 a
	0x00, //curve19 b
	0x20, //curve18 a
	0x00, //curve18 b
	0x20, //curve17 a
	0x00, //curve17 b
	0x20, //curve16 a
	0x00, //curve16 b
	0x20, //curve15 a
	0x00, //curve15 b
	0x20, //curve14 a
	0x00, //curve14 b
	0x20, //curve13 a
	0x00, //curve13 b
	0x20, //curve12 a
	0x00, //curve12 b
	0x20, //curve11 a
	0x00, //curve11 b
	0x20, //curve10 a
	0x00, //curve10 b
	0x20, //curve 9 a
	0x00, //curve 9 b
	0x20, //curve 8 a
	0x00, //curve 8 b
	0x20, //curve 7 a
	0x00, //curve 7 b
	0x20, //curve 6 a
	0x00, //curve 6 b
	0x20, //curve 5 a
	0x00, //curve 5 b
	0x20, //curve 4 a
	0x00, //curve 4 b
	0x20, //curve 3 a
	0x00, //curve 3 b
	0x20, //curve 2 a
	0x00, //curve 2 b
	0x20, //curve 1 a
	0x00, //curve 1 b
	0x04, //cc b3
	0x00,
	0x00, //cc b2
	0x00,
	0x00, //cc b1
	0x00,
	0x00, //cc g3
	0x00,
	0x04, //cc g2
	0x00,
	0x00, //cc g1
	0x00,
	0x00, //cc r3
	0x00,
	0x00, //cc r2
	0x00,
	0x04, //cc r1
	0x00,
};

static char DSI0_UI_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A, //password 5A
	0x00, //mask 000
	0x00, //data_width
	0x23, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //roi_ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, //scr Kb
	0xFF, //scr Wb
	0x00, //scr Kg
	0xFF, //scr Wg
	0x00, //scr Kr
	0xFF, //scr Wr
	0xFF, //scr Bb
	0x00, //scr Yb
	0x00, //scr Bg
	0xFF, //scr Yg
	0x00, //scr Br
	0xFF, //scr Yr
	0x00, //scr Gb
	0xFF, //scr Mb
	0xFF, //scr Gg
	0x00, //scr Mg
	0x00, //scr Gr
	0xFF, //scr Mr
	0x00, //scr Rb
	0xFF, //scr Cb
	0x00, //scr Rg
	0xFF, //scr Cg
	0xFF, //scr Rr
	0x00, //scr Cr
	0x02, //sharpen_set cc_en gamma_en 00 0 0
	0xff, //curve24 a
	0x00, //curve24 b
	0x20, //curve23 a
	0x00, //curve23 b
	0x20, //curve22 a
	0x00, //curve22 b
	0x20, //curve21 a
	0x00, //curve21 b
	0x20, //curve20 a
	0x00, //curve20 b
	0x20, //curve19 a
	0x00, //curve19 b
	0x20, //curve18 a
	0x00, //curve18 b
	0x20, //curve17 a
	0x00, //curve17 b
	0x20, //curve16 a
	0x00, //curve16 b
	0x20, //curve15 a
	0x00, //curve15 b
	0x20, //curve14 a
	0x00, //curve14 b
	0x20, //curve13 a
	0x00, //curve13 b
	0x20, //curve12 a
	0x00, //curve12 b
	0x20, //curve11 a
	0x00, //curve11 b
	0x20, //curve10 a
	0x00, //curve10 b
	0x20, //curve 9 a
	0x00, //curve 9 b
	0x20, //curve 8 a
	0x00, //curve 8 b
	0x20, //curve 7 a
	0x00, //curve 7 b
	0x20, //curve 6 a
	0x00, //curve 6 b
	0x20, //curve 5 a
	0x00, //curve 5 b
	0x20, //curve 4 a
	0x00, //curve 4 b
	0x20, //curve 3 a
	0x00, //curve 3 b
	0x20, //curve 2 a
	0x00, //curve 2 b
	0x20, //curve 1 a
	0x00, //curve 1 b
	0x04, //cc b3 0.2
	0xb5,
	0x1f, //cc b2
	0x88,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc g3
	0xe9,
	0x04, //cc g2
	0x54,
	0x1f, //cc g1
	0xc3,
	0x1f, //cc r3
	0xe9,
	0x1f, //cc r2
	0x88,
	0x04, //cc r1
	0x8f,
};

static char DSI0_VIDEO_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A, //password 5A
	0x00, //mask 000
	0x00, //data_width
	0x23, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //roi_ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, //scr Kb
	0xFF, //scr Wb
	0x00, //scr Kg
	0xFF, //scr Wg
	0x00, //scr Kr
	0xFF, //scr Wr
	0xFF, //scr Bb
	0x00, //scr Yb
	0x00, //scr Bg
	0xFF, //scr Yg
	0x00, //scr Br
	0xFF, //scr Yr
	0x00, //scr Gb
	0xFF, //scr Mb
	0xFF, //scr Gg
	0x00, //scr Mg
	0x00, //scr Gr
	0xFF, //scr Mr
	0x00, //scr Rb
	0xFF, //scr Cb
	0x00, //scr Rg
	0xFF, //scr Cg
	0xFF, //scr Rr
	0x00, //scr Cr
	0x02, //sharpen_set cc_en gamma_en 00 0 0
	0xff, //curve24 a
	0x00, //curve24 b
	0x20, //curve23 a
	0x00, //curve23 b
	0x20, //curve22 a
	0x00, //curve22 b
	0x20, //curve21 a
	0x00, //curve21 b
	0x20, //curve20 a
	0x00, //curve20 b
	0x20, //curve19 a
	0x00, //curve19 b
	0x20, //curve18 a
	0x00, //curve18 b
	0x20, //curve17 a
	0x00, //curve17 b
	0x20, //curve16 a
	0x00, //curve16 b
	0x20, //curve15 a
	0x00, //curve15 b
	0x20, //curve14 a
	0x00, //curve14 b
	0x20, //curve13 a
	0x00, //curve13 b
	0x20, //curve12 a
	0x00, //curve12 b
	0x20, //curve11 a
	0x00, //curve11 b
	0x20, //curve10 a
	0x00, //curve10 b
	0x20, //curve 9 a
	0x00, //curve 9 b
	0x20, //curve 8 a
	0x00, //curve 8 b
	0x20, //curve 7 a
	0x00, //curve 7 b
	0x20, //curve 6 a
	0x00, //curve 6 b
	0x20, //curve 5 a
	0x00, //curve 5 b
	0x20, //curve 4 a
	0x00, //curve 4 b
	0x20, //curve 3 a
	0x00, //curve 3 b
	0x20, //curve 2 a
	0x00, //curve 2 b
	0x20, //curve 1 a
	0x00, //curve 1 b
	0x04, //cc b3 0.2
	0xb5,
	0x1f, //cc b2
	0x88,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc g3
	0xe9,
	0x04, //cc g2
	0x54,
	0x1f, //cc g1
	0xc3,
	0x1f, //cc r3
	0xe9,
	0x1f, //cc r2
	0x88,
	0x04, //cc r1
	0x8f,
};

static char DSI0_VIDEO_WARM_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A,//password 5A
	0x00,//mask 000
	0x00,//data_width
	0x33,//scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00,//roi_ctrl
	0x00,//roi1 y end
	0x00,
	0x00,//roi1 y start
	0x00,
	0x00,//roi1 x end
	0x00,
	0x00,//roi1 x start
	0x00,
	0x00,//roi0 y end
	0x00,
	0x00,//roi0 y start
	0x00,
	0x00,//roi0 x end
	0x00,
	0x00,//roi0 x start
	0x00,
	0x00,//scr Kb
	0xe0,//scr Wb
	0x00,//scr Kg
	0xf1,//scr Wg
	0x00,//scr Kr
	0xff,//scr Wr
	0xFF,//scr Bb
	0x00,//scr Yb
	0x00,//scr Bg
	0xFF,//scr Yg
	0x00,//scr Br
	0xFF,//scr Yr
	0x00,//scr Gb
	0xFF,//scr Mb
	0xFF,//scr Gg
	0x00,//scr Mg
	0x00,//scr Gr
	0xFF,//scr Mr
	0x00,//scr Rb
	0xFF,//scr Cb
	0x00,//scr Rg
	0xFF,//scr Cg
	0xFF,//scr Rr
	0x00,//scr Cr
	0x06,//sharpen_set cc_en gamma_en 00 0 0
	0x20,//curve24 a
	0x00,//curve24 b
	0x20,//curve23 a
	0x00,//curve23 b
	0x20,//curve22 a
	0x00,//curve22 b
	0x20,//curve21 a
	0x00,//curve21 b
	0x20,//curve20 a
	0x00,//curve20 b
	0x20,//curve19 a
	0x00,//curve19 b
	0x20,//curve18 a
	0x00,//curve18 b
	0x20,//curve17 a
	0x00,//curve17 b
	0x20,//curve16 a
	0x00,//curve16 b
	0x20,//curve15 a
	0x00,//curve15 b
	0x20,//curve14 a
	0x00,//curve14 b
	0x20,//curve13 a
	0x00,//curve13 b
	0x20,//curve12 a
	0x00,//curve12 b
	0x20,//curve11 a
	0x00,//curve11 b
	0x20,//curve10 a
	0x00,//curve10 b
	0x20,//curve 9 a
	0x00,//curve 9 b
	0x20,//curve 8 a
	0x00,//curve 8 b
	0x20,//curve 7 a
	0x00,//curve 7 b
	0x20,//curve 6 a
	0x00,//curve 6 b
	0x20,//curve 5 a
	0x00,//curve 5 b
	0x20,//curve 4 a
	0x00,//curve 4 b
	0x20,//curve 3 a
	0x00,//curve 3 b
	0x20,//curve 2 a
	0x00,//curve 2 b
	0x20,//curve 1 a
	0x00,//curve 1 b
	0x05,//cc b3 0.3
	0x10,
	0x1f,//cc b2
	0x4c,
	0x1f,//cc b1
	0xa4,
	0x1f,//cc g3
	0xdd,
	0x04,//cc g2
	0x7f,
	0x1f,//cc g1
	0xa4,
	0x1f,//cc r3
	0xdd,
	0x1f,//cc r2
	0x4c,
	0x04,//cc r1
	0xd7,
};

static char DSI0_VIDEO_COLD_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A,//password 5A
	0x00,//mask 000
	0x00,//data_width
	0x33,//scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00,//roi_ctrl
	0x00,//roi1 y end
	0x00,
	0x00,//roi1 y start
	0x00,
	0x00,//roi1 x end
	0x00,
	0x00,//roi1 x start
	0x00,
	0x00,//roi0 y end
	0x00,
	0x00,//roi0 y start
	0x00,
	0x00,//roi0 x end
	0x00,
	0x00,//roi0 x start
	0x00,
	0x00,//scr Kb
	0xff,//scr Wb
	0x00,//scr Kg
	0xe9,//scr Wg
	0x00,//scr Kr
	0xe2,//scr Wr
	0xFF,//scr Bb
	0x00,//scr Yb
	0x00,//scr Bg
	0xFF,//scr Yg
	0x00,//scr Br
	0xFF,//scr Yr
	0x00,//scr Gb
	0xFF,//scr Mb
	0xFF,//scr Gg
	0x00,//scr Mg
	0x00,//scr Gr
	0xFF,//scr Mr
	0x00,//scr Rb
	0xFF,//scr Cb
	0x00,//scr Rg
	0xFF,//scr Cg
	0xFF,//scr Rr
	0x00,//scr Cr
	0x06,//sharpen_set cc_en gamma_en 00 0 0
	0x20,//curve24 a
	0x00,//curve24 b
	0x20,//curve23 a
	0x00,//curve23 b
	0x20,//curve22 a
	0x00,//curve22 b
	0x20,//curve21 a
	0x00,//curve21 b
	0x20,//curve20 a
	0x00,//curve20 b
	0x20,//curve19 a
	0x00,//curve19 b
	0x20,//curve18 a
	0x00,//curve18 b
	0x20,//curve17 a
	0x00,//curve17 b
	0x20,//curve16 a
	0x00,//curve16 b
	0x20,//curve15 a
	0x00,//curve15 b
	0x20,//curve14 a
	0x00,//curve14 b
	0x20,//curve13 a
	0x00,//curve13 b
	0x20,//curve12 a
	0x00,//curve12 b
	0x20,//curve11 a
	0x00,//curve11 b
	0x20,//curve10 a
	0x00,//curve10 b
	0x20,//curve 9 a
	0x00,//curve 9 b
	0x20,//curve 8 a
	0x00,//curve 8 b
	0x20,//curve 7 a
	0x00,//curve 7 b
	0x20,//curve 6 a
	0x00,//curve 6 b
	0x20,//curve 5 a
	0x00,//curve 5 b
	0x20,//curve 4 a
	0x00,//curve 4 b
	0x20,//curve 3 a
	0x00,//curve 3 b
	0x20,//curve 2 a
	0x00,//curve 2 b
	0x20,//curve 1 a
	0x00,//curve 1 b
	0x05,//cc b3 0.3
	0x10,
	0x1f,//cc b2
	0x4c,
	0x1f,//cc b1
	0xa4,
	0x1f,//cc g3
	0xdd,
	0x04,//cc g2
	0x7f,
	0x1f,//cc g1
	0xa4,
	0x1f,//cc r3
	0xdd,
	0x1f,//cc r2
	0x4c,
	0x04,//cc r1
	0xd7,
};

static char DSI0_CAMERA_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A, //password 5A
	0x00, //mask 000
	0x00, //data_width
	0x23, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //roi_ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, //scr Kb
	0xFF, //scr Wb
	0x00, //scr Kg
	0xFF, //scr Wg
	0x00, //scr Kr
	0xFF, //scr Wr
	0xFF, //scr Bb
	0x00, //scr Yb
	0x00, //scr Bg
	0xFF, //scr Yg
	0x00, //scr Br
	0xFF, //scr Yr
	0x00, //scr Gb
	0xFF, //scr Mb
	0xFF, //scr Gg
	0x00, //scr Mg
	0x00, //scr Gr
	0xFF, //scr Mr
	0x00, //scr Rb
	0xFF, //scr Cb
	0x00, //scr Rg
	0xFF, //scr Cg
	0xFF, //scr Rr
	0x00, //scr Cr
	0x02, //sharpen_set cc_en gamma_en 00 0 0
	0xff, //curve24 a
	0x00, //curve24 b
	0x20, //curve23 a
	0x00, //curve23 b
	0x20, //curve22 a
	0x00, //curve22 b
	0x20, //curve21 a
	0x00, //curve21 b
	0x20, //curve20 a
	0x00, //curve20 b
	0x20, //curve19 a
	0x00, //curve19 b
	0x20, //curve18 a
	0x00, //curve18 b
	0x20, //curve17 a
	0x00, //curve17 b
	0x20, //curve16 a
	0x00, //curve16 b
	0x20, //curve15 a
	0x00, //curve15 b
	0x20, //curve14 a
	0x00, //curve14 b
	0x20, //curve13 a
	0x00, //curve13 b
	0x20, //curve12 a
	0x00, //curve12 b
	0x20, //curve11 a
	0x00, //curve11 b
	0x20, //curve10 a
	0x00, //curve10 b
	0x20, //curve 9 a
	0x00, //curve 9 b
	0x20, //curve 8 a
	0x00, //curve 8 b
	0x20, //curve 7 a
	0x00, //curve 7 b
	0x20, //curve 6 a
	0x00, //curve 6 b
	0x20, //curve 5 a
	0x00, //curve 5 b
	0x20, //curve 4 a
	0x00, //curve 4 b
	0x20, //curve 3 a
	0x00, //curve 3 b
	0x20, //curve 2 a
	0x00, //curve 2 b
	0x20, //curve 1 a
	0x00, //curve 1 b
	0x04, //cc b3 0.2
	0xb5,
	0x1f, //cc b2
	0x88,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc g3
	0xe9,
	0x04, //cc g2
	0x54,
	0x1f, //cc g1
	0xc3,
	0x1f, //cc r3
	0xe9,
	0x1f, //cc r2
	0x88,
	0x04, //cc r1
	0x8f,
};

static char DSI0_GALLERY_MDNIE_CMDS[] ={
	//start
	0xE6,
	0x5A, //password 5A
	0x00, //mask 000
	0x00, //data_width
	0x23, //scr_roi 1 scr algo_roi 1 algo 00 1 0 00 1 0
	0x00, //roi_ctrl
	0x00, //roi1 y end
	0x00,
	0x00, //roi1 y start
	0x00,
	0x00, //roi1 x end
	0x00,
	0x00, //roi1 x start
	0x00,
	0x00, //roi0 y end
	0x00,
	0x00, //roi0 y start
	0x00,
	0x00, //roi0 x end
	0x00,
	0x00, //roi0 x start
	0x00,
	0x00, //scr Kb
	0xFF, //scr Wb
	0x00, //scr Kg
	0xFF, //scr Wg
	0x00, //scr Kr
	0xFF, //scr Wr
	0xFF, //scr Bb
	0x00, //scr Yb
	0x00, //scr Bg
	0xFF, //scr Yg
	0x00, //scr Br
	0xFF, //scr Yr
	0x00, //scr Gb
	0xFF, //scr Mb
	0xFF, //scr Gg
	0x00, //scr Mg
	0x00, //scr Gr
	0xFF, //scr Mr
	0x00, //scr Rb
	0xFF, //scr Cb
	0x00, //scr Rg
	0xFF, //scr Cg
	0xFF, //scr Rr
	0x00, //scr Cr
	0x02, //sharpen_set cc_en gamma_en 00 0 0
	0xff, //curve24 a
	0x00, //curve24 b
	0x20, //curve23 a
	0x00, //curve23 b
	0x20, //curve22 a
	0x00, //curve22 b
	0x20, //curve21 a
	0x00, //curve21 b
	0x20, //curve20 a
	0x00, //curve20 b
	0x20, //curve19 a
	0x00, //curve19 b
	0x20, //curve18 a
	0x00, //curve18 b
	0x20, //curve17 a
	0x00, //curve17 b
	0x20, //curve16 a
	0x00, //curve16 b
	0x20, //curve15 a
	0x00, //curve15 b
	0x20, //curve14 a
	0x00, //curve14 b
	0x20, //curve13 a
	0x00, //curve13 b
	0x20, //curve12 a
	0x00, //curve12 b
	0x20, //curve11 a
	0x00, //curve11 b
	0x20, //curve10 a
	0x00, //curve10 b
	0x20, //curve 9 a
	0x00, //curve 9 b
	0x20, //curve 8 a
	0x00, //curve 8 b
	0x20, //curve 7 a
	0x00, //curve 7 b
	0x20, //curve 6 a
	0x00, //curve 6 b
	0x20, //curve 5 a
	0x00, //curve 5 b
	0x20, //curve 4 a
	0x00, //curve 4 b
	0x20, //curve 3 a
	0x00, //curve 3 b
	0x20, //curve 2 a
	0x00, //curve 2 b
	0x20, //curve 1 a
	0x00, //curve 1 b
	0x04, //cc b3 0.2
	0xb5,
	0x1f, //cc b2
	0x88,
	0x1f, //cc b1
	0xc3,
	0x1f, //cc g3
	0xe9,
	0x04, //cc g2
	0x54,
	0x1f, //cc g1
	0xc3,
	0x1f, //cc r3
	0xe9,
	0x1f, //cc r2
	0x88,
	0x04, //cc r1
	0x8f,
};

static struct dsi_cmd_desc DSI0_NEGATIVE_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_NEGATIVE_MDNIE_CMDS)}, DSI0_NEGATIVE_MDNIE_CMDS},
};

///////////////////////////////////////////////////////////////////////////////////

static struct dsi_cmd_desc DSI0_UI_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_UI_MDNIE_CMDS)}, DSI0_UI_MDNIE_CMDS},
};

static struct dsi_cmd_desc DSI0_VIDEO_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_VIDEO_MDNIE_CMDS)}, DSI0_VIDEO_MDNIE_CMDS},
};

static struct dsi_cmd_desc DSI0_VIDEO_WARM_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_VIDEO_WARM_MDNIE_CMDS)}, DSI0_VIDEO_WARM_MDNIE_CMDS},
};

static struct dsi_cmd_desc DSI0_VIDEO_COLD_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_VIDEO_COLD_MDNIE_CMDS)}, DSI0_VIDEO_COLD_MDNIE_CMDS},
};

static struct dsi_cmd_desc DSI0_CAMERA_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_CAMERA_MDNIE_CMDS)}, DSI0_CAMERA_MDNIE_CMDS},
};

static struct dsi_cmd_desc DSI0_GALLERY_MDNIE[] = {
	{{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(DSI0_GALLERY_MDNIE_CMDS)}, DSI0_GALLERY_MDNIE_CMDS},
};

static struct dsi_cmd_desc *mdnie_tune_value_dsi0[MAX_APP_MODE][MAX_MODE][MAX_OUTDOOR_MODE] = {
		/*
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
			MAX_APP_MODE,
		*/

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
			{DSI0_UI_MDNIE,	NULL},
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
			{DSI0_VIDEO_MDNIE,	NULL},
		},
		//VIDEO_WARM_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		//VIDEO_COLD_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// CAMERA_APP
		{
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
			{DSI0_CAMERA_MDNIE,	NULL},
		},
		// NAVI_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// GALLERY_APP
		{
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
			{DSI0_GALLERY_MDNIE,	NULL},
		},
		// VT_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// BROWSER_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// eBOOK_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
		// EMAIL_APP
		{
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
			{DSI0_UI_MDNIE,	NULL},
		},
};

#endif /*_DSI_MDNIE_LITE_DATA_WVGA_HX8369B_H_*/
