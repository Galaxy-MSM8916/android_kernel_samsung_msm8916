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


#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/stat.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/input/input_booster.h>
#include <linux/ctype.h>
#include "ist30xxc.h"
#include "ist30xxc_update.h"
#if IST30XX_DEBUG
#include "ist30xxc_misc.h"
#endif
#if IST30XX_CMCS_TEST
#include "ist30xxc_cmcs.h"
#endif

#if SEC_FACTORY_MODE
#define COMMAND_LENGTH		64
#define FACTORY_BUF_SIZE    PAGE_SIZE
#define BUILT_IN            (0)
#define UMS                 (1)

#define CMD_STATE_WAITING   (0)
#define CMD_STATE_RUNNING   (1)
#define CMD_STATE_OK        (2)
#define CMD_STATE_FAIL      (3)
#define CMD_STATE_NA        (4)

#define TSP_NODE_DEBUG      (0)
#define TSP_CM_DEBUG        (0)

#define TSP_CH_UNUSED       (0)
#define TSP_CH_SCREEN       (1)
#define TSP_CH_GTX          (2)
#define TSP_CH_KEY          (3)
#define TSP_CH_UNKNOWN      (-1)

#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func
struct tsp_cmd {
	struct list_head	list;
	const char *		cmd_name;
	void			(*cmd_func)(void *dev_data);
};

u32 ist30xx_get_fw_ver(struct ist30xx_data *data)
{
	u32 ver = 0;
	int ret = 0;

	ret = ist30xx_read_cmd(data, eHCOM_GET_VER_FW, &ver);
	if (unlikely(ret))
	{
		ist30xx_reset(data, false);
		ist30xx_start(data);
		tsp_info("%s(), ret=%d\n", __func__, ret);
		return ver;
	}

	tsp_debug("Reg addr: %x, ver: %x\n", eHCOM_GET_VER_FW, ver);

	return ver;
}

u32 ist30xx_get_fw_chksum(struct ist30xx_data *data)
{

	u32 chksum = 0;
	int ret = 0;

	ret = ist30xx_read_cmd(data, eHCOM_GET_CRC32, &chksum);
	if (unlikely(ret)) {
		ist30xx_reset(data, false);
		ist30xx_start(data);
		tsp_info("%s(), ret=%d\n", __func__, ret);
		return 0;
	}

	tsp_debug("Reg addr: %x, chksum: %x\n", eHCOM_GET_CRC32,chksum);

	return chksum;
}

#define KEY_SENSITIVITY_OFFSET  0x10
u32 key_sensitivity = 0;
int ist30xx_get_key_sensitivity(struct ist30xx_data *data, int id)
{
	u32 addr = IST30XX_DA_ADDR(data->tags.algo_base) + KEY_SENSITIVITY_OFFSET;
	u32 val = 0;
	int ret = 0;

	if (unlikely(id >= data->tkey_info.key_num))
		return 0;

	if (ist30xx_intr_wait(data, 30) < 0)
		return 0;

	ret = ist30xx_read_cmd(data, addr, &val);
	if (ret) {
		ist30xx_reset(data, false);
		ist30xx_start(data);
		tsp_info("%s(), ret=%d\n", __func__, ret);
		return 0;
	}

	if ((val & 0xFFF) == 0xFFF)
		return (key_sensitivity >> (16 * id)) & 0xFFFF;

	key_sensitivity = val;

	tsp_debug("Reg addr: %x, val: %8x\n", addr, val);

	val >>= (16 * id);

	return (int)(val & 0xFFFF);
}


/* Factory CMD function */
static void set_default_result(struct sec_factory *sec)
{
	char delim = ':';

	memset(sec->cmd_result, 0, sec->cmd_result_length);
	memcpy(sec->cmd_result, sec->cmd, strlen(sec->cmd));
	strncat(sec->cmd_result, &delim, CMD_STATE_RUNNING);
}

static void set_cmd_result(struct sec_factory *sec, char *buf, int len)
{
	strncat(sec->cmd_result, buf, len);
}

static void not_support_cmd(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", "NA");
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));

	mutex_lock(&sec->cmd_lock);
	sec->cmd_is_running = false;
	mutex_unlock(&sec->cmd_lock);

	sec->cmd_state = CMD_STATE_NA;
	dev_info(&data->client->dev, "%s: \"%s(%d)\"\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
	return;
}

static void get_chip_vendor(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", TSP_CHIP_VENDOR);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void get_chip_name(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%s", TSP_CHIP_NAME);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void get_chip_id(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);
	snprintf(buf, sizeof(buf), "%#02x", data->chip_id);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}
