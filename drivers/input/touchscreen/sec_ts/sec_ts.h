/* drivers/input/touchscreen/sec_ts.h
 *
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
 * http://www.samsungsemi.com/
 *
 * Core file for Samsung TSC driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __SEC_TS_H__
#define __SEC_TS_H__

#ifdef CONFIG_SEC_DEBUG_TSP_LOG
#include <linux/sec_debug.h>
#endif
#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif
#include <linux/completion.h>
#include <linux/wakelock.h>
#include <linux/input/sec_cmd.h>

#define SEC_TS_I2C_NAME "sec_ts"
#define SEC_TS_DEVICE_NAME "SEC_TS"

#ifdef CONFIG_SEC_DEBUG_TSP_LOG
#define tsp_debug_dbg(mode, dev, fmt, ...)		\
({							\
	if (mode) {					\
		dev_dbg(dev, fmt, ## __VA_ARGS__);	\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);	\
	}						\
	else						\
		dev_dbg(dev, fmt, ## __VA_ARGS__);	\
})

#define tsp_debug_info(mode, dev, fmt, ...)		\
({							\
	if (mode) {					\
		dev_info(dev, fmt, ## __VA_ARGS__);	\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);	\
	}						\
	else						\
		dev_info(dev, fmt, ## __VA_ARGS__);	\
})

#define tsp_debug_err(mode, dev, fmt, ...)		\
({							\
	if (mode) {					\
		dev_err(dev, fmt, ## __VA_ARGS__);	\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);	\
	}						\
	else						\
		dev_err(dev, fmt, ## __VA_ARGS__);	\
})
#else
#define tsp_debug_dbg(mode, dev, fmt, ...)	dev_dbg(dev, fmt, ## __VA_ARGS__)
#define tsp_debug_info(mode, dev, fmt, ...)	dev_info(dev, fmt, ## __VA_ARGS__)
#define tsp_debug_err(mode, dev, fmt, ...)	dev_err(dev, fmt, ## __VA_ARGS__)
#endif

#define USE_OPEN_CLOSE

#ifdef USE_OPEN_DWORK
#define TOUCH_OPEN_DWORK_TIME		10
#endif

#define MASK_1_BITS					0x0001
#define MASK_2_BITS					0x0003
#define MASK_3_BITS					0x0007
#define MASK_4_BITS					0x000F
#define MASK_5_BITS					0x001F
#define MASK_6_BITS					0x003F
#define MASK_7_BITS					0x007F
#define MASK_8_BITS					0x00FF

/* support feature */
#undef SEC_TS_SUPPORT_HOVERING
#undef SEC_TS_SUPPORT_TA_MODE
#define SEC_TS_SUPPORT_STRINGLIB

#define TYPE_STATUS_EVENT_ACK		1
#define TYPE_STATUS_EVENT_ERR		2
#define TYPE_STATUS_EVENT_INFO		3
#define TYPE_STATUS_EVENT_GEST		6

/* SEC_TS_ACK : acknowledge event */
#define SEC_TS_ACK_OFFSET_CAL_DONE	0x01
#define SEC_TS_ACK_SELF_TEST_DONE	0x0A
#define SEC_TS_ACK_BOOT_COMPLETE	0x0C

#define BIT_STATUS_EVENT_ACK(a)		(a << TYPE_STATUS_EVENT_ACK)
#define BIT_STATUS_EVENT_ERR(a)		(a << TYPE_STATUS_EVENT_ERR)
#define BIT_STATUS_EVENT_INFO(a)	(a << TYPE_STATUS_EVENT_INFO)
#define BIT_STATUS_EVENT_GEST(a)	(a << TYPE_STATUS_EVENT_GEST)

#define DO_FW_CHECKSUM			(1 << 0)
#define DO_PARA_CHECKSUM		(1 << 1)
#define MAX_SUPPORT_TOUCH_COUNT		10
#define MAX_SUPPORT_HOVER_COUNT		1

