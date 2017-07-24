/* abov_touchkey.c -- Linux driver for abov chip as touchkey
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Author: Junkyeong Kim <jk0430.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/i2c/abov_touchkey.h>
#include <linux/io.h>
#include <mach/gpio.h>
#include <asm/unaligned.h>
#include <linux/regulator/consumer.h>

#include <linux/pinctrl/consumer.h>

#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif

/* registers */
#define ABOV_BTNSTATUS		0x00
#define ABOV_FW_VER			0x01
#define ABOV_PCB_VER		0x02
#define ABOV_COMMAND		0x03
#define ABOV_THRESHOLD		0x04
#define ABOV_SENS			0x05
#define ABOV_SETIDAC		0x06
#define ABOV_DIFFDATA		0x0A
#define ABOV_RAWDATA		0x0E
#define ABOV_VENDORID		0x12
#define ABOV_GLOVE			0x13
#define ABOV_DUAL_DETECT	0x16

/* command */
#define CMD_LED_ON			0x10
#define CMD_LED_OFF			0x20
#define CMD_DATA_UPDATE		0x40
#define CMD_LED_CTRL_ON		0x60
#define CMD_LED_CTRL_OFF	0x70
#define CMD_STOP_MODE		0x80
#define CMD_GLOVE_OFF		0x10
#define CMD_GLOVE_ON		0x20
#define CMD_DUAL_DETECT		0x10
#define CMD_SINGLE_DETECT	0x20

#define ABOV_BOOT_DELAY		45 //26
#define ABOV_RESET_DELAY	150

static struct device *sec_touchkey;

#define FW_VERSION 0xE
#define FW_CHECKSUM_H 0x95
#define FW_CHECKSUM_L 0x98

/* L OS support Screen Pinning concept
  * separate report of recent key and back key
  */
#define ABOV_DUAL_DETECTION_CMD_FW_VER	0x0C

#ifdef CONFIG_SEC_FACTORY
#define LED_TWINKLE_BOOTING
#endif

#ifdef LED_TWINKLE_BOOTING
static void led_twinkle_work(struct work_struct *work);
#endif

#define TK_FW_PATH_BIN "abov/abov_tk_a5.fw"
#define TK_FW_PATH_SDCARD "/sdcard/abov_fw.bin"

#define I2C_M_WR 0		/* for i2c */

enum {
	BUILT_IN = 0,
	SDCARD,
};

#define NOT_USE_THIS_ROUTINE	1

#ifdef CONFIG_SAMSUNG_LPM_MODE
extern int poweroff_charging;
#endif
extern int get_lcd_attached(char *);
extern unsigned int system_rev;
extern struct class *sec_class;
static int touchkey_keycode[] = { 0,
	KEY_RECENT, KEY_BACK,
};

struct abov_tk_info {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct abov_touchkey_platform_data *pdata;
	struct mutex lock;
	struct pinctrl *pinctrl;

	const struct firmware *firm_data_bin;
	const u8 *firm_data_ums;
	char phys[32];
	long firm_size;
	int irq;
	u16 menu_s;
	u16 back_s;
	u16 menu_raw;
	u16 back_raw;
	int (*power) (bool on);
	void (*input_event)(void *data);
	int touchkey_count;
	u8 fw_update_state;
	u8 fw_ver;
	u8 checksum_h;
	u8 checksum_l;
	bool enabled;
	bool glovemode;
#ifdef LED_TWINKLE_BOOTING
	struct delayed_work led_twinkle_work;
	bool led_twinkle_check;
#endif
	bool dual_mode;

};


static int abov_tk_input_open(struct input_dev *dev);
static void abov_tk_input_close(struct input_dev *dev);

static int abov_tk_i2c_read_checksum(struct abov_tk_info *info);
static void abov_tk_dual_detection_mode(struct abov_tk_info *info, int mode);

static int abov_touchkey_led_status;
static int abov_touchled_cmd_reserved;

static int abov_glove_mode_enable(struct i2c_client *client, u8 cmd)
{
	return i2c_smbus_write_byte_data(client, ABOV_GLOVE, cmd);
}

static int abov_tk_i2c_read(struct i2c_client *client,
		u8 reg, u8 *val, unsigned int len)
{
	struct abov_tk_info *info = i2c_get_clientdata(client);
	struct i2c_msg msg;
	int ret;
	int retry = 3;

	mutex_lock(&info->lock);
	msg.addr = client->addr;
	msg.flags = I2C_M_WR;
	msg.len = 1;
	msg.buf = &reg;
	while (retry--) {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret >= 0)
			break;

		dev_err(&client->dev, "%s fail(address set)(%d)\n",
			__func__, retry);
		msleep(10);
	}
	if (ret < 0) {
		mutex_unlock(&info->lock);
		return ret;
	}
	retry = 3;
	msg.flags = 1;/*I2C_M_RD*/
	msg.len = len;
	msg.buf = val;
	while (retry--) {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret >= 0) {
			mutex_unlock(&info->lock);
			return 0;
		}
		dev_err(&client->dev, "%s fail(data read)(%d)\n",
			__func__, retry);
		msleep(10);
	}
	mutex_unlock(&info->lock);
	return ret;
}

static int abov_tk_i2c_write(struct i2c_client *client,
		u8 reg, u8 *val, unsigned int len)
{
	struct abov_tk_info *info = i2c_get_clientdata(client);
	struct i2c_msg msg[1];
	unsigned char data[2];
	int ret;
	int retry = 3;

	mutex_lock(&info->lock);
	data[0] = reg;
	data[1] = *val;
	msg->addr = client->addr;
	msg->flags = I2C_M_WR;
	msg->len = 2;
	msg->buf = data;

	while (retry--) {
		ret = i2c_transfer(client->adapter, msg, 1);
		if (ret >= 0) {
			mutex_unlock(&info->lock);
			return 0;
		}
		dev_err(&client->dev, "%s fail(%d)\n",
			__func__, retry);
		msleep(10);
	}
	mutex_unlock(&info->lock);
	return ret;
}

static void release_all_fingers(struct abov_tk_info *info)
{
	struct i2c_client *client = info->client;
	int i;

	dev_dbg(&client->dev, "[TK] %s\n", __func__);

	for (i = 1; i < info->touchkey_count; i++) {
		input_report_key(info->input_dev,
			touchkey_keycode[i], 0);
	}
	input_sync(info->input_dev);
}

