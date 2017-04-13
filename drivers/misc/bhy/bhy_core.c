/*!
* @section LICENSE
 * (C) Copyright 2011~2015 Bosch Sensortec GmbH All Rights Reserved
 *
 * This software program is licensed subject to the GNU General
 * Public License (GPL).Version 2,June 1991,
 * available at http://www.fsf.org/copyleft/gpl.html
*
* @filename bhy_core.c
* @date     "Fri Oct 30 18:57:44 2015 +0800"
* @id       "3230a0f"
*
* @brief
* The implementation file for BHy driver core
*/

#define DRIVER_VERSION "1.3.0.0"

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/string.h>

#include <linux/time.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/swab.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/wakelock.h>
#include <linux/firmware.h>

#include "bhy_core.h"
#include "bhy_host_interface.h"
#include "bs_log.h"

#ifdef BHY_DEBUG
static s64 g_ts[4]; /* For fw load time test */
#endif /*~ BHY_DEBUG */

/** Monitor Thread **/
#define ACC_EVENT_TIMEOUT	15000000000ULL
#define RESET_TIMEOUT		1800000000000ULL  /* 30 min */

static int axis_matrix[8][9] = {
	{ 1, 0, 0, 0, 1, 0, 0, 0, 1, }, /* X Y Z */
	{ 0, 1, 0, -1, 0, 0, 0, 0, 1, }, /* Y -X Z */
	{ -1, 0, 0, 0, -1, 0, 0, 0, 1, }, /* -X -Y Z */
	{ 0, -1, 0, 1, 0, 0, 0, 0, 1, }, /* -Y X Z */
	{ -1, 0, 0, 0, 1, 0, 0, 0, -1, }, /* -X Y -Z */
	{ 0, 1, 0, 1, 0, 0, 0, 0, -1, }, /* Y X -Z */
	{ 1, 0, 0, 0, -1, 0, 0, 0, -1, }, /* X -Y -Z */
	{ 0, -1, 0, -1, 0, 0, 0, 0, -1,}, /* -Y -X Z */
};

static int check_watchdog_reset(struct bhy_client_data *client_data);
void report_last_step_counter_data(struct bhy_client_data *client_data);

static void int_debug(struct bhy_client_data *client_data,
	char *log, const char *func, int line)
{
	static int count;
	int ret;

	if (count++ > INT_DEBUG_COUNT) {
		printk(KERN_INFO "[D]" KERN_DEBUG MODULE_TAG
			"<%s><%d> %s \n", func, line, log);

		disable_irq_nosync(client_data->data_bus.irq);
		client_data->irq_force_disabled = true;

		ret = check_watchdog_reset(client_data);

		/*
		printk(KERN_INFO "[D]" KERN_DEBUG MODULE_TAG
			"<%s><%d> irq_force_disabled \n", func, line);
			*/
		count = 0;
	}
}

static void frame_debug(char *log, const char *func, int line)
{
	static int count;

	if (count++ > FRAME_DEBUG_COUNT) {
		printk(KERN_INFO "[D]" KERN_DEBUG MODULE_TAG
			"<%s><%d> %s \n", func, line, log);
		count = 0;
	}
}

static int bhy_read_reg(struct bhy_client_data *client_data,
		u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return client_data->data_bus.read(client_data->data_bus.dev,
		reg, data, len);
}

static int bhy_write_reg(struct bhy_client_data *client_data,
		u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return client_data->data_bus.write(client_data->data_bus.dev,
		reg, data, len);
}

static int bhy_read_parameter(struct bhy_client_data *client_data,
		u8 page_num, u8 param_num, u8 *data, u8 len)
{
	int ret;
	int retry = BHY_PARAM_ACK_WAIT_RETRY;
	u8 ack, u8_val;

	/* Select page */
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_PAGE_SEL, &page_num, 1);
	if (ret < 0) {
		PERR("Write page request failed");
		return ret;
	}
	/* Select param */
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_REQ, &param_num, 1);
	if (ret < 0) {
		PERR("Write param request failed");
		return ret;
	}
	/* Wait for ack */
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_PARAM_ACK, &ack, 1);
		if (ret < 0) {
			PERR("Read ack reg failed");
			return ret;
		}
		if (ack == 0x80) {
			PERR("Param is not accepted");
			return -EINVAL;
		}
		if (ack == param_num)
			break;
		usleep_range(10000, 20000);
	}
	if (retry == -1) {
		PERR("Wait for ack failed[%d, %d]", page_num, param_num);
		return -EINVAL;
	}
	/* Fetch param data */
	ret = bhy_read_reg(client_data, BHY_REG_SAVED_PARAM_0, data, len);
	if (ret < 0) {
		PERR("Read saved parameter failed");
		return ret;
	}
	/* Clear up */
	u8_val = 0;
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_PAGE_SEL, &u8_val, 1);
	if (ret < 0) {
		PERR("Write page sel failed");
		return ret;
	}
	u8_val = 0;
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_REQ, &u8_val, 1);
	if (ret < 0) {
		PERR("Write param_req failed");
		return ret;
	}
	return len;
}

static int bhy_write_parameter(struct bhy_client_data *client_data,
		u8 page_num, u8 param_num, u8 *data, u8 len)
{
	int ret;
	int retry = BHY_PARAM_ACK_WAIT_RETRY;
	u8 param_num_mod, ack, u8_val;

	/* Write param data */
	ret = bhy_write_reg(client_data, BHY_REG_LOAD_PARAM_0, data, len);
	if (ret < 0) {
		PERR("Write load parameter failed");
		return ret;
	}
	/* Select page */
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_PAGE_SEL, &page_num, 1);
	if (ret < 0) {
		PERR("Write page request failed");
		return ret;
	}
	/* Select param */
	param_num_mod = param_num | 0x80;
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_REQ, &param_num_mod, 1);
	if (ret < 0) {
		PERR("Write param request failed");
		return ret;
	}
	/* Wait for ack */
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_PARAM_ACK, &ack, 1);
		if (ret < 0) {
			PERR("Read ack reg failed");
			return ret;
		}
		if (ack == 0x80) {
			PERR("Param is not accepted");
			return -EINVAL;
		}
		if (ack == param_num_mod)
			break;
		usleep_range(10000, 20000);
	}
	if (retry == -1) {
		PERR("Wait for ack failed[%d, %d]", page_num, param_num);
		return -EINVAL;
	}
	/* Clear up */
	u8_val = 0;
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_PAGE_SEL, &u8_val, 1);
	if (ret < 0) {
		PERR("Write page sel failed");
		return ret;
	}
	u8_val = 0;
	ret = bhy_write_reg(client_data, BHY_REG_PARAM_REQ, &u8_val, 1);
	if (ret < 0) {
		PERR("Write param_req failed");
		return ret;
	}
	return len;
}

/* Soft pass thru op, support max length of 4 */
static int bhy_soft_pass_thru_read_reg(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int ret;
	u8 temp[8];
	int retry = BHY_SOFT_PASS_THRU_READ_RETRY;

	if (len > 4 || len <= 0) {
		PERR("Unsupported read len %d", len);
		return -EINVAL;
	}
	temp[0] = slave_addr;
	temp[1] = reg;
	temp[2] = len;
	ret = bhy_write_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
		BHY_PARAM_SOFT_PASS_THRU_READ, temp, 8);
	if (ret < 0) {
		PERR("Write BHY_PARAM_SOFT_PASS_THRU_READ parameter failed");
		return -EIO;
	}
	do {
		udelay(50);
		ret = bhy_read_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
			BHY_PARAM_SOFT_PASS_THRU_READ, temp, 8);
		if (ret < 0) {
			PERR("Read SOFT_PASS_THRU_READ parameter failed");
			return -EIO;
		}
		if (temp[3])
			break;
	} while (--retry);
	if (retry == 0) {
		PERR("Soft pass thru reg read timed out");
		return -EIO;
	}
	memcpy(data, temp + 4, len);

	return 0;
}

static int bhy_soft_pass_thru_write_reg(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int ret;
	u8 temp[8];
	int retry = BHY_SOFT_PASS_THRU_READ_RETRY;

	if (len > 4 || len <= 0) {
		PERR("Unsupported write len %d", len);
		return -EINVAL;
	}
	temp[0] = slave_addr;
	temp[1] = reg;
	temp[2] = len;
	memcpy(temp + 4, data, len);
	ret = bhy_write_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
		BHY_PARAM_SOFT_PASS_THRU_WRITE, temp, 8);
	if (ret < 0) {
		PERR("Write BHY_PARAM_SOFT_PASS_THRU_WRITE parameter failed");
		return -EIO;
	}
	do {
		udelay(50);
		ret = bhy_read_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
			BHY_PARAM_SOFT_PASS_THRU_WRITE, temp, 8);
		if (ret < 0) {
			PERR("Read SOFT_PASS_THRU_WRITE parameter failed");
			return -EIO;
		}
		if (temp[3])
			break;
	} while (--retry);
	if (retry == 0) {
		PERR("Soft pass thru reg read timed out");
		return -EIO;
	}

	return 0;
}

static int bhy_soft_pass_thru_read_reg_m(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int i;
	int ret;
	for (i = 0; i < len; ++i) {
		ret = bhy_soft_pass_thru_read_reg(client_data, slave_addr,
			reg + i, &data[i], 1);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int bhy_soft_pass_thru_write_reg_m(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int i;
	int ret;
	for (i = 0; i < len; ++i) {
		ret = bhy_soft_pass_thru_write_reg(client_data, slave_addr,
			reg + i, &data[i], 1);
		if (ret < 0)
			return ret;
	}
	return 0;
}

/* Soft pass thru op(non-bust version), support max length of 4 */
#ifdef BHY_RESERVE_FOR_LATER_USE
static int bhy_soft_pass_thru_read_reg_nb(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int ret;
	u8 temp[8];
	int retry = BHY_SOFT_PASS_THRU_READ_RETRY;

	if (len > 4 || len <= 0) {
		PERR("Unsupported read len %d", len);
		return -EINVAL;
	}
	temp[0] = slave_addr;
	temp[1] = reg;
	temp[2] = len;
	ret = bhy_write_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
		BHY_PARAM_SOFT_PASS_THRU_READ_NONBURST, temp, 8);
	if (ret < 0) {
		PERR("Write BHY_PARAM_SOFT_PASS_THRU_READ parameter failed");
		return -EIO;
	}
	do {
		udelay(50);
		ret = bhy_read_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
			BHY_PARAM_SOFT_PASS_THRU_READ_NONBURST, temp, 8);
		if (ret < 0) {
			PERR("Read SOFT_PASS_THRU_READ parameter failed");
			return -EIO;
		}
		if (temp[3])
			break;
	} while (--retry);
	if (retry == 0) {
		PERR("Soft pass thru reg read timed out");
		return -EIO;
	}
	memcpy(data, temp + 4, len);

	return 0;
}
#endif /*~ BHY_RESERVE_FOR_LATER_USE */

#ifdef BHY_RESERVE_FOR_LATER_USE
/* Still not working for now */
static int bhy_soft_pass_thru_write_reg_nb(struct bhy_client_data *client_data,
	u8 slave_addr, u8 reg, u8 *data, u8 len)
{
	int ret;
	u8 temp[8];
	int retry = BHY_SOFT_PASS_THRU_READ_RETRY;

	if (len > 4 || len <= 0) {
		PERR("Unsupported write len %d", len);
		return -EINVAL;
	}
	temp[0] = slave_addr;
	temp[1] = reg;
	temp[2] = len;
	memcpy(temp + 4, data, len);
	ret = bhy_write_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
		BHY_PARAM_SOFT_PASS_THRU_WRITE_NONBURST, temp, 8);
	if (ret < 0) {
		PERR("Write BHY_PARAM_SOFT_PASS_THRU_WRITE parameter failed");
		return -EIO;
	}
	do {
		udelay(50);
		ret = bhy_read_parameter(client_data, BHY_PAGE_SOFT_PASS_THRU,
			BHY_PARAM_SOFT_PASS_THRU_WRITE_NONBURST, temp, 8);
		if (ret < 0) {
			PERR("Read SOFT_PASS_THRU_WRITE parameter failed");
			return -EIO;
		}
		if (temp[3])
			break;
	} while (--retry);
	if (retry == 0) {
		PERR("Soft pass thru reg read timed out");
		return -EIO;
	}

	return 0;
}
#endif /*~ BHY_RESERVE_FOR_LATER_USE */

static int bmi160_read_reg(struct bhy_client_data *client_data,
	u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return bhy_soft_pass_thru_read_reg(client_data, BHY_SLAVE_ADDR_BMI160,
		reg, data, len);
}

static int bmi160_write_reg(struct bhy_client_data *client_data,
	u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return bhy_soft_pass_thru_write_reg_m(client_data,
		BHY_SLAVE_ADDR_BMI160, reg, data, len);
}

static int bma2x2_read_reg(struct bhy_client_data *client_data,
	u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return bhy_soft_pass_thru_read_reg(client_data, BHY_SLAVE_ADDR_BMA2X2,
		reg, data, len);
}

static int bma2x2_write_reg(struct bhy_client_data *client_data,
	u8 reg, u8 *data, u16 len)
{
	if (client_data == NULL)
		return -EIO;
	return bhy_soft_pass_thru_write_reg_m(client_data,
		BHY_SLAVE_ADDR_BMA2X2, reg, data, len);
}

static void bhy_get_ap_timestamp(s64 *ts_ap)
{
	struct timespec ts;
	get_monotonic_boottime(&ts);
	*ts_ap = ts.tv_sec;
	*ts_ap = *ts_ap * 1000000000 + ts.tv_nsec;
}

static int bhy_check_chip_id(struct bhy_data_bus *data_bus)
{
	int ret;
	u8 prod_id;
	ret = data_bus->read(data_bus->dev, BHY_REG_PRODUCT_ID, &prod_id,
		sizeof(u8));
	if (ret < 0) {
		PERR("Read prod id failed");
		return ret;
	}
	switch (prod_id) {
	case BST_FPGA_PRODUCT_ID_7181:
		PINFO("BST FPGA 7181 detected");
		break;
	case BHY_C1_PRODUCT_ID:
		PINFO("BHy C1 sample detected");
		break;
	case BST_FPGA_PRODUCT_ID_7183:
		PINFO("BST FPGA 7183 detected");
		break;
	default:
		PERR("Unknown product ID: 0X%02X", prod_id);
		return -ENODEV;
	}
	return 0;
}

static void sync_sensor(struct bhy_client_data *client_data);
static int bhy_request_irq(struct bhy_client_data *client_data);
static u64 get_current_timestamp(void);

static int bhy_load_ram_patch(struct bhy_client_data *client_data)
{
	ssize_t ret;
	u8 u8_val;
	u16 u16_val;
	u32 u32_val;
	int retry = BHY_RESET_WAIT_RETRY;
	/* int reset_flag_copy; */
	struct file *f;
	mm_segment_t old_fs;
	struct ram_patch_header header;
	loff_t pos;
	ssize_t read_len;
	char data_buf[64]; /* Must be less than burst write max buf */
	u16 remain;
	int i;

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[0]);
#endif /*~ BHY_DEBUG */

	wake_lock(&client_data->patch_wlock);
	mutex_lock(&client_data->mutex_bus_op);

	atomic_set(&client_data->ram_patch_loaded, RAM_PATCH_READY);
	PINFO("Check : ram_patch_loaded = %d ",
		atomic_read(&client_data->ram_patch_loaded));

	client_data->last_acc_check_time = get_current_timestamp();
	PINFO("Set to ram_patch_loaded = false");

	/* Reset FPGA */
	atomic_set(&client_data->reset_flag, RESET_FLAG_TODO);
	u8_val = 1;
	ret = bhy_write_reg(client_data, BHY_REG_RESET_REQ, &u8_val,
		sizeof(u8));
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write reset reg failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);

		wake_unlock(&client_data->patch_wlock);

		return ret;
	}
	/* Ignore checking
	while (retry--) {
		reset_flag_copy = atomic_read(&client_data->reset_flag);
		if (reset_flag_copy == RESET_FLAG_READY)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Reset ready status wait failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}
	*/
	PINFO("FPGA reset successfully");

	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			wake_unlock(&client_data->patch_wlock);
			return -EIO;
		}
		if (u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after reset");
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[1]);
#endif /*~ BHY_DEBUG */

	/* Init upload addr */
	u16_val = 0;
	if (bhy_write_reg(client_data, BHY_REG_UPLOAD_ADDR_0,
		(u8 *)&u16_val, 2) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Init upload addr failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

	/* Write upload request */
	u8_val = 2;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Set chip ctrl failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

	/* Upload data */
	f = filp_open(BHY_DEF_RAM_PATCH_FILE_PATH, O_RDONLY, 0);
	if (f == NULL || IS_ERR(f)) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("open file [%s] error\n", BHY_DEF_RAM_PATCH_FILE_PATH);
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}
	old_fs = get_fs();
	set_fs(get_ds());
	pos = 0;
	read_len = vfs_read(f, (char *)&header, sizeof(header), &pos);
	if (read_len < 0 || read_len != sizeof(header)) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read file header failed");
		set_fs(old_fs);
		filp_close(f, NULL);
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}
	remain = header.data_length;
	if (remain % 4 != 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("data length cannot be divided by 4");
		set_fs(old_fs);
		filp_close(f, NULL);
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EINVAL;
	}
	while (remain > 0) {
		read_len = vfs_read(f, data_buf, sizeof(data_buf), &pos);
		if (read_len < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read file data failed");
			set_fs(old_fs);
			filp_close(f, NULL);
			atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
			wake_unlock(&client_data->patch_wlock);
			return -EIO;
		}
		if (read_len == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("File ended abruptly");
			set_fs(old_fs);
			filp_close(f, NULL);
			atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
			wake_unlock(&client_data->patch_wlock);
			return -EINVAL;
		}
		for (i = 0; i < read_len; i += 4)
			*(u32 *)(data_buf + i) = swab32(*(u32 *)(data_buf + i));
		if (bhy_write_reg(client_data, BHY_REG_UPLOAD_DATA,
			(u8 *)data_buf, read_len) < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write ram patch data failed");
			set_fs(old_fs);
			filp_close(f, NULL);
			atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
			wake_unlock(&client_data->patch_wlock);
			return -EIO;
		}
		remain -= read_len;
	}
	set_fs(old_fs);
	filp_close(f, NULL);

	/* Check CRC */
	if (bhy_read_reg(client_data, BHY_REG_DATA_CRC_0,
		(u8 *)&u32_val, 4) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read CRC failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}
	if (u32_val != header.crc) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("CRC mismatch 0X%08X vs 0X%08X", u32_val, header.crc);
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

	/* Disable upload mode */
	u8_val = 0;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip ctrl reg failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}
	usleep_range(50, 60);
	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			wake_unlock(&client_data->patch_wlock);
			return -EIO;
		}
		if (u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after upload patch");
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[2]);
#endif /*~ BHY_DEBUG */

	/* Enable cpu run */
	u8_val = 1;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip ctrl reg failed #2");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			wake_unlock(&client_data->patch_wlock);
			return -EIO;
		}
		if (!(u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE))
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after CPU run request");
		wake_unlock(&client_data->patch_wlock);
		return -EIO;
	}

	mutex_unlock(&client_data->mutex_bus_op);
	PINFO("Ram patch loaded successfully.");

	if ((client_data->irq_enabled == false) &&
		(atomic_read(&client_data->ram_patch_loaded) == RAM_PATCH_READY)) {

		PERR("Request IRQ!");
		ret = bhy_request_irq(client_data);
		if (ret < 0)
			PERR("Request IRQ failed");
		client_data->irq_enabled = true;
	}

	atomic_set(&client_data->ram_patch_loaded, RAM_PATCH_LOADED);
	PINFO("Check : ram_patch_loaded = %d ",
		atomic_read(&client_data->ram_patch_loaded));

	client_data->last_acc_check_time = get_current_timestamp();

	msleep(300);
	sync_sensor(client_data);
	wake_unlock(&client_data->patch_wlock);

	return 0;
}

int bhy_reinit(struct bhy_client_data *client_data)
{
	int retry;
	u8 reg_data;
	int ret;

	mutex_lock(&client_data->mutex_bus_op);

	reg_data = 0;
	ret = bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip control reg failed");
		return -EIO;
	}
	retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		usleep_range(10000, 11000);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Wait for chip idle status timed out");
		return -EBUSY;
	}
	/* Clear self test bit */
	ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read host ctrl reg failed");
		return -EIO;
	}
	reg_data &= ~HOST_CTRL_MASK_SELF_TEST_REQ;
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write host ctrl reg failed");
		return -EIO;
	}
	/* Enable CPU run from chip control */
	reg_data = 1;
	ret = bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip control reg failed");
		return -EIO;
	}
	retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (!(reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE))
			break;
		usleep_range(10000, 11000);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Wait for chip running status timed out");
		return -EBUSY;
	}

	mutex_unlock(&client_data->mutex_bus_op);

	return 0;
}

