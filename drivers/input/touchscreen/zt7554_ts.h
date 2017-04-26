/*
 *
 * Zinitix zt7554 touch driver
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#if defined(CONFIG_PM_RUNTIME)
#include <linux/pm_runtime.h>
#endif
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/regulator/consumer.h>
#include <linux/input/mt.h>
#include <linux/of_gpio.h>
#ifdef USE_SPRD_CPU	// agui
#include <mach/i2c-sprd.h>	// fix compile error
#endif
#include <linux/power_supply.h>
#include <linux/async.h>
#include <linux/input/tsp_ta_callback.h>

#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif

#define TOUCH_BOOSTER 0	// blocked SPRD booster, use QC CONFIG_COMMON_INPUT_BOOSTER

#if TOUCH_BOOSTER
#include <linux/cpufreq.h>
#include <linux/cpufreq_limit.h>

#ifdef USE_SPRD_CPU	// agui
extern int _store_cpu_num_min_limit(unsigned int input);
#else
int _store_cpu_num_min_limit(unsigned int input)
{
	return 1;
}
#endif	// 

struct cpufreq_limit_handle *min_handle = NULL;
static const unsigned long touch_cpufreq_lock = 1350000;
#endif


#define SEC_FACTORY_TEST
#define SUPPORTED_TOUCH_KEY
//#define SUPPORTED_KEY_LED

#define TS_DRVIER_VERSION		"1.0.18_1"
#define ZT7554_TS_DEVICE		"zt7554_ts"

#define TOUCH_POINT_MODE		1
#define ZINITIX_MISC_DEBUG		1
#define CHECK_HWID			0
#define ZINITIX_DEBUG			0
#define ZINITIX_I2C_CHECKSUM		1
#define TSP_INIT_TEST_RATIO		100
#define MAX_SUPPORTED_FINGER_NUM	5 /* max 10 */

#define MAX_SUPPORTED_BUTTON_NUM	2 /* max 8 */
#define SUPPORTED_BUTTON_NUM		2

#define SUPPORTED_PALM_TOUCH            1

/* resolution offset */
#define ABS_PT_OFFSET				(-1)
#define TOUCH_FORCE_UPGRADE			1
#define USE_CHECKSUM				1
#define CHIP_OFF_DELAY				50 /*ms*/
#define CHIP_ON_DELAY				200 /*ms*/
#define FIRMWARE_ON_DELAY			150 /*ms*/
#define DELAY_FOR_SIGNAL_DELAY		30 /*us*/
#define DELAY_FOR_TRANSCATION		50
#define DELAY_FOR_POST_TRANSCATION	10
#define RAWDATA_DELAY_FOR_HOST		100

/* PMIC Regulator based supply to TSP */
#define TSP_REGULATOR_SUPPLY		1
/* gpio controlled LDO based supply to TSP */
#define TSP_LDO_SUPPLY				0

/* ESD Protection */
/*second : if 0, no use. if you have to use, 3 is recommended*/
#define ESD_TIMER_INTERVAL			1
#define SCAN_RATE_HZ				100
#define CHECK_ESD_TIMER				3

/*Test Mode (Monitoring Raw Data) */
#ifdef SUPPORTED_DND_SHIFT_VALUE
#define SEC_DND_SHIFT_VALUE		3
#endif

//DND
#define SEC_DND_CP_CTRL_L			0x1fb3
#define SEC_DND_V_FORCE				0
#define SEC_DND_AMP_V_SEL			0x0141
#define SEC_DND_N_COUNT				21
#define SEC_DND_U_COUNT				8
#define SEC_DND_FREQUENCY			289

#define MAX_RAW_DATA_SZ				32*22
#define MAX_TRAW_DATA_SZ	\
	(MAX_RAW_DATA_SZ + 4*MAX_SUPPORTED_FINGER_NUM + 2)


#define TOUCH_SEC_MODE				48
#define TOUCH_REF_MODE				10
#define TOUCH_NORMAL_MODE			5
#define TOUCH_DELTA_MODE			3
#define TOUCH_DND_MODE				11

#define	PALM_REPORT_WIDTH	200
#define	PALM_REJECT_WIDTH	255

#define TOUCH_V_FLIP	0x01
#define TOUCH_H_FLIP	0x02
#define TOUCH_XY_SWAP	0x04

#define TSP_NORMAL_EVENT_MSG	1
#define I2C_RETRY_TIMES				8

