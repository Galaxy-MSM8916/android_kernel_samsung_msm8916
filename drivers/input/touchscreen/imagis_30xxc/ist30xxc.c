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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/input/mt.h>

#include "ist30xxc.h"
#include "ist30xxc_update.h"
#include "ist30xxc_tracking.h"
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#endif

#if IST30XX_DEBUG
#include "ist30xxc_misc.h"
#endif

#if IST30XX_CMCS_TEST
#include "ist30xxc_cmcs.h"
#endif
#if IST30XX_CMCS_JIT_TEST
#include "ist30xxc_cmcs_jit.h"
#endif

#ifdef CONFIG_DUAL_TOUCH_IC_CHECK
static int probe_finished = 0;
#endif

#define J5_100_OHM_VALUE    0xECEC0001

#define MAX_ERR_CNT			(100)
#define EVENT_TIMER_INTERVAL		(HZ * timer_period_ms / 1000)
u32 event_ms = 0, timer_ms = 0;
static struct timer_list event_timer;
static struct timespec t_current;	/* nano seconds */
int timer_period_ms = 500;		/* 500 msec */

#if IST30XX_USE_KEY
int ist30xx_key_code[IST30XX_MAX_KEYS + 1] = { 0, KEY_RECENT, KEY_BACK, };
#endif

struct ist30xx_data *ts_data;

DEFINE_MUTEX(ist30xx_mutex);

int ist30xx_dbg_level = IST30XX_DEBUG_LEVEL;
void tsp_printk(int level, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	if (ist30xx_dbg_level < level)
		return;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	printk("%s %pV", IST30XX_DEBUG_TAG, &vaf);

	va_end(args);
}

long get_milli_second(void)
{
	ktime_get_ts(&t_current);

	return t_current.tv_sec * 1000 + t_current.tv_nsec / 1000000;
}

void ist30xx_delay(unsigned int ms)
{
	if (ms < 20)
		usleep_range(ms * 1000, ms * 1000);
	else
		msleep(ms);

}

int ist30xx_intr_wait(struct ist30xx_data *data, long ms)
{
	long start_ms = get_milli_second();
	long curr_ms = 0;

	while (1) {
		if (!data->irq_working)
			break;

		curr_ms = get_milli_second();
		if ((curr_ms < 0) || (start_ms < 0) || (curr_ms - start_ms > ms)) {
			tsp_info("%s() timeout(%dms)\n", __func__, ms);
			return -EPERM;
		}

		ist30xx_delay(2);
	}
	return 0;
}

void ist30xx_disable_irq(struct ist30xx_data *data)
{
	if (likely(data->irq_enabled)) {
		ist30xx_tracking(TRACK_INTR_DISABLE);
		disable_irq(data->client->irq);
		data->irq_enabled = 0;
		data->status.event_mode = false;
	}
}

void ist30xx_enable_irq(struct ist30xx_data *data)
{
	if (likely(!data->irq_enabled)) {
		ist30xx_tracking(TRACK_INTR_ENABLE);
		enable_irq(data->client->irq);
		ist30xx_delay(10);
		data->irq_enabled = 1;
		data->status.event_mode = true;
	}
}

void ist30xx_scheduled_reset(struct ist30xx_data *data)
{
	if (likely(data->initialized))
		schedule_delayed_work(&data->work_reset_check, 0);
}

static void ist30xx_request_reset(struct ist30xx_data *data)
{
	data->irq_err_cnt++;
	if (unlikely(data->irq_err_cnt >= data->max_irq_err_cnt)) {
		tsp_info("%s()\n", __func__);
		ist30xx_scheduled_reset(data);
		data->irq_err_cnt = 0;
	}
}

#define NOISE_MODE_TA		(0)
#define NOISE_MODE_CALL		(1)
#define NOISE_MODE_COVER	(2)
#define NOISE_MODE_EDGE		(4)
#define NOISE_MODE_POWER	(8)
void ist30xx_start(struct ist30xx_data *data)
{
	if (data->initialized) {
		data->scan_count = 0;
		data->scan_retry = 0;
		mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL * 2);
	}
	/* TA mode */
	ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_MODE_SPECIAL << 16) | (data->noise_mode & 0xFFFF)));

	/* Local Model info. do not use this. */
/*
	ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_LOCAL_MODEL << 16) | (TSP_LOCAL_CODE & 0xFFFF)));
	tsp_info("%s(), local : %d, mode : 0x%x\n", __func__,
			TSP_LOCAL_CODE & 0xFFFF, data->noise_mode & 0xFFFF);
*/

	if (data->report_rate >= 0) {
		ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
				((eHCOM_SET_TIME_ACTIVE << 16) | (data->report_rate & 0xFFFF)));
		tsp_info("%s: active rate : %dus\n", __func__, data->report_rate);
	}

	if (data->idle_rate >= 0) {
		ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
				((eHCOM_SET_TIME_IDLE << 16) | (data->idle_rate & 0xFFFF)));
		tsp_info("%s: idle rate : %dus\n", __func__, data->idle_rate);
	}

	if (data->jig_mode) {
		ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
				(eHCOM_SET_JIG_MODE << 16) | (IST30XX_JIG_TOUCH & 0xFFFF));
		tsp_info("%s: jig mode start\n", __func__);
	}

	if (data->debug_mode || data->jig_mode) {
		ist30xx_write_cmd(data->client, IST30XX_HIB_CMD,
				(eHCOM_SLEEP_MODE_EN << 16) | (IST30XX_DISABLE & 0xFFFF));
		tsp_info("%s: debug mode start\n", __func__);
	}

	ist30xx_cmd_start_scan(data);
}

int ist30xx_get_ver_info(struct ist30xx_data *data)
{
	int ret;

	data->fw.prev.main_ver = data->fw.cur.main_ver;
	data->fw.prev.fw_ver = data->fw.cur.fw_ver;
	data->fw.prev.core_ver = data->fw.cur.core_ver;
	data->fw.prev.test_ver = data->fw.cur.test_ver;
	data->fw.cur.main_ver = 0;
	data->fw.cur.fw_ver = 0;
	data->fw.cur.core_ver = 0;
	data->fw.cur.test_ver = 0;

	ret = ist30xx_cmd_hold(data, 1);
	if (unlikely(ret))
		return ret;

	ret = ist30xx_read_reg(data->client,
			IST30XX_DA_ADDR(eHCOM_GET_VER_MAIN), &data->fw.cur.main_ver);
	if (unlikely(ret))
		goto err_get_ver;

	ret = ist30xx_read_reg(data->client,
			IST30XX_DA_ADDR(eHCOM_GET_VER_FW), &data->fw.cur.fw_ver);
	if (unlikely(ret))
		goto err_get_ver;

	ret = ist30xx_read_reg(data->client,
			IST30XX_DA_ADDR(eHCOM_GET_VER_CORE), &data->fw.cur.core_ver);
	if (unlikely(ret))
		goto err_get_ver;

	ret = ist30xx_read_reg(data->client,
			IST30XX_DA_ADDR(eHCOM_GET_VER_TEST), &data->fw.cur.test_ver);
	if (unlikely(ret))
		goto err_get_ver;

	ret = ist30xx_cmd_hold(data, 0);
	if (unlikely(ret))
		goto err_get_ver;

	tsp_info("IC version main: %x, fw: %x, test: %x, core: %x\n",
			data->fw.cur.main_ver, data->fw.cur.fw_ver, data->fw.cur.test_ver,
			data->fw.cur.core_ver);

	return 0;

err_get_ver:
	ist30xx_reset(data, false);

	return ret;
}

