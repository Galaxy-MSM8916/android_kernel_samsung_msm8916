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
#include <asm/unaligned.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/err.h>

#include "ist30xx.h"
#include "ist30xx_update.h"
#include "ist30xx_tracking.h"

#if IST30XX_DEBUG
#include "ist30xx_misc.h"
#endif

#if IST30XX_INTERNAL_BIN
#include "core3_fw.h"
#endif // IST30XX_INTERNAL_BIN

struct ist30xx_tags *ts_tags;
extern unsigned int system_rev;

const u8 *ts_fw = ist30xxb_fw;
u32 ts_fw_size = sizeof(ist30xxb_fw);

u32 ist30xx_fw_ver = 0;
u32 ist30xx_param_ver = 0;
u32 ist30xx_sub_ver = 0;

extern struct ist30xx_data *ts_data;

extern void ist30xx_disable_irq(struct ist30xx_data *data);
extern void ist30xx_enable_irq(struct ist30xx_data *data);

int ist30xx_i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			 int msg_num, u8 *cmd_buf)
{
	int ret = 0;
	int idx = msg_num - 1;
	int size = msgs[idx].len;
	u8 *msg_buf = NULL;
	u8 *pbuf = NULL;
	int trans_size, max_size = 0;

	if (msg_num == WRITE_CMD_MSG_LEN)
		max_size = I2C_MAX_WRITE_SIZE;
	else if (msg_num == READ_CMD_MSG_LEN)
		max_size = I2C_MAX_READ_SIZE;

	if (unlikely(max_size == 0)) {
		tsp_err("%s: transaction size(%d)\n", __func__, max_size);
		return -EINVAL;
	}

	if (msg_num == WRITE_CMD_MSG_LEN) {
		msg_buf = kmalloc(max_size + IST30XX_ADDR_LEN, GFP_KERNEL);
		if (!msg_buf)
			return -ENOMEM;
		memcpy(msg_buf, cmd_buf, IST30XX_ADDR_LEN);
		pbuf = msgs[idx].buf;
	}

	while (size > 0) {
		trans_size = (size >= max_size ? max_size : size);

		msgs[idx].len = trans_size;
		if (msg_num == WRITE_CMD_MSG_LEN) {
			memcpy(&msg_buf[IST30XX_ADDR_LEN], pbuf, trans_size);
			msgs[idx].buf = msg_buf;
			msgs[idx].len += IST30XX_ADDR_LEN;
		}
		ret = i2c_transfer(adap, msgs, msg_num);
		if (unlikely(ret != msg_num)) {
			tsp_err("%s: i2c_transfer failed(%d), num=%d\n",
				__func__, ret, msg_num);
			break;
		}

		if (msg_num == WRITE_CMD_MSG_LEN)
			pbuf += trans_size;
		else
			msgs[idx].buf += trans_size;

		size -= trans_size;
	}

	if (msg_num == WRITE_CMD_MSG_LEN)
		kfree(msg_buf);

	return ret;
}

int ist30xx_read_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len)
{
	int ret, i;
	u32 le_reg = cpu_to_be32(cmd);

	struct i2c_msg msg[READ_CMD_MSG_LEN] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = IST30XX_ADDR_LEN,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = len * IST30XX_DATA_LEN,
			.buf = (u8 *)buf,
		},
	};

	ret = ist30xx_i2c_transfer(client->adapter, msg, READ_CMD_MSG_LEN, NULL);
	if (unlikely(ret != READ_CMD_MSG_LEN))
		return -EIO;

	for (i = 0; i < len; i++)
		buf[i] = cpu_to_be32(buf[i]);

	return 0;
}

int ist30xx_write_buf(struct i2c_client *client, u32 cmd, u32 *buf, u16 len)
{
	int i;
	int ret;
	struct i2c_msg msg;
	u8 cmd_buf[IST30XX_ADDR_LEN];
	u8 msg_buf[IST30XX_DATA_LEN * (len + 1)];

	put_unaligned_be32(cmd, cmd_buf);

	if (likely(len > 0)) {
		for (i = 0; i < len; i++)
			put_unaligned_be32(buf[i], msg_buf + (i * IST30XX_DATA_LEN));
	} else {
		/* then add dummy data(4byte) */
		put_unaligned_be32(0, msg_buf);
		len = 1;
	}

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = IST30XX_DATA_LEN * len;
	msg.buf = msg_buf;

	ret = ist30xx_i2c_transfer(client->adapter, &msg, WRITE_CMD_MSG_LEN,
				   cmd_buf);
	if (unlikely(ret != WRITE_CMD_MSG_LEN))
		return -EIO;

	return 0;
}

int ist30xxb_burst_read(struct i2c_client *client, u32 addr,
			u32 *buf32, int len)
{
	int ret = 0;
	int i;
	int max_len = I2C_MAX_READ_SIZE / IST30XX_DATA_LEN;

	addr |= IST30XXB_BURST_ACCESS;

	for (i = 0; i < len; i += max_len) {
		if (len < max_len) max_len = len;

		ret = ist30xx_read_buf(client, addr, buf32, max_len);
		if (unlikely(ret)) {
			tsp_err("%s: Burst fail, addr: %x\n", __func__, addr);
			return ret;
		}

		addr += max_len * IST30XX_DATA_LEN;
		buf32 += max_len;
	}

	return 0;
}

#define IST30XXB_ISP_READ       (1)
#define IST30XXB_ISP_WRITE      (2)
#define IST30XXB_ISP_ERASE_ALL  (3)
#define IST30XXB_ISP_ERASE_PAGE (4)