static void bhy_init_sensor_type_data_len(struct bhy_client_data *client_data)
{
	int i, len;
	s8 *p = client_data->sensor_data_len;
	len = (int)(sizeof(client_data->sensor_data_len) /
		sizeof(client_data->sensor_data_len[0]));
	for (i = 0; i < len; ++i)
		p[i] = -1;
	p[BHY_SENSOR_HANDLE_ACCELEROMETER] =
		BHY_SENSOR_DATA_LEN_ACCELEROMETER;
	p[BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD] =
		BHY_SENSOR_DATA_LEN_GEOMAGNETIC_FIELD;
	p[BHY_SENSOR_HANDLE_ORIENTATION] =
		BHY_SENSOR_DATA_LEN_ORIENTATION;
	p[BHY_SENSOR_HANDLE_GYROSCOPE] =
		BHY_SENSOR_DATA_LEN_GYROSCOPE;
	p[BHY_SENSOR_HANDLE_LIGHT] =
		BHY_SENSOR_DATA_LEN_LIGHT;
	p[BHY_SENSOR_HANDLE_PRESSURE] =
		BHY_SENSOR_DATA_LEN_PRESSURE;
	p[BHY_SENSOR_HANDLE_TEMPERATURE] =
		BHY_SENSOR_DATA_LEN_TEMPERATURE;
	p[BHY_SENSOR_HANDLE_PROXIMITY] =
		BHY_SENSOR_DATA_LEN_PROXIMITY;
	p[BHY_SENSOR_HANDLE_GRAVITY] =
		BHY_SENSOR_DATA_LEN_GRAVITY;
	p[BHY_SENSOR_HANDLE_LINEAR_ACCELERATION] =
		BHY_SENSOR_DATA_LEN_LINEAR_ACCELERATION;
	p[BHY_SENSOR_HANDLE_ROTATION_VECTOR] =
		BHY_SENSOR_DATA_LEN_ROTATION_VECTOR;
	p[BHY_SENSOR_HANDLE_RELATIVE_HUMIDITY] =
		BHY_SENSOR_DATA_LEN_RELATIVE_HUMIDITY;
	p[BHY_SENSOR_HANDLE_AMBIENT_TEMPERATURE] =
		BHY_SENSOR_DATA_LEN_AMBIENT_TEMPERATURE;
	p[BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED] =
		BHY_SENSOR_DATA_LEN_MAGNETIC_FIELD_UNCALIBRATED;
	p[BHY_SENSOR_HANDLE_GAME_ROTATION_VECTOR] =
		BHY_SENSOR_DATA_LEN_GAME_ROTATION_VECTOR;
	p[BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED] =
		BHY_SENSOR_DATA_LEN_GYROSCOPE_UNCALIBRATED;
	p[BHY_SENSOR_HANDLE_SIGNIFICANT_MOTION] =
		BHY_SENSOR_DATA_LEN_SIGNIFICANT_MOTION;
	p[BHY_SENSOR_HANDLE_STEP_DETECTOR] =
		BHY_SENSOR_DATA_LEN_STEP_DETECTOR;
	p[BHY_SENSOR_HANDLE_STEP_COUNTER] =
		BHY_SENSOR_DATA_LEN_STEP_COUNTER;
	p[BHY_SENSOR_HANDLE_GEOMAGNETIC_ROTATION_VECTOR] =
		BHY_SENSOR_DATA_LEN_GEOMAGNETIC_ROTATION_VECTOR;
	p[BHY_SENSOR_HANDLE_HEART_RATE] =
		BHY_SENSOR_DATA_LEN_HEART_RATE;
	p[BHY_SENSOR_HANDLE_ACCELEROMETER_WU] =
		BHY_SENSOR_DATA_LEN_ACCELEROMETER_WU;
	p[BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD_WU] =
		BHY_SENSOR_DATA_LEN_GEOMAGNETIC_FIELD_WU;
	p[BHY_SENSOR_HANDLE_ORIENTATION_WU] =
		BHY_SENSOR_DATA_LEN_ORIENTATION_WU;
	p[BHY_SENSOR_HANDLE_GYROSCOPE_WU] =
		BHY_SENSOR_DATA_LEN_GYROSCOPE_WU;
	p[BHY_SENSOR_HANDLE_LIGHT_WU] =
		BHY_SENSOR_DATA_LEN_LIGHT_WU;
	p[BHY_SENSOR_HANDLE_PRESSURE_WU] =
		BHY_SENSOR_DATA_LEN_PRESSURE_WU;
	p[BHY_SENSOR_HANDLE_TEMPERATURE_WU] =
		BHY_SENSOR_DATA_LEN_TEMPERATURE_WU;
	p[BHY_SENSOR_HANDLE_PROXIMITY_WU] =
		BHY_SENSOR_DATA_LEN_PROXIMITY_WU;
	p[BHY_SENSOR_HANDLE_GRAVITY_WU] =
		BHY_SENSOR_DATA_LEN_GRAVITY_WU;
	p[BHY_SENSOR_HANDLE_LINEAR_ACCELERATION_WU] =
		BHY_SENSOR_DATA_LEN_LINEAR_ACCELERATION_WU;
	p[BHY_SENSOR_HANDLE_ROTATION_VECTOR_WU] =
		BHY_SENSOR_DATA_LEN_ROTATION_VECTOR_WU;
	p[BHY_SENSOR_HANDLE_RELATIVE_HUMIDITY_WU] =
		BHY_SENSOR_DATA_LEN_RELATIVE_HUMIDITY_WU;
	p[BHY_SENSOR_HANDLE_AMBIENT_TEMPERATURE_WU] =
		BHY_SENSOR_DATA_LEN_AMBIENT_TEMPERATURE_WU;
	p[BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED_WU] =
		BHY_SENSOR_DATA_LEN_MAGNETIC_FIELD_UNCALIBRATED_WU;
	p[BHY_SENSOR_HANDLE_GAME_ROTATION_VECTOR_WU] =
		BHY_SENSOR_DATA_LEN_GAME_ROTATION_VECTOR_WU;
	p[BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED_WU] =
		BHY_SENSOR_DATA_LEN_GYROSCOPE_UNCALIBRATED_WU;
	p[BHY_SENSOR_HANDLE_STEP_DETECTOR_WU] =
		BHY_SENSOR_DATA_LEN_STEP_DETECTOR_WU;
	p[BHY_SENSOR_HANDLE_STEP_COUNTER_WU] =
		BHY_SENSOR_DATA_LEN_STEP_COUNTER_WU;
	p[BHY_SENSOR_HANDLE_GEOMAGNETIC_ROTATION_VECTOR_WU] =
		BHY_SENSOR_DATA_LEN_GEOMAGNETIC_ROTATION_VECTOR_WU;
	p[BHY_SENSOR_HANDLE_HEART_RATE_WU] =
		BHY_SENSOR_DATA_LEN_HEART_RATE_WU;
	p[BHY_SENSOR_HANDLE_TILT_DETECTOR] =
		BHY_SENSOR_DATA_LEN_TILT_DETECTOR;
	p[BHY_SENSOR_HANDLE_WAKE_GESTURE] =
		BHY_SENSOR_DATA_LEN_WAKE_GESTURE;
	p[BHY_SENSOR_HANDLE_GLANCE_GESTURE] =
		BHY_SENSOR_DATA_LEN_GLANCE_GESTURE;
	p[BHY_SENSOR_HANDLE_PICK_UP_GESTURE] =
		BHY_SENSOR_DATA_LEN_PICK_UP_GESTURE;
	p[BHY_SENSOR_HANDLE_ACTIVITY_RECOGNITION] =
		BHY_SENSOR_DATA_LEN_ACTIVITY_RECOGNITION;
	p[BHY_SENSOR_HANDLE_BSX_C] =
		BHY_SENSOR_DATA_LEN_BSX_C;
	p[BHY_SENSOR_HANDLE_BSX_B] =
		BHY_SENSOR_DATA_LEN_BSX_B;
	p[BHY_SENSOR_HANDLE_BSX_A] =
		BHY_SENSOR_DATA_LEN_BSX_A;
	p[BHY_SENSOR_HANDLE_TIMESTAMP_LSW] =
		BHY_SENSOR_DATA_LEN_TIMESTAMP_LSW;
	p[BHY_SENSOR_HANDLE_TIMESTAMP_MSW] =
		BHY_SENSOR_DATA_LEN_TIMESTAMP_MSW;
	p[BHY_SENSOR_HANDLE_META_EVENT] =
		BHY_SENSOR_DATA_LEN_META_EVENT;
	p[BHY_SENSOR_HANDLE_TIMESTAMP_LSW_WU] =
		BHY_SENSOR_DATA_LEN_TIMESTAMP_LSW_WU;
	p[BHY_SENSOR_HANDLE_TIMESTAMP_MSW_WU] =
		BHY_SENSOR_DATA_LEN_TIMESTAMP_MSW_WU;
	p[BHY_SENSOR_HANDLE_META_EVENT_WU] =
		BHY_SENSOR_DATA_LEN_META_EVENT_WU;
	p[BHY_SENSOR_HANDLE_DEBUG] =
		BHY_SENSOR_DATA_LEN_DEBUG;
	p[BHY_SENSOR_HANDLE_CUSTOM_1] =
		BHY_SENSOR_DATA_LEN_CUSTOM_1;
	p[BHY_SENSOR_HANDLE_CUSTOM_2] =
		BHY_SENSOR_DATA_LEN_CUSTOM_2;
	p[BHY_SENSOR_HANDLE_CUSTOM_3] =
		BHY_SENSOR_DATA_LEN_CUSTOM_3;
	p[BHY_SENSOR_HANDLE_CUSTOM_4] =
		BHY_SENSOR_DATA_LEN_CUSTOM_4;
	p[BHY_SENSOR_HANDLE_CUSTOM_5] =
		BHY_SENSOR_DATA_LEN_CUSTOM_5;
	p[BHY_SENSOR_HANDLE_CUSTOM_1_WU] =
		BHY_SENSOR_DATA_LEN_CUSTOM_1_WU;
	p[BHY_SENSOR_HANDLE_CUSTOM_2_WU] =
		BHY_SENSOR_DATA_LEN_CUSTOM_2_WU;
	p[BHY_SENSOR_HANDLE_CUSTOM_3_WU] =
		BHY_SENSOR_DATA_LEN_CUSTOM_3_WU;
	p[BHY_SENSOR_HANDLE_CUSTOM_4_WU] =
		BHY_SENSOR_DATA_LEN_CUSTOM_4_WU;
	p[BHY_SENSOR_HANDLE_CUSTOM_5_WU] =
		BHY_SENSOR_DATA_LEN_CUSTOM_5_WU;
}

#ifdef BHY_AR_HAL_SUPPORT
static int bhy_get_sensor_type_data_len(int sensor_type, int *report_to_ar)
{
	*report_to_ar = 0;
	switch (sensor_type) {
	default:
	case BHY_SENSOR_HANDLE_ZERO:
		return -EINVAL;
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		return BHY_SENSOR_DATA_LEN_ACCELEROMETER;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD:
		return BHY_SENSOR_DATA_LEN_GEOMAGNETIC_FIELD;
	case BHY_SENSOR_HANDLE_ORIENTATION:
		return BHY_SENSOR_DATA_LEN_ORIENTATION;
	case BHY_SENSOR_HANDLE_GYROSCOPE:
		return BHY_SENSOR_DATA_LEN_GYROSCOPE;
	case BHY_SENSOR_HANDLE_LIGHT:
		return BHY_SENSOR_DATA_LEN_LIGHT;
	case BHY_SENSOR_HANDLE_PRESSURE:
		return BHY_SENSOR_DATA_LEN_PRESSURE;
	case BHY_SENSOR_HANDLE_TEMPERATURE:
		return BHY_SENSOR_DATA_LEN_TEMPERATURE;
	case BHY_SENSOR_HANDLE_PROXIMITY:
		return BHY_SENSOR_DATA_LEN_PROXIMITY;
	case BHY_SENSOR_HANDLE_GRAVITY:
		return BHY_SENSOR_DATA_LEN_GRAVITY;
	case BHY_SENSOR_HANDLE_LINEAR_ACCELERATION:
		return BHY_SENSOR_DATA_LEN_LINEAR_ACCELERATION;
	case BHY_SENSOR_HANDLE_ROTATION_VECTOR:
		return BHY_SENSOR_DATA_LEN_ROTATION_VECTOR;
	case BHY_SENSOR_HANDLE_RELATIVE_HUMIDITY:
		return BHY_SENSOR_DATA_LEN_RELATIVE_HUMIDITY;
	case BHY_SENSOR_HANDLE_AMBIENT_TEMPERATURE:
		return BHY_SENSOR_DATA_LEN_AMBIENT_TEMPERATURE;
	case BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED:
		return BHY_SENSOR_DATA_LEN_MAGNETIC_FIELD_UNCALIBRATED;
	case BHY_SENSOR_HANDLE_GAME_ROTATION_VECTOR:
		return BHY_SENSOR_DATA_LEN_GAME_ROTATION_VECTOR;
	case BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED:
		return BHY_SENSOR_DATA_LEN_GYROSCOPE_UNCALIBRATED;
	case BHY_SENSOR_HANDLE_SIGNIFICANT_MOTION:
		return BHY_SENSOR_DATA_LEN_SIGNIFICANT_MOTION;
	case BHY_SENSOR_HANDLE_STEP_DETECTOR:
		return BHY_SENSOR_DATA_LEN_STEP_DETECTOR;
	case BHY_SENSOR_HANDLE_STEP_COUNTER:
		return BHY_SENSOR_DATA_LEN_STEP_COUNTER;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_ROTATION_VECTOR:
		return BHY_SENSOR_DATA_LEN_GEOMAGNETIC_ROTATION_VECTOR;
	case BHY_SENSOR_HANDLE_HEART_RATE:
		return BHY_SENSOR_DATA_LEN_HEART_RATE;
	case BHY_SENSOR_HANDLE_ACCELEROMETER_WU:
		return BHY_SENSOR_DATA_LEN_ACCELEROMETER_WU;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD_WU:
		return BHY_SENSOR_DATA_LEN_GEOMAGNETIC_FIELD_WU;
	case BHY_SENSOR_HANDLE_ORIENTATION_WU:
		return BHY_SENSOR_DATA_LEN_ORIENTATION_WU;
	case BHY_SENSOR_HANDLE_GYROSCOPE_WU:
		return BHY_SENSOR_DATA_LEN_GYROSCOPE_WU;
	case BHY_SENSOR_HANDLE_LIGHT_WU:
		return BHY_SENSOR_DATA_LEN_LIGHT_WU;
	case BHY_SENSOR_HANDLE_PRESSURE_WU:
		return BHY_SENSOR_DATA_LEN_PRESSURE_WU;
	case BHY_SENSOR_HANDLE_TEMPERATURE_WU:
		return BHY_SENSOR_DATA_LEN_TEMPERATURE_WU;
	case BHY_SENSOR_HANDLE_PROXIMITY_WU:
		return BHY_SENSOR_DATA_LEN_PROXIMITY_WU;
	case BHY_SENSOR_HANDLE_GRAVITY_WU:
		return BHY_SENSOR_DATA_LEN_GRAVITY_WU;
	case BHY_SENSOR_HANDLE_LINEAR_ACCELERATION_WU:
		return BHY_SENSOR_DATA_LEN_LINEAR_ACCELERATION_WU;
	case BHY_SENSOR_HANDLE_ROTATION_VECTOR_WU:
		return BHY_SENSOR_DATA_LEN_ROTATION_VECTOR_WU;
	case BHY_SENSOR_HANDLE_RELATIVE_HUMIDITY_WU:
		return BHY_SENSOR_DATA_LEN_RELATIVE_HUMIDITY_WU;
	case BHY_SENSOR_HANDLE_AMBIENT_TEMPERATURE_WU:
		return BHY_SENSOR_DATA_LEN_AMBIENT_TEMPERATURE_WU;
	case BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED_WU:
		return BHY_SENSOR_DATA_LEN_MAGNETIC_FIELD_UNCALIBRATED_WU;
	case BHY_SENSOR_HANDLE_GAME_ROTATION_VECTOR_WU:
		return BHY_SENSOR_DATA_LEN_GAME_ROTATION_VECTOR_WU;
	case BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED_WU:
		return BHY_SENSOR_DATA_LEN_GYROSCOPE_UNCALIBRATED_WU;
	case BHY_SENSOR_HANDLE_STEP_DETECTOR_WU:
		return BHY_SENSOR_DATA_LEN_STEP_DETECTOR_WU;
	case BHY_SENSOR_HANDLE_STEP_COUNTER_WU:
		return BHY_SENSOR_DATA_LEN_STEP_COUNTER_WU;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_ROTATION_VECTOR_WU:
		return BHY_SENSOR_DATA_LEN_GEOMAGNETIC_ROTATION_VECTOR_WU;
	case BHY_SENSOR_HANDLE_HEART_RATE_WU:
		return BHY_SENSOR_DATA_LEN_HEART_RATE_WU;
	case BHY_SENSOR_HANDLE_TILT_DETECTOR:
		return BHY_SENSOR_DATA_LEN_TILT_DETECTOR;
	case BHY_SENSOR_HANDLE_WAKE_GESTURE:
		return BHY_SENSOR_DATA_LEN_WAKE_GESTURE;
	case BHY_SENSOR_HANDLE_GLANCE_GESTURE:
		return BHY_SENSOR_DATA_LEN_GLANCE_GESTURE;
	case BHY_SENSOR_HANDLE_PICK_UP_GESTURE:
		return BHY_SENSOR_DATA_LEN_PICK_UP_GESTURE;
	case BHY_SENSOR_HANDLE_BSX_C:
		return BHY_SENSOR_DATA_LEN_BSX_C;
	case BHY_SENSOR_HANDLE_BSX_B:
		return BHY_SENSOR_DATA_LEN_BSX_B;
	case BHY_SENSOR_HANDLE_BSX_A:
		return BHY_SENSOR_DATA_LEN_BSX_A;
	case BHY_SENSOR_HANDLE_TIMESTAMP_LSW:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_TIMESTAMP_LSW;
	case BHY_SENSOR_HANDLE_TIMESTAMP_MSW:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_TIMESTAMP_MSW;
	case BHY_SENSOR_HANDLE_META_EVENT:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_META_EVENT;
	case BHY_SENSOR_HANDLE_TIMESTAMP_LSW_WU:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_TIMESTAMP_LSW_WU;
	case BHY_SENSOR_HANDLE_TIMESTAMP_MSW_WU:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_TIMESTAMP_MSW_WU;
	case BHY_SENSOR_HANDLE_META_EVENT_WU:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_META_EVENT_WU;
	case BHY_SENSOR_HANDLE_ACTIVITY_RECOGNITION:
		*report_to_ar = 1;
		return BHY_SENSOR_DATA_LEN_ACTIVITY_RECOGNITION;
	case BHY_SENSOR_HANDLE_DEBUG:
		return BHY_SENSOR_DATA_LEN_DEBUG;
	case BHY_SENSOR_HANDLE_CUSTOM_1:
		return BHY_SENSOR_DATA_LEN_CUSTOM_1;
	case BHY_SENSOR_HANDLE_CUSTOM_2:
		return BHY_SENSOR_DATA_LEN_CUSTOM_2;
	case BHY_SENSOR_HANDLE_CUSTOM_3:
		return BHY_SENSOR_DATA_LEN_CUSTOM_3;
	case BHY_SENSOR_HANDLE_CUSTOM_4:
		return BHY_SENSOR_DATA_LEN_CUSTOM_4;
	case BHY_SENSOR_HANDLE_CUSTOM_5:
		return BHY_SENSOR_DATA_LEN_CUSTOM_5;
	case BHY_SENSOR_HANDLE_CUSTOM_1_WU:
		return BHY_SENSOR_DATA_LEN_CUSTOM_1_WU;
	case BHY_SENSOR_HANDLE_CUSTOM_2_WU:
		return BHY_SENSOR_DATA_LEN_CUSTOM_2_WU;
	case BHY_SENSOR_HANDLE_CUSTOM_3_WU:
		return BHY_SENSOR_DATA_LEN_CUSTOM_3_WU;
	case BHY_SENSOR_HANDLE_CUSTOM_4_WU:
		return BHY_SENSOR_DATA_LEN_CUSTOM_4_WU;
	case BHY_SENSOR_HANDLE_CUSTOM_5_WU:
		return BHY_SENSOR_DATA_LEN_CUSTOM_5_WU;
	}
}
#endif /*~ BHY_AR_HAL_SUPPORT */


static void reset(struct bhy_client_data *client_data)
{
	mm_segment_t old_fs;
	struct file *filp = NULL;
	char temp = 0;
	int ret = 0;

	PINFO("Reset sequence started (%d)",
		client_data->irq_force_disabled);

	wake_lock(&client_data->reset_wlock);
	if (client_data->ldo_enable_pin < 0) {
		PINFO("no ldo_enable_pin");
		goto direct_ram_patch;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* Reset mag sensor power */
	PINFO("Reset mag sensor power");
	filp = filp_open(MAG_POWER, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		if (ret != -ENOENT)
			PERR("Can't open mag power file");
	} else {
		ret = filp->f_op->read(filp, (u8 *)&temp,
			sizeof(temp), &filp->f_pos);
		filp_close(filp, current->files);
	}

	/* Reset light sensor power */
	PINFO("Reset light sensor power");
	filp = filp_open(LIGHT_POWER, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		if (ret != -ENOENT)
			PERR("Can't open light power file");
	} else {
		ret = filp->f_op->read(filp, (u8 *)&temp,
			sizeof(temp), &filp->f_pos);
		filp_close(filp, current->files);
	}

	if (client_data->irq_enabled &&
		(!client_data->irq_force_disabled)) {
		disable_irq(client_data->data_bus.irq);
		msleep(20);
	}

	/* Set low for ldo_enable */
	PINFO("ldo_enable_pin = 0");
	gpio_set_value_cansleep(client_data->ldo_enable_pin, 0);
	msleep(20);

	/* Reset light sensor vdd power */
	PINFO("Reset light sensor vdd reset");
	filp = filp_open(LIGHT_VDD_RESET, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		if (ret != -ENOENT)
			PERR("Can't open light vdd power file");
	} else {
		ret = filp->f_op->read(filp, (u8 *)&temp,
			sizeof(temp), &filp->f_pos);
		filp_close(filp, current->files);
	}

	/* Set high for ldo_enable */
	PINFO("ldo_enable_pin = 1");
	gpio_set_value_cansleep(client_data->ldo_enable_pin, 1);
	msleep(20);

	/* reload ram patch */
	ret = bhy_load_ram_patch(client_data);
	if (ret < 0)
		PERR("bhy_load_ram_patch failed");

	if (client_data->irq_enabled)
		enable_irq(client_data->data_bus.irq);

	/* Init light sensor */
	PINFO("Init light sensor");
	filp = filp_open(LIGHT_RESET, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		if (ret != -ENOENT)
			PERR("Can't open light reset file");
	} else {
		ret = filp->f_op->read(filp, (u8 *)&temp,
			sizeof(temp), &filp->f_pos);
		filp_close(filp, current->files);
	}

	/* Init mag sensor */
	PINFO("Init mag sensor");
	filp = filp_open(MAG_RESET, O_RDONLY, 0666);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		if (ret != -ENOENT)
			PERR("Can't open mag reset file");
	} else {
		ret = filp->f_op->read(filp, (u8 *)&temp,
			sizeof(temp), &filp->f_pos);
		filp_close(filp, current->files);
	}

	set_fs(old_fs);
	wake_unlock(&client_data->reset_wlock);
	return;

direct_ram_patch:
	/* reload ram patch */
	ret = bhy_load_ram_patch(client_data);
	if (ret < 0)
		PERR("bhy_load_ram_patch failed");

	if (client_data->irq_force_disabled)
		enable_irq(client_data->data_bus.irq);
	
	wake_unlock(&client_data->reset_wlock);
}

u64 get_current_timestamp(void)
{
	u64 timestamp;
	struct timespec ts;
	/* ts = ktime_to_timespec(ktime_get_boottime()); */
	ts = ktime_to_timespec(ktime_get()); /* Use Monolitic */
	timestamp = ts.tv_sec * 1000000000ULL + ts.tv_nsec;

	return timestamp;
}

static int check_watchdog_reset(struct bhy_client_data *client_data)
{
	int ret = 0;
	u8 reg_data = 0;
	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
		&reg_data, 1);

	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PINFO("Read chip status failed. (%d)", ret);
		return 0; /* Ignore reg_read fail. */
	}

	if (reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE) {
		PINFO("%s : MCU Watchdog! (%d)", MODEL_NAME,
			(reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE));
		ret = 1;
	} else {
		ret = 0;
	}

	return ret;
}

static int mcu_monitor_thread(void *arg)
{
	struct bhy_client_data *client_data = (struct bhy_client_data *)arg;
	u64 tmp_timestamp = 0ULL;
	int ret = 0;
	int i;
	bool duplication_detected = false;

	client_data->cnt_reset = 0;
	client_data->skip_reset = false;
	client_data->cnt_total_reset = 0;
	client_data->last_reset_time_buf[0] = 0ULL;
	client_data->last_reset_time_buf[1] = 0ULL;
	client_data->last_reset_time_buf[2] = 0ULL;
	client_data->cnt_no_response = 0;

	while (likely(!kthread_should_stop())) {
		/* run thread if ram_patch is loaded */
		wait_event_interruptible(client_data->monitor_wq,
			kthread_should_stop() ||
			atomic_read(&client_data->ram_patch_loaded));

		ret = 0;

		if (unlikely(kthread_should_stop())) {
			PINFO("%s Stop!", __func__);
			break;
		}

		if (client_data->irq_force_disabled &&
			(!client_data->skip_reset)) {

			PINFO("Int High detected, Try to Reset#0");
			reset(client_data);
			client_data->irq_force_disabled = false;

			tmp_timestamp = get_current_timestamp();

			client_data->last_reset_time_buf[client_data->cnt_reset%3] = tmp_timestamp;
			client_data->cnt_reset++;
			client_data->cnt_total_reset++;
			client_data->last_acc_check_time = tmp_timestamp;
			client_data->last_reset_time = tmp_timestamp;
		} else {
			tmp_timestamp = get_current_timestamp();
		}

		if (client_data->acc_enabled) {
			/* Monitor by Accelerometer. */
			PINFO("DEBUG [%s]: %5d %5d %5d, RST: %d/3 (%d), SKIP: %d",
				MODEL_NAME,
				client_data->acc_buffer[0] / 4,
				client_data->acc_buffer[1] / 4,
				client_data->acc_buffer[2] / 4,
				client_data->cnt_reset,
				client_data->cnt_total_reset,
				client_data->skip_reset);

			PINFO("Monitor Info: %lld, %lld, %lld",
				client_data->last_reset_time_buf[0],
				client_data->last_reset_time_buf[1],
				client_data->last_reset_time_buf[2]);

			/** Check No event **/
			if (!client_data->skip_reset &&
				(tmp_timestamp > client_data->last_acc_check_time) &&
				(tmp_timestamp - client_data->last_acc_check_time) > ACC_EVENT_TIMEOUT) { /* 15 SEC */

				PINFO("Timeout detected, Try to Reset#1");
				reset(client_data);

				client_data->last_reset_time_buf[client_data->cnt_reset%3] = tmp_timestamp;
				client_data->cnt_reset++;
				client_data->cnt_total_reset++;
				client_data->last_acc_check_time = get_current_timestamp();
				client_data->last_reset_time = tmp_timestamp;
			}

			/** Check Duplicated events **/
			if (!client_data->skip_reset && likely(client_data->cnt_acc_history > 2)) {
				for (i = 1; i < client_data->cnt_acc_history; i++) {
					int j = i - 1;
					if ((client_data->acc_history[j][0] == client_data->acc_history[i][0]) &&
						(client_data->acc_history[j][1] == client_data->acc_history[i][1]) &&
						(client_data->acc_history[j][2] == client_data->acc_history[i][2])) {

						duplication_detected = true;
					} else {
						duplication_detected = false;
						break;
					}
				}
				if (duplication_detected == true) {
					PINFO("Duplication detected, Try to Reset#2");
					client_data->last_reset_time_buf[client_data->cnt_reset] = tmp_timestamp;
					reset(client_data);

					duplication_detected = false;

					client_data->cnt_acc_history = 0;
					memset(client_data->acc_history, 0, sizeof(client_data->acc_history[0][0]) * 10 * 3);

					client_data->cnt_reset++;
					client_data->cnt_total_reset++;
					client_data->last_reset_time = tmp_timestamp;
				}
			}

		} else {
			/* Reset by MCU Watchdog. */
			if (check_watchdog_reset(client_data)) {
				client_data->cnt_no_response++;
				PINFO("MCU Malfunction Detected. %d/3", client_data->cnt_no_response);
			}

			if (!client_data->skip_reset && (client_data->cnt_no_response > 0)) {
				PINFO("MCU Malfunction Detected, Try to Reset#3");
				reset(client_data);
				client_data->last_reset_time = tmp_timestamp;
				client_data->cnt_no_response = 0;
				client_data->cnt_reset++;
			}
		}

		/* reset 3 times in period, no more resets. */
		if (unlikely(client_data->cnt_reset >= 3))
			client_data->skip_reset = true;

		if (unlikely(client_data->skip_reset)) {
			if ((tmp_timestamp - client_data->last_reset_time) > RESET_TIMEOUT) {
				PINFO("Reset check again by timeout");
				client_data->skip_reset = false;
				client_data->cnt_reset = 0;
			}
		}

		msleep(10000);
	}
	return 0;
}