#define SEC_TS_EVENTID_HOVER		10

#define SEC_TS_DEFAULT_FW_NAME		"tsp_sec/s6smc44_img_REL_NA70.bin"
#define SEC_TS_DEFAULT_BL_NAME		"tsp_sec/s6smc44_blupdate_img_REL_NA70.bin"
#define SEC_TS_DEFAULT_PARA_NAME	"tsp_sec/s6smc44_para_REL_NA70.bin"
#define SEC_TS_DEFAULT_UMS_FW		"/sdcard/lsi.bin"
#define SEC_TS_DEFAULT_FFU_FW		"ffu_tsp.bin"
#define SEC_TS_MAX_FW_PATH		64
#define SEC_TS_SELFTEST_REPORT_SIZE	80

#define I2C_WRITE_BUFFER_SIZE		7

#define SEC_TS_FW_HEADER_SIGN		0x53494654
#define SEC_TS_FW_CHUNK_SIGN		0x53434654

#define SEC_TS_FW_UPDATE_ON_PROBE

#define AMBIENT_CAL			0
#define OFFSET_CAL			1

/* SEC_TS READ REGISTER ADDRESS */
#define SEC_TS_READ_FW_STATUS		0x51
#define SEC_TS_READ_DEVICE_ID		0x52
#define SEC_TS_READ_SUB_ID		0x53
#define SEC_TS_READ_BOOT_STATUS		0x55
#define SEC_TS_READ_RAW_CHANNEL		0x58
#define SEC_TS_READ_FLASH_ERASE_STATUS	0x59
#define SEC_TS_READ_SET_TOUCHFUNCTION	0x64
#define SEC_TS_READ_THRESHOLD		0x6D
#define SEC_TS_READ_TS_STATUS		0x70
#define SEC_TS_READ_ONE_EVENT		0x71
#define SEC_TS_READ_CALIBRATION_REPORT	0x73
#define SEC_TS_READ_TOUCH_RAWDATA	0x76
#define SEC_TS_READ_TOUCH_SELF_RAWDATA	0x77
#define SEC_TS_READ_FW_INFO		0xA2
#define SEC_TS_READ_FW_VERSION		0xA3
#define SEC_TS_READ_PARA_VERSION	0xA4
#define SEC_TS_READ_IMG_VERSION	0xA5
#define SEC_TS_READ_LV3			0xD2
#define SEC_TS_READ_BL_UPDATE_STATUS	0xDB
#define SEC_TS_READ_SELFTEST_RESULT   0x80
#define SEC_TS_READ_NVM   0x85

/* SEC_TS WRITE COMMAND */
#define SEC_TS_CMD_SENSE_ON		0x40
#define SEC_TS_CMD_SENSE_OFF		0x41
#define SEC_TS_CMD_SW_RESET		0x42
#define SEC_TS_CMD_CALIBRATION_AMBIENT	0x43
#define SEC_TS_CMD_ERASE_FLASH		0x45
#define SEC_TS_CMD_STATEMANAGE_ON	0x48
#define SEC_TS_CMD_CALIBRATION_OFFSET	0x4C
#define SEC_TS_CMD_WRITE_FW_BLK		0x53
#define SEC_TS_CMD_WRITE_FW_SHORT	0x54
#define SEC_TS_CMD_ENTER_FW_MODE	0x57
#define SEC_TS_CMD_WRITE_FW_LONG	0x5A
#define SEC_TS_CMD_CLEAR_EVENT_STACK	0x60
#define SEC_TS_CMD_SET_TOUCHFUNCTION	0x63
#define SEC_TS_CMD_SET_POWER_MODE	0x65
#define SEC_TS_CMD_STATUS_EVENT_TYPE	0x6B
#define SEC_TS_CMD_GESTURE_MODE		0x6C
#define SEC_TS_CMD_NOISE_MODE		0x77
#define SEC_TS_CMD_GET_CHECKSUM		0xA6
#define SEC_TS_CMD_CHG_SYSMODE		0xD7
#define SEC_TS_CMD_SELFTEST   0x51
#define SEC_TS_CMD_NVM    0x85

