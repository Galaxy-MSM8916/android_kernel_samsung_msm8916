/*
 * BU80003GUL NFC RF Controller
 *
 * Copyright (C) 2015 Samsung Electronics Co.Ltd
 * Author: Kyungmin Park <kmini.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/nfc/sec_nfc.h>
#include <linux/of_gpio.h>

/*
 * Security START
 *	#include <mach/scm.h>
 *	Needs to be changed as per the qualcomm's recommendation
 * Security END
 */

#include <linux/regulator/consumer.h>

#include <linux/wakelock.h>

#define BU80003GUL_DEBUG

#include "bu80003gul.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/i2c.h>

#ifdef BU80003GUL_DEBUG
#define EPC_DEBUG(fmt, args...)		printk(KERN_DEBUG fmt, ## args)
#define EPC_INFO(fmt, args...)		printk(KERN_INFO fmt, ## args)
#define EPC_ERR(fmt, args...)		printk(KERN_ERR fmt, ## args)
#else
#define EPC_DEBUG(fmt, args...)
#define EPC_INFO(fmt, args...)
#define EPC_ERR(fmt, args...)
#endif


#define I2C_ADDR (0x56 >> 1)
#define I2C_LOCK_ADDR 0x02
#define I2C_ANT_ADDR 0x01


/*	extern unsigned int system_rev;  Not using system_rev anymore	*/

#undef FEATURE_SET_DEFAULT_ANT_VAL

#ifdef CONFIG_NFC_EDC_TUNING
static unsigned char user_ant = 10;
#endif
static int felica_epc_ant_read(unsigned char *read_buff);
static int felica_epc_ant_write(char ant);
int felica_epc_reset(void);
static struct class *eeprom_class;
/*
 *	I2C device_id table
*/
static struct i2c_client *bu80003gul_i2c_client;
static const struct i2c_device_id bu80003gul_i2c_idtable[] = {
	{BU80003GUL_I2C_NAME, 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, bu80003gul_i2c_idtable);

/*
 * I2C match table
 */
static struct of_device_id bu80003gul_i2c_match_table[] = {
	{ .compatible = "felica,felica-i2c",},
	{},
};

/*
 * I2C driver
 */
static struct i2c_driver bu80003gul_i2c_driver = {
	.probe = bu80003gul_i2c_probe,
	.remove = bu80003gul_i2c_remove,
	.id_table = bu80003gul_i2c_idtable,
	.driver = {
				.name = BU80003GUL_I2C_NAME,
				.owner = THIS_MODULE,
				.of_match_table = bu80003gul_i2c_match_table,
			  },
};


/* felica_ant device related */

static dev_t dev_id_felica_epc;
static struct cdev cdev_felica_epc;
static const struct file_operations fops_felica_epc = {
	.owner = THIS_MODULE,
	.open = felica_epc_open,
	.read = felica_epc_read,
	.write = felica_epc_write,
	/* .unlocked_ioctl = felica_epc_ioctl, */
	.release = felica_epc_close,
};

int felica_epc_set_lock_state(int state)
{
	int ret;
	unsigned char write_buff[2];
	struct i2c_msg write_msgs[] = {
		{
			.addr	= I2C_ADDR,
			.flags	= 0,
			.len	= 2,
			.buf	= NULL,
		},
	};

	write_buff[0] = 0x02;
	write_buff[1] = state;
	write_msgs[0].buf = &write_buff[0];

	ret = i2c_transfer(bu80003gul_i2c_client->adapter, write_msgs, 1);
	if (ret < 0) {
		EPC_ERR(" %s ERROR(i2c_transfer), ret=[%d]",
				__func__, ret);
		return -EIO;
	}

	return ret;
}

int felica_epc_reset(void)
{
	int ret;
	unsigned char write_buff[2];
	struct i2c_msg write_msgs[] = {
		{
			.addr	= I2C_ADDR,
			.flags	= 0,
			.len	= 2,
			.buf	= NULL,
		},
	};

	write_buff[0] = 0x00;
	write_buff[1] = 1;
	write_msgs[0].buf = &write_buff[0];

	ret = i2c_transfer(bu80003gul_i2c_client->adapter, write_msgs, 1);
	if (ret < 0) {
		EPC_ERR(" %s ERROR(i2c_transfer), ret=[%d]",
				__func__, ret);
		return -EIO;
	}

	return ret;
}
static int felica_epc_register(void)
{
	struct device *device_felica_epc;
	int ret;

	dev_id_felica_epc = MKDEV(FELICA_EPC_MAJOR, FELICA_EPC_MINOR);
	ret = alloc_chrdev_region(&dev_id_felica_epc, FELICA_EPC_BASEMINOR,
				FELICA_EPC_MINOR_COUNT, FELICA_EPC_NAME);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s ERROR(alloc_chrdev_region), ret=[%d]",
			       __func__, ret);
		return ret;
	}

	cdev_init(&cdev_felica_epc, &fops_felica_epc);
	ret = cdev_add(&cdev_felica_epc, dev_id_felica_epc, FELICA_EPC_MINOR_COUNT);
	if (ret < 0) {
		unregister_chrdev_region(dev_id_felica_epc, FELICA_EPC_MINOR_COUNT);
		EPC_ERR("[MFDD] %s ERROR(cdev_add), ret=[%d]", __func__, ret);
		return -EIO;
	}

	eeprom_class = class_create(THIS_MODULE, "felica_eeprom");
	if (IS_ERR(eeprom_class)) {
		EPC_ERR("[MFDD] %s ERROR(class_create)", __func__);
		return PTR_ERR(eeprom_class);
	}

	device_felica_epc = device_create(eeprom_class, NULL,
								dev_id_felica_epc, NULL,
								FELICA_EPC_NAME);
	if (IS_ERR(device_felica_epc)) {
		cdev_del(&cdev_felica_epc);
		unregister_chrdev_region(dev_id_felica_epc, FELICA_EPC_MINOR_COUNT);
		EPC_ERR("[MFDD] %s ERROR(device_create)", __func__);
		return -EINVAL;
	}

	EPC_DEBUG("[MFDD] %s END, major=[%d], minor=[%d]", __func__,
			 MAJOR(dev_id_felica_epc), MINOR(dev_id_felica_epc));

	return 0;
}

