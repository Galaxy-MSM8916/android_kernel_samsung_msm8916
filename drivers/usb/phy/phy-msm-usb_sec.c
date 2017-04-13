/*
 * drivers/usb/otg/msm_otg_sec.c
 *
 * Copyright (c) 2013, Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/usb_notify.h>
#include <linux/gpio.h>
#ifdef CONFIG_EXTCON
#include <linux/extcon.h>
#include <linux/power_supply.h>
#endif

static void sec_otg_set_id_state(int id)
{
	struct msm_otg *motg = the_msm_otg;
	struct usb_phy *phy = &motg->phy;

	pr_info("msm_otg_set_id_state is called, ID : %d\n", id);

	if (id)
		set_bit(ID, &motg->inputs);
	else
		clear_bit(ID, &motg->inputs);

	if (atomic_read(&motg->in_lpm)) {
		pr_info("msm_otg_set_id_state : in LPM\n");
		pm_runtime_resume(phy->dev);
	}
	/* defer work-handling until pm_resume callback is handled */
	if (motg->phy.state != OTG_STATE_UNDEFINED) {
		if (atomic_read(&motg->pm_suspended))
			motg->sm_work_pending = true;
		else
		/* use use non-reentrant wq, so that we don't run sm_work on multiple cpus */
			queue_work(system_nrt_wq, &motg->sm_work);
	}
}

int sec_set_host(bool enable)
{
	if (enable) {
		pr_info("USB OTG START : ID clear\n");
		sec_otg_set_id_state(0);
	} else {
		pr_info("USB OTG STOP : ID set\n");
		sec_otg_set_id_state(1);
	}
	return 0;
}
EXPORT_SYMBOL(sec_set_host);

/*
 * Exported functions
 */

#ifdef CONFIG_USB_HOST_NOTIFY
static int ulpi_write(struct usb_phy *phy, u32 val, u32 reg);
static int ulpi_read(struct usb_phy *phy, u32 reg);
static void ulpi_init(struct msm_otg *motg);

static void msm_otg_host_phy_tune(struct msm_otg *otg,
		u32 paramb, u32 paramc, u32 paramd)
{
	pr_info("ULPI 0x%x: 0x%x: 0x%x: 0x%x - orig\n",
			ulpi_read(&otg->phy, 0x80),
			ulpi_read(&otg->phy, 0x81),
			ulpi_read(&otg->phy, 0x82),
			ulpi_read(&otg->phy, 0x83));

	ulpi_write(&otg->phy, paramb, 0x81);
	ulpi_write(&otg->phy, paramc, 0x82);
	ulpi_write(&otg->phy, paramd, 0x83);

	pr_info("ULPI 0x%x: 0x%x: 0x%x: 0x%x\n",
			ulpi_read(&otg->phy, 0x80),
			ulpi_read(&otg->phy, 0x81),
			ulpi_read(&otg->phy, 0x82),
			ulpi_read(&otg->phy, 0x83));
	mdelay(100);
}

static void msm_otg_host_notify(struct msm_otg *motg, int on)
{
	if (on)
		msm_otg_host_phy_tune(motg, 0x33, 0x14, 0x13);
	else
		ulpi_init(motg);
}
#endif

#ifdef CONFIG_EXTCON
struct sec_cable {
	struct work_struct work;
	struct notifier_block nb;
	struct extcon_specific_cable_nb extcon_nb;
	struct extcon_dev *edev;
	enum extcon_cable_name cable_type;
	int cable_state;
};

static struct sec_cable support_cable_list[] = {
	{ .cable_type = EXTCON_USB, },
#ifdef CONFIG_USB_HOST_NOTIFY
	{ .cable_type = EXTCON_USB_HOST, },
	{ .cable_type = EXTCON_USB_HOST_5V, },
#if defined (CONFIG_MUIC_SUPPORT_LANHUB)
	{ .cable_type = EXTCON_LANHUB, },
	{ .cable_type = EXTCON_LANHUB_TA, },
#endif
	{ .cable_type = EXTCON_AUDIODOCK, },
	{ .cable_type = EXTCON_SMARTDOCK_TA, },
#endif
	{ .cable_type = EXTCON_SMARTDOCK_USB, },
	{ .cable_type = EXTCON_JIG_USBON, },
	{ .cable_type = EXTCON_CHARGE_DOWNSTREAM, },
};