#define CALIB_MSG_MASK		(0xF0000FFF)
#define CALIB_MSG_VALID		(0x80000CAB)
#define TRACKING_INTR_VALID	(0x127EA597)
u32 tracking_intr_value = TRACKING_INTR_VALID;
int ist30xx_get_info(struct ist30xx_data *data)
{
	int ret;
	u32 calib_msg;
	u32 ms;

	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(data);

#if IST30XX_INTERNAL_BIN
#if IST30XX_UPDATE_BY_WORKQUEUE
	ist30xx_get_update_info(data, data->fw.buf, data->fw.buf_size);
#endif
	ist30xx_get_tsp_info(data);
#else
	ret = ist30xx_get_ver_info(data);
	if (unlikely(ret))
		goto get_info_end;

	ret = ist30xx_get_tsp_info(data);
	if (unlikely(ret))
		goto get_info_end;
#endif /* IST30XX_INTERNAL_BIN */

/* remove "print information" -> read sysfs : /sys/class/touch/sys/ic_inform */
/*
	ist30xx_print_info(data);
*/
	ret = ist30xx_read_cmd(data, eHCOM_GET_CAL_RESULT, &calib_msg);
	if (likely(ret == 0)) {
		tsp_info("calib status: 0x%08x\n", calib_msg);
		ms = get_milli_second();
		ist30xx_put_track_ms(ms);
		ist30xx_put_track(&tracking_intr_value, 1);
		ist30xx_put_track(&calib_msg, 1);
		if ((calib_msg & CALIB_MSG_MASK) != CALIB_MSG_VALID ||
				CALIB_TO_STATUS(calib_msg) > 0) {
			ist30xx_calibrate(data, IST30XX_MAX_RETRY_CNT);
		}
	}

#if IST30XX_CHECK_CALIB
	if (likely(!data->status.update)) {
		ret = ist30xx_cmd_check_calib(data->client);
		if (likely(!ret)) {
			data->status.calib = 1;
			data->status.calib_msg = 0;
			event_ms = (u32)get_milli_second();
			data->status.event_mode = true;
		}
	}
#else
	ist30xx_start(data);
#endif

#if !(IST30XX_INTERNAL_BIN)
get_info_end:
#endif

	ist30xx_enable_irq(data);
	mutex_unlock(&ist30xx_mutex);

	return ret;
}

#if IST30XX_GESTURE
#define GESTURE_MAGIC_STRING		(0x4170CF00)
#define GESTURE_MAGIC_MASK		(0xFFFFFF00)
#define GESTURE_MESSAGE_MASK		(~GESTURE_MAGIC_MASK)
#define PARSE_GESTURE_MESSAGE(n) \
	((n & GESTURE_MAGIC_MASK) == GESTURE_MAGIC_STRING ? \
	 (n & GESTURE_MESSAGE_MASK) : -EINVAL)
void ist30xx_gesture_cmd(struct ist30xx_data *data, int cmd)
{
	tsp_info("Gesture cmd: %d\n", cmd);

	switch (cmd) {
	case 0x01:
		input_report_key(data->input_dev, KEY_VOLUMEUP, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_VOLUMEUP, 0);
		input_sync(data->input_dev);
		break;

	case 0x02:
		input_report_key(data->input_dev, KEY_VOLUMEDOWN, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_VOLUMEDOWN, 0);
		input_sync(data->input_dev);
		break;

	case 0x03:
		input_report_key(data->input_dev, KEY_PREVIOUSSONG, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_PREVIOUSSONG, 0);
		input_sync(data->input_dev);
		break;

	case 0x04:
		input_report_key(data->input_dev, KEY_NEXTSONG, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_NEXTSONG, 0);
		input_sync(data->input_dev);
		break;

	case 0x11:
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 0);
		input_sync(data->input_dev);
		break;

	case 0x12:
		input_report_key(data->input_dev, KEY_SCREENLOCK, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_SCREENLOCK, 0);
		input_sync(data->input_dev);
		break;

	case 0x13:
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 0);
		input_sync(data->input_dev);
		break;

	case 0x14:
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_PLAYPAUSE, 0);
		input_sync(data->input_dev);
		break;

	case 0x21:
		input_report_key(data->input_dev, KEY_MUTE, 1);
		input_sync(data->input_dev);
		input_report_key(data->input_dev, KEY_MUTE, 0);
		input_sync(data->input_dev);
		break;

	default:
		break;
	}
}
#endif

#define PRESS_MSG_MASK		(0x01)
#define MULTI_MSG_MASK		(0x02)
#define TOUCH_DOWN_MESSAGE	("p")
#define TOUCH_UP_MESSAGE	("r")
#define TOUCH_MOVE_MESSAGE	(" ")
bool tsp_touched[IST30XX_MAX_MT_FINGERS] = { false, };
void print_tsp_event(struct ist30xx_data *data, finger_info *finger)
{
	int idx = finger->bit_field.id - 1;
	bool press;

	press = PRESSED_FINGER(data->t_status, finger->bit_field.id);

	if (press) {
		if (tsp_touched[idx] == false) {
			/* touch down */
			data->touch_pressed_num++;
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			tsp_noti("%s%d (%d, %d)(%d)\n",
					TOUCH_DOWN_MESSAGE, finger->bit_field.id,
					finger->bit_field.x, finger->bit_field.y, data->z_values[idx]);
#else
			tsp_noti("%s%d\n", TOUCH_DOWN_MESSAGE, finger->bit_field.id);
#endif
			tsp_touched[idx] = true;
		} else {
			/* touch move */
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			tsp_debug("%s%d (%d, %d)(%d)\n",
					TOUCH_MOVE_MESSAGE, finger->bit_field.id,
					finger->bit_field.x, finger->bit_field.y, data->z_values[idx]);
#endif
		}

		data->lx = finger->bit_field.x;
		data->ly = finger->bit_field.y;
	} else {
		if (tsp_touched[idx] == true) {
			/* touch up */
			data->touch_pressed_num--;
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			tsp_noti("%s%d(%d, %d)\n", TOUCH_UP_MESSAGE, finger->bit_field.id, data->lx, data->ly);
#else
			tsp_noti("%s%d\n", TOUCH_UP_MESSAGE, finger->bit_field.id);
#endif
			tsp_touched[idx] = false;

			data->lx = 0;
			data->ly = 0;
		}
	}
}
#if IST30XX_USE_KEY
#define PRESS_MSG_KEY           (0x06)
bool tkey_pressed[IST30XX_MAX_KEYS] = { false, };

/* key product_ship log will be add after unpin feature */
void print_tkey_event(struct ist30xx_data *data, int id)
{
	int idx = id - 1;
	bool press = PRESSED_KEY(data->t_status, id);

	if (press) {
		if (tkey_pressed[idx] == false) {
			/* tkey down */
			tsp_noti("k %s%d\n", TOUCH_DOWN_MESSAGE, id);
			tkey_pressed[idx] = true;
		}
	} else {
		if (tkey_pressed[idx] == true) {
			/* tkey up */
			tsp_noti("k %s%d\n", TOUCH_UP_MESSAGE, id);
			tkey_pressed[idx] = false;
		}
	}
}
#endif
static void release_finger(struct ist30xx_data *data, int id)
{
	input_mt_slot(data->input_dev, id - 1);
	input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, false);

	ist30xx_tracking(TRACK_POS_FINGER + id);
	tsp_info("%s() %d\n", __func__, id);

	tsp_touched[id - 1] = false;

	input_sync(data->input_dev);
}

#if IST30XX_USE_KEY
#define CANCEL_KEY  (0xFF)
#define RELEASE_KEY (0)
static void release_key(struct ist30xx_data *data, int id, u8 key_status)
{
	input_report_key(data->input_dev, ist30xx_key_code[id], key_status);

	ist30xx_tracking(TRACK_POS_KEY + id);
	tsp_info("%s() key%d, status: %d\n", __func__, id, key_status);

	tkey_pressed[id - 1] = false;

	input_sync(data->input_dev);
}
#endif
static void clear_input_data(struct ist30xx_data *data)
{
	int id = 1;
	u32 status;

	input_report_key(data->input_dev, BTN_TOUCH, 0);

	status = PARSE_FINGER_STATUS(data->t_status);
	while (status) {
		if (status & 1)
			release_finger(data, id);
		status >>= 1;
		id++;
	}
#if IST30XX_USE_KEY
	id = 1;
	status = PARSE_KEY_STATUS(data->t_status);
	while (status) {
		if (status & 1)
			release_key(data, id, RELEASE_KEY);
		status >>= 1;
		id++;
	}
#endif

#ifdef CONFIG_INPUT_BOOSTER
	if (data->tsp_booster && data->tsp_booster->dvfs_set)
		data->tsp_booster->dvfs_set(data->tsp_booster, -1);
	data->touch_pressed_num = 0;
#endif

	data->t_status = 0;
}

