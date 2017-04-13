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
#include <linux/gpio.h>
#include <linux/cpufreq.h>

#include <linux/input/mt.h>
#include <linux/pinctrl/consumer.h>

#include "ist30xx.h"
#include "ist30xx_sec.h"
#include "ist30xx_update.h"
#include "ist30xx_tracking.h"

#if IST30XX_CHECK_BATT_TEMP
#include <linux/power_supply.h>
#endif
#if IST30XX_DEBUG
#include "ist30xx_misc.h"
#endif

#if IST30XX_CMCS_TEST
#include "ist30xx_cmcs.h"
#endif

#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif

#define MAX_ERR_CNT             (100)

#define USE_OPEN_CLOSE
#if IST30XX_USE_KEY
int ist30xx_key_code[] = { 0, KEY_RECENT, KEY_BACK };
#endif
DEFINE_MUTEX(ist30xx_mutex);
volatile bool ist30xx_irq_working = false;
bool ist30xx_initialized = 0;
static int touch_is_pressed = 0;

struct ist30xx_data *ts_data;
EXPORT_SYMBOL(ts_data);

static struct delayed_work work_reset_check;
static struct delayed_work work_noise_protect, work_debug_algorithm;
static int ist30xx_power_status = -1;
static int ist30xx_ta_status = -1;
static int ist30xx_noise_mode = -1;

#if IST30XX_INTERNAL_BIN && IST30XX_UPDATE_BY_WORKQUEUE
static struct delayed_work work_fw_update;
#endif

int ist30xx_report_rate = -1;
int ist30xx_idle_rate = -1;

#if IST30XX_CHECK_BATT_TEMP
#define IST30XX_MAX_CHK_CNT     1   // 500msec unit
s16 ist30xx_batt_temp = 0;
int ist30xx_batt_chk_cnt = 0;
int ist30xx_batt_chk_max_cnt = IST30XX_MAX_CHK_CNT;
struct power_supply *ist30xx_psy;

int ist30xx_get_battery_temp(void)
{
	int ret=0;
	union power_supply_propval value;

	if(!ist30xx_psy){
		ist30xx_psy = power_supply_get_by_name("battery");
		if(!ist30xx_psy)
			return 0;
	}
	ret = ist30xx_psy->get_property(ist30xx_psy, POWER_SUPPLY_PROP_TEMP, &value);
	if (ret) {
			tsp_info("%s: fail to set power_suppy ONLINE property(%d)\n", __func__, ret);
	}
	return value.intval;
}
#endif
u32 event_ms = 0, timer_ms = 0;
int ist30xx_scan_retry = 0;

static struct timer_list event_timer;
static struct timespec t_current;                               // ns
int timer_period_ms = 500;                                      // 0.5sec
# define EVENT_TIMER_INTERVAL     (HZ * timer_period_ms / 1000) // 0.5sec

#if IST30XX_DEBUG
extern TSP_INFO ist30xx_tsp_info;
extern TKEY_INFO ist30xx_tkey_info;
#endif
extern unsigned int system_rev;

#define IST30XX_RAWDATA_DEBUG 1

#if IST30XX_RAWDATA_DEBUG
#define RAWDATA_TIMER_INTERVAL    (HZ * 3)    // 3sec

static struct timer_list rawdata_timer;
static struct delayed_work work_read_rawdata;

static void ist30xx_dump_raw_data(void)
{
	int ret;
	int i, j;
	u8 flag = NODE_FLAG_RAW | NODE_FLAG_BASE | NODE_FLAG_FILTER;

	TSP_INFO *tsp = &ist30xx_tsp_info;

	tsp_info("%s\n", __func__);

	ret = ist30xx_read_touch_node(flag, &tsp->node);
	if (ret) {
		tsp_warn("%s(), tsp node read fail!\n", __func__);
		return;
	}
	ist30xx_parse_touch_node(flag, &tsp->node);

	tsp_info("RAW - %d * %d\n", tsp->ch_num.tx, tsp->ch_num.rx);
	for (i = 0; i < tsp->ch_num.tx; i++) {
		printk("[ TSP ] ");
		for (j = 0; j < tsp->ch_num.rx; j++){
			if(j==tsp->ch_num.rx-1)
				printk("%4d\n", tsp->node.raw[i][j]);
			else
				printk("%4d ", tsp->node.raw[i][j]);
		}
	}
	printk("\n");

	tsp_info("BASE - %d * %d\n", tsp->ch_num.tx, tsp->ch_num.rx);
	for (i = 0; i < tsp->ch_num.tx; i++) {
		printk("[ TSP ] ");
		for (j = 0; j < tsp->ch_num.rx; j++){
			if(j==tsp->ch_num.rx-1)
				printk("%4d\n", tsp->node.base[i][j]);
			else
				printk("%4d ", tsp->node.base[i][j]);
		}
	}
	printk("\n");

	tsp_info("FILTER - %d * %d\n", tsp->ch_num.tx, tsp->ch_num.rx);
	for (i = 0; i < tsp->ch_num.tx; i++) {
		printk("[ TSP ] ");
		for (j = 0; j < tsp->ch_num.rx; j++){
			if(j==tsp->ch_num.rx-1)
				printk("%4d\n", tsp->node.filter[i][j]);
			else
				printk("%4d ", tsp->node.filter[i][j]);
		}
	}
	printk("\n");
}

static void rawdata_work_func(struct work_struct *work)
{
	ist30xx_dump_raw_data();

	mod_timer(&rawdata_timer, get_jiffies_64() + RAWDATA_TIMER_INTERVAL);
}

static void read_rawdata_timer_handler(unsigned long data)
{
	if (!ts_data) {
		tsp_err("%s: global ts_data is null", __func__);
		return;
	}

	if ((ts_data->status.power == 1) && (ts_data->status.update != 1)) {
		tsp_debug("%s: start work\n", __func__);
		schedule_delayed_work(&work_read_rawdata, 0);
	}
}

void tsp_start_read_rawdata(void)
{
	init_timer(&rawdata_timer);
	rawdata_timer.function = read_rawdata_timer_handler;
	rawdata_timer.expires = jiffies_64 + RAWDATA_TIMER_INTERVAL;

	add_timer(&rawdata_timer);
}
EXPORT_SYMBOL(tsp_start_read_rawdata);

void tsp_stop_read_rawdata(void)
{
	flush_work(&work_read_rawdata.work);

	del_timer(&rawdata_timer);
}
EXPORT_SYMBOL(tsp_stop_read_rawdata);
#else
void tsp_start_read_rawdata(void)
{
	tsp_info("%s: not define!\n", __func__);
	return;
}
EXPORT_SYMBOL(tsp_start_read_rawdata);

void tsp_stop_read_rawdata(void)
{
	tsp_info("%s: not defind!\n", __func__);
	return;
}
EXPORT_SYMBOL(tsp_stop_read_rawdata);
#endif

int prev_dbg_level = IST30XX_DEBUG_LEVEL;
int ist30xx_dbg_level = IST30XX_DEBUG_LEVEL;
void tsp_printk(int level, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;
	int r;

	if (level > ist30xx_dbg_level)
		return;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	r = printk("%s %pV", IST30XX_DEBUG_TAG, &vaf);

	va_end(args);
}

long get_milli_second(void)
{
	ktime_get_ts(&t_current);

	return t_current.tv_sec * 1000 + t_current.tv_nsec / 1000000;
}

