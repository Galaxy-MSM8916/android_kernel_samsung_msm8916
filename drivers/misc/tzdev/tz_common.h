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

#ifndef __TZ_COMMON_H__
#define __TZ_COMMON_H__

#include <linux/types.h>
#include <linux/limits.h>

/*
 * CA_ID is a full basename of CA executable.
 * It can not be longer than max supported filename length defined in Linux.
 */
#define CA_ID_LEN			(256)

#define TZ_IOC_MAGIC			'c'

#define TZIO_SMC			_IOWR(TZ_IOC_MAGIC, 101, struct tzio_smc_cmd *)
#define TZIO_WAIT_EVT			_IOR(TZ_IOC_MAGIC, 112, int *)
#define TZIO_GET_PIPE			_IOR(TZ_IOC_MAGIC, 113, int *)
#define TZIO_GET_ACCESS_INFO		_IOWR(TZ_IOC_MAGIC, 119, struct tzio_access_info *)
#define TZIO_MEM_REGISTER		_IOWR(TZ_IOC_MAGIC, 120, struct tzio_mem_register *)
#define TZIO_MEM_RELEASE		_IOW(TZ_IOC_MAGIC, 121, int)
#define TZIO_MIGRATE			_IOW(TZ_IOC_MAGIC, 122, int)
#if defined(CONFIG_TZDEV_QC_CRYPTO_CLOCKS_USR_MNG)
#define TZIO_SET_QC_CLK			_IOW(TZ_IOC_MAGIC, 123, int)
#endif

struct tzio_access_info {
	pid_t pid;			/* CA PID */
	pid_t gid;			/* CA GID - to be received from TZDEV */
	char ca_name[CA_ID_LEN];	/* CA identity - to be received from TZDEV */
};

struct tzio_smc_cmd {
	__s32 args[2];			/* SMC args (in) */
	__s32 ret;			/* SMC return value (out) */
};

struct tzio_mem_register {
	pid_t pid;			/* Memory region owner's PID (in) */
	const void *ptr;		/* Memory region start (in) */
	size_t size;			/* Memory region size (in) */
	int id;				/* Memory region ID (out) */
	int write;			/* 1 - rw, 0 - ro */
};

#endif /*!__TZ_COMMON_H__*/
