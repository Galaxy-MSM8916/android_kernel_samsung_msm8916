/*
 * muic_vps.c
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

#if defined (CONFIG_OF)
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#endif /* CONFIG_OF */

#include "muic-internal.h"
#include "muic_apis.h"
#include "muic_i2c.h"
#include "muic_vps.h"

/* Device Type 1 register */
#define DEV_TYPE1_USB_OTG		(0x1 << 7)
#define DEV_TYPE1_DEDICATED_CHG		(0x1 << 6)
#define DEV_TYPE1_CDP			(0x1 << 5)
#define DEV_TYPE1_T1_T2_CHG		(0x1 << 4)
#define DEV_TYPE1_UART			(0x1 << 3)
#define DEV_TYPE1_USB			(0x1 << 2)
#define DEV_TYPE1_AUDIO_2		(0x1 << 1)
#define DEV_TYPE1_AUDIO_1		(0x1 << 0)
#define DEV_TYPE1_USB_TYPES	(DEV_TYPE1_USB_OTG | DEV_TYPE1_CDP | \
				DEV_TYPE1_USB)
#define DEV_TYPE1_CHG_TYPES	(DEV_TYPE1_DEDICATED_CHG | DEV_TYPE1_CDP)

/* Device Type 2 register */
#define DEV_TYPE2_AV			(0x1 << 6)
#define DEV_TYPE2_TTY			(0x1 << 5)
#define DEV_TYPE2_PPD			(0x1 << 4)
#define DEV_TYPE2_JIG_UART_OFF		(0x1 << 3)
#define DEV_TYPE2_JIG_UART_ON		(0x1 << 2)
#define DEV_TYPE2_JIG_USB_OFF		(0x1 << 1)
#define DEV_TYPE2_JIG_USB_ON		(0x1 << 0)
#define DEV_TYPE2_JIG_USB_TYPES		(DEV_TYPE2_JIG_USB_OFF | \
					DEV_TYPE2_JIG_USB_ON)
#define DEV_TYPE2_JIG_UART_TYPES	(DEV_TYPE2_JIG_UART_OFF)
#define DEV_TYPE2_JIG_TYPES		(DEV_TYPE2_JIG_UART_TYPES | \
					DEV_TYPE2_JIG_USB_TYPES)

/* Device Type 3 register */
#define DEV_TYPE3_U200_CHG		(0x1 << 6)
#define DEV_TYPE3_AV_WITH_VBUS		(0x1 << 4)
#define DEV_TYPE3_NO_STD_CHG		(0x1 << 2)
#define DEV_TYPE3_MHL			(0x1 << 0)
#define DEV_TYPE3_CHG_TYPE	(DEV_TYPE3_U200_CHG | DEV_TYPE3_NO_STD_CHG)

