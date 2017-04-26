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

#ifndef _TZISR_IOCTL_H_
#define _TZISR_IOCTL_H_

#define IOC_MAGIC       'h'
#define TZIRS_NAME      "tzirs"
#define TZIRS_DEV       "/dev/"TZIRS_NAME

typedef enum {
	IRS_SET_FLAG_CMD        =           1,
	IRS_SET_FLAG_VALUE_CMD,
	IRS_INC_FLAG_CMD,
	IRS_GET_FLAG_VAL_CMD,
	IRS_ADD_FLAG_CMD,
	IRS_DEL_FLAG_CMD
} TZ_ISR_CMD;

typedef struct isr_ctx_s {
	volatile uint32_t id;            /* r1 - Flag ID */
	volatile uint32_t func_cmd;      /* r2 - Function CMD */
	volatile uint32_t value;         /* r3 - Value or irs_flag.param (IOCTL_ADD_FLAG) */
} isr_ctx_t;

#define IOCTL_IRS_CMD           _IOWR(IOC_MAGIC, 1, isr_ctx_t)

#endif /* _TZISR_IOCTL_H_ */