static int check_report_fingers(struct ist30xx_data *data, int finger_counts)
{
	int i;
	finger_info *fingers = (finger_info *)data->fingers;

	/* current finger info */
	for (i = 0; i < finger_counts; i++) {
		if (unlikely((fingers[i].bit_field.x >= data->max_x) ||
					(fingers[i].bit_field.y >= data->max_y))) {
			tsp_warn("Invalid touch data - %d: %d(%d, %d), 0x%08x\n", i,
					fingers[i].bit_field.id,
					fingers[i].bit_field.x,
					fingers[i].bit_field.y,
					fingers[i].full_field);

			fingers[i].bit_field.id = 0;
			ist30xx_tracking(TRACK_POS_UNKNOWN);
			return -EPERM;
		}
	}

	return 0;
}

static int check_valid_coord(u32 *msg, int cnt)
{
	u8 *buf = (u8 *)msg;
	u8 chksum1 = msg[0] >> 24;
	u8 chksum2 = 0;
	u32 tmp = msg[0];

	msg[0] &= 0x00FFFFFF;

	cnt *= IST30XX_DATA_LEN;

	while (cnt--)
		chksum2 += *buf++;

	msg[0] = tmp;

	if (chksum1 != chksum2) {
		tsp_err("intr chksum: %02x, %02x\n", chksum1, chksum2);
		return -EPERM;
	}

	return 0;
}

static void report_input_data(struct ist30xx_data *data, int finger_counts,
		int key_counts)
{
	int id;
	bool press = false;
	finger_info *fingers = (finger_info *)data->fingers;
	u32 *z_values = (u32 *)data->z_values;
	int idx = 0;
	u32 status;

	status = PARSE_FINGER_STATUS(data->t_status);
	for (id = 0; id < IST30XX_MAX_MT_FINGERS; id++) {
		press = (status & (1 << id)) ? true : false;

		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, press);

		fingers[idx].bit_field.id = id + 1;
		print_tsp_event(data, &fingers[idx]);

		if (press == false)
			continue;

		input_report_abs(data->input_dev, ABS_MT_POSITION_X,
				fingers[idx].bit_field.x);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
				fingers[idx].bit_field.y);
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
				fingers[idx].bit_field.area);
		if (data->jig_mode)
			input_report_abs(data->input_dev, ABS_MT_PRESSURE, z_values[idx]);

		input_report_key(data->input_dev, BTN_TOUCH, 1);

		idx++;
	}

#if IST30XX_USE_KEY
	status = PARSE_KEY_STATUS(data->t_status);
	for (id = 0; id < IST30XX_MAX_KEYS; id++) {
		press = (status & (1 << id)) ? true : false;

		input_report_key(data->input_dev, ist30xx_key_code[id + 1], press);

		print_tkey_event(data, id + 1);
	}
#endif /* IST30XX_USE_KEY */

	if (finger_counts == 0)
		input_report_key(data->input_dev, BTN_TOUCH, 0);

	data->irq_err_cnt = 0;
	data->scan_retry = 0;

	input_sync(data->input_dev);
}

/*
 * CMD : CMD_GET_COORD
 *
 *   1st  [31:24]   [23:21]   [20:16]   [15:12]   [11:10]   [9:0]
 *        Checksum  KeyCnt    KeyStatus FingerCnt Rsvd.     FingerStatus
 *   2nd  [31:28]   [27:24]   [23:12]   [11:0]
 *        ID        Area      X         Y
 */
#define TRACKING_INTR_DEBUG1_VALID	(0x127A6E81)
#define TRACKING_INTR_DEBUG2_VALID	(0x127A6E82)
#define TRACKING_INTR_DEBUG3_VALID	(0x127A6E83)
u32 tracking_intr_debug_value = 0;
u32 intr_debug_addr, intr_debug2_addr, intr_debug3_addr = 0;
u32 intr_debug_size, intr_debug2_size, intr_debug3_size = 0;
static irqreturn_t ist30xx_irq_thread(int irq, void *ptr)
{
	int i, ret;
	int key_cnt, finger_cnt, read_cnt;
	struct ist30xx_data *data = (struct ist30xx_data *)ptr;
	int offset = 1;
#if IST30XX_STATUS_DEBUG
	u32 touch_status;
#endif
	u32	t_status;
	u32 msg[IST30XX_MAX_MT_FINGERS * 2 + offset];
	u32 ms;

	data->irq_working = true;

	if (unlikely(!data->irq_enabled))
		goto irq_end;

	if (data->track_enable) {
		ms = get_milli_second();

		if (intr_debug_addr >= 0 && intr_debug_size > 0) {
			tsp_noti("Intr_debug (addr: 0x%08x)\n", intr_debug_addr);
			ist30xx_burst_read(data->client, IST30XX_DA_ADDR(intr_debug_addr),
					&msg[0], intr_debug_size, true);

			for (i = 0; i < intr_debug_size; i++)
				tsp_noti("\t%08x\n", msg[i]);

			tracking_intr_debug_value = TRACKING_INTR_DEBUG1_VALID;
			ist30xx_put_track_ms(ms);
			ist30xx_put_track(&tracking_intr_debug_value, 1);
			ist30xx_put_track(msg, intr_debug_size);
		}

		if (intr_debug2_addr >= 0 && intr_debug2_size > 0) {
			tsp_noti("Intr_debug2 (addr: 0x%08x)\n", intr_debug2_addr);
			ist30xx_burst_read(data->client, IST30XX_DA_ADDR(intr_debug2_addr),
					&msg[0], intr_debug2_size, true);

			for (i = 0; i < intr_debug2_size; i++)
				tsp_noti("\t%08x\n", msg[i]);

			tracking_intr_debug_value = TRACKING_INTR_DEBUG2_VALID;
			ist30xx_put_track_ms(ms);
			ist30xx_put_track(&tracking_intr_debug_value, 1);
			ist30xx_put_track(msg, intr_debug2_size);
		}
	}

	memset(msg, 0, sizeof(msg));

	ret = ist30xx_read_reg(data->client, IST30XX_HIB_INTR_MSG, msg);
	if (unlikely(ret))
		goto irq_err;

	tsp_verb("intr msg: 0x%08x\n", *msg);

	/* TSP IC Exception */
	if (unlikely((*msg & IST30XX_EXCEPT_MASK) == IST30XX_EXCEPT_VALUE)) {
		tsp_err("Occured IC exception(0x%02X)\n", *msg & 0xFF);
#if I2C_BURST_MODE
		ret = ist30xx_burst_read(data->client,
				IST30XX_HIB_COORD, &msg[offset], IST30XX_MAX_EXCEPT_SIZE, true);
#else
		for (i = 0; i < IST30XX_MAX_EXCEPT_SIZE; i++) {
			ret = ist30xx_read_reg(data->client,
					IST30XX_HIB_COORD + (i * 4), &msg[i + offset]);
		}
#endif
		if (unlikely(ret))
			tsp_err(" exception value read error(%d)\n", ret);
		else
			tsp_err(" exception value : 0x%08X, 0x%08X\n", msg[1], msg[2]);

		goto irq_ic_err;
	}

	if (unlikely(*msg == 0 || *msg == 0xFFFFFFFF)) /* Unknown CMD */
		goto irq_err;

	if (data->track_enable) {
		event_ms = ms;
		ist30xx_put_track_ms(event_ms);
		ist30xx_put_track(&tracking_intr_value, 1);
		ist30xx_put_track(msg, 1);
	}

	if (unlikely((*msg & CALIB_MSG_MASK) == CALIB_MSG_VALID)) {
		data->status.calib_msg = *msg;
		tsp_info("calib status: 0x%08x\n", data->status.calib_msg);

		goto irq_end;
	}

#if IST30XX_CMCS_TEST
	if (unlikely(*msg == IST30XX_CMCS_MSG_VALID)) {
		data->status.cmcs = 1;
		tsp_info("cmcs status: 0x%08x\n", *msg);

		goto irq_end;
	}
#endif

#if IST30XX_CMCS_JIT_TEST
	if (((*msg & CMCS_MSG_MASK) == CM_MSG_VALID) || ((*msg & CMCS_MSG_MASK) == CS_MSG_VALID)) {
		data->status.cmcs = *msg;
		tsp_info("cmcs status: 0x%08x\n", *msg);

		goto irq_end;
	}
#endif

#if IST30XX_GESTURE
	ret = PARSE_GESTURE_MESSAGE(*msg);
	if (unlikely(ret > 0)) {
		tsp_info("Gesture ID: %d (0x%08x)\n", ret, *msg);
		ist30xx_gesture_cmd(data, ret);

		goto irq_end;
	}
#endif

#if IST30XX_STATUS_DEBUG
	ret = ist30xx_read_reg(data->client,
			IST30XX_HIB_TOUCH_STATUS, &touch_status);

	if (ret == 0)
		tsp_debug("ALG : 0x%08X\n", touch_status);

	memset(data->fingers, 0, sizeof(data->fingers));
#endif

	/* Unknown interrupt data for extend coordinate */
	if (unlikely(!CHECK_INTR_STATUS(*msg)))
		goto irq_err;

	t_status = *msg;
	key_cnt = PARSE_KEY_CNT(t_status);
	finger_cnt = PARSE_FINGER_CNT(t_status);

	if (unlikely((finger_cnt > data->max_fingers) ||
				(key_cnt > data->max_keys))) {
		tsp_warn("Invalid touch count - finger: %d(%d), key: %d(%d)\n",
				finger_cnt, data->max_fingers, key_cnt, data->max_keys);
		goto irq_err;
	}

	if (finger_cnt > 0) {
#if I2C_BURST_MODE
		ret = ist30xx_burst_read(data->client,
				IST30XX_HIB_COORD, &msg[offset], finger_cnt, true);
		if (unlikely(ret))
			goto irq_err;

		for (i = 0; i < finger_cnt; i++)
			data->fingers[i].full_field = msg[i + offset];
#else
		for (i = 0; i < finger_cnt; i++) {
			ret = ist30xx_read_reg(data->client,
					IST30XX_HIB_COORD + (i * 4), &msg[i + offset]);
			if (unlikely(ret))
				goto irq_err;

			data->fingers[i].full_field = msg[i + offset];
		}
#endif /* I2C_BURST_MODE */

		if (data->jig_mode) {
			/* z-values ram address define */
			ret = ist30xx_burst_read(data->client,
					IST30XX_DA_ADDR(data->tags.zvalue_base),
					data->z_values, finger_cnt, true);
		}

		if (data->track_enable) {

			ist30xx_put_track(msg + offset, finger_cnt);
			for (i = 0; i < finger_cnt; i++) {
				tsp_verb("intr msg(%d): 0x%08x, %d\n",
						i + offset, msg[i + offset], data->z_values[i]);
			}

		}
	}

	read_cnt = finger_cnt + 1;

	ret = check_valid_coord(&msg[0], read_cnt);
	if (unlikely(ret))
		goto irq_err;

	if (data->track_enable) {
		ret = check_report_fingers(data, finger_cnt);
		if (unlikely(ret))
			goto irq_err;
	}

	data->t_status = t_status;
	report_input_data(data, finger_cnt, key_cnt);

#ifdef CONFIG_INPUT_BOOSTER
	if (data->tsp_booster && data->tsp_booster->dvfs_set)
		data->tsp_booster->dvfs_set(data->tsp_booster, !!data->touch_pressed_num);
#endif

	if (data->track_enable) {
		if (intr_debug3_addr >= 0 && intr_debug3_size > 0) {
			tsp_noti("Intr_debug3 (addr: 0x%08x)\n", intr_debug3_addr);
			ist30xx_burst_read(data->client, IST30XX_DA_ADDR(intr_debug3_addr),
					&msg[0], intr_debug3_size, true);

			for (i = 0; i < intr_debug3_size; i++)
				tsp_noti("\t%08x\n", msg[i]);

			tracking_intr_debug_value = TRACKING_INTR_DEBUG3_VALID;
			ist30xx_put_track_ms(ms);
			ist30xx_put_track(&tracking_intr_debug_value, 1);
			ist30xx_put_track(msg, intr_debug3_size);
		}

	}
	goto irq_end;

irq_err:
	tsp_err("intr msg: 0x%08x, ret: %d\n", msg[0], ret);
	ist30xx_request_reset(data);
irq_end:
	data->irq_working = false;

	if (data->track_enable)
		event_ms = (u32)get_milli_second();

	return IRQ_HANDLED;

irq_ic_err:
	ist30xx_scheduled_reset(data);
	data->irq_working = false;
	if (data->track_enable)
		event_ms = (u32)get_milli_second();

	return IRQ_HANDLED;
}

