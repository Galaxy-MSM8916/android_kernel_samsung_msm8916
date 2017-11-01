/*
 * driver/irda_ice40 IR Led driver
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/firmware.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include <linux/err.h>
#include <linux/irda_ice40.h>

#if defined(TEST_DEBUG)
#define pr_irda	pr_emerg
#else
#define pr_irda	pr_info
#endif

static int g_ack_number;
static int count_number;
static struct irda_ice40_platform_data *g_pdata;
static struct irda_ice40_data *g_data;
static struct mutex		en_mutex;
static int gpio_ir_led_en;

static void ice40_clock_en(int onoff)
{
	static struct clk *fpga_main_src_clk;
	static bool is_clk_enabled = false;
	struct device dev;
	dev = g_data->client->dev;

	pr_info("%s:%d - on : %d\n", __func__, __LINE__, onoff);

	if (!fpga_main_src_clk)
		fpga_main_src_clk = clk_get(&dev, "rf_clk2");	
	if (IS_ERR(fpga_main_src_clk))
		pr_err("%s: unable to get fpga_main_src_clk\n", __func__);

	if (onoff && !is_clk_enabled) {
		clk_prepare_enable(fpga_main_src_clk);
		is_clk_enabled = true;
	} else {
		clk_disable_unprepare(fpga_main_src_clk);
		clk_put(fpga_main_src_clk);
		fpga_main_src_clk = NULL;
		is_clk_enabled = false;
	}
	return;
}

static void fpga_enable(int enable_clk, int enable_rst_n)
{
	if (enable_clk) {
		mutex_lock(&en_mutex);
		ice40_clock_en(1);
		if (enable_rst_n)
			gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_HIGH);
		usleep_range(1000, 2000);
	} else {
		usleep_range(2000, 2500);
		gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_LOW);
		ice40_clock_en(0);
		mutex_unlock(&en_mutex);
	}
}

static void irled_power_onoff(int onoff)
{
	int ret;
	static struct regulator *pwr_ldo = NULL;
	static struct regulator *pwr_ldo2 = NULL;
	struct device dev;
	dev = g_data->client->dev;
	if (!pwr_ldo) {
		pwr_ldo = regulator_get(&dev, "vcc");
	}
	if (!pwr_ldo2) {
		pwr_ldo2 = regulator_get(&dev, "vccio1");
	}
	if (onoff) {
		ret = regulator_enable(pwr_ldo);
		if (ret) {
			printk(KERN_ERR"enable l5 failed, rc=%d\n", ret);
			return;
		}
		ret = regulator_enable(pwr_ldo2);
		if (ret) {
			printk(KERN_ERR"enable l2 failed, rc=%d\n", ret);
			return;
		}
		printk(KERN_DEBUG"ir_led power_on is finished.\n");
	} else {
		if (regulator_is_enabled(pwr_ldo)) {
			ret = regulator_disable(pwr_ldo);
			if (ret) {
				printk(KERN_ERR"disable l5 failed, rc=%d\n",
						ret);
				return;
			}
		}
		if (regulator_is_enabled(pwr_ldo2)) {
			ret = regulator_disable(pwr_ldo2);
			if (ret) {
				printk(KERN_ERR"disable l2 failed, rc=%d\n",
						ret);
				return;
			}
		}
		printk(KERN_DEBUG"ir_led power_off is finished.\n");
	}
}

#ifdef CONFIG_OF
static int irda_ice40_parse_dt(struct device *dev,
			struct irda_ice40_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	int ret;
	ret = of_property_read_u32(np, "irda_ice40,fw_ver", &pdata->fw_ver);
	if (ret < 0) {
		pr_err("[%s]: failed to read fw_ver\n", __func__);
		return ret;
	}
	gpio_ir_led_en = of_get_named_gpio(np, "irda_ice40,ir_led_en_gpio", 0);
	pdata->rst_n = of_get_named_gpio(np, "irda_ice40,reset_n", 0);
	pdata->spi_clk = of_get_named_gpio(np, "irda_ice40,scl-gpio", 0);
	pdata->spi_si = of_get_named_gpio(np, "irda_ice40,sda-gpio", 0);
	pdata->irda_irq = of_get_named_gpio(np, "irda_ice40,irq-gpio", 0);
	pdata->cresetb = of_get_named_gpio(np, "irda_ice40,cresetb", 0);

	return 0;
}
#else
static int irda_ice40_parse_dt(struct device *dev,
			struct irda_ice40_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static void irda_ice40_config(void)
{
	int rc = 0;

	pr_info("%s\n", __func__);
	rc = gpio_direction_output(g_pdata->spi_si, 1);
	if (rc)
		pr_err("%s: spi_si error : %d\n", __func__, rc);
	rc = gpio_direction_output(g_pdata->spi_clk, 1);
	if (rc)
		pr_err("%s: spi_clk error : %d\n", __func__, rc);

	rc = gpio_request(g_pdata->cresetb, "irda_creset");
	if (rc)
		pr_err("%s: cresetb error : %d\n", __func__, rc);

	if(gpio_ir_led_en != NO_PIN_DETECTED) {
		rc = gpio_request(gpio_ir_led_en, "irda_led_en");
		if (rc)
			pr_err("%s: Ir led en error : %d\n", __func__, rc);
		else
			gpio_direction_output(gpio_ir_led_en,1);
	}

	rc = gpio_request(g_pdata->rst_n, "irda_rst_n");
	if (rc)
		pr_err("%s: rst_n error : %d\n", __func__, rc);
	rc = gpio_direction_output(g_pdata->rst_n, 1);
	if (rc)
		pr_err("%s: rst_n error : %d\n", __func__, rc);
	rc = gpio_direction_output(g_pdata->rst_n, 0);	
	if (rc)
		pr_err("%s: rst_n error : %d\n", __func__, rc);

	rc = gpio_request(g_pdata->irda_irq, "irda_irq");
	if (rc)
		pr_err("%s: irda_irq error : %d\n", __func__, rc);
}

/*
 * Send ice40 fpga firmware data thougth spi communication
 */
