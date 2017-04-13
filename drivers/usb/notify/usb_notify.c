/*
 * Copyright (C) 2011-2013 Samsung Electronics Co. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define pr_fmt(fmt) "usb_notify: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/wakelock.h>
#include <linux/kthread.h>
#include <linux/usb_notify.h>
#include "dock_notify.h"
#include "usb_notify_sysfs.h"

#define DEFAULT_OVC_POLL_SEC 3

struct  ovc {
	wait_queue_head_t	 delay_wait;
	struct completion	scanning_done;
	struct task_struct *th;
	struct mutex ovc_lock;
	int thread_remove;
	int can_ovc;
	int poll_period;
	int prev_state;
	void *data;
	int (*check_state)(void *);
};

struct vbus_gpio {
	spinlock_t lock;
	int gpio_status;
};

struct otg_state_work {
	struct work_struct otg_work;
	unsigned long event;
	int enable;
};

struct otg_booting_delay {
	struct delayed_work booting_work;
	unsigned long reserve_state;
};

struct usb_notify {
	struct otg_notify *o_notify;
	struct notifier_block otg_nb;
	struct notifier_block extra_nb;
	struct vbus_gpio v_gpio;
	struct host_notify_dev ndev;
	struct usb_notify_dev udev;
	struct workqueue_struct *notifier_wq;
	struct wake_lock wlock;
	struct otg_booster *booster;
	struct ovc ovc_info;
	struct otg_booting_delay b_delay;
	int oc_noti;
	int diable_v_drive;
	unsigned long c_type;
	int c_status;
};

static struct usb_notify *u_notify;

/*
 *  Define event types.
 *  NOTIFY_EVENT_STATE can be called in both interrupt context
 *		and process context. But it executes queue_work.
 *  NOTIFY_EVENT_EXTRA can be called directly without queue_work.
 *           But it must be called in process context.
 *  NOTIFY_EVENT_DELAY events can not run inner booting delay.
 *  NOTIFY_EVENT_NEED_VBUSDRIVE events need to drive 5v out
 *           from phone charger ic
 *  NOTIFY_EVENT_NOBLOCKING events are not blocked by disable sysfs.
 *  NOTIFY_EVENT_NOSAVE events are not saved in cable type.
 */
static int check_event_type(enum otg_notify_events event)
{
	int ret = 0;

	switch (event) {
	case NOTIFY_EVENT_OVERCURRENT:
	case NOTIFY_EVENT_VBUSPOWER:
	case NOTIFY_EVENT_SMSC_OVC:
	case NOTIFY_EVENT_SMTD_EXT_CURRENT:
	case NOTIFY_EVENT_MMD_EXT_CURRENT:
		ret |= NOTIFY_EVENT_EXTRA;
		break;
	case NOTIFY_EVENT_VBUS:
	case NOTIFY_EVENT_SMARTDOCK_USB:
		ret |= (NOTIFY_EVENT_STATE | NOTIFY_EVENT_DELAY
				| NOTIFY_EVENT_NEED_CLIENT);
		break;
	case NOTIFY_EVENT_HOST:
	case NOTIFY_EVENT_HMT:
		ret |= (NOTIFY_EVENT_STATE | NOTIFY_EVENT_NEED_VBUSDRIVE
				| NOTIFY_EVENT_DELAY | NOTIFY_EVENT_NEED_HOST);
		break;
	case NOTIFY_EVENT_ALLDISABLE_NOTIFY:
	case NOTIFY_EVENT_HOSTDISABLE_NOTIFY:
	case NOTIFY_EVENT_CLIENTDISABLE_NOTIFY:
		ret |= (NOTIFY_EVENT_STATE | NOTIFY_EVENT_NOBLOCKING
				| NOTIFY_EVENT_NOSAVE);
		break;
	case NOTIFY_EVENT_DRIVE_VBUS:
	case NOTIFY_EVENT_LANHUB_TA:
		ret |= (NOTIFY_EVENT_STATE | NOTIFY_EVENT_NOSAVE
				| NOTIFY_EVENT_NEED_HOST);
		break;
	case NOTIFY_EVENT_SMARTDOCK_TA:
	case NOTIFY_EVENT_AUDIODOCK:
	case NOTIFY_EVENT_LANHUB:
	case NOTIFY_EVENT_MMDOCK:
		ret |= NOTIFY_EVENT_NEED_HOST;
	case NOTIFY_EVENT_CHARGER:
	case NOTIFY_EVENT_NONE:
	default:
		ret |= NOTIFY_EVENT_STATE;
		break;
	}
	return ret;
}