static int ist30xx_pinctrl_configure(struct ist30xx_data *data, bool active)
{
	struct pinctrl_state *set_state;

	int retval;
	tsp_err("%s: %s\n", __func__, active ? "ACTIVE" : "SUSPEND");

	set_state = pinctrl_lookup_state(data->pinctrl, active ? "active_state" : "suspend_state");
	if (IS_ERR(set_state)) {
		tsp_err("%s: cannot get active state\n", __func__);
		return -EINVAL;
	}

	retval = pinctrl_select_state(data->pinctrl, set_state);
	if (retval) {
		tsp_err("%s: cannot set pinctrl %s state\n",
				__func__, active ? "active" : "suspend");
		return -EINVAL;
	}

	return 0;
}

static int ist30xx_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ist30xx_data *data = i2c_get_clientdata(client);

	del_timer(&event_timer);
	cancel_delayed_work_sync(&data->work_noise_protect);
	cancel_delayed_work_sync(&data->work_reset_check);
	cancel_delayed_work_sync(&data->work_debug_algorithm);
	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(data);
	ist30xx_internal_suspend(data);
	clear_input_data(data);
#if IST30XX_GESTURE
	if (data->gesture) {
		ist30xx_start(data);
		data->status.noise_mode = false;
		ist30xx_enable_irq(data);
	}
#endif
	mutex_unlock(&ist30xx_mutex);

	return 0;
}

static int ist30xx_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ist30xx_data *data = i2c_get_clientdata(client);

	data->noise_mode |= (1 << NOISE_MODE_POWER);

	mutex_lock(&ist30xx_mutex);
	ist30xx_internal_resume(data);
	ist30xx_start(data);
	ist30xx_enable_irq(data);
	mutex_unlock(&ist30xx_mutex);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ist30xx_early_suspend(struct early_suspend *h)
{
	struct ist30xx_data *data = container_of(h, struct ist30xx_data,
			early_suspend);

	ist30xx_suspend(&data->client->dev);
}
static void ist30xx_late_resume(struct early_suspend *h)
{
	struct ist30xx_data *data = container_of(h, struct ist30xx_data,
			early_suspend);

	ist30xx_resume(&data->client->dev);
}
#endif

static void  ist30xx_ts_close(struct input_dev *dev)
{
	struct ist30xx_data *data = input_get_drvdata(dev);

	if (data->touch_stopped) {
		tsp_err("%s: IC is already on\n", __func__);
		return;
	}

	tsp_info("%s\n", __func__);
	data->touch_stopped = true;
	ist30xx_suspend(&data->client->dev);

	if (data->pinctrl) {
		int ret = ist30xx_pinctrl_configure(data, false);
		if (ret)
			tsp_err("%s: cannot set pinctrl state\n", __func__);
	}

}
static int ist30xx_ts_open(struct input_dev *dev)
{
	struct ist30xx_data *data = input_get_drvdata(dev);

	if (!data->touch_stopped) {
		tsp_err("%s: IC is already on\n", __func__);
		return 0;
	}
	tsp_info("%s\n", __func__);

	if (data->pinctrl) {
		int ret = ist30xx_pinctrl_configure(data, true);
		if (ret)
			tsp_err("%s: cannot set pinctrl state\n", __func__);
	}

	ist30xx_resume(&data->client->dev);
	data->touch_stopped = false;

	tsp_info("%s: IC version fw(main, test, core): %x(%x, %x, %x)\n", __func__,
			data->fw.cur.fw_ver, data->fw.cur.main_ver,
			data->fw.cur.test_ver, data->fw.cur.core_ver);

	return 0;
}

