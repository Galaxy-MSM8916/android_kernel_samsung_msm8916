/*
 * MELFAS MMS400 Touchscreen
 *
 * Copyright (C) 2014 MELFAS Inc.
 *
 */

#include "melfas_mms400.h"

#if MMS_USE_NAP_MODE
struct wake_lock mms_wake_lock;
#endif

/**
 * Reboot chip
 *
 * Caution : IRQ must be disabled before mms_reboot and enabled after mms_reboot.
 */
void mms_reboot(struct mms_ts_info *info)
{
	struct i2c_adapter *adapter = to_i2c_adapter(info->client->dev.parent);

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	i2c_lock_adapter(adapter);

	mms_power_control(info, 0);
	mms_power_control(info, 1);

	i2c_unlock_adapter(adapter);

	msleep(30);

	if (info->glove_mode)
		mms_enable_glove_mode(info, 1);

	dev_info(&info->client->dev, "%s [DONE]\n", __func__);
}

/**
 * I2C Read
 */
int mms_i2c_read(struct mms_ts_info *info, char *write_buf, unsigned int write_len,
		char *read_buf, unsigned int read_len)
{
	int retry = I2C_RETRY_COUNT;
	int res;

	struct i2c_msg msg[] = {
		{
			.addr = info->client->addr,
			.flags = 0,
			.buf = write_buf,
			.len = write_len,
		}, {
			.addr = info->client->addr,
			.flags = I2C_M_RD,
			.buf = read_buf,
			.len = read_len,
		},
	};

	while (retry--) {
		res = i2c_transfer(info->client->adapter, msg, ARRAY_SIZE(msg));

		if (res == ARRAY_SIZE(msg)) {
			goto DONE;
		} else if (res < 0) {
			dev_err(&info->client->dev,
					"%s [ERROR] i2c_transfer - errno[%d]\n", __func__, res);
		} else if (res != ARRAY_SIZE(msg)) {
			dev_err(&info->client->dev,
					"%s [ERROR] i2c_transfer - size[%d] result[%d]\n",
					__func__, ARRAY_SIZE(msg), res);
		} else {
			dev_err(&info->client->dev,
					"%s [ERROR] unknown error [%d]\n", __func__, res);
		}
	}

	goto ERROR_REBOOT;

ERROR_REBOOT:
	mms_reboot(info);
	return 1;

DONE:
	return 0;
}


/**
 * I2C Read (Continue)
 */
int mms_i2c_read_next(struct mms_ts_info *info, char *read_buf, int start_idx,
		unsigned int read_len)
{
	int retry = I2C_RETRY_COUNT;
	int res;
	u8 rbuf[read_len];

	while (retry--) {
		res = i2c_master_recv(info->client, rbuf, read_len);

		if (res == read_len) {
			goto DONE;
		} else if (res < 0) {
			dev_err(&info->client->dev,
					"%s [ERROR] i2c_master_recv - errno [%d]\n", __func__, res);
		} else if (res != read_len) {
			dev_err(&info->client->dev,
					"%s [ERROR] length mismatch - read[%d] result[%d]\n",
					__func__, read_len, res);
		} else {
			dev_err(&info->client->dev,
					"%s [ERROR] unknown error [%d]\n", __func__, res);
		}
	}

	goto ERROR_REBOOT;

ERROR_REBOOT:
	mms_reboot(info);
	return 1;

DONE:
	memcpy(&read_buf[start_idx], rbuf, read_len);

	return 0;
}

/**
 * I2C Write
 */
int mms_i2c_write(struct mms_ts_info *info, char *write_buf, unsigned int write_len)
{
	int retry = I2C_RETRY_COUNT;
	int res;

	while (retry--) {
		res = i2c_master_send(info->client, write_buf, write_len);

		if (res == write_len) {
			goto DONE;
		} else if (res < 0) {
			dev_err(&info->client->dev,
					"%s [ERROR] i2c_master_send - errno [%d]\n", __func__, res);
		} else if (res != write_len) {
			dev_err(&info->client->dev,
					"%s [ERROR] length mismatch - write[%d] result[%d]\n",
					__func__, write_len, res);
		} else {
			dev_err(&info->client->dev,
					"%s [ERROR] unknown error [%d]\n", __func__, res);
		}
	}

	goto ERROR_REBOOT;

ERROR_REBOOT:
	mms_reboot(info);
	return 1;

DONE:
	return 0;
}