u64 mcu_crystal_to_nano(unsigned int time)
{
	return (u64)((u64) time * MCU_CRY_TO_RT_NS);
}

void process_pedometer(struct bhy_client_data *client_data, u8 *data)
{
	struct pedometer_data new_data;

	memcpy(&new_data, data, sizeof(new_data));

	mutex_lock(&client_data->mutex_pedo);
	/* normal mode */
	if (!new_data.data_index) {
		/* stop mode */
		if (!new_data.step_status) {
			client_data->walk_mode = false;

		/* normal walking */
		} else {
			client_data->walk_mode = true;
			client_data->total_step = new_data.walk_count;
		}
	/* logging mode */
	} else {
		/* starting new logging data */
		if (new_data.data_index > 0 && !client_data->current_index)
			client_data->start_index = new_data.data_index;

		client_data->current_index = new_data.data_index;
		memcpy(&client_data->pedo[client_data->current_index],
			&new_data,
			sizeof(client_data->pedo[client_data->current_index]));
	}

	/* set interrupt */
	client_data->interrupt_mask = 0;

	/* ready to send logging data */
	if (client_data->current_index == 1)
		client_data->interrupt_mask |= LOGGING_DONE;

	/* new normal mode step */
	if (client_data->last_total_step != client_data->total_step) {
		client_data->last_total_step = client_data->total_step;
		client_data->interrupt_mask |= NEW_STEP;
	}

	/* start walking */
	if (!client_data->last_walk_mode && client_data->walk_mode) {
		client_data->last_walk_mode = client_data->walk_mode;
		client_data->interrupt_mask |= START_WALK;
	}

	/* stop walking */
	if (client_data->last_walk_mode && !client_data->walk_mode) {
		client_data->last_walk_mode = client_data->walk_mode;
		client_data->interrupt_mask |= STOP_WALK;
	}

	/* normal mode */
	if (!new_data.data_index) {
		complete(&client_data->int_done);
	/* logging mode */
	} else {
		if (client_data->start_index > 0 &&
			client_data->current_index == 1) {
			complete(&client_data->log_done);
			PINFO("logging complete");
			complete(&client_data->int_done);
		}
	}
	mutex_unlock(&client_data->mutex_pedo);
}

void process_step(struct bhy_client_data *client_data, u8 *data)
{
	struct pedometer_data new_data;
	unsigned int current_step = 0, step_diff = 0;
	static unsigned int last_step;

	if (!client_data->step_det_enabled
			&& !client_data->step_cnt_enabled)
		return;

	memcpy(&new_data, data, sizeof(new_data));

	/* normal mode */
	if (!new_data.data_index) {
		if (client_data->interrupt_mask >= NEW_STEP) {
			/* stop walking */
			if (client_data->interrupt_mask & STOP_WALK)
				return;

			current_step = new_data.walk_count;
			step_diff = current_step - last_step;
			last_step = current_step;

			if (step_diff > FIRST_STEP) {
				if (client_data->interrupt_mask & NEW_STEP) {
					step_diff = 1;
					if (client_data->interrupt_mask
						== START_WALK)
						step_diff = FIRST_STEP;
				}
			} else if (step_diff > 2 && step_diff < FIRST_STEP) {
				step_diff = 1;
			}
		}
	/* logging mode */
	} else {
		step_diff += new_data.walk_count;
		step_diff += new_data.run_count;
		last_step += step_diff;
		client_data->late_step_report = true;

		if (client_data->start_index > 0 &&
			client_data->current_index == 1) {
			client_data->late_step_report = false;
		}
	}

	if (client_data->step_cnt_enabled)
		client_data->step_count += step_diff;

	if (client_data->step_det_enabled)
		if (step_diff)
			client_data->step_det = true;
}

void process_data(struct bhy_client_data *client_data, u8 *data, u16 handle)
{
	struct pedometer_data new_data;
	short acc_temp[3] = { 0, };
	unsigned char recactive_alert_temp = 0;
	static int acc_count;
	int i;
	unsigned int tmp_idx = 0;

	switch (handle) {
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		memcpy(acc_temp, data, sizeof(acc_temp));

		for (i = 0; i < ARRAY_SIZE(acc_temp); i++)
			acc_temp[i] -= client_data->acc_cal[i];

		memcpy(data, acc_temp, sizeof(acc_temp));
		memcpy(&client_data->acc_buffer, data,
				sizeof(client_data->acc_buffer));

		/** Monitor Thread **/
		tmp_idx = client_data->idx_acc_history;
		if (client_data->cnt_acc_history < 10)
			client_data->cnt_acc_history++;

		memcpy(&client_data->acc_history[tmp_idx], data,
			sizeof(client_data->acc_history[tmp_idx]));
		client_data->idx_acc_history = (tmp_idx + 1) % 10;

		if (acc_count++ > ACC_PRINT_TIME * client_data->acc_delay) {
			acc_count = 0;

			/** Monitor Thread **/
			client_data->last_acc_check_time = get_current_timestamp();
		}
		break;

	case AR_SENSOR:
		PINFO("AR : %d", data[0]);
		break;

	case PEDOMETER_SENSOR:
		memcpy(&new_data, data, sizeof(new_data));
		if (new_data.data_index > MAX_LOGGING_SIZE) {
			PINFO("PEDO: Dummy data = %u", new_data.data_index);
		} else {
			PINFO("PEDO: %u, %d, %d, %u, %lld, %lld",
				new_data.data_index, new_data.walk_count,
				new_data.run_count, new_data.step_status,
				mcu_crystal_to_nano(new_data.start_time),
				mcu_crystal_to_nano(new_data.end_time));

			process_pedometer(client_data, data);
			process_step(client_data, data);
		}
		break;

	case REACTIVE_ALERT_SENSOR:
		memcpy(&recactive_alert_temp, data,
			sizeof(recactive_alert_temp));
		client_data->reactive_alert_reported = true;
		PINFO("reactive_alert = %d OK!\n", recactive_alert_temp);
		break;

	default:
		break;
	}
}

void generate_step_data(struct bhy_client_data *client_data)
{
	struct frame_queue *q = &client_data->data_queue;

	if (client_data->step_det_enabled) {
		if (!client_data->step_det)
			goto step_cnt;

		if (client_data->step_det_reported)
			goto step_cnt;

		q->frames[q->head].handle = BHY_SENSOR_HANDLE_STEP_DETECTOR;

		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost for scnt!",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}

		client_data->step_det = false;
		client_data->step_det_reported = true;
	}

step_cnt:
	if (client_data->step_cnt_enabled) {
		if (client_data->last_step_count == client_data->step_count)
			return;
		if (client_data->late_step_report == true)
			return;

		q->frames[q->head].handle = BHY_SENSOR_HANDLE_STEP_COUNTER;
		memcpy(q->frames[q->head].data, &client_data->step_count,
				BHY_SENSOR_DATA_LEN_STEP_COUNTER);

		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}

		client_data->last_step_count = client_data->step_count;
	}
}

void report_last_step_counter_data(
	struct bhy_client_data *client_data)
{
	struct frame_queue *q = &client_data->data_queue;
	if (client_data->step_cnt_enabled) {
		PINFO("STEP: last step cnt = %d", client_data->step_count);
		q->frames[q->head].handle = BHY_SENSOR_HANDLE_STEP_COUNTER;
		memcpy(q->frames[q->head].data, &client_data->step_count,
			BHY_SENSOR_DATA_LEN_STEP_COUNTER);

		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}
		client_data->last_step_count = client_data->step_count;
	}
}

void detect_init_event(struct bhy_client_data *client_data)
{
	u16 bytes_remain;
	u8 *data = client_data->fifo_buf;
	int sensor_type;
	int parse_index, data_len;
	int ret;
	/*int dummy;*/
	struct frame_queue *q = &client_data->data_queue;

	mutex_lock(&client_data->mutex_bus_op);
	if (bhy_read_reg(client_data, BHY_REG_BYTES_REMAIN_0,
			(u8 *)&bytes_remain, 2) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read bytes remain reg failed");
		return;
	}
#ifdef BHY_DEBUG
	if (client_data->enable_irq_log)
		PDEBUG("Fifo length: %d", bytes_remain);
#endif /*~ BHY_DEBUG */
	if (bytes_remain == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		int_debug(client_data, "Zero length FIFO detected",
			__func__, __LINE__);
		return;
	}
	if (bytes_remain > BHY_FIFO_LEN_MAX) {
		mutex_unlock(&client_data->mutex_bus_op);
		PDEBUG("Start up sequence error: Over sized FIFO");
		return;
	}
	ret = bhy_read_reg(client_data, BHY_REG_FIFO_BUFFER_0,
			data, bytes_remain);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read fifo data failed");
		return;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	mutex_lock(&q->lock);
	for (parse_index = 0; parse_index < bytes_remain;
			parse_index += data_len + 1) {
		sensor_type = data[parse_index];
		data_len = client_data->sensor_data_len[sensor_type];
		if (data_len < 0)
			break;
		if (parse_index + data_len >= bytes_remain) {
			PERR("Invalid FIFO data detected for sensor_type %d",
				sensor_type);
			break;
		}
		if (sensor_type == BHY_SENSOR_HANDLE_META_EVENT &&
			data[parse_index + 1] == META_EVENT_INITIALIZED) {
			atomic_set(&client_data->reset_flag,
					RESET_FLAG_INITIALIZED);
#ifdef BHY_DEBUG
			bhy_get_ap_timestamp(&g_ts[3]);
			PDEBUG("ts-0: %lld", g_ts[0]);
			PDEBUG("ts-1: %lld", g_ts[1]);
			PDEBUG("ts-2: %lld", g_ts[2]);
			PDEBUG("ts-3: %lld", g_ts[3]);
#endif /*~ BHY_DEBUG */
		}
		q->frames[q->head].handle = sensor_type;
		memcpy(q->frames[q->head].data,
				&data[parse_index + 1], data_len);
		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);
}

void detect_self_test_event(struct bhy_client_data *client_data)
{
	u16 bytes_remain;
	u8 *data = client_data->fifo_buf;
	int sensor_type;
	int parse_index, data_len;
	int ret;
	/*int dummy;*/
	struct frame_queue *q = &client_data->data_queue;
	int idx; /* For self test index */
	int result_detected = 0;
	int i;
	u8 param_data[16];

	mutex_lock(&client_data->mutex_bus_op);
	if (bhy_read_reg(client_data, BHY_REG_BYTES_REMAIN_0,
			(u8 *)&bytes_remain, 2) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read bytes remain reg failed");
		return;
	}
#ifdef BHY_DEBUG
	if (client_data->enable_irq_log)
		PDEBUG("Fifo length: %d", bytes_remain);
#endif /*~ BHY_DEBUG */
	if (bytes_remain == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		int_debug(client_data, "Zero length FIFO detected",
			__func__, __LINE__);
		return;
	}
	if (bytes_remain > BHY_FIFO_LEN_MAX) {
		PDEBUG("Start up sequence error: Over sized FIFO");
		return;
	}
	ret = bhy_read_reg(client_data, BHY_REG_FIFO_BUFFER_0,
			data, bytes_remain);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read fifo data failed");
		return;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	mutex_lock(&q->lock);
	for (parse_index = 0; parse_index < bytes_remain;
			parse_index += data_len + 1) {
		sensor_type = data[parse_index];
		data_len = client_data->sensor_data_len[sensor_type];
		if (data_len < 0)
			break;
		if (parse_index + data_len >= bytes_remain) {
			PERR("Invalid FIFO data detected for sensor_type %d",
				sensor_type);
			break;
		}
		if (sensor_type == BHY_SENSOR_HANDLE_META_EVENT &&
			data[parse_index + 1] == META_EVENT_SELF_TEST_RESULTS) {
			idx = -1;
			switch (data[parse_index + 2]) {
			case BHY_SENSOR_HANDLE_ACCELEROMETER:
				idx = SELF_TEST_RESULT_INDEX_ACC;
				break;
			case BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED:
				idx = SELF_TEST_RESULT_INDEX_MAG;
				break;
			case BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED:
				idx = SELF_TEST_RESULT_INDEX_GYRO;
				break;
			}
			if (idx != -1)
				client_data->self_test_result[idx] =
					(s8)data[parse_index + 3];
			result_detected = 1;
		}
		q->frames[q->head].handle = sensor_type;
		memcpy(q->frames[q->head].data,
				&data[parse_index + 1], data_len);
		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

	/* Reload ram patch */
	if (result_detected) {
		mutex_lock(&client_data->mutex_bus_op);
		for (i = 0; i < 3; ++i) {
			ret = bhy_read_parameter(client_data,
					BHY_PAGE_ALGORITHM,
					BHY_PARAM_SELFTEST_DIFF_X + i,
					param_data, 2);
			if (ret < 0) {
				PERR("Read self test diff failed");
				break;
			}
			client_data->acc_self_test_diff[i] = *(s16 *)param_data;
		}
		mutex_unlock(&client_data->mutex_bus_op);
		/* bhy_reinit(client_data); */
		atomic_set(&client_data->reset_flag,
			RESET_FLAG_INITIALIZED);
	}
}

#ifdef BHY_DEBUG
static void bhy_dump_fifo_data(const u8 *data, int len)
{
	int i, j;
	char buf[256];
	int line_char = 0;
	const int bytes_per_line = 8;
	PDEBUG("Data is");
	for (i = j = 0; i < len; ++i) {
		j += snprintf(buf + j, 16, "%02X ", *(data + i));
		if (++line_char == bytes_per_line) {
			buf[j - 1] = '\0';
			PDEBUG("%s", buf);
			line_char = 0;
			j = 0;
		}
	}
	if (line_char > 0) {
		buf[j - 1] = '\0';
		PDEBUG("%s", buf);
	}
}
#endif /*~ BHY_DEBUG */

static void bhy_read_fifo_data(struct bhy_client_data *client_data)
{
	int ret;
	u16 bytes_remain;
	int sensor_type;
	int parse_index, data_len;
#ifdef BHY_AR_HAL_SUPPORT
	int report_to_ar;
#endif /*~ BHY_AR_HAL_SUPPORT */
	struct frame_queue *q = &client_data->data_queue;
#ifdef BHY_AR_HAL_SUPPORT
	struct frame_queue *qa = &client_data->data_queue_ar;
#endif /*~ BHY_AR_HAL_SUPPORT */

	mutex_lock(&client_data->mutex_bus_op);
	if (bhy_read_reg(client_data, BHY_REG_BYTES_REMAIN_0,
			(u8 *)&bytes_remain, 2) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read bytes remain reg failed");
		return;
	}
#ifdef BHY_DEBUG
	if (client_data->enable_irq_log)
		PDEBUG("Fifo length: %d", bytes_remain);
#endif /*~ BHY_DEBUG */
	if (bytes_remain == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		int_debug(client_data, "Zero length FIFO detected",
			__func__, __LINE__);
		return;
	}

	/* Over sized FIFO detected */
	if (bytes_remain > BHY_FIFO_LEN_MAX) {
		mutex_unlock(&client_data->mutex_bus_op);
		PDEBUG("Over sized FIFO detected");
		return;
	}

	ret = bhy_read_reg(client_data, BHY_REG_FIFO_BUFFER_0,
			client_data->fifo_buf, bytes_remain);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read fifo data failed");
		return;
	}
	mutex_unlock(&client_data->mutex_bus_op);
#ifdef BHY_DEBUG
	if (client_data->enable_fifo_log)
		bhy_dump_fifo_data(client_data->fifo_buf, bytes_remain);
#endif /*~ BHY_DEBUG */

	mutex_lock(&q->lock);
#ifdef BHY_AR_HAL_SUPPORT
	mutex_lock(&qa->lock);
#endif /*~ BHY_AR_HAL_SUPPORT */
	for (parse_index = 0; parse_index < bytes_remain;
			parse_index += data_len + 1) {
		sensor_type = client_data->fifo_buf[parse_index];
#ifdef BHY_AR_HAL_SUPPORT
		data_len = bhy_get_sensor_type_data_len(sensor_type,
				&report_to_ar);
#endif /*~ BHY_AR_HAL_SUPPORT */
		data_len = client_data->sensor_data_len[sensor_type];

		if (sensor_type == BHY_SENSOR_HANDLE_STEP_DETECTOR
			|| sensor_type == BHY_SENSOR_HANDLE_STEP_COUNTER)
			continue;

		if (data_len < 0)
			break;
		if (parse_index + data_len >= bytes_remain) {
			PERR("Invalid FIFO data detected for sensor_type %d",
					sensor_type);
			break;
		}
		q->frames[q->head].handle = sensor_type;
		memcpy(q->frames[q->head].data,
			&client_data->fifo_buf[parse_index + 1], data_len);

		/* process sensor data */
		process_data(client_data,
			q->frames[q->head].data, q->frames[q->head].handle);

		if (q->head == BHY_FRAME_SIZE - 1)
			q->head = 0;
		else
			++q->head;
		if (q->head == q->tail) {
			frame_debug("One frame data lost",
				__func__, __LINE__);
			if (q->tail == BHY_FRAME_SIZE - 1)
				q->tail = 0;
			else
				++q->tail;
		}

		/* generate step detector or step counter data */
		if (sensor_type == PEDOMETER_SENSOR)
			generate_step_data(client_data);

#ifdef BHY_AR_HAL_SUPPORT
		if (report_to_ar) {
			qa->frames[qa->head].handle = sensor_type;
			memcpy(qa->frames[qa->head].data,
				&client_data->fifo_buf[parse_index + 1],
				data_len);
			if (qa->head == BHY_FRAME_SIZE_AR - 1)
				qa->head = 0;
			else
				++qa->head;
			if (qa->head == qa->tail) {
				if (qa->tail == BHY_FRAME_SIZE_AR - 1)
					qa->tail = 0;
				else
					++qa->tail;
			}
		}
#endif /*~ BHY_AR_HAL_SUPPORT */
	}
	client_data->step_det_reported = false;
#ifdef BHY_AR_HAL_SUPPORT
	mutex_unlock(&qa->lock);
#endif /*~ BHY_AR_HAL_SUPPORT */
	mutex_unlock(&q->lock);
}

static void bhy_irq_work_func(struct bhy_client_data *client_data);

static irqreturn_t bhy_irq_handler(int irq, void *handle)
{
	struct bhy_client_data *client_data = handle;
	int reset_flag_copy;
	if (client_data == NULL)
		return IRQ_HANDLED;
	reset_flag_copy = atomic_read(&client_data->reset_flag);
	client_data->irq_ready = true;
	if (reset_flag_copy == RESET_FLAG_TODO) {
		atomic_set(&client_data->reset_flag, RESET_FLAG_READY);
		return IRQ_HANDLED;
	}
	bhy_get_ap_timestamp(&client_data->timestamp_irq);

	bhy_irq_work_func(client_data);

	return IRQ_HANDLED;
}

static void bhy_irq_work_func(struct bhy_client_data *client_data)
{
	int reset_flag_copy, in_suspend_copy;
	int ret;
	u8 timestamp_fw[4];
	struct frame_queue *q = &client_data->data_queue;
#ifdef BHY_AR_HAL_SUPPORT
	struct frame_queue *qa = &client_data->data_queue_ar;
#endif /*~ BHY_AR_HAL_SUPPORT */
#ifdef BHY_DEBUG
	u8 irq_status;
#endif /*~ BHY_DEBUG */

	/* Detect reset event */
	reset_flag_copy = atomic_read(&client_data->reset_flag);
	switch (reset_flag_copy) {
	case RESET_FLAG_TODO:
		atomic_set(&client_data->reset_flag, RESET_FLAG_READY);
		return;
	case RESET_FLAG_READY:
		detect_init_event(client_data);
		return;
	case RESET_FLAG_SELF_TEST:
		detect_self_test_event(client_data);
		return;
	default:
		break;
	}

	in_suspend_copy = atomic_read(&client_data->in_suspend);
	if (in_suspend_copy) {
		wake_lock(&client_data->wlock);
		msleep(20);
	}

#ifdef BHY_DEBUG
	if (client_data->enable_irq_log) {
		irq_status = 0;
		mutex_lock(&client_data->mutex_bus_op);
		ret = bhy_read_reg(client_data, BHY_REG_INT_STATUS,
			&irq_status, 1);
		mutex_unlock(&client_data->mutex_bus_op);
		if (ret < 0)
			PERR("Read IRQ status failed");
		PDEBUG("In IRQ, timestamp: %llu, irq_type: 0x%02X",
			client_data->timestamp_irq, irq_status);
	}
#endif /*~ BHY_DEBUG */

	/* Report timestamp sync */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_HOST_IRQ_TIMESTAMP_1,
		timestamp_fw, 4);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0)
		PERR("Get firmware timestamp failed");
	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_TIMESTAMP_SYNC;
	memcpy(q->frames[q->head].data,
		&client_data->timestamp_irq, sizeof(u64));
	memcpy(q->frames[q->head].data + sizeof(u64),
		&timestamp_fw, sizeof(timestamp_fw));
#ifdef BHY_TS_LOGGING_SUPPORT
	++client_data->irq_count;
	memcpy(q->frames[q->head].data + sizeof(u64) + sizeof(timestamp_fw),
		&client_data->irq_count, sizeof(u32));
#endif /*~ BHY_TS_LOGGING_SUPPORT */
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);
#ifdef BHY_AR_HAL_SUPPORT
	mutex_lock(&qa->lock);
	qa->frames[qa->head].handle = BHY_SENSOR_HANDLE_TIMESTAMP_SYNC;
	memcpy(qa->frames[qa->head].data,
		&client_data->timestamp_irq, sizeof(u64));
	memcpy(qa->frames[qa->head].data + sizeof(u64),
		&timestamp_fw, sizeof(timestamp_fw));
	if (qa->head == BHY_FRAME_SIZE_AR - 1)
		qa->head = 0;
	else
		++qa->head;
	if (qa->head == qa->tail) {
		if (qa->tail == BHY_FRAME_SIZE_AR - 1)
			qa->tail = 0;
		else
			++qa->tail;
	}
	mutex_unlock(&qa->lock);
#endif /*~ BHY_AR_HAL_SUPPORT */

	/* Read FIFO data */
	bhy_read_fifo_data(client_data);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

#ifdef BHY_AR_HAL_SUPPORT
	input_event(client_data->input_ar, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input_ar);
#endif /*~ BHY_AR_HAL_SUPPORT */

	if (in_suspend_copy)
		wake_unlock(&client_data->wlock);
}

static int bhy_request_irq(struct bhy_client_data *client_data)
{
	struct bhy_data_bus *data_bus = &client_data->data_bus;
	int ret;
	int irq_gpio, irq;
	data_bus->irq = -1;
	irq_gpio = of_get_named_gpio_flags(data_bus->dev->of_node,
		"bhy,gpio_irq", 0, NULL);
	ret = gpio_request_one(irq_gpio, GPIOF_IN, "bhy_int");
	if (ret)
		return ret;
	ret = gpio_direction_input(irq_gpio);
	if (ret < 0)
		return ret;
	irq = gpio_to_irq(irq_gpio);
	/* INIT_WORK(&client_data->irq_work, bhy_irq_work_func);*/

	/* Do not use RISING EDGE, Use HIGH LEVEL
	ret = request_irq(irq, bhy_irq_handler, IRQF_TRIGGER_RISING,
			SENSOR_NAME, client_data);
	*/

	ret = request_threaded_irq(irq, NULL, bhy_irq_handler,
		IRQF_TRIGGER_HIGH | IRQF_ONESHOT, SENSOR_NAME, client_data);

	if (ret < 0)
		return ret;
	ret = device_init_wakeup(data_bus->dev, 1);
	if (ret < 0) {
		PDEBUG("Init device wakeup failed");
		return ret;
	}
	data_bus->irq = irq;
	return 0;
}

static int bhy_init_input_dev(struct bhy_client_data *client_data)
{
	struct input_dev *dev;
	int ret;

	dev = input_allocate_device();
	if (dev == NULL) {
		PERR("Allocate input device failed");
		return -ENOMEM;
	}

	dev->name = SENSOR_INPUT_DEV_NAME;
	dev->id.bustype = client_data->data_bus.bus_type;

	input_set_capability(dev, EV_MSC, MSC_RAW);
	input_set_drvdata(dev, client_data);

	ret = input_register_device(dev);
	if (ret < 0) {
		input_free_device(dev);
		PERR("Register input device failed");
		return ret;
	}
	client_data->input = dev;

#ifdef BHY_AR_HAL_SUPPORT
	dev = input_allocate_device();
	if (dev == NULL) {
		PERR("Allocate input device failed for AR");
		return -ENOMEM;
	}

	dev->name = SENSOR_AR_INPUT_DEV_NAME;
	dev->id.bustype = client_data->data_bus.bus_type;

	input_set_capability(dev, EV_MSC, MSC_RAW);
	input_set_drvdata(dev, client_data);

	ret = input_register_device(dev);
	if (ret < 0) {
		input_free_device(dev);
		PERR("Register input device for AR failed");
		return ret;
	}
	client_data->input_ar = dev;
#endif /*~ BHY_AR_HAL_SUPPORT */

	return 0;
}

static ssize_t bhy_show_rom_id(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	__le16 reg_data;
	u16 rom_id;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_ROM_VERSION_0,
		(u8 *)&reg_data, 2);
	mutex_unlock(&client_data->mutex_bus_op);

	if (ret < 0)
		return ret;
	rom_id = __le16_to_cpu(reg_data);
	ret = snprintf(buf, 32, "%d\n", (int)rom_id);

	return ret;
}

