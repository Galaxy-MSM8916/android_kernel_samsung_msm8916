/*!
 * @section LICENSE
 * (C) Copyright 2014 Bosch Sensortec GmbH All Rights Reserved
 *
 * This software program is licensed subject to the GNU General
 * Public License (GPL).Version 2,June 1991,
 * available at http://www.fsf.org/copyleft/gpl.html
 *
 * @filename bma2x2.c
 * @date    2014/04/04 16:13
 * @id       "564eaab"
 * @version  2.0.1
 *
 * @brief
 * This file contains all function implementations for the BMA2X2 in linux
 */

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <linux/regulator/consumer.h>

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/string.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include <linux/sensor/sensors_core.h>
#include "bma2x2_reg.h"

#define CHIP_VENDOR		"BOSCH"
#define CHIP_NAME		"BMC150"
#define SENSOR_NAME		"accelerometer_sensor"
#define CALIBRATION_FILE_PATH	"/efs/FactoryApp/calibration_data"
#define CALIBRATION_DATA_AMOUNT         20
#define MAX_ACCEL_1G			1024
#define MAX_ACCEL_1G_FOR4G		512
#define ACCEL_LOG_TIME                  10/* 10 sec */

struct bma2x2_v {
	union {
		s16 v[3];
		struct {
			s16 x;
			s16 y;
			s16 z;
		};
	};
};


enum {
	OFF = 0,
	ON = 1
};


/*!
 * we use a typedef to hide the detail,
 * because this type might be changed
*/
struct bma2x2_data {
	struct i2c_client *bma2x2_client;
	atomic_t delay;
	atomic_t enable;
	atomic_t selftest_result;
	unsigned int chip_id;
	unsigned int fifo_count;
	unsigned char fifo_datasel;
	unsigned char mode;
	signed char sensor_type;
	struct input_dev *input;

	struct bma2x2_v value;
	struct bma2x2_v caldata;
	struct mutex value_mutex;
	struct mutex enable_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	struct work_struct irq_work;
	int IRQ;
	u64 old_timestamp;

	int bma_mode_enabled;
	atomic_t reactive_enable;
	atomic_t reactive_state;
	atomic_t factory_mode;
	int time_count;

#ifdef CONFIG_SENSORS_BMC150_VDD
	struct regulator *reg_vdd;
#endif
	struct regulator *reg_vio;
	int place;
	int acc_int1;
	int range;
	unsigned char used_bw;
};

static int bma2x2_normal_to_suspend(struct bma2x2_data *bma2x2,
				unsigned char data1, unsigned char data2);

static int bma2x2_smbus_read_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	int retries = 0;

        do {
		dummy = i2c_smbus_read_byte_data(client, reg_addr);
		if (dummy >= 0)
			break;
        } while (retries++ < 3);

        if (dummy < 0) {
		pr_err("[SENSOR]: %s - i2c read error %d\n", __func__, dummy);
		return dummy;
        }

	*data = dummy & 0x000000ff;

	return 0;
}

static int bma2x2_smbus_write_byte(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	int retries = 0;

	do {
		dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
			if (dummy >= 0)
				break;
	} while (retries++ < 3);

	if (dummy < 0) {
		pr_err("[SENSOR]: %s - i2c write error %d\n", __func__, dummy);
		return dummy;
	}

	udelay(2);
	return 0;
}

static int bma2x2_smbus_read_byte_block(struct i2c_client *client,
		unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	int retries = 0;

	do {
		dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
			if (dummy >= 0)
				break;
	} while (retries++ < 3);

	if (dummy < 0) {
		pr_err("[SENSOR]: %s - i2c read error %d\n", __func__, dummy);
		return dummy;
	}

	return 0;
}

static int bma2x2_check_chip_id(struct i2c_client *client,
					struct bma2x2_data *data)
{
	int i = 0;
	int err = 0;
	unsigned char chip_id;
	unsigned char read_count = 0;
	unsigned char bma2x2_sensor_type_count = 0;

	bma2x2_sensor_type_count =
		sizeof(sensor_type_map) / sizeof(struct bma2x2_type_map_t);

	while (read_count++ < CHECK_CHIP_ID_TIME_MAX) {
		if (bma2x2_smbus_read_byte(client, BMA2X2_CHIP_ID_REG,
							&chip_id) < 0) {
			dev_err(&client->dev, "Bosch Sensortec Device not found"
			"i2c bus read error, read chip_id:%d\n", chip_id);
			continue;
		} else {
		for (i = 0; i < bma2x2_sensor_type_count; i++) {
			if (sensor_type_map[i].chip_id == chip_id) {
				data->sensor_type =
					sensor_type_map[i].sensor_type;
				data->chip_id = chip_id;
					dev_notice(&client->dev,
					"Bosch Sensortec Device detected,"
					"HW IC name: %s\n",
					sensor_type_map[i].sensor_name);
					return err;
			}
		}
		if (i < bma2x2_sensor_type_count)
			return err;
		else {
			if (read_count == CHECK_CHIP_ID_TIME_MAX) {
				dev_err(&client->dev,
					"Failed!Bosch Sensortec Device"
					"not found, mismatch chip_id:%d\n",
					chip_id);
					err = -ENODEV;
					return err;
			}
		}
		mdelay(1);
		}
	}
	return err;
}

#ifdef CONFIG_SENSORS_BMA2X2_ENABLE_INT1
static int bma2x2_set_int1_pad_sel(struct i2c_client *client, unsigned char
		int1sel)
{
	int comres = 0;
	unsigned char data;
	unsigned char state;
	state = 0x01;
	pr_info("%s: int1sel(%d)\n", __func__, int1sel);

	switch (int1sel) {
	case 0:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_LOWG__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_LOWG,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_LOWG__REG, &data);
		break;
	case 1:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_HIGHG__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_HIGHG,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_HIGHG__REG, &data);
		break;
	case 2:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_SLOPE__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_SLOPE,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_SLOPE__REG, &data);
		break;
	case 3:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_DB_TAP__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_DB_TAP,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_DB_TAP__REG, &data);
		break;
	case 4:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_SNG_TAP__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_SNG_TAP,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_SNG_TAP__REG, &data);
		break;
	case 5:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_ORIENT__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_ORIENT,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_ORIENT__REG, &data);
		break;
	case 6:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_FLAT__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_FLAT,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_FLAT__REG, &data);
		break;
	case 7:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_EN_INT1_PAD_SLO_NO_MOT__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_EN_INT1_PAD_SLO_NO_MOT,
				state);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_EN_INT1_PAD_SLO_NO_MOT__REG, &data);
		break;

	default:
		break;
	}

	return comres;
}
#endif /* CONFIG_SENSORS_BMA2X2_ENABLE_INT1 */

static int bma2x2_set_Int_Enable(struct i2c_client *client, unsigned char
		InterruptType , unsigned char value)
{
	int comres = 0;
	unsigned char data1;

	pr_info("%s: InterruptType(%d)\n", __func__, InterruptType);

	comres = bma2x2_smbus_read_byte(client, BMA2X2_INT_ENABLE1_REG, &data1);

	value = value & 1;
	switch (InterruptType) {
	case 5:
		/* Slope X Interrupt */
		data1 = BMA2X2_SET_BITSLICE(data1, BMA2X2_EN_SLOPE_X_INT,
				value);
		break;

	case 6:
		/* Slope Y Interrupt */
		data1 = BMA2X2_SET_BITSLICE(data1, BMA2X2_EN_SLOPE_Y_INT,
				value);
		break;

	case 7:
		/* Slope Z Interrupt */
		data1 = BMA2X2_SET_BITSLICE(data1, BMA2X2_EN_SLOPE_Z_INT,
				value);
		break;

	default:
		break;
	}
	comres = bma2x2_smbus_write_byte(client, BMA2X2_INT_ENABLE1_REG,
			&data1);

	return comres;
}