void ist30xx_set_ta_mode(bool mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (unlikely(mode == ((ts_data->noise_mode >> NOISE_MODE_TA) & 1)))
		return;

	if (mode)
		ts_data->noise_mode |= (1 << NOISE_MODE_TA);
	else
		ts_data->noise_mode &= ~(1 << NOISE_MODE_TA);

#if IST30XX_TA_RESET
	if (unlikely(ts_data->initialized))
		ist30xx_scheduled_reset(ts_data);
#else
	ist30xx_write_cmd(ts_data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_MODE_SPECIAL << 16) | (ts_data->noise_mode & 0xFFFF)));
#endif

	ist30xx_tracking(mode ? TRACK_CMD_TACON : TRACK_CMD_TADISCON);
}
EXPORT_SYMBOL(ist30xx_set_ta_mode);

void ist30xx_set_edge_mode(int mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (mode)
		ts_data->noise_mode |= (1 << NOISE_MODE_EDGE);
	else
		ts_data->noise_mode &= ~(1 << NOISE_MODE_EDGE);

	ist30xx_write_cmd(ts_data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_MODE_SPECIAL << 16) | (ts_data->noise_mode & 0xFFFF)));
}
EXPORT_SYMBOL(ist30xx_set_edge_mode);

void ist30xx_set_call_mode(int mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (unlikely(mode == ((ts_data->noise_mode >> NOISE_MODE_CALL) & 1)))
		return;

	if (mode)
		ts_data->noise_mode |= (1 << NOISE_MODE_CALL);
	else
		ts_data->noise_mode &= ~(1 << NOISE_MODE_CALL);

#if IST30XX_TA_RESET
	if (unlikely(ts_data->initialized))
		ist30xx_scheduled_reset(ts_data);
#else
	ist30xx_write_cmd(ts_data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_MODE_SPECIAL << 16) | (ts_data->noise_mode & 0xFFFF)));
#endif

	ist30xx_tracking(mode ? TRACK_CMD_CALL : TRACK_CMD_NOTCALL);
}
EXPORT_SYMBOL(ist30xx_set_call_mode);

void ist30xx_set_cover_mode(int mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (unlikely(mode == ((ts_data->noise_mode >> NOISE_MODE_COVER) & 1)))
		return;

	if (mode)
		ts_data->noise_mode |= (1 << NOISE_MODE_COVER);
	else
		ts_data->noise_mode &= ~(1 << NOISE_MODE_COVER);

#if IST30XX_TA_RESET
	if (unlikely(ts_data->initialized))
		ist30xx_scheduled_reset(ts_data);
#else
	ist30xx_write_cmd(ts_data->client, IST30XX_HIB_CMD,
			((eHCOM_SET_MODE_SPECIAL << 16) | (ts_data->noise_mode & 0xFFFF)));
#endif

	ist30xx_tracking(mode ? TRACK_CMD_COVER : TRACK_CMD_NOTCOVER);
}
EXPORT_SYMBOL(ist30xx_set_cover_mode);

#ifdef USE_TSP_TA_CALLBACKS
void charger_enable(struct tsp_callbacks *cb, int enable)
{
	bool charging = enable ? true : false;

	ist30xx_set_ta_mode(charging);
}

static void ist30xx_register_callback(struct tsp_callbacks *cb)
{
	charger_callbacks = cb;
	pr_info("%s\n", __func__);
}
#endif

static void reset_work_func(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct ist30xx_data *data = container_of(delayed_work,
			struct ist30xx_data, work_reset_check);

	if (unlikely((data == NULL) || (data->client == NULL)))
		return;

	tsp_info("Request reset function\n");

	if (likely((data->initialized == 1) && (data->status.power == 1) &&
				(data->status.update != 1) && (data->status.calib != 1))) {

		mutex_lock(&ist30xx_mutex);
		ist30xx_disable_irq(data);
#if IST30XX_GESTURE
		if (data->suspend)
			ist30xx_internal_suspend(data);
		else
#endif
			ist30xx_reset(data, false);
		clear_input_data(data);
		ist30xx_start(data);
#if IST30XX_GESTURE
		if (data->gesture && data->suspend)
			data->status.noise_mode = false;
#endif
		ist30xx_enable_irq(data);
		mutex_unlock(&ist30xx_mutex);
	}
}

#if IST30XX_INTERNAL_BIN
#if IST30XX_UPDATE_BY_WORKQUEUE
static void fw_update_func(struct work_struct *work)
{
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct ist30xx_data *data = container_of(delayed_work,
			struct ist30xx_data, work_fw_update);

	if (unlikely((data == NULL) || (data->client == NULL)))
		return;

	tsp_info("FW update function\n");

	if (likely(ist30xx_auto_bin_update(data)))
		ist30xx_disable_irq(data);
}
#endif /* IST30XX_UPDATE_BY_WORKQUEUE */
#endif /* IST30XX_INTERNAL_BIN */

#define TOUCH_STATUS_MAGIC	(0x00000075)
#define TOUCH_STATUS_MASK	(0x000000FF)
#define FINGER_ENABLE_MASK	(0x00100000)
#define SCAN_CNT_MASK		(0xFFE00000)
#define GET_FINGER_ENABLE(n)	((n & FINGER_ENABLE_MASK) >> 20)
#define GET_SCAN_CNT(n)		((n & SCAN_CNT_MASK) >> 21)
u32 ist30xx_algr_addr = 0, ist30xx_algr_size = 0;
static void noise_work_func(struct work_struct *work)
{
	int ret;
	u32 touch_status = 0;
	u32 scan_count = 0;
	struct delayed_work *delayed_work = to_delayed_work(work);
	struct ist30xx_data *data = container_of(delayed_work,
			struct ist30xx_data, work_noise_protect);

	ret = ist30xx_read_reg(data->client,
			IST30XX_HIB_TOUCH_STATUS, &touch_status);
	if (unlikely(ret)) {
		tsp_warn("Touch status read fail!\n");
		goto retry_timer;
	}

	ist30xx_put_track_ms(timer_ms);
	ist30xx_put_track(&touch_status, 1);

	tsp_verb("Touch Info: 0x%08x\n", touch_status);

	/* Check valid scan count */
	if (unlikely((touch_status & TOUCH_STATUS_MASK) != TOUCH_STATUS_MAGIC)) {
		tsp_warn("Touch status is not corrected! (0x%08x)\n", touch_status);
		goto retry_timer;
	}

	/* Status of IC is idle */
	if (GET_FINGER_ENABLE(touch_status) == 0) {
		if ((PARSE_FINGER_CNT(data->t_status) > 0) ||
				(PARSE_KEY_CNT(data->t_status) > 0))
			clear_input_data(data);
	}

	scan_count = GET_SCAN_CNT(touch_status);

	/* Status of IC is lock-up */
	if (unlikely(scan_count == data->scan_count)) {
		tsp_warn("TSP IC is not responded! (0x%08x)\n", scan_count);
		goto retry_timer;
	}

	data->scan_retry = 0;
	data->scan_count = scan_count;
	return;

retry_timer:
	data->scan_retry++;
	tsp_warn("Retry touch status!(%d)\n", data->scan_retry);

	if (unlikely(data->scan_retry == data->max_scan_retry)) {
		ist30xx_scheduled_reset(data);
		data->scan_retry = 0;
	}
}

static void debug_work_func(struct work_struct *work)
{
#if IST30XX_ALGORITHM_MODE
	int ret = -EPERM;
	int i;
	u32 *buf32;

	struct delayed_work *delayed_work = to_delayed_work(work);
	struct ist30xx_data *data = container_of(delayed_work,
			struct ist30xx_data, work_debug_algorithm);

	buf32 = kzalloc(ist30xx_algr_size, GFP_KERNEL);
	if (!buf32) {
		tsp_info("%s: Couldn't Allocate memory\n", __func__);
                return;
	}

	for (i = 0; i < ist30xx_algr_size; i++) {
		ret = ist30xx_read_buf(data->client,
				ist30xx_algr_addr + IST30XX_DATA_LEN * i, &buf32[i], 1);
		if (ret) {
			tsp_warn("Algorithm mem addr read fail!\n");
			return;
		}
	}

	ist30xx_put_track(buf32, ist30xx_algr_size);

	tsp_debug(" 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
			buf32[0], buf32[1], buf32[2], buf32[3], buf32[4]);
	kfree(buf32);
#endif
}