/**
 * Enable device
 */
int mms_enable(struct mms_ts_info *info)
{
	dev_dbg(&info->client->dev, "%s [START]\n", __func__);


	if (info->enabled) {
		dev_err(&info->client->dev,
				"%s : already enabled\n", __func__);
		return 0;
	}

	mutex_lock(&info->lock);

	if (!info->init)
		mms_power_control(info, 1);
	mms_pinctrl_configure(info, 1);

	enable_irq(info->client->irq);
	info->enabled = true;

	mutex_unlock(&info->lock);

#ifdef CONFIG_SEC_FACTORY
#ifdef APPLY_RESOLUTION
	if (!info->dtdata->fix_resolution) {
		u8 rbuf[8];
		mms_get_fw_version(info, rbuf);
		dev_err(&info->client->dev,
				"%s check fw ver : %x\n", __func__, info->config_ver_ic);
	}
#endif
#endif
	if (info->disable_esd == true) {
		mms_disable_esd_alert(info);
	}

	if (info->glove_mode)
		mms_enable_glove_mode(info, 1);

	dev_err(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;
}

/**
 * Disable device
 */
int mms_disable(struct mms_ts_info *info)
{
	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	if (!info->enabled) {
		dev_err(&info->client->dev,
				"%s : already disabled\n", __func__);
		return 0;
	}

	mutex_lock(&info->lock);

	info->enabled = false;
	disable_irq(info->client->irq);
	mms_clear_input(info);
	mms_power_control(info, 0);
	mms_pinctrl_configure(info, 0);

	mutex_unlock(&info->lock);

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_off(info->booster);
#endif

	dev_err(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;
}

#if MMS_USE_INPUT_OPEN_CLOSE
/**
 * Open input device
 */
static int mms_input_open(struct input_dev *dev)
{
	struct mms_ts_info *info = input_get_drvdata(dev);

	if (!info->init)
		mms_enable(info);

	return 0;
}

/**
 * Close input device
 */
static void mms_input_close(struct input_dev *dev)
{
	struct mms_ts_info *info = input_get_drvdata(dev);
	mms_disable(info);
	return;
}
#endif

#if defined(CONFIG_TOUCHSCREEN_DUMP_MODE)

struct delayed_work * p_ghost_check;
void run_intensity_for_ghosttouch(struct mms_ts_info *info)
{
	if (mms_get_image(info, MIP_IMG_TYPE_INTENSITY)) {
		dev_err(&info->client->dev, "%s \n", "NG");
	}
}
static void mms_ghost_touch_check(struct work_struct *work)
{
	struct mms_ts_info *info = container_of(work, struct mms_ts_info,
			ghost_check.work);
	int i;

	if (info->tsp_dump_lock == 1) {
		printk(KERN_ERR "%s, ignored ## already checking..\n", __func__);
		return;
	}

	info->tsp_dump_lock = 1;
	info->add_log_header = 1;
	for (i = 0; i < 5; i++) {
		dev_err(&info->client->dev, "%s, start ##\n", __func__);
		run_intensity_for_ghosttouch((void *)info);
		msleep(100);

	}
	dev_err(&info->client->dev, "%s, done ##\n", __func__);
	info->tsp_dump_lock = 0;
	info->add_log_header = 0;

}

void dump_tsp_log(void)
{
	printk(KERN_ERR "mms %s: start \n", __func__);

#if defined(CONFIG_SAMSUNG_LPM_MODE)
	if (poweroff_charging) {
		printk(KERN_ERR "%s, ignored ## lpm charging Mode!!\n", __func__);
		return;
	}
#endif
	if (p_ghost_check == NULL) {
		printk(KERN_ERR "%s, ignored ## tsp probe fail!!\n", __func__);
		return;
	}
	schedule_delayed_work(p_ghost_check, msecs_to_jiffies(100));
}
#else
void dump_tsp_log(void)
{
	printk(KERN_ERR "FTS %s: not support\n", __func__);
}

#endif

/**
 * Get ready status
 */
int mms_get_ready_status(struct mms_ts_info *info)
{
	u8 wbuf[16];
	u8 rbuf[16];
	int ret = 0;

	dev_info(&info->client->dev, "%s [START]\n", __func__);

	wbuf[0] = MIP_R0_CTRL;
	wbuf[1] = MIP_R1_CTRL_READY_STATUS;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 1)) {
		dev_err(&info->client->dev, "%s [ERROR] mms_i2c_read\n", __func__);
		goto ERROR;
	}
	ret = rbuf[0];

	//check status
	if ((ret == MIP_CTRL_STATUS_NONE) || (ret == MIP_CTRL_STATUS_LOG)
			|| (ret == MIP_CTRL_STATUS_READY)) {
		dev_info(&info->client->dev, "%s - status [0x%02X]\n", __func__, ret);
	} else {
		dev_err(&info->client->dev,
				"%s [ERROR] Unknown status [0x%02X]\n", __func__, ret);
		goto ERROR;
	}

	if (ret == MIP_CTRL_STATUS_LOG) {
		//skip log event
		wbuf[0] = MIP_R0_LOG;
		wbuf[1] = MIP_R1_LOG_TRIGGER;
		wbuf[2] = 0;
		if (mms_i2c_write(info, wbuf, 3)) {
			dev_err(&info->client->dev, "%s [ERROR] mms_i2c_write\n", __func__);
		}
	}

	dev_info(&info->client->dev, "%s [DONE]\n", __func__);
	return ret;

ERROR:
	dev_err(&info->client->dev, "%s [ERROR]\n", __func__);
	return -1;
}