static int abov_tk_reset_for_bootmode(struct abov_tk_info *info)
{
	int ret=0;

	if(info->pdata->gpio_tkey_led_en >= 0)
		gpio_direction_output(info->pdata->gpio_tkey_led_en,0);

	msleep(50);

	if(info->pdata->gpio_tkey_led_en >= 0)
		gpio_direction_output(info->pdata->gpio_tkey_led_en,1);

	return ret;

}

static void abov_tk_reset(struct abov_tk_info *info)
{
	struct i2c_client *client = info->client;

	if (info->enabled == false)
		return;

	dev_notice(&client->dev, "%s++\n", __func__);
	disable_irq_nosync(info->irq);

	info->enabled = false;

	release_all_fingers(info);

	abov_tk_reset_for_bootmode(info);
	msleep(ABOV_RESET_DELAY);

	abov_tk_dual_detection_mode(info, 1);

	if (info->glovemode)
		abov_glove_mode_enable(client, CMD_GLOVE_ON);

	info->enabled = true;

	enable_irq(info->irq);
	dev_notice(&client->dev, "%s--\n", __func__);
}

static irqreturn_t abov_tk_interrupt(int irq, void *dev_id)
{
	struct abov_tk_info *info = dev_id;
	struct i2c_client *client = info->client;
	int ret, retry;
	u8 buf;
	bool press;

	ret = abov_tk_i2c_read(client, ABOV_BTNSTATUS, &buf, 1);
	if (ret < 0) {
		retry = 3;
		while (retry--) {
			dev_err(&client->dev, "%s read fail(%d)\n",
				__func__, retry);
			ret = abov_tk_i2c_read(client, ABOV_BTNSTATUS, &buf, 1);
			if (ret == 0)
				break;
			else
				msleep(10);
		}
		if (retry == 0) {
			abov_tk_reset(info);
			return IRQ_HANDLED;
		}
	}

	if (info->dual_mode) {
		int menu_data = buf & 0x03;
		int back_data = (buf >> 2) & 0x03;
		u8 menu_press = !(menu_data % 2);
		u8 back_press = !(back_data % 2);

		if (menu_data)
			input_report_key(info->input_dev,
				touchkey_keycode[1], menu_press);
		if (back_data)
			input_report_key(info->input_dev,
				touchkey_keycode[2], back_press);

#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
		dev_notice(&client->dev,
			"key %s%s ver0x%02x\n",
			menu_data ? (menu_press ? "P" : "R") : "",
			back_data ? (back_press ? "P" : "R") : "",
			info->fw_ver);
#else
		dev_notice(&client->dev,
			"%s%s%x ver0x%02x\n",
			menu_data ? (menu_press ? "menu P " : "menu R ") : "",
			back_data ? (back_press ? "back P " : "back R ") : "",
			buf, info->fw_ver);
#endif
	} else {
		u8 button = buf & 0x03;
		press = !!(buf & 0x8);

		if (press) {
			input_report_key(info->input_dev,
				touchkey_keycode[button], 0);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			dev_notice(&client->dev,
				"key R\n");
#else
			dev_notice(&client->dev,
				"key R : %d(%d) ver0x%02x\n",
				touchkey_keycode[button], buf, info->fw_ver);
#endif
		} else {
			input_report_key(info->input_dev,
				touchkey_keycode[button], 1);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			dev_notice(&client->dev,
				"key P\n");
#else
			dev_notice(&client->dev,
				"key P : %d(%d)\n",
				touchkey_keycode[button], buf);
#endif
		}
	}

	input_sync(info->input_dev);

	return IRQ_HANDLED;
}

static int touchkey_led_set(struct abov_tk_info *info, int data)
{
	u8 cmd;
	int ret;

	if (data == 1)
		cmd = CMD_LED_ON;
	else
		cmd = CMD_LED_OFF;

	if (!info->enabled) {
		abov_touchled_cmd_reserved = 1;
		return 1;
	}

	ret = abov_tk_i2c_write(info->client, ABOV_BTNSTATUS, &cmd, 1);
	if (ret < 0) {
		dev_err(&info->client->dev, "%s fail(%d)\n", __func__, ret);
		abov_touchled_cmd_reserved = 1;
		return 1;
	}

	return 0;
}

static ssize_t touchkey_led_control(struct device *dev,
		 struct device_attribute *attr, const char *buf,
		 size_t count)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	int data;
	u8 cmd;
	int ret;

	ret = sscanf(buf, "%d", &data);
	if (ret != 1) {
		dev_err(&info->client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(data == 0 || data == 1)) {
		dev_err(&info->client->dev, "%s: wrong command(%d)\n",
			__func__, data);
		return count;
	}

	if (data == 1)
		cmd = CMD_LED_ON;
	else
		cmd = CMD_LED_OFF;

#ifdef LED_TWINKLE_BOOTING
	if(info->led_twinkle_check == 1){
		info->led_twinkle_check = 0;
		cancel_delayed_work(&info->led_twinkle_work);
	}
#endif

	if(touchkey_led_set(info, data))
		goto out;

	msleep(20);

	abov_touchled_cmd_reserved = 0;
	dev_notice(&info->client->dev, "%s data(%d)\n",__func__,data);

out:
	abov_touchkey_led_status =  cmd;

	return count;
}

static ssize_t touchkey_threshold_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	u8 r_buf;
	int ret;

	ret = abov_tk_i2c_read(client, ABOV_THRESHOLD, &r_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);
		r_buf = 0;
	}
	return sprintf(buf, "%d\n", r_buf);
}

static void get_diff_data(struct abov_tk_info *info)
{
	struct i2c_client *client = info->client;
	u8 r_buf[4];
	int ret;

	ret = abov_tk_i2c_read(client, ABOV_DIFFDATA, r_buf, 4);
	if (ret < 0) {
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);
		info->menu_s = 0;
		info->back_s = 0;
		return;
	}

	info->menu_s = (r_buf[0] << 8) | r_buf[1];
	info->back_s = (r_buf[2] << 8) | r_buf[3];
}

static ssize_t touchkey_menu_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);

	get_diff_data(info);

	return sprintf(buf, "%d\n", info->menu_s);
}

static ssize_t touchkey_back_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);

	get_diff_data(info);

	return sprintf(buf, "%d\n", info->back_s);
}