int ist30xxb_isp_reset(void)
{
	int ret = 0;

	ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_EEPMODE, 0x180);
	if (unlikely(ret))
		return ret;

	msleep(1);

	return ret;
}

int ist30xxb_isp_enable(bool enable)
{
	int ret = 0;

	if (enable) {
		ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_EEPISPEN, 0x1);
		if (unlikely(ret))
			return ret;

		ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_LDOOSC, 0x74C8);
		if (unlikely(ret))
			return ret;

		ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_CLKDIV, 0x3);
		if (unlikely(ret))
			return ret;

		msleep(10);

		ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_WDTCON, 0x0);
		if (unlikely(ret))
			return ret;
	} else {
		ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_EEPISPEN, 0x0);
		if (unlikely(ret))
			return ret;

		msleep(1);
	}

	return 0;
}

int ist30xxb_isp_mode(int mode)
{
	int ret = 0;
	u32 val = 0;

	switch (mode) {
	case IST30XXB_ISP_READ:
		val = 0x1C0;
		break;
	case IST30XXB_ISP_WRITE:
		val = 0x1A8;
		break;
	case IST30XXB_ISP_ERASE_ALL:
		val = 0x1A7;
		break;
	case IST30XXB_ISP_ERASE_PAGE:
		val = 0x1A3;
		break;
	default:
		tsp_err("%s: ISP fail, unknown mode\n", __func__);
		return -EINVAL;
	}

	ret = ist30xx_write_cmd(ts_data->client, IST30XXB_REG_EEPMODE, val);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_EEPMODE\n", __func__);
		return ret;
	}

	return 0;
}

int ist30xxb_isp_erase(struct i2c_client *client, int mode, u32 addr)
{
	int ret = 0;
	u32 val = 0x1A0;
	u8 buf[EEPROM_PAGE_SIZE] = { 0, };

	ret = ist30xxb_isp_mode(mode);
	if (unlikely(ret))
		return ret;

	ret = ist30xx_write_cmd(client, IST30XXB_REG_EEPADDR, addr);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_EEPADDR\n", __func__);
		return ret;
	}

	val = (EEPROM_PAGE_SIZE / IST30XX_DATA_LEN);
	ret = ist30xx_write_buf(client, IST30XXB_REG_EEPWDAT, (u32 *)buf, val);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_EEPWDAT\n", __func__);
		return ret;
	}

	msleep(30);

	ist30xxb_isp_reset();

	return 0;
}

int ist30xxb_isp_write(struct i2c_client *client, u32 addr,
		       const u32 *buf32, int len)
{
	int ret = 0;

	ret = ist30xx_write_cmd(client, IST30XXB_REG_EEPADDR, addr);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_EEPADDR\n", __func__);
		return ret;
	}

	ret = ist30xx_write_buf(client, IST30XXB_REG_EEPWDAT, (u32 *)buf32, len);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_EEPWDAT\n", __func__);
		return ret;
	}

	return 0;
}

int ist30xxb_isp_read(struct i2c_client *client, u32 addr,
		      u32 *buf32, int len)
{
	int ret = 0;
	int i;
	int max_len = I2C_MAX_READ_SIZE / IST30XX_DATA_LEN;

	for (i = 0; i < len; i += max_len) {
		if (len < max_len) max_len = len;

		/* IST30xxB ISP read mode */
		ret = ist30xxb_isp_mode(IST30XXB_ISP_READ);
		if (unlikely(ret))
			return ret;

		ret = ist30xx_write_cmd(client, IST30XXB_REG_EEPADDR, addr);
		if (unlikely(ret)) {
			tsp_err("%s: ISP fail, IST30XXB_REG_EEPADDR\n", __func__);
			return ret;
		}

		ret = ist30xx_read_buf(client, IST30XXB_REG_EEPRDAT, buf32, max_len);
		if (unlikely(ret)) {
			tsp_err("%s: ISP fail, IST30XXB_REG_EEPWDAT\n", __func__);
			return ret;
		}

		addr += max_len * IST30XX_DATA_LEN;
		buf32 += max_len;
	}

	return 0;
}

int ist30xxb_cmd_read_chksum(struct i2c_client *client,
			     u32 start_addr, u32 end_addr, u32 *chksum)
{
	int ret = 0;
	u32 val = (1 << 31); // Chkecksum enable

	val |= start_addr;
	val |= (end_addr / IST30XX_DATA_LEN - 1) << 16;

	ret = ist30xx_write_cmd(client, IST30XXB_REG_CHKSMOD, val);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_CHKSMOD (%x)\n", __func__, val);
		return ret;
	}

	msleep(10);

	ret = ist30xx_read_cmd(client, IST30XXB_REG_CHKSDAT, chksum);
	if (unlikely(ret)) {
		tsp_err("%s: ISP fail, IST30XXB_REG_CHKSDAT (%x)\n", __func__, chksum);
		return ret;
	}

	return 0;
}

int ist30xxb_read_chksum(struct i2c_client *client, u32 *chksum)
{
	int ret = 0;
	u32 start_addr, end_addr;
	u32 chksum1 = 0, chksum2 = 0;

	start_addr = 0;
	end_addr = ts_data->tags.fw_addr + ts_data->tags.fw_size;
	ret = ist30xxb_cmd_read_chksum(client, start_addr, end_addr, &chksum1);
	if (unlikely(ret))
		return ret;

	start_addr = ts_data->tags.cfg_addr;
	end_addr = ts_data->tags.sensor3_addr + ts_data->tags.sensor3_size;
	ret = ist30xxb_cmd_read_chksum(client, start_addr, end_addr, &chksum2);
	if (unlikely(ret))
		return ret;

	*chksum = chksum1 | (chksum2 << 16);

	tsp_info("%s: chksum: %x(%x~%x, %x~%x)\n", __func__, *chksum,
		 0, ts_data->tags.fw_addr + ts_data->tags.fw_size,
		 start_addr, end_addr);

	return 0;
}