/**
 * Read chip firmware version
 */
int mms_get_fw_version(struct mms_ts_info *info, u8 *ver_buf)
{
	u8 rbuf[8];
	u8 wbuf[2];
	int i;

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_VERSION_BOOT;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 8)) {
		goto ERROR;
	};

	for (i = 0; i < MMS_FW_MAX_SECT_NUM; i++) {
		ver_buf[0 + i * 2] = rbuf[1 + i * 2];
		ver_buf[1 + i * 2] = rbuf[0 + i * 2];
	}

	info->boot_ver_ic = ver_buf[1];
	info->core_ver_ic = ver_buf[3];
	info->config_ver_ic = ver_buf[5];

	dev_info(&info->client->dev,
			"%s: boot:%x.%x core:%x.%x custom:%x.%d parameter:%x.%x\n",
			__func__,ver_buf[0],ver_buf[1],ver_buf[2],ver_buf[3],ver_buf[4]
			,ver_buf[5],ver_buf[6],ver_buf[7]);

	return 0;

ERROR:
	dev_err(&info->client->dev, "%s [ERROR]\n", __func__);
	return 1;
}

/**
 * Read chip firmware version for u16
 */
int mms_get_fw_version_u16(struct mms_ts_info *info, u16 *ver_buf_u16)
{
	u8 rbuf[8];
	int i;

	if (mms_get_fw_version(info, rbuf)) {
		goto ERROR;
	}

	for (i = 0; i < MMS_FW_MAX_SECT_NUM; i++) {
		ver_buf_u16[i] = (rbuf[0 + i * 2] << 8) | rbuf[1 + i * 2];
	}

	return 0;

ERROR:
	dev_err(&info->client->dev, "%s [ERROR]\n", __func__);
	return 1;
}

/**
 * Disable ESD alert
 */