static void get_raw_data(struct abov_tk_info *info)
{
	struct i2c_client *client = info->client;
	u8 r_buf[4];
	int ret;

	ret = abov_tk_i2c_read(client, ABOV_RAWDATA, r_buf, 4);
	if (ret < 0) {
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);
		info->menu_raw = 0;
		info->back_raw = 0;
		return;
	}

	info->menu_raw = (r_buf[0] << 8) | r_buf[1];
	info->back_raw = (r_buf[2] << 8) | r_buf[3];
}

static ssize_t touchkey_menu_raw_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);

	get_raw_data(info);

	return sprintf(buf, "%d\n", info->menu_raw);
}

static ssize_t touchkey_back_raw_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);

	get_raw_data(info);

	return sprintf(buf, "%d\n", info->back_raw);
}

static ssize_t bin_fw_ver(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;

	dev_dbg(&client->dev, "fw version bin : 0x%x\n", FW_VERSION);

	return sprintf(buf, "0x%02x\n", FW_VERSION);
}

int get_tk_fw_version(struct abov_tk_info *info, bool bootmode)
{
	struct i2c_client *client = info->client;
	u8 buf;
	int ret;
	int retry = 3;

	ret = abov_tk_i2c_read(client, ABOV_FW_VER, &buf, 1);
	if (ret < 0) {
		while (retry--) {
			dev_err(&client->dev, "%s read fail(%d)\n",
				__func__, retry);
			if (!bootmode)
				abov_tk_reset(info);
			else
				return -1;
			ret = abov_tk_i2c_read(client, ABOV_FW_VER, &buf, 1);
			if (ret == 0)
				break;
		}
		if (retry == 0)
			return -1;
	}

	info->fw_ver = buf;
	dev_notice(&client->dev, "%s : 0x%x\n", __func__, buf);
	return 0;
}

static ssize_t read_fw_ver(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	int ret;

	ret = get_tk_fw_version(info, false);
	if (ret < 0) {
		dev_err(&client->dev, "%s read fail\n", __func__);
		info->fw_ver = 0;
	}

	abov_tk_i2c_read_checksum(info);

	return sprintf(buf, "0x%02x\n", info->fw_ver);
}

static int abov_load_fw(struct abov_tk_info *info, u8 cmd)
{
	struct i2c_client *client = info->client;
	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int ret = 0;

	switch(cmd) {
	case BUILT_IN:
		ret = request_firmware(&info->firm_data_bin,
			TK_FW_PATH_BIN, &client->dev);
		if (ret) {
			dev_err(&client->dev,
				"%s request_firmware fail(%d)\n", __func__, cmd);
			return ret;
		}
		info->firm_size = info->firm_data_bin->size;
		break;

	case SDCARD:
		old_fs = get_fs();
		set_fs(get_ds());
		fp = filp_open(TK_FW_PATH_SDCARD, O_RDONLY, S_IRUSR);
		if (IS_ERR(fp)) {
			dev_err(&client->dev,
				"%s %s open error\n", __func__, TK_FW_PATH_SDCARD);
			ret = -ENOENT;
			goto fail_sdcard_open;
		}

		fsize = fp->f_path.dentry->d_inode->i_size;
		info->firm_data_ums = kzalloc((size_t)fsize, GFP_KERNEL);
		if (!info->firm_data_ums) {
			dev_err(&client->dev,
				"%s fail to kzalloc for fw\n", __func__);
			ret = -ENOMEM;
			goto fail_sdcard_kzalloc;
		}

		nread = vfs_read(fp,
			(char __user *)info->firm_data_ums, fsize, &fp->f_pos);
		if (nread != fsize) {
			dev_err(&client->dev,
				"%s fail to vfs_read file\n", __func__);
			ret = -EINVAL;
			goto fail_sdcard_size;
		}
		filp_close(fp, current->files);
		set_fs(old_fs);
		info->firm_size = nread;
		break;

	default:
		ret = -1;
		break;
	}
	dev_notice(&client->dev, "fw_size : %lu\n", info->firm_size);
	dev_notice(&client->dev, "%s success\n", __func__);
	return ret;

fail_sdcard_size:
	kfree(&info->firm_data_ums);
fail_sdcard_kzalloc:
	filp_close(fp, current->files);
fail_sdcard_open:
	set_fs(old_fs);
	return ret;
}

#if NOT_USE_THIS_ROUTINE
void abov_i2c_start(int scl, int sda)
{
	gpio_direction_output(sda, 1);
	gpio_direction_output(scl, 1);
	usleep_range(15, 17);
	gpio_direction_output(sda, 0);
	usleep_range(10, 12);
	gpio_direction_output(scl, 0);
	usleep_range(10, 12);
}

void abov_i2c_stop(int scl, int sda)
{
	gpio_direction_output(scl, 0);
	usleep_range(10, 12);
	gpio_direction_output(sda, 0);
	usleep_range(10, 12);
	gpio_direction_output(scl, 1);
	usleep_range(10, 12);
	gpio_direction_output(sda, 1);
}

void abov_testdelay(void)
{
	u8 i;
	u8 delay;

	/* 120nms */
	for (i = 0; i < 15; i++)
		delay = 0;
}


void abov_byte_send(u8 data, int scl, int sda)
{
	u8 i;

	for (i = 0x80; i != 0; i >>= 1) {
		gpio_direction_output(scl, 0);
		usleep(1);

		if (data & i)
			gpio_direction_output(sda, 1);
		else
			gpio_direction_output(sda, 0);

		usleep(1);
		gpio_direction_output(scl, 1);
		usleep(1);
	}
	usleep(1);

	gpio_direction_output(scl, 0);
	gpio_direction_input(sda);
//	gpio_tlmm_config(GPIO_CFG(sda,0,GPIO_CFG_INPUT,GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	usleep(1);

	gpio_direction_output(scl, 1);
	usleep(1);

	gpio_get_value(sda);
	abov_testdelay();

	gpio_direction_output(scl, 0);
	gpio_direction_output(sda, 0);
	usleep(20);
}

