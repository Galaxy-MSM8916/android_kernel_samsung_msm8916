/*
 * STMicroelectronics lsm6ds3 core driver
 *
 * Copyright 2014 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger.h>
#include <linux/iio/buffer.h>
#include <linux/iio/events.h>
#include <asm/unaligned.h>

#include <linux/sensor/sensors_core.h>
#include <linux/iio/common/st_sensors.h>
#include <linux/platform_data/st_lsm6ds3_pdata.h>
#include "st_lsm6ds3.h"
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>

#define MS_TO_NS(msec)				((msec) * 1000 * 1000)

#define ST_LSM6DS3_DATA_FW			"st_lsm6ds3_data_fw"

/* ZGL: 180mg @ 4G */
#define ST_LSM6DS3_ACC_MIN_ZGL_XY			((int)(-180/0.122f) - 1)
#define ST_LSM6DS3_ACC_MAX_ZGL_XY			((int)(180/0.122f) + 1)
#define ST_LSM6DS3_ACC_MIN_ZGL_Z			((int)((1000-210)/0.122f) - 1)
#define ST_LSM6DS3_ACC_MAX_ZGL_Z			((int)((1000+210)/0.122f) + 1)
/* Selftest: 90~1700mg @ 2G */
#define ST_LSM6DS3_ACC_MIN_ST			((int)(90/0.061f))
#define ST_LSM6DS3_ACC_MAX_ST			((int)(1700/0.061f) + 1)
#define ST_LSM6DS3_ACC_DA_RETRY_COUNT			5

/* ZRL: 40dps @ 2000dps */
#define ST_LSM6DS3_GYR_MIN_ZRL			((int)(-40/0.07f) - 1)
#define ST_LSM6DS3_GYR_MAX_ZRL			((int)(40/0.07f) + 1)
#define ST_LSM6DS3_GYR_ZRL_DELTA			((int)(6/0.07f) + 1)
/* Selftest: 250~700dps @ 2000dps */
#define ST_LSM6DS3_GYR_MIN_ST			((int)(200/0.07f))
#define ST_LSM6DS3_GYR_MAX_ST			((int)(700/0.07f) + 1)
#define ST_LSM6DS3_GYR_DA_RETRY_COUNT			5

#ifndef ABS
#define ABS(a)		((a) > 0 ? (a) : -(a))
#endif

/* COMMON VALUES FOR ACCEL-GYRO SENSORS */
#define ST_LSM6DS3_WAI_ADDRESS			0x0f
#define ST_LSM6DS3_WAI_EXP			0x69
#define ST_LSM6DS3_AXIS_EN_MASK			0x38
#define ST_LSM6DS3_INT1_ADDR			0x0d
#define ST_LSM6DS3_INT2_ADDR			0x0e
#define ST_LSM6DS3_MD1_ADDR			0x5e
#define ST_LSM6DS3_MD2_ADDR			0x5f
#define ST_LSM6DS3_FIFO_CTRL1_ADDR			0x06
#define ST_LSM6DS3_FIFO_CTRL2_ADDR			0x07
#define ST_LSM6DS3_FIFO_CTRL3_ADDR			0x08
#define ST_LSM6DS3_FIFO_CTRL4_ADDR			0x09
#define ST_LSM6DS3_FIFO_CTRL5_ADDR			0x0a
#define ST_LSM6DS3_FIFO_STAT1_ADDR			0x3a
#define ST_LSM6DS3_FIFO_STAT2_ADDR			0x3b
#define ST_LSM6DS3_FIFO_STAT3_ADDR			0x3c
#define ST_LSM6DS3_FIFO_STAT4_ADDR			0x3d
#define ST_LSM6DS3_FIFO_OUT_L_ADDR			0x3e
#define ST_LSM6DS3_CTRL1_ADDR			0x10
#define ST_LSM6DS3_CTRL2_ADDR			0x11
#define ST_LSM6DS3_CTRL3_ADDR			0x12
#define ST_LSM6DS3_CTRL4_ADDR			0x13
#define ST_LSM6DS3_CTRL5_ADDR			0x14
#define ST_LSM6DS3_CTRL6_ADDR			0x15
#define ST_LSM6DS3_CTRL7_ADDR			0x16
#define ST_LSM6DS3_CTRL8_ADDR			0x17
#define ST_LSM6DS3_CTRL9_ADDR			0x18
#define ST_LSM6DS3_CTRL10_ADDR			0x19
#define ST_LSM6DS3_OUT_TEMP_L_ADDR			0x20
#define ST_LSM6DS3_WU_SRC_ADDR			0x1b
#define ST_LSM6DS3_WU_THS_ADDR			0x5b
#define ST_LSM6DS3_WU_DUR_ADDR			0x5c
#define ST_LSM6DS3_ODR_LIST_NUM			8
#define ST_LSM6DS3_ODR_POWER_OFF_VAL		0x00
#define ST_LSM6DS3_ODR_13HZ_VAL			0x01
#define ST_LSM6DS3_ODR_26HZ_VAL			0x02
#define ST_LSM6DS3_ODR_52HZ_VAL			0x03
#define ST_LSM6DS3_ODR_104HZ_VAL		0x04
#define ST_LSM6DS3_ODR_208HZ_VAL		0x05
#define ST_LSM6DS3_ODR_416HZ_VAL		0x06
#define ST_LSM6DS3_ODR_833HZ_VAL		0x07
#define ST_LSM6DS3_ODR_1660HZ_VAL		0x08
#define ST_LSM6DS3_ODR_3330HZ_VAL		0x09
#define ST_LSM6DS3_ODR_6660HZ_VAL		0x0a
#define ST_LSM6DS3_ACCEL_BW_LIST_NUM		4
#define ST_LSM6DS3_ACCEL_BW_50HZ_VAL		0x03
#define ST_LSM6DS3_ACCEL_BW_100HZ_VAL		0x02
#define ST_LSM6DS3_ACCEL_BW_200HZ_VAL		0x01
#define ST_LSM6DS3_ACCEL_BW_400HZ_VAL		0x00
#define ST_LSM6DS3_FS_LIST_NUM			4
#define ST_LSM6DS3_BDU_ADDR			0x12
#define ST_LSM6DS3_BDU_MASK			0x40
#define ST_LSM6DS3_EN_BIT			0x01
#define ST_LSM6DS3_DIS_BIT			0x00
#define ST_LSM6DS3_FUNC_EN_ADDR			0x19
#define ST_LSM6DS3_FUNC_EN_MASK			0x04
#define ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR		0x01
#define ST_LSM6DS3_FUNC_CFG_ACCESS_MASK		0x01
#define ST_LSM6DS3_FUNC_CFG_ACCESS_MASK2	0x04
#define ST_LSM6DS3_FUNC_CFG_REG2_MASK		0x80
#define ST_LSM6DS3_FUNC_CFG_START1_ADDR		0x62
#define ST_LSM6DS3_FUNC_CFG_START2_ADDR		0x63
#define ST_LSM6DS3_FUNC_CFG_DATA_WRITE_ADDR	0x64
#define ST_LSM6DS3_SELFTEST_ADDR		0x14
#define ST_LSM6DS3_SELFTEST_ACCEL_MASK		0x03
#define ST_LSM6DS3_SELFTEST_GYRO_MASK		0x0c
#define ST_LSM6DS3_GYRO_READY_TIME_FROM_PD_MS	150
#define ST_LSM6DS3_ACCEL_READY_TIME_FROM_PD_MS	10
#define ST_LSM6DS3_DEFAULT_STEP_C_MAXDR_MS	(1000UL)
#define ST_LSM6DS3_DEFAULT_HRTIMER_ODR_MS	(10UL)
#define ST_LSM6DS3_SELF_TEST_DISABLED_VAL	0x00
#define ST_LSM6DS3_SELF_TEST_POS_SIGN_VAL	0x01
#define ST_LSM6DS3_SELF_TEST_NEG_ACCEL_SIGN_VAL	0x02
#define ST_LSM6DS3_SELF_TEST_NEG_GYRO_SIGN_VAL	0x03
#define ST_LSM6DS3_FLASH_ADDR			0x00
#define ST_LSM6DS3_FLASH_MASK			0x40
#define ST_LSM6DS3_ORIENT_ADDRESS		0x4d
#define ST_LSM6DS3_ORIENT_MASK			0x07
#define ST_LSM6DS3_ORIENT_MODULE1		0x00
#define ST_LSM6DS3_ORIENT_MODULE2		0x05
#define ST_LSM6DS3_GAIN_X_ADDRESS		0x44
#define ST_LSM6DS3_GAIN_MASK			0x7f
#define ST_LSM6DS3_AD_FEATURES_ADDRESS		0x43
#define ST_LSM6DS3_AD_FEATURES_MASK		0x04
#define ST_LSM6DS3_LIR_ADDR			0x58
#define ST_LSM6DS3_LIR_MASK			0x01
#define ST_LSM6DS3_RESET_ADDR			0x12
#define ST_LSM6DS3_RESET_MASK			0x01
#define ST_LSM6DS3_FIFO_TEST_DEPTH			128
#define ST_LSM6DS3_SA_DYNAMIC_THRESHOLD			250	// mg

/* CUSTOM VALUES FOR ACCEL SENSOR */
#define ST_LSM6DS3_ACCEL_ODR_ADDR		0x10
#define ST_LSM6DS3_ACCEL_ODR_MASK		0xf0
#define ST_LSM6DS3_ACCEL_BW_ADDR		0x10
#define ST_LSM6DS3_ACCEL_BW_MASK		0x03
#define ST_LSM6DS3_ACCEL_FS_ADDR		0x10
#define ST_LSM6DS3_ACCEL_FS_MASK		0x0c
#define ST_LSM6DS3_ACCEL_FS_2G_VAL		0x00
#define ST_LSM6DS3_ACCEL_FS_4G_VAL		0x02
#define ST_LSM6DS3_ACCEL_FS_8G_VAL		0x03
#define ST_LSM6DS3_ACCEL_FS_16G_VAL		0x01
#define ST_LSM6DS3_ACCEL_FS_2G_GAIN		IIO_G_TO_M_S_2(61)
#define ST_LSM6DS3_ACCEL_FS_4G_GAIN		IIO_G_TO_M_S_2(122)
#define ST_LSM6DS3_ACCEL_FS_8G_GAIN		IIO_G_TO_M_S_2(244)
#define ST_LSM6DS3_ACCEL_FS_16G_GAIN		IIO_G_TO_M_S_2(488)
#define ST_LSM6DS3_ACCEL_OUT_X_L_ADDR		0x28
#define ST_LSM6DS3_ACCEL_OUT_Y_L_ADDR		0x2a
#define ST_LSM6DS3_ACCEL_OUT_Z_L_ADDR		0x2c
#define ST_LSM6DS3_ACCEL_AXIS_EN_ADDR		0x18
#define ST_LSM6DS3_ACCEL_DRDY_IRQ_MASK		0x01

/* CUSTOM VALUES FOR GYRO SENSOR */
#define ST_LSM6DS3_GYRO_ODR_ADDR		0x11
#define ST_LSM6DS3_GYRO_ODR_MASK		0xf0
#define ST_LSM6DS3_GYRO_FS_ADDR			0x11
#define ST_LSM6DS3_GYRO_FS_MASK			0x0c
#define ST_LSM6DS3_GYRO_FS_245_VAL		0x00
#define ST_LSM6DS3_GYRO_FS_500_VAL		0x01
#define ST_LSM6DS3_GYRO_FS_1000_VAL		0x02
#define ST_LSM6DS3_GYRO_FS_2000_VAL		0x03
#define ST_LSM6DS3_GYRO_FS_245_GAIN		IIO_DEGREE_TO_RAD(4375)
#define ST_LSM6DS3_GYRO_FS_500_GAIN		IIO_DEGREE_TO_RAD(8750)
#define ST_LSM6DS3_GYRO_FS_1000_GAIN		IIO_DEGREE_TO_RAD(17500)
#define ST_LSM6DS3_GYRO_FS_2000_GAIN		IIO_DEGREE_TO_RAD(70000)
#define ST_LSM6DS3_GYRO_OUT_X_L_ADDR		0x22
#define ST_LSM6DS3_GYRO_OUT_Y_L_ADDR		0x24
#define ST_LSM6DS3_GYRO_OUT_Z_L_ADDR		0x26
#define ST_LSM6DS3_GYRO_AXIS_EN_ADDR		0x19
#define ST_LSM6DS3_GYRO_DRDY_IRQ_MASK		0x02

/* CUSTOM VALUES FOR SIGNIFICANT MOTION SENSOR */
#define ST_LSM6DS3_SIGN_MOTION_EN_ADDR		0x19
#define ST_LSM6DS3_SIGN_MOTION_EN_MASK		0x01
#define ST_LSM6DS3_SIGN_MOTION_DRDY_IRQ_MASK	0x40

/* CUSTOM VALUES FOR STEP COUNTER SENSOR */
#define ST_LSM6DS3_STEP_COUNTER_EN_ADDR		0x58
#define ST_LSM6DS3_STEP_COUNTER_EN_MASK		0x40
#define ST_LSM6DS3_STEP_COUNTER_DRDY_IRQ_MASK	0x80
#define ST_LSM6DS3_STEP_COUNTER_OUT_L_ADDR	0x4b
#define ST_LSM6DS3_STEP_COUNTER_RES_ADDR	0x19
#define ST_LSM6DS3_STEP_COUNTER_RES_MASK	0x02
#define ST_LSM6DS3_STEP_COUNTER_THS_ADDR	0x0f
#define ST_LSM6DS3_STEP_COUNTER_THS_MASK	0x1f
#define ST_LSM6DS3_STEP_COUNTER_THS_VALUE	0x0f

/* CUSTOM VALUES FOR TILT SENSOR */
#define ST_LSM6DS3_TILT_EN_ADDR			0x58
#define ST_LSM6DS3_TILT_EN_MASK			0x20
#define ST_LSM6DS3_TILT_DRDY_IRQ_MASK		0x02

#define ST_LSM6DS3_LPF_ENABLE			0xf0
#define ST_LSM6DS3_LPF_DISABLE			0x00

#define ST_LSM6DS3_DEV_ATTR_SAMP_FREQ() \
		IIO_DEV_ATTR_SAMP_FREQ(S_IWUSR | S_IRUGO, \
			st_lsm6ds3_sysfs_get_sampling_frequency, \
			st_lsm6ds3_sysfs_set_sampling_frequency)

#define ST_LSM6DS3_DEV_ATTR_SAMP_FREQ_AVAIL() \
		IIO_DEV_ATTR_SAMP_FREQ_AVAIL( \
			st_lsm6ds3_sysfs_sampling_frequency_avail)

#define ST_LSM6DS3_DEV_ATTR_SCALE_AVAIL(name) \
		IIO_DEVICE_ATTR(name, S_IRUGO, \
			st_lsm6ds3_sysfs_scale_avail, NULL , 0);

static const u8 st_lsm6ds3_data[] = {
	#include "st_lsm6ds3_custom_data.dat"
};
DECLARE_BUILTIN_FIRMWARE(ST_LSM6DS3_DATA_FW, st_lsm6ds3_data);