int mms_disable_esd_alert(struct mms_ts_info *info)
{
	u8 wbuf[4];
	u8 rbuf[4];

	dev_info(&info->client->dev, "%s [START]\n", __func__);

	wbuf[0] = MIP_R0_CTRL;
	wbuf[1] = MIP_R1_CTRL_DISABLE_ESD_ALERT;
	wbuf[2] = 1;
	if (mms_i2c_write(info, wbuf, 3)) {
		dev_err(&info->client->dev, "%s [ERROR] mms_i2c_write\n", __func__);
		goto ERROR;
	}

	if (mms_i2c_read(info, wbuf, 2, rbuf, 1)) {
		dev_err(&info->client->dev, "%s [ERROR] mms_i2c_read\n", __func__);
		goto ERROR;
	}

	if (rbuf[0] != 1) {
		dev_info(&info->client->dev, "%s [ERROR] failed\n", __func__);
		goto ERROR;
	}

	dev_info(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;

ERROR:
	dev_err(&info->client->dev, "%s [ERROR]\n", __func__);
	return 1;
}

/**
 * Alert event handler - ESD
 */
static int mms_alert_handler_esd(struct mms_ts_info *info, u8 *rbuf)
{
	u8 frame_cnt = rbuf[2];

	dev_info(&info->client->dev, "%s [START] - frame_cnt[%d]\n",
			__func__, frame_cnt);

	if (frame_cnt == 0) {
		//sensor crack, not ESD
		info->esd_cnt++;
		dev_info(&info->client->dev, "%s - esd_cnt[%d]\n",
				__func__, info->esd_cnt);

		if (info->disable_esd == true) {
			mms_disable_esd_alert(info);
		} else if (info->esd_cnt > ESD_COUNT_FOR_DISABLE) {
			//Disable ESD alert
			if (mms_disable_esd_alert(info))
				dev_err(&info->client->dev,
						"%s - fail to disable esd alert\n", __func__);
			else
				info->disable_esd = true;
		} else {
			//Reset chip
			mms_reboot(info);
		}
	} else {
		//ESD detected
		//Reset chip
		mms_reboot(info);
		info->esd_cnt = 0;
	}

	dev_info(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;
}

/**
 * Interrupt handler
 */
static irqreturn_t mms_interrupt(int irq, void *dev_id)
{
	struct mms_ts_info *info = dev_id;
	struct i2c_client *client = info->client;
	u8 wbuf[8];
	u8 rbuf[256];
	unsigned int size = 0;
	int event_size = info->event_size;
	u8 category = 0;
	u8 alert_type = 0;

	dev_dbg(&client->dev, "%s [START]\n", __func__);

	//Read first packet
	wbuf[0] = MIP_R0_EVENT;
	wbuf[1] = MIP_R1_EVENT_PACKET_INFO;
	if (mms_i2c_read(info, wbuf, 2, rbuf, (1 + event_size))) {
		dev_err(&client->dev, "%s [ERROR] Read packet info\n", __func__);
		goto ERROR;
	}

	dev_dbg(&client->dev, "%s - info [0x%02X]\n", __func__, rbuf[0]);

	//Check event
	size = (rbuf[0] & 0x7F);

	dev_dbg(&client->dev, "%s - packet size [%d]\n", __func__, size);

	category = ((rbuf[0] >> 7) & 0x1);
	if (category == 0) {
		//Touch event
		if (size > event_size) {
			//Read next packet
			if (mms_i2c_read_next(info, rbuf, (1 + event_size), (size - event_size))) {
				dev_err(&client->dev, "%s [ERROR] Read next packet\n", __func__);
				goto ERROR;
			}
		}
		info->esd_cnt = 0;
		mms_input_event_handler(info, size, rbuf);
	} else {
		//Alert event
		alert_type = rbuf[1];

		dev_dbg(&client->dev, "%s - alert type [%d]\n", __func__, alert_type);

		if (alert_type == MIP_ALERT_ESD) {
			//ESD detection
			if (mms_alert_handler_esd(info, rbuf)) {
				goto ERROR;
			}
		} else {
			dev_err(&client->dev, "%s [ERROR] Unknown alert type [%d]\n",
					__func__, alert_type);
			goto ERROR;
		}
	}

	dev_dbg(&client->dev, "%s [DONE]\n", __func__);
	return IRQ_HANDLED;

ERROR:
	dev_err(&client->dev, "%s [ERROR]\n", __func__);
	if (RESET_ON_EVENT_ERROR) {
		dev_info(&client->dev, "%s - Reset on error\n", __func__);

		mms_disable(info);
		mms_clear_input(info);
		mms_enable(info);
	}
	return IRQ_HANDLED;
}

/**
 * Update firmware from kernel built-in binary
 */
int mms_fw_update_from_kernel(struct mms_ts_info *info, bool force, bool ffu)
{
	const char *fw_name = INTERNAL_FW_PATH;
	const char *ffu_fw_name = FFU_FW_PATH;
	const struct firmware *fw;
	int retires = 3;
	int ret;

	dev_err(&info->client->dev, "%s [START] %s, force %d\n",
			__func__, ffu ? "FFU" : "INTERNAL", force);

	//Disable IRQ
	mutex_lock(&info->lock);
	disable_irq(info->client->irq);
	mms_clear_input(info);

	//Get firmware
	if (ffu)
		request_firmware(&fw, ffu_fw_name, &info->client->dev);
	else if (info->dtdata->fw_path)
		request_firmware(&fw, info->dtdata->fw_path, &info->client->dev);
	else
		request_firmware(&fw, fw_name, &info->client->dev);

	if (!fw) {
		dev_err(&info->client->dev, "%s [ERROR] request_firmware\n", __func__);
		goto ERROR;
	}

	//Update fw
	do {
		ret = mms_flash_fw(info, fw->data, fw->size, force, true);
		if (ret >= fw_err_none) {
			break;
		}
	} while (--retires);

	if (!retires) {
		dev_err(&info->client->dev, "%s [ERROR] mms_flash_fw failed\n", __func__);
		ret = -1;
	}

	release_firmware(fw);

	//Enable IRQ
	enable_irq(info->client->irq);
	mutex_unlock(&info->lock);

	if (ret < 0) {
		goto ERROR;
	}

	dev_err(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;

ERROR:
	dev_err(&info->client->dev, "%s [ERROR]\n", __func__);
	return -1;
}

/**
 * Update firmware from external storage
 */
int mms_fw_update_from_storage(struct mms_ts_info *info, bool force)
{
	struct file *fp;
	mm_segment_t old_fs;
	size_t fw_size, nread;
	int ret = 0;

	dev_err(&info->client->dev, "%s [START]\n", __func__);

	//Disable IRQ
	mutex_lock(&info->lock);
	disable_irq(info->client->irq);
	mms_clear_input(info);

	//Get firmware
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open(EXTERNAL_FW_PATH, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
		dev_err(&info->client->dev, "%s [ERROR] file_open - path[%s]\n",
				__func__, EXTERNAL_FW_PATH);
		ret = fw_err_file_open;
		goto ERROR;
	}

	fw_size = fp->f_path.dentry->d_inode->i_size;
	if (0 < fw_size) {
		unsigned char *fw_data;
		fw_data = kzalloc(fw_size, GFP_KERNEL);
		nread = vfs_read(fp, (char __user *)fw_data, fw_size, &fp->f_pos);
		dev_info(&info->client->dev, "%s - path [%s] size [%u]\n",
				__func__,EXTERNAL_FW_PATH, fw_size);

		if (nread != fw_size) {
			dev_err(&info->client->dev, "%s [ERROR] vfs_read - size[%d] read[%d]\n",
					__func__, fw_size, nread);
			ret = fw_err_file_read;
		} else {
			//Update fw
			ret = mms_flash_fw(info, fw_data, fw_size, force, true);
		}

		kfree(fw_data);
	} else {
		dev_err(&info->client->dev, "%s [ERROR] fw_size [%d]\n", __func__, fw_size);
		ret = fw_err_file_read;
	}

	filp_close(fp, current->files);

ERROR:
	set_fs(old_fs);

	//Enable IRQ
	enable_irq(info->client->irq);
	mutex_unlock(&info->lock);

	if (ret == 0)
		dev_err(&info->client->dev, "%s [DONE]\n", __func__);
	else
		dev_err(&info->client->dev, "%s [ERROR]\n", __func__);

	return ret;
}

static ssize_t mms_sys_fw_update(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mms_ts_info *info = i2c_get_clientdata(client);
	int result = 0;
	u8 data[255];
	int ret = 0;

	memset(info->print_buf, 0, PAGE_SIZE);

	dev_info(&info->client->dev, "%s [START]\n", __func__);

	ret = mms_fw_update_from_storage(info, true);

	switch (ret) {
	case fw_err_none:
		sprintf(data, "F/W update success\n");
		break;
	case fw_err_uptodate:
		sprintf(data, "F/W is already up-to-date\n");
		break;
	case fw_err_download:
		sprintf(data, "F/W update failed : Download error\n");
		break;
	case fw_err_file_type:
		sprintf(data, "F/W update failed : File type error\n");
		break;
	case fw_err_file_open:
		sprintf(data, "F/W update failed : File open error [%s]\n", EXTERNAL_FW_PATH);
		break;
	case fw_err_file_read:
		sprintf(data, "F/W update failed : File read error\n");
		break;
	default:
		sprintf(data, "F/W update failed\n");
		break;
	}

	dev_info(&info->client->dev, "%s [DONE]\n", __func__);

	strcat(info->print_buf, data);
	result = snprintf(buf, PAGE_SIZE, "%s\n", info->print_buf);
	return result;
}
static DEVICE_ATTR(fw_update, 0666, mms_sys_fw_update, NULL);

/**
 * Sysfs attr info
 */
static struct attribute *mms_attrs[] = {
	&dev_attr_fw_update.attr,
	NULL,
};

/**
 * Sysfs attr group info
 */
static const struct attribute_group mms_attr_group = {
	.attrs = mms_attrs,
};

/**
 * Initial config
 */
static int mms_init_config(struct mms_ts_info *info)
{
	u8 wbuf[8];
	u8 rbuf[32];
	u8 tmp[4] = MMS_CONFIG_DATE;

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	/* read product name */
	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_PRODUCT_NAME;
	mms_i2c_read(info, wbuf, 2, rbuf, 16);
	memcpy(info->product_name, rbuf, 16);
	dev_info(&info->client->dev, "%s - product_name[%s]\n",
			__func__, info->product_name);

	/* read fw version */
	mms_get_fw_version(info, rbuf);

	/* read fw build date */
	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_BUILD_DATE;
	mms_i2c_read(info, wbuf, 2, rbuf, 4);

	if (!strncmp(rbuf, "", 4))
		memcpy(rbuf, tmp, 4);

	info->fw_year = (rbuf[0] << 8) | (rbuf[1]);
	info->fw_month = rbuf[2];
	info->fw_date = rbuf[3];

	dev_info(&info->client->dev, "%s - fw build date : %d/%d/%d\n",
			__func__, info->fw_year, info->fw_month, info->fw_date);

	/* read checksum */
	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_CHECKSUM_PRECALC;
	mms_i2c_read(info, wbuf, 2, rbuf, 4);

	info->pre_chksum = (rbuf[0] << 8) | (rbuf[1]);
	info->rt_chksum = (rbuf[2] << 8) | (rbuf[3]);
	dev_info(&info->client->dev,
			"%s - precalced checksum:%04X, real-time checksum:%04X\n",
			__func__, info->pre_chksum, info->rt_chksum);


	/* Set resolution using chip info */
	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_RESOLUTION_X;
	mms_i2c_read(info, wbuf, 2, rbuf, 7);

	info->max_x = (rbuf[0]) | (rbuf[1] << 8);
	info->max_y = (rbuf[2]) | (rbuf[3] << 8);
	dev_info(&info->client->dev, "%s - max_x[%d] max_y[%d]\n",
			__func__, info->max_x, info->max_y);

	info->node_x = rbuf[4];
	info->node_y = rbuf[5];
	info->node_key = rbuf[6];
	dev_info(&info->client->dev, "%s - node_x[%d] node_y[%d] node_key[%d]\n",
			__func__, info->node_x, info->node_y, info->node_key);

#if MMS_USE_TOUCHKEY
	/* Enable touchkey */
	if (info->node_key > 0) {
		info->tkey_enable = true;
	}
#endif
	info->event_size = 8;

	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);
	return 0;
}

/**
 * Initialize driver
 */
static int mms_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct mms_ts_info *info;
	struct input_dev *input_dev;
	int ret = 0;

	pr_err("%s [START]\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
				"%s [ERROR] i2c_check_functionality\n", __func__);
		ret = -EIO;
		goto ERROR;
	}

	if (get_lcd_attached("GET") == 0) {
		pr_err("%s : LCD is not attached\n", __func__);
		ret = -ENODEV;
		goto ERROR;
	}

	info = kzalloc(sizeof(struct mms_ts_info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "%s [ERROR] info alloc\n", __func__);
		ret = -ENOMEM;
		goto err_mem_alloc;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev, "%s [ERROR] input alloc\n", __func__);
		ret = -ENOMEM;
		goto err_input_alloc;
	}

	info->client = client;
	info->input_dev = input_dev;
	info->irq = -1;
	info->init = true;
	mutex_init(&info->lock);
	info->touch_count = 0;

