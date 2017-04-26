/*
 * mms_ts.c - Touchscreen driver for Melfas MMS-series touch controllers
 *
 * Copyright (C) 2011 Google Inc.
 * Author: Dima Zavin <dima@android.com>
 *         Simon Wilson <simonwilson@google.com>
 *
 * ISP reflashing code based on original code from Melfas.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#define DEBUG
/* #define VERBOSE_DEBUG */
#define SEC_TSP_DEBUG
#define USE_OPEN_CLOSE
/* #define FORCE_FW_FLASH */
/* #define FORCE_FW_PASS */
/* #define ESD_DEBUG */
#define DEBUG_PRINT2			1

#undef TSP_GESTURE_MODE	

#define SEC_TSP_FACTORY_TEST
#define TSP_BUF_SIZE 1024
#define RAW_FAIL -1
#include <linux/delay.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mfd/pm8xxx/gpio.h>
#include <linux/uaccess.h>
#include <linux/cpufreq.h>
#include <asm/mach-types.h>
#include <linux/delay.h>

#include <linux/regulator/consumer.h>
#include <linux/i2c/mms300.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>

#include <asm/unaligned.h>

#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif
#define TSP_GLOVE_MODE
#define TSP_SVIEW_COVER_MODE
#define COVER_OPEN 0
#define COVER_GLOVE 1
#define COVER_CLOSED 3

#define MMS300_DOWNLOAD
#define MMS300_RESET_DELAY	70

#define MAX_FINGERS		10
#define MAX_WIDTH		30
#define MAX_PRESSURE		255
#define MAX_HOVER			255
#define FINGER_EVENT_SZ			8
#define EVENT_SZ	8

/* Registers */
#define MMS_MODE_CONTROL	0x01
#define MMS_XYRES_HI		0x02
#define MMS_XRES_LO		0x03
#define MMS_YRES_LO		0x04

#define MMS_CORE_VERSION	0xE1
#define MMS_TSP_REVISION	0xF0
#define MMS_HW_REVISION		0xF1
#define MMS_COMPAT_GROUP	0xF2

#define MMS_TX_NUM			0x0B
#define MMS_RX_NUM			0x0C
#define MMS_KEY_NUM			0x0D
#define MMS_INPUT_EVENT_PKT_SZ	0x0F
#define MMS_INPUT_EVENT	0x10
#define MMS_UNIVERSAL_CMD		0xA0
#define MMS_UNIVERSAL_RESULT		0xAF

#define MMS_CMD_ENTER_ISC		0x5F
#define MMS_POWER_CONTROL		0xB0

#ifdef SEC_TSP_FACTORY_TEST
#define TX_NUM		27
#define RX_NUM		16
#define NODE_NUM	TX_NUM * RX_NUM

/* self diagnostic */
#define ADDR_CH_NUM		0x0B
#define ADDR_UNIV_CMD	0xA0
#define CMD_ENTER_TEST	0x40
#define CMD_EXIT_TEST	0x4F
#define CMD_CM_DELTA	0x41
#define CMD_GET_DELTA	0x42
#define CMD_CM_ABS		0X43
#define CMD_GET_ABS		0X44
#define CMD_CM_JITTER	0X45
#define CMD_GET_JITTER	0X46
#define CMD_GET_INTEN	0x70
#define CMD_GET_INTEN_KEY	0x71
#define CMD_RESULT_SZ	0XAE
#define CMD_RESULT		0XAF

/* VSC(Vender Specific Command)  */
#define MMS_VSC_CMD			0xB0	/* vendor specific command */
#define MMS_VSC_MODE			0x1A	/* mode of vendor */

#define MMS_VSC_CMD_ENTER		0X01
#define MMS_VSC_CMD_CM_DELTA		0X02
#define MMS_VSC_CMD_CM_ABS		0X03
#define MMS_VSC_CMD_EXIT		0X05
#define MMS_VSC_CMD_INTENSITY		0X04
#define MMS_VSC_CMD_RAW			0X06
#define MMS_VSC_CMD_REFER		0X07

#define TSP_CMD_STR_LEN 32
#define TSP_CMD_RESULT_STR_LEN 512
#define TSP_CMD_PARAM_NUM 8
#define tostring(x) #x
#endif /* SEC_TSP_FACTORY_TEST */

/* START - Added to support API's for TSP tuning */
#define ESD_DETECT_COUNT		10
#define FINGER_EVENT_SZ			8
#define MAX_LOG_LENGTH			128

/* Universal commands */
#define MMS_CMD_SET_LOG_MODE		0x20
#define MMS_EVENT_PKT_SZ		0x0F
#define MMS_INPUT_EVENT			0x10
#define MMS_UNIVERSAL_CMD		0xA0
#define MMS_UNIVERSAL_RESULT		0xAF
#define MMS_UNIVERSAL_RESULT_LENGTH	0xAE
#define MMS_UNIV_ENTER_TEST		0x40
#define MMS_UNIV_TEST_CM		0x41
#define MMS_UNIV_GET_DELTA		0x42
#define MMS_UNIV_GET_KEY_DELTA		0x4A
#define MMS_UNIV_GET_ABS		0x44
#define MMS_UNIV_GET_KEY_ABS		0x4B
#define MMS_UNIV_TEST_JITTER		0x45
#define MMS_UNIV_GET_JITTER		0x46
#define MMS_UNIV_GET_KEY_JITTER		0x4C
#define MMS_UNIV_EXIT_TEST		0x4F
#define MMS_UNIV_INTENSITY		0x70
#define MMS_UNIV_KEY_INTENSITY		0x71


/* Event types */
#define MMS_LOG_EVENT			0xD
#define MMS_NOTIFY_EVENT		0xE
#define MMS_ERROR_EVENT			0xF
#define MMS_TOUCH_KEY_EVENT		0x20 // for 345 IC

#ifdef TSP_GESTURE_MODE
#define LPM_MODE_REG			0x40	
#define LPM_OFF					0x1	
#define LPM_ON					0x2	

#define LPM_STATE_FLAG			0x40
#define LPM_DISTANCE_HIGH		0x41
#define LPM_DISTANCE_LOW		0x42
#define LPM_HOLD_FRAME_CNT		0x43
#endif

#ifdef MMS300_DOWNLOAD
#define MAX_SECTION_NUM		3
#define MMS_FW_VER		0xE1
/* Isc commands	*/
#define ISC_FULL_ERASE		{0xFB,0x4A,0x31,0x15,0x00,0x00}
#define ISC_READ_STATUS		{0xFB,0x4A,0x31,0xC8,0x00,0x00}
#define ISC_READ_PAGE		{0xFB,0x4A,0x31,0xC2,0x00,0x00}
#define ISC_WRITE_PAGE		{0xFB,0x4A,0x31,0xA5,0x00,0x00}
#endif

#define MMS300L_DOWNLOAD

#ifdef MMS300L_DOWNLOAD

typedef enum
{
	EC_NONE = -1,
	EC_DEPRECATED = 0,
	EC_BOOTLOADER_RUNNING = 1,
	EC_BOOT_ON_SUCCEEDED = 2,
	EC_ERASE_END_MARKER_ON_SLAVE_FINISHED = 3,
	EC_SLAVE_DOWNLOAD_STARTS = 4,
	EC_SLAVE_DOWNLOAD_FINISHED = 5,
	EC_2CHIP_HANDSHAKE_FAILED = 0x0E,
	EC_ESD_PATTERN_CHECKED = 0x0F,
	EC_LIMIT
} eErrCode_t;

typedef enum
{
	SEC_NONE = -1, SEC_BOOTLOADER = 0, SEC_CORE, SEC_CONFIG, SEC_LIMIT
} eSectionType_t;


#define SECTION_NUM                           		3
#define SECTION_START	SEC_CORE
#define SECTION_NAME_LEN                	        5

#define PAGE_HEADER                         		3
#define PAGE_DATA                              		128
#define PAGE_TAIL                            		2
#define PACKET_SIZE                           		(PAGE_HEADER + PAGE_DATA + PAGE_TAIL)
#define TIMEOUT_CNT                          		10
#define TS_WRITE_REGS_LEN				PACKET_SIZE

/*
 * State Registers
 */

#define MIP_ADDR_INPUT_INFORMATION           		0x01

#define ISC_ADDR_VERSION				0xE1
#define ISC_ADDR_SECTION_PAGE_INFO			0xE5

/*
 * Config Update Commands
 */
#define ISC_CMD_ENTER_ISC				0x5F
#define ISC_CMD_ENTER_ISC_PARA1				0x01
#define ISC_CMD_UPDATE_MODE				0xAE
#define ISC_SUBCMD_ENTER_UPDATE				0x55
#define ISC_SUBCMD_DATA_WRITE				0XF1
#define ISC_SUBCMD_LEAVE_UPDATE_PARA1			0x0F
#define ISC_SUBCMD_LEAVE_UPDATE_PARA2			0xF0
#define ISC_CMD_CONFIRM_STATUS				0xAF

#define ISC_STATUS_UPDATE_MODE				0x01
#define ISC_STATUS_CRC_CHECK_SUCCESS			0x03

#define ISC_CHAR_2_BCD(num)				(((num/10)<<4) + (num%10))
#define ISC_MAX(x, y)					( ((x) > (y))? (x) : (y) )

struct firmware_info {
	unsigned char version;
	unsigned char compatible_version;
	unsigned char start_addr;
	unsigned char end_addr;
	int bin_offset;
	u32 crc;
};

static const char section_name[SECTION_NUM][SECTION_NAME_LEN] =
{ "BOOT", "CORE", "CONF" };

static const unsigned char crc0_buf[31] =
		{ 0x1D, 0x2C, 0x05, 0x34, 0x95, 0xA4, 0x8D, 0xBC, 0x59, 0x68, 0x41,
		  0x70, 0xD1, 0xE0, 0xC9, 0xF8, 0x3F, 0x0E, 0x27, 0x16, 0xB7, 0x86, 
		  0xAF, 0x9E, 0x7B, 0x4A, 0x63, 0x52, 0xF3, 0xC2, 0xEB };

static const unsigned char crc1_buf[31] =
		{ 0x1E, 0x9C, 0xDF, 0x5D, 0x76, 0xF4, 0xB7, 0x35, 0x2A, 0xA8, 0xEB,
		  0x69, 0x42, 0xC0, 0x83, 0x01, 0x04, 0x86, 0xC5, 0x47, 0x6C, 0xEE,
		  0xAD, 0x2F, 0x30, 0xB2, 0xF1, 0x73, 0x58, 0xDA, 0x99 };

#endif

u8 irq_bit_mask;

enum {
	GET_RX_NUM	= 1,
	GET_TX_NUM,
	GET_EVENT_DATA,
};

enum {
	LOG_TYPE_U08	= 2,
	LOG_TYPE_S08,
	LOG_TYPE_U16,
	LOG_TYPE_S16,
	LOG_TYPE_U32	= 8,
	LOG_TYPE_S32,
};
/* END - Added to support API's for TSP tuning */


static struct device *sec_touchscreen;
#ifdef TOUCHKEY
static struct device *sec_touchkey;
#endif
/* Touch booster */
int touch_is_pressed;
static int tsp_power_enabled;

/* panel info */
#define ILJIN 0x4
#define EELY 0x0
#define NO_PANEL 0x7
#define YONGFAST 0x8
#define WINTEC 0x9

#define MAX_FW_PATH 255
#define TSP_FW_FILENAME "/sdcard/melfas_fw.bin"
#define MMS_COORDS_ARR_SIZE	4

#define FLASH_VERBOSE_DEBUG	1

#define FW_IMAGE_NAME	"tsp_melfas/mms345l_e5.fw";

#define ISC_PAGE_SZ 128

/* global variable for using firmware update data buffer */
static unsigned char *g_wr_buf;

enum {
	BUILT_IN = 0,
	UMS,
};

struct mms_bin_hdr {
	char tag[8];
	u16	core_version;
	u16	section_num;
	u16	contains_full_binary;
	u16	reserved0;

	u32	binary_offset;
	u32	binary_length;

	u32	extention_offset;
	u32	reserved1;
} __attribute__ ((packed));

struct mms_ext_hdr {
	u32	data_ID;
	u32	offset;
	u32	length;
	u32	next_item;
	u8	data[0];
} __attribute__ ((packed));

struct mms_fw_img {
	u16	type;
	u16	version;

	u16	start_page;
	u16	end_page;

	u32	offset;
	u32	length;
} __attribute__ ((packed));

struct isc_packet {
	u8	cmd;
	u32	addr;
	u8	data[0];
} __attribute__ ((packed));

struct mms_fw_image {
	__le32 hdr_len;
	__le32 data_len;
	__le32 fw_ver;
	__le32 hdr_ver;
	u8 data[0];
} __packed;

struct mms_ts_info {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct melfas_tsi_platform_data *pdata;
	struct regulator *vcc_i2c;
	struct completion init_done;
	struct firmware_info fw_info_ic[SECTION_NUM];
	struct firmware_info fw_info_bin[SECTION_NUM];
	struct mms_fw_img img_info[SECTION_NUM];
	struct pinctrl *pinctrl;

	bool section_update_flag[SECTION_NUM];
	char phys[32];
	int max_x;
	int max_y;
	bool invert_x;
	bool invert_y;
	u8 palm_flag;
	int irq;
	int (*power) (struct mms_ts_info *info,int on);
	void (*input_event)(void *data);
	const char* fw_path;

#ifdef CONFIG_INPUT_BOOSTER
	struct input_booster	*booster;
#endif


#ifdef TSP_GESTURE_MODE
	bool lowpower_mode;
	int lowpower_flag;

	int scrub_id;
	int scrub_x;
	int scrub_y;
#endif
	
#ifdef TOUCHKEY
	int (*keyled) (struct mms_ts_info *info,int on);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

#ifdef TOUCHKEY
	bool touchkey[3];
	int keycode[3];
	bool led_cmd;
#if defined(SEC_TSP_FACTORY_TEST)
	int menu_s;
	int back_s;
#endif
#endif

	/* protects the enabled flag */
	struct mutex lock;
	bool enabled;
	bool input_closed;
#ifdef USE_TSP_TA_CALLBACKS
	void (*register_cb) (struct tsp_callbacks *tsp_cb);
	struct tsp_callbacks callbacks;
#endif
	bool ta_status;
	bool noise_mode;
	bool threewave_mode;
#ifdef TSP_GLOVE_MODE
	bool glove_mode;
#endif
#ifdef TSP_SVIEW_COVER_MODE
	u8 cover_mode_retry;
	u8 cover_mode;
#endif
	bool sleep_wakeup_ta_check;

#if defined(SEC_TSP_DEBUG)
	unsigned char finger_state[MAX_FINGERS];
#endif

	u8 fw_update_state;
	u8 panel;
	u8 fw_boot_ver;
	u8 fw_core_ver;
	u8 fw_ic_ver;
#if defined(SEC_TSP_FACTORY_TEST)
	struct list_head cmd_list_head;
	u8 cmd_state;
	char cmd[TSP_CMD_STR_LEN];
	int cmd_param[TSP_CMD_PARAM_NUM];
	char cmd_result[TSP_CMD_RESULT_STR_LEN];
	struct mutex cmd_lock;
	bool cmd_is_running;

	unsigned int reference[NODE_NUM];
	unsigned int raw[NODE_NUM]; /* CM_ABS */
	unsigned int inspection[NODE_NUM];/* CM_DELTA */
	unsigned int intensity[NODE_NUM];
	bool ft_flag;
#endif				/* SEC_TSP_FACTORY_TEST */

#ifdef TSP_RAWDATA_DUMP
	struct delayed_work ghost_check;
#endif

	struct cdev			cdev;
	dev_t				mms_dev;
	struct class			*class;

	struct mms_log_data {
		u8			*data;
		int			cmd;
	} log;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mms_ts_early_suspend(struct early_suspend *h);
static void mms_ts_late_resume(struct early_suspend *h);
#endif
#if !defined(USE_OPEN_CLOSE)
static int mms_ts_resume(struct device *dev);
static int mms_ts_suspend(struct device *dev);
#endif

int mms_flash_fw_mms300(struct mms_ts_info *info, const u8 *fw_data, size_t fw_size);
static int fw_download_isp(struct mms_ts_info *info, const u8 *data, size_t len);
int fw_download_isc(struct mms_ts_info *info, const u8 *fw_data);


#if defined(SEC_TSP_FACTORY_TEST)
#define TSP_CMD(name, func) .cmd_name = name, .cmd_func = func

enum {
	WAITING = 0,
	RUNNING,
	OK,
	FAIL,
	NOT_APPLICABLE,
	NG,
};

struct tsp_cmd {
	struct list_head	list;
	const char	*cmd_name;
	void	(*cmd_func)(void *device_data);
};

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif

extern unsigned int system_rev;

static void fw_update(void *device_data);
static void get_fw_ver_bin(void *device_data);
static void get_fw_ver_ic(void *device_data);
static void get_config_ver(void *device_data);
static void get_threshold(void *device_data);
static void module_off_master(void *device_data);
static void module_on_master(void *device_data);
/*static void module_off_slave(void *device_data);
static void module_on_slave(void *device_data);*/
static void get_module_vendor(void *device_data);
static void get_chip_vendor(void *device_data);
static void get_chip_name(void *device_data);
static void get_reference(void *device_data);
static void get_cm_abs(void *device_data);
static void get_cm_delta(void *device_data);
static void get_intensity(void *device_data);
static void get_x_num(void *device_data);
static void get_y_num(void *device_data);
static void run_reference_read(void *device_data);
static void run_cm_abs_read(void *device_data);
static void run_cm_delta_read(void *device_data);
void run_intensity_read(void *device_data);
static void not_support_cmd(void *device_data);

#ifdef TSP_GESTURE_MODE
static void quick_shot_enable(void *device_data);
static void scrub_enable(void *device_data);
static void quick_app_access_enable(void *device_data);
static void direct_indicator_enable(void *device_data);

static void set_lowpower_mode(void *device_data);
#endif

#ifdef CONFIG_INPUT_BOOSTER
static void boost_level(void *device_data);
#endif

#ifdef TSP_GLOVE_MODE
static void glove_mode(void *device_data);
#endif

#ifdef TSP_SVIEW_COVER_MODE
static void clear_cover_mode(void *device_data);
#endif

struct tsp_cmd tsp_cmds[] = {
	{TSP_CMD("fw_update", fw_update),},
	{TSP_CMD("get_fw_ver_bin", get_fw_ver_bin),},
	{TSP_CMD("get_fw_ver_ic", get_fw_ver_ic),},
	{TSP_CMD("get_config_ver", get_config_ver),},
	{TSP_CMD("get_threshold", get_threshold),},
	{TSP_CMD("module_off_master", module_off_master),},
	{TSP_CMD("module_on_master", module_on_master),},
	{TSP_CMD("module_off_slave", not_support_cmd),},
	{TSP_CMD("module_on_slave", not_support_cmd),},
	{TSP_CMD("get_chip_vendor", get_chip_vendor),},
	{TSP_CMD("get_module_vendor", get_module_vendor),},
	{TSP_CMD("get_chip_name", get_chip_name),},
	{TSP_CMD("get_x_num", get_x_num),},
	{TSP_CMD("get_y_num", get_y_num),},
	{TSP_CMD("get_reference", get_reference),},
	{TSP_CMD("get_cm_abs", get_cm_abs),},
	{TSP_CMD("get_cm_delta", get_cm_delta),},
	{TSP_CMD("get_intensity", get_intensity),},
	{TSP_CMD("run_reference_read", run_reference_read),},
	{TSP_CMD("run_cm_abs_read", run_cm_abs_read),},
	{TSP_CMD("run_cm_delta_read", run_cm_delta_read),},
	{TSP_CMD("run_intensity_read", run_intensity_read),},
#ifdef TSP_GESTURE_MODE
	{TSP_CMD("quick_shot_enable", quick_shot_enable),},
	{TSP_CMD("scrub_enable", scrub_enable),},
	{TSP_CMD("quick_app_access_enable", quick_app_access_enable),},
	{TSP_CMD("direct_indicator_enable", direct_indicator_enable),},

