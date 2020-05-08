/*
 *
 * Zinitix zt7538 touchscreen driver
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
#include "zt7538.h"

u32 BUTTON_MAPPING_KEY[MAX_SUPPORTED_BUTTON_NUM] = {KEY_RECENT, KEY_BACK};

static int cal_mode;
static int get_boot_mode(char *str)
{

	get_option(&str, &cal_mode);
	printk(KERN_DEBUG "get_boot_mode, uart_mode : %d\n", cal_mode);
	return 1;
}
__setup("calmode=", get_boot_mode);

static void zinitix_delay(unsigned int ms)
{
	if (ms <= 20)
		usleep_range(ms * 1000, ms * 1000);
	else
		msleep(ms);
}

static s32 read_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	int count = 0;
retry_send:
	ret = i2c_master_send(client, (u8 *)&reg, 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to send. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry_send;
		return ret;
	}

	usleep_range(DELAY_FOR_TRANSCATION, DELAY_FOR_TRANSCATION);

	count = 0;
retry_read:
	ret = i2c_master_recv(client, values, length);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to recv. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry_read;
		return ret;
	}

	usleep_range(DELAY_FOR_POST_TRANSCATION, DELAY_FOR_POST_TRANSCATION);
	return length;
}

#if TOUCH_POINT_MODE
static s32 read_data_only(struct i2c_client *client, u8 *values, u16 length)
{
	s32 ret;
	int count = 0;

retry:
	ret = i2c_master_recv(client, values, length);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to recv. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry;
		return ret;
	}
	usleep_range(DELAY_FOR_TRANSCATION, DELAY_FOR_TRANSCATION);
	return length;
}
#endif

static inline s32 write_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	int count = 0;
	u8 pkt[10];

	pkt[0] = (reg) & 0xff;
	pkt[1] = (reg >> 8) & 0xff;
	memcpy((u8 *)&pkt[2], values, length);

retry:
	ret = i2c_master_send(client , pkt , length + 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to send. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry;
		return ret;
	}

	usleep_range(DELAY_FOR_POST_TRANSCATION, DELAY_FOR_POST_TRANSCATION);
	return length;
}

static s32 write_reg(struct i2c_client *client, u16 reg, u16 value)
{
	if (write_data(client, reg, (u8 *)&value, 2) < 0)
		return I2C_FAIL;

	return I2C_SUCCESS;
}

static s32 write_cmd(struct i2c_client *client, u16 reg)
{
	s32 ret;
	int count = 0;

retry:
	ret = i2c_master_send(client , (u8 *)&reg , 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to send. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry;
		return ret;
	}

	usleep_range(DELAY_FOR_POST_TRANSCATION, DELAY_FOR_POST_TRANSCATION);
	return I2C_SUCCESS;
}

static inline s32 read_raw_data(struct i2c_client *client, u16 reg, u8 *values, u16 length)
{
	s32 ret;
	int count = 0;

retry_send:
	/* select register */
	ret = i2c_master_send(client , (u8 *)&reg , 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to send. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry_send;
		return ret;
	}

	/* for setup tx transaction. */
	usleep_range(200, 200);

	count = 0;
retry_read:
	ret = i2c_master_recv(client, values, length);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to recv. ret:%d, try:%d\n",
							__func__, ret, count + 1);
		zinitix_delay(1);
		if (++count < I2C_RETRY_TIMES)
			goto retry_read;
		return ret;
	}

	usleep_range(DELAY_FOR_POST_TRANSCATION, DELAY_FOR_POST_TRANSCATION);
	return length;
}

static inline s32 read_firmware_data(struct i2c_client *client, u16 addr, u8 *values, u16 length)
{
	s32 ret;

	ret = i2c_master_send(client , (u8 *)&addr , 2);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to send. ret:%d\n", __func__, ret);
		return ret;
	}

	/* for setup tx transaction. */
	zinitix_delay(1);
	ret = i2c_master_recv(client , values , length);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to recv. ret:%d\n", __func__, ret);
		return ret;
	}

	usleep_range(DELAY_FOR_POST_TRANSCATION, DELAY_FOR_POST_TRANSCATION);
	return length;
}

static void zt7538_set_optional_mode(struct zt7538_ts_info *info, bool force)
{
	if (!info->device_enabled)
		return;
	if (m_prev_optional_mode == m_optional_mode && !force)
		return;

	if (write_reg(info->client, ZT7538_OPTIONAL_SETTING, m_optional_mode) == I2C_SUCCESS) {
		m_prev_optional_mode = m_optional_mode;
		dev_info(&misc_info->client->dev, "%s: 0x%04x\n",
						__func__, m_optional_mode);
	}
}
static void zt7538_set_ta_status(struct zt7538_ts_info *info)
{
	if (ta_connected)
		zinitix_bit_set(m_optional_mode, DEF_OPTIONAL_MODE_USB_DETECT_BIT);
	else
		zinitix_bit_clr(m_optional_mode, DEF_OPTIONAL_MODE_USB_DETECT_BIT);

	zt7538_set_optional_mode(info, false);
}

static void cover_set(struct zt7538_ts_info *info)
{
	if (g_cover_state == COVER_OPEN) {
		zinitix_bit_clr(m_optional_mode, DEF_OPTIONAL_MODE_SVIEW_DETECT_BIT);
	}
	else if (g_cover_state == COVER_CLOSED) {
		zinitix_bit_set(m_optional_mode, DEF_OPTIONAL_MODE_SVIEW_DETECT_BIT);
	}

	if (info->work_state == SUSPEND || info->work_state == EALRY_SUSPEND || info->work_state == PROBE)
		return;

	zt7538_set_optional_mode(info, true);
}

#ifdef TSP_MUIC_NOTIFICATION
int tsp_cable_check(muic_attached_dev_t attached_dev)
{
	int current_cable_type = -1;

	switch (attached_dev) {
	case ATTACHED_DEV_NONE_MUIC:
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
	case ATTACHED_DEV_MHL_MUIC:
	case ATTACHED_DEV_DESKDOCK_MUIC:
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		current_cable_type = 0;
		break;
	default:
		current_cable_type = 1;
		break;
	}

	return current_cable_type;

}

int zt7538_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	muic_attached_dev_t attached_dev = *(muic_attached_dev_t *)data;
	const char *cmd;
	int cable_type;

	switch (action) {
	case MUIC_NOTIFY_CMD_DETACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_DETACH:
		cmd = "DETACH";
		cable_type = 0;
		break;
	case MUIC_NOTIFY_CMD_ATTACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_ATTACH:
		cmd = "ATTACH";
		cable_type = tsp_cable_check(attached_dev);
		break;
	default:
		cmd = "ERROR";
		cable_type = -1;
		break;
	}

	pr_info("%s: cmd=%s, attached_dev=%d, cable_type=%d\n", __func__, cmd, attached_dev, cable_type);

	if (cable_type == 1)
		ta_connected = true;
	else
		ta_connected = false;

	zt7538_set_ta_status(misc_info);

	return 0;
}
#endif

#define I2C_BUFFER_SIZE 64
static bool get_raw_data(struct zt7538_ts_info *info, u8 *buff, int skip_cnt)
{
	struct i2c_client *client = info->client;
	struct zt7538_ts_dt_data *pdata = info->pdata;
	u32 total_node = info->cap_info.total_node_num;
	int sz;
	int i;
	u32 temp_sz;

	disable_irq(info->irq);

	down(&info->work_lock);
	if (info->work_state != NOTHING) {
		dev_err(&client->dev, "%s: other process occupied. (%d)\n",
			__func__, info->work_state);
		enable_irq(info->irq);
		up(&info->work_lock);
		return false;
	}

	info->work_state = RAW_DATA;

	for (i = 0; i < skip_cnt; i++) {
		while (gpio_get_value(pdata->gpio_int))
			zinitix_delay(1);

		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
		zinitix_delay(1);
	}

	sz = total_node * 2;

	while (gpio_get_value(pdata->gpio_int))
		zinitix_delay(1);

	for (i = 0; sz > 0; i++) {
		temp_sz = I2C_BUFFER_SIZE;

		if (sz < I2C_BUFFER_SIZE)
			temp_sz = sz;
		if (read_raw_data(client, ZT7538_RAWDATA_REG + i,
			(char *)(buff + (i * I2C_BUFFER_SIZE)), temp_sz) < 0) {

			dev_err(&info->client->dev, "error : read zinitix tc raw data\n");
			info->work_state = NOTHING;
			enable_irq(info->irq);
			up(&info->work_lock);
			return false;
		}
		sz -= I2C_BUFFER_SIZE;
	}

	write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
	info->work_state = NOTHING;
	enable_irq(info->irq);
	up(&info->work_lock);

	return true;
}

static bool ts_get_raw_data(struct zt7538_ts_info *info)
{
	struct i2c_client *client = info->client;
	u32 total_node = info->cap_info.total_node_num;
	int sz;
	u16 temp_sz;
	int i;

	if (down_trylock(&info->raw_data_lock)) {
		dev_err(&client->dev, "Failed to occupy sema\n");
		info->touch_info.status = 0;
		return true;
	}

	sz = total_node * 2 + sizeof(struct point_info);

	for (i = 0; sz > 0; i++) {
		temp_sz = I2C_BUFFER_SIZE;

		if (sz < I2C_BUFFER_SIZE)
			temp_sz = sz;

		if (read_raw_data(info->client, ZT7538_RAWDATA_REG + i,
			(char *)((u8*)(info->cur_data)+ (i * I2C_BUFFER_SIZE)), temp_sz) < 0) {

			dev_err(&client->dev, "Failed to read raw data\n");
			up(&info->raw_data_lock);
			return false;
		}
		sz -= I2C_BUFFER_SIZE;
	}

	info->update = 1;
	memcpy((u8 *)(&info->touch_info),
			(u8 *)&info->cur_data[total_node], sizeof(struct point_info));
	up(&info->raw_data_lock);

	return true;
}

static bool ts_read_coord(struct zt7538_ts_info *info)
{
	struct i2c_client *client = info->client;

#if TOUCH_POINT_MODE
	int i;
#endif

/* Debugging Tool for zinitix tuning */
	if (info->touch_mode != TOUCH_POINT_MODE) {
		if (info->update == 0) {
			if (!ts_get_raw_data(info))
				return false;
		} else
			info->touch_info.status = 0;
		dev_err(&client->dev, "status = 0x%04X\n", info->touch_info.status);
		goto out;
	}

#if TOUCH_POINT_MODE
		memset(&info->touch_info, 0x0, sizeof(struct point_info));

		if (read_data_only(info->client, (u8 *)(&info->touch_info), 10) < 0) {
			dev_err(&client->dev, "error read point info using i2c.-\r\n");
			return false;
		}

		if (info->touch_info.event_flag == 0 || info->touch_info.status == 0) {
			zt7538_set_optional_mode(info, false);
			write_cmd(info->client, ZT7538_CLEAR_INT_STATUS_CMD);
			return true;
		}

		for (i = 1; i < info->cap_info.multi_fingers; i++) {
			if (zinitix_bit_test(info->touch_info.event_flag, i)) {
				usleep_range(20, 20);
				if (read_data(info->client, ZT7538_POINT_STATUS_REG + 2 + (i * 4),
					(u8 *)(&info->touch_info.coord[i]), sizeof(struct coord)) < 0) {
					dev_err(&client->dev, "error read point info\n");
					return false;
				}
			}
		}
#endif

	if (read_data(info->client, ZT7538_POINT_STATUS_REG,
			(u8 *)(&info->touch_info), sizeof(struct point_info)) < 0) {
		dev_err(&client->dev, "Failed to read point info\n");
		return false;
	}
out:
	if (zinitix_bit_test(info->touch_info.status, BIT_MUST_ZERO)) {
		dev_err(&client->dev, "Invalid must zero bit(%04x)\n", info->touch_info.status);
		return false;
	}
#ifdef DEF_OPTIONAL_STATE_CHECK
	if (zinitix_bit_test(info->touch_info.status, BIT_DEBUG)) {
		if (read_data(info->client, ZT7538_DEBUG_REGSITER,	(u8 *)&m_debug_register, 2 ) < 0) {
			dev_err(&client->dev, "error read debug register\n");
			return false;
		}
	}
#endif
	write_cmd(info->client, ZT7538_CLEAR_INT_STATUS_CMD);

	return true;
}

#if ESD_TIMER_INTERVAL
static void esd_timeout_handler(unsigned long data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)data;

	info->p_esd_timeout_tmr = NULL;
	queue_work(esd_tmr_workqueue, &info->tmr_work);
}

static void esd_timer_start(u16 sec, struct zt7538_ts_info *info)
{
	unsigned long flags;
	spin_lock_irqsave(&info->lock, flags);
	if (info->p_esd_timeout_tmr)
#ifdef CONFIG_SMP
		del_singleshot_timer_sync(info->p_esd_timeout_tmr);
#else
		del_timer(info->p_esd_timeout_tmr);
#endif
	info->p_esd_timeout_tmr = NULL;
	init_timer(&(info->esd_timeout_tmr));
	info->esd_timeout_tmr.data = (unsigned long)(info);
	info->esd_timeout_tmr.function = esd_timeout_handler;
	info->esd_timeout_tmr.expires = jiffies + (HZ * sec);
	info->p_esd_timeout_tmr = &info->esd_timeout_tmr;
	add_timer(&info->esd_timeout_tmr);
	spin_unlock_irqrestore(&info->lock, flags);
}

static void esd_timer_stop(struct zt7538_ts_info *info)
{
	unsigned long flags;
	spin_lock_irqsave(&info->lock, flags);
	if (info->p_esd_timeout_tmr)
#ifdef CONFIG_SMP
		del_singleshot_timer_sync(info->p_esd_timeout_tmr);
#else
		del_timer(info->p_esd_timeout_tmr);
#endif

	info->p_esd_timeout_tmr = NULL;
	spin_unlock_irqrestore(&info->lock, flags);
}

static void esd_timer_init(struct zt7538_ts_info *info)
{
	unsigned long flags;
	spin_lock_irqsave(&info->lock, flags);
	init_timer(&(info->esd_timeout_tmr));
	info->esd_timeout_tmr.data = (unsigned long)(info);
	info->esd_timeout_tmr.function = esd_timeout_handler;
	info->p_esd_timeout_tmr = NULL;
	spin_unlock_irqrestore(&info->lock, flags);
}

static void ts_tmr_work(struct work_struct *work)
{
	struct zt7538_ts_info *info =
				container_of(work, struct zt7538_ts_info, tmr_work);
	struct i2c_client *client = info->client;

	if (down_trylock(&info->work_lock)) {
		dev_err(&client->dev, "%s: Failed to occupy work lock\n", __func__);
		esd_timer_start(CHECK_ESD_TIMER, info);
		return;
	}

	if (info->work_state != NOTHING) {
		dev_info(&client->dev, "%s: Other process occupied\n", __func__);
		up(&info->work_lock);
		return;
	}
	info->work_state = ESD_TIMER;

	disable_irq(info->irq);
	zt7538_power_control(info, POWER_OFF);
	zt7538_power_control(info, POWER_ON_SEQUENCE);

	clear_report_data(info);
	if (!mini_init_touch(info))
		goto fail_time_out_init;

	info->work_state = NOTHING;
	enable_irq(info->irq);
	up(&info->work_lock);

	return;
fail_time_out_init:
	dev_err(&client->dev, "%s: Failed to restart\n", __func__);
	esd_timer_start(CHECK_ESD_TIMER, info);
	info->work_state = NOTHING;
	enable_irq(info->irq);
	up(&info->work_lock);

	return;
}
#endif

static bool zt7538_power_sequence(struct zt7538_ts_info *info)
{
	struct i2c_client *client = info->client;
	int retry = 0;

retry_power_sequence:
	info->chip_code = 0;

	if (write_reg(client, 0xc000, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "Failed to send power sequence(vendor cmd enable)\n");
		goto fail_power_sequence;
	}
	usleep_range(10, 10);

	if (read_data(client, 0xcc00, (u8 *)&info->chip_code, 2) < 0) {
		dev_err(&client->dev, "Failed to read chip code\n");
		goto fail_power_sequence;
	}

	dev_dbg(&client->dev, "%s: chip code = 0x%x\n", __func__, info->chip_code);
	if (info->chip_code == ZT7538_IC_CHIP_CODE) {
		info->cap_info.ic_fw_size = 44 * 1024;
	} else {
		dev_err(&client->dev, "%s: Unknown IC!! Cannot set ic_fw_size!!\n", __func__);
		goto fail_power_sequence;
	}
	usleep_range(10, 10);

	if (write_cmd(client, 0xc004) != I2C_SUCCESS) {
		dev_err(&client->dev, "Failed to send power sequence(intn clear)\n");
		goto fail_power_sequence;
	}
	usleep_range(10, 10);

	if (write_reg(client, 0xc002, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "Failed to send power sequence(nvm init)\n");
		goto fail_power_sequence;
	}
	zinitix_delay(2);

	if (write_reg(client, 0xc001, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "Failed to send power sequence(program start)\n");
		goto fail_power_sequence;
	}

	zinitix_delay(FIRMWARE_ON_DELAY);	/* wait for checksum cal */

	return true;

fail_power_sequence:
	if (retry++ < 3) {
		zinitix_delay(CHIP_ON_DELAY);
		dev_info(&client->dev, "retry = %d\n", retry);
		goto retry_power_sequence;
	}

	dev_err(&client->dev, "Failed to send power sequence\n");
	return false;
}

