/*
 * Copyright (C) 2014 Samsung Electronics Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) "usb_notifier: " fmt

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/usb_notify.h>
#ifdef CONFIG_OF
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif
#ifdef CONFIG_MUIC_NOTIFIER
#include <linux/muic/muic.h>
#include <linux/muic/muic_notifier.h>
#endif
#ifdef CONFIG_VBUS_NOTIFIER
#include <linux/vbus_notifier.h>
#endif
#include <linux/battery/sec_charging_common.h>

struct usb_notifier_platform_data {
	struct	notifier_block usb_nb;
	struct	notifier_block vbus_nb;
	int	gpio_redriver_en;
	int disable_control_en;
	int	gpio_otg_en;
};

extern int sec_set_host(bool enable);

#ifdef CONFIG_OF
static void of_get_usb_redriver_dt(struct device_node *np,
		struct usb_notifier_platform_data *pdata)
{
	pdata->gpio_redriver_en = of_get_named_gpio(np, "qcom,gpios_redriver_en", 0);
	if (pdata->gpio_redriver_en < 0)
		pr_info("There is not USB 3.0 redriver pin\n");

	if (!gpio_is_valid(pdata->gpio_redriver_en))
		pr_err("%s: usb30_redriver_en: Invalied gpio pins\n", __func__);

	pr_info("redriver_en : %d\n", pdata->gpio_redriver_en);
}

static void of_get_disable_control_en_dt(struct device_node *np,
		struct usb_notifier_platform_data *pdata)
{
	of_property_read_u32(np, "qcom,disable_control_en", &pdata->disable_control_en);

	pr_info("disable_control_en : %d\n", pdata->disable_control_en);
}

static void of_get_otg_en_dt(struct device_node *np,
		struct usb_notifier_platform_data *pdata)
{
	pdata->gpio_otg_en = of_get_named_gpio(np, "qcom,gpios_otg_en", 0);
	if (pdata->gpio_otg_en < 0) {
		pr_info("There is no otg_en pin\n");
		return;
	}

	if (!gpio_is_valid(pdata->gpio_otg_en)) {
		pr_err("%s: gpios_otg_en : Invalied gpio pins\n", __func__);
		return;
	}

	gpio_set_value(pdata->gpio_otg_en, 0);
	pr_info("gpio_get_value(%d) = %d\n", pdata->gpio_otg_en, gpio_get_value(pdata->gpio_otg_en));
}

static int of_usb_notifier_dt(struct device *dev,
		struct usb_notifier_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	if (!np)
		return -EINVAL;

	of_get_usb_redriver_dt(np, pdata);
	of_get_disable_control_en_dt(np, pdata);
	of_get_otg_en_dt(np, pdata);
	return 0;
}
#endif

#ifdef CONFIG_MUIC_NOTIFIER
static int usb_handle_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	muic_attached_dev_t attached_dev = *(muic_attached_dev_t *)data;
	struct otg_notify *o_notify;

	o_notify = get_otg_notify();

	pr_info("%s action=%lu, attached_dev=%d\n",
		__func__, action, attached_dev);

	switch (attached_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_USB_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_CDP_MUIC:
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_VBUS, 0);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_VBUS, 1);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_OTG_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH) {
			send_otg_notify(o_notify, NOTIFY_EVENT_HOST, 0);
			send_otg_notify(o_notify, NOTIFY_EVENT_DRIVE_VBUS, 0);
		} else if (action == MUIC_NOTIFY_CMD_ATTACH) {
			send_otg_notify(o_notify, NOTIFY_EVENT_DRIVE_VBUS, 1);
			send_otg_notify(o_notify, NOTIFY_EVENT_HOST, 1);
		} else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_HMT_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH) {
			send_otg_notify(o_notify, NOTIFY_EVENT_HMT, 0);
			send_otg_notify(o_notify, NOTIFY_EVENT_DRIVE_VBUS, 0);
		} else if (action == MUIC_NOTIFY_CMD_ATTACH) {
			send_otg_notify(o_notify, NOTIFY_EVENT_DRIVE_VBUS, 1);
			send_otg_notify(o_notify, NOTIFY_EVENT_HMT, 1);
		} else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_OTG_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			pr_info("%s - USB_HOST_TEST_DETACHED\n", __func__);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			pr_info("%s - USB_HOST_TEST_ATTACHED\n", __func__);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_SMARTDOCK_TA_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_SMARTDOCK_TA, 0);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_SMARTDOCK_TA, 1);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_SMARTDOCK_USB_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			send_otg_notify
				(o_notify, NOTIFY_EVENT_SMARTDOCK_USB, 0);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			send_otg_notify
				(o_notify, NOTIFY_EVENT_SMARTDOCK_USB, 1);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_AUDIODOCK, 0);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_AUDIODOCK, 1);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	case ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC:
		if (action == MUIC_NOTIFY_CMD_DETACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_MMDOCK, 0);
		else if (action == MUIC_NOTIFY_CMD_ATTACH)
			send_otg_notify(o_notify, NOTIFY_EVENT_MMDOCK, 1);
		else
			pr_err("%s - ACTION Error!\n", __func__);
		break;
	default:
		break;
	}

	return 0;
}
#endif

#ifdef CONFIG_VBUS_NOTIFIER
static int vbus_handle_notification(struct notifier_block *nb,
		unsigned long cmd, void *data)
{
	vbus_status_t vbus_type = *(vbus_status_t *)data;
	struct otg_notify *o_notify;

	o_notify = get_otg_notify();

	pr_info("%s cmd=%lu, vbus_type=%s\n",
		__func__, cmd, vbus_type == STATUS_VBUS_HIGH ? "HIGH" : "LOW");

	switch (vbus_type) {
	case STATUS_VBUS_HIGH:
		send_otg_notify(o_notify, NOTIFY_EVENT_VBUSPOWER, 1);
		break;
	case STATUS_VBUS_LOW:
		send_otg_notify(o_notify, NOTIFY_EVENT_VBUSPOWER, 0);
		break;
	default:
		break;
	}
	return 0;
}
#endif

static struct usb_notifier_platform_data *of_get_usb_notifier_pdata(void)
{
	struct device_node *np = NULL;
	struct platform_device *pdev = NULL;
	struct usb_notifier_platform_data *pdata = NULL;

	np = of_find_compatible_node(NULL, NULL, "samsung,usb-notifier");
	if (!np) {
		pr_err("%s: failed to get the usb-notifier device node\n",
				__func__);
		return NULL;
	}

	pdev = of_find_device_by_node(np);
	if (!pdev) {
		pr_err("%s: failed to get the usb-notifier platform_device\n",
				__func__);
		return NULL;
	}
	pdata = pdev->dev.platform_data;
	of_node_put(np);

	return pdata;
}

static int otg_accessory_power(bool enable)
{
	struct usb_notifier_platform_data *pdata = of_get_usb_notifier_pdata();
	struct power_supply *psy_otg, *psy_battery;
	union power_supply_propval val;
	int on = !!enable;
	int current_cable_type;
	int ret = 0;
	pr_info("%s %d, enable=%d\n", __func__, __LINE__, enable);
	/* otg psy test */
	psy_otg = power_supply_get_by_name("otg");
	psy_battery = power_supply_get_by_name("battery");

	if (psy_otg) {
		val.intval = enable;
		ret = psy_otg->set_property(psy_otg, POWER_SUPPLY_PROP_ONLINE, &val);
	} else if (psy_battery) {
		if (enable)
			current_cable_type = POWER_SUPPLY_TYPE_OTG;
		else
			current_cable_type = POWER_SUPPLY_TYPE_BATTERY;

		val.intval = current_cable_type;
		ret = psy_battery->set_property(psy_battery, POWER_SUPPLY_PROP_ONLINE, &val);
	} else if (pdata->gpio_otg_en >= 0) {
		pr_info("before gpio_get_value(%d) = %d\n", pdata->gpio_otg_en, gpio_get_value(pdata->gpio_otg_en));
		gpio_set_value(pdata->gpio_otg_en, on);
		pr_info("after gpio_get_value(%d) = %d\n", pdata->gpio_otg_en, gpio_get_value(pdata->gpio_otg_en));
	} else {
		pr_err("%s: Fail to get psy battery\n", __func__);
		return -1;
	}
	if (ret) {
		pr_err("%s: fail to set power_suppy ONLINE property(%d)\n",
			__func__, ret);
	} else {
		pr_info("otg accessory power = %d\n", on);
	}

	return ret;
}

