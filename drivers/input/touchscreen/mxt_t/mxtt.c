/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c/mxtt.h>
#include <asm/unaligned.h>
#include <linux/firmware.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/qpnp/pin.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#if TSP_USE_ATMELDBG
#include <asm/bug.h>
#endif

#define USE_T100_MULTI_SLOT
extern unsigned int system_rev;
static int mxt_read_mem(struct mxt_data *data, u16 reg, u16 len, void *buf)
{
	int ret = 0, i = 0;
	u16 le_reg = cpu_to_le16(reg);
	struct i2c_msg msg[2] = {
		{
			.addr = data->client->addr,
			.flags = 0,
			.len = 2,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = data->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

	for (i = 0; i < 3 ; i++) {
		ret = i2c_transfer(data->client->adapter, msg, 2);
		if (ret < 0){
			tsp_debug_err(true, &data->client->dev, "%s fail[%d] address[0x%x]\n",
				__func__, ret, le_reg);
			if(gpio_get_value(data->pdata->tsp_en1) == 0)
				data->pdata->power_ctrl(data,1); //Turn on TSP chip if TSP_PWR_EN gpio is turned off
			continue;

			}
		else if (2 == ret)
			break;
	}

	if (2 != ret)
		tsp_debug_err(true, &data->client->dev,
			"%s fail[%d] address[0x%x]\n", __func__, ret, le_reg);

	return ret == 2 ? 0 : -EIO;
}

static int mxt_write_mem(struct mxt_data *data,
		u16 reg, u16 len, const u8 *buf)
{
	int ret = 0, i = 0;
	u8 tmp[len + 2];

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

	put_unaligned_le16(cpu_to_le16(reg), tmp);
	memcpy(tmp + 2, buf, len);

	for (i = 0; i < 3 ; i++) {
		ret = i2c_master_send(data->client, tmp, sizeof(tmp));
		if (ret < 0)
			tsp_debug_err(true, &data->client->dev,	"%s %d times write error on address[0x%x,0x%x]\n",
				__func__, i, tmp[1], tmp[0]);
		else
			break;
	}

	return ret == sizeof(tmp) ? 0 : -EIO;
}

static struct mxt_object *
	mxt_get_object(struct mxt_data *data, u8 type)
{
	struct mxt_object *object;
	int i;

	if (!data->objects) {
		tsp_debug_err(true, &data->client->dev,
			"%s object is NULL\n", __func__);
		return NULL;
	}

	for (i = 0; i < data->info.object_num; i++) {
		object = data->objects + i;
		if (object->type == type)
			return object;
	}

	tsp_debug_err(true, &data->client->dev, "Invalid object type T%d\n",
		type);

	return NULL;
}

static int mxt_read_message(struct mxt_data *data,
				 struct mxt_message *message)
{
	struct mxt_object *object;

	object = mxt_get_object(data, MXT_GEN_MESSAGEPROCESSOR_T5);
	if (!object)
		return -EINVAL;

	return mxt_read_mem(data, object->start_address,
			sizeof(struct mxt_message), message);
}

static int mxt_read_message_reportid(struct mxt_data *data,
	struct mxt_message *message, u8 reportid)
{
	int try = 0;
	int error;
	int fail_count;

	fail_count = data->max_reportid * 2;

	while (++try < fail_count) {
		error = mxt_read_message(data, message);
		if (error)
			return error;

		if (message->reportid == 0xff)
			continue;

		if (message->reportid == reportid)
			return 0;
	}

	return -EINVAL;
}

static int mxt_read_object(struct mxt_data *data,
				u8 type, u16 offset, u8 *val)
{
	struct mxt_object *object;
	int error = 0;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	error = mxt_read_mem(data, object->start_address + offset, 1, val);
	if (error)
		tsp_debug_err(true, &data->client->dev, "Error to read T[%d] offset[%d] val[%d]\n",
			type, offset, *val);
	return error;
}

static int mxt_write_object(struct mxt_data *data,
				 u8 type, u16 offset, u8 val)
{
	struct mxt_object *object;
	int error = 0;
	u16 reg;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if (offset >= object->size * object->instances) {
		tsp_debug_err(true, &data->client->dev, "Tried to write outside object T%d offset:%d, size:%d\n",
			type, offset, object->size);
		return -EINVAL;
	}
	reg = object->start_address;
	error = mxt_write_mem(data, reg + offset, 1, &val);
	if (error)
		tsp_debug_err(true, &data->client->dev, "Error to write T[%d] offset[%d] val[%d]\n",
			type, offset, val);

	return error;
}

static u32 mxt_make_crc24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 res;
	u16 data_word;

	data_word = (((u16)byte2) << 8) | byte1;
	res = (crc << 1) ^ (u32)data_word;

	if (res & 0x1000000)
		res ^= crcpoly;

	return res;
}

static int mxt_calculate_infoblock_crc(struct mxt_data *data,
		u32 *crc_pointer)
{
	u32 crc = 0;
	u8 mem[7 + data->info.object_num * 6];
	int ret;
	int i;

	ret = mxt_read_mem(data, 0, sizeof(mem), mem);

	if (ret)
		return ret;

	for (i = 0; i < sizeof(mem) - 1; i += 2)
		crc = mxt_make_crc24(crc, mem[i], mem[i + 1]);

	*crc_pointer = mxt_make_crc24(crc, mem[i], 0) & 0x00FFFFFF;

	return 0;
}

static int mxt_read_info_crc(struct mxt_data *data, u32 *crc_pointer)
{
	u16 crc_address;
	u8 msg[3];
	int ret;

	/* Read Info block CRC address */
	crc_address = MXT_OBJECT_TABLE_START_ADDRESS +
			data->info.object_num * MXT_OBJECT_TABLE_ELEMENT_SIZE;

	ret = mxt_read_mem(data, crc_address, 3, msg);
	if (ret)
		return ret;

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	return 0;
}
static int mxt_read_config_crc(struct mxt_data *data, u32 *crc)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	struct mxt_object *object;
	int error;

	object = mxt_get_object(data, MXT_GEN_COMMANDPROCESSOR_T6);
	if (!object)
		return -EIO;

	/* Try to read the config checksum of the existing cfg */
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
		MXT_COMMAND_REPORTALL, 1);

	/* Read message from command processor, which only has one report ID */
	error = mxt_read_message_reportid(data, &message, object->max_reportid);
	if (error) {
		tsp_debug_err(true, dev, "Failed to retrieve CRC\n");
		return error;
	}

	/* Bytes 1-3 are the checksum. */
	*crc = message.message[1] | (message.message[2] << 8) |
		(message.message[3] << 16);

	return 0;
}

static int mxt_check_instance(struct mxt_data *data, u8 type)
{
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		if (data->objects[i].type == type)
			return data->objects[i].instances;
	}
	return 0;
}

static int mxt_init_write_config(struct mxt_data *data,
		u8 type, const u8 *cfg)
{
	struct mxt_object *object;
	u8 *temp;
	int ret;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if ((object->size == 0) || (object->start_address == 0)) {
		tsp_debug_err(true, &data->client->dev,	"%s error T%d\n",
			 __func__, type);
		return -ENODEV;
	}

	ret = mxt_write_mem(data, object->start_address,
			object->size, cfg);
	if (ret) {
		tsp_debug_err(true, &data->client->dev,	"%s write error T%d address[0x%x]\n",
			__func__, type, object->start_address);
		return ret;
	}

	if (mxt_check_instance(data, type)) {
		temp = kzalloc(object->size, GFP_KERNEL);

		if (temp == NULL)
			return -ENOMEM;

		ret |= mxt_write_mem(data, object->start_address + object->size,
			object->size, temp);
		kfree(temp);
	}

	return ret;
}

static int mxt_write_config_from_pdata(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	u8 **tsp_config = (u8 **)data->pdata->config;
	u8 i;
	int ret = 0;

	if (!tsp_config) {
		tsp_debug_info(true, dev, "No cfg data in pdata\n");
		return 0;
	}

	for (i = 0; tsp_config[i][0] != MXT_RESERVED_T255; i++) {
		ret = mxt_init_write_config(data, tsp_config[i][0],
							tsp_config[i] + 1);
		if (ret)
			return ret;
	}
	return ret;
}