static int bma2x2_get_interruptstatus1(struct i2c_client *client, unsigned char
		*intstatus)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_STATUS1_REG, &data);
	*intstatus = data;

	pr_info("%s: ACC_INT_STATUS %x\n", __func__, data);

	return comres;
}

static int bma2x2_get_slope_first(struct i2c_client *client, unsigned char
	param, unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	switch (param) {
	case 0:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_STATUS_TAP_SLOPE_REG, &data);
		data = BMA2X2_GET_BITSLICE(data, BMA2X2_SLOPE_FIRST_X);
		*intstatus = data;
		break;
	case 1:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_STATUS_TAP_SLOPE_REG, &data);
		data = BMA2X2_GET_BITSLICE(data, BMA2X2_SLOPE_FIRST_Y);
		*intstatus = data;
		break;
	case 2:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_STATUS_TAP_SLOPE_REG, &data);
		data = BMA2X2_GET_BITSLICE(data, BMA2X2_SLOPE_FIRST_Z);
		*intstatus = data;
		break;
	default:
		break;
	}

	return comres;
}

static int bma2x2_get_slope_sign(struct i2c_client *client, unsigned char
		*intstatus)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_STATUS_TAP_SLOPE_REG,
			&data);
	data = BMA2X2_GET_BITSLICE(data, BMA2X2_SLOPE_SIGN_S);
	*intstatus = data;

	return comres;
}

static int bma2x2_set_Int_Mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data;


	comres = bma2x2_smbus_read_byte(client,
			BMA2X2_INT_MODE_SEL__REG, &data);
	data = BMA2X2_SET_BITSLICE(data, BMA2X2_INT_MODE_SEL, Mode);
	comres = bma2x2_smbus_write_byte(client,
			BMA2X2_INT_MODE_SEL__REG, &data);

	return comres;
}

static int bma2x2_set_slope_threshold(struct i2c_client *client,
		unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	data = threshold;
	pr_info("%s: data[%x]\n", __func__, data);

	comres = bma2x2_smbus_write_byte(client,
			BMA2X2_SLOPE_THRES__REG, &data);

	return comres;
}

static int bma2x2_set_slope_duration(struct i2c_client *client, unsigned char
		duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client,
			BMA2X2_SLOPE_DUR__REG, &data);

	pr_info("%s: data[%x]\n", __func__, data);

	data = BMA2X2_SET_BITSLICE(data, BMA2X2_SLOPE_DUR, duration);
	comres += bma2x2_smbus_write_byte(client,
			BMA2X2_SLOPE_DUR__REG, &data);

	return comres;
}

/*!
 * brief: bma2x2 switch from normal to suspend mode
 * @param[i] bma2x2
 * @param[i] data1, write to PMU_LPW
 * @param[i] data2, write to PMU_LOW_NOSIE
 *
 * @return zero success, none-zero failed
 */
static int bma2x2_normal_to_suspend(struct bma2x2_data *bma2x2,
				unsigned char data1, unsigned char data2)
{
	unsigned char current_fifo_mode;
	unsigned char current_op_mode;
	if (bma2x2 == NULL)
		return -1;
	/* get current op mode from mode register */
	if (bma2x2_get_mode(bma2x2->bma2x2_client, &current_op_mode) < 0)
		return -1;
	/* only aimed at operatiom mode chang from normal/lpw1 mode
	 * to suspend state.
	*/
	if (current_op_mode == BMA2X2_MODE_NORMAL ||
			current_op_mode == BMA2X2_MODE_LOWPOWER1) {
		/* get current fifo mode from fifo config register */
		if (bma2x2_get_fifo_mode(bma2x2->bma2x2_client,
							&current_fifo_mode) < 0)
			return -1;
		else {
			bma2x2_smbus_write_byte(bma2x2->bma2x2_client,
					BMA2X2_LOW_NOISE_CTRL_REG, &data2);
			bma2x2_smbus_write_byte(bma2x2->bma2x2_client,
					BMA2X2_MODE_CTRL_REG, &data1);
			bma2x2_smbus_write_byte(bma2x2->bma2x2_client,
				BMA2X2_FIFO_MODE__REG, &current_fifo_mode);
			mdelay(3);
			return 0;
		}
	} else {
		bma2x2_smbus_write_byte(bma2x2->bma2x2_client,
					BMA2X2_LOW_NOISE_CTRL_REG, &data2);
		bma2x2_smbus_write_byte(bma2x2->bma2x2_client,
					BMA2X2_MODE_CTRL_REG, &data1);
		mdelay(3);
		return 0;
	}

}

static int bma2x2_open_calibration(struct bma2x2_data *data)
{
	int ret = 0;
	mm_segment_t old_fs;
	struct file *cal_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		set_fs(old_fs);
		ret = PTR_ERR(cal_filp);

		data->caldata.x = 0;
		data->caldata.y = 0;
		data->caldata.z = 0;

		pr_info("[SENSOR]: %s - No Calibration\n", __func__);

		return ret;
	}

	ret = cal_filp->f_op->read(cal_filp, (char *)&data->caldata.v,
		3 * sizeof(s16), &cal_filp->f_pos);
	if (ret != 3 * sizeof(s16)) {
		pr_err("[SENSOR] %s: - Can't read the cal data\n", __func__);
		ret = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	pr_info("[SENSOR]: open accel calibration %d, %d, %d\n",
		data->caldata.x, data->caldata.y, data->caldata.z);

	if ((data->caldata.x == 0) && (data->caldata.y == 0)
		&& (data->caldata.z == 0))
		return -EIO;

	return ret;
}

static int bma2x2_set_range(struct i2c_client *client, unsigned char Range)
{
	int comres = 0;
	unsigned char data1;

	if ((Range == 3) || (Range == 5) || (Range == 8) || (Range == 12)) {
		comres = bma2x2_smbus_read_byte(client, BMA2X2_RANGE_SEL_REG,
				&data1);
		switch (Range) {
		case BMA2X2_RANGE_2G:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_RANGE_SEL, 3);
			break;
		case BMA2X2_RANGE_4G:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_RANGE_SEL, 5);
			break;
		case BMA2X2_RANGE_8G:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_RANGE_SEL, 8);
			break;
		case BMA2X2_RANGE_16G:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_RANGE_SEL, 12);
			break;
		default:
			break;
		}
		comres += bma2x2_smbus_write_byte(client, BMA2X2_RANGE_SEL_REG,
				&data1);
	} else {
		comres = -1;
	}

	return comres;
}

