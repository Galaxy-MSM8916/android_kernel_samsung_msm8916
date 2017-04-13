/*
 *  Copyright (C) 2010,Imagis Technology Co. Ltd. All Rights Reserved.
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

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/err.h>

#include "ist30xxc.h"
#include "ist30xxc_cmcs_jit.h"
#include "ist30xxc_cmcs_jit_bin.h"

#define TSP_CH_UNUSED               (0)
#define TSP_CH_SCREEN               (1)
#define TSP_CH_GTX                  (2)
#define TSP_CH_KEY                  (3)
#define TSP_CH_UNKNOWN              (-1)

#define CMCS_JIT_FLAG_CM            (1 << 0)
#define CMCS_JIT_FLAG_CM_SPEC       (1 << 1)
#define CMCS_JIT_FLAG_CM_SLOPE0     (1 << 2)
#define CMCS_JIT_FLAG_CM_SLOPE1     (1 << 3)
#define CMCS_JIT_FLAG_CMJIT         (1 << 4)
#define CMCS_JIT_FLAG_CS            (1 << 8)

#define CMCS_READY                  (0)
#define CMCS_NOT_READY              (-1)

#define CMCS_TIMEOUT                (10000) // unit : msec

#define CMCS_CMJIT                  ("CMJIT")
#define CMCS_CM                     ("CM")
#define CMCS_CS                     ("CS")

#define CMCS_PARSING_DEBUG          (0)
#define CMCS_TAGS_PARSE_OK          (0)

int cmcs_jit_ready = CMCS_READY;
u8 *ts_cmcs_jit_bin = NULL;
u32 ts_cmcs_jit_bin_size = 0;
CMCS_JIT_BIN_INFO ist30xx_cmcs_jit_bin;
CMCS_JIT_BIN_INFO *ts_cmcs_jit = (CMCS_JIT_BIN_INFO *)&ist30xx_cmcs_jit_bin;
CMCS_JIT_BUF ist30xx_cmcs_jit_buf;
CMCS_JIT_BUF *ts_cmcs_jit_buf = (CMCS_JIT_BUF *)&ist30xx_cmcs_jit_buf;

int ist30xx_parse_cmcs_jit_bin(const u8 *buf, const u32 size)
{
	int ret = -EPERM;
	int i;
	int node_spec_cnt;

	memcpy(ts_cmcs_jit->magic1, buf, sizeof(ts_cmcs_jit->magic1));
	memcpy(ts_cmcs_jit->magic2, &buf[size - sizeof(ts_cmcs_jit->magic2)],
	       sizeof(ts_cmcs_jit->magic2));

	if (!strncmp(ts_cmcs_jit->magic1, IST30XX_CMCS_JIT_MAGIC,
            sizeof(ts_cmcs_jit->magic1))
	    && !strncmp(ts_cmcs_jit->magic2, IST30XX_CMCS_JIT_MAGIC,
			sizeof(ts_cmcs_jit->magic2))) {
		int idx;

		idx = sizeof(ts_cmcs_jit->magic1);

		memcpy(&ts_cmcs_jit->items.cnt, &buf[idx],
            sizeof(ts_cmcs_jit->items.cnt));
		idx += sizeof(ts_cmcs_jit->items.cnt);
		ts_cmcs_jit->items.item = kmalloc(
			sizeof(struct CMCS_JIT_ITEM_INFO) * ts_cmcs_jit->items.cnt, GFP_KERNEL);
		for (i = 0; i < ts_cmcs_jit->items.cnt; i++) {
			memcpy(&ts_cmcs_jit->items.item[i], &buf[idx],
			       sizeof(struct CMCS_JIT_ITEM_INFO));
			idx += sizeof(struct CMCS_JIT_ITEM_INFO);
		}

		memcpy(&ts_cmcs_jit->cmds.cnt, &buf[idx],
            sizeof(ts_cmcs_jit->cmds.cnt));
		idx += sizeof(ts_cmcs_jit->cmds.cnt);
		ts_cmcs_jit->cmds.cmd = kmalloc(
			sizeof(struct CMCS_JIT_CMD_INFO) * ts_cmcs_jit->cmds.cnt, GFP_KERNEL);
		for (i = 0; i < ts_cmcs_jit->cmds.cnt; i++) {
			memcpy(&ts_cmcs_jit->cmds.cmd[i], &buf[idx],
			       sizeof(struct CMCS_JIT_CMD_INFO));
			idx += sizeof(struct CMCS_JIT_CMD_INFO);
		}

		memcpy(&ts_cmcs_jit->spec_slope, &buf[idx],
            sizeof(ts_cmcs_jit->spec_slope));
		idx += sizeof(ts_cmcs_jit->spec_slope);

		memcpy(&ts_cmcs_jit->spec_cr, &buf[idx], sizeof(ts_cmcs_jit->spec_cr));
		idx += sizeof(ts_cmcs_jit->spec_cr);

		memcpy(&ts_cmcs_jit->param, &buf[idx], sizeof(ts_cmcs_jit->param));
		idx += sizeof(ts_cmcs_jit->param);

		ts_cmcs_jit->spec_item = kmalloc(
			sizeof(struct CMCS_JIT_SPEC_TOTAL) * ts_cmcs_jit->items.cnt,
			GFP_KERNEL);
		for (i = 0; i < ts_cmcs_jit->items.cnt; i++) {
			if (!strncmp(ts_cmcs_jit->items.item[i].spec_type, "N", 64)) {
				memcpy(&node_spec_cnt, &buf[idx], sizeof(node_spec_cnt));
				ts_cmcs_jit->spec_item[i].spec_node.node_cnt = node_spec_cnt;
				idx += sizeof(node_spec_cnt);
				ts_cmcs_jit->spec_item[i].spec_node.buf_min = (u16 *)&buf[idx];
				idx += node_spec_cnt * sizeof(s16);
				ts_cmcs_jit->spec_item[i].spec_node.buf_max = (u16 *)&buf[idx];
				idx += node_spec_cnt * sizeof(s16);
			} else if (!strncmp(ts_cmcs_jit->items.item[i].spec_type, "T", 64)) {
				memcpy(&ts_cmcs_jit->spec_item[i].spec_total, &buf[idx],
				       sizeof(struct CMCS_JIT_SPEC_TOTAL));
				idx += sizeof(struct CMCS_JIT_SPEC_TOTAL);
			}
		}

		ts_cmcs_jit->buf_cmcs = (u8 *)&buf[idx];
		idx += ts_cmcs_jit->param.cmcs_size;

		ts_cmcs_jit->buf_cm_sensor = (u32 *)&buf[idx];
		idx += ts_cmcs_jit->param.cm_sensor1_size
               + ts_cmcs_jit->param.cm_sensor2_size
		       + ts_cmcs_jit->param.cm_sensor3_size;

		ts_cmcs_jit->buf_cs_sensor = (u32 *)&buf[idx];

		ret = 0;
	}

	tsp_verb("Magic1: %s, Magic2: %s\n", ts_cmcs_jit->magic1, 
        ts_cmcs_jit->magic2);
	tsp_verb(" item(%d)\n", ts_cmcs_jit->items.cnt);
	for (i = 0; i < ts_cmcs_jit->items.cnt; i++) {
		tsp_verb(" (%d): %s, 0x%08x, %d, %s, %s\n", i,
             ts_cmcs_jit->items.item[i].name, ts_cmcs_jit->items.item[i].addr,
			 ts_cmcs_jit->items.item[i].size, 
             ts_cmcs_jit->items.item[i].data_type,
			 ts_cmcs_jit->items.item[i].spec_type);
	}
	tsp_verb(" cmd(%d)\n", ts_cmcs_jit->cmds.cnt);
	for (i = 0; i < ts_cmcs_jit->cmds.cnt; i++)
		tsp_verb(" (%d): 0x%08x, 0x%08x\n",
			 i, ts_cmcs_jit->cmds.cmd[i].addr, ts_cmcs_jit->cmds.cmd[i].value);
	tsp_verb(" param\n");
	tsp_verb("  fw: 0x%08x, %d\n", ts_cmcs_jit->param.cmcs_size_addr,
		 ts_cmcs_jit->param.cmcs_size);
	tsp_verb("  enable: 0x%08x\n", ts_cmcs_jit->param.enable_addr);
	tsp_verb("  checksum: 0x%08x\n", ts_cmcs_jit->param.checksum_addr);
	tsp_verb("  endnotify: 0x%08x\n", ts_cmcs_jit->param.end_notify_addr);
	tsp_verb("  cm sensor1: 0x%08x, %d\n", ts_cmcs_jit->param.sensor1_addr,
		 ts_cmcs_jit->param.cm_sensor1_size);
	tsp_verb("  cm sensor2: 0x%08x, %d\n", ts_cmcs_jit->param.sensor2_addr,
		 ts_cmcs_jit->param.cm_sensor2_size);
	tsp_verb("  cm sensor3: 0x%08x, %d\n", ts_cmcs_jit->param.sensor3_addr,
		 ts_cmcs_jit->param.cm_sensor3_size);
	tsp_verb("  cs sensor1: 0x%08x, %d\n", ts_cmcs_jit->param.sensor1_addr,
		 ts_cmcs_jit->param.cs_sensor1_size);
	tsp_verb("  cs sensor2: 0x%08x, %d\n", ts_cmcs_jit->param.sensor2_addr,
		 ts_cmcs_jit->param.cs_sensor2_size);
	tsp_verb("  cs sensor3: 0x%08x, %d\n", ts_cmcs_jit->param.sensor3_addr,
		 ts_cmcs_jit->param.cs_sensor3_size);
	tsp_verb("  chksum: 0x%08x\n", ts_cmcs_jit->param.cmcs_chksum,
		 ts_cmcs_jit->param.cm_sensor_chksum,
         ts_cmcs_jit->param.cs_sensor_chksum);
	tsp_verb(" slope(%s)\n", ts_cmcs_jit->spec_slope.name);
	tsp_verb("  x(%d,%d),y(%d,%d),gtx_x(%d,%d),gtx_y(%d,%d),key(%d,%d)\n",
		 ts_cmcs_jit->spec_slope.x_min, ts_cmcs_jit->spec_slope.x_max,
		 ts_cmcs_jit->spec_slope.y_min, ts_cmcs_jit->spec_slope.y_max,
		 ts_cmcs_jit->spec_slope.gtx_x_min, ts_cmcs_jit->spec_slope.gtx_x_max,
		 ts_cmcs_jit->spec_slope.gtx_y_min, ts_cmcs_jit->spec_slope.gtx_y_max,
		 ts_cmcs_jit->spec_slope.key_min, ts_cmcs_jit->spec_slope.key_max);
	tsp_verb(" cr: screen(%4d, %4d), gtx(%4d, %4d), key(%4d, %4d)\n",
		 ts_cmcs_jit->spec_cr.screen_min, ts_cmcs_jit->spec_cr.screen_max,
		 ts_cmcs_jit->spec_cr.gtx_min, ts_cmcs_jit->spec_cr.gtx_max,
		 ts_cmcs_jit->spec_cr.key_min, ts_cmcs_jit->spec_cr.key_max);
	for (i = 0; i < ts_cmcs_jit->items.cnt; i++) {
		if (!strncmp(ts_cmcs_jit->items.item[i].spec_type, "N", 64)) {
			tsp_verb(" %s\n", ts_cmcs_jit->items.item[i].name);
			tsp_verb(" min: %x, %x, %x, %x\n",
				 ts_cmcs_jit->spec_item[i].spec_node.buf_min[0],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_min[1],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_min[2],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_min[3]);
			tsp_verb(" max: %x, %x, %x, %x\n",
				 ts_cmcs_jit->spec_item[i].spec_node.buf_max[0],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_max[1],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_max[2],
				 ts_cmcs_jit->spec_item[i].spec_node.buf_max[3]);
		} else if (!strncmp(ts_cmcs_jit->items.item[i].spec_type, "T", 64)) {
			tsp_verb(" %s: screen(%4d, %4d), gtx(%4d, %4d), key(%4d, %4d)\n",
				 ts_cmcs_jit->items.item[i].name,
				 ts_cmcs_jit->spec_item[i].spec_total.screen_min,
				 ts_cmcs_jit->spec_item[i].spec_total.screen_max,
				 ts_cmcs_jit->spec_item[i].spec_total.gtx_min,
				 ts_cmcs_jit->spec_item[i].spec_total.gtx_max,
				 ts_cmcs_jit->spec_item[i].spec_total.key_min,
				 ts_cmcs_jit->spec_item[i].spec_total.key_max);
		}
	}
	tsp_verb(" cmcs: %x, %x, %x, %x\n", ts_cmcs_jit->buf_cmcs[0],
		 ts_cmcs_jit->buf_cmcs[1], ts_cmcs_jit->buf_cmcs[2],
         ts_cmcs_jit->buf_cmcs[3]);
	tsp_verb(" cm sensor: %x, %x, %x, %x\n",
		 ts_cmcs_jit->buf_cm_sensor[0], ts_cmcs_jit->buf_cm_sensor[1],
		 ts_cmcs_jit->buf_cm_sensor[2], ts_cmcs_jit->buf_cm_sensor[3]);
	tsp_verb(" cs sensor: %x, %x, %x, %x\n",
		 ts_cmcs_jit->buf_cs_sensor[0], ts_cmcs_jit->buf_cs_sensor[1],
		 ts_cmcs_jit->buf_cs_sensor[2], ts_cmcs_jit->buf_cs_sensor[3]);

	return ret;
}

int ist30xx_get_cmcs_jit_info(const u8 *buf, const u32 size)
{
	int ret;

	cmcs_jit_ready = CMCS_NOT_READY;

	ret = ist30xx_parse_cmcs_jit_bin(buf, size);
	if (unlikely(ret != CMCS_TAGS_PARSE_OK))
		tsp_warn("Cannot find tags of CMCS, make a bin by 'cmcs2bin.exe'\n");

	return ret;
}

int ist30xx_set_cmcs_jit_sensor(struct i2c_client *client, CMCS_JIT_PARAM param,
			    u32 *buf32, int mode)
{
	int ret;
	int len = 0;
	u32 waddr;

	if (mode == CMCS_JIT_FLAG_CM) {
		waddr = IST30XX_DA_ADDR(param.sensor1_addr);
		len = (param.cm_sensor1_size / IST30XX_DATA_LEN) - 2;
		buf32 += 2;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cm sensor reg1 loaded!\n");
		}

		buf32 += len;
		waddr = IST30XX_DA_ADDR(param.sensor2_addr);
		len = param.cm_sensor2_size / IST30XX_DATA_LEN;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cm sensor reg2 loaded!\n");
		}

		buf32 += len;
		waddr = IST30XX_DA_ADDR(param.sensor3_addr);
		len = param.cm_sensor3_size / IST30XX_DATA_LEN;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cm sensor reg3 loaded!\n");
		}
	} else if (mode == CMCS_JIT_FLAG_CS) {
		waddr = IST30XX_DA_ADDR(param.sensor1_addr);
		len = (param.cs_sensor1_size / IST30XX_DATA_LEN) - 2;
		buf32 += 2;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cs sensor reg1 loaded!\n");
		}

		buf32 += len;
		waddr = IST30XX_DA_ADDR(param.sensor2_addr);
		len = param.cs_sensor2_size / IST30XX_DATA_LEN;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cs sensor reg2 loaded!\n");
		}

		buf32 += len;
		waddr = IST30XX_DA_ADDR(param.sensor3_addr);
		len = param.cs_sensor3_size / IST30XX_DATA_LEN;
		tsp_verb("%08x %08x %08x %08x\n",
			 buf32[0], buf32[1], buf32[2], buf32[3]);
		tsp_verb("%08x(%d)\n", waddr, len);

		if (len > 0) {
			ret = ist30xx_burst_write(client, waddr, buf32, len);
			if (ret)
				return ret;

			tsp_info("cs sensor reg3 loaded!\n");
		}
	}

	return 0;
}

int ist30xx_set_cmcs_jit_cmd(struct i2c_client *client, CMCS_JIT_CMD cmds)
{
	int ret;
	int i;
	u32 val;
	u32 waddr;

	for (i = 0; i < cmds.cnt; i++) {
		waddr = IST30XX_DA_ADDR(cmds.cmd[i].addr);
		val = cmds.cmd[i].value;
		ret = ist30xx_write_cmd(client, waddr, val);
		if (ret)
			return ret;
		tsp_verb("cmd%d(0x%08x): 0x%08x\n", i, waddr, val);
	}

	return 0;
}

int ist30xx_parse_cmcs_jit_buf(struct ist30xx_data *data, s16 *buf)
{
	int i, j;
	TSP_INFO *tsp = &data->tsp_info;

	tsp_info(" %d * %d\n", tsp->ch_num.tx, tsp->ch_num.rx);
	for (i = 0; i < tsp->ch_num.tx; i++) {
		tsp_info(" ");
		for (j = 0; j < tsp->ch_num.rx; j++)
			printk("%5d ", buf[i * tsp->ch_num.rx + j]);
		printk("\n");
	}

	return 0;
}

int check_ts_type(struct ist30xx_data *data, int tx, int rx)
{
	int i;
	TSP_INFO *tsp = &data->tsp_info;
	TKEY_INFO *tkey = &data->tkey_info;

	if ((tx >= tsp->ch_num.tx) || (tx < 0) ||
	    (rx >= tsp->ch_num.rx) || (rx < 0)) {
		tsp_warn("TSP channel is not correct!! (%d * %d)\n", tx, rx);
		return TSP_CH_UNKNOWN;
	}

	if ((tx >= tsp->screen.tx) || (rx >= tsp->screen.rx)) {
		for (i = 0; i < tsp->gtx.num; i++)
			if ((tx == tsp->gtx.ch_num[i]) && (rx < tsp->screen.rx))
				return TSP_CH_GTX;

		for (i = 0; i < tkey->key_num; i++) {
			if ((tx == tkey->ch_num[i].tx) &&
			    (rx == tkey->ch_num[i].rx))
				return TSP_CH_KEY;
		}
	} else {
		return TSP_CH_SCREEN;
	}

	return TSP_CH_UNUSED;
}

int ist30xx_get_cmcs_jit_buf(struct ist30xx_data *data, const char *mode,
			 CMCS_JIT_ITEM items, s16 *buf)
{
	int ret = 0;
	int i;
	bool success = false;
	u32 waddr;
	u16 len;

	for (i = 0; i < items.cnt; i++) {
		if (!strncmp(items.item[i].name, mode, 64)) {
			waddr = IST30XX_DA_ADDR(items.item[i].addr);
			len = items.item[i].size / IST30XX_DATA_LEN;
			ret = ist30xx_burst_read(data->client,
						 waddr, (u32 *)buf, len, true);
			if (unlikely(ret))
				return ret;
			tsp_verb("%s, 0x%08x, %d\n", __func__, waddr, len);
			success = true;
		}
	}

	if (success == false) {
        tsp_info("item(%s) dosen't exist!\n", mode);
	    return ret;
    }

#if CMCS_PARSING_DEBUG
	ret = ist30xx_parse_cmcs_jit_buf(data, buf);
#endif

	return ret;
}

int ist30xx_cmcs_jit_wait(struct ist30xx_data *data, int mode)
{
	int cnt = CMCS_TIMEOUT / 100;

	data->status.cmcs = 0;
	while (cnt-- > 0) {
		msleep(100);

		if (data->status.cmcs) {
			if (mode == CMCS_JIT_FLAG_CM)
				goto cm_end;
			else if (mode == CMCS_JIT_FLAG_CS)
				goto cs_end;
			else
				return -EPERM;
		}
	}
	tsp_warn("cmcs time out\n");

	return -EPERM;

cm_end:
	if ((data->status.cmcs & CMCS_MSG_MASK) == CM_MSG_VALID)
		if (!(data->status.cmcs & 0x1))
			return 0;

	tsp_warn("CM test fail\n");

	return -EPERM;

cs_end:
	if ((data->status.cmcs & CMCS_MSG_MASK) == CS_MSG_VALID)
		if (!(data->status.cmcs & 0x1))
			return 0;

	tsp_warn("CS test fail\n");

	return -EPERM;
}

#define cmcs_jit_next_step(ret)   { if (unlikely(ret)) goto end; msleep(20); }
int ist30xx_cmcs_jit_test(struct ist30xx_data *data, const u8 *buf, int size)
{
	int ret;
	int len;
	u32 waddr;
	u32 val;
	u32 chksum = 0;
	u32 *buf32;

	tsp_info("*** CM/CS test ***\n");

	ist30xx_disable_irq(data);
    ist30xx_reset(data, false);

	ret = ist30xx_write_cmd(data->client,
				IST30XX_HIB_CMD, (eHCOM_RUN_RAMCODE << 16) | 0);
	cmcs_jit_next_step(ret);

	/* Load cmcs test code */
	buf32 = (u32 *)ts_cmcs_jit->buf_cmcs;
	len = ts_cmcs_jit->param.cmcs_size / IST30XX_DATA_LEN - 1;
	tsp_verb("%08x %08x %08x %08x\n", buf32[0], buf32[1], buf32[2], buf32[3]);
	waddr = IST30XX_DA_ADDR(data->tags.ram_base);
	tsp_verb("%08x(%d)\n", waddr, len);

	ret = ist30xx_burst_write(data->client, waddr, buf32, len);
	cmcs_jit_next_step(ret);

	waddr = IST30XX_DA_ADDR(ts_cmcs_jit->param.cmcs_size_addr);
	val = ts_cmcs_jit->param.cmcs_size - 4;
	ret = ist30xx_write_cmd(data->client, waddr, val);
	cmcs_jit_next_step(ret);
	tsp_verb("size(0x%08x): 0x%08x\n", waddr, val);

	tsp_info("cmcs code loaded!\n");

	/* Set command */
	ret = ist30xx_set_cmcs_jit_cmd(data->client, ts_cmcs_jit->cmds);
	cmcs_jit_next_step(ret);
	tsp_info("cmcs command loaded!\n");

	/* Cal checksum & run cmcs */
	ret = ist30xx_write_cmd(data->client,
				IST30XX_HIB_CMD, (eHCOM_RUN_RAMCODE << 16) | 1);
	cmcs_jit_next_step(ret);

	/* Check checksum */
	waddr = IST30XX_DA_ADDR(ts_cmcs_jit->param.checksum_addr);
	ret = ist30xx_read_reg(data->client, waddr, &chksum);
	cmcs_jit_next_step(ret);
	if (chksum != ts_cmcs_jit->param.cmcs_chksum)
		goto end;

	/* Set cm sensor register */
	buf32 = ts_cmcs_jit->buf_cm_sensor;
	ret = ist30xx_set_cmcs_jit_sensor(data->client,
				      ts_cmcs_jit->param, buf32, CMCS_JIT_FLAG_CM);
	cmcs_jit_next_step(ret);

	waddr = IST30XX_DA_ADDR(ts_cmcs_jit->param.enable_addr);
	val = CMCS_JIT_FLAG_CM;
	ret = ist30xx_write_cmd(data->client, waddr, val);
	cmcs_jit_next_step(ret);
	tsp_info("CM test ready!!\n");

	/* Wait CM test result */
	ist30xx_enable_irq(data);
	if (ist30xx_cmcs_jit_wait(data, CMCS_JIT_FLAG_CM) == 0) {
		tsp_info("CM test OK\n");
	} else {
		tsp_info("CM test Fail!\n");
		ret = -ENOEXEC;
		goto end;
	}
	ist30xx_disable_irq(data);

	/* Read CMJIT data */
	memset(ts_cmcs_jit_buf->cm_jit, 0, sizeof(ts_cmcs_jit_buf->cm_jit));

	ret = ist30xx_get_cmcs_jit_buf(data,
				   CMCS_CMJIT, ts_cmcs_jit->items, ts_cmcs_jit_buf->cm_jit);
	cmcs_jit_next_step(ret);

	cmcs_jit_ready = CMCS_READY;