	{TSP_CMD("set_lowpower_mode", set_lowpower_mode),},
#endif
	{TSP_CMD("not_support_cmd", not_support_cmd),},
#ifdef CONFIG_INPUT_BOOSTER
        {TSP_CMD("boost_level", boost_level),},
#endif
#ifdef TSP_GLOVE_MODE
	 {TSP_CMD("glove_mode", glove_mode),},
#endif
#ifdef TSP_SVIEW_COVER_MODE
	 {TSP_CMD("clear_cover_mode", clear_cover_mode),},
#endif
};
#endif


#ifdef TSP_RAWDATA_DUMP
static void ghost_touch_check(struct work_struct *work);
struct delayed_work * p_ghost_check;
#endif


static void release_all_fingers(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int i;

	dev_dbg(&client->dev, "[TSP] %s\n", __func__);

#ifdef TOUCHKEY
	for (i = 1; i < 3; i++) {
		if (info->touchkey[i] == 1) {
			info->touchkey[i] = 0;
			input_report_key(info->input_dev,
				info->keycode[i], 0);
		}
	}
#endif

	input_report_key(info->input_dev, BTN_TOUCH, 0);

	for (i = 0; i < MAX_FINGERS; i++) {
#ifdef SEC_TSP_DEBUG
		if (info->finger_state[i] == 1)
			dev_notice(&client->dev, "finger %d up(force)\n", i);
#endif
		info->finger_state[i] = 0;
		input_mt_slot(info->input_dev, i);
		input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER,
					   false);
	}
	input_sync(info->input_dev);

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, 0);
#endif
}

static void mms_set_noise_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int bit1=0;
	int bit2=0;

	dev_notice(&client->dev, "%s Noise mode:%d, TA:%d\n", __func__, info->noise_mode, info->ta_status);

	if (!info->noise_mode){
		bit1 = 0x04;
	}
	if (info->ta_status) {
		bit2 = 0x01;
       } else {
		bit2 = 0x02;
		info->noise_mode = 0;
	}
/*	1xx , noise mode is 0, 1 is not
*	x01 , insert TA
*	x10 , pull out TA
*/
	i2c_smbus_write_byte_data(info->client, 0x30, (bit1 | bit2));
	dev_notice(&client->dev, "%s Reg:%d\n", __func__, (bit1 | bit2));
}

static void mms_set_threewave_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	dev_notice(&client->dev, "%s\n", __func__);

	i2c_smbus_write_byte_data(info->client, 0x32, 0x1);
}

#ifdef TSP_GLOVE_MODE
static void mms_set_glove_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	dev_notice(&client->dev, "%s\n", __func__);

	i2c_smbus_write_byte_data(info->client, 0x34, 0x1); //Enter
}
static void mms_unset_glove_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	dev_notice(&client->dev, "%s\n", __func__);

	i2c_smbus_write_byte_data(info->client, 0x34, 0x2); //Exit
}
#endif

#ifdef TSP_SVIEW_COVER_MODE
static void mms_set_sview_cover_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	dev_notice(&client->dev, "%s\n", __func__);

	i2c_smbus_write_byte_data(info->client, 0x37, 0x1); //Enter
}
static void mms_unset_sview_cover_mode(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	dev_notice(&client->dev, "%s\n", __func__);

	i2c_smbus_write_byte_data(info->client, 0x37, 0x2); //Exit
}
#endif

static int mms_reset(struct mms_ts_info *info)
{
	printk(KERN_ERR" %s excute\n", __func__);
	info->power(info,0);
	msleep(10);
	info->power(info,1);
	msleep(MMS300_RESET_DELAY);

	return 0;
}

static void reset_mms_ts(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;

	if (info->enabled == false)
		return;

	dev_notice(&client->dev, "%s++\n", __func__);
	/* Disabling irq as it is not required since we're using oneshot irq
	* and this function is called only from the irq handler
	*/
	//disable_irq_nosync(info->irq);
	info->enabled = false;

	touch_is_pressed = 0;
	release_all_fingers(info);

	mms_reset(info);

	info->enabled = true;
	if (info->ta_status) {
		dev_notice(&client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}
	mms_set_noise_mode(info);

	//enable_irq(info->irq);

	dev_notice(&client->dev, "%s--\n", __func__);
}

#ifdef USE_TSP_TA_CALLBACKS
static void melfas_ta_cb(struct tsp_callbacks *cb, int ta_status)
{
	struct mms_ts_info *info =
			container_of(cb, struct mms_ts_info, callbacks);
	if (!info) {
		pr_err("%s not support\n", __func__);
		return;
	}

	dev_notice(&info->client->dev, "%s\n", __func__);

	info->ta_status = ta_status;

	if (info->enabled) {
		if (info->ta_status) {
			dev_notice(&info->client->dev, "TA connect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
		} else {
			dev_notice(&info->client->dev, "TA disconnect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
		}
		mms_set_noise_mode(info);
	}

}
#endif

#ifdef TSP_GESTURE_MODE
/*
Function 1 ==>  LPM State Setting
	Register : buf[0] = LPM_STATE_FLAG
	Setting val : buf[1] = 1(Exit) or 2(Enter)
	
Function 2 ==>  Set the Coordinate for gesture recognition
	Register : buf[0] = LPM_DISTANCE_HIGH
	Setting val : buf[1] = Coordinate(High) 
			     buf[2] = Coordinate(Low) 

Function 3 ==>  Set the hold time after gesture operations
	Register : buf[0] = LPM_HOLD_FRAME_CNT
	Setting val : buf[1] = hold time (10 = 150ms) 

*/

static void lpm_mode_enter(struct mms_ts_info *info, int on)
{

	if(on){
		dev_err(&info->client->dev, "[TSP] lpm mode enter!!!\n");
		i2c_smbus_write_byte_data(info->client, LPM_MODE_REG, LPM_ON);
	}else{
		dev_err(&info->client->dev, "[TSP] lpm mode enter!!!\n");
		i2c_smbus_write_byte_data(info->client, LPM_MODE_REG, LPM_OFF);
	}
}

#endif
static irqreturn_t mms_ts_interrupt(int irq, void *dev_id)
{
	struct mms_ts_info *info = dev_id;
	struct i2c_client *client = info->client;
	int ret;
	int i;
	int sz = 0;
	u8 buf[MAX_FINGERS * EVENT_SZ] = { 0 };
	u8 reg = MMS_INPUT_EVENT;

	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &reg,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = buf,
		},
	};
	//dev_info(&client->dev, "%s, %d\n", __func__, __LINE__);
	mutex_lock(&info->lock);
	if (!info->enabled)
		goto out;

	sz = i2c_smbus_read_byte_data(client, MMS_INPUT_EVENT_PKT_SZ);
	if (sz < 0) {
		dev_err(&client->dev, "%s bytes=%d\n", __func__, sz);
		for (i = 0; i < 50; i++) {
			sz = i2c_smbus_read_byte_data(client,
						      MMS_INPUT_EVENT_PKT_SZ);
			if (sz > 0)
				break;
		}

		if (i == 50) {
			dev_dbg(&client->dev, "i2c failed... reset!!\n");
			reset_mms_ts(info);
			goto out;
		}
	}

	if (sz == 0)
		goto out;

	if (sz > MAX_FINGERS * EVENT_SZ) {
		dev_err(&client->dev, "abnormal data inputed\n");
		goto out;
	}

	msg[1].len = sz;
	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev,
			"failed to read %d bytes of touch data (%d)\n",
			sz, ret);
		if (ret < 0) {
			dev_err(&client->dev,
				"%s bytes=%d\n", __func__, sz);
			for (i = 0; i < 5; i++) {
				msleep(20);
				ret = i2c_transfer(client->adapter,
					msg, ARRAY_SIZE(msg));
				if (ret > 0)
					break;
			}
			if (i == 5) {
				dev_err(&client->dev,
					"[TSP] i2c failed E2... reset!!\n");
				reset_mms_ts(info);
				goto out;
			}
		}
	}
#if defined(VERBOSE_DEBUG)
	print_hex_dump(KERN_DEBUG, "mms_ts raw: ",
		       DUMP_PREFIX_OFFSET, 32, 1, buf, sz, false);

#endif
	if (buf[0] == 0x0F) {	/* ESD */
		if(buf[1] == 0x00)
		{
			dev_info(&client->dev, "ESD DETECT.... reset!!\n");
			reset_mms_ts(info);
		}
		else
			dev_info(&client->dev, "Recal Notify %d.... reset!!\n", buf[1]);
		goto out;
	}

	if (buf[0] == 0x0E) { /* NOISE MODE */
		dev_info(&client->dev, "[TSP] noise mode enter!!\n");
		info->noise_mode = 1;
		mms_set_noise_mode(info);
		goto out;
	}

	if (buf[0] == 0x0B) { /* THREEWAVE MODE */
		if (buf[1] == 0x01) {
			dev_info(&client->dev, "[TSP] three-wave mode enter!!\n");
			info->threewave_mode = 1;
		} else {
			dev_info(&client->dev, "[TSP] three-wave mode exit!!\n");
			info->threewave_mode = 0;
		}
		goto out;
	}

#if 0
#ifdef TSP_GLOVE_MODE
	if (buf[0] == 0x0A) { /* GLOVE TOUCH MODE */
		if (buf[1] == 0x01) {
			dev_info(&client->dev, "[TSP] Glove-touch mode enter!!\n");
			info->glove_mode = 1;
		} else {
			dev_info(&client->dev, "[TSP] Glove-touch mode exit!!\n");
			info->glove_mode = 0;
		}
		goto out;
	}
#endif
#endif

#ifdef TSP_GESTURE_MODE
// State = 1 && Mode Type =1  ==> Gesture 2 mode
// State = 0 && Mode Type =1  ==> Gesture 1 mode

	if ((buf[0] & 0x40)== 0x40) { /* LPM */
		int value = 0;

		value = (buf[0] & 0x3);
		
		if(value==2){		// only support id 2 (Quick App Access)
			input_report_key(info->input_dev, KEY_BLACK_UI_GESTURE, 1);
		
			info->scrub_id = value;
			info->scrub_x = buf[2] | ((buf[1] & 0xf) << 8);
			info->scrub_y = buf[3] | ((buf[1] >> 4) << 8);
			
			dev_info(&client->dev, "[TSP] Gesture %d mode(%d,%d)!!!\n", value, info->scrub_x, info->scrub_y);

			input_sync(info->input_dev);
			usleep(100);
			input_report_key(info->input_dev, KEY_BLACK_UI_GESTURE, 0);
		}else {
			dev_info(&client->dev, "[TSP] Gesture not support, value=%2X,%2X\n", value, buf[0]);
		}
		goto out;
	}
#endif

	

	for (i = 0; i < sz; i += EVENT_SZ) {
		u8 *tmp = &buf[i];
		int id, x, y, angle, palm;
#ifdef TOUCHKEY
		u8 keycode;
		u8 key_press;

		if ((tmp[0] & 0x20) == 0x0) { /* touch key */
			keycode = tmp[0] & 0xf;
			key_press = (tmp[0] >> 7);

			if (key_press == 0) {
				input_report_key(info->input_dev,
					info->keycode[keycode], 0);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
				dev_notice(&client->dev,
					"key R(%d)\n", info->panel);
#else
				dev_notice(&client->dev,
					"key R : %d(%d)(%d)\n",
					info->keycode[keycode],
					tmp[0] & 0xf, info->panel);
#endif
			} else {
				input_report_key(info->input_dev,
					info->keycode[keycode], 1);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
				dev_notice(&client->dev,
					"key P(%d)\n", info->panel);
#else
				dev_notice(&client->dev,
					"key P : %d(%d)(%d)\n",
					info->keycode[keycode],
					tmp[0] & 0xf, info->panel);
#endif
			}
			continue;
		}
#else
		if ((tmp[0] & 0x20) == 0x0)
			continue;
#endif
		id = (tmp[0] & 0xf) - 1;
		x = tmp[2] | ((tmp[1] & 0xf) << 8);
		y = tmp[3] | ((tmp[1] >> 4) << 8);
		angle = (tmp[5] >= 127) ? (-(256 - tmp[5])) : tmp[5];
		palm = (tmp[0] & 0x10) >> 4;

		if (info->invert_x) {
			x = info->max_x - x;
			if (x < 0)
				x = 0;
		}
		if (info->invert_y) {
			y = info->max_y - y;
			if (y < 0)
				y = 0;
		}

		if (palm) {
			if (info->palm_flag == 3) {
				info->palm_flag = 1;
			} else {
				info->palm_flag = 3;
				palm = 3;
			}
		} else {
			if (info->palm_flag == 2) {
				info->palm_flag = 0;
			} else {
				info->palm_flag = 2;
				palm = 2;
			}
		}

		if (id >= MAX_FINGERS) {
			dev_notice(&client->dev,
				"finger id error [%d]\n", id);
			reset_mms_ts(info);
			goto out;
		}

		//if (x == 0 && y == 0)
		//	continue;
		if ((tmp[0] & irq_bit_mask) == 0) {
			input_mt_slot(info->input_dev, id);
			input_mt_report_slot_state(info->input_dev,
						   MT_TOOL_FINGER, false);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			if (info->finger_state[id] != 0) {
				info->finger_state[id] = 0;
				touch_is_pressed--;
				dev_notice(&client->dev,
					"R [%2d](%d),(%d)", id, info->panel, info->fw_ic_ver);
			}
#else /*CONFIG_SAMSUNG_PRODUCT_SHIP */
			if (info->finger_state[id] != 0) {
				info->finger_state[id] = 0;
				touch_is_pressed--;
				dev_notice(&client->dev,
					"R[%d] x=%d y=%d (%d)(%d)",
					id, x, y, info->panel, info->fw_ic_ver);
			}

#endif /*CONFIG_SAMSUNG_PRODUCT_SHIP */
			continue;
		}

		input_mt_slot(info->input_dev, id);
		input_mt_report_slot_state(info->input_dev,
					   MT_TOOL_FINGER, true);
		input_report_abs(info->input_dev,
			ABS_MT_POSITION_X, x);
		input_report_abs(info->input_dev,
			ABS_MT_POSITION_Y, y);
		input_report_abs(info->input_dev,
			ABS_MT_WIDTH_MAJOR, tmp[4]);
		input_report_abs(info->input_dev,
			ABS_MT_TOUCH_MAJOR, tmp[6]);
		input_report_abs(info->input_dev,
			ABS_MT_TOUCH_MINOR, tmp[7]);
		input_report_abs(info->input_dev,
			ABS_MT_PALM, palm);

		input_report_key(info->input_dev, BTN_TOUCH, 1);

#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
		if (info->finger_state[id] == 0) {
			info->finger_state[id] = 1;
			touch_is_pressed++;
			dev_notice(&client->dev,
				"P [%2d](%d)", id, info->panel);
		}
#else /* CONFIG_SAMSUNG_PRODUCT_SHIP */
		if (info->finger_state[id] == 0) {
			info->finger_state[id] = 1;
			touch_is_pressed++;
			dev_notice(&client->dev,
				"P[%d] x=%d y=%d w=%d, major=%d, minor=%d, angle=%d, palm=%d(%d)",
				id, x, y, tmp[4], tmp[6], tmp[7],
				angle, palm, info->panel);
		}
#endif /* CONFIG_SAMSUNG_PRODUCT_SHIP */
	}
	input_sync(info->input_dev);

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, touch_is_pressed);
#endif

out:
	mutex_unlock(&info->lock);
	return IRQ_HANDLED;
}

int get_tsp_status(void)
{
	return touch_is_pressed;
}
EXPORT_SYMBOL(get_tsp_status);

static int get_fw_version(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg[2];
	u8 reg = MMS_CORE_VERSION;
	int ret;
	unsigned char buf[3] = { 0, };

	msg[0].addr = client->addr;
	msg[0].flags = 0x00;
	msg[0].len = 1;
	msg[0].buf = &reg;
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = MAX_SECTION_NUM;
	msg[1].buf = buf;

	ret = i2c_transfer(adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_info(&info->client->dev, "[mms] %s : read error : [%d]", __func__, ret);
		return ret;
	}

	info->fw_boot_ver = buf[0];
	info->fw_core_ver = buf[1];
	info->fw_ic_ver = buf[2];

	dev_err(&info->client->dev,
		"%s: [mms] boot : 0x%x, core : 0x%x, config : 0x%x\n",
		__func__, buf[0], buf[1], buf[2]);
	return 0;
}


#ifdef SEC_TSP_FACTORY_TEST
static void set_cmd_result(struct mms_ts_info *info, char *buff, int len)
{
	strncat(info->cmd_result, buff, len);
}

static int get_data(struct mms_ts_info *info, u8 addr, u8 size, u8 *array)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	u8 reg = addr;
	unsigned char buf[size];
	int ret;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = &reg;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0) {
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = size;
		msg.buf = buf;

		ret = i2c_transfer(adapter, &msg, 1);
	}
	if (ret < 0) {
		pr_err("[TSP] : read error : [%d]", ret);
		return ret;
	}

	memcpy(array, &buf, size);
	return size;
}

static void get_intensity_data(struct mms_ts_info *info)
{
	u8 w_buf[4];
	u8 r_buf;
	u8 read_buffer[60] = {0};
	int i, j;
	int ret;
	u16 max_value = 0, min_value = 0;
	u16 raw_data;
	char buff[TSP_CMD_STR_LEN] = {0};

	disable_irq(info->irq);

	w_buf[0] = ADDR_UNIV_CMD;
	w_buf[1] = CMD_GET_INTEN;
	w_buf[2] = 0xFF;
	for (i = 0; i < RX_NUM; i++) {
		w_buf[3] = i;

		ret = i2c_smbus_write_i2c_block_data(info->client,
			w_buf[0], 3, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;
		usleep_range(1, 5);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			CMD_RESULT_SZ, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = get_data(info, CMD_RESULT, r_buf, read_buffer);
		if (ret < 0)
			goto err_i2c;

		pr_info("mms300 RX:%02d =",i);

		for (j = 0; j < r_buf/2; j++) {
			raw_data = read_buffer[2*j] | (read_buffer[2*j+1] << 8);
			if (raw_data > 32767)
				raw_data = 0;
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}
			info->intensity[i * TX_NUM + j] = raw_data;
			if((j%5==0)&&(j!=0)) pr_cont(",");
			pr_cont(" %2d", info->intensity[i * TX_NUM + j]);
		}
		pr_cont("\n");
	}

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, MMS_VSC_CMD_INTENSITY);
}

