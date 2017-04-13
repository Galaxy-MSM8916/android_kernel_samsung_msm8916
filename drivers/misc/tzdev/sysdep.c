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

#include <linux/gfp.h>
#include <linux/spinlock.h>

#include "sysdep.h"

int sysdep_idr_alloc(struct idr *idr, void *mem)
{
	int res;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
	res = idr_alloc(idr, mem, 1, 0, GFP_KERNEL);
#else
	int tmp_id;
	do {
		if (!idr_pre_get(idr, GFP_KERNEL)) {
			res = -ENOMEM;
			break;
		}

		/* Reserving zero value helps with special case handling elsewhere */
		res = idr_get_new_above(idr, mem, 1, &tmp_id);
		if (res != -EAGAIN)
			break;
	} while (1);

	/* Simulate behaviour of new version */
	res = res ? -ENOMEM : tmp_id;
#endif
	return res;
}