static int bma2x2_set_mode(struct i2c_client *client, unsigned char mode,
						unsigned char enabled_mode)
{
	int comres = 0;
	unsigned char data1 = 0, data2 = 0;
	int ret = 0;
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	pr_info("[SENSOR] %s mode: [%d]\n", __func__, mode);

	mutex_lock(&bma2x2->mode_mutex);
	if (BMA2X2_MODE_SUSPEND == mode) {
		if (enabled_mode != BMA_ENABLED_ALL) {
			if ((bma2x2->bma_mode_enabled &
						(1<<enabled_mode)) == 0) {
				/* sensor is already closed in this mode */
				mutex_unlock(&bma2x2->mode_mutex);
				return 0;
			} else {
				bma2x2->bma_mode_enabled &= ~(1<<enabled_mode);
			}
		} else {
			/* shut down, close all and force do it*/
			bma2x2->bma_mode_enabled = 0;
		}
	} else {
		if ((bma2x2->bma_mode_enabled & (1<<enabled_mode)) == 1) {
			/* sensor is already enabled in this mode */
			mutex_unlock(&bma2x2->mode_mutex);
			return 0;
		} else {
			bma2x2->bma_mode_enabled |= (1<<enabled_mode);
		}
	}
	mutex_unlock(&bma2x2->mode_mutex);

	if (mode < 6) {
		comres = bma2x2_smbus_read_byte(client, BMA2X2_MODE_CTRL_REG,
				&data1);
		comres += bma2x2_smbus_read_byte(client,
				BMA2X2_LOW_NOISE_CTRL_REG,
				&data2);
		if (comres < 0)
			pr_info("[SENSOR] %s - i2c read error ",__func__);

		switch (mode) {
		case BMA2X2_MODE_NORMAL:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_MODE_CTRL, 0);
			data2  = BMA2X2_SET_BITSLICE(data2,
					BMA2X2_LOW_POWER_MODE, 0);
			comres = bma2x2_smbus_write_byte(client,
					BMA2X2_MODE_CTRL_REG, &data1);
			usleep_range(5000, 5100);
			comres += bma2x2_smbus_write_byte(client,
					BMA2X2_LOW_NOISE_CTRL_REG, &data2);
			if (comres < 0)
				pr_info("[SENSOR]:%s  - i2c write error ",__func__);

			ret = bma2x2_set_range(client, BMA2X2_RANGE_SET);
			if (ret < 0)
				pr_info("[SENSOR] %s - Error range setting ",__func__);
				bma2x2_open_calibration(bma2x2);
				break;
		case BMA2X2_MODE_LOWPOWER1:
			data1  = BMA2X2_SET_BITSLICE(data1,
					BMA2X2_MODE_CTRL, 2);
			data2  = BMA2X2_SET_BITSLICE(data2,
					BMA2X2_LOW_POWER_MODE, 0);
			comres += bma2x2_smbus_write_byte(client,
					BMA2X2_MODE_CTRL_REG, &data1);
			usleep_range(5000, 5100);
			comres += bma2x2_smbus_write_byte(client,
					BMA2X2_LOW_NOISE_CTRL_REG, &data2);
				break;
		case BMA2X2_MODE_SUSPEND:
			data1  = BMA2X2_SET_BITSLICE(data1,
						BMA2X2_MODE_CTRL, 4);
			data2  = BMA2X2_SET_BITSLICE(data2,
						BMA2X2_LOW_POWER_MODE, 0);
			/*aimed at anomaly resolution when switch to suspend*/
			ret = bma2x2_normal_to_suspend(bma2x2, data1, data2);
			if (ret < 0)
				pr_err("[SENSOR]: %s - Error switching to suspend\n",__func__);
				break;
		}
	} else {
		comres = -1;
		pr_err("[SENSOR]: %s - Error mode control\n", __func__);

	}
		pr_info("[SENSOR]: %s - comres: %d \n",__func__,comres);
	return comres;
}


static int bma2x2_get_mode(struct i2c_client *client, unsigned char *mode)
{
	int comres = 0;
	unsigned char data1, data2;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_MODE_CTRL_REG, &data1);
	comres = bma2x2_smbus_read_byte(client, BMA2X2_LOW_NOISE_CTRL_REG,
			&data2);

	data1  = (data1 & 0xE0) >> 5;
	data2  = (data2 & 0x40) >> 6;


	if ((data1 == 0x00) && (data2 == 0x00))
		*mode  = BMA2X2_MODE_NORMAL;
	else {
		if ((data1 == 0x02) && (data2 == 0x00)) 
			*mode  = BMA2X2_MODE_LOWPOWER1;
		else {
			if ((data1 == 0x04 || data1 == 0x06) &&
						(data2 == 0x00))
				*mode  = BMA2X2_MODE_SUSPEND;
		}
	}

	return comres;
}

static int bma2x2_set_bandwidth(struct i2c_client *client, unsigned char BW)
{
	int comres = 0;
	unsigned char data;
	int Bandwidth = 0;
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	if (atomic_read(&bma2x2->factory_mode)) {
		pr_info("%s, pass... factory_mode!\n", __func__);
		return comres;
	}

	if (BW > 7 && BW < 16) {
		switch (BW) {
		case BMA2X2_BW_7_81HZ:
			Bandwidth = BMA2X2_BW_7_81HZ;

			/*  7.81 Hz      64000 uS   */
			break;
		case BMA2X2_BW_15_63HZ:
			Bandwidth = BMA2X2_BW_15_63HZ;

			/*  15.63 Hz     32000 uS   */
			break;
		case BMA2X2_BW_31_25HZ:
			Bandwidth = BMA2X2_BW_31_25HZ;

			/*  31.25 Hz     16000 uS   */
			break;
		case BMA2X2_BW_62_50HZ:
			Bandwidth = BMA2X2_BW_62_50HZ;

			/*  62.50 Hz     8000 uS   */
			break;
		case BMA2X2_BW_125HZ:
			Bandwidth = BMA2X2_BW_125HZ;

			/*  125 Hz       4000 uS   */
			break;
		case BMA2X2_BW_250HZ:
			Bandwidth = BMA2X2_BW_250HZ;

			/*  250 Hz       2000 uS   */
			break;
		case BMA2X2_BW_500HZ:
			Bandwidth = BMA2X2_BW_500HZ;

			/*  500 Hz       1000 uS   */
			break;
		case BMA2X2_BW_1000HZ:
			Bandwidth = BMA2X2_BW_1000HZ;

			/*  1000 Hz      500 uS   */
			break;
		default:
			break;
		}
		comres = bma2x2_smbus_read_byte(client, BMA2X2_BANDWIDTH__REG,
				&data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_BANDWIDTH, Bandwidth);
		comres += bma2x2_smbus_write_byte(client, BMA2X2_BANDWIDTH__REG,
				&data);
		if (comres < 0)
			pr_err("%s [SENSOR] - i2c bandwidth error ",__func__);
	} else {
		comres = -1;
	}
	pr_info("[SENSOR] %s, [%d]\n", __func__, (int)Bandwidth);

	return comres;
}

static int bma2x2_get_bandwidth(struct i2c_client *client, unsigned char *BW)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_BANDWIDTH__REG, &data);
	data = BMA2X2_GET_BITSLICE(data, BMA2X2_BANDWIDTH);
	*BW = data;

	return comres;
}

static int bma2x2_get_fifo_mode(struct i2c_client *client, unsigned char
		*fifo_mode)
{
	int comres;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_FIFO_MODE__REG, &data);
	*fifo_mode = BMA2X2_GET_BITSLICE(data, BMA2X2_FIFO_MODE);

	return comres;
}
#if 0
static int bma2x2_set_offset_target(struct i2c_client *client, unsigned char
		channel, unsigned char offset)
{
	unsigned char data;
	int comres = 0;

	switch (channel) {
	case BMA2X2_CUT_OFF:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_COMP_CUTOFF__REG, &data);
		data = BMA2X2_SET_BITSLICE(data, BMA2X2_COMP_CUTOFF,
				offset);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_COMP_CUTOFF__REG, &data);
		break;
	case BMA2X2_OFFSET_TRIGGER_X:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_X__REG,
				&data);
		data = BMA2X2_SET_BITSLICE(data,
				BMA2X2_COMP_TARGET_OFFSET_X,
				offset);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_X__REG,
				&data);
		break;
	case BMA2X2_OFFSET_TRIGGER_Y:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_Y__REG,
				&data);
		data = BMA2X2_SET_BITSLICE(data,
				BMA2X2_COMP_TARGET_OFFSET_Y,
				offset);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_Y__REG,
				&data);
		break;
	case BMA2X2_OFFSET_TRIGGER_Z:
		comres = bma2x2_smbus_read_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_Z__REG,
				&data);
		data = BMA2X2_SET_BITSLICE(data,
				BMA2X2_COMP_TARGET_OFFSET_Z,
				offset);
		comres = bma2x2_smbus_write_byte(client,
				BMA2X2_COMP_TARGET_OFFSET_Z__REG,
				&data);
		break;
	default:
		comres = -1;
		break;
	}

	return comres;
}