/* SEC_TS FACTORY OPCODE COMMAND */
#define SEC_TS_OPCODE_MUTUAL_DATA_TYPE	0xF4
#define SEC_TS_OPCODE_SELF_DATA_TYPE	0xFA

/* SEC_TS FLASH COMMAND */
#define SEC_TS_CMD_FLASH_READ_ADDR	0xD0
#define SEC_TS_CMD_FLASH_READ_SIZE	0xD1
#define SEC_TS_CMD_FLASH_READ_DATA	0xD2

#define SEC_TS_CMD_FLASH_ERASE		0xD8
#define SEC_TS_CMD_FLASH_WRITE		0xD9
#define SEC_TS_CMD_FLASH_PADDING	0xDA

#define SEC_TS_FLASH_SIZE_64	64
#define SEC_TS_FLASH_SIZE_128	128
#define SEC_TS_FLASH_SIZE_256	256

#define SEC_TS_FLASH_SIZE_CMD	1
#define SEC_TS_FLASH_SIZE_ADDR	2
#define SEC_TS_FLASH_SIZE_CHECKSUM	1

#define SEC_TS_STATUS_BOOT_MODE		0x10
#define SEC_TS_STATUS_APP_MODE		0x20

#define SEC_TS_FIRMWARE_PAGE_SIZE_256	256
#define SEC_TS_FIRMWARE_PAGE_SIZE_128	128

/* SEC status event id */
#define SEC_TS_STATUS_EVENT		0
#define SEC_TS_COORDINATE_EVENT		1
#define SEC_TS_GESTURE_EVENT		2

#define SEC_TS_SID_GESTURE		0x14
#define SEC_TS_GESTURE_CODE_AOD		0x00
#define SEC_TS_GESTURE_CODE_SPAY	0x0A

#define SEC_TS_EVENT_BUFF_SIZE		8

#define SEC_TS_COORDINATE_ACTION_NONE		0
#define SEC_TS_COORDINATE_ACTION_PRESS		1
#define SEC_TS_COORDINATE_ACTION_MOVE		2
#define SEC_TS_COORDINATE_ACTION_RELEASE	3
#define SEC_TS_COORDINATE_ACTION_UNKNOWN	4

#define SEC_TS_TOUCHTYPE_NORMAL		0
#define SEC_TS_TOUCHTYPE_PROXIMITY	1
#define SEC_TS_TOUCHTYPE_FLIPCOVER	2
#define SEC_TS_TOUCHTYPE_GLOVE		3
#define SEC_TS_TOUCHTYPE_STYLUS		4
#define SEC_TS_TOUCHTYPE_HOVER		5
#define SEC_TS_TOUCHTYPE_PALM		6

#define SEC_TS_BIT_SETFUNC_TOUCH	(1 << 0)
#define SEC_TS_BIT_SETFUNC_MUTUAL	(1 << 0)
#define SEC_TS_BIT_SETFUNC_HOVER	(1 << 1)
#define SEC_TS_BIT_SETFUNC_CLEARCOVER	(1 << 2)
#define SEC_TS_BIT_SETFUNC_GLOVE	(1 << 3)
#define SEC_TS_BIT_SETFUNC_CHARGER	(1 << 4)
#define SEC_TS_BIT_SETFUNC_STYLUS	(1 << 5)
#define SEC_TS_BIT_SETFUNC_QWERTY_KEYBOARD	(1 << 7)

#define STATE_MANAGE_ON			1
#define STATE_MANAGE_OFF		0

#define SEC_TS_STATUS_CALIBRATION	0xA1
#define SEC_TS_STATUS_NOT_CALIBRATION	0x50

typedef enum {
	SEC_TS_STATE_POWER_OFF = 0,
	SEC_TS_STATE_LPM_SUSPEND,
	SEC_TS_STATE_LPM_RESUME,
	SEC_TS_STATE_POWER_ON
} TOUCH_POWER_MODE;

