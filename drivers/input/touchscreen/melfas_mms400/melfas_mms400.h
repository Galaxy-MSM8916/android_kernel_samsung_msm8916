/*
 * MELFAS MMS400 Touchscreen Driver
 *
 * Copyright (C) 2014 MELFAS Inc.
 *
 */

#ifndef __MELFAS_MMS400_H
#define __MELFAS_MMS400_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/limits.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/gpio_event.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>

#include "melfas_mms400_reg.h"
#include <linux/input/tsp_ta_callback.h>

#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif

#ifdef CONFIG_OF
#define MMS_USE_DEVICETREE		1
#else
#define MMS_USE_DEVICETREE		0
#endif

#define MMS_DEVICE_NAME		"mms_ts"
#define MMS_CONFIG_DATE		{0x7, 0xDE, 0x0C, 0x10} // 2014. 12. 16

//Chip info
#if defined(CONFIG_TOUCHSCREEN_MELFAS_MMS438)
#define CHIP_MMS438
#define CHIP_NAME		"MMS438"
#define CHIP_FW_CODE		"M4H0"
#define FW_UPDATE_TYPE		"MMS438"
#elif defined(CONFIG_TOUCHSCREEN_MELFAS_MMS449)
#define CHIP_MMS449
#define CHIP_NAME		"MMS449"
#define CHIP_FW_CODE		"M4HP"
#define FW_UPDATE_TYPE		"MMS438"
#elif defined(CONFIG_TOUCHSCREEN_MELFAS_MMS458)
#define CHIP_MMS458
#define CHIP_NAME		"MMS458"
#define CHIP_FW_CODE		"M4HN"
#define FW_UPDATE_TYPE		"MMS438"
#elif defined(CONFIG_TOUCHSCREEN_MELFAS_MMS492)
#define CHIP_MMS492
#define CHIP_NAME		"MMS492"
#define CHIP_FW_CODE		"M4HL"
#define FW_UPDATE_TYPE		"MMS492"
#endif

//Config driver
#define MMS_USE_INPUT_OPEN_CLOSE	1
#define I2C_RETRY_COUNT			3
#define RESET_ON_EVENT_ERROR		0
#define ESD_COUNT_FOR_DISABLE		7
#define MMS_USE_TOUCHKEY		0

//Features
#define MMS_USE_NAP_MODE		0
#define MMS_USE_TEST_MODE		1
#define MMS_USE_CMD_MODE		1
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
#define MMS_USE_DEV_MODE		0
#else
#define MMS_USE_DEV_MODE		1
#endif

//Input value
#define MAX_FINGER_NUM			10
#define INPUT_AREA_MIN			0
#define INPUT_AREA_MAX			255
#define INPUT_PRESSURE_MIN		0
#define INPUT_PRESSURE_MAX		255
#define INPUT_TOUCH_MAJOR_MIN		0
#define INPUT_TOUCH_MAJOR_MAX		255
#define INPUT_TOUCH_MINOR_MIN		0
#define INPUT_TOUCH_MINOR_MAX		255
#define INPUT_ANGLE_MIN			0
#define INPUT_ANGLE_MAX			255
#define INPUT_HOVER_MIN			0
#define INPUT_HOVER_MAX			255
#define INPUT_PALM_MIN			0
#define INPUT_PALM_MAX			1

//Firmware update
#if defined(CONFIG_SEC_K5_PROJECT)
#define INTERNAL_FW_PATH		"tsp_melfas/mms449_k5.fw"
#else	// K7, H7, A8
#define INTERNAL_FW_PATH		"tsp_melfas/mms449_a8.fw"
#define APPLY_RESOLUTION
#endif

#define EXTERNAL_FW_PATH		"/sdcard/melfas.mfsb"
#define FFU_FW_PATH			"ffu_tsp.bin"
#define MMS_USE_AUTO_FW_UPDATE		1
#define MMS_FW_MAX_SECT_NUM		4
#define MMS_FW_UPDATE_DEBUG		0
#define MMS_FW_UPDATE_SECTION		1
#define MMS_EXT_FW_FORCE_UPDATE		1

//Command mode
#define CMD_LEN				32
#define CMD_RESULT_LEN			512
#define CMD_PARAM_NUM			8

/**
 * Device info structure
 */
struct mms_ts_info {
	struct i2c_client *client;
	struct input_dev *input_dev;
	char phys[32];
	struct mms_devicetree_data *dtdata;
	struct pinctrl *pinctrl;

	dev_t mms_dev;
	struct class *class;

	struct mutex lock;
	struct mutex lock_test;
	struct mutex lock_cmd;
	struct mutex lock_dev;

	int irq;
	bool	 enabled;
	bool init;
	char *fw_name;

	u8 product_name[16];
	int max_x;
	int max_y;
	u8 node_x;
	u8 node_y;
	u8 node_key;
	u8 boot_ver_ic;
	u8 core_ver_ic;
	u8 config_ver_ic;
	u8 event_size;
	u16 fw_year;
	u8 fw_month;
	u8 fw_date;
	u16 pre_chksum;
	u16 rt_chksum;
	unsigned char finger_state[MAX_FINGER_NUM];
	int touch_count;

	bool tkey_enable;

	u8 nap_mode;
	u8 glove_mode;
	u8 charger_mode;
	u8 cover_mode;

	u8 esd_cnt;
	bool disable_esd;

	u8 *print_buf;
	int *image_buf;

