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

#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#include "tz_mmap_test.h"

MODULE_AUTHOR("Alex Matveev <alex.matveev@samsung.com>");
MODULE_LICENSE("GPL");

static char tz_mmap_test_stat[PAGE_SIZE];
static void *tz_mmap_test_dyn_v;
static struct page *tz_mmap_test_page;

static long tz_mmap_test_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg )
{
	switch (cmd) {
	case TZ_MMAP_GET_PADDR_STATIC:
		return virt_to_phys(&tz_mmap_test_stat);
	case TZ_MMAP_GET_PADDR_DYNAMIC:
		return page_to_phys(tz_mmap_test_page);
	case TZ_MMAP_FILL_STATIC:
		memset(tz_mmap_test_stat, arg, PAGE_SIZE);
		return 0;
	case TZ_MMAP_FILL_DYNAMIC:
		memset(tz_mmap_test_dyn_v, arg, PAGE_SIZE);
		return 0;
	}

	return 0;
}

static const struct file_operations tz_mmap_test_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = tz_mmap_test_unlocked_ioctl
};

static struct miscdevice tz_mmap_test = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tz_mmap_test",
	.fops = &tz_mmap_test_fops,
};

static int __init tz_mmap_test_init(void)
{
	int rc;

	rc = misc_register(&tz_mmap_test);
	if (unlikely(rc))
		return rc;

	tz_mmap_test_page = alloc_page(GFP_KERNEL);
	if (!tz_mmap_test_page)
		return -ENOMEM;

	tz_mmap_test_dyn_v = page_address(tz_mmap_test_page);

	return 0;
}

static void __exit tz_mmap_test_exit(void)
{
	__free_page(tz_mmap_test_page);
	misc_deregister(&tz_mmap_test);
}

module_init(tz_mmap_test_init);
module_exit(tz_mmap_test_exit);