end:
	if (unlikely(ret)) {
		tsp_warn("CmCs test Fail!, ret=%d\n", ret);
	} else if (unlikely(chksum != ts_cmcs_jit->param.cmcs_chksum)) {
		tsp_warn("Error CheckSum: %x(%x)\n",
			 chksum, ts_cmcs_jit->param.cmcs_chksum);
		ret = -ENOEXEC;
	}

    ist30xx_reset(data, false);
	ist30xx_start(data);
	ist30xx_enable_irq(data);

	return ret;
}

int print_cmcs_jit(struct ist30xx_data *data, s16 *buf16, char *buf)
{
	int i, j;
	int idx;
	int type;
	int count = 0;
	char msg[128];

	TSP_INFO *tsp = &data->tsp_info;

	int tx_num = tsp->ch_num.tx;
	int rx_num = tsp->ch_num.rx;

	for (i = 0; i < tx_num; i++) {
		for (j = 0; j < rx_num; j++) {
			type = check_ts_type(data, i, j);
			idx = (i * rx_num) + j;
			if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED))
				count += sprintf(msg, "%5d ", 0);
			else
				count += sprintf(msg, "%5d ", buf16[idx]);
			strcat(buf, msg);
		}

		count += sprintf(msg, "\n");
		strcat(buf, msg);
	}

	return count;
}