#if MMS_USE_DEVICETREE
	if (client->dev.of_node) {
		info->dtdata  =
			devm_kzalloc(&client->dev,
					sizeof(struct mms_devicetree_data), GFP_KERNEL);
		if (!info->dtdata) {
			dev_err(&client->dev,
					"%s [ERROR] dtdata devm_kzalloc\n", __func__);
			goto err_devm_alloc;
		}
		mms_parse_devicetree(&client->dev, info);
	} else
#endif
	{
		info->dtdata = client->dev.platform_data;
		if (info->dtdata == NULL) {
			dev_err(&client->dev, "%s [ERROR] dtdata is null\n", __func__);
			ret = -EINVAL;
			goto err_platform_data;
		}
	}

	info->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(info->pinctrl)) {
		if (PTR_ERR(info->pinctrl) == -EPROBE_DEFER)
			goto err_platform_data;

		pr_err("%s: Target does not use pinctrl\n", __func__);
		info->pinctrl = NULL;
	}


	if (info->pinctrl) {
		ret = mms_pinctrl_configure(info, 1);
		if (ret)
			pr_err("%s: cannot set pinctrl state\n", __func__);
	}

	snprintf(info->phys, sizeof(info->phys), "%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchscreen";
	input_dev->phys = info->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
#if MMS_USE_INPUT_OPEN_CLOSE
	input_dev->open = mms_input_open;
	input_dev->close = mms_input_close;
#endif

	input_set_events_per_packet(input_dev, 200);
	input_set_drvdata(input_dev, info);
	i2c_set_clientdata(client, info);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "%s [ERROR] input_register_device\n", __func__);
		ret = -EIO;
		goto err_input_register_device;
	}