static int bma2x2_get_cal_ready(struct i2c_client *client,
					unsigned char *calrdy)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_FAST_CAL_RDY_S__REG,
			&data);
	data = BMA2X2_GET_BITSLICE(data, BMA2X2_FAST_CAL_RDY_S);
	*calrdy = data;

	return comres;
}

static int bma2x2_set_cal_trigger(struct i2c_client *client, unsigned char
		caltrigger)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_CAL_TRIGGER__REG, &data);
	data = BMA2X2_SET_BITSLICE(data, BMA2X2_CAL_TRIGGER, caltrigger);
	comres = bma2x2_smbus_write_byte(client, BMA2X2_CAL_TRIGGER__REG,
			&data);

	return comres;
}

static int bma2x2_set_offset_x(struct i2c_client *client, unsigned char
		offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;

	comres = bma2x2_smbus_write_byte(client, BMA2X2_OFFSET_X_AXIS_REG,
						&data);

	return comres;
}


static int bma2x2_get_offset_x(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_OFFSET_X_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma2x2_set_offset_y(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;

	comres = bma2x2_smbus_write_byte(client, BMA2X2_OFFSET_Y_AXIS_REG,
						&data);
	return comres;
}

static int bma2x2_get_offset_y(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_OFFSET_Y_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma2x2_set_offset_z(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma2x2_smbus_write_byte(client, BMA2X2_OFFSET_Z_AXIS_REG,
						&data);

	return comres;
}

static int bma2x2_get_offset_z(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma2x2_smbus_read_byte(client, BMA2X2_OFFSET_Z_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}
#endif
static int bma2x2_soft_reset(struct i2c_client *client)
{
	int comres = 0;
	unsigned char data = BMA2X2_EN_SOFT_RESET_VALUE;

	comres = bma2x2_smbus_write_byte(client, BMA2X2_EN_SOFT_RESET__REG,
					&data);

	return comres;
}
#if 0
static int bma2x2_read_accel_z(struct i2c_client *client,
				signed char sensor_type, short *a_z)
{
	int comres = 0;
	unsigned char data[2];

	switch (sensor_type) {
	case 0:
		comres = bma2x2_smbus_read_byte_block(client,
				BMA2X2_ACC_Z12_LSB__REG, data, 2);
		*a_z = BMA2X2_GET_BITSLICE(data[0], BMA2X2_ACC_Z12_LSB)|
			(BMA2X2_GET_BITSLICE(data[1],
				BMA2X2_ACC_Z_MSB)<<(BMA2X2_ACC_Z12_LSB__LEN));
		*a_z = *a_z << (sizeof(short)*8-(BMA2X2_ACC_Z12_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		*a_z = *a_z >> (sizeof(short)*8-(BMA2X2_ACC_Z12_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		break;
	case 1:
		comres = bma2x2_smbus_read_byte_block(client,
				BMA2X2_ACC_Z10_LSB__REG, data, 2);
		*a_z = BMA2X2_GET_BITSLICE(data[0], BMA2X2_ACC_Z10_LSB)|
			(BMA2X2_GET_BITSLICE(data[1],
				BMA2X2_ACC_Z_MSB)<<(BMA2X2_ACC_Z10_LSB__LEN));
		*a_z = *a_z << (sizeof(short)*8-(BMA2X2_ACC_Z10_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		*a_z = *a_z >> (sizeof(short)*8-(BMA2X2_ACC_Z10_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		break;
	case 2:
		comres = bma2x2_smbus_read_byte_block(client,
				BMA2X2_ACC_Z8_LSB__REG, data, 2);
		*a_z = BMA2X2_GET_BITSLICE(data[0], BMA2X2_ACC_Z8_LSB)|
			(BMA2X2_GET_BITSLICE(data[1],
				BMA2X2_ACC_Z_MSB)<<(BMA2X2_ACC_Z8_LSB__LEN));
		*a_z = *a_z << (sizeof(short)*8-(BMA2X2_ACC_Z8_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		*a_z = *a_z >> (sizeof(short)*8-(BMA2X2_ACC_Z8_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		break;
	case 3:
		comres = bma2x2_smbus_read_byte_block(client,
				BMA2X2_ACC_Z14_LSB__REG, data, 2);
		*a_z = BMA2X2_GET_BITSLICE(data[0], BMA2X2_ACC_Z14_LSB)|
				(BMA2X2_GET_BITSLICE(data[1],
				BMA2X2_ACC_Z_MSB)<<(BMA2X2_ACC_Z14_LSB__LEN));
		*a_z = *a_z << (sizeof(short)*8-(BMA2X2_ACC_Z14_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		*a_z = *a_z >> (sizeof(short)*8-(BMA2X2_ACC_Z14_LSB__LEN
						+ BMA2X2_ACC_Z_MSB__LEN));
		break;
	default:
		break;
	}

	return comres;
}

static int bma2x2_open_cal(struct i2c_client *client)
{
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);
	int cal_data[3];
	int err;
	mm_segment_t old_fs;
	struct file *cal_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH,
		O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		/*pr_err("[ACC] %s: calibration has never done.\n",
			__func__);*/
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		return err;
	}

	err = cal_filp->f_op->read(cal_filp,
		(char *)cal_data,
		3 * sizeof(int), &cal_filp->f_pos);

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	if (err != 3 * sizeof(int)) {
		pr_err("[ACC] %s: Can't read the cal data from file\n",
			__func__);
		return -EIO;
	}

	bma2x2_set_offset_x(bma2x2->bma2x2_client, (unsigned char)cal_data[0]);
	bma2x2_set_offset_y(bma2x2->bma2x2_client, (unsigned char)cal_data[1]);
	bma2x2_set_offset_z(bma2x2->bma2x2_client, (unsigned char)cal_data[2]);

	pr_info("%s [%d, %d, %d]\n", __func__, cal_data[0], cal_data[1], cal_data[2]);
	return 0;
}
#endif

const int bma2x2_sensor_bitwidth[] = {
	12,  10,  8, 14
};

static int bma2x2_read_accel_xyz(struct i2c_client *client,
		signed char sensor_type, struct bma2x2_v *acc)
{
	int comres = 0;
	unsigned char data[6];
	struct bma2x2_data *client_data = i2c_get_clientdata(client);
	int bitwidth;
	comres = bma2x2_smbus_read_byte_block(client,
				BMA2X2_ACC_X12_LSB__REG, data, 6);
	if (comres < 0)
		pr_err("%s [SENSOR] - i2c read error ",__func__);

	if (sensor_type >= 4)
		return -EINVAL;

	acc->x = (data[1]<<8)|data[0];
	acc->y = (data[3]<<8)|data[2];
	acc->z = (data[5]<<8)|data[4];

	bitwidth = bma2x2_sensor_bitwidth[sensor_type];

	acc->x = (acc->x >> (16 - bitwidth));
	acc->y = (acc->y >> (16 - bitwidth));
	acc->z = (acc->z >> (16 - bitwidth));

	remap_sensor_data(acc->v, client_data->place);
	return comres;
}

static void bma2x2_work_func(struct work_struct *work)
{
	struct bma2x2_data *bma2x2 = container_of((struct delayed_work *)work,
			struct bma2x2_data, work);
	static struct bma2x2_v acc;
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma2x2->delay));
	struct timespec ts = ktime_to_timespec(ktime_get_boottime());
	u64 timestamp_new = ts.tv_sec * 1000000000ULL + ts.tv_nsec;
	u64 timestamp ;
	int time_hi, time_lo;
	int ret;

	ret = bma2x2_read_accel_xyz(bma2x2->bma2x2_client, bma2x2->sensor_type, &acc);
	if (ret < 0) {
		pr_err("%s [SENSOR] - i2c read error!!! ",__func__);
		goto exit;
	}
	bma2x2->value.x = acc.x - bma2x2->caldata.x;
	bma2x2->value.y = acc.y - bma2x2->caldata.y;
	bma2x2->value.z = acc.z - bma2x2->caldata.z;

	if (((timestamp_new - bma2x2->old_timestamp) > atomic_read(&bma2x2->delay)*1800000LL)\
		&& (bma2x2->old_timestamp != 0))
	{
	        timestamp = (timestamp_new + bma2x2->old_timestamp) >>  1;
		time_hi = (int)((timestamp & TIME_HI_MASK) >> TIME_HI_SHIFT);
		time_lo = (int)(timestamp & TIME_LO_MASK);

		input_report_rel(bma2x2->input, REL_X, acc.x);
		input_report_rel(bma2x2->input, REL_Y, acc.y);
		input_report_rel(bma2x2->input, REL_Z, acc.z);
		input_report_rel(bma2x2->input, REL_DIAL, time_hi);
		input_report_rel(bma2x2->input, REL_MISC, time_lo);
		input_sync(bma2x2->input);
	}
	time_hi = (int)((timestamp_new & TIME_HI_MASK) >> TIME_HI_SHIFT);
	time_lo = (int)(timestamp_new & TIME_LO_MASK);

	input_report_rel(bma2x2->input, REL_X, acc.x);
	input_report_rel(bma2x2->input, REL_Y, acc.y);
	input_report_rel(bma2x2->input, REL_Z, acc.z);
	input_report_rel(bma2x2->input, REL_DIAL, time_hi);
	input_report_rel(bma2x2->input, REL_MISC, time_lo);
	input_sync(bma2x2->input);

	bma2x2->old_timestamp = timestamp_new;

	exit:
	if ((atomic_read(&bma2x2->delay) * bma2x2->time_count)
		>= (ACCEL_LOG_TIME * MSEC_PER_SEC)) {
			pr_info("[SENSOR]: %s - x = %d, y = %d, z = %d \n",
				__func__, bma2x2->value.x, bma2x2->value.y,
			bma2x2->value.z);
		bma2x2->time_count = 0;
	} else {
		bma2x2->time_count++;
	}

	schedule_delayed_work(&bma2x2->work, delay);
}

static ssize_t bma2x2_raw_data_read(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bma2x2_data *bma2x2 = input_get_drvdata(input);
	struct bma2x2_v acc_value;

	if (atomic_read(&bma2x2->enable) == OFF) {
		bma2x2_set_mode(bma2x2->bma2x2_client,
				BMA2X2_MODE_NORMAL, BMA_ENABLED_INPUT);

		msleep(20);
		bma2x2_read_accel_xyz(bma2x2->bma2x2_client, bma2x2->sensor_type,
								&acc_value);

		bma2x2_set_mode(bma2x2->bma2x2_client,
				BMA2X2_MODE_SUSPEND, BMA_ENABLED_INPUT);

		acc_value.x = acc_value.x - bma2x2->caldata.x;
		acc_value.y = acc_value.y - bma2x2->caldata.y;
		acc_value.z = acc_value.z - bma2x2->caldata.z;
	} else {
		acc_value = bma2x2->value;
	}

	return sprintf(buf, "%d,%d,%d\n", acc_value.x, acc_value.y,
			acc_value.z);
}

static ssize_t bma2x2_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma2x2->delay));

}

static ssize_t bma2x2_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;
	int bw = 0;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	data = data / 1000000L;

	pr_info("[SENSOR] %s [%d]\n", __func__, (int)data);

	if (data > BMA2X2_MAX_DELAY)
		data = BMA2X2_MAX_DELAY;
	else if (data < BMA2X2_MIN_DELAY)
		data = BMA2X2_MIN_DELAY;
	atomic_set(&bma2x2->delay, (unsigned int) data);

	/*set bandwidth */
	switch (data) {
	case 0:
	case 1:
	case 5:
	case 10:
		bw = 0x0b; /*100Hz*/
		break;
	case 20:
		bw = 0x0a; /*50hz;*/
		break;
	case 50:
	case 66:
		bw = 0x09; /*20Hz;*/
		break;
	case 200:
		bw = 0x08; /*5Hz;*/
		break;
	default:
		break;
	}
	if (bw >= 0x08) {
		if (bma2x2_set_bandwidth(bma2x2->bma2x2_client,
					(unsigned char) bw) < 0) {
			pr_info("[SENSOR]: %s failed to set bandwidth\n", __func__);
			return -EINVAL;
		}
	}

	return count;
}


static ssize_t bma2x2_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma2x2->enable));

}

static void bma2x2_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&bma2x2->enable);

	pr_info("[SENSOR] %s enable: [%d], pre_enable: [%d]\n", __func__, enable, pre_enable);

	mutex_lock(&bma2x2->enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			bma2x2->old_timestamp = 0LL;
			bma2x2_set_mode(bma2x2->bma2x2_client,
					BMA2X2_MODE_NORMAL, BMA_ENABLED_INPUT);
			schedule_delayed_work(&bma2x2->work,
				msecs_to_jiffies(atomic_read(&bma2x2->delay)));
			atomic_set(&bma2x2->enable, 1);
			pr_info("[SENSOR] %s enable: [%d] \n", __func__, atomic_read(&bma2x2->enable));
		}
	} else {
		if (pre_enable == 1) {
			if (atomic_read(&bma2x2->reactive_enable) == 0)
				bma2x2_set_mode(bma2x2->bma2x2_client,
					BMA2X2_MODE_SUSPEND, BMA_ENABLED_INPUT);
			cancel_delayed_work_sync(&bma2x2->work);
			atomic_set(&bma2x2->enable, 0);
		}
	}
	mutex_unlock(&bma2x2->enable_mutex);

}

static ssize_t bma2x2_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data = 0;
	int error;

	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;

	pr_info("[SENSOR] %s [%lu]\n", __func__, data);

	if ((data == 0) || (data == 1))
		bma2x2_set_enable(dev, data);

	return count;
}

static ssize_t bma2x2_calibration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);
	int ret;

	ret = bma2x2_open_calibration(bma2x2);
	if (ret < 0)
		pr_err("[SENSOR]: %s - calibration open failed(%d)\n",
			__func__, ret);

	pr_info("[SENSOR]: %s - cal data %d %d %d - ret : %d\n", __func__,
		bma2x2->caldata.x, bma2x2->caldata.y, bma2x2->caldata.z, ret);

	return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n", ret, bma2x2->caldata.x,
			bma2x2->caldata.y, bma2x2->caldata.z);
}