static const char *event_string(enum otg_notify_events event)
{
	int virt;

	virt = IS_VIRTUAL(event);
	event = PHY_EVENT(event);

	switch (event) {
	case NOTIFY_EVENT_NONE:
		return "none";
	case NOTIFY_EVENT_VBUS:
		return virt ? "vbus(virtual)" : "vbus";
	case NOTIFY_EVENT_HOST:
		return virt ? "host_id(virtual)" : "host_id";
	case NOTIFY_EVENT_CHARGER:
		return virt ? "charger(virtual)" : "charger";
	case NOTIFY_EVENT_SMARTDOCK_TA:
		return virt ? "smartdock_ta(virtual)" : "smartdock_ta";
	case NOTIFY_EVENT_SMARTDOCK_USB:
		return virt ? "smartdock_usb(virtual)" : "smartdock_usb";
	case NOTIFY_EVENT_AUDIODOCK:
		return virt ? "audiodock(virtual)" : "audiodock";
	case NOTIFY_EVENT_LANHUB:
		return virt ? "lanhub(virtual)" : "lanhub";
	case NOTIFY_EVENT_LANHUB_TA:
		return virt ? "lanhub_ta(virtual)" : "lanhub_ta";
	case NOTIFY_EVENT_MMDOCK:
		return virt ? "mmdock(virtual)" : "mmdock";
	case NOTIFY_EVENT_HMT:
		return virt ? "hmt(virtual)" : "hmt";
	case NOTIFY_EVENT_DRIVE_VBUS:
		return "drive_vbus";
	case NOTIFY_EVENT_ALLDISABLE_NOTIFY:
		return "disable_all_notify";
	case NOTIFY_EVENT_HOSTDISABLE_NOTIFY:
		return "disable_host_notify";
	case NOTIFY_EVENT_CLIENTDISABLE_NOTIFY:
		return "disable_client_notify";
	case NOTIFY_EVENT_OVERCURRENT:
		return "overcurrent";
	case NOTIFY_EVENT_VBUSPOWER:
		return "vbus_power";
	case NOTIFY_EVENT_SMSC_OVC:
		return "smsc_ovc";
	case NOTIFY_EVENT_SMTD_EXT_CURRENT:
		return "smtd_ext_current";
	case NOTIFY_EVENT_MMD_EXT_CURRENT:
		return "mmd_ext_current";
	default:
		return "undefined";
	}
}

static const char *block_string(enum otg_notify_block_type type)
{
	switch (type) {
	case NOTIFY_BLOCK_TYPE_NONE:
		return "block_off";
	case NOTIFY_BLOCK_TYPE_HOST:
		return "block_host";
	case NOTIFY_BLOCK_TYPE_CLIENT:
		return "block_client";
	case NOTIFY_BLOCK_TYPE_ALL:
		return "block_all";
	default:
		return "undefined";
	}
}

static bool is_host_cable_block(void)
{
	if ((check_event_type(u_notify->c_type)
		& NOTIFY_EVENT_NEED_HOST) &&
			(u_notify->c_status == NOTIFY_EVENT_BLOCKED
				|| u_notify->c_status == NOTIFY_EVENT_BLOCKING))
		return true;
	else
		return false;
}

static bool is_host_cable_enable(void)
{
	if ((check_event_type(u_notify->c_type)
		& NOTIFY_EVENT_NEED_HOST) &&
			(u_notify->c_status == NOTIFY_EVENT_ENABLED
				|| u_notify->c_status == NOTIFY_EVENT_ENABLING))
		return true;
	else
		return false;
}

static bool is_client_cable_block(void)
{
	if ((check_event_type(u_notify->c_type)
		& NOTIFY_EVENT_NEED_CLIENT) &&
			(u_notify->c_status == NOTIFY_EVENT_BLOCKED
				|| u_notify->c_status == NOTIFY_EVENT_BLOCKING))
		return true;
	else
		return false;
}

static bool is_client_cable_enable(void)
{
	if ((check_event_type(u_notify->c_type)
		& NOTIFY_EVENT_NEED_CLIENT) &&
			(u_notify->c_status == NOTIFY_EVENT_ENABLED
				|| u_notify->c_status == NOTIFY_EVENT_ENABLING))
		return true;
	else
		return false;
}

static bool check_block_event(unsigned long event)
{
	if ((test_bit(NOTIFY_BLOCK_TYPE_HOST, &u_notify->udev.disable_state)
		&& (check_event_type(event) & NOTIFY_EVENT_NEED_HOST))
		|| (test_bit(NOTIFY_BLOCK_TYPE_CLIENT,
				&u_notify->udev.disable_state)
		&& (check_event_type(event) & NOTIFY_EVENT_NEED_CLIENT)))
		return true;
	else
		return false;
}

static void enable_ovc(struct usb_notify *u_noti, int enable)
{
	u_noti->ovc_info.can_ovc = enable;
}

static int ovc_scan_thread(void *data)
{
	struct ovc *ovcinfo = NULL;
	int state;

	if (!u_notify) {
		pr_err("%s u_notify is NULL\n", __func__);
		return 0;
	}

	ovcinfo = &u_notify->ovc_info;
	while (!kthread_should_stop()) {
		wait_event_interruptible_timeout(ovcinfo->delay_wait,
			ovcinfo->thread_remove, (ovcinfo->poll_period)*HZ);
		if (ovcinfo->thread_remove)
			break;
		mutex_lock(&u_notify->ovc_info.ovc_lock);
		if (ovcinfo->check_state
			&& ovcinfo->data
				&& ovcinfo->can_ovc) {

			state = ovcinfo->check_state(data);

			if (ovcinfo->prev_state != state) {
				if (state == HNOTIFY_LOW) {
					pr_err("%s overcurrent detected\n",
							__func__);
					host_state_notify(&u_notify->ndev,
						NOTIFY_HOST_OVERCURRENT);
				} else if (state == HNOTIFY_HIGH) {
					pr_info("%s vbus draw detected\n",
							__func__);
					host_state_notify(&u_notify->ndev,
						NOTIFY_HOST_NONE);
				}
			}
			ovcinfo->prev_state = state;
		}
		mutex_unlock(&u_notify->ovc_info.ovc_lock);
		if (!ovcinfo->can_ovc)
			ovcinfo->thread_remove = 1;
	}

	pr_info("ovc_scan_thread exit\n");
	complete_and_exit(&ovcinfo->scanning_done, 0);
	return 0;
}