static int get_raw_data(struct mms_ts_info *info, u8 cmd)
{
	u8 w_buf[4];
	u8 r_buf = 0;
	u8 read_buffer[60] = {0};
	int ret;
	int i, j;
	int max_value = 0, min_value = 0;
	int raw_data;
	int retry;
	char buff[TSP_CMD_STR_LEN] = {0};
	int gpio = info->pdata->gpio_int;

	disable_irq(info->irq);

	ret = i2c_smbus_write_byte_data(info->client,
		ADDR_UNIV_CMD, CMD_ENTER_TEST);
	if (ret < 0)
		goto err_i2c;

	/* event type check */
	retry = 1;
	while (retry) {
		while (gpio_get_value(gpio))
			udelay(100);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			0x0F, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = i2c_smbus_read_i2c_block_data(info->client,
			0x10, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		dev_info(&info->client->dev, "event type = 0x%x\n", r_buf);
		if (r_buf == 0x0C)
			retry = 0;
	}

	w_buf[0] = ADDR_UNIV_CMD;
	if (cmd == MMS_VSC_CMD_CM_DELTA)
		w_buf[1] = CMD_CM_DELTA;
	else
		w_buf[1] = CMD_CM_ABS;
	ret = i2c_smbus_write_i2c_block_data(info->client,
		 w_buf[0], 1, &w_buf[1]);
	if (ret < 0)
		goto err_i2c;
	while (gpio_get_value(gpio))
		udelay(100);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		CMD_RESULT_SZ, 1, &r_buf);
	if (ret < 0)
		goto err_i2c;
	ret = i2c_smbus_read_i2c_block_data(info->client,
		CMD_RESULT, 1, &r_buf);
	if (ret < 0)
		goto err_i2c;

	if (r_buf == 1)
		dev_info(&info->client->dev, "PASS\n");
	else
		dev_info(&info->client->dev, "FAIL\n");

	if (cmd == MMS_VSC_CMD_CM_DELTA)
		w_buf[1] = CMD_GET_DELTA;
	else
		w_buf[1] = CMD_GET_ABS;
	w_buf[2] = 0xFF;

	for (i = 0; i < RX_NUM; i++) {
		w_buf[3] = i;

		ret = i2c_smbus_write_i2c_block_data(info->client,
			w_buf[0], 3, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;

		while (gpio_get_value(gpio))
			udelay(100);

		ret = i2c_smbus_read_i2c_block_data(info->client,
			CMD_RESULT_SZ, 1, &r_buf);
		if (ret < 0)
			goto err_i2c;

		ret = get_data(info, CMD_RESULT, r_buf, read_buffer);
		if (ret < 0)
			goto err_i2c;

		for (j = 0; j < TX_NUM; j++) {
			raw_data = read_buffer[2*j] | (read_buffer[2*j+1] << 8);
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}

			if (cmd == MMS_VSC_CMD_CM_DELTA) {
				info->inspection[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] delta[%d][%d] = %d\n", j, i,
					info->inspection[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_ABS) {
				info->raw[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] raw[%d][%d] = %d\n", j, i,
					info->raw[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_REFER) {
				info->reference[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] reference[%d][%d] = %d\n", j, i,
					info->reference[i * TX_NUM + j]);
			}
		}
	}

	ret = i2c_smbus_write_byte_data(info->client,
		ADDR_UNIV_CMD, CMD_EXIT_TEST);

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	touch_is_pressed = 0;
	release_all_fingers(info);

	mms_reset(info);
	info->enabled = true;

	if (info->ta_status) {
		dev_notice(&info->client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&info->client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}
	mms_set_noise_mode(info);

	enable_irq(info->irq);

	if((min_value < 2 && max_value < 2)||(min_value > 1680 && max_value < 1730))
		return -ECHRNG;

	return 0;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
	enable_irq(info->irq);
	return -EIO;
}

static void get_raw_data_all(struct mms_ts_info *info, u8 cmd)
{
	u8 w_buf[6];
	u8 read_buffer[2];	/* 52 */
	int ret;
	int i, j;
	u32 max_value = 0, min_value = 0;
	u32 raw_data;
	char buff[TSP_CMD_STR_LEN] = {0};
	int gpio = info->pdata->gpio_int;

/*      gpio = msm_irq_to_gpio(info->irq); */
	disable_irq(info->irq);

	w_buf[0] = MMS_VSC_CMD;	/* vendor specific command id */
	w_buf[1] = MMS_VSC_MODE;	/* mode of vendor */
	w_buf[2] = 0;		/* tx line */
	w_buf[3] = 0;		/* rx line */
	w_buf[4] = 0;		/* reserved */
	w_buf[5] = 0;		/* sub command */

	if (cmd == MMS_VSC_CMD_EXIT) {
		w_buf[5] = MMS_VSC_CMD_EXIT;	/* exit test mode */

		ret = i2c_smbus_write_i2c_block_data(info->client,
						     w_buf[0], 5, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;
		enable_irq(info->irq);
		msleep(200);
		return;
	}

	/* MMS_VSC_CMD_CM_DELTA or MMS_VSC_CMD_CM_ABS
	 * this two mode need to enter the test mode
	 * exit command must be followed by testing.
	 */
	if (cmd == MMS_VSC_CMD_CM_DELTA || cmd == MMS_VSC_CMD_CM_ABS) {
		/* enter the debug mode */
		w_buf[2] = 0x0;	/* tx */
		w_buf[3] = 0x0;	/* rx */
		w_buf[5] = MMS_VSC_CMD_ENTER;

		ret = i2c_smbus_write_i2c_block_data(info->client,
						     w_buf[0], 5, &w_buf[1]);
		if (ret < 0)
			goto err_i2c;

		/* wating for the interrupt */
		while (gpio_get_value(gpio))
			udelay(100);
	}

	for (i = 0; i < RX_NUM; i++) {
		for (j = 0; j < TX_NUM; j++) {

			w_buf[2] = j;	/* tx */
			w_buf[3] = i;	/* rx */
			w_buf[5] = cmd;

			ret = i2c_smbus_write_i2c_block_data(info->client,
					w_buf[0], 5, &w_buf[1]);
			if (ret < 0)
				goto err_i2c;

			usleep_range(1, 5);

			ret = i2c_smbus_read_i2c_block_data(info->client, 0xBF,
							    2, read_buffer);
			if (ret < 0)
				goto err_i2c;

			raw_data = ((u16) read_buffer[1] << 8) | read_buffer[0];
			if (i == 0 && j == 0) {
				max_value = min_value = raw_data;
			} else {
				max_value = max(max_value, raw_data);
				min_value = min(min_value, raw_data);
			}

			if (cmd == MMS_VSC_CMD_INTENSITY) {
				info->intensity[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] intensity[%d][%d] = %d\n", j, i,
					info->intensity[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_DELTA) {
				info->inspection[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] delta[%d][%d] = %d\n", j, i,
					info->inspection[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_CM_ABS) {
				info->raw[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] raw[%d][%d] = %d\n", j, i,
					info->raw[i * TX_NUM + j]);
			} else if (cmd == MMS_VSC_CMD_REFER) {
				info->reference[i * TX_NUM + j] =
					raw_data;
				dev_dbg(&info->client->dev,
					"[TSP] reference[%d][%d] = %d\n", j, i,
					info->reference[i * TX_NUM + j]);
			}
		}

	}

	snprintf(buff, sizeof(buff), "%d,%d", min_value, max_value);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
}

static ssize_t show_close_tsp_test(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	get_raw_data_all(info, MMS_VSC_CMD_EXIT);
	info->ft_flag = 0;

	return snprintf(buf, TSP_BUF_SIZE, "%u\n", 0);
}

static void set_default_result(struct mms_ts_info *info)
{
	char delim = ':';

	memset(info->cmd_result, 0x00, ARRAY_SIZE(info->cmd_result));
	memcpy(info->cmd_result, info->cmd, strlen(info->cmd));
	strncat(info->cmd_result, &delim, 1);
}

static int check_rx_tx_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[TSP_CMD_STR_LEN] = {0};
	int node;

	if (info->cmd_param[0] < 0 ||
			info->cmd_param[0] >= TX_NUM  ||
			info->cmd_param[1] < 0 ||
			info->cmd_param[1] >= RX_NUM) {
		snprintf(buff, sizeof(buff) , "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = FAIL;

		dev_info(&info->client->dev, "%s: parameter error: %u,%u\n",
				__func__, info->cmd_param[0],
				info->cmd_param[1]);
		node = -1;
		return node;
}
	node = info->cmd_param[1] * TX_NUM + info->cmd_param[0];
	dev_info(&info->client->dev, "%s: node = %d\n", __func__,
			node);
	return node;

}

static void not_support_cmd(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buff[16] = {0};

	set_default_result(info);
	sprintf(buff, "%s", "NA");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = WAITING;
	dev_info(&info->client->dev, "%s: \"%s(%d)\"\n", __func__,
				buff, strnlen(buff, sizeof(buff)));
	return;
}

#ifdef TSP_GESTURE_MODE

#define MMS_MODE_QUICK_SHOT	 			(1 << 0)
#define MMS_FLAG_BLACK_UI				(1 << 1)
#define MMS_FLAG_QUICK_APP_ACCESS		(1 << 2)
#define MMS_FLAG_DIRECT_INDICATOR		(1 << 3)


static void quick_shot_enable(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};

	dev_info(&client->dev, "%s, (%d)\n", __func__, info->cmd_param[0]);
	set_default_result(info);

	/* ################ */
	
	if (info->cmd_param[0]) {
		info->lowpower_flag = info->lowpower_flag | MMS_MODE_QUICK_SHOT;
	} else {
		info->lowpower_flag = info->lowpower_flag & ~(MMS_MODE_QUICK_SHOT);
	}
	if((info->lowpower_flag)&0xF){		
		info->lowpower_mode = 1;
	}else{
		info->lowpower_mode = 0;
	}
		
	dev_info(&client->dev, "%s, [%d,%d]\n", __func__, info->lowpower_flag, info->lowpower_mode);


	/* ################ */
	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));	

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

static void scrub_enable(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};

	dev_info(&client->dev, "%s, (%d)\n", __func__, info->cmd_param[0]);
	set_default_result(info);

	/* ################ */
	
	if (info->cmd_param[0]) {
		info->lowpower_flag = info->lowpower_flag | MMS_FLAG_BLACK_UI;
	} else {
		info->lowpower_flag = info->lowpower_flag & ~(MMS_FLAG_BLACK_UI);
	}
	if((info->lowpower_flag)&0xF){		
		info->lowpower_mode = 1;
	}else{
		info->lowpower_mode = 0;
	}
		
	dev_info(&client->dev, "%s, [%d,%d]\n", __func__, info->lowpower_flag, info->lowpower_mode);


	/* ################ */
	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));	

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

static void quick_app_access_enable(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};

	dev_info(&client->dev, "%s, (%d)\n", __func__, info->cmd_param[0]);
	set_default_result(info);

	/* ################ */
	
	if (info->cmd_param[0]) {
		info->lowpower_flag = info->lowpower_flag | MMS_FLAG_QUICK_APP_ACCESS;
	} else {
		info->lowpower_flag = info->lowpower_flag & ~(MMS_FLAG_QUICK_APP_ACCESS);
	}
	if((info->lowpower_flag)&0xF){		
		info->lowpower_mode = 1;
	}else{
		info->lowpower_mode = 0;
	}
		
	dev_info(&client->dev, "%s, [%d,%d]\n", __func__, info->lowpower_flag, info->lowpower_mode);


	/* ################ */
	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));	

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}


static void direct_indicator_enable(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};

	dev_info(&client->dev, "%s, (%d)\n", __func__, info->cmd_param[0]);
	set_default_result(info);

	/* ################ */
	
	if (info->cmd_param[0]) {
		info->lowpower_flag = info->lowpower_flag | MMS_FLAG_DIRECT_INDICATOR;
	} else {
		info->lowpower_flag = info->lowpower_flag & ~(MMS_FLAG_DIRECT_INDICATOR);
	}
	if((info->lowpower_flag)&0xF){		
		info->lowpower_mode = 1;
	}else{
		info->lowpower_mode = 0;
	}
		
	dev_info(&client->dev, "%s, [%d,%d]\n", __func__, info->lowpower_flag, info->lowpower_mode);


	/* ################ */
	snprintf(buff, sizeof(buff), "%s", "OK");
	info->cmd_state = OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));	

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);
	info->cmd_state = WAITING;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}



#define BLACK_UI_GESTURE			0x01
#define BLACK_UI_QUICKAPP_ACESS		0x02
#define BLACK_UI_TEMP1				0x04
#define BLACK_UI_TEMP2				0x08

static void set_lowpower_mode(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};

	dev_info(&client->dev, "%s\n", __func__);

	set_default_result(info);

	/*  ######### */

	
	if(info->cmd_param[0]>1 || info->cmd_param[0]<0){		
		dev_info(&client->dev, "cmd is over, %d,%d fail\n",info->cmd_param[0],info->cmd_param[1]);	
		goto error_case;
	}

	if (info->cmd_param[0]==1) {
		info->lowpower_mode = 1;
	}else {
		info->lowpower_mode = 0;
	}

	dev_info(&client->dev,"%s: on = %d,%d\n", __func__, info->cmd_param[0],info->lowpower_mode );


	/*  ######### */

	snprintf(buff, sizeof(buff) , "%s", "OK");
	set_cmd_result(info, buff,
		strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	return;

error_case :
	snprintf(buff, sizeof(buff) , "%s", "NG");
	set_cmd_result(info, buff,
		strnlen(buff, sizeof(buff)));
	info->cmd_state = FAIL;

}


#endif

#ifdef TSP_GLOVE_MODE
static void glove_mode(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	int ret = 0;
	char buff[16] = {0};

	dev_info(&client->dev, "%s\n", __func__);

	set_default_result(info);
	switch (info->cmd_param[0]) {
	case 0:
		dev_info(&client->dev, "Unset Glove Mode\n");
		mms_unset_glove_mode(info);
		info->glove_mode = 0;
		break;
	case 1:
		dev_info(&client->dev, "Set Glove Mode\n");
		mms_set_glove_mode(info);
		info->glove_mode = 1;
		break;
	default:
		dev_err(&client->dev, "Invalid argument\n");
		ret = -1;
		break;
	}

	if (ret == 0) {
		snprintf(buff, sizeof(buff) , "%s", "OK");
		info->cmd_state = OK;
	} else {
		snprintf(buff, sizeof(buff) , "%s", "NG");
		info->cmd_state = FAIL;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = WAITING;
	return;
}
#endif

#ifdef TSP_SVIEW_COVER_MODE
static void clear_cover_mode(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	int ret = 0;
	char buff[16] = {0};

	dev_info(&client->dev, "%s\n", __func__);

	set_default_result(info);

	if(info->cmd_param[0]==COVER_CLOSED)	
			info->cover_mode = 3;
	else
		info->cover_mode = 0;


	if(info->enabled){
		if(info->cover_mode==3){		
		mms_set_sview_cover_mode(info);
			dev_info(&client->dev, "Set S-View Cover Mode\n");
		}else{
			mms_unset_sview_cover_mode(info);			
			dev_info(&client->dev, "Unset S-View Cover Mode\n");
	}
		info->cover_mode_retry = 0;
	}else{
		info->cover_mode_retry = (u8)info->cmd_param[0];
		dev_info(&client->dev, "Wait, IC off, %d \n",(u8)info->cmd_param[0] );
	}
	
	if (ret == 0) {
		snprintf(buff, sizeof(buff) , "%s", "OK");
		info->cmd_state = OK;
	} else {
		snprintf(buff, sizeof(buff) , "%s", "NG");
		info->cmd_state = FAIL;
	}
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = WAITING;
	return;
}
#endif

#ifdef CONFIG_INPUT_BOOSTER
static void boost_level(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	char buff[16] = {0};
	int stage;

	dev_info(&client->dev, "%s\n", __func__);

	set_default_result(info);


	stage = 1 << info->cmd_param[0];
	if (!(info->booster->dvfs_stage & stage)) {
		snprintf(buff, sizeof(buff), "NG");
		info->cmd_state = FAIL;
		dev_err(&info->client->dev,"%s: %d is not supported(%04x != %04x).\n",__func__,
			info->cmd_param[0], stage, info->booster->dvfs_stage);

		goto boost_out;
	}

	info->booster->dvfs_boost_mode = stage;
	snprintf(buff, sizeof(buff), "OK");
	info->cmd_state = OK;

	if (info->booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		if (info->booster && info->booster->dvfs_set)
			info->booster->dvfs_set(info->booster, -1);
	}

	boost_out:

	set_cmd_result(info, buff,
			strnlen(buff, sizeof(buff)));

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = WAITING;

	return;
}
#endif

#ifdef MMS300_DOWNLOAD
static int get_fw_ver(struct i2c_client *client, u8 *buf)
{
	u8 cmd = MMS_FW_VER;
	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &cmd,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = buf,
			.len = MAX_SECTION_NUM,
		},
	};

	return (i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg)) != ARRAY_SIZE(msg));
}

static int mms_isc_write_cmd(struct mms_ts_info *info, const char *buf, int count)
{
	struct i2c_client *client = info->client;

	struct i2c_msg msg[1] = {
		{
			.addr = client->addr,
			.flags = I2C_M_NOSTART,
			.buf = (char *)buf,
			.len = count,
		},
	};

	if(i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg))!=ARRAY_SIZE(msg)){
		dev_err(&info->client->dev, "%s - i2c write failed\n",__func__);
	}

	return count;
}


static int mms_isc_read_status(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	u8 cmd[6] =  ISC_READ_STATUS;
	u8 result = 0;
	int cnt = 10;
	int ret = 0;

	struct i2c_msg msg[2] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = cmd,
			.len = 6,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = &result,
			.len = 1,
		},
	};

	do {
		if(i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg))!=ARRAY_SIZE(msg)){
			dev_err(&info->client->dev, "%s - i2c transfer failed\n",__func__);
			return -1;
		}
		if (result == 0xAD){
			ret = 0;
			break;
		}else{
			ret = -1;
			dev_err(&info->client->dev, "%s - i2c retry :%d, value:%2x\n",__func__,cnt, result);
			msleep(1);
		}
	} while (--cnt);

	if (!cnt) {
		dev_err(&info->client->dev,
			"status read fail. cnt : %d, val : 0x%x\n",
			cnt, result);
	}
	return ret;
}

static int mms_isc_read_page(struct mms_ts_info *info, int offset, u8 *data)
{
	u8 write_buf[6] =ISC_READ_PAGE;
	struct i2c_msg msg[2] = {
		{
			.addr = info->client->addr,
			.flags = 0,
			.buf = write_buf,
			.len = 6,
		}, {
			.addr = info->client->addr,
			.flags = I2C_M_RD,
			.buf = data,
			.len = ISC_PAGE_SZ,
		},
	};

	write_buf[4] = (u8)(((offset/2)>>8)&0xFF );
	write_buf[5] = (u8)(((offset/2)>>0)&0xFF );
	if(i2c_transfer(info->client->adapter, msg, ARRAY_SIZE(msg)) != ARRAY_SIZE(msg)){
		dev_err(&info->client->dev,
		"%s i2c failed.\n", __func__);
		return -1;
	}
	return 0;
}

static int mms_isc_write_page(struct mms_ts_info *info, int offset,const u8 *data, int length)
{
	u8 write_buf[134] = ISC_WRITE_PAGE;
	if( length > 128 )
		return -1;
	write_buf[4] = (u8)(((offset/2)>>8)&0xFF );
	write_buf[5] = (u8)(((offset/2)>>0)&0xFF );
	memcpy( &write_buf[6], data, length);
	if(mms_isc_write_cmd(info, write_buf, length+6 )!=length+6){
		dev_err(&info->client->dev,
		"%s i2c failed.\n", __func__);
		return -1;
	}

	mdelay(2);

	if(mms_isc_read_status(info)){
		dev_err(&info->client->dev,"%s: write failed.\n", __func__);
		//return -1;
	}
	return 0;
}


static int mms_isc_erase_mess(struct mms_ts_info *info)
{
	u8 cmd[6]=ISC_FULL_ERASE;
	if(i2c_master_send(info->client,cmd,6)!=6){
		dev_err(&info->client->dev,
		"%s: Erase failed.\n", __func__);
		return -1;
	}

	msleep(5);

	if(mms_isc_read_status(info)){
		dev_err(&info->client->dev,"%s: Erase failed.\n", __func__);
		return -1;
	}

	return 0;
}


static int mms_flash_fw(struct mms_ts_info *info, const u8 *fw_data, size_t fw_size)
{

	struct mms_bin_hdr *fw_hdr;
	struct mms_fw_img **img;
	struct i2c_client *client = info->client;
	int i;
	int retires = 1;
	int ret;
	int nRet;
	int nStartAddr;
	int nWriteLength;
	int nLast;
	int nOffset;
	int nTransferLength;
	int size;
	u8 *data;
	u8 *cpydata;

	int offset = sizeof(struct mms_bin_hdr);

	bool update_flag = false; //force update , false;

	u8 ver[MAX_SECTION_NUM];
	u8 target[MAX_SECTION_NUM];


	fw_hdr = (struct mms_bin_hdr *)fw_data;
	img = kzalloc(sizeof(*img) * fw_hdr->section_num, GFP_KERNEL);

	while (retires--) {
		if (!get_fw_ver(client, ver)){
			break;
		}else{
			mms_reset(info);
			dev_err(&client->dev, "Can't read firmware version\n");
		}
	}
	if (retires < 0) {
		dev_warn(&client->dev, "failed to obtain ver. info\n");
		memset(ver, 0xff, sizeof(ver));
	} else {
		print_hex_dump(KERN_INFO, "mms_ts fw ver : ", DUMP_PREFIX_NONE, 16, 1,
				ver, MAX_SECTION_NUM, false);
	}

	for (i = 0; i < fw_hdr->section_num; i++, offset += sizeof(struct mms_fw_img)) {
		img[i] = (struct mms_fw_img *)(fw_data + offset);
		target[i] = img[i]->version;
		dev_info(&client->dev, "section[%d] version[%x/%x]\n",
			i,ver[img[i]->type], target[i]);
		if (ver[img[i]->type] != target[i]) {
			update_flag = true;
			dev_info(&client->dev,
				"section [%d] is need to be updated. ver : 0x%x, bin : 0x%x\n",
				img[i]->type, ver[img[i]->type], target[i]);
		}
	}

	if(!update_flag){
		dev_info(&client->dev, "firmware is already updated\n");
		kfree(img);
		return 0;
	}


	data = kzalloc(sizeof(u8) * fw_hdr->binary_length, GFP_KERNEL);
	size = fw_hdr->binary_length;
	cpydata = kzalloc(ISC_PAGE_SZ, GFP_KERNEL);


	if(mms_isc_erase_mess(info)){
		dev_err(&info->client->dev,
		"%s: Erase failed.\n", __func__);
		nRet = -1;
		goto EXIT;
	}

	mms_reset(info);
	//msleep(200);

	if(mms_isc_erase_mess(info)){
		dev_err(&info->client->dev,
		"%s: Erase failed.\n", __func__);
		nRet = -1;
		goto EXIT;
	}


	if( size % 128 !=0 ){
		size += ( 128 - (size % 128) );
	}

	nStartAddr   = 0;
	nWriteLength = size;
	nLast		 = nStartAddr + nWriteLength;

	if( (nStartAddr + nWriteLength) % 8 != 0 ){
		nRet = -1;
		goto EXIT;
	}else{
		memcpy(data,fw_data + fw_hdr->binary_offset,fw_hdr->binary_length);
		dev_info(&client->dev, "firmware data copy\n");
	}

	nOffset = nStartAddr + nWriteLength - 128;
	nTransferLength = 128;

	while( nOffset >= 0 )
	{
		nRet = mms_isc_write_page(info, nOffset, &data[nOffset],  nTransferLength);
		if( nRet != 0 ){
			dev_err(&client->dev, "nRet: %d error\n", nRet);
			goto EXIT;
		}
		/* verify firmware */
		if (mms_isc_read_page(info, nOffset, cpydata)){
			dev_err(&client->dev, "mms_isc_read_page: %d error\n", 1);
			goto EXIT;
		}
		if (memcmp(&data[nOffset], cpydata, ISC_PAGE_SZ)) {
#if FLASH_VERBOSE_DEBUG
			print_hex_dump(KERN_ERR, "mms fw wr : ",
					DUMP_PREFIX_OFFSET, 16, 1,
					data, ISC_PAGE_SZ, false);
			print_hex_dump(KERN_ERR, "mms fw rd : ",
					DUMP_PREFIX_OFFSET, 16, 1,
					cpydata, ISC_PAGE_SZ, false);
#endif
			dev_err(&client->dev, "flash verify failed\n");
			ret = -1;
			goto EXIT;
		}
		nOffset -= nTransferLength;
	}
	mms_reset(info);
	if (get_fw_ver(client, ver)) {
		dev_err(&client->dev, "failed to obtain version after flash\n");
		nRet = -1;
		goto EXIT;
	} else {
		int recheck_version = 3;

		do{
			get_fw_ver(client, ver);
			if(ver[0] ==0x03){
				break;
			}else{
				dev_err(&client->dev, "recheck_version error, retry:%d \n", recheck_version);
				msleep(10);
			}
		}while(recheck_version--);


		info->fw_boot_ver = ver[0];
		info->fw_core_ver = ver[1];
		info->fw_ic_ver = ver[2];


		for (i = 0; i < fw_hdr->section_num; i++) {
			if (ver[img[i]->type] != target[i]) {
				dev_info(&client->dev,
					"version mismatch after flash. [%d] 0x%x != 0x%x\n",
					i, ver[img[i]->type], target[i]);

				nRet = -1;
				goto EXIT;
			}
		}

	}
	dev_err(&client->dev,"firmware update finish\n");
	kfree(cpydata);
	kfree(data);
	kfree(img);
	return 0;

EXIT:
	dev_err(&client->dev,"firmware update failed\n");
	kfree(cpydata);
	kfree(data);
	kfree(img);
	return nRet;

}