void timer_handler(unsigned long timer_data)
{
	struct ist30xx_data *data = (struct ist30xx_data *)timer_data;
	struct ist30xx_status *status = &data->status;

	if (data->irq_working)
		goto restart_timer;

	if (status->event_mode) {
		if (likely((status->power == 1) && (status->update != 1))) {
			timer_ms = (u32)get_milli_second();
			if (unlikely(status->calib == 1)) {
				/* Check calibration */
				if ((status->calib_msg & CALIB_MSG_MASK) == CALIB_MSG_VALID) {
					tsp_info("Calibration check OK!!\n");
					schedule_delayed_work(&data->work_reset_check, 0);
					status->calib = 0;
				} else if (timer_ms - event_ms >= 3000) {
					/* over 3 second */
					tsp_info("calibration timeout over 3sec\n");
					schedule_delayed_work(&data->work_reset_check, 0);
					status->calib = 0;
				}
			} else if (likely(status->noise_mode)) {
				/* 100ms after last interrupt */
				if (timer_ms - event_ms > 100)
					schedule_delayed_work(&data->work_noise_protect, 0);
			}

#if IST30XX_ALGORITHM_MODE
			if ((ist30xx_algr_addr >= IST30XX_DIRECT_ACCESS) &&
					(ist30xx_algr_size > 0)) {
				/* 100ms after last interrupt */
				if (timer_ms - event_ms > 100)
					schedule_delayed_work(&data->work_debug_algorithm, 0);
			}
#endif
		}
	}

restart_timer:
	mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL);
}

static void ist30xx_request_gpio(struct i2c_client *client,
		struct ist30xx_data *data)
{
	int ret;

	tsp_info("%s\n", __func__);

	if (gpio_is_valid(data->dt_data->touch_en_gpio)) {
		ret = gpio_request(data->dt_data->touch_en_gpio,
				"imagis,touch_en_gpio");
		if (ret) {
			tsp_err("%s: unable to request touch_en_gpio [%d]\n",
					__func__, data->dt_data->touch_en_gpio);
			return;
		}
	}
#if defined(CONFIG_TOUCH_KEY_LED)
	if (gpio_is_valid(data->dt_data->keyled_en_gpio)) {
		ret = gpio_request(data->dt_data->keyled_en_gpio,
				"imagis,touch_en_gpio");
		if (ret) {
			tsp_err("%s: unable to request touch_en_gpio [%d]\n",
					__func__, data->dt_data->keyled_en_gpio);
			return;
		}
	}
#endif
	if (gpio_is_valid(data->dt_data->irq_gpio)) {
		ret = gpio_request(data->dt_data->irq_gpio, "imagis,irq_gpio");
		if (ret) {
			tsp_err("%s: unable to request irq_gpio [%d]\n",
					__func__, data->dt_data->irq_gpio);
			return;
		}

		ret = gpio_direction_input(data->dt_data->irq_gpio);
		if (ret) {
			tsp_err("%s: unable to set direction for gpio [%d]\n",
					__func__, data->dt_data->irq_gpio);
		}
		client->irq = gpio_to_irq(data->dt_data->irq_gpio);
	}
}

static void ist30xx_free_gpio(struct ist30xx_data *data)
{
	tsp_info("%s\n", __func__);

	if (gpio_is_valid(data->dt_data->touch_en_gpio))
		gpio_free(data->dt_data->touch_en_gpio);

	if (gpio_is_valid(data->dt_data->irq_gpio))
		gpio_free(data->dt_data->irq_gpio);
#if defined(CONFIG_TOUCH_KEY_LED)
	if (gpio_is_valid(data->dt_data->keyled_en_gpio))
		gpio_free(data->dt_data->keyled_en_gpio);
#endif
}

#ifdef CONFIG_OF
static int ist30xx_parse_dt(struct device *dev, struct ist30xx_data *data)
{
	struct device_node *np = dev->of_node;
	int rc = 0;

	data->dt_data->irq_gpio = of_get_named_gpio(np, "imagis,irq-gpio", 0);
	data->dt_data->touch_en_gpio = of_get_named_gpio(np, "vdd_en-gpio", 0);
#if defined(CONFIG_TOUCH_KEY_LED)
	data->dt_data->keyled_en_gpio = of_get_named_gpio(np, "keyled_en-gpio", 0);
#endif
	if (of_property_read_u32(np, "imagis,bring-up", &data->dt_data->bringup))
		tsp_info("%s() bring up: %d\n", __func__, data->dt_data->bringup);

	if (of_property_read_u32(np, "imagis,fw-bin", &data->dt_data->fw_bin))
		tsp_info("%s() fw-bin: %d\n", __func__, data->dt_data->fw_bin);

	if (of_property_read_u32(np, "imagis,tkey", &data->dt_data->tkey) >= 0 )
		tsp_info("%s() tkey: %d\n", __func__, data->dt_data->tkey);

	if (of_property_read_u32(np, "imagis,octa-hw", &data->dt_data->octa_hw) >= 0 )
		tsp_info("%s() octa-hw: %d\n", __func__, data->dt_data->octa_hw);

	if (of_property_read_u32(np, "imagis,multiple-tsp", &data->dt_data->multiple_tsp) >= 0 )
		tsp_info("%s() multiple_tsp: %d\n", __func__, data->dt_data->multiple_tsp);

	if (of_property_read_string(np, "imagis,ic-version", &data->dt_data->ic_version) >= 0)
		tsp_info("%s() ic_version: %s\n", __func__, data->dt_data->ic_version);

	if (of_property_read_string(np, "imagis,project-name", &data->dt_data->project_name) >= 0)
		tsp_info("%s() project_name: %s\n", __func__, data->dt_data->project_name);

	if (of_property_read_string(np, "imagis,extra-string", &data->dt_data->extra_string) >= 0)
		tsp_info("%s() extra_string: %s\n", __func__, data->dt_data->extra_string);

	if (data->dt_data->ic_version && data->dt_data->project_name) {
		if (!data->dt_data->multiple_tsp) {
			if (data->dt_data->extra_string)
				snprintf(data->dt_data->fw_path, FIRMWARE_PATH_LENGTH,
						"%s%s_%s_%s.fw", FIRMWARE_PATH,
						data->dt_data->ic_version, data->dt_data->project_name,
						data->dt_data->extra_string);
			else
				snprintf(data->dt_data->fw_path, FIRMWARE_PATH_LENGTH,
						"%s%s_%s.fw", FIRMWARE_PATH,
						data->dt_data->ic_version, data->dt_data->project_name);

			tsp_info("%s() firm path: %s\n", __func__, data->dt_data->fw_path);
		}

		snprintf(data->dt_data->cmcs_path, FIRMWARE_PATH_LENGTH,
				"%s%s_%s_cmcs.bin", FIRMWARE_PATH,
				data->dt_data->ic_version, data->dt_data->project_name);
		tsp_info("%s() cmcs bin path: %s\n", __func__, data->dt_data->cmcs_path);
	}

	rc = of_property_read_string(np, "vdd_ldo_name", &data->dt_data->tsp_vdd_name);
	if (rc < 0)
		tsp_err("%s: Unable to read TSP ldo name\n", __func__);

#if defined(CONFIG_TOUCH_KEY_LED)
	tsp_info("%s() irq:%d, touch_en:%d, keyled_en:%d\n",
			__func__, data->dt_data->irq_gpio, data->dt_data->touch_en_gpio,
			data->dt_data->keyled_en_gpio);
#else
	tsp_info("%s() irq:%d, touch_en:%d, tsp_ldo: %s\n",
			__func__, data->dt_data->irq_gpio, data->dt_data->touch_en_gpio,
			data->dt_data->tsp_vdd_name);

#endif
	return 0;
}
#else
static int ist30xx_parse_dt(struct device *dev, struct ist30xx_data *data)
{
	return -ENODEV;
}
#endif

