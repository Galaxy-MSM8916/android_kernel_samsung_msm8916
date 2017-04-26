#ifndef _LINUX_FTS_TS_H_
#define _LINUX_FTS_TS_H_

#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/i2c/fts.h>
#include <linux/pinctrl/consumer.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#ifdef CONFIG_SEC_DEBUG_TSP_LOG
#include <linux/sec_debug.h>
#endif

#ifdef CONFIG_SEC_DEBUG_TSP_LOG
#define tsp_debug_dbg(mode, dev, fmt, ...)	\
({								\
	if (mode) {					\
		dev_dbg(dev, fmt, ## __VA_ARGS__);	\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);		\
	}				\
	else					\
		dev_dbg(dev, fmt, ## __VA_ARGS__);	\
})

#define tsp_debug_info(mode, dev, fmt, ...)	\
({								\
	if (mode) {							\
		dev_info(dev, fmt, ## __VA_ARGS__);		\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);		\
	}				\
	else					\
		dev_info(dev, fmt, ## __VA_ARGS__);	\
})

#define tsp_debug_err(mode, dev, fmt, ...)	\
({								\
	if (mode) {					\
		dev_err(dev, fmt, ## __VA_ARGS__);	\
		sec_debug_tsp_log(fmt, ## __VA_ARGS__);	\
	}				\
	else					\
		dev_err(dev, fmt, ## __VA_ARGS__); \
})
#else
#define tsp_debug_dbg(mode, dev, fmt, ...)	dev_dbg(dev, fmt, ## __VA_ARGS__)
#define tsp_debug_info(mode, dev, fmt, ...)	dev_info(dev, fmt, ## __VA_ARGS__)
#define tsp_debug_err(mode, dev, fmt, ...)	dev_err(dev, fmt, ## __VA_ARGS__)
#endif

#define USE_OPEN_CLOSE

#include <linux/input/input_booster.h>

#ifdef USE_OPEN_DWORK
#define TOUCH_OPEN_DWORK_TIME 10
#endif

#ifdef CONFIG_MACH_E7_CHN_CTC
#undef USE_WARKAROUND_CODE
#endif

#define FIRMWARE_IC					"fts_ic"

#define FTS_MAX_FW_PATH	64

#define FTS_TS_DRV_NAME			"fts_touch"
#define FTS_TS_DRV_VERSION		"0132"

#define STM_DEVICE_NAME	"STM"

#define FTS_ID0							0x39
#define FTS_ID1							0x80

#define FTS_FIFO_MAX					32
#define FTS_EVENT_SIZE				8

#define PRESSURE_MIN					0
#define PRESSURE_MAX				127
#define P70_PATCH_ADDR_START	0x00420000
#define FINGER_MAX						10
#define AREA_MIN							PRESSURE_MIN
#define AREA_MAX						PRESSURE_MAX

#define EVENTID_NO_EVENT					0x00
#define EVENTID_ENTER_POINTER				0x03
#define EVENTID_LEAVE_POINTER				0x04
#define EVENTID_MOTION_POINTER				0x05
#define EVENTID_HOVER_ENTER_POINTER			0x07
#define EVENTID_HOVER_LEAVE_POINTER			0x08
#define EVENTID_HOVER_MOTION_POINTER		0x09
#define EVENTID_PROXIMITY_IN				0x0B
#define EVENTID_PROXIMITY_OUT				0x0C
#define EVENTID_MSKEY						0x0E

#define EVENTID_ERROR						0x0F
#define EVENTID_CONTROLLER_READY			0x10
#define EVENTID_SLEEPOUT_CONTROLLER_READY	0x11
#define EVENTID_RESULT_READ_REGISTER        0x12
#define EVENTID_STATUS_EVENT            	0x16
#define EVENTID_INTERNAL_RELEASE_INFO       0x19
#define EVENTID_EXTERNAL_RELEASE_INFO       0x1A

#define EVENTID_GESTURE                     0x20

#define INT_ENABLE						0x41
#define INT_DISABLE						0x00

#define READ_STATUS					0x84
#define READ_ONE_EVENT				0x85
#define READ_ALL_EVENT				0x86

#define SLEEPIN							0x90
#define SLEEPOUT						0x91
#define SENSEOFF						0x92
#define SENSEON							0x93
#define SENSEON_SLOW					0x9C


#define FTS_CMD_HOVER_OFF           0x94
#define FTS_CMD_HOVER_ON            0x95

#define FTS_CMD_FLIPCOVER_OFF		0x96
#define FTS_CMD_FLIPCOVER_ON		0x97
#define FTS_RETRY_COUNT		30

#define FTS_CMD_KEY_SENSE_ON		0x9B


#define FTS_CMD_SET_FAST_GLOVE_MODE	0x9D

#define FTS_CMD_MSHOVER_OFF         0x9E
#define FTS_CMD_MSHOVER_ON          0x9F
#define FTS_CMD_SET_NOR_GLOVE_MODE	0x9F

#define FLUSHBUFFER					0xA1
#define FORCECALIBRATION			0xA2
#define CX_TUNNING					0xA3
#define SELF_AUTO_TUNE				0xA4
#define KEY_CX_TUNNING				0x96

#define FTS_CMD_CHARGER_PLUGGED     0xA8
#define FTS_CMD_CHARGER_UNPLUGGED	0xAB

#define FTS_CMD_RELEASEINFO     0xAA
#define FTS_CMD_STYLUS_OFF          0xAB
#define FTS_CMD_STYLUS_ON           0xAC

#define FTS_CMD_WRITE_PRAM          0xF0
#define FTS_CMD_BURN_PROG_FLASH     0xF2
#define FTS_CMD_ERASE_PROG_FLASH    0xF3
#define FTS_CMD_READ_FLASH_STAT     0xF4
#define FTS_CMD_UNLOCK_FLASH        0xF7
#define FTS_CMD_SAVE_FWCONFIG       0xFB
#define FTS_CMD_SAVE_CX_TUNING      0xFC

#define TSP_BUF_SIZE 2048
#define CMD_STR_LEN 32
#define CMD_RESULT_STR_LEN 2048
#define CMD_PARAM_NUM 8

#define RAW_MAX	3750

#undef TSP_INIT_COMPLETE

/**
 * struct fts_finger - Represents fingers.
 * @ state: finger status (Event ID).
 * @ mcount: moving counter for debug.
 */
struct fts_finger {
	unsigned char state;
	unsigned short mcount;
};

struct fts_ts_platform_data {
	u32 gpio_int;
	int gpio_ldo_en;
	int gpio_scl;
	int gpio_sda;
	int gpio_io_en;
#ifdef USE_TSP_TA_CALLBACKS
	struct tsp_callbacks callbacks;
	void (*register_cb)(struct tsp_callbacks *);
#endif	
};

struct fts_ts_info {
	struct device *dev;
	struct i2c_client *client;
	struct fts_ts_platform_data	*pdata;
	struct input_dev *input_dev;
	struct hrtimer timer;
	struct timer_list timer_charger;
	struct timer_list timer_firmware;
	struct work_struct work;
	struct pinctrl *pinctrl;	
	int irq;
	int irq_type;
	bool irq_enabled;
	struct fts_i2c_platform_data *board;
#ifdef USE_TSP_TA_CALLBACKS
	struct tsp_callbacks callbacks;
	void (*register_cb)(struct tsp_callbacks *);
#endif	
	struct mutex lock;
	bool enabled;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
#ifdef SEC_TSP_FACTORY_TEST
	struct device *fac_dev_ts;
	struct list_head cmd_list_head;
	u8 cmd_state;
	char cmd[CMD_STR_LEN];
	int cmd_param[CMD_PARAM_NUM];
	char *cmd_result;
	int cmd_buffer_size;
	struct mutex cmd_lock;
	bool cmd_is_running;
	int SenseChannelLength;
	int ForceChannelLength;
	short *pFrame;
	unsigned char *cx_data;
#endif

#ifdef USE_WARKAROUND_CODE
	struct delayed_work work_io_gpio;
#endif

#ifdef TSP_BOOSTER
	struct input_booster	*booster;
#endif

	struct completion init_done;

	bool slow_report_rate;
	bool hover_ready;
	bool hover_enabled;
	bool mshover_enabled;
	bool fast_mshover_enabled;
#ifdef CLEAR_COVER	
	bool flip_enable;
#endif
	bool run_autotune;

#ifdef USE_TSP_TA_CALLBACKS
	bool TA_Pluged;
#endif

	int touch_count;
	struct fts_finger finger[FINGER_MAX];

	int touch_mode;

	int ic_product_id;			/* product id of ic */
	int ic_revision_of_ic;		/* revision of reading from IC */
	int fw_version_of_ic;		/* firmware version of IC */
	int ic_revision_of_bin;		/* revision of reading from binary */
	int fw_version_of_bin;		/* firmware version of binary */
	int config_version_of_ic;		/* Config release data from IC */
	int config_version_of_bin;	/* Config release data from IC */
	unsigned short fw_main_version_of_ic;	/* firmware main version of IC */
	unsigned short fw_main_version_of_bin;	/* firmware main version of binary */
	int panel_revision;			/* Octa panel revision */

#ifdef USE_OPEN_DWORK
	struct delayed_work open_work;
#endif

#ifdef FTS_SUPPORT_NOISE_PARAM
	struct fts_noise_param noise_param;
	int (*fts_get_noise_param_address) (struct fts_ts_info *info);
#endif

	struct mutex i2c_mutex;
	struct mutex device_mutex;
	bool touch_stopped;
	bool reinit_done;

#ifdef FTS_SUPPORT_TOUCH_KEY
	unsigned char tsp_keystatus;
	bool report_dummy_key;
	bool ignore_menu_key;
	bool ignore_back_key;
	bool ignore_menu_key_by_back;
	bool ignore_back_key_by_menu;
	int touchkey_threshold;
#endif // FTS_SUPPORT_TOUCH_KEY

	unsigned char data[FTS_EVENT_SIZE * FTS_FIFO_MAX];

	int (*stop_device) (struct fts_ts_info * info);
	int (*start_device) (struct fts_ts_info * info);

	int (*fts_write_reg)(struct fts_ts_info *info, unsigned char *reg, unsigned short num_com);
	int (*fts_read_reg)(struct fts_ts_info *info, unsigned char *reg, int cnum, unsigned char *buf, int num);
	void (*fts_systemreset)(struct fts_ts_info *info);
	int (*fts_wait_for_ready)(struct fts_ts_info *info);
	void (*fts_command)(struct fts_ts_info *info, unsigned char cmd);
	int (*fts_get_version_info)(struct fts_ts_info *info);
#ifdef READ_LCD_ID
	int lcd_id;
#endif
};

int fts_fw_update_on_probe(struct fts_ts_info *info);
int fts_fw_update_on_hidden_menu(struct fts_ts_info *info, int update_type);
void fts_fw_init(struct fts_ts_info *info);
int GetSystemStatus(struct fts_ts_info *info, unsigned char *val1, unsigned char *val2);

#endif				//_LINUX_FTS_TS_H_