static struct st_lsm6ds3_gain_table {
	u8 code;
	u16 value;
} st_lsm6ds3_gain_table[] = {
	{ .code = 0x00, .value = 5005 },
	{ .code = 0x01, .value = 4813 },
	{ .code = 0x02, .value = 4635 },
	{ .code = 0x03, .value = 5959 },
	{ .code = 0x04, .value = 5753 },
	{ .code = 0x05, .value = 5562 },
	{ .code = 0x06, .value = 5382 },
	{ .code = 0x07, .value = 5214 },
	{ .code = 0x08, .value = 5056 },
	{ .code = 0x09, .value = 4907 },
	{ .code = 0x0a, .value = 4767 },
	{ .code = 0x0b, .value = 4635 },
	{ .code = 0x0c, .value = 5637 },
	{ .code = 0x0d, .value = 5488 },
	{ .code = 0x0e, .value = 5348 },
	{ .code = 0x0f, .value = 5214 },
	{ .code = 0x10, .value = 5087 },
	{ .code = 0x11, .value = 4966 },
	{ .code = 0x12, .value = 4850 },
	{ .code = 0x13, .value = 4740 },
	{ .code = 0x14, .value = 4635 },
	{ .code = 0x15, .value = 6348 },
	{ .code = 0x16, .value = 6212 },
	{ .code = 0x17, .value = 6083 },
	{ .code = 0x18, .value = 5959 },
	{ .code = 0x19, .value = 5840 },
	{ .code = 0x1a, .value = 5725 },
	{ .code = 0x1b, .value = 5615 },
	{ .code = 0x1c, .value = 5509 },
	{ .code = 0x1d, .value = 5407 },
	{ .code = 0x1e, .value = 6067 },
	{ .code = 0x1f, .value = 5959 },
	{ .code = 0x20, .value = 5854 },
	{ .code = 0x21, .value = 5753 },
	{ .code = 0x22, .value = 5656 },
	{ .code = 0x23, .value = 5561 },
	{ .code = 0x24, .value = 5470 },
	{ .code = 0x25, .value = 5382 },
	{ .code = 0x26, .value = 5297 },
	{ .code = 0x40, .value = 5005 },
	{ .code = 0x41, .value = 5212 },
	{ .code = 0x42, .value = 5441 },
	{ .code = 0x43, .value = 5688 },
	{ .code = 0x44, .value = 5959 },
	{ .code = 0x45, .value = 6257 },
	{ .code = 0x46, .value = 6586 },
	{ .code = 0x47, .value = 4635 },
	{ .code = 0x48, .value = 4907 },
	{ .code = 0x49, .value = 5214 },
	{ .code = 0x4a, .value = 5562 },
	{ .code = 0x4b, .value = 5959 },
	{ .code = 0x4c, .value = 6417 },
	{ .code = 0x4d, .value = 6952 },
	{ .code = 0x4e, .value = 7584 },
	{ .code = 0x4f, .value = 8343 },
};

static struct st_lsm6ds3_selftest_table {
	char *string_mode;
	u8 accel_value;
	u8 gyro_value;
	u8 accel_mask;
	u8 gyro_mask;
} st_lsm6ds3_selftest_table[] = {
	[0] = {
		.string_mode = "disabled",
		.accel_value = ST_LSM6DS3_SELF_TEST_DISABLED_VAL,
		.gyro_value = ST_LSM6DS3_SELF_TEST_DISABLED_VAL,
	},
	[1] = {
		.string_mode = "positive-sign",
		.accel_value = ST_LSM6DS3_SELF_TEST_POS_SIGN_VAL,
		.gyro_value = ST_LSM6DS3_SELF_TEST_POS_SIGN_VAL
	},
	[2] = {
		.string_mode = "negative-sign",
		.accel_value = ST_LSM6DS3_SELF_TEST_NEG_ACCEL_SIGN_VAL,
		.gyro_value = ST_LSM6DS3_SELF_TEST_NEG_GYRO_SIGN_VAL
	},
};

struct st_lsm6ds3_odr_reg {
	unsigned int hz;
	unsigned long ns;
	u8 value;
};

static struct st_lsm6ds3_odr_table {
	u8 addr[2];
	u8 mask[2];
	struct st_lsm6ds3_odr_reg odr_avl[ST_LSM6DS3_ODR_LIST_NUM];
} st_lsm6ds3_odr_table = {
	.addr[ST_INDIO_DEV_ACCEL] = ST_LSM6DS3_ACCEL_ODR_ADDR,
	.mask[ST_INDIO_DEV_ACCEL] = ST_LSM6DS3_ACCEL_ODR_MASK,
	.addr[ST_INDIO_DEV_GYRO] = ST_LSM6DS3_GYRO_ODR_ADDR,
	.mask[ST_INDIO_DEV_GYRO] = ST_LSM6DS3_GYRO_ODR_MASK,
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	.odr_avl[0] = { .hz = 5,
			.value = ST_LSM6DS3_ODR_13HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/5)},
	.odr_avl[1] = { .hz = 15,
			.value = ST_LSM6DS3_ODR_26HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/15)},
	.odr_avl[2] = { .hz = 50,
			.value = ST_LSM6DS3_ODR_52HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/50)},
	.odr_avl[3] = { .hz = 104,
			.value = ST_LSM6DS3_ODR_104HZ_VAL,
			/* to limit fusion odr */
			.ns = (unsigned long)(NSEC_PER_SEC/104)}, 
	.odr_avl[4] = { .hz = 208,
			.value = ST_LSM6DS3_ODR_208HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
	.odr_avl[5] = { .hz = 416,
			.value = ST_LSM6DS3_ODR_416HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/416)},
	.odr_avl[6] = { .hz = 1660,
			.value = ST_LSM6DS3_ODR_1660HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
	.odr_avl[7] = { .hz = 6660,
			.value = ST_LSM6DS3_ODR_6660HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
			
#else
	.odr_avl[0] = { .hz = 13,
			.value = ST_LSM6DS3_ODR_13HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/13)},
	.odr_avl[1] = { .hz = 26,
			.value = ST_LSM6DS3_ODR_26HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/26)},
	.odr_avl[2] = { .hz = 52,
			.value = ST_LSM6DS3_ODR_52HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/52)},
	.odr_avl[3] = { .hz = 104,
			.value = ST_LSM6DS3_ODR_104HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/104)},
	.odr_avl[4] = { .hz = 208,
			.value = ST_LSM6DS3_ODR_208HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
	.odr_avl[6] = { .hz = 1660,
			.value = ST_LSM6DS3_ODR_1660HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
	.odr_avl[7] = { .hz = 6660,
			.value = ST_LSM6DS3_ODR_6660HZ_VAL,
			.ns = (unsigned long)(NSEC_PER_SEC/208)},
			
#endif
};

struct st_lsm6ds3_fs_reg {
	unsigned int gain;
	u8 value;
};

static struct st_lsm6ds3_fs_table {
	u8 addr;
	u8 mask;
	struct st_lsm6ds3_fs_reg fs_avl[ST_LSM6DS3_FS_LIST_NUM];
} st_lsm6ds3_fs_table[ST_INDIO_DEV_NUM] = {
	[ST_INDIO_DEV_ACCEL] = {
		.addr = ST_LSM6DS3_ACCEL_FS_ADDR,
		.mask = ST_LSM6DS3_ACCEL_FS_MASK,
		.fs_avl[0] = { .gain = ST_LSM6DS3_ACCEL_FS_2G_GAIN,
					.value = ST_LSM6DS3_ACCEL_FS_2G_VAL },
		.fs_avl[1] = { .gain = ST_LSM6DS3_ACCEL_FS_4G_GAIN,
					.value = ST_LSM6DS3_ACCEL_FS_4G_VAL },
		.fs_avl[2] = { .gain = ST_LSM6DS3_ACCEL_FS_8G_GAIN,
					.value = ST_LSM6DS3_ACCEL_FS_8G_VAL },
		.fs_avl[3] = { .gain = ST_LSM6DS3_ACCEL_FS_16G_GAIN,
					.value = ST_LSM6DS3_ACCEL_FS_16G_VAL },
	},
	[ST_INDIO_DEV_GYRO] = {
		.addr = ST_LSM6DS3_GYRO_FS_ADDR,
		.mask = ST_LSM6DS3_GYRO_FS_MASK,
		.fs_avl[0] = { .gain = ST_LSM6DS3_GYRO_FS_245_GAIN,
					.value = ST_LSM6DS3_GYRO_FS_245_VAL },
		.fs_avl[1] = { .gain = ST_LSM6DS3_GYRO_FS_500_GAIN,
					.value = ST_LSM6DS3_GYRO_FS_500_VAL },
		.fs_avl[2] = { .gain = ST_LSM6DS3_GYRO_FS_1000_GAIN,
					.value = ST_LSM6DS3_GYRO_FS_1000_VAL },
		.fs_avl[3] = { .gain = ST_LSM6DS3_GYRO_FS_2000_GAIN,
					.value = ST_LSM6DS3_GYRO_FS_2000_VAL },
	}
};

static const struct iio_chan_spec st_lsm6ds3_accel_ch[] = {
	ST_LSM6DS3_LSM_CHANNELS(IIO_ACCEL, 0, IIO_MOD_X, IIO_LE,
					16, ST_LSM6DS3_ACCEL_OUT_X_L_ADDR),
	ST_LSM6DS3_LSM_CHANNELS(IIO_ACCEL, 1, IIO_MOD_Y, IIO_LE,
					16, ST_LSM6DS3_ACCEL_OUT_Y_L_ADDR),
	ST_LSM6DS3_LSM_CHANNELS(IIO_ACCEL, 2, IIO_MOD_Z, IIO_LE,
					16, ST_LSM6DS3_ACCEL_OUT_Z_L_ADDR),
	IIO_CHAN_SOFT_TIMESTAMP(3)
};

static const struct iio_chan_spec st_lsm6ds3_gyro_ch[] = {
	ST_LSM6DS3_LSM_CHANNELS(IIO_ANGL_VEL, 0, IIO_MOD_X, IIO_LE,
					16, ST_LSM6DS3_GYRO_OUT_X_L_ADDR),
	ST_LSM6DS3_LSM_CHANNELS(IIO_ANGL_VEL, 1, IIO_MOD_Y, IIO_LE,
					16, ST_LSM6DS3_GYRO_OUT_Y_L_ADDR),
	ST_LSM6DS3_LSM_CHANNELS(IIO_ANGL_VEL, 2, IIO_MOD_Z, IIO_LE,
					16, ST_LSM6DS3_GYRO_OUT_Z_L_ADDR),
	IIO_CHAN_SOFT_TIMESTAMP(3)
};

static const struct iio_chan_spec st_lsm6ds3_sign_motion_ch[] = {
	{
		.type = IIO_SIGN_MOTION,
		.channel = 0,
		.modified = 0,
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	},
	IIO_CHAN_SOFT_TIMESTAMP(1)
};

static const struct iio_chan_spec st_lsm6ds3_step_c_ch[] = {
	{
		.type = IIO_STEP_COUNTER,
		.modified = 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.address = ST_LSM6DS3_STEP_COUNTER_OUT_L_ADDR,
		.scan_type = {
			.sign = 'u',
			.realbits = 16,
			.storagebits = 16,
			.endianness = IIO_LE,
		},
	},
	IIO_CHAN_SOFT_TIMESTAMP(1)
};

static const struct iio_chan_spec st_lsm6ds3_step_d_ch[] = {
	{
		.type = IIO_STEP_DETECTOR,
		.channel = 0,
		.modified = 0,
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	},
	IIO_CHAN_SOFT_TIMESTAMP(1)
};

static const struct iio_chan_spec st_lsm6ds3_tilt_ch[] = {
	{
		.type = IIO_TILT,
		.channel = 0,
		.modified = 0,
		.event_mask = IIO_EV_BIT(IIO_EV_TYPE_THRESH, IIO_EV_DIR_RISING),
	},
	IIO_CHAN_SOFT_TIMESTAMP(1)
};

int st_acc_open_calibration(struct lsm6ds3_data *cdata)
{
	int ret = 0;
	mm_segment_t old_fs;
	struct file *cal_filp = NULL;

	pr_info("%s\n", __func__);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY,
		S_IRUGO | S_IWUSR | S_IWGRP);
	if (IS_ERR(cal_filp)) {
		set_fs(old_fs);
		ret = PTR_ERR(cal_filp);

		cdata->accel_cal_data[0] = 0;
		cdata->accel_cal_data[1] = 0;
		cdata->accel_cal_data[2] = 0;

		pr_info("%s - No Calibration\n", __func__);

		return ret;
	}

	ret = cal_filp->f_op->read(cal_filp, (char *)&cdata->accel_cal_data,
		3 * sizeof(s16), &cal_filp->f_pos);
	if (ret != 3 * sizeof(s16)) {
		pr_err("%s: - Can't read the cal data\n", __func__);
		ret = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	pr_info("%s open accel calibration %d, %d, %d\n", __func__,
		cdata->accel_cal_data[0], cdata->accel_cal_data[1],
		cdata->accel_cal_data[2]);

	if ((cdata->accel_cal_data[0] == 0) && (cdata->accel_cal_data[1] == 0)
		&& (cdata->accel_cal_data[2] == 0))
		return -EIO;

	return ret;
}
EXPORT_SYMBOL(st_acc_open_calibration);

int st_gyro_open_calibration(struct lsm6ds3_data *cdata)
{
	int ret = 0;
	mm_segment_t old_fs;
	struct file *cal_filp = NULL;

	pr_info("%s\n", __func__);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(GYRO_CALIBRATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		set_fs(old_fs);
		ret = PTR_ERR(cal_filp);

		cdata->gyro_cal_data[0] = 0;
		cdata->gyro_cal_data[1] = 0;
		cdata->gyro_cal_data[2] = 0;

		pr_info("%s - No Calibration\n", __func__);

		return ret;
	}

	ret = cal_filp->f_op->read(cal_filp, (char *)&cdata->gyro_cal_data,
		3 * sizeof(s16), &cal_filp->f_pos);
	if (ret != 3 * sizeof(s16)) {
		pr_err("%s: - Can't read the cal data\n", __func__);
		ret = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	pr_info("%s open accel calibration %d, %d, %d\n", __func__,
		cdata->gyro_cal_data[0], cdata->gyro_cal_data[1],
		cdata->gyro_cal_data[2]);

	return ret;
}
EXPORT_SYMBOL(st_gyro_open_calibration);

int st_lsm6ds3_write_data_with_mask(struct lsm6ds3_data *common_data,
						u8 reg_addr, u8 mask, u8 data)
{
	int err;
	u8 new_data = 0x00;

	err = common_data->tf->read(&common_data->tb,
				common_data->dev, reg_addr, 1, &new_data);
	if (err < 0)
		return err;

	new_data = ((new_data & (~mask)) | ((data << __ffs(mask)) & mask));
	err = common_data->tf->write(&common_data->tb,
				common_data->dev, reg_addr, 1, &new_data);

	return err;
}
EXPORT_SYMBOL(st_lsm6ds3_write_data_with_mask);

int st_lsm6ds3_set_drdy_irq(struct lsm6ds3_sensor_data *sdata, bool state)
{
	u8 reg_addr, mask, value;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	struct lsm6ds3_sensor_data *sd;
#endif

	if (state)
		value = ST_LSM6DS3_EN_BIT;
	else
		value = ST_LSM6DS3_DIS_BIT;

	if (sdata->cdata->drdy_int_pin == 1) {
		if (sdata->sindex == ST_INDIO_DEV_TILT)
			reg_addr = ST_LSM6DS3_MD1_ADDR;
		else
			reg_addr = ST_LSM6DS3_INT1_ADDR;
	} else {
		if (sdata->sindex == ST_INDIO_DEV_TILT)
			reg_addr = ST_LSM6DS3_MD2_ADDR;
		else
			reg_addr = ST_LSM6DS3_INT2_ADDR;
	}

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		mask = ST_LSM6DS3_ACCEL_DRDY_IRQ_MASK;
		break;
	case ST_INDIO_DEV_GYRO:
		mask = ST_LSM6DS3_GYRO_DRDY_IRQ_MASK;
		break;
	case ST_INDIO_DEV_SIGN_MOTION:
		if (sdata->cdata->patch_feature)
			mask = ST_LSM6DS3_SIGN_MOTION_DRDY_IRQ_MASK;
		else {
			if ((sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_STEP_COUNTER)) ||
						(sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_STEP_DETECTOR)))
				return 0;

			mask = ST_LSM6DS3_STEP_COUNTER_DRDY_IRQ_MASK;
		}
		break;
	case ST_INDIO_DEV_STEP_COUNTER:
		if (sdata->cdata->patch_feature) {
			if (sdata->cdata->sensors_enabled &
					(1 << ST_INDIO_DEV_STEP_DETECTOR))
				return 0;
		} else {
			if ((sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_SIGN_MOTION)) ||
						(sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_STEP_DETECTOR)))
				return 0;
		}

		mask = ST_LSM6DS3_STEP_COUNTER_DRDY_IRQ_MASK;
		break;
	case ST_INDIO_DEV_STEP_DETECTOR:
		if (sdata->cdata->patch_feature) {
			if (sdata->cdata->sensors_enabled &
					(1 << ST_INDIO_DEV_STEP_COUNTER))
				return 0;
		} else {
			if ((sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_SIGN_MOTION)) ||
						(sdata->cdata->sensors_enabled &
						(1 << ST_INDIO_DEV_STEP_COUNTER)))
				return 0;
		}

		mask = ST_LSM6DS3_STEP_COUNTER_DRDY_IRQ_MASK;
		break;
	case ST_INDIO_DEV_TILT:
		mask = ST_LSM6DS3_TILT_DRDY_IRQ_MASK;
		break;
	default:
		return -EINVAL;
	}

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	dev_info(sdata->cdata->dev, "%s sensors_enabled %d (%x)...\n", __func__, sdata->sindex, sdata->cdata->sensors_enabled);
	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
	case ST_INDIO_DEV_SIGN_MOTION:
	case ST_INDIO_DEV_STEP_COUNTER:
	case ST_INDIO_DEV_STEP_DETECTOR:
	case ST_INDIO_DEV_TILT:
		mask = (1 << ST_INDIO_DEV_ACCEL) | (1 << ST_INDIO_DEV_SIGN_MOTION)
					| (1 << ST_INDIO_DEV_STEP_COUNTER) | (1 << ST_INDIO_DEV_STEP_DETECTOR)
					| (1 << ST_INDIO_DEV_TILT);
		sd = iio_priv(sdata->cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
		if (state) {
			if ((sdata->cdata->sensors_enabled & mask) && !sd->hr_timer_en) {
				hrtimer_start(&sd->hr_timer, sd->ktime, HRTIMER_MODE_REL);
				sd->hr_timer_en = 1;
				dev_info(sdata->cdata->dev, "%s HRTimer start (%d)...\n", __func__, sd->sindex);
			}
		}
		else {
			if (!(sdata->cdata->sensors_enabled & mask)) {
				hrtimer_cancel(&sd->hr_timer);
				sd->hr_timer_en = 0;
				dev_info(sdata->cdata->dev, "%s HRTimer cancel (%d)...\n", __func__, sd->sindex);
			}
		}
		break;
	case ST_INDIO_DEV_GYRO:
		sd = iio_priv(sdata->cdata->indio_dev[ST_INDIO_DEV_GYRO]);
		if (state) {
			hrtimer_start(&sd->hr_timer, sd->ktime, HRTIMER_MODE_REL);
			sd->hr_timer_en = 1;
			dev_info(sdata->cdata->dev, "%s HRTimer start (%d)...\n", __func__, sd->sindex);
		}
		else {
			hrtimer_cancel(&sd->hr_timer);
			sd->hr_timer_en = 0;
			dev_info(sdata->cdata->dev, "%s HRTimer cancel (%d)...\n", __func__, sd->sindex);
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
#else
	return st_lsm6ds3_write_data_with_mask(sdata->cdata,
							reg_addr, mask, value);
#endif
}
EXPORT_SYMBOL(st_lsm6ds3_set_drdy_irq);

int st_lsm6ds3_set_axis_enable(struct lsm6ds3_sensor_data *sdata, u8 value)
{
	u8 reg_addr;

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		reg_addr = ST_LSM6DS3_ACCEL_AXIS_EN_ADDR;
		break;
	case ST_INDIO_DEV_GYRO:
		reg_addr = ST_LSM6DS3_GYRO_AXIS_EN_ADDR;
		break;
	default:
		return 0;
	}

	return st_lsm6ds3_write_data_with_mask(sdata->cdata,
				reg_addr, ST_LSM6DS3_AXIS_EN_MASK, value);
}
EXPORT_SYMBOL(st_lsm6ds3_set_axis_enable);