void ovc_start(struct usb_notify *u_noti)
{
	struct otg_notify *o_notify = get_otg_notify();

	if (!o_notify) {
		pr_err("%s o_notify is NULL\n", __func__);
		goto skip;
	}

	if (!u_noti->ovc_info.can_ovc)
		goto skip;

	u_noti->ovc_info.prev_state = HNOTIFY_INITIAL;
	u_noti->ovc_info.poll_period = (o_notify->smsc_ovc_poll_sec) ?
			o_notify->smsc_ovc_poll_sec : DEFAULT_OVC_POLL_SEC;
	INIT_COMPLETION(u_noti->ovc_info.scanning_done);
	u_noti->ovc_info.thread_remove = 0;
	u_noti->ovc_info.th = kthread_run(ovc_scan_thread,
			u_noti->ovc_info.data, "ovc-scan-thread");
	if (IS_ERR(u_noti->ovc_info.th)) {
		pr_err("Unable to start the ovc-scanning thread\n");
		complete(&u_noti->ovc_info.scanning_done);
	}
	pr_info("%s on\n", __func__);
	return;
skip:
	complete(&u_noti->ovc_info.scanning_done);
	pr_info("%s skip\n", __func__);
}

void ovc_stop(struct usb_notify *u_noti)
{
	u_noti->ovc_info.thread_remove = 1;
	wake_up_interruptible(&u_noti->ovc_info.delay_wait);
	wait_for_completion(&u_noti->ovc_info.scanning_done);
	mutex_lock(&u_noti->ovc_info.ovc_lock);
	u_noti->ovc_info.check_state = NULL;
	u_noti->ovc_info.data = 0;
	mutex_unlock(&u_noti->ovc_info.ovc_lock);
	pr_info("%s\n", __func__);
}

static void ovc_init(struct usb_notify *u_noti)
{
	init_waitqueue_head(&u_noti->ovc_info.delay_wait);
	init_completion(&u_noti->ovc_info.scanning_done);
	mutex_init(&u_noti->ovc_info.ovc_lock);
	u_noti->ovc_info.prev_state = HNOTIFY_INITIAL;
	pr_info("%s\n", __func__);
}

static irqreturn_t vbus_irq_isr(int irq, void *data)
{
	struct otg_notify *otg_notify;
	unsigned long flags = 0;
	int gpio_value = 0;
	irqreturn_t ret = IRQ_NONE;

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		return ret;
	}

	otg_notify = u_notify->o_notify;

	if (!otg_notify) {
		pr_err("otg_notify is NULL\n");
		return ret;
	}

	spin_lock_irqsave(&u_notify->v_gpio.lock, flags);
	gpio_value = gpio_get_value(otg_notify->vbus_detect_gpio);
	if (u_notify->v_gpio.gpio_status != gpio_value) {
		u_notify->v_gpio.gpio_status = gpio_value;
		ret = IRQ_WAKE_THREAD;
	} else
		ret = IRQ_HANDLED;
	spin_unlock_irqrestore(&u_notify->v_gpio.lock, flags);

	return ret;
}

static irqreturn_t vbus_irq_thread(int irq, void *data)
{
	struct otg_notify *notify = NULL;
	unsigned long flags = 0;
	int gpio_value = 0;

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		return IRQ_HANDLED;
	}
	notify = u_notify->o_notify;

	spin_lock_irqsave(&u_notify->v_gpio.lock, flags);
	gpio_value = u_notify->v_gpio.gpio_status;
	spin_unlock_irqrestore(&u_notify->v_gpio.lock, flags);

	if (gpio_value) {
		u_notify->ndev.booster = NOTIFY_POWER_ON;
		pr_info("vbus on detect\n");
		if (notify->post_vbus_detect)
			notify->post_vbus_detect(NOTIFY_POWER_ON);
	} else {
		if ((u_notify->ndev.mode == NOTIFY_HOST_MODE)
			&& (u_notify->ndev.booster == NOTIFY_POWER_ON
				&& u_notify->oc_noti)) {
			host_state_notify(&u_notify->ndev,
					NOTIFY_HOST_OVERCURRENT);
			pr_err("OTG overcurrent!!!!!!\n");
		} else {
			pr_info("vbus off detect\n");
			if (notify->post_vbus_detect)
				notify->post_vbus_detect(NOTIFY_POWER_OFF);
		}
		u_notify->ndev.booster = NOTIFY_POWER_OFF;
	}
	return IRQ_HANDLED;
}

int register_gpios(struct otg_notify *n)
{
	int ret = 0;
	int vbus_irq = 0;
	int vbus_gpio = -1;
	int redriver_gpio = -1;

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		return ret;
	}

	if (!gpio_is_valid(n->vbus_detect_gpio))
		goto redriver_en_gpio_phase;

	vbus_gpio = n->vbus_detect_gpio;

	spin_lock_init(&u_notify->v_gpio.lock);

	if (n->pre_gpio)
		n->pre_gpio(vbus_gpio, NOTIFY_VBUS);

	ret = gpio_request(vbus_gpio, "vbus_detect_notify");
	if (ret) {
		pr_err("failed to request %d\n", vbus_gpio);
		goto err;
	}
	gpio_direction_input(vbus_gpio);

	u_notify->v_gpio.gpio_status
		= gpio_get_value(vbus_gpio);
	vbus_irq = gpio_to_irq(vbus_gpio);
	ret = request_threaded_irq(vbus_irq,
			vbus_irq_isr,
			vbus_irq_thread,
			(IRQF_TRIGGER_FALLING |
				IRQF_TRIGGER_RISING |
					IRQF_ONESHOT),
			"vbus_irq_notify",
			NULL);
	if (ret) {
		pr_err("Failed to register IRQ\n");
		goto err;
	}
	if (n->post_gpio)
		n->post_gpio(vbus_gpio, NOTIFY_VBUS);

	pr_info("vbus detect gpio %d is registered.\n", vbus_gpio);