static ssize_t bhy_show_ram_id(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	__le16 reg_data;
	u16 ram_id;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_RAM_VERSION_0,
		(u8 *)&reg_data, 2);
	mutex_unlock(&client_data->mutex_bus_op);

	if (ret < 0)
		return ret;
	ram_id = __le16_to_cpu(reg_data);
	ret = snprintf(buf, 32, "%d\n", (int)ram_id);

	return ret;
}

static ssize_t bhy_store_load_ram_patch(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	long req;
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = kstrtol(buf, 10, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid request");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EINVAL;
	}

	ret = bhy_load_ram_patch(client_data);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t bhy_show_status_bank(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	int i;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	for (i = BHY_PARAM_SYSTEM_STAUS_BANK_0;
			i <= BHY_PARAM_SYSTEM_STAUS_BANK_3; ++i) {
		ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM, i,
				(u8 *)(buf + (i - BHY_PARAM_SYSTEM_STAUS_BANK_0)
				* 16), 16);
		if (ret < 0) {
			PERR("Read BHY_PARAM_SYSTEM_STAUS_BANK_%d error",
					i - BHY_PARAM_SYSTEM_STAUS_BANK_0);
			mutex_unlock(&client_data->mutex_bus_op);
			return ret;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	return BHY_SENSOR_STATUS_BANK_LEN;
}

static ssize_t bhy_store_sensor_sel(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	client_data->sensor_sel = buf[0];

	return count;
}

static ssize_t bhy_show_sensor_info(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	if (client_data->sensor_sel <= 0 ||
			client_data->sensor_sel > BHY_SENSOR_HANDLE_MAX) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
	ret = bhy_read_parameter(client_data, BHY_PAGE_SENSOR,
			BHY_PARAM_SENSOR_INFO_0 + client_data->sensor_sel,
			(u8 *)buf, 16);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read parameter error");
		return ret;
	}

	return 16;
}

static ssize_t bhy_show_sensor_conf(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	if (client_data->sensor_sel <= 0 ||
			client_data->sensor_sel > BHY_SENSOR_HANDLE_MAX) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
	ret = bhy_read_parameter(client_data, BHY_PAGE_SENSOR,
			BHY_PARAM_SENSOR_CONF_0 + client_data->sensor_sel,
			(u8 *)buf, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read parameter error");
		return ret;
	}

	return 8;
}

static int accel_open_calibration(struct bhy_client_data *client_data);
static int enable_pedometer(struct bhy_client_data *client_data, bool enable);

static ssize_t bhy_store_sensor_conf(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	if (client_data->sensor_sel <= 0 ||
			client_data->sensor_sel > BHY_SENSOR_HANDLE_MAX) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel: %d", client_data->sensor_sel);
		return -EINVAL;
	}
	ret = bhy_write_parameter(client_data, BHY_PAGE_SENSOR,
			BHY_PARAM_SENSOR_CONF_0 + client_data->sensor_sel,
			(u8 *)buf, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write parameter error");
		return ret;
	}

	PINFO("sensor: %d, enable: %d",
		client_data->sensor_sel, buf[0] | buf[1]);

	/* check acc sensor is enabled */
	if (client_data->sensor_sel == BHY_SENSOR_HANDLE_ACCELEROMETER) {
		client_data->acc_enabled = buf[0] | buf[1];
		if (client_data->acc_enabled) {
			accel_open_calibration(client_data);
			client_data->acc_delay = buf[1] << 8 | buf[0];

			/** Monitor Thread **/
			client_data->last_acc_check_time = get_current_timestamp();
		}
	} else if (client_data->sensor_sel == BHY_SENSOR_HANDLE_STEP_DETECTOR) {
		ret = enable_pedometer(client_data, (bool)(buf[0] | buf[1]));
		if (ret < 0)
			return ret;

		client_data->step_det_enabled = buf[0] | buf[1];
	} else if (client_data->sensor_sel == BHY_SENSOR_HANDLE_STEP_COUNTER) {
		ret = enable_pedometer(client_data, (bool)(buf[0] | buf[1]));
		if (ret < 0)
			return ret;

		client_data->step_cnt_enabled = buf[0] | buf[1];
		report_last_step_counter_data(client_data);
	} else if (client_data->sensor_sel == BHY_SENSOR_HANDLE_TILT_DETECTOR) {
		client_data->tilt_enabled = buf[0] | buf[1];
	} else if (client_data->sensor_sel
		== BHY_SENSOR_HANDLE_PICK_UP_GESTURE) {
		client_data->pickup_enabled = buf[0] | buf[1];
	} else if (client_data->sensor_sel
		== BHY_SENSOR_HANDLE_SIGNIFICANT_MOTION) {
		client_data->smd_enabled = buf[0] | buf[1];
	} else if (client_data->sensor_sel == AR_SENSOR) {
		client_data->ar_enabled = buf[0] | buf[1];
	}

	/** Monitor Thread **/
	wake_up(&client_data->monitor_wq);

	return count;
}

static ssize_t bhy_store_sensor_flush(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 sensor_sel = buf[0];

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	if (sensor_sel <= 0 || (sensor_sel > BHY_SENSOR_HANDLE_MAX
		&& sensor_sel != BHY_FLUSH_DISCARD_ALL
		&& sensor_sel != BHY_FLUSH_FLUSH_ALL)) {
		PERR("Invalid sensor sel: %d", sensor_sel);
		return -EINVAL;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_reg(client_data, BHY_REG_FIFO_FLUSH, &sensor_sel, 1);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write flush sensor reg error");
		return ret;
	}

	return count;
}

static ssize_t bhy_show_calib_profile(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	u8 param_num;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
#ifdef BHY_CALIB_PROFILE_OP_IN_FUSER_CORE
	switch (client_data->sensor_sel) {
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		param_num = BHY_PARAM_OFFSET_ACC_2;
		break;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD:
		param_num = BHY_PARAM_OFFSET_MAG_2;
		break;
	case BHY_SENSOR_HANDLE_GYROSCOPE:
		param_num = BHY_PARAM_OFFSET_GYRO_2;
		break;
	default:
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
#else
	switch (client_data->sensor_sel) {
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		param_num = BHY_PARAM_OFFSET_ACC;
		break;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD:
		param_num = BHY_PARAM_OFFSET_MAG;
		break;
	case BHY_SENSOR_HANDLE_GYROSCOPE:
		param_num = BHY_PARAM_OFFSET_GYRO;
		break;
	default:
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
#endif /*~ BHY_CALIB_PROFILE_OP_IN_FUSER_CORE */
	ret = bhy_read_parameter(client_data, BHY_PAGE_ALGORITHM,
		param_num, (u8 *)buf, BHY_CALIB_PROFILE_LEN);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read parameter error");
		return ret;
	}

	return BHY_CALIB_PROFILE_LEN;
}

static ssize_t bhy_store_calib_profile(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 param_num;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
#ifdef BHY_CALIB_PROFILE_OP_IN_FUSER_CORE
	switch (client_data->sensor_sel) {
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		param_num = BHY_PARAM_OFFSET_ACC_2;
		break;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD:
		param_num = BHY_PARAM_OFFSET_MAG_2;
		break;
	case BHY_SENSOR_HANDLE_GYROSCOPE:
		param_num = BHY_PARAM_OFFSET_GYRO_2;
		break;
	default:
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
#else
	switch (client_data->sensor_sel) {
	case BHY_SENSOR_HANDLE_ACCELEROMETER:
		param_num = BHY_PARAM_OFFSET_ACC;
		break;
	case BHY_SENSOR_HANDLE_GEOMAGNETIC_FIELD:
		param_num = BHY_PARAM_OFFSET_MAG;
		break;
	case BHY_SENSOR_HANDLE_GYROSCOPE:
		param_num = BHY_PARAM_OFFSET_GYRO;
		break;
	default:
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Invalid sensor sel");
		return -EINVAL;
	}
#endif /*~ BHY_CALIB_PROFILE_OP_IN_FUSER_CORE */
	ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
		param_num, (u8 *)buf, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write parameter error");
		return ret;
	}

	return count;
}

static ssize_t bhy_show_sic_matrix(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	u8 data[36];
	int i;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	for (i = BHY_PARAM_SIC_MATRIX_0_1; i <= BHY_PARAM_SIC_MATRIX_8; ++i) {
		ret = bhy_read_parameter(client_data, BHY_PAGE_ALGORITHM,
				i, (u8 *)(data + (i - 1) * 8),
				i == BHY_PARAM_SIC_MATRIX_8 ? 4 : 8);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read parameter error");
			return ret;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);
	ret = 0;
	for (i = 0; i < 9; ++i)
		ret += snprintf(buf + ret, 16, "%02X %02X %02X %02X\n",
				data[i * 4], data[i * 4 + 1],
				data[i * 4 + 2], data[i * 4 + 3]);

	return ret;
}

static ssize_t bhy_store_sic_matrix(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	int i;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	for (i = BHY_PARAM_SIC_MATRIX_0_1; i <= BHY_PARAM_SIC_MATRIX_8; ++i) {
		ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
				i, (u8 *)(buf + (i - 1) * 8),
				i == BHY_PARAM_SIC_MATRIX_8 ? 4 : 8);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write parameter error");
			return ret;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	return count;
}

static ssize_t bhy_show_meta_event_ctrl(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	u8 data[8];
	int i, j;
	ssize_t len;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
			BHY_PARAM_SYSTEM_META_EVENT_CTRL, data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read meta event ctrl failed");
		return -EIO;
	}
	len = 0;
	len += snprintf(buf + len, 64, "Non wake up meta event\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 4; ++j)
			len += snprintf(buf + len, 64,
					"Meta event #%d: event_en=%d, irq_en=%d\n",
					i * 4 + j + 1,
					(data[i] >> (j * 2 + 1)) & 1,
					(data[i] >> (j * 2)) & 1);
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
			BHY_PARAM_SYSTEM_WAKE_UP_META_EVENT_CTRL, data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read wake up meta event ctrl failed");
		return -EIO;
	}
	len += snprintf(buf + len, 64, "Wake up meta event\n");
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 4; ++j)
			len += snprintf(buf + len, 64,
					"Meta event #%d: event_en=%d, irq_en=%d\n",
					i * 4 + j + 1,
					(data[i] >> (j * 2 + 1)) & 1,
					(data[i] >> (j * 2)) & 1);
	}

	return len;
}

/* Byte0: meta event type; Byte1: event enable; Byte2: IRQ enable;
   Byte3: 0 for non-wakeup, 1 for wakeup */
static ssize_t bhy_store_meta_event_ctrl(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 data[8];
	int type, event_en, irq_en, num, bit;
	u8 param;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	type = buf[0];
	if (type <= 0 || type > 32) {
		PERR("Invalid meta event type");
		return -EINVAL;
	}
	event_en = buf[1] & 0x1;
	irq_en = buf[2] & 0x1;
	num = (type - 1) / 4;
	bit = (type - 1) % 4;
	param = buf[3] ? BHY_PARAM_SYSTEM_WAKE_UP_META_EVENT_CTRL :
		BHY_PARAM_SYSTEM_META_EVENT_CTRL;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM, param,
		data, 8);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read meta event failed");
		return -EIO;
	}
	if (event_en)
		data[num] |= (1 << (bit * 2 + 1));
	else
		data[num] &= ~(1 << (bit * 2 + 1));
	if (irq_en)
		data[num] |= (1 << (bit * 2));
	else
		data[num] &= ~(1 << (bit * 2));
	ret = bhy_write_parameter(client_data, BHY_PAGE_SYSTEM, param,
		data, 8);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write meta event ctrl failed");
		return -EIO;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	return count;
}

static ssize_t bhy_show_fifo_ctrl(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
			BHY_PARAM_SYSTEM_FIFO_CTRL, buf,
			BHY_FIFO_CTRL_PARAM_LEN);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read fifo ctrl failed");
		return -EIO;
	}

	return BHY_FIFO_CTRL_PARAM_LEN;
}

static ssize_t bhy_store_fifo_ctrl(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_SYSTEM,
			BHY_PARAM_SYSTEM_FIFO_CTRL, (u8 *)buf, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write fifo ctrl failed");
		return -EIO;
	}

	return count;
}

#ifdef BHY_AR_HAL_SUPPORT
static ssize_t bhy_store_activate_ar_hal(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	long req;
	struct frame_queue *qa = &client_data->data_queue_ar;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = kstrtol(buf, 10, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid request");
		return -EINVAL;
	}

	mutex_lock(&qa->lock);
	qa->frames[qa->head].handle = BHY_AR_ACTIVATE;
	if (qa->head == BHY_FRAME_SIZE_AR - 1)
		qa->head = 0;
	else
		++qa->head;
	if (qa->head == qa->tail) {
		if (qa->tail == BHY_FRAME_SIZE_AR - 1)
			qa->tail = 0;
		else
			++qa->tail;
	}
	mutex_unlock(&qa->lock);

	input_event(client_data->input_ar, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input_ar);
	PDEBUG("AR HAL activate message sent");

	return count;
}
#endif /*~ BHY_AR_HAL_SUPPORT */

static ssize_t bhy_show_reset_flag(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int reset_flag_copy;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	reset_flag_copy = atomic_read(&client_data->reset_flag);
	buf[0] = (u8)reset_flag_copy;

	return 1;
}

/* 16-bit working mode value */
static ssize_t bhy_show_working_mode(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_WORKING_MODE_ENABLE, buf, 2);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read working mode mask failed");
		return -EIO;
	}

	return BHY_FIFO_CTRL_PARAM_LEN;
}

static ssize_t bhy_store_working_mode(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_WORKING_MODE_ENABLE, (u8 *)buf, 2);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write working mode mask failed");
		return -EIO;
	}

	return count;
}

static ssize_t bhy_show_op_mode(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	u8 data[2];
	char op_mode[64];
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_OPERATING_MODE, data, 2);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read op mode failed");
		return -EIO;
	}

	switch (data[1]) {
	case 0:
		strlcpy(op_mode, "SLEEP", 64);
		break;
	case 1:
		strlcpy(op_mode, "ACCONLY", 64);
		break;
	case 2:
		strlcpy(op_mode, "GYROONLY", 64);
		break;
	case 3:
		strlcpy(op_mode, "MAGONLY", 64);
		break;
	case 4:
		strlcpy(op_mode, "ACCGYRO", 64);
		break;
	case 5:
		strlcpy(op_mode, "ACCMAG", 64);
		break;
	case 6:
		strlcpy(op_mode, "MAGGYRO", 64);
		break;
	case 7:
		strlcpy(op_mode, "AMG", 64);
		break;
	case 8:
		strlcpy(op_mode, "IMUPLUS", 64);
		break;
	case 9:
		strlcpy(op_mode, "COMPASS", 64);
		break;
	case 10:
		strlcpy(op_mode, "M4G", 64);
		break;
	case 11:
		strlcpy(op_mode, "NDOF", 64);
		break;
	case 12:
		strlcpy(op_mode, "NDOF_FMC_OFF", 64);
		break;
	case 13:
		strlcpy(op_mode, "NDOF_GEORV", 64);
		break;
	case 14:
		strlcpy(op_mode, "NDOF_GEORV_FMC_OFF", 64);
		break;
	default:
		snprintf(op_mode, 64, "Unrecoginized op mode[%d]",
				data[1]);
		break;
	}

	ret = snprintf(buf, 128, "Current op mode: %s, odr: %dHz\n",
			op_mode, data[0]);

	return ret;
}

static ssize_t bhy_show_bsx_version(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	u8 data[8];
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_BSX_VERSION, data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read BSX version failed");
		return -EIO;
	}

	ret = snprintf(buf, 128, "%d.%d.%d.%d\n",
			*(u16 *)data, *(u16 *)(data + 2),
			*(u16 *)(data + 4), *(u16 *)(data + 6));

	return ret;
}

static ssize_t bhy_show_driver_version(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = snprintf(buf, 128, "Driver version: %s\n",
			DRIVER_VERSION);

	return ret;
}

#ifdef BHY_AR_HAL_SUPPORT
static ssize_t bhy_show_fifo_frame_ar(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	struct frame_queue *qa = &client_data->data_queue_ar;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&qa->lock);
	if (qa->tail == qa->head) {
		mutex_unlock(&qa->lock);
		return 0;
	}
	memcpy(buf, &qa->frames[qa->tail], sizeof(struct fifo_frame));
	if (qa->tail == BHY_FRAME_SIZE_AR - 1)
		qa->tail = 0;
	else
		++qa->tail;
	mutex_unlock(&qa->lock);

	return sizeof(struct fifo_frame);
}
#endif /*~ BHY_AR_HAL_SUPPORT */

static ssize_t bhy_show_bmi160_foc_offset_acc(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	u8 data[3];

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bmi160_read_reg(client_data, BMI160_REG_ACC_OFFSET_X,
		data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read hw reg failed");
		return ret;
	}

	return snprintf(buf, 64, "%11d %11d %11d\n",
		*(s8 *)data, *(s8 *)(data + 1), *(s8 *)(data + 2));
}

static ssize_t bhy_store_bmi160_foc_offset_acc(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int temp[3];
	s8 data[3];
	u8 val;
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = sscanf(buf, "%11d %11d %11d", &temp[0], &temp[1], &temp[2]);
	if (ret != 3) {
		PERR("Invalid input");
		return -EINVAL;
	}
	data[0] = temp[0];
	data[1] = temp[1];
	data[2] = temp[2];
	mutex_lock(&client_data->mutex_bus_op);
	/* Write offset */
	ret = bmi160_write_reg(client_data, BMI160_REG_ACC_OFFSET_X,
		(u8 *)data, sizeof(data));
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write hw reg failed");
		return ret;
	}
	/* Write offset enable bit */
	ret = bmi160_read_reg(client_data, BMI160_REG_OFFSET_6,
		&val, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read hw reg for enable bit failed");
		return ret;
	}
	val |= BMI160_OFFSET_6_BIT_ACC_EN;
	ret = bmi160_write_reg(client_data, BMI160_REG_OFFSET_6,
		&val, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write hw reg for enable bit failed");
		return ret;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	return count;
}

static ssize_t bhy_show_bmi160_foc_offset_gyro(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	s8 data[4];
	s16 x, y, z, h;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bmi160_read_reg(client_data, BMI160_REG_GYRO_OFFSET_X,
		(u8 *)data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read hw reg failed");
		return ret;
	}

	/* left shift 6 bits to make sign bit msb, then shift back */
	h = (data[3] & BMI160_OFFSET_6_MASK_GYRO_X) >>
		BMI160_OFFSET_6_OFFSET_GYRO_X;
	x = ((h << 8) | data[0]) << 6;
	x >>= 6;
	h = (data[3] & BMI160_OFFSET_6_MASK_GYRO_Y) >>
		BMI160_OFFSET_6_OFFSET_GYRO_Y;
	y = ((h << 8) | data[1]) << 6;
	y >>= 6;
	h = (data[3] & BMI160_OFFSET_6_MASK_GYRO_Z) >>
		BMI160_OFFSET_6_OFFSET_GYRO_Z;
	z = ((h << 8) | data[2]) << 6;
	z >>= 6;

	return snprintf(buf, 64, "%11d %11d %11d\n", x, y, z);
}

static ssize_t bhy_store_bmi160_foc_offset_gyro(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int x, y, z;
	u8 data[3];
	u8 val;
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = sscanf(buf, "%11d %11d %11d", &x, &y, &z);
	if (ret != 3) {
		PERR("Invalid input");
		return -EINVAL;
	}

	data[0] = x & 0xFF;
	data[1] = y & 0xFF;
	data[2] = z & 0xFF;
	mutex_lock(&client_data->mutex_bus_op);
	/* Set low 8-bit */
	ret = bmi160_write_reg(client_data, BMI160_REG_GYRO_OFFSET_X,
		data, sizeof(data));
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write hw reg failed");
		return ret;
	}
	/* Set high bit, extract 9th bit and 10th bit from x, y, z
	* Set enable bit */
	ret = bmi160_read_reg(client_data, BMI160_REG_OFFSET_6,
		&val, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read hw reg for enable bit failed");
		return ret;
	}
	val &= ~BMI160_OFFSET_6_MASK_GYRO_X;
	val |= ((x >> 8) & 0x03) << BMI160_OFFSET_6_OFFSET_GYRO_X;
	val &= ~BMI160_OFFSET_6_MASK_GYRO_Y;
	val |= ((y >> 8) & 0x03) << BMI160_OFFSET_6_OFFSET_GYRO_Y;
	val &= ~BMI160_OFFSET_6_MASK_GYRO_Z;
	val |= ((z >> 8) & 0x03) << BMI160_OFFSET_6_OFFSET_GYRO_Z;
	val |= BMI160_OFFSET_6_BIT_GYRO_EN;
	ret = bmi160_write_reg(client_data, BMI160_REG_OFFSET_6,
		&val, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write hw reg for enable bit failed");
		return ret;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	return count;
}

static ssize_t bhy_show_bmi160_foc_conf(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int x, y, z, g;
	int out[3], in[3], i;
	const char *disp[4] = {
		"disabled",
		"1g",
		"-1g",
		"0"
	};
	u8 conf;
	ssize_t ret = 0;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	conf = client_data->bmi160_foc_conf;

	x = (conf & BMI160_FOC_CONF_MASK_ACC_X) >> BMI160_FOC_CONF_OFFSET_ACC_X;
	y = (conf & BMI160_FOC_CONF_MASK_ACC_Y) >> BMI160_FOC_CONF_OFFSET_ACC_Y;
	z = (conf & BMI160_FOC_CONF_MASK_ACC_Z) >> BMI160_FOC_CONF_OFFSET_ACC_Z;
	g = (conf & BMI160_FOC_CONF_MASK_GYRO) >> BMI160_FOC_CONF_OFFSET_GYRO;

	out[0] = x;
	out[1] = y;
	out[2] = z;
	for (i = 0; i < 3; ++i) {
		in[i] = out[0] * client_data->mapping_matrix_acc_inv[0][i] +
			out[1] * client_data->mapping_matrix_acc_inv[1][i] +
			out[2] * client_data->mapping_matrix_acc_inv[2][i];
		switch (in[i]) {
		case -1:
			in[i] = 2;
			break;
		case -2:
			in[i] = 1;
			break;
		case -3:
			in[i] = 3;
			break;
		default:
			break;
		}
	}

	ret += snprintf(buf + ret, 128, "Acc conf: %s %s %s Gyro: %s\n",
		disp[x], disp[y], disp[z], g ? "enabled" : "disabled");
	ret += snprintf(buf + ret, 128, "Original acc conf: %s %s %s\n",
		disp[in[0]], disp[in[1]], disp[in[2]]);

	return ret;
}

static ssize_t bhy_store_bmi160_foc_conf(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i;
	int mask, offset;
	u8 conf = 0;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	for (i = 0; i < count; ++i) {
		mask = 0;
		switch (buf[i]) {
		case 'x':
		case 'X':
			mask = BMI160_FOC_CONF_MASK_ACC_X;
			offset = BMI160_FOC_CONF_OFFSET_ACC_X;
			break;
		case 'y':
		case 'Y':
			mask = BMI160_FOC_CONF_MASK_ACC_Y;
			offset = BMI160_FOC_CONF_OFFSET_ACC_Y;
			break;
		case 'z':
		case 'Z':
			mask = BMI160_FOC_CONF_MASK_ACC_Z;
			offset = BMI160_FOC_CONF_OFFSET_ACC_Z;
			break;
		case 'g':
		case 'G':
			mask = BMI160_FOC_CONF_MASK_GYRO;
			offset = BMI160_FOC_CONF_OFFSET_GYRO;
			break;
		}
		if (mask == 0)
			continue;
		if (i >= count - 1)
			break;
		conf &= ~mask;
		++i;
		switch (buf[i]) {
		case 'x': /* Set to disable */
		case 'X':
			conf |= BMI160_FOC_CONF_DISABLE << offset;
			break;
		case 'g': /* set to 1g, enable for gyro */
		case 'G':
			conf |= BMI160_FOC_CONF_1G << offset;
			break;
		case 'n': /* set to -1g */
		case 'N':
			if (offset == BMI160_FOC_CONF_OFFSET_GYRO)
				break;
			conf |= BMI160_FOC_CONF_N1G << offset;
			break;
		case '0': /* set to 0 */
			if (offset == BMI160_FOC_CONF_OFFSET_GYRO)
				break;
			conf |= BMI160_FOC_CONF_0 << offset;
			break;
		}
	}
	client_data->bmi160_foc_conf = conf;

	return count;
}

static ssize_t bhy_show_bmi160_foc_exec(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64,
		"Use echo 1 > bmi160_foc_exec to begin foc\n");
}

