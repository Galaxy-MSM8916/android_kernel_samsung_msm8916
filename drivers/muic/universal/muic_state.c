/*
 * muic_state.c
 *
 * Copyright (C) 2014 Samsung Electronics
 * Thomas Ryu <smilesr.ryu@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/host_notify.h>

#include <linux/muic/muic.h>

#if defined(CONFIG_MUIC_NOTIFIER)
#include <linux/muic/muic_notifier.h>
#endif /* CONFIG_MUIC_NOTIFIER */

#if defined(CONFIG_VBUS_NOTIFIER)
#include <linux/vbus_notifier.h>
#endif /* CONFIG_VBUS_NOTIFIER */

#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif /* CONFIG_OF */

#include "muic-internal.h"
#include "muic_apis.h"
#include "muic_i2c.h"
#include "muic_vps.h"
#include "muic_regmap.h"

extern int muic_wakeup_noti;

void muic_set_wakeup_noti(int flag)
{
	pr_info("%s: %d\n", __func__, flag);
	muic_wakeup_noti = flag;
}

static void muic_handle_attach(muic_data_t *pmuic,
			muic_attached_dev_t new_dev, int adc, u8 vbvolt)
{
#if defined(CONFIG_MUIC_UNIVERSAL_SM5705_AFC)
	struct afc_ops *afcops = pmuic->regmapdesc->afcops;
#endif
	int ret = 0;
	bool noti_f = true;

	pr_info("%s:%s attached_dev:%d new_dev:%d adc:0x%02x, vbvolt:%02x\n",
		MUIC_DEV_NAME, __func__, pmuic->attached_dev, new_dev, adc, vbvolt);

	if((new_dev == pmuic->attached_dev) &&
		(new_dev != ATTACHED_DEV_JIG_UART_OFF_MUIC)) {
		pr_info("%s:%s Duplicated device %d just ignore\n",
				MUIC_DEV_NAME, __func__,pmuic->attached_dev);
		return;
	}
	switch (pmuic->attached_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_usb(pmuic);
		}
		break;
	case ATTACHED_DEV_OTG_MUIC:
	/* OTG -> LANHUB, meaning TA is attached to LANHUB(OTG) */
		if (new_dev == ATTACHED_DEV_USB_LANHUB_MUIC) {
			pr_info("%s:%s OTG+TA=>LANHUB. Do not detach OTG.\n",
					__func__, MUIC_DEV_NAME);
			noti_f = false;
			break;
		}

		if (new_dev == pmuic->attached_dev) {
			noti_f = false;
			break;
		}
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_otg_usb(pmuic);
		}
		break;

	case ATTACHED_DEV_AUDIODOCK_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_audiodock(pmuic);
		}
		break;

	case ATTACHED_DEV_TA_MUIC:
		pmuic->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		if (new_dev != ATTACHED_DEV_JIG_UART_OFF_MUIC) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_jig_uart_boot_off(pmuic);
		}
		break;

	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
				MUIC_DEV_NAME, __func__, new_dev,
				pmuic->attached_dev);

			if (pmuic->is_factory_start)
				ret = detach_deskdock(pmuic);
			else
				ret = detach_jig_uart_boot_on(pmuic);

			muic_set_wakeup_noti(pmuic->is_factory_start ? 1: 0);
		}
		break;
	case ATTACHED_DEV_DESKDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_VB_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_deskdock(pmuic);
		}
		break;
	case ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_otg_usb(pmuic);
		}
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		if (new_dev != pmuic->attached_dev) {
			pr_warn("%s:%s new(%d)!=attached(%d), assume detach\n",
					MUIC_DEV_NAME, __func__, new_dev,
					pmuic->attached_dev);
			ret = detach_ps_cable(pmuic);
		}
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		break;

	default:
		noti_f = false;
	}

	if (noti_f)
		muic_notifier_detach_attached_dev(pmuic->attached_dev);

	noti_f = true;
	switch (new_dev) {
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
		ret = attach_usb(pmuic, new_dev);
		break;
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		ret = attach_otg_usb(pmuic, new_dev);
		break;
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		ret = attach_audiodock(pmuic, new_dev, vbvolt);
		break;
	case ATTACHED_DEV_TA_MUIC:
		attach_ta(pmuic);
#if defined(CONFIG_MUIC_UNIVERSAL_SM5705_AFC)
		if (pmuic->irq_n == -1) {
	            pmuic->attached_dev = new_dev;
	                muic_notifier_attach_attached_dev(new_dev);;
			afcops->afc_init_check(pmuic->regmapdesc);
			noti_f = false;
		} else
#endif
		{
			pmuic->attached_dev = new_dev;
		}
		break;
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		new_dev = attach_jig_uart_boot_off(pmuic, new_dev, vbvolt);
		break;
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		/* Keep AP UART path and
		 *  call attach_deskdock to wake up the device in the Facory Build Binary.
		 */
		 if (pmuic->is_factory_start)
			ret = attach_deskdock(pmuic, new_dev);
		else
			ret = attach_jig_uart_boot_on(pmuic, new_dev);

		muic_set_wakeup_noti(pmuic->is_factory_start ? 1: 0);
		break;
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
		ret = attach_jig_usb_boot_off(pmuic, vbvolt);
		break;
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
		ret = attach_jig_usb_boot_on(pmuic, vbvolt);
		break;
	case ATTACHED_DEV_MHL_MUIC:
		ret = attach_mhl(pmuic);
		break;
	case ATTACHED_DEV_DESKDOCK_MUIC:
		if (vbvolt)
			new_dev = ATTACHED_DEV_DESKDOCK_VB_MUIC;
		ret = attach_deskdock(pmuic, new_dev);
		break;
	case ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC:
		ret = attach_otg_usb(pmuic, new_dev);
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		ret = attach_ps_cable(pmuic, new_dev);
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		com_to_open_with_vbus(pmuic);
		break;
	case ATTACHED_DEV_VZW_INCOMPATIBLE_MUIC:
		com_to_open_with_vbus(pmuic);
		break;
	default:
		pr_warn("%s:%s unsupported dev=%d, adc=0x%x, vbus=%c\n",
				MUIC_DEV_NAME, __func__, new_dev, adc,
				(vbvolt ? 'O' : 'X'));
		break;
	}

	if (noti_f)
		muic_notifier_attach_attached_dev(new_dev);
	else
		pr_info("%s:%s attach Noti. for (%d) discarded.\n",
				MUIC_DEV_NAME, __func__, new_dev);

	if (ret < 0)
		pr_warn("%s:%s something wrong with attaching %d (ERR=%d)\n",
				MUIC_DEV_NAME, __func__, new_dev, ret);
}