int print_line_cmcs_jit(struct ist30xx_data *data, int mode, s16 *buf16, char *buf)
{
	int i, j;
	int idx;
	int type;
	int count = 0;
	int key_index[5] = { 0, };
	int key_cnt = 0;
	char msg[128];

	TSP_INFO *tsp = &data->tsp_info;

	int tx_num = tsp->ch_num.tx;
	int rx_num = tsp->ch_num.rx;

	for (i = 0; i < tx_num; i++) {
		for (j = 0; j < rx_num; j++) {
			type = check_ts_type(data, i, j);
			if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED))
				continue;   // Ignore

			if ((mode == CMCS_JIT_FLAG_CM_SLOPE0) &&
			    (j == (tsp->screen.rx - 1)))
				continue;
			else if ((mode == CMCS_JIT_FLAG_CM_SLOPE1) &&
				 (i == (tsp->screen.tx - 1)))
				continue;

			if ((mode == CMCS_JIT_FLAG_CM_SLOPE0) && (type == TSP_CH_GTX) &&
			    (j == (tsp->screen.rx - 1)))
				continue;
			else if ((mode == CMCS_JIT_FLAG_CM_SLOPE1) &&
				 (type == TSP_CH_GTX) &&
				 (i == tsp->gtx.ch_num[tsp->gtx.num - 1]))
				continue;

			idx = (i * rx_num) + j;

			if (type == TSP_CH_KEY) {
				key_index[key_cnt++] = idx;
				continue;
			}

			count += sprintf(msg, "%5d ", buf16[idx]);
			strcat(buf, msg);
		}
	}

	tsp_info("key cnt: %d\n", key_cnt);
	if ((mode != CMCS_JIT_FLAG_CM_SLOPE0) && (mode != CMCS_JIT_FLAG_CM_SLOPE1)) {
		tsp_info("key cnt: %d\n", key_cnt);
		for (i = 0; i < key_cnt; i++) {
			count += sprintf(msg, "%5d ", buf16[key_index[i]]);
			strcat(buf, msg);
		}
	}

	count += sprintf(msg, "\n");
	strcat(buf, msg);

	return count;
}

