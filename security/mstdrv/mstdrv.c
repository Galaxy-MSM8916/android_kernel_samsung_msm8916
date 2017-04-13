/*
 * @file mstdrv.c
 * @brief MST drv Support
 * Copyright (c) 2015, Samsung Electronics Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/qseecom.h>
#include "mstdrv.h"

/* defines */
#define	ON				1	// On state
#define	OFF				0	// Off state
#define	TRACK1				1	// Track1 data
#define	TRACK2				2	// Track2 data
#define CMD_MST_LDO_OFF			'0'	// MST LDO off
#define CMD_MST_LDO_ON			'1'	// MST LDO on
#define CMD_SEND_TRACK1_DATA		'2'	// Send track1 test data
#define CMD_SEND_TRACK2_DATA		'3'	// send track2 test data
#define CMD_HW_RELIABILITY_TEST_START	'4'	// start HW reliability test
#define CMD_HW_RELIABILITY_TEST_STOP	'5'	// stop HW reliability test
#define ERROR_VALUE			-1	// Error value
#define TEST_RESULT_LEN			256	// result array length
#define SVC_MST_ID			0x000A0000	// need to check ID
#define MST_CREATE_CMD(x)		(SVC_MST_ID | x)	// Create MST commands
#define MST_TA				"mst"

/* enum definitions */
typedef enum {
	MST_CMD_TRACK1_TEST	= MST_CREATE_CMD(0x00000000),
	MST_CMD_TRACK2_TEST	= MST_CREATE_CMD(0x00000001),
	MST_CMD_UNKNOWN		= MST_CREATE_CMD(0x7FFFFFFF)
} mst_cmd_type;

/* struct definitions */
struct qseecom_handle {
    void *dev; /* in/out */
    unsigned char *sbuf; /* in/out */
    uint32_t sbuf_len; /* in/out */
};

typedef struct mst_req_s {
	mst_cmd_type cmd_id;
	uint32_t data;
} __attribute__ ((packed)) mst_req_t;

typedef struct mst_rsp_s {
	uint32_t data;
	uint32_t status;
} __attribute__ ((packed)) mst_rsp_t;

/* extern function declarations */
extern int qseecom_start_app(struct qseecom_handle **handle, char *app_name, uint32_t size);
extern int qseecom_shutdown_app(struct qseecom_handle **handle);
extern int qseecom_send_command(struct qseecom_handle *handle, void *send_buf, uint32_t sbuf_len, void *resp_buf, uint32_t rbuf_len);

/* function declarations */
static ssize_t show_mst_drv(struct device *dev,
        struct device_attribute *attr, char *buf);
static ssize_t store_mst_drv(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
static int mst_ldo_device_probe(struct platform_device *pdev);

/* global variables */
static int escape_loop = 1;		// for HW reliability test
static struct wake_lock   mst_wakelock;	// wake_lock used for HW reliability test
static int mst_ldo_en = 0;		// MST_LDO_EN pin
static int mst_pwr_en = 0;		// MST_PWR_EN pin
static struct class *mst_drv_class;	// mst_drv class
struct device *mst_drv_dev;		// mst_drv driver handle
static struct qseecom_handle *qhandle;	// qseecom handle for TA
static DEVICE_ATTR(transmit, 0770, show_mst_drv, store_mst_drv);	// define device attribute

DEFINE_MUTEX(mst_mutex);

/* device driver structures */
static struct of_device_id mst_match_ldo_table[] = {
	{ .compatible = "sec-mst",},
	{},
};

static struct platform_driver sec_mst_ldo_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "mstldo",
		.of_match_table = mst_match_ldo_table,
	},
	.probe = mst_ldo_device_probe,
};

EXPORT_SYMBOL_GPL(mst_drv_dev);

/**
 * of_mst_hw_onoff - Enable/Disable MST LDO GPIO pin (or Regulator)
 * @on: on/off value
 */
static void of_mst_hw_onoff(bool on){
	if(on) {
		gpio_set_value(mst_ldo_en, ON);
	}else{
		gpio_set_value(mst_ldo_en, OFF);
	}
}
/**
 * transmit_mst_data - Transmit test track data
 * @track: 1:track1, 2:track2
 */
