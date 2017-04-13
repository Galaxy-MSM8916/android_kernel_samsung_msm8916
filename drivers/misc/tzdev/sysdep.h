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

#ifndef __SYSDEP_H__
#define __SYSDEP_H__

#include <linux/idr.h>
#include <linux/version.h>

#include <asm/cacheflush.h>

#if defined(CONFIG_ARM64)
#define outer_inv_range(s, e)
#else
#define __flush_dcache_area(s, e)	__cpuc_flush_dcache_area(s, e)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
#define IDR_REMOVE_ALL(id)
#else
#define IDR_REMOVE_ALL(id)	idr_remove_all(id)
#endif

int sysdep_idr_alloc(struct idr *idr, void *mem);

#endif /* __SYSDEP_H__ */