/* sysfs: /sys/class/touch/cmcs/cmcs_jit_binary */
ssize_t ist30xx_cmcs_jit_binary_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	struct ist30xx_data *data = dev_get_drvdata(dev);

	if ((ts_cmcs_jit_bin == NULL) || (ts_cmcs_jit_bin_size == 0))
		return sprintf(buf, "Binary is not correct(%d)\n",
                ts_cmcs_jit_bin_size);

	ret = ist30xx_get_cmcs_jit_info(ts_cmcs_jit_bin, ts_cmcs_jit_bin_size);
	if (ret)
		goto binary_end;

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_jit_test(data, ts_cmcs_jit_bin, ts_cmcs_jit_bin_size);
	mutex_unlock(&ist30xx_mutex);

binary_end:
	return sprintf(buf, (ret == 0 ? "OK\n" : "Fail\n"));
}

/* sysfs: /sys/class/touch/cmcs/cmcs_jit_custom */
ssize_t ist30xx_cmcs_jit_custom_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret;
	int bin_size = 0;
	u8 *bin = NULL;
	const struct firmware *req_bin = NULL;
	struct ist30xx_data *data = dev_get_drvdata(dev);

	ret = request_firmware(&req_bin, IST30XX_CMCS_JIT_NAME, &data->client->dev);
	if (ret)
		return sprintf(buf, "File not found, %s\n", IST30XX_CMCS_JIT_NAME);

	bin = (u8 *)req_bin->data;
	bin_size = (u32)req_bin->size;

	ret = ist30xx_get_cmcs_jit_info(bin, bin_size);
	if (ret)
		goto custom_end;

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_jit_test(data, bin, bin_size);
	mutex_unlock(&ist30xx_mutex);