int ist30xxb_read_chksum_all(struct i2c_client *client, u32 *chksum)
{
	int ret = 0;
	u32 start_addr, end_addr;

	start_addr = 0;
	end_addr = IST30XX_EEPROM_SIZE;
	ret = ist30xxb_cmd_read_chksum(client, start_addr, end_addr, chksum);
	if (unlikely(ret))
		return ret;

	tsp_info("%s: chksum: %x(%x~%x)\n", __func__, *chksum,
		 start_addr, end_addr);

	return 0;
}

int ist30xxb_isp_fw_read(struct i2c_client *client, u32 *buf32)
{
	int ret = 0;
	int i;

	u16 addr = EEPROM_BASE_ADDR;
	int len = EEPROM_PAGE_SIZE / IST30XX_DATA_LEN;

	ist30xx_reset(true);

	/* IST30xxB ISP enable */
	ret = ist30xxb_isp_enable(true);
	if (unlikely(ret))
		return ret;

	for (i = 0; i < IST30XX_EEPROM_SIZE; i += EEPROM_PAGE_SIZE) {
		ret = ist30xxb_isp_read(client, addr, buf32, len);
		if (unlikely(ret))
			goto isp_fw_read_end;

		addr += EEPROM_PAGE_SIZE;
		buf32 += len;
	}

isp_fw_read_end:
	/* IST30xxB ISP disable */
	ist30xxb_isp_enable(false);
	return ret;
}

int ist30xxb_isp_fw_update(struct i2c_client *client, const u8 *buf,
			   u32 *chksum)
{
	int ret = 0;
	int i;
	u32 page_cnt = IST30XX_EEPROM_SIZE / EEPROM_PAGE_SIZE;
	u16 addr = EEPROM_BASE_ADDR;
	int len = EEPROM_PAGE_SIZE / IST30XX_DATA_LEN;

	ist30xx_tracking(TRACK_CMD_FWUPDATE);

	/* IST30xxB ISP enable */
	ret = ist30xxb_isp_enable(true);
	if (unlikely(ret))
		goto isp_fw_update_end;

	/* IST30xxB ISP erase */
	ret = ist30xxb_isp_erase(client, IST30XXB_ISP_ERASE_ALL, 0);
	if (unlikely(ret))
		goto isp_fw_update_end;

	/* IST30xxB ISP read all checksum */
	ret = ist30xxb_read_chksum_all(client, chksum);
	if (unlikely(ret))
		goto isp_fw_update_end;

	/* IST30xxB compare checksum */
	if (unlikely(*chksum != 0))
		goto isp_fw_update_end;

	/* IST30xxB power reset */
	ist30xx_reset(true);

	/* IST30xxB ISP enable */
	ret = ist30xxb_isp_enable(true);
	if (unlikely(ret))
		goto isp_fw_update_end;

	for (i = 0; i < page_cnt; i++) {
		/* IST30xxB ISP write mode */
		ret = ist30xxb_isp_mode(IST30XXB_ISP_WRITE);
		if (unlikely(ret))
			goto isp_fw_update_end;

		ret = ist30xxb_isp_write(client, addr, (u32 *)buf, len);
		if (unlikely(ret))
			goto isp_fw_update_end;

		addr += EEPROM_PAGE_SIZE;
		buf += EEPROM_PAGE_SIZE;

		msleep(5);

		ist30xxb_isp_reset();
	}

#if (IMAGIS_TSP_IC == IMAGIS_IST3038)
	ist30xxb_read_chksum_all(client, chksum);
#endif

isp_fw_update_end:
	/* IST30xxB ISP disable */
	ist30xxb_isp_enable(false);
	return ret;
}

int ist30xx_fw_write(struct i2c_client *client, const u8 *buf)
{
	int ret;
	int len;
	u32 *buf32 = (u32 *)(buf + ts_data->fw.index);
	u32 size = ts_data->fw.size;

	if (unlikely((size <= 0) || (size > IST30XX_EEPROM_SIZE)))
		return -ENOEXEC;

	while (size > 0) {
		len = (size >= EEPROM_PAGE_SIZE ? EEPROM_PAGE_SIZE : size);

		ret = ist30xx_write_buf(client, CMD_ENTER_FW_UPDATE, buf32, (len >> 2));
		if (unlikely(ret))
			return ret;

		buf32 += (len >> 2);
		size -= len;

		msleep(5);
	}
	return 0;
}


u32 ist30xx_parse_ver(int flag, const u8 *buf)
{
	u32 ver = 0;
	u32 *buf32 = (u32 *)buf;

	if (flag == FLAG_FW)
		ver = (u32)buf32[(ts_data->tags.flag_addr + 60) >> 2];
	else if (flag == FLAG_SUB)
		ver = (u32)buf32[(ts_data->tags.flag_addr + 52) >> 2];
	else if (flag == FLAG_PARAM)
		ver = (u32)(buf32[(ts_data->tags.cfg_addr + 4) >> 2] & 0xFFFF);
	else
		tsp_warn("%s: Parsing ver's flag is not corrent!\n", __func__);

	return ver;
}