static int mxt_write_config(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_object *object;
	struct mxt_cfg_data *cfg_data;
	u32 current_crc;
	u8 i, val = 0;
	u16 reg, index;
	int ret;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

	if (!fw_info->cfg_raw_data) {
		tsp_debug_info(true, dev, "No cfg data in file\n");
		ret = mxt_write_config_from_pdata(data);
		return ret;
	}

	/* Get config CRC from device */
	ret = mxt_read_config_crc(data, &current_crc);
	if (ret)
		return ret;

	/* Check Version information */
	if (fw_info->fw_ver != data->info.version) {
		tsp_debug_err(true, dev, "Warning: version mismatch! %s\n", __func__);
		return 0;
	}
	if (fw_info->build_ver != data->info.build) {
		printk(KERN_ALERT"fw build info is %d and data info is %d\n",fw_info->build_ver,data->info.build);
		tsp_debug_err(true, dev, "Warning: build num mismatch! %s\n", __func__);
		return 0;
	}

	/* Check config CRC */
	if (current_crc == fw_info->cfg_crc) {
		tsp_debug_info(true, dev, "Skip writing Config:[CRC 0x%06X] as current_crc == fw_info->cfg_crc\n",
			current_crc);
		return 0;
	}

	tsp_debug_info(true, dev, "Writing Config:[CRC 0x%06X!=0x%06X]\n",
		current_crc, fw_info->cfg_crc);

	/* Write config info */
	for (index = 0; index < fw_info->cfg_len;) {

		if (index + sizeof(struct mxt_cfg_data) >= fw_info->cfg_len) {
			tsp_debug_err(true, dev, "index(%d) of cfg_data exceeded total size(%d)!!\n",
				index + sizeof(struct mxt_cfg_data),
				fw_info->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each object */
		cfg_data = (struct mxt_cfg_data *)
					(&fw_info->cfg_raw_data[index]);

		index += sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > fw_info->cfg_len) {
			tsp_debug_err(true, dev, "index(%d) of cfg_data exceeded total size(%d) in T%d object!!\n",
				index, fw_info->cfg_len, cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			tsp_debug_err(true, dev, "T%d is Invalid object type\n",
				cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > object->size) {
			tsp_debug_err(true, dev, "T%d Object length exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= object->instances) {
			tsp_debug_err(true, dev, "T%d Object instances exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}

		tsp_debug_dbg(false, dev, "Writing config for obj %d len %d instance %d (%d/%d)\n",
			cfg_data->type, object->size,
			cfg_data->instance, index, fw_info->cfg_len);

		reg = object->start_address + object->size * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size,
					 cfg_data->register_val);
		if (ret) {
			tsp_debug_err(true, dev, "Write T%d Object failed\n",
				object->type);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < object->size) {
			tsp_debug_err(true, dev, "Warning: zeroing %d byte(s) in T%d\n",
				 object->size - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < object->size; i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}
	tsp_debug_info(true, dev, "Updated configuration\n");

	return ret;
}

#if TSP_PATCH
#include "mxtt_patch.c"
#endif

/* TODO TEMP_ADONIS: need to inspect below functions */
#if TSP_INFORM_CHARGER
static int set_charger_config(struct mxt_data *data)
{
	tsp_debug_info(true, &data->client->dev, "Current state is %s",
		data->charging_mode ? "Charging mode" : "Battery mode");

	if (data->chargin_status != data->charging_mode) {
		if (data->charging_mode) {
#if TSP_PATCH
			tsp_debug_info(true, &data->client->dev, "CHARG-ON Event\n");
			if (data->patch.event_cnt)
				mxt_patch_test_event(data, 1);
#endif
		} else {
#if TSP_PATCH
			tsp_debug_info(true, &data->client->dev, "CHARG-OFF Event\n");
			if (data->patch.event_cnt)
				mxt_patch_test_event(data, 0);
#endif
		}
		data->chargin_status = data->charging_mode;
	}
	return 0;
}

static void inform_charger(struct mxt_callbacks *cb,
	bool en)
{
	struct mxt_data *data = container_of(cb,
			struct mxt_data, callbacks);

	cancel_delayed_work_sync(&data->noti_dwork);
	data->charging_mode = en;
	schedule_delayed_work(&data->noti_dwork, HZ / 5);
}

static void charger_noti_dwork(struct work_struct *work)
{
	struct mxt_data *data =
		container_of(work, struct mxt_data,
		noti_dwork.work);

	if (!data->mxt_enabled) {
		schedule_delayed_work(&data->noti_dwork, HZ / 5);
		return ;
	}

	tsp_debug_info(true, &data->client->dev, "%s mode\n",
		data->charging_mode ? "charging" : "battery");

	set_charger_config(data);
}

static void inform_charger_init(struct mxt_data *data)
{
	INIT_DELAYED_WORK(&data->noti_dwork, charger_noti_dwork);
}
#endif

#ifdef CHARGER_NOTIFIER
static struct extcon_tsp_ta_cable support_cable_list[] = {
	{ .cable_type = EXTCON_USB, },
	{ .cable_type = EXTCON_TA, },
};

static struct mxt_data *tsp_driver = NULL;
static void mxt_set_tsp_info(struct mxt_data *tsp_data)
{
	if (tsp_data != NULL)
		tsp_driver = tsp_data;
	else
		pr_info("%s : tsp info is null\n", __func__);
}
static struct mxt_data * mxt_get_tsp_info(void)
{
	return tsp_driver;
}
static void mxt_charger_notify_work(struct work_struct *work)
{
	struct extcon_tsp_ta_cable *cable =
			container_of(work, struct extcon_tsp_ta_cable, work);
	struct mxt_data *tsp_data = mxt_get_tsp_info();
	//int rc;
	if (!tsp_data){
		pr_err("%s tsp driver is null\n", __func__);
		return;
	}

	tsp_data->charging_mode = cable->cable_state;

	if (!tsp_data->mxt_enabled) {
		pr_err("%s tsp is stopped\n", __func__);
	//	schedule_delayed_work(&tsp_data->noti_dwork, HZ / 5);
		return ;
	}

	tsp_debug_info(true, &tsp_data->client->dev, "%s mode\n",
		tsp_data->charging_mode ? "charging" : "battery");

	set_charger_config(tsp_data);
}

static int mxt_charger_notify(struct notifier_block *nb,
					unsigned long stat, void *ptr)
{
	struct extcon_tsp_ta_cable *cable =
			container_of(nb, struct extcon_tsp_ta_cable, nb);
	pr_info("%s, %ld\n", __func__, stat);
	cable->cable_state = stat;

	schedule_work(&cable->work);

	return NOTIFY_DONE;

}

static int __init mxt_init_charger_notify(void)
{
	struct mxt_data *tsp_data = mxt_get_tsp_info();
	struct extcon_tsp_ta_cable *cable;
	int ret;
	int i;

	if (!tsp_data){
		pr_info("%s tsp driver is null\n", __func__);
		return 0;
	}
	pr_info("%s register extcon notifier for usb and ta\n", __func__);
	for (i = 0; i < ARRAY_SIZE(support_cable_list); i++) {
		cable = &support_cable_list[i];
		INIT_WORK(&cable->work, mxt_charger_notify_work);
		cable->nb.notifier_call = mxt_charger_notify;

		ret = extcon_register_interest(&cable->extcon_nb,
				EXTCON_DEV_NAME,
				extcon_cable_name[cable->cable_type],
				&cable->nb);
		if (ret)
			pr_err("%s: fail to register extcon notifier(%s, %d)\n",
				__func__, extcon_cable_name[cable->cable_type],
				ret);

		cable->edev = cable->extcon_nb.edev;
		if (!cable->edev)
			pr_err("%s: fail to get extcon device\n", __func__);
	}
	return 0;
}
device_initcall_sync(mxt_init_charger_notify);
#endif

static void mxt_report_input_data(struct mxt_data *data)
{
	int i;
	int count = 0;
	int report_count = 0;
//	bool booster_restart = false;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;

		input_mt_slot(data->input_dev, i);
		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			input_mt_report_slot_state(data->input_dev,
					MT_TOOL_FINGER, false);
		} else {
			input_mt_report_slot_state(data->input_dev,
					MT_TOOL_FINGER, true);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					data->fingers[i].x);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					data->fingers[i].y);

#if defined(CONFIG_N2A) && USE_FOR_SUFACE
			/* Change for palm swape motion (20131211 USE_FOR_SUFACE) */
			if (!data->charging_mode)
				data->fingers[i].w += 10;
#endif
#ifdef CONFIG_SEC_FACTORY
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 data->fingers[i].z);
#endif
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					 data->fingers[i].m);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MINOR,
					 data->fingers[i].n);
#if TSP_USE_SHAPETOUCH
			input_report_abs(data->input_dev, ABS_MT_COMPONENT,
					data->fingers[i].component);

#if USE_FOR_SUFACE
			/* Change for palm swape motion (20131211 USE_FOR_SUFACE) */
			if (!data->charging_mode) {
				if(data->sumsize > 20)
					data->sumsize += 30;
			}
#endif
			input_report_abs(data->input_dev, ABS_MT_SUMSIZE,
					data->sumsize);
#endif
#if TSP_USE_PALM_FLAG
			input_report_abs(data->input_dev, ABS_MT_PALM,
					data->palm);
#endif

			if (data->fingers[i].type
				 == MXT_T100_TYPE_HOVERING_FINGER)
				/* hover is reported */
				input_report_key(data->input_dev,
					BTN_TOUCH, 0);
			else
				/* finger or passive stylus are reported */
				input_report_key(data->input_dev,
					BTN_TOUCH, 1);
		}
		report_count++;

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)

#if TSP_USE_SHAPETOUCH

#if TSP_USE_PALM_FLAG
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			tsp_debug_info(true, &data->client->dev, "[P][%d]: T[%d][%d] X[%d],Y[%d],Z[%d],M[%d],N[%d],P[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,data->fingers[i].x,data->fingers[i].y,
				data->fingers[i].z,data->fingers[i].m,data->fingers[i].n,data->palm);
		}
#else
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			tsp_debug_info(true, &data->client->dev, "[P][%d]: T[%d][%d] X[%d],Y[%d],Z[%d],M[%d],N[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,data->fingers[i].x,data->fingers[i].y,
				data->fingers[i].z,data->fingers[i].m,data->fingers[i].n);
		}
#endif /*TSP_USE_PALM_FLAG*/

#else
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			tsp_debug_err(true, &data->client->dev, "[P][%d]: T[%d][%d] X[%d],Y[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,
				data->fingers[i].x, data->fingers[i].y);
		}
#endif

#else
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			tsp_debug_info(true, &data->client->dev, "[P][%d]: T[%d][%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event);
		}
#endif
		else if (data->fingers[i].state == MXT_STATE_RELEASE)
			tsp_debug_err(true, &data->client->dev, "[R][%d]: T[%d][%d] M[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,
				data->fingers[i].mcount);


		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			data->fingers[i].state = MXT_STATE_INACTIVE;
			data->fingers[i].mcount = 0;
		} else {
			data->fingers[i].state = MXT_STATE_MOVE;
			count++;
		}
	}

	if (count == 0)
		input_report_key(data->input_dev, BTN_TOUCH, 0);

	if (report_count > 0) {
#if TSP_USE_ATMELDBG
		if (!data->atmeldbg.stop_sync)
#endif
			input_sync(data->input_dev);
	}
#ifdef COMMON_INPUT_BOOSTER
		if (count){
			data->tsp_booster->dvfs_set(data->tsp_booster,count);
		}
		else{
			data->tsp_booster->dvfs_set(data->tsp_booster, -1);
		}
#endif


	data->finger_mask = 0;
}

static void mxt_release_all_finger(struct mxt_data *data)
{
	int i;
	int count = 0;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;
		data->fingers[i].z = 0;
		data->fingers[i].state = MXT_STATE_RELEASE;
		count++;
	}
	if (count) {
		tsp_debug_err(true, &data->client->dev, "%s\n", __func__);
		mxt_report_input_data(data);
	}
}

#if TSP_HOVER_WORKAROUND
static void mxt_current_calibration(struct mxt_data *data)
{
	tsp_debug_info(true, &data->client->dev, "%s\n", __func__);

	mxt_write_object(data, MXT_SPT_SELFCAPHOVERCTECONFIG_T102, 1, 1);
}
#endif

#if ENABLE_TOUCH_KEY
static void mxt_release_all_keys(struct mxt_data *data)
{
	int i = 0, code = 0;
	u8 back_falg, recent_flag;

	switch (data->pdata->num_touchkey) {
	case 4:
	if (system_rev  >= 3) {
		back_falg = TOUCH_KEY_RECENT_4;
		recent_flag = TOUCH_KEY_D_RECENT_4;
	} else {
		back_falg = TOUCH_KEY_BACK_4;
		recent_flag = TOUCH_KEY_RECENT_4;
	}
		break;
	case 6:
	default:
		back_falg = TOUCH_KEY_BACK;
		recent_flag = TOUCH_KEY_RECENT;
		break;
	}

	if (data->tsp_keystatus != TOUCH_KEY_NULL) {
		if (data->report_dummy_key) {
			for (i = 0 ; i < data->pdata->num_touchkey ; i++) {
				if (data->tsp_keystatus & data->pdata->touchkey[i].value) {
				/* report all touch-key event */
					input_report_key(data->input_dev,
						data->pdata->touchkey[i].keycode, KEY_RELEASE);
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] %s R!\n", data->pdata->touchkey[i].name);
				}
			}

		} else {
			/* recent key check*/
			if (data->tsp_keystatus & recent_flag) {
				if(data->ignore_menu_key)
					tsp_debug_info(true, &data->client->dev,
					"[TSP_KEY] Ignore menu R! by dummy key\n");
				else
					code = KEY_RECENT;
			}

			/* back key check*/
			if (data->tsp_keystatus & back_falg) {
				if (data->ignore_back_key)
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] Ignore back R! by dummy key\n");
				else
					code = KEY_BACK;
			}
		}

		input_report_key(data->input_dev, code, KEY_RELEASE);
		tsp_debug_info(true, &data->client->dev,
			"[TSP_KEY] %d R!\n", code);
		input_sync(data->input_dev);
		data->tsp_keystatus = TOUCH_KEY_NULL;
		data->ignore_menu_key = false;
		data->ignore_back_key = false;
	}
}