typedef enum {
	TOUCH_SYSTEM_MODE_BOOT		= 0,
	TOUCH_SYSTEM_MODE_CALIBRATION	= 1,
	TOUCH_SYSTEM_MODE_TOUCH		= 2,
	TOUCH_SYSTEM_MODE_SELFTEST	= 3,
	TOUCH_SYSTEM_MODE_FLASH		= 4,
	TOUCH_SYSTEM_MODE_LOWPOWER	= 5,
	TOUCH_SYSTEM_MODE_LISTEN
} TOUCH_SYSTEM_MODE;

typedef enum {
	TOUCH_MODE_STATE_IDLE		= 0,
	TOUCH_MODE_STATE_HOVER		= 1,
	TOUCH_MODE_STATE_TOUCH		= 2,
	TOUCH_MODE_STATE_NOISY		= 3,
	TOUCH_MODE_STATE_CAL		= 4,
	TOUCH_MODE_STATE_CAL2		= 5,
	TOUCH_MODE_STATE_WAKEUP		= 10
} TOUCH_MODE_STATE;

enum switch_system_mode {
	TO_TOUCH_MODE			= 0,
	TO_LOWPOWER_MODE		= 1,
	TO_SELFTEST_MODE		= 2,
	TO_FLASH_MODE			= 3,
};

#define CMD_RESULT_WORD_LEN		10

#define SEC_TS_I2C_RETRY_CNT		10
#define SEC_TS_WAIT_RETRY_CNT		100

#undef SMARTCOVER_COVER		/* for  Various Cover */

#ifdef SMARTCOVER_COVER
#define MAX_W				16	/* zero is 16 x 28 */
#define MAX_H				32	/* byte size to IC */
#define MAX_TX MAX_W
#define MAX_BYTE MAX_H
#endif

#define SEC_TS_LOWP_FLAG_QUICK_CAM		(1 << 0)
#define SEC_TS_LOWP_FLAG_2ND_SCREEN		(1 << 1)
#define SEC_TS_LOWP_FLAG_BLACK_UI		(1 << 2)
#define SEC_TS_LOWP_FLAG_QUICK_APP_ACCESS	(1 << 3)
#define SEC_TS_LOWP_FLAG_DIRECT_INDICATOR	(1 << 4)
#define SEC_TS_LOWP_FLAG_SPAY			(1 << 5)
#define SEC_TS_LOWP_FLAG_TEMP_CMD		(1 << 6)

#define SEC_TS_MODE_QUICK_SHOT			(1 << 0)
#define SEC_TS_MODE_SCRUB			(1 << 1)
#define SEC_TS_MODE_SPAY			(1 << 1)
#define SEC_TS_MODE_QUICK_APP_ACCESS		(1 << 2)
#define SEC_TS_MODE_DIRECT_INDICATOR		(1 << 3)

#define SEC_TS_MODE_LSI_DOUBLE_TAP		(1 << 0)
#define SEC_TS_MODE_LSI_SPAY			(1 << 1)

enum sec_ts_cover_id {
	SEC_TS_FLIP_WALLET = 0,
	SEC_TS_VIEW_COVER,
	SEC_TS_COVER_NOTHING1,
	SEC_TS_VIEW_WIRELESS,
	SEC_TS_COVER_NOTHING2,
	SEC_TS_CHARGER_COVER,
	SEC_TS_VIEW_WALLET,
	SEC_TS_LED_COVER,
	SEC_TS_CLEAR_FLIP_COVER,
	SEC_TS_QWERTY_KEYBOARD_EUR,
	SEC_TS_QWERTY_KEYBOARD_KOR,
	SEC_TS_MONTBLANC_COVER = 100,
};

#ifdef SEC_TS_SUPPORT_TA_MODE
extern struct sec_ts_callbacks *charger_callbacks;
struct sec_ts_callbacks {
	void (*inform_charger)(struct sec_ts_callbacks *, int type);
};
#endif

