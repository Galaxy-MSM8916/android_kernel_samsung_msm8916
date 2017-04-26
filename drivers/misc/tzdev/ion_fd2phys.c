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

#define pr_fmt(fmt) "IONFD2PHYS: " fmt
/* #define DEBUG */

#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ion.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>

#if defined(CONFIG_ARCH_MSM)
#include <linux/msm_ion.h>
#endif

/* Define type for exchange with Secure kernel */
#ifdef CONFIG_TZDEV_32BIT_SECURE_KERNEL
typedef	u32	sk_pfn_t;
#else
typedef	u64	sk_pfn_t;
#endif

struct ionfd2phys {
	int fd;
	size_t nr_pfns;
	sk_pfn_t *pfns;
};

#ifdef CONFIG_COMPAT
struct ionfd2phys32 {
	int fd;
	unsigned int nr_pfns;
	u32 pfns;
};
#endif

#if defined(CONFIG_ARCH_EXYNOS)
extern struct ion_device *ion_exynos;
#endif

static struct ion_client *client;

static long __ionfd2phys_ioctl(int fd, size_t nr_pfns, sk_pfn_t *pfns)
{
	int ret;
	struct ion_handle *handle;
	void *addr;
	int pfn;
	size_t size = 0;

	handle = ion_import_dma_buf(client, fd);
	if (IS_ERR_OR_NULL(handle)) {
		pr_err("Failed to import an ION FD\n");
		ret = handle ? PTR_ERR(handle) : -EINVAL;
		goto out;
	}

	ret = ion_phys(client, handle, (ion_phys_addr_t *)&addr, &size);
	if (ret) {
		pr_err("Failed ION FD: ion_phys()\n");
		addr = ion_map_kernel(client, handle);
		if (IS_ERR_OR_NULL(addr)) {
			pr_err("Failed to map an ION FD\n");
			ret = addr ? PTR_ERR(addr) : -EINVAL;
			goto handle_free;
		}

		/* There is no kernel public API for getting imported buffer
		*  size. So just use what user has supplied */
		for (pfn = 0; pfn < nr_pfns; ++pfn) {
			pfns[pfn] = vmalloc_to_pfn(addr);
			addr += PAGE_SIZE;
		}
		ion_unmap_kernel(client, handle);
	} else {
		for (pfn = 0; pfn < nr_pfns; ++pfn) {
			pfns[pfn] = (sk_pfn_t)((unsigned long)addr >> PAGE_SHIFT);
			addr += PAGE_SIZE;
		}
	}

	ret = 0;

handle_free:
	ion_free(client, handle);
out:
	return ret;
}

static long ionfd2phys_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret;
	struct ionfd2phys data;
	sk_pfn_t *pfns;

	if (copy_from_user(&data, (void __user *)arg, sizeof(data))) {
		pr_err("Failed ION FD: copy_from_user()\n");
		ret = -EFAULT;
		goto out;
	}

	pfns = kzalloc(data.nr_pfns * sizeof(sk_pfn_t), GFP_KERNEL);
	if (!pfns) {
		pr_err("Failed ION FD: kzalloc(%zu)\n", data.nr_pfns);
		ret = -ENOMEM;
		goto out;
	}

	ret = __ionfd2phys_ioctl(data.fd, data.nr_pfns, pfns);
	if (ret) {
		pr_err("Failed ION FD: __ionfd2phys_ioctl()\n");
		goto pfns_free;
	}

	if (copy_to_user((void __user *)data.pfns, pfns,
				data.nr_pfns * sizeof(sk_pfn_t))) {
		pr_err("Failed ION FD: copy_to_user()\n");
		ret = -EFAULT;
		goto pfns_free;
	}

pfns_free:
	kfree(pfns);
out:
	return ret;
}

#ifdef CONFIG_COMPAT
static long compat_ionfd2phys_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret;
	struct ionfd2phys32 data;
	sk_pfn_t *pfns;

	if (copy_from_user(&data, (void __user *)arg, sizeof(data))) {
		pr_err("Failed ION FD: copy_from_user()\n");
		ret = -EFAULT;
		goto out;
	}

	pfns = kzalloc(data.nr_pfns * sizeof(sk_pfn_t), GFP_KERNEL);
	if (!pfns) {
		pr_err("Failed ION FD: kzalloc(%d)\n", data.nr_pfns);
		ret = -ENOMEM;
		goto out;
	}

	ret = __ionfd2phys_ioctl(data.fd, (size_t)data.nr_pfns, pfns);
	if (ret) {
		pr_err("Failed ION FD: __ionfd2phys_ioctl()\n");
		goto pfns_free;
	}

	if (copy_to_user(compat_ptr(data.pfns), pfns,
				data.nr_pfns * sizeof(sk_pfn_t))) {
		pr_err("Failed ION FD: copy_to_user()\n");
		ret = -EFAULT;
		goto pfns_free;
	}

pfns_free:
	kfree(pfns);
out:
	return ret;
}
#endif

static struct file_operations ionfd2phys_fops = {
	.unlocked_ioctl = ionfd2phys_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_ionfd2phys_ioctl,
#endif
};

static struct miscdevice ionfd2phys_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ionfd2phys",
	.fops = &ionfd2phys_fops,
};

static ssize_t system_heap_id_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
#if defined(CONFIG_ARCH_EXYNOS)
	return sprintf(buf, "%d\n", ION_HEAP_TYPE_SYSTEM);
#elif defined(CONFIG_ARCH_MSM)
	return sprintf(buf, "%d\n", ION_SYSTEM_HEAP_ID);
#endif
	return -ENOSYS;
}

static DEVICE_ATTR(system_heap_id, S_IRUGO, system_heap_id_show, NULL);

static int __init ionfd2phys_init(void)
{
	int ret;

	pr_devel("module init\n");
#if defined(CONFIG_ARCH_EXYNOS)
	client = ion_client_create(ion_exynos, "IONFD2PHYS");
#elif defined(CONFIG_ARCH_MSM)
/* The msm_ion_client_create() function has two parameters for old version of
 * linux kernel. This parameter is not used by function. In more fresh version
 * of linux kernel the function contains only one parameter. New version of
 * linux kernel is used for MSM8939, so parameter is 1
 */
#if defined(CONFIG_ARCH_MSM8939)
	client = msm_ion_client_create("IONFD2PHYS");
#else
	client = msm_ion_client_create(-1, "IONFD2PHYS");
#endif
#endif
	if (IS_ERR_OR_NULL(client)) {
		pr_err("Failed to create an ION client\n");
		ret = client ? PTR_ERR(client) : -ENOSYS;
		goto out;
	}

	ret = misc_register(&ionfd2phys_dev);
	if (ret) {
		pr_err("Failed to register misc device\n");
		goto ion_client_destroy;
	}

	ret = device_create_file(ionfd2phys_dev.this_device,
			&dev_attr_system_heap_id);
	if (ret) {
		pr_err("system_heap_id sysfs file creation failed\n");
		goto out_deregister;
	}

	return 0;

out_deregister:
	misc_deregister(&ionfd2phys_dev);
ion_client_destroy:
	ion_client_destroy(client);
out:
	return ret;
}

static void __exit ionfd2phys_exit(void)
{
	pr_devel("module exit\n");
	misc_deregister(&ionfd2phys_dev);
	ion_client_destroy(client);
}

module_init(ionfd2phys_init);
module_exit(ionfd2phys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Fedorov <s.fedorov@samsung.com>");