#include <linux/uaccess.h>
#define MAX_FW_PATH 255
static void fw_update(void *dev_data)
{
	int ret;
	char buf[16] = { 0 };
	mm_segment_t old_fs = {0};
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	u8 *fw;
	char fw_path[MAX_FW_PATH+1];
	const struct firmware *firmware = NULL;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	tsp_info("%s(), %d\n", __func__, sec->cmd_param[0]);

	switch (sec->cmd_param[0]) {
	case BUILT_IN:
		sec->cmd_state = CMD_STATE_OK;

		if (data->dt_data->fw_bin) {
			ret = request_firmware(&firmware, data->dt_data->fw_path, &data->client->dev);
			if (ret) {
				tsp_err("%s: do not request firmware: %d\n", __func__, ret);
				sec->cmd_state = CMD_STATE_FAIL;

				break;
			}

			data->fw.buf = (u8 *)firmware->data;
			data->fw.size = firmware->size;
		}
		ret = ist30xx_fw_recovery(data);
		if (ret < 0)
			sec->cmd_state = CMD_STATE_FAIL;
		break;
	case UMS:
		sec->cmd_state = CMD_STATE_OK;
		old_fs = get_fs();
		set_fs(get_ds());

		snprintf(fw_path, MAX_FW_PATH, "/sdcard/%s", IST30XX_FW_NAME);
		fp = filp_open(fw_path, O_RDONLY, 0);
		if (IS_ERR(fp)) {
			tsp_warn("%s(), file %s open error:%d\n", __func__,
					fw_path, (s32)fp);
			sec->cmd_state= CMD_STATE_FAIL;
			set_fs(old_fs);
			break;
		}

		fsize = fp->f_path.dentry->d_inode->i_size;
		if (fsize != data->fw.buf_size) {
			tsp_warn("%s(), invalid fw size!!\n", __func__);
			sec->cmd_state = CMD_STATE_FAIL;
			set_fs(old_fs);
			break;
		}
		fw = kzalloc((size_t)fsize, GFP_KERNEL);
		if (!fw) {
			tsp_warn("%s(), failed to alloc buffer for fw\n", __func__);
			sec->cmd_state = CMD_STATE_FAIL;
			filp_close(fp, NULL);
			set_fs(old_fs);
			break;
		}
		nread = vfs_read(fp, (char __user *)fw, fsize, &fp->f_pos);
		if (nread != fsize) {
			tsp_warn("%s(), failed to read fw\n", __func__);
			sec->cmd_state = CMD_STATE_FAIL;
			filp_close(fp, NULL);
			set_fs(old_fs);
			break;
		}

		filp_close(fp, current->files);
		set_fs(old_fs);
		tsp_info("%s(), ums fw is loaded!!\n", __func__);

		ret = ist30xx_get_update_info(data, fw, fsize);
		if (ret) {
			sec->cmd_state = CMD_STATE_FAIL;
			break;
		}
		data->fw.bin.main_ver = ist30xx_parse_ver(data, FLAG_MAIN, fw);
		data->fw.bin.fw_ver = ist30xx_parse_ver(data, FLAG_FW, fw);
		data->fw.bin.test_ver = ist30xx_parse_ver(data, FLAG_TEST, fw);
		data->fw.bin.core_ver = ist30xx_parse_ver(data, FLAG_CORE, fw);

		mutex_lock(&ist30xx_mutex);
		ret = ist30xx_fw_update(data, fw, fsize);
		if (ret) {
			sec->cmd_state = CMD_STATE_FAIL;
			mutex_unlock(&ist30xx_mutex);
			break;
		}

		mutex_unlock(&ist30xx_mutex);

		ist30xx_calibrate(data, 1);
		break;

	default:
		tsp_warn("%s(), Invalid fw file type!\n", __func__);
		break;
	}

	if (sec->cmd_state == CMD_STATE_OK)
		snprintf(buf, sizeof(buf), "%s", "OK");
	else
		snprintf(buf, sizeof(buf), "%s", "NG");

	if (data->dt_data->fw_bin && firmware) {
		release_firmware(firmware);
		data->fw.buf = NULL;
	}

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
}


static void get_fw_ver_bin(void *dev_data)
{
	u32 ver = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	const struct firmware *firmware = NULL;
	int ret;

	set_default_result(sec);

	if (data->dt_data->fw_bin) {
		ret = request_firmware(&firmware, data->dt_data->fw_path, &data->client->dev);
		if (ret) {
			tsp_err("%s: do not request firmware: %d\n", __func__, ret);
			sec->cmd_state = CMD_STATE_FAIL;

			return;
		}

		data->fw.buf = (u8 *)firmware->data;
		data->fw.size = firmware->size;
	}

	ver = ist30xx_parse_ver(data, FLAG_FW, data->fw.buf);
	snprintf(buf, sizeof(buf), "IM00%04x", ver);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));

	if (data->dt_data->fw_bin && firmware) {
		release_firmware(firmware);
		data->fw.buf = NULL;
	}
}
static void get_config_ver(void *dev_data)
{
        struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
        struct sec_factory *sec = (struct sec_factory *)&data->sec;
	u8 month, day;

        char buff[255] = {0};
        char name[255] = {0};

        set_default_result(sec);

#if IST30XX_INTERNAL_BIN
	month = data->tags.month;
	day = data->tags.day;
#else
	month = day = 0;
#endif

        if(data->dt_data->project_name) {
              strcpy(name, data->dt_data->project_name);
              name[0] = toupper(name[0]);
              snprintf(buff, sizeof(buff),
                            "%s_%s_%02d%02d", name, TSP_CHIP_VENDOR, month, day);
        }else {
              snprintf(buff, sizeof(buff), "%s_%s", TSP_CHIP_VENDOR, TSP_CHIP_NAME);
        }

        set_cmd_result(sec, buff, strnlen(buff, sizeof(buff)));
        sec->cmd_state = CMD_STATE_OK;
        tsp_info("%s(): %s(%d)\n", __func__, buff,
                 strnlen(buff, sizeof(buff)));
}
static void get_checksum_data(void *dev_data)
{

	char buf[16] = {0};
	u32 chksum = 0;
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->status.power == 1) {
		chksum = ist30xx_get_fw_chksum(data);
		if (chksum == 0) {
			tsp_info("%s(), Failed get the checksum data \n", __func__);
			snprintf(buf, sizeof(buf), "%s", "NG");
			set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
			sec->cmd_state = CMD_STATE_FAIL;
			return;
		}
	}

	snprintf(buf, sizeof(buf), "0x%06X", chksum);

	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void get_fw_ver_ic(void *dev_data)
{
	u32 ver = 0;
	char msg[8];
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->status.power == 1) {
		ver = ist30xx_get_fw_ver(data);
		snprintf(buf, sizeof(buf), "IM00%04x", ver & 0xFFFF);
	} else {
		snprintf(buf, sizeof(buf), "IM00%04x", data->fw.cur.fw_ver & 0xFFFF);
	}

	if (data->fw.cur.test_ver > 0) {
		sprintf(msg, "(T%d)", data->fw.cur.test_ver);
		strcat(buf, msg);
	}

	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void set_edge_mode(void *dev_data)
{
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	tsp_info("%s(), %d\n", __func__, sec->cmd_param[0]);

	switch (sec->cmd_param[0]) {
	case 0:
		sec->cmd_state = CMD_STATE_OK;
		tsp_info("%s(), Set Edge Mode\n", __func__);
		ist30xx_set_edge_mode(1);
		break;
	case 1:
		sec->cmd_state = CMD_STATE_OK;
		tsp_info("%s(), Unset Edge Mode\n", __func__);
		ist30xx_set_edge_mode(0);
		break;
	default:
		tsp_info("%s(), Invalid Argument\n", __func__);
		break;
	}
	if (sec->cmd_state == CMD_STATE_OK)
		snprintf(buf, sizeof(buf), "%s", "OK");
	else
		snprintf(buf, sizeof(buf), "%s", "NG");

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void get_threshold(void *dev_data)
{
	int ret = 0;
	u32 val = 0;
	char buf[16] = { 0 };
	int threshold;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	ret = ist30xx_read_cmd(data, eHCOM_GET_TOUCH_TH, &val);
	if (unlikely(ret)) {
		ist30xx_reset(data, false);
		ist30xx_start(data);
		tsp_info("%s(), ret=%d\n", __func__, ret);
		val = 0;
	}

	set_default_result(sec);

	threshold = (int)(val & 0xFFFF);

	snprintf(buf, sizeof(buf), "%d", threshold);
	tsp_info("%s(), %s\n", __func__, buf);

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	sec->cmd_state = CMD_STATE_OK;
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__,
			buf, strnlen(buf, sizeof(buf)));
}