u8 abov_byte_read(bool type, int scl, int sda)
{
	u8 i;
	u8 data = 0;
	u8 index = 0x7;

	gpio_direction_output(scl, 0);
	gpio_direction_input(sda);
//	gpio_tlmm_config(GPIO_CFG(sda,0,GPIO_CFG_INPUT,GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	usleep(1);

	for (i = 0; i < 8; i++) {
		gpio_direction_output(scl, 0);
		usleep(1);
		gpio_direction_output(scl, 1);
		usleep(1);

		data = data | (u8)(gpio_get_value(sda) << index);
		index -= 1;
	}
		usleep(1);
	gpio_direction_output(scl, 0);

	gpio_direction_output(sda, 0);
		usleep(1);

	if (type) { /*ACK */
		gpio_direction_output(sda, 0);
		usleep(1);
		gpio_direction_output(scl, 1);
		usleep(1);
		gpio_direction_output(scl, 0);
		usleep(1);
	} else { /* NAK */
		gpio_direction_output(sda, 1);
		usleep(1);
		gpio_direction_output(scl, 1);
		usleep(1);
		gpio_direction_output(scl, 0);
		usleep(1);
		gpio_direction_output(sda, 0);
		usleep(1);
	}
	usleep(20);

	return data;
}

void abov_enter_mode(int scl, int sda)
{
	abov_i2c_start(scl, sda);
	abov_testdelay();
	abov_byte_send(ABOV_ID, scl, sda);
	abov_byte_send(0xAC, scl, sda);
	abov_byte_send(0x5B, scl, sda);
	abov_byte_send(0x2D, scl, sda);
	abov_i2c_stop(scl, sda);
}

void abov_firm_write(const u8 *fw_data, int block, int scl, int sda)
{
	int i, j;
	u16 pos = 0;
	u8 addr[2];
	u8 data[32] = {0, };

	addr[0] = 0x10;
	addr[1] = 0x00;
	for (i = 0; i < block; i++) {
		if (i % 8 == 0) {
			addr[0] = 0x10 + i/8;
			addr[1] = 0;
		} else
			addr[1] = addr[1] + 0x20;
		memcpy(data, fw_data + pos, 32);
		abov_i2c_start(scl, sda);
		abov_testdelay();
		abov_byte_send(ABOV_ID, scl, sda);
		abov_byte_send(0xAC, scl, sda);
		abov_byte_send(0x7A, scl, sda);
		abov_byte_send(addr[0], scl, sda);
		abov_byte_send(addr[1], scl, sda);
		for (j = 0; j < 32; j++)
			abov_byte_send(data[j], scl, sda);
		abov_i2c_stop(scl, sda);

		pos += 0x20;

		usleep(3000); //usleep(2000); //msleep(2);
	}
}

void abov_read_address_set(int scl, int sda)
{
		abov_i2c_start(scl, sda);
		abov_testdelay();
		abov_byte_send(ABOV_ID, scl, sda);
		abov_byte_send(0xAC, scl, sda);
		abov_byte_send(0x9E, scl, sda);
		abov_byte_send(0x10, scl, sda); /* start addr H */
		abov_byte_send(0x00, scl, sda); /* start addr L */
		abov_byte_send(0x3F, scl, sda); /* end addr H  */
		abov_byte_send(0xFF, scl, sda); /* end addr L  */
		abov_i2c_stop(scl, sda);
}

void abov_checksum(struct abov_tk_info *info, int scl, int sda)
{
	struct i2c_client *client = info->client;

	u8 status;
	u8 bootver;
	u8 firmver;
	u8 checksumh;
	u8 checksuml;

	abov_read_address_set(scl, sda);
	msleep(5);

	abov_i2c_start(scl, sda);
	abov_testdelay();
	abov_byte_send(ABOV_ID, scl, sda);
	abov_byte_send(0x00, scl, sda);

	abov_i2c_start(scl, sda); /* restart */
	abov_testdelay();
	abov_byte_send(ABOV_ID + 1, scl, sda);
	status = abov_byte_read(true, scl, sda);
	bootver = abov_byte_read(true, scl, sda);
	firmver = abov_byte_read(true, scl, sda);
	checksumh = abov_byte_read(true, scl, sda);
	checksuml = abov_byte_read(false, scl, sda);
	abov_i2c_stop(scl, sda);
	msleep(3);

	info->checksum_h = checksumh;
	info->checksum_l = checksuml;

	dev_err(&client->dev,
		"%s status(0x%x), boot(0x%x), firm(0x%x), cs_h(0x%x), cs_l(0x%x)\n",
		__func__, status, bootver, firmver, checksumh, checksuml);
}

void abov_exit_mode(int scl, int sda)
{
	abov_i2c_start(scl, sda);
	abov_testdelay();
	abov_byte_send(ABOV_ID, scl, sda);
	abov_byte_send(0xAC, scl, sda);
	abov_byte_send(0x5B, scl, sda);
	abov_byte_send(0xE1, scl, sda);
	abov_i2c_stop(scl, sda);
}

static int abov_fw_update(struct abov_tk_info *info,
				const u8 *fw_data, int block, int scl, int sda)
{

	usleep(ABOV_BOOT_DELAY * 1000);
	abov_enter_mode(scl, sda);
	msleep(1100); //msleep(600);
	abov_firm_write(fw_data, block, scl, sda);
	abov_checksum(info, scl, sda);
	return 0;
}
#endif

static int abov_tk_check_busy(struct abov_tk_info *info)
{
	int ret, count = 0;
	unsigned char val = 0x00;

	do {
		ret = i2c_master_recv(info->client, &val, sizeof(val));

		if (val) {
			count++;
		} else {
			break;
		}

	} while(1);

	if (count > 1000)
		pr_err("%s: busy %d\n", __func__, count);
	return ret;
}

static int abov_tk_i2c_read_checksum(struct abov_tk_info *info)
{
	unsigned char data[6] = {0xAC, 0x9E, 0x10, 0x00, 0x3F, 0xFF};
	unsigned char checksum[5] = {0, };
	int ret;
	unsigned char reg = 0x00;

	i2c_master_send(info->client, data, 6);

	usleep(5 * 1000);

	abov_tk_check_busy(info);

	ret = abov_tk_i2c_read(info->client, reg, checksum, 5);

	dev_info(&info->client->dev, "%s: ret:%d [%X][%X][%X][%X][%X]\n",
			__func__, ret, checksum[0], checksum[1], checksum[2]
			, checksum[3], checksum[4]);
	info->checksum_h = checksum[3];
	info->checksum_l = checksum[4];
	return 0;
}

static void abov_release_fw(struct abov_tk_info *info, u8 cmd)
{
	switch(cmd) {
	case BUILT_IN:
		release_firmware(info->firm_data_bin);
		break;

	case SDCARD:
		kfree(info->firm_data_ums);
		break;

	default:
		break;
	}
}

