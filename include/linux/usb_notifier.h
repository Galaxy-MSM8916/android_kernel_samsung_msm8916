/*
 *  usb notify header
 *
 * Copyright (C) 2011-2013 Samsung, Inc.
 * Author: Dongrak Shin <dongrak.shin@samsung.com>
 *
*/

#ifndef __LINUX_USB_NOTIFIER_H__
#define __LINUX_USB_NOTIFIER_H__

#include <linux/usb_notify.h>

enum chip {
	BAYTRAIL_NOTIFY,
	MERRIFIELD_NOTIFY,
	MOOREFIELD_NOTIFY,
};

struct usb_notifier_platform_data {
	int	gpio_redriver_en;
	int	gpio_vbus_detect;
	int	gpio_usb3_mux_sel;
	int	gpio_shdn_sel;
	int	gpio_usb_swtich;
	int	gpio_otg_en;
	char *booster_name;
	char *muic_name;
	int	chip_type;
};

#ifdef CONFIG_USB_NOTIFY_LAYER
extern int dwc3_intel_usb_handle_notification_sec
		(unsigned long event, bool enable);
#else
static inline int dwc3_intel_usb_handle_notification_sec
		(unsigned long event, bool enable) {return 0; }
#endif

#endif /* __LINUX_USB_NOTIFIER_H__ */