/*  Other Things */
#define INIT_RETRY_CNT				1
#define I2C_SUCCESS					0
#define I2C_FAIL					1

#define TOUCH_IOCTL_BASE	0xbc
#define TOUCH_IOCTL_GET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE, 0, int)
#define TOUCH_IOCTL_SET_DEBUGMSG_STATE		_IOW(TOUCH_IOCTL_BASE, 1, int)
#define TOUCH_IOCTL_GET_CHIP_REVISION		_IOW(TOUCH_IOCTL_BASE, 2, int)
#define TOUCH_IOCTL_GET_FW_VERSION			_IOW(TOUCH_IOCTL_BASE, 3, int)
#define TOUCH_IOCTL_GET_REG_DATA_VERSION	_IOW(TOUCH_IOCTL_BASE, 4, int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_SIZE		_IOW(TOUCH_IOCTL_BASE, 5, int)
#define TOUCH_IOCTL_VARIFY_UPGRADE_DATA		_IOW(TOUCH_IOCTL_BASE, 6, int)
#define TOUCH_IOCTL_START_UPGRADE			_IOW(TOUCH_IOCTL_BASE, 7, int)
#define TOUCH_IOCTL_GET_X_NODE_NUM			_IOW(TOUCH_IOCTL_BASE, 8, int)
#define TOUCH_IOCTL_GET_Y_NODE_NUM			_IOW(TOUCH_IOCTL_BASE, 9, int)
#define TOUCH_IOCTL_GET_TOTAL_NODE_NUM		_IOW(TOUCH_IOCTL_BASE, 10, int)
#define TOUCH_IOCTL_SET_RAW_DATA_MODE		_IOW(TOUCH_IOCTL_BASE, 11, int)
#define TOUCH_IOCTL_GET_RAW_DATA			_IOW(TOUCH_IOCTL_BASE, 12, int)
#define TOUCH_IOCTL_GET_X_RESOLUTION		_IOW(TOUCH_IOCTL_BASE, 13, int)
#define TOUCH_IOCTL_GET_Y_RESOLUTION		_IOW(TOUCH_IOCTL_BASE, 14, int)
#define TOUCH_IOCTL_HW_CALIBRAION			_IOW(TOUCH_IOCTL_BASE, 15, int)
#define TOUCH_IOCTL_GET_REG					_IOW(TOUCH_IOCTL_BASE, 16, int)
#define TOUCH_IOCTL_SET_REG					_IOW(TOUCH_IOCTL_BASE, 17, int)
#define TOUCH_IOCTL_SEND_SAVE_STATUS		_IOW(TOUCH_IOCTL_BASE, 18, int)
#define TOUCH_IOCTL_DONOT_TOUCH_EVENT		_IOW(TOUCH_IOCTL_BASE, 19, int)