static ssize_t bhy_store_bmi160_foc_exec(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	long req;
	int for_acc, for_gyro;
	int pmu_status_acc = 0, pmu_status_gyro = 0;
	u8 conf;
	u8 reg_data;
	int retry;
	int in[3], out[3], i;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 16, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid input");
		return -EINVAL;
	}
	conf = client_data->bmi160_foc_conf;

	/* Recalc acc conf according to real axis mapping */
	out[0] = (conf & BMI160_FOC_CONF_MASK_ACC_X) >>
		BMI160_FOC_CONF_OFFSET_ACC_X;
	out[1] = (conf & BMI160_FOC_CONF_MASK_ACC_Y) >>
		BMI160_FOC_CONF_OFFSET_ACC_Y;
	out[2] = (conf & BMI160_FOC_CONF_MASK_ACC_Z) >>
		BMI160_FOC_CONF_OFFSET_ACC_Z;
	for (i = 0; i < 3; ++i) {
		in[i] = out[0] * client_data->mapping_matrix_acc_inv[0][i] +
			out[1] * client_data->mapping_matrix_acc_inv[1][i] +
			out[2] * client_data->mapping_matrix_acc_inv[2][i];
		switch (in[i]) {
		case -1:
			in[i] = 2;
			break;
		case -2:
			in[i] = 1;
			break;
		case -3:
			in[i] = 3;
			break;
		default:
			break;
		}
	}
	conf &= ~BMI160_FOC_CONF_MASK_ACC_X;
	conf |= in[0] << BMI160_FOC_CONF_OFFSET_ACC_X;
	conf &= ~BMI160_FOC_CONF_MASK_ACC_Y;
	conf |= in[1] << BMI160_FOC_CONF_OFFSET_ACC_Y;
	conf &= ~BMI160_FOC_CONF_MASK_ACC_Z;
	conf |= in[2] << BMI160_FOC_CONF_OFFSET_ACC_Z;

	for_acc = (conf & 0x3F) ? 1 : 0;
	for_gyro = (conf & 0xC0) ? 1 : 0;
	if (for_acc == 0 && for_gyro == 0) {
		PERR("No need to do foc");
		return -EINVAL;
	}

	mutex_lock(&client_data->mutex_bus_op);
	/* Set acc range to 4g */
	if (for_acc) {
		ret = bmi160_read_reg(client_data, BMI160_REG_ACC_RANGE,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read acc range failed");
			return -EIO;
		}
		if ((reg_data & BMI160_ACC_RANGE_MASK) != BMI160_ACC_RANGE_4G) {
			reg_data = BMI160_ACC_RANGE_4G;
			ret = bmi160_write_reg(client_data,
				BMI160_REG_ACC_RANGE,
				&reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Write acc range failed");
				return -EIO;
			}
			retry = BMI160_OP_RETRY;
			do {
				ret = bmi160_read_reg(client_data,
					BMI160_REG_ACC_RANGE, &reg_data, 1);
				if (ret < 0) {
					mutex_unlock(
						&client_data->mutex_bus_op);
					PERR("Read acc range #2 failed");
					return -EIO;
				}
				if ((reg_data & BMI160_ACC_RANGE_MASK) ==
					BMI160_ACC_RANGE_4G)
					break;
				udelay(50);
			} while (--retry);
			if (retry == 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Wait for acc 4g range failed");
				return -EBUSY;
			}
		}
	}
	/* Set normal power mode */
	ret = bmi160_read_reg(client_data, BMI160_REG_PMU_STATUS,
		&reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read acc pmu status failed");
		return -EIO;
	}
	pmu_status_acc = (reg_data & BMI160_PMU_STATUS_MASK_ACC)
		>> BMI160_PMU_STATUS_OFFSET_ACC;
	pmu_status_gyro = (reg_data & BMI160_PMU_STATUS_MASK_GYRO)
		>> BMI160_PMU_STATUS_OFFSET_GYRO;
	if (for_acc && pmu_status_acc != BMI160_PMU_STATUS_NORMAL) {
		reg_data = BMI160_CMD_PMU_BASE_ACC + BMI160_PMU_STATUS_NORMAL;
		ret = bmi160_write_reg(client_data, BMI160_REG_CMD,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu cmd failed");
			return -EIO;
		}
		retry = BMI160_OP_RETRY;
		do {
			ret = bmi160_read_reg(client_data,
				BMI160_REG_PMU_STATUS, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu status #2 failed");
				return -EIO;
			}
			reg_data = (reg_data & BMI160_PMU_STATUS_MASK_ACC)
				>> BMI160_PMU_STATUS_OFFSET_ACC;
			if (reg_data == BMI160_PMU_STATUS_NORMAL)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for acc normal mode status failed");
			return -EBUSY;
		}
	}
	if (for_gyro && pmu_status_gyro != BMI160_PMU_STATUS_NORMAL) {
		reg_data = BMI160_CMD_PMU_BASE_GYRO + BMI160_PMU_STATUS_NORMAL;
		ret = bmi160_write_reg(client_data, BMI160_REG_CMD,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write gyro pmu cmd failed");
			return -EIO;
		}
		retry = BMI160_OP_RETRY;
		do {
			ret = bmi160_read_reg(client_data,
				BMI160_REG_PMU_STATUS, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read gyro pmu status #2 failed");
				return -EIO;
			}
			reg_data = (reg_data & BMI160_PMU_STATUS_MASK_GYRO)
				>> BMI160_PMU_STATUS_OFFSET_GYRO;
			if (reg_data == BMI160_PMU_STATUS_NORMAL)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for gyro normal mode status failed");
			return -EBUSY;
		}
	}
	/* Write offset enable bits */
	ret = bmi160_read_reg(client_data, BMI160_REG_OFFSET_6, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read offset config failed");
		return -EIO;
	}
	if (for_acc)
		reg_data |= BMI160_OFFSET_6_BIT_ACC_EN;
	if (for_gyro)
		reg_data |= BMI160_OFFSET_6_BIT_GYRO_EN;
	ret = bmi160_write_reg(client_data, BMI160_REG_OFFSET_6, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write offset enable failed");
		return ret;
	}
	/* Write configuration status */
	ret = bmi160_write_reg(client_data, BMI160_REG_FOC_CONF, &conf, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write configuration status failed");
		return ret;
	}
	/* Execute FOC command */
	reg_data = BMI160_CMD_START_FOC;
	ret = bmi160_write_reg(client_data, BMI160_REG_CMD, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Execute FOC failed");
		return ret;
	}
	reg_data = 0;
	retry = BMI160_OP_RETRY;
	do {
		ret = bmi160_read_reg(client_data, BMI160_REG_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read status after exec FOC failed");
			return ret;
		}
		if (reg_data & BMI160_STATUS_BIT_FOC_RDY)
			break;
		usleep_range(2000, 2200);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Cannot read the right status after exec FOC");
		return -EBUSY;
	}
	/* Restore old power mode */
	if (for_acc && pmu_status_acc != BMI160_PMU_STATUS_NORMAL) {
		reg_data = BMI160_CMD_PMU_BASE_ACC
			+ pmu_status_acc;
		ret = bmi160_write_reg(client_data, BMI160_REG_CMD,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu cmd #2 failed");
			return -EIO;
		}
		retry = BMI160_OP_RETRY;
		do {
			ret = bmi160_read_reg(client_data,
				BMI160_REG_PMU_STATUS, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu status #2 failed");
				return -EIO;
			}
			reg_data = (reg_data & BMI160_PMU_STATUS_MASK_ACC)
				>> BMI160_PMU_STATUS_OFFSET_ACC;
			if (reg_data == pmu_status_acc)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for acc normal mode status #2 failed");
			return -EBUSY;
		}
	}
	if (for_gyro && pmu_status_gyro != BMI160_PMU_STATUS_NORMAL) {
		reg_data = BMI160_CMD_PMU_BASE_GYRO
			+ pmu_status_gyro;
		ret = bmi160_write_reg(client_data, BMI160_REG_CMD,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write gyro pmu cmd #2 failed");
			return -EIO;
		}
		retry = BMI160_OP_RETRY;
		do {
			ret = bmi160_read_reg(client_data,
				BMI160_REG_PMU_STATUS, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read gyro pmu status #2 failed");
				return -EIO;
			}
			reg_data = (reg_data & BMI160_PMU_STATUS_MASK_GYRO)
				>> BMI160_PMU_STATUS_OFFSET_GYRO;
			if (reg_data == pmu_status_gyro)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for gyro normal mode status #2 failed");
			return -EBUSY;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);
	/* Reset foc conf*/
	client_data->bmi160_foc_conf = 0;

	PINFO("FOC executed successfully");

	return count;
}

static ssize_t bhy_show_bmi160_foc_save_to_nvm(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64,
		"Use echo 1 > bmi160_foc_save_to_nvm to save to nvm\n");
}

static ssize_t bhy_store_bmi160_foc_save_to_nvm(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	long req;
	u8 reg_data;
	int retry;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 16, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid input");
		return -EINVAL;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bmi160_read_reg(client_data, BMI160_REG_CONF, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read conf failed");
		return ret;
	}
	reg_data |= BMI160_CONF_BIT_NVM;
	ret = bmi160_write_reg(client_data, BMI160_REG_CONF, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Enable NVM writing failed");
		return ret;
	}
	reg_data = BMI160_CMD_PROG_NVM;
	ret = bmi160_write_reg(client_data, BMI160_REG_CMD, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Execute NVM prog failed");
		return ret;
	}
	reg_data = 0;
	retry = BMI160_OP_RETRY;
	do {
		ret = bmi160_read_reg(client_data, BMI160_REG_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read status after exec FOC failed");
			return ret;
		}
		if (reg_data & BMI160_STATUS_BIT_NVM_RDY)
			break;
		usleep_range(2000, 2200);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Cannot read the right status after write to NVM");
		return -EBUSY;
	}
	ret = bmi160_read_reg(client_data, BMI160_REG_CONF, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read conf after exec nvm prog failed");
		return ret;
	}
	reg_data &= ~BMI160_CONF_BIT_NVM;
	ret = bmi160_write_reg(client_data, BMI160_REG_CONF, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Disable NVM writing failed");
		return ret;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	PINFO("NVM successfully written");

	return count;
}

static ssize_t bhy_show_bma2x2_foc_offset(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	s8 data[3];

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_read_reg(client_data, BMA2X2_REG_OFC_OFFSET_X,
		(u8 *)data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read hw reg failed");
		return ret;
	}

	return snprintf(buf, 64, "%11d %11d %11d\n", data[0], data[1], data[2]);
}

static ssize_t bhy_store_bma2x2_foc_offset(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int x, y, z;
	s8 data[3];
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = sscanf(buf, "%11d %11d %11d", &x, &y, &z);
	if (ret != 3) {
		PERR("Invalid input");
		return -EINVAL;
	}
	data[0] = x & 0xFF;
	data[1] = y & 0xFF;
	data[2] = z & 0xFF;
	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_write_reg(client_data, BMA2X2_REG_OFC_OFFSET_X,
		(u8 *)data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write hw reg failed");
		return ret;
	}

	return count;
}

static ssize_t bhy_show_bma2x2_foc_conf(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int x, y, z;
	int out[3], in[3], i;
	const char *disp[4] = {
		"disabled",
		"1g",
		"-1g",
		"0"
	};
	u8 conf;
	ssize_t ret = 0;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	conf = client_data->bma2x2_foc_conf;

	x = (conf & BMA2X2_OFC_CONF_MASK_X) >> BMA2X2_OFC_CONF_OFFSET_X;
	y = (conf & BMA2X2_OFC_CONF_MASK_Y) >> BMA2X2_OFC_CONF_OFFSET_Y;
	z = (conf & BMA2X2_OFC_CONF_MASK_Z) >> BMA2X2_OFC_CONF_OFFSET_Z;

	out[0] = x;
	out[1] = y;
	out[2] = z;
	for (i = 0; i < 3; ++i) {
		in[i] = out[0] * client_data->mapping_matrix_acc_inv[0][i] +
			out[1] * client_data->mapping_matrix_acc_inv[1][i] +
			out[2] * client_data->mapping_matrix_acc_inv[2][i];
		switch (in[i]) {
		case -1:
			in[i] = 2;
			break;
		case -2:
			in[i] = 1;
			break;
		case -3:
			in[i] = 3;
			break;
		default:
			break;
		}
	}

	ret += snprintf(buf + ret, 128, "Acc conf: %s %s %s\n",
		disp[x], disp[y], disp[z]);
	ret += snprintf(buf + ret, 128, "Original acc conf: %s %s %s\n",
		disp[in[0]], disp[in[1]], disp[in[2]]);

	return ret;
}

static ssize_t bhy_store_bma2x2_foc_conf(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i;
	int mask, offset;
	u8 conf = 0;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	for (i = 0; i < count; ++i) {
		mask = 0;
		switch (buf[i]) {
		case 'x':
		case 'X':
			mask = BMA2X2_OFC_CONF_MASK_X;
			offset = BMA2X2_OFC_CONF_OFFSET_X;
			break;
		case 'y':
		case 'Y':
			mask = BMA2X2_OFC_CONF_MASK_Y;
			offset = BMA2X2_OFC_CONF_OFFSET_Y;
			break;
		case 'z':
		case 'Z':
			mask = BMA2X2_OFC_CONF_MASK_Z;
			offset = BMA2X2_OFC_CONF_OFFSET_Z;
			break;
		}
		if (mask == 0)
			continue;
		if (i >= count - 1)
			break;
		conf &= ~mask;
		++i;
		switch (buf[i]) {
		case 'x': /* Set to disable */
		case 'X':
			conf |= BMA2X2_OFC_CONF_DISABLE << offset;
			break;
		case 'g': /* set to 1g, enable for gyro */
		case 'G':
			conf |= BMA2X2_OFC_CONF_1G << offset;
			break;
		case 'n': /* set to -1g */
		case 'N':
			conf |= BMA2X2_OFC_CONF_N1G << offset;
			break;
		case '0': /* set to 0 */
			conf |= BMA2X2_OFC_CONF_0 << offset;
			break;
		}
	}
	client_data->bma2x2_foc_conf = conf;

	return count;
}

static ssize_t bhy_show_bma2x2_foc_exec(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64,
		"Use echo 1 > bma2x2_foc_exec to begin foc\n");
}

static ssize_t bhy_store_bma2x2_foc_exec(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	long req;
	u8 pmu_status_old;
	u8 conf;
	u8 reg_data;
	int retry;
	int in[3], out[3], i;
	u8 trigger_axis[3] = {
		BMA2X2_CAL_TRIGGER_X,
		BMA2X2_CAL_TRIGGER_Y,
		BMA2X2_CAL_TRIGGER_Z
	};

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 16, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid input");
		return -EINVAL;
	}
	conf = client_data->bma2x2_foc_conf;

	/* Recalc acc conf according to real axis mapping */
	out[0] = (conf & BMA2X2_OFC_CONF_MASK_X) >>
		BMA2X2_OFC_CONF_OFFSET_X;
	out[1] = (conf & BMA2X2_OFC_CONF_MASK_Y) >>
		BMA2X2_OFC_CONF_OFFSET_Y;
	out[2] = (conf & BMA2X2_OFC_CONF_MASK_Z) >>
		BMA2X2_OFC_CONF_OFFSET_Z;
	for (i = 0; i < 3; ++i) {
		in[i] = out[0] * client_data->mapping_matrix_acc_inv[0][i] +
			out[1] * client_data->mapping_matrix_acc_inv[1][i] +
			out[2] * client_data->mapping_matrix_acc_inv[2][i];
		switch (in[i]) {
		case -1:
			in[i] = 2;
			break;
		case -2:
			in[i] = 1;
			break;
		case -3:
			in[i] = 3;
			break;
		default:
			break;
		}
	}
	conf &= ~BMA2X2_OFC_CONF_MASK_X;
	conf |= in[0] << BMA2X2_OFC_CONF_OFFSET_X;
	conf &= ~BMA2X2_OFC_CONF_MASK_Y;
	conf |= in[1] << BMA2X2_OFC_CONF_OFFSET_Y;
	conf &= ~BMA2X2_OFC_CONF_MASK_Z;
	conf |= in[2] << BMA2X2_OFC_CONF_OFFSET_Z;

	/* Set 2g range */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_RANGE,
		&reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read acc pmu range failed");
		return -EIO;
	}
	if (reg_data != BMA2X2_PMU_RANGE_2G) {
		reg_data = BMA2X2_PMU_RANGE_2G;
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_PMU_RANGE,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu range failed");
			return -EIO;
		}
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_RANGE,
				&reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu range failed");
				return -EIO;
			}
			if (reg_data == BMA2X2_PMU_RANGE_2G)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for 4g range failed");
			return -EBUSY;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	/* Set BW to 62.5Hz if it is greater than 125Hz */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_BW,
		&reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read acc pmu range failed");
		return -EIO;
	}
	if (reg_data > BMA2X2_PMU_BW_125) {
		reg_data = BMA2X2_PMU_BW_62_5;
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_PMU_BW,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu bw failed");
			return -EIO;
		}
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_BW,
				&reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu range failed");
				return -EIO;
			}
			if (reg_data == BMA2X2_PMU_BW_62_5)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for 62.5Hz BW failed");
			return -EBUSY;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	/* Set normal power mode */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_LPW,
		&reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read acc pmu status failed");
		return -EIO;
	}
	pmu_status_old = reg_data;
	reg_data &= BMA2X2_PMU_CONF_MASK;
	if (reg_data != BMA2X2_PMU_CONF_NORMAL) {
		reg_data = BMA2X2_PMU_CONF_NORMAL;
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_PMU_LPW,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu cmd failed");
			return -EIO;
		}
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data,
				BMA2X2_REG_PMU_LPW, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu status #2 failed");
				return -EIO;
			}
			reg_data &= BMA2X2_PMU_CONF_MASK;
			if (reg_data == BMA2X2_PMU_CONF_NORMAL)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for acc normal mode status failed");
			return -EBUSY;
		}
	}
	/* Write configuration status */
	ret = bma2x2_write_reg(client_data, BMA2X2_REG_OFC_SETTING, &conf, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write configuration status failed");
		return ret;
	}
	/* Execute FOC command */
	ret = bma2x2_read_reg(client_data,
		BMA2X2_REG_OFC_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read ofc_ctrl failed");
		return -EIO;
	}
	if ((reg_data & BMA2X2_CAL_RDY_MASK) == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("OFC cal rdy status error!");
		return -EIO;
	}
	for (i = 0; i < 3; ++i) {
		if (in[i] == 0) /* disabled */
			continue;
		reg_data = trigger_axis[i];
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_OFC_CTRL,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Execute FOC failed");
			return ret;
		}
		reg_data = 0;
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data,
				BMA2X2_REG_OFC_CTRL, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read ofc_ctrl failed");
				return -EIO;
			}
			if (reg_data & BMA2X2_CAL_RDY_MASK)
				break;
			usleep_range(2000, 2200);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Cannot read the right status after exec FOC");
			return -EBUSY;
		}
	}
	/* Restore old power mode */
	reg_data = pmu_status_old;
	reg_data &= BMA2X2_PMU_CONF_MASK;
	if (reg_data != BMA2X2_PMU_CONF_NORMAL) {
		reg_data = pmu_status_old;
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_PMU_LPW,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu cmd #2 failed");
			return -EIO;
		}
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data,
				BMA2X2_REG_PMU_LPW, &reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu status #2 failed");
				return -EIO;
			}
			if (reg_data == pmu_status_old)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for acc normal mode status #2 failed");
			return -EBUSY;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	/* Restore 4g range */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_RANGE,
		&reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read acc pmu range failed");
		return -EIO;
	}
	if (reg_data != BMA2X2_PMU_RANGE_4G) {
		reg_data = BMA2X2_PMU_RANGE_4G;
		ret = bma2x2_write_reg(client_data, BMA2X2_REG_PMU_RANGE,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write acc pmu range failed");
			return -EIO;
		}
		retry = BMA2X2_OP_RETRY;
		do {
			ret = bma2x2_read_reg(client_data, BMA2X2_REG_PMU_RANGE,
				&reg_data, 1);
			if (ret < 0) {
				mutex_unlock(&client_data->mutex_bus_op);
				PERR("Read acc pmu range failed");
				return -EIO;
			}
			if (reg_data == BMA2X2_PMU_RANGE_4G)
				break;
			udelay(50);
		} while (--retry);
		if (retry == 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Wait for 4g range failed");
			return -EBUSY;
		}
	}
	mutex_unlock(&client_data->mutex_bus_op);

	/* Reset foc conf*/
	client_data->bma2x2_foc_conf = 0;

	PINFO("FOC executed successfully");

	return count;
}

static ssize_t bhy_show_self_test(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64,
		"Use echo 1 > self_test to do self-test\n");
}

static ssize_t bhy_store_self_test(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	long req;
	u8 reg_data;
	int retry;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 16, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid input");
		return -EINVAL;
	}

	atomic_set(&client_data->reset_flag, RESET_FLAG_SELF_TEST);

	mutex_lock(&client_data->mutex_bus_op);
	/* Make algorithm standby */
	/*ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read algorithm standby reg failed");
		return -EIO;
	}
	reg_data |= HOST_CTRL_MASK_ALGORITHM_STANDBY;
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write algorithm standby reg failed");
		return -EIO;
	}
	retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_HOST_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read host status failed");
			return -EIO;
		}
		if (reg_data & BHY_HOST_STATUS_MASK_ALGO_STANDBY)
			break;
		usleep_range(10000, 10000);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Algo standby does not take effect");
		return -EBUSY;
	}*/
	/* Disable CPU run from chip control */
	reg_data = 0;
	ret = bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip control reg failed");
		return -EIO;
	}
	retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		usleep_range(10000, 11000);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Wait for chip idle status timed out");
		return -EBUSY;
	}
	/* Write self test bit */
	ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read host ctrl reg failed");
		return -EIO;
	}
	reg_data |= HOST_CTRL_MASK_SELF_TEST_REQ;
	/*reg_data &= ~HOST_CTRL_MASK_ALGORITHM_STANDBY;*/
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write host ctrl reg failed");
		return -EIO;
	}
	/* Enable CPU run from chip control */
	reg_data = 1;
	ret = bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip control reg failed");
		return -EIO;
	}
	retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (!(reg_data & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE))
			break;
		usleep_range(10000, 11000);
	} while (--retry);
	if (retry == 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Wait for chip running status timed out");
		return -EBUSY;
	}
	/*retry = 1000;
	do {
		ret = bhy_read_reg(client_data, BHY_REG_HOST_STATUS,
			&reg_data, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read host status failed");
			return -EIO;
		}
		if (!(reg_data & BHY_HOST_STATUS_MASK_ALGO_STANDBY))
			break;
		usleep_range(10000, 10000);
	} while (--retry);*/
	/* clear self test bit */
	/*ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read host ctrl reg failed #2");
		return -EIO;
	}
	reg_data &= ~HOST_CTRL_MASK_SELF_TEST_REQ;
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &reg_data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write host ctrl reg failed #2");
		return -EIO;
	}*/
	mutex_unlock(&client_data->mutex_bus_op);

	return count;
}

static ssize_t bhy_show_self_test_result(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i, handle, count;
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret = 0;
	count = 0;
	for (i = 0; i < SELF_TEST_RESULT_COUNT; ++i) {
		if (client_data->self_test_result[i] != -1) {
			switch (i) {
			case SELF_TEST_RESULT_INDEX_ACC:
				handle = BHY_PHYS_HANDLE_ACC;
				break;
			case SELF_TEST_RESULT_INDEX_MAG:
				handle = BHY_PHYS_HANDLE_MAG;
				break;
			case SELF_TEST_RESULT_INDEX_GYRO:
				handle = BHY_PHYS_HANDLE_GYRO;
				break;
			}
			ret += snprintf(buf + ret, 64,
				"Result for sensor[%d]: %d\n",
				handle, client_data->self_test_result[i]);
			++count;
		}
	}
	ret += snprintf(buf + ret, 64, "Totally %d sensor(s) tested.\n", count);
	ret += snprintf(buf + ret, 128, "Diff value is %d, %d, %d.\n",
			client_data->acc_self_test_diff[0],
			client_data->acc_self_test_diff[1],
			client_data->acc_self_test_diff[2]);

	return ret;
}

static ssize_t bhy_store_update_device_info(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	int i;
	u8 id[4];

	/* Set device type */
	for (i = 0; i < sizeof(client_data->dev_type) - 1 && buf[i]; ++i)
		client_data->dev_type[i] = buf[i];
	client_data->dev_type[i] = '\0';
	/* Set rom & ram ID */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_ROM_VERSION_0, id, 4);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read rom id failed");
		return -EIO;
	}
	client_data->rom_id = *(u16 *)id;
	client_data->ram_id = *((u16 *)id + 1);

	return count;
}

static ssize_t bhy_show_mapping_matrix_acc(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i, j;
	ssize_t ret = 0;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	ret += snprintf(buf + ret, 64, "Matrix:\n");
	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			ret += snprintf(buf + ret, 16, "%d ",
			client_data->mapping_matrix_acc[i][j]);
		buf[ret++] = '\n';
	}

	ret += snprintf(buf + ret, 64, "Inverse:\n");
	for (i = 0; i < 3; ++i) {
		for (j = 0; j < 3; ++j)
			ret += snprintf(buf + ret, 16, "%d ",
			client_data->mapping_matrix_acc_inv[i][j]);
		buf[ret++] = '\n';
	}
	buf[ret++] = '\0';

	return ret;
}

