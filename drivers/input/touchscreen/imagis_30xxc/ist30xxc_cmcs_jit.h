/*
 *  Copyright (C) 2010, Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __IST30XXC_CMCS_JIT_H__
#define __IST30XXC_CMCS_JIT_H__

#define CMCS_MSG_MASK				(0xFFFFFFF0)
#define CM_MSG_VALID				(0x0E7DC770)
#define CS_MSG_VALID				(0x0E7DC500)

#define IST30XX_CMCS_JIT_NAME		"ist30xxc_jit.cms"
#define IST30XX_CMCS_JIT_MAGIC		"CMCS2TAG"

struct CMCS_JIT_SPEC_NODE {
    u32 node_cnt;
    u16 *buf_min;
    u16 *buf_max;
};

struct CMCS_JIT_SPEC_TOTAL {
	s16 screen_min;
	s16 screen_max;
	s16 gtx_min;
	s16 gtx_max;
	s16 key_min;
	s16 key_max;
};

struct CMCS_JIT_ITEM_INFO {
    char name[8];
    u32 addr;
    u32 size;
    char data_type[2];
    char spec_type[2];
};

typedef struct _CMCS_JIT_ITEM {
    u32 cnt;
    struct CMCS_JIT_ITEM_INFO *item;
} CMCS_JIT_ITEM;

struct CMCS_JIT_CMD_INFO {
    u32 addr;
    u32 value;
};

typedef struct _CMCS_JIT_CMD {
    u32 cnt;
    struct CMCS_JIT_CMD_INFO *cmd;
} CMCS_JIT_CMD;

union CMCS_JIT_SPEC_ITEM {
    struct CMCS_JIT_SPEC_NODE spec_node;
    struct CMCS_JIT_SPEC_TOTAL spec_total;
};

struct CMCS_JIT_SPEC_SLOPE {
    char name[8];
    s16	x_min;
	s16	x_max;
	s16	gtx_x_min;
	s16	gtx_x_max;
    s16	y_min;
	s16	y_max;
	s16	gtx_y_min;
	s16	gtx_y_max;
	s16	key_min;
	s16	key_max;
};

struct CMCS_JIT_REG_INFO {
    char name[8];
    u32 addr;
    u32 size;
};

typedef struct _CMCS_JIT_PARAM {
	u32 cmcs_size_addr;
	u32 cmcs_size;
	u32 enable_addr;
	u32 checksum_addr;
	u32 end_notify_addr;
	u32 sensor1_addr;
	u32 sensor2_addr;
	u32 sensor3_addr;
	u32 cm_sensor1_size;
	u32 cm_sensor2_size;
	u32	cm_sensor3_size;
	u32	cs_sensor1_size;
	u32	cs_sensor2_size;
	u32 cs_sensor3_size;
    u32 cmcs_chksum;
    u32 cm_sensor_chksum;
    u32 cs_sensor_chksum;
} CMCS_JIT_PARAM;

typedef struct _CMCS_JIT_BIN_INFO {
	char magic1[8];
    CMCS_JIT_ITEM items;
    CMCS_JIT_CMD cmds;
    struct CMCS_JIT_SPEC_SLOPE spec_slope;
    struct CMCS_JIT_SPEC_TOTAL spec_cr;
    CMCS_JIT_PARAM param;
    union CMCS_JIT_SPEC_ITEM *spec_item;
    u8 *buf_cmcs;
	u32 *buf_cm_sensor;
	u32 *buf_cs_sensor;
	char magic2[8];
} CMCS_JIT_BIN_INFO;

typedef struct _CMCS_JIT_BUF {
    s16 cm[IST30XX_NODE_TOTAL_NUM];
	s16	cm_jit[IST30XX_NODE_TOTAL_NUM];
	s16 spec_min[IST30XX_NODE_TOTAL_NUM];
    s16 spec_max[IST30XX_NODE_TOTAL_NUM];
	s16 slope0[IST30XX_NODE_TOTAL_NUM];
	s16 slope1[IST30XX_NODE_TOTAL_NUM];
	s16 cs[IST30XX_NODE_TOTAL_NUM];
} CMCS_JIT_BUF;

int ist30xx_init_cmcs_jit_sysfs(struct ist30xx_data *data);
#endif  // __IST30XXC_CMCS_H__