static int abov_flash_fw(struct abov_tk_info *info, bool probe, u8 cmd)
{
	struct i2c_client *client = info->client;
	int retry = 2;
	int ret;
	int block_count;
	const u8 *fw_data;

	ret = get_tk_fw_version(info, probe);
	if (ret)
		info->fw_ver = 0;

	ret = abov_load_fw(info, cmd);
	if (ret) {
		dev_err(&client->dev,
			"%s fw load fail\n", __func__);
		return ret;
	}

	switch(cmd) {
	case BUILT_IN:
		fw_data = info->firm_data_bin->data;
		break;

	case SDCARD:
		fw_data = info->firm_data_ums;
		break;

	default:
		return -1;
		break;
	}

	block_count = (int)(info->firm_size / 32);

	while (retry--) {
#if NOT_USE_THIS_ROUTINE
		abov_tk_reset_for_bootmode(info);
		abov_fw_update(info, fw_data, block_count,
			info->pdata->gpio_scl, info->pdata->gpio_sda);
#endif

		if (cmd == BUILT_IN) {
			if ((info->checksum_h != FW_CHECKSUM_H) ||
				(info->checksum_l != FW_CHECKSUM_L)) {
				dev_err(&client->dev,
					"%s checksum fail.(0x%x,0x%x),(0x%x,0x%x) retry:%d\n",
					__func__, info->checksum_h, info->checksum_l,
					FW_CHECKSUM_H, FW_CHECKSUM_L, retry);
				ret = -1;
				continue;
			}
		}
		abov_tk_reset_for_bootmode(info);
		msleep(ABOV_RESET_DELAY);
		ret = get_tk_fw_version(info, true);
		if (ret) {
			dev_err(&client->dev, "%s fw version read fail\n", __func__);
			ret = -1;
			continue;
		}

		if (info->fw_ver == 0) {
			dev_err(&client->dev, "%s fw version fail (0x%x)\n",
				__func__, info->fw_ver);
			ret = -1;
			continue;
		}

		if ((cmd == BUILT_IN) && (info->fw_ver != FW_VERSION)) {
			dev_err(&client->dev, "%s fw version fail 0x%x, 0x%x\n",
				__func__, info->fw_ver, FW_VERSION);
			ret = -1;
			continue;
		}
		ret = 0;
		break;
	}

	abov_release_fw(info, cmd);

	return ret;
}

static ssize_t touchkey_fw_update(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	int ret;
	u8 cmd;

	switch(*buf) {
	case 's':
	case 'S':
		cmd = BUILT_IN;
		break;
	case 'i':
	case 'I':
		cmd = SDCARD;
		break;
	default:
		info->fw_update_state = 2;
		goto touchkey_fw_update_out;
	}

	info->fw_update_state = 1;
	disable_irq(info->irq);
	info->enabled = false;
	ret = abov_flash_fw(info, false, cmd);

	abov_tk_dual_detection_mode(info, 1);
	if (info->glovemode)
		abov_glove_mode_enable(client, CMD_GLOVE_ON);
	info->enabled = true;
	enable_irq(info->irq);
	if (ret) {
		dev_err(&client->dev, "%s fail\n", __func__);
//		info->fw_update_state = 2;
		info->fw_update_state = 0;

	} else {
		dev_notice(&client->dev, "%s success\n", __func__);
		info->fw_update_state = 0;
	}

touchkey_fw_update_out:
	dev_dbg(&client->dev, "%s : %d\n", __func__, info->fw_update_state);

	return count;
}

static ssize_t touchkey_fw_update_status(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	int count = 0;

	dev_info(&client->dev, "%s : %d\n", __func__, info->fw_update_state);

	if (info->fw_update_state == 0)
		count = sprintf(buf, "PASS\n");
	else if (info->fw_update_state == 1)
		count = sprintf(buf, "Downloading\n");
	else if (info->fw_update_state == 2)
		count = sprintf(buf, "Fail\n");

	return count;
}

static ssize_t abov_glove_mode(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	struct i2c_client *client = info->client;
	int scan_buffer;
	int ret;
	u8 cmd;

	ret = sscanf(buf, "%d", &scan_buffer);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(scan_buffer == 0 || scan_buffer == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (!info->enabled)
		return count;

	if (info->glovemode == scan_buffer) {
		dev_info(&client->dev, "%s same command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (scan_buffer == 1) {
		dev_notice(&client->dev, "%s glove mode\n", __func__);
		cmd = CMD_GLOVE_ON;
	} else {
		dev_notice(&client->dev, "%s normal mode\n", __func__);
		cmd = CMD_GLOVE_OFF;
	}

	ret = abov_glove_mode_enable(client, cmd);
	if (ret < 0) {
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);
		return count;
	}

	info->glovemode = scan_buffer;

	return count;
}

static ssize_t abov_glove_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", info->glovemode);
}

static void abov_tk_dual_detection_mode(struct abov_tk_info *info, int mode)
{
	u8 cmd;
	int ret;

	if (info->fw_ver < ABOV_DUAL_DETECTION_CMD_FW_VER){
		info->dual_mode = false;
		return;
	}
	dev_info(&info->client->dev,
			"%s: %s\n", __func__, mode ? "on" : "off");

	if (mode)
		cmd = CMD_DUAL_DETECT;
	else
		cmd = CMD_SINGLE_DETECT;

	ret = abov_tk_i2c_write(info->client, ABOV_DUAL_DETECT, &cmd, 1);
	if (ret < 0)
		dev_err(&info->client->dev,
			"%s %d : fail %d\n", __func__, __LINE__, ret);

	info->dual_mode = !!mode;
}

static ssize_t abov_set_dual_detection_mode(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct abov_tk_info *info = dev_get_drvdata(dev);
	int scan_buffer;
	int ret;

	ret = sscanf(buf, "%d", &scan_buffer);
	if (ret != 1) {
		dev_err(&info->client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!info->enabled)
		return count;

	abov_tk_dual_detection_mode(info, !!scan_buffer);

	return count;
}


static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold_show, NULL);
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
			touchkey_led_control);
static DEVICE_ATTR(touchkey_recent, S_IRUGO, touchkey_menu_show, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO, touchkey_back_show, NULL);
static DEVICE_ATTR(touchkey_raw_recent, S_IRUGO, touchkey_menu_raw_show, NULL);
static DEVICE_ATTR(touchkey_raw_back, S_IRUGO, touchkey_back_raw_show, NULL);
static DEVICE_ATTR(touchkey_firm_version_phone, S_IRUGO, bin_fw_ver, NULL);
static DEVICE_ATTR(touchkey_firm_version_panel, S_IRUGO, read_fw_ver, NULL);
static DEVICE_ATTR(touchkey_firm_update, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
			touchkey_fw_update);
