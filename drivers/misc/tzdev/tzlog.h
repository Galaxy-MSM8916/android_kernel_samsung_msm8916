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

#ifndef __TZLOG_H__
#define __TZLOG_H__

extern int tzdev_verbosity;
#define tzdev_print(lvl, fmt, ...) \
	do { \
		if (lvl <= tzdev_verbosity) \
			printk("%s: "fmt, __func__, ##__VA_ARGS__); \
	} while (0)

#endif /* __TZLOG_H__ */