static struct vps_cfg cfg_MHL = {
	.name = "MHL",
	.attr = MATTR(VCOM_OPEN, VB_ANY)
};
static struct vps_cfg cfg_OTG = {
	.name = "OTG",
	.attr = MATTR(VCOM_OPEN, VB_CHK),
};
static struct vps_cfg cfg_VZW_ACC = {
	.name = "VZW Accessory",
	.attr = MATTR(VCOM_OPEN, VB_ANY),
};
static struct vps_cfg cfg_VZW_INCOMPATIBLE = {
	.name = "VZW Incompatible",
	.attr = MATTR(VCOM_OPEN, VB_ANY),
};
static struct vps_cfg cfg_SMARTDOCK = {
	.name = "Smartdock",
	.attr = MATTR(VCOM_OPEN, VB_CHK),
};
static struct vps_cfg cfg_HMT = {
	.name = "HMT",
	.attr = MATTR(VCOM_USB, VB_ANY),
};
static struct vps_cfg cfg_AUDIODOCK = {
	.name = "Audiodock",
	.attr = MATTR(VCOM_USB, VB_HIGH),
};
static struct vps_cfg cfg_USB_LANHUB = {
	.name = "USB LANHUB",
	.attr = MATTR(VCOM_OPEN, VB_ANY),
};
static struct vps_cfg cfg_CHARGING_CABLE = {
	.name = "Charging Cable",
	.attr = MATTR(VCOM_USB, VB_ANY),
};
static struct vps_cfg cfg_UNIVERSAL_MMDOCK = {
	.name = "Universal Multimedia dock",
	.attr = MATTR(VCOM_USB, VB_HIGH),
};
static struct vps_cfg cfg_JIG_USB_OFF = {
	.name = "Jig USB Off",
	.attr = MATTR(VCOM_USB, VB_HIGH) | MATTR_FACT_SUPP,
};
static struct vps_cfg cfg_JIG_USB_ON = {
	.name = "Jig USB On",
	.attr = MATTR(VCOM_USB, VB_HIGH) | MATTR_FACT_SUPP,
};
static struct vps_cfg cfg_DESKDOCK = {
	.name = "Deskdock",
	.attr = MATTR(VCOM_OPEN, VB_CHK),
};
static struct vps_cfg cfg_TYPE2_CHG = {
	.name = "TYPE2 Charger",
	.attr = MATTR(VCOM_OPEN, VB_ANY),
};
static struct vps_cfg cfg_JIG_UART_OFF = {
	.name = "Jig UART Off",
	.attr = MATTR(VCOM_UART, VB_ANY) | MATTR_FACT_SUPP,
};
//CONFIG_SEC_FACTORY
static struct vps_cfg cfg_JIG_UART_ON = {
	.name = "Jig UART On",
	.attr = MATTR(VCOM_UART, VB_ANY) | MATTR_FACT_SUPP,
};
static struct vps_cfg cfg_TA = {
	.name = "TA",
	.attr = MATTR(VCOM_OPEN, VB_HIGH) | MATTR_CDET_SUPP,
};
static struct vps_cfg cfg_USB = {
	.name = "USB",
	.attr = MATTR(VCOM_OPEN, VB_HIGH) | MATTR_CDET_SUPP,
};
static struct vps_cfg cfg_CDP = {
	.name = "CDP",
	.attr = MATTR(VCOM_OPEN, VB_HIGH) | MATTR_CDET_SUPP,
};
static struct vps_cfg cfg_UNDEFINED_CHARGING = {
	.name = "Undefined Charging",
	.attr = MATTR(VCOM_OPEN, VB_HIGH) | MATTR_SUPP,
};

static struct vps_tbl_data vps_table[] = {
	[MDEV(OTG)]			= {0x00, "GND",	&cfg_OTG,},
	[MDEV(MHL)]			= {0xfe, "1K",		&cfg_MHL,},
	/* 0x01 ~ 0x0D : Remote Sx Button */
	[MDEV(VZW_ACC)]		= {0x0e, "28.7K",	&cfg_VZW_ACC,},
	[MDEV(VZW_INCOMPATIBLE)]	= {0x0f, "34K",	&cfg_VZW_INCOMPATIBLE,},
	[MDEV(SMARTDOCK)]		= {0x10, "40.2K",	&cfg_SMARTDOCK,},
	[MDEV(HMT)]			= {0x11, "49.9K",	&cfg_HMT,},
	[MDEV(AUDIODOCK)]		= {0x12, "64.9K",	&cfg_AUDIODOCK,},
	[MDEV(USB_LANHUB)]		= {0x13, "80.07K",	&cfg_USB_LANHUB,},
	[MDEV(CHARGING_CABLE)]	= {0x14, "102K",	&cfg_CHARGING_CABLE,},
	[MDEV(UNIVERSAL_MMDOCK)]	= {0x15, "121K",	&cfg_UNIVERSAL_MMDOCK,},
	/* 0x16: UART Cable */
	/* 0x17: CEA-936A Type 1 Charger */
	[MDEV(JIG_USB_OFF)]		= {0x18, "255K",	&cfg_JIG_USB_OFF,},
	[MDEV(JIG_USB_ON)]		= {0x19, "301K",	&cfg_JIG_USB_ON,},
	[MDEV(DESKDOCK)]		= {0x1a, "365K",	&cfg_DESKDOCK,},
	[MDEV(TYPE2_CHG)]		= {0x1b, "442K",	&cfg_TYPE2_CHG,},
	[MDEV(JIG_UART_OFF)]		= {0x1c, "523K",	&cfg_JIG_UART_OFF,},
	[MDEV(JIG_UART_ON)]		= {0x1d, "619K",	&cfg_JIG_UART_ON,},
	/* 0x1e: Audio Mode with Remote */
	[MDEV(TA)]			= {0x1f, "OPEN",	&cfg_TA,},
	[MDEV(USB)]			= {0x1f, "OPEN",	&cfg_USB,},
	[MDEV(CDP)]			= {0x1f, "OPEN",	&cfg_CDP,},
	[MDEV(UNDEFINED_CHARGING)]	= {0xfe, "UNDEFINED",	&cfg_UNDEFINED_CHARGING,},
	[ATTACHED_DEV_NUM]		= {0x00, "NUM", NULL,},
};