/* 0320 */
static void check_rf_radiation(struct mxt_data *data) 
{
    u8 i,j,k;
	u8 ret;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE) {
			continue;
		}

    	if (data->fingers[i].state == MXT_STATE_PRESS) {

    		data->pos_of_ghost[0][i] = 0;  /* 0320 */
    		data->pos_of_ghost[1][i] = 0;  /* 0320 */
    		data->pos_of_ghost[2][i] = 0;  /* 0320 */
    		data->pos_of_ghost[3][i] = 0;  /* 0320 */
    	
    	    if(data->fingers[i].x == 0){   /* 0320 */
    	        data->pos_of_ghost[0][i] = data->fingers[i].y;
    	    }
    	    if(data->fingers[i].x == 4095)  {  /* 0320 */
    	        data->pos_of_ghost[1][i] = data->fingers[i].y;
    	    }
    	    if(data->fingers[i].y == 0)  {  /* 0320 */
    	        data->pos_of_ghost[2][i] = data->fingers[i].x;
    	    }
    	    if(data->fingers[i].y == 4095)  { /* 0320 */
    	        data->pos_of_ghost[3][i] = data->fingers[i].x;
    	    }
    	    
    	}
	}

   
	for (k = 0; k < 4; k++) {   /* 0320 */
		for (i = 0; i < (MXT_MAX_FINGER - 2); i++) {
			if(data->pos_of_ghost[k][i] != 0) {
		        for (j = i+1; j < (MXT_MAX_FINGER - 1); j++) {                
		            if(data->pos_of_ghost[k][i] == data->pos_of_ghost[k][j]) {
		                tsp_debug_err(true, &data->client->dev, "Ghost touch check [%d],[%d] == [%d],[%d] \n"
						, i,data->pos_of_ghost[k][i], j, data->pos_of_ghost[k][i]);
						
						/* send calibration command to the chip */
		                ret = mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
		                        MXT_COMMAND_CALIBRATE, 1);
		                /* all pos clear */
		                memset(&data->pos_of_ghost, 0, sizeof(data->pos_of_ghost));   /* 0320 */
		                return;      
		            }
		        }
	        }
		}
	}
}

static void mxt_treat_T15_object(struct mxt_data *data,
						struct mxt_message *message)
{
	struct input_dev *input = data->input_dev;
	u8 input_status = message->message[MXT_MSG_T15_STATUS] & MXT_MSGB_T15_DETECT;
	u8 input_message = message->message[MXT_MSG_T15_KEYSTATE];
	int i = 0, code_recent = 0, code_back = 0;
	u8 change_state = input_message ^ data->tsp_keystatus;
	u8 key_state = 0, key_state_recent = 0, key_state_back = 0;
	u8 back_falg, recent_flag, back_d_flag, recent_d_flag;
	printk("tkey irq handler\n");
	switch (data->pdata->num_touchkey) {
	case 4:
	if (system_rev  >= 3) {
		back_falg = TOUCH_KEY_RECENT_4;
		recent_flag = TOUCH_KEY_D_RECENT_4;
	}else {
		back_falg = TOUCH_KEY_BACK_4;
		recent_flag = TOUCH_KEY_RECENT_4;
	}
		back_d_flag = TOUCH_KEY_D_BACK_4;
		recent_d_flag = TOUCH_KEY_D_RECENT_4;
		break;
	case 6:
	default:
		back_falg = TOUCH_KEY_BACK;
		recent_flag = TOUCH_KEY_RECENT;
		back_d_flag = TOUCH_KEY_D_BACK;
		recent_d_flag = TOUCH_KEY_D_RECENT;
		break;
	}

	/* single key configuration*/
	if (input_status) { /* press */
		if (data->report_dummy_key) {
			for (i = 0 ; i < data->pdata->num_touchkey ; i++) {
				if (change_state & data->pdata->touchkey[i].value) {
					key_state = input_message & data->pdata->touchkey[i].value;
					input_report_key(input, data->pdata->touchkey[i].keycode,
						key_state != 0 ? KEY_PRESS : KEY_RELEASE);
					input_sync(input);
					tsp_debug_info(true, &data->client->dev, "[TSP_KEY] %s %s\n",
						data->pdata->touchkey[i].name , key_state != 0 ? "P" : "R");
				}
			}
			input_sync(input);
		} else {
			/* recent key check*/
			if (change_state & recent_flag) {
				key_state_recent = input_message & recent_flag;

				if (data->ignore_menu_key)
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] Ignore recent %s by dummy key\n", key_state_recent != 0 ? "P" : "R");
				else {
					code_recent = KEY_RECENT;
				}
			}

			/* back key check*/
			if (change_state & back_falg) {
				key_state_back = input_message & back_falg;
				if (data->ignore_back_key)
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] Ignore back %s by dummy key\n", key_state_back != 0 ? "P" : "R");
				else {
					code_back = KEY_BACK;
				}
			}

			/* dummy recent key check*/
			if (change_state & recent_d_flag) {
				key_state_recent = input_message & recent_d_flag;
				if ((key_state_recent != 0) && !data->ignore_menu_key && !(input_message & recent_flag)) {
					data->ignore_menu_key = true;
					tsp_debug_info(true, &data->client->dev, "[TSP_KEY] ignore_recent_key Enable\n");
				} else if (!key_state_recent && data->ignore_menu_key && !(input_message & recent_flag)) {
					data->ignore_menu_key = false;
					tsp_debug_info(true, &data->client->dev, "[TSP_KEY] ignore_recent_key Disable\n");
				}
			}

			/* dummy back key check*/
			if (change_state & back_d_flag) {
				key_state_back = input_message & back_d_flag;

				if ((key_state_back != 0) && !data->ignore_back_key && !(input_message & back_falg)) {
					data->ignore_back_key = true;
					tsp_debug_info(true, &data->client->dev, "[TSP_KEY] ignore_back_key Enable\n");
				} else if (!key_state_back && data->ignore_back_key && !(input_message & back_falg)) {
					data->ignore_back_key = false;
					tsp_debug_info(true, &data->client->dev, "[TSP_KEY] ignore_back_key Disable\n");
				}
			}

			if (code_recent || code_back) {
				if(code_recent) {
					input_report_key(input, code_recent, !!key_state_recent);
					printk(KERN_ALERT "[TSP_KEY] %d %s\n", code_recent, !!key_state_recent ? "P" : "R");
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] %d %s\n", code_recent, !!key_state_recent ? "P" : "R");
				}
				if(code_back) {
					input_report_key(input, code_back, !!key_state_back);
					printk(KERN_ALERT "[TSP_KEY] %d %s\n", code_back, !!key_state_back ? "P" : "R");
					tsp_debug_info(true, &data->client->dev,
						"[TSP_KEY] %d %s\n", code_back, !!key_state_back ? "P" : "R");
				}
				input_sync(input);
			}
		}
	} else
		mxt_release_all_keys(data);

	data->tsp_keystatus = input_message;

	return;
}
#endif

static void mxt_gt5_change_config(struct mxt_data *data)
{

	u8 ret = 0;

	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							0, 128);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							10, 0);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							11, 0);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							18, 30);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							20, 0x33);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							21, 20);
	ret = mxt_write_object(data, MXT_SPT_SELFCAPCONFIG_T111,
							22, 0);
#if defined(CONFIG_MACH_GT5NOTE10_EUR_OPEN) || defined(CONFIG_MACH_GT5NOTE10_CHN_OPEN) || defined(CONFIG_MACH_GT5NOTE103G_EUR_OPEN)  \
	|| defined(CONFIG_MACH_GT5NOTE10WIFI_EUR_OPEN) || defined(CONFIG_MACH_GT5NOTE10_KOR_OPEN)
	/* 0310 for Back Key */
	ret = mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8,
	                        6, 10);
	ret = mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8,
							7, 30);
	ret = mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8,
							14, 2);
	/* 0311 for Edge detection area */
	ret = mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T100,
							11, 4);	

    /* 0318 */
	ret = mxt_write_object(data, MXT_PROCI_LENSBENDING_T65,
							23, 129);	
	ret = mxt_write_object(data, MXT_PROCI_LENSBENDING_T65,
							24, 38);
	ret = mxt_write_object(data, MXT_PROCI_LENSBENDING_T65,
							46, 129);	
	ret = mxt_write_object(data, MXT_PROCI_LENSBENDING_T65,
							47, 38);

	/* 0320 */
	ret = mxt_write_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTROLLER_T70,
							140, 0);
	ret = mxt_write_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTROLLER_T70,
							150, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							0, 25);

	/* 0324 */
	ret = mxt_write_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTROLLER_T70,
							180, 0);
	ret = mxt_write_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTROLLER_T70,
							190, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							0, 27);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							1, 1);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							2, 100);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							3, 60);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							4, 30);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							5, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							6, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							7, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							8, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
							9, 0);
	ret = mxt_write_object(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80,
						   10, 0);
	ret = mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, /* 0324_2 */
						   3, 100);
	ret = mxt_write_object(data, MXT_PROCI_TOUCHSUPPRESSION_T42, /* 0324_2 */
						   4, 64);
#endif
}

static void mxt_treat_T6_object(struct mxt_data *data,
		struct mxt_message *message)
{
	/* Normal mode */
	if (message->message[0] == 0x00) {
		tsp_debug_info(true, &data->client->dev, "Normal mode\n");
#if TSP_INFORM_CHARGER
		set_charger_config(data);
#endif

#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed.
*/
		if (data->pdata->revision == MXT_REVISION_I
			&& data->cur_cal_status) {
			mxt_current_calibration(data);
			data->cur_cal_status = false;
		}
#endif
	}
	/* I2C checksum error */
	if (message->message[0] & 0x04)
		tsp_debug_err(true, &data->client->dev, "I2C checksum error\n");
	/* Config error */
	if (message->message[0] & 0x08)
		tsp_debug_err(true, &data->client->dev, "Config error\n");
	/* Calibration */
	if (message->message[0] & 0x10) {
		tsp_debug_info(true, &data->client->dev, "Calibration is on going !!\n");
	}
	/* Signal error */
	if (message->message[0] & 0x20)
		tsp_debug_err(true, &data->client->dev, "Signal error\n");
	/* Overflow */
	if (message->message[0] & 0x40)
		tsp_debug_err(true, &data->client->dev, "Overflow detected\n");
	/* Reset */
	if (message->message[0] & 0x80) {
		tsp_debug_info(true, &data->client->dev, "Reset is ongoing\n");

		mxt_gt5_change_config(data);	//0324
	                        
	mxt_release_all_finger(data);
#if ENABLE_TOUCH_KEY
      		mxt_release_all_keys(data);
#endif
#if TSP_INFORM_CHARGER
		data->chargin_status = 0xff;
#endif
#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed.
 * it added just for hover. Current firmware shoud set the acqusition mode
 * with free-run and run current calibration after receive reset command
 * to support hover functionality.
 * it is bug of firmware. and it will be fixed in firmware level.
 */
		if (data->pdata->revision == MXT_REVISION_I) {
			int error = 0;
			u8 value = 0;

			error = mxt_read_object(data,
				MXT_SPT_TOUCHSCREENHOVER_T101, 0, &value);

			if (error) {
				tsp_debug_err(true, &data->client->dev, "Error read hover enable status[%d]\n"
					, error);
			} else {
				if (value)
					data->cur_cal_status = true;
			}
		}
#endif
	}
}

