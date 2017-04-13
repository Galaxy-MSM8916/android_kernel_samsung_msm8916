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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/unaligned.h>

#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>

#include "ist30xxc.h"
#include "ist30xxc_tracking.h"

/******************************************************************************
 * Return value of Error
 * EPERM  : 1 (Operation not permitted)
 * ENOENT : 2 (No such file or directory)
 * EIO    : 5 (I/O error)
 * ENXIO  : 6 (No such device or address)
 * EINVAL : 22 (Invalid argument)
 *****************************************************************************/

int ist30xx_cmd_gesture(struct i2c_client *client, int value)
{
	int ret = -EIO;

	if (value > 3)
		return ret;

	ret = ist30xx_write_cmd(client,
		IST30XX_HIB_CMD, (eHCOM_GESTURE_EN << 16) | (value & 0xFFFF));

	return ret;
}

int ist30xx_cmd_start_scan(struct ist30xx_data *data)
{
	int ret = ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
		(eHCOM_FW_START << 16) | (IST30XX_ENABLE & 0xFFFF));

	ist30xx_tracking(TRACK_CMD_SCAN);

	data->status.noise_mode = true;

	return ret;
}

int ist30xx_cmd_calibrate(struct i2c_client *client)
{
	int ret = ist30xx_write_cmd(client,
		IST30XX_HIB_CMD, (eHCOM_RUN_CAL_AUTO << 16));

	ist30xx_tracking(TRACK_CMD_CALIB);

	tsp_info("%s\n", __func__);

	return ret;
}

int ist30xx_cmd_check_calib(struct i2c_client *client)
{
	int ret = ist30xx_write_cmd(client, IST30XX_HIB_CMD,
		(eHCOM_RUN_CAL_PARAM << 16) | (IST30XX_ENABLE & 0xFFFF));

	ist30xx_tracking(TRACK_CMD_CHECK_CALIB);

	tsp_info("*** Check Calibration cmd ***\n");

	return ret;
}

int ist30xx_cmd_hold(struct ist30xx_data *data, int enable)
{
	int ret;
	struct i2c_client *client = data->client;

	if (!data->initialized && (data->status.update != 1))
		return 0;

	ret = ist30xx_write_cmd(client,
		IST30XX_HIB_CMD, (eHCOM_FW_HOLD << 16) | (enable & 0xFFFF));
	tsp_info("%s: FW HOLDE %s\n", __func__, enable ? "Enable" : "Disable");

	if (enable)
		ist30xx_tracking(TRACK_CMD_ENTER_REG);
	else
		ist30xx_tracking(TRACK_CMD_EXIT_REG);

	return ret;
}

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
		tsp_err("%s() : transaction size(%d)\n", __func__, max_size);
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
			tsp_err("%s() : i2c_transfer failed(%d), num=%d\n",
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
	msg.len = len * IST30XX_DATA_LEN;
	msg.buf = msg_buf;

	ret = ist30xx_i2c_transfer(client->adapter, &msg, WRITE_CMD_MSG_LEN,
				   cmd_buf);
	if (unlikely(ret != WRITE_CMD_MSG_LEN))
		return -EIO;

	return 0;
}

int ist30xx_read_reg(struct i2c_client *client, u32 reg, u32 *buf)
{
	int ret;
	u32 le_reg = cpu_to_be32(reg);

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
			.len = IST30XX_DATA_LEN,
			.buf = (u8 *)buf,
		},
	};

	ret = i2c_transfer(client->adapter, msg, READ_CMD_MSG_LEN);
	if (ret != READ_CMD_MSG_LEN) {
		tsp_err("%s: i2c failed (%d), cmd: %x\n", __func__, ret, reg);
		return -EIO;
	}
	*buf = cpu_to_be32(*buf);

	return 0;
}

int ist30xx_read_cmd(struct ist30xx_data *data, u32 cmd, u32 *buf)
{
	int ret;

	ret = ist30xx_cmd_hold(data, 1);
	if (unlikely(ret))
		return ret;

	ist30xx_read_reg(data->client, IST30XX_DA_ADDR(cmd), buf);

	ret = ist30xx_cmd_hold(data, 0);
	if (unlikely(ret)) {
		ist30xx_reset(data, false);
		return ret;
	}

	return ret;
}