static void get_scr_x_num(void *dev_data)
{
	int val = -1;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->tsp_info.dir.swap_xy)
		val = data->tsp_info.screen.tx;
	else
		val = data->tsp_info.screen.rx;

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
				strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
				"%s: fail to read num of x (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

static void get_scr_y_num(void *dev_data)
{
	int val = -1;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->tsp_info.dir.swap_xy)
		val = data->tsp_info.screen.rx;
	else
		val = data->tsp_info.screen.tx;

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
				strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
				"%s: fail to read num of y (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

static void get_all_x_num(void *dev_data)
{
	int val = - 1;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->tsp_info.dir.swap_xy)
		val = data->tsp_info.ch_num.tx;
	else
		val = data->tsp_info.ch_num.rx;

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
				strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
				"%s: fail to read all num of x (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

static void get_all_y_num(void *dev_data)
{
	int val = -1;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if (data->tsp_info.dir.swap_xy)
		val = data->tsp_info.ch_num.rx;
	else
		val = data->tsp_info.ch_num.tx;

	if (val >= 0) {
		snprintf(buf, sizeof(buf), "%u", val);
		sec->cmd_state = CMD_STATE_OK;
		dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
				strnlen(buf, sizeof(buf)));
	} else {
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
		dev_info(&data->client->dev,
				"%s: fail to read all num of y (%d).\n", __func__, val);
	}
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

/*
 *	DVFS_STAGE_NONE	0x0001  : 0000 0000 0000 0001
 *	DVFS_STAGE_SINGLE	0x0002  : 0000 0000 0000 0010
 *	DVFS_STAGE_DUAL	0x0004  : 0000 0000 0000 0100
 */
static void boost_level(void *dev_data)
{
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	char buf[16] = { 0 };

	set_default_result(sec);

	//input_booster_set_level_change(sec->cmd_param[0]);

	snprintf(buf, sizeof(buf), "%u", sec->cmd_param[0]);
	sec->cmd_state = CMD_STATE_OK;

	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));

	sec->cmd_state = CMD_STATE_WAITING;

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), %s\n", __func__, buf);
}

int check_tsp_channel(void *dev_data, int width, int height)
{
	int node = -EPERM;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	if (data->tsp_info.dir.swap_xy) {
		if ((sec->cmd_param[0] < 0) || (sec->cmd_param[0] >= height) ||
			(sec->cmd_param[1] < 0) || (sec->cmd_param[1] >= width)) {
			tsp_info("%s: parameter error: %u,%u\n",
				 __func__, sec->cmd_param[0], sec->cmd_param[1]);
		} else {
			node = sec->cmd_param[1] + sec->cmd_param[0] * width;
			tsp_info("%s: node = %d\n", __func__, node);
		}
	} else {
		if ((sec->cmd_param[0] < 0) || (sec->cmd_param[0] >= width) ||
			(sec->cmd_param[1] < 0) || (sec->cmd_param[1] >= height)) {
		tsp_info("%s: parameter error: %u,%u\n",
				__func__, sec->cmd_param[0], sec->cmd_param[1]);
		} else {
		node = sec->cmd_param[0] + sec->cmd_param[1] * width;
		tsp_info("%s: node = %d\n", __func__, node);
		}
	}

	return node;
}

static u16 node_value[IST30XX_NODE_TOTAL_NUM];
static u16 key_node_value[IST30XX_MAX_KEYS];
void get_raw_array(void *dev_data)
{
	int i, ret;
	int count = 0;
	char *buf;
	const int msg_len = 16;
	char msg[msg_len];
	u8 flag = NODE_FLAG_RAW;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_read_touch_node(data, flag, &tsp->node);
	if (ret) {
		mutex_unlock(&ist30xx_mutex);
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	mutex_unlock(&ist30xx_mutex);
	ist30xx_parse_touch_node(data, flag, &tsp->node);

	ret = parse_tsp_node(data, flag, &tsp->node, node_value, TS_RAW_ALL);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		return;
	}

	buf = kmalloc(IST30XX_NODE_TOTAL_NUM * 5, GFP_KERNEL);
	if (!buf) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}
	memset(buf, 0, IST30XX_NODE_TOTAL_NUM * 5);

	for (i = 0; i < tsp->ch_num.rx * tsp->ch_num.tx; i++) {
#if TSP_NODE_DEBUG
		if ((i % tsp->ch_num.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", node_value[i]);
#endif
		count += snprintf(msg, msg_len, "%d,", node_value[i]);
		strncat(buf, msg, msg_len);
	}

	printk("\n");
	tsp_info("%s: %d\n", __func__, count - 1);
	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf, count);
	kfree(buf);
}