static int mms_fw_update_controller(struct mms_ts_info *info, const struct firmware *fw)
{
	int retires = 1;//3;
	int ret = 0;

	if (!fw) {
		dev_err(&info->client->dev, "failed to read firmware\n");
		complete_all(&info->init_done);
		return -1;
	}

	do {
#if 1   // for 345L IC
		if ((info->pdata->use_isp > 0) || (info->pdata->use_isc > 0))
			ret = mms_flash_fw_mms300(info, fw->data, fw->size);
		else
#endif
			ret = mms_flash_fw(info, fw->data, fw->size);
	} while (ret && --retires);

	if (!retires) {
		dev_err(&info->client->dev, "failed to flash firmware after retires\n");
	}
	return ret;
}

#endif

static int mms_load_fw_from_kernel(struct mms_ts_info *info, const char *fw_path)
{
	int retval;
	const struct firmware *fw_entry = NULL;

	if (!fw_path) {
		dev_err(&info->client->dev, "%s: Firmware name is not defined\n",
				__func__);
		return -EINVAL;
	}

	dev_info(&info->client->dev, "%s: Load firmware : %s\n", __func__, fw_path);

	retval = request_firmware(&fw_entry, fw_path, &info->client->dev);

	if (retval) {
		dev_err(&info->client->dev, "%s: Firmware image %s not available\n",
				__func__, fw_path);
		goto done;
	}

	retval = mms_fw_update_controller(info, fw_entry);
	if (retval < 0)
		dev_err(&info->client->dev, "%s: failed update firmware\n", __func__);
done:
	if (fw_entry)
		release_firmware(fw_entry);

	return retval;
}

static int mms_load_fw_from_ums(struct mms_ts_info *info)
{
	struct file *fp;
	mm_segment_t old_fs;
	int nread;
	int error = 0;
	struct firmware fw;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(TSP_FW_FILENAME, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
		dev_err(&info->client->dev,
				"%s: failed to open %s.\n", __func__, TSP_FW_FILENAME);
		error = -ENOENT;
		goto open_err;
	}

	fw.size = fp->f_path.dentry->d_inode->i_size;
	if (fw.size > 0) {
		unsigned char *fw_data;
		fw_data = kzalloc(fw.size, GFP_KERNEL);
		nread = vfs_read(fp, (char __user *)fw_data,
				fw.size, &fp->f_pos);

		dev_info(&info->client->dev,
				"%s: start, file path %s, size %u Bytes\n", __func__,
				TSP_FW_FILENAME, fw.size);

		if (nread != fw.size) {
			dev_err(&info->client->dev,
					"%s: failed to read firmware file, nread %u Bytes\n",
					__func__,
					nread);
			error = -EIO;
		} else {
			/* UMS case */
			fw.data = fw_data;
			error = mms_fw_update_controller(info, &fw);
		}
		if (error < 0)
			dev_err(&info->client->dev, "%s: failed update firmware\n",
					__func__);

		kfree(fw_data);
	}

	filp_close(fp, current->files);

open_err:
	set_fs(old_fs);
	return error;
}

static void fw_update(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	int ret = 0;
	int fw_bin_ver = 0;
	int fw_core_ver = 0;
	int retries = 1;//4;
	char result[16] = {0};

	fw_bin_ver = FW_VERSION;
	fw_core_ver = CORE_VERSION;


	set_default_result(info);
	dev_info(&client->dev,
		"fw_ic_ver = 0x%02x, fw_bin_ver = 0x%02x\n",
		info->fw_ic_ver, fw_bin_ver);

	if (info->pdata->gpio_seperated < 1) {
		snprintf(result, sizeof(result) , "%s", "NG");
		set_cmd_result(info, result,
			strnlen(result, sizeof(result)));
		info->cmd_state = FAIL;

		return;
	}

	disable_irq_nosync(info->irq);

	switch (info->cmd_param[0]) {
	case BUILT_IN:
		dev_info(&client->dev, "built in fw update\n");
		while (retries--) {
			ret = mms_load_fw_from_kernel(info, info->fw_path);
			if (ret == 0) {
				pr_err("[TSP] fw update success");
				break;
			} else {
				pr_err("[TSP] fw update fail[%d], retry = %d",
					ret, retries);
			}
		}
		break;
	case UMS:
		dev_info(&client->dev, "UMS fw update!!\n");
		while (retries--) {
			ret = mms_load_fw_from_ums(info);
			if (ret == 0) {
				pr_err("[TSP] fw update success");
				break;
			} else {
				pr_err("[TSP] fw update fail[%d], retry = %d",
					ret, retries);
			}
		}
		break;
	default:
		dev_err(&client->dev, "invalid fw update type\n");
		ret = -1;
		break;
	}

	dev_info(&client->dev, "boot version : %d\n",
		info->fw_boot_ver);
	dev_info(&client->dev, "core version : 0x%02x\n",
		info->fw_core_ver);
	dev_info(&client->dev, "config version : 0x%02x\n",
		info->fw_ic_ver);

	enable_irq(info->irq);

	if (ret == 0) {
		snprintf(result, sizeof(result) , "%s", "OK");
		set_cmd_result(info, result,
			strnlen(result, sizeof(result)));
		info->cmd_state = OK;
	} else {
		snprintf(result, sizeof(result) , "%s", "NG");
		set_cmd_result(info, result,
			strnlen(result, sizeof(result)));
		info->cmd_state = FAIL;
	}
}

static void get_fw_ver_bin(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "ME%02x%02x%02x",
		info->panel, CORE_VERSION, FW_VERSION);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_fw_ver_ic(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	if (info->enabled){
		if(get_fw_version(info)<0)
			pr_err("[TSP] : get_fw_version");
	}

	snprintf(buff, sizeof(buff), "ME%02x%02x%02x",
		info->panel, info->fw_core_ver, info->fw_ic_ver);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_config_ver(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[20] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "A500Q_Me_0801_XX");

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_threshold(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	int threshold;

	set_default_result(info);

	threshold = i2c_smbus_read_byte_data(info->client, 0x05);
	if (threshold < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = FAIL;
		return;
	}
	snprintf(buff, sizeof(buff), "%d", threshold);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}
static int is_melfas_vdd_on(struct mms_ts_info *info)
{
	int ret;

	ret = gpio_get_value(info->pdata->vdd_en);
	pr_info("[TSP] %s = %d\n", __func__, ret);

	if (ret)
		return 1;
	return 0;
}
static void module_off_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[3] = {0};

	mutex_lock(&info->lock);
	if (info->enabled) {
		disable_irq_nosync(info->irq);
		info->enabled = false;
		touch_is_pressed = 0;
	}
	mutex_unlock(&info->lock);

	info->power(info, 0);

	if (is_melfas_vdd_on(info) == 0)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = OK;
	else
		info->cmd_state = FAIL;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);
}

static void module_on_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[3] = {0};

	mms_reset(info);

	mutex_lock(&info->lock);
	if (!info->enabled) {
		enable_irq(info->irq);
		info->enabled = true;
	}
	mutex_unlock(&info->lock);

	if (is_melfas_vdd_on(info) == 1)
		snprintf(buff, sizeof(buff), "%s", "OK");
	else
		snprintf(buff, sizeof(buff), "%s", "NG");

	set_default_result(info);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	if (strncmp(buff, "OK", 2) == 0)
		info->cmd_state = OK;
	else
		info->cmd_state = FAIL;

	dev_info(&info->client->dev, "%s: %s\n", __func__, buff);

}
/*
static void module_off_slave(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	not_support_cmd(info);
}

static void module_on_slave(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	not_support_cmd(info);
}
*/
static void get_module_vendor(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buff[16] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "%s,00", tostring(OK));
	info->cmd_state = OK;
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

		return;
}
static void get_chip_vendor(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	snprintf(buff, sizeof(buff), "%s", "MELFAS");
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_chip_name(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};

	set_default_result(info);

	if (info->pdata->gpio_seperated > 0)
		snprintf(buff, sizeof(buff), "%s", MELFAS_CHIP_NAME_345L);
	else
		snprintf(buff, sizeof(buff), "%s", MELFAS_CHIP_NAME_345);

	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;
	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));
}

static void get_reference(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->reference[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));

	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__,
			buff, strnlen(buff, sizeof(buff)));

}

static void get_cm_abs(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->raw[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_cm_delta(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->inspection[node];
	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_intensity(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	unsigned int val;
	int node;

	set_default_result(info);
	node = check_rx_tx_num(info);

	if (node < 0)
		return;

	val = info->intensity[node];

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

#if DEBUG_PRINT2
#define BUF_SIZE PAGE_SIZE

static char get_debug_data[BUF_SIZE];

static int get_intensity1(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int r, t;
	int ret = 0;
	u8 sz = 0;
	u8 buf[100]={0, };
	u8 reg[4]={ 0, };
	u8 tx_num;
	u8 rx_num;
	u8 key_num;
	s16 cmdata;
	char data[6];
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = reg,
		},{
			.addr = client->addr,
			.flags = I2C_M_RD,
		},
	};
	tx_num = i2c_smbus_read_byte_data(client, 0x0B);
	rx_num = i2c_smbus_read_byte_data(client, 0x0C);
	key_num = i2c_smbus_read_byte_data(client, 0x0D);
	disable_irq_nosync(info->irq);
	memset(get_debug_data,0,BUF_SIZE);
	sprintf(get_debug_data,"%s", "start intensity\n");
	for(r = 0 ; r < rx_num ; r++)
	{
		printk("[%2d]",r);
		sprintf(data,"[%2d]",r);
		strcat(get_debug_data,data);
		memset(data,0,5);

		reg[0] = 0xA0;
		reg[1] = 0x70;
		reg[2] = 0xFF;
		reg[3] = r;
		msg[0].len = 4;

		msleep(1);
		if(i2c_transfer(client->adapter, &msg[0],1)!=1){
			dev_err(&client->dev, "intensity i2c transfer failed\n");
			ret = -1;
			enable_irq(info->irq);
			return ret;
		}

		sz = i2c_smbus_read_byte_data(client, 0xAE);

		reg[0] =0xAF;
		msg[0].len = 1;
		msg[1].len = sz;
		msg[1].buf = buf;


		if(i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg))!=ARRAY_SIZE(msg)){
			ret = -1;
			enable_irq(info->irq);
			return ret;
		}

		sz >>=1;
		for(t = 0 ; t <sz ; t++){
			cmdata = (s16)(buf[2*t] | (buf[2*t+1] << 8));
			printk("%3d",cmdata);
			sprintf(data,"%3d",cmdata);
			strcat(get_debug_data,data);
			memset(data,0,5);
		}
		printk("\n");
		sprintf(data,"\n");
		strcat(get_debug_data,data);
		memset(data,0,5);

	}

	if (key_num)
	{
		printk("---key intensity---\n");
		strcat(get_debug_data,"key intensity\n");
		memset(data,0,5);

		reg[0] = 0xA0;
		reg[1] = 0x71;
		reg[2] = 0xFF;
		reg[3] = 0x00;
		msg[0].len = 4;

		if(i2c_transfer(client->adapter, &msg[0],1)!=1){
			dev_err(&client->dev, "Cm delta i2c transfer failed\n");
			ret = -1;
			enable_irq(info->irq);
			return ret;
		}

		while (gpio_get_value(info->pdata->gpio_int)){
		}

		sz = i2c_smbus_read_byte_data(info->client, 0xAE);

		reg[0] =0xAF;
		msg[0].len = 1;
		msg[1].len = sz;
		msg[1].buf = buf;
		if(i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg))!=ARRAY_SIZE(msg)){
			ret = -1;
			enable_irq(info->irq);
			return ret;
		}
		for(t = 0; t< key_num; t++){
			cmdata = (s16)(buf[2*t] | (buf[2*t+1] << 8));
			printk("%5d",cmdata);
			sprintf(data,"%5d",cmdata);
			strcat(get_debug_data,data);
			memset(data,0,5);
		}
		printk("\n");
		sprintf(data,"\n");
		strcat(get_debug_data,data);
		memset(data,0,5);

	}
	enable_irq(info->irq);

	return 0;
}

static ssize_t mms_intensity(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	int ret;
	dev_info(&info->client->dev, "Intensity Test\n");
	if(get_intensity1(info)!=0){
		dev_err(&info->client->dev, "Intensity Test failed\n");
		return -1;
	}
	ret = snprintf(buf,BUF_SIZE,"%s\n",get_debug_data);
	return ret;
}

static DEVICE_ATTR(intensity, 0664, mms_intensity, NULL);

static struct attribute *mms_attrs[] = {
	&dev_attr_intensity.attr,
	NULL,
};

static const struct attribute_group mms_attr_group = {
	.attrs = mms_attrs,
};
#endif

static int using = 0;
static struct mms_ts_info *ts = NULL;

void dump_tsp_log(void){
#if DEBUG_PRINT2
	if(using ==0){
		using = 1;
		dev_info(&ts->client->dev, "demp_tsp_log, Intensity value\n");
		if(get_intensity1(ts)!=0){
			dev_err(&ts->client->dev, "Intensity Test failed\n");
		}
		using = 0;
	}else{
		dev_err(&ts->client->dev, "Intensity Testing. wait.\n");
	}
#else
	dev_info(&ts->client->dev, "demp_tsp_log, not debug2 mode\n");
#endif
}
EXPORT_SYMBOL(dump_tsp_log);

static void get_x_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	u32 val;
	u8 r_buf[2];
	int ret;

	set_default_result(info);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		ADDR_CH_NUM, 2, r_buf);
	val = r_buf[0];
	if (ret < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = FAIL;

		dev_info(&info->client->dev,
			"%s: fail to read num of x (%d).\n",
			__func__, val);
		return;
	}

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void get_y_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	char buff[16] = {0};
	u32 val;
	u8 r_buf[2];
	int ret;

	set_default_result(info);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		ADDR_CH_NUM, 2, r_buf);
	val = r_buf[1];
	if (ret < 0) {
		snprintf(buff, sizeof(buff), "%s", "NG");
		set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
		info->cmd_state = FAIL;

		dev_info(&info->client->dev,
			"%s: fail to read num of x (%d).\n",
			__func__, val);
		return;
	}

	snprintf(buff, sizeof(buff), "%u", val);
	set_cmd_result(info, buff, strnlen(buff, sizeof(buff)));
	info->cmd_state = OK;

	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__, buff,
			strnlen(buff, sizeof(buff)));
}

static void run_reference_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	if(!get_raw_data(info, MMS_VSC_CMD_REFER))
		info->cmd_state = OK;
	else
		info->cmd_state = NG;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static void run_cm_abs_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	if(!get_raw_data(info, MMS_VSC_CMD_CM_ABS))
		info->cmd_state = OK;
	else
		info->cmd_state = NG;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static void run_cm_delta_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	if(!get_raw_data(info, MMS_VSC_CMD_CM_DELTA))
		info->cmd_state = OK;
	else
		info->cmd_state = NG;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

void run_intensity_read(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;

	set_default_result(info);
	get_intensity_data(info);
	info->cmd_state = OK;

/*	dev_info(&info->client->dev, "%s: %s(%d)\n", __func__); */
}

static ssize_t store_cmd(struct device *dev, struct device_attribute
				  *devattr, const char *buf, size_t count)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;

	char *cur, *start, *end;
	char buff[TSP_CMD_STR_LEN] = {0};
	int len, i;
	struct tsp_cmd *tsp_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;
	int ret;

	if (info->cmd_is_running == true) {
		dev_err(&info->client->dev, "tsp_cmd: other cmd is running.\n");
		goto err_out;
	}


	/* check lock  */
	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = true;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = RUNNING;

	for (i = 0; i < ARRAY_SIZE(info->cmd_param); i++)
		info->cmd_param[i] = 0;

	len = (int)count;
	if (*(buf + len - 1) == '\n')
		len--;
	memset(info->cmd, 0x00, ARRAY_SIZE(info->cmd));
	memcpy(info->cmd, buf, len);

	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(buff, buf, cur - buf);
	else
		memcpy(buff, buf, len);

	/* find command */
	list_for_each_entry(tsp_cmd_ptr, &info->cmd_list_head, list) {
		if (!strcmp(buff, tsp_cmd_ptr->cmd_name)) {
			cmd_found = true;
			break;
		}
	}

	/* set not_support_cmd */
	if (!cmd_found) {
		list_for_each_entry(tsp_cmd_ptr, &info->cmd_list_head, list) {
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
				ret = kstrtoint(buff, 10,\
						info->cmd_param + param_cnt);
				start = cur + 1;
				memset(buff, 0x00, ARRAY_SIZE(buff));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}

	dev_info(&client->dev, "cmd = %s\n", tsp_cmd_ptr->cmd_name);
	for (i = 0; i < param_cnt; i++)
		dev_info(&client->dev, "cmd param %d= %d\n", i,
							info->cmd_param[i]);

	tsp_cmd_ptr->cmd_func(info);


err_out:
	return count;
}

static ssize_t show_cmd_status(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	char buff[16] = {0};

	dev_info(&info->client->dev, "tsp cmd: status:%d\n",
			info->cmd_state);

	if (info->cmd_state == WAITING)
		snprintf(buff, sizeof(buff), "WAITING");

	else if (info->cmd_state == RUNNING)
		snprintf(buff, sizeof(buff), "RUNNING");

	else if (info->cmd_state == OK)
		snprintf(buff, sizeof(buff), "OK");

	else if (info->cmd_state == FAIL)
		snprintf(buff, sizeof(buff), "FAIL");

	else if (info->cmd_state == NOT_APPLICABLE)
		snprintf(buff, sizeof(buff), "NOT_APPLICABLE");

	else if (info->cmd_state == NG)
		snprintf(buff, sizeof(buff), "NG");

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buff);
}

static ssize_t show_cmd_result(struct device *dev, struct device_attribute
				    *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	dev_info(&info->client->dev, "tsp cmd: result: %s\n", info->cmd_result);

	mutex_lock(&info->cmd_lock);
	info->cmd_is_running = false;
	mutex_unlock(&info->cmd_lock);

	info->cmd_state = WAITING;

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", info->cmd_result);
}


#ifdef TSP_GESTURE_MODE
static ssize_t mms_scrub_position(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	char buff[32] = { 0 };

	if (!info) {
		printk(KERN_ERR "%s: No platform data found\n",__func__);
		return -1;
	}

	if (!info->input_dev) {
		printk(KERN_ERR "%s: No input_dev data found\n",__func__);
		return -1;
	}

	dev_info(&info->client->dev, "%s: %d %d %d\n",
				__func__, info->scrub_id, info->scrub_x, info->scrub_y);
	snprintf(buff, sizeof(buff), "%d %d %d", info->scrub_id, info->scrub_x, info->scrub_y);

	info->scrub_id = 0;
	info->scrub_x = 0;
	info->scrub_y = 0;

	return snprintf(buf, TSP_BUF_SIZE, "%s\n", buff);

}