static int ice40_fpga_send_firmware_data(const u8 *data, int len)
{
	unsigned int i, j;
	unsigned char spibit;

	i = 0;
	while (i < len) {
		j = 0;
		spibit = data[i];
		while (j < 8) {
			gpio_set_value_cansleep(g_pdata->spi_clk,
						GPIO_LEVEL_LOW);

			if (spibit & 0x80)
				gpio_set_value_cansleep(g_pdata->spi_si,
						GPIO_LEVEL_HIGH);
			else
				gpio_set_value_cansleep(g_pdata->spi_si,
						GPIO_LEVEL_LOW);

			j = j+1;
			gpio_set_value_cansleep(g_pdata->spi_clk,
						GPIO_LEVEL_HIGH);
			spibit = spibit<<1;
		}
		i = i+1;
	}

	i = 0;
	while (i < 200) {
		gpio_set_value_cansleep(g_pdata->spi_clk, GPIO_LEVEL_LOW);
		i = i+1;
		gpio_set_value_cansleep(g_pdata->spi_clk, GPIO_LEVEL_HIGH);
	}
	return 0;
}

static int ice40_fpga_firmware_update_start(const u8 *data, int len)
{
	int retry = FIRMWARE_MAX_RETRY;
	int rc =0;
	do{
		pr_irda("%s : FPGA firmware update cnt : %d\n", __func__,FIRMWARE_MAX_RETRY - retry);
		gpio_set_value(g_pdata->rst_n, GPIO_LEVEL_LOW);
		usleep_range(30, 50);
		gpio_set_value(g_pdata->cresetb, GPIO_LEVEL_LOW);
		usleep_range(50, 80);
		gpio_set_value(g_pdata->cresetb, GPIO_LEVEL_HIGH);
		usleep_range(1000, 1300);
		ice40_fpga_send_firmware_data(data, len);
		usleep_range(50, 70);
		udelay(50);
	}while((gpio_get_value(g_pdata->irda_irq) != GPIO_LEVEL_HIGH) && (retry-- >= 0));
	if(gpio_get_value(g_pdata->irda_irq) == GPIO_LEVEL_LOW)
		pr_irda("%s : FPGA firmware update Fail\n", __func__);
	else
		pr_irda("%s : FPGA firmware update success\n", __func__);
	//initalized
	fpga_enable(1,1);
	usleep_range(50, 70);
	fpga_enable(0,0);

	//IN - NP to i2c line because of sleep current leakage
	rc = gpio_direction_input(g_pdata->spi_si);
	if (rc)
		pr_err("%s: spi_si error : %d\n", __func__, rc);
	rc = gpio_direction_input(g_pdata->spi_clk);
	if (rc)
		pr_err("%s: spi_clk error : %d\n", __func__, rc);

	return 0;
}