static void mxt_treat_T9_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int id;
	u8 *msg = message->message;

	id = data->reportids[message->reportid].index;

	/* If not a touch event, return */
	if (id >= MXT_MAX_FINGER) {
		tsp_debug_err(true, &data->client->dev, "MAX_FINGER exceeded!\n");
		return;
	}

	if (data->finger_mask & (1U << id))
		mxt_report_input_data(data);

	if (msg[0] & MXT_RELEASE_MSG_MASK) {
		data->fingers[id].z = 0;
		data->fingers[id].w = msg[4];
		data->fingers[id].state = MXT_STATE_RELEASE;
		mxt_report_input_data(data); // 0819
	} else if ((msg[0] & MXT_DETECT_MSG_MASK)
		&& (msg[0] & (MXT_PRESS_MSG_MASK | MXT_MOVE_MSG_MASK))) {
		data->fingers[id].x = (msg[1] << 4) | (msg[3] >> 4);
		data->fingers[id].y = (msg[2] << 4) | (msg[3] & 0xF);
        if (msg[4] != 0) {
#if TSP_INFORM_CHARGER
            if (data->charging_mode)
                data->fingers[id].w = msg[4];
            else
#endif
                data->fingers[id].w = msg[4] + 4;
        } else
            data->fingers[id].w = 1;    //Passive stylus
		data->fingers[id].z = msg[5];
#if TSP_USE_SHAPETOUCH
		data->fingers[id].component = msg[6];
#endif

		if (data->pdata->max_x < 1024)
			data->fingers[id].x = data->fingers[id].x >> 2;
		if (data->pdata->max_y < 1024)
			data->fingers[id].y = data->fingers[id].y >> 2;

		if (msg[0] & MXT_PRESS_MSG_MASK) {
			data->fingers[id].state = MXT_STATE_PRESS;
			data->fingers[id].mcount = 0;
		} else if (msg[0] & MXT_MOVE_MSG_MASK) {
			data->fingers[id].mcount += 1;
		}

#if TSP_USE_PALM_FLAG
		if (msg[0] & MXT_SUPPRESS_MSG_MASK) {
			if (data->palm == 0)
				tsp_debug_info(true, &data->client->dev, "%s : palm detect!\n", __func__);
			data->palm = 1;
		} else
			data->palm = 0;
#endif
	} else if ((msg[0] & MXT_SUPPRESS_MSG_MASK)
		&& (data->fingers[id].state != MXT_STATE_INACTIVE)) {
			if((msg[0] & MXT_DETECT_MSG_MASK) != MXT_DETECT_MSG_MASK){
				data->fingers[id].z = 0;
				data->fingers[id].w = msg[4];
				data->fingers[id].state = MXT_STATE_RELEASE;
#if TSP_USE_PALM_FLAG //0910
				data->palm = 0;
#endif
				mxt_report_input_data(data); // 0904
				tsp_debug_info(true, &data->client->dev,
							"%s: Only Suppress w/o Detect\n", __func__);
			}

	} else {
		/* ignore changed amplitude and vector messsage */
		if (!((msg[0] & MXT_DETECT_MSG_MASK)
				&& (msg[0] & MXT_AMPLITUDE_MSG_MASK
				 || msg[0] & MXT_VECTOR_MSG_MASK)))
			tsp_debug_err(true, &data->client->dev, "Unknown state %#02x %#02x\n",
				msg[0], msg[1]);
	}
	data->finger_mask |= 1U << id;
}

static void mxt_treat_T42_object(struct mxt_data *data,
		struct mxt_message *message)
{
	tsp_debug_info(true, &data->client->dev, "%s\n", __func__);

	if (message->message[0] & 0x01) {
		/* Palm Press */
		tsp_debug_info(true, &data->client->dev, "palm touch detected\n");
	} else {
		/* Palm release */
		tsp_debug_info(true, &data->client->dev, "palm touch released\n");
	}
}

static void mxt_treat_T57_object(struct mxt_data *data,
		struct mxt_message *message)
{
#if TSP_USE_SHAPETOUCH
	data->sumsize = message->message[0] + (message->message[1] << 8);
#endif	/* TSP_USE_SHAPETOUCH */
}

static void mxt_treat_T100_object(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 id, index;
	u8 *msg = message->message;
	u8 touch_type = 0, touch_event = 0, touch_detect = 0;

	index = data->reportids[message->reportid].index;

	/* Treate screen messages */
	if (index < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (index == MXT_T100_SCREEN_MSG_FIRST_RPT_ID)
			/* TODO: Need to be implemeted after fixed protocol
			 * This messages will indicate TCHAREA, ATCHAREA
			 */
			tsp_debug_dbg(true, &data->client->dev, "SCRSTATUS:[%02X] %02X %04X %04X %04X\n",
				 msg[0], msg[1], (msg[3] << 8) | msg[2],
				 (msg[5] << 8) | msg[4],
				 (msg[7] << 8) | msg[6]);
#if TSP_USE_SHAPETOUCH
			data->sumsize = (msg[3] << 8) | msg[2];
#endif	/* TSP_USE_SHAPETOUCH */
		return;
	}

	/* Treate touch status messages */
	id = index - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
	touch_detect = msg[0] >> MXT_T100_DETECT_MSG_MASK;
	touch_type = (msg[0] & 0x70) >> 4;
	touch_event = msg[0] & 0x0F;

	tsp_debug_dbg(true, &data->client->dev, "TCHSTATUS [%d] : DETECT[%d] TYPE[%d] EVENT[%d] %d,%d,%d,%d,%d\n",
		id, touch_detect, touch_type, touch_event,
		msg[1] | (msg[2] << 8),	msg[3] | (msg[4] << 8),
		msg[5], msg[6], msg[7]);

#ifdef USE_T100_MULTI_SLOT
	if (data->finger_mask & (1U << id))
		mxt_report_input_data(data);
#endif
	switch (touch_type)	{
	case MXT_T100_TYPE_FINGER:
	case MXT_T100_TYPE_PASSIVE_STYLUS:
	case MXT_T100_TYPE_HOVERING_FINGER:
	case MXT_T100_TYPE_GLOVE:
	case MXT_T100_TYPE_LARGE:

#if TSP_USE_PALM_FLAG
		if(touch_type==MXT_T100_TYPE_LARGE){
			data->palm =1;
		}
		else{
			data->palm =0;
		}
#endif
		/* There are no touch on the screen */
		if (!touch_detect) {
			if (touch_event == MXT_T100_EVENT_UP
				|| touch_event == MXT_T100_EVENT_SUPPESS) {

				data->fingers[id].z = 0;
				data->fingers[id].n = 0;
				data->fingers[id].m = 0;
				data->fingers[id].state = MXT_STATE_RELEASE;
				data->fingers[id].type = touch_type;
				data->fingers[id].event = touch_event;
#if TSP_USE_PALM_FLAG
				data->palm = 0;
#endif
				mxt_report_input_data(data);
			} else {
				tsp_debug_err(true, &data->client->dev, "Untreated Undetectd touch : type[%d], event[%d]\n",
					touch_type, touch_event);
			}
			break;
		}

		/* There are touch on the screen */
		if (touch_event == MXT_T100_EVENT_DOWN
			|| touch_event == MXT_T100_EVENT_UNSUPPRESS
			|| touch_event == MXT_T100_EVENT_MOVE
			|| touch_event == MXT_T100_EVENT_NONE) {

			data->fingers[id].x = msg[1] | (msg[2] << 8);
			data->fingers[id].y = msg[3] | (msg[4] << 8);

			/* AUXDATA[n]'s order is depended on which values are
			 * enabled or not.
			 */
			data->fingers[id].z = msg[5];
			data->fingers[id].n = min(msg[6],msg[7]);
			data->fingers[id].m = max(msg[6],msg[7]);
#if TSP_USE_SHAPETOUCH
			data->fingers[id].component = msg[5];
#endif

			if (touch_type == MXT_T100_TYPE_HOVERING_FINGER) {
				data->fingers[id].n = 0;
				data->fingers[id].m = 0;
				data->fingers[id].z = 0;
			}

			if (touch_event == MXT_T100_EVENT_DOWN
				|| touch_event == MXT_T100_EVENT_UNSUPPRESS) {
				data->fingers[id].state = MXT_STATE_PRESS;
				data->fingers[id].mcount = 0;
			} else {
				data->fingers[id].state = MXT_STATE_MOVE;
				data->fingers[id].mcount += 1;
			}
			data->fingers[id].type = touch_type;
			data->fingers[id].event = touch_event;

#ifdef USE_T100_MULTI_SLOT
			data->finger_mask |= 1U << id;
#else
			mxt_report_input_data(data);
#endif
			
			check_rf_radiation(data); /* 0309 */
			

		} else {
			tsp_debug_err(true, &data->client->dev, "Untreated Detectd touch : type[%d], event[%d]\n",
				touch_type, touch_event);
		}
		break;
	case MXT_T100_TYPE_ACTIVE_STYLUS:
		break;
	}
}

static irqreturn_t mxt_irq_thread(int irq, void *ptr)
{
	struct mxt_data *data = ptr;
	const struct mxt_platform_data *pdata = data->pdata;
	struct mxt_message message;
	struct device *dev = &data->client->dev;
	u8 reportid = 0, type = 0;
	int retry = 3;

	do {
		if (mxt_read_message(data, &message)) {
			tsp_debug_err(true, dev, "Failed to read message\n");
			if (0 == retry)
				goto end;
			--retry;
		} else {
#if TSP_USE_ATMELDBG
			if (data->atmeldbg.display_log) {
				print_hex_dump(KERN_INFO, "MXT MSG:",
					DUMP_PREFIX_NONE, 16, 1,
					&message,
					sizeof(struct mxt_message), false);
			}
#endif
			reportid = message.reportid;

			if (reportid > data->max_reportid)
				goto end;

			type = data->reportids[reportid].type;

			switch (type) {
			case MXT_RESERVED_T0:
				goto end;
				break;
			case MXT_GEN_COMMANDPROCESSOR_T6:
				mxt_treat_T6_object(data, &message);
				break;
			case MXT_TOUCH_MULTITOUCHSCREEN_T9:
				mxt_treat_T9_object(data, &message);
				break;
#if ENABLE_TOUCH_KEY
			case MXT_TOUCH_KEYARRAY_T15:
				printk("Inside touchkey irq \n");
				mxt_treat_T15_object(data, &message);
				break;
#endif
			case MXT_SPT_SELFTEST_T25:
				tsp_debug_err(true, dev, "Self test fail [0x%x 0x%x 0x%x 0x%x]\n",
					message.message[0], message.message[1],
					message.message[2], message.message[3]);
				break;
			case MXT_PROCI_TOUCHSUPPRESSION_T42:
				mxt_treat_T42_object(data, &message);
				break;
			case MXT_PROCI_EXTRATOUCHSCREENDATA_T57:
				mxt_treat_T57_object(data, &message);
				break;
			case MXT_PROCG_NOISESUPPRESSION_T62:
				break;
			case MXT_TOUCH_MULTITOUCHSCREEN_T100:
				mxt_treat_T100_object(data, &message);
				break;
			default:
				tsp_debug_info(false, dev, "Untreated Object type[%d]\tmessage[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n",
					type, message.message[0],
					message.message[1], message.message[2],
					message.message[3], message.message[4],
					message.message[5], message.message[6]);
				break;
			}
#if TSP_PATCH
			mxt_patch_message(data, &message);
#endif
		}
	} while (!gpio_get_value(pdata->gpio_irq));

	if (data->finger_mask)
		mxt_report_input_data(data);
end:
	return IRQ_HANDLED;
}

static int mxt_get_bootloader_version(struct i2c_client *client, u8 val)
{
	u8 buf[3];

	if (val & MXT_BOOT_EXTENDED_ID) {
		if (i2c_master_recv(client, buf, sizeof(buf)) != sizeof(buf)) {
			tsp_debug_err(true, &client->dev, "%s: i2c recv failed\n",
				 __func__);
			return -EIO;
		}
		tsp_debug_info(true, &client->dev, "Bootloader ID:%d Version:%d",
			 buf[1], buf[2]);
	} else {
		tsp_debug_info(true, &client->dev, "Bootloader ID:%d",
			 val & MXT_BOOT_ID_MASK);
	}
	return 0;
}