static int bma2x2_do_calibrate(struct bma2x2_data *data, int enable)
{
	int sum[3] = { 0, };
	int ret = 0, cnt;
	struct file *cal_filp = NULL;
	struct bma2x2_v acc;
	mm_segment_t old_fs;

	if (enable) {
		data->caldata.x = 0;
		data->caldata.y = 0;
		data->caldata.z = 0;

		for (cnt = 0; cnt < CALIBRATION_DATA_AMOUNT; cnt++) {
			bma2x2_read_accel_xyz(data->bma2x2_client, data->sensor_type,
										&acc);
			sum[0] += acc.x;
			sum[1] += acc.y;
			sum[2] += acc.z;
			mdelay(10);
		}

		data->caldata.x = (sum[0] / CALIBRATION_DATA_AMOUNT);
		data->caldata.y = (sum[1] / CALIBRATION_DATA_AMOUNT);
		data->caldata.z = (sum[2] / CALIBRATION_DATA_AMOUNT);

		if (data->range == BMA2X2_RANGE_4G) {
		if (data->caldata.z > 0)
			data->caldata.z -= MAX_ACCEL_1G_FOR4G;
		else if (data->caldata.z < 0)
			data->caldata.z += MAX_ACCEL_1G_FOR4G;
	} else {
			if (data->caldata.z > 0)
				data->caldata.z -= MAX_ACCEL_1G;
			else if (data->caldata.z < 0)
				data->caldata.z += MAX_ACCEL_1G;

		}
	} else {
		data->caldata.x = 0;
		data->caldata.y = 0;
		data->caldata.z = 0;
	}

	pr_info("[SENSOR]: %s - do accel calibrate %d, %d, %d\n", __func__,
		data->caldata.x, data->caldata.y, data->caldata.z);

		old_fs = get_fs();
		set_fs(KERNEL_DS);

		cal_filp = filp_open(CALIBRATION_FILE_PATH,
		O_CREAT | O_TRUNC | O_WRONLY, 0666);
		if (IS_ERR(cal_filp)) {
		pr_err("[SENSOR]: %s - Can't open calibration file\n",
							__func__);
			set_fs(old_fs);
		ret = PTR_ERR(cal_filp);
		return ret;
		}

	ret = cal_filp->f_op->write(cal_filp, (char *)&data->caldata.v,
		3 * sizeof(s16), &cal_filp->f_pos);
	if (ret != 3 * sizeof(s16)) {
		pr_err("[SENSOR]: %s - Can't write the caldata to file\n",
								__func__);
		ret = -EIO;
		}

		filp_close(cal_filp, current->files);
		set_fs(old_fs);

	return ret;
		}