void run_raw_read(void *dev_data)
{
	int i;
	int ret = 0;
	int min_val, max_val;
	char buf[16] = { 0 };
	u8 flag = NODE_FLAG_RAW;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	ret = ist30xx_read_touch_node(data, flag, &tsp->node);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	ist30xx_parse_touch_node(data, flag, &tsp->node);

	ret = parse_tsp_node(data, flag, &tsp->node, node_value, TS_RAW_SCREEN);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		return;
	}

	min_val = max_val = node_value[0];
	for (i = 0; i < tsp->screen.rx * tsp->screen.tx; i++) {         // modify block
#if TSP_NODE_DEBUG
		if ((i % tsp->screen.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", node_value[i]);
#endif
		max_val = max(max_val, (int)node_value[i]);

		min_val = min(min_val, (int)node_value[i]);
	}

	snprintf(buf, sizeof(buf), "%d,%d", min_val, max_val);
	tsp_info("%s(), %s\n", __func__, buf);

	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

void run_raw_read_key(void *dev_data)
{
	int ret = 0;
	char buf[16] = { 0 };
	u8 flag = NODE_FLAG_RAW;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	ret = ist30xx_read_touch_node(data, flag, &tsp->node);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	ist30xx_parse_touch_node(data, flag, &tsp->node);

	ret = parse_tsp_node(data, flag, &tsp->node, key_node_value, TS_RAW_KEY);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		return;
	}

	snprintf(buf, sizeof(buf), "%d,%d", key_node_value[0], key_node_value[1]);
	tsp_info("%s(), %s\n", __func__, buf);

	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

void get_raw_value(void *dev_data)
{
	int idx = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	idx = check_tsp_channel(data, tsp->screen.rx, tsp->screen.tx);
	if (idx < 0) { // Parameter parsing fail
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
	} else {
		snprintf(buf, sizeof(buf), "%d", node_value[idx]);
		sec->cmd_state = CMD_STATE_OK;
	}

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), [%d][%d]: %s\n", __func__,
			sec->cmd_param[0], sec->cmd_param[1], buf);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

extern u8 *ts_cmcs_bin;
extern u32 ts_cmcs_bin_size;
extern CMCS_BIN_INFO *ts_cmcs;
extern CMCS_BUF *ts_cmcs_buf;

void get_raw_all_data(void *dev_data) {
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;
	int i;
	int ret;
	u8 flag = NODE_FLAG_RAW;
	char *raw_buffer;
	char temp[10];

	set_default_result(sec);

	raw_buffer = kzalloc(sec->cmd_result_length, GFP_KERNEL);
	if (!raw_buffer) {
		tsp_err("%s: failed to allication memory\n", __func__);
		goto out_raw_all_data;
	}

	ret = ist30xx_read_touch_node(data, flag, &tsp->node);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		goto err_read_data;
	}
	ist30xx_parse_touch_node(data, flag, &tsp->node);

	ret = parse_tsp_node(data, flag, &tsp->node, node_value, TS_RAW_SCREEN);
	if (ret) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp node parse fail - flag: %d\n", __func__, flag);
		goto err_read_data;
	}

	for (i = 0; i < tsp->screen.rx * tsp->screen.tx; i++) {
		snprintf(temp, 10, "%d,", node_value[i]);
		strncat(raw_buffer, temp, 10);
	}

	set_cmd_result(sec, raw_buffer, sec->cmd_result_length);

	sec->cmd_state = CMD_STATE_OK;

	kfree(raw_buffer);
	return;

err_read_data:
	kfree(raw_buffer);
out_raw_all_data:
	sec->cmd_state = CMD_STATE_FAIL;
}

int get_read_all_data(struct ist30xx_data *data, u8 flag)
{
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	int ii;
	int count = 0;
	int type;
	char *buffer;
	char *temp;
	int ret;

	if ((ts_cmcs_bin == NULL) || (ts_cmcs_bin_size == 0)) {
		tsp_err("%s: cmcs binary is NULL!\n", __func__);
		goto cm_err_out;
	}

	ret = ist30xx_get_cmcs_info(ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		tsp_err("%s: tsp get cmcs infomation fail!\n", __func__);
		goto cm_err_out;
	}

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_test(data, ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		mutex_unlock(&ist30xx_mutex);
		tsp_err("%s: tsp cmcs test fail!\n", __func__);
		goto cm_err_out;
	}
	mutex_unlock(&ist30xx_mutex);

	buffer = kzalloc(sec->cmd_result_length, GFP_KERNEL);
	if (!buffer) {
		tsp_err("%s: failed to buffer alloc\n", __func__);
		goto cm_err_out;
	}

	temp = kzalloc(10, GFP_KERNEL);
	if (!temp) {
		tsp_err("%s: failed to temp alloc\n", __func__);
		goto cm_err_alloc_out;
	}

	for (ii = 0; ii < tsp->ch_num.rx * tsp->ch_num.tx; ii++) {

		type = check_tsp_type(data, ii / tsp->ch_num.rx, ii % tsp->ch_num.rx);
		if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED)) {
			count += snprintf(temp, 10, "%d,", 0);
		} else {
			switch (flag) {
			case TEST_CM_ALL_DATA:
				count += snprintf(temp, 10, "%d,", ts_cmcs_buf->cm[ii]);
				break;
			case TEST_SLOPE0_ALL_DATA:
				count += snprintf(temp, 10, "%d,", ts_cmcs_buf->slope0[ii]);
				break;
			case TEST_SLOPE1_ALL_DATA:
				count += snprintf(temp, 10, "%d,", ts_cmcs_buf->slope1[ii]);
				break;
			case TEST_CS_ALL_DATA:
				count += snprintf(temp, 10, "%d,", ts_cmcs_buf->cs[ii]);
				break;
			}
		}
		strncat(buffer, temp, 10);
	}

	set_cmd_result(sec, buffer, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buffer, count);
	kfree(buffer);
	kfree(temp);
	return 0;

cm_err_alloc_out:
	kfree(buffer);
cm_err_out:
	sec->cmd_state = CMD_STATE_FAIL;
	set_cmd_result(sec, "NULL", sec->cmd_result_length);
	return -1;

}

void get_cm_all_data(void *dev_data) {
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	int ret;

	set_default_result(sec);

	ret = get_read_all_data(data, TEST_CM_ALL_DATA);
	if (ret < 0)
		sec->cmd_state = CMD_STATE_FAIL;
	else
		sec->cmd_state = CMD_STATE_OK;

}

void get_slope0_all_data(void *dev_data) {
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	int ret;

	set_default_result(sec);

	ret = get_read_all_data(data, TEST_SLOPE0_ALL_DATA);
	if (ret < 0)
		sec->cmd_state = CMD_STATE_FAIL;
	else
		sec->cmd_state = CMD_STATE_OK;
}

void get_slope1_all_data(void *dev_data) {
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	int ret;

	set_default_result(sec);

	ret = get_read_all_data(data, TEST_SLOPE1_ALL_DATA);
	if (ret < 0)
		sec->cmd_state = CMD_STATE_FAIL;
	else
		sec->cmd_state = CMD_STATE_OK;
}