extern void set_ncm_ready(bool ready);
static int qcom_set_peripheral(bool enable)
{
	struct power_supply *psy;


	if(!enable)
		set_ncm_ready(false);

	psy = power_supply_get_by_name("msm-usb");
	pr_info("usb: msm-usb power_supply_set_present(%d)", enable);
	if (psy)
		power_supply_set_present(psy, enable);
	else
		pr_err("usb: dwc-usb power supply is null!\n");

	return 0;
}

static int set_online(int event, int state)
{
	union power_supply_propval value;
	struct power_supply *psy;

	pr_info("set_online: %d, %d\n", event, state);

	psy = power_supply_get_by_name("battery");
	if (!psy) {
		pr_err("%s: fail to get battery power_supply\n", __func__);
		return -1;
	}

	if (state)
		value.intval = POWER_SUPPLY_TYPE_SMART_OTG;
	else
		value.intval = POWER_SUPPLY_TYPE_SMART_NOTG;

	psy->set_property(psy, POWER_SUPPLY_PROP_ONLINE, &value);
	return 0;
}

static struct otg_notify sec_otg_notify = {
	.vbus_drive	= otg_accessory_power,
	.set_peripheral	= qcom_set_peripheral,
	.set_host = sec_set_host,
	.vbus_detect_gpio = -1,
	.is_wakelock = 1,
	.disable_control = 1,
	.set_battcall = set_online,
};

