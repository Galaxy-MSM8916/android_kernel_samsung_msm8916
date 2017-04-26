/* include/linux/android_alarm.h
 *
 * Copyright (C) 2006-2007 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_ANDROID_ALARM_H
#define _LINUX_ANDROID_ALARM_H

#include <linux/compat.h>
#include <linux/ioctl.h>
#include <linux/time.h>
#include <linux/compat.h>

#include "uapi/android_alarm.h"

/* Disable alarm */
#define ANDROID_ALARM_CLEAR(type)           _IO('a', 0 | ((type) << 4))

/* Ack last alarm and wait for next */
#define ANDROID_ALARM_WAIT                  _IO('a', 1)

#define ALARM_IOW(c, type, size)            _IOW('a', (c) | ((type) << 4), size)
/* Set alarm */
#define ANDROID_ALARM_SET(type)             ALARM_IOW(2, type, struct timespec)
#define ANDROID_ALARM_SET_AND_WAIT(type)    ALARM_IOW(3, type, struct timespec)
#define ANDROID_ALARM_GET_TIME(type)        ALARM_IOW(4, type, struct timespec)
#define ANDROID_ALARM_SET_RTC               _IOW('a', 5, struct timespec)
#if defined(CONFIG_RTC_AUTO_PWRON)
#define ANDROID_ALARM_SET_ALARM_BOOT	    _IOW('a', 7, struct timespec)
#endif

#define ANDROID_ALARM_BASE_CMD(cmd)         (cmd & ~(_IOC(0, 0, 0xf0, 0)))
#define ANDROID_ALARM_IOCTL_TO_TYPE(cmd)    (_IOC_NR(cmd) >> 4)

#ifdef CONFIG_COMPAT
#define ANDROID_ALARM_SET_COMPAT(type)		ALARM_IOW(2, type, \
							struct compat_timespec)
#define ANDROID_ALARM_SET_AND_WAIT_COMPAT(type)	ALARM_IOW(3, type, \
							struct compat_timespec)
#define ANDROID_ALARM_GET_TIME_COMPAT(type)	ALARM_IOW(4, type, \
							struct compat_timespec)
#define ANDROID_ALARM_SET_RTC_COMPAT		_IOW('a', 5, \
							struct compat_timespec)
#define ANDROID_ALARM_IOCTL_NR(cmd)		(_IOC_NR(cmd) & ((1<<4)-1))
#define ANDROID_ALARM_COMPAT_TO_NORM(cmd)  \
				ALARM_IOW(ANDROID_ALARM_IOCTL_NR(cmd), \
					ANDROID_ALARM_IOCTL_TO_TYPE(cmd), \
					struct timespec)

#endif

#endif
