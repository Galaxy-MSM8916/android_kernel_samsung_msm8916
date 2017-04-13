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

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#include <mach/hardware.h>

#include <asm/io.h>
#include <linux/ioport.h>

static struct dentry  *file;


static int efuse_open(struct inode *inode, struct file *filp)
{
	return 0; /* success */
}

static int efuse_release(struct inode *inode, struct file *filp)
{
	return 0; /* success */
}


static int read_word(unsigned long addr)
{
	const unsigned int size = 4;
	void *base = 0;
	unsigned int val = 0;

	if (check_mem_region(addr, size)) {
		printk(KERN_ERR "efuse: address already in use\n");
		return -2;
	}

	request_mem_region(addr, size, "efuse_test");
	base = ioremap(addr, size); /* get virtuall address */

	val = readl(base);

	iounmap(base);
	release_mem_region(addr, size);

	return (int)val;
}

#define EFUSE_IOC_MAGIC 'r'
#define EFUSE_GET_TICK_CNT   _IOW(EFUSE_IOC_MAGIC,  0, unsigned long)
#define EFUSE_GET_EFUSE      _IOW(EFUSE_IOC_MAGIC,  1, unsigned long)

static long efuse_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
	case EFUSE_GET_TICK_CNT:
		return read_word(0x10050100);

	case EFUSE_GET_EFUSE:
		return read_word(0x10100000);

	default:
		return -ENOTTY;
	}
}

static struct file_operations efuse_fops =
{
	.open =  efuse_open,
	.release = efuse_release,
	.unlocked_ioctl = efuse_ioctl,
};


static int __init efuse_module_init(void)
{
	file = debugfs_create_file("efuse_test", 0644, NULL, NULL, &efuse_fops);
	if(file <= 0) {
		printk(KERN_ERR "efuse test - file creation failed!!!\n");
		return PTR_ERR(file);
	}


	return 0;
}

static void __exit efuse_module_exit(void)
{
	debugfs_remove(file);
}

module_init(efuse_module_init);
module_exit(efuse_module_exit);

MODULE_AUTHOR("Oleg Isakov");
MODULE_DESCRIPTION("Read device key test module");
MODULE_LICENSE("GPL");