static DEVICE_ATTR(touchkey_firm_update_status, S_IRUGO | S_IWUSR | S_IWGRP,
			touchkey_fw_update_status, NULL);
static DEVICE_ATTR(glove_mode, S_IRUGO | S_IWUSR | S_IWGRP,
			abov_glove_mode_show, abov_glove_mode);
static DEVICE_ATTR(detection_mode, S_IRUGO | S_IWUSR | S_IWGRP,
			NULL, abov_set_dual_detection_mode);

static struct attribute *sec_touchkey_attributes[] = {
	&dev_attr_touchkey_threshold.attr,
	&dev_attr_brightness.attr,
	&dev_attr_touchkey_recent.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_raw_back.attr,
	&dev_attr_touchkey_raw_recent.attr,
	&dev_attr_touchkey_firm_version_phone.attr,
	&dev_attr_touchkey_firm_version_panel.attr,
	&dev_attr_touchkey_firm_update.attr,
	&dev_attr_touchkey_firm_update_status.attr,
	&dev_attr_glove_mode.attr,
	&dev_attr_detection_mode.attr,
	NULL,
};

static struct attribute_group sec_touchkey_attr_group = {
	.attrs = sec_touchkey_attributes,
};

static int abov_tk_fw_check(struct abov_tk_info *info)
{
	struct i2c_client *client = info->client;
	int ret;

	ret = get_tk_fw_version(info, true);

#ifdef LED_TWINKLE_BOOTING
	if(ret)
		dev_err(&client->dev,
			"%s: i2c fail...[%d], addr[%d]\n",
			__func__, ret, info->client->addr);
		dev_err(&client->dev,
			"%s: touchkey driver unload\n", __func__);
		
		if (get_lcd_attached("GET") == 0) {
			dev_err(&client->dev,
				"%s : lcd is not attached\n", __func__);
				return ret;
		}
#endif

	if ((info->fw_ver == 0) || info->fw_ver < FW_VERSION  || info->fw_ver > 0xF0) {
		dev_err(&client->dev, "excute tk firmware update (0x%x -> 0x%x\n",
			info->fw_ver, FW_VERSION);
		ret = abov_flash_fw(info, true, BUILT_IN);
		if (ret) {
			dev_err(&client->dev,
				"failed to abov_flash_fw (%d)\n", ret);
		} else {
			dev_info(&client->dev,
				"fw update success\n");
		}
	}

	return ret;
}

int abov_power(struct abov_touchkey_platform_data *pdata, bool on)
{
	int ret = 0;

	if (on) {
		if (pdata->vdd_io_vreg) {
			ret = regulator_enable(pdata->vdd_io_vreg);
			if(ret){
				pr_err("[TKEY] %s: iovdd reg enable fail\n", __func__);
				return ret;
			}
		}

		if (pdata->avdd_vreg) {
			ret = regulator_enable(pdata->avdd_vreg);
			if(ret){
				pr_err("[TKEY] %s: avdd reg enable fail\n", __func__);
				return ret;
			}
		}
	} else {
		if (pdata->vdd_io_vreg) {
			ret = regulator_disable(pdata->vdd_io_vreg);
			if(ret){
				pr_err("[TKEY] %s: iovdd reg disable fail\n", __func__);
				return ret;
			}
		}

		if (pdata->avdd_vreg) {
			ret = regulator_disable(pdata->avdd_vreg);
			if(ret){
				pr_err("[TKEY] %s: avdd reg disable fail\n", __func__);
				return ret;
			}
		}
	}

	if (pdata->gpio_tkey_led_en >= 0) {
		gpio_direction_output(pdata->gpio_tkey_led_en, on);
		pr_info("[TKEY] %s: %s: %d\n", __func__, on ? "on" : "off",
			gpio_get_value(pdata->gpio_tkey_led_en));
	}

	return ret;
}

static int abov_pinctrl_configure(struct abov_tk_info *info,
							bool active)
{
	struct pinctrl_state *set_state;
	int retval;

	if (active) {
		set_state =
			pinctrl_lookup_state(info->pinctrl,
						"touchkey_int_gpio_active");
		if (IS_ERR(set_state)) {
			pr_err("%s: cannot get ts pinctrl active state\n", __func__);
			return PTR_ERR(set_state);
		}
	} else {
		set_state =
			pinctrl_lookup_state(info->pinctrl,
						"touchkey_int_gpio_suspend");
		if (IS_ERR(set_state)) {
			pr_err("%s: cannot get gpiokey pinctrl sleep state\n", __func__);
			return PTR_ERR(set_state);
		}
	}
	retval = pinctrl_select_state(info->pinctrl, set_state);
	if (retval) {
		pr_err("%s: cannot set ts pinctrl active state\n", __func__);
		return retval;
	}

	return 0;
}

int abov_gpio_reg_init(struct device *dev,
			struct abov_touchkey_platform_data *pdata)
{
	int ret = 0;

	if (pdata->gpio_rst > 0) {
		ret = gpio_request(pdata->gpio_rst, "tkey_gpio_rst");
		if(ret < 0){
			dev_err(dev, "unable to request gpio_rst\n");
			return ret;
		}
	}
	ret = gpio_request(pdata->gpio_int, "tkey_gpio_int");
	if(ret < 0){
		dev_err(dev, "unable to request gpio_int\n");
		return ret;
	}

	if (pdata->gpio_seperated) {
	        if (pdata->gpio_en > 0) {
		      ret = gpio_request(pdata->gpio_en, "tkey_gpio_en");
		      if(ret < 0){
			 dev_err(dev, "unable to request gpio_en\n");
			 return ret;
		   }
                }
	}

	ret = gpio_request(pdata->gpio_tkey_led_en, "gpio_tkey_led_en");
	if(ret < 0){
		dev_err(dev, "unable to request gpio_tkey_led_en..Tkey led will not work...ignoring\n");
		ret = 0;
	}

	pdata->vdd_io_vreg = regulator_get(dev, "vddo");
	if (IS_ERR(pdata->vdd_io_vreg)) {
		pdata->vdd_io_vreg = NULL;
		dev_err(dev, "pdata->vdd_io_vreg get error, ignoring\n");
	} else {
		regulator_set_voltage(pdata->vdd_io_vreg, 1800000, 1800000);
	}

