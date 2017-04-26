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

#ifndef __TZPM_H__
#define __TZPM_H__

#define QSEE_CE_CLK_100MHZ      100000000
#define QSEE_CLK_ON             0x1
#define QSEE_CLK_OFF            0x0

/* Initialize secure crypto clocks */
int tzdev_qc_pm_clock_initialize(void);
/* Free secure crypto clocks */
void tzdev_qc_pm_clock_finalize(void);
/* Enable secure crypto clocks */
void tzdev_qc_pm_clock_enable(void);
/* Disable secure crypto clocks */
void tzdev_qc_pm_clock_disable(void);

/* Register tzdev as platform device */
int tzdev_platform_register(void);
/* Unregister tzdev platform device */
void tzdev_platform_unregister(void);

#endif /* __TZPM_H__ */
