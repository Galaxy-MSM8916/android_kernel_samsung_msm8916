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

#ifndef __IST30XXC_CMCS_H__
#define __IST30XXC_CMCS_H__

#define CMCS_PARSING_DEBUG          (0)
#define CMCS_RAMCODE_READ           (0)

#define CMCS_FLAG_CM                (1)
#define CMCS_FLAG_CM_SPEC           (1 << 1)
#define CMCS_FLAG_CM_SLOPE0         (1 << 2)
#define CMCS_FLAG_CM_SLOPE1         (1 << 3)
#define CMCS_FLAG_CS                (1 << 4)

#define CMCS_READY                  (0)
#define CMCS_NOT_READY              (-1)

#define IST30XX_CMCS_MSG_VALID      (0x8FAB0FAB)
#define IST30XX_CMCS_TIMEOUT        (10000) // unit : msec

#define IST30XX_CMCS_CM             ("CM")
#define IST30XX_CMCS_CS             ("CS")

// CMCS addr
#define IST30XX_CMCS_CHECKSUM       IST30XX_DA_ADDR(0x300B0100)
#define IST30XX_CMCS_CS_CHECKSUM    IST30XX_DA_ADDR(0x300B0108)
#define IST30XX_CMCS_PATTERN        IST30XX_DA_ADDR(0x300B0104)

#define ENABLE_CM_MODE(n)           (n & 1)
#define ENABLE_CS_MODE(n)           ((n >> 1) & 1)

#define IST30XX_CMCS_NAME           "ist30xxc.cms"
#define IST30XX_CMCS_MAGIC          "CMCS2TAG"

struct CMCS_SPEC_NODE {
    u32 node_cnt;
    u16 *buf_min;
    u16 *buf_max;
};

struct CMCS_SPEC_TOTAL {
	u16	screen_min;
	u16	screen_max;
	u16	gtx_min;
	u16	gtx_max;
	u16	key_min;
	u16	key_max;
};

struct CMCS_ITEM_INFO {
    char name[8];
    u32 addr;
    u32 size;
    char data_type[2];
    char spec_type[2];
};

typedef struct _CMCS_ITEM {
    u32 cnt;
    struct CMCS_ITEM_INFO *item;
} CMCS_ITEM;

struct CMCS_CMD_INFO {
    u32 addr;
    u32 value;
};

typedef struct _CMCS_CMD {
    u32 cnt;
    struct CMCS_CMD_INFO *cmd;
} CMCS_CMD;

union CMCS_SPEC_ITEM {
    struct CMCS_SPEC_NODE spec_node;
    struct CMCS_SPEC_TOTAL spec_total;
};

struct CMCS_SPEC_SLOPE {
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

struct CMCS_REG_INFO {
    char name[8];
    u32 addr;
    u32 size;
};

typedef struct _CMCS_PARAM {
    u32 cmcs_size_addr;
    u32 cmcs_size;
    u32 cm_sensor1_addr;
    u32 cm_sensor1_size;
    u32 cm_sensor2_addr;
    u32 cm_sensor2_size;
    u32 cm_sensor3_addr;
    u32 cm_sensor3_size;
    u32 cs_sensor1_addr;
    u32 cs_sensor1_size;
    u32 cs_sensor2_addr;
    u32 cs_sensor2_size;
    u32 cs_sensor3_addr;
    u32 cs_sensor3_size;
    u32 cmcs_chksum;
    u32 cm_sensor_chksum;
    u32 cs_sensor_chksum;
} CMCS_PARAM;

typedef struct _CMCS_BIN_INFO {
	char		            magic1[8];
    CMCS_ITEM	            items;
    CMCS_CMD                cmds;
    struct CMCS_SPEC_SLOPE  spec_slope;
    struct CMCS_SPEC_TOTAL  spec_cr;
    CMCS_PARAM              param;
    union CMCS_SPEC_ITEM    *spec_item;
    u8                      *buf_cmcs;
    u32                     *buf_sensor;
	char		            magic2[8];
} CMCS_BIN_INFO;

typedef struct _CMCS_BUF {
	s16	cm[IST30XX_NODE_TOTAL_NUM];
	s16	spec_min[IST30XX_NODE_TOTAL_NUM];
    s16 spec_max[IST30XX_NODE_TOTAL_NUM];
	s16	slope0[IST30XX_NODE_TOTAL_NUM];
	s16	slope1[IST30XX_NODE_TOTAL_NUM];
	s16	cs[IST30XX_NODE_TOTAL_NUM];
} CMCS_BUF;

int check_tsp_type(struct ist30xx_data *data, int tx, int rx);
int ist30xx_get_cmcs_info(const u8 *buf, const u32 size);
int ist30xx_cmcs_test(struct ist30xx_data *data, const u8 *buf, int size);
int ist30xx_init_cmcs_sysfs(struct ist30xx_data *data);
#endif  // __IST30XXC_CMCS_H__