int ist30xx_intr_wait(long ms)
{
	long start_ms = get_milli_second();
	long curr_ms = 0;

	while (1) {
		if (!ist30xx_irq_working)
			break;

		curr_ms = get_milli_second();
		if ((curr_ms < 0) || (start_ms < 0) || (curr_ms - start_ms > ms)) {
			tsp_info("%s() timeout(%dms)\n", __func__, ms);
			return -EPERM;
		}

		msleep(2);
	}
	return 0;
}

#if defined(TOUCH_BOOSTER)
static void change_dvfs_lock(struct work_struct *work)
{
	struct ist30xx_data *info = container_of(work,
						 struct ist30xx_data, work_dvfs_chg.work);
	int retval = 0;
	mutex_lock(&info->dvfs_lock);

	if (info->dvfs_boost_mode == DVFS_STAGE_DUAL) {
                if (info->stay_awake) {
                        tsp_debug("%s: do fw update, do not change cpu frequency.\n", __func__);
                } else {
                        retval = set_freq_limit(DVFS_TOUCH_ID,
                                        MIN_TOUCH_LIMIT_SECOND);
                        info->dvfs_freq = MIN_TOUCH_LIMIT_SECOND;
                }
        } else if (info->dvfs_boost_mode == DVFS_STAGE_SINGLE ||
                        info->dvfs_boost_mode == DVFS_STAGE_TRIPLE) {
                retval = set_freq_limit(DVFS_TOUCH_ID, -1);
                info->dvfs_freq = -1;
        }
	if (retval < 0)
		tsp_err ("[TSP] %s: booster stop failed(%d)\n",__func__, retval);

	mutex_unlock(&info->dvfs_lock);
}
static void set_dvfs_off(struct work_struct *work)
{
	struct ist30xx_data *info = container_of(work,
						 struct ist30xx_data, work_dvfs_off.work);
	int retval;	

	if (info->stay_awake) {
                tsp_debug("%s: do fw update, do not change cpu frequency.\n", __func__);
        } else {
                mutex_lock(&info->dvfs_lock);

                retval = set_freq_limit(DVFS_TOUCH_ID, -1);
                info->dvfs_freq = -1;

                if (retval < 0)
                        tsp_err("%s: booster stop failed(%d).\n", __func__, retval);
                info->dvfs_lock_status = false;

                mutex_unlock(&info->dvfs_lock);
        }
}

static void set_dvfs_lock(struct ist30xx_data *info, int32_t on)
{
	int ret = 0;

	if (info->dvfs_boost_mode == DVFS_STAGE_NONE) {
		tsp_debug( "%s: DVFS stage is none(%d)\n", __func__, info->dvfs_boost_mode);
		return;
	}	
	mutex_lock(&info->dvfs_lock);
	if (on == 0) {
		if (info->dvfs_lock_status) {
			schedule_delayed_work(&info->work_dvfs_off,
					      msecs_to_jiffies(TOUCH_BOOSTER_OFF_TIME));
		}
	} else if (on > 0) {
                cancel_delayed_work(&info->work_dvfs_off);

                if ((!info->dvfs_lock_status) || (info->dvfs_old_status < on)) {
                        cancel_delayed_work(&info->work_dvfs_chg);

                        if (info->dvfs_freq != MIN_TOUCH_LIMIT) {
#ifdef SECOND_MINLOCK_FOR_LEVEL1
                                if (info->dvfs_boost_mode == DVFS_STAGE_SINGLE ||
					info->dvfs_boost_mode == DVFS_STAGE_TRIPLE)
#else
                                if (info->dvfs_boost_mode == DVFS_STAGE_TRIPLE)
#endif
                                        ret = set_freq_limit(DVFS_TOUCH_ID,
                                                MIN_TOUCH_LIMIT_SECOND);
                                else
                                        ret = set_freq_limit(DVFS_TOUCH_ID,
                                                MIN_TOUCH_LIMIT);
                                info->dvfs_freq = MIN_TOUCH_LIMIT;

                                if (ret < 0)
					tsp_err("%s: cpu first lock failed(%d)\n", __func__, ret);
                        }
			schedule_delayed_work(&info->work_dvfs_chg,
				msecs_to_jiffies(TOUCH_BOOSTER_CHG_TIME));
                        info->dvfs_lock_status = true;
                }
        } else if (on < 0) {
                if (info->dvfs_lock_status) {
                        cancel_delayed_work(&info->work_dvfs_off);
                        cancel_delayed_work(&info->work_dvfs_chg);
                        schedule_work(&info->work_dvfs_off.work);
                }
        }
        info->dvfs_old_status = on;
        mutex_unlock(&info->dvfs_lock);
}
static void init_dvfs(struct ist30xx_data *info)
{
	mutex_init(&info->dvfs_lock);

	info->dvfs_boost_mode = DVFS_STAGE_DUAL;

	INIT_DELAYED_WORK(&info->work_dvfs_off, set_dvfs_off);
	INIT_DELAYED_WORK(&info->work_dvfs_chg, change_dvfs_lock);

	info->dvfs_lock_status = false;
}
#endif

void ist30xx_disable_irq(struct ist30xx_data *data)
{
	if (likely(data->irq_enabled)) {
		ist30xx_tracking(TRACK_INTR_DISABLE);
		disable_irq(data->client->irq);
		tsp_debug("%s: %d\n", __func__, data->client->irq);
		data->irq_enabled = 0;
		data->status.event_mode = false;
	}
}

void ist30xx_enable_irq(struct ist30xx_data *data)
{
	if (likely(!data->irq_enabled)) {
		ist30xx_tracking(TRACK_INTR_ENABLE);
		enable_irq(data->client->irq);
		tsp_debug("%s: %d\n", __func__, data->client->irq);
		msleep(10);
		data->irq_enabled = 1;
		data->status.event_mode = true;
	}
}

int ist30xx_max_error_cnt = MAX_ERR_CNT;
int ist30xx_error_cnt = 0;
void ist30xx_scheduled_reset(void)
{
	if (likely(ist30xx_initialized))
		schedule_delayed_work(&work_reset_check, 0);
}

static void ist30xx_request_reset(void)
{
	ist30xx_error_cnt++;
	if (unlikely(ist30xx_error_cnt >= ist30xx_max_error_cnt)) {
		tsp_info("%s()\n", __func__);
		ist30xx_scheduled_reset();
		ist30xx_error_cnt = 0;
	}
}

#define NOISE_MODE_TA       (0)
#define NOISE_MODE_CALL     (1)
#define NOISE_MODE_COVER    (2)
#define NOISE_MODE_POWER    (8)
u32 ist30xx_scan_count = 0;
void ist30xx_start(struct ist30xx_data *data)
{
	if (ist30xx_initialized) {
#if IST30XX_EVENT_MODE
        ist30xx_scan_count = 0;
        ist30xx_scan_retry = 0;
		mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL * 2);
#endif
	}

	ist30xx_tracking(ist30xx_ta_status ?
			 TRACK_CMD_TACON : TRACK_CMD_TADISCON);

	ist30xx_noise_mode &= ~(1 << NOISE_MODE_TA);
	ist30xx_noise_mode |= (ist30xx_ta_status << NOISE_MODE_TA);

	ist30xx_noise_mode &= ~(1 << NOISE_MODE_POWER);
	ist30xx_noise_mode |= (ist30xx_power_status << NOISE_MODE_POWER);
				 
	ist30xx_noise_mode |= (TSP_LOCAL_CODE << 16);
	ist30xx_write_cmd(data->client, CMD_SET_NOISE_MODE, ist30xx_noise_mode);

	tsp_info("%s(), local : %d, mode : 0x%x\n", __func__,
		 (ist30xx_noise_mode >> 16) & 0xFFFF, ist30xx_noise_mode & 0xFFFF);

	if (ist30xx_report_rate >= 0) {
		ist30xx_write_cmd(data->client, CMD_SET_REPORT_RATE,
				  ist30xx_report_rate);
		tsp_info(" reporting rate : %dus\n", ist30xx_report_rate);
	}

	if (ist30xx_idle_rate >= 0) {
		ist30xx_write_cmd(data->client, CMD_SET_IDLE_TIME, ist30xx_idle_rate);
		tsp_info(" idle rate : %dus\n", ist30xx_idle_rate);
	}

	ist30xx_cmd_start_scan(data->client);
}


