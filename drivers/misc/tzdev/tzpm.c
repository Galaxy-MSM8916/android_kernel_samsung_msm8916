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

#include <linux/device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/timer.h>
#include "tzlog.h"
#include "tzpm.h"

/* Define a tzdev device structure for use with dev_debug() etc */
static struct device_driver tzdev_drv = {
	.name = "tzdev"
};

static struct device tzd = {
	.driver = &tzdev_drv
};

static struct device *tzdev_dev = &tzd;

static struct clk *tzdev_core_src = NULL;
static struct clk *tzdev_core_clk = NULL;
static struct clk *tzdev_iface_clk = NULL;
static struct clk *tzdev_bus_clk = NULL;

int tzdev_qc_pm_clock_initialize(void)
{
	int ret = 0;
	int freq_val = 0;

	tzdev_core_src = clk_get(tzdev_dev, "core_clk_src");
	if (IS_ERR(tzdev_core_src)) {
		tzdev_print(0, "no tzdev_core_src, ret = %d", ret);
		ret = PTR_ERR(tzdev_core_src);
		goto error;
	}
	if (of_property_read_u32(tzdev_dev->of_node,
				 "qcom,freq-val",
				 &freq_val)) {
		freq_val = QSEE_CE_CLK_100MHZ;
		tzdev_print(0, "unable to get frequency value with %d", freq_val);
	}
	ret = clk_set_rate(tzdev_core_src, freq_val);
	if (ret) {
		tzdev_print(0, "clk_set_rate failed, ret = %d", ret);
		ret = -EIO;
		goto put_core_src_clk;
	}

	tzdev_core_clk = clk_get(tzdev_dev, "core_clk");
	if (IS_ERR(tzdev_core_clk)) {
		tzdev_print(0, "no tzdev_core_clk");
		ret = PTR_ERR(tzdev_core_clk);
		goto clear_core_clk;
	}
	tzdev_iface_clk = clk_get(tzdev_dev, "iface_clk");
	if (IS_ERR(tzdev_iface_clk)) {
		tzdev_print(0, "no tzdev_iface_clk");
		ret = PTR_ERR(tzdev_iface_clk);
		goto put_core_clk;
	}
	tzdev_bus_clk = clk_get(tzdev_dev, "bus_clk");
	if (IS_ERR(tzdev_bus_clk)) {
		tzdev_print(0, "no tzdev_bus_clk");
		ret = PTR_ERR(tzdev_bus_clk);
		goto put_iface_clk;
	}

	tzdev_print(0, "Got QC HW crypto clks\n");
	return ret;

put_iface_clk:
	clk_put(tzdev_iface_clk);
	tzdev_bus_clk = NULL;

put_core_clk:
	clk_put(tzdev_core_clk);
	tzdev_iface_clk = NULL;

clear_core_clk:
	tzdev_core_clk = NULL;

put_core_src_clk:
	clk_put(tzdev_core_src);

error:
	tzdev_core_src = NULL;

	return ret;
}

void tzdev_qc_pm_clock_finalize(void)
{
	clk_put(tzdev_bus_clk);
	clk_put(tzdev_iface_clk);
	clk_put(tzdev_core_clk);
	clk_put(tzdev_core_src);
}

void tzdev_qc_pm_clock_enable(void)
{
	if (clk_prepare_enable(tzdev_core_clk)) {
		tzdev_print(0, "no core clk\n");
		return;
	} else if (clk_prepare_enable(tzdev_iface_clk)) {
		tzdev_print(0, "no iface clk\n");
		goto unprepare_core_clk;
	} else if (clk_prepare_enable(tzdev_bus_clk)) {
		tzdev_print(0, "no bus clk\n");
		goto unprepare_iface_clk;
	}

	return;

unprepare_iface_clk:
	clk_disable_unprepare(tzdev_iface_clk);
unprepare_core_clk:
	clk_disable_unprepare(tzdev_core_clk);

	return;
}

void tzdev_qc_pm_clock_disable(void)
{
	clk_disable_unprepare(tzdev_iface_clk);
	clk_disable_unprepare(tzdev_core_clk);
	clk_disable_unprepare(tzdev_bus_clk);
}

static int tzdev_qc_probe(struct platform_device *pdev)
{
	tzdev_dev->of_node = pdev->dev.of_node;
	return 0;
}

static int tzdev_qc_remove(struct platform_device *pdev)
{
	return 0;
}

static int tzdev_qc_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int tzdev_qc_resume(struct platform_device *pdev)
{
	return 0;
}

struct of_device_id tzdev_qc_match[] = {
	{
		.compatible = "qcom,tzd",
	},
	{}
};

struct platform_driver tzdev_qc_plat_driver = {
	.probe = tzdev_qc_probe,
	.remove = tzdev_qc_remove,
	.suspend = tzdev_qc_suspend,
	.resume = tzdev_qc_resume,

	.driver = {
		.name = "tzdev",
		.owner = THIS_MODULE,
		.of_match_table = tzdev_qc_match,
	},
};

int tzdev_platform_register(void)
{
	return platform_driver_register(&tzdev_qc_plat_driver);
}

void tzdev_platform_unregister(void)
{
	platform_driver_unregister(&tzdev_qc_plat_driver);
}