static void muic_handle_detach(muic_data_t *pmuic)
{
	int ret = 0;
	bool noti_f = true;

	ret = com_to_open_with_vbus(pmuic);

	//muic_enable_accdet(pmuic);

	switch (pmuic->attached_dev) {
	case ATTACHED_DEV_JIG_USB_OFF_MUIC:
	case ATTACHED_DEV_JIG_USB_ON_MUIC:
	case ATTACHED_DEV_USB_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
		ret = detach_usb(pmuic);
		break;
	case ATTACHED_DEV_OTG_MUIC:
	case ATTACHED_DEV_USB_LANHUB_MUIC:
		ret = detach_otg_usb(pmuic);
		break;
	case ATTACHED_DEV_TA_MUIC:
		pmuic->attached_dev = ATTACHED_DEV_NONE_MUIC;
		detach_ta(pmuic);
		break;
	case ATTACHED_DEV_JIG_UART_OFF_VB_MUIC:
	case ATTACHED_DEV_JIG_UART_OFF_MUIC:
		ret = detach_jig_uart_boot_off(pmuic);
		break;
	case ATTACHED_DEV_JIG_UART_ON_MUIC:
		if (pmuic->is_factory_start)
			ret = detach_deskdock(pmuic);
		else {
			noti_f = false;
			ret = detach_jig_uart_boot_on(pmuic);
		}
		break;
	case ATTACHED_DEV_DESKDOCK_MUIC:
	case ATTACHED_DEV_DESKDOCK_VB_MUIC:
		ret = detach_deskdock(pmuic);
		break;
	case ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC:
		ret = detach_otg_usb(pmuic);
		break;
	case ATTACHED_DEV_AUDIODOCK_MUIC:
		ret = detach_audiodock(pmuic);
		break;
	case ATTACHED_DEV_MHL_MUIC:
		ret = detach_mhl(pmuic);
		break;
	case ATTACHED_DEV_CHARGING_CABLE_MUIC:
		ret = detach_ps_cable(pmuic);
		break;
	case ATTACHED_DEV_NONE_MUIC:
		pmuic->is_afc_device = 0;
		pr_info("%s:%s duplicated(NONE)\n", MUIC_DEV_NAME, __func__);
		break;
	case ATTACHED_DEV_UNDEFINED_CHARGING_MUIC:
		pr_info("%s:%s UNKNOWN\n", MUIC_DEV_NAME, __func__);
		pmuic->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	default:
		pr_info("%s:%s invalid attached_dev type(%d)\n", MUIC_DEV_NAME,
			__func__, pmuic->attached_dev);
		pmuic->attached_dev = ATTACHED_DEV_NONE_MUIC;
		break;
	}

	if (noti_f)
	muic_notifier_detach_attached_dev(pmuic->attached_dev);

	else
		pr_info("%s:%s detach Noti. for (%d) discarded.\n",
				MUIC_DEV_NAME, __func__, pmuic->attached_dev);
	if (ret < 0)
		pr_warn("%s:%s something wrong with detaching %d (ERR=%d)\n",
				MUIC_DEV_NAME, __func__, pmuic->attached_dev, ret);

}

void muic_detect_dev(muic_data_t *pmuic)
{
	muic_attached_dev_t new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
	int intr = MUIC_INTR_DETACH;

	get_vps_data(pmuic, &pmuic->vps);


	pr_info("%s:%s dev[1:0x%x, 2:0x%x, 3:0x%x], adc:0x%x, vbvolt:0x%x\n",
		MUIC_DEV_NAME, __func__, pmuic->vps.s.val1, pmuic->vps.s.val2,
		pmuic->vps.s.val3, pmuic->vps.s.adc, pmuic->vps.s.vbvolt);


	vps_resolve_dev(pmuic, &new_dev, &intr);

	if (intr == MUIC_INTR_ATTACH) {
		muic_handle_attach(pmuic, new_dev,
			pmuic->vps.s.adc, pmuic->vps.s.vbvolt);
	} else {
		muic_handle_detach(pmuic);
	}
#if defined(CONFIG_VBUS_NOTIFIER)
	vbus_notifier_handle(!!pmuic->vps.s.vbvolt ? STATUS_VBUS_HIGH : STATUS_VBUS_LOW);
#endif /* CONFIG_VBUS_NOTIFIER */

}