void ice40_fpga_firmware_update(void)
{
	struct i2c_client *client = g_data->client;

	switch (g_pdata->fw_ver) {
	case 1:
		pr_irda("%s[%d] fw_ver %d\n", __func__,
				__LINE__, g_pdata->fw_ver);
		if (request_firmware(&g_data->fw, "ice40xx/i2c_top_bitmap_1.fw", &client->dev))
			pr_err("%s: Can't open firmware file\n", __func__);
		ice40_fpga_firmware_update_start(g_data->fw->data,g_data->fw->size);
		release_firmware(g_data->fw);
		break;

//	thunder
	case 2:
		pr_irda("%s[%d] fw_ver %d\n", __func__,
				__LINE__, g_pdata->fw_ver);
		if (request_firmware(&g_data->fw,"ice40xx/i2c_top_bitmap_2.fw", &client->dev))
			pr_err("%s: Can't open firmware file\n", __func__);
		else
			ice40_fpga_firmware_update_start(g_data->fw->data,
					g_data->fw->size);
		release_firmware(g_data->fw);
		break;
	default:
		pr_err("[%s] Not supported [fw_ver = %d]\n",
				__func__, g_pdata->fw_ver);
		break;
	}
	usleep_range(10000, 12000);
}

static ssize_t ice40_fpga_fw_update_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct file *fp = NULL;
	long fsize = 0, nread = 0;
	const u8 *buff = 0;
	char fw_path[SEC_FPGA_MAX_FW_PATH];
	int locate, ret, rc;
	mm_segment_t old_fs = get_fs();

	pr_irda("%s\n", __func__);

	ret = sscanf(buf, "%d", &locate);
	if (!ret) {
		pr_err("[%s] force select extSdCard\n", __func__);
		locate = 0;
	}

	old_fs = get_fs();
	set_fs(get_ds());

	if (locate) {
		snprintf(fw_path, SEC_FPGA_MAX_FW_PATH,
				"/storage/sdcard0/%s", SEC_FPGA_FW_FILENAME);
	} else {
		snprintf(fw_path, SEC_FPGA_MAX_FW_PATH,
				"/storage/extSdCard/%s", SEC_FPGA_FW_FILENAME);
	}

	fp = filp_open(fw_path, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		pr_err("file %s open error:%d\n",
				fw_path, (s32)fp);
		goto err_open;
	}

	fsize = fp->f_path.dentry->d_inode->i_size;
	pr_irda("fpga firmware size: %ld\n", fsize);

	buff = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!buff) {
		pr_err("fail to alloc buffer for fw\n");
		goto err_alloc;
	}

	nread = vfs_read(fp, (char __user *)buff, fsize, &fp->f_pos);
	if (nread != fsize) {
		pr_err("fail to read file %s (nread = %ld)\n",
				fw_path, nread);
		goto err_fw_size;
	}

	rc = gpio_direction_output(g_pdata->spi_si, 1);
	if (rc)
		pr_err("%s: spi_si error : %d\n", __func__, rc);
	rc = gpio_direction_output(g_pdata->spi_clk, 1);
	if (rc)
		pr_err("%s: spi_clk error : %d\n", __func__, rc);

	ice40_fpga_firmware_update_start((unsigned char *)buff, fsize);