int calib_ms_delay = WAIT_CALIB_CNT;
int ist30xx_calib_wait(void)
{
	int cnt = calib_ms_delay;

	ts_data->status.calib_msg = 0;
	while (cnt-- > 0) {
		msleep(100);

		if (ts_data->status.calib_msg) {
			tsp_info("%s: Calibration status : %d, Max raw gap : %d - (%08x)\n",
				 __func__, CALIB_TO_STATUS(ts_data->status.calib_msg),
				 CALIB_TO_GAP(ts_data->status.calib_msg),
				 ts_data->status.calib_msg);

			if (CALIB_TO_OS_VALUE(ts_data->status.calib_msg) == 0xFFFF)
				return 1;
			else if (CALIB_TO_STATUS(ts_data->status.calib_msg) == 0)
				return 0;  // Calibrate success
			else
				return -EAGAIN;
		}
	}
	tsp_warn("%s: Calibration time out\n", __func__);

	return -EPERM;
}

int ist30xx_calibrate(int wait_cnt)
{
	int ret = -ENOEXEC;

	tsp_info("%s: *** Calibrate %ds ***\n", __func__, calib_ms_delay / 10);

	ts_data->status.update = 1;
	while (wait_cnt--) {
		ist30xx_disable_irq(ts_data);
		ist30xx_reset(false);

		ret = ist30xx_cmd_calibrate(ts_data->client);
		if (unlikely(ret))
			continue;

		ist30xx_enable_irq(ts_data);
		ret = ist30xx_calib_wait();
		if (likely(!ret))
			break;
	}

	ist30xx_disable_irq(ts_data);

	ist30xx_reset(false);

	ts_data->status.update = 2;

	ist30xx_enable_irq(ts_data);

	return ret;
}


int ist30xx_parse_tags(struct ist30xx_data *data, const u8 *buf, const u32 size)
{
	int ret = -EPERM;

	ts_tags = (struct ist30xx_tags *)(&buf[size - sizeof(struct ist30xx_tags)]);

	if (!strncmp(ts_tags->magic1, IST30XX_TAG_MAGIC, sizeof(ts_tags->magic1))
	    && !strncmp(ts_tags->magic2, IST30XX_TAG_MAGIC, sizeof(ts_tags->magic2))
	    ) {
		data->fw.index = ts_tags->fw_addr;
		data->fw.size = ts_tags->flag_addr - ts_tags->fw_addr +
				ts_tags->flag_size;
		data->fw.chksum = ts_tags->chksum;
		data->tags = *ts_tags;

		ret = 0;
	}

	tsp_verb("%s: Tagts magic1: %s, magic2: %s\n", __func__,
		 ts_tags->magic1, ts_tags->magic2);
	tsp_verb("%s:  fw: %x(%x)\n", __func__, ts_tags->fw_addr, ts_tags->fw_size);
	tsp_verb("%s:  flag: %x(%x)\n", __func__, ts_tags->flag_addr,
		 ts_tags->flag_size);
	tsp_verb("%s:  cfg: %x(%x)\n", __func__, ts_tags->cfg_addr,
		 ts_tags->cfg_size);
	tsp_verb("%s:  sensor1: %x(%x)\n", __func__, ts_tags->sensor1_addr,
		 ts_tags->sensor1_size);
	tsp_verb("%s:  sensor2: %x(%x)\n", __func__, ts_tags->sensor2_addr,
		 ts_tags->sensor2_size);
	tsp_verb("%s:  sensor3: %x(%x)\n", __func__, ts_tags->sensor3_addr,
		 ts_tags->sensor3_size);
	tsp_verb("%s:  chksum: %x\n", __func__, ts_tags->chksum);
	tsp_verb("%s:  build time : %04d/%02d/%02d (%02d:%02d:%02d)\n", __func__,
		 ts_tags->year, ts_tags->month, ts_tags->day,
		 ts_tags->hour, ts_tags->min, ts_tags->sec);

	return ret;
}

void ist30xx_get_update_info(struct ist30xx_data *data, const u8 *buf,
			     const u32 size)
{
	int ret;

	ret = ist30xx_parse_tags(data, buf, size);
	if (unlikely(ret != TAGS_PARSE_OK))
		tsp_warn("%s: Cannot find tags of F/W, make a tags by 'tagts.exe'\n",
			 __func__);
}

#if (IST30XX_DEBUG) && (IST30XX_INTERNAL_BIN)
extern TSP_INFO ist30xx_tsp_info;
extern TKEY_INFO ist30xx_tkey_info;
int ist30xx_get_tkey_info(struct ist30xx_data *data)
{
	int ret = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;
	TKEY_INFO *tkey = &ist30xx_tkey_info;
	u8 *cfg_buf;

	cfg_buf = (u8 *)&data->fw.buf[data->tags.cfg_addr];

	tkey->enable = (bool)(cfg_buf[0x321] & 1);
	tkey->axis_rx = (bool)((cfg_buf[0x321] >> 1) & 1);
	tkey->key_num = (u8)cfg_buf[0x322];
    tkey->baseline = (u16)((cfg_buf[0x324] << 8) | cfg_buf[0x323]);    
	tkey->ch_num[0] = (u8)cfg_buf[0x326];
	tkey->ch_num[1] = (u8)cfg_buf[0x327];
	tkey->ch_num[2] = (u8)cfg_buf[0x328];
	tkey->ch_num[3] = (u8)cfg_buf[0x329];
	tkey->ch_num[4] = (u8)cfg_buf[0x32A];

	if (tkey->axis_rx) {
		if (tsp->dir.swap_xy)
			tsp->height -= 1;
		else
			tsp->width -= 1;
	} else {
		if (tsp->dir.swap_xy)
			tsp->width -= 1;
		else
			tsp->height -= 1;
	}

	return ret;
}