static void sec_cable_event_worker(struct work_struct *work)
{
	struct sec_cable *cable =
			    container_of(work, struct sec_cable, work);
	struct otg_notify *n = get_otg_notify();

	pr_info("sec otg: %s is %s\n",
		extcon_cable_name[cable->cable_type],
		cable->cable_state ? "attached" : "detached");

	switch (cable->cable_type) {
	case EXTCON_USB:
	case EXTCON_SMARTDOCK_USB:
	case EXTCON_JIG_USBON:
	case EXTCON_CHARGE_DOWNSTREAM:
		if (cable->cable_state)
			send_otg_notify(n, NOTIFY_EVENT_VBUS, 1);
		else
			send_otg_notify(n, NOTIFY_EVENT_VBUS, 0);
		break;
#ifdef CONFIG_USB_HOST_NOTIFY
	case EXTCON_USB_HOST:
		if (cable->cable_state) {
			send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 1);
			send_otg_notify(n, NOTIFY_EVENT_HOST, 1);
		} else {
			send_otg_notify(n, NOTIFY_EVENT_HOST, 0);
			send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 0);
		}
		break;
	case EXTCON_USB_HOST_5V:
		if (cable->cable_state)
			send_otg_notify(n, NOTIFY_EVENT_VBUSPOWER, 1);
		else
			send_otg_notify(n, NOTIFY_EVENT_VBUSPOWER, 0);
		break;
#if defined (CONFIG_MUIC_SUPPORT_LANHUB)
	case EXTCON_LANHUB:
		if (cable->cable_state)
			send_otg_notify(n, NOTIFY_EVENT_LANHUB, 1);
		else
			send_otg_notify(n, NOTIFY_EVENT_LANHUB, 0);
		break;
	case EXTCON_LANHUB_TA:
		if (cable->cable_state) {
			pr_info("sec otg: TA attached with lanhub, otg off\n");
			send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 0);
		} else {
			if (extcon_get_cable_state(cable->edev, extcon_cable_name[EXTCON_USB_HOST])) {
				pr_info("sec otg: TA detached with lanhub, otg on\n");
				send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 1);
			} else {
				pr_info("sec otg: usb host detached already, do nothing\n");
			}
		}
		break;
#endif
	case EXTCON_AUDIODOCK:
		if (cable->cable_state)
			send_otg_notify(n, NOTIFY_EVENT_AUDIODOCK, 1);
		else
			send_otg_notify(n, NOTIFY_EVENT_AUDIODOCK, 0);
		break;
	case EXTCON_SMARTDOCK_TA:
		if (cable->cable_state)
			send_otg_notify(n, NOTIFY_EVENT_SMARTDOCK_TA, 1);
		else
			send_otg_notify(n, NOTIFY_EVENT_SMARTDOCK_TA, 0);
		break;
#endif
	default : break;
	}

}

static int sec_cable_notifier(struct notifier_block *nb,
					unsigned long stat, void *ptr)
{
	struct sec_cable *cable =
			container_of(nb, struct sec_cable, nb);

	cable->cable_state = stat;

	schedule_work(&cable->work);

	return NOTIFY_DONE;
}

static int __init sec_otg_init_cable_notify(void)
{
	struct sec_cable *cable;
	int i;
	int ret;

	pr_info("%s register extcon notifier for usb and ta\n", __func__);
	for (i = 0; i < ARRAY_SIZE(support_cable_list); i++) {
		cable = &support_cable_list[i];

		INIT_WORK(&cable->work, sec_cable_event_worker);
		cable->nb.notifier_call = sec_cable_notifier;

		ret = extcon_register_interest(&cable->extcon_nb,
				EXTCON_DEV_NAME,
				extcon_cable_name[cable->cable_type],
				&cable->nb);
		if (ret)
			pr_err("%s: fail to register extcon notifier(%s, %d)\n",
				__func__, extcon_cable_name[cable->cable_type],
				ret);

		cable->edev = cable->extcon_nb.edev;
		if (!cable->edev)
			pr_err("%s: fail to get extcon device\n", __func__);
	}
	return 0;
}
device_initcall_sync(sec_otg_init_cable_notify);
#endif