int ist30xx_get_ver_info(struct ist30xx_data *data)
{
	int ret;

	data->fw.prev_core_ver = data->fw.core_ver;
	data->fw.prev_param_ver = data->fw.param_ver;
	data->fw.core_ver = data->fw.param_ver = 0;

	ret = ist30xx_read_cmd(data->client, CMD_GET_FW_VER, &data->fw.core_ver);
	if (unlikely(ret))
		return ret;

	ret = ist30xx_read_cmd(data->client, CMD_GET_PARAM_VER, &data->fw.param_ver);
	if (unlikely(ret))
		return ret;

	ret = ist30xx_read_cmd(data->client, CMD_GET_SUB_VER, &data->fw.sub_ver);
	if (unlikely(ret))
		return ret;

	tsp_info("%s: IC version read core: %x, param: %x, sub: %x\n",
		 __func__, data->fw.core_ver, data->fw.param_ver, data->fw.sub_ver);

	return 0;
}

#if IST30XX_DEBUG
void ist30xx_print_info(void)
{
	TSP_INFO *tsp = &ist30xx_tsp_info;
	TKEY_INFO *tkey = &ist30xx_tkey_info;

	tsp_debug("%s: *** TSP/TKEY info ***\n", __func__);
	tsp_debug("%s:  tscn finger num : %d\n", __func__, tsp->finger_num);
	tsp_debug("%s:  tscn dir swap: %d, flip x: %d, y: %d\n", __func__,
		  tsp->dir.swap_xy, tsp->dir.flip_x, tsp->dir.flip_y);
	tsp_debug("%s:  tscn ch_num tx: %d, rx: %d\n", __func__,
		  tsp->ch_num.tx, tsp->ch_num.rx);
	tsp_debug("%s:  tscn width: %d, height: %d\n", __func__,
		  tsp->width, tsp->height);
	tsp_debug("%s:  tkey enable: %d, key num: %d, axis rx: %d \n", __func__,
		  tkey->enable, tkey->key_num, tkey->axis_rx);
	tsp_debug("%s:  tkey ch_num[0] %d, [1] %d, [2] %d, [3] %d, [4] %d\n",
		  __func__, tkey->ch_num[0], tkey->ch_num[1], tkey->ch_num[2],
		  tkey->ch_num[3], tkey->ch_num[4]);
}
#endif

#define CALIB_MSG_MASK          (0xF0000FFF)
#define CALIB_MSG_VALID         (0x80000CAB)
#define TRACKING_INTR_VALID     (0x127EA597)
u32 tracking_intr_value = TRACKING_INTR_VALID;
extern const u8 *ts_fw;
extern u32 ts_fw_size;
int ist30xx_get_info(struct ist30xx_data *data)
{
	int ret;
	u32 calib_msg;
	u32 ms;

	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(data);

#if IST30XX_INTERNAL_BIN
#if IST30XX_DEBUG
#if IST30XX_UPDATE_BY_WORKQUEUE
	ist30xx_get_update_info();
#endif
	ist30xx_get_tsp_info(data);
	ist30xx_get_tkey_info(data);
#endif  // IST30XX_DEBUG
#else

	ret = ist30xx_get_ver_info(data);
	if (unlikely(ret))
		goto get_info_end;

#if IST30XX_DEBUG
	ret = ist30xx_tsp_update_info();
	if (unlikely(ret))
		goto get_info_end;

	ret = ist30xx_tkey_update_info();
	if (unlikely(ret))
		goto get_info_end;
#endif  // IST30XX_DEBUG
#endif  // IST30XX_INTERNAL_BIN

#if IST30XX_DEBUG
	ist30xx_print_info();
	data->max_fingers = ist30xx_tsp_info.finger_num;
	data->max_keys = ist30xx_tkey_info.key_num;
#endif  // IST30XX_DEBUG

	ret = ist30xx_read_cmd(ts_data->client, CMD_GET_CALIB_RESULT, &calib_msg);
	if (likely(ret == 0)) {
		tsp_info("%s: calib status: 0x%08x\n", __func__, calib_msg);
		ms = get_milli_second();
		ist30xx_put_track_ms(ms);
		ist30xx_put_track(&tracking_intr_value, 1);
		ist30xx_put_track(&calib_msg, 1);
		if ((calib_msg & CALIB_MSG_MASK) != CALIB_MSG_VALID ||
		    CALIB_TO_STATUS(calib_msg) > 0) {
			ist30xx_calibrate(IST30XX_FW_UPDATE_RETRY);

			ist30xx_reset(false);
		}
	}

#if (IST30XX_EVENT_MODE && IST30XX_CHECK_CALIB)
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
	ist30xx_start(ts_data);
#endif

#if !(IST30XX_INTERNAL_BIN)
get_info_end:
#endif
	if (likely(ret == 0))
		ist30xx_enable_irq(data);
	mutex_unlock(&ist30xx_mutex);

	return ret;
}


#define PRESS_MSG_MASK          (0x01)
#define MULTI_MSG_MASK          (0x02)
#define PRESS_MSG_KEY           (0x6)

#define TOUCH_DOWN_MESSAGE      ("P")
#define TOUCH_UP_MESSAGE        ("R")
#define TOUCH_MOVE_MESSAGE      (" ")
bool tsp_touched[IST30XX_MAX_MT_FINGERS] = { false, };
int tsp_count[IST30XX_MAX_MT_FINGERS] = { 0, };
bool tkey_pressed[IST30XX_MAX_KEYS] = { false, };

void print_tsp_event(finger_info *finger)
{
	int idx = finger->bit_field.id - 1;
	bool press;

	press = PRESSED_FINGER(ts_data->t_status, finger->bit_field.id);

	if (press) {
		if (tsp_touched[idx] == false) { // touch down
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			tsp_notc("%s[%d] (%d, %d)\n",
				 TOUCH_DOWN_MESSAGE, finger->bit_field.id,
				 finger->bit_field.x, finger->bit_field.y);
#else
			tsp_notc("%s[%d]\n",
				 TOUCH_DOWN_MESSAGE, finger->bit_field.id);
#endif
			tsp_touched[idx] = true;
		} else {                    // touch move
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
			tsp_debug("%s[%d] (%d, %d)\n",
				  TOUCH_MOVE_MESSAGE, finger->bit_field.id,
				  finger->bit_field.x, finger->bit_field.y);
#else
			tsp_debug("%s[%d]\n",
				  TOUCH_MOVE_MESSAGE, finger->bit_field.id);
#endif
			tsp_count[idx]++;
		}
	} else {
		if (tsp_touched[idx] == true) { // touch up
			tsp_notc("%s[%d] M(%d) V(%02x)\n",
				 TOUCH_UP_MESSAGE, finger->bit_field.id, tsp_count[idx],
				 ts_data->fw.param_ver);
			tsp_touched[idx] = false;
			tsp_count[idx] = 0;
		}
	}
}

