/*
 * STMicroelectronics lsm6ds3 trigger driver
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
#include <linux/iio/iio.h>
#include <linux/iio/trigger.h>
#include <linux/interrupt.h>
#include <linux/iio/events.h>

#include "st_lsm6ds3.h"

#define ST_LSM6DS3_SRC_ACCEL_GYRO_REG		0x1e
#define ST_LSM6DS3_SRC_FUNC_REG			0x53

#define ST_LSM6DS3_SRC_ACCEL_DATA_AVL		0x01
#define ST_LSM6DS3_SRC_GYRO_DATA_AVL		0x02
#define ST_LSM6DS3_SRC_SIGN_MOTION_DATA_AVL	0x40
#define ST_LSM6DS3_SRC_STEP_COUNTER_DATA_AVL	0x10
#define ST_LSM6DS3_SRC_TILT_DATA_AVL		0x20

static struct workqueue_struct *st_lsm6ds3_wq = NULL;

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
static int st_lsm6ds3_data_poll(struct iio_dev *indio_dev)
{
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	struct lsm6ds3_data *cdata = sdata->cdata;

	mutex_lock(&indio_dev->mlock);
	cdata->timestamp = iio_get_time_ns();
	iio_trigger_poll_chained(indio_dev->trig, 0);
	mutex_unlock(&indio_dev->mlock);

	return 0;
}

static int st_lsm6ds3_event_poll(struct iio_dev *indio_dev, int sensor)
{
	int err;
	u8 src_value_reg = 0x00;
	struct lsm6ds3_sensor_data *sdata = iio_priv(indio_dev);
	struct lsm6ds3_data *cdata = sdata->cdata;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
				ST_LSM6DS3_SRC_FUNC_REG, 1, &src_value_reg);
	if (err < 0)
		return err;

	mutex_lock(&indio_dev->mlock);

	cdata->timestamp = iio_get_time_ns();

	if (sensor == ST_INDIO_DEV_SIGN_MOTION
) {
		if (src_value_reg & ST_LSM6DS3_SRC_SIGN_MOTION_DATA_AVL) {
			iio_push_event(cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION],
					IIO_UNMOD_EVENT_CODE(IIO_SIGN_MOTION,
					0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
					cdata->timestamp);
		}
	}
	else if (sensor == ST_INDIO_DEV_STEP_DETECTOR
) {
		if (src_value_reg & ST_LSM6DS3_SRC_STEP_COUNTER_DATA_AVL) {
			iio_trigger_poll_chained(
					cdata->trig[ST_INDIO_DEV_STEP_COUNTER], 0);
			iio_push_event(cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR],
					IIO_UNMOD_EVENT_CODE(IIO_STEP_DETECTOR,
					0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
					cdata->timestamp);
		}
	}
	else if (sensor == ST_INDIO_DEV_TILT
) {
		if (src_value_reg & ST_LSM6DS3_SRC_TILT_DATA_AVL) {
			iio_push_event(cdata->indio_dev[ST_INDIO_DEV_TILT],
					IIO_UNMOD_EVENT_CODE(IIO_TILT,
					0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
					cdata->timestamp);
		}
	}

	mutex_unlock(&indio_dev->mlock);

	//dev_info(cdata->dev, "%s run...%d\n", __func__, src_value_reg);

	return 0;
}

void st_lsm6ds3_poll_work_acc(struct work_struct *data_work)
{
	struct lsm6ds3_sensor_data *sdata;

	sdata = container_of((struct work_struct *)data_work,
						struct lsm6ds3_sensor_data, tmr_work);

	if (sdata->cdata->sensors_enabled & (1 << ST_INDIO_DEV_ACCEL))
		st_lsm6ds3_data_poll(sdata->cdata->indio_dev[ST_INDIO_DEV_ACCEL]);
	if (sdata->cdata->sensors_enabled & (1 << ST_INDIO_DEV_SIGN_MOTION))
		st_lsm6ds3_event_poll(sdata->cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION],
													ST_INDIO_DEV_SIGN_MOTION);
	if (sdata->cdata->sensors_enabled & (1 << ST_INDIO_DEV_STEP_DETECTOR))
		st_lsm6ds3_event_poll(sdata->cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR],
													ST_INDIO_DEV_STEP_DETECTOR);
	if (sdata->cdata->sensors_enabled & (1 << ST_INDIO_DEV_TILT))
		st_lsm6ds3_event_poll(sdata->cdata->indio_dev[ST_INDIO_DEV_TILT],
													ST_INDIO_DEV_TILT);

	return;
}
EXPORT_SYMBOL(st_lsm6ds3_poll_work_acc);

void st_lsm6ds3_poll_work_gyr(struct work_struct *data_work)
{
	struct lsm6ds3_sensor_data *sdata;

	sdata = container_of((struct work_struct *)data_work,
						struct lsm6ds3_sensor_data, tmr_work);

	if (sdata->sindex == ST_INDIO_DEV_GYRO)
		st_lsm6ds3_data_poll(sdata->cdata->indio_dev[sdata->sindex]);

	return;
}
EXPORT_SYMBOL(st_lsm6ds3_poll_work_gyr);
#endif

irqreturn_t st_lsm6ds3_save_timestamp(int irq, void *private)
{
	struct lsm6ds3_data *cdata = private;

	cdata->timestamp = st_lsm6ds3_iio_get_boottime_ns();
	queue_work(st_lsm6ds3_wq, &cdata->data_work);

	disable_irq_nosync(irq);

	return IRQ_HANDLED;
}

static void st_lsm6ds3_irq_management(struct work_struct *data_work)
{
	int err;
	struct lsm6ds3_data *cdata;
	u8 src_value_reg1 = 0x00, src_value_reg2 = 0x00;

	cdata = container_of((struct work_struct *)data_work,
						struct lsm6ds3_data, data_work);

	err = cdata->tf->read(&cdata->tb, cdata->dev,
			ST_LSM6DS3_SRC_ACCEL_GYRO_REG, 1, &src_value_reg1);
	if (err < 0)
		goto st_lsm6ds3_irq_enable_irq;

	err = cdata->tf->read(&cdata->tb, cdata->dev,
				ST_LSM6DS3_SRC_FUNC_REG, 1, &src_value_reg2);
	if (err < 0)
		goto st_lsm6ds3_irq_enable_irq;

	if (src_value_reg1 & ST_LSM6DS3_SRC_ACCEL_DATA_AVL)
		iio_trigger_poll_chained(cdata->trig[ST_INDIO_DEV_ACCEL], 0);

	if (src_value_reg1 & ST_LSM6DS3_SRC_GYRO_DATA_AVL)
		iio_trigger_poll_chained(cdata->trig[ST_INDIO_DEV_GYRO], 0);

	if (cdata->patch_feature) {
	if (src_value_reg2 & ST_LSM6DS3_SRC_SIGN_MOTION_DATA_AVL) {
		iio_push_event(cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION],
				IIO_UNMOD_EVENT_CODE(IIO_SIGN_MOTION,
				0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
				cdata->timestamp);
		}
	}

	if (src_value_reg2 & ST_LSM6DS3_SRC_STEP_COUNTER_DATA_AVL) {
		iio_trigger_poll_chained(
				cdata->trig[ST_INDIO_DEV_STEP_COUNTER], 0);
		iio_push_event(cdata->indio_dev[ST_INDIO_DEV_STEP_DETECTOR],
				IIO_UNMOD_EVENT_CODE(IIO_STEP_DETECTOR,
				0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
				cdata->timestamp);

		if ((cdata->sign_motion_event_ready) && (!cdata->patch_feature)) {
			iio_push_event(cdata->indio_dev[ST_INDIO_DEV_SIGN_MOTION],
					IIO_UNMOD_EVENT_CODE(IIO_SIGN_MOTION,
					0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
					cdata->timestamp);

			cdata->sign_motion_event_ready = false;
		}
	}

	if (src_value_reg2 & ST_LSM6DS3_SRC_TILT_DATA_AVL) {
		iio_push_event(cdata->indio_dev[ST_INDIO_DEV_TILT],
				IIO_UNMOD_EVENT_CODE(IIO_TILT,
				0, IIO_EV_TYPE_THRESH, IIO_EV_DIR_EITHER),
				cdata->timestamp);
	}

st_lsm6ds3_irq_enable_irq:
	enable_irq(cdata->irq);
	return;
}

int st_lsm6ds3_allocate_triggers(struct lsm6ds3_data *cdata,
			const struct iio_trigger_ops *trigger_ops, int irq)
{
	int err, i, n;

#if (LSM6DS3_HRTIMER_TRIGGER > 0)
	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		cdata->trig[i] = iio_trigger_alloc("%s-trigger-%d",
				cdata->indio_dev[i]->name, cdata->dev->id);
				//cdata->indio_dev[i]->name, i);
		if (!cdata->trig[i]) {
			dev_err(cdata->dev, "failed to allocate iio trigger.\n");
			err = -ENOMEM;
			goto deallocate_trigger;
		}
		iio_trigger_set_drvdata(cdata->trig[i], cdata->indio_dev[i]);
		cdata->trig[i]->ops = trigger_ops;
		cdata->trig[i]->dev.parent = cdata->dev;
	}
	for (n = 0; n < ST_INDIO_DEV_NUM; n++) {
		err = iio_trigger_register(cdata->trig[n]);
		if (err < 0) {
			dev_err(cdata->dev, "failed to register iio trigger.\n");
			goto unregister_trigger;
		}
		cdata->indio_dev[n]->trig = cdata->trig[n];
	}

	return 0;
#endif

	if (irq <= 0) {
		return 0;
	}

	if (!st_lsm6ds3_wq)
		st_lsm6ds3_wq = create_workqueue(cdata->name);

	if (!st_lsm6ds3_wq)
		return -EINVAL;

	INIT_WORK(&cdata->data_work, st_lsm6ds3_irq_management);

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		cdata->trig[i] = iio_trigger_alloc("%s-trigger-%d",
				cdata->indio_dev[i]->name, cdata->dev->id);
		if (!cdata->trig[i]) {
			dev_err(cdata->dev, "failed to allocate iio trigger.\n");
			err = -ENOMEM;
			goto deallocate_trigger;
		}
		iio_trigger_set_drvdata(cdata->trig[i], cdata->indio_dev[i]);
		cdata->trig[i]->ops = trigger_ops;
		cdata->trig[i]->dev.parent = cdata->dev;
	}

	cdata->irq = irq;

	err = request_threaded_irq(irq, st_lsm6ds3_save_timestamp, NULL,
					IRQF_TRIGGER_HIGH, cdata->name, cdata);
	if (err)
		goto deallocate_trigger;

	for (n = 0; n < ST_INDIO_DEV_NUM; n++) {
		err = iio_trigger_register(cdata->trig[n]);
		if (err < 0) {
			dev_err(cdata->dev, "failed to register iio trigger.\n");
			goto free_irq;
		}
		cdata->indio_dev[n]->trig = cdata->trig[n];
	}

	return 0;

free_irq:
	free_irq(irq, cdata);
#if (LSM6DS3_HRTIMER_TRIGGER > 0)
unregister_trigger:
#endif
	for (n--; n >= 0; n--)
		iio_trigger_unregister(cdata->trig[n]);
deallocate_trigger:
	for (i--; i >= 0; i--)
		iio_trigger_free(cdata->trig[i]);

	return err;
}
EXPORT_SYMBOL(st_lsm6ds3_allocate_triggers);

void st_lsm6ds3_deallocate_triggers(struct lsm6ds3_data *cdata, int irq)
{
	int i;

	if (irq > 0)
		free_irq(irq, cdata);

	for (i = 0; i < ST_INDIO_DEV_NUM; i++) {
		iio_trigger_unregister(cdata->trig[i]);
		iio_trigger_free(cdata->trig[i]);
	}

	if (st_lsm6ds3_wq)
		destroy_workqueue(st_lsm6ds3_wq);
}
EXPORT_SYMBOL(st_lsm6ds3_deallocate_triggers);

MODULE_AUTHOR("Denis Ciocca <denis.ciocca@st.com>");
MODULE_DESCRIPTION("STMicroelectronics lsm6ds3 trigger driver");
MODULE_LICENSE("GPL v2");