void get_cs_all_data(void *dev_data) {
	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	int ret;

	set_default_result(sec);

	ret = get_read_all_data(data, TEST_CS_ALL_DATA);
	if (ret < 0)
		sec->cmd_state = CMD_STATE_FAIL;
	else
		sec->cmd_state = CMD_STATE_OK;
}

void run_cm_test(void *dev_data)
{
	int i, j;
	int ret = 0;
	char buf[16] = { 0 };
	int type, idx;
	int min_val, max_val;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	if ((ts_cmcs_bin == NULL) || (ts_cmcs_bin_size == 0)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), Binary is not correct!\n", __func__);
		return;
	}

	ret = ist30xx_get_cmcs_info(ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp get cmcs infomation fail!\n", __func__);
		return;
	}

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_test(data, ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		mutex_unlock(&ist30xx_mutex);
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp cmcs test fail!\n", __func__);
		return;
	}
	mutex_unlock(&ist30xx_mutex);

	min_val = max_val = ts_cmcs_buf->cm[0];
	for (i = 0; i < tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		printk("%s", IST30XX_DEBUG_TAG);
#endif
		for (j = 0; j < tsp->ch_num.rx; j++) {
			idx = (i * tsp->ch_num.rx) + j;
			type = check_tsp_type(data, i, j);

			if ((type == TSP_CH_SCREEN) || (type == TSP_CH_GTX)) {
				max_val = max(max_val, (int)ts_cmcs_buf->cm[idx]);
				min_val = min(min_val, (int)ts_cmcs_buf->cm[idx]);
			}
#if TSP_CM_DEBUG
			printk("%5d ", ts_cmcs_buf->cm[idx]);
#endif
		}
#if TSP_CM_DEBUG
		printk("\n");
#endif
	}

	snprintf(buf, sizeof(buf), "%d,%d", min_val, max_val);
	tsp_info("%s(), %s\n", __func__, buf);

	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

void run_cm_test_key(void *dev_data)
{
	int i, j;
	int ret = 0;
	char buf[16] = { 0 };
	int type, idx;
	int cm_key[IST30XX_MAX_KEYS] = { 0 };
	int key_count = 0;

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	if ((ts_cmcs_bin == NULL) || (ts_cmcs_bin_size == 0)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), Binary is not correct!\n", __func__);
		return;
	}

	ret = ist30xx_get_cmcs_info(ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp get cmcs infomation fail!\n", __func__);
		return;
	}

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_test(data, ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		mutex_unlock(&ist30xx_mutex);
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp cmcs test fail!\n", __func__);
		return;
	}
	mutex_unlock(&ist30xx_mutex);

	for (i = 0; i < tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		printk("%s", IST30XXC_DEBUG_TAG);
#endif
		for (j = 0; j < tsp->ch_num.rx; j++) {
			idx = (i * tsp->ch_num.rx) + j;
			type = check_tsp_type(data, i, j);

			if (type == TSP_CH_KEY) {
				cm_key[key_count++] = (int)ts_cmcs_buf->cm[idx];
			}
#if TSP_CM_DEBUG
			printk("%5d ", tsp_cmcs_buf->cm[idx]);
#endif
		}
#if TSP_CM_DEBUG
		printk("\n");
#endif
	}

	snprintf(buf, sizeof(buf), "%d,%d", cm_key[0], cm_key[1]);
	tsp_info("%s(), %s\n", __func__, buf);

	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

void get_cm_value(void *dev_data)
{
	int idx = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	idx = check_tsp_channel(data, tsp->ch_num.rx, tsp->ch_num.tx);
	if (idx < 0) { // Parameter parsing fail
		snprintf(buf, sizeof(buf), "%s", "NG");
		sec->cmd_state = CMD_STATE_FAIL;
	} else {
		snprintf(buf, sizeof(buf), "%d", ts_cmcs_buf->cm[idx]);
		sec->cmd_state = CMD_STATE_OK;
	}

	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	tsp_info("%s(), [%d][%d]: %s\n", __func__,
			sec->cmd_param[0], sec->cmd_param[1], buf);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf,
			strnlen(buf, sizeof(buf)));
}

void run_cmcs_test(void *dev_data)
{
	int ret = 0;
	char buf[16] = { 0 };

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	set_default_result(sec);

	if ((ts_cmcs_bin == NULL) || (ts_cmcs_bin_size == 0)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), Binary is not correct!\n", __func__);
		return;
	}

	ret = ist30xx_get_cmcs_info(ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp get cmcs infomation fail!\n", __func__);
		return;
	}

	mutex_lock(&ist30xx_mutex);
	ret = ist30xx_cmcs_test(data, ts_cmcs_bin, ts_cmcs_bin_size);
	if (unlikely(ret)) {
		mutex_unlock(&ist30xx_mutex);
		sec->cmd_state = CMD_STATE_FAIL;
		tsp_warn("%s(), tsp cmcs test fail!\n", __func__);
		return;
	}
	mutex_unlock(&ist30xx_mutex);

	snprintf(buf, sizeof(buf), "%s", "OK");
	tsp_info("%s(), %s\n", __func__, buf);

	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, strnlen(buf, sizeof(buf)));
	dev_info(&data->client->dev, "%s: %s(%d)\n",
			__func__, buf,	strnlen(buf, sizeof(buf)));
}

void get_cm_array(void *dev_data)
{
	int i;
	int count = 0;
	int type;
	char *buf;
	const int msg_len = 16;
	char msg[msg_len];

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	buf = kmalloc(IST30XX_NODE_TOTAL_NUM * 5, GFP_KERNEL);
	if (!buf) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}
	memset(buf, 0, IST30XX_NODE_TOTAL_NUM * 5);

	for (i = 0; i < tsp->ch_num.rx * tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		if ((i % tsp->ch_num.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", ts_cmcs_buf->cm[i]);
#endif
		type = check_tsp_type(data, i / tsp->ch_num.rx, i % tsp->ch_num.rx);
		if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED))
			count += snprintf(msg, msg_len, "%d,", 0);
		else
			count += snprintf(msg, msg_len, "%d,", ts_cmcs_buf->cm[i]);
		strncat(buf, msg, msg_len);
	}

	printk("\n");
	tsp_info("%s: %d\n", __func__, count - 1);
	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf, count);
	kfree(buf);
}