#endif


#ifdef TOUCHKEY
static ssize_t touch_led_control(struct device *dev,
		 struct device_attribute *attr, const char *buf,
		 size_t count)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	u32 data;
	int ret;

	ret = sscanf(buf, "%d", &data);
	//ret = kstrtol(buf,12, &data);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(data == 0 || data == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
			__func__, data);
		return count;
	}

	if (data == 1) {
		dev_notice(&client->dev, "led on\n");
		info->keyled(info, 1);
		info->led_cmd = true;
	} else {
		dev_notice(&client->dev, "led off\n");
		info->keyled(info, 0);
		info->led_cmd = false;
	}
	return count;
}

static ssize_t touchkey_threshold_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	u8 r_buf[2];
	int i;

	if (!info->enabled)
		return sprintf(buf, "0 0\n");

	for (i = 0; i < 2; i++) {
		i2c_smbus_write_byte_data(info->client, 0x20, i+1);
		r_buf[i] = i2c_smbus_read_byte_data(info->client, 0x20);
	}

	//return sprintf(buf, "%d %d\n", r_buf[0], r_buf[1]);
	return sprintf(buf,"%d\n", r_buf[0]);
}

static void get_intensity_key(struct mms_ts_info *info)
{
	u8 w_buf[4];
	u8 r_buf;
	u8 read_buffer[10] = {0};
	int i;
	int ret;
	u16 raw_data;
	int data;

	if (!info->enabled) {
		info->menu_s = 0;
		info->back_s = 0;
		return;
	}

	disable_irq(info->irq);

	w_buf[0] = ADDR_UNIV_CMD;
	w_buf[1] = CMD_GET_INTEN_KEY;
	w_buf[2] = 0xFF;
	w_buf[3] = 0;

	ret = i2c_smbus_write_i2c_block_data(info->client,
		w_buf[0], 3, &w_buf[1]);
	if (ret < 0)
		goto err_i2c;
	usleep_range(1, 5);

	ret = i2c_smbus_read_i2c_block_data(info->client,
		CMD_RESULT_SZ, 1, &r_buf);
	if (ret < 0)
		goto err_i2c;

	ret = get_data(info, CMD_RESULT, r_buf, read_buffer);
	if (ret < 0)
		goto err_i2c;

	for (i = 0; i < r_buf/2; i++) {
		raw_data = read_buffer[2*i] | (read_buffer[2*i+1] << 8);

		if (raw_data < 32767)
			data = (int)raw_data;
		else
			data = -(65536 - raw_data);

		if (i == 0)
			info->menu_s = data;
		if (i == 1)
			info->back_s = data;
	}

	enable_irq(info->irq);

	return;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, MMS_VSC_CMD_INTENSITY);

	info->menu_s = 0;
	info->back_s = 0;

	release_all_fingers(info);
	mms_reset(info);

	enable_irq(info->irq);
}


static ssize_t touchkey_recent_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	get_intensity_key(info);

	return sprintf(buf, "%d\n", info->menu_s);
}

static ssize_t touchkey_back_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);

	get_intensity_key(info);

	return sprintf(buf, "%d\n", info->back_s);
}

static ssize_t touchkey_fw_read(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	#if 0
	struct mms_ts_info *info = dev_get_drvdata(dev);

	if(info->panel == ILJIN) {
		return snprintf(buf, 10, "ME%02x%02x%02x",
			info->panel, CORE_VERSION_IJ, FW_VERSION_IJ);
	} else { /* EELY */
		return snprintf(buf, 10, "ME%02x%02x%02x",
			info->panel, CORE_VERSION_EL, FW_VERSION_EL);
	}
	#endif
	return snprintf(buf, 4, "%s", "N");
}

#endif

#ifdef ESD_DEBUG

static bool intensity_log_flag;

static u32 get_raw_data_one(struct mms_ts_info *info, u16 rx_idx, u16 tx_idx,
			    u8 cmd)
{
	u8 w_buf[6];
	u8 read_buffer[2];
	int ret;
	u32 raw_data;

	w_buf[0] = MMS_VSC_CMD;	/* vendor specific command id */
	w_buf[1] = MMS_VSC_MODE;	/* mode of vendor */
	w_buf[2] = 0;		/* tx line */
	w_buf[3] = 0;		/* rx line */
	w_buf[4] = 0;		/* reserved */
	w_buf[5] = 0;		/* sub command */

	if (cmd != MMS_VSC_CMD_INTENSITY && cmd != MMS_VSC_CMD_RAW &&
	    cmd != MMS_VSC_CMD_REFER) {
		dev_err(&info->client->dev, "%s: not profer command(cmd=%d)\n",
			__func__, cmd);
		return RAW_FAIL;
	}

	w_buf[2] = tx_idx;	/* tx */
	w_buf[3] = rx_idx;	/* rx */
	w_buf[5] = cmd;		/* sub command */

	ret = i2c_smbus_write_i2c_block_data(info->client, w_buf[0], 5,
					     &w_buf[1]);
	if (ret < 0)
		goto err_i2c;

	ret = i2c_smbus_read_i2c_block_data(info->client, 0xBF, 2, read_buffer);
	if (ret < 0)
		goto err_i2c;

	raw_data = ((u16) read_buffer[1] << 8) | read_buffer[0];
	if (cmd == MMS_VSC_CMD_REFER)
		raw_data = raw_data >> 4;

	return raw_data;

err_i2c:
	dev_err(&info->client->dev, "%s: fail to i2c (cmd=%d)\n",
		__func__, cmd);
	return RAW_FAIL;
}

static ssize_t show_intensity_logging_on(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	struct file *fp;
	char log_data[160] = { 0, };
	char buff[16] = { 0, };
	mm_segment_t old_fs;
	long nwrite;
	u32 val;
	int i, c;
	u32 y;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

#define MELFAS_DEBUG_LOG_PATH "/sdcard/melfas_log"

	dev_info(&client->dev, "%s: start.\n", __func__);
	fp = filp_open(MELFAS_DEBUG_LOG_PATH, O_RDWR | O_CREAT,
		       S_IRWXU | S_IRWXG | S_IRWXO);
	if (IS_ERR(fp)) {
		dev_err(&client->dev, "%s: fail to open log file\n", __func__);
		goto open_err;
	}

	intensity_log_flag = 1;
	do {
		for (y = 0; y < 3; y++) {
			/* for tx chanel 0~2 */
			memset(log_data, 0x00, 160);

			snprintf(buff, 16, "%1u: ", y);
			strncat(log_data, buff, strnlen(buff, 16));

			for (i = 0; i < RX_NUM; i++) {
				val = get_raw_data_one(info, i, y,
						       MMS_VSC_CMD_INTENSITY);
				snprintf(buff, 16, "%5u, ", val);
				strncat(log_data, buff, strnlen(buff, 16));
			}
			memset(buff, '\n', 2);
			c = (y == 2) ? 2 : 1;
			strncat(log_data, buff, c);
			nwrite = vfs_write(fp, (const char __user *)log_data,
					   strnlen(log_data, 160), &fp->f_pos);
		}
		usleep_range(3000, 5000);
	} while (intensity_log_flag);

	filp_close(fp, current->files);
	set_fs(old_fs);

	return 0;

open_err:
	set_fs(old_fs);
	return RAW_FAIL;
}

static ssize_t show_intensity_logging_off(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	intensity_log_flag = 0;
	usleep_range(10000, 12000);
	get_raw_data_all(info, MMS_VSC_CMD_EXIT);
	return 0;
}

#endif

static DEVICE_ATTR(close_tsp_test, S_IRUGO, show_close_tsp_test, NULL);
static DEVICE_ATTR(cmd, S_IWUSR | S_IWGRP, NULL, store_cmd);
static DEVICE_ATTR(cmd_status, S_IRUGO, show_cmd_status, NULL);
static DEVICE_ATTR(cmd_result, S_IRUGO, show_cmd_result, NULL);

#ifdef TSP_GESTURE_MODE
static DEVICE_ATTR(scrub_pos, S_IRUGO, mms_scrub_position, NULL);
#endif


#ifdef TOUCHKEY
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold_show, NULL);
static DEVICE_ATTR(touchkey_recent, S_IRUGO, touchkey_recent_show, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO, touchkey_back_show, NULL);
static DEVICE_ATTR(touchkey_firm_version_panel, S_IRUGO,
                                touchkey_fw_read, NULL);
static DEVICE_ATTR(touchkey_firm_version_phone, S_IRUGO,
                                touchkey_fw_read, NULL);

#endif
#ifdef ESD_DEBUG
static DEVICE_ATTR(intensity_logging_on, S_IRUGO, show_intensity_logging_on,
			NULL);
static DEVICE_ATTR(intensity_logging_off, S_IRUGO, show_intensity_logging_off,
			NULL);
#endif

static struct attribute *sec_touch_facotry_attributes[] = {
	&dev_attr_close_tsp_test.attr,
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
#ifdef TSP_GESTURE_MODE
	&dev_attr_scrub_pos.attr,
#endif
#ifdef ESD_DEBUG
	&dev_attr_intensity_logging_on.attr,
	&dev_attr_intensity_logging_off.attr,
#endif
	NULL,
};

static struct attribute_group sec_touch_factory_attr_group = {
	.attrs = sec_touch_facotry_attributes,
};
#endif /* SEC_TSP_FACTORY_TEST */

#ifdef TOUCHKEY
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
			touch_led_control);

static struct attribute *sec_touchkeyled_attributes[] = {
	&dev_attr_brightness.attr,
	NULL,
};

static struct attribute *sec_touchkey_attributes[] = {
#ifdef SEC_TSP_FACTORY_TEST
	&dev_attr_touchkey_threshold.attr,
	&dev_attr_touchkey_recent.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_firm_version_panel.attr,
	&dev_attr_touchkey_firm_version_phone.attr,
#endif
	NULL,
};

static struct attribute_group sec_touchkeyled_attr_group = {
	.attrs = sec_touchkeyled_attributes,
};

static struct attribute_group sec_touchkey_attr_group = {
	.attrs = sec_touchkey_attributes,
};
#endif

#if 1

//MMS300L : M3HL
#define CHIP_CHECK_CODE_0	0x4D	//M
#define CHIP_CHECK_CODE_1	0x33	//3
#define CHIP_CHECK_CODE_2	0x48	//H
#define CHIP_CHECK_CODE_3	0x4C	//L

enum {
	ISP_MODE_FLASH_ERASE	= 0x59F3,
	ISP_MODE_FLASH_WRITE	= 0x62CD,
	ISP_MODE_FLASH_READ	= 0x6AC9,
};

/* each address addresses 4-byte words */
#define ISP_MAX_FW_SIZE		(0x1F00 * 7)
#define ISP_IC_INFO_ADDR	0x1F00

#define ISP_CAL_INFO_ADDR	7936
#define ISP_CAL_DATA_SIZE	256

static void hw_reboot(struct mms_ts_info *info, bool bootloader)
{
	gpio_direction_output(info->pdata->vdd_en, 0);
	gpio_direction_output(info->pdata->gpio_sda, bootloader ? 0 : 1);
	gpio_direction_output(info->pdata->gpio_scl, bootloader ? 0 : 1);
	gpio_direction_output(info->pdata->gpio_int, 0);
	msleep(30);
	gpio_set_value(info->pdata->vdd_en, 1);
	msleep(30);

	if (bootloader) {
		gpio_set_value(info->pdata->gpio_scl, 0);
		gpio_set_value(info->pdata->gpio_sda, 1);
	} else {
		gpio_set_value(info->pdata->gpio_int, 1);
		gpio_direction_input(info->pdata->gpio_int);
		gpio_direction_input(info->pdata->gpio_scl);
		gpio_direction_input(info->pdata->gpio_sda);
	}
	msleep(40);
}

static void isp_toggle_clk(struct mms_ts_info *info, int start_lvl, int end_lvl,
			   int hold_us, int hold_us2)
{
	//gpio_set_value(info->pdata->gpio_scl, start_lvl);	
	gpio_direction_output(info->pdata->gpio_scl, start_lvl);
	udelay(hold_us);
	//gpio_set_value(info->pdata->gpio_scl, end_lvl);
	gpio_direction_output(info->pdata->gpio_scl, end_lvl);
	udelay(hold_us2);
}

/* 1 <= cnt <= 32 bits to write */
static void isp_send_bits(struct mms_ts_info *info, u32 data, int cnt)
{
	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 0);

	/* clock out the bits, msb first */
	udelay(2);
	while (cnt--) {
		gpio_set_value(info->pdata->gpio_sda, (data >> cnt) & 1);
		udelay(2);
		isp_toggle_clk(info, 1, 0, 2, 2);
	}
}

/* 1 <= cnt <= 32 bits to read */
static u32 isp_recv_bits(struct mms_ts_info *info, int cnt)
{
	u32 data = 0;

	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_set_value(info->pdata->gpio_sda, 0);
	gpio_direction_input(info->pdata->gpio_sda);

	/* clock in the bits, msb first */
	while (cnt--) {
		udelay(3);
		isp_toggle_clk(info, 0, 1, 1, 1);
		data = (data << 1) | (!!gpio_get_value(info->pdata->gpio_sda));
	}

	gpio_direction_output(info->pdata->gpio_sda, 0);
	return data;
}

static void isp_enter_mode(struct mms_ts_info *info, u32 mode)
{
	int cnt;
	unsigned long flags;

	local_irq_save(flags);
	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 1);
	udelay(2);

	mode &= 0xffff;
	for (cnt = 15; cnt >= 0; cnt--) {
		gpio_set_value(info->pdata->gpio_int, (mode >> cnt) & 1);
		udelay(2);
		isp_toggle_clk(info, 1, 0, 2, 2);
	}

	gpio_set_value(info->pdata->gpio_int, 0);
	local_irq_restore(flags);
}

static void isp_exit_mode(struct mms_ts_info *info)
{
	int i;
	unsigned long flags;

	local_irq_save(flags);
	gpio_direction_output(info->pdata->gpio_int, 0);
	udelay(3);

	for (i = 0; i < 10; i++)
		isp_toggle_clk(info, 1, 0, 2, 2);
	local_irq_restore(flags);
}

static void flash_set_address(struct mms_ts_info *info, u16 addr)
{
	/* Only 13 bits of addr are valid.
	 * The addr is in bits 13:1 of cmd */
	isp_send_bits(info, (u32)(addr & 0x1fff) << 1, 18);
}

static void flash_erase(struct mms_ts_info *info)
{
	isp_enter_mode(info, ISP_MODE_FLASH_ERASE);

	gpio_direction_output(info->pdata->gpio_int, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 1);

	/* 4 clock cycles with different timings for the erase to
	 * get processed, clk is already 0 from above */
	udelay(5);
	isp_toggle_clk(info, 1, 0, 2, 2);
	udelay(5);
	isp_toggle_clk(info, 1, 0, 2, 2);
	usleep_range(25000, 35000);
	isp_toggle_clk(info, 1, 0, 2, 2);
	usleep_range(150, 200);
	isp_toggle_clk(info, 1, 0, 2, 2);

	gpio_set_value(info->pdata->gpio_sda, 0);

	isp_exit_mode(info);
}

static u32 flash_readl(struct mms_ts_info *info, u16 addr)
{
	int i;
	u32 val;
	unsigned long flags;

	local_irq_save(flags);
	isp_enter_mode(info, ISP_MODE_FLASH_READ);
	flash_set_address(info, addr);

	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_sda, 0);
	udelay(40);

	/* data load cycle */
	for (i = 0; i < 6; i++)
		isp_toggle_clk(info, 1, 0, 2, 2);

	val = isp_recv_bits(info, 32);
	isp_exit_mode(info);
	local_irq_restore(flags);

	return val;
}

static void flash_writel(struct mms_ts_info *info, u16 addr, u32 val)
{
	unsigned long flags;

	local_irq_save(flags);
	isp_enter_mode(info, ISP_MODE_FLASH_WRITE);
	flash_set_address(info, addr);
	isp_send_bits(info, val, 32);

	gpio_direction_output(info->pdata->gpio_sda, 1);
	/* 6 clock cycles with different timings for the data to get written
	 * into flash */
	isp_toggle_clk(info, 0, 1, 5, 5);
	isp_toggle_clk(info, 0, 1, 5, 5);
	isp_toggle_clk(info, 0, 1, 10, 10);
	isp_toggle_clk(info, 0, 1, 20, 5);
	isp_toggle_clk(info, 0, 1, 5, 5);
	isp_toggle_clk(info, 0, 1, 5, 5);

	isp_toggle_clk(info, 1, 0, 1, 1);

	gpio_direction_output(info->pdata->gpio_sda, 0);
	isp_exit_mode(info);
	local_irq_restore(flags);
	usleep_range(300, 400);
}

static int fw_write_image(struct mms_ts_info *info, const u8 *data, size_t len)
{
	struct i2c_client *client = info->client;
	u16 addr = 0;
	u32 val = 0;

	for (addr = 0; addr < (len / 4); addr++, data += 4) {

		u32 verify_val;
		int retries = 3;

		val = get_unaligned_le32(data);

		if (addr == 0)
			dev_info(&client->dev, "%s: [%X]:[%08X]\n", __func__, addr, val);

		while (retries--) {
			flash_writel(info, addr, val);
			verify_val = flash_readl(info, addr);
			if (val == verify_val)
				break;
			dev_err(&client->dev,
				"mismatch @ addr 0x%x: 0x%x != 0x%x\n",
				addr, verify_val, val);
			hw_reboot(info, true);
			continue;
		}
		if (retries < 0)
			return -ENXIO;
	}

	dev_info(&client->dev, "%s: [%X]:[%08X]\n", __func__, addr, val);

	return 0;
}

static int fw_download_isp(struct mms_ts_info *info, const u8 *data, size_t len)
{
	struct i2c_client *client = info->client;
	u32 val;
	int ret = 0;
	int i;
	int addr = ISP_CAL_INFO_ADDR; 
	u32 *buf = kzalloc(ISP_CAL_DATA_SIZE * 4, GFP_KERNEL);
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

//	int binOffset = 0;
	int coreStart = 1024; 

	dev_err(&client->dev, "%s: fw image size %d(0x%X)\n", __func__, len, len);

	dev_info(&client->dev, "%s: %X, %X, %X, %X\n", __func__, data[0], data[1], data[2], data[3]);

	if (len % 4) {
		dev_err(&client->dev,
			"fw image size (%d) must be a multiple of 4 bytes\n",
			len);
		ret = -EINVAL;
		goto out;
	} else if (len > ISP_MAX_FW_SIZE) {
		dev_err(&client->dev,
			"fw image is too big, %d > %d\n", len, ISP_MAX_FW_SIZE);
		ret = -EINVAL;
		goto out;
	}

	//Binary Check
//	binOffset = data[0x10] + (data[0x11] << 8) + (data[0x12] << 16) + (data[0x13] << 24);
//	dev_info(&client->dev, "ISP : F/W Offset 0x%X\n", binOffset);
//	coreStart = binOffset + 1024;
	dev_info(&client->dev, "ISP : F/W Start 0x%X\n", coreStart);
#if 0
	if(((data[coreStart] == CHIP_CHECK_CODE_0) && (data[coreStart + 1] == CHIP_CHECK_CODE_1) && (data[coreStart + 2] == CHIP_CHECK_CODE_2) && (data[coreStart + 3] == CHIP_CHECK_CODE_3)) != 1) {
		dev_err(&client->dev,
			"ISP : F/W is not for MMS300L (0x%02X 0x%02X 0x%02X 0x%02X) : 0x%02X 0x%02X 0x%02X 0x%02X\n",
			CHIP_CHECK_CODE_0, CHIP_CHECK_CODE_1, CHIP_CHECK_CODE_2, CHIP_CHECK_CODE_3, 
			data[coreStart], data[coreStart + 1], data[coreStart + 2], data[coreStart + 3]);		
		ret = -EINVAL;
		goto out;
	}
#endif
	dev_info(&client->dev, "fw download start\n");

	i2c_lock_adapter(adapter);
//	info->pdata->mux_fw_flash(true);

	dev_info(&client->dev, "%d, %d, %d, %d\n", info->pdata->vdd_en, info->pdata->gpio_sda, info->pdata->gpio_scl, info->pdata->gpio_int);

	gpio_direction_output(info->pdata->vdd_en, 0);
	gpio_direction_output(info->pdata->gpio_sda, 0);
	gpio_direction_output(info->pdata->gpio_scl, 0);
	gpio_direction_output(info->pdata->gpio_int, 0);

	hw_reboot(info, true);


	dev_info(&client->dev, "calibration data backup\n");
	for (i = 0; i < ISP_CAL_DATA_SIZE; i++) {
		buf[i] = flash_readl(info, addr + i);
//		dev_info(&client->dev, "cal data : 0x%02x\n", buf[i]);
	}

	val = flash_readl(info, ISP_IC_INFO_ADDR);
	dev_info(&client->dev, "IC info: 0x%02x (%x)\n", val & 0xff, val);

	dev_info(&client->dev, "fw erase...\n");
	flash_erase(info);

	dev_info(&client->dev, "fw write...\n");
	/* XXX: what does this do?! */
	usleep_range(1000, 1500);
	ret = fw_write_image(info, data, len);
	if (ret)
		goto out;

	dev_info(&client->dev, "fw download done...\n");


	dev_info(&client->dev, "restoring data\n");
	for (i = 0; i < ISP_CAL_DATA_SIZE; i++) {
		flash_writel(info, addr + i, buf[i]);
	}

	hw_reboot(info, false);
	msleep(200);
	ret = 0;

out:
	if (ret != 0)
		dev_err(&client->dev, "fw download failed...\n");

	hw_reboot(info, false);
	kfree(buf);

//	info->pdata->mux_fw_flash(false);
	i2c_unlock_adapter(adapter);

//	gpio_free(info->pdata->gpio_sda);
//	gpio_free(info->pdata->gpio_scl);
	return ret;
}