static void felica_epc_deregister(void)
{
	EPC_DEBUG("[MFDD] %s START", __func__);
	device_destroy(eeprom_class, dev_id_felica_epc);
	cdev_del(&cdev_felica_epc);
	unregister_chrdev_region(dev_id_felica_epc, FELICA_EPC_MINOR_COUNT);
	EPC_DEBUG("[MFDD] %s END", __func__);
	return;
}


static int felica_epc_open(struct inode *inode, struct file *file)
{
	if (felica_epc_set_lock_state(0x81) < 0)
		EPC_ERR("[MFDD} Failed to UnLock the EEPROM.\n");

	return 0;
}

static int felica_epc_close(struct inode *inode, struct file *file)
{
	if (felica_epc_set_lock_state(0x80) < 0)
		EPC_ERR("[MFDD} Failed to UnLock the EEPROM.\n");

	return 0;
}

static int felica_epc_ant_read(unsigned char *read_buff)
{
	int ret;
	unsigned char address = I2C_ANT_ADDR;
	struct i2c_msg read_msgs[] = {
		{
			.addr	= I2C_ADDR,
			.flags	= 0,
			.len	= 1,
			.buf	= &address,
		},
		{
			.addr	= I2C_ADDR,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= read_buff,
		},
	};

	if (bu80003gul_i2c_client == NULL) {
		EPC_ERR("[MFDD] bu80003gul_i2c_client is NULL %s -EIO", __func__);
		return -EIO;
	}

	*read_buff = 0;
	ret = i2c_transfer(bu80003gul_i2c_client->adapter, &read_msgs[0], 1);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s ERROR(i2c_transfer[0]), ret=[%d]",
				__func__, ret);
		return -EIO;
	}
	ret = i2c_transfer(bu80003gul_i2c_client->adapter, &read_msgs[1], 1);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s ERROR(i2c_transfer[1]), ret=[%d]",
				__func__, ret);
		return -EIO;
	}

	pr_info("%s : ant : %d\n", __func__, *read_buff);

	return 0;

}

static int felica_epc_ant_write(char ant)
{
	int ret = 0;
	char write_buff[2];
	struct i2c_msg write_msgs[] = {
		{
			.addr	= I2C_ADDR,
			.flags	= 0,
			.len	= 2,
			.buf	= NULL,
		},
	};

	if (bu80003gul_i2c_client == NULL) {
		EPC_ERR("[MFDD] bu80003gul_i2c_client is NULL %s", __func__);
		return -EIO;
	}

	/* why is bit 7 set ? */
	pr_info("%s : ant : %d\n", __func__, ant&0x7F);

	write_buff[0] = I2C_ANT_ADDR;
	write_buff[1] = ant;
	write_msgs[0].buf = &write_buff[0];

	ret = i2c_transfer(bu80003gul_i2c_client->adapter, &write_msgs[0], 1);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s ERROR(i2c_transfer), ret=[%d]",
				__func__, ret);
		return -EIO;
	}

	return ret;
}