err_fw_size:
		kfree(buff);
err_alloc:
		filp_close(fp, NULL);
err_open:
		set_fs(old_fs);

	return size;
}

static ssize_t ice40_fpga_fw_update_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}

static int irda_ice40_read(struct i2c_client *client, u16 slave_addr,
		u16 reg_addr, u16 length, u8 *value)
{
	struct i2c_msg msg[2];
	int ret;

	pr_irda("client address before read %u\n", client->addr);
	*value = 0;
	client->addr = slave_addr;

	msg[0].addr = client->addr;
	msg[0].flags = 0x00;
	msg[0].len = 1;
	msg[0].buf = (u8 *)&reg_addr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD | I2C_CLIENT_PEC;
	msg[1].len = length;
	msg[1].buf = (u8 *)value;

	fpga_enable(1, 1);

	ret = i2c_transfer(client->adapter, msg, 2);
	if  (ret != 2) {
		pr_irda("%s: err1 %d\n", __func__, ret);
		ret = i2c_transfer(client->adapter, msg, 2);
		if (ret != 2) {
			pr_irda("%s: err2 %d\n", __func__, ret);
			fpga_enable(0, 0);
			return -ret;
		} else {
			fpga_enable(0, 0);
			return 0;
		}
	} else {
		fpga_enable(0, 0);
		return 0;
	}
}

static ssize_t ice40_ver_check_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct irda_ice40_data *data = dev_get_drvdata(dev);
	char *bufp = buf;
	u8 fw_ver, read_val;

	irda_ice40_read(data->client, IRDA_I2C_ADDR, FW_VER_ADDR, 1, &read_val);

	pr_irda("%s Actual value read 0x%x\n", __func__, read_val);
	bufp += snprintf(bufp, SNPRINT_BUF_SIZE, "val 0x%x,", read_val);
	bufp += snprintf(bufp, SNPRINT_BUF_SIZE,
					"operation 0x%x,", read_val&0x3);
	fw_ver = (read_val >> 2) & 0x3;
	bufp += snprintf(bufp, SNPRINT_BUF_SIZE, "ver %d\n", fw_ver + 11);

	irda_ice40_read(data->client, IRDA_I2C_ADDR, 0x00, 1, &read_val);
	fw_ver = (read_val >> 4) & 0xf;
	bufp += snprintf(bufp, SNPRINT_BUF_SIZE, "0x00 read ver %d\n", fw_ver);

	return strlen(buf);
}

static void fw_work(struct work_struct *work)
{
	ice40_fpga_firmware_update();
}