static int zt7538_power(struct i2c_client *client, int on)
{
	int ret;
	struct zt7538_ts_info *info = i2c_get_clientdata(client);
	struct zt7538_ts_dt_data *pdata = info->pdata;
	static struct regulator *vddo;
	static struct regulator *avdd;
	static bool reg_boot_on = true;

	if (on == POWER_ON_SEQUENCE)
		on = POWER_ON;

	if (!pdata) {
		dev_err(&client->dev, "%s: pdata is NULL \n", __func__);
		return -ENODEV;
	}

	if (reg_boot_on && !pdata->reg_boot_on)
		reg_boot_on = false;

	if (gpio_is_valid(pdata->vdd_en)) {
		ret = gpio_direction_output(pdata->vdd_en, on);
		if (ret) {
			dev_err(&client->dev, "%s: unable to set_direction for zt_vdd_en [%d]\n",
				__func__, pdata->vdd_en);
			return -EINVAL;
		}
	} else {
		if (IS_ERR_OR_NULL(avdd)) {
			avdd = regulator_get(&info->client->dev, "avdd");
			if (IS_ERR_OR_NULL(avdd)) {
				dev_err(&client->dev, "%s: could not get avdd, ret = %ld\n",
					__func__, IS_ERR(avdd));
				return -EINVAL;
			}

			ret = regulator_set_voltage(avdd, 3300000, 3300000);
			if (ret) {
				dev_err(&client->dev, "%s: could not set voltage avdd, ret = %d\n",
					__func__, ret);
			}
		}

		if (on) {
			if (!reg_boot_on && regulator_is_enabled(avdd)) {
				dev_info(&client->dev, "%s: avdd is already enabled\n", __func__);
			} else {
				ret = regulator_enable(avdd);
				if (ret) {
					dev_err(&client->dev, "%s: avdd enable failed (%d)\n", __func__, ret);
					return -EINVAL;
				}
			}
		} else {
			if (regulator_is_enabled(avdd)) {
				ret = regulator_disable(avdd);
				if (ret) {
					dev_err(&client->dev, "%s: avdd disable failed (%d)\n", __func__, ret);
					return -EINVAL;
			}
			} else {
				dev_info(&client->dev, "%s: avdd is already disabled\n", __func__);
			}
		}
	}

	if (IS_ERR_OR_NULL(vddo)) {
		vddo = regulator_get(&info->client->dev, "vddo");
		if (IS_ERR_OR_NULL(vddo)) {
			dev_err(&client->dev, "%s: could not get vddo, ret = %ld\n",
				__func__, IS_ERR(vddo));
			return -EINVAL;
		}

		ret = regulator_set_voltage(vddo, 1800000, 1800000);
		if (ret) {
			dev_err(&client->dev, "%s: could not set voltage vddo, ret = %d\n",
				__func__, ret);
		}
	}

	if (on) {
		ret = regulator_enable(vddo);
		if (ret) {
			dev_err(&client->dev, "%s: vddo enable failed (%d)\n", __func__, ret);
			return -EINVAL;
		}
	} else {
		ret = regulator_disable(vddo);
		if (ret) {
			dev_err(&client->dev, "%s: vddo disable failed (%d)\n", __func__, ret);
			return -EINVAL;
		}
	}

	pr_info("%s %s: ", __func__, on ? "on" : "off");
	if (gpio_is_valid(pdata->vdd_en))
		pr_cont("vdd_en %d", gpio_get_value(pdata->vdd_en));
	else if (!IS_ERR_OR_NULL(avdd))
		pr_cont("avdd %d", regulator_is_enabled(avdd));
	if (!IS_ERR_OR_NULL(vddo))
		pr_cont(" vddo %d", regulator_is_enabled(vddo));
	pr_cont("\n");

	if (on >= POWER_ON) {
		if (!reg_boot_on)
			zinitix_delay(CHIP_ON_DELAY);
		reg_boot_on = false;
	}
	else {
		zinitix_delay(CHIP_OFF_DELAY);
	}

	return 0;
}
static bool zt7538_power_control(struct zt7538_ts_info *info, u8 ctl)
{
	int ret;

	ret = info->pdata->tsp_power(info->client, ctl);
	if (ret)
		return false;

	if (ctl == POWER_ON_SEQUENCE) {
		return zt7538_power_sequence(info);
	}

	return true;
}

static bool ts_check_need_upgrade(struct zt7538_ts_info *info,
	u16 cur_version, u16 cur_minor_version, u16 cur_reg_version, u16 cur_hw_id)
{
	u16	new_version;
	u16	new_minor_version;
	u16	new_reg_version;
#if CHECK_HWID
	u16	new_hw_id;
#endif

	if (!info->fw_data) {
		dev_err(&info->client->dev, "%s: fw_data is NULL\n", __func__);
		return false;
	}

	new_version = (u16) (info->fw_data[52] | (info->fw_data[53]<<8));
	new_minor_version = (u16) (info->fw_data[56] | (info->fw_data[57]<<8));
	new_reg_version = (u16) (info->fw_data[60] | (info->fw_data[61]<<8));

#if CHECK_HWID
	new_hw_id = (u16) (info->fw_data[48] | (info->fw_data[49]<<8));
	dev_dbg(&info->client->dev, "cur HW_ID = 0x%x, new HW_ID = 0x%x\n",
							cur_hw_id, new_hw_id);
#endif

	dev_info(&info->client->dev, "cur version = 0x%x, new version = 0x%x\n",
							cur_version, new_version);
	dev_info(&info->client->dev, "cur minor version = 0x%x, new minor version = 0x%x\n",
						cur_minor_version, new_minor_version);
	dev_info(&info->client->dev, "cur reg data version = 0x%x, new reg data version = 0x%x\n",
						cur_reg_version, new_reg_version);
	if (cal_mode) {
		dev_info(&info->client->dev, "didn't update TSP F/W!! in CAL MODE\n");
		return false;
	}

	if (cur_reg_version == 0xffff)
		return true;
	if (cur_version > 0xFF)
		return true;
	if (cur_version < new_version)
		return true;
	else if (cur_version > new_version)
		return false;
#if CHECK_HWID
	if (cur_hw_id != new_hw_id)
		return true;
#endif
	if (cur_minor_version < new_minor_version)
		return true;
	else if (cur_minor_version > new_minor_version)
		return false;
	if (cur_reg_version < new_reg_version)
		return true;

	return false;
}

#define TC_SECTOR_SZ		8

static bool ts_upgrade_firmware(struct zt7538_ts_info *info, const u8 *firmware_data, u32 size)
{
	struct i2c_client *client = info->client;
	u32 flash_addr;
	u8 *verify_data;
	int i;
	int page_sz = 64;
	u16 chip_code;

	if (!firmware_data) {
		dev_err(&client->dev, "firmware is NULL\n");
		return false;
	}

	verify_data = devm_kzalloc(&client->dev, size, GFP_KERNEL);
	if (!verify_data) {
		dev_err(&client->dev, "cannot alloc verify buffer\n");
		return false;
	}

	zt7538_power_control(info, POWER_OFF);
	zt7538_power_control(info, POWER_ON);

	zinitix_delay(10);

	if (write_reg(client, 0xc000, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "power sequence error (vendor cmd enable)\n");
		goto fail_upgrade;
	}

	usleep_range(10, 10);

	if (read_data(client, 0xcc00, (u8 *)&chip_code, 2) < 0) {
		dev_err(&client->dev, "failed to read chip code\n");
		goto fail_upgrade;
	}
	dev_info(&client->dev, "chip code = 0x%x\n", chip_code);

	usleep_range(10, 10);

	if (write_cmd(client, 0xc004) != I2C_SUCCESS) {
		dev_err(&client->dev, "power sequence error (intn clear)\n");
		goto fail_upgrade;
	}

	usleep_range(10, 10);

	if (write_reg(client, 0xc002, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "power sequence error (nvm init)\n");
		goto fail_upgrade;
	}

	zinitix_delay(5);

	dev_info(&client->dev, "init flash\n");

	if (write_reg(client, 0xc003, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to write nvm vpp on\n");
		goto fail_upgrade;
	}

	if (write_reg(client, 0xc104, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to write nvm wp disable\n");
		goto fail_upgrade;
	}

	if (write_cmd(client, ZT7538_INIT_FLASH) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to init flash\n");
		goto fail_upgrade;
	}

	// Mass Erase
	//====================================================
	if (write_cmd(client, 0x01DF) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to mass erase\n");
		goto fail_upgrade;
	}

	zinitix_delay(100);

	// Mass Erase End
	//====================================================

	if (write_reg(client, 0x01DE, 0x0001) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to enter upgrade mode\n");
		goto fail_upgrade;
	}

	zinitix_delay(1);

	if (write_reg(client, 0x01D3, 0x0008) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to init upgrade mode\n");
		goto fail_upgrade;
	}

	dev_info(&client->dev, "writing firmware data\n");
	for (flash_addr = 0; flash_addr < size; ) {
		for (i = 0; i < page_sz/TC_SECTOR_SZ; i++) {
			if (write_data(client, ZT7538_WRITE_FLASH,
						(u8 *)&firmware_data[flash_addr], TC_SECTOR_SZ) < 0) {
				dev_err(&client->dev, "error : write zinitix tc firmare\n");
				goto fail_upgrade;
			}
			flash_addr += TC_SECTOR_SZ;
			usleep_range(100, 100);
		}
		zinitix_delay(8);	/*for fuzing delay*/
	}

	if (write_reg(client, 0xc003, 0x0000) != I2C_SUCCESS) {
		dev_err(&client->dev, "nvm write vpp off\n");
		goto fail_upgrade;
	}

	if (write_reg(client, 0xc104, 0x0000) != I2C_SUCCESS) {
		dev_err(&client->dev, "nvm wp enable\n");
		goto fail_upgrade;
	}

	dev_info(&client->dev, "init flash\n");
	if (write_cmd(client, ZT7538_INIT_FLASH) != I2C_SUCCESS) {
		dev_err(&client->dev, "failed to init flash\n");
		goto fail_upgrade;
	}

	dev_info(&client->dev, "read firmware data\n");
	for (flash_addr = 0; flash_addr < size; ) {
		for (i = 0; i < page_sz/TC_SECTOR_SZ; i++) {
			if (read_firmware_data(client, ZT7538_READ_FLASH,
						(u8 *)&verify_data[flash_addr], TC_SECTOR_SZ) < 0) {
				dev_err(&client->dev, "Failed to read firmare\n");
				goto fail_upgrade;
			}
			flash_addr += TC_SECTOR_SZ;
		}
	}

	/* verify */
	dev_info(&client->dev, "verify firmware data\n");
	if (memcmp((u8 *)&firmware_data[0], (u8 *)&verify_data[0], size) == 0) {
		dev_info(&client->dev, "upgrade finished\n");
		devm_kfree(&client->dev, verify_data);
		zt7538_power_control(info, POWER_OFF);
		zt7538_power_control(info, POWER_ON_SEQUENCE);
		return true;
	}

fail_upgrade:
	zt7538_power_control(info, POWER_OFF);
	zt7538_power_control(info, POWER_ON);

	devm_kfree(&client->dev, verify_data);

	dev_info(&client->dev, "Failed to upgrade\n");

	return false;
}

static bool ts_hw_calibration(struct zt7538_ts_info *info)
{
	struct i2c_client *client = info->client;
	u16 chip_eeprom_info;
	int time_out = 0;

	dev_info(&client->dev, "%s +++\n", __func__);

	if (write_reg(client, ZT7538_TOUCH_MODE, 0x07) != I2C_SUCCESS)
		return false;
	zinitix_delay(10);
	write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
	zinitix_delay(10);
	write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
	zinitix_delay(50);
	write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
	zinitix_delay(10);

	if (write_cmd(client, ZT7538_CALIBRATE_CMD) != I2C_SUCCESS)
		return false;

	if (write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD) != I2C_SUCCESS)
		return false;

	zinitix_delay(10);
	write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);

	/* wait for h/w calibration*/
	do {
		zinitix_delay(500);
		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);

		if (read_data(client, ZT7538_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
			return false;

		dev_dbg(&client->dev, "touch eeprom info = 0x%04X\n", chip_eeprom_info);
		if (!zinitix_bit_test(chip_eeprom_info, 0))
			break;

		if (time_out++ == 4) {
			write_cmd(client, ZT7538_CALIBRATE_CMD);
			zinitix_delay(10);
			write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
			dev_err(&client->dev, "h/w calibration retry timeout.\n");
		}

		if (time_out++ > 10) {
			dev_err(&client->dev, "h/w calibration timeout.\n");
			break;
		}

	} while (true);

	if (write_reg(client, ZT7538_TOUCH_MODE, TOUCH_POINT_MODE) != I2C_SUCCESS)
		return false;

	if (info->cap_info.ic_int_mask) {
		if (write_reg(client, ZT7538_INT_ENABLE_FLAG,
					info->cap_info.ic_int_mask) != I2C_SUCCESS)
			return false;
	}

	usleep_range(100, 100);
	if (write_cmd(client, ZT7538_SAVE_CALIBRATION_CMD) != I2C_SUCCESS)
		return false;

	zinitix_delay(1000);

	dev_info(&client->dev, "%s ---\n", __func__);
	return true;
}

static bool init_touch(struct zt7538_ts_info *info, bool fw_skip)
{
	struct zt7538_ts_dt_data *pdata = info->pdata;
	struct i2c_client *client = info->client;
	struct capa_info *cap = &(info->cap_info);
	u16 reg_val;
	int i;
	u16 chip_eeprom_info;
	u16 chip_check_sum;
	bool checksum_err;
	int retry_cnt = 0;
	const struct firmware *tsp_fw = NULL;
	char fw_path[ZINITIX_MAX_FW_PATH];
	int ret;

	if (!info->fw_data) {
		snprintf(fw_path, ZINITIX_MAX_FW_PATH, "%s%s", ZINITIX_FW_PATH, info->pdata->fw_name);
		ret = request_firmware(&tsp_fw, fw_path, &info->client->dev);
		if (ret) {
			dev_err(&info->client->dev, "%s: failed to request_firmware %s\n",
						__func__, fw_path);
			return false;
		} else {
			info->fw_data = (unsigned char *)tsp_fw->data;
		}
	}

	info->ref_scale_factor = TSP_INIT_TEST_RATIO;
retry_init:
	for (i = 0; i < INIT_RETRY_CNT; i++) {
		if (read_data(client, ZT7538_EEPROM_INFO_REG,
						(u8 *)&chip_eeprom_info, 2) < 0) {
			dev_err(&client->dev, "Failed to read eeprom info(%d)\n", i);
			zinitix_delay(10);
			continue;
		} else
			break;
	}

	if (i == INIT_RETRY_CNT)
		goto fail_init;

	checksum_err = false;
	for (i = 0; i < INIT_RETRY_CNT; i++) {
		if (read_data(client, ZT7538_CHECKSUM_RESULT,
						(u8 *)&chip_check_sum, 2) < 0) {
			zinitix_delay(10);
			continue;
		}

		if (chip_check_sum != 0x55aa)
			checksum_err = true;
		break;
	}

	if (i == INIT_RETRY_CNT || checksum_err) {
		dev_err(&client->dev, "Failed to check firmware data\n");
		if (checksum_err && retry_cnt < INIT_RETRY_CNT)
			retry_cnt = INIT_RETRY_CNT;
		goto fail_init;
	}

	if (write_cmd(client, ZT7538_SWRESET_CMD) != I2C_SUCCESS) {
		dev_err(&client->dev, "Failed to write reset command\n");
		goto fail_init;
	}

	cap->button_num = SUPPORTED_BUTTON_NUM;

	reg_val = 0;
	zinitix_bit_set(reg_val, BIT_PT_CNT_CHANGE);
	zinitix_bit_set(reg_val, BIT_DOWN);
	zinitix_bit_set(reg_val, BIT_MOVE);
	zinitix_bit_set(reg_val, BIT_UP);
#ifdef DEF_OPTIONAL_STATE_CHECK
	zinitix_bit_set(reg_val, BIT_DEBUG);
#endif
	if (cap->button_num > 0)
		zinitix_bit_set(reg_val, BIT_ICON_EVENT);

#ifdef SUPPORTED_PALM_TOUCH
	zinitix_bit_set(reg_val, BIT_PALM);
	zinitix_bit_set(reg_val, BIT_PALM_REJECT);
#endif

	cap->ic_int_mask = reg_val;

	if (write_reg(client, ZT7538_INT_ENABLE_FLAG, 0x0) != I2C_SUCCESS)
		goto fail_init;

	dev_dbg(&client->dev, "%s: Send reset command\n", __func__);
	if (write_cmd(client, ZT7538_SWRESET_CMD) != I2C_SUCCESS)
		goto fail_init;

	/* get chip information */
	if (read_data(client, ZT7538_VENDOR_ID, (u8 *)&cap->vendor_id, 2) < 0) {
		dev_err(&client->dev, "failed to read vendor id\n");
		goto fail_init;
	}

	if (read_data(client, ZT7538_CHIP_REVISION, (u8 *)&cap->ic_revision, 2) < 0) {
		dev_err(&client->dev, "failed to read chip revision\n");
		goto fail_init;
	}

	if (info->chip_code == ZT7538_IC_CHIP_CODE) {
		cap->ic_fw_size = 44 * 1024;
	} else {
		dev_err(&client->dev, "%s: Unknown IC!! Cannot set ic_fw_size!!\n", __func__);
		goto fail_init;
	}

	if (read_data(client, ZT7538_HW_ID, (u8 *)&cap->hw_id, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_THRESHOLD, (u8 *)&cap->threshold, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_BUTTON_SENSITIVITY, (u8 *)&cap->key_threshold, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DUMMY_BUTTON_SENSITIVITY, (u8 *)&cap->dummy_threshold, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_TOTAL_NUMBER_OF_X, (u8 *)&cap->x_node_num, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_TOTAL_NUMBER_OF_Y, (u8 *)&cap->y_node_num, 2) < 0)
		goto fail_init;

	cap->total_node_num = cap->x_node_num * cap->y_node_num;

	if (read_data(client, ZT7538_DND_CP_CTRL_L, (u8 *)&cap->cp_ctrl_l, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DND_V_FORCE, (u8 *)&cap->v_force, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DND_AMP_V_SEL, (u8 *)&cap->amp_v_sel, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DND_N_COUNT, (u8 *)&cap->N_cnt, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DND_U_COUNT, (u8 *)&cap->u_cnt, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_AFE_FREQUENCY, (u8 *)&cap->afe_frequency, 2) < 0)
		goto fail_init;

	/* get chip firmware version */
	if (read_data(client, ZT7538_FIRMWARE_VERSION, (u8 *)&cap->fw_version, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_MINOR_FW_VERSION, (u8 *)&cap->fw_minor_version, 2) < 0)
		goto fail_init;

	if (read_data(client, ZT7538_DATA_VERSION_REG, (u8 *)&cap->reg_data_version, 2) < 0)
		goto fail_init;

	if (!fw_skip && ts_check_need_upgrade(info, cap->fw_version,
			cap->fw_minor_version, cap->reg_data_version, cap->hw_id)) {

		dev_info(&client->dev, "%s: start upgrade firmware\n", __func__);

		if (!ts_upgrade_firmware(info, info->fw_data, cap->ic_fw_size))
			goto fail_init;

		if (read_data(client, ZT7538_CHECKSUM_RESULT, (u8 *)&chip_check_sum, 2) < 0)
			goto fail_init;

		if (chip_check_sum != 0x55aa)
			goto fail_init;

		if (!ts_hw_calibration(info))
			goto fail_init;

		/* disable chip interrupt */
		if (write_reg(client, ZT7538_INT_ENABLE_FLAG, 0) != I2C_SUCCESS)
			goto fail_init;

		/* get chip firmware version */
		if (read_data(client, ZT7538_FIRMWARE_VERSION, (u8 *)&cap->fw_version, 2) < 0)
			goto fail_init;

		if (read_data(client, ZT7538_MINOR_FW_VERSION, (u8 *)&cap->fw_minor_version, 2) < 0)
			goto fail_init;

		if (read_data(client, ZT7538_DATA_VERSION_REG, (u8 *)&cap->reg_data_version, 2) < 0)
			goto fail_init;
	}

	if (read_data(client, ZT7538_EEPROM_INFO_REG, (u8 *)&chip_eeprom_info, 2) < 0)
		goto fail_init;

	if (zinitix_bit_test(chip_eeprom_info, 0)) {
		if (!ts_hw_calibration(info))
			goto fail_init;

		/* disable chip interrupt */
		if (write_reg(client, ZT7538_INT_ENABLE_FLAG, 0) != I2C_SUCCESS)
			goto fail_init;
	}

	if (write_reg(client, ZT75XX_RESOLUTION_EXPANDER, 4))  //resolution * 4
		goto fail_init;

	cap->MinX = (u32)0;
	cap->MinY = (u32)0;
	cap->MaxX = (u32)pdata->x_resolution;
	cap->MaxY = (u32)pdata->y_resolution;

	if (write_reg(client, ZT7538_BUTTON_SUPPORTED_NUM, (u16)cap->button_num) != I2C_SUCCESS)
		goto fail_init;

	if (write_reg(client, ZT7538_SUPPORTED_FINGER_NUM, (u16)MAX_SUPPORTED_FINGER_NUM) != I2C_SUCCESS)
		goto fail_init;

	cap->multi_fingers = MAX_SUPPORTED_FINGER_NUM;
	cap->gesture_support = 0;

	if (write_reg(client, ZT7538_INITIAL_TOUCH_MODE, TOUCH_POINT_MODE) != I2C_SUCCESS)
		goto fail_init;

	if (write_reg(client, ZT7538_TOUCH_MODE, info->touch_mode) != I2C_SUCCESS)
		goto fail_init;

#ifdef ZINITIX_I2C_CHECKSUM
	if (read_data(client, ZINITIX_INTERNAL_FLAG_02, (u8 *)&reg_val, 2) < 0)
			goto fail_init;
	cap->i2s_checksum = !(!zinitix_bit_test(reg_val, 15));
	dev_dbg(&client->dev, "use i2s checksum = %d\n", cap->i2s_checksum);
#endif

	zt7538_set_ta_status(info);
	zt7538_set_optional_mode(info, true);

	if (write_reg(client, ZT7538_INT_ENABLE_FLAG, cap->ic_int_mask) != I2C_SUCCESS)
		goto fail_init;

	/* read garbage data */
	for (i = 0; i < 10; i++) {
		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
		usleep_range(10, 10);
	}

	if (info->touch_mode != TOUCH_POINT_MODE) { /* Test Mode */
		if (write_reg(client, ZT7538_DELAY_RAW_FOR_HOST,
					RAWDATA_DELAY_FOR_HOST) != I2C_SUCCESS) {
			dev_err(&client->dev, "%s: Failed to set DELAY_RAW_FOR_HOST\n", __func__);
			goto fail_init;
		}
	}
#if ESD_TIMER_INTERVAL
	if (write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL,
				SCAN_RATE_HZ * ESD_TIMER_INTERVAL) != I2C_SUCCESS)
		goto fail_init;

	read_data(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL, (u8 *)&reg_val, 2);
#endif
	if (info->fw_data) {
		release_firmware(tsp_fw);
		info->fw_data = NULL;
	}

	dev_info(&client->dev, "%s: initialize done!\n", __func__);

	return true;

fail_init:
	printk("zt7538, fail_init\n");

	if (cal_mode) {
		dev_err(&client->dev, "didn't update TSP F/W!! in CAL MODE\n");
		goto retry_fail_init;
	}
	if (++retry_cnt <= INIT_RETRY_CNT) {
		zt7538_power_control(info, POWER_OFF);
		zt7538_power_control(info, POWER_ON_SEQUENCE);
		goto retry_init;

	} else if (retry_cnt == INIT_RETRY_CNT + 1) {
		if (info->chip_code != ZT7538_IC_CHIP_CODE) {
			dev_err(&client->dev, "%s: Unknown IC!!\n", __func__);
			goto retry_fail_init;
		}
		if (!ts_upgrade_firmware(info, info->fw_data, cap->ic_fw_size)) {
			dev_err(&client->dev, "firmware upgrade fail!\n");
			goto retry_fail_init;
		}
		zinitix_delay(100);

		/* hw calibration and make checksum */
		if (!ts_hw_calibration(info)) {
			dev_err(&client->dev, "failed to initiallize\n");
			goto retry_fail_init;
		}
		goto retry_init;
	}

retry_fail_init:
	if (info->fw_data) {
		release_firmware(tsp_fw);
		info->fw_data = NULL;
	}

	return false;
}