static int mxt_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		tsp_debug_err(true, &client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
		if (mxt_get_bootloader_version(client, val))
			return -EIO;
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK)
			goto recheck;
		if (val == MXT_FRAME_CRC_FAIL) {
			tsp_debug_err(true, &client->dev, "Bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		tsp_debug_err(true, &client->dev,
			 "Invalid bootloader mode state 0x%X\n", val);
		return -EINVAL;
	}

	return 0;
}

static int mxt_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2] = {MXT_UNLOCK_CMD_LSB, MXT_UNLOCK_CMD_MSB};

	if (i2c_master_send(client, buf, 2) != 2) {
		tsp_debug_err(true, &client->dev, "%s: i2c send failed\n", __func__);

		return -EIO;
	}

	return 0;
}

static int mxt_probe_bootloader(struct i2c_client *client)
{
	u8 val;

	if (i2c_master_recv(client, &val, 1) != 1) {
		tsp_debug_err(true, &client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	if (val & (~MXT_BOOT_STATUS_MASK)) {
		if (val & MXT_APP_CRC_FAIL)
			tsp_debug_err(true, &client->dev, "Application CRC failure\n");
		else
			tsp_debug_err(true, &client->dev, "Device in bootloader mode\n");
	} else {
		tsp_debug_err(true, &client->dev, "%s: Unknow status\n", __func__);
		return -EIO;
	}
	return 0;
}

static int mxt_fw_write(struct i2c_client *client,
				const u8 *frame_data, unsigned int frame_size)
{
	if (i2c_master_send(client, frame_data, frame_size) != frame_size) {
		tsp_debug_err(true, &client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

int mxt_verify_fw(struct mxt_fw_info *fw_info, const struct firmware *fw)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_fw_image *fw_img;

	if (!fw) {
		tsp_debug_err(true, dev, "could not find firmware file\n");
		return -ENOENT;
	}

	fw_img = (struct mxt_fw_image *)fw->data;

	if (le32_to_cpu(fw_img->magic_code) != MXT_FW_MAGIC) {
		/* In case, firmware file only consist of firmware */
		tsp_debug_dbg(false, dev, "Firmware file only consist of raw firmware\n");
		fw_info->fw_len = fw->size;
		fw_info->fw_raw_data = fw->data;
	} else {
		/*
		 * In case, firmware file consist of header,
		 * configuration, firmware.
		 */
		tsp_debug_info(true, dev, "Firmware file consist of header, configuration, firmware\n");
		fw_info->fw_ver = fw_img->fw_ver;
		fw_info->build_ver = fw_img->build_ver;
		fw_info->hdr_len = le32_to_cpu(fw_img->hdr_len);
		fw_info->cfg_len = le32_to_cpu(fw_img->cfg_len);
		fw_info->fw_len = le32_to_cpu(fw_img->fw_len);
		fw_info->cfg_crc = le32_to_cpu(fw_img->cfg_crc);

		/* Check the firmware file with header */
		if (fw_info->hdr_len != sizeof(struct mxt_fw_image)
			|| fw_info->hdr_len + fw_info->cfg_len
				+ fw_info->fw_len != fw->size) {
#if TSP_PATCH
			struct patch_header* ppheader;
			u32 ppos = fw_info->hdr_len + fw_info->cfg_len + fw_info->fw_len;
			ppheader = (struct patch_header*)(fw->data + ppos);
			if(ppheader->magic == MXT_PATCH_MAGIC){
				tsp_debug_info(true, dev, "Firmware file has patch size: %d\n", ppheader->size);
				if(ppheader->size){
					u8* patch=NULL;
					if(!data->patch.patch){
						kfree(data->patch.patch);
					}
					patch = kzalloc(ppheader->size, GFP_KERNEL);
					memcpy(patch, (u8*)ppheader, ppheader->size);
					data->patch.patch = patch;
				}
			}
			else
#endif
			{
			    tsp_debug_err(true, dev, "Firmware file is invaild !!hdr size[%d] cfg,fw size[%d,%d] filesize[%d]\n",
				    fw_info->hdr_len, fw_info->cfg_len,
				    fw_info->fw_len, fw->size);
			    return -EINVAL;
			}
		}

		if (!fw_info->cfg_len) {
			tsp_debug_err(true, dev, "Firmware file dose not include configuration data\n");
			return -EINVAL;
		}
		if (!fw_info->fw_len) {
			tsp_debug_err(true, dev, "Firmware file dose not include raw firmware data\n");
			return -EINVAL;
		}

		/* Get the address of configuration data */
		fw_info->cfg_raw_data = fw_img->data;

		/* Get the address of firmware data */
		fw_info->fw_raw_data = fw_img->data + fw_info->cfg_len;

#if TSP_SEC_FACTORY
		data->fdata->fw_ver = fw_info->fw_ver;
		data->fdata->build_ver = fw_info->build_ver;
#endif
	}

	return 0;
}

static int mxt_wait_for_chg(struct mxt_data *data, u16 time)
{
	int timeout_counter = 0;
	const struct mxt_platform_data *pdata = data->pdata;

	msleep(time);

	if (gpio_get_value(pdata->gpio_irq)) {
		while (gpio_get_value(pdata->gpio_irq)
			&& timeout_counter++ <= 20) {

			msleep(MXT_RESET_INTEVAL_TIME);
			tsp_debug_err(true, &data->client->dev, "Spend %d th\n",
				(MXT_RESET_INTEVAL_TIME * timeout_counter)
				 + time);
		}
	}

	return 0;
}

static int mxt_command_reset(struct mxt_data *data, u8 value)
{
	int error;

	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_RESET, value);

	error = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (error)
		tsp_debug_err(true, &data->client->dev, "Not respond after reset command[%d]\n",
			value);

	return error;
}

static int mxt_command_backup(struct mxt_data *data, u8 value)
{
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_BACKUPNV, value);

	msleep(MXT_BACKUP_TIME);

	return 0;
}

static int mxt_flash_fw(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct i2c_client *client = data->client_boot;
	struct device *dev = &data->client->dev;
	const u8 *fw_data = fw_info->fw_raw_data;
	size_t fw_size = fw_info->fw_len;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;
	printk(KERN_ALERT "%s\n", __func__);

	if (!fw_data) {
		tsp_debug_err(true, dev, "firmware data is Null\n");
		return -ENOMEM;
	}

	ret = mxt_check_bootloader(client, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		/*may still be unlocked from previous update attempt */
		ret = mxt_check_bootloader(client, MXT_WAITING_FRAME_DATA);
		if (ret)
			goto out;
	} else {
		tsp_debug_info(true, dev, "Unlocking bootloader\n");
		/* Unlock bootloader */
		mxt_unlock_bootloader(client);
	}
	while (pos < fw_size) {
		ret = mxt_check_bootloader(client,
					MXT_WAITING_FRAME_DATA);
		if (ret) {
			tsp_debug_err(true, dev, "Fail updating firmware. wating_frame_data err\n");
			goto out;
		}

		frame_size = ((*(fw_data + pos) << 8) | *(fw_data + pos + 1));

		/*
		* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/

		frame_size += 2;

		/* Write one frame to device */
		mxt_fw_write(client, fw_data + pos, frame_size);

		ret = mxt_check_bootloader(client,
						MXT_FRAME_CRC_PASS);
		if (ret) {
			tsp_debug_err(true, dev, "Fail updating firmware. frame_crc err\n");
			goto out;
		}

		pos += frame_size;

		tsp_debug_dbg(false, dev, "Updated %d bytes / %zd bytes\n",
				pos, fw_size);

		msleep(20);
	}

	ret = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (ret) {
		tsp_debug_err(true, dev, "Not respond after F/W  finish reset\n");
		goto out;
	}

	tsp_debug_info(true, dev, "success updating firmware\n");
out:
	return ret;
}
#if 0
static void mxt_handle_T62_object(struct mxt_data *data)
{
	int ret;
	u8 value;

	ret = mxt_read_object(data, MXT_PROCG_NOISESUPPRESSION_T62, 0, &value);
	if (ret) {
		tsp_debug_err(true, &data->client->dev, "%s: failed to read T62 object.\n",
				__func__);
		return;
	}

	value &= ~(0x02);

	ret = mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T62,
						0, value);
	if (ret)
		tsp_debug_err(true, &data->client->dev, "%s: failed to write T62 object.\n",
				__func__);
	else
		tsp_debug_err(true, &data->client->dev, "%s: Setting T62 report disable.\n",
				__func__);
}
static void mxt_handle_init_data(struct mxt_data *data)
{
/*
 * Caution : This function is called before backup NV. So If you write
 * register vaules directly without config file in this function, it can
 * be a cause of that configuration CRC mismatch or unintended values are
 * stored in Non-volatile memory in IC. So I would recommed do not use
 * this function except for bring up case. Please keep this in your mind.
 */

/* disable T62 report bit. */
	mxt_handle_T62_object(data);

	return;
}
#endif
static int mxt_read_id_info(struct mxt_data *data)
{
	int ret = 0;
	u8 id[MXT_INFOMATION_BLOCK_SIZE];

	/* Read IC information */
	ret = mxt_read_mem(data, 0, MXT_INFOMATION_BLOCK_SIZE, id);
	if (ret) {
		tsp_debug_err(true, &data->client->dev, "Read fail. IC information\n");
		goto out;
	} else {
		tsp_debug_info(true, &data->client->dev,
			"family: 0x%x variant: 0x%x version: 0x%x"
			" build: 0x%x matrix X,Y size:  %d,%d"
			" number of obect: %d\n"
			, id[0], id[1], id[2], id[3], id[4], id[5], id[6]);
		data->info.family_id = id[0];
		data->info.variant_id = id[1];
		data->info.version = id[2];
		data->info.build = id[3];
		data->info.matrix_xsize = id[4];
		data->info.matrix_ysize = id[5];
		data->info.object_num = id[6];
	}

out:
	return ret;
}

static int mxt_get_object_table(struct mxt_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[MXT_OBJECT_TABLE_ELEMENT_SIZE];

	for (i = 0; i < data->info.object_num; i++) {
		struct mxt_object *object = data->objects + i;

		reg = MXT_OBJECT_TABLE_START_ADDRESS +
				MXT_OBJECT_TABLE_ELEMENT_SIZE * i;
		error = mxt_read_mem(data, reg,
				MXT_OBJECT_TABLE_ELEMENT_SIZE, buf);
		if (error)
			return error;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		/* the real size of object is buf[3]+1 */
		object->size = buf[3] + 1;
		/* the real instances of object is buf[4]+1 */
		object->instances = buf[4] + 1;
		object->num_report_ids = buf[5];

		tsp_debug_dbg(false, &data->client->dev,
			"Object:T%d\t\t\t Address:0x%x\tSize:%d\tInstance:%d\tReport Id's:%d\n",
			object->type, object->start_address, object->size,
			object->instances, object->num_report_ids);

		if (object->num_report_ids) {
			reportid += object->num_report_ids * object->instances;
			object->max_reportid = reportid;
		}
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;
	tsp_debug_dbg(false, &data->client->dev, "maXTouch: %d report ID\n",
			data->max_reportid);

	return 0;
}

static void mxt_make_reportid_table(struct mxt_data *data)
{
	struct mxt_object *objects = data->objects;
	struct mxt_reportid *reportids = data->reportids;
	int i, j;
	int id = 0;

	for (i = 0; i < data->info.object_num; i++) {
		for (j = 0; j < objects[i].num_report_ids *
				objects[i].instances; j++) {
			id++;

			reportids[id].type = objects[i].type;
			reportids[id].index = j;

			tsp_debug_dbg(false, &data->client->dev, "Report_id[%d]:\tT%d\tIndex[%d]\n",
				id, reportids[id].type, reportids[id].index);
		}
	}
}

static int mxt_initialize(struct mxt_data *data)
{
	struct i2c_client *client = data->client;

	u32 read_info_crc, calc_info_crc;
	int ret;
	printk(KERN_ALERT "%s\n", __func__);

	ret = mxt_read_id_info(data);
	if (ret)
		return ret;

	data->objects = kcalloc(data->info.object_num,
				sizeof(struct mxt_object),
				GFP_KERNEL);
	if (!data->objects) {
		tsp_debug_err(true, &client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Get object table infomation */
	ret = mxt_get_object_table(data);
	if (ret)
		goto out;

	data->reportids = kcalloc(data->max_reportid + 1,
			sizeof(struct mxt_reportid),
			GFP_KERNEL);
	if (!data->reportids) {
		tsp_debug_err(true, &client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Make report id table */
	mxt_make_reportid_table(data);

	/* Verify the info CRC */
	ret = mxt_read_info_crc(data, &read_info_crc);
	if (ret)
		goto out;

	ret = mxt_calculate_infoblock_crc(data, &calc_info_crc);
	if (ret)
		goto out;

	if (read_info_crc != calc_info_crc) {
		tsp_debug_err(true, &data->client->dev, "Infomation CRC error :[CRC 0x%06X!=0x%06X]\n",
				read_info_crc, calc_info_crc);
		ret = -EFAULT;
		goto out;
	}
	return 0;

out:
	return ret;
}

static int mxt_check_config_crc(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	u32 current_crc;
	int ret = CONFIG_IS_MISMATCH;

	/* allocation of the object table memory  */
	if(data->objects ==  NULL) {
		data->objects = kcalloc(data->info.object_num,
							sizeof(struct mxt_object),
							GFP_KERNEL);
		if (!data->objects) {
			dev_err(dev, "%s Failed to allocate memory\n",
				__func__);
			ret = -ENOMEM;
			goto out;
		}
	}

	/* Get object table information*/
	ret = mxt_get_object_table(data);
	if (ret)
		goto err_free_mem;

	/* Get config CRC from device */
	ret = mxt_read_config_crc(data, &current_crc);
	if (ret)
		goto err_free_mem;

	/* Check config CRC */
	if (current_crc == fw_info->cfg_crc) {
		dev_err(dev, "Config is Same:[CRC 0x%06X]\n",
			current_crc);

		ret = CONFIG_IS_SAME;
	}else{
		dev_err(dev, "Config is Mismatch:[CRC 0x%06X != 0x%06X]\n",
			current_crc, fw_info->cfg_crc);

		ret = CONFIG_IS_MISMATCH;
	}

	goto out;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;

out:
	return ret;
}

static int  mxt_rest_initialize(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int ret = 0;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

        if(mxt_check_config_crc(fw_info) == CONFIG_IS_MISMATCH){
		/* Restore memory and stop event handing */
		ret = mxt_command_backup(data, MXT_DISALEEVT_VALUE);
		if (ret) {
			tsp_debug_err(true, dev, "Failed Restore NV and stop event\n");
			goto out;
		}

		/* Write config */
		ret = mxt_write_config(fw_info);
		if (ret) {
			tsp_debug_err(true, dev, "Failed to write config from file\n");
			goto out;
		}
#if 0
	/* Handle data for init */
	mxt_handle_init_data(data);
#endif
		/* Backup to memory */
		ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
		if (ret) {
			tsp_debug_err(true, dev, "Failed backup NV data\n");
			goto out;
		}

		/* Soft reset */
		ret = mxt_command_reset(data, MXT_RESET_VALUE);
		if (ret) {
			dev_err(dev, "Failed Reset IC\n");
			goto out;
		}
        }
#if TSP_PATCH
	if(data->patch.patch)
		ret = mxt_patch_init(data, data->patch.patch);
#endif

out:
	return ret;
}

static int mxt_power_on(struct mxt_data *data)
{
/*
 * If do not turn off the power during suspend, you can use deep sleep
 * or disable scan to use T7, T9 Object. But to turn on/off the power
 * is better.
 */
	int error = 0;

	if (data->mxt_enabled)
		return 0;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

	if (!data->pdata->power_ctrl) {
		dev_warn(&data->client->dev, "Power on function is not defined\n");
		error = -EINVAL;
		goto out;
	}

	error = data->pdata->power_ctrl(data, true);
	if (error) {
		tsp_debug_err(true, &data->client->dev, "Failed to power on\n");
		goto out;
	}

/*	error = mxt_wait_for_chg(data, MXT_HW_RESET_TIME);
	if (error)
		tsp_debug_err(true, &data->client->dev, "Not respond after H/W reset\n");
*/
	data->mxt_enabled = true;

out:
	return error;
}

static int mxt_power_off(struct mxt_data *data)
{
	int error = 0;

	if (!data->mxt_enabled)
		return 0;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

	if (!data->pdata->power_ctrl) {
		dev_warn(&data->client->dev, "Power off function is not defined\n");
		error = -EINVAL;
		goto out;
	}

	error = data->pdata->power_ctrl(data, false);
	if (error) {
		tsp_debug_err(true, &data->client->dev, "Failed to power off\n");
		goto out;
	}

	data->mxt_enabled = false;

out:
	return error;
}

/* Need to be called by function that is blocked with mutex */
static int mxt_start(struct mxt_data *data)
{
	int error = 0;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

	if (data->mxt_enabled) {
		tsp_debug_err(true, &data->client->dev,
			"%s. but touch already on\n", __func__);
		return error;
	}

	error = mxt_power_on(data);
	if (error)
		tsp_debug_err(true, &data->client->dev, "Fail to start touch\n");
	else
		enable_irq(data->client->irq);

	return error;
}

/* Need to be called by function that is blocked with mutex */
static int mxt_stop(struct mxt_data *data)
{
	int error = 0;
	printk(KERN_ALERT "[TSP] %s\n", __func__);

	if (!data->mxt_enabled) {
		tsp_debug_err(true, &data->client->dev,
			"%s. but touch already off\n", __func__);
		return error;
	}
	disable_irq(data->client->irq);

	error = mxt_power_off(data);
	if (error) {
		tsp_debug_err(true, &data->client->dev, "Fail to stop touch\n");
		goto err_power_off;
	}
	mxt_release_all_finger(data);

#if ENABLE_TOUCH_KEY
	mxt_release_all_keys(data);
#endif
#ifdef COMMON_INPUT_BOOSTER
	if(data->tsp_booster)
		data->tsp_booster->dvfs_set(data->tsp_booster, -1);
#endif
	return 0;

err_power_off:
	enable_irq(data->client->irq);
	return error;
}
static int mxt_pinctrl_configure(struct mxt_data *info, bool active)
{
	struct pinctrl_state *set_state_i2c;
	int retval;

	dev_err(&info->client->dev, "%s: %s\n", __func__, active ? "ACTIVE" : "SUSPEND");

	if (active) {
	set_state_i2c =
			pinctrl_lookup_state(info->pinctrl,
						"tsp_gpio_active");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) active state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}
	} else {
		set_state_i2c =
			pinctrl_lookup_state(info->pinctrl,
						"tsp_gpio_suspend");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) sleep state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}
	}

	retval = pinctrl_select_state(info->pinctrl, set_state_i2c);
	if (retval) {
		dev_err(&info->client->dev, "%s: cannot set pinctrl(i2c) %s state\n",
				__func__, active ? "active" : "suspend");
		return retval;
	}
printk("TSP IRQ value is %d\n",gpio_get_value(info->pdata->gpio_irq));
	return 0;
}

static int mxt_input_open(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);
	int ret;
	printk(KERN_ALERT "[TSP] %s\n", __func__);
	mxt_pinctrl_configure(data,1);

	ret = mxt_start(data);
	if (ret) {
		tsp_debug_info(true, &data->client->dev, "%s fail\n", __func__);
		return ret;
	}
	
	tsp_debug_info(true, &data->client->dev, "%s\n", __func__);

	return 0;
}

static void mxt_input_close(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);
	printk(KERN_ALERT "[TSP] %s\n", __func__);
	mxt_pinctrl_configure(data,0);

	mxt_stop(data);
	

	tsp_debug_info(true, &data->client->dev, "%s\n", __func__);
}