static int st_lsm6ds3_set_extra_dependency(struct lsm6ds3_sensor_data *sdata,
								bool enable)
{
	int err;

	if (!((sdata->cdata->sensors_enabled &
			ST_LSM6DS3_ACCEL_DEPENDENCY) & ~(1 << sdata->sindex))) {
		if (enable) {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[ST_INDIO_DEV_ACCEL],
				st_lsm6ds3_odr_table.mask[ST_INDIO_DEV_ACCEL],
				st_lsm6ds3_odr_table.odr_avl[0].value);
			if (err < 0)
				return err;
		} else {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[ST_INDIO_DEV_ACCEL],
				st_lsm6ds3_odr_table.mask[ST_INDIO_DEV_ACCEL],
				ST_LSM6DS3_ODR_POWER_OFF_VAL);
			if (err < 0)
				return err;
		}
	}

	if (!((sdata->cdata->sensors_enabled &
			ST_LSM6DS3_EXTRA_DEPENDENCY) & ~(1 << sdata->sindex))) {
		if (enable) {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
							ST_LSM6DS3_FUNC_EN_ADDR,
							ST_LSM6DS3_FUNC_EN_MASK,
							ST_LSM6DS3_EN_BIT);
			if (err < 0)
				return err;
		} else {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
							ST_LSM6DS3_FUNC_EN_ADDR,
							ST_LSM6DS3_FUNC_EN_MASK,
							ST_LSM6DS3_DIS_BIT);
			if (err < 0)
				return err;
		}
	}
	return 0;
}

static int st_lsm6ds3_enable_step_c(struct lsm6ds3_sensor_data *sdata,
								bool enable)
{
	int err;
	u8 value = ST_LSM6DS3_DIS_BIT;

	if ((sdata->cdata->sensors_enabled & ~(1 << sdata->sindex)) &
					ST_LSM6DS3_STEP_COUNTER_DEPENDENCY)
		return 0;

	if (enable)
		value = ST_LSM6DS3_EN_BIT;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
						ST_LSM6DS3_STEP_COUNTER_EN_ADDR,
						ST_LSM6DS3_STEP_COUNTER_EN_MASK,
						value);
	if (err < 0)
		return err;

	err = st_lsm6ds3_set_extra_dependency(sdata, enable);
	if (err < 0)
		return err;

	return 0;
}

static int st_lsm6ds3_enable_sensors(struct lsm6ds3_sensor_data *sdata)
{
	int err, i;

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
	case ST_INDIO_DEV_GYRO:
		for (i = 0; i < ST_LSM6DS3_ODR_LIST_NUM; i++) {
			if (st_lsm6ds3_odr_table.odr_avl[i].hz == sdata->c_odr)
				break;
		}
		if (i == ST_LSM6DS3_ODR_LIST_NUM)
			return -EINVAL;

		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[sdata->sindex],
				st_lsm6ds3_odr_table.mask[sdata->sindex],
				st_lsm6ds3_odr_table.odr_avl[i].value);
		if (err < 0)
			return err;

		sdata->c_odr = st_lsm6ds3_odr_table.odr_avl[i].hz;

		if (sdata->sindex == ST_INDIO_DEV_GYRO)
			msleep(ST_LSM6DS3_GYRO_READY_TIME_FROM_PD_MS);
		else
			msleep(ST_LSM6DS3_ACCEL_READY_TIME_FROM_PD_MS);

		break;
	case ST_INDIO_DEV_SIGN_MOTION:
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
						ST_LSM6DS3_SIGN_MOTION_EN_ADDR,
						ST_LSM6DS3_SIGN_MOTION_EN_MASK,
						ST_LSM6DS3_EN_BIT);
		if (err < 0)
			return err;

		if (sdata->cdata->patch_feature) {
			err = st_lsm6ds3_enable_step_c(sdata, true);
			if (err < 0)
				return err;
		} else {
			if ((sdata->cdata->sensors_enabled & ~(1 << sdata->sindex)) &
						ST_LSM6DS3_STEP_COUNTER_DEPENDENCY) {
				err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
							ST_LSM6DS3_STEP_COUNTER_EN_ADDR,
							ST_LSM6DS3_STEP_COUNTER_EN_MASK,
							ST_LSM6DS3_DIS_BIT);
				if (err < 0)
					return err;

				err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
							ST_LSM6DS3_STEP_COUNTER_EN_ADDR,
							ST_LSM6DS3_STEP_COUNTER_EN_MASK,
							ST_LSM6DS3_EN_BIT);
				if (err < 0)
					return err;
			} else {
				err = st_lsm6ds3_enable_step_c(sdata, true);
				if (err < 0)
					return err;
			}
		}

		break;
	case ST_INDIO_DEV_STEP_COUNTER:
	case ST_INDIO_DEV_STEP_DETECTOR:
		err = st_lsm6ds3_enable_step_c(sdata, true);
		if (err < 0)
			return err;

		break;
	case ST_INDIO_DEV_TILT:
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_TILT_EN_ADDR,
					ST_LSM6DS3_TILT_EN_MASK,
					ST_LSM6DS3_EN_BIT);
		if (err < 0)
			return err;

		err = st_lsm6ds3_set_extra_dependency(sdata, true);
		if (err < 0)
			return err;

		break;
	default:
		return -EINVAL;
	}

	sdata->cdata->sensors_enabled |= (1 << sdata->sindex);

	dev_info(sdata->cdata->dev, "%s sensors_enabled %d (%x)...\n", __func__, sdata->sindex, sdata->cdata->sensors_enabled);

	return 0;
}

static int st_lsm6ds3_disable_sensors(struct lsm6ds3_sensor_data *sdata)
{
	int err;

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		if (sdata->cdata->sensors_enabled &
						ST_LSM6DS3_EXTRA_DEPENDENCY) {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[sdata->sindex],
				st_lsm6ds3_odr_table.mask[sdata->sindex],
				st_lsm6ds3_odr_table.odr_avl[0].value);
		} else {
			err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[sdata->sindex],
				st_lsm6ds3_odr_table.mask[sdata->sindex],
				ST_LSM6DS3_ODR_POWER_OFF_VAL);
		}
		if (err < 0)
			return err;

		break;
	case ST_INDIO_DEV_GYRO:
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[sdata->sindex],
				st_lsm6ds3_odr_table.mask[sdata->sindex],
				ST_LSM6DS3_ODR_POWER_OFF_VAL);
		if (err < 0)
			return err;

		break;
	case ST_INDIO_DEV_SIGN_MOTION:
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
						ST_LSM6DS3_SIGN_MOTION_EN_ADDR,
						ST_LSM6DS3_SIGN_MOTION_EN_MASK,
						ST_LSM6DS3_DIS_BIT);
		if (err < 0)
			return err;

		err = st_lsm6ds3_enable_step_c(sdata, false);
		if (err < 0)
			return err;

		break;
	case ST_INDIO_DEV_STEP_COUNTER:
	case ST_INDIO_DEV_STEP_DETECTOR:
		err = st_lsm6ds3_enable_step_c(sdata, false);
		if (err < 0)
			return err;

		break;
	case ST_INDIO_DEV_TILT:
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
							ST_LSM6DS3_TILT_EN_ADDR,
							ST_LSM6DS3_TILT_EN_MASK,
							ST_LSM6DS3_DIS_BIT);
		if (err < 0)
			return err;

		err = st_lsm6ds3_set_extra_dependency(sdata, false);
		if (err < 0)
			return err;

		break;
	default:
		return -EINVAL;
	}

	sdata->cdata->sensors_enabled &= ~(1 << sdata->sindex);

	dev_info(sdata->cdata->dev, "%s sensors_enabled %d (%x)...\n", __func__, sdata->sindex, sdata->cdata->sensors_enabled);

	return 0;
}

int st_lsm6ds3_reg_write(struct lsm6ds3_data *cdata, u8 reg_addr, int len, u8 *data)
{
	int err, i;

	for (i = 0; i < len; i++) {
		err = cdata->tf->write(&cdata->tb, cdata->dev,
						reg_addr + i, 1, &data[i]);
		if (err < 0) {
			dev_err(cdata->dev, "failed to write reg (%d).\n", *data);
			return err;
		}
	}

	return i;
}

int st_lsm6ds3_reg_read(struct lsm6ds3_data *cdata, u8 reg_addr, int len, u8 *data)
{
	int err, i;

	for (i = 0; i < len; i++) {
		err = cdata->tf->read(&cdata->tb, cdata->dev,
						reg_addr + i, 1, &data[i]);
		if (err < 0) {
			dev_err(cdata->dev, "failed to read reg (%d).\n", reg_addr + i);
			return err;
		}
	}

	return i;
}


void st_lsm6ds3_reg_dump(struct lsm6ds3_data *cdata)
{
	int err, i;
	u8 regs[10] = {0,};

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_INT1_ADDR, 2, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read int registers.\n");
	}
	for (i = 0; i < 2; i++) {
		dev_info(cdata->dev, "INT [%d]: %x\n", i, regs[i]);
	}

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_MD1_ADDR, 2, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read md registers.\n");
	}
	for (i = 0; i < 2; i++) {
		dev_info(cdata->dev, "MD [%d]: %x\n", i, regs[i]);
	}

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_FIFO_CTRL1_ADDR, 5, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read fifo registers.\n");
	}
	for (i = 0; i < 5; i++) {
		dev_info(cdata->dev, "FIFO [%d]: %x\n", i, regs[i]);
	}

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_CTRL1_ADDR, 10, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read ctrl registers.\n");
	}
	for (i = 0; i < 10; i++) {
		dev_info(cdata->dev, "CTRL [%d]: %x\n", i, regs[i]);
	}

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_THS_ADDR, 2, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read wu registers.\n");
	}
	for (i = 0; i < 2; i++) {
		dev_info(cdata->dev, "WU [%d]: %x\n", i, regs[i]);
	}

	err = st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_SRC_ADDR, 1, regs);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read wu registers.\n");
	}
	for (i = 0; i < 1; i++) {
		dev_info(cdata->dev, "WU SRC [%d]: %x\n", i, regs[i]);
	}
}

int st_lsm6ds3_set_enable(struct lsm6ds3_sensor_data *sdata, bool enable)
{
	int ret;

	if (enable)
		ret = st_lsm6ds3_enable_sensors(sdata);
	else
		ret = st_lsm6ds3_disable_sensors(sdata);

	return ret;
}
EXPORT_SYMBOL(st_lsm6ds3_set_enable);

static int st_lsm6ds3_set_odr(struct lsm6ds3_sensor_data *sdata,
							unsigned int odr)
{
	int err, i;

	for (i = 0; i < ST_LSM6DS3_ODR_LIST_NUM; i++) {
		if (st_lsm6ds3_odr_table.odr_avl[i].hz == odr)
			break;
	}
	if (i == ST_LSM6DS3_ODR_LIST_NUM)
		return -EINVAL;

	if (sdata->cdata->sensors_enabled & (1 << sdata->sindex)) {
		err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
				st_lsm6ds3_odr_table.addr[sdata->sindex],
				st_lsm6ds3_odr_table.mask[sdata->sindex],
				st_lsm6ds3_odr_table.odr_avl[i].value);
		if (err < 0)
			return err;
	}

	sdata->c_odr = st_lsm6ds3_odr_table.odr_avl[i].hz;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	// HRTIMER
	sdata->ktime = ktime_set(0, st_lsm6ds3_odr_table.odr_avl[i].ns);

	dev_info(sdata->cdata->dev, "%s (%d) hz = %d, ns = %lld\n",
						__func__, sdata->sindex, sdata->c_odr, sdata->ktime.tv64);
#else
	dev_info(sdata->cdata->dev, "%s (%d) hz = %d\n",
						__func__, sdata->sindex, sdata->c_odr);
#endif

	sdata->cdata->odr = sdata->c_odr;
	return 0;
}

static int st_lsm6ds3_set_fs(struct lsm6ds3_sensor_data *sdata,
							unsigned int gain)
{
	int err, i;

	for (i = 0; i < ST_LSM6DS3_FS_LIST_NUM; i++) {
		if (st_lsm6ds3_fs_table[sdata->sindex].fs_avl[i].gain == gain)
			break;
	}
	if (i == ST_LSM6DS3_FS_LIST_NUM)
		return -EINVAL;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
			st_lsm6ds3_fs_table[sdata->sindex].addr,
			st_lsm6ds3_fs_table[sdata->sindex].mask,
			st_lsm6ds3_fs_table[sdata->sindex].fs_avl[i].value);
	if (err < 0)
		return err;

	sdata->c_gain = gain;

	dev_info(sdata->cdata->dev, "%s (%d) fs = %d\n",
						__func__, sdata->sindex, st_lsm6ds3_fs_table[sdata->sindex].fs_avl[i].value);

	return 0;
}