#endif

#if 1

static int mms300l_i2c_read(struct mms_ts_info *info, u16 addr, u16 length, u8 *value)
{
	struct i2c_client *client = info->client;
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_msg msg;
	int ret = -1;

	msg.addr = client->addr;
	msg.flags = 0x00;
	msg.len = 1;
	msg.buf = (u8 *) &addr;

	ret = i2c_transfer(adapter, &msg, 1);

	if (ret >= 0)
	{
		msg.addr = client->addr;
		msg.flags = I2C_M_RD;
		msg.len = length;
		msg.buf = (u8 *) value;

		ret = i2c_transfer(adapter, &msg, 1);
	}

	if (ret < 0)
	{
		dev_err(&info->client->dev, "%s: read error : [%d]", __func__, ret);
	}

	return ret;
}

static int mms300l_i2c_write(struct mms_ts_info *info, char *buf, int length)
{
	struct i2c_client *client = info->client;

	int i, temp, num;
	char *data;
	char temp_data[250] = {0, };
	data = kzalloc(sizeof(char)*TS_WRITE_REGS_LEN, GFP_KERNEL);
	if (length > TS_WRITE_REGS_LEN)
	{
		dev_err(&info->client->dev, "%s: size error : [%d]", __func__, length);
		kfree(data);
		return -EINVAL;
	}

	for (i = 0; i < length; i++)
		data[i] = *buf++;


	if (length > 250) {
		temp = length % 250;
		num = length / 250;
		for (i = 0; i < num; i++) {

			if (i == (num -1)) {
				memset(temp_data, 0, 250);
				memcpy(temp_data, &data[i * 250], 250);

				i = i2c_master_send(client, temp_data, temp);
				if (i != temp) {
					dev_err(&info->client->dev, "%s, %d: write error : [%d]", __func__, __LINE__, i);
					kfree(data);
					return -EIO;
				}		

			} else {
				memset(temp_data, 0, 250);
				memcpy(temp_data, &data[i * 250], 250);

				i = i2c_master_send(client, temp_data, 250);
				if (i != 250) {
					dev_err(&info->client->dev, "%s, %d: write error : [%d]", __func__, __LINE__,  i);
					kfree(data);
					return -EIO;
				}		

			}

		}

	} else {

		i = i2c_master_send(client, (char *) data, length);

		if (i == length){
			kfree(data);
			return length;
		}		
		else
		{
			kfree(data);	
			dev_err(&info->client->dev, "%s, %d: write error : [%d]", __func__, __LINE__,  i);
			return -EIO;
		}
	}


	kfree(data);
	return length;
}

static int mms300l_compare_version_info(struct mms_ts_info *info, const u8 *fw_data, struct mms_bin_hdr *fw_hdr)
{
	unsigned char rd_buf[8] = {0, };
	int retval, ii = 0, size;
	int offset;

	dev_info(&info->client->dev, "%s\n", __func__);

	memset(rd_buf, 0, 8);
	retval = mms300l_i2c_read(info, ISC_ADDR_VERSION, SECTION_NUM, rd_buf);
	if (retval < 0) {
		dev_info(&info->client->dev, "%s: mms do not read ISC_ADDR_VERSION\n", __func__);
		return -1;
	}

	dev_info(&info->client->dev, "[IC FW version] %s: %X, %X, %X, %X, %X, %X, %X, %X, \n",
			__func__, rd_buf[0], rd_buf[1], rd_buf[2], rd_buf[3], 
			rd_buf[4], rd_buf[5], rd_buf[6], rd_buf[7]);

	/* rd_buf[0] = bootloader, rd_buf[1] = core, rd_buf[2] = config */
	for (ii = 0; ii < SECTION_NUM; ii++)
		info->fw_info_ic[ii].version = rd_buf[ii];

	/* i donw understand below compatible version */
	info->fw_info_ic[SEC_CORE].compatible_version = info->fw_info_ic[SEC_BOOTLOADER].version;
	info->fw_info_ic[SEC_CONFIG].compatible_version = info->fw_info_ic[SEC_CORE].version;

	memset(rd_buf, 0, 8);
	retval = mms300l_i2c_read(info, ISC_ADDR_SECTION_PAGE_INFO, sizeof(rd_buf), rd_buf);
	if (retval < 0) {
		dev_info(&info->client->dev, "%s: mms do not read ISC_ADDR_SECTION_PAGE_INFO\n", __func__);
		return -1;
	}
	dev_info(&info->client->dev, "%s: [IC page info] %X, %X, %X, %X, %X, %X, %X, %X, \n",
			__func__, rd_buf[0], rd_buf[1], rd_buf[2], rd_buf[3], 
			rd_buf[4], rd_buf[5], rd_buf[6], rd_buf[7]);

	for (ii = 0; ii < SECTION_NUM; ii++) {
		info->fw_info_ic[ii].start_addr = rd_buf[ii];
//		info->fw_info_ic[ii].start_addr = rd_buf[ii + (sizeof(rd_buf) / 2)];
		info->fw_info_ic[ii].end_addr = rd_buf[ii + SECTION_NUM +1]; // bug fixed

	}

	for (ii = 0; ii < SECTION_NUM; ii++) {
		dev_info(&info->client->dev,
				"%s: Section(%d) version: 0x%02X\n",
				__func__, ii, info->fw_info_ic[ii].version);
		dev_info(&info->client->dev,
				"%s: Section(%d) Start Address: 0x%02X\n",
				__func__, ii, info->fw_info_ic[ii].start_addr);
		dev_info(&info->client->dev,
				"%s: Section(%d) End Address: 0x%02X\n",
				__func__, ii, info->fw_info_ic[ii].end_addr);
		dev_info(&info->client->dev,
				"%s: Section(%d) Compatibility: 0x%02X\n",
				__func__, ii, info->fw_info_ic[ii].compatible_version);
	}

/********************* Upper : check IC firmare configuration. ************************/
/********************* Lower : check BIN firmare configuration. ************************/

	size = fw_hdr->binary_length;
	offset = sizeof(struct mms_bin_hdr);

	dev_info(&info->client->dev,
		"%s, tag:%s, core_version:0x%X, section_number:%d, contain_full_bin:%d, binary_offset:0x%X, lenght:%d[0x%X], offset:%X\n",
		__func__, fw_hdr->tag,
		fw_hdr->core_version, fw_hdr->section_num, 
		fw_hdr->contains_full_binary, fw_hdr->binary_offset, 
		fw_hdr->binary_length, fw_hdr->binary_length, offset);

	for (ii = 0; ii < fw_hdr->section_num; ii++) {
		memcpy(&info->img_info[ii], fw_data + offset, sizeof(struct mms_fw_img));
		offset += sizeof(struct mms_fw_img);

		dev_info(&info->client->dev,
				"%s: type:%d, version:0x%X, start:0x%X, end:0x%X, offset:0x%X, length:0x%X\n",
				__func__, info->img_info[ii].type, info->img_info[ii].version,
				info->img_info[ii].start_page, info->img_info[ii].end_page,
				info->img_info[ii].offset, info->img_info[ii].length);


		info->fw_info_bin[ii].version = info->img_info[ii].version;
		info->fw_info_bin[ii].start_addr = info->img_info[ii].start_page;
		info->fw_info_bin[ii].end_addr= info->img_info[ii].end_page;
		info->fw_info_bin[ii].compatible_version = info->img_info[(ii == 0) ? ii : (ii -1)].version;

		dev_info(&info->client->dev,
				"%s: type:%d, version:0x%X, compatible_version:0x%X\n",
				__func__, ii, info->fw_info_bin[ii].version,
				info->fw_info_bin[ii].compatible_version);

	}

	for (ii = SECTION_START; ii < SECTION_NUM; ii++) {
		/* SET all section update True */
		info->section_update_flag[ii] = true;

		/* SET section update flag, select highest level for update section.
		 * if boot updae: all update, core update: core, config update, config update: only config update.
		 */
//		if (info->fw_info_ic[ii].version < info->fw_info_bin[ii].version) {
		if ((info->fw_info_ic[ii].version < info->fw_info_bin[ii].version)||(info->fw_info_ic[ii].version >= 0xC0)) {  // bug fixed
			info->section_update_flag[ii] = true;
			retval = ii;
			break;
		} else {
			info->section_update_flag[ii] = false;
			retval = 0;
		}
	}

///////////////////////////////
//   always update
//	info->section_update_flag[2] = true;
//	retval = 2;
///////////////////////////////

	dev_info(&info->client->dev,
			"%s: update section: boot: %d, core: %d, config: %d\n",
			__func__, info->section_update_flag[0],
			info->section_update_flag[1], info->section_update_flag[2]);

	return retval;
}

static int mms300l_ISC_clear_page(struct mms_ts_info *info, unsigned char page_addr)
{
	int ret;
	unsigned char rd_buf;

	dev_info(&info->client->dev, "%s\n", __func__);
	memset(&g_wr_buf[3], 0xFF, PAGE_DATA);

	g_wr_buf[0] = ISC_CMD_UPDATE_MODE; // command
	g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE; // sub_command
	g_wr_buf[2] = page_addr;

	g_wr_buf[PAGE_HEADER + PAGE_DATA] = crc0_buf[page_addr];
	g_wr_buf[PAGE_HEADER + PAGE_DATA + 1] = crc1_buf[page_addr];

	ret = mms300l_i2c_write(info, g_wr_buf, PACKET_SIZE);
	if (ret < 0) {
		dev_err(&info->client->dev,"%s: i2c write fail[%d] \n", __func__, ret);
		return -1;
	}

	ret = mms300l_i2c_read(info, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);
	if (ret < 0) {
		dev_err(&info->client->dev,"%s: i2c read fail[%d] \n", __func__, ret);
		return -1;
	}

	if (rd_buf != ISC_STATUS_CRC_CHECK_SUCCESS){
		dev_err(&info->client->dev,"%s: i2c rd_buf(0x3) crc error[%d] \n", __func__, rd_buf);
		return -1 * rd_buf;
	}

	return rd_buf;

}

static int mms300l_ISC_clear_validate_markers(struct mms_ts_info *info)
{
	int retval;
	int ii, jj;
	bool is_matched_address;

	dev_info(&info->client->dev, "%s\n", __func__);

	for (ii = SEC_CORE; ii <= SEC_CONFIG; ii++) {
		if (info->section_update_flag[ii]) {
			if (info->fw_info_ic[ii].end_addr <= 30 &&
				info->fw_info_ic[ii].end_addr > 0) {

				retval = mms300l_ISC_clear_page(info, info->fw_info_ic[ii].end_addr);
				if (retval < 0)
					return retval;
			}
		}
	}

	dev_info(&info->client->dev, "%s2\n", __func__);

	for (ii = SEC_CORE; ii <= SEC_CONFIG; ii++) {
		if (info->section_update_flag[ii]) {
			is_matched_address = false;
			for (jj = SEC_CORE; jj <= SEC_CONFIG; jj++) {
				if (info->fw_info_bin[ii].end_addr == info->fw_info_ic[ii].end_addr) {
					is_matched_address = true;
					break;
				}
			}

			if (!is_matched_address) {
				if (info->fw_info_bin[ii].end_addr <= 30 && info->fw_info_bin[ii].end_addr > 0) {
					retval = mms300l_ISC_clear_page(info, info->fw_info_bin[ii].end_addr);

					if (retval < 0)
						return retval;
				}
			}
		}
	}

	dev_info(&info->client->dev, "%s done\n", __func__);

	return 0;;
}

static void mms300l_calc_crc(struct mms_ts_info *info, unsigned char *crc, int page_addr, unsigned char* ptr_fw)
{
	int i, j;

	unsigned char ucData;

	unsigned short SeedValue;
	unsigned short CRC_check_buf;
	unsigned short CRC_send_buf;
	unsigned short IN_data;
	unsigned short XOR_bit_1;
	unsigned short XOR_bit_2;
	unsigned short XOR_bit_3;

	// Seed

	dev_info(&info->client->dev, "%s\n", __func__);

	CRC_check_buf = 0xFFFF;
	SeedValue = (unsigned short) page_addr;

	for (i = 7; i >= 0; i--)
	{
		IN_data = (SeedValue >> i) & 0x01;
		XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
		XOR_bit_2 = XOR_bit_1 ^ (CRC_check_buf >> 11 & 0x01);
		XOR_bit_3 = XOR_bit_1 ^ (CRC_check_buf >> 4 & 0x01);
		CRC_send_buf = (XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
		CRC_send_buf = (CRC_send_buf << 7) | (XOR_bit_2 << 6) | (CRC_check_buf >> 5 & 0x3F);
		CRC_send_buf = (CRC_send_buf << 4) | (XOR_bit_3 << 3) | (CRC_check_buf >> 1 & 0x0007);
		CRC_check_buf = CRC_send_buf;
	}

	for (i = 0; i < 1024; i++)
	{
		ucData = ptr_fw[i];

		for (j = 7; j >= 0; j--)
		{
			IN_data = (ucData >> j) & 0x0001;
			XOR_bit_1 = (CRC_check_buf & 0x0001) ^ IN_data;
			XOR_bit_2 = XOR_bit_1 ^ (CRC_check_buf >> 11 & 0x01);
			XOR_bit_3 = XOR_bit_1 ^ (CRC_check_buf >> 4 & 0x01);
			CRC_send_buf = (XOR_bit_1 << 4) | (CRC_check_buf >> 12 & 0x0F);
			CRC_send_buf = (CRC_send_buf << 7) | (XOR_bit_2 << 6) | (CRC_check_buf >> 5 & 0x3F);
			CRC_send_buf = (CRC_send_buf << 4) | (XOR_bit_3 << 3) | (CRC_check_buf >> 1 & 0x0007);
			CRC_check_buf = CRC_send_buf;
		}
	}

	crc[0] = (unsigned char) ((CRC_check_buf >> 8) & 0xFF);
	crc[1] = (unsigned char) ((CRC_check_buf >> 0) & 0xFF);
}

static int mms300l_update_section_data(struct mms_ts_info *info, const u8 *firm_data, struct mms_bin_hdr *fw_hdr)
{
	int ii, jj, retval; 
	unsigned char rd_buf;
	unsigned char crc[2]; 
	int ptr;
	const u8 *ptr_fw;
	int page_addr;
	const u8 *fw_data;

	dev_info(&info->client->dev, "%s\n", __func__);
	
	fw_data = (u8 *)firm_data + fw_hdr->binary_offset;
	
	dev_info(&info->client->dev, "%s: 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, \n",
			__func__, fw_data[0], fw_data[1], fw_data[2], fw_data[3], fw_data[4]);

	for (ii = 0; ii < fw_hdr->section_num; ii++) {
		dev_info(&info->client->dev, "%s: update flag (%d)\n",
				__func__, info->section_update_flag[ii]);

		ptr = info->img_info[ii].offset;

		if (info->section_update_flag[ii]) {
			ptr_fw = fw_data + ptr;

			for (page_addr = info->fw_info_bin[ii].start_addr; page_addr <= info->fw_info_bin[ii].end_addr; page_addr++) {
				if (page_addr - info->fw_info_bin[ii].start_addr > 0)
					ptr_fw += 1024;

				g_wr_buf[0] = ISC_CMD_UPDATE_MODE;
				g_wr_buf[1] = ISC_SUBCMD_DATA_WRITE;
				g_wr_buf[2] = (unsigned char) page_addr;

				for (jj = 0; jj < 1024; jj += 4) {
					g_wr_buf[3 + jj] = ptr_fw[jj + 3];
					g_wr_buf[3 + jj + 1] = ptr_fw[jj + 2];
					g_wr_buf[3 + jj + 2] = ptr_fw[jj + 1];
					g_wr_buf[3 + jj + 3] = ptr_fw[jj + 0];
				}

				mms300l_calc_crc(info, crc, page_addr, &g_wr_buf[3]);

				g_wr_buf[1027] = crc[0];
				g_wr_buf[1028] = crc[1];

				dev_info(&info->client->dev, "%s: crc val : %X%X\n",
						__func__, crc[0], crc[1]);

				retval = mms300l_i2c_write(info, g_wr_buf, PACKET_SIZE);
				if (retval < 0) {
					dev_err(&info->client->dev, "%s: i2c write fail[%d] \n", __func__, retval);
					return -1;
				}

				retval = mms300l_i2c_read(info, ISC_CMD_CONFIRM_STATUS, 1, &rd_buf);
				if (retval < 0) {
					dev_err(&info->client->dev, "%s: i2c read fail[%d] \n", __func__, retval);
					return -1;
				}

				if (rd_buf != ISC_STATUS_CRC_CHECK_SUCCESS)
					return -2;

				info->section_update_flag[ii] = false;
				dev_info(&info->client->dev, "%s: section(%d) updated.\n", __func__, ii);
			}
		}
	}

	dev_info(&info->client->dev, "%s: End mms300l_update_section_data()\n", __func__);

	return 0;
}

static int mms_pinctrl_configure(struct mms_ts_info *info, 
							int active)
{
	struct pinctrl_state *set_state_i2c;
//	struct pinctrl_state *set_state_en;
	int retval;

	if (info->pdata->gpio_seperated < 1) {
		dev_err(&info->client->dev, "%s: not support this ftn. under mms345\n", __func__);
		return 0;
	}

	dev_err(&info->client->dev, "%s: %d\n", __func__, active);


	if(active == 0){ // default
		set_state_i2c =	pinctrl_lookup_state(info->pinctrl,	"tsp_gpio_default");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) active state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}

	}else if(active == 1) {	// firmup enter
		set_state_i2c =	pinctrl_lookup_state(info->pinctrl,	"tsp_firmup_start");
			if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) firmup on state\n", __func__);
				return PTR_ERR(set_state_i2c);
			}
	}else if(active == 2) { // firmup exit
		set_state_i2c =	pinctrl_lookup_state(info->pinctrl,	"tsp_firmup_end");
			if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev, "%s: cannot get pinctrl(i2c) firmup off state\n", __func__);
				return PTR_ERR(set_state_i2c);
			}
	}else{
		dev_err(&info->client->dev, "%s: cannot set pinctrl(i2c) %d state\n",__func__, active);
		return 1;
	}
	retval = pinctrl_select_state(info->pinctrl, set_state_i2c);
	if (retval) {
		dev_err(&info->client->dev, "%s: cannot set pinctrl(i2c) %d state\n", __func__, active);
		return retval;
	}

	return 0;
}