#if FOR_BRINGUP
static int mxt_make_highchg(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	int count = data->max_reportid * 2;
	int error;

	/* Read dummy message to make high CHG pin */
	do {
		error = mxt_read_message(data, &message);
		if (error)
			return error;
	} while (message.reportid != 0xff && --count);

	if (!count) {
		tsp_debug_err(true, dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}

	return 0;
}
#endif

static int mxt_touch_finish_init(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	//int error;

	printk(KERN_ALERT "%s\n", __func__);

	/*
	* to prevent unnecessary report of touch event
	* it will be enabled in open function
	*/
	//mxt_stop(data);
#if FOR_BRINGUP
	error = mxt_make_highchg(data);
	if (error) {
		tsp_debug_err(true, &client->dev, "Failed to clear CHG pin\n");
		goto err_req_irq;
	}
#endif
	tsp_debug_info(true, &client->dev,  "Mxt touch controller initialized\n");

	return 0;

//err_req_irq:
	//return error;
}

static int mxt_touch_rest_init(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int error;
	printk(KERN_ALERT "%s\n", __func__);

	error = mxt_initialize(data);
	if (error) {
		tsp_debug_err(true, dev, "Failed to initialize\n");
		goto err_free_mem;
	}

	error = mxt_rest_initialize(fw_info);
	if (error) {
		tsp_debug_err(true, dev, "Failed to rest initialize\n");
		goto err_free_mem;
	}

	error = mxt_touch_finish_init(data);
	if (error)
		goto err_free_mem;

	return 0;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;
	kfree(data->reportids);
	data->reportids = NULL;
	return error;
}

static int mxt_enter_bootloader(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int error;

	data->objects = kcalloc(data->info.object_num,
				     sizeof(struct mxt_object),
				     GFP_KERNEL);
	if (!data->objects) {
		tsp_debug_err(true, dev, "%s Failed to allocate memory\n",
			__func__);
		error = -ENOMEM;
		goto out;
	}

	/* Get object table information*/
	error = mxt_get_object_table(data);
	if (error)
		goto err_free_mem;

	/* Change to the bootloader mode */
	error = mxt_command_reset(data, MXT_BOOT_VALUE);
	if (error)
		goto err_free_mem;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;

out:
	return error;
}

static int mxt_flash_fw_on_probe(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int error;
	printk(KERN_ALERT "%s\n", __func__);

	/*Check memory allocation of object table */
	if(data->objects != NULL)
	{
		kfree(data->objects);
		data->objects = NULL;
	}

	error = mxt_read_id_info(data);

	if (error) {
		/* need to check IC is in boot mode */
		error = mxt_probe_bootloader(data->client_boot);
		if (error) {
			tsp_debug_err(true, dev, "Failed to verify bootloader's status\n");
			goto out;
		}

		tsp_debug_info(true, dev, "Updating firmware from boot-mode\n");
		goto load_fw;
	}

#if 0
		/* compare the version to verify necessity of firmware updating */
		if (data->info.version == fw_info->fw_ver
				&& data->info.build == fw_info->build_ver) {
			dev_dbg(dev, "Firmware version is same with in IC\n");
			goto out;
		}
#else
		if(mxt_check_config_crc(fw_info)== CONFIG_IS_SAME){
			dev_dbg(dev, "Firmware Config CRC is same with in IC\n");
			goto out;
		}

		if(data->objects != NULL) {
		    kfree(data->objects);
	        data->objects = NULL;
		}
#endif


	tsp_debug_info(true, dev, "Updating firmware from app-mode : IC:0x%x,0x%x =! FW:0x%x,0x%x\n",
			data->info.version, data->info.build,
			fw_info->fw_ver, fw_info->build_ver);

	error = mxt_enter_bootloader(data);
	if (error) {
		printk(KERN_ALERT "Failed updating firmware\n");
		tsp_debug_err(true, dev, "Failed updating firmware\n");
		goto out;
	}

load_fw:
	printk(KERN_ALERT "TSP Loading Firmware \n");
	error = mxt_flash_fw(fw_info);
	if (error)
		tsp_debug_err(true, dev, "Failed updating firmware\n");
	else
		tsp_debug_info(true, dev, "succeeded updating firmware\n");
out:
	return error;
}

static void mxt_request_firmware_work(const struct firmware *fw,
		void *context)
{
	struct mxt_data *data = context;
	struct mxt_fw_info fw_info;
	int error;
	printk(KERN_ALERT "%s\n", __func__);

	memset(&fw_info, 0, sizeof(struct mxt_fw_info));
	fw_info.data = data;

	error = mxt_verify_fw(&fw_info, fw);
	if (error) //SKIP firmware update during bringup..NEed to change once get new firmware
		goto ts_rest_init;

	/* Skip update on boot up if firmware file does not have a header */
	if (!fw_info.hdr_len)
		goto ts_rest_init;

	error = mxt_flash_fw_on_probe(&fw_info);
	if (error)
		goto out;

ts_rest_init:
	error = mxt_touch_rest_init(&fw_info);

out:
	release_firmware(fw);

}

static int  mxt_touch_init(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	const char *firmware_name =
		 data->pdata->firmware_name ?: MXT_DEFAULT_FIRMWARE_NAME;
	int ret = 0;
	const struct firmware *fw = NULL;
	char fw_path[MXT_MAX_FW_PATH];
	printk(KERN_ALERT "%s\n", __func__);

#if TSP_INFORM_CHARGER
	/* Register callbacks */
	/* To inform tsp , charger connection status*/
	data->callbacks.inform_charger = inform_charger;
	if (data->pdata->register_cb) {
		data->pdata->register_cb(&data->callbacks);
		inform_charger_init(data);
	}
#endif

	memset(&fw_path, 0, MXT_MAX_FW_PATH);

	snprintf(fw_path, MXT_MAX_FW_PATH, "%s%s",
		MXT_FIRMWARE_INKERNEL_PATH, firmware_name);

	tsp_debug_err(true, &client->dev, "%s\n", fw_path);

	ret = request_firmware(&fw, fw_path, &client->dev);
	if (ret) {
		tsp_debug_err(true, &client->dev,
			"error requesting built-in firmware\n");
		goto out;
	}

	mxt_request_firmware_work(fw, data);
out:
	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#define mxt_suspend	NULL
#define mxt_resume	NULL

static void mxt_early_suspend(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
#if TSP_INFORM_CHARGER
	cancel_delayed_work_sync(&data->noti_dwork);
#endif

	mutex_lock(&data->input_dev->mutex);

	mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
}

static void mxt_late_resume(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
	mutex_lock(&data->input_dev->mutex);

	mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
}
#else
static int mxt_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&data->input_dev->mutex);

	if (input_dev->users)
		mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);
	struct input_dev *input_dev = data->input_dev;

	mutex_lock(&data->input_dev->mutex);

	if (input_dev->users)
		mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}