/* Register Map*/
#define ZT7554_SWRESET_CMD				0x0000
#define ZT7554_WAKEUP_CMD				0x0001
#define ZT7554_IDLE_CMD					0x0004
#define ZT7554_SLEEP_CMD				0x0005
#define ZT7554_CLEAR_INT_STATUS_CMD			0x0003
#define ZT7554_CALIBRATE_CMD				0x0006
#define ZT7554_SAVE_STATUS_CMD				0x0007
#define ZT7554_SAVE_CALIBRATION_CMD			0x0008
#define ZT7554_RECALL_FACTORY_CMD			0x000f
#define ZT7554_THRESHOLD				0x0020
#define ZT7554_DEBUG_REG				0x0115 /* 0~7 */
#define ZT7554_TOUCH_MODE				0x0010
#define ZT7554_CHIP_REVISION				0x0011
#define ZT7554_FIRMWARE_VERSION				0x0012
#define ZT7554_MINOR_FW_VERSION				0x0121
#define ZT7554_VENDOR_ID				0x001C
#define ZT7554_HW_ID					0x0014
#define ZT7554_DATA_VERSION_REG				0x0013
#define ZT7554_SUPPORTED_FINGER_NUM			0x0015
#define ZT7554_EEPROM_INFO				0x0018
#define ZT7554_INITIAL_TOUCH_MODE			0x0019
#define ZT7554_TOTAL_NUMBER_OF_X			0x0060
#define ZT7554_TOTAL_NUMBER_OF_Y			0x0061
#define ZT7554_DELAY_RAW_FOR_HOST			0x007f
#define ZT7554_BUTTON_SUPPORTED_NUM			0x00B0
#define ZT7554_BUTTON_SENSITIVITY			0x00B2
#define ZT7554_DUMMY_BUTTON_SENSITIVITY			0X00C8
#define ZT7554_X_RESOLUTION				0x00C0
#define ZT7554_Y_RESOLUTION				0x00C1
#define ZT7554_POINT_STATUS_REG				0x0080
#define ZT7554_ICON_STATUS_REG				0x00AA
#ifdef SUPPORTED_DND_SHIFT_VALUE
#define ZT7554_DND_SHIFT_VALUE				0x012B
#endif
#define ZT7554_AFE_FREQUENCY				0x0100
#define ZT7554_DND_N_COUNT				0x0122
#define ZT7554_DND_U_COUNT				0x0135
#define ZT7554_DND_V_FORCE				0x02F1
#define ZT7554_DND_AMP_V_SEL				0x02F9
#define ZT7554_DND_CP_CTRL_L				0x02bd
#define ZT7554_RAWDATA_REG				0x0200
#define ZT7554_EEPROM_INFO_REG				0x0018
#define ZT7554_INT_ENABLE_FLAG				0x00f0
#define ZT7554_PERIODICAL_INTERRUPT_INTERVAL		0x00f1
#define ZT7554_BTN_WIDTH				0x0316
#define ZT7554_CHECKSUM_RESULT				0x012c
#define ZT7554_INIT_FLASH				0x01d0
#define ZT7554_WRITE_FLASH				0x01d1
#define ZT7554_READ_FLASH				0x01d2
#define ZINITIX_INTERNAL_FLAG_02			0x011e
#define ZT7554_OPTIONAL_SETTING				0x0116

/* Interrupt & status register flag bit
-------------------------------------------------
*/
#define BIT_PT_CNT_CHANGE	0
#define BIT_DOWN			1
#define BIT_MOVE			2
#define BIT_UP				3
#define BIT_PALM			4
#define BIT_PALM_REJECT		5
#define RESERVED_0			6
#define RESERVED_1			7
#define BIT_WEIGHT_CHANGE	8
#define BIT_PT_NO_CHANGE	9
#define BIT_REJECT			10
#define BIT_PT_EXIST		11
#define RESERVED_2			12
#define BIT_MUST_ZERO		13
#define BIT_DEBUG			14
#define BIT_ICON_EVENT		15

/* button */
#define BIT_O_ICON0_DOWN	0
#define BIT_O_ICON1_DOWN	1
#define BIT_O_ICON2_DOWN	2
#define BIT_O_ICON3_DOWN	3
#define BIT_O_ICON4_DOWN	4
#define BIT_O_ICON5_DOWN	5
#define BIT_O_ICON6_DOWN	6
#define BIT_O_ICON7_DOWN	7

#define BIT_O_ICON0_UP		8
#define BIT_O_ICON1_UP		9
#define BIT_O_ICON2_UP		10
#define BIT_O_ICON3_UP		11
#define BIT_O_ICON4_UP		12
#define BIT_O_ICON5_UP		13
#define BIT_O_ICON6_UP		14
#define BIT_O_ICON7_UP		15

#define SUB_BIT_EXIST		0
#define SUB_BIT_DOWN		1
#define SUB_BIT_MOVE		2
#define SUB_BIT_UP			3
#define SUB_BIT_UPDATE		4
#define SUB_BIT_WAIT		5

#define zinitix_bit_set(val, n)		((val) &= ~(1<<(n)), (val) |= (1<<(n)))
#define zinitix_bit_clr(val, n)		((val) &= ~(1<<(n)))
#define zinitix_bit_test(val, n)	((val) & (1<<(n)))
#define zinitix_swap_v(a, b, t)		((t) = (a), (a) = (b), (b) = (t))
#define zinitix_swap_16(s)		(((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))

#ifdef SEC_FACTORY_TEST
/* Touch Screen */
#define TSP_CMD_STR_LEN			32
#define TSP_CMD_RESULT_STR_LEN	512
#define TSP_CMD_PARAM_NUM		8
#define TSP_CMD_Y_NUM			22
#define TSP_CMD_X_NUM			32
#define TSP_CMD_NODE_NUM		(TSP_CMD_Y_NUM * TSP_CMD_X_NUM)
#define REG_EDGE_XF_OFFSET      0xEC
#define REG_EDGE_XL_OFFSET      0xED
#define REG_EDGE_YF_OFFSET      0xEE
#define REG_EDGE_YL_OFFSET      0xEF