int fw_download_isc(struct mms_ts_info *info, const u8 *fw_data)
{
	int retval;
	unsigned char rd_buf[10] = {0, };
	unsigned char wr_buf[10] = {0, };
	int binOffset = 0;
	int coreStart = 1024;
	struct mms_bin_hdr *fw_hdr = (struct mms_bin_hdr *)fw_data;

	dev_info(&info->client->dev, "%s\n", __func__);

	memset(rd_buf, 0, 10);

	mms_reset(info);
	dev_info(&info->client->dev, "%s: reset done.\n", __func__);

	retval = mms300l_i2c_read(info, ISC_ADDR_VERSION, SECTION_NUM, rd_buf);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not operate, %d\n", __func__, retval);
		return -1;
	}

	binOffset = fw_data[0x10] + (fw_data[0x11] << 8) + (fw_data[0x12] << 16) + (fw_data[0x13] << 24);
	dev_info(&info->client->dev, "ISP : F/W Offset 0x%X\n", binOffset);
	coreStart = binOffset + 1024;
	dev_info(&info->client->dev, "ISP : F/W Start 0x%X\n", coreStart);


	if(((fw_data[coreStart] == CHIP_CHECK_CODE_0) &&
		(fw_data[coreStart + 1] == CHIP_CHECK_CODE_1) &&
		(fw_data[coreStart + 2] == CHIP_CHECK_CODE_2) &&
		(fw_data[coreStart + 3] == CHIP_CHECK_CODE_3)) != 1) {
		dev_err(&info->client->dev,
			"ISC : F/W is not for MMS300L (0x%02X 0x%02X 0x%02X 0x%02X) : 0x%02X 0x%02X 0x%02X 0x%02X\n",
			CHIP_CHECK_CODE_0, CHIP_CHECK_CODE_1, CHIP_CHECK_CODE_2, CHIP_CHECK_CODE_3, 
			fw_data[coreStart], fw_data[coreStart + 1], fw_data[coreStart + 2], fw_data[coreStart + 3]);		
		return -1;
	}

	/* check firmware version, select update section. */
	retval = mms300l_compare_version_info(info, fw_data, fw_hdr);
	if (retval < 0)
		return retval;
	else if (retval == 0)
		return 0;
	else
		dev_info(&info->client->dev, "%s: update section, %d\n", __func__, retval);

	/* enter ISC, ISC Update mode */
	wr_buf[0] = ISC_CMD_ENTER_ISC;
	wr_buf[1] = ISC_CMD_ENTER_ISC_PARA1;

	retval = mms300l_i2c_write(info, wr_buf, 2);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not enter ISC mode, %d\n", __func__, retval);
		return -1;
	} else {
		dev_info(&info->client->dev, "%s: mms enter ISC mode, %d\n", __func__, retval);
	}

	msleep(50);

	wr_buf[0] = ISC_CMD_UPDATE_MODE;
	wr_buf[1] = ISC_SUBCMD_ENTER_UPDATE;

	retval = mms300l_i2c_write(info, wr_buf, 10);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not enter ISC Update mode[WRITE], %d\n", __func__, retval);
		return -1;
	} else {
		dev_info(&info->client->dev, "%s: mms enter ISC Update mode[WRITE], %d\n", __func__, retval);
	}

	memset(rd_buf, 0, 10);
	retval = mms300l_i2c_read(info, ISC_CMD_CONFIRM_STATUS, 1, rd_buf);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not enter ISC Update mode[READ], %d\n", __func__, retval);
		return -1;
	} else {
		dev_info(&info->client->dev, "%s: mms enter ISC Update mode[READ], %d\n", __func__, retval);
	}

	g_wr_buf = kzalloc(1024 + 3 + 2, GFP_KERNEL);

	retval = mms300l_ISC_clear_validate_markers(info);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not clear validata merkers, %d\n", __func__, retval);
		goto error_update;
	} else {
		dev_info(&info->client->dev, "%s: mms clear validata merkers, %d\n", __func__, retval);
	}	

	retval = mms300l_update_section_data(info, fw_data, fw_hdr);
	if (retval < 0) {
		dev_err(&info->client->dev, "%s: mms do not update section, %d\n", __func__, retval);
		goto error_update;
	} else {
		dev_info(&info->client->dev, "%s: mms update section, %d\n", __func__, retval);
	}	

error_update:
	kfree(g_wr_buf);

	return 0;
}

int mms_flash_fw_mms300(struct mms_ts_info *info, const u8 *fw_data, size_t fw_size)
{
	int ret;
	struct mms_bin_hdr *fw_hdr;

	fw_hdr = (struct mms_bin_hdr *)fw_data;
	
	pr_err("%s: %d\n", __func__, __LINE__);
	if (info->pdata->use_isp){
		
		if (info->pinctrl) {
			ret = mms_pinctrl_configure(info, 1);
			if (ret)
				pr_err("%s: cannot set pinctrl state, %d\n", __func__, __LINE__);
		}
		ret = fw_download_isp(info, fw_data + fw_hdr->binary_offset, fw_hdr->binary_length);
		
		if (info->pinctrl) {
			ret = mms_pinctrl_configure(info, 2);
			if (ret)
				pr_err("%s: cannot set pinctrl state, %d\n", __func__, __LINE__);
		}
		
	}else
		ret = fw_download_isc(info, fw_data);
	/* ISP write all firmware exclude only header */
	/* ISC can sellect boot, core, config. need read header contents. */
	return ret;
}

#endif


static int mms_ts_fw_check(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int ret, retry;
	char buf[4] = { 0, };
	bool update = false;
	info->fw_path = FW_IMAGE_NAME;


	ret = i2c_master_recv(client, buf, 1);
	if (ret < 0) {		/* tsp connect check */
		dev_err(&client->dev, "%s: i2c fail...[%d], addr[%d]\n",
			   __func__, ret, info->client->addr);
	}

	ret = get_fw_version(info);
	if (ret != 0) {
			dev_err(&client->dev, "fw_version read fail\n");
			update = true;
			goto try_firmup;
	}

        // for 345 IC, 345L IC
	if((info->fw_boot_ver > 0xA0)||(info->fw_core_ver>0xA0)||(info->fw_ic_ver>0xA0)){
		update = true;
	}
	
	if (!update) {
		if ((info->fw_boot_ver < BOOT_VERSION) ||
			(info->fw_core_ver < CORE_VERSION) ||
			(info->fw_ic_ver < FW_VERSION)) {
			if (info->pdata->gpio_seperated > 0) {
				dev_err(&client->dev,
					"%s: update firmware IC[%X,%X,%X], BIN[%X,%X,%X]\n",
					__func__, info->fw_boot_ver, info->fw_core_ver, info->fw_ic_ver
					,BOOT_VERSION, CORE_VERSION, FW_VERSION);
				update = true;
			} else {
				dev_err(&client->dev,
					"%s: MMS345 is not update firmware. only upadte MMS345L\n",
					__func__);
				update = false;
			}
		}
	}

	if(update){
		if(info->fw_ic_ver <= 0x02){
			update = false;
			dev_err(&client->dev, "%d module don't firmup, up:%d",info->fw_ic_ver, update );
		}
	}
try_firmup:	
	if (update) {
		dev_err(&client->dev, "excute mms_flash_fw\n");
		retry = 1;
		while (retry--) {
			ret = mms_load_fw_from_kernel(info, info->fw_path);
			if (ret) {
				dev_err(&client->dev,
					"failed to mms_flash_fw (%d)\n", ret);
				dev_err(&client->dev,
					"retry flash (%d)\n", retry);
			} else {
				dev_info(&client->dev,
					"fw update success\n");
				break;
			}
		}
	}

	if (update && !!ret)
		return -1;

	return 0;
}

static void melfas_request_gpio(struct melfas_tsi_platform_data *pdata)
{
	int ret;
	pr_info("[TSP] %s called \n", __func__);

	ret = gpio_request(pdata->gpio_int, "melfas_tsp_irq");
	if (ret) {
		pr_err("[TSP]%s: unable to request melfas_tsp_irq [%d]\n",
				__func__, pdata->gpio_int);
		return;
	}
	ret = gpio_request(pdata->vdd_en, "melfas_vdd_en");
	if (ret) {
		pr_err("[TSP]%s: unable to request melfas_vdd_en [%d]\n",
				__func__, pdata->vdd_en);
		return;
	}
	ret = gpio_request(pdata->gpio_sda, "melfas_gpio_sda");
	if (ret) {
		pr_err("[TSP]%s: unable to request melfas_gpio_sda [%d]\n",
				__func__, pdata->gpio_sda);
		return;
	}
	ret = gpio_request(pdata->gpio_scl, "melfas_gpio_scl");
	if (ret) {
		pr_err("[TSP]%s: unable to request melfas_gpio_scl [%d]\n",
				__func__, pdata->gpio_scl);
		return;
	}
}
#ifdef CONFIG_OF

static int mms_get_dt_coords(struct device *dev, char *name,
				struct melfas_tsi_platform_data *pdata)
{
	u32 coords[MMS_COORDS_ARR_SIZE];
	struct property *prop;
	struct device_node *np = dev->of_node;
	int coords_size, rc;

	prop = of_find_property(np, name, NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;

	coords_size = prop->length / sizeof(u32);
	if (coords_size != MMS_COORDS_ARR_SIZE) {
		dev_err(dev, "invalid %s\n", name);
		return -EINVAL;
	}

	rc = of_property_read_u32_array(np, name, coords, coords_size);
	if (rc && (rc != -EINVAL)) {
		dev_err(dev, "Unable to read %s\n", name);
		return rc;
	}

	if (strncmp(name, "melfas,panel-coords",
			sizeof("melfas,panel-coords")) == 0) {
		pdata->invert_x = coords[0];
		pdata->invert_y = coords[1];
		pdata->max_x = coords[2];
		pdata->max_y = coords[3];
	} else {
		dev_err(dev, "unsupported property %s\n", name);
		return -EINVAL;
	}

	return 0;
}



int melfas_power(struct mms_ts_info *info, int on){

	int ret, rc = 0;

	if (tsp_power_enabled == on)
		return 0;
	printk(KERN_DEBUG "[TSP] %s %s\n",
				__func__, on ? "on" : "off");

	if (!info->vcc_i2c) {
		if (info->pdata->i2c_pull_up) {
			info->vcc_i2c = regulator_get(&info->client->dev,
				"8916_l6");
			if (IS_ERR(info->vcc_i2c)) {
				rc = PTR_ERR(info->vcc_i2c);
				dev_err(&info->client->dev,
					"Regulator get failed rc=%d\n", rc);
				return 0;
			}
		}
	}

	if (info->vcc_i2c) {
		if(on){
			rc = regulator_enable(info->vcc_i2c);
			if (rc) {
				dev_err(&info->client->dev,
				"Regulator 8916_l5 enable failed rc=%d\n",
				rc);
				return 0;
			}
			
		}else{
			rc = regulator_disable(info->vcc_i2c);
			if (rc) {
				dev_err(&info->client->dev,
				"Regulator 8916_l5 enable failed rc=%d\n",
				rc);
				return 0;
			}
		}
	}
	
	ret = gpio_direction_output(info->pdata->vdd_en, on);
		if (ret) {
			dev_err(&info->client->dev,"[TSP]%s: unable to set_direction for mms_vdd_en [%d]\n",
					__func__, info->pdata->vdd_en);
		return 0;
	}

	tsp_power_enabled = on;
	return 0;
}

#ifdef TOUCHKEY
int key_led_control(struct mms_ts_info *info, int on)
{

	printk(KERN_DEBUG "[TSP] %s %s\n",
		__func__, on ? "on" : "off");
	if(info->pdata->tkey_led_en >= 0){
		gpio_set_value(info->pdata->tkey_led_en,on);
	}
	return 0;
}

#endif
static int mms_parse_dt(struct device *dev,
			struct melfas_tsi_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	struct property *prop;
	u32 isp_isc[2];
	int size, rc;

	rc = mms_get_dt_coords(dev, "melfas,panel-coords", pdata);
	if (rc)
		return rc;

	/* regulator info */
	pdata->i2c_pull_up = of_property_read_bool(np, "melfas,i2c-pull-up");
	pdata->vdd_en = of_get_named_gpio(np, "melfas,tsppwr_en", 0);

	pdata->gpio_temp = of_get_named_gpio(np, "melfas,temp", 0);

	/* reset, irq gpio info */
	pdata->gpio_scl = of_get_named_gpio_flags(np, "melfas,scl-gpio",
				0, &pdata->scl_gpio_flags);
	pdata->gpio_sda = of_get_named_gpio_flags(np, "melfas,sda-gpio",
				0, &pdata->sda_gpio_flags);
	pdata->gpio_int = of_get_named_gpio_flags(np, "melfas,irq-gpio",
				0, &pdata->irq_gpio_flags);
	pdata->config_fw_version = of_get_property(np,
				"melfas,config_fw_version", NULL);

	of_property_read_u32(np, "melfas,gpio_seperated", &pdata->gpio_seperated);

	prop = of_find_property(np, "melfas,update-func", NULL);
	if (prop && prop->value) {

		size = prop->length / sizeof(u32);
		if (size != 2) {
			dev_err(dev, "invalid %s\n", "melfas,update-func");
		} else {

			rc = of_property_read_u32_array(np, "melfas,update-func", isp_isc, size);
			if (rc && (rc != -EINVAL)) {
				dev_err(dev, "Unable to read %s\n", "melfas,update-func");
			} else {
				pdata->use_isp = isp_isc[0];
				pdata->use_isc = isp_isc[1];
			}
		}

	}

	pr_info("%s: en_gpio:%d, seperated:%d, isp:%d, isc:%d\n", __func__,
			pdata->vdd_en, pdata->gpio_seperated, pdata->use_isp, pdata->use_isc);

	return 0;
}
#else
static int mms_parse_dt(struct device *dev,
			struct melfas_tsi_platform_data *pdata)
{
	return -ENODEV;
}
#endif

#if 0
static ssize_t melfas_enabled_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	long input;
	int ret = 0;

	ret = kstrtol(buf, 10, &input);
	if(ret || ret < 0){
            printk(KERN_INFO "Error value melfas_enabled_store \n");
            return ret;
        }
	pr_info("TSP enabled: %d\n",(int)input);

	if(input == 1)
		mms_ts_resume(dev);
	else if(input == 0)
		mms_ts_suspend(dev);
	else
		return -EINVAL;

	return count;
}

static struct device_attribute attrs[] = {
	__ATTR(enabled, (S_IRUGO | S_IWUSR | S_IWGRP),
			NULL,
			melfas_enabled_store),
};
#endif

/* START - Added to support API's for TSP tuning */
static int mms_fs_open(struct inode *node, struct file *fp)
{
	struct mms_ts_info *info;
	struct i2c_client *client;
	struct i2c_msg msg;
	u8 buf[3] = {
		ADDR_UNIV_CMD,//MMS_UNIVERSAL_CMD,
		MMS_CMD_SET_LOG_MODE,
		true,
	};

	info = container_of(node->i_cdev, struct mms_ts_info, cdev);
	client = info->client;

	disable_irq(info->irq);
	fp->private_data = info;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = buf;
	msg.len = sizeof(buf);

	i2c_transfer(client->adapter, &msg, 1);

	info->log.data = kzalloc(MAX_LOG_LENGTH * 20 + 5, GFP_KERNEL);

	//mms_clear_input_data(info);
	touch_is_pressed = 0;
	release_all_fingers(info);

	return 0;
}

static int mms_fs_release(struct inode *node, struct file *fp)
{
	struct mms_ts_info *info = fp->private_data;

//	mms_clear_input_data(info);
	touch_is_pressed = 0;
	release_all_fingers(info);

	//mms_reboot(info);
	mms_reset(info);

	kfree(info->log.data);
	enable_irq(info->irq);

	return 0;
}

static int esd_cnt;
static void mms_report_input_data(struct mms_ts_info *info, u8 sz, u8 *buf)
{
	int i;
	struct i2c_client *client = info->client;
	int id;
	int x;
	int y;
	int touch_major;
	int pressure;
	int key_code;
	int key_state;
	u8 *tmp;

	if (buf[0] == MMS_NOTIFY_EVENT) {
		dev_info(&client->dev, "TSP mode changed (%d)\n", buf[1]);
		goto out;
	} else if (buf[0] == MMS_ERROR_EVENT) {
		dev_info(&client->dev, "Error detected, restarting TSP\n");
		//mms_clear_input_data(info);
		touch_is_pressed = 0;
		release_all_fingers(info);

		//mms_reboot(info);
		mms_reset(info);
		esd_cnt++;
		if (esd_cnt>= ESD_DETECT_COUNT)
		{
			i2c_smbus_write_byte_data(info->client, MMS_MODE_CONTROL, 0x04);
			esd_cnt = 0;
		}
		goto out;
	}

	for (i = 0; i < sz; i += FINGER_EVENT_SZ) {
		tmp = buf + i;
		esd_cnt = 0;
		if (tmp[0] & MMS_TOUCH_KEY_EVENT) {
			switch (tmp[0] & 0xf) {
			case 1:
				key_code = info->pdata->key1;
				break;
			case 2:
				key_code = KEY_BACK;
				break;
			default:
				dev_err(&client->dev, "unknown key type\n");
				goto out;
				break;
			}
			key_state = (tmp[0] & irq_bit_mask) ? 1 : 0;
			input_report_key(info->input_dev, key_code, key_state);
		} else {
			id = (tmp[0] & 0xf) -1;
			x = tmp[2] | ((tmp[1] & 0xf) << 8);
			y = tmp[3] | (((tmp[1] >> 4 ) & 0xf) << 8);
			touch_major = tmp[4];
			pressure = tmp[5];

			input_mt_slot(info->input_dev, id);
		
			if(!(tmp[0] & irq_bit_mask)) {
				input_report_key(info->input_dev, BTN_TOUCH, 0);
				input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, false);
				continue;
			}

			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, true);
			input_report_abs(info->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(info->input_dev, ABS_MT_POSITION_Y, y);
			input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR, touch_major);
			input_report_abs(info->input_dev, ABS_MT_PRESSURE, pressure);
			input_report_key(info->input_dev, BTN_TOUCH, 1);
		}
	}

	input_sync(info->input_dev);

out:
	return;

}

static void mms_event_handler(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int sz;
	int ret;
	int row_num;
	u8 reg = MMS_INPUT_EVENT;
	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.buf = &reg,
			.len = 1,
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.buf = info->log.data,
		},

	};
	struct mms_log_pkt {
		u8	marker;
		u8	log_info;
		u8	code;
		u8	element_sz;
		u8	row_sz;
	} __attribute__ ((packed)) *pkt = (struct mms_log_pkt *)info->log.data;

	memset(pkt, 0, sizeof(*pkt));

	//if (gpio_get_value(info->pdata->gpio_int))
	//	return;

	sz = i2c_smbus_read_byte_data(client, MMS_EVENT_PKT_SZ);
	msg[1].len = sz;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret != ARRAY_SIZE(msg)) {
		dev_err(&client->dev,
			"failed to read %d bytes of data\n",
			sz);
		return;
	}

	if ((pkt->marker & 0xf) == MMS_LOG_EVENT) {
		if ((pkt->log_info & 0x7) == 0x1) {
			pkt->element_sz = 0;
			pkt->row_sz = 0;

			return;
		}

		switch (pkt->log_info >> 4) {
		case LOG_TYPE_U08:
		case LOG_TYPE_S08:
			msg[1].len = pkt->element_sz;
			break;
		case LOG_TYPE_U16:
		case LOG_TYPE_S16:
			msg[1].len = pkt->element_sz * 2;
			break;
		case LOG_TYPE_U32:
		case LOG_TYPE_S32:
			msg[1].len = pkt->element_sz * 4;
			break;
		default:
			dev_err(&client->dev, "invalied log type\n");
			return;
		}

		msg[1].buf = info->log.data + sizeof(struct mms_log_pkt);
		reg = CMD_RESULT;//MMS_UNIVERSAL_RESULT;
		row_num = pkt->row_sz ? pkt->row_sz : 1;

		while (row_num--) {
			//while (gpio_get_value(info->pdata->gpio_int))
			//	;
			ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
			msg[1].buf += msg[1].len;
		};
	} else {
		mms_report_input_data(info, sz, info->log.data);
		memset(pkt, 0, sizeof(*pkt));
	}

	return;
}

static ssize_t mms_fs_read(struct file *fp, char *rbuf, size_t cnt, loff_t *fpos)
{
	struct mms_ts_info *info = fp->private_data;
	struct i2c_client *client = info->client;
	int ret = 0;
	char rx_num, tx_num;

	switch (info->log.cmd) {
	case GET_RX_NUM:
		rx_num = RX_NUM;
		ret = copy_to_user(rbuf, &rx_num, 1);
		break;
	case GET_TX_NUM:
		tx_num = TX_NUM;
		ret = copy_to_user(rbuf, &tx_num, 1);
		break;
	case GET_EVENT_DATA:
		mms_event_handler(info);
		/* copy data without log marker */
		ret = copy_to_user(rbuf, info->log.data + 1, cnt);
		break;
	default:
		dev_err(&client->dev, "unknown command\n");
		ret = -EFAULT;
		break;
	}

	return ret;

}

static ssize_t mms_fs_write(struct file *fp, const char *wbuf, size_t cnt, loff_t *fpos)
{
	struct mms_ts_info *info = fp->private_data;
	struct i2c_client *client = info->client;
	u8 *buf;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = cnt,
	};
	int ret = 0;

	mutex_lock(&info->lock);

	if (!info->enabled)
		goto tsp_disabled;

	msg.buf = buf = kzalloc(cnt + 1, GFP_KERNEL);

	if ((buf == NULL) || copy_from_user(buf, wbuf, cnt)) {
		dev_err(&client->dev, "failed to read data from user\n");
		ret = -EIO;
		goto out;
	}

	if (cnt == 1) {
		info->log.cmd = *buf;
	} else {
		if (i2c_transfer(client->adapter, &msg, 1) != 1) {
			dev_err(&client->dev, "failed to transfer data\n");
			ret = -EIO;
			goto out;
		}
	}

	ret = 0;