static ssize_t bhy_store_mapping_matrix_acc(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	long req;
	u8 data[16];
	int i, j, k;
	s8 m[3][6], tmp;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 16, &req);
	if (ret < 0 || req != 1) {
		PERR("Invalid input");
		return -EINVAL;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
		BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_ACC,
		data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read param failed");
		return ret;
	}
	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j) {
			k = i * 3 + j;
			client_data->mapping_matrix_acc[i][j] =
				k % 2 == 0 ? data[11 + k / 2] & 0xF :
				data[11 + k / 2] >> 4;
			if (client_data->mapping_matrix_acc[i][j] == 0xF)
				client_data->mapping_matrix_acc[i][j] = -1;
		}

	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j) {
			m[i][j] = client_data->mapping_matrix_acc[i][j];
			m[i][j + 3] = i == j ? 1 : 0;
		}
	for (i = 0; i < 3; ++i) {
		if (m[i][i] == 0) {
			for (j = i + 1; j < 3; ++j) {
				if (m[j][i]) {
					for (k = 0; k < 6; ++k) {
						tmp = m[j][k];
						m[j][k] = m[i][k];
						m[i][k] = tmp;
					}
					break;
				}
			}
			if (j >= 3) { /* Fail check */
				PERR("Matrix invalid");
				break;
			}
		}
		if (m[i][i] < 0) {
			for (j = 0; j < 6; ++j)
				m[i][j] = -m[i][j];
		}
	}

	for (i = 0; i < 3; ++i)
		for (j = 0; j < 3; ++j)
			client_data->mapping_matrix_acc_inv[i][j] = m[i][j + 3];

	return count;
}

static ssize_t bhy_store_sensor_data_size(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int handle;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	handle = (u8)buf[0];
	if (handle >= 255) {
		PERR("Invalid handle");
		return -EIO;
	}
	client_data->sensor_data_len[handle] = (s8)buf[1];

	return count;
}

static ssize_t bhy_show_mapping_matrix(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i, j, k, ii;
	int ret;
	ssize_t len = 0;
	u8 data[16];
	u8 map[32];
	u8 handle[3] = {
		BHY_SENSOR_HANDLE_ACCELEROMETER,
		BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED,
		BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED,
	};
	char *name[3] = {
		"Accelerometer",
		"Magnetometer",
		"Gyroscope"
	};
	u8 param;
	s8 mapping[3][3];

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	/* Check sensor existance */
	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
		BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_PRESENT,
		data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read param failed");
		return ret;
	}
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 8; ++j) {
			if (data[i] & (1 << j))
				map[i * 8 + j] = 1;
			else
				map[i * 8 + j] = 0;
		}
	}

	/* Get orientation matrix */
	for (ii = 0; ii < 3; ++ii) {
		if (!map[handle[ii]])
			continue;
		param = BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0 + handle[ii];
		mutex_lock(&client_data->mutex_bus_op);
		ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
			param, data, sizeof(data));
		mutex_unlock(&client_data->mutex_bus_op);
		if (ret < 0) {
			PERR("Read param failed #2");
			return ret;
		}
		for (i = 0; i < 3; ++i)
			for (j = 0; j < 3; ++j) {
				k = i * 3 + j;
				mapping[i][j] =
					k % 2 == 0 ? data[11 + k / 2] & 0xF :
					data[11 + k / 2] >> 4;
				if (mapping[i][j] == 0xF)
					mapping[i][j] = -1;
			}
		len += snprintf(buf + len, 128, "Matrix for %s:\n", name[ii]);
		for (i = 0; i < 3; ++i) {
			for (j = 0; j < 3; ++j)
				len += snprintf(buf + len, 16, "%d ",
					mapping[i][j]);
			buf[len++] = '\n';
		}
		buf[len++] = '\n';
		buf[len++] = '\0';
	}

	return len;
}

static ssize_t bhy_store_mapping_matrix(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int i;
	int handle;
	int matrix[9];
	u8 data[8];
	int ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = sscanf(buf, "%11d %11d %11d %11d %11d %11d %11d %11d %11d %11d",
		&handle, &matrix[0], &matrix[1], &matrix[2], &matrix[3],
		&matrix[4], &matrix[5], &matrix[6], &matrix[7], &matrix[8]);
	if (ret != 10) {
		PERR("Invalid input for matrix");
		return -EINVAL;
	}
	if (handle != BHY_SENSOR_HANDLE_ACCELEROMETER &&
		handle != BHY_SENSOR_HANDLE_MAGNETIC_FIELD_UNCALIBRATED &&
		handle != BHY_SENSOR_HANDLE_GYROSCOPE_UNCALIBRATED) {
		PERR("Invalid sensor handle: %d", handle);
		return -EINVAL;
	}

	for (i = 0; i < 5; ++i) {
		switch (matrix[2 * i]) {
		case 0:
			data[i] = 0;
			break;
		case 1:
			data[i] = 1;
			break;
		case -1:
			data[i] = 0xF;
			break;
		default:
			PERR("Invalid matrix data: %d", matrix[2 * i]);
			return -EINVAL;
		}
		if (i == 4)
			break;
		switch (matrix[2 * i + 1]) {
		case 0:
			break;
		case 1:
			data[i] |= 0x10;
			break;
		case -1:
			data[i] |= 0xF0;
			break;
		default:
			PERR("Invalid matrix data: %d", matrix[2 * i + 1]);
			return -EINVAL;
		}
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_SYSTEM,
		BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0 + handle,
		data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write mapping matrix failed");
		return ret;
	}

	return count;
}

static ssize_t bhy_show_custom_version(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int ret;
	ssize_t len = 0;
	u8 data[16];
	int custom_version;
	int year, month, day, hour, minute, second;
	__le16 reg_data;
	int rom_id, ram_id;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_ROM_VERSION_0,
		(u8 *)&reg_data, 2);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read rom version failed");
		return ret;
	}
	rom_id = (u16)__le16_to_cpu(reg_data);
	len += snprintf(buf + len, 64, "Rom version: %d\n", rom_id);

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_RAM_VERSION_0,
		(u8 *)&reg_data, 2);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read ram version failed");
		return ret;
	}
	ram_id = (u16)__le16_to_cpu(reg_data);
	len += snprintf(buf + len, 64, "Ram version: %d\n", ram_id);

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM,
		BHY_PARAM_SYSTEM_CUSTOM_VERSION, data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read custom version failed");
		return ret;
	}

	reg_data = *(__le16 *)data;
	custom_version = (u16)__le16_to_cpu(reg_data);
	reg_data = *(__le16 *)(data + 2);
	year = (u16)__le16_to_cpu(reg_data);
	month = *(u8 *)(data + 4);
	day = *(u8 *)(data + 5);
	hour = *(u8 *)(data + 6);
	minute = *(u8 *)(data + 7);
	second = *(u8 *)(data + 8);

	len += snprintf(buf + len, 64, "Custom version: %d\n", custom_version);
	len += snprintf(buf + len, 64,
		"Build date: %04d-%02d-%02d %02d:%02d:%02d\n",
		year, month, day, hour, minute, second);

	return len;
}

static ssize_t bhy_store_req_fw(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 u8_val;
	u16 u16_val;
	u32 u32_val;
	int retry = BHY_RESET_WAIT_RETRY;
	int reset_flag_copy;
	struct ram_patch_header header;
	ssize_t read_len;
	char data_buf[64]; /* Must be less than burst write max buf */
	u16 remain;
	int i;
	const struct firmware *fw;
	ssize_t pos;

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[0]);
#endif /*~ BHY_DEBUG */

	mutex_lock(&client_data->mutex_bus_op);

	/* Reset FPGA */
	atomic_set(&client_data->reset_flag, RESET_FLAG_TODO);
	u8_val = 1;
	ret = bhy_write_reg(client_data, BHY_REG_RESET_REQ, &u8_val,
		sizeof(u8));
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write reset reg failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return ret;
	}
	while (retry--) {
		reset_flag_copy = atomic_read(&client_data->reset_flag);
		if (reset_flag_copy == RESET_FLAG_READY)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Reset ready status wait failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}
	PINFO("BHy reset successfully");

	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after reset");
		return -EIO;
	}

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[1]);
#endif /*~ BHY_DEBUG */

	/* Init upload addr */
	u16_val = 0;
	if (bhy_write_reg(client_data, BHY_REG_UPLOAD_ADDR_0,
		(u8 *)&u16_val, 2) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Init upload addr failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}

	/* Write upload request */
	u8_val = 2;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Set chip ctrl failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}

	/* Request firmware data */
	ret = request_firmware(&fw, "ram_patch.fw", dev);
	if (ret < 0) {
		PERR("Request firmware failed");
		return -EIO;
	}
	pos = 0;

	/* PDEBUG("Firmware size is %d", fw->size); */

	/* Upload data */
	if (fw->size < sizeof(header)) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("firmware size error");
	}
	memcpy(&header, fw->data, sizeof(header));
	pos += sizeof(header);
	remain = header.data_length;
	if (remain % 4 != 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("data length cannot be divided by 4");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EINVAL;
	}
	if (fw->size < sizeof(header) + remain) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("firmware size error");
	}
	while (remain > 0) {
		read_len = remain > sizeof(data_buf) ? sizeof(data_buf) : remain;
		memcpy(data_buf, fw->data + pos, read_len);
		pos += read_len;
		for (i = 0; i < read_len; i += 4)
			*(u32 *)(data_buf + i) = swab32(*(u32 *)(data_buf + i));
		if (bhy_write_reg(client_data, BHY_REG_UPLOAD_DATA,
			(u8 *)data_buf, read_len) < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Write ram patch data failed");
			atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
			return -EIO;
		}
		remain -= read_len;
	}

	/* Release firmware */
	release_firmware(fw);

	/* Check CRC */
	if (bhy_read_reg(client_data, BHY_REG_DATA_CRC_0,
		(u8 *)&u32_val, 4) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read CRC failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}
	if (u32_val != header.crc) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("CRC mismatch 0X%08X vs 0X%08X", u32_val, header.crc);
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}

	/* Disable upload mode */
	u8_val = 0;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip ctrl reg failed");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}
	udelay(50);

	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE)
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after upload patch");
		return -EIO;
	}

#ifdef BHY_DEBUG
	bhy_get_ap_timestamp(&g_ts[2]);
#endif /*~ BHY_DEBUG */

	/* Enable cpu run */
	u8_val = 1;
	if (bhy_write_reg(client_data, BHY_REG_CHIP_CTRL, &u8_val, 1) < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write chip ctrl reg failed #2");
		atomic_set(&client_data->reset_flag, RESET_FLAG_ERROR);
		return -EIO;
	}

	/* Check chip status */
	retry = 1000;
	while (retry--) {
		ret = bhy_read_reg(client_data, BHY_REG_CHIP_STATUS,
			&u8_val, 1);
		if (ret < 0) {
			mutex_unlock(&client_data->mutex_bus_op);
			PERR("Read chip status failed");
			return -EIO;
		}
		if (!(u8_val & BHY_CHIP_STATUS_BIT_FIRMWARE_IDLE))
			break;
		udelay(50);
	}
	if (retry <= 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Chip status error after CPU run request");
		return -EIO;
	}

	mutex_unlock(&client_data->mutex_bus_op);
	PINFO("Ram patch loaded successfully.");

	return count;
}

#ifdef BHY_DEBUG

static ssize_t bhy_show_reg_sel(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64, "reg=0X%02X, len=%d\n",
		client_data->reg_sel, client_data->reg_len);
}

static ssize_t bhy_store_reg_sel(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = sscanf(buf, "%11X %11d",
		&client_data->reg_sel, &client_data->reg_len);
	if (ret != 2) {
		PERR("Invalid argument");
		return -EINVAL;
	}

	return count;
}

static ssize_t bhy_show_reg_val(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 reg_data[128], i;
	int pos;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, client_data->reg_sel,
		reg_data, client_data->reg_len);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Reg op failed");
		return ret;
	}

	pos = 0;
	for (i = 0; i < client_data->reg_len; ++i) {
		pos += snprintf(buf + pos, 16, "%02X", reg_data[i]);
		buf[pos++] = (i + 1) % 16 == 0 ? '\n' : ' ';
	}
	if (buf[pos - 1] == ' ')
		buf[pos - 1] = '\n';

	return pos;
}

static ssize_t bhy_store_reg_val(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 reg_data[32] = { 0, };
	int i, j, status, digit;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	status = 0;
	for (i = j = 0; i < count && j < client_data->reg_len; ++i) {
		if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t' ||
			buf[i] == '\r') {
			status = 0;
			++j;
			continue;
		}
		digit = buf[i] & 0x10 ? (buf[i] & 0xF) : ((buf[i] & 0xF) + 9);
		PDEBUG("digit is %d", digit);
		switch (status) {
		case 2:
			++j; /* Fall thru */
		case 0:
			reg_data[j] = digit;
			status = 1;
			break;
		case 1:
			reg_data[j] = reg_data[j] * 16 + digit;
			status = 2;
			break;
		}
	}
	if (status > 0)
		++j;
	if (j > client_data->reg_len)
		j = client_data->reg_len;
	else if (j < client_data->reg_len) {
		PERR("Invalid argument");
		return -EINVAL;
	}
	PDEBUG("Reg data read as");
	for (i = 0; i < j; ++i)
		PDEBUG("%d", reg_data[i]);

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_reg(client_data, client_data->reg_sel,
		reg_data, client_data->reg_len);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Reg op failed");
		return ret;
	}

	return count;
}

static ssize_t bhy_show_param_sel(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64, "Page=%d, param=%d\n",
		client_data->page_sel, client_data->param_sel);
}

static ssize_t bhy_store_param_sel(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = sscanf(buf, "%11d %11d",
		&client_data->page_sel, &client_data->param_sel);
	if (ret != 2) {
		PERR("Invalid argument");
		return -EINVAL;
	}

	return count;
}

static ssize_t bhy_show_param_val(struct device *dev
		, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 data[16];
	int pos, i;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_parameter(client_data, client_data->page_sel,
		client_data->param_sel, data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Read param failed");
		return ret;
	}

	pos = 0;
	for (i = 0; i < 16; ++i) {
		pos += snprintf(buf + pos, 16, "%02X", data[i]);
		buf[pos++] = ' ';
	}
	if (buf[pos - 1] == ' ')
		buf[pos - 1] = '\n';

	return pos;
}

static ssize_t bhy_store_param_val(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 data[8];
	int i, j, status, digit;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	status = 0;
	for (i = j = 0; i < count && j < 8; ++i) {
		if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t' ||
			buf[i] == '\r') {
			status = 0;
			++j;
			continue;
		}
		digit = buf[i] & 0x10 ? (buf[i] & 0xF) : ((buf[i] & 0xF) + 9);
		switch (status) {
		case 2:
			++j; /* Fall thru */
		case 0:
			data[j] = digit;
			status = 1;
			break;
		case 1:
			data[j] = data[j] * 16 + digit;
			status = 2;
			break;
		}
	}
	if (status > 0)
		++j;
	if (j == 0) {
		PERR("Invalid argument");
		return -EINVAL;
	} else if (j > 8)
		j = 8;
	/* Alway write 8 bytes, the bytes is 0 if not provided*/
	for (i = j; i < 8; ++i)
		data[i] = 0;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, client_data->page_sel,
		client_data->param_sel, data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write param failed");
		return ret;
	}

	return count;
}

static ssize_t bhy_store_log_raw_data(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	long req;
	u8 param_data[8];
	struct frame_queue *q;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 10, &req);
	if (ret < 0) {
		PERR("Invalid request");
		return -EINVAL;
	}
	q = &client_data->data_queue;

	memset(param_data, 0, sizeof(param_data));
	if (req)
		param_data[0] = param_data[1] = param_data[2] = 1;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_VIRTUAL_BSX_ENABLE, param_data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write raw data cfg failed");
		return ret;
	}

	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_DATA_LOG_TYPE;
	q->frames[q->head].data[0] = BHY_DATA_LOG_TYPE_RAW;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

	return count;
}

static ssize_t bhy_store_log_input_data_gesture(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	long req;
	u8 param_data[8];
	struct frame_queue *q;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 10, &req);
	if (ret < 0) {
		PERR("Invalid request");
		return -EINVAL;
	}
	q = &client_data->data_queue;

	memset(param_data, 0, sizeof(param_data));
	if (req)
		param_data[3] = param_data[4] = param_data[5] = 1;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_VIRTUAL_BSX_ENABLE, param_data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write raw data cfg failed");
		return ret;
	}

	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_DATA_LOG_TYPE;
	q->frames[q->head].data[0] = BHY_DATA_LOG_TYPE_INPUT_GESTURE;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

	return count;
}

static ssize_t bhy_store_log_input_data_tilt_ar(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	long req;
	u8 param_data[8];
	struct frame_queue *q;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 10, &req);
	if (ret < 0) {
		PERR("Invalid request");
		return -EINVAL;
	}
	q = &client_data->data_queue;

	memset(param_data, 0, sizeof(param_data));
	if (req)
		param_data[6] = param_data[7] = 1;

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_ALGORITHM,
			BHY_PARAM_VIRTUAL_BSX_ENABLE, param_data, 8);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Write raw data cfg failed");
		return ret;
	}

	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_DATA_LOG_TYPE;
	q->frames[q->head].data[0] = BHY_DATA_LOG_TYPE_INPUT_TILT_AR;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

	return count;
}

static ssize_t bhy_store_log_fusion_data(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	long req;
	struct frame_queue *q;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtol(buf, 10, &req);
	if (ret < 0) {
		PERR("Invalid request");
		return -EINVAL;
	}
	q = &client_data->data_queue;

	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_LOG_FUSION_DATA;
	q->frames[q->head].data[0] = req ? BHY_FUSION_DATA_LOG_ENABLE :
		BHY_FUSION_DATA_LOG_NONE;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);

	return count;
}

static ssize_t bhy_store_enable_pass_thru(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 u8_val;
	int enable;
	int retry;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtoint(buf, 10, &enable);
	if (ret < 0) {
		PERR("invalid input");
		return ret;
	}

	mutex_lock(&client_data->mutex_bus_op);

	if (enable) {
		/* Make algorithm standby */
		ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &u8_val, 1);
		if (ret < 0) {
			PERR("Read algorithm standby reg failed");
			goto _exit;
		}
		u8_val |= HOST_CTRL_MASK_ALGORITHM_STANDBY;
		ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &u8_val, 1);
		if (ret < 0) {
			PERR("Write algorithm standby reg failed");
			goto _exit;
		}
		retry = 10;
		do {
			ret = bhy_read_reg(client_data, BHY_REG_HOST_STATUS,
					&u8_val, 1);
			if (ret < 0) {
				PERR("Read host status again failed");
				goto _exit;
			}
			if (u8_val & BHY_HOST_STATUS_MASK_ALGO_STANDBY)
				break;
			msleep(1000);
		} while (--retry);
		if (retry == 0) {
			ret = -EIO;
			PERR("Algo standby does not take effect");
			goto _exit;
		}

		/* Enable pass thru mode */
		u8_val = 1;
		ret = bhy_write_reg(client_data, BHY_REG_PASS_THRU_CFG,
				&u8_val, 1);
		if (ret < 0) {
			PERR("Write pass thru cfg reg failed");
			goto _exit;
		}
		retry = 1000;
		do {
			ret = bhy_read_reg(client_data, BHY_REG_PASS_THRU_READY,
					&u8_val, 1);
			if (ret < 0) {
				PERR("Read pass thru ready reg failed");
				goto _exit;
			}
			if (u8_val & 1)
				break;
			usleep_range(1000, 1100);
		} while (--retry);
		if (retry == 0) {
			ret = -EIO;
			PERR("Pass thru does not take effect");
			goto _exit;
		}
	} else {
		/* Disable pass thru mode */
		u8_val = 0;
		ret = bhy_write_reg(client_data, BHY_REG_PASS_THRU_CFG,
				&u8_val, 1);
		if (ret < 0) {
			PERR("Write pass thru cfg reg failed");
			goto _exit;
		}
		retry = 1000;
		do {
			ret = bhy_read_reg(client_data, BHY_REG_PASS_THRU_READY,
					&u8_val, 1);
			if (ret < 0) {
				PERR("Read pass thru ready reg failed");
				goto _exit;
			}
			if (!(u8_val & 1))
				break;
			usleep_range(1000, 1100);
		} while (--retry);
		if (retry == 0) {
			ret = -EIO;
			PERR("Pass thru disable does not take effect");
			goto _exit;
		}

		/* Make algorithm standby */
		ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &u8_val, 1);
		if (ret < 0) {
			PERR("Read algorithm standby reg failed");
			goto _exit;
		}
		u8_val &= ~HOST_CTRL_MASK_ALGORITHM_STANDBY;
		ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL,
				&u8_val, 1);
		if (ret < 0) {
			PERR("Write algorithm standby reg failed");
			goto _exit;
		}
		retry = 10;
		do {
			ret = bhy_read_reg(client_data, BHY_REG_HOST_STATUS,
					&u8_val, 1);
			if (ret < 0) {
				PERR("Read host status again failed");
				goto _exit;
			}
			if (!(u8_val & BHY_HOST_STATUS_MASK_ALGO_STANDBY))
				break;
			msleep(1000);
		} while (--retry);
		if (retry == 0) {
			ret = -EIO;
			PERR("Pass thru enable does not take effect");
			goto _exit;
		}
	}

	ret = count;

_exit:

	mutex_unlock(&client_data->mutex_bus_op);
	return ret;
}

static ssize_t bhy_store_enable_irq_log(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	int enable;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtoint(buf, 10, &enable);
	if (ret < 0) {
		PERR("invalid input");
		return ret;
	}
	client_data->enable_irq_log = enable;

	return count;
}

static ssize_t bhy_store_enable_fifo_log(struct device *dev
		, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	int enable;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = kstrtoint(buf, 10, &enable);
	if (ret < 0) {
		PERR("invalid input");
		return ret;
	}
	client_data->enable_fifo_log = enable;

	return count;
}

static ssize_t bhy_show_hw_reg_sel(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	return snprintf(buf, 64, "slave_addr=0X%02X, reg=0X%02X, len=%d\n",
		client_data->hw_slave_addr, client_data->hw_reg_sel,
		client_data->hw_reg_len);
}

static ssize_t bhy_store_hw_reg_sel(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	ret = sscanf(buf, "%11X %11X %11d", &client_data->hw_slave_addr,
		&client_data->hw_reg_sel, &client_data->hw_reg_len);
	if (ret != 3) {
		PERR("Invalid argument");
		return -EINVAL;
	}

	return count;
}

static ssize_t bhy_show_hw_reg_val(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 reg_data[128], i;
	int pos;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_soft_pass_thru_read_reg_m(client_data,
		client_data->hw_slave_addr, client_data->hw_reg_sel,
		reg_data, client_data->hw_reg_len);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Reg op failed");
		return ret;
	}

	pos = 0;
	for (i = 0; i < client_data->hw_reg_len; ++i) {
		pos += snprintf(buf + pos, 16, "%02X", reg_data[i]);
		buf[pos++] = (i + 1) % 16 == 0 ? '\n' : ' ';
	}
	if (buf[pos - 1] == ' ')
		buf[pos - 1] = '\n';

	return pos;
}

static ssize_t bhy_store_hw_reg_val(struct device *dev
	, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	ssize_t ret;
	u8 reg_data[32] = { 0, };
	int i, j, status, digit;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}
	status = 0;
	for (i = j = 0; i < count && j < client_data->hw_reg_len; ++i) {
		if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t' ||
			buf[i] == '\r') {
			status = 0;
			++j;
			continue;
		}
		digit = buf[i] & 0x10 ? (buf[i] & 0xF) : ((buf[i] & 0xF) + 9);
		PDEBUG("digit is %d", digit);
		switch (status) {
		case 2:
			++j; /* Fall thru */
		case 0:
			reg_data[j] = digit;
			status = 1;
			break;
		case 1:
			reg_data[j] = reg_data[j] * 16 + digit;
			status = 2;
			break;
		}
	}
	if (status > 0)
		++j;
	if (j > client_data->hw_reg_len)
		j = client_data->hw_reg_len;
	else if (j < client_data->hw_reg_len) {
		PERR("Invalid argument");
		return -EINVAL;
	}
	PDEBUG("Reg data read as");
	for (i = 0; i < j; ++i)
		PDEBUG("%d", reg_data[i]);

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_soft_pass_thru_write_reg_m(client_data,
		client_data->hw_slave_addr, client_data->hw_reg_sel,
		reg_data, client_data->hw_reg_len);
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0) {
		PERR("Reg op failed");
		return ret;
	}

	return count;
}

static ssize_t axis_show(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);

	return snprintf(buf, 64, "%d\n", client_data->acc_axis);
}

static int change_axis(struct bhy_client_data *client_data, int axis)
{
	int handle = BHY_SENSOR_HANDLE_ACCELEROMETER;
	int matrix[9];
	int i;
	u8 data[8];
	int ret;

	memcpy(matrix, axis_matrix[axis], sizeof(matrix));

	for (i = 0; i < 5; ++i) {
		switch (matrix[2 * i]) {
		case 0:
			data[i] = 0;
			break;
		case 1:
			data[i] = 1;
			break;
		case -1:
			data[i] = 0xF;
			break;
		default:
			PERR("Invalid matrix data: %d", matrix[2 * i]);
			return -EINVAL;
		}
		if (i == 4)
			break;
		switch (matrix[2 * i + 1]) {
		case 0:
			break;
		case 1:
			data[i] |= 0x10;
			break;
		case -1:
			data[i] |= 0xF0;
			break;
		default:
			PERR("Invalid matrix data: %d", matrix[2 * i + 1]);
			return -EINVAL;
		}
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_parameter(client_data, BHY_PAGE_SYSTEM,
		BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0 + handle,
		data, sizeof(data));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0)
		PERR("Write mapping matrix failed");

	return ret;
}