custom_end:
	release_firmware(req_bin);

	return sprintf(buf, (ret == 0 ? "OK\n" : "Fail\n"));
}

#define MAX_FILE_PATH   255
/* sysfs: /sys/class/touch/cmcs/cmcs_jit_sdcard */
ssize_t ist30xx_cmcs_jit_sdcard_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret = 0;
	int bin_size = 0;
	u8 *bin = NULL;
	const u8 *buff = 0;
	mm_segment_t old_fs = { 0 };
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	char fw_path[MAX_FILE_PATH];
	struct ist30xx_data *data = dev_get_drvdata(dev);

	old_fs = get_fs();
	set_fs(get_ds());

	snprintf(fw_path, MAX_FILE_PATH, "/sdcard/%s",
		 IST30XX_CMCS_JIT_NAME);
	fp = filp_open(fw_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		tsp_info("file %s open error:%d\n", fw_path, (s32)fp);
		ret = -ENOENT;
		goto err_file_open;
	}

	fsize = fp->f_path.dentry->d_inode->i_size;

	buff = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!buff) {
		tsp_info("fail to alloc buffer\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
	if (nread != fsize) {
		tsp_info("mismatch fw size\n");

		goto err_fw_size;
	}

	bin = (u8 *)buff;
	bin_size = (u32)fsize;

	filp_close(fp, current->files);
	set_fs(old_fs);
	tsp_info("firmware is loaded!!\n");

	ret = ist30xx_get_cmcs_jit_info(bin, bin_size);
	if (ret)
		goto sdcard_end;

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_jit_test(data, bin, bin_size);
	mutex_unlock(&ist30xx_mutex);

err_fw_size:
	if (buff)
		kfree(buff);
err_alloc:
	if (fp)
		filp_close(fp, NULL);
err_file_open:
	set_fs(old_fs);

sdcard_end:
	tsp_info("size: %d\n", sprintf(buf, (ret == 0 ? "OK\n" : "Fail\n")));

	return sprintf(buf, (ret == 0 ? "OK\n" : "Fail\n"));
}

/* sysfs: /sys/class/touch/cmcs/cm_jit */
ssize_t ist30xx_cm_jit_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	TSP_INFO *tsp = &data->tsp_info;

	if (cmcs_jit_ready == CMCS_NOT_READY)
		return sprintf(buf, "CMCS test is not work!!\n");

	tsp_verb("CMJIT (%d * %d)\n", tsp->ch_num.tx, tsp->ch_num.rx);

	return print_cmcs_jit(data, ts_cmcs_jit_buf->cm_jit, buf);
}