out:
	kfree(buf);
tsp_disabled:
	mutex_unlock(&info->lock);

	return ret;
}

static struct file_operations mms_fops = {
	.owner		= THIS_MODULE,
	.open		= mms_fs_open,
	.release	= mms_fs_release,
	.read		= mms_fs_read,
	.write		= mms_fs_write,
};
/* END - Added to support API's for TSP tuning */

#ifdef USE_OPEN_CLOSE
static int mms_ts_input_open(struct input_dev *dev)
{
	struct mms_ts_info *info;

	info = input_get_drvdata(dev);

	if (!info->input_closed) {
		dev_info(&info->client->dev, "%s: already opened\n", __func__);
		return 0;
	}

	dev_info(&info->client->dev, "%s, v:0x%02x, g:%d\n", __func__, info->fw_ic_ver,
#ifdef TSP_GLOVE_MODE
		info->glove_mode);
#else
		0);
#endif
	

#ifdef TSP_GESTURE_MODE
	if(info->lowpower_mode){		
		dev_info(&info->client->dev, "lowpoer mode enter, f:%d, m:%d\n",info->lowpower_flag,info->lowpower_mode );
		disable_irq_wake(info->client->irq);
		lpm_mode_enter(info,0);
	//	info->enabled = true;
	}else
#endif
	{
		info->power(info, 1);
		//mms_pinctrl_configure(info, true, false);
		msleep(MMS300_RESET_DELAY);
		if (info->threewave_mode)
			mms_set_threewave_mode(info);

#ifdef TSP_SVIEW_COVER_MODE
		if((info->cover_mode_retry == COVER_CLOSED) || (info->cover_mode==COVER_CLOSED)){
			mms_set_sview_cover_mode(info);
			info->cover_mode_retry = 0;
		}
#endif

#ifdef TSP_GLOVE_MODE
		if (info->glove_mode)
			mms_set_glove_mode(info);
#endif

		if (info->ta_status) {
			dev_notice(&info->client->dev, "TA connect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
		} else {
			dev_notice(&info->client->dev, "TA disconnect!!!\n");
			i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
		}
		info->enabled = true;
		mms_set_noise_mode(info);

		enable_irq(info->irq);
	}

	info->input_closed = false;

	return 0;
//TEST
//	return mms_ts_resume(&info->client->dev);
}
static void mms_ts_input_close(struct input_dev *dev)
{
	struct mms_ts_info *info;

	info = input_get_drvdata(dev);

	if (info->input_closed) {
		dev_info(&info->client->dev, "%s: already closed\n", __func__);
		return;
	}
	info->input_closed = true;

	dev_info(&info->client->dev, "%s\n", __func__);


#ifdef TSP_GESTURE_MODE
	if(info->lowpower_mode){
		dev_info(&info->client->dev, "lowpoer mode enter, f:%d, m:%d\n",info->lowpower_flag,info->lowpower_mode );
		enable_irq_wake(info->client->irq);
		lpm_mode_enter(info,1);
	//	info->enabled = false;
	}else
#endif
	{
		info->enabled = false;
		disable_irq_nosync(info->irq);
		usleep(2000);	// for all irq clear

		touch_is_pressed = 0;
		release_all_fingers(info);

		info->power(info, 0);
	// mms_pinctrl_configure(info, false, false);
	}

#ifdef CONFIG_INPUT_BOOSTER
	dev_info(&info->client->dev, "%s force dvfs off\n", __func__);
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, -1);
#endif

	return;
//TEST
//	mms_ts_suspend(&info->client->dev);
}
#endif

#ifdef USE_TSP_TA_CALLBACKS
void melfas_register_callback(struct tsp_callbacks *cb)
{
	charger_callbacks = cb;
	pr_info("%s\n", __func__);
}
#endif

//extern int get_samsung_lcd_attached(void);  // temp2

static int mms_ts_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct melfas_tsi_platform_data *pdata;
	struct mms_ts_info *info;
	struct input_dev *input_dev;
	int ret = 0;
	int error;
	unsigned char data[3] = {0, };
#ifdef SEC_TSP_FACTORY_TEST
	int i;
	struct device *fac_dev_ts;
#endif
#ifdef TOUCHKEY
	struct device *touchkey_dev;
#endif
	touch_is_pressed = 0;

#if defined(CONFIG_SEC_A5_EUR_PROJECT) || defined(CONFIG_SEC_E7_EUR_PROJECT)
	if( system_rev == 1 ){
		irq_bit_mask = 0x40;
	}
	else
#endif
		irq_bit_mask = 0x80;

	pr_err("%s+\n", __func__);


	/*if (get_samsung_lcd_attached() == 0) {
               dev_err(&client->dev, "%s : get_samsung_lcd_attached()=0 \n", __func__);
                return -EIO;
        }*/

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;
	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct melfas_tsi_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		error = mms_parse_dt(&client->dev, pdata);
		if (error)
			return error;

#ifdef USE_TSP_TA_CALLBACKS
		pdata->register_cb = melfas_register_callback;
#endif


	} else
		pdata = client->dev.platform_data;

	if (!pdata)
		return -EINVAL;
	melfas_request_gpio(pdata);
        gpio_direction_input(pdata->gpio_int);
	info = kzalloc(sizeof(struct mms_ts_info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}
	ts = info;

	info->client = client;
	info->pdata = pdata;

	/* Get pinctrl if target uses pinctrl */
	info->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(info->pinctrl)) {
		if (PTR_ERR(info->pinctrl) == -EPROBE_DEFER)
			goto err_input_alloc;

		pr_err("%s: Target does not use pinctrl\n", __func__);
		info->pinctrl = NULL;
	}

	if (info->pinctrl) {
		ret = mms_pinctrl_configure(info, 0);
		if (ret)
			pr_err("%s: cannot set pinctrl state\n", __func__);
	}


	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev, "Failed to allocate memory for input device\n");
		ret = -ENOMEM;
		goto err_input_alloc;
	}
	info->input_dev = input_dev;
#ifdef TOUCHKEY
	if(pdata->tkey_led_en >= 0)
		info->keyled = key_led_control;
#endif
	if (NULL == info->pdata) {
		pr_err("failed to get platform data\n");
		goto err_config;
	}
	info->irq = -1;
	mutex_init(&info->lock);

	info->ta_status = 0;
	info->max_x = info->pdata->max_x;
	info->max_y = info->pdata->max_y;
	info->invert_x = info->pdata->invert_x;
	info->invert_y = info->pdata->invert_y;
	info->input_event = info->pdata->input_event;

#ifdef USE_TSP_TA_CALLBACKS
	info->register_cb = info->pdata->register_cb;
#endif

	info->threewave_mode = false;
#ifdef TSP_GLOVE_MODE
	info->glove_mode = false;
#endif
	info->power = melfas_power;
#ifdef TOUCHKEY
	info->keycode[0] = 0;
	info->keycode[1] = pdata->key1;
	info->keycode[2] = KEY_BACK;

	for (i = 0; i < 3; i++)
		info->touchkey[i] = 0;

	info->led_cmd = false;
	if(info->keyled)
		info->keyled(info, 0);
	info->menu_s = 0;
	info->back_s = 0;
#endif

	i2c_set_clientdata(client, info);
	if (info->power == NULL) {
		dev_err(&client->dev,
			"missing power control\n");
		goto err_config;
	} else {
		info->power(info, 1);
		msleep(MMS300_RESET_DELAY);
	}
	info->input_closed = false;

	info->panel = info->pdata->panel;

	printk("%s: [TSP] system_rev = %d\n", __func__, system_rev);

	ret = mms_ts_fw_check(info);
	if (ret) {
		dev_err(&client->dev,
			"failed to initialize (%d)\n", ret);
		goto err_reg_input_dev;
	}


#ifdef USE_TSP_TA_CALLBACKS
	info->callbacks.inform_charger = melfas_ta_cb;
	if (info->register_cb)
		info->register_cb(&info->callbacks);
#endif

	/* MMS345, MMS345L screen definition is not same. check firmware value */
	if (!pdata->max_x && !pdata->max_y) {
		data[0] = i2c_smbus_read_byte_data(client, MMS_XYRES_HI);
		dev_info(&client->dev, "%s: MMS_XYRES_HI:%X\n", __func__, data[0]);

		data[1] = i2c_smbus_read_byte_data(client, MMS_XRES_LO);
		dev_info(&client->dev, "%s: MMS_XRES_LO:%X\n", __func__, data[1]);

		data[2] = i2c_smbus_read_byte_data(client, MMS_YRES_LO);
		dev_info(&client->dev, "%s: MMS_YRES_LO:%X\n", __func__, data[2]);

		if (data[0] || data[1] || data[2]) {
			info->max_x = ((data[0] & 0x0F) << 8) | (data[1] & 0xFF);
			info->max_y = ((data[0] >> 4) << 8) | (data[2] & 0xFF);

			dev_info(&client->dev, "%s: max_x = %d, max_y = %d\n",
					__func__, info->max_x, info->max_y);
		}
	}
	snprintf(info->phys, sizeof(info->phys),
		 "%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchscreen";
	input_dev->phys = info->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

#ifdef TOUCHKEY
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(pdata->key1, input_dev->keybit);
	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);
#endif
#ifdef TSP_GESTURE_MODE
	set_bit(KEY_BLACK_UI_GESTURE, info->input_dev->keybit);
	info->lowpower_mode = 0;
	info->lowpower_flag = 0;
#endif

	input_mt_init_slots(input_dev, MAX_FINGERS, INPUT_MT_DIRECT);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
				0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
				0, MAX_PRESSURE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				0, (info->max_x)-1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				0, (info->max_y)-1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR,
				0, MAX_WIDTH, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PALM,
				0, 1, 0, 0);

#ifdef USE_OPEN_CLOSE
	input_dev->open = mms_ts_input_open;
	input_dev->close = mms_ts_input_close;
#endif
	input_set_drvdata(input_dev, info);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "failed to register input dev (%d)\n",
			ret);
		goto err_reg_input_dev;
	}

#ifdef CONFIG_INPUT_BOOSTER
	info->booster = input_booster_allocate(INPUT_BOOSTER_ID_TSP);
	if (!info->booster) {
		dev_err(&client->dev, "%s: Error, failed to allocate input booster\n",__func__);
		goto error_alloc_booster_failed;
	}
#endif
	info->enabled = true;

	client->irq = gpio_to_irq(pdata->gpio_int);
	ret = request_threaded_irq(client->irq, NULL, mms_ts_interrupt,
                                     IRQF_TRIGGER_LOW  | IRQF_ONESHOT,
				   MELFAS_TS_NAME, info);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}
	info->irq = client->irq;

#ifdef CONFIG_HAS_EARLYSUSPEND
	info->early_suspend.level = EARLY_SUSPEND_LEVEL_STOP_DRAWING;
	info->early_suspend.suspend = mms_ts_early_suspend;
	info->early_suspend.resume = mms_ts_late_resume;
	register_early_suspend(&info->early_suspend);
#endif

/* START - Added to support API's for TSP tuning */
	if (alloc_chrdev_region(&info->mms_dev, 0, 1, "mms_ts")) {
		dev_err(&client->dev, "failed to allocate device region\n");
		return -ENOMEM;
	}

	cdev_init(&info->cdev, &mms_fops);
	info->cdev.owner = THIS_MODULE;

	if (cdev_add(&info->cdev, info->mms_dev, 1)) {
		dev_err(&client->dev, "failed to add ch dev\n");
		return -EIO;
	}

	info->class = class_create(THIS_MODULE, "mms_ts");
	device_create(info->class, NULL, info->mms_dev, NULL, "mms_ts");
/* END - Added to support API's for TSP tuning */

	sec_touchscreen = device_create(sec_class,
					NULL, 0, info, "sec_touchscreen");
	if (IS_ERR(sec_touchscreen)) {
		dev_err(&client->dev,
			"Failed to create device for the sysfs1\n");
		ret = -ENODEV;
	}

#if DEBUG_PRINT2
	if (sysfs_create_group(&sec_touchscreen->kobj, &mms_attr_group)) {
		dev_err(&client->dev, "failed to create sysfs group, debug2 \n");
		return -EAGAIN;
	}
	if (sysfs_create_link(NULL, &sec_touchscreen->kobj, "sec_touchscreen")) {
		dev_err(&client->dev, "failed to create sysfs symlink, debug2 \n");
		return -EAGAIN;
	}

#endif

	#if 0
	ret = sysfs_create_file(&info->input_dev->dev.kobj,
			&attrs[0].attr);
	if (ret < 0) {
		dev_err(&client->dev,
				"%s: Failed to create sysfs attributes\n",
				__func__);
	}
	#endif
#ifdef TOUCHKEY
	touchkey_dev = device_create(sec_class,
		NULL, 0, info, "sec_touchkey");

	if (IS_ERR(touchkey_dev))
		dev_err(&client->dev,
		"Failed to create device for the touchkey sysfs\n");

	ret = sysfs_create_group(&touchkey_dev->kobj,
		&sec_touchkey_attr_group);
	if (ret)
		dev_err(&client->dev, "Failed to create sysfs group\n");

	if(info->keyled){
		ret = sysfs_create_group(&touchkey_dev->kobj,
			&sec_touchkeyled_attr_group);
		if (ret)
			dev_err(&client->dev, "Failed to create sysfs group\n");
	}

#endif

#ifdef SEC_TSP_FACTORY_TEST
	INIT_LIST_HEAD(&info->cmd_list_head);
	for (i = 0; i < ARRAY_SIZE(tsp_cmds); i++)
		list_add_tail(&tsp_cmds[i].list, &info->cmd_list_head);

	mutex_init(&info->cmd_lock);
	info->cmd_is_running = false;

	fac_dev_ts = device_create(sec_class,
		NULL, 0, info, "tsp");
	if (IS_ERR(fac_dev_ts))
		dev_err(&client->dev,
		"Failed to create device for the tsp sysfs\n");

	ret = sysfs_create_group(&fac_dev_ts->kobj,
		 &sec_touch_factory_attr_group);
	if (ret)
		dev_err(&client->dev, "Failed to create sysfs group\n");

#endif

	ret = sysfs_create_link(&fac_dev_ts->kobj,
		&info->input_dev->dev.kobj, "input");
	if (ret < 0) {
		dev_err(&info->client->dev,
			"%s: Failed to create input symbolic link\n",
			__func__);
	}


#ifdef TSP_RAWDATA_DUMP
		INIT_DELAYED_WORK(&info->ghost_check, ghost_touch_check);
		p_ghost_check = &info->ghost_check;
#endif


	return 0;

err_req_irq:
	input_unregister_device(input_dev);

#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(info->booster);
	info->booster = NULL;
error_alloc_booster_failed:
#endif

err_reg_input_dev:
	info->power(info,0);
#ifdef TOUCHKEY
	if(info->keyled)
		info->keyled(info, 0);
#endif
err_config:
	input_free_device(input_dev);
	/*input_dev = NULL;*/
err_input_alloc:
	kfree(info);
err_alloc:
	return ret;

}

#ifdef TSP_RAWDATA_DUMP
static void ghost_touch_check(struct work_struct *work)
{
	struct mms_ts_info *info = container_of(work, struct mms_ts_info,
						ghost_check.work);
	int i;
	static int lock;

	if(lock==1)
		dev_err(&info->client->dev, "%s, ## checking.. ignored.\n", __func__);

	lock =1;	
	for(i=0; i<5; i++){		
		dev_err(&info->client->dev, "%s, ## run Intensity data ##, %d\n", __func__, __LINE__);
		run_intensity_read((void *)info);
		msleep(100);
		
	}
	dev_err(&info->client->dev, "%s, ## Done ##, %d\n", __func__, __LINE__);

	lock = 0;
	
}

void tsp_dump(void)
{
	printk(KERN_ERR "FTS %s: start \n", __func__);
	schedule_delayed_work(p_ghost_check, msecs_to_jiffies(100));
}
#else
void tsp_dump(void)
{
	printk(KERN_ERR "FTS %s: not support\n", __func__);
}
#endif


static int mms_ts_remove(struct i2c_client *client)
{
	struct mms_ts_info *info = i2c_get_clientdata(client);

	if (info->enabled)
		info->power(info,0);

#if DEBUG_PRINT2
	sysfs_remove_link(NULL, "sec_touchscreen");
	sysfs_remove_group(&client->dev.kobj, &mms_attr_group);
#endif

#ifdef TOUCHKEY
	if (info->led_cmd)
		if(info->keyled)
			info->keyled(info, 0);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&info->early_suspend);
#endif
	if (info->irq >= 0)
		free_irq(info->irq, info);
	input_unregister_device(info->input_dev);
	kfree(info);

	return 0;
}

static void mms_ts_shutdown(struct i2c_client *client)
{
	struct mms_ts_info *info = i2c_get_clientdata(client);

	if (info->irq >= 0){
		free_irq(info->irq, info);
	}	
	if (info->enabled)
		info->power(info,0);
#ifdef TOUCHKEY
	if (info->led_cmd)
		if(info->keyled)
			info->keyled(info, 0);
#endif
}

#if !defined(USE_OPEN_CLOSE)
static int mms_ts_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

	mutex_lock(&info->lock);
	if (!info->enabled)
		goto out;
	if (!info->input_dev->users)
		goto out;

	info->enabled = false;
	disable_irq_nosync(info->irq);

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);

	touch_is_pressed = 0;
	release_all_fingers(info);
	info->power(info,0);
	info->sleep_wakeup_ta_check = info->ta_status;
#ifdef TOUCHKEY
	if (info->led_cmd == true) {
		if(info->keyled)
			info->keyled(info, 0);
		info->led_cmd = false;
	}
#endif
	/* This delay needs to prevent unstable POR by
	rapid frequently pressing of PWR key. */
	msleep(50);

out:
	mutex_unlock(&info->lock);
	return 0;
}

static int mms_ts_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);

	if (info->enabled)
		return 0;

	if (!info->input_dev->users)
		return 0;

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);
	info->power(info, 1);
	msleep(MMS300_RESET_DELAY);

	if (info->threewave_mode)
		mms_set_threewave_mode(info);

#ifdef TSP_GLOVE_MODE
	if (info->glove_mode)
		mms_set_glove_mode(info);
#endif

	if (info->ta_status) {
		dev_notice(&client->dev, "TA connect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x1);
	} else {
		dev_notice(&client->dev, "TA disconnect!!!\n");
		i2c_smbus_write_byte_data(info->client, 0x33, 0x2);
	}
	info->enabled = true;
	mms_set_noise_mode(info);

	/* Because irq_type by EXT_INTxCON register is changed to low_level
	 *  after wakeup, irq_type set to falling edge interrupt again.
	 */
	enable_irq(info->irq);

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mms_ts_early_suspend(struct early_suspend *h)
{
	struct mms_ts_info *info;
	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_suspend(&info->client->dev);

}

static void mms_ts_late_resume(struct early_suspend *h)
{
	struct mms_ts_info *info;
	info = container_of(h, struct mms_ts_info, early_suspend);
	mms_ts_resume(&info->client->dev);
}
#endif

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND) && !defined(USE_OPEN_CLOSE)
static const struct dev_pm_ops mms_ts_pm_ops = {
	.suspend = mms_ts_suspend,
	.resume = mms_ts_resume,
#ifdef CONFIG_HIBERNATION
	.freeze = mms_ts_suspend,
	.thaw = mms_ts_resume,
	.restore = mms_ts_resume,
#endif
};
#endif

static const struct i2c_device_id mms_ts_id[] = {
	{MELFAS_TS_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, mms_ts_id);

#ifdef CONFIG_OF
static struct of_device_id mms_match_table[] = {
	{ .compatible = "melfas,mms300-ts",},
	{ },
};
#else
#define mms_match_table NULL
#endif
static struct i2c_driver mms_ts_driver = {
	.probe = mms_ts_probe,
	.remove = mms_ts_remove,
	.shutdown = mms_ts_shutdown,
	.driver = {
		   .name = MELFAS_TS_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = mms_match_table,
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND) && !defined(USE_OPEN_CLOSE)
		   .pm = &mms_ts_pm_ops,
#endif
	},
	.id_table = mms_ts_id,
};

static int mms_ts_init(void)
{
	pr_err("%s\n", __func__);
	
	#ifdef CONFIG_SAMSUNG_LPM_MODE
	if (poweroff_charging) {
		pr_notice("%s : LPM Charging Mode!!\n", __func__);
		return 0;
	}
	#endif
	return i2c_add_driver(&mms_ts_driver);

}

static void __exit mms_ts_exit(void)
{
	i2c_del_driver(&mms_ts_driver);
}

module_init(mms_ts_init);
module_exit(mms_ts_exit);

/* Module information */
MODULE_DESCRIPTION("Touchscreen driver for Melfas MMS-series controllers");
MODULE_LICENSE("GPL");