static ssize_t bma2x2_calibration_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);
	int64_t dEnable;
	int err;

	err = kstrtoll(buf, 10, &dEnable);
	if (err)
		return err;

	err = bma2x2_do_calibrate(bma2x2, (int)dEnable);
	if (err < 0)
		pr_err("[SENSOR]: %s - accel calibrate failed\n", __func__);


	return count;
}

static ssize_t bma2x2_reactive_enable_show(struct device *dev,
					struct device_attribute	*attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n",
		atomic_read(&bma2x2->reactive_state));
}

static ssize_t bma2x2_reactive_enable_store(struct device *dev,
					struct device_attribute	*attr, const char *buf,
							size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *bma2x2 = i2c_get_clientdata(client);
	bool onoff = false;
	unsigned long value = 0;
	int err = count;
#if defined(DEFENCE_REACTIVE_ALERT)
	int this_cpu;
	unsigned long long t;
	unsigned long nanosec_rem;
#endif
	unsigned char data;

	if (kstrtoul(buf, 10, &value)) {
		err = -EINVAL;
		return err;
	}
	pr_info("%s: value=%lu\n", __func__, value);

	switch (value) {
	case 0:
		bma2x2_set_Int_Enable(bma2x2->bma2x2_client, 7, 0);
		if (atomic_read(&bma2x2->factory_mode))
			atomic_set(&bma2x2->factory_mode, false);
		if (!atomic_read(&bma2x2->enable))
			bma2x2_set_mode(bma2x2->bma2x2_client,
				BMA2X2_MODE_SUSPEND, BMA_ENABLED_INPUT);
		break;
	case 1:
		onoff = true;
		data = 0x01;
		bma2x2_set_slope_duration(bma2x2->bma2x2_client, data);
		usleep_range(3000, 3100);
		data = 0x05;
		bma2x2_set_slope_threshold(bma2x2->bma2x2_client, data);
		usleep_range(3000, 3100);
		bma2x2_set_mode(bma2x2->bma2x2_client,
					BMA2X2_MODE_LOWPOWER1, BMA_ENABLED_INPUT);
		usleep_range(3000, 3100);
		bma2x2_set_Int_Enable(bma2x2->bma2x2_client, 7, 1);
		break;
	case 2:
		onoff = true;
		bma2x2_set_mode(bma2x2->bma2x2_client,
					BMA2X2_MODE_NORMAL, BMA_ENABLED_INPUT);
		usleep_range(3000, 3100);
		bma2x2_set_bandwidth(bma2x2->bma2x2_client, BMA2X2_BW_1000HZ);
		data = 0x00;
		bma2x2_set_slope_duration(bma2x2->bma2x2_client, data);
		usleep_range(3000, 3100);
		data = 0x00;
		bma2x2_set_slope_threshold(bma2x2->bma2x2_client, data);
		usleep_range(3000, 3100);
		bma2x2_set_Int_Enable(bma2x2->bma2x2_client, 7, 1);
		atomic_set(&bma2x2->factory_mode, true);
		break;
	default:
		err = -EINVAL;
		pr_err("%s: invalid value %d\n", __func__, *buf);
		return count;
	}

	if (bma2x2->IRQ) {
		if (!value) {
			disable_irq_wake(bma2x2->IRQ);
			disable_irq(bma2x2->IRQ);
		} else {
			enable_irq(bma2x2->IRQ);
			enable_irq_wake(bma2x2->IRQ);
		}
	}

	atomic_set(&bma2x2->reactive_enable, onoff);
	atomic_set(&bma2x2->reactive_state, false);
	usleep_range(1000, 1100);
#if defined(DEFENCE_REACTIVE_ALERT)
	this_cpu = raw_smp_processor_id();
	t = cpu_clock(this_cpu);
	nanosec_rem = do_div(t, 1000000000);
	t_before = t;
	nanosec_before = nanosec_rem;
#endif

	pr_info("%s: onoff = %d\n", __func__,
		atomic_read(&bma2x2->reactive_enable));
	return count;
}

static ssize_t bma2x2_lowpassfilter_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	unsigned char data;
	struct bma2x2_data *bma2x2 = dev_get_drvdata(dev);

	ret = bma2x2_get_bandwidth(bma2x2->bma2x2_client, &data);
	if (ret < 0)
		pr_err("%s Read error:%d\n", __func__, ret);

	if (data == BMA2X2_BW_1000HZ)
		ret = 0;
	else
		ret = 1;
	pr_info("%s %d\n", __func__, data);

	return sprintf(buf, "%d\n", ret);
}

static ssize_t bma2x2_lowpassfilter_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned char data;
	int64_t dEnable;
	struct bma2x2_data *bma2x2 = dev_get_drvdata(dev);

	ret = kstrtoll(buf, 10, &dEnable);
	if (ret < 0)
		pr_err("%s - kstrtoll failed\n", __func__);

	ret = bma2x2_get_bandwidth(bma2x2->bma2x2_client, &data);
	if (ret < 0)
		pr_err("%s Read error:%d\n", __func__, ret);

	pr_info("%s data:%d, used_bw:%d\n", __func__, data, bma2x2->used_bw);

	if (dEnable) {
		if (bma2x2->used_bw)
			data = bma2x2->used_bw;
		bma2x2->used_bw = 0;
	} else {
		if (!bma2x2->used_bw)
			bma2x2->used_bw = data;
		data = BMA2X2_BW_1000HZ;
	}
	pr_info("%s data:%d, used_bw:%d\n", __func__, data, bma2x2->used_bw);

	ret = bma2x2_set_bandwidth(bma2x2->bma2x2_client, data);
	if (ret < 0)
		pr_err("%s - bma2x2_set_bandwidth failed\n", __func__);

	return count;
}