int print_total_result(struct ist30xx_data *data, s16 *buf16, char *buf,
		       const char *mode)
{
	int i, j;
	bool success = false;
	int type, idx;
	int count = 0, err_cnt = 0;
	int min_spec, max_spec;
	char msg[128];

	TSP_INFO *tsp = &data->tsp_info;
	struct CMCS_JIT_SPEC_TOTAL *spec;

	for (i = 0; i < ts_cmcs_jit->items.cnt; i++) {
		if (!strncmp(ts_cmcs_jit->items.item[i].name, mode, 64)) {
			if (!strncmp(ts_cmcs_jit->items.item[i].spec_type, "T", 64)) {
				spec =
					(struct CMCS_JIT_SPEC_TOTAL *)&ts_cmcs_jit->spec_item[i].spec_total;
				success = true;
				break;
			}
		}
	}

	if (success == false)
		return 0;

	count = sprintf(msg, "Spec: screen(%d~%d), gtx(%d~%d), key(%d~%d)\n",
			spec->screen_min, spec->screen_max, spec->gtx_min,
			spec->gtx_max, spec->key_min, spec->key_max);
	strcat(buf, msg);

	for (i = 0; i < tsp->ch_num.tx; i++) {
		for (j = 0; j < tsp->ch_num.rx; j++) {
			idx = (i * tsp->ch_num.rx) + j;

			type = check_ts_type(data, i, j);
			if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED))
				continue;   // Ignore

			if (type == TSP_CH_SCREEN) {
				min_spec = spec->screen_min;
				max_spec = spec->screen_max;
			} else if (type == TSP_CH_GTX) {
				min_spec = spec->gtx_min;
				max_spec = spec->gtx_max;
			} else {    // TSP_CH_KEY
				min_spec = spec->key_min;
				max_spec = spec->key_max;
			}

			if ((buf16[idx] >= min_spec) && (buf16[idx] <= max_spec))
				continue;   // OK

			count += sprintf(msg, "%2d,%2d:%4d\n", i, j, buf16[idx]);
			strcat(buf, msg);

			err_cnt++;
		}
	}

	/* Check error count */
	if (err_cnt == 0)
		count += sprintf(msg, "OK\n");
	else
		count += sprintf(msg, "Fail, node count: %d\n", err_cnt);
	strcat(buf, msg);

	return count;
}