static int ist30xx_set_input_device(struct ist30xx_data *data)
{
	int ret;

	set_bit(EV_ABS, data->input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, data->input_dev->propbit);

	input_set_abs_params(data->input_dev, ABS_MT_POSITION_X,
			0, data->max_x - 1, 0, 0);
	input_set_abs_params(data->input_dev, ABS_MT_POSITION_Y,
			0, data->max_y - 1, 0, 0);

	input_set_abs_params(data->input_dev, ABS_MT_TOUCH_MAJOR, 0, IST30XX_MAX_W, 0, 0);
#ifdef CONFIG_SEC_FACTORY
	input_set_abs_params(data->input_dev, ABS_MT_PRESSURE, 0, 0x1000, 0, 0);
#endif

#if IST30XX_USE_KEY
	{
		int i;
		set_bit(EV_KEY, data->input_dev->evbit);
		set_bit(EV_SYN, data->input_dev->evbit);
		set_bit(EV_LED, data->input_dev->evbit);
	        set_bit(LED_MISC, data->input_dev->ledbit);
		for (i = 1; i < ARRAY_SIZE(ist30xx_key_code); i++)
			set_bit(ist30xx_key_code[i], data->input_dev->keybit);
	}

#if IST30XX_GESTURE
	input_set_capability(data->input_dev, EV_KEY, KEY_POWER);
	input_set_capability(data->input_dev, EV_KEY, KEY_PLAYPAUSE);
	input_set_capability(data->input_dev, EV_KEY, KEY_NEXTSONG);
	input_set_capability(data->input_dev, EV_KEY, KEY_PREVIOUSSONG);
	input_set_capability(data->input_dev, EV_KEY, KEY_VOLUMEUP);
	input_set_capability(data->input_dev, EV_KEY, KEY_VOLUMEDOWN);
	input_set_capability(data->input_dev, EV_KEY, KEY_MUTE);
#endif
#endif

	set_bit(BTN_TOUCH, data->input_dev->keybit);

	input_set_drvdata(data->input_dev, data);
	ret = input_register_device(data->input_dev);

	tsp_err("%s: input register device:%d\n", __func__, ret);

	return ret;
}

static int ist30xx_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int ret;
	int retry = 3;
	u32 xy_res;
	u32 xy_swap;
	u32 tsp_type = 0;

	struct ist30xx_data *data;
	struct input_dev *input_dev;

	tsp_info("### IMAGIS probe(ver:%s, protocol:%X, addr:0x%02X) ###\n",
			IMAGIS_DD_VERSION, IMAGIS_PROTOCOL_B, client->addr);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		return -EIO;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

#ifdef CONFIG_OF
	data->dt_data = NULL;
	data->irq_enabled = 1;
	data->client = client;
	i2c_set_clientdata(client, data);

	if (client->dev.of_node) {
		data->dt_data = kzalloc(sizeof(struct ist30xx_dt_data), GFP_KERNEL);

		if (!data->dt_data)
			goto err_alloc_dev;

		ret = ist30xx_parse_dt(&client->dev, data);
		if (ret)
			goto err_alloc_dt;
	} else {
		data->dt_data = NULL;
		tsp_err("%s: TSP failed to align dtsi\n", __func__);
	}

	if (data->dt_data)
		ist30xx_request_gpio(client, data);

	/* Get pinctrl if target uses pinctrl */
	data->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(data->pinctrl)) {
		if (PTR_ERR(data->pinctrl) == -EPROBE_DEFER)
			goto err_pinctrl;

		tsp_err("%s: Target does not use pinctrl\n", __func__);
		data->pinctrl = NULL;
	}

	if (data->pinctrl) {
		ret = ist30xx_pinctrl_configure(data, true);
		if (ret)
			tsp_err("%s: cannot set pinctrl state\n", __func__);
	}
#endif
	input_dev = input_allocate_device();
	if (unlikely(!input_dev)) {
		tsp_err("input_allocate_device failed\n");
		goto err_alloc_dev;
	}

	tsp_info("client->irq : %d\n", client->irq);
	data->max_fingers = IST30XX_MAX_MT_FINGERS;
	data->max_keys = IST30XX_MAX_KEYS;
	data->irq_enabled = 1;
	data->client = client;
	data->input_dev = input_dev;
	i2c_set_clientdata(client, data);

	input_mt_init_slots(input_dev, IST30XX_MAX_MT_FINGERS, 0);

	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = ist30xx_ts_open;
	input_dev->close = ist30xx_ts_close;


#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = ist30xx_early_suspend;
	data->early_suspend.resume = ist30xx_late_resume;
	register_early_suspend(&data->early_suspend);
#endif
	ts_data = data;

	ret = ist30xx_init_system(data);
	if (unlikely(ret)) {
		tsp_err("chip initialization failed\n");
		goto err_init_drv;
	}

#ifdef CONFIG_DUAL_TOUCH_IC_CHECK
	while (retry-- > 0) {
		ret = ist30xxc_isp_info_read(data, 0, &tsp_type, 1);
	    tsp_info("%s: ret: %d, tsp_type: %x\n", __func__, ret, tsp_type);
		if(ret < 0) {
			tsp_info("no imagis chip!\n");
			goto err_init_drv;
		}
		mdelay(10);
	}
	retry = 3;
#endif

	if (data->dt_data->multiple_tsp) {
	   ret = ist30xxc_isp_info_read(data, 0, &tsp_type, 1);
	   tsp_info("%s: ret: %d, tsp_type: %x\n", __func__, ret, tsp_type);

	   if (data->dt_data->ic_version && data->dt_data->project_name && data->dt_data->extra_string) {
		   if (ret || (tsp_type == J5_100_OHM_VALUE))
			   snprintf(data->dt_data->fw_path, FIRMWARE_PATH_LENGTH,
					   "%s%s_%s_%s.fw", FIRMWARE_PATH,
					   data->dt_data->ic_version, data->dt_data->project_name,
					   data->dt_data->extra_string);
		   else
			   snprintf(data->dt_data->fw_path, FIRMWARE_PATH_LENGTH,
					   "%s%s_%s.fw", FIRMWARE_PATH,
					   data->dt_data->ic_version, data->dt_data->project_name);

		   tsp_info("%s() firm path: %s\n", __func__, data->dt_data->fw_path);
	   }
	}

	/* FW do not enter sleep mode in probe */
	ret = ist30xx_write_cmd(data->client,
		IST30XX_HIB_CMD, (eHCOM_FW_HOLD << 16) | (1 & 0xFFFF));
	tsp_info("%s: set FW_HOLD\n", __func__);

	while (retry-- > 0) {
		ret = ist30xx_read_cmd(data, IST30XX_REG_CHIPID, &data->chip_id);
		data->chip_id &= 0xFFFF;
		if (unlikely(ret == 0)) {
			if ((data->chip_id == IST30XX_CHIP_ID) ||
					(data->chip_id == IST30XXC_DEFAULT_CHIP_ID) ||
					(data->chip_id == IST3048C_DEFAULT_CHIP_ID)) {
				break;
			}
		} else if (unlikely(retry == 0)) {
			goto err_init_drv;
		}

		ist30xx_reset(data, false);
	}

	ret = request_threaded_irq(client->irq, NULL, ist30xx_irq_thread,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "ist30xx_ts", data);
	if (unlikely(ret))
		goto err_init_drv;

	ist30xx_disable_irq(data);

#if (IMAGIS_TSP_IC < IMAGIS_IST3038C)
	retry = 3;
	data->tsp_type = TSP_TYPE_UNKNOWN;
	while (retry-- > 0) {

		ret = ist30xx_read_cmd(data, IST30XX_REG_TSPTYPE, &data->tsp_type);
		if (likely(ret == 0)) {
			data->tsp_type = IST30XX_PARSE_TSPTYPE(data->tsp_type);
			tsp_info("tsptype: %x\n", data->tsp_type);
			break;
		}

		if (unlikely(retry == 0))
			goto err_irq;
	}

	tsp_info("TSP IC: %x, TSP Vendor: %x\n", data->chip_id, data->tsp_type);
#else
	tsp_info("TSP IC: %x\n", data->chip_id);
#endif
	data->status.event_mode = false;

#if IST30XX_INTERNAL_BIN
#if IST30XX_UPDATE_BY_WORKQUEUE
	INIT_DELAYED_WORK(&data->work_fw_update, fw_update_func);
	schedule_delayed_work(&data->work_fw_update, IST30XX_UPDATE_DELAY);
