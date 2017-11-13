/*
 * STMicroelectronics lsm6ds3 driver
 *
 * Copyright 2014 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 * v. 1.1.2
 * Licensed under the GPL-2.
 */

#ifndef ST_LSM6DS3_H
#define ST_LSM6DS3_H

#include <linux/types.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/iio/trigger.h>

#define LSM6DS3_HRTIMER_TRIGGER			1
#define LSM6DS3_SMART_ALERT			LSM6DS3_HRTIMER_TRIGGER

#if (LSM6DS3_SMART_ALERT > 0)
#include <linux/wakelock.h>
#endif
#define LSM6DS3_DEV_NAME		"lsm6ds3"
#define VENDOR_NAME			"STM"
#define MODULE_NAME_ACC			"accelerometer_sensor"
#define MODULE_NAME_GYRO		"gyro_sensor"

#define CALIBRATION_FILE_PATH		"/efs/accel_calibration_data"
#define GYRO_CALIBRATION_FILE_PATH	"/efs/gyro_cal_data"

#define CALIBRATION_DATA_AMOUNT		20
#define MAX_ACCEL_1G			16384

#define ST_INDIO_DEV_ACCEL			0
#define ST_INDIO_DEV_GYRO			1
#define ST_INDIO_DEV_SIGN_MOTION		2
#define ST_INDIO_DEV_STEP_COUNTER		3
#define ST_INDIO_DEV_STEP_DETECTOR		4
#define ST_INDIO_DEV_TILT			5
#define ST_INDIO_DEV_NUM			6

#define ST_LSM6DS3_ACCEL_DEPENDENCY	((1 << ST_INDIO_DEV_ACCEL) | \
					(1 << ST_INDIO_DEV_STEP_COUNTER) | \
					(1 << ST_INDIO_DEV_TILT) | \
					(1 << ST_INDIO_DEV_SIGN_MOTION) | \
					(1 << ST_INDIO_DEV_STEP_DETECTOR))

#define ST_LSM6DS3_STEP_COUNTER_DEPENDENCY \
					((1 << ST_INDIO_DEV_STEP_COUNTER) | \
					(1 << ST_INDIO_DEV_SIGN_MOTION) | \
					(1 << ST_INDIO_DEV_STEP_DETECTOR))

#define ST_LSM6DS3_EXTRA_DEPENDENCY	((1 << ST_INDIO_DEV_STEP_COUNTER) | \
					(1 << ST_INDIO_DEV_TILT) | \
					(1 << ST_INDIO_DEV_SIGN_MOTION) | \
					(1 << ST_INDIO_DEV_STEP_DETECTOR))

#define ST_LSM6DS3_USE_BUFFER		((1 << ST_INDIO_DEV_ACCEL) | \
					(1 << ST_INDIO_DEV_GYRO) | \
					(1 << ST_INDIO_DEV_STEP_COUNTER))

#define ST_LSM6DS3_TX_MAX_LENGTH		12
#define ST_LSM6DS3_RX_MAX_LENGTH		4097

#define ST_LSM6DS3_NUMBER_DATA_CHANNELS		3
#define ST_LSM6DS3_BYTE_FOR_CHANNEL		2

#define ST_LSM6DS3_LSM_CHANNELS(device_type, index, mod, endian, bits, addr) \
{ \
	.type = device_type, \
	.modified = 1, \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | \
			BIT(IIO_CHAN_INFO_SCALE), \
	.scan_index = index, \
	.channel2 = mod, \
	.address = addr, \
	.scan_type = { \
		.sign = 's', \
		.realbits = bits, \
		.shift = 16 - bits, \
		.storagebits = 16, \
		.endianness = endian, \
	}, \
}

struct st_lsm6ds3_transfer_buffer {
	struct mutex buf_lock;
	u8 rx_buf[ST_LSM6DS3_RX_MAX_LENGTH];
	u8 tx_buf[ST_LSM6DS3_TX_MAX_LENGTH] ____cacheline_aligned;
};

struct st_lsm6ds3_transfer_function {
	int (*write) (struct st_lsm6ds3_transfer_buffer *tb,
			struct device *dev, u8 reg_addr, int len, u8 *data);
	int (*read) (struct st_lsm6ds3_transfer_buffer *tb,
			struct device *dev, u8 reg_addr, int len, u8 *data);
};

struct lsm6ds3_steps {
	bool new_steps;
	u16 steps;

	struct hrtimer hr_timer;
	ktime_t ktime;

	s64 last_step_timestamp;
};

struct lsm6ds3_data {
	const char *name;

	u8 drdy_int_pin;
	u8 sensors_enabled;