#define TSP_INFO_SWAP_XY    (1 << 0)
#define TSP_INFO_FLIP_X     (1 << 1)
#define TSP_INFO_FLIP_Y     (1 << 2)
int ist30xx_get_tsp_info(struct ist30xx_data *data)
{
	int ret = 0;
	TSP_INFO *tsp = &ist30xx_tsp_info;
	u8 *cfg_buf, *sensor_buf;

	cfg_buf = (u8 *)&data->fw.buf[data->tags.cfg_addr];
	sensor_buf = (u8 *)&data->fw.buf[data->tags.sensor1_addr];

	tsp->finger_num = (u8)cfg_buf[0x304];
	tsp->dir.swap_xy = (bool)(cfg_buf[0x305] & TSP_INFO_SWAP_XY ? true : false);
	tsp->dir.flip_x = (bool)(cfg_buf[0x305] & TSP_INFO_FLIP_X ? true : false);
	tsp->dir.flip_y = (bool)(cfg_buf[0x305] & TSP_INFO_FLIP_Y ? true : false);
    tsp->baseline = (u16)((cfg_buf[0x319] << 8) | cfg_buf[0x318]);

	tsp->ch_num.tx = (u8)sensor_buf[0x40];
	tsp->ch_num.rx = (u8)sensor_buf[0x41];

	tsp->node.len = tsp->ch_num.tx * tsp->ch_num.rx;
	tsp->height = (tsp->dir.swap_xy ? tsp->ch_num.rx : tsp->ch_num.tx);
	tsp->width = (tsp->dir.swap_xy ? tsp->ch_num.tx : tsp->ch_num.rx);

	return ret;
}
#endif // (IST30XX_DEBUG) && (IST30XX_INTERNAL_BIN)


#define update_next_step(ret)   { if (unlikely(ret)) goto end; }
int ist30xx_fw_update(struct i2c_client *client, const u8 *buf, int size,
		      bool mode)
{
	int ret = 0;
	u32 chksum = 0;
	struct ist30xx_fw *fw = &ts_data->fw;

	tsp_info("%s: *** Firmware update ***\n", __func__);
	tsp_info("%s: core: %x, param: %x, sub: %x (addr: 0x%x ~ 0x%x)\n", __func__,
		 ist30xx_fw_ver, ist30xx_param_ver, ist30xx_sub_ver,
		 fw->index, (fw->index + fw->size));

	ts_data->status.update = 1;

	ist30xx_disable_irq(ts_data);

	ist30xx_reset(true);

	if (mode) { /* ISP Mode */
		ret = ist30xxb_isp_fw_update(client, buf, &chksum);
		update_next_step(ret);
	} else { /* I2C SW Mode */
		ret = ist30xx_cmd_update(client, CMD_ENTER_FW_UPDATE);
		update_next_step(ret);

		ret = ist30xx_fw_write(client, buf);
		update_next_step(ret);
	}
	msleep(50);

	buf += IST30XX_EEPROM_SIZE;
	size -= IST30XX_EEPROM_SIZE;

	ist30xx_reset(false);

	ret = ist30xx_read_cmd(client, CMD_GET_CHECKSUM, &chksum);
	if (unlikely((ret) || (chksum != fw->chksum)))
		goto end;

	ret = ist30xx_get_ver_info(ts_data);
	update_next_step(ret);

end:
	if (unlikely(ret)) {
		tsp_warn("%s: Firmware update Fail!, ret=%d\n", __func__, ret);
	} else if (unlikely(chksum != fw->chksum)) {
		tsp_warn("%s: Error CheckSum: %x(%x)\n", __func__, chksum, fw->chksum);
		ret = -ENOEXEC;
	}

	ist30xx_enable_irq(ts_data);

	ts_data->status.update = 2;

	return ret;
}

int ist30xx_fw_recovery(struct ist30xx_data *data)
{
	int ret = -EPERM;
	u8 *fw = data->fw.buf;
	int fw_size = data->fw.buf_size;

	ist30xx_get_update_info(data, fw, fw_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw);
	ist30xx_sub_ver = ist30xx_parse_ver(FLAG_SUB, fw);

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_fw_update(data->client, fw, fw_size, true);
	mutex_unlock(&ist30xx_mutex);

	ist30xx_calibrate(1);

	ist30xx_start(data);

	return ret;
}

int ist30xx_fw_force_sdcard(struct ist30xx_data *data,
			    const unsigned char *fw_data, const u32 fw_size)
{
	int ret = -EPERM;
	u8 *fw = (u8 *)fw_data;

	ist30xx_get_update_info(data, fw, fw_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw);

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_fw_update(data->client, fw, fw_size, true);
	mutex_unlock(&ist30xx_mutex);

	ist30xx_calibrate(1);

	ist30xx_start(data);

	return ret;
}

#if IST30XX_INTERNAL_BIN
int ist30xx_check_fw(struct ist30xx_data *data, const u8 *buf)
{
	int ret;
	u32 chksum;

	ret = ist30xx_read_cmd(data->client, CMD_GET_CHECKSUM, &chksum);
	if (unlikely(ret))
		return ret;

	if (unlikely(chksum != data->fw.chksum)) {
		tsp_warn("%s: Checksum compare error %08x(%08x)\n",
			 __func__, chksum, data->fw.chksum);
		return -EPERM;
	}

	return 0;
}