enum {
	WAITING = 0,
	RUNNING,
	OK,
	FAIL,
	NOT_APPLICABLE,
};
#endif

enum power_control {
	POWER_OFF,
	POWER_ON,
	POWER_ON_SEQUENCE,
};

enum key_event {
	ICON_BUTTON_UNCHANGE,
	ICON_BUTTON_DOWN,
	ICON_BUTTON_UP,
};

enum work_state {
	NOTHING = 0,
	NORMAL,
	ESD_TIMER,
	EALRY_SUSPEND,
	SUSPEND,
	RESUME,
	LATE_RESUME,
	UPGRADE,
	REMOVE,
	SET_MODE,
	HW_CALIBRAION,
	RAW_DATA,
	PROBE,
};

enum {
	BUILT_IN = 0,
	UMS,
	REQ_FW,
};

struct raw_ioctl {
	u32 sz;
	u32 buf;
};

struct reg_ioctl {
	u32 addr;
	u32 *val;
};

struct coord {
	u16	x;
	u16	y;
	u8	width;
	u8	sub_status;
};

struct point_info {
	u16	status;
#if TOUCH_POINT_MODE
	u16 event_flag;
#else
	u8	finger_cnt;
	u8	time_stamp;
#endif
	struct coord coord[MAX_SUPPORTED_FINGER_NUM];
};

struct capa_info {
	u16	vendor_id;
	u16	ic_revision;
	u16	fw_version;
	u16	fw_minor_version;
	u16	reg_data_version;
	u16	threshold;
	u16	key_threshold;
	u16	dummy_threshold;
	u32	ic_fw_size;
	u32	MaxX;
	u32	MaxY;
	u32	MinX;
	u32	MinY;
	u8	gesture_support;
	u16	multi_fingers;
	u16	button_num;
	u16	ic_int_mask;
	u16	x_node_num;
	u16	y_node_num;
	u16	total_node_num;
	u16	hw_id;
	u16	afe_frequency;
#ifdef SUPPORTED_DND_SHIFT_VALUE
	u16	shift_value;
#endif
	u16	v_force;
	u16	amp_v_sel;
	u16	N_cnt;
	u16	u_cnt;
	u16	cp_ctrl_l;
	u16	i2s_checksum;
};

struct zt7554_ts_dt_data {
	int		gpio_int;
	int		vdd_en;
	int		gpio_scl;
	int		gpio_sda;
	int		gpio_ldo_en;
	int             (*tsp_power)(struct i2c_client *client, int on);
	u32		x_resolution;
	u32		y_resolution;
	const char	*fw_name;
	const char	*ext_fw_name;
	const char	*model_name;
	u32		page_size;
	u32		orientation;
	u32		tsp_supply_type;
	u32		core_num;
#ifdef USE_TSP_TA_CALLBACKS
	struct tsp_callbacks callbacks;
#endif
};

struct zt7554_ts_info {
	struct i2c_client			*client;
	struct input_dev			*input_dev;
	struct zt7554_ts_dt_data		*pdata;
	char					phys[32];
	struct capa_info			cap_info;
	struct point_info			touch_info;
	struct point_info			reported_touch_info;
	u16					icon_event_reg;
	u16					prev_icon_event;
	int					irq;
	u8					button[MAX_SUPPORTED_BUTTON_NUM];
	u8					work_state;
	struct semaphore			work_lock;
#if ESD_TIMER_INTERVAL
	struct work_struct			tmr_work;
	struct timer_list			esd_timeout_tmr;
	struct timer_list			*p_esd_timeout_tmr;
	spinlock_t				lock;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend			early_suspend;
#endif
	struct semaphore			raw_data_lock;
	u16					touch_mode;
	s16					cur_data[MAX_TRAW_DATA_SZ];
	u8					update;
#ifdef SEC_FACTORY_TEST
	struct tsp_factory_info			*factory_info;
	struct tsp_raw_data			*raw_data;
#endif
	u16					dnd_data[MAX_RAW_DATA_SZ];
	s16					ref_data[MAX_RAW_DATA_SZ];