static int st_lsm6ds3_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *ch, int *val,
							int *val2, long mask)
{
	int err;
	u8 outdata[ST_LSM6DS3_BYTE_FOR_CHANNEL];
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&indio_dev->mlock);

		if (indio_dev->currentmode == INDIO_BUFFER_TRIGGERED) {
			mutex_unlock(&indio_dev->mlock);
			return -EBUSY;
		}

		err = st_lsm6ds3_set_enable(sdata, true);
		if (err < 0) {
			mutex_unlock(&indio_dev->mlock);
			return -EBUSY;
		}

		err = sdata->cdata->tf->read(&sdata->cdata->tb,
					sdata->cdata->dev, ch->address,
					ST_LSM6DS3_BYTE_FOR_CHANNEL, outdata);
		if (err < 0) {
			mutex_unlock(&indio_dev->mlock);
			return err;
		}

		*val = (s16)get_unaligned_le16(outdata);
		*val = *val >> ch->scan_type.shift;

		err = st_lsm6ds3_set_enable(sdata, false);

		mutex_unlock(&indio_dev->mlock);

		return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
		*val = 0;
		*val2 = sdata->c_gain;
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		return -EINVAL;
	}

	return 0;
}

static int st_lsm6ds3_write_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan, int val, int val2, long mask)
{
	int err;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_SCALE:
		mutex_lock(&indio_dev->mlock);
		err = st_lsm6ds3_set_fs(sdata, val2);
		mutex_unlock(&indio_dev->mlock);
		break;
	default:
		return -EINVAL;
	}

	return err;
}

static int st_lsm6ds3_custom_data(struct lsm6ds3_data *cdata)
{
	int err;
	u8 data = 0x00;
	const struct firmware *fw;

	err = request_firmware(&fw, ST_LSM6DS3_DATA_FW, cdata->dev);
	if (err < 0)
		return err;

	err = cdata->tf->write(&cdata->tb, cdata->dev,
				ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR, 1, &data);
	if (err < 0)
		goto realease_firmware;

	err = st_lsm6ds3_write_data_with_mask(cdata,
					ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
					ST_LSM6DS3_FUNC_CFG_ACCESS_MASK,
					ST_LSM6DS3_EN_BIT);
	if (err < 0)
		goto realease_firmware;

	err = cdata->tf->write(&cdata->tb, cdata->dev,
				ST_LSM6DS3_FUNC_CFG_START1_ADDR, 1, &data);
	if (err < 0)
		goto realease_firmware;

	err = cdata->tf->write(&cdata->tb, cdata->dev,
				ST_LSM6DS3_FUNC_CFG_START2_ADDR, 1, &data);
	if (err < 0)
		goto realease_firmware;

	err = cdata->tf->write(&cdata->tb, cdata->dev,
					ST_LSM6DS3_FUNC_CFG_DATA_WRITE_ADDR,
					fw->size, (u8 *)fw->data);
	if (err < 0)
		goto realease_firmware;

	err = st_lsm6ds3_write_data_with_mask(cdata,
					ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
					ST_LSM6DS3_FUNC_CFG_ACCESS_MASK,
					ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		goto realease_firmware;

	err = st_lsm6ds3_write_data_with_mask(cdata,
					ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
					ST_LSM6DS3_FUNC_CFG_ACCESS_MASK2,
					ST_LSM6DS3_EN_BIT);

realease_firmware:
	release_firmware(fw);
	return err < 0 ? err : 0;
}

static enum hrtimer_restart lsm6ds3_hrtimer_callback(struct hrtimer *timer)
{
	ktime_t now;
	struct lsm6ds3_data *cdata;
	struct lsm6ds3_steps *steps;
	struct lsm6ds3_sensor_data *sdata;

	steps = container_of(timer, struct lsm6ds3_steps, hr_timer);
	cdata = container_of(steps, struct lsm6ds3_data, step_c);
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]);

	now = hrtimer_cb_get_time(timer);
	hrtimer_forward(timer, now, steps->ktime);

	if (steps->new_steps) {
		memcpy(sdata->buffer_data,
			(u8 *)&steps->steps, ST_LSM6DS3_BYTE_FOR_CHANNEL);

		if (cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]->scan_timestamp)
			*(s64 *)((u8 *)sdata->buffer_data +
				ALIGN(ST_LSM6DS3_BYTE_FOR_CHANNEL,
				sizeof(s64))) = steps->last_step_timestamp;

		iio_push_to_buffers(cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER],
							sdata->buffer_data);
		steps->new_steps = false;
	}

	return HRTIMER_RESTART;
}

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
static enum hrtimer_restart lsm6ds3_hrtimer_callback_acc_trigger(struct hrtimer *timer)
{
	struct lsm6ds3_sensor_data *sdata;

	sdata = container_of(timer, struct lsm6ds3_sensor_data, hr_timer);

	hrtimer_forward_now(timer, sdata->ktime);

	if (sdata->sindex == ST_INDIO_DEV_ACCEL) {
		queue_work(sdata->tmr_wq, &sdata->tmr_work);
	}

	return HRTIMER_RESTART;
}

static enum hrtimer_restart lsm6ds3_hrtimer_callback_gyr_trigger(struct hrtimer *timer)
{
	struct lsm6ds3_sensor_data *sdata;

	sdata = container_of(timer, struct lsm6ds3_sensor_data, hr_timer);

	hrtimer_forward_now(timer, sdata->ktime);

	if (sdata->sindex == ST_INDIO_DEV_GYRO) {
		queue_work(sdata->tmr_wq, &sdata->tmr_work);
	}

	return HRTIMER_RESTART;
}
#endif

#if (LSM6DS3_SMART_ALERT> 0)
static void st_lsm6ds3_sa_irq_work(struct work_struct *work)
{
	struct lsm6ds3_data *cdata;
	u8 val;

	cdata = container_of((struct delayed_work *)work, struct lsm6ds3_data, sa_irq_work);

	st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_SRC_ADDR, 1, &val);
	dev_info(cdata->dev, "%s run...%x\n", __func__, val);

	if (cdata->drdy_int_pin == 1) {
		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_MD1_ADDR, 1, &val);
		val &= ~0x20;
		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_MD1_ADDR, 1, &val);
	}
	else {
		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_MD2_ADDR, 1, &val);
		val &= ~0x20;
		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_MD2_ADDR, 1, &val);
	}
}

static irqreturn_t st_lsm6ds3_sa_isr(int irq, void *private)
{
	struct lsm6ds3_data *cdata = (struct lsm6ds3_data *)private;

	pr_info("%s - reactive alert irq handler is called\n", __func__);

	cdata->sa_irq_state = 1;
	wake_lock_timeout(&cdata->sa_wake_lock, msecs_to_jiffies(2000));
	schedule_delayed_work(&cdata->sa_irq_work, msecs_to_jiffies(100));

	return IRQ_HANDLED;
}
#endif

static int st_lsm6ds3_gyro_selftest_parameters(struct lsm6ds3_data *cdata)
{
	int err, i;
	u8 orientation = 0x00, gain[3], temp_gain;
	bool found_x = false, found_y = false, found_z = false;

	err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
						ST_LSM6DS3_FUNC_CFG_REG2_MASK,
						ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FLASH_ADDR,
						ST_LSM6DS3_FLASH_MASK,
						ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
				ST_LSM6DS3_ORIENT_ADDRESS, 1, &orientation);
	if (err < 0)
		return err;

	orientation &= ST_LSM6DS3_ORIENT_MASK;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
					ST_LSM6DS3_GAIN_X_ADDRESS, 3, gain);
	if (err < 0)
		return err;

	gain[0] &= ST_LSM6DS3_GAIN_MASK;
	gain[1] &= ST_LSM6DS3_GAIN_MASK;
	gain[2] &= ST_LSM6DS3_GAIN_MASK;

	if (orientation == ST_LSM6DS3_ORIENT_MODULE1) {
		cdata->gyro_module = 1;
	}
	else if (orientation == ST_LSM6DS3_ORIENT_MODULE2) {
		cdata->gyro_module = 2;
		temp_gain = gain[1];
		gain[1] = gain[2];
		gain[2] = temp_gain;
	} else
		return -EINVAL;

	err = st_lsm6ds3_write_data_with_mask(cdata,
							ST_LSM6DS3_FLASH_ADDR,
							ST_LSM6DS3_FLASH_MASK,
							ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	for (i = 0; i < ARRAY_SIZE(st_lsm6ds3_gain_table); i++) {
		if (st_lsm6ds3_gain_table[i].code == gain[0]) {
			cdata->gyro_selftest_delta[0] =
						st_lsm6ds3_gain_table[i].value;
			found_x = true;
		}
		if (st_lsm6ds3_gain_table[i].code == gain[1]) {
			cdata->gyro_selftest_delta[1] =
						st_lsm6ds3_gain_table[i].value;
			found_y = true;
		}
		if (st_lsm6ds3_gain_table[i].code == gain[2]) {
			cdata->gyro_selftest_delta[2] =
						st_lsm6ds3_gain_table[i].value;
			found_z = true;
		}
		if (found_x && found_y && found_z)
			break;
	}
	if (i == ARRAY_SIZE(st_lsm6ds3_gain_table))
		return -EINVAL;

	return 0;
}

static int st_lsm6ds3_read_ad_feature_version(struct lsm6ds3_data *cdata)
{
	int err;
	u8 patch_mode = 0x00;

	err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
						ST_LSM6DS3_FUNC_CFG_REG2_MASK,
						ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FLASH_ADDR,
						ST_LSM6DS3_FLASH_MASK,
						ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
				ST_LSM6DS3_AD_FEATURES_ADDRESS, 1, &patch_mode);
	if (err < 0)
		return err;

	patch_mode &= ST_LSM6DS3_AD_FEATURES_MASK;

	if (patch_mode)
		cdata->patch_feature = false;
	else
		cdata->patch_feature = true;

	err = st_lsm6ds3_write_data_with_mask(cdata, ST_LSM6DS3_FLASH_ADDR,
							ST_LSM6DS3_FLASH_MASK,
							ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	return 0;
}

static int st_lsm6ds3_init_sensor(struct lsm6ds3_data *cdata)
{
	int err, i;
	u8 val;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	int j;
#endif
	struct lsm6ds3_sensor_data *sdata;

	mutex_init(&cdata->tb.buf_lock);

	cdata->step_c.ktime = ktime_set(
			ST_LSM6DS3_DEFAULT_STEP_C_MAXDR_MS / 1000,
			MS_TO_NS(ST_LSM6DS3_DEFAULT_STEP_C_MAXDR_MS % 1000));

	hrtimer_init(&cdata->step_c.hr_timer,
					CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cdata->step_c.hr_timer.function = &lsm6ds3_hrtimer_callback;

	cdata->step_c.steps = 0;
	cdata->step_c.last_step_timestamp = iio_get_time_ns();

	val = 0x01;
	st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_CTRL3_ADDR, 1, &val);

	cdata->sign_motion_event_ready = false;

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sdata = iio_priv(cdata->indio_dev[i]);

		err = st_lsm6ds3_set_enable(sdata, false);
		if (err < 0)
			return err;

#if !(LSM6DS3_HRTIMER_TRIGGER > 0)
		err = st_lsm6ds3_set_drdy_irq(sdata, false);
		if (err < 0)
			return err;
#else
		sdata->tmr_wq = NULL;
		sdata->hr_timer_en = 0;
#endif

		if ((sdata->sindex == ST_INDIO_DEV_ACCEL) ||
					(sdata->sindex == ST_INDIO_DEV_GYRO)) {
			err = st_lsm6ds3_set_fs(sdata, sdata->c_gain);
			if (err < 0)
				return err;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
			// HRTIMER
			for (j = 0; j < ST_LSM6DS3_ODR_LIST_NUM; i++) {
				if (st_lsm6ds3_odr_table.odr_avl[j].hz == sdata->c_odr)
					break;
			}
			sdata->ktime = ktime_set(0, st_lsm6ds3_odr_table.odr_avl[j].ns);

			hrtimer_init(&sdata->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
			if (sdata->sindex == ST_INDIO_DEV_ACCEL)
				sdata->hr_timer.function = &lsm6ds3_hrtimer_callback_acc_trigger;
			else
				sdata->hr_timer.function = &lsm6ds3_hrtimer_callback_gyr_trigger;

			sdata->hr_timer_en = 0;

			sdata->tmr_wq = create_workqueue(sdata->cdata->indio_dev[sdata->sindex]->name);
			if (!sdata->tmr_wq)
				return -EINVAL;

			if (sdata->sindex == ST_INDIO_DEV_ACCEL)
				INIT_WORK(&sdata->tmr_work, st_lsm6ds3_poll_work_acc);
			else
				INIT_WORK(&sdata->tmr_work, st_lsm6ds3_poll_work_gyr);

			dev_dbg(cdata->dev, "HRTimer init (%d)...\n", sdata->sindex);
#endif
		}
		else if (sdata->sindex == ST_INDIO_DEV_STEP_COUNTER) {
			sdata->c_odr = ST_LSM6DS3_DEFAULT_STEP_C_MAXDR_MS;
		}
	}

	cdata->gyro_selftest_status = 0;
	cdata->accel_selftest_status = 0;

	err = st_lsm6ds3_read_ad_feature_version(cdata);
	if (err < 0)
		return err;

	if (cdata->patch_feature) {
		err = st_lsm6ds3_gyro_selftest_parameters(cdata);
		if (err < 0)
			return err;
	}

	err = st_lsm6ds3_write_data_with_mask(cdata, ST_LSM6DS3_WU_THS_ADDR,
					0x3f, 0x3f);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata, ST_LSM6DS3_WU_DUR_ADDR,
					0x60, 0x00);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata, ST_LSM6DS3_LIR_ADDR,
					ST_LSM6DS3_LIR_MASK, ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata, ST_LSM6DS3_BDU_ADDR,
					ST_LSM6DS3_BDU_MASK, ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	if (cdata->patch_feature) {
		err = st_lsm6ds3_custom_data(cdata);
		if (err < 0)
			return err;
	}

	err = st_lsm6ds3_write_data_with_mask(cdata,
					ST_LSM6DS3_STEP_COUNTER_RES_ADDR,
					ST_LSM6DS3_STEP_COUNTER_RES_MASK,
					ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(cdata,
					ST_LSM6DS3_STEP_COUNTER_RES_ADDR,
					ST_LSM6DS3_STEP_COUNTER_RES_MASK,
					ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	if (cdata->patch_feature) {
		err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
						ST_LSM6DS3_FUNC_CFG_REG2_MASK,
						ST_LSM6DS3_EN_BIT);
		if (err < 0)
			return err;

		err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_STEP_COUNTER_THS_ADDR,
						ST_LSM6DS3_STEP_COUNTER_THS_MASK,
						ST_LSM6DS3_STEP_COUNTER_THS_VALUE);
		if (err < 0)
			return err;

		err = st_lsm6ds3_write_data_with_mask(cdata,
						ST_LSM6DS3_FUNC_CFG_ACCESS_ADDR,
						ST_LSM6DS3_FUNC_CFG_REG2_MASK,
						ST_LSM6DS3_DIS_BIT);
		if (err < 0)
			return err;
	}

	return 0;
}

static int st_lsm6ds3_set_selftest(struct lsm6ds3_sensor_data *sdata, int index)
{
	int err;
	u8 mode, mask;

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		mask = ST_LSM6DS3_SELFTEST_ACCEL_MASK;
		mode = st_lsm6ds3_selftest_table[index].accel_value;
		break;
	case ST_INDIO_DEV_GYRO:
		mask = ST_LSM6DS3_SELFTEST_GYRO_MASK;

		if (sdata->cdata->patch_feature)
			mode = st_lsm6ds3_selftest_table[0].gyro_value;
		else
			mode = st_lsm6ds3_selftest_table[index].gyro_value;

		break;
	default:
		return -EINVAL;
	}

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_SELFTEST_ADDR, mask, mode);
	if (err < 0)
		return err;

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		sdata->cdata->accel_selftest_status = index;
		break;
	case ST_INDIO_DEV_GYRO:
		sdata->cdata->gyro_selftest_status = index;
		break;
	}

	return 0;
}

static int st_lsm6ds3_acc_hw_selftest(struct lsm6ds3_sensor_data *sdata,
		s32 NOST[3], s32 ST[3], s32 DIFF_ST[3])
{
	int err;
	u8 buf[1] = {0x00,};
	s16 nOutData[3] = {0,};
	s32 i, retry;
	u8 testset_regs[10] = {0x30, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x38};

	for (i = 0; i < 3; i++) {
		NOST[i] = ST[i] = 0;
	}

	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_CTRL1_ADDR, 10, testset_regs);
	if (err < 0)
		goto XL_HW_SELF_EXIT;