void print_tkey_event(int id)
{
	int idx = id - 1;
	bool press = PRESSED_KEY(ts_data->t_status, id);

	if (press) {
		if (tkey_pressed[idx] == false) { // tkey down
			tsp_notc("button %d %s\n", id, TOUCH_DOWN_MESSAGE);
			tkey_pressed[idx] = true;
		}
	} else {
		if (tkey_pressed[idx] == true) { // tkey up
			tsp_notc("button %d %s\n", id, TOUCH_UP_MESSAGE);
			tkey_pressed[idx] = false;
		}
	}
}

static void release_finger(int id)
{
	input_mt_slot(ts_data->input_dev, id - 1);
	input_mt_report_slot_state(ts_data->input_dev, MT_TOOL_FINGER, false);

	ist30xx_tracking(TRACK_POS_FINGER + id);
	tsp_info("%s() %d\n", __func__, id);

	tsp_touched[id - 1] = false;
	tsp_count[id - 1] = 0;

	input_sync(ts_data->input_dev);
}

#define CANCEL_KEY  (0xff)
#define RELEASE_KEY (0)
static void release_key(int id, u8 key_status)
{
	input_report_key(ts_data->input_dev, ist30xx_key_code[id], key_status);

	ist30xx_tracking(TRACK_POS_KEY + id);
	tsp_info("%s() key%d, status: %d\n", __func__, id, key_status);

	tkey_pressed[id - 1] = false;

	input_sync(ts_data->input_dev);
}

static void clear_input_data(struct ist30xx_data *data)
{
	int id = 1;
	u32 status;

//add
	input_report_key(data->input_dev, BTN_TOUCH, 0);

	status = PARSE_FINGER_STATUS(data->t_status);
	while (status) {
		if (status & 1)
			release_finger(id);
		status >>= 1;
		id++;
	}

	id = 1;
	status = PARSE_KEY_STATUS(data->t_status);
	while (status) {
		if (status & 1)
			release_key(id, RELEASE_KEY);
		status >>= 1;
		id++;
	}
	data->t_status &= ~((0x7 << 21) | (0x1F << 16) | (0xF << 12) | 0x3FF);
	tsp_debug("%s: t_status: 0x%x, gt_status: 0x%x\n",
		  __func__, data->t_status, ts_data->t_status);

#if defined(TOUCH_BOOSTER)
	set_dvfs_lock(data, -1);
#endif
}

static int check_report_fingers(struct ist30xx_data *data, int finger_counts)
{
	int i;
	finger_info *fingers = (finger_info *)data->fingers;

	for (i = 0; i < finger_counts; i++) {
		if (unlikely((fingers[i].bit_field.x > IST30XX_MAX_X) ||
			     (fingers[i].bit_field.y > IST30XX_MAX_Y))) {
			tsp_warn("%s: Invalid touch data - %d: %d(%d, %d), 0x%08x\n",
				 __func__, i,
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
	int ret = 0;
	u8 *buf = (u8 *)msg;
	u8 chksum1 = msg[0] >> 24;
	u8 chksum2 = 0;

	msg[0] &= 0x00FFFFFF;

	cnt *= IST30XX_DATA_LEN;

	while (cnt--)
		chksum2 += *buf++;

	if (chksum1 != chksum2) {
		tsp_err("intr chksum: %02x, %02x\n", chksum1, chksum2);
		ret = -EPERM;
	}

	return (chksum1 == chksum2) ? 0 : -EPERM;
}

static void report_input_data(struct ist30xx_data *data, int finger_counts,
			      int key_counts)
{
	int id;
	bool press = false;
	finger_info *fingers = (finger_info *)data->fingers;

	int idx = 0;
	u32 status;

	touch_is_pressed = 0;

	status = PARSE_FINGER_STATUS(data->t_status);
	for (id = 0; id < data->max_fingers; id++) {
		press = (status & (1 << id)) ? true : false;

		input_mt_slot(data->input_dev, id);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, press);

		fingers[idx].bit_field.id = id + 1;
		print_tsp_event(&fingers[idx]);

		if (press == false)
			continue;

		input_report_abs(data->input_dev, ABS_MT_POSITION_X,
				 fingers[idx].bit_field.x);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
				 fingers[idx].bit_field.y);
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
				 fingers[idx].bit_field.area);
		input_report_key(data->input_dev, BTN_TOUCH, 1);

		idx++;

		if (press)
			touch_is_pressed = 1;
	}

#if IST30XX_USE_KEY
	status = PARSE_KEY_STATUS(data->t_status);
	for (id = 0; id < data->max_keys; id++) {
		press = (status & (1 << id)) ? true : false;

		input_report_key(data->input_dev, ist30xx_key_code[id + 1], press);

		print_tkey_event(id + 1);

		if (press)
			touch_is_pressed = 1;
	}
#endif  // IST30XX_USE_KEY

	if (finger_counts == 0)
		input_report_key(data->input_dev, BTN_TOUCH, 0);

	ist30xx_error_cnt = 0;
	ist30xx_scan_retry = 0;
	if(touch_is_pressed == 0)
		data->irq_cnt = 0;

	input_sync(data->input_dev);
}

/*
 * CMD : CMD_GET_COORD
 *
 * IST30XX_EXTEND_COORD == 0 (IST3026, IST3032, IST3026B, IST3032B)
 *
 *               [31:30]  [29:26]  [25:16]  [15:10]  [9:0]
 *   Multi(1st)  UDMG     Rsvd.    NumOfKey Rsvd.    NumOfFinger
 *    Single &   UDMG     ID       X        Area     Y
 *   Multi(2nd)
 *
 *   UDMG [31] 0/1 : single/multi
 *   UDMG [30] 0/1 : unpress/press
 *
 * IST30XX_EXTEND_COORD == 1 (IST3038, IST3044)
 *
 *   1st  [31:24]   [23:21]   [20:16]   [15:12]   [11:10]   [9:0]
 *        '0x71'    KeyCnt    KeyStatus FingerCnt Rsvd.     FingerStatus
 *   2nd  [31:28]   [27:24]   [23:12]   [11:0]
 *        ID        Area      X         Y
 */