bool ist30xx_check_valid_vendor(u32 tsp_vendor)
{
	switch (tsp_vendor) {
	case TSP_TYPE_ALPS:
	case TSP_TYPE_EELY:
	case TSP_TYPE_TOP:
	case TSP_TYPE_MELFAS:
	case TSP_TYPE_ILJIN:
	case TSP_TYPE_SYNOPEX:
	case TSP_TYPE_SMAC:
	case TSP_TYPE_OTHERS:
		return true;
	default:
		return false;
	}

	return false;
}

#if IST30XX_MULTIPLE_TSP
void ist30xx_set_tsp_fw(struct ist30xx_data *data)
{
	char *str;
	struct ist30xx_fw *fw = &data->fw;

	switch (data->tsp_type) {
	case TSP_TYPE_ALPS:
	case TSP_TYPE_ILJIN:
		str = "ALPS/ILJIN";
		fw->buf = (u8 *)ist30xxb_fw;
		fw->buf_size = sizeof(ist30xxb_fw);
		break;
	case TSP_TYPE_SYNOPEX:
		str = "SYNOPEX";
		fw->buf = (u8 *)ist30xxb_fw1;
		fw->buf_size = sizeof(ist30xxb_fw1);
		break;
	case TSP_TYPE_UNKNOWN:
	default:
		str = "Unknown";
		tsp_warn("%s: Unknown TSP vendor(0x%x)\n", __func__, data->tsp_type);
		break;
	}
	tsp_info("%s: TSP vendor : %s(%x)\n", __func__, str, data->tsp_type);

	ist30xx_get_update_info(ts_data, fw->buf, fw->buf_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw->buf);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw->buf);
	ist30xx_sub_ver = ist30xx_parse_ver(FLAG_SUB, fw->buf);
}
#endif  // IST30XX_MULTIPLE_TSP


int ist30xx_check_auto_update(struct ist30xx_data *data)
{
	int ret = 0;
	int retry = IST30XX_FW_UPDATE_RETRY;
	u32 tsp_type = TSP_TYPE_UNKNOWN;
	u32 chksum;
	bool tsp_check = false;
	struct ist30xx_fw *fw = &data->fw;

	while (retry--) {
		ret = ist30xx_read_cmd(data->client,
				       CMD_GET_TSP_PANNEL_TYPE, &tsp_type);
		if (likely(ret == 0)) {
			tsp_info("%s: tsp type: %x\n",  __func__, tsp_type);
			if (likely(ist30xx_check_valid_vendor(tsp_type) == true))
				tsp_check = true;
			break;
		}
	}

	retry = IST30XX_FW_UPDATE_RETRY;

	if (unlikely(!tsp_check))
		goto fw_check_end;

	ist30xx_get_ver_info(data);

	if (likely((fw->param_ver > 0) && (fw->param_ver < 0xFFFFFFFF))) {
		if (unlikely(((fw->core_ver & MASK_FW_VER) != IST30XX_FW_VER3) &&
			     ((fw->core_ver & MASK_FW_VER) != IST30XX_FW_VER4) &&
				((fw->core_ver & MASK_FW_VER) != IST30XX_FW_VER5)))
			goto fw_check_end;

		tsp_info("%s: Version compare IC: %x(%x), BIN: %x(%x)\n", __func__,
			 fw->param_ver, fw->core_ver, ist30xx_param_ver, ist30xx_fw_ver);

		/* If FW version is same, check FW checksum */
		if (likely((fw->core_ver == ist30xx_fw_ver) &&
			   (fw->param_ver == ist30xx_param_ver) &&
               (fw->sub_ver == 0))) {
			ret = ist30xx_read_cmd(data->client, CMD_GET_CHECKSUM, &chksum);
			if (unlikely((ret) || (chksum != fw->chksum))) {
				tsp_warn("%s: Checksum error, IC: %x, Bin: %x (ret: %d)\n",
					 __func__, chksum, fw->chksum, ret);
				goto fw_check_end;
			}
		}

		/*
		 *  fw->core_ver : FW core version in TSP IC
		 *  fw->param_ver : FW version if TSP IC
		 *  ist30xx_fw_ver : FW core version in FW Binary
		 *  ist30xx_fw_ver : FW version in FW Binary
		 */
		/* If the ver of binary is higher than ver of IC, FW update operate. */

		if (likely((fw->core_ver >= ist30xx_fw_ver) &&
			   (fw->param_ver >= ist30xx_param_ver)))
			return 0;
	}

fw_check_end:
	tsp_err("%s: Firmware update is needed (IC %02x < BIN %02x)\n",
		__func__, fw->param_ver, ist30xx_param_ver);
	return -EAGAIN;
}

int ist30xx_auto_bin_update(struct ist30xx_data *data)
{
	int ret = 0;
	int retry = IST30XX_FW_UPDATE_RETRY;
	struct ist30xx_fw *fw = &data->fw;

	fw->buf = (u8 *)ist30xxb_fw;
	fw->buf_size = sizeof(ist30xxb_fw);

#if IST30XX_MULTIPLE_TSP
	ist30xx_set_tsp_fw(data);
#else
	ist30xx_get_update_info(data, fw->buf, fw->buf_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw->buf);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw->buf);
	ist30xx_sub_ver = ist30xx_parse_ver(FLAG_SUB, fw->buf);
#endif

	tsp_info("%s: IC: %x, Binary ver core: %x, param: %x\n", __func__,
		 data->chip_id, ist30xx_fw_ver, ist30xx_param_ver);

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_check_auto_update(data);
	mutex_unlock(&ist30xx_mutex);

	if (likely(ret >= 0))
		return ret;