static ssize_t bma2x2_read_name(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CHIP_NAME);
}

static ssize_t bma2x2_read_vendor(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CHIP_VENDOR);
}

static DEVICE_ATTR(raw_data, S_IRUGO,
		bma2x2_raw_data_read, NULL);
static DEVICE_ATTR(poll_delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_delay_show, bma2x2_delay_store);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_delay_show, bma2x2_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_enable_show, bma2x2_enable_store);
static DEVICE_ATTR(calibration, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_calibration_show,
		bma2x2_calibration_store);
static DEVICE_ATTR(reactive_alert, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_reactive_enable_show, bma2x2_reactive_enable_store);
static DEVICE_ATTR(lowpassfilter, S_IRUGO|S_IWUSR|S_IWGRP,
		bma2x2_lowpassfilter_show, bma2x2_lowpassfilter_store);
static DEVICE_ATTR(name, S_IRUGO, bma2x2_read_name, NULL);
static DEVICE_ATTR(vendor, S_IRUGO, bma2x2_read_vendor, NULL);

static struct attribute *bma2x2_attributes[] = {
	&dev_attr_raw_data.attr,
	&dev_attr_poll_delay.attr,
	&dev_attr_enable.attr,
	NULL
};
static struct device_attribute *bma2x2_attributes_sensors[] = {
	&dev_attr_raw_data,
	&dev_attr_delay,
	&dev_attr_enable,
	&dev_attr_calibration,
	&dev_attr_reactive_alert,
	&dev_attr_lowpassfilter,
	&dev_attr_name,
	&dev_attr_vendor,
	NULL
};

static struct attribute_group bma2x2_attribute_group = {
	.attrs = bma2x2_attributes
};

static void bma2x2_slope_interrupt_handle(struct bma2x2_data *bma2x2)
{
	unsigned char first_value = 0;
	unsigned char sign_value = 0;
	int i;
	for (i = 0; i < 3; i++) {
		bma2x2_get_slope_first(bma2x2->bma2x2_client, i, &first_value);
		if (first_value == 1) {
			bma2x2_get_slope_sign(bma2x2->bma2x2_client,
								&sign_value);

			pr_info("%s Slop interrupt happened,exis is %d, first is %d,sign is %d\n",
				__func__, i, first_value, sign_value);
		}
	}
}

static void bma2x2_irq_work_func(struct work_struct *work)
{
	struct bma2x2_data *bma2x2 = container_of((struct work_struct *)work,
			struct bma2x2_data, irq_work);
	unsigned char status = 0;
#if defined(DEFENCE_REACTIVE_ALERT)
	int this_cpu;
	unsigned long long t;
	unsigned long nanosec_rem;
#endif

	bma2x2_get_interruptstatus1(bma2x2->bma2x2_client, &status);
	pr_info("bma2x2_irq_work_func, status = 0x%x\n", status);

	switch (status) {
	case 0x04:
		bma2x2_slope_interrupt_handle(bma2x2);
		break;
	default:
		break;
	}
#if defined(DEFENCE_REACTIVE_ALERT)
	if ((status == 0x4) && (atomic_read(&bma2x2->factory_mode) == false)) {
		this_cpu = raw_smp_processor_id();
		t = cpu_clock(this_cpu);
		nanosec_rem = do_div(t, 1000000000);
		/*pr_info("%s: ======this_cpu:%d [%5lu.%06lu] \n", __func__,this_cpu, (unsigned long) t,nanosec_rem / 1000);*/
		if ((t-t_before) < 2) {
			unsigned long long calc;
			calc = (unsigned long long)((t-t_before)*1000 + (nanosec_rem/1000000))
					-(unsigned long long)(nanosec_before/1000000);
			if (calc < 750) {
				pr_info("%s: But acc INT interval too short!!! (calc=%llu)!!!!! \n", __func__, calc);
				return;
			}
		}
	}
#endif

	atomic_set(&bma2x2->reactive_state, true);
}

static irqreturn_t bma2x2_irq_handler(int irq, void *handle)
{
	struct bma2x2_data *data = handle;

	if (data == NULL)
		return IRQ_HANDLED;
	if (data->bma2x2_client == NULL)
		return IRQ_HANDLED;

	schedule_work(&data->irq_work);

	return IRQ_HANDLED;
}