static ssize_t felica_epc_read(struct file *file, char __user *buf,
		size_t len, loff_t *ppos)
{
	int ret;
	unsigned char read_buff = 0;

	ret = felica_epc_ant_read(&read_buff);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s felica_epc_ant_read fail, ret=[%d]",
				__func__, ret);
		return -EFAULT;
	}

	ret = copy_to_user(buf, &read_buff, len);
	if (ret != 0) {
		EPC_ERR("[MFDD] %s ERROR(copy_to_user), ret=[%d]",
				__func__, ret);
		return -EFAULT;
	}

	*ppos += 1;
	EPC_DEBUG("[MFDD] %s END\n", __func__);

	return 1; /* Only one byte at a time will be read. Hence return 1 on success. */

}

static ssize_t felica_epc_write(struct file *file, const char __user *data,
		size_t len, loff_t *ppos)
{
	int ret;
	char ant;

	if (len > sizeof(ant))
		len = sizeof(ant);

	ret = copy_from_user(&ant, data, len);
	if (ret != 0) {
		EPC_ERR("[MFDD] %s ERROR(copy_from_user), ret=[%d]",
				__func__, ret);
		return -EFAULT;
	}

	ret = felica_epc_ant_write(ant);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s felica_epc_ant_write fail, ret=[%d]",
				__func__, ret);
		return -EFAULT;
	}

#ifdef CONFIG_NFC_EDC_TUNING
	user_ant = ant & 0x7F;
#endif
	EPC_DEBUG("[MFDD] %s END\n", __func__);

	return 1;
}

#ifdef CONFIG_NFC_EDC_TUNING
static void felica_ant_tuning_work(struct work_struct *work)
{
	int i, ret;
	char ant;

	ant = user_ant;
	for (i = 0; i < 10; i++) {
		if (ant_tune_req != 1)
			break;
		ant = ant > 2 ? ant - 2 : 1;
		ret = felica_epc_ant_write(ant);
		pr_info("%s : felica_tune_work ant: %d\n", __func__, ant);
		msleep(1000);
	}
	ret = felica_epc_ant_write(user_ant);
	ant_tune_req = 0;
}
static DECLARE_DELAYED_WORK(felica_ant_work, felica_ant_tuning_work);
int felica_ant_tuning(int evt)
{
	pr_info("%s : felica_tune_req : %d, event: %d\n", __func__, ant_tune_req, evt);

	ant_tune_req = evt;
	if (evt == 1) {
		schedule_delayed_work(&felica_ant_work, 0);
	}

	return 1;
}
EXPORT_SYMBOL(felica_ant_tuning);
#endif

/*
 * Name : bu80003gul_i2c_probe
 * Description : Probe the I2C device.
 */
static int bu80003gul_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *devid)
{
	int ret;

	EPC_DEBUG("[MFDD] %s START", __func__);

	bu80003gul_i2c_client = client;
	if (!bu80003gul_i2c_client) {
		EPC_ERR("[MFDD] %s ERROR(bu80003gul_i2c_client==NULL)",
					__func__);
		return -EINVAL;
	}

	ret = felica_epc_register();
	if (ret < 0) {
		EPC_ERR("[MFDD] %s Failed to register the device[ret:%d]\n",
				__func__, ret);
	}

#ifdef FEATURE_SET_DEFAULT_ANT_VAL
	/* set default value temporarily */
	ret = felica_epc_ant_write(FEATURE_SET_DEFAULT_ANT_VAL);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s felica_epc_ant_write fail, ret=[%d]",
				__func__, ret);
		return -EFAULT;
	}
#endif
#ifdef CONFIG_NFC_EDC_TUNING
        ret = felica_epc_reset();
        if (ret < 0) {
                EPC_ERR("[MFDD] %s felica_epc_reset fail, ret=[%d]",
                                __func__, ret);
        }
	ret = felica_epc_ant_read(&user_ant);
	if (ret < 0) {
		EPC_ERR("[MFDD] %s felica_epc_ant_read fail, ret=[%d]",
				__func__, ret);
		user_ant = 10;
		return -EFAULT;
	}
	pr_info("%s : felica_ant : %d\n", __func__, user_ant);
#endif

	EPC_DEBUG("[MFDD] %s END", __func__);

	return 0;
}

/*
 * Name : bu80003gul_i2c_remove
 * Description : Remove the I2C device.
 */
static int bu80003gul_i2c_remove(struct i2c_client *client)
{
	bu80003gul_i2c_client = NULL;
	EPC_DEBUG("[MFDD] %s END", __func__);
	i2c_del_driver(&bu80003gul_i2c_driver);
	felica_epc_deregister();
	class_destroy(eeprom_class);
#ifdef CONFIG_NFC_EDC_TUNING
	ant_tune_req = 0;
	flush_delayed_work(&felica_ant_work);
#endif
	return 0;
}

#ifndef	CONFIG_SEC_NFC_I2C
module_i2c_driver(bu80003gul_i2c_driver);
#endif