	bool test_busy;
	bool cmd_busy;
	bool dev_busy;

	bool read_all_data;

#if MMS_USE_CMD_MODE
	dev_t cmd_dev_t;
	struct device *cmd_dev;
	struct class *cmd_class;
	struct list_head cmd_list_head;
	u8 cmd_state;
	char cmd[CMD_LEN];
	char *cmd_result;
	int cmd_param[CMD_PARAM_NUM];
	int cmd_buffer_size;
#endif

#if MMS_USE_DEV_MODE
	struct cdev cdev;
	u8 *dev_fs_buf;
#endif

#ifdef USE_TSP_TA_CALLBACKS
	void (*register_cb)(struct tsp_callbacks *);
	struct tsp_callbacks callbacks;
#endif

#ifdef CONFIG_INPUT_BOOSTER
	struct input_booster *booster;
#endif
#ifdef CONFIG_TOUCHSCREEN_DUMP_MODE
	struct delayed_work ghost_check;
	u8 tsp_dump_lock;
	u8 add_log_header;
#endif

};

/**
 * Platform Data
 */
struct mms_devicetree_data {
	int gpio_intr;
	int gpio_vdd;
	int gpio_vio;
	int gpio_sda;
	int gpio_scl;
	struct regulator *vreg_vio;
	const char *fw_path;
	bool fix_resolution;
	bool support_glove_mode;
	bool thr_read_from_ic;
};

/**
 * Firmware binary header info
 */
struct mms_bin_hdr {
	char	tag[8];
	u16	core_version;
	u16	section_num;
	u16	contains_full_binary;
	u16	reserved0;

	u32	binary_offset;
	u32	binary_length;

	u32	extention_offset;
	u32	reserved1;
} __attribute__ ((packed));

/**
 * Firmware image info
 */
struct mms_fw_img {
	u16	type;
	u16	version;

	u16	start_page;
	u16	end_page;

	u32	offset;
	u32	length;
} __attribute__ ((packed));

/**
 * Firmware update error code
 */
enum fw_update_errno{
	fw_err_file_read = -4,
	fw_err_file_open = -3,
	fw_err_file_type = -2,
	fw_err_download = -1,
	fw_err_none = 0,
	fw_err_uptodate = 1,
};

/**
 * Declarations
 */
//main
void mms_reboot(struct mms_ts_info *info);
int mms_i2c_read(struct mms_ts_info *info, char *write_buf, unsigned int write_len,
		char *read_buf, unsigned int read_len);
int mms_i2c_read_next(struct mms_ts_info *info, char *read_buf, int start_idx,
		unsigned int read_len);
int mms_i2c_write(struct mms_ts_info *info, char *write_buf, unsigned int write_len);
int mms_enable(struct mms_ts_info *info);
int mms_disable(struct mms_ts_info *info);
int mms_get_ready_status(struct mms_ts_info *info);
int mms_get_fw_version(struct mms_ts_info *info, u8 *ver_buf);
int mms_get_fw_version_u16(struct mms_ts_info *info, u16 *ver_buf_u16);
int mms_disable_esd_alert(struct mms_ts_info *info);
int mms_fw_update_from_kernel(struct mms_ts_info *info, bool force, bool ffu);
int mms_fw_update_from_storage(struct mms_ts_info *info, bool force);
int mms_suspend(struct device *dev);
int mms_resume(struct device *dev);

//mod
int mms_power_control(struct mms_ts_info *info, int enable);
void mms_clear_input(struct mms_ts_info *info);
void mms_report_input_event(struct mms_ts_info *info, u8 sz, u8 *buf);
void mms_input_event_handler(struct mms_ts_info *info, u8 sz, u8 *buf);
int mms_pinctrl_configure(struct mms_ts_info *info, int active);
#if MMS_USE_DEVICETREE
int mms_parse_devicetree(struct device *dev, struct mms_ts_info *info);
#endif
void mms_config_input(struct mms_ts_info *info);
int mms_enable_glove_mode(struct mms_ts_info *info, int enable);

//fw_update
int mms_flash_fw(struct mms_ts_info *info, const u8 *fw_data, size_t fw_size,
		bool force, bool section);

//test
#if MMS_USE_DEV_MODE
int mms_dev_create(struct mms_ts_info *info);
int mms_get_log(struct mms_ts_info *info);
#endif
int mms_run_test(struct mms_ts_info *info, u8 test_type);
int mms_get_image(struct mms_ts_info *info, u8 image_type);
#if MMS_USE_TEST_MODE
int mms_sysfs_create(struct mms_ts_info *info);
void mms_sysfs_remove(struct mms_ts_info *info);
static const struct attribute_group mms_test_attr_group;
#endif

//cmd
#if MMS_USE_CMD_MODE
int mms_sysfs_cmd_create(struct mms_ts_info *info);
void mms_sysfs_cmd_remove(struct mms_ts_info *info);
static const struct attribute_group mms_cmd_attr_group;
extern struct class *sec_class;
#endif

#ifdef USE_TSP_TA_CALLBACKS
void mms_charger_status_cb(struct tsp_callbacks *cb, int status);
void mms_register_callback(struct tsp_callbacks *cb);
#endif
#ifdef CHARGER_NOTIFIER
void mms_set_tsp_info(struct mms_ts_info *tsp_data);
void set_charger_config(struct mms_ts_info *tsp_data);
#endif
#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif
extern int get_lcd_attached(char *mode);

#endif /* __MELFAS_MMS400_H */