static bool mini_init_touch(struct zt7538_ts_info *info)
{
	struct i2c_client *client = info->client;
	int i;
	u16 chip_check_sum;
	if (read_data(client, ZT7538_CHECKSUM_RESULT, (u8 *)&chip_check_sum, 2) < 0)
		goto fail_mini_init;

	if (chip_check_sum != 0x55aa) {
		dev_err(&client->dev, "Failed to check firmware\n");
		goto fail_mini_init;
	}

	info->ref_scale_factor = TSP_INIT_TEST_RATIO;

	if (write_cmd(client, ZT7538_SWRESET_CMD) != I2C_SUCCESS) {
		dev_info(&client->dev, "Failed to write reset command\n");
		goto fail_mini_init;
	}

	if (write_reg(client, ZT75XX_RESOLUTION_EXPANDER, 4)) //resolution * 4
		goto fail_mini_init;

	if (write_reg(client, ZT7538_BUTTON_SUPPORTED_NUM, (u16)info->cap_info.button_num) != I2C_SUCCESS)
		goto fail_mini_init;

	if (write_reg(client, ZT7538_SUPPORTED_FINGER_NUM, (u16)MAX_SUPPORTED_FINGER_NUM) != I2C_SUCCESS)
		goto fail_mini_init;

	if (write_reg(client, ZT7538_INITIAL_TOUCH_MODE, TOUCH_POINT_MODE) != I2C_SUCCESS)
		goto fail_mini_init;

	if (write_reg(client, ZT7538_TOUCH_MODE, info->touch_mode) != I2C_SUCCESS)
		goto fail_mini_init;

	zt7538_set_ta_status(info);
	zt7538_set_optional_mode(info, true);

	/* soft calibration */
	if (write_cmd(client, ZT7538_CALIBRATE_CMD) != I2C_SUCCESS)
		goto fail_mini_init;

	if (write_reg(client, ZT7538_INT_ENABLE_FLAG, info->cap_info.ic_int_mask) != I2C_SUCCESS)
		goto fail_mini_init;

	/* read garbage data */
	for (i = 0; i < 10; i++) {
		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
		usleep_range(10, 10);
	}

	if (info->touch_mode != TOUCH_POINT_MODE) {
		if (write_reg(client, ZT7538_DELAY_RAW_FOR_HOST,
					RAWDATA_DELAY_FOR_HOST) != I2C_SUCCESS){
			dev_err(&client->dev, "Failed to set ZT7538_DELAY_RAW_FOR_HOST\n");
			goto fail_mini_init;
		}
	}

#if ESD_TIMER_INTERVAL
	if (write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL,
			SCAN_RATE_HZ * ESD_TIMER_INTERVAL) != I2C_SUCCESS)
		goto fail_mini_init;

	esd_timer_start(CHECK_ESD_TIMER, info);
#endif

	dev_info(&client->dev, "Successfully mini initialized\r\n");
	return true;

fail_mini_init:
	dev_err(&client->dev, "Failed to initialize mini init\n");
	zt7538_power_control(info, POWER_OFF);
	zt7538_power_control(info, POWER_ON_SEQUENCE);

	if (!init_touch(info, false)) {
		dev_err(&client->dev, "Failed to initialize\n");
		return false;
	}

#if ESD_TIMER_INTERVAL
	esd_timer_start(CHECK_ESD_TIMER, info);
#endif
	return true;
}

static void clear_report_data(struct zt7538_ts_info *info)
{
	int i;
	bool reported = false;
	u8 sub_status;

	for (i = 0; i < info->cap_info.button_num; i++) {
		if (info->button[i] == ICON_BUTTON_DOWN) {
			info->button[i] = ICON_BUTTON_UP;
			input_report_key(info->input_dev, BUTTON_MAPPING_KEY[i], 0);
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			dev_info(&info->client->dev, "key %d up\n", BUTTON_MAPPING_KEY[i]);
#else
			dev_info(&info->client->dev, "key up\n");
#endif
			reported = true;
		}
	}

	for (i = 0; i < info->cap_info.multi_fingers; i++) {
		sub_status = info->reported_touch_info.coord[i].sub_status;
		if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
			dev_info(&info->client->dev, "[%d][R] M[%d] Ver[%02x] Mode[%02x]\n",
					i, info->move_cnt[i],
					info->cap_info.reg_data_version, m_optional_mode);
			info->move_cnt[i] = 0;
			input_mt_slot(info->input_dev, i);
#ifdef REPORT_2D_Z
			input_report_abs(info->input_dev, ABS_MT_PRESSURE, 0);
#endif
			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, 0);
			reported = true;
		}
		info->reported_touch_info.coord[i].sub_status = 0;
		info->finger_cnt = 0;
	}

	if (reported)
		input_sync(info->input_dev);

	input_mt_slot(info->input_dev, 0);
	input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, 0);

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, -1);
#endif
}

static irqreturn_t zt7538_touch_work(int irq, void *data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)data;
	struct i2c_client *client = info->client;
	int i = 0;
	u8 sub_status;
	u8 prev_sub_status;
	u32 x, y, maxX, maxY;
	u32 w, minor_w;
#ifdef REPORT_2D_Z
	u16 z = 0;
	int ret = 0;
#endif
	u8 palm = 0;

#ifdef DEF_OPTIONAL_STATE_CHECK
	m_debug_register = 0;
#endif

	if (gpio_get_value(info->pdata->gpio_int)) {
		dev_err(&client->dev, "Invalid interrupt\n");
		return IRQ_HANDLED;
	}

	if (down_trylock(&info->work_lock)) {
		dev_err(&client->dev, "%s: Failed to occupy work lock\n", __func__);
		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
		return IRQ_HANDLED;
	}
#if ESD_TIMER_INTERVAL
	esd_timer_stop(info);
#endif

	if (info->work_state != NOTHING) {
		dev_err(&client->dev, "%s: Other process occupied\n", __func__);
		usleep_range(DELAY_FOR_SIGNAL_DELAY, DELAY_FOR_SIGNAL_DELAY);
		if (!gpio_get_value(info->pdata->gpio_int)) {
			write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
			usleep_range(DELAY_FOR_SIGNAL_DELAY, DELAY_FOR_SIGNAL_DELAY);
		}
		goto out;
	}
	info->work_state = NORMAL;

	if (!ts_read_coord(info) || info->touch_info.status == 0xffff 
		|| info->touch_info.status == 0x1) {
		dev_err(&client->dev, "Failed to read info coord\n");
		zt7538_power_control(info, POWER_OFF);
		zt7538_power_control(info, POWER_ON_SEQUENCE);
		clear_report_data(info);
		mini_init_touch(info);
		goto out;
	}

	/* invalid : maybe periodical repeated int. */
	if (info->touch_info.status == 0x0)
		goto out;

#ifdef SUPPORTED_TOUCH_KEY
	if (zinitix_bit_test(info->touch_info.status, BIT_ICON_EVENT)) {
		if (read_data(info->client, ZT7538_ICON_STATUS_REG,
			(u8 *)(&info->icon_event_reg), 2) < 0) {
			dev_err(&client->dev, "Failed to read button info\n");
			write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
			goto out;
		}

		for (i = 0; i < info->cap_info.button_num; i++) {
			if (zinitix_bit_test(info->icon_event_reg, (BIT_O_ICON0_DOWN + i))) {
				input_report_key(info->input_dev, BUTTON_MAPPING_KEY[i], 1);
				if (info->button[i] != ICON_BUTTON_DOWN)
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
					dev_info(&client->dev, "key %d down\n", BUTTON_MAPPING_KEY[i]);
#else
					dev_info(&client->dev, "key down\n");
#endif
				info->button[i] = ICON_BUTTON_DOWN;
			}
		}

		for (i = 0; i < info->cap_info.button_num; i++) {
			if (zinitix_bit_test(info->icon_event_reg, (BIT_O_ICON0_UP + i))) {
				info->button[i] = ICON_BUTTON_UP;
				input_report_key(info->input_dev, BUTTON_MAPPING_KEY[i], 0);
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
				dev_info(&client->dev, "key %d up\n", BUTTON_MAPPING_KEY[i]);
#else
				dev_info(&client->dev, "key up\n");
#endif
			}
		}
	}
#endif

#ifdef SUPPORTED_PALM_TOUCH
		if(zinitix_bit_test(info->touch_info.status, BIT_PALM)){
				dev_dbg(&client->dev, "palm report\n");
			palm = 1;
		}
	
		if(zinitix_bit_test(info->touch_info.status, BIT_PALM_REJECT)){
			dev_dbg(&client->dev, "palm reject\n");
			palm = 2;
		}
#endif

	for (i = 0; i < info->cap_info.multi_fingers; i++) {
		sub_status = info->touch_info.coord[i].sub_status;
		prev_sub_status = info->reported_touch_info.coord[i].sub_status;

		if (zinitix_bit_test(sub_status, SUB_BIT_EXIST)) {
			x = info->touch_info.coord[i].x;
			y = info->touch_info.coord[i].y;
			w = info->touch_info.coord[i].width;
			maxX = info->cap_info.MaxX;
			maxY = info->cap_info.MaxY;

			if (x > maxX || y > maxY) {
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
					dev_err(&client->dev,
								"Invalid coord %d : x=%d, y=%d\n", i, x, y);
#endif
					continue;
			}

			info->touch_info.coord[i].x = x;
			info->touch_info.coord[i].y = y;
			if (zinitix_bit_test(sub_status, SUB_BIT_DOWN)) {
				info->finger_cnt++;
			} else if (zinitix_bit_test(sub_status, SUB_BIT_MOVE)) {
				info->move_cnt[i]++;
			}
			if (w == 0)
				w = 1;

			input_mt_slot(info->input_dev, i);
			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, 1);

#ifdef SUPPORTED_PALM_TOUCH
			if (palm == 0) {
				if (w >= PALM_REPORT_WIDTH)
					w = PALM_REPORT_WIDTH - 10;
				minor_w = w;
			}

			else if (palm == 1) {	//palm report
				w = PALM_REPORT_WIDTH;
				minor_w = PALM_REPORT_WIDTH/3;
			} else if(palm == 2){	// palm reject
				w = PALM_REJECT_WIDTH;
				minor_w = PALM_REJECT_WIDTH;
			}

			input_report_abs(info->input_dev,
				ABS_MT_TOUCH_MINOR, (u32)minor_w);
			input_report_abs(info->input_dev, ABS_MT_PALM, (palm > 0) ? 1:0);
#endif

			input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR, (u32)w);