u32 intr_debug_addr, intr_debug2_addr, intr_debug3_addr = 0;
u32 intr_debug_size, intr_debug2_size, intr_debug3_size = 0;
static irqreturn_t ist30xx_irq_thread(int irq, void *ptr)
{
	int i, ret;
	int key_cnt, finger_cnt, read_cnt;
	u32 msg[IST30XX_MAX_MT_FINGERS + 1];
	int offset = 1;
	u32 ms;
	struct ist30xx_data *data = ts_data;


	ist30xx_irq_working = true;

	if (unlikely(!data->irq_enabled))
		goto irq_end;

	ms = get_milli_second();

#if IST30XX_CHECK_BATT_TEMP
    if (ist30xx_batt_chk_cnt >= ist30xx_batt_chk_max_cnt) {
        ist30xx_batt_temp = ist30xx_get_battery_temp();
        ist30xx_write_cmd(data->client, IST30XXB_MEM_BATT_TEMP, 
                (((u32)ist30xx_batt_temp & 0xFFFF) << 8) | BATTERY_TEMP_MAGIC);

        tsp_verb("battery temperature: %d\n", ist30xx_batt_temp);
        ist30xx_batt_chk_cnt = 0;
    }
#endif

    if (intr_debug_addr > 0 && intr_debug_size > 0) {
        tsp_notc("Intr_debug (addr: 0x%08x)\n", intr_debug_addr);
		for (i = 0; i < intr_debug_size; i++) {
            ist30xx_read_buf(data->client, 
                intr_debug_addr + IST30XX_DATA_LEN * i, &msg[i], 1);

            tsp_notc("\t%08x\n", msg[i]);
        }
		ist30xx_put_track(msg, intr_debug_size);
	}
	memset(msg, 0, sizeof(msg));

	ret = ist30xx_get_position(data->client, msg, 1);
	if (unlikely(ret))
		goto irq_err;

	tsp_verb("intr msg: 0x%08x\n", *msg);
	data->irq_cnt++;

	if (unlikely(*msg == 0xE11CE970))     // TSP IC Exception
		goto irq_ic_err;

	if (unlikely(*msg == 0 || *msg == 0xFFFFFFFF)) // Unknown CMD
		goto irq_err;

	event_ms = ms;
	ist30xx_put_track_ms(event_ms);
	ist30xx_put_track(&tracking_intr_value, 1);
	ist30xx_put_track(msg, 1);

	if (unlikely((*msg & CALIB_MSG_MASK) == CALIB_MSG_VALID)) {
		data->status.calib_msg = *msg;
		tsp_info("%s: calib status: 0x%08x\n", __func__,
			 data->status.calib_msg);

		goto irq_end;
	} else if (unlikely(*msg == IST30XX_CHKIC_END)) {
		data->status.chkic_msg = *msg;
		tsp_info("%s: IC check status: 0x%08x\n", __func__,
			 data->status.chkic_msg);

		goto irq_end;
	}

	memset(data->fingers, 0, sizeof(data->fingers));

	if (unlikely(!CHECK_INTR_STATUS(*msg)))
		goto irq_err;

	data->t_status = *msg;
	key_cnt = PARSE_KEY_CNT(data->t_status);
	finger_cnt = PARSE_FINGER_CNT(data->t_status);
	read_cnt = finger_cnt;

	if (unlikely((finger_cnt > data->max_fingers) ||
		     (key_cnt > data->max_keys))) {
		tsp_warn("%s: Invalid touch count - finger: %d(%d), key: %d(%d)\n",
			 __func__, finger_cnt, data->max_fingers,
			 key_cnt, data->max_keys);
		goto irq_err;
	}

	if (read_cnt > 0) {
		ret = ist30xx_get_position(data->client, &msg[offset], read_cnt);
		if (unlikely(ret))
			goto irq_err;

		for (i = 0; i < read_cnt; i++)
			data->fingers[i].full_field = msg[i + offset];


		ist30xx_put_track(msg + offset, read_cnt);
	}

	ret = check_valid_coord(&msg[0], read_cnt + 1);
	if (unlikely(ret < 0))
		goto irq_err;

	if (unlikely(check_report_fingers(data, finger_cnt)))
		goto irq_end;

	report_input_data(data, finger_cnt, key_cnt);

    if (intr_debug2_addr > 0 && intr_debug2_size > 0) {
        tsp_notc("Intr_debug2 (addr: 0x%08x)\n", intr_debug2_addr);
		for (i = 0; i < intr_debug2_size; i++) {
            ist30xx_read_buf(data->client, 
                intr_debug2_addr + IST30XX_DATA_LEN * i, &msg[i], 1);

            tsp_notc("\t%08x\n", msg[i]);
        }
        ist30xx_put_track(msg, intr_debug2_size);
	}

    if (intr_debug3_addr > 0 && intr_debug3_size > 0) {
        tsp_notc("Intr_debug3 (addr: 0x%08x)\n", intr_debug3_addr);
		for (i = 0; i < intr_debug3_size; i++) {
            ist30xx_read_buf(data->client, 
                intr_debug3_addr + IST30XX_DATA_LEN * i, &msg[i], 1);

            tsp_notc("\t%08x\n", msg[i]);
        }
        ist30xx_put_track(msg, intr_debug3_size);
	}

	goto irq_end;

irq_err:
	tsp_err("intr msg: 0x%08x, ret: %d\n", msg[0], ret);
	ist30xx_request_reset();
irq_end:
	ist30xx_irq_working = false;
	event_ms = (u32)get_milli_second();

#if defined(TOUCH_BOOSTER)
	set_dvfs_lock(data, !!touch_is_pressed);
#endif
	return IRQ_HANDLED;

irq_ic_err:
	tsp_err("%s: Occured IC exception\n", __func__);
	ist30xx_scheduled_reset();
	ist30xx_irq_working = false;
	event_ms = (u32)get_milli_second();

#if defined(TOUCH_BOOSTER)
	set_dvfs_lock(data, !!touch_is_pressed);
#endif
	return IRQ_HANDLED;
}