redriver_en_gpio_phase:
	if (!gpio_is_valid(n->redriver_en_gpio))
		goto err;

	redriver_gpio = n->redriver_en_gpio;

	if (n->pre_gpio)
		n->pre_gpio(redriver_gpio, NOTIFY_REDRIVER);

	ret = gpio_request(redriver_gpio, "usb30_redriver_en");
	if (ret) {
		pr_err("failed to request %d\n", redriver_gpio);
		goto err;
	}
	gpio_direction_output(redriver_gpio, 0);
	if (n->post_gpio)
		n->post_gpio(redriver_gpio, NOTIFY_REDRIVER);

	pr_info("redriver en gpio %d is registered.\n", redriver_gpio);
err:
	return ret;
}

int do_notify_blockstate(struct otg_notify *notify, unsigned long event,
					int type, int enable)
{
	int ret = 0;

	if (!u_notify) {
		pr_err("%s u_notify is NULL\n", __func__);
		goto skip;
	}

	switch (event) {
	case NOTIFY_EVENT_NONE:
	case NOTIFY_EVENT_CHARGER:
		break;
	case NOTIFY_EVENT_SMARTDOCK_USB:
	case NOTIFY_EVENT_VBUS:
		break;
	case NOTIFY_EVENT_LANHUB:
	case NOTIFY_EVENT_HMT:
	case NOTIFY_EVENT_HOST:
	case NOTIFY_EVENT_MMDOCK:
	case NOTIFY_EVENT_SMARTDOCK_TA:
	case NOTIFY_EVENT_AUDIODOCK:
		if (notify->unsupport_host) {
			pr_err("This model doesn't support usb host\n");
			goto skip;
		}
		if (enable)
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_BLOCK);
		else
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_NONE);
		break;
	case NOTIFY_EVENT_DRIVE_VBUS:
		ret = -ESRCH;
		break;
	default:
		break;
	}

skip:
	return ret;
}

static void update_cable_status(struct otg_notify *notify, unsigned long event,
		int virtual, int enable, int start)
{
	if (enable) {
		u_notify->c_type = event;
		if (check_block_event(event) ||
			(check_event_type(u_notify->c_type)
				& NOTIFY_EVENT_NEED_HOST &&
					notify->unsupport_host))
			u_notify->c_status = (start) ?
				NOTIFY_EVENT_BLOCKING : NOTIFY_EVENT_BLOCKED;
		else
			u_notify->c_status = (start) ?
				NOTIFY_EVENT_ENABLING : NOTIFY_EVENT_ENABLED;
	} else {
		if (virtual)
			u_notify->c_status = (start) ?
				NOTIFY_EVENT_BLOCKING : NOTIFY_EVENT_BLOCKED;
		else {
			u_notify->c_type = NOTIFY_EVENT_NONE;
			u_notify->c_status = (start) ?
				NOTIFY_EVENT_DISABLING : NOTIFY_EVENT_DISABLED;
		}
	}
}