static int transmit_mst_data(int track)
{
	int ret=0;
	int qsee_ret = 0;
	char app_name[MAX_APP_NAME_SIZE];
	mst_req_t *kreq = NULL;
	mst_rsp_t *krsp = NULL;
	int req_len = 0, rsp_len = 0;

	mutex_lock(&mst_mutex);
	snprintf(app_name, MAX_APP_NAME_SIZE, "%s", MST_TA);
	if ( NULL == qhandle ) {
		/* start the mst tzapp only when it is not loaded. */
		qsee_ret = qseecom_start_app(&qhandle, app_name, 1024);
	}

	if ( NULL == qhandle ) {
		/* qhandle is still NULL. It seems we couldn't start mst tzapp. */
		printk("[MST] cannot get tzapp handle from kernel.\n");
		mutex_unlock(&mst_mutex);
		ret=ERROR_VALUE;
		goto exit; /* leave the function now. */
	}

	kreq = (struct mst_req_s *)qhandle->sbuf;

	switch(track)
	{
		case TRACK1: kreq->cmd_id = MST_CMD_TRACK1_TEST;
			     break;

		case TRACK2: kreq->cmd_id = MST_CMD_TRACK2_TEST;
			     break;

		default:ret=ERROR_VALUE;
			goto exit;
			break;
	}

	req_len = sizeof(mst_req_t);
	krsp =(struct mst_rsp_s *)(qhandle->sbuf + req_len);
	rsp_len = sizeof(mst_rsp_t);

	printk("[MST] cmd_id = %x, req_len = %d, rsp_len = %d\n", kreq->cmd_id, req_len, rsp_len);

	of_mst_hw_onoff(ON);
	gpio_set_value(mst_pwr_en, OFF);
	udelay(300);
	gpio_set_value(mst_pwr_en, ON);
	qsee_ret = qseecom_send_command(qhandle, kreq, req_len, krsp, rsp_len);
	gpio_set_value(mst_pwr_en, OFF);
	of_mst_hw_onoff(OFF);

	if (qsee_ret) {
		ret=ERROR_VALUE;
		printk("[MST] failed to send cmd to qseecom; qsee_ret = %d.\n", qsee_ret);
		printk("[MST] shutting down the tzapp.\n");
		qsee_ret = qseecom_shutdown_app(&qhandle);
		if ( qsee_ret ) {
			printk("[MST] failed to shut down the tzapp.\n");
		} else{
			qhandle = NULL;
		}
	}

	if (krsp->status) {
		ret=ERROR_VALUE;
		printk("[MST] generate sample track data from TZ -- failed. %d\n", krsp->status);
	}
	mutex_unlock(&mst_mutex);
exit:
	return ret;
}
/**
 * show_mst_drv - device attribute show sysfs operation
 */
static ssize_t show_mst_drv(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    if (!dev)
        return -ENODEV;

    // todo
    if(escape_loop == 0){
		return sprintf(buf, "%s\n", "activating");
    }else{
		return sprintf(buf, "%s\n", "waiting");
    }
}

/**
 * store_mst_drv - device attribute store sysfs operation
 */
static ssize_t store_mst_drv(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	char test_result[TEST_RESULT_LEN]={0,};

	sscanf(buf, "%20s\n", test_result);

	switch(test_result[0]){

		case CMD_MST_LDO_OFF:
			of_mst_hw_onoff(OFF);
			break;

		case CMD_MST_LDO_ON:
			of_mst_hw_onoff(ON);
			break;

		case CMD_SEND_TRACK1_DATA:
			if(transmit_mst_data(TRACK1))
				printk("[MST] Send track1 data --> failed\n");
			else
				printk("[MST] Send track1 data --> successful\n");
			break;

		case CMD_SEND_TRACK2_DATA:
			if(transmit_mst_data(TRACK2))
				printk("[MST] Send track2 data --> failed\n");
			else
				printk("[MST] Send track2 data --> successful\n");
			break;

		case CMD_HW_RELIABILITY_TEST_START:
			if(escape_loop){
				wake_lock_init(&mst_wakelock, WAKE_LOCK_SUSPEND, "mst_wakelock");
				wake_lock(&mst_wakelock);
			}
			escape_loop = 0;
			while( 1 ) {
				if(escape_loop == 1)
					break;
				mdelay(10);
				if (transmit_mst_data(TRACK2)) {
					printk("[MST] Send track2 data --> failed\n");
					break;
				}
				printk("[MST] Send track2 data --> successful\n");
				mdelay(1000);
			}
			break;

		case CMD_HW_RELIABILITY_TEST_STOP:
			if(!escape_loop)
				wake_lock_destroy(&mst_wakelock);
			escape_loop = 1;
			printk("[MST] MST escape_loop value = 1\n");
			break;

		default:
			printk(KERN_ERR "[MST] MST invalid value : %s\n", test_result);
			break;
	}
	return count;
}

