/*!
* @section LICENSE
 * (C) Copyright 2011~2015 Bosch Sensortec GmbH All Rights Reserved
 *
 * This software program is licensed subject to the GNU General
 * Public License (GPL).Version 2,June 1991,
 * available at http://www.fsf.org/copyleft/gpl.html
*
* @filename bhy_core.h
* @date     "Fri Oct 30 18:57:44 2015 +0800"
* @id       "3230a0f"
*
* @brief
* The header file for BHy driver core
*/

#ifndef BHY_CORE_H

#include <linux/types.h>
#include <linux/wakelock.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/events.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/types.h>
#include <linux/sensor/sensors_core.h>
#include "bstclass.h"

#ifndef MODULE_TAG
#define MODULE_TAG "BHY"
#endif

#define BHY_DEBUG
/*
 * BHY_RESERVE_FOR_LATER_USE:
 * We need this funtion for future use
 */

/*#define BHY_AR_HAL_SUPPORT*/

/* Support for timestamp logging for analysis */
/*#define BHY_TS_LOGGING_SUPPORT*/

#ifdef BHY_TS_LOGGING_SUPPORT
#define BHY_SENSOR_HANDLE_AP_SLEEP_STATUS	128
#define BHY_AP_STATUS_SUSPEND	1
#define BHY_AP_STATUS_RESUME	2
#endif /*~ BHY_TS_LOGGING_SUPPORT */

#ifdef BHY_DEBUG
#define BHY_SENSOR_HANDLE_DATA_LOG_TYPE	129
#define BHY_DATA_LOG_TYPE_NONE	0
#define BHY_DATA_LOG_TYPE_RAW	1
#define BHY_DATA_LOG_TYPE_INPUT_GESTURE	2
#define BHY_DATA_LOG_TYPE_INPUT_TILT_AR	3
#define BHY_SENSOR_HANDLE_LOG_FUSION_DATA	130
#define BHY_FUSION_DATA_LOG_NONE	0
#define BHY_FUSION_DATA_LOG_ENABLE	1
#endif /*~ BHY_DEBUG */

/* Supporting calib profile loading in fuser core */
#define BHY_CALIB_PROFILE_OP_IN_FUSER_CORE

#define SENSOR_NAME					"bhy"
#define SENSOR_INPUT_DEV_NAME		SENSOR_NAME
#ifdef BHY_AR_HAL_SUPPORT
#define SENSOR_AR_INPUT_DEV_NAME	"bhy_ar"
#endif /*~ BHY_AR_HAL_SUPPORT */

enum IIO_ATTR_ADDR {
	ATTR_SHEALTH_ENABLE,
	ATTR_SHEALTH_FLUSH_CADENCE,
	ATTR_PEDOMETER_STEPS,
	ATTR_SHEALTH_CADENCE,
};

struct bhy_data_bus {
	struct device *dev;
	s32 (*read)(struct device *dev, u8 reg, u8 *data, u16 len);
	s32 (*write)(struct device *dev, u8 reg, u8 *data, u16 len);
	int irq;
	int bus_type;
};

struct __attribute__((__packed__)) fifo_frame {
	u16 handle;
	u8 data[20];
};

#define BHY_FRAME_SIZE		7000
#define BHY_FRAME_SIZE_AR	50

#define LOG_TIMEOUT		(1*HZ)
#define LOGGING_REG		0x56
#define MAX_LOGGING_SIZE	20
#define PEDOMETER_CYCLE		50 /* HZ */
#define PEDOMETER_SENSOR	BHY_SENSOR_HANDLE_CUSTOM_3_WU
#define AR_SENSOR		BHY_SENSOR_HANDLE_CUSTOM_1
#define FIRST_STEP		6

#define LOGGING_DONE		0x01
#define NEW_STEP		0x02
#define START_WALK		0x06
#define STOP_WALK		0x08

#define REACTIVE_ALERT_SENSOR BHY_SENSOR_HANDLE_CUSTOM_4_WU

#define ACC_NAME		"accelerometer_sensor"
#define CALIBRATION_FILE_PATH   "/efs/FactoryApp/calibration_data"
#define CALIBRATION_DATA_AMOUNT 20

#define MAX_ACCEL_1G            8192
#define MAX_ACCEL_2G            16384
#define MIN_ACCEL_2G            -16383
#define MAX_ACCEL_4G            32768

#define MODEL_NAME		"BHA250"
#define FIRMWARE_REVISION	15102100


/* CRYSTAL 32000 = 1 SEC */
#define MCU_CRY_TO_RT_NS	32000
#define ACC_PRINT_TIME		5 /* second */

#define WAIT_PATCH_TIMEOUT	(10 * HZ)
#define RAM_PATCH_READY	0
#define RAM_PATCH_LOADED	1
#define LIGHT_POWER		"/sys/class/sensors/light_sensor/power_reset"
#define LIGHT_RESET		"/sys/class/sensors/light_sensor/sw_reset"
#define LIGHT_VDD_RESET		"/sys/class/sensors/light_sensor/vdd_reset"
#define MAG_POWER		"/sys/class/sensors/magnetic_sensor/power_reset"
#define MAG_RESET		"/sys/class/sensors/magnetic_sensor/sw_reset"