#ifdef REPORT_2D_Z
			ret = read_data(client, ZT7538_REAL_WIDTH + i, (u8*)&z, 2);
			if (ret < 0)
				dev_info(&client->dev, ": Failed to read %d's Real width %s\n", i, __func__);
			if (z < 1) z = 1;
			input_report_abs(info->input_dev, ABS_MT_PRESSURE, (u32)z);
#endif
			input_report_abs(info->input_dev, ABS_MT_WIDTH_MAJOR,
								(u32)((palm == 1) ? (w - 40) : w));
			input_report_abs(info->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(info->input_dev, ABS_MT_POSITION_Y, y);
		} else if (zinitix_bit_test(sub_status, SUB_BIT_UP) ||
			zinitix_bit_test(prev_sub_status, SUB_BIT_EXIST)) {
			info->finger_cnt--;
			memset(&info->touch_info.coord[i], 0x0, sizeof(struct coord));
			input_mt_slot(info->input_dev, i);
			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, 0);

		} else
			memset(&info->touch_info.coord[i], 0x0, sizeof(struct coord));

		input_sync(info->input_dev);

		if (zinitix_bit_test(sub_status, SUB_BIT_EXIST) && zinitix_bit_test(sub_status, SUB_BIT_DOWN)) {
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			dev_info(&client->dev, "[%d][P] x = %d, y = %d,"
					" w = %d, p = %d\n",i, x, y, w, palm);
#else
			dev_info(&client->dev,
					"[%d][P] w = %d, p = %d\n", i, w, palm);
#endif
		} else if ( (!zinitix_bit_test(sub_status, SUB_BIT_EXIST)) && (zinitix_bit_test(sub_status, SUB_BIT_UP) ||
			zinitix_bit_test(prev_sub_status, SUB_BIT_EXIST)) ) {
			dev_info(&client->dev, "[%d][R] M[%d] Ver[%02x] Mode[%02x]\n",
					i, info->move_cnt[i],
					info->cap_info.reg_data_version, m_optional_mode);
			info->move_cnt[i] = 0;
		}
	}
	memcpy((char *)&info->reported_touch_info, (char *)&info->touch_info,
							sizeof(struct point_info));
out:
	zt7538_set_optional_mode(info, false);
#ifdef DEF_OPTIONAL_STATE_CHECK
	if(m_debug_register) {
		dev_info(&client->dev, "debug_register = [0x%04x]\n", m_debug_register);
	}
#endif
#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, info->finger_cnt > 0 ? 1 : 0);
#endif

	if (info->work_state == NORMAL) {
#if ESD_TIMER_INTERVAL
		esd_timer_start(CHECK_ESD_TIMER, info);
#endif
		info->work_state = NOTHING;
	}

	up(&info->work_lock);

	return IRQ_HANDLED;
}

#if defined(CONFIG_PM)
static int zt7538_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct zt7538_ts_info *info = i2c_get_clientdata(client);

	if(info->device_enabled) {
		dev_err(&client->dev, "%s: already enabled\n", __func__);
		return 0;
	}
	down(&info->work_lock);
	if (info->work_state != SUSPEND) {
		dev_err(&client->dev, "%s: Invalid work proceedure\n", __func__);
		up(&info->work_lock);
		return 0;
	}
	zt7538_pinctrl_configure(info, 1);
	zt7538_power_control(info, POWER_ON_SEQUENCE);
	info->device_enabled = 1;

	info->work_state = NOTHING;
	if (!mini_init_touch(info))
		dev_err(&client->dev, "Failed to resume\n");
	enable_irq(info->irq);

	up(&info->work_lock);

	return 0;
}

static int zt7538_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct zt7538_ts_info *info = i2c_get_clientdata(client);

	if (!info->device_enabled) {
		dev_err(&client->dev, "%s: already disabled\n", __func__);
		return 0;
	}
	info->device_enabled = 0;

	disable_irq(info->irq);
#if ESD_TIMER_INTERVAL
	flush_work(&info->tmr_work);
#endif

	down(&info->work_lock);
	if (info->work_state != NOTHING && info->work_state != SUSPEND) {
		dev_err(&client->dev, "%s: Invalid work proceedure\n", __func__);
		up(&info->work_lock);
		enable_irq(info->irq);
		return 0;
	}

	clear_report_data(info);
#if ESD_TIMER_INTERVAL
	esd_timer_stop(info);
#endif

	write_cmd(info->client, ZT7538_SLEEP_CMD);
	info->work_state = SUSPEND;
	zt7538_power_control(info, POWER_OFF);
	zt7538_pinctrl_configure(info, 0);

	up(&info->work_lock);

	return 0;
}
#endif

static int zt7538_input_open(struct input_dev *dev)
{
	struct zt7538_ts_info *info;

	info = input_get_drvdata(dev);
	dev_info(&info->client->dev, "%s\n", __func__);
	return zt7538_ts_resume(&info->client->dev);
}
static void zt7538_input_close(struct input_dev *dev)
{
	struct zt7538_ts_info *info;

	info = input_get_drvdata(dev);
	dev_info(&info->client->dev, "%s\n", __func__);
	zt7538_ts_suspend(&info->client->dev);
}

/* For DND*/
static bool ts_set_touchmode(u16 value)
{
	int i;

	disable_irq(misc_info->irq);

	down(&misc_info->work_lock);
	if (misc_info->work_state != NOTHING) {
		dev_err(&misc_info->client->dev, "other process occupied.\n");
		enable_irq(misc_info->irq);
		up(&misc_info->work_lock);
		return -1;
	}

	write_cmd(misc_info->client, ZT7538_WAKEUP_CMD);
	zinitix_delay(50);
	if (misc_info->touch_mode == TOUCH_POINT_MODE) {
		/* factory data */
		read_data(misc_info->client, ZT7538_MUTUAL_AMP_V_SEL,
				(u8 *)&misc_info->cap_info.mutual_amp_v_sel, 2);
		read_data(misc_info->client, ZT7538_AFE_FREQUENCY,
				(u8 *)&misc_info->cap_info.afe_frequency, 2);
		read_data(misc_info->client, ZT7538_DND_SHIFT_VALUE,
				(u8 *)&misc_info->cap_info.shift_value, 2);
	}
	misc_info->work_state = SET_MODE;

	if (value == TOUCH_DND_MODE) {
		if (write_reg(misc_info->client, ZT7538_DND_N_COUNT,
				SEC_DND_N_COUNT)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_DND_N_COUNT %d.\n", SEC_DND_N_COUNT);
		if (write_reg(misc_info->client, ZT7538_DND_U_COUNT,
				SEC_DND_U_COUNT)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_DND_U_COUNT %d.\n", SEC_DND_U_COUNT);
		if (write_reg(misc_info->client, ZT7538_AFE_FREQUENCY,
				SEC_DND_FREQUENCY)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_AFE_FREQUENCY %d.\n", SEC_DND_FREQUENCY);
	} else if (misc_info->touch_mode == TOUCH_DND_MODE) {
		if (write_reg(misc_info->client, ZT7538_MUTUAL_AMP_V_SEL,
				misc_info->cap_info.mutual_amp_v_sel) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_MUTUAL_AMP_V_SEL %d.\n",
					misc_info->cap_info.mutual_amp_v_sel);

		if (write_reg(misc_info->client, ZT7538_DND_SHIFT_VALUE,
				misc_info->cap_info.shift_value) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_DND_SHIFT_VALUE %d.\n",
					misc_info->cap_info.shift_value);

		if (write_reg(misc_info->client, ZT7538_AFE_FREQUENCY,
				misc_info->cap_info.afe_frequency) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_AFE_FREQUENCY %d.\n",
					misc_info->cap_info.afe_frequency);
	}

	if (value == TOUCH_SEC_MODE)
		misc_info->touch_mode = TOUCH_POINT_MODE;
	else
		misc_info->touch_mode = value;

	dev_info(&misc_info->client->dev, "[zinitix_touch] tsp_set_testmode, "
			"touchkey_testmode = %d\r\n", misc_info->touch_mode);

	if (misc_info->touch_mode != TOUCH_POINT_MODE) {
		if (write_reg(misc_info->client, ZT7538_DELAY_RAW_FOR_HOST,
				RAWDATA_DELAY_FOR_HOST) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev,
					"Fail to set ZT7538_DELAY_RAW_FOR_HOST.\r\n");
	}

	if (write_reg(misc_info->client, ZT7538_TOUCH_MODE,
			misc_info->touch_mode) != I2C_SUCCESS)
		dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
				"Fail to set ZINITX_TOUCH_MODE %d.\r\n",
				misc_info->touch_mode);

	if (write_cmd(misc_info->client, ZT7538_SWRESET_CMD) != I2C_SUCCESS) {
		dev_info(&misc_info->client->dev, "Failed to write reset command\n");
	}

	/* clear garbage data */
	for (i = 0; i < 10; i++) {
		zinitix_delay(20);
		write_cmd(misc_info->client, ZT7538_CLEAR_INT_STATUS_CMD);
	}

	misc_info->work_state = NOTHING;
	enable_irq(misc_info->irq);
	up(&misc_info->work_lock);
	return 1;
}

/* For HFDND */
static bool ts_set_touchmode2(u16 value)
{
	int i;

	disable_irq(misc_info->irq);

	down(&misc_info->work_lock);
	if (misc_info->work_state != NOTHING) {
		dev_info(&misc_info->client->dev, "other process occupied.. (%d)\n",
			misc_info->work_state);
		enable_irq(misc_info->irq);
		up(&misc_info->work_lock);
		return -1;
	}

	write_cmd(misc_info->client, ZT7538_WAKEUP_CMD);
	zinitix_delay(50);
	if (misc_info->touch_mode == TOUCH_POINT_MODE) {
		/* factory data */
		read_data(misc_info->client, ZT7538_MUTUAL_AMP_V_SEL,
				(u8 *)&misc_info->cap_info.mutual_amp_v_sel, 2);
		read_data(misc_info->client, ZT7538_AFE_FREQUENCY,
				(u8 *)&misc_info->cap_info.afe_frequency, 2);
		read_data(misc_info->client, ZT7538_DND_SHIFT_VALUE,
				(u8 *)&misc_info->cap_info.shift_value, 2);
	}
	misc_info->work_state = SET_MODE;

	if(value == TOUCH_DND_MODE) {
		if (write_reg(misc_info->client, ZT7538_DND_N_COUNT,
				SEC_HFDND_N_COUNT)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_HFDND_N_COUNT %d.\n", SEC_HFDND_N_COUNT);
		if (write_reg(misc_info->client, ZT7538_DND_U_COUNT,
				SEC_HFDND_U_COUNT)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_HFDND_U_COUNT %d.\n", SEC_HFDND_U_COUNT);
		if (write_reg(misc_info->client, ZT7538_AFE_FREQUENCY,
				SEC_HFDND_FREQUENCY)!=I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to set ZT7538_AFE_FREQUENCY %d.\n", SEC_HFDND_FREQUENCY);
	} else if (misc_info->touch_mode == TOUCH_DND_MODE) {
		if (write_reg(misc_info->client, ZT7538_MUTUAL_AMP_V_SEL,
				misc_info->cap_info.mutual_amp_v_sel) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_MUTUAL_AMP_V_SEL %d.\n",
					misc_info->cap_info.mutual_amp_v_sel);

		if (write_reg(misc_info->client, ZT7538_DND_SHIFT_VALUE,
				misc_info->cap_info.shift_value) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_DND_SHIFT_VALUE %d.\n",
					misc_info->cap_info.shift_value);

		if (write_reg(misc_info->client, ZT7538_AFE_FREQUENCY,
				misc_info->cap_info.afe_frequency) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
					"Fail to reset ZT7538_AFE_FREQUENCY %d.\n",
					misc_info->cap_info.afe_frequency);
	}
	if (value == TOUCH_SEC_MODE)
		misc_info->touch_mode = TOUCH_POINT_MODE;
	else
		misc_info->touch_mode = value;

	dev_info(&misc_info->client->dev, "[zinitix_touch] tsp_set_testmode, "
			"touchkey_testmode = %d\r\n", misc_info->touch_mode);

	if (misc_info->touch_mode != TOUCH_POINT_MODE) {
		if (write_reg(misc_info->client, ZT7538_DELAY_RAW_FOR_HOST,
				RAWDATA_DELAY_FOR_HOST) != I2C_SUCCESS)
			dev_info(&misc_info->client->dev,
					"Fail to set ZT7538_DELAY_RAW_FOR_HOST.\r\n");
	}

	if (write_reg(misc_info->client, ZT7538_TOUCH_MODE,
			misc_info->touch_mode) != I2C_SUCCESS)
		dev_info(&misc_info->client->dev, "[zinitix_touch] TEST Mode : "
				"Fail to set ZINITX_TOUCH_MODE %d.\r\n", misc_info->touch_mode);

	if (write_cmd(misc_info->client, ZT7538_SWRESET_CMD) != I2C_SUCCESS) {
		dev_info(&misc_info->client->dev, "Failed to write reset command\n");
	}

	/* clear garbage data */
	for (i = 0; i < 10; i++) {
		zinitix_delay(20);
		write_cmd(misc_info->client, ZT7538_CLEAR_INT_STATUS_CMD);
	}

	misc_info->work_state = NOTHING;
	enable_irq(misc_info->irq);
	up(&misc_info->work_lock);
	return 1;
}
static int ts_upgrade_sequence(const u8 *firmware_data)
{
	bool ret = true;

	disable_irq(misc_info->irq);
	down(&misc_info->work_lock);
	misc_info->work_state = UPGRADE;

#if ESD_TIMER_INTERVAL
	esd_timer_stop(misc_info);
#endif
	clear_report_data(misc_info);

	dev_info(&misc_info->client->dev, "start upgrade firmware\n");
	if (!ts_upgrade_firmware(misc_info, firmware_data, misc_info->cap_info.ic_fw_size))
		ret = false;

	if (!init_touch(misc_info, true))
		ret = false;

#if ESD_TIMER_INTERVAL
	esd_timer_start(CHECK_ESD_TIMER, misc_info);
#endif
	enable_irq(misc_info->irq);
	misc_info->work_state = NOTHING;
	up(&misc_info->work_lock);

	return (ret) ? 0 : -1;
}

#ifdef SEC_FACTORY_TEST
#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func

#ifdef CONFIG_INPUT_BOOSTER
static void boost_level(void *device_data);
#endif

static struct tsp_cmd tsp_cmds[] = {
	{TSP_CMD("fw_update", fw_update),},
	{TSP_CMD("get_fw_ver_bin", get_fw_ver_bin),},
	{TSP_CMD("get_fw_ver_ic", get_fw_ver_ic),},
	{TSP_CMD("get_threshold", get_threshold),},
	{TSP_CMD("get_chip_vendor", get_chip_vendor),},
	{TSP_CMD("get_chip_name", get_chip_name),},
	{TSP_CMD("get_x_num", get_x_num),},
	{TSP_CMD("get_y_num", get_y_num),},
	{TSP_CMD("not_support_cmd", not_support_cmd),},

	/* vendor dependant command */
	{TSP_CMD("run_dnd_read", run_dnd_read),},
	{TSP_CMD("get_dnd", get_dnd),},
	{TSP_CMD("get_dnd_all_data", get_dnd_all_data),},
	{TSP_CMD("run_dnd_v_gap_read", run_dnd_v_gap_read),},
	{TSP_CMD("get_dnd_v_gap", get_dnd_v_gap),},
	{TSP_CMD("run_dnd_h_gap_read", run_dnd_h_gap_read),},
	{TSP_CMD("get_dnd_h_gap", get_dnd_h_gap),},
	{TSP_CMD("run_hfdnd_read", run_hfdnd_read),},
	{TSP_CMD("get_hfdnd", get_hfdnd),},
	{TSP_CMD("get_hfdnd_all_data", get_hfdnd_all_data),},
	{TSP_CMD("run_hfdnd_v_gap_read", run_hfdnd_v_gap_read),},
	{TSP_CMD("get_hfdnd_v_gap", get_hfdnd_v_gap),},
	{TSP_CMD("run_hfdnd_h_gap_read", run_hfdnd_h_gap_read),},
	{TSP_CMD("get_hfdnd_h_gap", get_hfdnd_h_gap),},
	{TSP_CMD("run_delta_read", run_delta_read),},
	{TSP_CMD("get_delta", get_delta),},
	{TSP_CMD("get_delta_all_data", get_delta_all_data),},
	{TSP_CMD("dead_zone_enable", dead_zone_enable),},
	{TSP_CMD("clear_cover_mode", clear_cover_mode),},
	{TSP_CMD("clear_reference_data", clear_reference_data),},
	{TSP_CMD("run_ref_calibration", run_ref_calibration),},
#ifdef CONFIG_INPUT_BOOSTER
	{TSP_CMD("boost_level", boost_level),},
#endif
	{TSP_CMD("hfdnd_spec_adjust", hfdnd_spec_adjust),},
};

static inline void set_cmd_result(struct tsp_factory_info *finfo, char *buff, int len)
{
	strncat(finfo->cmd_result, buff, len);
}

static inline void set_default_result(struct tsp_factory_info *finfo)
{
	char delim = ':';
	memset(finfo->cmd_result, 0x00, ARRAY_SIZE(finfo->cmd_result));
	memcpy(finfo->cmd_result, finfo->cmd, strlen(finfo->cmd));
	strncat(finfo->cmd_result, &delim, 1);
}

#ifdef CONFIG_INPUT_BOOSTER
static void boost_level(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	int stage;

	dev_info(&client->dev, "%s\n", __func__);

	set_default_result(finfo);

	stage = 1 << finfo->cmd_param[0];
	if (!(info->booster->dvfs_stage & stage)) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "NG");
		finfo->cmd_state = FAIL;
		dev_err(&info->client->dev,"%s: %d is not supported(%04x != %04x).\n",__func__,
			finfo->cmd_param[0], stage, info->booster->dvfs_stage);

		goto boost_out;
	}

	info->booster->dvfs_boost_mode = stage;
	input_booster_set_level_change(finfo->cmd_param[0]);
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "OK");
	finfo->cmd_state = OK;

	if (info->booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		if (info->booster && info->booster->dvfs_set)
			info->booster->dvfs_set(info->booster, -1);
	}

boost_out:
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	mutex_lock(&finfo->cmd_lock);
	finfo->cmd_is_running = false;
	mutex_unlock(&finfo->cmd_lock);

	finfo->cmd_state = WAITING;

	return;
}
#endif

static void fw_update(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	int ret = 0;
	u8 *buff = 0;
	mm_segment_t old_fs = {0};
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	const struct firmware *tsp_fw = NULL;
	char fw_path[ZINITIX_MAX_FW_PATH];

	set_default_result(finfo);
/*
	* 0 : [BUILT_IN] Getting firmware which is for user.
	* 1 : [UMS] Getting firmware from sd card.
	* 2 : [FFU] Getting firmware from air.
*/
	switch (finfo->cmd_param[0]) {
	case BUILT_IN:
		if (!info->fw_data) {
			snprintf(fw_path, ZINITIX_MAX_FW_PATH, "%s%s", ZINITIX_FW_PATH, info->pdata->fw_name);
			ret = request_firmware(&tsp_fw, fw_path, &info->client->dev);
			if (ret) {
				dev_err(&info->client->dev, "%s: failed to request_firmware %s\n",
							__func__, fw_path);
				finfo->cmd_state = FAIL;
				return;
			} else {
				info->fw_data = (unsigned char *)tsp_fw->data;
			}
		}

		ret = ts_upgrade_sequence((u8 *)info->fw_data);
		if (ret < 0) {
			finfo->cmd_state = FAIL;
			return;
		}

		if (info->fw_data) {
			release_firmware(tsp_fw);
			info->fw_data = NULL;
		}
		break;
	case UMS:
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		fp = filp_open(ZINITIX_DEFAULT_UMS_FW, O_RDONLY, S_IRUSR);
		if (IS_ERR(fp)) {
			dev_err(&client->dev, "file(%s) open error:%d\n", ZINITIX_DEFAULT_UMS_FW, (s32)fp);
			finfo->cmd_state = FAIL;
			goto err_open;
		}

		fsize = fp->f_path.dentry->d_inode->i_size;

		if (fsize != info->cap_info.ic_fw_size) {
			dev_err(&client->dev, "invalid fw size!! size:%ld\n", fsize);
			finfo->cmd_state = FAIL;
			goto err_open;
		}

		buff = devm_kzalloc(&client->dev, (size_t)fsize, GFP_KERNEL);
		if (!buff) {
			dev_err(&client->dev, "failed to alloc buffer for fw\n");
			finfo->cmd_state = FAIL;
			goto err_alloc;
		}

		nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
		if (nread != fsize) {
			finfo->cmd_state = FAIL;
			goto err_fw_size;
		}

		filp_close(fp, current->files);
		set_fs(old_fs);
		dev_info(&client->dev, "ums fw is loaded!!\n");

		ret = ts_upgrade_sequence((u8 *)buff);
		if (ret < 0) {
			devm_kfree(&client->dev, buff);
			finfo->cmd_state = FAIL;
			goto update_fail;
		}
		break;

	case FFU:
		snprintf(fw_path, ZINITIX_MAX_FW_PATH, "%s", ZINITIX_DEFAULT_FFU_FW);

		dev_err(&info->client->dev, "%s: Load firmware : %s\n", __func__,
			 fw_path);

		ret = request_firmware(&tsp_fw, fw_path, &info->client->dev);
		if (ret) {
			dev_err(&info->client->dev, "%s: failed to request_firmware %s\n",
						__func__, fw_path);
			finfo->cmd_state = FAIL;
			return;
		} else {
			info->fw_data = (unsigned char *)tsp_fw->data;
		}

		ret = ts_upgrade_sequence((u8 *)info->fw_data);
		if (ret < 0) {
			finfo->cmd_state = FAIL;
			return;
		}

		if (info->fw_data) {
			release_firmware(tsp_fw);
			info->fw_data = NULL;
		}
		break;

	default:
		dev_err(&client->dev, "invalid fw file type!!\n");
		goto update_fail;
	}

	finfo->cmd_state = OK;
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff) , "%s", "OK");
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	devm_kfree(&client->dev, buff);

	return;

err_fw_size:
	devm_kfree(&client->dev, buff);
err_alloc:
	filp_close(fp, NULL);
err_open:
	set_fs(old_fs);
update_fail:
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff) , "%s", "NG");
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}