void get_slope0_array(void *dev_data)
{
	int i;
	int count = 0;
	char *buf;
	const int msg_len = 16;
	char msg[msg_len];

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	buf = kmalloc(IST30XX_NODE_TOTAL_NUM * 5, GFP_KERNEL);
	if (!buf) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}
	memset(buf, 0, IST30XX_NODE_TOTAL_NUM * 5);

	for (i = 0; i < tsp->ch_num.rx * tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		if ((i % tsp->ch_num.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", ts_cmcs_buf->slope0[i]);
#endif
		count += snprintf(msg, msg_len, "%d,", ts_cmcs_buf->slope0[i]);
		strncat(buf, msg, msg_len);
	}

	printk("\n");
	tsp_info("%s: %d\n", __func__, count - 1);
	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf, count);
	kfree(buf);
}

void get_slope1_array(void *dev_data)
{
	int i;
	int count = 0;
	char *buf;
	const int msg_len = 16;
	char msg[msg_len];

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	buf = kmalloc(IST30XX_NODE_TOTAL_NUM * 5, GFP_KERNEL);
	if (!buf) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}
	memset(buf, 0, IST30XX_NODE_TOTAL_NUM * 5);

	for (i = 0; i < tsp->ch_num.rx * tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		if ((i % tsp->ch_num.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", ts_cmcs_buf->slope1[i]);
#endif
		count += snprintf(msg, msg_len, "%d,", ts_cmcs_buf->slope1[i]);
		strncat(buf, msg, msg_len);
	}

	printk("\n");
	tsp_info("%s: %d\n", __func__, count - 1);
	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf, count);
	kfree(buf);
}

void get_cs_array(void *dev_data)
{
	int i;
	int count = 0;
	int type;
	char *buf;
	const int msg_len = 16;
	char msg[msg_len];

	struct ist30xx_data *data = (struct ist30xx_data *)dev_data;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	TSP_INFO *tsp = &data->tsp_info;

	set_default_result(sec);

	buf = kmalloc(IST30XX_NODE_TOTAL_NUM * 5, GFP_KERNEL);
	if (!buf) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}
	memset(buf, 0, IST30XX_NODE_TOTAL_NUM * 5);

	for (i = 0; i < tsp->ch_num.rx * tsp->ch_num.tx; i++) {
#if TSP_CM_DEBUG
		if ((i % tsp->ch_num.rx) == 0)
			printk("\n%s %4d: ", IST30XX_DEBUG_TAG, i);

		printk("%4d ", ts_cmcs_buf->cs[i]);
#endif
		type = check_tsp_type(data, i / tsp->ch_num.rx, i % tsp->ch_num.rx);
		if ((type == TSP_CH_UNKNOWN) || (type == TSP_CH_UNUSED))
			count += snprintf(msg, msg_len, "%d,", 0);
		else
			count += snprintf(msg, msg_len, "%d,", ts_cmcs_buf->cs[i]);
		strncat(buf, msg, msg_len);
	}

	printk("\n");
	tsp_info("%s: %d\n", __func__, count - 1);
	sec->cmd_state = CMD_STATE_OK;
	set_cmd_result(sec, buf, count - 1);
	dev_info(&data->client->dev, "%s: %s(%d)\n", __func__, buf, count);
	kfree(buf);
}

/* sysfs: /sys/class/sec/tsp/close_tsp_test */
static ssize_t show_close_tsp_test(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, FACTORY_BUF_SIZE, "%u\n", 0);
}

/* sysfs: /sys/class/sec/tsp/cmd */
static ssize_t store_cmd(struct device *dev, struct device_attribute
		*devattr, const char *buf, size_t count)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	struct i2c_client *client = data->client;

	char *cur, *start, *end;
	char msg[SEC_CMD_STR_LEN] = { 0 };
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;
	int ret;

	if (sec->cmd_is_running == true) {
		dev_err(&client->dev, "tsp_cmd: other cmd is running.\n");
		tsp_err("tsp_cmd: other cmd is running.\n");
		goto err_out;
	}

	/* check lock  */
	mutex_lock(&sec->cmd_lock);
	sec->cmd_is_running = true;
	mutex_unlock(&sec->cmd_lock);

	sec->cmd_state = CMD_STATE_RUNNING;

	for (i = 0; i < ARRAY_SIZE(sec->cmd_param); i++)
		sec->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(sec->cmd, 0, ARRAY_SIZE(sec->cmd));
	memcpy(sec->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(msg, buf, cur - buf);
	else
		memcpy(msg, buf, len);
	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &sec->cmd_list_head, list) {
		if (!strcmp(msg, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &sec->cmd_list_head, list) {
			if (!strcmp("not_support_cmd", tsp_cmd_ptr->cmd_name))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(msg, 0, ARRAY_SIZE(msg));
		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(msg, start, end - start);
				*(msg + strlen(msg)) = '\0';
				ret = kstrtoint(msg, 10, \
						sec->cmd_param + param_cnt);
				start = cur + 1;
				memset(msg, 0, ARRAY_SIZE(msg));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}
	tsp_info("SEC CMD = %s\n", tsp_cmd_ptr->cmd_name);

	for (i = 0; i < param_cnt; i++)
		tsp_info("SEC CMD Param %d= %d\n", i, sec->cmd_param[i]);

	tsp_cmd_ptr->cmd_func(data);

err_out:
	return count;
}

/* sysfs: /sys/class/sec/tsp/cmd_status */
static ssize_t show_cmd_status(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;
	char msg[16] = { 0 };

	dev_info(&data->client->dev, "tsp cmd: status:%d\n", sec->cmd_state);

	if (sec->cmd_state == CMD_STATE_WAITING)
		snprintf(msg, sizeof(msg), "WAITING");
	else if (sec->cmd_state == CMD_STATE_RUNNING)
		snprintf(msg, sizeof(msg), "RUNNING");
	else if (sec->cmd_state == CMD_STATE_OK)
		snprintf(msg, sizeof(msg), "OK");
	else if (sec->cmd_state == CMD_STATE_FAIL)
		snprintf(msg, sizeof(msg), "FAIL");
	else if (sec->cmd_state == CMD_STATE_NA)
		snprintf(msg, sizeof(msg), "NOT_APPLICABLE");

	return snprintf(buf, FACTORY_BUF_SIZE, "%s\n", msg);
}

/* sysfs: /sys/class/sec/tsp/cmd_result */
static ssize_t show_cmd_result(struct device *dev, struct device_attribute
		*devattr, char *buf)
{
	struct ist30xx_data *data = dev_get_drvdata(dev);
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	dev_info(&data->client->dev, "tsp cmd: result: %s\n", sec->cmd_result);

	mutex_lock(&sec->cmd_lock);
	sec->cmd_is_running = false;
	mutex_unlock(&sec->cmd_lock);

	sec->cmd_state = CMD_STATE_WAITING;

	return snprintf(buf, FACTORY_BUF_SIZE, "%s\n", sec->cmd_result);
}

/* sysfs: /sys/class/sec/sec_touchkey/touchkey_recent */
static ssize_t recent_sensitivity_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int sensitivity = 0;
	struct ist30xx_data *data = dev_get_drvdata(dev);

	sensitivity = ist30xx_get_key_sensitivity(data, 0);

	tsp_info("%s(), %d\n", __func__, sensitivity);

	return sprintf(buf, "%d", sensitivity);
}