	pdata->avdd_vreg = regulator_get(dev, "avdd");
	if (IS_ERR(pdata->avdd_vreg)) {
		pdata->avdd_vreg = NULL;
		dev_err(dev, "pdata->avdd_vreg get error, ignoring\n");
	} else {
		regulator_set_voltage(pdata->avdd_vreg, 2850000, 2850000);
	}

	pdata->power = abov_power;

	return ret;
}

#ifdef CONFIG_OF
static int abov_parse_dt(struct device *dev,
			struct abov_touchkey_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->gpio_rst = of_get_named_gpio(np, "abov,rst-gpio", 0);
	if(pdata->gpio_rst < 0){
		dev_err(dev, "unable to get gpio_rst\n");
	}

	pdata->gpio_en = of_get_named_gpio(np, "abov,tkey_en-gpio", 0);
	if(pdata->gpio_int < 0){
		dev_err(dev, "unable to get gpio_en\n");
	}

	of_property_read_u32(np, "abov,gpio_seperated", &pdata->gpio_seperated);

	pdata->gpio_int = of_get_named_gpio(np, "abov,irq-gpio", 0);
	if(pdata->gpio_int < 0){
		dev_err(dev, "unable to get gpio_int\n");
		return pdata->gpio_int;
	}

	pdata->gpio_scl = of_get_named_gpio(np, "abov,scl-gpio", 0);
	if(pdata->gpio_scl < 0){
		dev_err(dev, "unable to get gpio_scl\n");
		return pdata->gpio_scl;
	}

	pdata->gpio_sda = of_get_named_gpio(np, "abov,sda-gpio", 0);
	if(pdata->gpio_sda < 0){
		dev_err(dev, "unable to get gpio_sda\n");
		return pdata->gpio_sda;
	}

	pdata->gpio_tkey_led_en = of_get_named_gpio(np, "abov,tkey_led_en-gpio", 0);
	if(pdata->gpio_tkey_led_en < 0){
		dev_err(dev, "unable to get gpio_tkey_led_en...ignoring\n");
	}

	dev_info(dev, "%s: gpio_en:%d, gpio_int:%d, gpio_scl:%d, gpio_sda:%d, gpio_led_en:%d, gpio_seperated:%d\n",
			__func__, pdata->gpio_en, pdata->gpio_int, pdata->gpio_scl,
			pdata->gpio_sda, pdata->gpio_tkey_led_en, pdata->gpio_seperated);

	return 0;
}
#else
static int abov_parse_dt(struct device *dev,
			struct abov_touchkey_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int abov_tk_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct abov_tk_info *info;
	struct input_dev *input_dev;
	int ret = 0;

#ifndef LED_TWINKLE_BOOTING
	if (get_lcd_attached("GET") == 0) {
		dev_err(&client->dev,
			"%s : lcd is not attached\n", __func__);
		return -EIO;
	}
#endif
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
			"i2c_check_functionality fail\n");
		return -EIO;
	}

	info = kzalloc(sizeof(struct abov_tk_info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev,
			"Failed to allocate memory for input device\n");
		ret = -ENOMEM;
		goto err_input_alloc;
	}

	info->client = client;
	info->input_dev = input_dev;

	if (client->dev.of_node) {
		struct abov_touchkey_platform_data *pdata;
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct abov_touchkey_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_config;
		}

		ret = abov_parse_dt(&client->dev, pdata);
		if (ret)
			return ret;

		info->pdata = pdata;
	} else
		info->pdata = client->dev.platform_data;

	if (info->pdata == NULL) {
		pr_err("failed to get platform data\n");
		goto err_config;
	}

	/* Get pinctrl if target uses pinctrl */
		info->pinctrl = devm_pinctrl_get(&client->dev);
		if (IS_ERR(info->pinctrl)) {
			if (PTR_ERR(info->pinctrl) == -EPROBE_DEFER)
				goto err_config;

			pr_err("%s: Target does not use pinctrl\n", __func__);
			info->pinctrl = NULL;
		}

		if (info->pinctrl) {
			ret = abov_pinctrl_configure(info, true);
			if (ret)
				pr_err("%s: cannot set ts pinctrl active state\n", __func__);
		}

	ret = abov_gpio_reg_init(&client->dev, info->pdata);
	if(ret){
		dev_err(&client->dev, "failed to init reg\n");
		goto pwr_config;
	}
	if (info->pdata->power)
		info->pdata->power(info->pdata, true);
	msleep(ABOV_RESET_DELAY);

	info->enabled = true;
	info->irq = -1;
	mutex_init(&info->lock);

	info->input_event = info->pdata->input_event;
	info->touchkey_count = sizeof(touchkey_keycode) / sizeof(int);
	i2c_set_clientdata(client, info);
	ret = abov_tk_fw_check(info);
	if (ret) {
		dev_err(&client->dev,
			"failed to firmware check (%d)\n", ret);
		goto err_reg_input_dev;
	}
	abov_tk_dual_detection_mode(info, 1);

	snprintf(info->phys, sizeof(info->phys),
		 "%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchkey";
	input_dev->phys = info->phys;
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &client->dev;
	input_dev->open = abov_tk_input_open;
	input_dev->close = abov_tk_input_close;

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(KEY_RECENT, input_dev->keybit);
	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);
	input_set_drvdata(input_dev, info);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "failed to register input dev (%d)\n",
			ret);
		goto err_reg_input_dev;
	}

	if (!info->pdata->irq_flag) {
		dev_err(&client->dev, "no irq_flag\n");
		ret = request_threaded_irq(client->irq, NULL, abov_tk_interrupt,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, ABOV_TK_NAME, info);
	} else {
		ret = request_threaded_irq(client->irq, NULL, abov_tk_interrupt,
			info->pdata->irq_flag, ABOV_TK_NAME, info);
	}
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}
	info->irq = client->irq;

	sec_touchkey = device_create(sec_class,
		NULL, 0, info, "sec_touchkey");
	if (IS_ERR(sec_touchkey))
		dev_err(&client->dev,
		"Failed to create device for the touchkey sysfs\n");

	ret = sysfs_create_group(&sec_touchkey->kobj,
		&sec_touchkey_attr_group);
	if (ret)
		dev_err(&client->dev, "Failed to create sysfs group\n");

	ret = sysfs_create_link(&sec_touchkey->kobj,
		&info->input_dev->dev.kobj, "input");
	if (ret < 0) {
		dev_err(&info->client->dev,
			"%s: Failed to create input symbolic link\n",
			__func__);
	}