	u8 gyro_module;
	u16 gyro_selftest_delta[3];
	u8 gyro_selftest_status;
	u8 accel_selftest_status;

	bool patch_feature;
	bool sign_motion_event_ready;

	int irq;
	int irq_gpio;
	s16 accel_data[3];
	s16 gyro_data[3];

	s16 accel_cal_data[3];
	s16 gyro_cal_data[3];

	int64_t timestamp;

	struct work_struct data_work;

	struct device *dev;
	struct device *acc_factory_dev;
	struct device *gyro_factory_dev;
	struct iio_dev *indio_dev[ST_INDIO_DEV_NUM];
	struct iio_trigger *trig[ST_INDIO_DEV_NUM];
	struct lsm6ds3_steps step_c;

	const struct st_lsm6ds3_transfer_function *tf;
	struct st_lsm6ds3_transfer_buffer tb;

#if (LSM6DS3_SMART_ALERT > 0)
	struct delayed_work sa_irq_work;
	struct wake_lock sa_wake_lock;
	int sa_irq_state;
	int sa_flag;
#endif

	struct regulator *reg_vio;
	struct i2c_client *client;

	u8 odr;
};

struct lsm6ds3_sensor_data {
	struct lsm6ds3_data *cdata;

	unsigned int c_odr;
	unsigned int c_gain;
	unsigned int bandwidth;

	int lpf_on;

	u8 sindex;
	u8 *buffer_data;

	int st_mode;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	struct hrtimer hr_timer;
	ktime_t ktime;
	int hr_timer_en;
	struct workqueue_struct *tmr_wq;
	struct work_struct tmr_work;
#endif
};

int st_lsm6ds3_common_probe(struct lsm6ds3_data *cdata, int irq);
void st_lsm6ds3_common_remove(struct lsm6ds3_data *cdata, int irq);

int st_lsm6ds3_set_enable(struct lsm6ds3_sensor_data *sdata, bool enable);
int st_lsm6ds3_set_axis_enable(struct lsm6ds3_sensor_data *sdata, u8 value);
int st_lsm6ds3_set_drdy_irq(struct lsm6ds3_sensor_data *sdata, bool state);

#ifdef CONFIG_IIO_BUFFER
int st_lsm6ds3_allocate_rings(struct lsm6ds3_data *cdata);
void st_lsm6ds3_deallocate_rings(struct lsm6ds3_data *cdata);
int st_lsm6ds3_trig_set_state(struct iio_trigger *trig, bool state);
#define ST_LSM6DS3_TRIGGER_SET_STATE (&st_lsm6ds3_trig_set_state)
#else /* CONFIG_IIO_BUFFER */
static inline int st_lsm6ds3_allocate_rings(struct lsm6ds3_data *cdata)
{
	return 0;
}
static inline void st_lsm6ds3_deallocate_rings(struct lsm6ds3_data *cdata)
{
}
#define ST_LSM6DS3_TRIGGER_SET_STATE NULL
#endif /* CONFIG_IIO_BUFFER */

#ifdef CONFIG_IIO_TRIGGER
int st_lsm6ds3_allocate_triggers(struct lsm6ds3_data *cdata,
			const struct iio_trigger_ops *trigger_ops, int irq);

void st_lsm6ds3_deallocate_triggers(struct lsm6ds3_data *cdata, int irq);

#else /* CONFIG_IIO_TRIGGER */
static inline int st_lsm6ds3_allocate_triggers(struct lsm6ds3_data *cdata,
			const struct iio_trigger_ops *trigger_ops, int irq)
{
	return 0;
}
static inline void st_lsm6ds3_deallocate_triggers(struct lsm6ds3_data *cdata,
								int irq)
{
	return;
}
#endif /* CONFIG_IIO_TRIGGER */

static inline s64 st_lsm6ds3_iio_get_boottime_ns(void)
{
	struct timespec ts;

	ts = ktime_to_timespec(ktime_get_boottime());

	return timespec_to_ns(&ts);
}

#ifdef CONFIG_PM
int st_lsm6ds3_common_suspend(struct lsm6ds3_data *cdata);
int st_lsm6ds3_common_resume(struct lsm6ds3_data *cdata);
#endif /* CONFIG_PM */

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
void st_lsm6ds3_poll_work_acc(struct work_struct *data_work);
void st_lsm6ds3_poll_work_gyr(struct work_struct *data_work);
#endif

int st_acc_open_calibration(struct lsm6ds3_data *cdata);
int st_gyro_open_calibration(struct lsm6ds3_data *cdata);

#endif /* ST_LSM6DS3_H */