static int ir_remocon_work(struct irda_ice40_data *ir_data, int count)
{
	struct irda_ice40_data *data = ir_data;
	struct i2c_client *client = data->client;
	int buf_size = count;
	int ret;
	int emission_time;
	int ack_pin_onoff;
	int ack_number;
	int f_checksum;
	int retry;

	if (count_number >= 100)
		count_number = 0;

	count_number++;

	pr_irda("%s: total buf_size: %d\n", __func__, buf_size);

	fpga_enable(1, 1);
	mutex_lock(&data->mutex);

	client->addr = IRDA_I2C_ADDR;

	data->i2c_block_transfer.addr = 0x00;
	data->i2c_block_transfer.data[0] = (count >> 8) & 0xFF;
	data->i2c_block_transfer.data[1] = count & 0xFF;
	buf_size++;
	f_checksum = 0;
	retry = 0;
	while (!f_checksum) {
		ret = i2c_master_send(client,
				(unsigned char *) &(data->i2c_block_transfer), buf_size);
		if (ret < 0) {
			dev_err(&client->dev, "%s: err1 %d\n", __func__, ret);
			ret = i2c_master_send(client,
					(unsigned char *) &(data->i2c_block_transfer), buf_size);
			if (ret < 0) {
				dev_err(&client->dev, "%s: err1 %d\n", __func__, ret);
				ret = i2c_master_send(client,
						data->i2c_block_transfer.data, count);
				if (ret < 0)
					dev_err(&client->dev, "%s: err2 %d\n",
							__func__, ret);
			}
		}
		usleep_range(10000, 12000);

		ack_pin_onoff = 0;

		if (gpio_get_value(g_pdata->irda_irq)) {
			ack_pin_onoff = 1;
			retry++;
		} else {
			ack_pin_onoff = 2;
			f_checksum = 1;
		}
		if (retry > 5)
			break;
	}
	if (ack_pin_onoff == 1)
		pr_irda("%s : %d %d Checksum NG!\n",
				__func__, count_number, retry);
	else {
		if (!retry)
			pr_irda("%s : %d %d Checksum OK!\n",
					__func__, count_number, retry);
		else
			pr_irda("%s : %d %d Checksum RE!\n",
					__func__, count_number, retry);
	}
	ack_number = ack_pin_onoff;

	mutex_unlock(&data->mutex);

	emission_time = data->ir_sum;

	if (emission_time > 0)
		usleep(emission_time);

	pr_irda("%s: emission_time = %d\n",
			__func__, emission_time);

	retry = 0;
	while (!gpio_get_value(g_pdata->irda_irq)) {
		usleep_range(10000, 12000);
		pr_irda("%s : try to check irda_irq %d, %d\n",
				__func__, emission_time, retry);
		if (retry++ > 5)
			break;
	}

	if (gpio_get_value(g_pdata->irda_irq)) {
		pr_irda("%s : %d Sending IR OK!\n",
				__func__, count_number);
		ack_pin_onoff = 4;
	} else {
		pr_irda("%s : %d Sending IR NG!\n",
				__func__, count_number);
		ack_pin_onoff = 2;
	}

	ack_number += ack_pin_onoff;

	data->ir_freq = 0;
	data->ir_sum = 0;
	data->count = 0;
	data->length = 0;
	data->operation = 0xffff;

	fpga_enable(0, 0);

	g_ack_number = ack_number;
	if (ack_number == 6)
		return SEND_SUCCESS;
	else
		return SEND_FAIL;
}

static ssize_t remocon_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct irda_ice40_data *data = dev_get_drvdata(dev);
	unsigned int _data;
	unsigned int count = 2, i = 0;
	int ret;

	pr_irda("%s ir_send called[%d]\n", __func__, __LINE__);

	for (i = 0; i < MAX_SIZE; i++) {
		if (sscanf(buf++, "%u", &_data) == 1) {
			if (_data == 0 || buf == '\0')
				break;
			else if(count >= MAX_SIZE - 4) {
				pr_irda("%s: array overflow! %d\n", __func__,sizeof(buf));
				break;
			}
			if (count == 2) {
				data->ir_freq = _data;
				data->operation = IRDA_SINGLE;
				/* operation cmd */
				/* single mode */
				data->i2c_block_transfer.data[2]
						= IRDA_SINGLE;
				/* frequency cmd 15~8 */
				data->i2c_block_transfer.data[3]
						= (_data >> 8) & 0xFF;
				/* frequency cmd 7~0 */
				data->i2c_block_transfer.data[4]
							= _data & 0xFF;
				count += 3;
			} else {
				data->ir_sum += _data;
				data->i2c_block_transfer.data[count++] = _data >> 24;
				data->i2c_block_transfer.data[count++] = _data >> 16;
				data->i2c_block_transfer.data[count++] = _data >> 8;
				data->i2c_block_transfer.data[count++] = _data & 0xFF;
			}
			while (_data > 0) {
				buf++;
				_data /= 10;
			}
		} else {
			break;
		}
	}
	data->count = count;

	ret = ir_remocon_work(data, data->count);
	if (ret < 0)
		pr_info("%s, failed Send ir led\n", __func__);
	return size;
}