#define INT_DEBUG_COUNT		1000
#define FRAME_DEBUG_COUNT	1000

struct frame_queue {
	struct fifo_frame *frames;
	int head;
	int tail;
	struct mutex lock;
};

enum {
	SELF_TEST_RESULT_INDEX_ACC = 0,
	SELF_TEST_RESULT_INDEX_MAG,
	SELF_TEST_RESULT_INDEX_GYRO,
	SELF_TEST_RESULT_COUNT
};

struct pedometer_data {
	union {
		struct {
			unsigned char data_index;
			unsigned short walk_count;
			unsigned short run_count;
			unsigned char step_status;
			unsigned int start_time;
			unsigned int end_time;
		} __attribute__((__packed__));
		unsigned char data[14];
	} __attribute__((__packed__));
} __attribute__((__packed__));

struct bhy_client_data {
	struct mutex mutex_bus_op;
	struct bhy_data_bus data_bus;
	/*struct work_struct irq_work;*/
	struct input_dev *input;
#ifdef BHY_AR_HAL_SUPPORT
	struct input_dev *input_ar;
#endif /*~ BHY_AR_HAL_SUPPORT */
	struct attribute_group *input_attribute_group;
#ifdef BHY_AR_HAL_SUPPORT
	struct attribute_group *input_ar_attribute_group;
#endif /*~ BHY_AR_HAL_SUPPORT */
	struct attribute_group *bst_attribute_group;
	atomic_t reset_flag;
	int sensor_sel;
	s64 timestamp_irq;
	atomic_t in_suspend;
	struct wake_lock wlock;
	u8 *fifo_buf;
	struct frame_queue data_queue;
#ifdef BHY_AR_HAL_SUPPORT
	struct frame_queue data_queue_ar;
#endif /*~ BHY_AR_HAL_SUPPORT */
	u8 bmi160_foc_conf;
	u8 bma2x2_foc_conf;
	struct bst_dev *bst_dev;
	u16 rom_id;
	u16 ram_id;
	char dev_type[16];
	s8 mapping_matrix_acc[3][3];
	s8 mapping_matrix_acc_inv[3][3];
	s8 self_test_result[SELF_TEST_RESULT_COUNT];
	s16 acc_self_test_diff[3];
	s8 sensor_data_len[256];
#ifdef BHY_DEBUG
	int reg_sel;
	int reg_len;
	int page_sel;
	int param_sel;
	int enable_irq_log;
	int enable_fifo_log;
	int hw_slave_addr;
	int hw_reg_sel;
	int hw_reg_len;
#endif /*~ BHY_DEBUG */
#ifdef BHY_TS_LOGGING_SUPPORT
	u32 irq_count;
#endif /*~ BHY_TS_LOGGING_SUPPORT */

	struct device *acc_device;
	struct iio_dev *indio;
	struct pedometer_data pedo[MAX_LOGGING_SIZE + 1];
	unsigned int total_step;
	unsigned int last_total_step;
	unsigned int step_count;
	unsigned int last_step_count;
	bool late_step_report;
	unsigned char start_index;
	unsigned char current_index;
	unsigned short acc_delay;
	bool log_mode;
	bool walk_mode;
	bool last_walk_mode;
	bool acc_enabled;
	bool pedo_enabled;
	bool reactive_alert_enabled;
	bool reactive_alert_reported;
	bool reactive_alert_selftest;
	bool ar_enabled;
	bool step_det_enabled;
	bool step_cnt_enabled;
	bool tilt_enabled;
	bool pickup_enabled;
	bool smd_enabled;
	bool step_det;
	bool step_det_reported;
	struct mutex mutex_pedo;
	struct completion log_done;
	struct completion int_done;
	short acc_buffer[3];
	short acc_cal[3];
	u16 interrupt_mask;
	u8 bandwidth;
	unsigned int fw_version;
	int acc_axis;

	/* Monitor Thread */
	struct task_struct *monitor_task;
	atomic_t ram_patch_loaded;
	struct wake_lock patch_wlock;
	struct wake_lock reset_wlock;
	bool irq_enabled;
	bool irq_ready;
	wait_queue_head_t monitor_wq;
	unsigned int cnt_reset;
	unsigned int cnt_total_reset;
	unsigned int cnt_no_response;
	u64 last_reset_time_buf[3];
	bool skip_reset;
	u64 last_reset_time;
	u64 last_acc_check_time;
	short acc_history[10][3];
	unsigned int idx_acc_history;
	unsigned int cnt_acc_history;

	int ldo_enable_pin;
	bool irq_force_disabled;
};


int bhy_suspend(struct device *dev);
int bhy_resume(struct device *dev);
int bhy_probe(struct bhy_data_bus *data_bus);
int bhy_remove(struct device *dev);

#ifdef CONFIG_PM
int bhy_suspend(struct device *dev);
int bhy_resume(struct device *dev);
#endif

#endif /** BHY_CORE_H */