static int ist30xx_pinctrl_configure(struct ist30xx_data *data, bool active)
{
#if SUPPORTED_TOUCH_KEY_LED
	struct pinctrl_state *set_state_led;
#endif
	struct pinctrl_state *set_state_int;
	struct pinctrl_state *set_state_en;
	int retval;
	tsp_err("%s: %s\n", __func__, active ? "ACTIVE" : "SUSPEND");

	if (active) {
#if SUPPORTED_TOUCH_KEY_LED
		set_state_led =
			pinctrl_lookup_state(data->pinctrl,
						"keyled_en_gpio_active");
		if (IS_ERR(set_state_led)) {
			tsp_err("%s: cannot get pinctrl(led_en) active state\n", __func__);
			return PTR_ERR(set_state_led);
		}
#endif
		set_state_int = 
			pinctrl_lookup_state(data->pinctrl,
						"tsp_int_active");
		if (IS_ERR(set_state_int)) {
			tsp_err("%s: cannot get pinctrl(tsp_int) active state\n", __func__);
			return PTR_ERR(set_state_int);
		}
		set_state_en = 
			pinctrl_lookup_state(data->pinctrl,
						"tsp_en_gpio_active");
		if (IS_ERR(set_state_en)) {
			tsp_err("%s: cannot get pinctrl(tsp_en) active state\n", __func__);
			return PTR_ERR(set_state_en);
		}
	} else {
#if SUPPORTED_TOUCH_KEY_LED
		set_state_led =
			pinctrl_lookup_state(data->pinctrl,
						"keyled_en_gpio_suspend");
		if (IS_ERR(set_state_led)) {
			tsp_err("%s: cannot get pinctrl(led_en) sleep state\n", __func__);
			return PTR_ERR(set_state_led);
		}
#endif
		set_state_en = 
			pinctrl_lookup_state(data->pinctrl,
						"tsp_en_gpio_suspend");
		if (IS_ERR(set_state_en)) {
			tsp_err("%s: cannot get pinctrl(tsp_en) sleep state\n", __func__);
			return PTR_ERR(set_state_en);
		}
	}

#if SUPPORTED_TOUCH_KEY_LED
	retval = pinctrl_select_state(data->pinctrl, set_state_led);
	if (retval) {
		tsp_err("%s: cannot set pinctrl(led_en) %s state\n",
				__func__, active ? "active" : "suspend");
		return retval;
	}
#endif
	retval = pinctrl_select_state(data->pinctrl, set_state_en);
	if (retval) {
		tsp_err("%s: cannot set pinctrl(tsp_en) %s state\n",
				__func__, active ? "active" : "suspend");
		return retval;
	}
	if (active) {
		retval = pinctrl_select_state(data->pinctrl, set_state_int);
		if (retval) {
			tsp_err("%s: cannot set_state_int %s state\n",
					__func__, active ? "active" : "suspend");
			return retval;
		}
	}

	return 0;
}
#if defined(USE_OPEN_CLOSE)
static void  imagis_ts_close(struct input_dev *dev)
{
	struct ist30xx_data *data = input_get_drvdata(dev);

	tsp_info("%s\n", __func__);

	mutex_lock(&ist30xx_mutex);
	ist30xx_disable_irq(data);
	del_timer(&event_timer);
	cancel_delayed_work_sync(&work_noise_protect);
	cancel_delayed_work(&work_reset_check);
	cancel_delayed_work_sync(&work_debug_algorithm);
	ist30xx_internal_suspend(data);
	clear_input_data(data);
	ist30xx_scan_retry = 0;
	ist30xx_pinctrl_configure(data, false);
	mutex_unlock(&ist30xx_mutex);
}
static int  imagis_ts_open(struct input_dev *dev)
{
	struct ist30xx_data *data = input_get_drvdata(dev);

	tsp_info("%s\n", __func__);

    ist30xx_power_status = 1;

	mutex_lock(&ist30xx_mutex);
	ist30xx_pinctrl_configure(data, true);
	ist30xx_internal_resume(data);
	ist30xx_start(data);
	ist30xx_enable_irq(data);
	mutex_unlock(&ist30xx_mutex);

	tsp_info("%s: IC version core: %x, param: %x, sub: %x\n", __func__,
		 data->fw.core_ver, data->fw.param_ver, data->fw.sub_ver);

	return 0;
}
#endif
void ist30xx_set_ta_mode(bool charging)
{
	tsp_info("%s(), charging = %d\n", __func__, charging);

	if (unlikely(charging == ist30xx_ta_status))
		return;

	if (unlikely(ist30xx_noise_mode == -1)) {
		ist30xx_ta_status = charging ? 1 : 0;
		return;
	}

	ist30xx_ta_status = charging ? 1 : 0;

	ist30xx_scheduled_reset();
}
EXPORT_SYMBOL(ist30xx_set_ta_mode);

void ist30xx_set_call_mode(int mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (unlikely(mode == ((ist30xx_noise_mode >> NOISE_MODE_CALL) & 1)))
		return;

	ist30xx_noise_mode &= ~(1 << NOISE_MODE_CALL);
	if (mode)
		ist30xx_noise_mode |= (1 << NOISE_MODE_CALL);

	ist30xx_scheduled_reset();
}
EXPORT_SYMBOL(ist30xx_set_call_mode);

void ist30xx_set_cover_mode(int mode)
{
	tsp_info("%s(), mode = %d\n", __func__, mode);

	if (unlikely(mode == ((ist30xx_noise_mode >> NOISE_MODE_COVER) & 1)))
		return;

	ist30xx_noise_mode &= ~(1 << NOISE_MODE_COVER);
	if (mode)
		ist30xx_noise_mode |= (1 << NOISE_MODE_COVER);

	ist30xx_scheduled_reset();
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
#else
void charger_enable(int enable)
{
	bool charging = enable ? true : false;

	ist30xx_set_ta_mode(charging);
}
#endif

static void reset_work_func(struct work_struct *work)
{
	if (unlikely((ts_data == NULL) || (ts_data->client == NULL)))
		return;

	tsp_info("%s: Request reset function\n", __func__);

	if (likely((ist30xx_initialized == 1) && (ts_data->status.power == 1) &&
		   (ts_data->status.update != 1) && (ts_data->status.calib != 1))) {
		mutex_lock(&ist30xx_mutex);
		ist30xx_disable_irq(ts_data);

		clear_input_data(ts_data);

		ist30xx_reset(false);

		ist30xx_start(ts_data);

		ist30xx_enable_irq(ts_data);
		mutex_unlock(&ist30xx_mutex);
	}
}

#if IST30XX_INTERNAL_BIN && IST30XX_UPDATE_BY_WORKQUEUE
static void fw_update_func(struct work_struct *work)
{
	if (unlikely((ts_data == NULL) || (ts_data->client == NULL)))
		return;

	tsp_info("%s: FW update function\n", __func__);

	if (likely(ist30xx_auto_bin_update(ts_data)))
		ist30xx_disable_irq(ts_data);
}
#endif // IST30XX_INTERNAL_BIN && IST30XX_UPDATE_BY_WORKQUEUE


u32 ist30xx_max_scan_retry = 2;
u32 ist30xx_algr_addr = 0, ist30xx_algr_size = 0;

#define SCAN_STATUS_MAGIC   (0x3C000000)
#define SCAN_STATUS_MASK    (0xFF000000)
#define FINGER_CNT_MASK     (0x00F00000)
#define SCAN_CNT_MASK       (0x000FFFFF)
#define GET_FINGER_CNT(k)   ((k & FINGER_CNT_MASK) >> 20)
#define GET_SCAN_CNT(k)     (k & SCAN_CNT_MASK)

static void noise_work_func(struct work_struct *work)
{
	int ret;
	u32 scan_status = 0;

	ret = ist30xx_read_cmd(ts_data->client, IST30XXB_MEM_COUNT, &scan_status);
	if (unlikely(ret)) {
		tsp_warn("%s: Mem scan count read fail!\n", __func__);
		goto retry_timer;
	}

	ist30xx_put_track_ms(timer_ms);
	ist30xx_put_track(&scan_status, 1);

	tsp_verb("%s: scan status: 0x%x\n", __func__, scan_status);

	/* Check valid scan count */
	if (unlikely((scan_status & SCAN_STATUS_MASK) != SCAN_STATUS_MAGIC)) {
		tsp_warn("%s: Scan status is not corrected!\n", __func__);
		goto retry_timer;
	}
	/* Status of IC is idle */
	if (GET_FINGER_CNT(scan_status) == 0) {
		if ((PARSE_FINGER_CNT(ts_data->t_status) > 0) ||
		    (PARSE_KEY_CNT(ts_data->t_status) > 0))
			clear_input_data(ts_data);
	}

	scan_status &= SCAN_CNT_MASK;

	/* Status of IC is lock-up */
	if (unlikely(scan_status == ist30xx_scan_count)) {
		tsp_warn("%s: TSP IC is not responded!\n", __func__);
		goto retry_timer;
	} else {
		ist30xx_scan_retry = 0;
	}

	ist30xx_scan_count = scan_status;

#if IST30XX_CHECK_BATT_TEMP
    if (ist30xx_batt_chk_cnt >= ist30xx_batt_chk_max_cnt) {
        ist30xx_batt_temp = ist30xx_get_battery_temp();
        ist30xx_write_cmd(ts_data->client, IST30XXB_MEM_BATT_TEMP, 
                (((u32)ist30xx_batt_temp & 0xFFFF) << 8) | BATTERY_TEMP_MAGIC);

        tsp_verb("battery temperature: %d\n", ist30xx_batt_temp);
        ist30xx_batt_chk_cnt = 0;
    }
#endif
	return;

retry_timer:
	ist30xx_scan_retry++;
	tsp_warn("%s: Retry scan status!(%d)\n", __func__, ist30xx_scan_retry);

	if (unlikely(ist30xx_scan_retry == ist30xx_max_scan_retry)) {
		ist30xx_scheduled_reset();
		ist30xx_scan_retry = 0;
	}
}

static void debug_work_func(struct work_struct *work)
{
#if IST30XX_ALGORITHM_MODE
	int ret = -EPERM;
	ALGR_INFO algr;
	u32 *buf32 = (u32 *)&algr;

	ret = ist30xxb_burst_read(ts_data->client,
				  ist30xx_algr_addr, (u32 *)&algr, ist30xx_algr_size);
	if (unlikely(ret)) {
		tsp_warn("%s: Algorithm mem addr read fail!\n", __func__);
		return;
	}

	ist30xx_put_track(buf32, ist30xx_algr_size);

	tsp_debug("%s:  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", __func__,
		  buf32[0], buf32[1], buf32[2], buf32[3], buf32[4]);

	tsp_debug("%s:   Scanstatus: %x\n", __func__, algr.scan_status);
	tsp_debug("%s:   TouchCnt: %d\n", __func__, algr.touch_cnt);
	tsp_debug("%s:   IntlTouchCnt: %d\n", __func__, algr.intl_touch_cnt);
	tsp_debug("%s:   StatusFlag: %d\n", __func__, algr.status_flag);
	tsp_debug("%s:   RAWPeakMax: %d\n", __func__, algr.raw_peak_max);
	tsp_debug("%s:   RAWPeakMin: %d\n", __func__, algr.raw_peak_min);
	tsp_debug("%s:   FLTPeakMax: %d\n", __func__, algr.flt_peak_max);
	tsp_debug("%s:   AdptThreshold: %d\n", __func__, algr.adpt_threshold);
	tsp_debug("%s:   KeyRawData0: %d\n", __func__, algr.key_raw_data[0]);
	tsp_debug("%s:   KeyRawData1: %d\n", __func__, algr.key_raw_data[1]);
#endif  // IST30XX_ALGORITHM_MODE
}

void timer_handler(unsigned long data)
{
	struct ist30xx_status *status = &ts_data->status;

	if (ist30xx_irq_working)
		goto restart_timer;

	if (status->event_mode) {
		if (likely((status->power == 1) && (status->update != 1))) {
			timer_ms = (u32)get_milli_second();
			if (unlikely(status->calib == 1)) {
				if ((status->calib_msg & CALIB_MSG_MASK) == CALIB_MSG_VALID) {
					tsp_info("Calibration check OK!!\n");
					schedule_delayed_work(&work_reset_check, 0);
					status->calib = 0;
				} else if (timer_ms - event_ms >= 3000) {   // 3second
					tsp_debug("%s: calibration timeout over 3sec\n", __func__);
					schedule_delayed_work(&work_reset_check, 0);
					status->calib = 0;
				}
			} else if (likely(status->noise_mode)) {
				if (timer_ms - event_ms > 100)     // 100ms after last interrupt
					schedule_delayed_work(&work_noise_protect, 0);
			}

#if IST30XX_ALGORITHM_MODE
			if ((ist30xx_algr_addr >= IST30XXB_ACCESS_ADDR) &&
			    (ist30xx_algr_size > 0))
                if (timer_ms - event_ms > 100)     // 100ms after last interrupt
    				schedule_delayed_work(&work_debug_algorithm, 0);
#endif
		}
	}

restart_timer:
#if IST30XX_CHECK_BATT_TEMP    
    ist30xx_batt_chk_cnt++;
#endif
	mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL);
}