/**
 * sec_mst_gpio_init - Initialize GPIO pins used by driver
 * @dev: driver handle
 */
static int sec_mst_gpio_init(struct device *dev)
{
	int ret = 0;

	mst_pwr_en = of_get_named_gpio(dev->of_node, "sec-mst,mst-pwr-gpio", OFF);
	mst_ldo_en = of_get_named_gpio(dev->of_node, "sec-mst,mst-ldo-gpio", OFF);

	/* check if gpio pin is inited */
	if (mst_pwr_en < 0) {
		printk(KERN_ERR "[MST] %s : Cannot create the gpio\n", __func__);
		return 1;
	}
	/* gpio request */
	ret = gpio_request(mst_pwr_en, "sec-mst,mst-pwr-gpio");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get en gpio : %d, %d\n", ret, mst_pwr_en);
	}
	/* set gpio direction */
	if (!(ret < 0)  && (mst_pwr_en > 0)) {
		gpio_direction_output(mst_pwr_en, OFF);
		printk(KERN_ERR "[MST] %s : Send Output\n", __func__);
	}

	if (mst_ldo_en < 0) {
		printk(KERN_ERR "[MST] %s : Cannot create the gpio\n", __func__);
		return 1;
	}
	ret = gpio_request(mst_ldo_en, "sec-mst,mst-ldo-gpio");
	if (ret) {
		printk(KERN_ERR "[MST] failed to get en gpio : %d, %d\n", ret, mst_ldo_en);
	}
	if (!(ret < 0)  && (mst_ldo_en > 0)) {
		gpio_direction_output(mst_ldo_en, OFF);
		printk(KERN_ERR "[MST] %s : Send Output\n", __func__);
	}

	return 0;
}

/**
 * mst_ldo_device_probe - Driver probe function
 */
static int mst_ldo_device_probe(struct platform_device *pdev)
{
	int retval = 0;

	printk("[MST] %s init start\n", __func__);

	if (sec_mst_gpio_init(&pdev->dev)) {
		retval = ERROR_VALUE;
		printk(KERN_ERR "[MST] %s: driver initialization failed, retval : %d\n", __FILE__, retval);
		goto done;
	}

	mst_drv_class = class_create(THIS_MODULE, "mstldo");
	if (IS_ERR(mst_drv_class)) {
		retval = PTR_ERR(mst_drv_class);
		printk(KERN_ERR "[MST] %s: driver initialization failed, retval : %d\n", __FILE__, retval);
		goto done;
	}

	mst_drv_dev = device_create(mst_drv_class,
			NULL /* parent */, 0 /* dev_t */, NULL /* drvdata */,
			MST_DRV_DEV);
	if (IS_ERR(mst_drv_dev)) {
		retval = PTR_ERR(mst_drv_dev);
		kfree(mst_drv_dev);
		device_destroy(mst_drv_class, 0);
		printk(KERN_ERR "[MST] %s: driver initialization failed, retval : %d\n", __FILE__, retval);
		goto done;
	}

	/* register this mst device with the driver core */
	retval = device_create_file(mst_drv_dev, &dev_attr_transmit);
	if (retval)
	{
		kfree(mst_drv_dev);
		device_destroy(mst_drv_class, 0);
		printk(KERN_ERR "[MST] %s: driver initialization failed, retval : %d\n", __FILE__, retval);
		goto done;
	}

	printk(KERN_DEBUG "[MST] MST drv driver (%s) is initialized.\n", MST_DRV_DEV);

done:
	return retval;
}

/**
 * mst_drv_init - Driver init function
 */
static int __init mst_drv_init(void)
{
	int ret=0;
	printk(KERN_ERR "[MST] %s\n", __func__);
	ret = platform_driver_register(&sec_mst_ldo_driver);
	printk(KERN_ERR "[MST] MST_LDO_DRV]]] init , ret val : %d\n",ret);
	return ret;
}

/**
 * mst_drv_exit - Driver exit function
 */
static void __exit mst_drv_exit (void)
{
    class_destroy(mst_drv_class);
    printk(KERN_ALERT "[MST] %s\n", __func__);
}

MODULE_AUTHOR("JASON KANG, j_seok.kang@samsung.com");
MODULE_DESCRIPTION("MST drv driver");
MODULE_VERSION("0.1");

module_init(mst_drv_init);
module_exit(mst_drv_exit);