//	st_lsm6ds3_reg_dump(sdata);

	mdelay(200);

	retry = ST_LSM6DS3_ACC_DA_RETRY_COUNT;
	do {
		mdelay(1);
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
		if (err < 0) {
			goto XL_HW_SELF_EXIT;
		}
		retry--;
		if (!retry) break;
	} while (!(buf[0] & 0x01));

	err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_ACCEL_OUT_X_L_ADDR, 6, (u8*)nOutData);
	if (err < 0) {
		goto XL_HW_SELF_EXIT;
	}

	for (i = 0; i < 6; i++) {
		retry = ST_LSM6DS3_ACC_DA_RETRY_COUNT;
		do {
			mdelay(1);
			err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
			if (err < 0) {
				goto XL_HW_SELF_EXIT;
			}
			retry--;
			if (!retry) break;
		} while (!(buf[0] & 0x01));

		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_ACCEL_OUT_X_L_ADDR, 6, (u8*)nOutData);
		if (err < 0) {
			goto XL_HW_SELF_EXIT;
		}

		if (i > 0) {
			NOST[0] += nOutData[0];
			NOST[1] += nOutData[1];
			NOST[2] += nOutData[2];
		}
	}

	NOST[0] /= 5;
	NOST[1] /= 5;
	NOST[2] /= 5;

	err = st_lsm6ds3_set_selftest(sdata, 1);
	if (err < 0)
		goto XL_HW_SELF_EXIT;

	mdelay(200);

	retry = ST_LSM6DS3_ACC_DA_RETRY_COUNT;
	do {
		mdelay(1);
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
		if (err < 0) {
			goto XL_HW_SELF_EXIT;
		}

		retry--;
		if (!retry) break;
	} while (!(buf[0] & 0x01));

	err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_ACCEL_OUT_X_L_ADDR, 6, (u8*)nOutData);
	if (err < 0) {
		goto XL_HW_SELF_EXIT;
	}

	for (i = 0; i < 6; i++) {
		retry = ST_LSM6DS3_ACC_DA_RETRY_COUNT;
		do {
			mdelay(1);
			err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
			if (err < 0) {
				goto XL_HW_SELF_EXIT;
			}
			retry--;
			if (!retry) break;
		} while (!(buf[0] & 0x01));

		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_ACCEL_OUT_X_L_ADDR, 6, (u8*)nOutData);
		if (err < 0) {
			goto XL_HW_SELF_EXIT;
		}

		if (i > 0) {
			ST[0] += nOutData[0];
			ST[1] += nOutData[1];
			ST[2] += nOutData[2];
		}
	}

	ST[0] /= 5;
	ST[1] /= 5;
	ST[2] /= 5;

	buf[0] = 0x00;
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_CTRL1_ADDR, 1, buf);
	if (err < 0)
		goto XL_HW_SELF_EXIT;

	err = st_lsm6ds3_set_selftest(sdata, 0);
	if (err < 0)
		goto XL_HW_SELF_EXIT;

	retry = 0;
	for (i = 0; i < 3; i++) {
		DIFF_ST[i] = ABS(ST[i] - NOST[i]);
		if ((ST_LSM6DS3_ACC_MIN_ST > DIFF_ST[i]) || (ST_LSM6DS3_ACC_MAX_ST < DIFF_ST[i])) {
			retry++;
		}
	}

	if (retry > 0) {
		return 0;
	}
	else {
		return 1;
	}

XL_HW_SELF_EXIT:

	return err;
}

static int st_lsm6ds3_gyr_fifo_test(struct lsm6ds3_sensor_data *sdata,
	s16 *zero_rate_lsb, s32 *fifo_cnt, u8 *cal_pass, u8 *fifo_pass, s16 *slot_raw, char *out_str)
{
	int err;
	u8 buf[5] = {0x00,};
	s16 nFifoDepth = (ST_LSM6DS3_FIFO_TEST_DEPTH * 3) - 1;
	bool zero_rate_read_2nd = 0;
	s16 raw[3] = {0,}, zero_rate_delta[3] = {0,}, length = 0;
	s16 data[ST_LSM6DS3_FIFO_TEST_DEPTH * 3] = {0,};
	s32 i = 0, j = 0, sum_raw[3] = {0,};
	struct file *cal_filp = NULL;
	mm_segment_t old_fs;
	pr_info("%s, start\n", __func__);
	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_CTRL4_ADDR, 0x40, ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_CTRL2_ADDR, 0xf0, ST_LSM6DS3_ODR_104HZ_VAL);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_CTRL2_ADDR, 0x0c, ST_LSM6DS3_GYRO_FS_2000_VAL);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_CTRL3_ADDR, 0x40, ST_LSM6DS3_DIS_BIT);
	if (err < 0)
		return err;

	buf[0] = (u8)(nFifoDepth & 0xff);
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_CTRL1_ADDR, 1, buf);
	if (err < 0)
		return err;

	buf[0] = (u8)((nFifoDepth >> 8) & 0x0f);
	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_FIFO_CTRL2_ADDR, 0x0f, buf[0]);
	if (err < 0)
		return err;

	buf[0] = (0x01 << 3);
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_CTRL3_ADDR, 1, buf);
	if (err < 0)
		return err;

	buf[0] = 0x00;
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_CTRL4_ADDR, 1, buf);
	if (err < 0)
		return err;

	buf[0] = (0x04 << 3) | 0x01;
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_CTRL5_ADDR, 1, buf);
	if (err < 0)
		return err;

//	st_lsm6ds3_reg_dump(sdata);

	mdelay(800);


	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_FIFO_CTRL5_ADDR, 0x07, 0x00);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_FIFO_CTRL5_ADDR, 0x07, 0x01);
	if (err < 0)
		return err;

	length = ST_LSM6DS3_FIFO_TEST_DEPTH * 3;
	while (1) {
		mdelay(20);
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_STAT1_ADDR, 4, buf);
		if (err < 0) {
			return err;
		}
		else {
			if (buf[1] & 0x80) {
				break;
			}
			else {
				if ((--length) == 0) {
					dev_err(sdata->cdata->dev, "FIFO not filled.\n");
					return -EBUSY;
				}
			}
		}
	}

	length = ST_LSM6DS3_FIFO_TEST_DEPTH * 3;
	for (i = 0; i < length; i++) {
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_OUT_L_ADDR, 2, (u8 *)(data + i));
		if (err < 0) {
			dev_err(sdata->cdata->dev, "Reading FIFO output is fail.\n");
			return err;
		}
	}

	*fifo_cnt = 0;
	/* Check fifo pass or fail */
	for (i = 0; i < length ; i += 3 ) {
		*fifo_cnt += 1;

		raw[0] = data[i];
		raw[1] = data[i + 1];
		raw[2] = data[i + 2];

		sum_raw[0] += raw[0];
		sum_raw[1] += raw[1];
		sum_raw[2] += raw[2];

		for (j = 0; j < 3; j++) {
			if (raw[j] < ST_LSM6DS3_GYR_MIN_ZRL || raw[j] > ST_LSM6DS3_GYR_MAX_ZRL) {
				slot_raw[0] = raw[0] * 7 / 100;
				slot_raw[1] = raw[1] * 7 / 100;
				slot_raw[2] = raw[2] * 7 / 100;
				*cal_pass = 0;
				*fifo_pass = 0;
				pr_info("%s, fifo fail : %d\n", __func__, *fifo_pass);
				return -EAGAIN;
			}
		}
	}
	*fifo_pass = 1;
	pr_info("%s, fifo pass : %d\n", __func__, *fifo_pass);

	for (i = 0; i < 3; i++) {
		zero_rate_lsb[i] = (sum_raw[i] + (ST_LSM6DS3_FIFO_TEST_DEPTH / 2)) / ST_LSM6DS3_FIFO_TEST_DEPTH;
	}
read_zero_rate_again:
	if (zero_rate_read_2nd == 1) {
		/* check zero_rate second time */
		zero_rate_delta[0] -= zero_rate_lsb[0];
		zero_rate_delta[1] -= zero_rate_lsb[1];
		zero_rate_delta[2] -= zero_rate_lsb[2];
		*cal_pass = 0;
		for (i = 0; i < 3; i++) {
			if (ABS(zero_rate_delta[i]) > ST_LSM6DS3_GYR_ZRL_DELTA) {
				pr_info("%s, cal fail : %d\n", __func__, *cal_pass);
				return -EAGAIN;
			}
		}
		sdata->cdata->gyro_cal_data[0] = zero_rate_lsb[0];
		sdata->cdata->gyro_cal_data[1] = zero_rate_lsb[1];
		sdata->cdata->gyro_cal_data[2] = zero_rate_lsb[2];

		/* save cal data */
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		cal_filp = filp_open(GYRO_CALIBRATION_FILE_PATH,
					O_CREAT | O_TRUNC | O_WRONLY, 0666);
		if (IS_ERR(cal_filp)) {
				pr_err("%s: Can't open calibration file\n",
					__func__);
				set_fs(old_fs);
				err = PTR_ERR(cal_filp);
				return err;
		}
		err = cal_filp->f_op->write(cal_filp,
				(char *)&sdata->cdata->gyro_cal_data, 3 * sizeof(s16),
				&cal_filp->f_pos);
		if (err != 3 * sizeof(s16)) {
			pr_err("%s: Can't write the cal data to file\n",
				__func__);
		}

		filp_close(cal_filp, current->files);
		set_fs(old_fs);

		*cal_pass = 1;
		pr_info("%s, cal pass : %d\n", __func__, *cal_pass);
	}
	else {
		/*check zero_rate first time, go to check again*/

		zero_rate_read_2nd = 1;
		sum_raw[0] = 0;
		sum_raw[1] = 0;
		sum_raw[2] = 0;
		zero_rate_delta[0] = zero_rate_lsb[0];
		zero_rate_delta[1] = zero_rate_lsb[1];
		zero_rate_delta[2] = zero_rate_lsb[2];

		goto read_zero_rate_again;
	}

	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	err = st_lsm6ds3_reg_write(sdata->cdata, ST_LSM6DS3_FIFO_CTRL5_ADDR, 1, buf);
	if (err < 0)
		return err;

	return 0;
}

static int st_lsm6ds3_gyr_hw_selftest(struct lsm6ds3_sensor_data *sdata,
		s32 NOST[3], s32 ST[3], s32 DIFF_ST[3])
{
	int err;
	u8 buf[1] = {0x00,};
	s16 nOutData[3] = {0,};
	s32 i, retry;
	u8 testset_regs[10] = {0x00, 0x5C, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x38};
	pr_info("%s, start\n", __func__);
	for (i = 0; i < 3; i++) {
		NOST[i] = ST[i] = 0;
	}

	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_CTRL1_ADDR, 10, testset_regs);
	if (err < 0)
		goto G_HW_SELF_EXIT;

//	st_lsm6ds3_reg_dump(sdata);

	mdelay(800);

	retry = ST_LSM6DS3_GYR_DA_RETRY_COUNT;
	do {
		mdelay(1);
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
		if (err < 0) {
			goto G_HW_SELF_EXIT;
		}
		retry--;
		if (!retry) break;
	} while (!(buf[0] & 0x02));

	err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_GYRO_OUT_X_L_ADDR, 6, (u8*)nOutData);
	if (err < 0) {
		goto G_HW_SELF_EXIT;
	}

	for (i = 0; i < 6; i++) {
		retry = ST_LSM6DS3_GYR_DA_RETRY_COUNT;
		do {
			mdelay(1);
			err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
			if (err < 0) {
				goto G_HW_SELF_EXIT;
			}
			retry--;
			if (!retry) break;
		} while (!(buf[0] & 0x02));

		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_GYRO_OUT_X_L_ADDR, 6, (u8*)nOutData);
		if (err < 0) {
			goto G_HW_SELF_EXIT;
		}

		if (i > 0) {
			NOST[0] += nOutData[0];
			NOST[1] += nOutData[1];
			NOST[2] += nOutData[2];
		}
	}

	NOST[0] /= 5;
	NOST[1] /= 5;
	NOST[2] /= 5;

	err = st_lsm6ds3_set_selftest(sdata, 1);
	if (err < 0)
		goto G_HW_SELF_EXIT;

	mdelay(60);

	retry = ST_LSM6DS3_GYR_DA_RETRY_COUNT;
	do {
		mdelay( 1);
		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
		if (err < 0) {
			goto G_HW_SELF_EXIT;
		}

		retry--;
		if (!retry) break;
	} while (!(buf[0] & 0x02));

	err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_GYRO_OUT_X_L_ADDR, 6, (u8*)nOutData);
	if (err < 0) {
		goto G_HW_SELF_EXIT;
	}

	for (i = 0; i < 6; i++) {
		retry = ST_LSM6DS3_GYR_DA_RETRY_COUNT;
		do {
			mdelay(1);
			err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					0x1e, 1, buf);
			if (err < 0) {
				goto G_HW_SELF_EXIT;
			}
			retry--;
			if (!retry) break;
		} while (!(buf[0] & 0x02));

		err = sdata->cdata->tf->read(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_GYRO_OUT_X_L_ADDR, 6, (u8*)nOutData);
		if (err < 0) {
			goto G_HW_SELF_EXIT;
		}

		if (i > 0) {
			ST[0] += nOutData[0];
			ST[1] += nOutData[1];
			ST[2] += nOutData[2];
			if ((sdata->cdata->patch_feature) &&
					(sdata->cdata->gyro_selftest_status > 0)) {
				if (sdata->cdata->gyro_selftest_status == 1) {
					ST[0] += sdata->cdata->gyro_selftest_delta[0];
					ST[1] += sdata->cdata->gyro_selftest_delta[1];
					ST[2] += sdata->cdata->gyro_selftest_delta[2];
				}
				else {
					ST[0] -= sdata->cdata->gyro_selftest_delta[0];
					ST[1] -= sdata->cdata->gyro_selftest_delta[1];
					ST[2] -= sdata->cdata->gyro_selftest_delta[2];
				}
			}
		}
	}

	ST[0] /= 5;
	ST[1] /= 5;
	ST[2] /= 5;

	buf[0] = 0x00;
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_CTRL2_ADDR, 1, buf);
	if (err < 0)
		goto G_HW_SELF_EXIT;

	err = st_lsm6ds3_set_selftest(sdata, 0);
	if (err < 0)
		goto G_HW_SELF_EXIT;

	retry = 0;
	for (i = 0; i < 3; i++) {
		DIFF_ST[i] = ABS(ST[i] - NOST[i]);
		if ((ST_LSM6DS3_GYR_MIN_ST > DIFF_ST[i]) || (ST_LSM6DS3_GYR_MAX_ST < DIFF_ST[i])) {
			retry++;
		}
	}

	if (retry > 0) {
		return 0;
	}
	else {
		return 1;
	}

G_HW_SELF_EXIT:

	return err;
}

static int st_lsm6ds3_selftest_run(struct lsm6ds3_sensor_data *sdata,	int test, char *out_str)
{
	int err, i;
	u8 zero[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 backup_regs1[5] = {0,}, backup_regs2[10] = {0,};
	u8 backup_regs3[2] = {0,}, backup_regs4[2] = {0,};
	u8 init_status = 0, cal_pass = 0, fifo_pass = 0;
	s16 zero_rate_data[3] = {0,}, slot_raw[3] = {0,};
	s32 NOST[3] = {0,}, ST[3] = {0,}, DIFF_ST[3] = {0,};
	s32 fifo_count = 0, hw_st_ret;

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	struct lsm6ds3_sensor_data *sd;
#endif
	pr_info("%s, start\n", __func__);
	/* check ID */
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_WAI_ADDRESS, 1, &init_status);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to read Who-Am-I register.\n");
		return err;
	}
	if (init_status != ST_LSM6DS3_WAI_EXP) {
		dev_err(sdata->cdata->dev, "Who-Am-I register value is wrong %x.\n", init_status);
		init_status = 0;
	}
	else {
		init_status = 1;
	}

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sd = iio_priv(sdata->cdata->indio_dev[i]);

		if (sd->hr_timer_en > 0) {
			hrtimer_cancel(&sdata->hr_timer);
			dev_info(sdata->cdata->dev, "%s timer cancel for %d\n", __func__, i);
		}
	}