static int usb_notifier_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct usb_notifier_platform_data *pdata = NULL;

	pr_info("notifier_probe\n");

	if (pdev->dev.of_node) {
		pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct usb_notifier_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_err(&pdev->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

		ret = of_usb_notifier_dt(&pdev->dev, pdata);
		if (ret < 0) {
			dev_err(&pdev->dev, "Failed to get device of_node\n");
			return ret;
		}

		pdev->dev.platform_data = pdata;
	} else
		pdata = pdev->dev.platform_data;

	sec_otg_notify.redriver_en_gpio = pdata->gpio_redriver_en;
	if (pdata->disable_control_en == 1)
		sec_otg_notify.disable_control = 1;
	set_otg_notify(&sec_otg_notify);
	set_notify_data(&sec_otg_notify, pdata);
#ifdef CONFIG_MUIC_NOTIFIER
	muic_notifier_register(&pdata->usb_nb, usb_handle_notification,
			       MUIC_NOTIFY_DEV_USB);
#endif
#ifdef CONFIG_VBUS_NOTIFIER
	vbus_notifier_register(&pdata->vbus_nb, vbus_handle_notification,
			       MUIC_NOTIFY_DEV_USB);
#endif

	dev_info(&pdev->dev, "usb notifier probe\n");
	return 0;
}

static int usb_notifier_remove(struct platform_device *pdev)
{
#if defined(CONFIG_MUIC_NOTIFIER) || defined(CONFIG_VBUS_NOTIFIER)
	struct usb_notifier_platform_data *pdata = dev_get_platdata(&pdev->dev);
#endif
#ifdef CONFIG_MUIC_NOTIFIER
	muic_notifier_unregister(&pdata->usb_nb);
#endif
#ifdef CONFIG_VBUS_NOTIFIER
	muic_notifier_unregister(&pdata->vbus_nb);
#endif
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id usb_notifier_dt_ids[] = {
	{ .compatible = "samsung,usb-notifier",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, usb_notifier_dt_ids);
#endif

static struct platform_driver usb_notifier_driver = {
	.probe		= usb_notifier_probe,
	.remove		= usb_notifier_remove,
	.driver		= {
		.name	= "usb_notifier",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table	= of_match_ptr(usb_notifier_dt_ids),
#endif
	},
};

static int __init usb_notifier_init(void)
{
	return platform_driver_register(&usb_notifier_driver);
}

static void __init usb_notifier_exit(void)
{
	platform_driver_unregister(&usb_notifier_driver);
}

late_initcall(usb_notifier_init);
module_exit(usb_notifier_exit);

MODULE_AUTHOR("Samsung USB Team");
MODULE_DESCRIPTION("USB Notifier");
MODULE_LICENSE("GPL");