static void ist30xx_request_gpio(struct i2c_client *	client,
				 struct ist30xx_data *	data)
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
	#if SUPPORTED_TOUCH_KEY_LED
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
}


#ifdef CONFIG_OF
static int ist30xx_parse_dt(struct device *dev, struct ist30xx_data *data)
{
	struct device_node *np = dev->of_node;

	data->dt_data->irq_gpio = of_get_named_gpio(np, "imagis,irq-gpio", 0);
	data->dt_data->touch_en_gpio = of_get_named_gpio(np, "vdd_en-gpio", 0);
#if SUPPORTED_TOUCH_KEY_LED
	data->dt_data->keyled_en_gpio = of_get_named_gpio(np, "keyled_en-gpio", 0);
#endif
#if SUPPORTED_TOUCH_KEY_LED
	tsp_info("%s() irq:%d, touch_en:%d, keyled_en:%d, system_rev = %d\n",
		 __func__, data->dt_data->irq_gpio, data->dt_data->touch_en_gpio,
		 data->dt_data->keyled_en_gpio, system_rev);
#else
	tsp_info("%s() irq:%d, touch_en:%d, system_rev = %d\n",
		 __func__, data->dt_data->irq_gpio, data->dt_data->touch_en_gpio,
		  system_rev);

#endif
	return 0;
}
#else
static int ist30xx_parse_dt(struct device *dev, struct ist30xx_data *data)
{
	return -ENODEV;
}
#endif

extern int sec_touch_sysfs(struct ist30xx_data *data);
extern int sec_fac_cmd_init(struct ist30xx_data *data);
#define CHECKIC_BEF_REGISTER_INPUTDEV

static int ist30xx_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret;
	int retry = 3;
	struct ist30xx_data *data;
	struct input_dev *input_dev;

	prev_dbg_level = ist30xx_dbg_level;
	ist30xx_dbg_level = DEV_DEBUG;

	tsp_verb("%s(), the i2c addr=0x%x\n", __func__, client->addr);

	data = kzalloc(sizeof(struct ist30xx_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	ts_data = data;
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

#if defined(CHECKIC_BEF_REGISTER_INPUTDEV)
	data->i2cPower_flag = false;

	ret = ist30xx_init_system();
	if (ret) {
		tsp_err("%s: chip initialization failed\n", __func__);
		goto err_reg_dev;
	}
#endif

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
	input_dev = input_allocate_device();
	if (!input_dev) {
		ret = -ENOMEM;
		tsp_err("%s(), input_allocate_device failed (%d)\n", __func__, ret);
		goto err_alloc_dt;
	}

	data->max_fingers = data->max_keys = IST30XX_MAX_MT_FINGERS;
	data->input_dev = input_dev;

	input_mt_init_slots(input_dev, IST30XX_MAX_MT_FINGERS, INPUT_MT_DIRECT);

	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#if SUPPORTED_TOUCH_KEY_LED
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);
#endif
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, IST30XX_MAX_X - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, IST30XX_MAX_Y - 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, IST30XX_MAX_W, 0, 0);

#if IST30XX_USE_KEY
	{
		int i;
		set_bit(EV_KEY, input_dev->evbit);
		set_bit(EV_SYN, input_dev->evbit);
		for (i = 1; i < ARRAY_SIZE(ist30xx_key_code); i++)
			set_bit(ist30xx_key_code[i], input_dev->keybit);
	}
#endif

	input_set_drvdata(input_dev, data);

	ret = input_register_device(input_dev);
	if (unlikely(ret)) {
		input_free_device(input_dev);
		goto err_reg_dev;
	}

#if defined(TOUCH_BOOSTER)
	init_dvfs(data);
#endif

	INIT_DELAYED_WORK(&work_reset_check, reset_work_func);
	INIT_DELAYED_WORK(&work_noise_protect, noise_work_func);
	INIT_DELAYED_WORK(&work_debug_algorithm, debug_work_func);

