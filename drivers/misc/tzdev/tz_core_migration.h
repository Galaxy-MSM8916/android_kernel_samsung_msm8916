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

#ifndef __TZ_CORE_MIGRATION_H__
#define __TZ_CORE_MIGRATION_H__

#include <linux/errno.h>

#if defined(CONFIG_TZ_CORE_MIGRATION)

void tzdev_init_migration(void);
void tzdev_fini_migration(void);
int tzdev_migrate(void);
int tzdev_migration_request(unsigned long arg);

#else

static inline void tzdev_init_migration(void)
{
	return;
}

static inline void tzdev_fini_migration(void)
{
	return;
}

static inline int tzdev_migrate(void)
{
	return -EINVAL;
}

static inline int tzdev_migration_request(unsigned long arg)
{
	(void)arg;

	return -EINVAL;
}

#endif /* CONFIG_TZ_CORE_MIGRATION */
#endif /* __TZ_CORE_MIGRATION_H__ */