int ist30xx_write_cmd(struct i2c_client *client, u32 cmd, u32 val)
{
	int ret;

	struct i2c_msg msg;
	u8 msg_buf[IST30XX_ADDR_LEN + IST30XX_DATA_LEN];

	put_unaligned_be32(cmd, msg_buf);
	put_unaligned_be32(val, msg_buf + IST30XX_ADDR_LEN);

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = IST30XX_ADDR_LEN + IST30XX_DATA_LEN;
	msg.buf = msg_buf;

	ret = i2c_transfer(client->adapter, &msg, WRITE_CMD_MSG_LEN);
	if (ret != WRITE_CMD_MSG_LEN) {
		tsp_err("%s: i2c failed (%d), cmd: %x(%x)\n", __func__, ret, cmd, val);
		return -EIO;
	}

	msleep(40);

	return 0;
}

int ist30xx_burst_read(struct i2c_client *client, u32 addr,
			u32 *buf32, u16 len, bool bit_en)
{
	int ret = 0;
	int i;
	u16 max_len = I2C_MAX_READ_SIZE / IST30XX_DATA_LEN;
	u16 remain_len = len;

	if (bit_en)
		addr = IST30XX_BA_ADDR(addr);

	for (i = 0; i < len; i += max_len) {
		if (remain_len < max_len) max_len = remain_len;

		ret = ist30xx_read_buf(client, addr, buf32, max_len);
		if (unlikely(ret)) {
			tsp_err("Burst fail, addr: %x\n", __func__, addr);
			return ret;
		}

		addr += max_len * IST30XX_DATA_LEN;
		buf32 += max_len;
		remain_len -= max_len;
	}

	return 0;
}

int ist30xx_burst_write(struct i2c_client *client, u32 addr,
		u32 *buf32, u16 len)
{
	int ret = 0;
	int i;
	u16 max_len = I2C_MAX_WRITE_SIZE / IST30XX_DATA_LEN;
	u16 remain_len = len;

	addr = IST30XX_BA_ADDR(addr);

	for (i = 0; i < len; i += max_len) {
		if (remain_len < max_len) max_len = remain_len;

		ret = ist30xx_write_buf(client, addr, buf32, max_len);
		if (unlikely(ret)) {
			tsp_err("Burst fail, addr: %x\n", __func__, addr);
			return ret;
		}

		addr += max_len * IST30XX_DATA_LEN;
		buf32 += max_len;
		remain_len -= max_len;
	}

	return 0;
}

//static struct regulator *touch_regulator;
int ts_power_enable(struct ist30xx_data *data, int en)
{
	int rc = 0;
	static struct regulator *ldo6;
	const char *reg_name;

	reg_name = "8916_l6";

	tsp_info("%s %s\n", __func__, (en) ? "on" : "off");

	if (!ldo6) {
		ldo6 = regulator_get(NULL, reg_name);

		if (IS_ERR(ldo6)) {
			tsp_err("%s: could not get %s, rc = %ld\n",
				__func__, reg_name, PTR_ERR(ldo6));
			return -EINVAL;
		}
		rc = regulator_set_voltage(ldo6, 1800000, 1800000);
		if (rc)
			tsp_err("%s: %s set_level failed (%d)\n", __func__, reg_name, rc);
	}

	/*Power from MFD internal LDO */
	if(data->dt_data->tsp_vdd_name) {
		if(!data->dt_data->tsp_power) {
			data->dt_data->tsp_power = regulator_get(NULL, data->dt_data->tsp_vdd_name);

			if (IS_ERR(data->dt_data->tsp_power)) {
				tsp_err("%s: could not get tsp_power, rc = %ld\n",
					__func__,PTR_ERR(data->dt_data->tsp_power));
				return -EINVAL;
			}
			rc = regulator_set_voltage(data->dt_data->tsp_power, 3000000, 3000000);
			if (rc)
				tsp_err("%s: %s set_level failed (%d)\n", __func__, data->dt_data->tsp_vdd_name, rc);
		}
	}

	if (en) {
		rc = regulator_enable(ldo6);
		if (rc) {
			tsp_err("%s: %s enable failed (%d)\n", __func__, reg_name, rc);
			return -EINVAL;
		}
		if(data->dt_data->tsp_vdd_name) {
			rc = regulator_set_voltage(data->dt_data->tsp_power, 3000000, 3000000);
			if (rc)
				tsp_err("%s: %s set_level failed (%d)\n", __func__, data->dt_data->tsp_vdd_name, rc);
			rc = regulator_enable(data->dt_data->tsp_power);
			if (rc) {
				tsp_err("%s: %s enable failed (%d)\n", __func__, data->dt_data->tsp_vdd_name, rc);
				return -EINVAL;
			}
		}
	} else {
		rc = regulator_disable(ldo6);
		if (rc) {
			tsp_err("%s: %s disable failed (%d)\n", __func__, reg_name, rc);
			return -EINVAL;
		}
		if(data->dt_data->tsp_vdd_name) {
			rc = regulator_disable(data->dt_data->tsp_power);
			if (rc) {
				tsp_err("%s: %s disable failed (%d)\n", __func__, data->dt_data->tsp_vdd_name, rc);
				return -EINVAL;
			}
		}
	}
	if(!data->dt_data->tsp_vdd_name) {
		rc = gpio_direction_output(data->dt_data->touch_en_gpio, en);
			if (rc) {
			tsp_err("%s: unable to set_direction for TSP_EN [%d]\n",
				__func__, data->dt_data->touch_en_gpio);
			}
		tsp_info("%s: touch_en: %d, %s: %d \n", __func__, gpio_get_value(data->dt_data->touch_en_gpio), reg_name, regulator_is_enabled(ldo6));
	} else {
		tsp_info("%s: %s: %d %s: %d\n", __func__, reg_name, regulator_is_enabled(ldo6), data->dt_data->tsp_vdd_name, regulator_is_enabled(data->dt_data->tsp_power));
	}
	return rc;
}