/* sysfs: /sys/class/touch/cmcs_jit/cm_jit_result */
ssize_t ist30xx_cm_jit_result_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);

	if (cmcs_jit_ready == CMCS_NOT_READY)
		return sprintf(buf, "CMCS test is not work!!\n");

	return print_total_result(data, ts_cmcs_jit_buf->cm_jit, buf, CMCS_CMJIT);
}

/* sysfs: /sys/class/touch/cmcs_jit/line_cm_jit */
ssize_t ist30xx_line_cm_jit_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);

	if (cmcs_jit_ready == CMCS_NOT_READY)
		return sprintf(buf, "CMCS test is not work!!\n");

	return print_line_cmcs_jit(data, CMCS_JIT_FLAG_CMJIT, ts_cmcs_jit_buf->cm_jit, buf);
}

int check_cm_jit(struct ist30xx_data *data, s16 *buf16, char *buf)
{
	int i, j;
	int idx;
	int type;
	int count = 0;
	char msg[128];
	TSP_INFO *tsp = &data->tsp_info;
    s32 all_sum = 0;
    s32 all_count = 0;
    s32 all_avg = 0;
    s32 rx_sum[38] = { 0 };
    s32 rx_count[38] = { 0 };
    s32 rx_avg = 0;    
    s32 result = 0;

	for (i = 0; i < tsp->ch_num.tx; i++) {
		for (j = 0; j < tsp->ch_num.rx; j++) {
			type = check_ts_type(data, i, j);
			if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED) || 
                    (type == TSP_CH_KEY))
				continue;   // Ignore

			idx = (i * tsp->ch_num.rx) + j;
            all_sum += buf16[idx];
            all_count++;
            rx_sum[j] += buf16[idx];
            rx_count[j]++;
		}
	}

    all_avg = all_sum / all_count;
    for (i = 0; i < tsp->screen.rx; i++) {
        rx_avg = rx_sum[i] / rx_count[i];
        result = rx_avg - all_avg;
        count += sprintf(msg, "%5d ", result);
		strcat(buf, msg);
    }
   
    return count;
}