static ssize_t axis_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	int axis;
	int ret;

	ret = kstrtoint(buf, 10, &axis);
	if (ret < 0) {
		PERR("invalid input");
		return ret;
	}

	ret = change_axis(client_data, axis);
	if (ret < 0) {
		PERR("change_axis failed");
		return ret;
	}

	return count;
}
#endif /*~ BHY_DEBUG */

static DEVICE_ATTR(rom_id, S_IRUGO,
	bhy_show_rom_id, NULL);
static DEVICE_ATTR(ram_id, S_IRUGO,
	bhy_show_ram_id, NULL);
static DEVICE_ATTR(load_ram_patch, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_load_ram_patch);
static DEVICE_ATTR(status_bank, S_IRUGO,
	bhy_show_status_bank, NULL);
static DEVICE_ATTR(sensor_sel, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_sensor_sel);
static DEVICE_ATTR(sensor_info, S_IRUGO,
	bhy_show_sensor_info, NULL);
static DEVICE_ATTR(sensor_conf, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_sensor_conf, bhy_store_sensor_conf);
static DEVICE_ATTR(sensor_flush, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_sensor_flush);
static DEVICE_ATTR(calib_profile, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_calib_profile, bhy_store_calib_profile);
static DEVICE_ATTR(sic_matrix, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_sic_matrix, bhy_store_sic_matrix);
static DEVICE_ATTR(meta_event_ctrl, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_meta_event_ctrl, bhy_store_meta_event_ctrl);
static DEVICE_ATTR(fifo_ctrl, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_fifo_ctrl, bhy_store_fifo_ctrl);
#ifdef BHY_AR_HAL_SUPPORT
static DEVICE_ATTR(activate_ar_hal, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_activate_ar_hal);
#endif /*~ BHY_AR_HAL_SUPPORT */
static DEVICE_ATTR(reset_flag, S_IRUGO,
	bhy_show_reset_flag, NULL);
static DEVICE_ATTR(working_mode, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_working_mode, bhy_store_working_mode);
static DEVICE_ATTR(op_mode, S_IRUGO,
	bhy_show_op_mode, NULL);
static DEVICE_ATTR(bsx_version, S_IRUGO,
	bhy_show_bsx_version, NULL);
static DEVICE_ATTR(driver_version, S_IRUGO,
	bhy_show_driver_version, NULL);
#ifdef BHY_AR_HAL_SUPPORT
static DEVICE_ATTR(fifo_frame_ar, S_IRUGO,
	bhy_show_fifo_frame_ar, NULL);
#endif /*~ BHY_AR_HAL_SUPPORT */
static DEVICE_ATTR(bmi160_foc_offset_acc, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bmi160_foc_offset_acc, bhy_store_bmi160_foc_offset_acc);
static DEVICE_ATTR(bmi160_foc_offset_gyro,
	S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bmi160_foc_offset_gyro, bhy_store_bmi160_foc_offset_gyro);
static DEVICE_ATTR(bmi160_foc_conf, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bmi160_foc_conf, bhy_store_bmi160_foc_conf);
static DEVICE_ATTR(bmi160_foc_exec, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bmi160_foc_exec, bhy_store_bmi160_foc_exec);
static DEVICE_ATTR(bmi160_foc_save_to_nvm,
	S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bmi160_foc_save_to_nvm, bhy_store_bmi160_foc_save_to_nvm);
static DEVICE_ATTR(bma2x2_foc_offset,
	S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bma2x2_foc_offset, bhy_store_bma2x2_foc_offset);
static DEVICE_ATTR(bma2x2_foc_conf, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bma2x2_foc_conf, bhy_store_bma2x2_foc_conf);
static DEVICE_ATTR(bma2x2_foc_exec, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_bma2x2_foc_exec, bhy_store_bma2x2_foc_exec);
static DEVICE_ATTR(self_test, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_self_test, bhy_store_self_test);
static DEVICE_ATTR(self_test_result, S_IRUGO,
	bhy_show_self_test_result, NULL);
static DEVICE_ATTR(update_device_info, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_update_device_info);
static DEVICE_ATTR(mapping_matrix_acc, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_mapping_matrix_acc, bhy_store_mapping_matrix_acc);
static DEVICE_ATTR(sensor_data_size, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_sensor_data_size);
static DEVICE_ATTR(mapping_matrix, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_mapping_matrix, bhy_store_mapping_matrix);
static DEVICE_ATTR(custom_version, S_IRUGO,
	bhy_show_custom_version, NULL);
static DEVICE_ATTR(req_fw, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_req_fw);
#ifdef BHY_DEBUG
static DEVICE_ATTR(reg_sel, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_reg_sel, bhy_store_reg_sel);
static DEVICE_ATTR(reg_val, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_reg_val, bhy_store_reg_val);
static DEVICE_ATTR(param_sel, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_param_sel, bhy_store_param_sel);
static DEVICE_ATTR(param_val, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_param_val, bhy_store_param_val);
static DEVICE_ATTR(log_raw_data, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_log_raw_data);
static DEVICE_ATTR(log_input_data_gesture, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_log_input_data_gesture);
static DEVICE_ATTR(log_input_data_tilt_ar, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_log_input_data_tilt_ar);
static DEVICE_ATTR(log_fusion_data, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_log_fusion_data);
static DEVICE_ATTR(enable_pass_thru, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_enable_pass_thru);
static DEVICE_ATTR(enable_irq_log, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_enable_irq_log);
static DEVICE_ATTR(enable_fifo_log, S_IWUSR | S_IWGRP | S_IWOTH,
	NULL, bhy_store_enable_fifo_log);
static DEVICE_ATTR(hw_reg_sel, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_hw_reg_sel, bhy_store_hw_reg_sel);
static DEVICE_ATTR(hw_reg_val, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	bhy_show_hw_reg_val, bhy_store_hw_reg_val);
static DEVICE_ATTR(axis,  S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	axis_show, axis_store);
#endif /*~ BHY_DEBUG */

static struct attribute *input_attributes[] = {
	&dev_attr_rom_id.attr,
	&dev_attr_ram_id.attr,
	&dev_attr_load_ram_patch.attr,
	&dev_attr_status_bank.attr,
	&dev_attr_sensor_sel.attr,
	&dev_attr_sensor_info.attr,
	&dev_attr_sensor_conf.attr,
	&dev_attr_sensor_flush.attr,
	&dev_attr_calib_profile.attr,
	&dev_attr_sic_matrix.attr,
	&dev_attr_meta_event_ctrl.attr,
	&dev_attr_fifo_ctrl.attr,
#ifdef BHY_AR_HAL_SUPPORT
	&dev_attr_activate_ar_hal.attr,
#endif /*~ BHY_AR_HAL_SUPPORT */
	&dev_attr_reset_flag.attr,
	&dev_attr_working_mode.attr,
	&dev_attr_op_mode.attr,
	&dev_attr_bsx_version.attr,
	&dev_attr_driver_version.attr,
	&dev_attr_bmi160_foc_offset_acc.attr,
	&dev_attr_bmi160_foc_offset_gyro.attr,
	&dev_attr_bmi160_foc_conf.attr,
	&dev_attr_bmi160_foc_exec.attr,
	&dev_attr_bmi160_foc_save_to_nvm.attr,
	&dev_attr_bma2x2_foc_offset.attr,
	&dev_attr_bma2x2_foc_conf.attr,
	&dev_attr_bma2x2_foc_exec.attr,
	&dev_attr_self_test.attr,
	&dev_attr_self_test_result.attr,
	&dev_attr_update_device_info.attr,
	&dev_attr_mapping_matrix_acc.attr,
	&dev_attr_sensor_data_size.attr,
	&dev_attr_mapping_matrix.attr,
	&dev_attr_custom_version.attr,
	&dev_attr_req_fw.attr,
#ifdef BHY_DEBUG
	&dev_attr_reg_sel.attr,
	&dev_attr_reg_val.attr,
	&dev_attr_param_sel.attr,
	&dev_attr_param_val.attr,
	&dev_attr_log_raw_data.attr,
	&dev_attr_log_input_data_gesture.attr,
	&dev_attr_log_input_data_tilt_ar.attr,
	&dev_attr_log_fusion_data.attr,
	&dev_attr_enable_pass_thru.attr,
	&dev_attr_enable_irq_log.attr,
	&dev_attr_enable_fifo_log.attr,
	&dev_attr_hw_reg_sel.attr,
	&dev_attr_hw_reg_val.attr,
	&dev_attr_axis.attr,
#endif /*~ BHY_DEBUG */
	NULL
};

#ifdef BHY_AR_HAL_SUPPORT
static struct attribute *input_ar_attributes[] = {
	&dev_attr_rom_id.attr,
	&dev_attr_status_bank.attr,
	&dev_attr_sensor_sel.attr,
	&dev_attr_sensor_conf.attr,
	&dev_attr_sensor_flush.attr,
	&dev_attr_meta_event_ctrl.attr,
	&dev_attr_reset_flag.attr,
	&dev_attr_fifo_frame_ar.attr,
	NULL
};
#endif /*~ BHY_AR_HAL_SUPPORT */

static ssize_t bhy_show_fifo_frame(struct file *file
	, struct kobject *kobj, struct bin_attribute *attr,
	char *buffer, loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct input_dev *input = to_input_dev(dev);
	struct bhy_client_data *client_data = input_get_drvdata(input);
	struct frame_queue *q = &client_data->data_queue;

	if (client_data == NULL) {
		PERR("Invalid client_data pointer");
		return -ENODEV;
	}

	mutex_lock(&q->lock);
	if (q->tail == q->head) {
		mutex_unlock(&q->lock);
		return 0;
	}
	memcpy(buffer, &q->frames[q->tail], sizeof(struct fifo_frame));
	if (q->tail == BHY_FRAME_SIZE - 1)
		q->tail = 0;
	else
		++q->tail;
	mutex_unlock(&q->lock);

	return sizeof(struct fifo_frame);
}

static ssize_t bhy_store_fifo_frame(struct file *file
	, struct kobject *kobj, struct bin_attribute *attr,
	char *buffer, loff_t pos, size_t size)
{
	PDEBUG("bhy_store_fifo_frame(dummy)");

	return size;
}

static struct bin_attribute bin_attr_fifo_frame = {
	.attr = {
		.name = "fifo_frame",
		.mode = S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH,
	},
	.size = 0,
	.read = bhy_show_fifo_frame,
	.write = bhy_store_fifo_frame,
};

static ssize_t bhy_bst_show_rom_id(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct bst_dev *bst_dev = to_bst_dev(dev);
	struct bhy_client_data *client_data = bst_get_drvdata(bst_dev);
	ssize_t ret;

	ret = snprintf(buf, 32, "%d\n", client_data->rom_id);

	return ret;
}

static ssize_t bhy_bst_show_ram_id(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct bst_dev *bst_dev = to_bst_dev(dev);
	struct bhy_client_data *client_data = bst_get_drvdata(bst_dev);
	ssize_t ret;

	ret = snprintf(buf, 32, "%d\n", client_data->ram_id);

	return ret;
}

static ssize_t bhy_bst_show_dev_type(struct device *dev
	, struct device_attribute *attr, char *buf)
{
	struct bst_dev *bst_dev = to_bst_dev(dev);
	struct bhy_client_data *client_data = bst_get_drvdata(bst_dev);
	ssize_t ret;

	ret = snprintf(buf, 32, "%s\n", client_data->dev_type);

	return ret;
}

static DEVICE_ATTR(bhy_rom_id, S_IRUGO,
	bhy_bst_show_rom_id, NULL);
static DEVICE_ATTR(bhy_ram_id, S_IRUGO,
	bhy_bst_show_ram_id, NULL);
static DEVICE_ATTR(bhy_dev_type, S_IRUGO,
	bhy_bst_show_dev_type, NULL);

static struct attribute *bst_attributes[] = {
	&dev_attr_bhy_rom_id.attr,
	&dev_attr_bhy_ram_id.attr,
	&dev_attr_bhy_dev_type.attr,
	NULL
};

static ssize_t enable_logging(struct bhy_client_data *client_data, bool enable)
{
	u8 data = 0;
	int ret;

	if (enable) {
		data = 1;
		PINFO("turn on logging");
	} else {
		PINFO("turn off logging");
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_write_reg(client_data, LOGGING_REG, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("set logging mode fail");
		return ret;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	client_data->log_mode = (bool)enable;
	return ret;
}

static ssize_t shealth_cadence_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int64_t enable;
	int ret;

	if (kstrtoll(buf, 10, &enable) < 0)
		return -EINVAL;

	ret = enable_logging(client_data, (bool)enable);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t shealth_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	int pos = 0;
	int ret = 0;
	int i, tmp_idx;

	mutex_lock(&client_data->mutex_pedo);
	switch (this_attr->address) {
	case ATTR_SHEALTH_ENABLE:
		mutex_unlock(&client_data->mutex_pedo);
		return snprintf(buf, PAGE_SIZE, "%d\n", client_data->log_mode);

	case ATTR_SHEALTH_FLUSH_CADENCE:
		if (!client_data->log_mode) {
			mutex_unlock(&client_data->mutex_pedo);
			return -EIO;
		}

		ret = shealth_cadence_enable_store(dev, attr, "0", 1);
		if (ret < 0) {
			PERR("flusing err");
			mutex_unlock(&client_data->mutex_pedo);
			return ret;
		}

		mutex_unlock(&client_data->mutex_pedo);
		ret = wait_for_completion_timeout(&client_data->log_done,
					LOG_TIMEOUT);
		if (unlikely(!ret))
			PERR("wait timed out");
		else if (unlikely(ret < 0))
			PERR("log done completion err(%d)", ret);

		mutex_lock(&client_data->mutex_pedo);
		ret = shealth_cadence_enable_store(dev, attr, "1", 1);
		if (ret < 0)
			PERR("flusing err");

	case ATTR_SHEALTH_CADENCE:
		tmp_idx = client_data->start_index;
		pos += snprintf(buf, PAGE_SIZE, "%lld,%lld,%d",
			mcu_crystal_to_nano(client_data->pedo[tmp_idx].start_time),
			mcu_crystal_to_nano(client_data->pedo[1].end_time),
			tmp_idx);

		for (i = 1; i <= client_data->start_index; i++) {
			unsigned char run =
					(char)client_data->pedo[i].run_count;
			unsigned char walk =
					(char)client_data->pedo[i].walk_count;
			unsigned short run_walk = (run << 8) | walk;
			pos += snprintf(buf + pos, PAGE_SIZE, ",%d", run_walk);
		}

		pos += snprintf(buf + pos, PAGE_SIZE, "\n");
		client_data->start_index = 0;
		client_data->current_index = 0;
		mutex_unlock(&client_data->mutex_pedo);
		return pos;

	case ATTR_PEDOMETER_STEPS:
		mutex_unlock(&client_data->mutex_pedo);
		return snprintf(buf, PAGE_SIZE, "%d\n",
				client_data->total_step);
	}

	mutex_unlock(&client_data->mutex_pedo);
	return 0;
}

static ssize_t shealth_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t shealth_int_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	u16 interrupt_mask = 0;

	PINFO("wait int");
	wait_for_completion_interruptible(&client_data->int_done);

	mutex_lock(&client_data->mutex_pedo);
	interrupt_mask = client_data->interrupt_mask;
	client_data->interrupt_mask = 0;
	mutex_unlock(&client_data->mutex_pedo);

	PINFO("interrupt_mask = %d", interrupt_mask);
	return snprintf(buf, PAGE_SIZE, "%d\n", interrupt_mask);
}

static ssize_t shealth_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", client_data->pedo_enabled);
}

static int enable_sensor(struct bhy_client_data *client_data,
			unsigned char sensor, bool enable, short delay)
{
	struct device *device = &client_data->input->dev;
	struct device_attribute *attr = NULL;
	ssize_t ret;
	short buffer[4] = { 0, };

	if (enable)
		buffer[0] = delay;

	ret = bhy_store_sensor_sel(device, attr,
			(const char *)&sensor, sizeof(sensor));
	if (ret < 0) {
		PERR("select sensor error");
		return ret;
	}

	ret = bhy_store_sensor_conf(device, attr,
			(const char *)buffer, sizeof(buffer));
	if (ret < 0)
		PERR("config sensor error");

	return ret;
}

static int enable_pedometer(struct bhy_client_data *client_data, bool enable)
{
	static int count;
	int ret;

	if (enable) {
		count++;
	} else {
		if (count-- <= 0)
			count = 0;
	}

	if ((int)enable != count)
		return 0;

	PINFO("enable pedometer %u", enable);
	ret = enable_sensor(client_data, PEDOMETER_SENSOR,
		enable, PEDOMETER_CYCLE);
	if (ret < 0) {
		if (enable)
			PERR("enable pedometer error");
		else
			PERR("disable pedometer error");
	}

	return ret;
}

static void sync_sensor(struct bhy_client_data *client_data)
{
	int ret;

	if (client_data->acc_axis >= 0) {
		PINFO("set acc axis %d", client_data->acc_axis);
		ret = change_axis(client_data, client_data->acc_axis);
		if (ret < 0)
			PERR("set acc axis failed");
	}

	if (client_data->acc_enabled) {
		PINFO("re-enable acc sensor");
		ret = enable_sensor(client_data,
			BHY_SENSOR_HANDLE_ACCELEROMETER,
			1, client_data->acc_delay);
		if (ret < 0)
			PERR("re-enable acc sensor err");
	}

	if (client_data->pedo_enabled
		|| client_data->step_det_enabled
		|| client_data->step_cnt_enabled) {
		PINFO("re-enable pedometer");
		ret = enable_sensor(client_data, PEDOMETER_SENSOR,
			1, PEDOMETER_CYCLE);
		if (ret < 0)
			PERR("re-enable pedometer error");
	}

	if (client_data->tilt_enabled) {
		PINFO("re-enable tilt sensor");
		ret = enable_sensor(client_data,
			BHY_SENSOR_HANDLE_TILT_DETECTOR, 1, 50);
		if (ret < 0)
			PERR("re-enable tilt sensor error");
	}

	if (client_data->pickup_enabled) {
		PINFO("re-enable pickup sensor");
		ret = enable_sensor(client_data,
			BHY_SENSOR_HANDLE_PICK_UP_GESTURE, 1, 50);
		if (ret < 0)
			PERR("re-enable pickup sensor error");
	}

	if (client_data->smd_enabled) {
		PINFO("re-enable smd sensor");
		ret = enable_sensor(client_data,
			BHY_SENSOR_HANDLE_SIGNIFICANT_MOTION, 1, 50);
		if (ret < 0)
			PERR("re-enable smd sensor error");
	}

	if (client_data->ar_enabled) {
		PINFO("re-enable ar sensor");
		ret = enable_sensor(client_data, AR_SENSOR, 1, 14);
		if (ret < 0)
			PERR("re-enable ar sensor error");
	}
}

static ssize_t shealth_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int64_t enable;
	ssize_t ret;

	if (kstrtoll(buf, 10, &enable) < 0)
		return -EINVAL;

	ret = enable_pedometer(client_data, (bool)enable);
	if (ret < 0)
		return ret;

	client_data->pedo_enabled = (bool)enable;

	return count;
}

static IIO_DEVICE_ATTR(shealth_cadence_enable,
	S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
	shealth_show, shealth_cadence_enable_store, ATTR_SHEALTH_ENABLE);

static IIO_DEVICE_ATTR(shealth_flush_cadence,
	S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
	shealth_show, NULL, ATTR_SHEALTH_FLUSH_CADENCE);

static IIO_DEVICE_ATTR(pedometer_steps,
	S_IRUGO | S_IWUSR,
	shealth_show, shealth_store, ATTR_PEDOMETER_STEPS);

static IIO_DEVICE_ATTR(shealth_cadence,
	S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
	shealth_show, NULL, ATTR_SHEALTH_CADENCE);

static DEVICE_ATTR(event_shealth_int,
	S_IRUGO,
	shealth_int_show, NULL);

static DEVICE_ATTR(shealth_enable,
	S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
	shealth_enable_show, shealth_enable_store);

static struct attribute *shealth_attributes[] = {
	&iio_dev_attr_shealth_cadence_enable.dev_attr.attr,
	&iio_dev_attr_shealth_flush_cadence.dev_attr.attr,
	&iio_dev_attr_pedometer_steps.dev_attr.attr,
	&iio_dev_attr_shealth_cadence.dev_attr.attr,
	&dev_attr_event_shealth_int.attr,
	&dev_attr_shealth_enable.attr,
	NULL,
};

static const struct attribute_group iio_attribute_group = {
	.name = SENSOR_NAME,
	.attrs = shealth_attributes,
};

static const struct iio_info info = {
	.driver_module = THIS_MODULE,
	.attrs = &iio_attribute_group,
};

static const struct iio_chan_spec channels[] = {
	IIO_CHAN_SOFT_TIMESTAMP(1),
};

static int ssp_preenable(struct iio_dev *indio_dev)
{
	return iio_sw_buffer_preenable(indio_dev);
}

static int ssp_predisable(struct iio_dev *indio_dev)
{
	return 0;
}

static const struct iio_buffer_setup_ops ssp_iio_ring_setup_ops = {
	.preenable = &ssp_preenable,
	.predisable = &ssp_predisable,
};

int init_indio_dev(struct bhy_client_data *client_data)
{
	struct iio_buffer *ring;
	int ret = 0;

	client_data->indio = iio_device_alloc(0);
	if (!client_data->indio)
		return -EIO;

	client_data->indio->name = SENSOR_NAME;
	client_data->indio->dev.parent = client_data->data_bus.dev;
	client_data->indio->info = &info;
	client_data->indio->channels = channels;
	client_data->indio->num_channels = ARRAY_SIZE(channels);
	client_data->indio->modes = INDIO_DIRECT_MODE;
	client_data->indio->currentmode = INDIO_DIRECT_MODE;

	ring = iio_kfifo_allocate(client_data->indio);
	if (!ring)
		goto err_config_ring;

	ring->scan_timestamp = true;
	ring->bytes_per_datum = 8;
	client_data->indio->buffer = ring;
	client_data->indio->setup_ops = &ssp_iio_ring_setup_ops;
	client_data->indio->modes |= INDIO_BUFFER_HARDWARE;

	ret = iio_buffer_register(client_data->indio,
				client_data->indio->channels,
				client_data->indio->num_channels);
	if (ret)
		goto err_register_buffer;

	ret = iio_device_register(client_data->indio);
	if (ret)
		goto err_register_device;

	dev_set_drvdata(&client_data->indio->dev, client_data);

	return 0;

err_register_device:
	iio_buffer_unregister(client_data->indio);
err_register_buffer:
	iio_kfifo_free(client_data->indio->buffer);
err_config_ring:
	iio_device_free(client_data->indio);

	return -EIO;
}

void remove_indio_dev(struct bhy_client_data *client_data)
{
	iio_device_unregister(client_data->indio);
}

static ssize_t accel_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "BHA250\n");
}

static ssize_t accel_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "BOSCH\n");
}

static int accel_open_calibration(struct bhy_client_data *client_data)
{
	struct file *filp = NULL;
	mm_segment_t old_fs;
	int ret = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		set_fs(old_fs);
		ret = PTR_ERR(filp);
		return ret;
	}

	ret = filp->f_op->read(filp, (char *)&client_data->acc_cal,
		sizeof(client_data->acc_cal), &filp->f_pos);
	if (ret != sizeof(client_data->acc_cal))
		ret = -EIO;

	filp_close(filp, current->files);
	set_fs(old_fs);

	PINFO("open accel calibration %d, %d, %d",
			client_data->acc_cal[0],
			client_data->acc_cal[1],
			client_data->acc_cal[2]);

	if ((client_data->acc_cal[0] == 0) && (client_data->acc_cal[1] == 0)
		&& (client_data->acc_cal[2] == 0))
		return -EIO;

	return ret;
}

static ssize_t accel_calibration_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int ret;

	ret = accel_open_calibration(client_data);
	if (ret < 0)
		PERR("calibration open failed(%d)", ret);

	PINFO("Cal data : %d %d %d %d", ret,
			client_data->acc_cal[0],
			client_data->acc_cal[1],
			client_data->acc_cal[2]);

	return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n", ret,
			client_data->acc_cal[0],
			client_data->acc_cal[1],
			client_data->acc_cal[2]);
}