#endif

static void mxt_shutdown(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

	tsp_debug_info(true, &client->dev, "%s called!\n", __func__);

	mxt_stop(data);
#if ENABLE_TOUCH_KEY
//	data->pdata->led_power_ctrl(data, 0);
#endif
}

/* Added for samsung dependent codes such as Factory test,
 * Touch booster, Related debug sysfs.
 */
#if 1//FOR_BRINGUP
#include "mxtt_sec.c"
#endif


#if ENABLE_TOUCH_KEY





struct mxt_touchkey mxt_touchkey_data[] = {
	{
		.value = TOUCH_KEY_D_RECENT_4,
		.keycode = KEY_DUMMY_MENU,
		.name = "d_menu",
		.xnode = 2,
		.ynode = 43,
		.deltaobj = 2,
	},
	{
		.value = TOUCH_KEY_RECENT_4,
		.keycode = KEY_RECENT,
		.name = "recent",
		.xnode = 4,
		.ynode = 43,
		.deltaobj = 0,
	},
	{
		.value = TOUCH_KEY_BACK_4,
		.keycode = KEY_BACK,
		.name = "back",
		.xnode = 5,
		.ynode = 43,
		.deltaobj = 1,
	},
	{
		.value = TOUCH_KEY_D_BACK_4,
		.keycode = KEY_DUMMY_BACK,
		.name = "d_back",
		.xnode = 3,
		.ynode = 43,
		.deltaobj = 3,
	},
};
#endif




#ifdef CONFIG_OF

static int mxt_request_gpio(struct mxt_data *data)
{
	int ret;
	printk(KERN_ALERT "%s\n", __func__);

	ret = gpio_request(data->pdata->gpio_reset, "mxts,rst");
	if (ret) {
		pr_err("%s: unable to request tsp_rst [%d]\n",
				__func__, data->pdata->gpio_reset);
		return ret;
	}
	/*gpio_tlmm_config(GPIO_CFG(data->pdata->gpio_reset, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);*/

	ret = gpio_direction_output(data->pdata->gpio_reset, 0);
	if (ret) {
		   dev_err(&data->client->dev,"[TSP]%s: unable to set_direction for data->pdata->gpio_reset [%d]\n",
							__func__,data->pdata->gpio_reset);
		   return ret;
	}
	ret = gpio_request(data->pdata->tsp_en1, "mxts,external_ldo");
	if (ret) {
		pr_err("%s: unable to request tsppwer_en [%d]\n",
				__func__,data->pdata->tsp_en1);
		return ret;
	}
/*	gpio_tlmm_config(GPIO_CFG(data->pdata->tsp_en1, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);*/

        if ( data->pdata->tsp_en2 > 0) {
		ret = gpio_request(data->pdata->tsp_en2, "mxts,external_ldo2");
		if (ret) {
			pr_err("%s: unable to request tsppwer_en [%d]\n",
				__func__, data->pdata->tsp_en2);
			return ret;
		}
	}
/*	gpio_tlmm_config(GPIO_CFG(data->pdata->tsp_en2, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 1);*/
	
	ret = gpio_request(data->pdata->gpio_irq, "mxts,irq-gpio");
	if (ret) {
		pr_err("%s: unable to request tsp_int [%d]\n",
				__func__, data->pdata->gpio_irq);
		return ret;
	}
        if ( data->pdata->tsp_vendor1 >  0) {
		ret = gpio_request(data->pdata->tsp_vendor1, "mxts,tsp_vendor1");
		if (ret) {
			pr_err("%s: unable to request tsp_vendor1 [%d]\n",
				__func__, data->pdata->tsp_vendor1);
			return ret;
		}
	}
        if ( data->pdata->tsp_vendor2 > 0) {
	    	ret = gpio_request(data->pdata->tsp_vendor2, "mxts,tsp_vendor2");
		if (ret) {
			pr_err("%s: unable to request tsp_vendor2 [%d]\n",
				__func__, data->pdata->tsp_vendor2);
			return ret;
		}
        }
/*	ret = gpio_tlmm_config(GPIO_CFG(data->pdata->gpio_irq,
				0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
					GPIO_CFG_2MA), GPIO_CFG_ENABLE);*/

	return ret;
}

static int mxt_power_ctrl(void *ddata, bool on)
{
	struct mxt_data *info = (struct mxt_data *)ddata;
	int rc;
	printk(KERN_ALERT "[TSP] %s %s\n",
				__func__, on ? "on" : "off");
#if 0
	if (info->vcc_supply) {
		if(on){
			printk("mxt_power_ctrl is %d\n",on);
			rc = regulator_enable(info->vcc_supply);
			if (rc) {
				dev_err(&info->client->dev,
				"Regulator 8916_l5 enable failed rc=%d\n",
				rc);
				return 0;
			}
			
		}else{
			rc = regulator_disable(info->vcc_supply);
			if (rc) {
				dev_err(&info->client->dev,
				"Regulator 8916_l5 enable failed rc=%d\n",
				rc);
				return 0;
			}
		}
	}
#endif

	rc = gpio_direction_output(info->pdata->tsp_en1, on);
		if (rc) {
			dev_err(&info->client->dev,"[TSP]%s: unable to set_direction for info->pdata->external_ldo [%d]\n",
					__func__, info->pdata->tsp_en1);
		return rc;
	}
	if (on)
		msleep(1);
	else
		mdelay(1);

        if ( info->pdata->tsp_en2 > 0) {
		rc = gpio_direction_output(info->pdata->tsp_en2, on);
			if (rc) {
				dev_err(&info->client->dev,"[TSP]%s: unable to set_direction for info->pdata->external_ldo2  [%d]\n",
					__func__, info->pdata->tsp_en2);
			return rc;
		}
	}
	if(on)
		msleep(30);
	else
		mdelay(1);

	rc = gpio_direction_output(info->pdata->gpio_reset, on);
				if (rc) {
					dev_err(&info->client->dev,"[TSP]%s: unable to set_direction for info->pdata->external_ldo2 [%d]\n",
							__func__,info->pdata->gpio_reset);
				return rc;
			}
	if(on)
		msleep(100);

	return 0;
}


static int mxt_parse_dt(struct device *dev, struct mxt_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	const char *model = NULL;
	u32 coords[4];
	int ret;

	/* reset, irq gpio info */
	pdata->tsp_en1= of_get_named_gpio(np, "mxts,external_ldo", 0);
	pdata->tsp_en2= of_get_named_gpio(np, "mxts,external_ldo2", 0);
	pdata->gpio_irq= of_get_named_gpio(np, "mxts,irq-gpio", 0);
	pdata->gpio_reset = of_get_named_gpio(np, "mxts,rst", 0);
	pdata->tsp_vendor1 = of_get_named_gpio(np, "mxts,tsp_vendor1", 0);
	pdata->tsp_vendor2 = of_get_named_gpio(np, "mxts,tsp_vendor2", 0);
	pr_err("%s tsp_en= %d, tsp_en1= %d, tsp_int= %d, rst= %d, tsp_vendor1 = %d, tsp_vendor2 = %d\n",
			__func__, pdata->tsp_en1, pdata->tsp_en2, pdata->gpio_irq, pdata->gpio_reset,pdata->tsp_vendor1,pdata->tsp_vendor2);

	ret = of_property_read_u32_array(np, "mxts,tsp_coord", coords, 4);
	of_property_read_string(np, "mxts,pname", &model);
	if (ret && (ret != -EINVAL)) {
		printk(KERN_ERR "%s: Unable to read mxts,tsp_coord\n", __func__);
		return ret;
	}

	pdata->model_name = model;
	pdata->num_xnode = coords[0];
	pdata->num_ynode = coords[1];
	pdata->max_x = coords[2];
	pdata->max_y = coords[3];
	pdata->boot_address = MXT_BOOT_ADDRESS;
        if (system_rev	>= 3) {
		pdata->firmware_name = MXT_FIRMWARE_NAME_NEW_REVISION;
	}
	else {
		pdata->firmware_name = MXT_FIRMWARE_NAME;
	}

	pdata->num_touchkey = ARRAY_SIZE(mxt_touchkey_data);
	pdata->touchkey = mxt_touchkey_data;
        if (system_rev	>= 3) {
		pdata->touchkey[1].value = TOUCH_KEY_D_RECENT_4;
		pdata->touchkey[2].value = TOUCH_KEY_RECENT_4;
        }

	pr_err("%s num_xnode= %d, num_ynode= %d, max_x= %d, max_y= %d, boot_addr= 0x%02x model name is %s\n",
			__func__, pdata->num_xnode, pdata->num_ynode,
			pdata->max_x, pdata->max_y, pdata->boot_address,pdata->model_name);

	return 0;
}
#endif