static void get_fw_ver_bin(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	u16 fw_version, fw_minor_version, reg_version, hw_id, vendor_id;
	u32 version, length;
	const struct firmware *tsp_fw = NULL;
	char fw_path[ZINITIX_MAX_FW_PATH];
	int ret;

	set_default_result(finfo);

	if (!info->fw_data) {
		snprintf(fw_path, ZINITIX_MAX_FW_PATH, "%s%s", ZINITIX_FW_PATH, info->pdata->fw_name);
		ret = request_firmware(&tsp_fw, fw_path, &info->client->dev);
		if (ret) {
			dev_err(&info->client->dev, "%s: failed to request_firmware %s\n",
						__func__, fw_path);
			finfo->cmd_state = FAIL;
			return;
		} else {
			info->fw_data = (unsigned char *)tsp_fw->data;
		}
	}

	fw_version = (u16)(info->fw_data[52] | (info->fw_data[53] << 8));
	fw_minor_version = (u16)(info->fw_data[56] | (info->fw_data[57] << 8));
	reg_version = (u16)(info->fw_data[60] | (info->fw_data[61] << 8));
	hw_id = (u16)(info->fw_data[48] | (info->fw_data[49] << 8));
	version = (u32)((u32)(hw_id & 0xff) << 16) | ((fw_version & 0xf) << 12)
				| ((fw_minor_version & 0xf) << 8) | (reg_version & 0xff);

	length = sizeof(vendor_id);
	snprintf(finfo->cmd_buff, length + 1, "%s", "ZI");
	snprintf(finfo->cmd_buff + length, sizeof(finfo->cmd_buff) - length, "%06X", version);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	if (info->fw_data) {
		release_firmware(tsp_fw);
		info->fw_data = NULL;
	}

	return;
}

static void get_fw_ver_ic(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	u16 fw_version = 0;
	u16 fw_minor_version = 0;
	u16 reg_version = 0;
	u16 hw_id = 0;
	u16 vendor_id = 0;
	u32 version = 0;
	u32 length = 0;

	set_default_result(finfo);

/* Read firmware version from IC */
	down(&info->work_lock);
	write_cmd(client, ZT7538_WAKEUP_CMD);
	zinitix_delay(50);

	if (read_data(client, ZT7538_FIRMWARE_VERSION, (u8 *)&fw_version, 2) < 0)
		dev_err(&client->dev, "Failed to read firmware version\n");
	if (read_data(client, ZT7538_MINOR_FW_VERSION, (u8 *)&fw_minor_version, 2) < 0)
		dev_err(&client->dev, "Failed to read minor version\n");
	if (read_data(client, ZT7538_DATA_VERSION_REG, (u8 *)&reg_version, 2) < 0)
		dev_err(&client->dev, "Failed to read register version\n");
	if (read_data(client, ZT7538_HW_ID, (u8 *)&hw_id, 2) < 0)
		dev_err(&client->dev, "Failed to read HW_ID version");

	up(&info->work_lock);

	vendor_id = ntohs(info->cap_info.vendor_id);
	version = (u32)((u32)(hw_id & 0xff) << 16) | ((fw_version & 0xf) << 12)
		| ((fw_minor_version & 0xf) << 8) | (reg_version & 0xff);

	length = sizeof(vendor_id);
	snprintf(finfo->cmd_buff, length + 1, "%s", (u8 *)&vendor_id);
	snprintf(finfo->cmd_buff + length, sizeof(finfo->cmd_buff) - length, "%06X", version);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_threshold(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff),
				"%d", info->cap_info.threshold);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

#define ZT7538_VENDOR_NAME "ZINITIX"

static void get_chip_vendor(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", ZT7538_VENDOR_NAME);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

#define ZT7538_CHIP_NAME "ZT7538"

static void get_chip_name(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", ZT7538_CHIP_NAME);

	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_x_num(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff),
				"%u", info->cap_info.x_node_num);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_y_num(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff),
				"%u", info->cap_info.y_node_num);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void not_support_cmd(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	sprintf(finfo->cmd_buff, "%s", "NA");
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = NOT_APPLICABLE;

	mutex_lock(&finfo->cmd_lock);
	finfo->cmd_is_running = false;
	mutex_unlock(&finfo->cmd_lock);

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_all_data(struct zt7538_ts_info *info,
			run_func_t run_func, void *data, enum data_type type)
{
	struct tsp_factory_info *finfo = info->factory_info;
	struct i2c_client *client = info->client;
	char *buff;
	int node_num;
	int page_size, len;
	static int index = 0;

	set_default_result(finfo);
	info->get_all_data = true;

	if (!data) {
		dev_err(&client->dev, "%s: data is NULL\n", __func__);
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "FAIL");
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		goto all_data_out;
	}

	if (finfo->cmd_param[0] == 0) {
		run_func(info);
		if (finfo->cmd_state != RUNNING)
			goto all_data_out;
		index = 0;
	} else {
		if (index == 0) {
			dev_info(&client->dev,
				"%s: Please do cmd_param '0' first\n",
				__func__);
			snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
			set_cmd_result(finfo, finfo->cmd_buff,
					strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
			finfo->cmd_state = NOT_APPLICABLE;
			goto all_data_out;
		}
	}

	page_size = TSP_CMD_RESULT_STR_LEN - strlen(finfo->cmd) - 10;
	node_num = info->cap_info.x_node_num * info->cap_info.y_node_num;
	buff = devm_kzalloc(&client->dev, TSP_CMD_RESULT_STR_LEN, GFP_KERNEL);
	if (buff != NULL) {
		char *pBuf = buff;
		for (; index < node_num; index++) {
			switch(type) {
			case DATA_UNSIGNED_CHAR: {
				unsigned char *ddata = data;
				len = snprintf(pBuf, 5, "%u,", ddata[index]);
				break;}
			case DATA_SIGNED_CHAR: {
				char *ddata = data;
				len = snprintf(pBuf, 5, "%d,", ddata[index]);
				break;}
			case DATA_UNSIGNED_SHORT: {
				unsigned short *ddata = data;
				len = snprintf(pBuf, 10, "%u,", ddata[index]);
				break;}
			case DATA_SIGNED_SHORT: {
				short *ddata = data;
				len = snprintf(pBuf, 10, "%d,", ddata[index]);
				break;}
			case DATA_UNSIGNED_INT: {
				unsigned int *ddata = data;
				len = snprintf(pBuf, 15, "%u,", ddata[index]);
				break;}
			case DATA_SIGNED_INT: {
				int *ddata = data;
				len = snprintf(pBuf, 15, "%d,", ddata[index]);
				break;}
			default:
				dev_err(&client->dev,
					"%s: not defined data type\n",
					__func__);
				snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
				set_cmd_result(finfo, finfo->cmd_buff,
						strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
				finfo->cmd_state = NOT_APPLICABLE;
				devm_kfree(&client->dev, buff);
				goto all_data_out;
			}

			if (page_size - len < 0) {
				snprintf(pBuf, 5, "cont");
				break;
			} else {
				page_size -= len;
				pBuf += len;
			}
		}
		if (index == node_num)
			index = 0;

		set_cmd_result(finfo, buff, TSP_CMD_RESULT_STR_LEN);
		finfo->cmd_state = OK;

		devm_kfree(&client->dev, buff);
	} else {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "kzalloc failed");
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = NOT_APPLICABLE;
	}

all_data_out:
	dev_info(&client->dev, "%s\n", finfo->cmd_result);
	info->get_all_data = false;
}

static void hfdnd_spec_adjust(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	int test;

	set_default_result(finfo);

	test = finfo->cmd_param[0];

	if (test) {	/* set : assy */
		info->dnd_max_spec = DND_MAX_Ref_data;
		info->dnd_min_spec = DND_MIN_Ref_data;
		info->dnd_v_gap_spec = DND_V_GAP_Ref_data;
		info->dnd_h_gap_spec = DND_H_GAP_Ref_data;
		info->hfdnd_max_spec = HF_DND_MAX_Ref_data;
		info->hfdnd_min_spec = HF_DND_MIN_Ref_data;
		info->hfdnd_v_gap_spec = HF_DND_V_GAP_Ref_data;
		info->hfdnd_h_gap_spec = HF_DND_H_GAP_Ref_data;
		dev_info(&client->dev, "%s: set set spec: %d\n", __func__, test);
	} else {	/* module */
		info->dnd_max_spec = Module_DND_MAX_Ref_data;
		info->dnd_min_spec = Module_DND_MIN_Ref_data;
		info->dnd_v_gap_spec = Module_DND_V_GAP_Ref_data;
		info->dnd_h_gap_spec = Module_DND_H_GAP_Ref_data;
		info->hfdnd_max_spec = Module_HF_DND_MAX_Ref_data;
		info->hfdnd_min_spec = Module_HF_DND_MIN_Ref_data;
		info->hfdnd_v_gap_spec = Module_HF_DND_V_GAP_Ref_data;
		info->hfdnd_h_gap_spec = Module_HF_DND_H_GAP_Ref_data;
		dev_info(&client->dev, "%s: set module spec: %d\n", __func__, test);
	}

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "OK");
	set_cmd_result(finfo, finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	return;
}

static void run_dnd_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	u16 min, max;
	s32 i, j,offset;
	int fx, fy;
	bool result = true;

#if ESD_TIMER_INTERVAL
	esd_timer_stop(misc_info);
#endif

	set_default_result(finfo);

	ts_set_touchmode(TOUCH_DND_MODE);
	get_raw_data(info, (u8 *)raw_data->dnd_data, 1);
	ts_set_touchmode(TOUCH_POINT_MODE);

	min = 0xFFFF;
	max = 0x0000;

	for (i = 0; i < info->cap_info.x_node_num; i++) {
		pr_info("%s: dnd_data[%2d] :", client->name, i);
		for (j = 0; j < info->cap_info.y_node_num; j++) {
			offset = i * info->cap_info.y_node_num + j;
			pr_cont(" %5d", raw_data->dnd_data[offset]);
			if (raw_data->dnd_data[offset] < min &&raw_data->dnd_data[offset] != 0)
				min = raw_data->dnd_data[offset];
			if (raw_data->dnd_data[offset] > max)
				max = raw_data->dnd_data[offset];
			if (result && offset < ZT7538_DND_DATA_SIZE &&
				(raw_data->dnd_data[offset] > info->dnd_max_spec[offset]
				|| raw_data->dnd_data[offset] < info->dnd_min_spec[offset])) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	if(result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * info->cap_info.y_node_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					info->dnd_min_spec[offset],
					info->dnd_max_spec[offset],
					raw_data->dnd_data[offset]);
		pr_info("%s: dnd data view fail: [%d][%d]\n", info->client->name, fx, fy);
	}
	if (!info->get_all_data) {
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = OK;
	}

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strlen(finfo->cmd_buff));

#if ESD_TIMER_INTERVAL
	esd_timer_start(CHECK_ESD_TIMER, misc_info);
#endif

	return;
}

static void get_dnd(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	unsigned int val;
	int x_node, y_node;
	int node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= info->cap_info.x_node_num ||
		y_node < 0 || y_node >= info->cap_info.y_node_num) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "abnormal");
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = x_node * info->cap_info.y_node_num + y_node;

	val = raw_data->dnd_data[node_num];
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%u", val);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_dnd_all_data(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;

	get_all_data(info, run_dnd_read, info->raw_data->dnd_data, DATA_UNSIGNED_SHORT);
}

static void run_dnd_v_gap_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;
	int i, j, offset, val, cur_val, next_val;
	u16 screen_max = 0x0000;
	u16 touchkey_max = 0x0000;
	int fx, fy;
	bool result = true;
	set_default_result(finfo);

	memset(raw_data->vgap_data, 0x00, TSP_CMD_NODE_NUM);

	dev_info(&client->dev, "DND V Gap start\n");

	for (i = 0; i < x_num - 1; i++) {
		pr_info("%s: [%2d] :", client->name, i);
		for (j = 0; j < y_num; j++) {
			offset = (i * y_num) + j;

			cur_val = raw_data->dnd_data[offset];
			next_val = raw_data->dnd_data[offset + y_num];
			if ((i >= x_num - 2) && !next_val) {	/* touchkey node */
				raw_data->vgap_data[offset] = next_val;
				continue;
			}

			if (next_val > cur_val)
				val = 100 - ((cur_val * 100) / next_val);
			else
				val = 100 - ((next_val * 100) / cur_val);

			pr_cont(" %d", val);

			raw_data->vgap_data[offset] = val;

			if (i < x_num - 2){
				if (raw_data->vgap_data[offset] > screen_max)
					screen_max = raw_data->vgap_data[offset];
			} else {	/* touchkey node */
				if (raw_data->vgap_data[offset] > touchkey_max)
					touchkey_max = raw_data->vgap_data[offset];
			}
			if (result && offset < ZT7538_V_GAP_DATA_SIZE &&
				raw_data->vgap_data[offset] > info->dnd_v_gap_spec[offset]) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	dev_info(&client->dev, "DND V Gap screen_max %d touchkey_max %d\n", screen_max, touchkey_max);

	if (result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * y_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					0, info->dnd_v_gap_spec[offset],
					raw_data->vgap_data[offset]);
		pr_info("%s: dnd v gap view fail: [%d][%d]\n", info->client->name, fx, fy);
	}

	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	return;
}