#endif

	/* backup registers */
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_INT1_ADDR, 2, backup_regs4);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to read int registers.\n");
		goto restore_exit;
	}
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_MD1_ADDR, 2, backup_regs3);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to read md registers.\n");
		goto restore_int;
	}
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_CTRL1_ADDR, 10, backup_regs2);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to read ctrl registers.\n");
		goto restore_md;
	}
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_FIFO_CTRL1_ADDR, 5, backup_regs1);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to read fifo registers.\n");
		goto restore_ctrl;
	}

	/* disable int */
	err = st_lsm6ds3_reg_write(sdata->cdata,	ST_LSM6DS3_MD1_ADDR, 2, zero);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write 0 into MD registers.\n");
		goto restore_regs;
	}
	err = st_lsm6ds3_reg_write(sdata->cdata, ST_LSM6DS3_INT1_ADDR, 2, zero);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write 0 into INT registers.\n");
		goto restore_regs;
	}
	err = st_lsm6ds3_reg_write(sdata->cdata, ST_LSM6DS3_FIFO_CTRL1_ADDR, 5, zero);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write 0 into CTRL registers.\n");
		goto restore_regs;
	}
	zero[2] = 0x04;
	zero[8] = 0x38;
	zero[9] = 0x38;
	err = st_lsm6ds3_reg_read(sdata->cdata, ST_LSM6DS3_CTRL1_ADDR, 10, zero);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write 0 into CTRL registers.\n");
		goto restore_regs;
	}

	/* fifo test */
	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		if (test & 2) {
			hw_st_ret = st_lsm6ds3_acc_hw_selftest(sdata, NOST, ST, DIFF_ST);
			if (hw_st_ret > 0) {
				pr_info("%s, ST = 1 %d %d %d\n", __func__,
					DIFF_ST[0], DIFF_ST[1], DIFF_ST[2]);
			}
			else {
				pr_info("%s, ST = 0 %d %d %d\n", __func__,
					DIFF_ST[0], DIFF_ST[1], DIFF_ST[2]);

				goto restore_regs;
			}
		}
		break;
	case ST_INDIO_DEV_GYRO:
		if (test & 1) {
			err = st_lsm6ds3_gyr_fifo_test(sdata, zero_rate_data,
				&fifo_count, &cal_pass, &fifo_pass, slot_raw, out_str);
			if (err < 0) {
				dev_err(sdata->cdata->dev, "Gyro FIFO test fail (%d).\n", fifo_count);
				pr_info("%s, fail raw = %d,%d,%d\n", __func__, slot_raw[0], slot_raw[1], slot_raw[2]);
				goto restore_regs;
			}
		}
		if (test & 2) {
			hw_st_ret = st_lsm6ds3_gyr_hw_selftest(sdata, NOST, ST, DIFF_ST);
			zero_rate_data[0] = zero_rate_data[0] * 7 / 100;
			zero_rate_data[1] = zero_rate_data[1] * 7 / 100;
			zero_rate_data[2] = zero_rate_data[2] * 7 / 100;
			NOST[0] = NOST[0] * 7 / 100;
			NOST[1] = NOST[1] * 7 / 100;
			NOST[2] = NOST[2] * 7 / 100;
			ST[0] = ST[0] * 7 / 100;
			ST[1] = ST[1] * 7 / 100;
			ST[2] = ST[2] * 7 / 100;
			DIFF_ST[0] = DIFF_ST[0] * 7 / 100;
			DIFF_ST[1] = DIFF_ST[1] * 7 / 100;
			DIFF_ST[2] = DIFF_ST[2] * 7 / 100;

			pr_info("%s, zero rate = %d %d %d\nST = %d %d %d\nNOST = %d %d %d\nst_diff = %d %d %d\nfifo = %d cal = %d\n",
				__func__, zero_rate_data[0], zero_rate_data[1], zero_rate_data[2],
				ST[0], ST[1], ST[2], NOST[0], NOST[1], NOST[2],
				DIFF_ST[0], DIFF_ST[1], DIFF_ST[2], fifo_pass, cal_pass);
			if (hw_st_ret > 0)
				pr_info("%s, Gyro selftest pass\n", __func__);
			else {
				pr_info("%s, Gyro selftest fail\n", __func__);
				goto restore_regs;
			}
		}
		break;
	}
restore_regs:
	/* backup registers */
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_FIFO_CTRL1_ADDR, 5, backup_regs1);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write fifo registers.\n");
	}
restore_ctrl:
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_CTRL1_ADDR, 10, backup_regs2);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write ctrl registers.\n");
	}
restore_md:
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_MD1_ADDR, 2, backup_regs3);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write md registers.\n");
	}
restore_int:
	err = sdata->cdata->tf->write(&sdata->cdata->tb, sdata->cdata->dev,
					ST_LSM6DS3_INT1_ADDR, 2, backup_regs4);
	if (err < 0) {
		dev_err(sdata->cdata->dev, "failed to write int registers.\n");
	}
restore_exit:
//	st_lsm6ds3_reg_dump(sdata);

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sd = iio_priv(sdata->cdata->indio_dev[i]);

		if (sd->hr_timer_en > 0) {
			hrtimer_start(&sdata->hr_timer, sdata->ktime, HRTIMER_MODE_REL);
			dev_info(sdata->cdata->dev, "%s timer start for %d\n", __func__, i);
		}
	}
#endif

	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		if(hw_st_ret > 0)
			return snprintf(out_str, PAGE_SIZE, "1,%d,%d,%d\n",
					DIFF_ST[0], DIFF_ST[1], DIFF_ST[2]);
		else
			return snprintf(out_str, PAGE_SIZE, "0,%d,%d,%d\n",
					DIFF_ST[0], DIFF_ST[1], DIFF_ST[2]);
		break;

	case ST_INDIO_DEV_GYRO:
		if(!fifo_pass)
			return snprintf(out_str, PAGE_SIZE, "%d,%d,%d\n"
			,slot_raw[0], slot_raw[1], slot_raw[2]);
		else
			return snprintf(out_str, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
			,zero_rate_data[0], zero_rate_data[1], zero_rate_data[2]
			,NOST[0], NOST[1], NOST[2]
			,ST[0], ST[1], ST[2]
			,DIFF_ST[0], DIFF_ST[1], DIFF_ST[2], fifo_pass, cal_pass);
		break;
	}

	return 0;
}

static ssize_t st_lsm6ds3_sysfs_set_max_delivery_rate(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int err;
	bool status;
	unsigned int max_delivery_rate, time_remaining_ms;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	err = kstrtouint(buf, 10, &max_delivery_rate);
	if (err < 0)
		return -EINVAL;

	if (max_delivery_rate < 50)
		max_delivery_rate = 50;

	if (max_delivery_rate == sdata->c_odr)
		return size;

	sdata->cdata->step_c.ktime = ktime_set(max_delivery_rate / 1000,
					MS_TO_NS(max_delivery_rate % 1000));

	status = hrtimer_active(&sdata->cdata->step_c.hr_timer);
	if (status) {
		time_remaining_ms = (unsigned int)
			ktime_to_ms(hrtimer_get_remaining(
					&sdata->cdata->step_c.hr_timer));

		if (time_remaining_ms > max_delivery_rate) {
			hrtimer_cancel(&sdata->cdata->step_c.hr_timer);
			hrtimer_start(&sdata->cdata->step_c.hr_timer,
				sdata->cdata->step_c.ktime, HRTIMER_MODE_REL);
		}
	}
	sdata->c_odr = max_delivery_rate;

	return err < 0 ? err : size;
}

static ssize_t st_lsm6ds3_sysfs_get_max_delivery_rate(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));

	return sprintf(buf, "%d\n", sdata->c_odr);
}

static ssize_t st_lsm6ds3_sysfs_reset_counter(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int err;
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_STEP_COUNTER_RES_ADDR,
					ST_LSM6DS3_STEP_COUNTER_RES_MASK,
					ST_LSM6DS3_EN_BIT);
	if (err < 0)
		return err;

	err = st_lsm6ds3_write_data_with_mask(sdata->cdata,
					ST_LSM6DS3_STEP_COUNTER_RES_ADDR,
					ST_LSM6DS3_STEP_COUNTER_RES_MASK,
					ST_LSM6DS3_DIS_BIT);

	return err < 0 ? err : size;
}

static ssize_t st_lsm6ds3_sysfs_get_sampling_frequency(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));

	return sprintf(buf, "%d\n", sdata->c_odr);
}

static ssize_t st_lsm6ds3_sysfs_set_sampling_frequency(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int err;
	unsigned int odr;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	err = kstrtoint(buf, 10, &odr);
	if (err < 0)
		return err;

	mutex_lock(&indio_dev->mlock);
	err = st_lsm6ds3_set_odr(sdata, odr);
	mutex_unlock(&indio_dev->mlock);

	return err < 0 ? err : size;
}

static ssize_t st_lsm6ds3_sysfs_sampling_frequency_avail(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int i, len = 0;

	for (i = 0; i < ST_LSM6DS3_ODR_LIST_NUM; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len, "%d ",
					st_lsm6ds3_odr_table.odr_avl[i].hz);
	}
	buf[len - 1] = '\n';

	return len;
}

static ssize_t st_lsm6ds3_sysfs_scale_avail(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int i, len = 0;
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));

	for (i = 0; i < ST_LSM6DS3_FS_LIST_NUM; i++) {
		len += scnprintf(buf + len, PAGE_SIZE - len, "0.%06u ",
			st_lsm6ds3_fs_table[sdata->sindex].fs_avl[i].gain);
	}
	buf[len - 1] = '\n';

	return len;
}

static ssize_t st_lsm6ds3_sysfs_get_selftest_available(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s, %s, %s\n",
				st_lsm6ds3_selftest_table[0].string_mode,
				st_lsm6ds3_selftest_table[1].string_mode,
				st_lsm6ds3_selftest_table[2].string_mode);
}

static ssize_t st_lsm6ds3_sysfs_get_selftest_status(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 index;
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));
	pr_info("%s    %d\n", __func__, sdata->sindex);
	switch (sdata->sindex) {
	case ST_INDIO_DEV_ACCEL:
		index = sdata->cdata->accel_selftest_status;
		break;
	case ST_INDIO_DEV_GYRO:
		index = sdata->cdata->gyro_selftest_status;
		break;
	default:
		return -EINVAL;
	}

	return sprintf(buf, "%s\n",
				st_lsm6ds3_selftest_table[index].string_mode);
}

static ssize_t st_lsm6ds3_sysfs_set_selftest_status(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int err, i;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	mutex_lock(&indio_dev->mlock);
	if (indio_dev->currentmode == INDIO_BUFFER_TRIGGERED) {
		err = -EBUSY;
		goto set_selftest_status_mutex_unlock;
	}

	for (i = 0; i < ARRAY_SIZE(st_lsm6ds3_selftest_table); i++) {
		if (strncmp(buf, st_lsm6ds3_selftest_table[i].string_mode,
								size - 2) == 0)
			break;
	}
	if (i == ARRAY_SIZE(st_lsm6ds3_selftest_table)) {
		err = -EINVAL;
		goto set_selftest_status_mutex_unlock;
	}

	err = st_lsm6ds3_set_selftest(sdata, i);
	if (err < 0)
		goto set_selftest_status_mutex_unlock;

set_selftest_status_mutex_unlock:
	mutex_unlock(&indio_dev->mlock);

	return err < 0 ? err : size;
}

static ssize_t st_lsm6ds3_sysfs_get_selftest_run(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t ret;

	struct lsm6ds3_sensor_data *sdata;
	struct lsm6ds3_data *cdata;
	cdata = dev_get_drvdata(dev);
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_GYRO]);

	pr_info("%s %d", __func__, sdata->sindex);
	mutex_lock(&cdata->indio_dev[sdata->sindex]->mlock);
	ret = st_lsm6ds3_selftest_run(sdata, sdata->st_mode, buf);
	mutex_unlock(&cdata->indio_dev[sdata->sindex]->mlock);
	pr_info("%s done", __func__);

	return ret;
}

static ssize_t st_lsm6ds3_sysfs_set_selftest_run(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t size)
{
	int err;
	struct lsm6ds3_sensor_data *sdata;
	struct lsm6ds3_data *cdata;
	cdata = dev_get_drvdata(dev);
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_GYRO]);
	err = kstrtoint(buf, 10, &sdata->st_mode);
	if (err < 0)
		return err;

	return size;
}

static ssize_t st_lsm6ds3_acc_sysfs_get_selftest_run(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t ret;

	struct lsm6ds3_sensor_data *sdata;
	struct lsm6ds3_data *cdata;
	cdata = dev_get_drvdata(dev);
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);

	pr_info("%s %d", __func__, sdata->sindex);
	mutex_lock(&cdata->indio_dev[sdata->sindex]->mlock);
	ret = st_lsm6ds3_selftest_run(sdata, sdata->st_mode, buf);
	mutex_unlock(&cdata->indio_dev[sdata->sindex]->mlock);

	return ret;
}

static ssize_t st_lsm6ds3_acc_sysfs_set_selftest_run(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t size)
{
	int err;
	struct lsm6ds3_sensor_data *sdata;
	struct lsm6ds3_data *cdata;
	cdata = dev_get_drvdata(dev);
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
	err = kstrtoint(buf, 10, &sdata->st_mode);
	if (err < 0)
		return err;

	return size;
}

#if (LSM6DS3_SMART_ALERT> 0)
static ssize_t st_lsm6ds3_sysfs_get_smart_alert(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lsm6ds3_data *cdata = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", cdata->sa_irq_state);
}

static ssize_t st_lsm6ds3_sysfs_set_smart_alert(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t size)
{
	struct lsm6ds3_data *cdata = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata;

	u8 val, addr, bak;
	int err;
	unsigned int threshold = 0, fs;
	int enable = 0, factory_mode = 0, i;

	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);

	pr_info("%s - is called\n", __func__);

	if (sdata->sindex != ST_INDIO_DEV_ACCEL) {
		dev_err(cdata->dev, "%s - invalid device: %d\n", __func__, sdata->sindex);
		return -EINVAL;
	}

	if (sysfs_streq(buf, "0")) {
		enable = 0;
		factory_mode = 0;
		dev_info(cdata->dev, "%s - disable\n", __func__);
	} else if (sysfs_streq(buf, "1")) {
		enable = 1;
		factory_mode = 0;
		dev_info(cdata->dev, "%s - enable\n", __func__);
	} else if (sysfs_streq(buf, "2")) {
		enable = 1;
		factory_mode = 1;
		dev_info(cdata->dev, "%s - factory mode\n", __func__);
	} else {
		dev_err(cdata->dev, "%s - invalid value %s\n", __func__, buf);
		return -EINVAL;
	}

	if ((enable == 1) && (cdata->sa_flag == 0)) {
		cdata->sa_irq_state = 0;
		cdata->sa_flag = 1;

		if (factory_mode == 1) {
			threshold = 0;
		} else {
			st_lsm6ds3_set_odr(sdata,
				st_lsm6ds3_odr_table.odr_avl[0].hz);

			for (i = 0; i < ST_LSM6DS3_FS_LIST_NUM; i++) {
				if (st_lsm6ds3_fs_table[sdata->sindex].fs_avl[i].gain == sdata->c_gain)
					break;
			}

			switch (i) {
			case 0:
				fs = 2000;
				break;
			case 1:
				fs = 4000;
				break;
			case 2:
				fs = 8000;
				break;
			case 3:
				fs = 16000;
				break;
			default:
				return -EINVAL;
			}

			threshold = ST_LSM6DS3_SA_DYNAMIC_THRESHOLD * 64 + fs / 2;
			threshold = (threshold / fs) & 0x3f;
			dev_info(cdata->dev, "fs= %d, thr= %d[mg] -> %d\n",
								fs, ST_LSM6DS3_SA_DYNAMIC_THRESHOLD, threshold);
		}

		if (cdata->drdy_int_pin == 1) {
			addr = ST_LSM6DS3_MD1_ADDR;
		}
		else {
			addr = ST_LSM6DS3_MD2_ADDR;
		}

		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_CTRL1_ADDR, 1, &bak);
		val = bak & ~ST_LSM6DS3_ACCEL_ODR_MASK;
		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_CTRL1_ADDR, 1, &val);
		mdelay(100);
		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_SRC_ADDR, 1, &val);

		st_lsm6ds3_reg_read(cdata, addr, 1, &val);
		val |= 0x20;
		st_lsm6ds3_reg_write(cdata, addr, 1, &val);

		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_THS_ADDR, 1, &val);
		val &= ~0x3f;
		val |= threshold;
		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_WU_THS_ADDR, 1, &val);

		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_CTRL1_ADDR, 1, &bak);
		mdelay(100);
		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_SRC_ADDR, 1, &val);
		dev_info(cdata->dev, "%s - src= %x\n", __func__, val);

		enable_irq_wake(cdata->irq);
		enable_irq(cdata->irq);

		for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
			sdata = iio_priv(cdata->indio_dev[i]);
			err = st_lsm6ds3_set_enable(sdata, false);

			if (err < 0) {
				pr_err("%s - enable failed err = %d\n",
					__func__, err);
				return err;
			}
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
			if (sdata->hr_timer_en > 0) {
				hrtimer_cancel(&sdata->hr_timer);
			}