struct vps_tbl_data * mdev_to_vps(muic_attached_dev_t mdev)
{
	if (mdev >= ATTACHED_DEV_NUM) {
		pr_err("%s Out of range mdev=%d\n", __func__, mdev);
		return NULL;
	}

	return &vps_table[mdev];
}

bool vps_name_to_mdev(const char *name, int *sdev)
{
	struct vps_tbl_data *pvps;
	int mdev;

	for (mdev = MDEV(NONE); mdev < ATTACHED_DEV_NUM; mdev++) {
		pvps = &vps_table[mdev];
		if (!pvps->cfg)
			continue;

		if (!strcmp(pvps->cfg->name, name)){
			break;
		}
	}

	if (mdev >= ATTACHED_DEV_NUM) {
		pr_err("%s Out of range mdev=%d, %s\n", __func__,
						mdev, name);
		return false;
	}

	pr_info("%s:%s->[%2d]\n", __func__, pvps->cfg->name, mdev);

	*sdev = mdev;

	return true;
}

bool vps_is_supported_dev(muic_attached_dev_t mdev)
{
	struct vps_tbl_data *pvps = mdev_to_vps(mdev);
	int attr;

	if (!pvps || !pvps->cfg)
		return false;

	attr = pvps->cfg->attr;

	if (MATTR_TO_SUPP(attr))
		return true;

	return false;
}

void vps_update_supported_attr(muic_attached_dev_t mdev, bool supported)
{
	struct vps_tbl_data *pvps = mdev_to_vps(mdev);

	if (!pvps || !pvps->cfg)
		return;

	if (supported)
		pvps->cfg->attr |= MATTR_SUPP;
	else
		pvps->cfg->attr &= (~MATTR_SUPP) & 0xFFFFFFFF;
}

bool vps_is_factory_dev(muic_attached_dev_t mdev)
{
	struct vps_tbl_data *pvps = mdev_to_vps(mdev);
	int attr;

	if (!pvps || !pvps->cfg)
		return false;

	attr = pvps->cfg->attr;

	if (MATTR_TO_FACT(attr))
		return true;

	return false;
}

static bool vps_is_1k_mhl_cable(vps_data_t *pmsr)
{
#define STATUS1_ADC1K_SHIFT		7
	u8 adc1k = 0x1 << STATUS1_ADC1K_SHIFT; /* VPS_VENDOR */

	return (pmsr->t.adc1k == adc1k) ? true : false;
}

static bool vps_is_adc(vps_data_t *pmsr, struct vps_tbl_data *pvps)
{
	if (pmsr->t.adc == pvps->adc)
		return true;

	 return false;
}


static bool vps_is_vbvolt(vps_data_t *pmsr, struct vps_tbl_data *pvps)
{
	int attr = pvps->cfg->attr;

	if (pmsr->t.vbvolt == MATTR_TO_VBUS(attr))
		return true;

	if (MATTR_TO_VBUS(attr) == VB_ANY)
		return true;

	 return false;
}