update_bin:   // TSP is not ready / FW update
	tsp_info("%s: Update version. param(core): %x(%x, %x) -> %x(%x, %x)\n",
		 __func__, fw->param_ver, fw->core_ver, fw->sub_ver,
		 ist30xx_param_ver, ist30xx_fw_ver, ist30xx_sub_ver);

	mutex_lock(&ist30xx_mutex);
	while (retry--) {
		ret = ist30xx_fw_update(data->client, fw->buf, fw->buf_size, true);
		if (unlikely(!ret))
			break;
	}
	mutex_unlock(&ist30xx_mutex);

	if (unlikely(ret))
		return ret;

	if (unlikely(retry > 0 && ist30xx_check_fw(data, fw->buf)))
		goto update_bin;

	ist30xx_calibrate(IST30XX_FW_UPDATE_RETRY);

	return ret;
}
#endif // IST30XX_INTERNAL_BIN

#define MAX_FILE_PATH   255
/* sysfs: /sys/class/touch/firmware/firmware */
ssize_t ist30xx_fw_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t size)
{
	int ret;
	int fw_size = 0;
	u8 *fw = NULL;
	const u8 *buff = 0;
	mm_segment_t old_fs = { 0 };
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	char fw_path[MAX_FILE_PATH];
	const struct firmware *request_fw = NULL;
	int mode = 0;
	int calib = 1;

	sscanf(buf, "%d %d", &mode, &calib);

	switch (mode) {
	case MASK_UPDATE_INTERNAL:
#if IST30XX_INTERNAL_BIN
		fw = ts_data->fw.buf;
		fw_size = ts_data->fw.buf_size;
#else
		tsp_warn("%s: Not support internal bin!!\n", __func__);
		return size;
#endif
		break;

	case MASK_UPDATE_FW:
		ret = request_firmware(&request_fw, IST30XXB_FW_NAME,
				       &ts_data->client->dev);
		if (ret) {
			tsp_warn("%s: File not found, %s\n", __func__, IST30XXB_FW_NAME);
			return size;
		}

		fw = (u8 *)request_fw->data;
		fw_size = (u32)request_fw->size;
		break;

	case MASK_UPDATE_SDCARD:
		old_fs = get_fs();
		set_fs(get_ds());

		snprintf(fw_path, MAX_FILE_PATH, "/sdcard/touch/firmware/%s",
			 IST30XXB_FW_NAME);
		fp = filp_open(fw_path, O_RDONLY, 0);
		if (IS_ERR(fp)) {
			tsp_warn("%s: file %s open error:%d\n", __func__, fw_path, (s32)fp);
			goto err_file_open;
		}

		fsize = fp->f_path.dentry->d_inode->i_size;

		buff = kzalloc((size_t)fsize, GFP_KERNEL);
		if (!buff) {
			tsp_warn("%s: fail to alloc buffer\n", __func__);
			goto err_alloc;
		}

		nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
		if (nread != fsize) {
			tsp_warn("%s: mismatch fw size\n", __func__);
			goto err_fw_size;
		}

		fw = (u8 *)buff;
		fw_size = (u32)fsize;

		filp_close(fp, current->files);
		tsp_info("%s: firmware is loaded!!\n", __func__);
		break;

	case MASK_UPDATE_ERASE:
		tsp_info("%s: EEPROM all erase!!\n", __func__);

		mutex_lock(&ist30xx_mutex);
		ist30xx_disable_irq(ts_data);
		ist30xxb_isp_enable(true);
		ist30xxb_isp_erase(ts_data->client, IST30XXB_ISP_ERASE_ALL, 0);
		ist30xxb_isp_enable(false);
		ist30xx_reset(false);
		ist30xx_enable_irq(ts_data);
		mutex_unlock(&ist30xx_mutex);

		ist30xx_start(ts_data);

	default:
		return size;
	}

	ist30xx_get_update_info(ts_data, fw, fw_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw);
	ist30xx_sub_ver = ist30xx_parse_ver(FLAG_SUB, fw);

	mutex_lock(&ist30xx_mutex);
	ist30xx_fw_update(ts_data->client, fw, fw_size, true);

	if (calib)
		ist30xx_calibrate(1);

	mutex_unlock(&ist30xx_mutex);    
	ist30xx_start(ts_data);

	if (request_fw != NULL)
		release_firmware(request_fw);

	if (fp != NULL) {
err_fw_size:
		kfree(buff);
err_alloc:
		filp_close(fp, NULL);
err_file_open:
		set_fs(old_fs);
	}

	return size;
}

/* sysfs: /sys/class/touch/firmware/fw_sdcard */
ssize_t ist30xx_fw_sdcard_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	int fw_size = 0;
	u8 *fw = NULL;
	const u8 *buff = 0;
	mm_segment_t old_fs = { 0 };
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	char fw_path[MAX_FILE_PATH];

	old_fs = get_fs();
	set_fs(get_ds());

	snprintf(fw_path, MAX_FILE_PATH, "/sdcard/touch/firmware/%s",
		 IST30XXB_FW_NAME);
	fp = filp_open(fw_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		tsp_info("%s: file %s open error:%d\n", __func__, fw_path, (s32)fp);
		goto err_file_open;
	}

	fsize = fp->f_path.dentry->d_inode->i_size;

	buff = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!buff) {
		tsp_info("%s: fail to alloc buffer\n", __func__);
		goto err_alloc;
	}

	nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
	if (nread != fsize) {
		tsp_info("%s: mismatch fw size\n", __func__);
		goto err_fw_size;
	}

	fw = (u8 *)buff;
	fw_size = (u32)fsize;

	filp_close(fp, current->files);
	tsp_info("%s: firmware is loaded!!\n", __func__);

	ist30xx_get_update_info(ts_data, fw, fw_size);
	ist30xx_fw_ver = ist30xx_parse_ver(FLAG_FW, fw);
	ist30xx_param_ver = ist30xx_parse_ver(FLAG_PARAM, fw);
	ist30xx_sub_ver = ist30xx_parse_ver(FLAG_SUB, fw);

	mutex_lock(&ist30xx_mutex);
	ist30xx_fw_update(ts_data->client, fw, fw_size, true);
	mutex_unlock(&ist30xx_mutex);

	ist30xx_start(ts_data);

	if (fp != NULL) {
err_fw_size:
		kfree(buff);
err_alloc:
		filp_close(fp, NULL);
err_file_open:
		set_fs(old_fs);
	}

	return 0;
}