#else

	ret = ist30xx_auto_bin_update(data);
	if (unlikely(ret != 0))
		goto err_irq;
#endif /* IST30XX_UPDATE_BY_WORKQUEUE */
#endif /* IST30XX_INTERNAL_BIN */

	ret = ist30xx_read_cmd(data, IST30XX_REG_XY_SWAP, &xy_swap);
	tsp_err("%s: ret:%d, swap:%x\n", __func__, ret, xy_swap & 0x01);
	ret = ist30xx_read_cmd(data, IST30XX_REG_XY_RES, &xy_res);
	if (xy_swap & 0x01) {
		data->max_x = (u16)(xy_res & 0xFFFF);
		data->max_y = (u16)(xy_res >> 16);
	} else {
		data->max_x = (u16)(xy_res >> 16);
		data->max_y = (u16)(xy_res & 0xFFFF);
	}
	tsp_err("%s: ret:%d, xy:%X, x:%d, y:%d\n", __func__, ret, xy_res, data->max_x, data->max_y);

	ret = ist30xx_set_input_device(data);
	if (ret < 0)
		goto err_irq;

#ifdef CONFIG_INPUT_BOOSTER
	tsp_info("input Booster\n");
	data->tsp_booster = input_booster_allocate(INPUT_BOOSTER_ID_TSP);
	if (!data->tsp_booster) {
		tsp_err("%s booster allocation is failed\n", __func__);
		goto err_irq;
	}
#endif

	ret = ist30xx_init_update_sysfs(data);
	if (unlikely(ret))
		goto err_sysfs;

#if IST30XX_DEBUG
	ret = ist30xx_init_misc_sysfs(data);
	if (unlikely(ret))
		goto err_sysfs;
#endif

#if IST30XX_CMCS_TEST
	ret = ist30xx_init_cmcs_sysfs(data);
	if (unlikely(ret))
		tsp_err("%s: do not init cmcs\n",__func__);
#endif

#if IST30XX_CMCS_JIT_TEST
	ret = ist30xx_init_cmcs_jit_sysfs(data);
	if (unlikely(ret))
		tsp_err("%s: do not init cmcs jitter\n",__func__);
#endif

#if IST30XX_TRACKING_MODE
	ret = ist30xx_init_tracking_sysfs(data);
	if (unlikely(ret))
		goto err_sysfs;
#endif

#if SEC_FACTORY_MODE
	ret = sec_fac_cmd_init(data);
	if (unlikely(ret))
		goto err_sysfs;

	ret = sec_touch_sysfs(data);
	if (unlikely(ret))
		goto err_sec_sysfs;
#endif

	/* initialize data variable */
#if IST30XX_GESTURE
	data->suspend = false;
	data->gesture = false;
#endif
	data->irq_working = false;
	data->max_scan_retry = 2;
	data->max_irq_err_cnt = MAX_ERR_CNT;
	data->report_rate = -1;
	data->idle_rate = -1;
#ifdef CONFIG_SEC_FACTORY
	data->jig_mode = 1;
#endif
	INIT_DELAYED_WORK(&data->work_reset_check, reset_work_func);
	INIT_DELAYED_WORK(&data->work_noise_protect, noise_work_func);
	INIT_DELAYED_WORK(&data->work_debug_algorithm, debug_work_func);

	init_timer(&event_timer);
	event_timer.data = (unsigned long)data;
	event_timer.function = timer_handler;
	event_timer.expires = jiffies_64 + (EVENT_TIMER_INTERVAL);
	mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL * 2);

	ret = ist30xx_get_info(data);
	tsp_info("Get info: %s\n", (ret == 0 ? "success" : "fail"));

#ifdef USE_TSP_TA_CALLBACKS
	data->callbacks.inform_charger = charger_enable;
	ist30xx_register_callback(&data->callbacks);
#endif
	data->initialized = true;

#ifdef CONFIG_DUAL_TOUCH_IC_CHECK
	probe_finished = 1;
#endif

	tsp_info("### IMAGIS probe success ###\n");

	/* release Firmware hold mode(forced active mode) */
	ret = ist30xx_write_cmd(data->client,
		IST30XX_HIB_CMD, (eHCOM_FW_HOLD << 16) | (0 & 0xFFFF));
	tsp_info("%s: release FW_HOLD\n", __func__);

	return 0;

err_sec_sysfs:
#if SEC_FACTORY_MODE
	sec_fac_cmd_remove(data);
	sec_touch_sysfs_remove(data);
#endif
err_sysfs:
	class_destroy(ist30xx_class);
	input_unregister_device(input_dev);
#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(data->tsp_booster);
	data->tsp_booster = NULL;
#endif
err_irq:
	tsp_info("ChipID: %x\n", data->chip_id);
	ist30xx_disable_irq(data);
	free_irq(client->irq, data);
err_init_drv:
	data->status.event_mode = false;
	ist30xx_power_off(data);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
err_pinctrl:
err_alloc_dt:
	if (data->dt_data) {
		tsp_err("%s: Error, ist30xx mem free, line:%d\n", __func__, __LINE__);
		kfree(data->dt_data);
	}
err_alloc_dev:
	kfree(data);
	tsp_err("Error, ist30xx init driver\n");
	return -ENODEV;
}


static int ist30xx_remove(struct i2c_client *client)
{
	struct ist30xx_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_DUAL_TOUCH_IC_CHECK
	if(!probe_finished)
	{
		printk("%s: imagis not register!\n", __func__);
		return -ENODEV;
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif

	ist30xx_disable_irq(data);
	free_irq(client->irq, data);
	ist30xx_power_off(data);

	sec_fac_cmd_remove(data);

	if (data->dt_data)
		ist30xx_free_gpio(data);

	input_unregister_device(data->input_dev);
#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(data->tsp_booster);
	data->tsp_booster = NULL;
#endif
	kfree(data);

	return 0;
}

static void ist30xx_shutdown(struct i2c_client *client)
{
	struct ist30xx_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_DUAL_TOUCH_IC_CHECK
	if(!probe_finished)
	{
		printk("%s: imagis not register!\n", __func__);
		return ;
	}
#endif

	tsp_err("%s is called\n", __func__);
	del_timer(&event_timer);
	cancel_delayed_work_sync(&data->work_noise_protect);
	cancel_delayed_work_sync(&data->work_reset_check);
	cancel_delayed_work_sync(&data->work_debug_algorithm);
	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(data);
	ist30xx_internal_suspend(data);
	clear_input_data(data);
	mutex_unlock(&ist30xx_mutex);
#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(data->tsp_booster);
	data->tsp_booster = NULL;
#endif
}

static struct i2c_device_id ist30xx_idtable[] = {
	{ IST30XX_DEV_NAME, 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, ist30xx_idtable);

#ifdef CONFIG_OF
static struct of_device_id ist30xx_match_table[] = {
	{ .compatible = "imagis,ist30xx-ts", },
	{ },
};
#else
#define ist30xx_match_table NULL
#endif

static struct i2c_driver ist30xx_i2c_driver = {
	.id_table	= ist30xx_idtable,
	.probe		= ist30xx_probe,
	.remove		= ist30xx_remove,
	.shutdown	= ist30xx_shutdown,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= IST30XX_DEV_NAME,
		.of_match_table = ist30xx_match_table,
	},
};

extern int get_lcd_attached(char *mode);

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif
static int __init ist30xx_init(void)
{
	tsp_info("%s()\n", __func__);
#ifdef CONFIG_SAMSUNG_LPM_MODE
	if (poweroff_charging) {
		tsp_info("%s() LPM Charging Mode!!\n", __func__);
		return 0;
	}
#endif

	if (!get_lcd_attached("GET")) {
		tsp_err("%s: LCD is not attached\n", __func__);
		return 0;
	}
	return i2c_add_driver(&ist30xx_i2c_driver);
}


static void __exit ist30xx_exit(void)
{
	tsp_info("%s()\n", __func__);
	i2c_del_driver(&ist30xx_i2c_driver);
}

module_init(ist30xx_init);
module_exit(ist30xx_exit);

MODULE_DESCRIPTION("Imagis IST30XX touch driver");
MODULE_LICENSE("GPL");
