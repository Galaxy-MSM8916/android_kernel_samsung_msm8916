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

#define pr_fmt(fmt) "tzasc_test: " fmt
#define DEBUG

#include <linux/version.h> /* for linux kernel version */
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/highmem.h>

#include "sysdep.h"

static char tzasc_buf[(PAGE_SIZE << (CONFIG_TZASC_TEST_PAGE_ORDER + 1)) - 1];
static phys_addr_t tzasc_buf_phys;

static int tzasc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	return remap_pfn_range(vma, vma->vm_start, __phys_to_pfn(tzasc_buf_phys) + vma->vm_pgoff,
			vma->vm_end - vma->vm_start, vma->vm_page_prot);
}

static struct file_operations tzasc_fops = {
	.mmap = tzasc_mmap,
};

static struct miscdevice tzasc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tzasc_test",
	.fops = &tzasc_fops,
};

static ssize_t physattr_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%#08llx\n", (unsigned long long)tzasc_buf_phys);
};

static DEVICE_ATTR(physaddr, S_IRUSR | S_IRGRP, physattr_show, NULL);

static int __init tzasc_test_init(void)
{
	int ret;

	tzasc_buf_phys = ALIGN(virt_to_phys(tzasc_buf), PAGE_SIZE << CONFIG_TZASC_TEST_PAGE_ORDER);

	__flush_dcache_area(phys_to_virt(tzasc_buf_phys),
			PAGE_SIZE << CONFIG_TZASC_TEST_PAGE_ORDER);
	outer_inv_range(tzasc_buf_phys, tzasc_buf_phys +
			(PAGE_SIZE << CONFIG_TZASC_TEST_PAGE_ORDER));

	ret = misc_register(&tzasc_dev);
	if (ret) {
		pr_err("misc device registration failed\n");
		goto out;
	}

	ret = device_create_file(tzasc_dev.this_device, &dev_attr_physaddr);
	if (ret) {
		pr_err("physaddr sysfs file creation failed\n");
		goto out_deregister;
	}

	return 0;

out_deregister:
	misc_deregister(&tzasc_dev);
out:
	return ret;
}

static void __exit tzasc_test_exit(void)
{
	device_remove_file(tzasc_dev.this_device, &dev_attr_physaddr);
	misc_deregister(&tzasc_dev);
}

module_init(tzasc_test_init);
module_exit(tzasc_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Fedorov <s.fedorov@samsung.com>");
