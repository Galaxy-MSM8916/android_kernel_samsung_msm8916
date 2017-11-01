/*
 * STMicroelectronics lsm6ds3 buffer driver
 *
 * Copyright 2014 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

#include "st_lsm6ds3.h"

#define ST_LSM6DS3_ENABLE_AXIS			0x07
#define ST_LMS6DS3_PRINT_SEC			10

irqreturn_t st_lsm6ds3_iio_pollfunc_store_boottime(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	pf->timestamp = st_lsm6ds3_iio_get_boottime_ns();
	return IRQ_WAKE_THREAD;
}

static int st_lsm6ds3_get_buffer_element(struct iio_dev *indio_dev, u8 *buf)
{
	int i, n = 0, len;
	u8 addr[ST_LSM6DS3_NUMBER_DATA_CHANNELS];
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	struct lsm6ds3_data *cdata = sdata->cdata;

	for (i = 0; i < ST_LSM6DS3_NUMBER_DATA_CHANNELS; i++) {
		if (test_bit(i, indio_dev->active_scan_mask)) {
			addr[n] = indio_dev->channels[i].address;
			n++;
		}
	}
	switch (n) {
	case 1:
		len = cdata->tf->read(&cdata->tb, cdata->dev,
				addr[0], ST_LSM6DS3_BYTE_FOR_CHANNEL, buf);
		break;
	case 2:
		if ((addr[1] - addr[0]) == ST_LSM6DS3_BYTE_FOR_CHANNEL) {
			len = cdata->tf->read(&cdata->tb,
					cdata->dev, addr[0],
					ST_LSM6DS3_BYTE_FOR_CHANNEL * n, buf);
		} else {
			u8 rx_array[ST_LSM6DS3_BYTE_FOR_CHANNEL*
					ST_LSM6DS3_NUMBER_DATA_CHANNELS];
			len = cdata->tf->read(&cdata->tb,
				cdata->dev, addr[0],
				ST_LSM6DS3_BYTE_FOR_CHANNEL *
				ST_LSM6DS3_NUMBER_DATA_CHANNELS, rx_array);
			if (len < 0)
				goto read_data_channels_error;

			for (i = 0; i < n * ST_LSM6DS3_NUMBER_DATA_CHANNELS;
									i++) {
				if (i < n)
					buf[i] = rx_array[i];
				else
					buf[i] = rx_array[n + i];
			}
		}
		break;
	case 3:
		len = cdata->tf->read(&cdata->tb, cdata->dev,
					addr[0], ST_LSM6DS3_BYTE_FOR_CHANNEL*
					ST_LSM6DS3_NUMBER_DATA_CHANNELS, buf);
		if (len < 0)
			goto read_data_channels_error;

		break;
	default:
		len = -EINVAL;
		goto read_data_channels_error;
	}
	len = ST_LSM6DS3_BYTE_FOR_CHANNEL * n;

	if ((cdata->patch_feature) && (sdata->sindex == ST_INDIO_DEV_GYRO) &&
					(cdata->gyro_selftest_status > 0)) {
		n = 0;

		for (i = 0; i < ST_LSM6DS3_NUMBER_DATA_CHANNELS; i++) {
			if (test_bit(i, indio_dev->active_scan_mask)) {
				if (cdata->gyro_selftest_status == 1)
					*(u16 *)((u8 *)&buf[2 * n]) +=
						cdata->gyro_selftest_delta[i];
				else
					*(u16 *)((u8 *)&buf[2 * n]) -=
						cdata->gyro_selftest_delta[i];

				n++;
			}
		}
	}

read_data_channels_error:
	return len;
}

static irqreturn_t st_lsm6ds3_accel_gyro_trigger_handler(int irq, void *p)
{
	int len;
	static int pr_cnt = 0;
	static s64 pr_timestamp = 0, pr_timestamp_current = 0;
	s64 pr_delay = 0;

	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	len = st_lsm6ds3_get_buffer_element(indio_dev, sdata->buffer_data);
	if (len < 0)
		goto st_lsm6ds3_get_buffer_element_error;

	if (indio_dev->scan_timestamp)
		*(s64 *)((u8 *)sdata->buffer_data +
			ALIGN(len, sizeof(s64))) = sdata->cdata->timestamp;

	if(sdata->sindex == ST_INDIO_DEV_ACCEL){
		sdata->cdata->accel_data[0] = ((sdata->buffer_data[1] << 8) | sdata->buffer_data[0]);
		sdata->cdata->accel_data[1] = ((sdata->buffer_data[3] << 8) | sdata->buffer_data[2]);
		sdata->cdata->accel_data[2] = ((sdata->buffer_data[5] << 8) | sdata->buffer_data[4]);

		sdata->cdata->accel_data[0] -= sdata->cdata->accel_cal_data[0];
		sdata->cdata->accel_data[1] -= sdata->cdata->accel_cal_data[1];
		sdata->cdata->accel_data[2] -= sdata->cdata->accel_cal_data[2];

		sdata->buffer_data[0] = (u8)(sdata->cdata->accel_data[0] & 0x00ff);
		sdata->buffer_data[1] = (u8)((sdata->cdata->accel_data[0] & 0xff00) >> 8);
		sdata->buffer_data[2] = (u8)(sdata->cdata->accel_data[1] & 0x00ff);
		sdata->buffer_data[3] = (u8)((sdata->cdata->accel_data[1] & 0xff00) >> 8);
		sdata->buffer_data[4] = (u8)(sdata->cdata->accel_data[2] & 0x00ff);
		sdata->buffer_data[5] = (u8)((sdata->cdata->accel_data[2] & 0xff00) >> 8);
	}
	else if(sdata->sindex == ST_INDIO_DEV_GYRO){
		sdata->cdata->gyro_data[0] = ((sdata->buffer_data[1] << 8) | sdata->buffer_data[0]);
		sdata->cdata->gyro_data[1] = ((sdata->buffer_data[3] << 8) | sdata->buffer_data[2]);
		sdata->cdata->gyro_data[2] = ((sdata->buffer_data[5] << 8) | sdata->buffer_data[4]);

		sdata->cdata->gyro_data[0] -= sdata->cdata->gyro_cal_data[0];
		sdata->cdata->gyro_data[1] -= sdata->cdata->gyro_cal_data[1];
		sdata->cdata->gyro_data[2] -= sdata->cdata->gyro_cal_data[2];

		sdata->buffer_data[0] = (u8)(sdata->cdata->gyro_data[0] & 0x00ff);
		sdata->buffer_data[1] = (u8)((sdata->cdata->gyro_data[0] & 0xff00) >> 8);
		sdata->buffer_data[2] = (u8)(sdata->cdata->gyro_data[1] & 0x00ff);
		sdata->buffer_data[3] = (u8)((sdata->cdata->gyro_data[1] & 0xff00) >> 8);
		sdata->buffer_data[4] = (u8)(sdata->cdata->gyro_data[2] & 0x00ff);
		sdata->buffer_data[5] = (u8)((sdata->cdata->gyro_data[2] & 0xff00) >> 8);
	}

	iio_push_to_buffers(indio_dev, sdata->buffer_data);

	if(!pr_timestamp) {
		pr_timestamp = st_lsm6ds3_iio_get_boottime_ns();
	}

	pr_timestamp_current = st_lsm6ds3_iio_get_boottime_ns();
	pr_delay = (pr_timestamp_current - pr_timestamp);

	if (((s64)ST_LMS6DS3_PRINT_SEC * NSEC_PER_SEC) <=
		(pr_delay * (s64)pr_cnt)) {
		pr_info("%s, acc_x = %d, acc_y = %d, acc_z = %d\n", __func__,
			sdata->cdata->accel_data[0],
			sdata->cdata->accel_data[1],
			sdata->cdata->accel_data[2]);

		pr_info("%s, gyro_x = %d, gyro_y = %d, gyro_z = %d\n", __func__,
			sdata->cdata->gyro_data[0],
			sdata->cdata->gyro_data[1],
			sdata->cdata->gyro_data[2]);
		pr_cnt = 0;
	} else {
		pr_cnt++;
	}

	pr_timestamp = st_lsm6ds3_iio_get_boottime_ns();

st_lsm6ds3_get_buffer_element_error:
	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

static irqreturn_t st_lsm6ds3_step_counter_trigger_handler(int irq, void *p)
{
	int err;
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);

	err = sdata->cdata->tf->read(&sdata->cdata->tb,
					sdata->cdata->dev,
					(u8)indio_dev->channels[0].address,
					ST_LSM6DS3_BYTE_FOR_CHANNEL,
					(u8 *)&sdata->cdata->step_c.steps);
	if (err < 0)
		goto step_counter_done;

	sdata->cdata->step_c.new_steps = true;
	sdata->cdata->step_c.last_step_timestamp = sdata->cdata->timestamp;

step_counter_done:
	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

static inline irqreturn_t st_lsm6ds3_handler_empty(int irq, void *p)
{
	return IRQ_HANDLED;
}

int st_lsm6ds3_trig_set_state(struct iio_trigger *trig, bool state)
{
	int err;
	struct lsm6ds3_sensor_data *sdata;

	sdata = iio_priv(iio_trigger_get_drvdata(trig));

	err = st_lsm6ds3_set_drdy_irq(sdata, state);

	return err < 0 ? err : 0;
}

static int st_lsm6ds3_buffer_preenable(struct iio_dev *indio_dev)
{
	int err;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	pr_info("%s, start! (%d)\n", __func__, sdata->sindex);

	if (sdata->sindex == ST_INDIO_DEV_ACCEL)
		err = st_acc_open_calibration(sdata->cdata);
	else if(sdata->sindex == ST_INDIO_DEV_GYRO)
		err = st_gyro_open_calibration(sdata->cdata);
	err = st_lsm6ds3_set_enable(sdata, true);
	if (err < 0)
		return err;

	return iio_sw_buffer_preenable(indio_dev);
}

static int st_lsm6ds3_buffer_postenable(struct iio_dev *indio_dev)
{
	int err = 0;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	pr_info("%s, start! (%d)\n", __func__, sdata->sindex);
	if ((1 << sdata->sindex) & ST_LSM6DS3_USE_BUFFER) {
		sdata->buffer_data = kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
		if (sdata->buffer_data == NULL)
			return -ENOMEM;
	}

	if ((sdata->sindex == ST_INDIO_DEV_ACCEL) ||
					(sdata->sindex == ST_INDIO_DEV_GYRO)) {
		err = st_lsm6ds3_set_axis_enable(sdata,
					(u8)indio_dev->active_scan_mask[0]);
		if (err < 0)
			goto free_buffer_data;
	}

	err = iio_triggered_buffer_postenable(indio_dev);
	if (err < 0)
		goto free_buffer_data;

	if (sdata->sindex == ST_INDIO_DEV_STEP_COUNTER) {
		sdata->cdata->step_c.new_steps = true;
		iio_trigger_poll_chained(
			sdata->cdata->trig[ST_INDIO_DEV_STEP_COUNTER], 0);
		hrtimer_start(&sdata->cdata->step_c.hr_timer,
				sdata->cdata->step_c.ktime, HRTIMER_MODE_REL);
	}

	if (sdata->sindex == ST_INDIO_DEV_SIGN_MOTION)
		sdata->cdata->sign_motion_event_ready = true;

	return err;

free_buffer_data:
	if ((1 << sdata->sindex) & ST_LSM6DS3_USE_BUFFER)
		kfree(sdata->buffer_data);

	return err;
}

static int st_lsm6ds3_buffer_predisable(struct iio_dev *indio_dev)
{
	int err;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	pr_info("%s, start! (%d)\n", __func__, sdata->sindex);

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	if( !((sdata->sindex == ST_INDIO_DEV_ACCEL) && sdata->cdata->sa_flag) ){
		err = st_lsm6ds3_set_enable(sdata, false);
		if (err < 0)
			return err;
	}
#endif

	err = iio_triggered_buffer_predisable(indio_dev);
	if (err < 0)
		return err;

	if ((sdata->sindex == ST_INDIO_DEV_ACCEL) ||
					(sdata->sindex == ST_INDIO_DEV_GYRO)) {
		err = st_lsm6ds3_set_axis_enable(sdata, ST_LSM6DS3_ENABLE_AXIS);
		if (err < 0)
			return err;
	}

	if (sdata->sindex == ST_INDIO_DEV_SIGN_MOTION)
		sdata->cdata->sign_motion_event_ready = false;

#if !(LSM6DS3_HRTIMER_TRIGGER > 0)
	err = st_lsm6ds3_set_enable(sdata, false);
	if (err < 0)
		return err;
#endif

	if (sdata->sindex == ST_INDIO_DEV_STEP_COUNTER)
		hrtimer_cancel(&sdata->cdata->step_c.hr_timer);

	if ((1 << sdata->sindex) & ST_LSM6DS3_USE_BUFFER)
		kfree(sdata->buffer_data);

	return 0;
}

static const struct iio_buffer_setup_ops st_lsm6ds3_buffer_setup_ops = {
	.preenable = &st_lsm6ds3_buffer_preenable,
	.postenable = &st_lsm6ds3_buffer_postenable,
	.predisable = &st_lsm6ds3_buffer_predisable,
};

int st_lsm6ds3_allocate_rings(struct lsm6ds3_data *cdata)
{
	int err;

	err = iio_triggered_buffer_setup(cdata->indio_dev[ST_INDIO_DEV_ACCEL],
				&st_lsm6ds3_iio_pollfunc_store_boottime,
				&st_lsm6ds3_accel_gyro_trigger_handler,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		return err;

	err = iio_triggered_buffer_setup(cdata->indio_dev[ST_INDIO_DEV_GYRO],
				&st_lsm6ds3_iio_pollfunc_store_boottime,
				&st_lsm6ds3_accel_gyro_trigger_handler,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		goto buffer_cleanup_accel;

	err = iio_triggered_buffer_setup(
				cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION],
				&st_lsm6ds3_handler_empty, NULL,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		goto buffer_cleanup_gyro;

	err = iio_triggered_buffer_setup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER],
				&st_lsm6ds3_iio_pollfunc_store_boottime,
				&st_lsm6ds3_step_counter_trigger_handler,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		goto buffer_cleanup_sign_motion;

	err = iio_triggered_buffer_setup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR],
				&st_lsm6ds3_handler_empty, NULL,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		goto buffer_cleanup_step_counter;

	err = iio_triggered_buffer_setup(
				cdata->indio_dev[ST_INDIO_DEV_TILT],
				&st_lsm6ds3_handler_empty, NULL,
				&st_lsm6ds3_buffer_setup_ops);
	if (err < 0)
		goto buffer_cleanup_step_detector;

	return 0;

buffer_cleanup_step_detector:
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]);
buffer_cleanup_step_counter:
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]);
buffer_cleanup_sign_motion:
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]);
buffer_cleanup_gyro:
	iio_triggered_buffer_cleanup(cdata->indio_dev[ST_INDIO_DEV_GYRO]);
buffer_cleanup_accel:
	iio_triggered_buffer_cleanup(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
	return err;
}

void st_lsm6ds3_deallocate_rings(struct lsm6ds3_data *cdata)
{
	iio_triggered_buffer_cleanup(cdata->indio_dev[ST_INDIO_DEV_TILT]);
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR]);
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_STEP_COUNTER]);
	iio_triggered_buffer_cleanup(
				cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION]);
	iio_triggered_buffer_cleanup(cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
	iio_triggered_buffer_cleanup(cdata->indio_dev[ST_INDIO_DEV_GYRO]);
}

MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics lsm6ds3 buffer driver");
MODULE_LICENSE("GPL v2");