/* sysfs: /sys/class/sec/sec_touchkey/touchkey_back */
static ssize_t back_sensitivity_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int sensitivity = 0;
	struct ist30xx_data *data = dev_get_drvdata(dev);

	sensitivity = ist30xx_get_key_sensitivity(data, 1);

	tsp_info("%s(), %d\n", __func__, sensitivity);

	return sprintf(buf, "%d", sensitivity);
}

/* sysfs: /sys/class/sec/sec_touchkey/touchkey_threshold */
static ssize_t touchkey_threshold_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	u32 val = 0;
	int threshold = 0;
	struct ist30xx_data *data = dev_get_drvdata(dev);

	ret = ist30xx_read_cmd(data, eHCOM_GET_TOUCH_TH, &val);
	if (unlikely(ret)) {
		ist30xx_reset(data, false);
		ist30xx_start(data);
		val = 0;
	}

	threshold = (int)((val >> 16) & 0xFFFF);

	tsp_info("%s(), %d (ret=%d)\n", __func__, threshold, ret);

	return sprintf(buf, "%d", threshold);
}

struct tsp_cmd tsp_cmds[] = {
	{ TSP_CMD("get_chip_vendor", get_chip_vendor), },
	{ TSP_CMD("get_chip_name",   get_chip_name),   },
	{ TSP_CMD("get_chip_id",     get_chip_id),     },
	{ TSP_CMD("fw_update",	     fw_update),       },
	{ TSP_CMD("get_fw_ver_bin",  get_fw_ver_bin),  },
	{ TSP_CMD("get_fw_ver_ic",   get_fw_ver_ic),   },
	{ TSP_CMD("get_threshold",   get_threshold),   },
	{ TSP_CMD("get_x_num",	     get_scr_x_num),   },
	{ TSP_CMD("get_y_num",	     get_scr_y_num),   },
	{ TSP_CMD("get_all_x_num",	 get_all_x_num),},
	{ TSP_CMD("get_all_y_num",	 get_all_y_num),},
	{ TSP_CMD("clear_cover_mode", not_support_cmd),},
	{ TSP_CMD("boost_level", boost_level),},
	{ TSP_CMD("run_reference_read", run_raw_read),  },
	{ TSP_CMD("run_reference_read_key", run_raw_read_key),},
	{ TSP_CMD("get_reference",   get_raw_value),   },
	{ TSP_CMD("run_raw_read",    run_raw_read),    },
	{ TSP_CMD("run_raw_read_key", run_raw_read_key),},
	{ TSP_CMD("get_raw_value",   get_raw_value),   },
	{ TSP_CMD("get_raw_all_data", get_raw_all_data),},
	{ TSP_CMD("get_cm_all_data", get_cm_all_data),},
	{ TSP_CMD("get_slope0_all_data", get_slope0_all_data),},
	{ TSP_CMD("get_slope1_all_data", get_slope1_all_data),},
	{ TSP_CMD("get_cs_all_data", get_cs_all_data),},
	{ TSP_CMD("get_checksum_data", get_checksum_data),},
	{ TSP_CMD("run_cm_test",     run_cm_test),     },
	{ TSP_CMD("get_cm_value",    get_cm_value),    },
	{ TSP_CMD("run_cm_test_key", run_cm_test_key),},
	{ TSP_CMD("dead_zone_enable", set_edge_mode),},
	{ TSP_CMD("get_raw_array",   get_raw_array),   },
	{ TSP_CMD("run_cmcs_test",   run_cmcs_test),   },
	{ TSP_CMD("get_cm_array",    get_cm_array),    },
	{ TSP_CMD("get_slope0_array", get_slope0_array),},
	{ TSP_CMD("get_slope1_array", get_slope1_array),},
	{ TSP_CMD("get_cs_array",    get_cs_array),    },
	{ TSP_CMD("get_cs0_array",   get_cs_array),    },
	{ TSP_CMD("get_cs1_array",   get_cs_array),    },
	{ TSP_CMD("get_config_ver", get_config_ver),   },
	{ TSP_CMD("not_support_cmd", not_support_cmd), },
};

/* sysfs: /sys/class/sec/tsp/cmd_list */
static ssize_t show_cmd_list(struct device *dev, struct device_attribute
		*devattr, char *buf)
{
	char *buffer;
	char *temp;
	unsigned int ii = 0;
	int ret;

	buffer = kzalloc(FACTORY_BUF_SIZE, GFP_KERNEL);
	if (!buffer)
		goto err_alloc;

	temp = kzalloc(COMMAND_LENGTH, GFP_KERNEL);
	if (!temp)
		goto err_alloc_tmp;

	snprintf(buffer, COMMAND_LENGTH, "++factory command list++\n");

	if (!tsp_cmds[ii].cmd_name)
		goto err_cmd;

	while(strncmp(tsp_cmds[ii].cmd_name, "not_support_cmd", 16) != 0) {
		snprintf(temp, COMMAND_LENGTH, "%s\n", tsp_cmds[ii].cmd_name);
		strncat(buffer, temp, COMMAND_LENGTH);
		ii++;
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", buffer);

	tsp_info("%s(%d)\n", buffer, ii);

	kfree(buffer);
	kfree(temp);

	return ret;
err_cmd:
	kfree(temp);
err_alloc_tmp:
	kfree(buffer);
err_alloc:
	return snprintf(buf, PAGE_SIZE, "NULL\n");
}
#if defined(CONFIG_TOUCH_KEY_LED)
static ssize_t touch_led_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct ist30xx_data *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	int data;
	int ret;

	ret = sscanf(buf, "%d", &data);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return size;
	}

	if (!(data == 0 || data == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
				__func__, data);
		return size;
	}


	tsp_info("[TKEY] %s : %d\n", __func__, data);

	if (data == 1) {
		if (info->dt_data->keyled_en_gpio >= 0)
			gpio_direction_output(info->dt_data->keyled_en_gpio, 1);
	} else {
		if (info->dt_data->keyled_en_gpio >= 0)
			gpio_direction_output(info->dt_data->keyled_en_gpio, 0);
	}

	return size;
}
#endif