static void run_dnd_h_gap_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;
	int i, j, offset, val, cur_val, next_val;
	u16 screen_max = 0x0000;
	u16 touchkey_max = 0x0000;
	int fx, fy;
	bool result = true;

	set_default_result(finfo);

	memset(raw_data->hgap_data, 0x00, TSP_CMD_NODE_NUM);

	dev_info(&client->dev, "DND H Gap start\n");

	for (i = 0; i < x_num ; i++) {
		pr_info("%s: [%2d] :", client->name, i);
		for (j = 0; j < y_num - 1; j++) {
			offset = (i * y_num) + j;

			cur_val = raw_data->dnd_data[offset];
			if ((i >= x_num - 1) && !cur_val) {	/* touchkey node */
				raw_data->hgap_data[offset] = cur_val;
				continue;
			}

			next_val = raw_data->dnd_data[offset + 1];
			if ((i >= x_num - 1) && !next_val) {	/* touchkey node */
				raw_data->hgap_data[offset] = next_val;
				for (++j; j < y_num - 1; j++) {
					offset = (i * y_num) + j;

					next_val = raw_data->dnd_data[offset];
					if (!next_val) {
						raw_data->hgap_data[offset] = next_val;
						continue;
					}
					break;
				}
			}

			if (next_val > cur_val)
				val = 100 - ((cur_val * 100) / next_val);
			else
				val = 100 - ((next_val * 100) / cur_val);

			pr_cont(" %d", val);

			raw_data->hgap_data[offset] = val;

			if (i < x_num - 1) {
					if (raw_data->hgap_data[offset] > screen_max)
						screen_max = raw_data->hgap_data[offset];
			} else {	/* touchkey node */
				if (raw_data->hgap_data[offset] > touchkey_max)
					touchkey_max = raw_data->hgap_data[offset];
			}
			if (result && offset < ZT7538_H_GAP_DATA_SIZE &&
				raw_data->hgap_data[offset] > info->dnd_h_gap_spec[offset]) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	dev_info(&client->dev, "DND H Gap screen_max %d, touchkey_max %d\n", screen_max, touchkey_max);

	if(result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * y_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					0, info->dnd_h_gap_spec[offset],
					raw_data->hgap_data[offset]);
		pr_info("%s: dnd h gap view fail: [%d][%d]\n", info->client->name, fx, fy);
	}
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	return;
}

static void get_dnd_h_gap(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_node, y_node;
	int node_num;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= x_num || y_node < 0 || y_node >= y_num - 1) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
		set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = (x_node * y_num) + y_node;

	sprintf(finfo->cmd_buff, "%d", raw_data->hgap_data[node_num]);
	set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}

static void get_dnd_v_gap(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_node, y_node;
	int node_num;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= x_num - 1 || y_node < 0 || y_node >= y_num) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
		set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = (x_node * y_num) + y_node;

	sprintf(finfo->cmd_buff, "%d", raw_data->vgap_data[node_num]);
	set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}


static void run_hfdnd_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;
	int i, j, offset;
	u16 min = 0xFFFF, max = 0x0000;
	int fx, fy;
	bool result = true;

#if ESD_TIMER_INTERVAL
	esd_timer_stop(misc_info);
#endif
	set_default_result(finfo);

	ts_set_touchmode2(TOUCH_DND_MODE);
	get_raw_data(info, (u8 *)raw_data->hfdnd_data, 2);
	ts_set_touchmode(TOUCH_POINT_MODE);

	dev_info(&client->dev, "HF DND start\n");

	for (i = 0; i < x_num; i++) {
		pr_info("%s: hfdnd_data[%2d] :", client->name, i);
		for (j = 0; j < y_num; j++) {
			offset = (i * y_num) + j;
			pr_cont(" %5d", raw_data->hfdnd_data[offset]);
			if (raw_data->hfdnd_data[offset] < min && raw_data->hfdnd_data[offset] != 0)
				min = raw_data->hfdnd_data[offset];
			if (raw_data->hfdnd_data[offset] > max)
				max = raw_data->hfdnd_data[offset];
			if (result && offset < ZT7538_DND_DATA_SIZE &&
				(raw_data->hfdnd_data[offset] > info->hfdnd_max_spec[offset]
				|| raw_data->hfdnd_data[offset] < info->hfdnd_min_spec[offset])) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	if(result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * info->cap_info.y_node_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					info->hfdnd_min_spec[offset],
					info->hfdnd_max_spec[offset],
					raw_data->hfdnd_data[offset]);
		pr_info("%s: hfdnd data view fail: [%d][%d]\n", info->client->name, fx, fy);
	}
	if (!info->get_all_data) {
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = OK;
	}

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
		(int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

#if ESD_TIMER_INTERVAL
	esd_timer_start(CHECK_ESD_TIMER, misc_info);
#endif
	return;
}

static void get_hfdnd(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	unsigned int val;
	int x_node, y_node;
	int node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= info->cap_info.x_node_num ||
		y_node < 0 || y_node >= info->cap_info.y_node_num) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "abnormal");
		set_cmd_result(finfo, finfo->cmd_buff,
		strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = x_node * info->cap_info.y_node_num + y_node;

	val = raw_data->hfdnd_data[node_num];
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%u", val);
	set_cmd_result(finfo, finfo->cmd_buff,
		strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
		(int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_hfdnd_all_data(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;

	get_all_data(info, run_hfdnd_read, info->raw_data->hfdnd_data, DATA_SIGNED_SHORT);
}

static void run_hfdnd_v_gap_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;
	int i, j, offset, val, cur_val, next_val;
	u16 screen_max = 0x0000;
	u16 touchkey_max = 0x0000;
	int fx, fy;
	bool result = true;

	set_default_result(finfo);

	memset(raw_data->hfvgap_data, 0x00, TSP_CMD_NODE_NUM);

	dev_info(&client->dev, "HFDND V Gap start\n");

	for (i = 0; i < x_num - 1; i++) {
		pr_info("%s: [%2d] :", client->name, i);
		for (j = 0; j < y_num; j++) {
			offset = (i * y_num) + j;

			cur_val = raw_data->hfdnd_data[offset];
			next_val = raw_data->hfdnd_data[offset + y_num];
			if ((i >= x_num - 2) && !next_val) {	/* touchkey node */
				raw_data->hfvgap_data[offset] = next_val;
				continue;
			}

			if (next_val > cur_val)
				val = 100 - ((cur_val * 100) / next_val);
			else
				val = 100 - ((next_val * 100) / cur_val);

			pr_cont(" %d", val);

			raw_data->hfvgap_data[offset] = val;

			if (i < x_num - 2){
				if (raw_data->hfvgap_data[offset] > screen_max)
					screen_max = raw_data->hfvgap_data[offset];
			} else {	/* touchkey node */
				if (raw_data->hfvgap_data[offset] > touchkey_max)
					touchkey_max = raw_data->hfvgap_data[offset];
			}
			if (result && offset < ZT7538_V_GAP_DATA_SIZE &&
				raw_data->hfvgap_data[offset] > info->hfdnd_v_gap_spec[offset]) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	dev_info(&client->dev, "HFDND V Gap screen_max %d touchkey_max %d\n", screen_max, touchkey_max);

	if (result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * y_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					0, info->hfdnd_v_gap_spec[offset],
					raw_data->hfvgap_data[offset]);
		pr_info("%s: hfdnd v gap view fail: [%d][%d]\n", info->client->name, fx, fy);
	}
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_hfdnd_v_gap(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_node, y_node;
	int node_num;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= x_num - 1 || y_node < 0 || y_node >= y_num) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
		set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = (x_node * y_num) + y_node;

	sprintf(finfo->cmd_buff, "%d", raw_data->hfvgap_data[node_num]);
	set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}

static void run_hfdnd_h_gap_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;
	int i, j, offset, val, cur_val, next_val;
	u16 screen_max = 0x0000;
	u16 touchkey_max = 0x0000;
	int fx, fy;
	bool result = true;
	set_default_result(finfo);

	memset(raw_data->hgap_data, 0x00, TSP_CMD_NODE_NUM);

	dev_info(&client->dev, "HFDND H Gap start\n");

	for (i = 0; i < x_num ; i++) {
		pr_info("%s: [%2d] :", client->name, i);
		for (j = 0; j < y_num - 1; j++) {
			offset = (i * y_num) + j;

			cur_val = raw_data->hfdnd_data[offset];
			if ((i >= x_num - 1) && !cur_val) {	/* touchkey node */
				raw_data->hfhgap_data[offset] = cur_val;
				continue;
			}

			next_val = raw_data->hfdnd_data[offset + 1];
			if ((i >= x_num - 1) && !next_val) {	/* touchkey node */
				raw_data->hfhgap_data[offset] = next_val;
				for (++j; j < y_num - 1; j++) {
					offset = (i * y_num) + j;

					next_val = raw_data->hfdnd_data[offset];
					if (!next_val) {
						raw_data->hfhgap_data[offset] = next_val;
						continue;
					}
					break;
				}
			}

			if (next_val > cur_val)
				val = 100 - ((cur_val * 100) / next_val);
			else
				val = 100 - ((next_val * 100) / cur_val);

			pr_cont(" %d", val);

			raw_data->hfhgap_data[offset] = val;

			if (i < x_num - 1) {
					if (raw_data->hfhgap_data[offset] > screen_max)
						screen_max = raw_data->hfhgap_data[offset];
			} else {	/* touchkey node */
				if (raw_data->hfhgap_data[offset] > touchkey_max)
					touchkey_max = raw_data->hfhgap_data[offset];
			}
			if (result && offset < ZT7538_H_GAP_DATA_SIZE &&
				raw_data->hfhgap_data[offset] > info->hfdnd_h_gap_spec[offset]) {
				result = false;
				fx = i;
				fy = j;
				pr_cont("(E)");
			}
		}
		pr_cont("\n");
	}

	dev_info(&client->dev, "DND H Gap screen_max %d, touchkey_max %d\n", screen_max, touchkey_max);

	if (result) {
		sprintf(finfo->cmd_buff, "%s", "OK");
	} else {
		offset = (fx * y_num) + fy;
		sprintf(finfo->cmd_buff, "%d,%d,%d,%d,%d", fx, fy,
					0, info->hfdnd_h_gap_spec[offset],
					raw_data->hfhgap_data[offset]);
		pr_info("%s: hfdnd h gap view fail: [%d][%d]\n", info->client->name, fx, fy);
	}

	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_hfdnd_h_gap(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	int x_node, y_node;
	int node_num;
	int x_num = info->cap_info.x_node_num, y_num = info->cap_info.y_node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= x_num || y_node < 0 || y_node >= y_num - 1) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
		set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = (x_node * y_num) + y_node;

	sprintf(finfo->cmd_buff, "%d", raw_data->hfhgap_data[node_num]);
	set_cmd_result(finfo, finfo->cmd_buff, strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			finfo->cmd_buff, (int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}

static void run_delta_read(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	s16 min, max;
	s32 i, j;

#if ESD_TIMER_INTERVAL
	esd_timer_stop(misc_info);
#endif

	set_default_result(finfo);

	ts_set_touchmode(TOUCH_DELTA_MODE);
	get_raw_data(info, (u8 *)raw_data->delta_data, 10);
	ts_set_touchmode(TOUCH_POINT_MODE);

	min = (s16)0x7FFF;
	max = (s16)0x8000;

	for (i = 0; i < info->cap_info.x_node_num; i++) {
		pr_info("%s: delta_data[%2d] : ", client->name, i);
		for (j = 0; j < info->cap_info.y_node_num; j++) {
			pr_cont("[%3d]", raw_data->delta_data[i * info->cap_info.y_node_num + j]);
			if (raw_data->delta_data[i * info->cap_info.y_node_num + j] < min &&
				raw_data->delta_data[i * info->cap_info.y_node_num + j] != 0)
				min = raw_data->delta_data[i * info->cap_info.y_node_num + j];
			if (raw_data->delta_data[i * info->cap_info.y_node_num + j] > max)
				max = raw_data->delta_data[i * info->cap_info.y_node_num + j];

		}
		pr_cont("\n");
	}

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%d,%d", min, max);
	if (!info->get_all_data) {
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = OK;
	}

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strlen(finfo->cmd_buff));

#if ESD_TIMER_INTERVAL
	esd_timer_start(CHECK_ESD_TIMER, misc_info);
#endif

	return;
}

static void get_delta(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	struct tsp_raw_data *raw_data = info->raw_data;
	unsigned int val;
	int x_node, y_node;
	int node_num;

	set_default_result(finfo);

	x_node = finfo->cmd_param[0];
	y_node = finfo->cmd_param[1];

	if (x_node < 0 || x_node >= info->cap_info.x_node_num ||
		y_node < 0 || y_node >= info->cap_info.y_node_num) {
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "abnormal");
		set_cmd_result(finfo, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
		finfo->cmd_state = FAIL;
		return;
	}

	node_num = x_node * info->cap_info.y_node_num + y_node;

	val = raw_data->delta_data[node_num];
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%u", val);
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void get_delta_all_data(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;

	get_all_data(info, run_delta_read, info->raw_data->delta_data, DATA_SIGNED_SHORT);
}

static void dead_zone_enable(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

	if (finfo->cmd_param[0] == 1) {	/* enable */
		zinitix_bit_clr(m_optional_mode, DEF_OPTIONAL_MODE_EDGE_SELECT);
	} else if (finfo->cmd_param[0] == 0) {
		zinitix_bit_set(m_optional_mode, DEF_OPTIONAL_MODE_EDGE_SELECT);
	} else {
		finfo->cmd_state = FAIL;
		sprintf(finfo->cmd_buff, "%s", "NG");
		goto err;
	}
	zt7538_set_optional_mode(info, false);

	finfo->cmd_state = OK;
	sprintf(finfo->cmd_buff, "%s", "OK");
err:
	set_cmd_result(finfo, finfo->cmd_buff,
			strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}

static void clear_cover_mode(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct tsp_factory_info *finfo = info->factory_info;
	int arg = finfo->cmd_param[0];

	set_default_result(finfo);
	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%u",
							(unsigned int) arg);

	g_cover_state = arg;
	cover_set(info);
	set_cmd_result(finfo, finfo->cmd_buff,
					strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	finfo->cmd_is_running = false;
	finfo->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
				strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));

	return;
}
static void clear_reference_data(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	set_default_result(finfo);

#if ESD_TIMER_INTERVAL
	esd_timer_stop(info);
	write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL, 0);
#endif

	write_reg(client, ZT7538_EEPROM_INFO_REG, 0xffff);

	write_reg(client, 0xc003, 0x0001);
	write_reg(client, 0xc104, 0x0001);
	usleep_range(100, 100);
	if (write_cmd(client, ZT7538_SAVE_STATUS_CMD) != I2C_SUCCESS) {
		dev_err(&client->dev, "%s: failed to TSP clear calibration bit\n", __func__);
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "NG");
		goto out;
	}

	zinitix_delay(500);
	write_reg(client, 0xc003, 0x0000);
	write_reg(client, 0xc104, 0x0000);
	usleep_range(100, 100);

#if ESD_TIMER_INTERVAL
	write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL,
		SCAN_RATE_HZ * ESD_TIMER_INTERVAL);
	esd_timer_start(CHECK_ESD_TIMER, info);
#endif
	dev_info(&client->dev, "%s: TSP clear calibration bit\n", __func__);

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", "OK");
out:
	set_cmd_result(finfo, finfo->cmd_buff,
		strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
		(int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	return;
}

static void run_ref_calibration(void *device_data)
{
	struct zt7538_ts_info *info = (struct zt7538_ts_info *)device_data;
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	int i;
	bool ret;

	set_default_result(finfo);

#if ESD_TIMER_INTERVAL
	esd_timer_stop(info);
	write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL, 0);
#endif
	disable_irq(info->irq);

	ret = ts_hw_calibration(info);
	dev_info(&client->dev, "%s: TSP calibration %s\n",
				__func__, ret ? "Pass" : "Fail");

	for (i = 0; i < 5; i++) {
		write_cmd(client, ZT7538_CLEAR_INT_STATUS_CMD);
		usleep_range(10, 10);
	}

	enable_irq(info->irq);

#if ESD_TIMER_INTERVAL
	write_reg(client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL,
		SCAN_RATE_HZ * ESD_TIMER_INTERVAL);
	esd_timer_start(CHECK_ESD_TIMER, info);
#endif

	snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "%s", ret ? "OK" : "NG");
	set_cmd_result(finfo, finfo->cmd_buff,
		strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
	finfo->cmd_state = OK;

	dev_info(&client->dev, "%s: %s(%d)\n", __func__, finfo->cmd_buff,
		(int)strnlen(finfo->cmd_buff, sizeof(finfo->cmd_buff)));
}


static ssize_t store_cmd(struct device *dev, struct device_attribute
				  *devattr, const char *buf, size_t count)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;
	char *cur, *start, *end;
	char buff[TSP_CMD_STR_LEN] = {0};
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;

	if (finfo->cmd_is_running == true) {
		dev_err(&client->dev, "%s: other cmd is running\n", __func__);
		goto err_out;
	}

	/* check lock  */
	mutex_lock(&finfo->cmd_lock);
	finfo->cmd_is_running = true;
	mutex_unlock(&finfo->cmd_lock);

	finfo->cmd_state = RUNNING;

	for (i = 0; i < ARRAY_SIZE(finfo->cmd_param); i++)
		finfo->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;

	memset(finfo->cmd, 0x00, ARRAY_SIZE(finfo->cmd));
	memcpy(finfo->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &finfo->cmd_list_head, list) {
		if (!strcmp(buff, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &finfo->cmd_list_head, list) {
			if (!strcmp("not_support_cmd", tsp_cmd_ptr->cmd_name))
				break;
		}
	}

	/* parsing parameters */
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(buff, 0x00, ARRAY_SIZE(buff));
		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(buff, start, end - start);
				*(buff + strlen(buff)) = '\0';
				finfo->cmd_param[param_cnt] = (int)simple_strtol(buff, NULL, 10);
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}

	dev_dbg(&client->dev, "cmd = %s\n", tsp_cmd_ptr->cmd_name);

	tsp_cmd_ptr->cmd_func(info);

err_out:
	return count;
}