#define TEST_MODE_MIN_MAX		false
#define TEST_MODE_ALL_NODE		true
#define TEST_MODE_READ_FRAME		false
#define TEST_MODE_READ_CHANNEL		true

/* factory test mode */
struct sec_ts_test_mode {
	u8 type;
	short min;
	short max;
	bool allnode;
	bool frame_channel;
};

struct sec_ts_fw_file {
	u8 *data;
	u32 pos;
	size_t size;
};

/* ----------------------------------------
 * write 0xE4 [ 11 | 10 | 01 | 00 ]
 * MSB <-------------------> LSB
 * read 0xE4
 * mapping sequnce : LSB -> MSB
 * struct sec_ts_test_result {
 * * assy : front + OCTA assay
 * * module : only OCTA
 *	 union {
 *		 struct {
 *			 u8 assy_count:2;		-> 00
 *			 u8 assy_result:2;		-> 01
 *			 u8 module_count:2;	-> 10
 *			 u8 module_result:2;	-> 11
 *		 } __attribute__ ((packed));
 *		 unsigned char data[1];
 *	 };
 *};
 * ---------------------------------------- */
struct sec_ts_test_result {
	union {
		struct {
			u8 assy_count:2;
			u8 assy_result:2;
			u8 module_count:2;
			u8 module_result:2;
		} __attribute__ ((packed));
		unsigned char data[1];
	};
};

/* 8 byte */
struct sec_ts_gesture_status {
	u8 stype:6;
	u8 eid:2;
	u8 scode;
	u8 gesture;
	u8 y_4_2:3;
	u8 x:5;
	u8 h_4:1;
	u8 w:5;
	u8 y_1_0:2;
	u8 reserved:4;
	u8 h_3_0:4;
	u8 reserved2;
	u8 reserved3;
} __attribute__ ((packed));

/* 8 byte */
struct sec_ts_event_status {
	u8 tchsta:3;
	u8 ttype:3;
	u8 eid:2;
	u8 sid;
	u8 buff2;
	u8 buff3;
	u8 buff4;
	u8 buff5;
	u8 buff6;
	u8 buff7;
} __attribute__ ((packed));

/* 8 byte */
struct sec_ts_event_coordinate {
	u8 tchsta:3;
	u8 ttype:3;
	u8 eid:2;
	u8 tid:4;
	u8 nt:4;
	u8 x_11_4;
	u8 y_11_4;
	u8 y_3_0:4;
	u8 x_3_0:4;
	u8 z;
	u8 major;
	u8 minor;
} __attribute__ ((packed));

/* not fixed */
struct sec_ts_coordinate {
	u8 id;
	u8 ttype;
	u8 action;
	u16 x;
	u16 y;
	u8 touch_width;
	u8 hover_flag;
	u8 glove_flag;
	u8 touch_height;
	u16 mcount;
	u8 major;
	u8 minor;
	bool palm;
};

struct sec_ts_data {
	u32 isr_pin;

	u32 crc_addr;
	u32 fw_addr;
	u32 para_addr;
	u8 boot_ver[3];

	struct device *dev;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct sec_ts_plat_data *plat_data;
	struct sec_ts_coordinate coord[MAX_SUPPORT_TOUCH_COUNT + MAX_SUPPORT_HOVER_COUNT];

	uint32_t flags;
	unsigned char lowpower_flag;
	bool lowpower_mode;

	int touch_count;
	int tx_count;
	int rx_count;
	int i2c_burstmax;
	int ta_status;
	int power_status;
	int raw_status;
	int touchkey_glove_mode_status;
	u8 glove_enables;
#ifdef SEC_TS_SUPPORT_HOVERING
	u8 hover_enables;
#endif
	struct sec_ts_event_coordinate touchtype;
	bool touched[11];
	u8 gesture_status[6];

#ifdef SEC_TS_SUPPORT_TA_MODE
	struct sec_ts_callbacks callbacks;
#endif
	struct mutex lock;
	struct mutex device_mutex;
	struct mutex i2c_mutex;

#ifdef USE_OPEN_DWORK
	struct delayed_work open_work;
#endif