/* sysfs - touchkey */
static DEVICE_ATTR(touchkey_menu, S_IRUGO,
		recent_sensitivity_show, NULL);
static DEVICE_ATTR(touchkey_recent, S_IRUGO,
		recent_sensitivity_show, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO,
		back_sensitivity_show, NULL);
static DEVICE_ATTR(touchkey_threshold, S_IRUGO,
		touchkey_threshold_show, NULL);

/* sysfs - tsp */
static DEVICE_ATTR(close_tsp_test, S_IRUGO, show_close_tsp_test, NULL);
static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);
static DEVICE_ATTR(cmd_list, S_IRUGO, show_cmd_list, NULL);
#if defined(CONFIG_TOUCH_KEY_LED)
static DEVICE_ATTR(brightness, 0664, NULL, touch_led_control);
#endif
static struct attribute *sec_tkey_attributes[] = {
	&dev_attr_touchkey_menu.attr,
	&dev_attr_touchkey_recent.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_threshold.attr,
#if defined(CONFIG_TOUCH_KEY_LED)
	&dev_attr_brightness.attr,
#endif
	NULL,
};

static struct attribute *sec_touch_facotry_attributes[] = {
	&dev_attr_close_tsp_test.attr,
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	&dev_attr_cmd_list.attr,
	NULL,
};

static struct attribute_group sec_tkey_attr_group = {
	.attrs	= sec_tkey_attributes,
};

static struct attribute_group sec_touch_factory_attr_group = {
	.attrs	= sec_touch_facotry_attributes,
};

struct device *sec_touchkey;
struct device *sec_fac_dev;

int sec_touch_sysfs(struct ist30xx_data *data)
{
	int ret;

	/* /sys/class/sec/sec_touchkey */
	if (!data->dt_data->tkey) {
		sec_touchkey = device_create(sec_class, NULL, 1, data, "sec_touchkey");
		if (IS_ERR(sec_touchkey)) {
			tsp_err("Failed to create device (%s)!\n", "sec_touchkey");
			goto err_sec_touchkey;
		}
		/* /sys/class/sec/sec_touchkey/... */
		if (sysfs_create_group(&sec_touchkey->kobj, &sec_tkey_attr_group)) {
			tsp_err("Failed to create sysfs group(%s)!\n", "sec_touchkey");
			goto err_sec_touchkey_attr;
		}
	}
	/* /sys/class/sec/tsp */
	sec_fac_dev = device_create(sec_class, NULL, 2, data, "tsp");
	if (IS_ERR(sec_fac_dev)) {
		tsp_err("Failed to create device (%s)!\n", "tsp");
		goto err_sec_fac_dev;
	}
	ret = sysfs_create_link(&sec_fac_dev->kobj, &data->input_dev->dev.kobj, "input");
	if (ret < 0)
		tsp_err("%s: Failed to create input symbolic link\n", __func__);
	/* /sys/class/sec/tsp/... */
	if (sysfs_create_group(&sec_fac_dev->kobj, &sec_touch_factory_attr_group)) {
		tsp_err("Failed to create sysfs group(%s)!\n", "tsp");
		goto err_sec_fac_dev_attr;
	}

	return 0;

err_sec_fac_dev_attr:
	device_destroy(sec_class, 2);
err_sec_fac_dev:
err_sec_touchkey_attr:
        if (!data->dt_data->tkey) {
		device_destroy(sec_class, 1);
	}
err_sec_touchkey:
	return -ENODEV;
}
EXPORT_SYMBOL(sec_touch_sysfs);
int sec_fac_cmd_init(struct ist30xx_data *data)
{
	int i;
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	INIT_LIST_HEAD(&sec->cmd_list_head);
	for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
		list_add_tail(&tsp_cmds[i].list, &sec->cmd_list_head);

	sec->cmd_result_length = IST30XX_NODE_TOTAL_NUM * (sizeof(u32) / sizeof(char) + 1);
	sec->cmd_result = kzalloc(sec->cmd_result_length, GFP_KERNEL);
	if (!sec->cmd_result) {
		tsp_err("%s: cmd_result is not allocated memory\n", __func__);
		return -ENOMEM;
	}

	tsp_info("%s: cmd_result lenght is %d\n", __func__, sec->cmd_result_length);

	mutex_init(&sec->cmd_lock);
	sec->cmd_is_running = false;

	return 0;
}
EXPORT_SYMBOL(sec_fac_cmd_init);

void sec_touch_sysfs_remove(struct ist30xx_data *data)
{
	sysfs_remove_link(&sec_fac_dev->kobj, "input");
	sysfs_remove_group(&sec_fac_dev->kobj, &sec_touch_factory_attr_group);
	if (!data->dt_data->tkey) {
		sysfs_remove_group(&sec_touchkey->kobj, &sec_tkey_attr_group);
		device_destroy(sec_class, 1);
	}
	device_destroy(sec_class, 2);
}
EXPORT_SYMBOL(sec_touch_sysfs_remove);

void sec_fac_cmd_remove(struct ist30xx_data *data)
{
	struct sec_factory *sec = (struct sec_factory *)&data->sec;

	list_del(&sec->cmd_list_head);
	mutex_destroy(&sec->cmd_lock);
}
EXPORT_SYMBOL(sec_fac_cmd_remove);
#endif