static void otg_notify_state(unsigned long event, int enable)
{
	struct otg_notify *notify = NULL;
	int type = 0;
	int virtual = 0;

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		goto err;
	}

	pr_info("%s+ event=%s(%lu), enable=%s\n", __func__,
		event_string(event), event, enable == 0 ? "off" : "on");

	notify = get_otg_notify();
	if (!notify) {
		pr_err("notify is NULL\n");
		goto no_save_event;
	}

	virtual = IS_VIRTUAL(event);
	event = PHY_EVENT(event);

	type = check_event_type(event);

	if (!(type & NOTIFY_EVENT_NOSAVE))
		update_cable_status(notify, event, virtual, enable, 1);

	if (check_block_event(event) &&
			!(type & NOTIFY_EVENT_NOBLOCKING)) {
		pr_err("%s usb notify is blocked\n", __func__);
		if (do_notify_blockstate(notify, event, type, enable))
			goto no_save_event;
		else
			goto err2;
	}

	switch (event) {
	case NOTIFY_EVENT_NONE:
		break;
	case NOTIFY_EVENT_SMARTDOCK_USB:
	case NOTIFY_EVENT_VBUS:
		if (enable) {
			u_notify->ndev.mode = NOTIFY_PERIPHERAL_MODE;
			if (notify->is_wakelock)
				wake_lock(&u_notify->wlock);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 1);
			if (notify->set_peripheral)
				notify->set_peripheral(true);
		} else {
			u_notify->ndev.mode = NOTIFY_NONE_MODE;
			if (notify->set_peripheral)
				notify->set_peripheral(false);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 0);
			if (notify->is_wakelock)
				wake_unlock(&u_notify->wlock);
		}
		break;
	case NOTIFY_EVENT_LANHUB_TA:
		u_notify->diable_v_drive = enable;
		if (enable)
			u_notify->oc_noti = 0;
		if (notify->set_lanhubta)
			notify->set_lanhubta(enable);
		break;
	case NOTIFY_EVENT_LANHUB:
		if (notify->unsupport_host) {
			pr_err("This model doesn't support usb host\n");
			goto err2;
		}
		u_notify->diable_v_drive = enable;
		if (enable) {
			u_notify->oc_noti = 0;
			u_notify->ndev.mode = NOTIFY_HOST_MODE;
			if (notify->is_wakelock)
				wake_lock(&u_notify->wlock);
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_ADD);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 1);
			if (notify->set_host)
				notify->set_host(true);
		} else {
			u_notify->ndev.mode = NOTIFY_NONE_MODE;
			if (notify->set_host)
				notify->set_host(false);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 0);
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_REMOVE);
			if (notify->is_wakelock)
				wake_unlock(&u_notify->wlock);
		}
		break;
	case NOTIFY_EVENT_HMT:
	case NOTIFY_EVENT_HOST:
		if (notify->unsupport_host) {
			pr_err("This model doesn't support usb host\n");
			goto err2;
		}
		u_notify->diable_v_drive = 0;
		if (enable) {
			u_notify->ndev.mode = NOTIFY_HOST_MODE;
			if (notify->is_wakelock)
				wake_lock(&u_notify->wlock);
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_ADD);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 1);
			if (notify->set_host)
				notify->set_host(true);
			if (notify->auto_drive_vbus) {
				u_notify->oc_noti = 1;
				if (notify->vbus_drive)
					notify->vbus_drive(1);
			}
		} else {
			u_notify->ndev.mode = NOTIFY_NONE_MODE;
			if (notify->auto_drive_vbus) {
				u_notify->oc_noti = 0;
				if (notify->vbus_drive)
					notify->vbus_drive(0);
			}
			if (notify->set_host)
				notify->set_host(false);
			if (gpio_is_valid(notify->redriver_en_gpio))
				gpio_direction_output
					(notify->redriver_en_gpio, 0);
			host_state_notify(&u_notify->ndev, NOTIFY_HOST_REMOVE);
			if (notify->is_wakelock)
				wake_unlock(&u_notify->wlock);
		}
		break;
	case NOTIFY_EVENT_CHARGER:
		if (notify->set_charger)
			notify->set_charger(enable);
		break;
	case NOTIFY_EVENT_MMDOCK:
		enable_ovc(u_notify, enable);
		/* To detect overcurrent, ndev state is initialized */
		if (enable)
			host_state_notify(&u_notify->ndev,
							NOTIFY_HOST_NONE);
	case NOTIFY_EVENT_SMARTDOCK_TA:
	case NOTIFY_EVENT_AUDIODOCK:
		if (notify->unsupport_host) {
			pr_err("This model doesn't support usb host\n");
			goto err2;
		}
		u_notify->diable_v_drive = enable;
		if (enable) {
			u_notify->ndev.mode = NOTIFY_HOST_MODE;
			if (notify->is_wakelock)
				wake_lock(&u_notify->wlock);
			if (notify->set_host)
				notify->set_host(true);
		} else {
			u_notify->ndev.mode = NOTIFY_NONE_MODE;
			if (notify->set_host)
				notify->set_host(false);
			if (notify->is_wakelock)
				wake_unlock(&u_notify->wlock);
		}
		break;
	case NOTIFY_EVENT_DRIVE_VBUS:
		if (notify->unsupport_host) {
			pr_err("This model doesn't support usb host\n");
			goto no_save_event;
		}
		if (u_notify->diable_v_drive) {
			pr_info("cable type=%s disable vbus draw\n",
					event_string(u_notify->c_type));
			goto no_save_event;
		}
		u_notify->oc_noti = enable;
		if (notify->vbus_drive)
			notify->vbus_drive((bool)enable);
		goto no_save_event;
	case NOTIFY_EVENT_ALLDISABLE_NOTIFY:
		if (!notify->disable_control) {
			pr_err("This model doesn't support disable_control\n");
			goto no_save_event;
		}
		if (enable) {
			set_bit(NOTIFY_BLOCK_TYPE_HOST,
				&u_notify->udev.disable_state);
			set_bit(NOTIFY_BLOCK_TYPE_CLIENT,
				&u_notify->udev.disable_state);
		} else {
			clear_bit(NOTIFY_BLOCK_TYPE_HOST,
				&u_notify->udev.disable_state);
			clear_bit(NOTIFY_BLOCK_TYPE_CLIENT,
				&u_notify->udev.disable_state);
		}
		goto no_save_event;
	case NOTIFY_EVENT_HOSTDISABLE_NOTIFY:
		if (!notify->disable_control) {
			pr_err("This model doesn't support disable_control\n");
			goto no_save_event;
		}
		if (enable) {
			clear_bit(NOTIFY_BLOCK_TYPE_CLIENT,
				&u_notify->udev.disable_state);
			set_bit(NOTIFY_BLOCK_TYPE_HOST,
				&u_notify->udev.disable_state);
		}
		goto no_save_event;
	case NOTIFY_EVENT_CLIENTDISABLE_NOTIFY:
		if (!notify->disable_control) {
			pr_err("This model doesn't support disable_control\n");
			goto no_save_event;
		}
		if (enable) {
			clear_bit(NOTIFY_BLOCK_TYPE_HOST,
				&u_notify->udev.disable_state);
			set_bit(NOTIFY_BLOCK_TYPE_CLIENT,
				&u_notify->udev.disable_state);
		}
		goto no_save_event;
	default:
		break;
	}
err2:
	update_cable_status(notify, event, virtual, enable, 0);

no_save_event:
	pr_info("%s- event=%s, cable=%s\n", __func__,
		event_string(event),
			event_string(u_notify->c_type));
err:
	return;
}

