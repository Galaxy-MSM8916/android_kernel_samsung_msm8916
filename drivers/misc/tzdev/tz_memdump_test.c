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

#define DEBUG

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/highmem.h>
#include <linux/version.h>
#include <asm/io.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
#define tz_memdump_kmap_atomic(v)	kmap_atomic(v)
#define tz_memdump_kunmap_atomic(v)	kunmap_atomic(v)
#else
/* kmap/kumap_atomic with two arguments are deprecated */
#define tz_memdump_kmap_atomic(v)	kmap_atomic(v, KM_USER0)
#define tz_memdump_kunmap_atomic(v)	kunmap_atomic(v, KM_USER0)
#endif

ulong tz_memdump_phys = 0;
module_param_named(phys, tz_memdump_phys, ulong, 0644);
MODULE_PARM_DESC(phys, "phys address of memory to dump");

ulong tz_memdump_size = 0;
module_param_named(size, tz_memdump_size, ulong, 0644);
MODULE_PARM_DESC(size, "size of memory to dump");

static struct miscdevice tz_memdump_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tz_memdump_test",
};

static int __init tz_memdump_test_init(void)
{
	int ret;
	ulong off, off_in_page;
	void __iomem	*virt;
	ulong *ptr;
	int type = 0;

	if (!tz_memdump_phys) {
		pr_err("phys address should be specified and != 0\n");
		return -EINVAL;
	}

	if (!tz_memdump_size) {
		pr_err("size should be specified and != 0\n");
		return -EINVAL;
	}

	if ((tz_memdump_phys & PAGE_MASK) != tz_memdump_phys) {
		pr_err("phys address should be page size aligned\n");
		return -EINVAL;
	}

	if ((tz_memdump_size & PAGE_MASK) != tz_memdump_size) {
		pr_err("size should be page size aligned\n");
		return -EINVAL;
	}

	pr_err("tz_memdump_phys=0x%lx\n",tz_memdump_phys);
	pr_err("tz_memdump_size=0x%lx\n",tz_memdump_size);

	ret = misc_register(&tz_memdump_dev);
	if (ret) {
		pr_err("misc device registration failed\n");
		goto out;
	}

	type = pfn_valid(__phys_to_pfn(tz_memdump_phys));
	for (off = 0; off < tz_memdump_size; off += PAGE_SIZE) {
		if (type)
			virt = tz_memdump_kmap_atomic(phys_to_page(tz_memdump_phys + off));
		else
			virt = (void *)ioremap(tz_memdump_phys + off, PAGE_SIZE);

		if (!virt) {
			ret = -ENOMEM;
			goto out_unreg;
		}
		for (off_in_page = 0; off_in_page < PAGE_SIZE; off_in_page += sizeof(ulong) * 4) {
			ptr = virt + off_in_page;
			pr_err("[0x%08lx-0x%08lx]: 0x%08lx 0x%08lx 0x%08lx 0x%08lx\n",
				tz_memdump_phys + off + off_in_page,
				tz_memdump_phys + off + off_in_page + 0xf,
				ptr[0], ptr[1], ptr[2], ptr[3]);
		}
		if (type)
			tz_memdump_kunmap_atomic(virt);
		else
			iounmap(virt);
	}
	return 0;
out_unreg:
	misc_deregister(&tz_memdump_dev);
out:
	return ret;
}

static void __exit tz_memdump_test_exit(void)
{
	pr_devel("module exit\n");
	misc_deregister(&tz_memdump_dev);
}

module_init(tz_memdump_test_init);
module_exit(tz_memdump_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vasily Leonenko <v.leonenko@samsung.com>");