#ifdef CONFIG_INPUT_BOOSTER
	info->booster = input_booster_allocate(INPUT_BOOSTER_ID_TSP);
	if (!info->booster) {
		dev_err(&client->dev,
				"%s [ERROR] failed to allocate input booster\n", __func__);
		goto error_alloc_booster_failed;
	}
#endif

	mms_power_control(info, 1);

#if MMS_USE_AUTO_FW_UPDATE
	ret = mms_fw_update_from_kernel(info, false, false);
	if (ret) {
		dev_err(&client->dev, "%s [ERROR] mms_fw_update_from_kernel\n", __func__);
		goto error_fw_update;
	}
#endif

	mms_init_config(info);
	mms_config_input(info);

#ifdef USE_TSP_TA_CALLBACKS
	info->register_cb = mms_register_callback;
	info->callbacks.inform_charger = mms_charger_status_cb;
	if (info->register_cb)
		info->register_cb(&info->callbacks);
#endif
#ifdef CHARGER_NOTIFIER
	mms_set_tsp_info(info);
#endif
	ret = request_threaded_irq(client->irq, NULL, mms_interrupt,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, MMS_DEVICE_NAME, info);
	if (ret) {
		dev_err(&client->dev, "%s [ERROR] request_threaded_irq\n", __func__);
		goto err_request_irq;
	}

	disable_irq(client->irq);
	info->irq = client->irq;