int resolve_dev_based_on_adc_chgtype(muic_data_t *pmuic, vps_data_t *pmsr)
{
	int dev_type;

	pr_info("%s: adc=%02x, chgtyp=%02x\n",__func__, pmsr->t.adc, pmsr->t.chgtyp);

	switch(pmsr->t.adc){
	case ADC_OPEN:
		if(pmsr->t.chgtyp == CHGTYP_DEDICATED_CHARGER)
			dev_type = ATTACHED_DEV_TA_MUIC;
		else if(pmsr->t.chgtyp == CHGTYP_UNOFFICIAL_CHARGER)
			dev_type = ATTACHED_DEV_TA_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_USB)
                        dev_type= ATTACHED_DEV_USB_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_CDP)
                        dev_type = ATTACHED_DEV_CDP_MUIC;
                else{
			pr_info("%s not able to resolve using ADC and CHGTYPE\n",__func__);
	                return -1;
		}
		break;
	case ADC_219:
		if(pmsr->t.chgtyp == CHGTYP_DEDICATED_CHARGER)
                        dev_type = ATTACHED_DEV_TA_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_UNOFFICIAL_CHARGER)
                        dev_type = ATTACHED_DEV_TA_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_CDP)
                        dev_type = ATTACHED_DEV_CDP_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_USB)
                        dev_type = ATTACHED_DEV_USB_MUIC;
                else {
                        pr_info("%s not able to resolve using ADC and CHGTYPE\n",__func__);
                        return -1;
                }
		break;
	case ADC_SMARTDOCK:
		if(pmsr->t.chgtyp == CHGTYP_DEDICATED_CHARGER)
                        dev_type = ATTACHED_DEV_SMARTDOCK_MUIC;
                else if(pmsr->t.chgtyp == CHGTYP_USB)
                        dev_type = ATTACHED_DEV_SMARTDOCK_MUIC;
                else{
                        pr_info("%s not able to resolve using ADC and CHGTYPE\n",__func__);
                        return -1;
                }
		break;
	default:
		pr_info("%s not able to resolve using ADC and CHGTYPE\n",__func__);
		return -1;
	}

	return dev_type;
}

int vps_find_attached_dev(muic_data_t *pmuic, muic_attached_dev_t *pdev, int *pintr)
{
	struct vps_tbl_data *pvps;
	muic_attached_dev_t new_dev = ATTACHED_DEV_UNKNOWN_MUIC, mdev;
	vps_data_t *pmsr = &pmuic->vps;
	int attr;
	int intr = MUIC_INTR_ATTACH;

	pr_info("%s\n",__func__);

	for (mdev = MDEV(NONE); mdev < ATTACHED_DEV_NUM; mdev++) {
		pvps = &vps_table[mdev];
		if (!pvps->cfg)
			continue;

		attr = pvps->cfg->attr;

		if (vps_is_1k_mhl_cable(pmsr)) {
			new_dev = mdev;
			break;
		}

		if (!vps_is_adc(pmsr, pvps))
			continue;

		if (!vps_is_vbvolt(pmsr, pvps))
			continue;

		if(MATTR_TO_CDET(attr)){
			/* some function for cdeten check */
			new_dev = resolve_dev_based_on_adc_chgtype(pmuic,pmsr);
			if(new_dev != -1)
				break;
		}

		pr_info("%s:%s vps table match found at mdev:%d(%s)\n",
				MUIC_DEV_NAME, __func__, mdev, pvps->cfg->name);
		new_dev = mdev;
		break;
	}

	if (mdev == ATTACHED_DEV_NUM) {
		if (pmsr->t.vbvolt == VB_HIGH) {
			new_dev = ATTACHED_DEV_UNDEFINED_CHARGING_MUIC;
			pr_info("%s:%s unsupported ID + VB\n", MUIC_DEV_NAME, __func__);
		}
		else
		{
			intr = MUIC_INTR_DETACH;
			new_dev = ATTACHED_DEV_NONE_MUIC;
			if(pvps->cfg)
				pr_info("%s:%s vps table match found at mdev:%d(%s)\n",
						MUIC_DEV_NAME, __func__, mdev, pvps->cfg->name);
		}
	}

	*pintr = intr;
	*pdev = new_dev;

	return new_dev;
}