static void extra_notify_state(unsigned long event, int enable)
{
	struct otg_notify *notify = NULL;

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		goto err;
	}

	pr_info("%s+ event=%s(%lu), enable=%s\n", __func__,
		event_string(event), event, enable == 0 ? "off" : "on");

	notify = get_otg_notify();
	if (!notify) {
		pr_err("notify is NULL\n");
		goto err1;
	}

	switch (event) {
	case NOTIFY_EVENT_NONE:
		break;
	case NOTIFY_EVENT_OVERCURRENT:
		if (!u_notify->ndev.dev) {
			pr_err("ndev is NULL. Maybe usb host is not supported.\n");
			break;
		}
		host_state_notify(&u_notify->ndev,
						NOTIFY_HOST_OVERCURRENT);
		pr_err("OTG overcurrent!!!!!!\n");
		break;
	case NOTIFY_EVENT_VBUSPOWER:
		 if (enable)
			u_notify->ndev.booster = NOTIFY_POWER_ON;
		 else
			u_notify->ndev.booster = NOTIFY_POWER_OFF;
		break;
	case NOTIFY_EVENT_SMSC_OVC:
		if (enable)
			ovc_start(u_notify);
		else
			ovc_stop(u_notify);
		break;
	case NOTIFY_EVENT_SMTD_EXT_CURRENT:
		if (u_notify->c_type != NOTIFY_EVENT_SMARTDOCK_TA) {
			pr_err("No smart dock!!!!!!\n");
			break;
		}
		if (notify->set_battcall)
			notify->set_battcall
				(NOTIFY_EVENT_SMTD_EXT_CURRENT, enable);
		break;
	case NOTIFY_EVENT_MMD_EXT_CURRENT:
		if (u_notify->c_type != NOTIFY_EVENT_MMDOCK) {
			pr_err("No mmdock!!!!!!\n");
			break;
		}
		if (notify->set_battcall)
			notify->set_battcall
				(NOTIFY_EVENT_MMD_EXT_CURRENT, enable);
		break;
	default:
		break;
	}
err1:
	pr_info("%s- event=%s(%lu), cable=%s\n", __func__,
		event_string(event), event,
		event_string(u_notify->c_type));
err:
	return;
}

static void otg_notify_work(struct work_struct *data)
{
	struct otg_state_work *state_work =
		container_of(data, struct otg_state_work, otg_work);

	otg_notify_state(state_work->event, state_work->enable);

	kfree(state_work);
}

static int otg_notifier_callback(struct notifier_block *nb,
		unsigned long event, void *param)
{
	struct otg_state_work *state_work;

	pr_info("%s event=%s(%lu)\n", __func__,
			event_string(event), event);

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		return NOTIFY_DONE;
	}

	if (event > VIRT_EVENT(NOTIFY_EVENT_VBUSPOWER)) {
		pr_err("%s event is invalid\n", __func__);
		return NOTIFY_DONE;
	}

	state_work = kmalloc(sizeof(struct otg_state_work), GFP_ATOMIC);
	if (!state_work)
		return notifier_from_errno(-ENOMEM);

	INIT_WORK(&state_work->otg_work, otg_notify_work);
	state_work->event = event;
	state_work->enable = *(int *)param;
	queue_work(u_notify->notifier_wq, &state_work->otg_work);
	return NOTIFY_OK;
}

static int extra_notifier_callback(struct notifier_block *nb,
		unsigned long event, void *param)
{
	pr_info("%s event=%s(%lu)\n", __func__,
			event_string(event), event);

	if (!u_notify) {
		pr_err("u_notify is NULL\n");
		return NOTIFY_DONE;
	}

	if (event > VIRT_EVENT(NOTIFY_EVENT_VBUSPOWER)) {
		pr_err("%s event is invalid\n", __func__);
		return NOTIFY_DONE;
	}

	extra_notify_state(event, *(int *)param);

	return NOTIFY_OK;
}

static int create_usb_notify(void)
{
	int ret = 0;

	if (u_notify)
		goto err;

	u_notify = kzalloc(sizeof(struct usb_notify), GFP_KERNEL);
	if (!u_notify) {
		ret = -ENOMEM;
		goto err;
	}
	u_notify->notifier_wq
		= create_singlethread_workqueue("usb_notify");
	 if (!u_notify->notifier_wq) {
		pr_err("%s failed to create work queue\n", __func__);
		ret = -ENOMEM;
		goto err1;
	 }
	ret = usb_notify_class_init();
	if (ret) {
		pr_err("unable to do usb_notify_class_init\n");
		goto err2;
	}
	ovc_init(u_notify);
	return 0;
err2:
	flush_workqueue(u_notify->notifier_wq);
	destroy_workqueue(u_notify->notifier_wq);
err1:
	kfree(u_notify);
err:
	return ret;
}

static void reserve_state_check(struct work_struct *work)
{
	struct otg_booting_delay *o_b_d = container_of(work,
			struct otg_booting_delay, booting_work.work);
	struct usb_notify *u_noti = container_of(o_b_d,
			struct usb_notify, b_delay);
	int enable = 1;
	unsigned long state = 0;

	state = u_noti->b_delay.reserve_state;
	u_noti->o_notify->booting_delay_sec = 0;
	pr_info("%s booting delay finished\n", __func__);

	if (u_noti->b_delay.reserve_state != NOTIFY_EVENT_NONE) {
		pr_info("%s event=%s(%lu) enable=%d\n", __func__,
				event_string(state), state, enable);
		if (check_event_type(state) & NOTIFY_EVENT_EXTRA)
			blocking_notifier_call_chain
				(&u_noti->o_notify->extra_notifier,
						state, &enable);
		else
			atomic_notifier_call_chain
				(&u_noti->o_notify->otg_notifier,
						state, &enable);
	}
}