static int accel_do_calibrate(struct bhy_client_data *client_data, int enable)
{
	struct file *filp = NULL;
	mm_segment_t old_fs;
	int sum[3] = { 0, };
	int ret = 0, count, i;
	bool acc_enabled = client_data->acc_enabled;

	if (enable) {
		memset(client_data->acc_cal, 0, sizeof(client_data->acc_cal));
		if (!acc_enabled) {
			ret = enable_sensor(client_data,
					BHY_SENSOR_HANDLE_ACCELEROMETER, 1, 50);
			if (ret < 0) {
				PERR("enable acc sensor err");
				return -EIO;
			}
		}
		msleep(300);

		for (count = 0; count < CALIBRATION_DATA_AMOUNT; count++) {
			for (i = 0; i < ARRAY_SIZE(sum); i++)
				sum[i] += client_data->acc_buffer[i];

			usleep_range(10000, 15000);
		}

		if (!acc_enabled) {
			ret = enable_sensor(client_data,
					BHY_SENSOR_HANDLE_ACCELEROMETER, 0, 50);
			if (ret < 0) {
				PERR("disable acc sensor err");
				return -EIO;
			}
		}

		for (i = 0; i < ARRAY_SIZE(client_data->acc_cal); i++) {
			client_data->acc_cal[i]
				= sum[i] / CALIBRATION_DATA_AMOUNT;
		}

		if (client_data->acc_cal[2] > 0)
			client_data->acc_cal[2] -= MAX_ACCEL_1G;
		else if (client_data->acc_cal[2] < 0)
			client_data->acc_cal[2] += MAX_ACCEL_1G;
	} else {
		memset(client_data->acc_cal, 0, sizeof(client_data->acc_cal));
	}

	PINFO("do accel calibrate %d, %d, %d",
			client_data->acc_cal[0],
			client_data->acc_cal[1],
			client_data->acc_cal[2]);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	filp = filp_open(CALIBRATION_FILE_PATH,
			O_CREAT | O_TRUNC | O_WRONLY, 0664);
	if (IS_ERR(filp)) {
		PERR("Can't open calibration file");
		set_fs(old_fs);
		ret = PTR_ERR(filp);
		return ret;
	}

	ret = filp->f_op->write(filp, (char *)&client_data->acc_cal,
		sizeof(client_data->acc_cal), &filp->f_pos);
	if (ret != sizeof(client_data->acc_cal)) {
		PERR("Can't write the acc_cal to file");
		ret = -EIO;
	}

	filp_close(filp, current->files);
	set_fs(old_fs);

	return ret;
}

static ssize_t accel_calibration_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int64_t enable;
	int ret;

	ret = kstrtoll(buf, 10, &enable);
	if (ret < 0)
		return ret;

	ret = accel_do_calibrate(client_data, (int)enable);
	if (ret < 0)
		PERR("accel_do_calibrate() failed");

	return size;
}

static ssize_t raw_data_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n",
			client_data->acc_buffer[0] / 4,
			client_data->acc_buffer[1] / 4,
			client_data->acc_buffer[2] / 4);
}

static ssize_t accel_reactive_alert_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int ret = 0;

	if (client_data->reactive_alert_selftest) {
		PINFO("Factory Mode!\n");
		if (client_data->irq_ready)
			ret = 1;
		else
			ret = 0;
		client_data->reactive_alert_selftest = false;
	} else {
		if (client_data->reactive_alert_reported)
			ret = 1;
		else
			ret = 0;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t accel_reactive_alert_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int ret;
	bool reactive_alert_enabled = client_data->reactive_alert_enabled;
	bool enable = false;

	client_data->reactive_alert_selftest = false;

	if (sysfs_streq(buf, "0")) {
		enable = false;
	} else if (sysfs_streq(buf, "1")) {
		enable = true;
	} else if (sysfs_streq(buf, "2")) {
		client_data->reactive_alert_selftest = true;
		return size;

	} else {
		PERR("Invalid Value %d\n", *buf);
		return -EINVAL;
	}

	if ((enable == true) && (reactive_alert_enabled == false)) {
		ret = enable_sensor(client_data, BHY_SENSOR_HANDLE_CUSTOM_4_WU,
				true, 50);
		if (ret < 0) {
			PERR("ReactiveAlert - enable failed\n");
			goto accel_reactive_alert_store_out;
		}
		PINFO("ReactiveAlert - Enabled!\n");
		client_data->reactive_alert_enabled = true;
		client_data->reactive_alert_reported = false;
	} else if ((enable == false) && (reactive_alert_enabled == true)) {
		ret = enable_sensor(client_data, BHY_SENSOR_HANDLE_CUSTOM_4_WU,
				false, 50);
		if (ret < 0) {
			PERR("ReactiveAlert - disable failed\n");
			goto accel_reactive_alert_store_out;
		}
		PINFO("ReactiveAlert - Disabled!\n");
		client_data->reactive_alert_enabled = false;
		client_data->reactive_alert_reported = false;
	}
accel_reactive_alert_store_out:
	return size;
}

static ssize_t accel_selftest_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int selftest_result = -1;
	int ret = 0;

	PINFO("start");

	/* do selftest */
	bhy_store_self_test(dev, attr, "1", 1);
	msleep(600);

	if (!client_data->self_test_result[SELF_TEST_RESULT_INDEX_ACC])
		selftest_result = 1;

	/* reload ram patch */
	ret = bhy_load_ram_patch(client_data);
	if (ret < 0)
		PERR("bhy_load_ram_patch failed");

	PINFO("test result: %d,%d,%d,%d\n", selftest_result,
			client_data->acc_self_test_diff[0],
			client_data->acc_self_test_diff[1],
			client_data->acc_self_test_diff[2]);

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d\n", selftest_result,
			client_data->acc_self_test_diff[0],
			client_data->acc_self_test_diff[1],
			client_data->acc_self_test_diff[2]);
}

static int enable_lowpassfilter(struct bhy_client_data *client_data,
				bool enable)
{
	u8 bandwidth;
	int ret;

	PINFO("enable: %d", enable);

	mutex_lock(&client_data->mutex_bus_op);

	/* Get timing margin. */
	usleep_range(4000, 5000);

	if (enable) {
		bandwidth = client_data->bandwidth;
		PINFO("LPF ON : BW 0xF -> 0x%02X\n", client_data->bandwidth);
	} else {

		/* mutex_lock(&client_data->mutex_bus_op); */
		ret = bhy_soft_pass_thru_read_reg(client_data,
			BHY_SLAVE_ADDR_BMA2X2, BMI160_CMD_PMU_BASE_ACC,
			&bandwidth, sizeof(bandwidth));
		PINFO("READ BW : 0x%02X", bandwidth);
		/* mutex_unlock(&client_data->mutex_bus_op); */

		if (ret < 0) {
			PERR("read err");
			mutex_unlock(&client_data->mutex_bus_op);
			return -EIO;
		}

		client_data->bandwidth = bandwidth;
		PINFO("prev bw = %d", client_data->bandwidth);

		bandwidth = 0xF;
		PINFO("LPF OFF : BW 0x%02X -> 0xF\n", client_data->bandwidth);
	}

	PINFO("new bw = %d", bandwidth);
	/* mutex_lock(&client_data->mutex_bus_op); */
	ret = bhy_soft_pass_thru_write_reg(client_data,
			BHY_SLAVE_ADDR_BMA2X2, BMI160_CMD_PMU_BASE_ACC,
			&bandwidth, sizeof(bandwidth));
	mutex_unlock(&client_data->mutex_bus_op);
	if (ret < 0)
		PERR("write err");

	return ret;
}

static ssize_t accel_lowpassfilter_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int64_t buffer;
	int ret;

	ret = kstrtoll(buf, 10, &buffer);
	if (ret < 0)
		return ret;

	if (buffer != 0 && buffer != 1)
		return -EINVAL;

	ret = enable_lowpassfilter(client_data, (bool)buffer);
	if (ret < 0)
		return -EIO;

	return size;
}


static void parse_dt(struct device *dev, struct bhy_client_data *client_data)
{
	struct device_node *np = dev->of_node;
	struct bhy_data_bus *data_bus = &client_data->data_bus;

	/* ldo enable pin */
	client_data->ldo_enable_pin
		= of_get_named_gpio_flags(data_bus->dev->of_node,
					"bhy,ldo_enable", 0, NULL);
	if (client_data->ldo_enable_pin < 0)
		PERR("no ldo_enable pin");

	/* acc sensor positions */
	if (of_property_read_u32(np, "bhy,acc-axis", &client_data->acc_axis)) {
		client_data->acc_axis = -1;
		PINFO("no axis info, set as %d", client_data->acc_axis);
	}
}

ssize_t mcu_revision_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int len = 0;
	u8 buffer[6];
	char tmp[9];

	short rev = 0;
	short year = 0;
	char month = 0;
	char day = 0;

	/* BHY_PAGE_SYSTEM (1) / REV (24) to get mcu revision.
	   REV REV YY YY MM DD */
	len = bhy_read_parameter(client_data, BHY_PAGE_SYSTEM, 24,  buffer, 6);
	if (len < 6) {
		PERR("Error to read mcu revision (%d)\n", len);
	} else {
		rev = buffer[0];
		year = (buffer[2] | (buffer[3] << 8)) % 1000;
		month = buffer[4];
		day = buffer[5];
	}

	snprintf(tmp, 9, "%02d%02d%02d%02d", year, month, day, rev);

	/* MCU Version / KERNEL Version */
	return snprintf(buf, PAGE_SIZE, "BC00%s,BC00%u\n",
			tmp, FIRMWARE_REVISION);
}

ssize_t mcu_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", MODEL_NAME);
}

ssize_t mcu_reset_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);

	reset(client_data);
	return snprintf(buf, PAGE_SIZE, "1\n");
}


static DEVICE_ATTR(name, S_IRUGO, accel_name_show, NULL);
static DEVICE_ATTR(vendor, S_IRUGO, accel_vendor_show, NULL);
static DEVICE_ATTR(calibration, S_IRUGO | S_IWUSR | S_IWGRP,
	accel_calibration_show, accel_calibration_store);
static DEVICE_ATTR(raw_data, S_IRUGO, raw_data_read, NULL);
static DEVICE_ATTR(reactive_alert, S_IRUGO | S_IWUSR | S_IWGRP,
	accel_reactive_alert_show, accel_reactive_alert_store);
static DEVICE_ATTR(selftest, S_IRUGO, accel_selftest_show, NULL);
static DEVICE_ATTR(lowpassfilter, S_IWUSR | S_IWGRP,
	NULL, accel_lowpassfilter_store);
static DEVICE_ATTR(mcu_rev, S_IRUGO, mcu_revision_show, NULL);
static DEVICE_ATTR(mcu_name, S_IRUGO, mcu_name_show, NULL);
static DEVICE_ATTR(mcu_reset, S_IRUGO, mcu_reset_show, NULL);


static struct device_attribute *acc_attrs[] = {
	&dev_attr_name,
	&dev_attr_vendor,
	&dev_attr_calibration,
	&dev_attr_raw_data,
	&dev_attr_reactive_alert,
	&dev_attr_selftest,
	&dev_attr_lowpassfilter,
	&dev_attr_mcu_rev,
	&dev_attr_mcu_name,
	&dev_attr_mcu_reset,
	NULL,
};

int init_sysfs(struct bhy_client_data *client_data)
{
	sensors_register(client_data->acc_device, client_data,
				acc_attrs, ACC_NAME);
	return 0;
}

void remove_sysfs(struct bhy_client_data *client_data)
{
	sensors_unregister(client_data->acc_device, acc_attrs);
}

static void bhy_clear_up(struct bhy_client_data *client_data)
{
	if (client_data != NULL) {
		remove_sysfs(client_data);
		remove_indio_dev(client_data);
		complete_all(&client_data->int_done);
		complete_all(&client_data->log_done);
		mutex_destroy(&client_data->mutex_pedo);
		mutex_destroy(&client_data->mutex_bus_op);
		mutex_destroy(&client_data->data_queue.lock);
#ifdef BHY_AR_HAL_SUPPORT
		mutex_destroy(&client_data->data_queue_ar.lock);
#endif /*~ BHY_AR_HAL_SUPPORT */
		if (client_data->input_attribute_group != NULL) {
			sysfs_remove_group(&client_data->input->dev.kobj,
				client_data->input_attribute_group);
			kfree(client_data->input_attribute_group);
			client_data->input_attribute_group = NULL;
		}
		sysfs_remove_bin_file(&client_data->input->dev.kobj,
			&bin_attr_fifo_frame);
		if (client_data->input != NULL) {
			input_unregister_device(client_data->input);
			input_free_device(client_data->input);
			client_data->input = NULL;
		}
#ifdef BHY_AR_HAL_SUPPORT
		if (client_data->input_ar_attribute_group != NULL) {
			sysfs_remove_group(&client_data->input_ar->dev.kobj,
				client_data->input_ar_attribute_group);
			kfree(client_data->input_ar_attribute_group);
			client_data->input_ar_attribute_group = NULL;
		}
		if (client_data->input_ar != NULL) {
			input_unregister_device(client_data->input_ar);
			input_free_device(client_data->input_ar);
			client_data->input_ar = NULL;
		}
#endif /*~ BHY_AR_HAL_SUPPORT */
		if (client_data->bst_attribute_group != NULL) {
			sysfs_remove_group(&client_data->bst_dev->dev.kobj,
				client_data->bst_attribute_group);
			kfree(client_data->bst_attribute_group);
			client_data->bst_attribute_group = NULL;
		}
		if (client_data->bst_dev != NULL) {
			bst_unregister_device(client_data->bst_dev);
			bst_free_device(client_data->bst_dev);
			client_data->bst_dev = NULL;
		}
		if (client_data->data_bus.irq != -1)
			free_irq(client_data->data_bus.irq, client_data);
		if (client_data->fifo_buf != NULL) {
			kfree(client_data->fifo_buf);
			client_data->fifo_buf = NULL;
		}
		if (client_data->data_queue.frames != NULL) {
			kfree(client_data->data_queue.frames);
			client_data->data_queue.frames = NULL;
		}
#ifdef BHY_AR_HAL_SUPPORT
		if (client_data->data_queue_ar.frames != NULL) {
			kfree(client_data->data_queue_ar.frames);
			client_data->data_queue_ar.frames = NULL;
		}
#endif /*~ BHY_AR_HAL_SUPPORT */
		wake_lock_destroy(&client_data->wlock);
		wake_lock_destroy(&client_data->patch_wlock);
		wake_lock_destroy(&client_data->reset_wlock);

		kfree(client_data);
	}
}

int bhy_probe(struct bhy_data_bus *data_bus)
{
	struct bhy_client_data *client_data = NULL;
	int ret;

	usleep_range(4000, 5000);
	PINFO("bhy_probe function entrance");

	/* check chip id */
	ret = bhy_check_chip_id(data_bus);
	if (ret < 0) {
		PERR("Bosch Sensortec Device not found, chip id mismatch");
		goto err_exit;
	}
	PNOTICE("Bosch Sensortec Device %s detected", SENSOR_NAME);

	/* init client_data */
	client_data = kzalloc(sizeof(struct bhy_client_data), GFP_KERNEL);
	if (client_data == NULL) {
		PERR("no memory available for struct bhy_client_data");
		ret = -ENOMEM;
		goto err_exit;
	}
	dev_set_drvdata(data_bus->dev, client_data);
	client_data->data_bus = *data_bus;
	mutex_init(&client_data->mutex_bus_op);
	mutex_init(&client_data->data_queue.lock);
#ifdef BHY_AR_HAL_SUPPORT
	mutex_init(&client_data->data_queue_ar.lock);
#endif /*~ BHY_AR_HAL_SUPPORT */
	mutex_init(&client_data->mutex_pedo);

	client_data->rom_id = 0;
	client_data->ram_id = 0;
	client_data->dev_type[0] = '\0';
	memset(client_data->self_test_result, -1, SELF_TEST_RESULT_COUNT);
	memset(client_data->acc_self_test_diff,
		0, sizeof(client_data->acc_self_test_diff));
#ifdef BHY_TS_LOGGING_SUPPORT
	client_data->irq_count = 0;
#endif /*~ BHY_TS_LOGGING_SUPPORT */
	init_completion(&client_data->log_done);
	init_completion(&client_data->int_done);
	atomic_set(&client_data->ram_patch_loaded, RAM_PATCH_READY);

	/* Move it to load_ram_patch
	ret = bhy_request_irq(client_data);
	if (ret < 0) {
		PERR("Request IRQ failed");
		goto err_exit;
	}
	disable_irq(data_bus->irq);
	client_data->irq_enabled = false;
	PINFO("Disable IRQ (%d), before Ram Patch", data_bus->irq);
	*/
	client_data->irq_enabled = false;

	/* init input devices */
	ret = bhy_init_input_dev(client_data);
	if (ret < 0) {
		PERR("Init input dev failed");
		goto err_exit;
	}

	/* sysfs input node creation */
	client_data->input_attribute_group =
		kzalloc(sizeof(struct attribute_group), GFP_KERNEL);
	if (client_data->input_attribute_group == NULL) {
		ret = -ENOMEM;
		PERR("No mem for input_attribute_group");
		goto err_exit;
	}
	client_data->input_attribute_group->attrs = input_attributes;
	ret = sysfs_create_group(&client_data->input->dev.kobj,
		client_data->input_attribute_group);
	if (ret < 0) {
		kfree(client_data->input_attribute_group);
		client_data->input_attribute_group = NULL;
		goto err_exit;
	}

	ret = sysfs_create_bin_file(&client_data->input->dev.kobj,
		&bin_attr_fifo_frame);
	if (ret < 0) {
		sysfs_remove_bin_file(&client_data->input->dev.kobj,
			&bin_attr_fifo_frame);
		goto err_exit;
	}

#ifdef BHY_AR_HAL_SUPPORT
	/* sysfs input node for AR creation */
	client_data->input_ar_attribute_group =
		kzalloc(sizeof(struct attribute_group), GFP_KERNEL);
	if (client_data->input_ar_attribute_group == NULL) {
		ret = -ENOMEM;
		PERR("No mem for input_ar_attribute_group");
		goto err_exit;
	}
	client_data->input_ar_attribute_group->attrs = input_ar_attributes;
	ret = sysfs_create_group(&client_data->input_ar->dev.kobj,
		client_data->input_ar_attribute_group);
	if (ret < 0) {
		kfree(client_data->input_ar_attribute_group);
		client_data->input_ar_attribute_group = NULL;
		goto err_exit;
	}
#endif /*~ BHY_AR_HAL_SUPPORT */

	/* bst device creation */
	client_data->bst_dev = bst_allocate_device();
	if (!client_data->bst_dev) {
		PERR("Allocate bst device failed");
		goto err_exit;
	}
	client_data->bst_dev->name = SENSOR_NAME;
	bst_set_drvdata(client_data->bst_dev, client_data);
	ret = bst_register_device(client_data->bst_dev);
	if (ret < 0) {
		bst_free_device(client_data->bst_dev);
		client_data->bst_dev = NULL;
		PERR("Register bst device failed");
		goto err_exit;
	}
	client_data->bst_attribute_group =
		kzalloc(sizeof(struct attribute_group), GFP_KERNEL);
	if (client_data->bst_attribute_group == NULL) {
		ret = -ENOMEM;
		PERR("No mem for bst_attribute_group");
		goto err_exit;
	}
	client_data->bst_attribute_group->attrs = bst_attributes;
	ret = sysfs_create_group(&client_data->bst_dev->dev.kobj,
		client_data->bst_attribute_group);
	if (ret < 0) {
		PERR("Create sysfs nodes for bst device failed");
		goto err_exit;
	}

	client_data->fifo_buf = kmalloc(BHY_FIFO_LEN_MAX, GFP_KERNEL);
	if (!client_data->fifo_buf) {
		PERR("Allocate FIFO buffer failed");
		ret = -ENOMEM;
		goto err_exit;
	}

	client_data->data_queue.frames = kmalloc(BHY_FRAME_SIZE *
			sizeof(struct fifo_frame), GFP_KERNEL);
	if (!client_data->data_queue.frames) {
		PERR("Allocate FIFO frame buffer failed");
		ret = -ENOMEM;
		goto err_exit;
	}
	client_data->data_queue.head = 0;
	client_data->data_queue.tail = 0;
#ifdef BHY_AR_HAL_SUPPORT
	client_data->data_queue_ar.frames = kmalloc(BHY_FRAME_SIZE_AR *
			sizeof(struct fifo_frame), GFP_KERNEL);
	if (!client_data->data_queue_ar.frames) {
		PERR("Allocate ar FIFO frame buffer failed");
		ret = -ENOMEM;
		goto err_exit;
	}
	client_data->data_queue_ar.head = 0;
	client_data->data_queue_ar.tail = 0;
#endif /*~ BHY_AR_HAL_SUPPORT */

	bhy_init_sensor_type_data_len(client_data);
	client_data->late_step_report = false;

	wake_lock_init(&client_data->wlock, WAKE_LOCK_SUSPEND, "bhy");

	atomic_set(&client_data->reset_flag, RESET_FLAG_TODO);

	ret = init_indio_dev(client_data);
	if (ret < 0) {
		PERR("init indio dev failed");
		goto err_exit;
	}

	ret = init_sysfs(client_data);
	if (ret < 0) {
		PERR("init sysfs failed");
		goto err_exit;
	}

	parse_dt(data_bus->dev, client_data);

	/** Monitor Thread **/
	init_waitqueue_head(&client_data->monitor_wq);
	atomic_set(&client_data->ram_patch_loaded, RAM_PATCH_READY);
	client_data->irq_force_disabled = false;

	wake_lock_init(&client_data->patch_wlock,
			WAKE_LOCK_SUSPEND, "bhy_patch");
	wake_lock_init(&client_data->reset_wlock,
		WAKE_LOCK_SUSPEND, "bhy_reset");

	client_data->monitor_task = kthread_run(mcu_monitor_thread,
				(void *)client_data, "bhy_monitor_thread");
	if (IS_ERR(client_data->monitor_task)) {
		ret = PTR_ERR(client_data->monitor_task);
		goto err_exit;
	}

	PNOTICE("sensor %s probed successfully", SENSOR_NAME);
	return 0;

err_exit:
	bhy_clear_up(client_data);

	return ret;
}
EXPORT_SYMBOL(bhy_probe);

int bhy_remove(struct device *dev)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	bhy_clear_up(client_data);
	return 0;
}
EXPORT_SYMBOL(bhy_remove);

#ifdef CONFIG_PM
int bhy_suspend(struct device *dev)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int ret;
	u8 data;
#ifdef BHY_TS_LOGGING_SUPPORT
	struct frame_queue *q = &client_data->data_queue;
#endif /*~ BHY_TS_LOGGING_SUPPORT */

	PINFO("Enter suspend");

	if (client_data->step_det_enabled || client_data->step_cnt_enabled) {
		if (!client_data->pedo_enabled) {
			ret = enable_logging(client_data, true);
			if (ret < 0)
				return ret;
		}
	}

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read host ctrl reg failed");
		return -EIO;
	}
	data |= HOST_CTRL_MASK_AP_SUSPENDED;
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write host ctrl reg failed");
		return -EIO;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	if (!client_data->irq_force_disabled)
		enable_irq_wake(client_data->data_bus.irq);

	atomic_set(&client_data->in_suspend, 1);

#ifdef BHY_TS_LOGGING_SUPPORT
	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_AP_SLEEP_STATUS;
	q->frames[q->head].data[0] = BHY_AP_STATUS_SUSPEND;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);
#endif /*~ BHY_TS_LOGGING_SUPPORT */

	return 0;
}
EXPORT_SYMBOL(bhy_suspend);

int bhy_resume(struct device *dev)
{
	struct bhy_client_data *client_data = dev_get_drvdata(dev);
	int ret;
	u8 data;
#ifdef BHY_TS_LOGGING_SUPPORT
	struct frame_queue *q = &client_data->data_queue;
#endif /*~ BHY_TS_LOGGING_SUPPORT */

	PINFO("Enter resume");

	/** Monitor Thread **/
	client_data->last_acc_check_time = get_current_timestamp();

	if (!client_data->irq_force_disabled)
		disable_irq_wake(client_data->data_bus.irq);

	mutex_lock(&client_data->mutex_bus_op);
	ret = bhy_read_reg(client_data, BHY_REG_HOST_CTRL, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Read host ctrl reg failed");
		return -EIO;
	}
	data &= ~HOST_CTRL_MASK_AP_SUSPENDED;
	ret = bhy_write_reg(client_data, BHY_REG_HOST_CTRL, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write host ctrl reg failed");
		return -EIO;
	}
	/* Flush all sensor data */
	data = 0xFF;
	ret = bhy_write_reg(client_data, BHY_REG_FIFO_FLUSH, &data, 1);
	if (ret < 0) {
		mutex_unlock(&client_data->mutex_bus_op);
		PERR("Write flush sensor reg error");
		return ret;
	}
	mutex_unlock(&client_data->mutex_bus_op);

	atomic_set(&client_data->in_suspend, 0);

#ifdef BHY_TS_LOGGING_SUPPORT
	client_data->irq_count = 0;
	mutex_lock(&q->lock);
	q->frames[q->head].handle = BHY_SENSOR_HANDLE_AP_SLEEP_STATUS;
	q->frames[q->head].data[0] = BHY_AP_STATUS_RESUME;
	if (q->head == BHY_FRAME_SIZE - 1)
		q->head = 0;
	else
		++q->head;
	if (q->head == q->tail) {
		frame_debug("One frame data lost",
			__func__, __LINE__);
		if (q->tail == BHY_FRAME_SIZE - 1)
			q->tail = 0;
		else
			++q->tail;
	}
	mutex_unlock(&q->lock);

	input_event(client_data->input, EV_MSC, MSC_RAW, 0);
	input_sync(client_data->input);
#endif /*~ BHY_TS_LOGGING_SUPPORT */

	if (client_data->step_det_enabled || client_data->step_cnt_enabled) {
		if (!client_data->pedo_enabled) {
			ret = enable_logging(client_data, false);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}
EXPORT_SYMBOL(bhy_resume);
#endif /*~ CONFIG_PM */
