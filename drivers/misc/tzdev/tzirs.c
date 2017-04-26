/*
 * Copyright (C) 2015 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include "tzirs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleksii Mosolab");

static int tzirs_fd_open = 0;
static DEFINE_MUTEX(tzirs_lock);

static int tzisr_open(struct inode *n, struct file *f)
{
	int ret = 0;

	mutex_lock(&tzirs_lock);
	if (tzirs_fd_open) {
		ret = -EBUSY;
		goto out;
	}
	tzirs_fd_open++;
	DBG("open\n");

out:
	mutex_unlock(&tzirs_lock);
	return ret;
}

static inline int tzisr_release(struct inode *inode, struct file *file)
{
	mutex_lock(&tzirs_lock);
	if (tzirs_fd_open)
		tzirs_fd_open--;
	mutex_unlock(&tzirs_lock);
	DBG("release\n");
	return 0;
}

static long tzisr_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *ioargp = (void __user *) arg;
	isr_ctx_t ctx = {0};

	if ( _IOC_TYPE(cmd) != IOC_MAGIC ) {
		ERR("INVALID CMD = %d\n", cmd);
		return -ENOTTY;
	}
	switch (cmd) {
	case IOCTL_IRS_CMD:
		DBG("IOCT_IRS_CMD\n");
		/* get flag id */
		ret = copy_from_user(&ctx, ioargp, sizeof(ctx));
		if (ret != 0) {
			ERR("IRS_CMD copy_from_user failed, ret = 0x%08x\n", ret);
			return -EFAULT;
		}

		DBG("IRS_CMD before: id = 0x%08x, cmd = 0x%08x, value = 0x%08x\n", ctx.id, ctx.func_cmd, ctx.value);

		ret = tzirs_smc(&(ctx.id), &(ctx.value), &(ctx.func_cmd));
		if (ret) {
			ERR("Unable to send IRS_CMD : id = 0x%08x, ret = %d\n", ctx.id, ret);
			return -EFAULT;
		}

		DBG("IRS_CMD after: id = 0x%08x, cmd = 0x%08x, value = 0x%08x\n", ctx.id, ctx.func_cmd, ctx.value);

		ret = copy_to_user(ioargp, &ctx, sizeof(ctx));
		if (ret != 0) {
			ERR("IRS_CMD copy_to_user failed, ret = 0x%08x\n", ret);
			return -EFAULT;
		}
		break;

	default:
		ERR("UNKNOWN CMD, cmd = 0x%08x\n", cmd);
		return -ENOTTY;
	}

	return 0;
}

static const struct file_operations misc_fops = {
	.owner = THIS_MODULE,
	.open = tzisr_open,
	.unlocked_ioctl = tzisr_unlocked_ioctl,
	.release = tzisr_release,
};

static struct miscdevice tzisr_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	TZIRS_NAME,
	&misc_fops,
};

static int __init test_init(void)
{
	int ret = 0;

	ret = misc_register(&tzisr_dev);
	if (ret) {
		ERR("Unable to register TZIRS driver, minor = 0x%08x, ret = 0x%08x\n", tzisr_dev.minor, ret);
		return ret;
	}

	DBG("INSTALLED\n");
	return 0;
}

static void __exit test_exit(void)
{
	misc_deregister(&tzisr_dev);
	DBG("REMOVED\n");
}

module_init(test_init);
module_exit(test_exit);