int set_notify_disable(struct usb_notify_dev *udev, int disable)
{
	struct otg_notify *n = udev->o_notify;

	if (!n) {
		pr_err("%s otg_notify is null\n", __func__);
		goto skip;
	}

	if (!n->disable_control) {
		pr_err("%s disable_control is not supported\n", __func__);
		goto skip;
	}

	pr_info("%s disable=%s(%d)\n", __func__,
			block_string(disable), disable);

	switch (disable) {
	case NOTIFY_BLOCK_TYPE_ALL:
		if (is_host_cable_enable() ||
			is_client_cable_enable()) {

			pr_info("%s event=%s(%lu) disable\n", __func__,
				event_string(VIRT_EVENT(u_notify->c_type)),
					VIRT_EVENT(u_notify->c_type));

			if (!n->auto_drive_vbus &&
				check_event_type(u_notify->c_type)
					& NOTIFY_EVENT_NEED_VBUSDRIVE)
				send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 0);

			send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 0);
		}
		send_otg_notify(n, NOTIFY_EVENT_ALLDISABLE_NOTIFY, 1);
		break;
	case NOTIFY_BLOCK_TYPE_HOST:
		if (is_host_cable_enable()) {

			pr_info("%s event=%s(%lu) disable\n", __func__,
				event_string(VIRT_EVENT(u_notify->c_type)),
					VIRT_EVENT(u_notify->c_type));

			if (!n->auto_drive_vbus &&
				check_event_type(u_notify->c_type)
					& NOTIFY_EVENT_NEED_VBUSDRIVE)
				send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 0);

			send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 0);
		}

		send_otg_notify(n, NOTIFY_EVENT_HOSTDISABLE_NOTIFY, 1);

		if (!is_client_cable_block())
			goto skip;

		pr_info("%s event=%s(%lu) enable\n", __func__,
			event_string(VIRT_EVENT(u_notify->c_type)),
				VIRT_EVENT(u_notify->c_type));

		send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 1);
		break;
	case NOTIFY_BLOCK_TYPE_CLIENT:
		if (is_client_cable_enable()) {

			pr_info("%s event=%s(%lu) disable\n", __func__,
				event_string(VIRT_EVENT(u_notify->c_type)),
					VIRT_EVENT(u_notify->c_type));

			send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 0);
		}

		send_otg_notify(n, NOTIFY_EVENT_CLIENTDISABLE_NOTIFY, 1);

		if (!is_host_cable_block())
			goto skip;

		if (n->unsupport_host)
			goto skip;

		pr_info("%s event=%s(%lu) enable\n", __func__,
			event_string(VIRT_EVENT(u_notify->c_type)),
				VIRT_EVENT(u_notify->c_type));
		if (!n->auto_drive_vbus &&
			check_event_type(u_notify->c_type)
					& NOTIFY_EVENT_NEED_VBUSDRIVE)
			send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 1);

		send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 1);
		break;
	case NOTIFY_BLOCK_TYPE_NONE:
		send_otg_notify(n, NOTIFY_EVENT_ALLDISABLE_NOTIFY, 0);
		if (!is_host_cable_block() && !is_client_cable_block())
			goto skip;

		if (check_event_type(u_notify->c_type)
				& NOTIFY_EVENT_NEED_HOST && n->unsupport_host)
			goto skip;

		pr_info("%s event=%s(%lu) enable\n", __func__,
			event_string(VIRT_EVENT(u_notify->c_type)),
				VIRT_EVENT(u_notify->c_type));
		if (!n->auto_drive_vbus &&
			check_event_type(u_notify->c_type)
					& NOTIFY_EVENT_NEED_VBUSDRIVE)
			send_otg_notify(n, NOTIFY_EVENT_DRIVE_VBUS, 1);

		send_otg_notify(n, VIRT_EVENT(u_notify->c_type), 1);
		break;
	}
skip:
	return 0;
}

void send_otg_notify(struct otg_notify *n,
				unsigned long event, int enable)
{
	int type = 0;

	if (!n) {
		pr_err("%s otg_notify is null\n", __func__);
		return;
	}

	pr_info("%s event=%s(%lu) enable=%d\n", __func__,
			event_string(event), event, enable);

	type = check_event_type(event);

	if (type & NOTIFY_EVENT_DELAY) {
		if (n->booting_delay_sec) {
			if (u_notify) {
				u_notify->b_delay.reserve_state =
					(enable) ? event : NOTIFY_EVENT_NONE;
				pr_info("%s reserve event\n", __func__);
			} else
				pr_err("%s u_notify is null\n", __func__);
			goto end;
		}
	}

	if (type & NOTIFY_EVENT_EXTRA)
		blocking_notifier_call_chain
			(&n->extra_notifier, event, &enable);
	else if (type & NOTIFY_EVENT_STATE)
		atomic_notifier_call_chain
			(&n->otg_notifier, event, &enable);
	else
		goto end;
end:
	return;
}
EXPORT_SYMBOL(send_otg_notify);

void *get_notify_data(struct otg_notify *n)
{
	if (n)
		return n->o_data;
	else
		return NULL;
}
EXPORT_SYMBOL(get_notify_data);

void set_notify_data(struct otg_notify *n, void *data)
{
	n->o_data = data;
}
EXPORT_SYMBOL(set_notify_data);

struct otg_notify *get_otg_notify(void)
{
	if (!u_notify)
		return NULL;
	if (!u_notify->o_notify)
		return NULL;
	return u_notify->o_notify;
}
EXPORT_SYMBOL(get_otg_notify);

struct otg_booster *find_get_booster(void)
{
	int ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("unable create_usb_notify\n");
			goto err;
		}
	}

	if (!u_notify->booster) {
		pr_err("error. No matching booster\n");
		goto err;
	}

	return u_notify->booster;
err:
	return NULL;
}
EXPORT_SYMBOL(find_get_booster);

int register_booster(struct otg_booster *b)
{
	int ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("unable create_usb_notify\n");
			goto err;
		}
	}

	u_notify->booster = b;
err:
	return ret;
}
EXPORT_SYMBOL(register_booster);

int register_ovc_func(int (*check_state)(void *), void *data)
{
	int ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("%s unable create_usb_notify\n", __func__);
			return -EFAULT;
		}
	}
	mutex_lock(&u_notify->ovc_info.ovc_lock);
	u_notify->ovc_info.check_state = check_state;
	u_notify->ovc_info.data = data;
	mutex_unlock(&u_notify->ovc_info.ovc_lock);
	pr_info("%s\n", __func__);
	return ret;
}
EXPORT_SYMBOL(register_ovc_func);