#if IST30XX_RAWDATA_DEBUG
	INIT_DELAYED_WORK(&work_read_rawdata, rawdata_work_func);
#endif

#if !defined(CHECKIC_BEF_REGISTER_INPUTDEV)
	data->i2cPower_flag = false;

	ret = ist30xx_init_system();
	if (ret) {
		tsp_err("%s: chip initialization failed\n", __func__);
		goto err_init_drv;
	}
#endif

	ret = ist30xx_init_update_sysfs();
	if (unlikely(ret))
		goto err_init_drv;

#if IST30XX_DEBUG
	ret = ist30xx_init_misc_sysfs();
	if (unlikely(ret))
		goto err_init_drv;
#endif

#if IST30XX_CMCS_TEST
	ret = ist30xx_init_cmcs_sysfs();
	if (unlikely(ret))
		goto err_init_drv;
#endif

#if SEC_FACTORY_MODE
	ret = sec_fac_cmd_init(data);
	if (unlikely(ret))
		goto err_init_drv;
	ret = sec_touch_sysfs(data);
	if (unlikely(ret))
		goto err_init_drv;
#endif

#if IST30XX_TRACKING_MODE
	ret = ist30xx_init_tracking_sysfs();
	if (unlikely(ret))
		goto err_init_drv;
#endif
	tsp_debug("%s: client->irq : %d\n", __func__, client->irq);
	ret = request_threaded_irq(client->irq, NULL, ist30xx_irq_thread,
				   IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "ist30xx_ts", data);
	if (unlikely(ret))
		goto err_irq;

	ist30xx_disable_irq(data);

	while ((data->chip_id != IST30XXB_CHIP_ID) &&
	       (data->chip_id != IST3038_CHIP_ID)) {
		ist30xx_read_cmd(data->client, IST30XXB_REG_CHIPID, &data->chip_id);
		if (data->chip_id == 0x3000B)
			data->chip_id = IST30XXB_CHIP_ID;

		if (retry-- == 0)
			break;
	}

	retry = 3;
	while (retry-- > 0) {
		ret = ist30xx_read_cmd(data->client, IST30XXB_REG_TSPTYPE,
				       &data->tsp_type);
		if (unlikely(ret)) continue;

		tsp_verb("%s: tsptype: %x\n", __func__, data->tsp_type);
		data->tsp_type = IST30XXB_PARSE_TSPTYPE(data->tsp_type);

		if (likely(ret == 0))
			break;

		data->tsp_type = TSP_TYPE_UNKNOWN;
	}

	tsp_info("%s: IC: %x, tsp_type: %x\n", __func__, data->chip_id,
		 data->tsp_type);

	data->status.event_mode = false;

#if IST30XX_INTERNAL_BIN
	data->fw.buf = (u8 *)ts_fw;
	data->fw.buf_size = ts_fw_size;
	data->fw.core_ver = 0;
	data->fw.param_ver = 0;
#endif

#if IST30XX_INTERNAL_BIN
#if IST30XX_UPDATE_BY_WORKQUEUE
	INIT_DELAYED_WORK(&work_fw_update, fw_update_func);
	schedule_delayed_work(&work_fw_update, IST30XX_UPDATE_DELAY);
#else
	ret = ist30xx_auto_bin_update(data);
	if (unlikely(ret != 0))
		tsp_err("%s: ist30xx auto bin update fail\n", __func__);
#endif
#endif
	if (ist30xx_power_status < 0)
		ist30xx_power_status = 0;

	if (ist30xx_ta_status < 0)
		ist30xx_ta_status = 0;

	if (ist30xx_noise_mode < 0)
		ist30xx_noise_mode = 0;

	ret = ist30xx_get_info(data);
	tsp_info("%s: Get info: %s\n", __func__, (ret == 0 ? "success" : "fail"));

#if defined(USE_OPEN_CLOSE)
	input_dev->open = imagis_ts_open;
	input_dev->close = imagis_ts_close;
#endif

	init_timer(&event_timer);
	event_timer.function = timer_handler;
	event_timer.expires = jiffies_64 + (EVENT_TIMER_INTERVAL);
	mod_timer(&event_timer, get_jiffies_64() + EVENT_TIMER_INTERVAL);

#ifdef USE_TSP_TA_CALLBACKS
	data->register_cb = ist30xx_register_callback;
	data->callbacks.inform_charger = charger_enable;
	if (data->register_cb)
		data->register_cb(&data->callbacks);
#endif
	ist30xx_initialized = 1;
	tsp_err("%s: Probe end\n", __func__);
	ist30xx_dbg_level = prev_dbg_level;

	return 0;

err_irq:
	tsp_debug("%s: ChipID: %x\n", __func__, data->chip_id);
	ist30xx_disable_irq(data);
	free_irq(client->irq, data);
err_init_drv:
	data->status.event_mode = false;
	tsp_err("%s: Error, ist30xx init driver\n", __func__);
	ist30xx_power_off();
	input_unregister_device(input_dev);
	input_free_device(input_dev);
	kfree(data->pinctrl);
err_pinctrl:
err_reg_dev:
err_alloc_dt:
	if (data->dt_data) {
		tsp_err("%s: Error, ist30xx mem free, line:%d\n", __func__, __LINE__);
		kfree(data->dt_data);
	}
err_alloc_dev:
	tsp_err("%s: Error, ist30xx mem free\n", __func__);
	kfree(data);
	return 0;
}


static int ist30xx_remove(struct i2c_client *client)
{
	struct ist30xx_data *data = i2c_get_clientdata(client);

	ist30xx_disable_irq(data);
	free_irq(client->irq, data);
	ist30xx_power_off();

	if (data->dt_data)
		ist30xx_free_gpio(data);

	input_unregister_device(data->input_dev);
	input_free_device(data->input_dev);
#if defined(TOUCH_BOOSTER)
	mutex_destroy(&data->dvfs_lock);
#endif	

	if (data->dt_data)
		kfree(data->dt_data);

	kfree(data);

	return 0;
}


static struct i2c_device_id ist30xx_idtable[] = {
	{ IST30XX_DEV_NAME, 0 },
	{},
};

#ifdef CONFIG_OF
static struct of_device_id ist30xx_match_table[] = {
	{ .compatible = "imagis,ist30xx-ts", },
	{ },
};
#else
#define ist30xx_match_table NULL
#endif


MODULE_DEVICE_TABLE(i2c, ist30xx_idtable);

static struct i2c_driver ist30xx_i2c_driver = {
	.id_table		= ist30xx_idtable,
	.probe			= ist30xx_probe,
	.remove			= ist30xx_remove,
	.driver			= {
		.owner		= THIS_MODULE,
		.name		= IST30XX_DEV_NAME,
		.of_match_table = ist30xx_match_table,
	},
};

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif

static int __init ist30xx_init(void)
{
#ifdef CONFIG_SAMSUNG_LPM_MODE
	if (poweroff_charging) {
		tsp_info("%s() LPM Charging Mode!!\n", __func__);
		return 0;
	}
#endif

	return i2c_add_driver(&ist30xx_i2c_driver);
}


static void __exit ist30xx_exit(void)
{
	i2c_del_driver(&ist30xx_i2c_driver);
}

module_init(ist30xx_init);
module_exit(ist30xx_exit);

MODULE_DESCRIPTION("Imagis IST30XX touch driver");
MODULE_LICENSE("GPL");