	struct completion resume_done;
	struct wake_lock wakelock;

	struct sec_cmd_data sec;
	short *pFrame;

	bool reinit_done;
	bool flip_enable;
	unsigned int cover_type;

#ifdef FTS_SUPPORT_2NDSCREEN
	u8 SIDE_Flag;
	u8 previous_SIDE_value;
#endif

#ifdef SMARTCOVER_COVER
	bool smart_cover[MAX_BYTE][MAX_TX];
	bool changed_table[MAX_TX][MAX_BYTE];
	u8 send_table[MAX_TX][4];
#endif

	int tspid_val;
	int tspid2_val;

#ifdef SEC_TS_SUPPORT_STRINGLIB
	unsigned int scrub_id;
	unsigned int scrub_x;
	unsigned int scrub_y;
	unsigned short string_addr;
#endif

	int temp;
	int (*sec_ts_i2c_write)(struct sec_ts_data *ts, u8 reg, u8 *data, int len);
	int (*sec_ts_i2c_read)(struct sec_ts_data *ts, u8 reg, u8 *data, int len);
	int (*sec_ts_i2c_write_burst)(struct sec_ts_data *ts, u8 *data, int len);
	int (*sec_ts_i2c_read_bulk)(struct sec_ts_data *ts, u8 *data, int len);
#ifdef SEC_TS_SUPPORT_STRINGLIB
	int (*sec_ts_read_from_string)(struct sec_ts_data *ts, unsigned short *reg, unsigned char *data, int length);
	int (*sec_ts_write_to_string)(struct sec_ts_data *ts, unsigned short *reg, unsigned char *data, int length);
#endif

};

struct sec_ts_plat_data {
	int max_x;
	int max_y;
	int num_tx;
	int num_rx;
	unsigned gpio;
	int irq_type;
	int i2c_burstmax;
	int vdd_en_gpio;	// NA7
	int bringup;

	const char *firmware_name;
	const char *parameter_name;
	const char *model_name;
	const char *project_name;
	const char *regulator_dvdd;
	const char *regulator_avdd;

	u32 panel_revision;
	u8 img_version_of_ic[4];
	u8 img_version_of_bin[4];
	u8 para_version_of_ic[4];
	u8 para_version_of_bin[4];	
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_sleep;

	int (*power)(struct sec_ts_data *ts, bool on);
	void (*recovery_mode)(bool on);
	void (*enable_sync)(bool on);
#ifdef SEC_TS_SUPPORT_TA_MODE
	void (*register_cb)(struct sec_ts_callbacks *);
#endif
	unsigned tspid;
	unsigned tspid2;
};

int sec_ts_firmware_update_on_probe(struct sec_ts_data *ts);
int sec_ts_firmware_update_on_hidden_menu(struct sec_ts_data *ts, int update_type);
int sec_ts_glove_mode_enables(struct sec_ts_data *ts, int mode);
#ifdef SEC_TS_SUPPORT_HOVERING
int sec_ts_hover_enables(struct sec_ts_data *ts, int enables);
#endif
int sec_ts_set_cover_type(struct sec_ts_data *ts, bool enable);
int sec_ts_wait_for_ready(struct sec_ts_data *ts, unsigned int ack);
int sec_ts_function(int (*func_init)(void *device_data), void (*func_remove)(void));
int sec_ts_fn_init(struct sec_ts_data *ts);
int sec_ts_read_calibration_report(struct sec_ts_data *ts);
int sec_ts_execute_force_calibration(struct sec_ts_data *ts, int cal_mode);
void sec_ts_delay(unsigned int ms);

extern struct class *sec_class;

extern int get_lcd_attached(char *);
// extern bool tsp_init_done;	// NA7

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif

#endif