static ssize_t remocon_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct irda_ice40_data *data = dev_get_drvdata(dev);
	int i;
	char *bufp = buf;

	for (i = 5; i < MAX_SIZE - 1; i++) {
		if (data->i2c_block_transfer.data[i] == 0
			&& data->i2c_block_transfer.data[i+1] == 0)
			break;
		else
			bufp += snprintf(bufp, SNPRINT_BUF_SIZE, "%u,",
					data->i2c_block_transfer.data[i]);
	}
	return strlen(buf);
}

/* sysfs node ir_send_result */
static ssize_t remocon_ack(struct device *dev, struct device_attribute *attr,
		char *buf)
{

	pr_irda("%s : g_ack_number = %d\n", __func__, g_ack_number);

	if (g_ack_number == 6)
		return snprintf(buf, SNPRINT_BUF_SIZE, "1\n");
	else
		return snprintf(buf, SNPRINT_BUF_SIZE, "0\n");
}

static int irda_read_device_info(struct irda_ice40_data *ir_data)
{
	struct irda_ice40_data *data = ir_data;
	struct i2c_client *client = data->client;
	u8 buf_ir_test[8];
	int ret;

	pr_irda("%s called\n", __func__);

	fpga_enable(1, 1);

	client->addr = IRDA_I2C_ADDR;
	ret = i2c_master_recv(client, buf_ir_test, READ_LENGTH);

	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	pr_irda("%s: buf_ir dev_id: 0x%02x, 0x%02x\n", __func__,
			buf_ir_test[2], buf_ir_test[3]);
	ret = data->dev_id = (buf_ir_test[2] << 8 | buf_ir_test[3]);

	fpga_enable(0, 0);

	return ret;
}

/* sysfs node check_ir */
static ssize_t check_ir_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct irda_ice40_data *data = dev_get_drvdata(dev);
	int ret;
	ret = irda_read_device_info(data);
	return snprintf(buf, 1, "%d\n", ret);
}

/* sysfs node irda_test */
static ssize_t irda_test_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int ret, i;
	struct irda_ice40_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	struct {
			unsigned char addr;
			unsigned char data[IRDA_TEST_CODE_SIZE-1];
	} i2c_block_transfer;
	unsigned char BSR_data[IRDA_TEST_CODE_SIZE-1] = {
		0x8D, 0x00, 0x96, 0x00, 0x01, 0x50, 0x00, 0xA8,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x15, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x15,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F, 0x00, 0x15, 0x00, 0x3F,
		0x00, 0x15, 0x00, 0x3F
	};
	pr_irda("IRDA test code start\n");

	/* change address for IRDA */
	client->addr = IRDA_I2C_ADDR;

	/* make data for sending */
	for (i = 0; i < IRDA_TEST_CODE_SIZE - 1; i++)
		i2c_block_transfer.data[i] = BSR_data[i];

	fpga_enable(1, 1);

	/* sending data by I2C */
	i2c_block_transfer.addr = IRDA_TEST_CODE_ADDR;
	ret = i2c_master_send(client, (unsigned char *) &i2c_block_transfer,
			IRDA_TEST_CODE_SIZE);
	if (ret < 0) {
		pr_err("%s: err1 %d\n", __func__, ret);
		ret = i2c_master_send(client,
		(unsigned char *) &i2c_block_transfer, IRDA_TEST_CODE_SIZE);
		if (ret < 0)
			pr_err("%s: err2 %d\n", __func__, ret);
	}

	fpga_enable(0, 0);

	return size;
}

static ssize_t irda_test_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return strlen(buf);
}

static struct device_attribute ice40_attrs[] = {
	__ATTR(ice40_fpga_fw_update, S_IRUGO|S_IWUSR|S_IWGRP,
			ice40_fpga_fw_update_show, ice40_fpga_fw_update_store),
	__ATTR(ice40_ver_check, S_IRUGO,ice40_ver_check_show, NULL),
	__ATTR(check_ir, S_IRUGO,check_ir_show, NULL),
	__ATTR(ir_send, S_IRUGO|S_IWUSR|S_IWGRP, remocon_show, remocon_store),
	__ATTR(ir_send_result, S_IRUGO, remocon_ack, NULL),
	__ATTR(irda_test, S_IRUGO|S_IWUSR|S_IWGRP,
			irda_test_show, irda_test_store)
};