/* sysfs: /sys/class/touch/firmware/firmware */
ssize_t ist30xx_fw_status_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	int count;

	switch (ts_data->status.update) {
	case 1:
		count = sprintf(buf, "Downloading\n");
		break;
	case 2:
		count = sprintf(buf, "Update success, ver %x(%x) -> %x(%x, %x), "
				"status : %d, gap : %d\n",
				ts_data->fw.prev_param_ver, ts_data->fw.prev_core_ver,
				ts_data->fw.param_ver, ts_data->fw.core_ver,
				ts_data->fw.sub_ver,
				CALIB_TO_STATUS(ts_data->status.calib_msg),
				CALIB_TO_GAP(ts_data->status.calib_msg));
		break;
	default:
		count = sprintf(buf, "Pass\n");
	}

	return count;
}

/* sysfs: /sys/class/touch/firmware/fw_read */
u32 buf32_eeprom[IST30XX_EEPROM_SIZE / IST30XX_DATA_LEN];
ssize_t ist30xx_fw_read_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	int i;
	u8 *buf8 = (u8 *)buf32_eeprom;;

	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(ts_data);

	ist30xxb_isp_fw_read(ts_data->client, buf32_eeprom);
	for (i = 0; i < IST30XX_EEPROM_SIZE; i += 16) {
		tsp_debug("%07x: %02x%02x %02x%02x %02x%02x %02x%02x "
			  "%02x%02x %02x%02x %02x%02x %02x%02x\n", i,
			  buf8[i], buf8[i + 1], buf8[i + 2], buf8[i + 3],
			  buf8[i + 4], buf8[i + 5], buf8[i + 6], buf8[i + 7],
			  buf8[i + 8], buf8[i + 9], buf8[i + 10], buf8[i + 11],
			  buf8[i + 12], buf8[i + 13], buf8[i + 14], buf8[i + 15]);
	}

	ist30xx_reset(false);
	ist30xx_enable_irq(ts_data);
	mutex_unlock(&ist30xx_mutex);

	ist30xx_start(ts_data);

	return 0;
}

/* sysfs: /sys/class/touch/firmware/version */
ssize_t ist30xx_fw_version_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "ID: %x, f/w core: %x, param: %x, sub: %x\n",
			ts_data->chip_id, ts_data->fw.core_ver,
			ts_data->fw.param_ver, ts_data->fw.sub_ver);

#if IST30XX_INTERNAL_BIN
	{
		char msg[128];

		ist30xx_get_update_info(ts_data, ts_data->fw.buf, ts_data->fw.buf_size);

		count += snprintf(msg, sizeof(msg),
				  " Header - f/w ver: %x, param: %x, sub: %x\r\n",
				  ist30xx_parse_ver(FLAG_FW, ts_data->fw.buf),
				  ist30xx_parse_ver(FLAG_PARAM, ts_data->fw.buf),
				  ist30xx_parse_ver(FLAG_SUB, ts_data->fw.buf));
		strncat(buf, msg, sizeof(msg));
	}
#endif

	return count;
}


#define UPDATE_DEFAULT_ATTR     (0644)
/* sysfs  */
static DEVICE_ATTR(fw_read, UPDATE_DEFAULT_ATTR, ist30xx_fw_read_show, NULL);
static DEVICE_ATTR(firmware, UPDATE_DEFAULT_ATTR,
		   ist30xx_fw_status_show, ist30xx_fw_store);
static DEVICE_ATTR(fw_sdcard, UPDATE_DEFAULT_ATTR,
		   ist30xx_fw_sdcard_show, NULL);
static DEVICE_ATTR(version, UPDATE_DEFAULT_ATTR, ist30xx_fw_version_show, NULL);

struct class *ist30xx_class;
struct device *ist30xx_fw_dev;

static struct attribute *fw_attributes[] = {
	&dev_attr_fw_read.attr,
	&dev_attr_firmware.attr,
	&dev_attr_fw_sdcard.attr,
	&dev_attr_version.attr,
	NULL,
};

static struct attribute_group fw_attr_group = {
	.attrs	= fw_attributes,
};


int ist30xx_init_update_sysfs(void)
{
	/* /sys/class/touch */
	ist30xx_class = class_create(THIS_MODULE, "touch");

	/* /sys/class/touch/firmware */
	ist30xx_fw_dev = device_create(ist30xx_class, NULL, 0, NULL, "firmware");

	/* /sys/class/touch/firmware/... */
	if (unlikely(sysfs_create_group(&ist30xx_fw_dev->kobj, &fw_attr_group)))
		tsp_err("%s: Failed to create sysfs group(%s)!\n", __func__,
			"firmware");

	ts_data->status.update = 0;
	ts_data->status.calib = 0;

	return 0;
}