static int bma2x2_acc_power_onoff(struct bma2x2_data *data, bool onoff)
{
	int ret = 0;

	pr_info("%s :%d\n", __func__, onoff);
#ifdef CONFIG_SENSORS_BMC150_VDD
	data->reg_vdd = devm_regulator_get(&data->bma2x2_client->dev, "bma2x2,vdd");
	if (IS_ERR(data->reg_vdd)) {
		pr_err("%s: could not get vdd, %ld\n", __func__, PTR_ERR(data->reg_vdd));
		ret = -ENOMEM;
		goto err_vdd;
	} else if (!regulator_get_voltage(data->reg_vdd)) {
		ret = regulator_set_voltage(data->reg_vdd, 2850000, 2850000);
	}
#endif
	data->reg_vio = devm_regulator_get(&data->bma2x2_client->dev, "bma2x2,vio");
	if (IS_ERR(data->reg_vio)) {
		pr_err("%s: could not get vio, %ld\n", __func__,
			PTR_ERR(data->reg_vio));
		ret = -ENOMEM;
		goto err_vio;
	} else if (!regulator_get_voltage(data->reg_vio)) {
		ret = regulator_set_voltage(data->reg_vio, 1800000, 1800000);
	}

	if (onoff) {
#ifdef CONFIG_SENSORS_BMC150_VDD
		ret = regulator_enable(data->reg_vdd);
		if (ret) {
			pr_err("%s: Failed to enable vdd.\n", __func__);
		}
#endif
		ret = regulator_enable(data->reg_vio);
		if (ret) {
			pr_err("%s: Failed to enable vio.\n", __func__);
		}
		msleep(30);
	} else {
#ifdef CONFIG_SENSORS_BMC150_VDD
		ret = regulator_disable(data->reg_vdd);
		if (ret) {
			pr_err("%s: Failed to disable vdd.\n", __func__);
		}
#endif
		ret = regulator_disable(data->reg_vio);
		if (ret) {
			pr_err("%s: Failed to disable vio.\n", __func__);
	}
	}
	pr_info("%s success:%d\n", __func__, onoff);

	devm_regulator_put(data->reg_vio);
err_vio:
#ifdef CONFIG_SENSORS_BMC150_VDD
	devm_regulator_put(data->reg_vdd);
err_vdd:
#endif
	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id bma2x2_dt_ids[] = {
	{ .compatible = "bma2x2", },
	{},
};
MODULE_DEVICE_TABLE(of, bma2x2_dt_ids);

static int bma2x2_parse_dt(struct bma2x2_data *data, struct device *dev)
{

	struct device_node *np = dev->of_node;
	enum of_gpio_flags flags;
	int ret = 0;

	if (np == NULL) {
		pr_err("%s no dev_node\n", __func__);
		return -ENODEV;
	}

	data->acc_int1 = of_get_named_gpio_flags(np, "bma2x2,gpio_int1", 0, &flags);
	if (data->acc_int1 < 0) {
		pr_err("%s - get gpio_int error\n", __func__);
		return -ENODEV;
	}
	ret = of_property_read_u32(np, "bma2x2,accel_place", &data->place);
	if (unlikely(ret)) {
		dev_err(dev, "error reading property acc_place from device node %d\n",
			data->place);
		goto error;
	}
	pr_info("%s place[%d] acc_int[%d]\n",
		__func__, data->place, data->acc_int1);

	return 0;
error:
	return ret;
}
#endif

static int bma2x2_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err = 0;
	struct bma2x2_data *data;
	struct input_dev *dev;

	pr_info("[ACC]%s ...\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_info("%s -i2c_check_functionality error\n", __func__);

		err = -EIO;
		goto exit;
	}
	data = kzalloc(sizeof(struct bma2x2_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->bma2x2_client = client;
	err = bma2x2_parse_dt(data, &client->dev);
	if (err < 0) {
		pr_err("%s Error getting platform data from device node\n",
			__func__);
		goto kfree_exit;
	}
	err = bma2x2_acc_power_onoff(data, true);
	if (err < 0) {
		pr_err("%s - No regulator\n", __func__);
		goto kfree_exit;
	}

	/* do soft reset */
	usleep_range(2900, 3000);
	if (bma2x2_soft_reset(client) < 0) {
		dev_err(&client->dev,
			"i2c bus write error, pls check HW connection\n");
		err = -EINVAL;
		goto kfree_exit;
	}
	usleep_range(4900, 5000);

	/* read and check chip id */
	if (bma2x2_check_chip_id(client, data) < 0) {
		err = -EINVAL;
		goto kfree_exit;
	}

	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma2x2_set_bandwidth(client, BMA2X2_BW_SET);
	bma2x2_set_range(client, BMA2X2_RANGE_SET);
	data->range = BMA2X2_RANGE_SET;

#ifdef CONFIG_SENSORS_BMA2X2_ENABLE_INT1
	/* maps interrupt to INT1 pin */
	bma2x2_set_int1_pad_sel(client, PAD_SLOP);
#endif

	bma2x2_set_Int_Mode(client, 1);/*latch interrupt 250ms*/

	/* do not open any interrupt here  */
	/*10,orient  11,flat*/
	/*bma2x2_set_Int_Enable(client, 10, 1); */
	/*bma2x2_set_Int_Enable(client, 11, 1); */

	err = gpio_request(data->acc_int1,"bma2x2");
	if(err < 0)
	{
		printk("%s request gpio [%d] fail",__func__,data->acc_int1);
		goto kfree_exit;
	}
	err = gpio_direction_input(data->acc_int1);
	if(err < 0)
	{
		printk("%s: failed to set gpio %d as input (%d)\n",
			__func__, data->acc_int1, err);
		goto err_gpio_direction_input;
	}
	data->IRQ = gpio_to_irq(data->acc_int1);
	err = request_irq(data->IRQ, bma2x2_irq_handler, IRQF_TRIGGER_RISING,
			"bma2x2", data);
	if (err)
		pr_err("%s could not request irq\n", __func__);

	disable_irq(data->IRQ);

	INIT_WORK(&data->irq_work, bma2x2_irq_work_func);

	INIT_DELAYED_WORK(&data->work, bma2x2_work_func);
	atomic_set(&data->delay, BMA2X2_MAX_DELAY);
	atomic_set(&data->enable, 0);

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;

	/* only value events reported */
	dev->name = SENSOR_NAME;
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_REL, REL_X);
	input_set_capability(dev, EV_REL, REL_Y);
	input_set_capability(dev, EV_REL, REL_Z);
	input_set_capability(dev, EV_REL, REL_DIAL);
	input_set_capability(dev, EV_REL, REL_MISC);

	input_set_drvdata(dev, data);
	err = input_register_device(dev);
	if (err < 0)
		goto err_register_input_device;
	data->input = dev;

	err = sensors_create_symlink(&data->input->dev.kobj, data->input->name);
	if (err < 0) {
		pr_err("%s failed sensors_create_symlink\n", __func__);
	goto error_sysfs;
	}

	err = sysfs_create_group(&data->input->dev.kobj,
			&bma2x2_attribute_group);
	if (err < 0)
		goto error_sysfs;

	data->bma_mode_enabled = 0;
	data->time_count = 0;

	err = sensors_register(bma2x2_device, data,
		bma2x2_attributes_sensors, "accelerometer_sensor");

	if (unlikely(err < 0)) {
		pr_err("%s: could not sensors_register\n", __func__);
		goto error_sensors_register;
	}

	pr_info("%s -BMA2x2 driver probe successfully\n", __func__);


	return 0;

error_sensors_register:
	sysfs_remove_group(&data->input->dev.kobj,
		&bma2x2_attribute_group);

error_sysfs:
	input_unregister_device(data->input);
	sensors_remove_symlink(&data->input->dev.kobj, data->input->name);
	input_unregister_device(data->input);

err_register_input_device:
	input_free_device(dev);
err_gpio_direction_input:
	gpio_free(data->acc_int1);
kfree_exit:
	kfree(data);
exit:
	return err;
}


static int bma2x2_remove(struct i2c_client *client)
{
	struct bma2x2_data *data = i2c_get_clientdata(client);

	bma2x2_set_enable(&client->dev, 0);
	sysfs_remove_group(&data->input->dev.kobj, &bma2x2_attribute_group);
	input_unregister_device(data->input);

	kfree(data);

	return 0;
}

static int bma2x2_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *data = i2c_get_clientdata(client);

	pr_info("%s reactive_enable[%d] reactive_state[%d]\n", __func__,
		atomic_read(&data->reactive_enable),
		atomic_read(&data->reactive_state));

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		if (!atomic_read(&data->reactive_enable)) {
			bma2x2_set_mode(data->bma2x2_client,
				BMA2X2_MODE_SUSPEND, BMA_ENABLED_INPUT);
		}
		cancel_delayed_work_sync(&data->work);
	}
	mutex_unlock(&data->enable_mutex);

	return 0;
}

static int bma2x2_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma2x2_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma2x2_set_mode(data->bma2x2_client,
			BMA2X2_MODE_NORMAL, BMA_ENABLED_INPUT);
		schedule_delayed_work(&data->work,
				msecs_to_jiffies(atomic_read(&data->delay)));
	}
	mutex_unlock(&data->enable_mutex);
	pr_info("%s called.\n", __func__);
	return 0;
}

static const struct i2c_device_id bma2x2_id[] = {
	{ SENSOR_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, bma2x2_id);

static const struct dev_pm_ops bma_pm_ops = {
	.suspend = bma2x2_suspend,
	.resume = bma2x2_resume
};

static struct i2c_driver bma2x2_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= SENSOR_NAME,
		.pm	= &bma_pm_ops,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(bma2x2_dt_ids),
#endif
	},
	.id_table	= bma2x2_id,
	.probe		= bma2x2_probe,
	.remove		= bma2x2_remove,
};

static int __init BMA2X2_init(void)
{
	return i2c_add_driver(&bma2x2_driver);
}

static void __exit BMA2X2_exit(void)
{
	i2c_del_driver(&bma2x2_driver);
}

MODULE_AUTHOR("contact@bosch-sensortec.com");
MODULE_DESCRIPTION("BMA2X2 ACCELEROMETER SENSOR DRIVER");
MODULE_LICENSE("GPL v2");

module_init(BMA2X2_init);
module_exit(BMA2X2_exit);