static ssize_t show_cmd_status(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	dev_dbg(&client->dev, "tsp cmd: status:%d\n", finfo->cmd_state);

	if (finfo->cmd_state == WAITING)
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "WAITING");
	else if (finfo->cmd_state == RUNNING)
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "RUNNING");
	else if (finfo->cmd_state == OK)
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "OK");
	else if (finfo->cmd_state == FAIL)
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "FAIL");
	else if (finfo->cmd_state == NOT_APPLICABLE)
		snprintf(finfo->cmd_buff, sizeof(finfo->cmd_buff), "NOT_APPLICABLE");

	return snprintf(buf, sizeof(finfo->cmd_buff), "%s\n", finfo->cmd_buff);
}

static ssize_t show_cmd_result(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct tsp_factory_info *finfo = info->factory_info;

	dev_info(&client->dev, "tsp cmd: result: %s\n", finfo->cmd_result);

	finfo->cmd_state = WAITING;

	mutex_lock(&finfo->cmd_lock);
	finfo->cmd_is_running = false;
	mutex_unlock(&finfo->cmd_lock);

	return snprintf(buf, sizeof(finfo->cmd_result), "%s\n", finfo->cmd_result);
}

static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);

static struct attribute *touchscreen_attributes[] = {
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	NULL,
};

static struct attribute_group touchscreen_attr_group = {
	.attrs = touchscreen_attributes,
};

#ifdef SUPPORTED_TOUCH_KEY
static ssize_t show_touchkey_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct capa_info *cap = &(info->cap_info);

	dev_info(&client->dev, "%s: key threshold = %d\n", __func__, cap->key_threshold);

	return snprintf(buf, 41, "%d", cap->key_threshold);
}

static ssize_t show_touchkey_sensitivity(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	u16 val;
	int ret;
	int i;

	if (!strcmp(attr->attr.name, "touchkey_recent"))
		i = 0;
	else if (!strcmp(attr->attr.name, "touchkey_back"))
		i = 1;
	else {
		dev_err(&client->dev, "%s: Invalid attribute\n", __func__);
		goto err_out;
	}
	down(&info->work_lock);
	ret = read_data(client, ZT7538_BTN_WIDTH + i, (u8 *)&val, 2);
	up(&info->work_lock);
	if (ret < 0) {
		dev_err(&client->dev, "failed to read %d's key sensitivity\n", i);
		goto err_out;
	}

	dev_info(&client->dev, "%d's key sensitivity = %d\n", i, val);

	return snprintf(buf, 6, "%d", val);

err_out:
	return sprintf(buf, "NG");
}
#endif

#ifdef SUPPORTED_KEY_LED
static ssize_t touchkey_led_control(struct device *dev,
		 struct device_attribute *attr, const char *buf,
		 size_t count)
{
	struct zt7538_ts_info *info = dev_get_drvdata(dev);
	int data;
	int ret;

	ret = sscanf(buf, "%d", &data);
	if (ret != 1) {
		dev_err(&info->client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(data == 0 || data == 1)) {
		dev_err(&info->client->dev, "%s: wrong command(%d)\n",
			__func__, data);
		return count;
	}

	if (IS_ERR_OR_NULL(info->led_ldo))
		return count;

	if (data == 1)
		ret = regulator_enable(info->led_ldo);
	else
		ret = regulator_disable(info->led_ldo);

	zinitix_delay(20);

	dev_info(&info->client->dev, "%s data(%d), ret(%d)\n",
		__func__, data, regulator_is_enabled(info->led_ldo));

	return count;
}
#endif

#ifdef SUPPORTED_TOUCH_KEY
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, show_touchkey_threshold, NULL);
static DEVICE_ATTR(touchkey_recent, S_IRUGO, show_touchkey_sensitivity, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO, show_touchkey_sensitivity, NULL);
#endif
#ifdef SUPPORTED_KEY_LED
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL, touchkey_led_control);
#endif

#ifdef SUPPORTED_TOUCH_KEY
static struct attribute *touchkey_attributes[] = {
	&dev_attr_touchkey_threshold.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_recent.attr,
#ifdef SUPPORTED_KEY_LED
	&dev_attr_brightness.attr,
#endif
	NULL,
};
static struct attribute_group touchkey_attr_group = {
	.attrs = touchkey_attributes,
};
#endif

static int init_sec_factory(struct zt7538_ts_info *info)
{
	struct device *factory_ts_dev;
#ifdef SUPPORTED_TOUCH_KEY
	struct device *factory_tk_dev;
#endif
	struct tsp_factory_info *factory_info;
	struct tsp_raw_data *raw_data;
	int ret;
	int i;

	factory_info = devm_kzalloc(&info->client->dev, sizeof(struct tsp_factory_info), GFP_KERNEL);
	if (unlikely(!factory_info)) {
		dev_err(&info->client->dev, "failed to allocate factory_info\n");
		ret = -ENOMEM;
		goto err_alloc1;
	}

	raw_data = devm_kzalloc(&info->client->dev, sizeof(struct tsp_raw_data), GFP_KERNEL);
	if (unlikely(!raw_data)) {
		dev_err(&info->client->dev, "failed to allocate raw_data\n");
		ret = -ENOMEM;
		goto err_alloc2;
	}

	INIT_LIST_HEAD(&factory_info->cmd_list_head);
	for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
		list_add_tail(&tsp_cmds[i].list, &factory_info->cmd_list_head);

	factory_ts_dev = device_create(sec_class, NULL, 0, info, "tsp");
	if (unlikely(!factory_ts_dev)) {
		dev_err(&info->client->dev, "ailed to create factory dev\n");
		ret = -ENODEV;
		goto err_create_device;
	}

#ifdef SUPPORTED_TOUCH_KEY
	factory_tk_dev = device_create(sec_class, NULL, 0, info, "sec_touchkey");
	if (IS_ERR(factory_tk_dev)) {
		dev_err(&info->client->dev, "failed to create factory dev\n");
		ret = -ENODEV;
		goto err_create_device;
	}
#endif

	ret = sysfs_create_group(&factory_ts_dev->kobj, &touchscreen_attr_group);
	if (unlikely(ret)) {
		dev_err(&info->client->dev, "Failed to create touchscreen sysfs group\n");
		goto err_create_sysfs;
	}

#ifdef SUPPORTED_TOUCH_KEY
	ret = sysfs_create_group(&factory_tk_dev->kobj, &touchkey_attr_group);
	if (unlikely(ret)) {
		dev_err(&info->client->dev, "Failed to create touchkey sysfs group\n");
		goto err_create_sysfs;
	}
#endif

	ret = sysfs_create_link(&factory_ts_dev->kobj,
		&info->input_dev->dev.kobj, "input");
	if (ret < 0) {
		dev_err(&info->client->dev,
			"%s: Failed to create input symbolic link %d\n",
			__func__, ret);
	}

	mutex_init(&factory_info->cmd_lock);
	factory_info->cmd_is_running = false;

	info->factory_info = factory_info;
	info->raw_data = raw_data;

	return ret;

err_create_sysfs:
err_create_device:
	devm_kfree(&info->client->dev, raw_data);
err_alloc2:
	devm_kfree(&info->client->dev, factory_info);
err_alloc1:

	return ret;
}
#endif /* end of SEC_FACTORY_TEST */

#ifdef CONFIG_OF
static const struct of_device_id tsp_dt_ids[] = {
	{ .compatible = "Zinitix,zt7538", },
	{},
};
MODULE_DEVICE_TABLE(of, tsp_dt_ids);
#else
#define tsp_dt_ids NULL
#endif

static int zt7538_ts_parse_dt(struct device_node *np, struct device *dev,
					struct zt7538_ts_dt_data *pdata)
{
	int ret;

	if (!np)
		return -EINVAL;

	pr_info("%s: ", __func__);

	/* gpio irq */
	pdata->gpio_int = of_get_named_gpio(np, "zinitix,irq-gpio", 0);
	if (pdata->gpio_int < 0) {
		dev_err(dev, "failed to get irq number\n");
		return -EINVAL;
	}
	pr_cont("int:%d, ", pdata->gpio_int);

	ret = gpio_request(pdata->gpio_int, "zt7538_irq");
	if (ret < 0) {
		dev_err(dev, "failed to request gpio_irq\n");
		return -EINVAL;
	}
	gpio_direction_input(pdata->gpio_int);

	pdata->gpio_scl = of_get_named_gpio(np, "zinitix,scl-gpio", 0);
	if (pdata->gpio_scl < 0) {
		dev_err(dev, "failed to get scl number\n");
		return -EINVAL;
	}
	pr_cont("scl:%d, ", pdata->gpio_scl);

	pdata->gpio_sda = of_get_named_gpio(np, "zinitix,sda-gpio", 0);
	if (pdata->gpio_sda < 0) {
		dev_err(dev, "failed to get sda number\n");
		return -EINVAL;
	}
	pr_cont("sda:%d, ", pdata->gpio_sda);

	/* gpio power enable */
	pdata->vdd_en = of_get_named_gpio(np, "zinitix,tsppwr_en", 0);
	if (pdata->vdd_en < 0) {
		pr_info("%s: ", __func__);
	}
	pr_cont("vdd_en:%d, ", pdata->vdd_en);

	if (gpio_is_valid(pdata->vdd_en)) {
		ret = gpio_request(pdata->vdd_en, "zt7538_vdd_en");
		if (ret < 0) {
			dev_err(dev, "failed to request gpio_vdd_en\n");
			return -EINVAL;
		}
	}

	ret = of_property_read_string(np, "zt7538,fw_name", &pdata->fw_name);
	if (ret < 0) {
		dev_err(dev, "failed to get firmware path!\n");
		return -EINVAL;
	}
	pr_cont("fw_name:%s, ", pdata->fw_name);

	ret = of_property_read_u32(np, "zt7538,x_resolution", &pdata->x_resolution);
	if (ret < 0) {
		dev_err(dev, "failed to get x_resolution\n");
		return ret;
	}
	pr_cont("max_x:%d, ", pdata->x_resolution);

	ret = of_property_read_u32(np, "zt7538,y_resolution", &pdata->y_resolution);
	if (ret < 0) {
		dev_err(dev, "failed to get y_resolution\n");
		return ret;
	}
	pr_cont("max_y:%d, ", pdata->y_resolution);

	ret = of_property_read_string(np, "zt7538,model_name", &pdata->model_name);
	if (ret < 0) {
		pdata->model_name = "";
	}
	pr_cont("model:%s, ", pdata->model_name);

	pdata->reg_boot_on = of_property_read_bool(np, "zt7538,reg_boot_on");
	pr_cont("reg_boot_on:%d, ", pdata->reg_boot_on);

	pdata->tsp_power = zt7538_power;
	pr_cont("end\n");
	return 0;

}

#if ZINITIX_MISC_DEBUG
static int ts_misc_fops_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int ts_misc_fops_close(struct inode *inode, struct file *filp)
{
	return 0;
}

static long ts_misc_fops_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct raw_ioctl raw_ioctl;
	u8 *u8Data;
	int ret = 0;
	size_t sz = 0;
//	u16 version;
	u16 mode;

	struct reg_ioctl reg_ioctl;
	u16 val;
	int nval = 0;

	if (!misc_info) {
		dev_err(&misc_info->client->dev, "misc device NULL?\n");
		return -1;
	}

	switch (cmd) {

	case TOUCH_IOCTL_GET_DEBUGMSG_STATE:
		ret = m_ts_debug_mode;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_SET_DEBUGMSG_STATE:
		if (copy_from_user(&nval, argp, 4)) {
			dev_err(&misc_info->client->dev, "error : copy_from_user\n");
			return -1;
		}
		if (nval)
			dev_err(&misc_info->client->dev, "on debug mode (%d)\n", nval);
		else
			dev_err(&misc_info->client->dev, "off debug mode (%d)\n", nval);
		m_ts_debug_mode = nval;
		break;

	case TOUCH_IOCTL_GET_CHIP_REVISION:
		ret = misc_info->cap_info.ic_revision;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_FW_VERSION:
		ret = misc_info->cap_info.fw_version;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_REG_DATA_VERSION:
		ret = misc_info->cap_info.reg_data_version;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_VARIFY_UPGRADE_SIZE:
		if (copy_from_user(&sz, argp, sizeof(size_t)))
			return -1;

		dev_info(&misc_info->client->dev, "firmware size = %d\n", sz);
		if (misc_info->cap_info.ic_fw_size != sz) {
			dev_err(&misc_info->client->dev, ": firmware size error\r\n");
			return -1;
		}
		break;
/*
	case TOUCH_IOCTL_VARIFY_UPGRADE_DATA:
		if (copy_from_user(m_firmware_data,
			argp, misc_info->cap_info.ic_fw_size))
			return -1;

		version = (u16) (m_firmware_data[52] | (m_firmware_data[53]<<8));

		dev_err(&misc_info->client->dev, "firmware version = %x\n", version);

		if (copy_to_user(argp, &version, sizeof(version)))
			return -1;
		break;

	case TOUCH_IOCTL_START_UPGRADE:
		return ts_upgrade_sequence((u8 *)m_firmware_data);
*/
	case TOUCH_IOCTL_GET_X_RESOLUTION:
		ret = misc_info->pdata->x_resolution;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_Y_RESOLUTION:
		ret = misc_info->pdata->y_resolution;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_X_NODE_NUM:
		ret = misc_info->cap_info.x_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_Y_NODE_NUM:
		ret = misc_info->cap_info.y_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_GET_TOTAL_NODE_NUM:
		ret = misc_info->cap_info.total_node_num;
		if (copy_to_user(argp, &ret, sizeof(ret)))
			return -1;
		break;

	case TOUCH_IOCTL_HW_CALIBRAION:
		ret = -1;
		disable_irq(misc_info->irq);
		down(&misc_info->work_lock);
		if (misc_info->work_state != NOTHING) {
			dev_err(&misc_info->client->dev, ": other process occupied.. (%d)\n",
				misc_info->work_state);
			up(&misc_info->work_lock);
			return -1;
		}
		misc_info->work_state = HW_CALIBRAION;
		zinitix_delay(100);

		/* h/w calibration */
		if (ts_hw_calibration(misc_info))
			ret = 0;

		mode = misc_info->touch_mode;
		if (write_reg(misc_info->client,
			ZT7538_TOUCH_MODE, mode) != I2C_SUCCESS) {
			dev_err(&misc_info->client->dev, "failed to set touch mode %d.\n", mode);
			goto fail_hw_cal;
		}

		if (write_cmd(misc_info->client, ZT7538_SWRESET_CMD) != I2C_SUCCESS)
			goto fail_hw_cal;

		enable_irq(misc_info->irq);
		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return ret;
fail_hw_cal:
		enable_irq(misc_info->irq);
		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return -1;

	case TOUCH_IOCTL_SET_RAW_DATA_MODE:
		if (misc_info == NULL) {
			dev_err(&misc_info->client->dev, "misc device NULL?\n");
			return -1;
		}
		if (copy_from_user(&nval, argp, 4)) {
			dev_err(&misc_info->client->dev, " error : copy_from_user\r\n");
			misc_info->work_state = NOTHING;
			return -1;
		}
		ts_set_touchmode((u16)nval);

		return 0;

	case TOUCH_IOCTL_GET_REG:
		if (misc_info == NULL) {
			dev_err(&misc_info->client->dev, "misc device NULL?\n");
			return -1;
		}
		down(&misc_info->work_lock);
		if (misc_info->work_state != NOTHING) {
			dev_err(&misc_info->client->dev, ":other process occupied.. (%d)\n",
				misc_info->work_state);
			up(&misc_info->work_lock);
			return -1;
		}

		misc_info->work_state = SET_MODE;

		if (copy_from_user(&reg_ioctl,
			argp, sizeof(struct reg_ioctl))) {
			misc_info->work_state = NOTHING;
			up(&misc_info->work_lock);
			dev_err(&misc_info->client->dev, " error : copy_from_user(1)\n");
			return -1;
		}

		if (read_data(misc_info->client,
			(u16)reg_ioctl.addr, (u8 *)&val, 2) < 0)
			ret = -1;

		nval = (int)val;

		if (copy_to_user((void *)reg_ioctl.val, (u8 *)&nval, 4)) {
			misc_info->work_state = NOTHING;
			up(&misc_info->work_lock);
			dev_err(&misc_info->client->dev, " error : copy_to_user(2)\n");
			return -1;
		}

		dev_err(&misc_info->client->dev, "read : reg addr = 0x%x, val = 0x%x\n",
			reg_ioctl.addr, nval);

		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return ret;

	case TOUCH_IOCTL_SET_REG:

		down(&misc_info->work_lock);
		if (misc_info->work_state != NOTHING) {
			dev_err(&misc_info->client->dev, ": other process occupied.. (%d)\n",
				misc_info->work_state);
			up(&misc_info->work_lock);
			return -1;
		}

		misc_info->work_state = SET_MODE;
		if (copy_from_user(&reg_ioctl,
				argp, sizeof(struct reg_ioctl))) {
			misc_info->work_state = NOTHING;
			up(&misc_info->work_lock);
			dev_err(&misc_info->client->dev, " error : copy_from_user(1)\n");
			return -1;
		}

		if (copy_from_user(&val, (void *)reg_ioctl.val, 4)) {
			misc_info->work_state = NOTHING;
			up(&misc_info->work_lock);
			dev_err(&misc_info->client->dev, " error : copy_from_user(2)\n");
			return -1;
		}

		if (write_reg(misc_info->client,
			(u16)reg_ioctl.addr, val) != I2C_SUCCESS)
			ret = -1;

		dev_err(&misc_info->client->dev, "write : reg addr = 0x%x, val = 0x%x\r\n",
			reg_ioctl.addr, val);
		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return ret;

	case TOUCH_IOCTL_DONOT_TOUCH_EVENT:
		if (misc_info == NULL) {
			dev_err(&misc_info->client->dev, "misc device NULL?\n");
			return -1;
		}
		down(&misc_info->work_lock);
		if (misc_info->work_state != NOTHING) {
			dev_err(&misc_info->client->dev, ": other process occupied.. (%d)\r\n",
				misc_info->work_state);
			up(&misc_info->work_lock);
			return -1;
		}

		misc_info->work_state = SET_MODE;
		if (write_reg(misc_info->client,
			ZT7538_INT_ENABLE_FLAG, 0) != I2C_SUCCESS)
			ret = -1;
		dev_err(&misc_info->client->dev, "write : reg addr = 0x%x, val = 0x0\r\n",
			ZT7538_INT_ENABLE_FLAG);

		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return ret;

	case TOUCH_IOCTL_SEND_SAVE_STATUS:
		if (misc_info == NULL) {
			dev_err(&misc_info->client->dev, "misc device NULL?\n");
			return -1;
		}
		down(&misc_info->work_lock);
		if (misc_info->work_state != NOTHING) {
			dev_err(&misc_info->client->dev, ": other process occupied.." \
				"(%d)\r\n", misc_info->work_state);
			up(&misc_info->work_lock);
			return -1;
		}
		misc_info->work_state = SET_MODE;
		ret = 0;
		if (write_cmd(misc_info->client,
			ZT7538_SAVE_STATUS_CMD) != I2C_SUCCESS)
			ret =  -1;

		zinitix_delay(1000);	/* for fusing eeprom */

		misc_info->work_state = NOTHING;
		up(&misc_info->work_lock);
		return ret;

	case TOUCH_IOCTL_GET_RAW_DATA:
		if (misc_info == NULL) {
			dev_err(&misc_info->client->dev, "misc device NULL?\n");
			return -1;
		}

		if (misc_info->touch_mode == TOUCH_POINT_MODE)
			return -1;

		down(&misc_info->raw_data_lock);
		if (misc_info->update == 0) {
			up(&misc_info->raw_data_lock);
			return -2;
		}

		if (copy_from_user(&raw_ioctl,
			argp, sizeof(struct raw_ioctl))) {
			up(&misc_info->raw_data_lock);
			dev_err(&misc_info->client->dev, "error: copy_from_user\r\n");
			return -1;
		}

		misc_info->update = 0;

		u8Data = (u8 *)&misc_info->cur_data[0];
		if (raw_ioctl.sz > MAX_TRAW_DATA_SZ * 2)
			raw_ioctl.sz = MAX_TRAW_DATA_SZ * 2;
		if (copy_to_user((void *)raw_ioctl.buf, (u8 *)u8Data, raw_ioctl.sz)) {
			up(&misc_info->raw_data_lock);
			return -1;
		}

		up(&misc_info->raw_data_lock);
		return 0;

	default:
		break;
	}
	return 0;
}