/*
  * vps_show_tablbe() functions displays the VPS table as a below format
dev   ADC      (RID)         attr                     vps_name
 --------------------------------------------------------------
[ 1] = 1f(      OPEN) 00000521:S:_                          USB
[ 2] = 1f(      OPEN) 00000521:S:_                          CDP
[ 3] = 00(       GND) 00000122:S:_                          OTG
[ 4] = 1f(      OPEN) 00000521:S:_                           TA
[12] = fe( UNDEFINED) 00000121:S:_           Undefined Charging
[13] = 1a(      365K) 00000122:S:_                     Deskdock
[16] = 1c(      523K) 00000333:S:F                 Jig UART Off
[20] = 1d(      619K) 00000333:S:F                  Jig UART On
[21] = 18(      255K) 00000351:S:F                  Jig USB Off
[22] = 19(      301K) 00000351:S:F                   Jig USB On
[23] = 10(     40.2K) 00000022:_:_                    Smartdock
[27] = 15(      121K) 00000151:S:_    Universal Multimedia dock
[28] = 12(     64.9K) 00000051:_:_                    Audiodock
[29] = fe(        1K) 00000123:S:_                          MHL
[30] = 14(      102K) 00000153:S:_               Charging Cable
[45] = 11(     49.9K) 00000053:_:_                          HMT
[46] = 0e(     28.7K) 00000023:_:_                VZW Accessory
[47] = 0f(       34K) 00000023:_:_             VZW Incompatible
[48] = 13(    80.07K) 00000123:S:_                   USB LANHUB
[49] = 1b(      442K) 00000123:S:_                TYPE2 Charger
*/
void vps_show_table(void)
{
	struct vps_tbl_data *pvps;
	int mdev, attr;

	pr_info(" %4s%6s%10s %12s %29s\n", "dev", "ADC", "(RID)", "attr", "vps_name");
	for (mdev = MDEV(NONE); mdev < ATTACHED_DEV_NUM; mdev++) {

		pvps = &vps_table[mdev];

		if (!pvps->cfg)
			continue;

		attr = pvps->cfg->attr;

		pr_info(" [%2d] = %02x(%10s) %08x:%c:%c %28s\n", mdev,
			pvps->adc, pvps->rid, pvps->cfg->attr,
			MATTR_TO_SUPP(attr) ? 'S' : '_',
			MATTR_TO_FACT(attr) ? 'F' : '_',
			pvps->cfg->name);
	}

	pr_info("done.\n");

}

void vps_show_supported_list(void)
{
	struct vps_tbl_data *pvps;
	int mdev, attr;

	pr_info(" %4s%6s%10s %12s %30s\n", "dev", "ADC", "(RID)", "attr", "vps_name");
	for (mdev = MDEV(NONE); mdev < ATTACHED_DEV_NUM; mdev++) {

		pvps = &vps_table[mdev];

		if (!pvps->cfg)
			continue;

		attr = pvps->cfg->attr;

		if (!MATTR_TO_SUPP(attr))
			continue;

		pr_info(" [%2d] = %02x(%10s) %08x:%c:%c %28s\n", mdev,
			pvps->adc, pvps->rid, pvps->cfg->attr,
			MATTR_TO_SUPP(attr) ? 'S' : '_',
			MATTR_TO_FACT(attr) ? 'F' : '_',
			pvps->cfg->name);

	}

	pr_info("done.\n");

}