static int ice40_open(struct inode *inode, struct file *file)
{
	int err = 0;

	pr_irda("ice40_open %s\n", __func__);
	err = nonseekable_open(inode, file);
	if (err)
		return err;
	file->private_data = g_data;

	return 0;
}

static int ice40_close(struct inode *inode, struct file *file)
{
	pr_irda("ice40_close %s\n", __func__);
	return 0;
}



static const struct file_operations ice40_fops = {
	.owner          = THIS_MODULE,
	.open           = ice40_open,
	.release        = ice40_close,

};

static int irda_ice40_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct irda_ice40_data *data;
	struct irda_ice40_platform_data *pdata;
	struct device *irda_ice40_dev;

	int i, error;
	pr_irda("%s probe!\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct irda_ice40_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}
		error = irda_ice40_parse_dt(&client->dev, pdata);
		if (error)
			return error;
	} else
		pdata = client->dev.platform_data;

	g_pdata = pdata;
	data = kzalloc(sizeof(struct irda_ice40_data), GFP_KERNEL);
	if (NULL == data) {
		pr_err("Failed to data allocate %s\n", __func__);
		error = -ENOMEM;
		goto err_free_mem;
	}

	data->client = client;
	mutex_init(&en_mutex);
	mutex_init(&data->mutex);
	data->ir_sum = 0;
	data->operation = 0xFFFF;
	data->count = 0;
	i2c_set_clientdata(client, data);

	g_data = data;

	irled_power_onoff(POWER_ON);
	irda_ice40_config();

	irda_ice40_dev = device_create(sec_class, NULL, 0, data, "sec_ir");
	if (IS_ERR(irda_ice40_dev))
		pr_err("Failed to create irda_ice40_dev device in sec_ir\n");

	/* sysfs entries */
	for (i = 0; i < ARRAY_SIZE(ice40_attrs); i++) {
		if (device_create_file(irda_ice40_dev, &ice40_attrs[i]) < 0)
			pr_err("Failed to create device file(%s)!\n",
					ice40_attrs[i].attr.name);
	}

	/*Create dedicated thread so that
	 the delay of our work does not affect others*/
	data->firmware_dl =
		create_singlethread_workqueue("ice40_firmware_dl");
	INIT_DELAYED_WORK(&data->fw_dl, fw_work);
	/* min 1ms is needed */
	queue_delayed_work(data->firmware_dl,
			&data->fw_dl, msecs_to_jiffies(20));

	pr_irda("%s complete[%d]\n", __func__, __LINE__);
	return 0;

err_free_mem:
	return error;
}

static int irda_ice40_remove(struct i2c_client *client)
{
	struct irda_ice40_data *data = i2c_get_clientdata(client);

	i2c_set_clientdata(client, NULL);
	kfree(data);
	return 0;
}

static const struct i2c_device_id irda_ice40_id[] = {
	{"irda_ice40", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, barcode_id);

#ifdef CONFIG_OF
static struct of_device_id irda_ice40_match_table[] = {
	{ .compatible = "irda_ice40",},
	{ },
};
#else
#define irda_ice40_match_table	NULL
#endif

static struct i2c_driver ice40_i2c_driver = {
	.driver = {
		.name = "irda_ice40",
		.owner = THIS_MODULE,
		.of_match_table = irda_ice40_match_table,
	},
	.probe = irda_ice40_probe,
	.remove = irda_ice40_remove,
	.id_table = irda_ice40_id,
};

static int __init irda_ice40_init(void)
{
	pr_irda("%s\n", __func__);
	return i2c_add_driver(&ice40_i2c_driver);
}
module_init(irda_ice40_init);

static void __exit irda_ice40_exit(void)
{
	i2c_del_driver(&ice40_i2c_driver);
}
module_exit(irda_ice40_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SEC IrDA");
