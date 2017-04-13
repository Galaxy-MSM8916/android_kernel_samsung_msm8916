/*
 * Copyright (C) 2014 Samsung Electronics Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) "usb_notify: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/usb.h>
#include <linux/notifier.h>
#include <linux/version.h>
#include <linux/usb_notify.h>
#include "../core/hub.h"

#define SMARTDOCK_INDEX	1
#define MMDOCK_INDEX	2

struct dev_table {
	struct usb_device_id dev;
	int index;
};

static struct dev_table enable_notify_hub_table[] = {
	{ .dev = { USB_DEVICE(0x0424, 0x2514), },
	   .index = SMARTDOCK_INDEX,
	}, /* SMART DOCK HUB 1 */
	{ .dev = { USB_DEVICE(0x1a40, 0x0101), },
	   .index = SMARTDOCK_INDEX,
	}, /* SMART DOCK HUB 2 */
	{ .dev = { USB_DEVICE(0x0424, 0x9512), },
	   .index = MMDOCK_INDEX,
	}, /* SMSC USB LAN HUB 9512 */
	{}
};

static struct dev_table essential_device_table[] = {
	{ .dev = { USB_DEVICE(0x08bb, 0x2704), },
	   .index = SMARTDOCK_INDEX,
	}, /* TI USB Audio DAC 1 */
	{ .dev = { USB_DEVICE(0x08bb, 0x27c4), },
	   .index = SMARTDOCK_INDEX,
	}, /* TI USB Audio DAC 2 */
	{ .dev = { USB_DEVICE(0x0424, 0xec00), },
	   .index = MMDOCK_INDEX,
	}, /* SMSC LAN Driver */
	{}
};

static struct dev_table update_autotimer_device_table[] = {
	{ .dev = { USB_DEVICE(0x04e8, 0xa500), },
	   .index = 5, /* 5 sec timer */
	}, /* GearVR1 */
	{ .dev = { USB_DEVICE(0x04e8, 0xa501), },
	   .index = 5,
	}, /* GearVR2 */
	{ .dev = { USB_DEVICE(0x04e8, 0xa502), },
	   .index = 5,
	}, /* GearVR3 */
	{}
};

static int check_essential_device(struct usb_device *dev, int index)
{
	struct dev_table *id;
	int ret = 0;

	/* check VID, PID */
	for (id = essential_device_table; id->dev.match_flags; id++) {
		if ((id->dev.match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
		(id->dev.match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
		id->dev.idVendor == le16_to_cpu(dev->descriptor.idVendor) &&
		id->dev.idProduct == le16_to_cpu(dev->descriptor.idProduct) &&
		id->index == index) {
			ret = 1;
			break;
		}
	}
	return ret;
}

static int is_notify_hub(struct usb_device *dev)
{
	struct dev_table *id;
	struct usb_device *hdev;
	int ret = 0;

	hdev = dev->parent;
	if (!hdev)
		goto skip;
	/* check VID, PID */
	for (id = enable_notify_hub_table; id->dev.match_flags; id++) {
		if ((id->dev.match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
		(id->dev.match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
		id->dev.idVendor == le16_to_cpu(hdev->descriptor.idVendor) &&
		id->dev.idProduct == le16_to_cpu(hdev->descriptor.idProduct)) {
			ret = (hdev->parent &&
			(hdev->parent == dev->bus->root_hub)) ? id->index : 0;
			break;
		}
	}
skip:
	return ret;
}

static int get_autosuspend_time(struct usb_device *dev)
{
	struct dev_table *id;
	int ret = 0;

	/* check VID, PID */
	for (id = update_autotimer_device_table; id->dev.match_flags; id++) {
		if ((id->dev.match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
		(id->dev.match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
		id->dev.idVendor == le16_to_cpu(dev->descriptor.idVendor) &&
		id->dev.idProduct == le16_to_cpu(dev->descriptor.idProduct)) {
			ret = id->index;
			break;
		}
	}
	return ret;
}

static int call_battery_notify(struct usb_device *dev, bool on)
{
	struct usb_device *hdev;
	struct usb_device *udev;
	struct usb_hub *hub;
	struct otg_notify *o_notify = get_otg_notify();
	int index = 0;
	int count = 0;
	int port;

	index = is_notify_hub(dev);
	if (!index)
		goto skip;
	if (check_essential_device(dev, index))
		goto skip;

	hdev = dev->parent;
	hub = usb_hub_to_struct_hub(hdev);
	if (!hub)
		goto skip;

	for (port = 1; port <= hdev->maxchild; port++) {
		udev = hub->ports[port-1]->child;
		if (udev) {
			if (!check_essential_device(udev, index)) {
				if (!on && (udev == dev))
					continue;
				else
					count++;
			}
		}
	}

	pr_info("%s : VID : 0x%x, PID : 0x%x, on=%d, count=%d\n", __func__,
		dev->descriptor.idVendor, dev->descriptor.idProduct,
			on, count);
	if (on) {
		if (count == 1) {
			if (index == SMARTDOCK_INDEX)
				send_otg_notify(o_notify,
					NOTIFY_EVENT_SMTD_EXT_CURRENT, 1);
			else if (index == MMDOCK_INDEX)
				send_otg_notify(o_notify,
					NOTIFY_EVENT_MMD_EXT_CURRENT, 1);
		}
	} else {
		if (!count) {
			if (index == SMARTDOCK_INDEX)
				send_otg_notify(o_notify,
					NOTIFY_EVENT_SMTD_EXT_CURRENT, 0);
			else if (index == MMDOCK_INDEX)
				send_otg_notify(o_notify,
					NOTIFY_EVENT_MMD_EXT_CURRENT, 0);
		}
	}
skip:
	return 0;
}

static int update_hub_autosuspend_timer(struct usb_device *dev)
{
	struct usb_device *hdev;
	int time = 0;

	if (!dev)
		goto skip;

	hdev = dev->parent;

	if (hdev == NULL || dev->bus->root_hub != hdev)
		goto skip;

	/* hdev is root hub */
	time = get_autosuspend_time(dev);
	if (time == hdev->dev.power.autosuspend_delay)
		goto skip;

	pm_runtime_set_autosuspend_delay(&hdev->dev, time*1000);
	pr_info("set autosuspend delay time=%d sec\n", time);
skip:
	return 0;
}

static int dev_notify(struct notifier_block *self,
			       unsigned long action, void *dev)
{
	switch (action) {
	case USB_DEVICE_ADD:
		call_battery_notify(dev, 1);
		update_hub_autosuspend_timer(dev);
		break;
	case USB_DEVICE_REMOVE:
		call_battery_notify(dev, 0);
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block dev_nb = {
	.notifier_call = dev_notify,
};

void register_usbdev_notify(void)
{
	usb_register_notify(&dev_nb);
}
EXPORT_SYMBOL(register_usbdev_notify);