static int resolve_dedicated_dev(muic_data_t *pmuic, muic_attached_dev_t *pdev, int *pintr)
{
	muic_attached_dev_t new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
	int intr = MUIC_INTR_DETACH;
	int vbvolt = 0, adc = 0;

#if defined(CONFIG_MUIC_UNIVERSAL_SM5504)
	intr = MUIC_INTR_ATTACH;
	new_dev = pmuic->vps.t.attached_dev;
	adc = pmuic->vps.s.adc;
	vbvolt = pmuic->vps.s.vbvolt;
#else
	int val1, val2, val3;

	val1 = pmuic->vps.s.val1;
	val2 = pmuic->vps.s.val2;
	val3 = pmuic->vps.s.val3;
	adc = pmuic->vps.s.adc;
	vbvolt = pmuic->vps.s.vbvolt;

	/* Attached */
	switch (val1) {
	case DEV_TYPE1_CDP:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_CDP_MUIC;
		pr_info("%s : USB_CDP DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE1_USB:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_USB_MUIC;
		pr_info("%s : USB DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE1_DEDICATED_CHG:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_TA_MUIC;
		pr_info("%s : DEDICATED CHARGER DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE1_USB_OTG:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_OTG_MUIC;
		pr_info("%s : USB_OTG DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE1_AUDIO_2:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_USB_LANHUB_MUIC;
		pr_info("%s : LANHUB DETECTED\n", MUIC_DEV_NAME);
		break;

	default:
		break;
	}

	switch (val2) {
	case DEV_TYPE2_JIG_UART_OFF:
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_UART_OFF_MUIC;
		pr_info("%s : JIG_UART_OFF DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE2_JIG_USB_OFF:
		if (!vbvolt) break;
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_USB_OFF_MUIC;
		pr_info("%s : JIG_USB_OFF DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE2_JIG_USB_ON:
		if (!vbvolt) break;
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_JIG_USB_ON_MUIC;
		pr_info("%s : JIG_USB_ON DETECTED\n", MUIC_DEV_NAME);
		break;
	case DEV_TYPE2_TTY:
		if (!vbvolt) break;
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC;
		pr_info("%s : UNIVERSAL_MMDOCK DETECTED\n", MUIC_DEV_NAME);
		break;

	default:
		break;
	}

	if (val3 & DEV_TYPE3_CHG_TYPE)
	{
		intr = MUIC_INTR_ATTACH;

		if (val3 & DEV_TYPE3_NO_STD_CHG) {
			new_dev = ATTACHED_DEV_USB_MUIC;
			pr_info("%s : TYPE3 DCD_OUT_TIMEOUT DETECTED\n", MUIC_DEV_NAME);

		} else {
			new_dev = ATTACHED_DEV_TA_MUIC;
			pr_info("%s : TYPE3_CHARGER DETECTED\n", MUIC_DEV_NAME);
		}
	}

	if (val2 & DEV_TYPE2_AV || val3 & DEV_TYPE3_AV_WITH_VBUS)
	{
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_DESKDOCK_MUIC;
		pr_info("%s : DESKDOCK DETECTED\n", MUIC_DEV_NAME);
	}

	if (val3 & DEV_TYPE3_MHL)
	{
		intr = MUIC_INTR_ATTACH;
		new_dev = ATTACHED_DEV_MHL_MUIC;
		pr_info("%s : MHL DETECTED\n", MUIC_DEV_NAME);
	}
#endif

	/* If there is no matching device found using device type registers
		use ADC to find the attached device */
	if(new_dev == ATTACHED_DEV_UNKNOWN_MUIC) {
		switch (adc) {
		case ADC_USB_LANHUB:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_USB_LANHUB_MUIC;
			pr_info("%s : LANHUB DETECTED\n", MUIC_DEV_NAME);
			break;

		case ADC_CEA936ATYPE1_CHG : /*200k ohm */
		{
			/* For LG USB cable which has 219k ohm ID */
			int rescanned_dev = do_BCD_rescan(pmuic);

			if (rescanned_dev > 0) {
				pr_info("%s : TYPE1 CHARGER DETECTED(USB)\n", MUIC_DEV_NAME);
				intr = MUIC_INTR_ATTACH;
				new_dev = rescanned_dev;
			}
			break;
		}
		case ADC_CEA936ATYPE2_CHG:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_TA_MUIC;
			pr_info("%s : TYPE1/2 CHARGER DETECTED(TA)\n", MUIC_DEV_NAME);
			break;
		case ADC_JIG_USB_OFF: /* 255k */
			if (!vbvolt) break;
			if (new_dev != ATTACHED_DEV_JIG_USB_OFF_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_USB_OFF_MUIC;
				pr_info("%s : ADC JIG_USB_OFF DETECTED\n", MUIC_DEV_NAME);
			}
			break;
		case ADC_JIG_USB_ON:
			if (!vbvolt) break;
			if (new_dev != ATTACHED_DEV_JIG_USB_ON_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_USB_ON_MUIC;
				pr_info("%s : ADC JIG_USB_ON DETECTED\n", MUIC_DEV_NAME);
			}
			break;
		case ADC_JIG_UART_OFF:
			if (new_dev != ATTACHED_DEV_JIG_UART_OFF_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_UART_OFF_MUIC;
				pr_info("%s : ADC JIG_UART_OFF DETECTED\n", MUIC_DEV_NAME);
			}
			break;
		case ADC_JIG_UART_ON:
			/* This is the mode to wake up device during factory mode.
			 *  This device type SHOULD be handled in muic_state.c to
			 *  support both factory & rustproof mode.
			 */
			if (new_dev != ATTACHED_DEV_JIG_UART_ON_MUIC) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_JIG_UART_ON_MUIC;
				pr_info("%s : ADC JIG_UART_ON DETECTED\n", MUIC_DEV_NAME);
			}
			break;
#ifdef CONFIG_MUIC_SM5703_SUPPORT_AUDIODOCK
		case ADC_AUDIODOCK:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_AUDIODOCK_MUIC;
			pr_info("%s : ADC AUDIODOCK DETECTED\n", MUIC_DEV_NAME);
			break;
#endif
		case ADC_CHARGING_CABLE:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_CHARGING_CABLE_MUIC;
			pr_info("%s : PS_CABLE DETECTED\n", MUIC_DEV_NAME);
			break;
		case ADC_DESKDOCK:
			intr = MUIC_INTR_ATTACH;
			new_dev = ATTACHED_DEV_DESKDOCK_MUIC;
			pr_info("%s : ADC DESKDOCK DETECTED\n", MUIC_DEV_NAME);
			break;
		case ADC_OPEN:
			/* sometimes muic fails to catch JIG_UART_OFF detaching */
			/* double check with ADC */
			if (new_dev == ATTACHED_DEV_JIG_UART_OFF_MUIC) {
				new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
				intr = MUIC_INTR_DETACH;
				pr_info("%s : ADC OPEN DETECTED\n", MUIC_DEV_NAME);
			}
			break;
		case ADC_UNIVERSAL_MMDOCK:
			pr_info("%s : ADC UNIVERSAL_MMDOCK Discarded\n", MUIC_DEV_NAME);
			break;

		case ADC_RESERVED_VZW:
			new_dev = ATTACHED_DEV_VZW_ACC_MUIC;
			intr = MUIC_INTR_ATTACH;
			pr_info("%s : ADC VZW_ACC DETECTED\n", MUIC_DEV_NAME);
			break;

		case ADC_INCOMPATIBLE_VZW:
			new_dev = ATTACHED_DEV_VZW_INCOMPATIBLE_MUIC;
			intr = MUIC_INTR_ATTACH;
			pr_info("%s : ADC INCOMPATIBLE_VZW DETECTED\n", MUIC_DEV_NAME);
			break;

		default:
			pr_warn("%s:%s unsupported ADC(0x%02x)\n", MUIC_DEV_NAME,
				__func__, adc);
			if(vbvolt) {
				intr = MUIC_INTR_ATTACH;
				new_dev = ATTACHED_DEV_UNDEFINED_CHARGING_MUIC;
				pr_info("%s : UNDEFINED VB DETECTED\n", MUIC_DEV_NAME);
			} else
				intr = MUIC_INTR_DETACH;
			break;
		}
	}

	/* Check it the cable type is supported.
	  */
	if (vps_is_supported_dev(new_dev))
		pr_info("%s:Supported.\n", __func__);
	else if(vbvolt && (intr == MUIC_INTR_ATTACH)) {
		new_dev = ATTACHED_DEV_UNDEFINED_CHARGING_MUIC;
		pr_info("%s:Unsupported->UNDEFINED_CHARGING\n", __func__);
	} else {
		intr = MUIC_INTR_DETACH;
		new_dev = ATTACHED_DEV_UNKNOWN_MUIC;
		pr_info("%s:Unsupported->Discarded.\n", __func__);
	}

	*pintr = intr;
	*pdev = new_dev;

	return 0;
}

int vps_resolve_dev(muic_data_t *pmuic, muic_attached_dev_t *pdev, int *pintr)
{
	if(pmuic->vps_table == VPS_TYPE_TABLE)
		return vps_find_attached_dev(pmuic,pdev,pintr);
	else
		return resolve_dedicated_dev(pmuic, pdev, pintr);
}