	s16					ref_scale_factor;
	s16					ref_btn_option;
	s16					hdiff_max_x;
	s16					hdiff_max_y;
	s16					hdiff_max_val;
	s16					hdiff_min_x;
	s16					hdiff_min_y;
	s16					hdiff_min_val;

	s16					vdiff_max_x;
	s16					vdiff_max_y;
	s16					vdiff_max_val;
	s16					vdiff_min_x;
	s16					vdiff_min_y;
	s16					vdiff_min_val;
	u8					finger_cnt;
	struct pinctrl 			*pinctrl;
#ifdef CONFIG_INPUT_BOOSTER
	struct input_booster	*booster;
#endif
#ifdef SUPPORTED_KEY_LED
	struct regulator	*led_ldo;
#endif
};

#ifdef SEC_FACTORY_TEST
struct tsp_factory_info {
	struct list_head cmd_list_head;
	char cmd[TSP_CMD_STR_LEN];
	char cmd_param[TSP_CMD_PARAM_NUM];
	char cmd_result[TSP_CMD_RESULT_STR_LEN];
	char cmd_buff[TSP_CMD_RESULT_STR_LEN];
	struct mutex cmd_lock;
	bool cmd_is_running;
	u8 cmd_state;
};

struct tsp_raw_data {
	u16 ref_data[TSP_CMD_NODE_NUM];
	u16 pref_data[TSP_CMD_NODE_NUM];
	s16 delta_data[TSP_CMD_NODE_NUM];
};

struct tsp_cmd {
	struct list_head list;
	const char *cmd_name;
	void (*cmd_func)(void *device_data);
};
#endif /* SEC_FACTORY_TEST */

struct zt7554_ts_info *misc_info;
#if ESD_TIMER_INTERVAL
static struct workqueue_struct *esd_tmr_workqueue;
#endif

#ifdef USE_SPRD_CPU	// agui
extern void (*tsp_charger_status_cb)(int);
#else
void (*tsp_charger_status_cb)(int);
#endif
static bool ta_connected;
static u16 m_optional_mode;
static u16 m_prev_optional_mode;
static int m_ts_debug_mode = ZINITIX_DEBUG;

extern struct class *sec_class;


#ifdef CONFIG_HAS_EARLYSUSPEND
static void zt7554_ts_early_suspend(struct early_suspend *h);
static void zt7554_ts_late_resume(struct early_suspend *h);
#endif
static bool zt7554_power_control(struct zt7554_ts_info *info, u8 ctl);
static int zt7554_power(struct i2c_client *client, int on);
static bool init_touch(struct zt7554_ts_info *info, bool forced);
static bool mini_init_touch(struct zt7554_ts_info *info);
static void clear_report_data(struct zt7554_ts_info *info);
#if ESD_TIMER_INTERVAL
static void esd_timer_start(u16 sec, struct zt7554_ts_info *info);
static void esd_timer_stop(struct zt7554_ts_info *info);
static void esd_timer_init(struct zt7554_ts_info *info);
static void esd_timeout_handler(unsigned long data);
#endif

#ifdef SEC_FACTORY_TEST
static void fw_update(void *device_data);
static void get_fw_ver_bin(void *device_data);
static void get_fw_ver_ic(void *device_data);
static void get_threshold(void *device_data);
static void get_chip_vendor(void *device_data);
static void get_chip_name(void *device_data);
static void get_x_num(void *device_data);
static void get_y_num(void *device_data);
static void dead_zone_enable(void *device_data);
static void get_config_ver(void *device_data);
static void not_support_cmd(void *device_data);
/* Vendor dependant command */
static void run_reference_read(void *device_data);
static void get_reference_max_V_Diff(void *device_data);
static void get_reference_max_H_Diff(void *device_data);
static void get_reference(void *device_data);
static void run_preference_read(void *device_data);
static void get_preference(void *device_data);
static void run_delta_read(void *device_data);
static void get_delta(void *device_data);
static void run_ref_calibration(void *device_data);
#endif

int ts_upgrade_sequence(const u8 *firmware_data);
bool ts_hw_calibration(struct zt7554_ts_info *info);
s32 write_reg(struct i2c_client *client, u16 reg, u16 value);
s32 write_cmd(struct i2c_client *client, u16 reg);
bool ts_set_touchmode(u16 value);
s32 read_data(struct i2c_client *client, u16 reg, u8 *values, u16 length);