#if MMS_USE_NAP_MODE
	//Wake lock for nap mode
	wake_lock_init(&mms_wake_lock, WAKE_LOCK_SUSPEND, "mms_wake_lock");
#endif

	mms_enable(info);

#if MMS_USE_DEV_MODE
	if (mms_dev_create(info)) {
		dev_err(&client->dev, "%s [ERROR] mms_dev_create\n", __func__);
		ret = -EAGAIN;
		goto err_test_dev_create;
	}

	info->class = class_create(THIS_MODULE, MMS_DEVICE_NAME);
	device_create(info->class, NULL, info->mms_dev, NULL, MMS_DEVICE_NAME);
#endif

#if MMS_USE_TEST_MODE
	if (mms_sysfs_create(info)) {
		dev_err(&client->dev, "%s [ERROR] mms_sysfs_create\n", __func__);
		ret = -EAGAIN;
		goto err_test_sysfs_create;
	}
#endif

#if MMS_USE_CMD_MODE
	if (mms_sysfs_cmd_create(info)) {
		dev_err(&client->dev, "%s [ERROR] mms_sysfs_cmd_create\n", __func__);
		ret = -EAGAIN;
		goto err_fac_cmd_create;
	}
#endif

	if (sysfs_create_group(&client->dev.kobj, &mms_attr_group)) {
		dev_err(&client->dev, "%s [ERROR] sysfs_create_group\n", __func__);
		ret = -EAGAIN;
		goto err_create_attr_group;
	}

	if (sysfs_create_link(NULL, &client->dev.kobj, MMS_DEVICE_NAME)) {
		dev_err(&client->dev, "%s [ERROR] sysfs_create_link\n", __func__);
		ret = -EAGAIN;
		goto err_create_dev_link;
	}