#endif
		}

		sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);

		err = st_lsm6ds3_set_enable(sdata, true);
		if (err < 0)
			return err;

		dev_info(cdata->dev, "%s - smart alert is on!\n", __func__);
	}
	else if ((enable == 0) && (cdata->sa_flag == 1)) {
		disable_irq_wake(cdata->irq);
		disable_irq_nosync(cdata->irq);
		cdata->sa_flag = 0;

		if (cdata->drdy_int_pin == 1) {
			addr = ST_LSM6DS3_MD1_ADDR;
		}
		else {
			addr = ST_LSM6DS3_MD2_ADDR;
		}

		st_lsm6ds3_reg_read(cdata, addr, 1, &val);
		val &= ~0x20;
		st_lsm6ds3_reg_write(cdata, addr, 1, &val);

		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_THS_ADDR, 1, &val);
		val &= ~0x3f;
		val |= 0x3f;
		st_lsm6ds3_reg_write(cdata, ST_LSM6DS3_WU_THS_ADDR, 1, &val);
		st_lsm6ds3_reg_read(cdata, ST_LSM6DS3_WU_SRC_ADDR, 1, &val);
		dev_info(cdata->dev, "%s - src= %x\n", __func__, val);

		for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
			sdata = iio_priv(cdata->indio_dev[i]);

			if ((sdata->sindex == ST_INDIO_DEV_ACCEL) &&
				cdata->sa_flag) {
				err = st_lsm6ds3_set_enable(sdata, false);
				if (err < 0) {
					pr_err("%s - enable failed err = %d\n",
						__func__, err);
					return err;
				}
			}
			if ((1 << sdata->sindex) & cdata->sensors_enabled) {
				err = st_lsm6ds3_set_enable(sdata, true);
				if (err < 0)
					return err;
			}
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
			if (sdata->hr_timer_en > 0) {
				hrtimer_start(&sdata->hr_timer, sdata->ktime,
					HRTIMER_MODE_REL);
			}
#endif
		}

		dev_info(cdata->dev, "%s - smart alert is off! irq = %d\n",
			__func__, cdata->sa_irq_state);
	}

	return size;
}
#endif

static ssize_t st_lsm6ds3_sysfs_get_reg_dump(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	struct lsm6ds3_data *cdata = sdata->cdata;

	st_lsm6ds3_reg_dump(cdata);

	return snprintf(buf, PAGE_SIZE, "reg_dump.\n");
}

static ST_LSM6DS3_DEV_ATTR_SAMP_FREQ();
static ST_LSM6DS3_DEV_ATTR_SAMP_FREQ_AVAIL();
static ST_LSM6DS3_DEV_ATTR_SCALE_AVAIL(in_accel_scale_available);
static ST_LSM6DS3_DEV_ATTR_SCALE_AVAIL(in_anglvel_scale_available);

static IIO_DEVICE_ATTR(reset_counter, S_IWUSR,
				NULL, st_lsm6ds3_sysfs_reset_counter, 0);

static IIO_DEVICE_ATTR(max_delivery_rate, S_IWUSR | S_IRUGO,
				st_lsm6ds3_sysfs_get_max_delivery_rate,
				st_lsm6ds3_sysfs_set_max_delivery_rate, 0);

static IIO_DEVICE_ATTR(self_test_available, S_IRUGO,
				st_lsm6ds3_sysfs_get_selftest_available,
				NULL, 0);

static IIO_DEVICE_ATTR(self_test, S_IWUSR | S_IRUGO,
				st_lsm6ds3_sysfs_get_selftest_status,
				st_lsm6ds3_sysfs_set_selftest_status, 0);

static IIO_DEVICE_ATTR(self_test_run, S_IWUSR | S_IRUGO,
				st_lsm6ds3_sysfs_get_selftest_run,
				st_lsm6ds3_sysfs_set_selftest_run, 0);

static IIO_DEVICE_ATTR(reg_dump, S_IRUGO,
				st_lsm6ds3_sysfs_get_reg_dump,
				NULL, 0);


static struct attribute *st_lsm6ds3_accel_attributes[] = {
	&iio_dev_attr_sampling_frequency_available.dev_attr.attr,
	&iio_dev_attr_in_accel_scale_available.dev_attr.attr,
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_self_test_available.dev_attr.attr,
	&iio_dev_attr_self_test.dev_attr.attr,
	&iio_dev_attr_self_test_run.dev_attr.attr,
	&iio_dev_attr_reg_dump.dev_attr.attr,
	NULL,
};

static const struct attribute_group st_lsm6ds3_accel_attribute_group = {
	.attrs = st_lsm6ds3_accel_attributes,
};

static const struct iio_info st_lsm6ds3_accel_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_accel_attribute_group,
	.read_raw = &st_lsm6ds3_read_raw,
	.write_raw = &st_lsm6ds3_write_raw,
};

static struct attribute *st_lsm6ds3_gyro_attributes[] = {
	&iio_dev_attr_sampling_frequency_available.dev_attr.attr,
	&iio_dev_attr_in_anglvel_scale_available.dev_attr.attr,
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_self_test_available.dev_attr.attr,
	&iio_dev_attr_self_test.dev_attr.attr,
	&iio_dev_attr_self_test_run.dev_attr.attr,
	NULL,
};

static const struct attribute_group st_lsm6ds3_gyro_attribute_group = {
	.attrs = st_lsm6ds3_gyro_attributes,
};

static const struct iio_info st_lsm6ds3_gyro_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_gyro_attribute_group,
	.read_raw = &st_lsm6ds3_read_raw,
	.write_raw = &st_lsm6ds3_write_raw,
};

static struct attribute *st_lsm6ds3_sign_motion_attributes[] = {
	NULL,
};

static const struct attribute_group st_lsm6ds3_sign_motion_attribute_group = {
	.attrs = st_lsm6ds3_sign_motion_attributes,
};

static const struct iio_info st_lsm6ds3_sign_motion_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_sign_motion_attribute_group,
};

static struct attribute *st_lsm6ds3_step_c_attributes[] = {
	&iio_dev_attr_reset_counter.dev_attr.attr,
	&iio_dev_attr_max_delivery_rate.dev_attr.attr,
	NULL,
};

static const struct attribute_group st_lsm6ds3_step_c_attribute_group = {
	.attrs = st_lsm6ds3_step_c_attributes,
};

static const struct iio_info st_lsm6ds3_step_c_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_step_c_attribute_group,
	.read_raw = &st_lsm6ds3_read_raw,
};

static struct attribute *st_lsm6ds3_step_d_attributes[] = {
	NULL,
};

static const struct attribute_group st_lsm6ds3_step_d_attribute_group = {
	.attrs = st_lsm6ds3_step_d_attributes,
};

static const struct iio_info st_lsm6ds3_step_d_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_step_d_attribute_group,
};

static struct attribute *st_lsm6ds3_tilt_attributes[] = {
	NULL,
};

static const struct attribute_group st_lsm6ds3_tilt_attribute_group = {
	.attrs = st_lsm6ds3_tilt_attributes,
};

static const struct iio_info st_lsm6ds3_tilt_info = {
	.driver_module = THIS_MODULE,
	.attrs = &st_lsm6ds3_tilt_attribute_group,
};

#ifdef CONFIG_IIO_TRIGGER
static const struct iio_trigger_ops st_lsm6ds3_trigger_ops = {
	.owner = THIS_MODULE,
	.set_trigger_state = ST_LSM6DS3_TRIGGER_SET_STATE,
};
#define ST_LSM6DS3_TRIGGER_OPS (&st_lsm6ds3_trigger_ops)
#else
#define ST_LSM6DS3_TRIGGER_OPS NULL
#endif

static ssize_t st_acc_raw_data_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	signed short cx, cy, cz;
	struct lsm6ds3_data *cdata;
	
	cdata = dev_get_drvdata(dev);

	cx = cdata->accel_data[0];
	cy = cdata->accel_data[1];
	cz = cdata->accel_data[2];

	return snprintf(buf, PAGE_SIZE, "%d, %d, %d\n", cx, cy, cz);
}

static ssize_t st_gyro_raw_data_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	signed short cx, cy, cz;
	struct lsm6ds3_data *cdata;
	
	cdata = dev_get_drvdata(dev);

	cx = cdata->gyro_data[0];
	cy = cdata->gyro_data[1];
	cz = cdata->gyro_data[2];

	return snprintf(buf, PAGE_SIZE, "%d, %d, %d\n", cx, cy, cz);
}

static int st_acc_do_calibrate(struct lsm6ds3_data *cdata, int enable)
{
	int sum[3] = { 0, };
	int ret = 0, cnt;
	struct file *cal_filp = NULL;
	mm_segment_t old_fs;
	struct lsm6ds3_sensor_data *sdata;
	sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);

	pr_info("%s\n", __func__);

	if (enable) {
		cdata->accel_cal_data[0] = 0;
		cdata->accel_cal_data[1] = 0;
		cdata->accel_cal_data[2] = 0;

//		ret = st_lsm6ds3_set_enable(sdata, true);
//		msleep(300);

		for (cnt = 0; cnt < CALIBRATION_DATA_AMOUNT; cnt++) {
			sum[0] += cdata->accel_data[0];
			sum[1] += cdata->accel_data[1];
			sum[2] += cdata->accel_data[2];
			msleep(20);
		}

//		ret = st_lsm6ds3_set_enable(sdata, false);

		cdata->accel_cal_data[0] = (sum[0] / CALIBRATION_DATA_AMOUNT);
		cdata->accel_cal_data[1] = (sum[1] / CALIBRATION_DATA_AMOUNT);
		cdata->accel_cal_data[2] = (sum[2] / CALIBRATION_DATA_AMOUNT);

		if (cdata->accel_cal_data[2] > 0)
			cdata->accel_cal_data[2] -= MAX_ACCEL_1G;
		else if (cdata->accel_cal_data[2] < 0)
			cdata->accel_cal_data[2] += MAX_ACCEL_1G;

	} else {
		cdata->accel_cal_data[0] = 0;
		cdata->accel_cal_data[1] = 0;
		cdata->accel_cal_data[2] = 0;
	}

	pr_info("%s - do accel calibrate %d, %d, %d\n", __func__,
		cdata->accel_cal_data[0], cdata->accel_cal_data[1], cdata->accel_cal_data[2]);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH,
		O_CREAT | O_TRUNC | O_WRONLY, S_IRUGO | S_IWUSR | S_IWGRP);
	if (IS_ERR(cal_filp)) {
		pr_err("%s - Can't open calibration file\n", __func__);
		set_fs(old_fs);
		ret = PTR_ERR(cal_filp);
		return ret;
	}

	ret = cal_filp->f_op->write(cal_filp, (char *)&cdata->accel_cal_data,
		3 * sizeof(s16), &cal_filp->f_pos);
	if (ret != 3 * sizeof(s16)) {
		pr_err("%s - Can't write the caldata to file\n", __func__);
		ret = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return ret;
}

static ssize_t st_acc_calibration_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	struct lsm6ds3_data *cdata = dev_get_drvdata(dev);

	pr_info("%s\n", __func__);

	ret = st_acc_open_calibration(cdata);
	if (ret < 0)
		pr_err("%s - calibration open failed(%d)\n", __func__, ret);

	pr_info("%s - cal data %d %d %d - ret : %d\n", __func__,
		cdata->accel_cal_data[0], cdata->accel_cal_data[1], cdata->accel_cal_data[2], ret);

	return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n", ret, cdata->accel_cal_data[0],
			cdata->accel_cal_data[1], cdata->accel_cal_data[2]);
}

static ssize_t st_acc_calibration_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	int64_t dEnable;
	struct lsm6ds3_data *cdata = dev_get_drvdata(dev);

	pr_info("%s\n", __func__);

	ret = kstrtoll(buf, 10, &dEnable);
	if (ret < 0){
		pr_err("%s - kstrtoll failed\n", __func__);
		return ret;
	}

	ret = st_acc_do_calibrate(cdata, (int)dEnable);
	if (ret < 0)
		pr_err("%s - accel calibrate failed\n", __func__);

	return size;
}


static ssize_t st_lsm6ds3_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", VENDOR_NAME);
}

static ssize_t st_lsm6ds3_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", LSM6DS3_DEV_NAME);
}