/* sysfs: /sys/class/touch/cmcs_jit/check_cm_jit */
ssize_t ist30xx_check_cm_jit_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);

	if (cmcs_jit_ready == CMCS_NOT_READY)
		return sprintf(buf, "CMCS test is not work!!\n");

    return check_cm_jit(data, ts_cmcs_jit_buf->cm_jit, buf);
}

/* sysfs : cmcs */
static DEVICE_ATTR(cmcs_jit_binary, S_IRUGO,
        ist30xx_cmcs_jit_binary_show, NULL);
static DEVICE_ATTR(cmcs_jit_custom, S_IRUGO,
        ist30xx_cmcs_jit_custom_show, NULL);
static DEVICE_ATTR(cmcs_jit_sdcard, S_IRUGO,
        ist30xx_cmcs_jit_sdcard_show, NULL);
static DEVICE_ATTR(cm_jit, S_IRUGO, ist30xx_cm_jit_show, NULL);
static DEVICE_ATTR(cm_jit_result, S_IRUGO, ist30xx_cm_jit_result_show, NULL);
static DEVICE_ATTR(line_cm_jit, S_IRUGO, ist30xx_line_cm_jit_show, NULL);
static DEVICE_ATTR(check_cm_jit, S_IRUGO, ist30xx_check_cm_jit_show, NULL);

static struct attribute *cmcs_jit_attributes[] = {
	&dev_attr_cmcs_jit_binary.attr,
	&dev_attr_cmcs_jit_custom.attr,
	&dev_attr_cmcs_jit_sdcard.attr,
	&dev_attr_cm_jit.attr,
	&dev_attr_cm_jit_result.attr,
	&dev_attr_line_cm_jit.attr,
    &dev_attr_check_cm_jit.attr,
	NULL,
};

static struct attribute_group cmcs_jit_attr_group = {
	.attrs	= cmcs_jit_attributes,
};

extern struct class *ist30xx_class;
struct device *ist30xx_cmcs_jit_dev;

int ist30xx_init_cmcs_jit_sysfs(struct ist30xx_data *data)
{

	tsp_info("%s:++\n", __func__);
	
	/* /sys/class/touch/cmcs_jit */
	ist30xx_cmcs_jit_dev = device_create(ist30xx_class, NULL, 0, data, "cmcs_jit");

	/* /sys/class/touch/cmcs/... */
	if (unlikely(sysfs_create_group(&ist30xx_cmcs_jit_dev->kobj,
					&cmcs_jit_attr_group)))
		tsp_err("Failed to create sysfs group(%s)!\n", "cmcs");

	ts_cmcs_jit_bin = (u8 *)ist30xx_cmcs_jit;
	ts_cmcs_jit_bin_size = sizeof(ist30xx_cmcs_jit);

	ist30xx_get_cmcs_jit_info(ts_cmcs_jit_bin, ts_cmcs_jit_bin_size);

	return 0;
}