#if defined(CONFIG_TOUCHSCREEN_DUMP_MODE)
	INIT_DELAYED_WORK(&info->ghost_check, mms_ghost_touch_check);
	p_ghost_check = &info->ghost_check;
#endif

	info->init = false;
	dev_info(&client->dev,
			"MELFAS " CHIP_NAME " Touchscreen is initialized successfully\n");
	return 0;


err_create_dev_link:
	sysfs_remove_group(&client->dev.kobj, &mms_attr_group);
err_create_attr_group:
#if MMS_USE_CMD_MODE
	mms_sysfs_cmd_remove(info);
err_fac_cmd_create:
#endif
#if MMS_USE_TEST_MODE
	mms_sysfs_remove(info);
err_test_sysfs_create:
#endif
#if MMS_USE_DEV_MODE
	device_destroy(info->class, info->mms_dev);
	class_destroy(info->class);
err_test_dev_create:
#endif
	mms_disable(info);
	free_irq(info->irq, info);
err_request_irq:
error_fw_update:
#ifdef CONFIG_INPUT_BOOSTER
	input_booster_free(info->booster);
	info->booster = NULL;
error_alloc_booster_failed:
#endif
	input_unregister_device(info->input_dev);
	info->input_dev = NULL;
err_input_register_device:
err_platform_data:
#if MMS_USE_DEVICETREE
err_devm_alloc:
#endif
	if (info->input_dev)
		input_free_device(info->input_dev);
err_input_alloc:
	kfree(info);
err_mem_alloc:
ERROR:
	pr_err("MELFAS " CHIP_NAME " Touchscreen initialization failed\n");
	return ret;
}

/**
 * Remove driver
 */
static int mms_remove(struct i2c_client *client)
{
	struct mms_ts_info *info = i2c_get_clientdata(client);

	if (info->irq >= 0) {
		free_irq(info->irq, info);
	}

#if MMS_USE_CMD_MODE
	mms_sysfs_cmd_remove(info);
#endif

#if MMS_USE_TEST_MODE
	mms_sysfs_remove(info);
#endif

	sysfs_remove_group(&info->client->dev.kobj, &mms_attr_group);
	sysfs_remove_link(NULL, MMS_DEVICE_NAME);

#if MMS_USE_DEV_MODE
	device_destroy(info->class, info->mms_dev);
	class_destroy(info->class);
#endif

	input_unregister_device(info->input_dev);

	kfree(info->fw_name);
	kfree(info);

	return 0;
}

#if MMS_USE_DEVICETREE
/**
 * Device tree match table
 */
static const struct of_device_id mms_match_table[] = {
	{ .compatible = "melfas,mms_ts",},
	{},
};
MODULE_DEVICE_TABLE(of, mms_match_table);
#endif

/**
 * I2C Device ID
 */
static const struct i2c_device_id mms_id[] = {
	{MMS_DEVICE_NAME, 0},
};
MODULE_DEVICE_TABLE(i2c, mms_id);

/**
 * I2C driver info
 */
static struct i2c_driver mms_driver = {
	.id_table	= mms_id,
	.probe = mms_probe,
	.remove = mms_remove,
	.driver = {
		.name = MMS_DEVICE_NAME,
		.owner = THIS_MODULE,
#if MMS_USE_DEVICETREE
		.of_match_table = mms_match_table,
#endif
	},
};

/**
 * Init driver
 */
static int __init mms_init(void)
{
	pr_err("%s\n", __func__);
#if defined(CONFIG_SAMSUNG_LPM_MODE)
	if (poweroff_charging) {
		pr_notice("%s : LPM Charging Mode!!\n", __func__);
		return 0;
	}
#endif
	return i2c_add_driver(&mms_driver);
}

/**
 * Exit driver
 */
static void __exit mms_exit(void)
{
	i2c_del_driver(&mms_driver);
}

module_init(mms_init);
module_exit(mms_exit);

MODULE_DESCRIPTION("MELFAS MMS400 Touchscreen");
MODULE_VERSION("2014.12.05");
MODULE_AUTHOR("Jee, SangWon <jeesw@melfas.com>");
MODULE_LICENSE("GPL");