static int  mxt_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct mxt_platform_data *pdata = client->dev.platform_data;
	struct mxt_data *data;
	struct input_dev *input_dev;
	u16 boot_address;
	int error = 0;
	int ret = 0;
#if ENABLE_TOUCH_KEY
	int  i = 0;
#endif
	tsp_debug_err(false, &client->dev, "%s\n", __func__);
	printk(KERN_ALERT "%s\n", __func__);
	printk("%s: [TSP] system_rev = %d\n", __func__, system_rev);
#ifdef CONFIG_OF
	/* parse dt */
	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct mxt_platform_data), GFP_KERNEL);

		if (!pdata) {
			tsp_debug_err(true, &client->dev, "Failed to allocate platform data\n");
			return -ENOMEM;
		}

	//	client->dev.platform_data = pdata;
		error = mxt_parse_dt(&client->dev,pdata);
		if (error) {
			tsp_debug_err(true, &client->dev, "Failed to parse dt\n");
			devm_kfree(&client->dev, pdata);
			return -EINVAL;
		}
	} else {
		pdata = client->dev.platform_data;
	}
#else
	pdata = client->dev.platform_data;
#endif
	if (!pdata) {
		tsp_debug_err(true, &client->dev, "Platform data is not proper\n");
		return -EINVAL;
	}

	data = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	if (!data) {
		tsp_debug_err(true, &client->dev, "Failed to allocate memory\n");
		error = -ENOMEM;
		goto err_allocate_data;
	}

	pdata->power_ctrl = mxt_power_ctrl;
	input_dev = input_allocate_device();
	if (!input_dev) {
		error = -ENOMEM;
		tsp_debug_err(true, &client->dev, "Input device allocation failed\n");
		goto err_allocate_input_device;
	}

	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;


	data->client = client;
	data->input_dev = input_dev;
	data->pdata = pdata;
	printk("Inside probe tsp int gpio is %d,tsp en gpio is %d, tsp en2 gpio is %d tsp rst gpio is %d\n",data->pdata->gpio_irq,data->pdata->tsp_en1,
		data->pdata->tsp_en2,data->pdata->gpio_reset);
	mxt_request_gpio(data);

	data->vcc_supply = regulator_get(&data->client->dev,
				"8916_l5");
			if (IS_ERR(data->vcc_supply)) {
				error = PTR_ERR(data->vcc_supply);
				dev_err(&data->client->dev,
					"Regulator get failed ret=%d\n", ret);
				goto err_regulator;
	} 
	else
		regulator_set_voltage(data->vcc_supply,1800000,1800000);

		/* Get pinctrl if target uses pinctrl */

		data->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(data->pinctrl)) {
		if (PTR_ERR(data->pinctrl) == -EPROBE_DEFER)
			
		printk("TSP doesnt use pinctrl configuration\n");
		//tsp_debug_info(true, &ddata->client->dev,"%s: Target does not use pinctrl\n", __func__);
		data->pinctrl = NULL;
	}
	printk("TSP uses pinctrl configuration \n");
	if (data->pinctrl) {
		ret = mxt_pinctrl_configure(data, true);
		if (ret)
			printk("TSP error in pinctrl configuration\n");
			//tsp_debug_info(true, &ddata->client->dev,"%s: cannot set pinctrl state\n", __func__);
	}

	input_dev->open = mxt_input_open;
	input_dev->close = mxt_input_close;
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, input_dev->keybit);
#ifdef CHARGER_NOTIFIER
	mxt_set_tsp_info(data);
#endif

#if ENABLE_TOUCH_KEY
	for (i = 0 ; i < data->pdata->num_touchkey ; i++){
		set_bit(data->pdata->touchkey[i].keycode, input_dev->keybit);
			//input_set_capability(input_dev, EV_KEY,
				//data->pdata->touchkey[i].keycode);
		}
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);

	data->report_dummy_key = false; /*Disable dummy key!*/
#endif

	input_mt_init_slots(input_dev, MXT_MAX_FINGER, INPUT_MT_DIRECT);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				0, pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				0, pdata->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
				0, MXT_AREA_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
				0, MXT_AREA_MIN, 0, 0);
#ifdef CONFIG_SEC_FACTORY
	input_set_abs_params(input_dev, ABS_MT_PRESSURE,
				0, MXT_AMPLITUDE_MAX, 0, 0);
#endif
#if TSP_USE_SHAPETOUCH
	input_set_abs_params(input_dev, ABS_MT_COMPONENT,
				0, MXT_COMPONENT_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_SUMSIZE,
				0, MXT_SUMSIZE_MAX, 0, 0);
#endif
#if TSP_USE_PALM_FLAG
	input_set_abs_params(input_dev, ABS_MT_PALM,
				0, MXT_PALM_MAX, 0, 0);
#endif

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);

	if (data->pdata->boot_address) {
		boot_address = data->pdata->boot_address;
	} else {
		if (client->addr == MXT_APP_LOW)
			boot_address = MXT_BOOT_LOW;
		else
			boot_address = MXT_BOOT_HIGH;
	}
	data->client_boot = i2c_new_dummy(client->adapter, boot_address);
	if (!data->client_boot) {
		tsp_debug_err(true, &client->dev, "Fail to register sub client[0x%x]\n",
			 boot_address);
		error = -ENODEV;
		goto err_create_sub_client;
	}

	/* regist input device */
	error = input_register_device(input_dev);
	if (error)
		goto err_register_input_device;

	error = mxt_sysfs_init(client);
	if (error < 0) {
		tsp_debug_err(true, &client->dev, "Failed to create sysfs\n");
		goto err_sysfs_init;
	}



	error = mxt_power_on(data);
	if (error) {
		tsp_debug_err(true, &client->dev, "Failed to power_on\n");
		goto err_power_on;
	}

	error = mxt_touch_init(data);
	if (error) {
		tsp_debug_err(true, &client->dev, "Failed to init driver\n");
		goto err_touch_init;
	}

	mxt_gt5_change_config(data);	//0324

	client->irq = gpio_to_irq(data->pdata->gpio_irq);
	dev_info(&data->client->dev, "%s: tsp int gpio is %d : gpio_to_irq : %d\n",
			__func__, data->pdata->gpio_irq,client->irq);
	
	error = request_threaded_irq(client->irq, NULL, mxt_irq_thread,
		IRQF_TRIGGER_LOW | IRQF_ONESHOT, client->dev.driver->name, data);

	if (error) {
		tsp_debug_err(true, &client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}
	else
		printk("TSP interrupt registered successfully\n");

#ifdef COMMON_INPUT_BOOSTER
	printk("TSP COMMON_INPUT_BOOSTER init\n");

	data->touch_pressed_num = 0;
	data->tsp_booster = kzalloc(sizeof(struct input_booster), GFP_KERNEL);
	if (!data->tsp_booster) {
		dev_err(&client->dev,
			"%s: Failed to alloc mem for tsp_booster\n", __func__);
		error = -ENOMEM;
		goto err_get_tsp_booster;
	} else {
		input_booster_init_dvfs(data->tsp_booster, INPUT_BOOSTER_ID_TSP);
	}
#endif



#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = mxt_early_suspend;
	data->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	return 0;
	
#ifdef COMMON_INPUT_BOOSTER
	kfree(data->tsp_booster);
err_get_tsp_booster:
#endif
	free_irq(client->irq,data);
err_req_irq:
err_touch_init:
	mxt_power_off(data);
err_power_on:
	mxt_sysfs_remove(data);
err_sysfs_init:
	input_unregister_device(input_dev);
	input_dev = NULL;
err_register_input_device:
	i2c_unregister_device(data->client_boot);
err_create_sub_client:
	regulator_put(data->vcc_supply);
err_regulator:
	input_free_device(input_dev);
err_allocate_input_device:
#ifdef CHARGER_NOTIFIER
	tsp_driver = NULL;
#endif
	kfree(data);
err_allocate_data:
	if (client->dev.of_node) {
#if ENABLE_TOUCH_KEY
		devm_kfree(&client->dev, (void *)pdata->touchkey);
#endif
		devm_kfree(&client->dev, (void *)pdata);
	}

	return error;
}

static int  mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);
	kfree(data->objects);
	kfree(data->reportids);
	input_unregister_device(data->input_dev);
	i2c_unregister_device(data->client_boot);
	mxt_sysfs_remove(data);
	mxt_power_off(data);
	if (client->dev.of_node) {
#if ENABLE_TOUCH_KEY
		devm_kfree(&client->dev, (void *)data->pdata->touchkey);
#endif
		devm_kfree(&client->dev, (void *)data->pdata);
	}
	kfree(data);

	return 0;
}

static struct i2c_device_id mxt_idtable[] = {
	{MXT_DEV_NAME, 0},
};

MODULE_DEVICE_TABLE(i2c, mxt_idtable);

#ifdef CONFIG_OF
static struct of_device_id mxt_dt_ids[] = {
	{ .compatible = "atmel,mxt_t"},
	{ }
};
#endif

static const struct dev_pm_ops mxt_pm_ops = {
	.suspend = mxt_suspend,
	.resume = mxt_resume,
};

static struct i2c_driver mxt_i2c_driver = {
	.id_table = mxt_idtable,
	.probe = mxt_probe,
	.remove = mxt_remove,
	.shutdown = mxt_shutdown,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= MXT_DEV_NAME,
#ifdef CONFIG_PM
		.pm	= &mxt_pm_ops,
#endif
#ifdef CONFIG_OF
		.of_match_table = mxt_dt_ids,
#endif
	},
};

static int  mxt_i2c_init(void)
{
#ifdef CONFIG_SAMSUNG_LPM_MODE
		if (poweroff_charging) {
			pr_err("%s : LPM Charging Mode!!\n", __func__);
			return 0;
		}
#endif
	printk(KERN_ALERT "%s\n", __func__);
	return i2c_add_driver(&mxt_i2c_driver);
}

static void  mxt_i2c_exit(void)
{
	printk(KERN_ALERT "%s\n", __func__);
	i2c_del_driver(&mxt_i2c_driver);
}

module_init(mxt_i2c_init);
module_exit(mxt_i2c_exit);

MODULE_DESCRIPTION("Atmel MaXTouch driver");
MODULE_AUTHOR("bumwoo.lee<bw365.lee@samsung.com>");
MODULE_LICENSE("GPL");