static int st_lsm6ds3_i2c_read_dev(struct lsm6ds3_data *data,
		unsigned char reg_addr, unsigned char *buf, unsigned int len)
{
	int ret, retries = 0;
	struct i2c_msg msg[2];

	msg[0].addr = data->client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg_addr;

	msg[1].addr = data->client->addr;
	msg[1].flags = 1;
	msg[1].len = len;
	msg[1].buf = buf;

	do {
		ret = i2c_transfer(data->client->adapter, msg, 2);
		if (ret >= 0)
			break;
	} while (retries++ < 2);

	if (ret < 0) {
		pr_err("[SENSOR]: %s - i2c read error %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int st_lsm6ds3_i2c_write_dev(struct lsm6ds3_data *data,
		unsigned char reg_addr, unsigned char buf)
{
	int ret, retries = 0;
	struct i2c_msg msg;
	unsigned char w_buf[2];

	w_buf[0] = reg_addr;
	w_buf[1] = buf;

	msg.addr = data->client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = (char *)w_buf;

	do {
		ret = i2c_transfer(data->client->adapter, &msg, 1);
		if (ret >= 0)
			break;
	} while (retries++ < 2);

	if (ret < 0) {
		pr_err("[SENSOR]: %s - i2c write error %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int st_lsm6ds3_set_lpf(struct lsm6ds3_data *cdata, int onoff)
{
	int ret;
	int i;
	u8 buf, bw, odr, lpf_odr;

	pr_info("%s cdata->odr = %x onoff = %d\n", __func__, cdata->odr, onoff);

	for (i = 0; i < ST_LSM6DS3_ODR_LIST_NUM; i++) {
		if(cdata->odr == st_lsm6ds3_odr_table.odr_avl[i].hz) {
			pr_info("%s odr_avl channel is %d\n", __func__, i);
			lpf_odr = st_lsm6ds3_odr_table.odr_avl[i].value;
			break;
		}
	}

	if (onoff) {
		odr = lpf_odr;
		bw = 0x00;
	} else {
		odr = ST_LSM6DS3_ODR_6660HZ_VAL;
		bw = 0x00;
	}
	/*
	 * K6DS3 CTRL1_XL(0X10) register configuration
	 * [7]ODR_XL3	[6]ODR_XL2	[5]ODR_XL1	[4]ODR_XL0
	 * [3]FS_XL1	[2]FS_XL0	[1]BW_XL1	[0]BW_XL0
	 */
	ret = st_lsm6ds3_i2c_read_dev(cdata, ST_LSM6DS3_ACCEL_ODR_ADDR, &buf, 1);
	buf = (ST_LSM6DS3_ACCEL_ODR_MASK & (odr << 4)) | \
		((~ST_LSM6DS3_ACCEL_ODR_MASK) & buf);
	ret += st_lsm6ds3_i2c_write_dev(cdata, ST_LSM6DS3_ACCEL_ODR_ADDR, buf);


	ret = st_lsm6ds3_i2c_read_dev(cdata, ST_LSM6DS3_ACCEL_BW_ADDR, &buf, 1);
	buf = (ST_LSM6DS3_ACCEL_BW_MASK & bw) | \
		((~ST_LSM6DS3_ACCEL_BW_MASK) & buf);
	ret += st_lsm6ds3_i2c_write_dev(cdata, ST_LSM6DS3_ACCEL_BW_ADDR, buf);

	return ret;
}

static ssize_t st_lsm6ds3_lowpassfilter_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct lsm6ds3_sensor_data *sdata = iio_priv(dev_get_drvdata(dev));

	if (sdata->lpf_on)
		ret = 1;
	else
		ret = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t st_lsm6ds3_lowpassfilter_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;
	int64_t dEnable;
#if 0
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
#endif
	struct lsm6ds3_data *cdata;
	cdata = dev_get_drvdata(dev);

	pr_err("%s - is called\n", __func__);

	ret = kstrtoll(buf, 10, &dEnable);
	if (ret < 0)
		pr_err("%s - kstrtoll failed\n", __func__);

	ret = st_lsm6ds3_set_lpf(cdata , dEnable);

	if (ret < 0)
		pr_err("%s - st_lsm6ds3_acc_set_lpf failed ret = %d\n",
			__func__, ret);

	return size;
}

static DEVICE_ATTR(vendor, S_IRUGO, st_lsm6ds3_vendor_show, NULL);
static DEVICE_ATTR(name, S_IRUGO, st_lsm6ds3_name_show, NULL);
static DEVICE_ATTR(lowpassfilter, S_IRUGO | S_IWUSR | S_IWGRP,
	st_lsm6ds3_lowpassfilter_show, st_lsm6ds3_lowpassfilter_store);

static struct device_attribute dev_attr_acc_raw_data =
	__ATTR(raw_data, S_IRUGO, st_acc_raw_data_show, NULL);

static struct device_attribute dev_attr_acc_self_test =
	__ATTR(selftest, S_IRUGO | S_IWUSR | S_IWGRP,
	st_lsm6ds3_acc_sysfs_get_selftest_run,
	st_lsm6ds3_acc_sysfs_set_selftest_run);

static DEVICE_ATTR(calibration, S_IRUGO | S_IWUSR | S_IWGRP,
	st_acc_calibration_show, st_acc_calibration_store);

#if (LSM6DS3_SMART_ALERT> 0)
static DEVICE_ATTR(reactive_alert, S_IWUSR | S_IRUGO,
				st_lsm6ds3_sysfs_get_smart_alert,
				st_lsm6ds3_sysfs_set_smart_alert);
#endif

static struct device_attribute *acc_sensor_attrs[] = {
	&dev_attr_acc_raw_data,
	&dev_attr_vendor,
	&dev_attr_name,
	&dev_attr_acc_self_test,
	&dev_attr_calibration,
	&dev_attr_lowpassfilter,
	&dev_attr_reactive_alert,
	NULL
};

static struct device_attribute dev_attr_gyro_raw_data =
	__ATTR(raw_data, S_IRUGO, st_gyro_raw_data_show, NULL);
static struct device_attribute dev_attr_gyro_self_test =
	__ATTR(selftest, S_IRUGO | S_IWUSR | S_IWGRP,
	st_lsm6ds3_sysfs_get_selftest_run,
	st_lsm6ds3_sysfs_set_selftest_run);

static struct device_attribute *gyro_sensor_attrs[] = {
	&dev_attr_gyro_raw_data,
	&dev_attr_vendor,
	&dev_attr_name,
	&dev_attr_gyro_self_test,
	NULL
};

void st_lsm6ds3_vdd_on(struct lsm6ds3_data *data, bool onoff)
{
	int ret;

	pr_info("%s\n", __func__);

	data->reg_vio = devm_regulator_get(&data->client->dev, "st,vio");
	if (IS_ERR(data->reg_vio)) {
		pr_err("could not get vio, %ld\n", PTR_ERR(data->reg_vio));
		ret = -ENOMEM;
		goto err_vio;
	} else if (!regulator_get_voltage(data->reg_vio)) {
		ret = regulator_set_voltage(data->reg_vio, 1800000, 1800000);
	}

	if (onoff == 1) {
		ret = regulator_enable(data->reg_vio);
		if (ret)
			pr_err("%s: error enablinig regulator st,vio\n", __func__);
	} else if (onoff == 0) {
		if (regulator_is_enabled(data->reg_vio)) {
			ret = regulator_disable(data->reg_vio);
			if (ret)
				pr_err("%s: error st,vio disabling regulator\n",__func__);
		}
	}
	msleep(30);

	devm_regulator_put(data->reg_vio);
err_vio:
	return;
}



#ifdef CONFIG_OF
static int st_lsm6ds3_parse_dt(struct lsm6ds3_data *cdata)
{
	u32 val;
	struct device_node *np;
	enum of_gpio_flags flags;

	np = cdata->dev->of_node;
	if (!np)
		return -EINVAL;

	if (!of_property_read_u32(np, "st,drdy-int-pin", &val) &&
							(val <= 2) && (val > 0))
		cdata->drdy_int_pin = (u8)val;
	else
		cdata->drdy_int_pin = 1;

	cdata->irq_gpio = of_get_named_gpio_flags(np, "st,irq_gpio", 0, &flags);
	if (cdata->irq_gpio < 0) {
		pr_err("%s : get irq_gpio(%d) error\n", __func__,
			cdata->irq_gpio);
		return -ENODEV;
	}
	cdata->irq = gpio_to_irq(cdata->irq_gpio);

	pr_info("%s - drdy : %u\n", __func__, val);
	return 0;
}
#endif /* CONFIG_OF */

int st_lsm6ds3_common_probe(struct lsm6ds3_data *cdata, int irq)
{
	int i, n, err;
	u8 wai = 0x00;
	struct lsm6ds3_sensor_data *sdata;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
					ST_LSM6DS3_WAI_ADDRESS, 1, &wai);
	if (err < 0) {
		dev_err(cdata->dev, "failed to read Who-Am-I register.\n");
		return err;
	}
	if (wai != ST_LSM6DS3_WAI_EXP) {
		dev_err(cdata->dev, "Who-Am-I value not valid.\n");
		return -ENODEV;
	}

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		cdata->indio_dev[i] = iio_device_alloc(sizeof(*sdata));
		if (cdata->indio_dev[i] == NULL) {
			err = -ENOMEM;
			goto iio_device_free;
		}
		sdata = iio_priv(cdata->indio_dev[i]);
		sdata->cdata = cdata;
		sdata->sindex = i;
		sdata->st_mode = 3;

		if (i == ST_INDIO_DEV_ACCEL) {
			sdata->c_odr = st_lsm6ds3_odr_table.odr_avl[0].hz;
			sdata->c_gain = st_lsm6ds3_fs_table[i].fs_avl[1].gain;
		}
		if (i == ST_INDIO_DEV_GYRO) {
			sdata->c_odr = st_lsm6ds3_odr_table.odr_avl[0].hz;
			sdata->c_gain = st_lsm6ds3_fs_table[i].fs_avl[3].gain;
		}
		cdata->indio_dev[i]->modes = INDIO_DIRECT_MODE;
	}

	if (irq > 0) {
#ifdef CONFIG_OF
		err = st_lsm6ds3_parse_dt(cdata);
		if (err < 0)
			goto iio_device_free;
#else /* CONFIG_OF */
		if (cdata->dev->platform_data) {
			cdata->drdy_int_pin =
				((struct st_lsm6ds3_platform_data *)
				cdata->dev->platform_data)->drdy_int_pin;

			if ((cdata->drdy_int_pin > 2) ||
						(cdata->drdy_int_pin < 1))
				cdata->drdy_int_pin = 1;
		} else
			cdata->drdy_int_pin = 1;
#endif /* CONFIG_OF */

		dev_info(cdata->dev, "driver use DRDY int pin %d\n",
							cdata->drdy_int_pin);
	}

	cdata->indio_dev[ST_INDIO_DEV_ACCEL]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "accel");
	cdata->indio_dev[ST_INDIO_DEV_ACCEL]->info = &st_lsm6ds3_accel_info;
	cdata->indio_dev[ST_INDIO_DEV_ACCEL]->channels = st_lsm6ds3_accel_ch;
	cdata->indio_dev[ST_INDIO_DEV_ACCEL]->num_channels =
						ARRAY_SIZE(st_lsm6ds3_accel_ch);

	cdata->indio_dev[ST_INDIO_DEV_GYRO]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "gyro");
	cdata->indio_dev[ST_INDIO_DEV_GYRO]->info = &st_lsm6ds3_gyro_info;
	cdata->indio_dev[ST_INDIO_DEV_GYRO]->channels = st_lsm6ds3_gyro_ch;
	cdata->indio_dev[ST_INDIO_DEV_GYRO]->num_channels =
						ARRAY_SIZE(st_lsm6ds3_gyro_ch);

	cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "sign_motion");
	cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]->info =
						&st_lsm6ds3_sign_motion_info;
	cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]->channels =
						st_lsm6ds3_sign_motion_ch;
	cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]->num_channels =
					ARRAY_SIZE(st_lsm6ds3_sign_motion_ch);

	cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "step_c");
	cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]->info =
						&st_lsm6ds3_step_c_info;
	cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]->channels =
						st_lsm6ds3_step_c_ch;
	cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]->num_channels =
					ARRAY_SIZE(st_lsm6ds3_step_c_ch);

	cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "step_d");
	cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]->info =
						&st_lsm6ds3_step_d_info;
	cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]->channels =
						st_lsm6ds3_step_d_ch;
	cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]->num_channels =
					ARRAY_SIZE(st_lsm6ds3_step_d_ch);

	cdata->indio_dev[ST_INDIO_DEV_TILT]->name =
		kasprintf(GFP_KERNEL, "%s_%s", cdata->name, "tilt");
	cdata->indio_dev[ST_INDIO_DEV_TILT]->info = &st_lsm6ds3_tilt_info;
	cdata->indio_dev[ST_INDIO_DEV_TILT]->channels = st_lsm6ds3_tilt_ch;
	cdata->indio_dev[ST_INDIO_DEV_TILT]->num_channels =
					ARRAY_SIZE(st_lsm6ds3_tilt_ch);

	st_lsm6ds3_vdd_on(cdata, 1);

	err = st_lsm6ds3_init_sensor(cdata);
	if (err < 0)
		goto iio_device_free;

	err = st_lsm6ds3_allocate_rings(cdata);
	if (err < 0)
		goto iio_device_free;

	err = st_lsm6ds3_allocate_triggers(cdata,
					ST_LSM6DS3_TRIGGER_OPS, irq);
	if (err < 0)
			goto deallocate_ring;

	for (n = 0; n < ST_INDIO_DEV_NUM; n++) {
		err = iio_device_register(cdata->indio_dev[n]);
		if (err)
			goto iio_device_unregister_and_trigger_deallocate;
	}
	
	err = sensors_register(cdata->gyro_factory_dev, cdata, gyro_sensor_attrs,
		MODULE_NAME_GYRO);
	if (err < 0) {
		pr_err("%s: cound not register acc sensor device(%d).\n",
			__func__, err);
		goto err_gyro_sensor_register_failed;
	}
	
	err = sensors_register(cdata->acc_factory_dev, cdata, acc_sensor_attrs,
		MODULE_NAME_ACC);
	if (err < 0) {
		pr_err("%s: cound not register acc sensor device(%d).\n",
			__func__, err);
		goto err_accel_sensor_register_failed;
	}
#if (LSM6DS3_SMART_ALERT > 0)
	cdata->sa_irq_state = 0;
	cdata->sa_flag = 0;

	if (cdata->irq > 0) {
		wake_lock_init(&cdata->sa_wake_lock, WAKE_LOCK_SUSPEND,
		       LSM6DS3_DEV_NAME "_sa_wake_lock");
		err = request_irq(irq, st_lsm6ds3_sa_isr,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT,
					cdata->name, cdata);
		if (err < 0) {
			dev_err(cdata->dev, "%s - can't allocate irq.\n", __func__);
			goto sa_request_irq;
		}
		disable_irq(cdata->irq);

		INIT_DELAYED_WORK(&cdata->sa_irq_work, st_lsm6ds3_sa_irq_work);

		dev_info(cdata->dev, "%s - Smart alert init (irq = %d).\n", __func__, cdata->irq);
	}
#endif

	return 0;
#if (LSM6DS3_SMART_ALERT > 0)
sa_request_irq:
	wake_lock_destroy(&cdata->sa_wake_lock);
#endif
err_accel_sensor_register_failed:
	sensors_unregister(cdata->dev, gyro_sensor_attrs);
err_gyro_sensor_register_failed:
iio_device_unregister_and_trigger_deallocate:
	for (n--; n >= 0; n--)
		iio_device_unregister(cdata->indio_dev[n]);

	if (irq > 0)
		st_lsm6ds3_deallocate_triggers(cdata, irq);
deallocate_ring:
	st_lsm6ds3_deallocate_rings(cdata);
iio_device_free:
	for (i--; i >= 0; i--)
		iio_device_free(cdata->indio_dev[i]);
	pr_info("%s - err %d\n", __func__, err);
	
	return err;
}
EXPORT_SYMBOL(st_lsm6ds3_common_probe);

void st_lsm6ds3_common_remove(struct lsm6ds3_data *cdata, int irq)
{
	int i;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	struct lsm6ds3_sensor_data *sdata;

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sdata = iio_priv(cdata->indio_dev[i]);
		if (sdata->tmr_wq != NULL) {
			destroy_workqueue(sdata->tmr_wq);
			sdata->tmr_wq = NULL;
		}
	}
#endif

	for (i = 0; i < ST_INDIO_DEV_NUM; i++)
		iio_device_unregister(cdata->indio_dev[i]);

	if (irq > 0)
		st_lsm6ds3_deallocate_triggers(cdata, irq);

	st_lsm6ds3_deallocate_rings(cdata);

	for (i = 0; i < ST_INDIO_DEV_NUM; i++)
		iio_device_free(cdata->indio_dev[i]);

#if (LSM6DS3_SMART_ALERT > 0)
	cancel_delayed_work_sync(&cdata->sa_irq_work);
	wake_lock_destroy(&cdata->sa_wake_lock);
#endif
}
EXPORT_SYMBOL(st_lsm6ds3_common_remove);

int st_lsm6ds3_common_suspend(struct lsm6ds3_data *cdata)
{
	int err, i;
	u8 tmp_sensors_enabled;
	struct lsm6ds3_sensor_data *sdata;

	pr_info("%s called\n", __func__);

	tmp_sensors_enabled = cdata->sensors_enabled;

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sdata = iio_priv(cdata->indio_dev[i]);
		err = st_lsm6ds3_set_enable(sdata, false);
		if (err < 0)
			return err;
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
		if (sdata->hr_timer_en > 0) {
			hrtimer_cancel(&sdata->hr_timer);
		}
#endif
	}

#if (LSM6DS3_SMART_ALERT > 0)
	if (cdata->sa_flag) {
		sdata = iio_priv(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
		err = st_lsm6ds3_set_enable(sdata, true);
		if (err < 0)
			return err;
	}
#endif

	cdata->sensors_enabled = tmp_sensors_enabled;

	dev_info(cdata->dev, "%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(st_lsm6ds3_common_suspend);

int st_lsm6ds3_common_resume(struct lsm6ds3_data *cdata)
{
	int err, i;
	struct lsm6ds3_sensor_data *sdata;

	pr_info("%s - Start!\n", __func__);

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		sdata = iio_priv(cdata->indio_dev[i]);

#if (LSM6DS3_SMART_ALERT > 0)
		if ((sdata->sindex == ST_INDIO_DEV_ACCEL) && cdata->sa_flag) {
			err = st_lsm6ds3_set_enable(sdata, false);
			if (err < 0)
				return err;
		}
#endif

		if ((1 << sdata->sindex) & cdata->sensors_enabled) {
			err = st_lsm6ds3_set_enable(sdata, true);
			if (err < 0)
				return err;
		}
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
		if (sdata->hr_timer_en > 0) {
			hrtimer_start(&sdata->hr_timer, sdata->ktime, HRTIMER_MODE_REL);
		}
#endif
	}
	dev_info(cdata->dev, "%s\n", __func__);

	return 0;
}
EXPORT_SYMBOL(st_lsm6ds3_common_resume);


MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics lsm6ds3 core driver");
MODULE_LICENSE("GPL v2");