static const struct file_operations ts_misc_fops = {
	.owner = THIS_MODULE,
	.open = ts_misc_fops_open,
	.release = ts_misc_fops_close,
	.unlocked_ioctl = ts_misc_fops_ioctl,
};

static struct miscdevice touch_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "zinitix_touch_misc",
	.fops = &ts_misc_fops,
};
#endif

static int zt7538_pinctrl_configure(struct zt7538_ts_info *info,
							bool active)
{
	struct pinctrl_state *set_state_i2c;
	int retval;

	dev_err(&info->client->dev, "%s: %s\n", __func__, active ? "ACTIVE" : "SUSPEND");

	if (active) {
		set_state_i2c =
			pinctrl_lookup_state(info->pinctrl,
						"tsp_i2c_gpio_active");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) active state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}
	} else {
		set_state_i2c =
			pinctrl_lookup_state(info->pinctrl,
						"tsp_i2c_suspend");
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

	return 0;
}

static int zt7538_ts_probe(struct i2c_client *client, const struct i2c_device_id *i2c_id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct zt7538_ts_dt_data *pdata = client->dev.platform_data;
	struct zt7538_ts_info *info;
	struct input_dev *input_dev;
	int ret = -1;
	int i;
	struct device_node *np = client->dev.of_node;

	if (IS_ENABLED(CONFIG_OF)) {
		if (!pdata) {
			pdata = devm_kzalloc(&client->dev,
					sizeof(*pdata), GFP_KERNEL);
			if (!pdata)
				return -ENOMEM;
		}
		ret = zt7538_ts_parse_dt(np, &client->dev, pdata);
		if (ret)
			goto err_no_platform_data;
	} else if (!pdata) {
		dev_err(&client->dev, "Not exist platform data\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "Not compatible i2c function\n");
		ret = -EIO;
		goto err_no_platform_data;
	}

	info = devm_kzalloc(&client->dev, sizeof(struct zt7538_ts_info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_no_platform_data;
	}

	i2c_set_clientdata(client, info);
	info->client = client;
	info->pdata = pdata;
	info->device_enabled = 1;

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev, "Failed to allocate input device\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	info->input_dev = input_dev;
	info->work_state = PROBE;

	/* Get pinctrl if target uses pinctrl */
	info->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(info->pinctrl)) {
		if (PTR_ERR(info->pinctrl) == -EPROBE_DEFER) {
			ret = -ENODEV;
			goto err_alloc;	// err_input_alloc;
		}

		dev_err(&client->dev, "%s: Target does not use pinctrl\n", __func__);
		info->pinctrl = NULL;
	}

	if (info->pinctrl) {
		ret = zt7538_pinctrl_configure(info, true);
		if (ret)
			dev_err(&client->dev, "%s: cannot set pinctrl state\n", __func__);
	}

#ifdef SUPPORTED_KEY_LED
	info->led_ldo = devm_regulator_get(&client->dev, "key-led");
	if (IS_ERR(info->led_ldo)) {
		if (PTR_ERR(info->led_ldo) == -EPROBE_DEFER) {
			ret = -ENODEV;
			goto err_alloc;	// err_input_alloc;
		}

		dev_err(&client->dev, "%s: Target does not use KEY LED\n", __func__);
		info->led_ldo = NULL;
	}
	if (!IS_ERR_OR_NULL(info->led_ldo)) {
		ret = regulator_set_voltage(info->led_ldo, 3300000, 3300000);
		if (ret) {
			dev_err(&client->dev, "%s: could not set voltage led, ret = %d\n",
				__func__, ret);
		}
	}
#endif

	/* power on */
	if (!zt7538_power_control(info, POWER_ON_SEQUENCE)) {
		ret = -EPERM;
		goto err_power_sequence;
	}

	memset(&info->reported_touch_info, 0x0, sizeof(struct point_info));
	sema_init(&info->work_lock, 1);

	/* init touch mode */
	info->touch_mode = TOUCH_POINT_MODE;
	misc_info = info;

	if (!init_touch(info, false)) {
		ret = -EPERM;
		goto err_input_unregister_device;
	}

	for (i = 0; i < MAX_SUPPORTED_BUTTON_NUM; i++)
		info->button[i] = ICON_BUTTON_UNCHANGE;

	snprintf(info->phys, sizeof(info->phys), "%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->phys = info->phys;
	input_dev->dev.parent = &client->dev;

	set_bit(EV_SYN, info->input_dev->evbit);
	set_bit(EV_KEY, info->input_dev->evbit);
	set_bit(EV_ABS, info->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, info->input_dev->propbit);

#ifdef SUPPORTED_KEY_LED
	set_bit(LED_MISC, info->input_dev->ledbit);
	set_bit(EV_LED, info->input_dev->evbit);
#endif

	for (i = 0; i < MAX_SUPPORTED_BUTTON_NUM; i++)
		set_bit(BUTTON_MAPPING_KEY[i], info->input_dev->keybit);

	if (pdata->orientation & TOUCH_XY_SWAP) {
		input_set_abs_params(info->input_dev, ABS_MT_POSITION_Y,
			info->cap_info.MinX, info->cap_info.MaxX + ABS_PT_OFFSET, 0, 0);
		input_set_abs_params(info->input_dev, ABS_MT_POSITION_X,
			info->cap_info.MinY, info->cap_info.MaxY + ABS_PT_OFFSET, 0, 0);
	} else {
		input_set_abs_params(info->input_dev, ABS_MT_POSITION_X,
			info->cap_info.MinX, info->cap_info.MaxX + ABS_PT_OFFSET, 0, 0);
		input_set_abs_params(info->input_dev, ABS_MT_POSITION_Y,
			info->cap_info.MinY, info->cap_info.MaxY + ABS_PT_OFFSET, 0, 0);
	}

	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

#ifdef SUPPORTED_PALM_TOUCH
	input_set_abs_params(info->input_dev, ABS_MT_TOUCH_MINOR,
			0, 255, 0, 0);
	input_set_abs_params(info->input_dev, ABS_MT_PALM,
				0, 1, 0, 0);
#endif

#ifdef REPORT_2D_Z
	input_set_abs_params(info->input_dev, ABS_MT_PRESSURE,
			0, REAL_Z_MAX, 0, 0);
#endif

	set_bit(MT_TOOL_FINGER, info->input_dev->keybit);
	input_mt_init_slots(info->input_dev, info->cap_info.multi_fingers, 0);

	info->input_dev->open = zt7538_input_open;
	info->input_dev->close = zt7538_input_close;

	input_set_drvdata(info->input_dev, info);
	ret = input_register_device(info->input_dev);
	if (ret) {
		dev_err(&info->client->dev, "unable to register input device\n");
		goto err_input_register_device;
	}

	info->work_state = NOTHING;
	info->finger_cnt = 0;

#if ESD_TIMER_INTERVAL
	spin_lock_init(&info->lock);
	INIT_WORK(&info->tmr_work, ts_tmr_work);
	esd_tmr_workqueue =
		create_singlethread_workqueue("esd_tmr_workqueue");

	if (!esd_tmr_workqueue) {
		dev_err(&client->dev, "Failed to create esd tmr work queue\n");
		ret = -EPERM;
		goto err_esd_input_unregister_device;
	}

	esd_timer_init(info);
	esd_timer_start(CHECK_ESD_TIMER, info);
#endif

#ifdef CONFIG_INPUT_BOOSTER
	info->booster = input_booster_allocate(INPUT_BOOSTER_ID_TSP);
	if (!info->booster) {
		dev_err(&client->dev, "%s: Error, failed to allocate input booster\n",__func__);
		ret = -ENODEV;
		goto error_alloc_booster_failed;
	}
#endif

	info->irq = gpio_to_irq(pdata->gpio_int);
	if (info->irq < 0) {
		dev_err(&client->dev, "failed to get gpio_to_irq\n");
		ret = -ENODEV;
		goto err_gpio_irq;
	}
	ret = request_threaded_irq(info->irq, NULL, zt7538_touch_work,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT , ZT7538_TS_DEVICE, info);

	if (ret) {
		dev_err(&client->dev, "failed to request irq.\n");
		goto err_request_irq;
	}

#if defined(CONFIG_PM_RUNTIME)
	pm_runtime_enable(&client->dev);
#endif

	sema_init(&info->raw_data_lock, 1);

#if ZINITIX_MISC_DEBUG
	ret = misc_register(&touch_misc_device);
	if (ret) {
		dev_err(&client->dev, "Failed to register touch misc device\n");
		goto err_misc_register;
	}
#endif

#ifdef SEC_FACTORY_TEST
	ret = init_sec_factory(info);
	if (ret) {
		dev_err(&client->dev, "Failed to init sec factory device\n");
		goto err_kthread_create_failed;
	}
	info->factory_info->cmd_param[0] = 1;
	hfdnd_spec_adjust(info);
	info->factory_info->cmd_state = WAITING;
#endif

#ifdef TSP_MUIC_NOTIFICATION
	muic_notifier_register(&info->charger_nb,
		zt7538_notification, MUIC_NOTIFY_DEV_TSP);
#endif

	dev_info(&client->dev, "zinitix touch probe done.\n");

	return 0;

#ifdef SEC_FACTORY_TEST
err_kthread_create_failed:
	devm_kfree(&client->dev, info->factory_info);
	devm_kfree(&client->dev, info->raw_data);
#endif
#if ZINITIX_MISC_DEBUG
err_misc_register:
#endif
	free_irq(info->irq, info);
err_request_irq:
err_gpio_irq:
#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(info->booster);
	info->booster = NULL;
error_alloc_booster_failed:
#endif
#if ESD_TIMER_INTERVAL
err_esd_input_unregister_device:
#endif
	input_unregister_device(info->input_dev);
err_input_unregister_device:
err_input_register_device:
	zt7538_power_control(info, POWER_OFF);
err_power_sequence:
	input_free_device(info->input_dev);
err_alloc:
	devm_kfree(&client->dev, info);
err_no_platform_data:
	if (IS_ENABLED(CONFIG_OF))
		devm_kfree(&client->dev, (void *)pdata);

	dev_info(&client->dev, "Failed to probe\n");
	return ret;
}

static int zt7538_ts_remove(struct i2c_client *client)
{
	struct zt7538_ts_info *info = i2c_get_clientdata(client);
	struct zt7538_ts_dt_data *pdata = info->pdata;

	disable_irq(info->irq);
	down(&info->work_lock);

	info->work_state = REMOVE;

#ifdef SEC_FACTORY_TEST
	devm_kfree(&client->dev, info->factory_info);
	devm_kfree(&client->dev, info->raw_data);
#endif
#if ESD_TIMER_INTERVAL
	flush_work(&info->tmr_work);
	write_reg(info->client, ZT7538_PERIODICAL_INTERRUPT_INTERVAL, 0);
	esd_timer_stop(info);
	destroy_workqueue(esd_tmr_workqueue);
#endif

	if (info->irq)
		free_irq(info->irq, info);

#if ZINITIX_MISC_DEBUG
	misc_deregister(&touch_misc_device);
#endif

	if (gpio_is_valid(pdata->gpio_int) != 0)
		gpio_free(pdata->gpio_int);

	input_unregister_device(info->input_dev);
	input_free_device(info->input_dev);
	up(&info->work_lock);
	devm_kfree(&client->dev, info);

	return 0;
}

void zt7538_ts_shutdown(struct i2c_client *client)
{
	struct zt7538_ts_info *info = i2c_get_clientdata(client);
	disable_irq(info->irq);
	down(&info->work_lock);
#if ESD_TIMER_INTERVAL
	flush_work(&info->tmr_work);
	esd_timer_stop(info);
#endif
	up(&info->work_lock);
	zt7538_power_control(info, POWER_OFF);
}

static struct i2c_device_id zt7538_idtable[] = {
	{ZT7538_TS_DEVICE, 0},
	{ }
};

static struct i2c_driver zt7538_ts_driver = {
	.probe	= zt7538_ts_probe,
	.remove	= zt7538_ts_remove,
	.shutdown = zt7538_ts_shutdown,
	.id_table	= zt7538_idtable,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= ZT7538_TS_DEVICE,
		.of_match_table = tsp_dt_ids,
	},
};

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif

extern int get_lcd_attached(char *mode);
static int __init zt7538_ts_init(void)
{
	printk("zt7538_ts_init\n");

#if defined(CONFIG_SAMSUNG_LPM_MODE)
	if (poweroff_charging) {
		pr_notice("%s : LPM Charging Mode!!\n", __func__);
		return 0;
	}
#endif

	if (!get_lcd_attached("GET")) {
		pr_notice("%s: LCD is not attached\n", __func__);
		return 0;
	}

	return i2c_add_driver(&zt7538_ts_driver);
}

static void __exit zt7538_ts_exit(void)
{
	i2c_del_driver(&zt7538_ts_driver);
}

module_init(zt7538_ts_init);
module_exit(zt7538_ts_exit);

MODULE_DESCRIPTION("touch-screen device driver using i2c interface");
MODULE_AUTHOR("<mika.kim@samsung.com>");
MODULE_LICENSE("GPL");
