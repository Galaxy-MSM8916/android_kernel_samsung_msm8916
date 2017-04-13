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

#define pr_fmt(fmt) "tzpc_test: " fmt
#define DEBUG

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/mm.h>
#include <linux/clk.h>
#include <asm/io.h>

#define MAP_ADDRESS	0x10830000
#define MAP_SIZE	0x1000
#define CLK_NAME	"secss"

static struct clk *clk;

static int tzpc_test_open(struct inode *inodp, struct file *filp)
{
	clk_enable(clk);

	return 0;
}

static int tzpc_test_release(struct inode *inodp, struct file *filp)
{
	clk_disable(clk);

	return 0;
}

static int tzpc_test_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long phys = MAP_ADDRESS + off;
	unsigned long vsize = vma->vm_end - vma->vm_start;
	unsigned long psize = MAP_SIZE - off;
	int ret;

	if (vsize > psize) {
		return -EINVAL;
	}

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	ret = remap_pfn_range(vma, vma->vm_start, __phys_to_pfn(phys), vsize, vma->vm_page_prot);
	if (ret) {
		goto out;
	}

	return 0;
out:
	return ret;
}

static struct file_operations tzpc_test_fops = {
	.open = tzpc_test_open,
	.release = tzpc_test_release,
	.mmap = tzpc_test_mmap,
};

static struct miscdevice tzpc_test_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tzpc_test",
	.fops = &tzpc_test_fops,
};

static int __init tzpc_test_init(void)
{
	int ret;

	pr_devel("module init\n");

	clk = clk_get(NULL, CLK_NAME);
	if (IS_ERR(clk)) {
		pr_err("Clock get failed\n");
		ret = -ENOENT;
		goto out;
	}

	ret = misc_register(&tzpc_test_dev);
	if (ret) {
		pr_err("misc device registration failed\n");
		goto out;
	}

	return 0;

out:
	return ret;
}

static void __exit tzpc_test_exit(void)
{
	pr_devel("module exit\n");
	misc_deregister(&tzpc_test_dev);
}

module_init(tzpc_test_init);
module_exit(tzpc_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Fedorov <s.fedorov@samsung.com>");