#ifdef LED_TWINKLE_BOOTING
	if (get_lcd_attached("GET") == 0) {
		dev_err(&client->dev,
			"%s : LCD is not connected. so start LED twinkle \n", __func__);
		INIT_DELAYED_WORK(&info->led_twinkle_work, led_twinkle_work);
		info->led_twinkle_check =  1;
		schedule_delayed_work(&info->led_twinkle_work, msecs_to_jiffies(400));
	}
#endif


	dev_dbg(&client->dev, "%s done\n", __func__);
	return 0;

err_req_irq:
	input_unregister_device(input_dev);
err_reg_input_dev:
	mutex_destroy(&info->lock);
pwr_config:
err_config:
	input_free_device(input_dev);
err_input_alloc:
	kfree(info);
err_alloc:
	return ret;

}


#ifdef LED_TWINKLE_BOOTING
static void led_twinkle_work(struct work_struct *work)
{
	struct abov_tk_info *info = container_of(work, struct abov_tk_info,
						led_twinkle_work.work);
	static bool led_on = 1;
	static int count = 0;
	dev_err(&info->client->dev, "%s, on=%d, c=%d\n",__func__, led_on, count++ );

	if(info->led_twinkle_check == 1){

		touchkey_led_set(info,led_on);
		if(led_on)	led_on = 0;
		else		led_on = 1;

		schedule_delayed_work(&info->led_twinkle_work, msecs_to_jiffies(400));
	}else{

		if(led_on == 0)
			touchkey_led_set(info, 0);
	}

}
#endif

static int abov_tk_remove(struct i2c_client *client)
{
	struct abov_tk_info *info = i2c_get_clientdata(client);

/*	if (info->enabled)
		info->pdata->power(0);
*/
	info->enabled = false;
	if (info->irq >= 0)
		free_irq(info->irq, info);
	input_unregister_device(info->input_dev);
	input_free_device(info->input_dev);
	kfree(info);

	return 0;
}

static void abov_tk_shutdown(struct i2c_client *client)
{
	struct abov_tk_info *info = i2c_get_clientdata(client);
	u8 cmd = CMD_LED_OFF;
	printk("Inside abov_tk_shutdown \n");

	if (info->enabled){
		disable_irq(info->irq);
	abov_tk_i2c_write(client, ABOV_BTNSTATUS, &cmd, 1);
		info->pdata->power(info->pdata, false);
	}
	info->enabled = false;
}

static int abov_tk_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct abov_tk_info *info = i2c_get_clientdata(client);

	if (!info->enabled) {
		dev_info(&client->dev, "%s: already power off\n", __func__);
		return 0;
	}

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);

	disable_irq(info->irq);
	info->enabled = false;
	release_all_fingers(info);

	if (info->pdata->power)
		info->pdata->power(info->pdata, false);
	return 0;
}

static int abov_tk_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct abov_tk_info *info = i2c_get_clientdata(client);
	u8 led_data;

	if (info->enabled) {
		dev_info(&client->dev, "%s: already power on\n", __func__);
		return 0;
	}

	dev_notice(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);

	if (info->pdata->power) {
		info->pdata->power(info->pdata, true);
		msleep(ABOV_RESET_DELAY);
	} else /* touchkey on by i2c */
		get_tk_fw_version(info, true);

	abov_tk_dual_detection_mode(info, 1);

	info->enabled = true;

	if (abov_touchled_cmd_reserved && \
		abov_touchkey_led_status == CMD_LED_ON) {
		abov_touchled_cmd_reserved = 0;
		led_data=abov_touchkey_led_status;

		abov_tk_i2c_write(client, ABOV_BTNSTATUS, &led_data, 1);

		dev_notice(&info->client->dev, "%s: LED reserved on\n", __func__);
	}
	enable_irq(info->irq);

	return 0;
}

static int abov_tk_input_open(struct input_dev *dev)
{
	struct abov_tk_info *info = input_get_drvdata(dev);

	dev_info(&info->client->dev, "%s: users=%d, v:0x%02x, g:%d\n", __func__,
		   info->input_dev->users, info->fw_ver ,info->glovemode);

	gpio_direction_input(info->pdata->gpio_scl);
	gpio_direction_input(info->pdata->gpio_sda);

	abov_tk_resume(&info->client->dev);
	if (info->pinctrl)
		abov_pinctrl_configure(info, true);

	if (info->glovemode)
		abov_glove_mode_enable(info->client, CMD_GLOVE_ON);

	return 0;
}
static void abov_tk_input_close(struct input_dev *dev)
{
	struct abov_tk_info *info = input_get_drvdata(dev);
	dev_info(&info->client->dev, "%s: users=%d\n", __func__,
		   info->input_dev->users);

	abov_tk_suspend(&info->client->dev);
	gpio_set_value(info->pdata->gpio_scl, 1);
	gpio_set_value(info->pdata->gpio_sda, 1);
	if (info->pinctrl)
		abov_pinctrl_configure(info, false);

#ifdef LED_TWINKLE_BOOTING
	info->led_twinkle_check = 0;
#endif

}

static const struct i2c_device_id abov_tk_id[] = {
	{ABOV_TK_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, abov_tk_id);

#ifdef CONFIG_OF
static struct of_device_id abov_match_table[] = {
	{ .compatible = "abov,mc96ft16xx",},
	{ },
};
#else
#define abov_match_table NULL
#endif

static struct i2c_driver abov_tk_driver = {
	.probe = abov_tk_probe,
	.remove = abov_tk_remove,
	.shutdown = abov_tk_shutdown,
	.driver = {
		   .name = ABOV_TK_NAME,
		   .of_match_table = abov_match_table,
	},
	.id_table = abov_tk_id,
};

static int __init touchkey_init(void)
{
	pr_err("%s: abov,mc96ft16xx\n", __func__);
#ifdef CONFIG_SAMSUNG_LPM_MODE
	if (poweroff_charging) {
		pr_notice("%s : LPM Charging Mode!!\n", __func__);
		return 0;
	}
#endif

	return i2c_add_driver(&abov_tk_driver);
}

static void __exit touchkey_exit(void)
{
	i2c_del_driver(&abov_tk_driver);
}

module_init(touchkey_init);
module_exit(touchkey_exit);

/* Module information */
MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("Touchkey driver for Abov MF16xx chip");
MODULE_LICENSE("GPL");