int ist30xx_power_on(struct ist30xx_data *data, bool download)
{
	int rc = 0;
	if (data->status.power != 1) {
		tsp_info("%s()\n", __func__);
		ist30xx_tracking(TRACK_PWR_ON);
		/* VDD enable */
/*		msleep(5);*/
		/* VDDIO enable */
		rc = ts_power_enable(data, 1);
		if (download)
			msleep(8);
        else
			msleep(60);

	if (!rc) /*power is enabled successfully*/
		data->status.power = 1;
	}

	return rc;
}

int ist30xx_power_off(struct ist30xx_data *data)
{
	int rc = 0;
	if (data->status.power != 0) {
		tsp_info("%s()\n", __func__);
		ist30xx_tracking(TRACK_PWR_OFF);
		/* VDDIO disable */
/*		msleep(5);*/
		/* VDD disable */
		rc = ts_power_enable(data, 0);
		msleep(50);
		if (!rc) /*power is disabled successfully*/
			data->status.power = 0;
		data->status.noise_mode = false;
	}

	return 0;
}

int ist30xx_reset(struct ist30xx_data *data, bool download)
{
	tsp_info("%s()\n", __func__);
	ist30xx_power_off(data);
	msleep(10);
	ist30xx_power_on(data, download);

	return 0;
}

int ist30xx_internal_suspend(struct ist30xx_data *data)
{
#if IST30XX_GESTURE
	data->suspend = true;
    if (data->gesture) {
        ist30xx_reset(data, false);
        ist30xx_cmd_gesture(data->client, 3);
    } else {
        ist30xx_power_off(data);
    }
#else
	ist30xx_power_off(data);
#endif
	return 0;
}

int ist30xx_internal_resume(struct ist30xx_data *data)
{
#if IST30XX_GESTURE
	data->suspend = false;
    if (data->gesture)
		ist30xx_reset(data, false);
    else
        ist30xx_power_on(data, false);
#else
	ist30xx_power_on(data, false);
#endif

	return 0;
}

int ist30xx_init_system(struct ist30xx_data *data)
{
	int ret;

	// TODO : place additional code here.
	ret = ist30xx_power_on(data, false);
	if (ret) {
		tsp_err("%s: ist30xx_init_system failed (%d)\n", __func__, ret);
		return -EIO;
	}

#if 0
	ret = ist30xx_reset(data, false);
	if (ret) {
		tsp_err("%s: ist30xx_reset failed (%d)\n", __func__, ret);
		return -EIO;
	}
#endif

	return 0;
}