int get_usb_mode(void)
{
	int ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("unable create_usb_notify\n");
			return -EFAULT;
		}
	}
	pr_info("%s usb mode=%d\n", __func__, u_notify->ndev.mode);
	ret = u_notify->ndev.mode;
	return ret;
}
EXPORT_SYMBOL(get_usb_mode);

unsigned long get_cable_type(void)
{
	unsigned long ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("unable create_usb_notify\n");
			return -EFAULT;
		}
	}
	pr_info("%s cable type =%s\n", __func__,
			event_string(u_notify->c_type));
	ret = u_notify->c_type;
	return ret;
}
EXPORT_SYMBOL(get_cable_type);

int set_otg_notify(struct otg_notify *n)
{
	int ret = 0;

	if (!u_notify) {
		ret = create_usb_notify();
		if (ret) {
			pr_err("unable create_usb_notify\n");
			goto err;
		}
	}

	if (u_notify->o_notify && n) {
		pr_err("error : already set o_notify\n");
		goto err;
	}

	pr_info("registered otg_notify +\n");
	if (!n) {
		pr_err("otg notify structure is null\n");
		ret = -EFAULT;
		goto err1;
	}
	u_notify->o_notify = n;

	ATOMIC_INIT_NOTIFIER_HEAD(&u_notify->o_notify->otg_notifier);
	u_notify->otg_nb.notifier_call = otg_notifier_callback;
	ret = atomic_notifier_chain_register(&u_notify->o_notify->otg_notifier,
				&u_notify->otg_nb);
	if (ret < 0) {
		pr_err("atomic_notifier_chain_register failed\n");
		goto err1;
	}

	BLOCKING_INIT_NOTIFIER_HEAD(&u_notify->o_notify->extra_notifier);
	u_notify->extra_nb.notifier_call = extra_notifier_callback;
	ret = blocking_notifier_chain_register
		(&u_notify->o_notify->extra_notifier, &u_notify->extra_nb);
	if (ret < 0) {
		pr_err("blocking_notifier_chain_register failed\n");
		goto err2;
	}

	if (!n->unsupport_host) {
		u_notify->ndev.name = "usb_otg";
		u_notify->ndev.set_booster = n->vbus_drive;
		ret = host_notify_dev_register(&u_notify->ndev);
		if (ret < 0) {
			pr_err("host_notify_dev_register is failed\n");
			goto err3;
		}

		if (!n->vbus_drive) {
			pr_err("vbus_drive is null\n");
			goto err4;
		}
	}

	u_notify->udev.name = "usb_control";
	u_notify->udev.set_disable = set_notify_disable;
	u_notify->udev.o_notify = n;
	ret = usb_notify_dev_register(&u_notify->udev);
	if (ret < 0) {
		pr_err("usb_notify_dev_register is failed\n");
		goto err4;
	}

	if (gpio_is_valid(n->vbus_detect_gpio) ||
			gpio_is_valid(n->redriver_en_gpio)) {
		ret = register_gpios(n);
		if (ret < 0) {
			pr_err("register_gpios is failed\n");
			goto err5;
		}
	}

	if (n->is_wakelock)
		wake_lock_init(&u_notify->wlock,
			WAKE_LOCK_SUSPEND, "usb_notify");

	if (n->booting_delay_sec) {
		INIT_DELAYED_WORK(&u_notify->b_delay.booting_work,
				  reserve_state_check);
		schedule_delayed_work(&u_notify->b_delay.booting_work,
				n->booting_delay_sec*HZ);
	}
	register_usbdev_notify();

	pr_info("registered otg_notify -\n");
	return 0;
err5:
	usb_notify_dev_unregister(&u_notify->udev);
err4:
	if (!n->unsupport_host)
		host_notify_dev_unregister(&u_notify->ndev);
err3:
	blocking_notifier_chain_unregister(&u_notify->o_notify->extra_notifier,
				&u_notify->extra_nb);
err2:
	atomic_notifier_chain_unregister(&u_notify->o_notify->otg_notifier,
				&u_notify->otg_nb);
err1:
	u_notify->o_notify = NULL;
err:
	return ret;
}
EXPORT_SYMBOL(set_otg_notify);

void put_otg_notify(struct otg_notify *n)
{
	if (n->booting_delay_sec)
		cancel_delayed_work_sync(&u_notify->b_delay.booting_work);
	if (n->is_wakelock)
		wake_lock_destroy(&u_notify->wlock);
	if (gpio_is_valid(n->vbus_detect_gpio))
		free_irq(gpio_to_irq(n->vbus_detect_gpio), NULL);
	host_notify_dev_unregister(&u_notify->ndev);
	blocking_notifier_chain_unregister(&u_notify->o_notify->extra_notifier,
				&u_notify->extra_nb);
	atomic_notifier_chain_unregister(&u_notify->o_notify->otg_notifier,
				&u_notify->otg_nb);
	u_notify->o_notify = NULL;
}
EXPORT_SYMBOL(put_otg_notify);

static int __init usb_notify_init(void)
{
	return create_usb_notify();
}

static void __exit usb_notify_exit(void)
{
	if (!u_notify)
		return;
	usb_notify_class_exit();
	flush_workqueue(u_notify->notifier_wq);
	destroy_workqueue(u_notify->notifier_wq);
	kfree(u_notify);
}

module_init(usb_notify_init);
module_exit(usb_notify_exit);

MODULE_AUTHOR("Samsung USB Team");
MODULE_DESCRIPTION("USB Notify Layer");
MODULE_LICENSE("GPL");
