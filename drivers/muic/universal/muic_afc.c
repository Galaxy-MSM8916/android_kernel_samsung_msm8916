/*
 * muic_afc.c
 *
 * Copyright (C) 2014 Samsung Electronics
 * Jeongrae Kim <jryu.kim@samsung.com>
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
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/module.h>

#include <linux/muic/muic.h>
#include <linux/muic/muic_afc.h>
#include "muic-internal.h"
#include "muic_regmap.h"
#include "muic_i2c.h"
#if defined(CONFIG_MUIC_NOTIFIER)
#include <linux/muic/muic_notifier.h>
#endif /* CONFIG_MUIC_NOTIFIER */

#define	REG_AFCTXD	0x19
#define	REG_VBUSSTAT	0x1b

muic_data_t *gpmuic;
struct delayed_work afc_restart_work;
static int afc_work_state;

static int muic_is_afc_voltage(void);
static int muic_dpreset_afc(void);
static int muic_restart_afc(void);

int muic_check_afc_state(int state)
{
	struct afc_ops *afcops = gpmuic->regmapdesc->afcops;
	int ret, retry;

	pr_info("%s state = %d\n", __func__, state);

	if (state && gpmuic->is_afc_device) {
		/*	Flash on state	*/
		if (muic_is_afc_voltage()) {
			ret = muic_dpreset_afc();
			if (ret < 0) {
				pr_err("%s:failed to AFC reset(%d)\n",
						__func__, ret);
			}
			msleep(60); // 60ms delay

			afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_VBUS_READ, 1);
			afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_VBUS_READ, 0);
			for (retry = 0; retry <10; retry++) {
				mdelay(20);
				ret = muic_is_afc_voltage();
				if (!ret) {
					pr_info("%s:AFC Reset Success(%d)\n",
							__func__, ret);
					gpmuic->is_flash_on = 1;
					return 1;
				} else {
					pr_info("%s:AFC Reset Failed(%d)\n",
							__func__, ret);
					gpmuic->is_flash_on = -1;
				}
			}
		} else {
			pr_info("%s:Not connected AFC\n",__func__);
			gpmuic->is_flash_on = 1;
			return 1;
		}
	} else {
		/*	Flash off state	*/
		if (gpmuic->is_afc_device)
			if (!muic_is_afc_voltage())
				muic_restart_afc();
		gpmuic->is_flash_on = 0;
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(muic_check_afc_state);

int muic_torch_prepare(int state)
{
	struct afc_ops *afcops = gpmuic->regmapdesc->afcops;
	int ret, retry;

	pr_info("%s state = %d\n", __func__, state);

	if (afc_work_state == 1) {
		pr_info("%s:%s cancel_delayed_work  afc_work_state=%d\n",MUIC_DEV_NAME, __func__, afc_work_state);
		cancel_delayed_work(&afc_restart_work);
		afc_work_state = 0;
	}

	if (state && gpmuic->is_afc_device) {
		/*	Torch on state	*/
		if (muic_is_afc_voltage()) {
			ret = muic_dpreset_afc();
			msleep(60); // 60ms delay
			if (ret < 0) {
				pr_err("%s:failed to AFC reset(%d)\n",
						__func__, ret);
			}
			afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_VBUS_READ, 1);
			afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_VBUS_READ, 0);
			for (retry = 0; retry <10; retry++) {
				mdelay(20);
				ret = muic_is_afc_voltage();
				if (!ret) {
					pr_info("%s:AFC Reset Success(%d)\n",
							__func__, ret);
					gpmuic->is_flash_on = 1;
					return 1;
				} else {
					pr_info("%s:AFC Reset Failed(%d)\n",
							__func__, ret);
					gpmuic->is_flash_on = -1;
				}
			}
		} else {
			pr_info("%s:Not connected AFC\n",__func__);
			gpmuic->is_flash_on = 1;
			return 1;
		}
	} else {
		/*	Torch off state	*/
		gpmuic->is_flash_on = 0;
		if ((gpmuic->is_afc_device) && (gpmuic->attached_dev != ATTACHED_DEV_AFC_CHARGER_9V_MUIC)) {
			schedule_delayed_work(&afc_restart_work, msecs_to_jiffies(5000)); // 20sec
			pr_info("%s:%s AFC_torch_work start \n",MUIC_DEV_NAME, __func__ );
			afc_work_state = 1;
		}
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(muic_torch_prepare);

static int muic_is_afc_voltage(void)
{
	struct i2c_client *i2c = gpmuic->i2c;
	int vbus_status;

	vbus_status = muic_i2c_read_byte(i2c, REG_VBUSSTAT);
	vbus_status = (vbus_status & 0x0F);
	pr_info("%s vbus_status (%d)\n", __func__, vbus_status);
	if (vbus_status == 0x00)
		return 0;
	else
		return 1;
}

static int muic_dpreset_afc(void)
{
	struct afc_ops *afcops = gpmuic->regmapdesc->afcops;

	pr_info("%s: gpmuic->attached_dev = %d\n", __func__, gpmuic->attached_dev);
	if ((gpmuic->attached_dev == ATTACHED_DEV_AFC_CHARGER_9V_MUIC) ||
			(gpmuic->attached_dev == ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC)) {
		pr_info("%s:DP_RESET \n", __func__);

		afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_DP_RESET, 1);

		gpmuic->attached_dev = ATTACHED_DEV_AFC_CHARGER_5V_MUIC;
		muic_notifier_attach_attached_dev(ATTACHED_DEV_AFC_CHARGER_5V_MUIC);
	}

	return 0;
}

static int muic_restart_afc(void)
{
	struct i2c_client *i2c = gpmuic->i2c;
	int ret, value;
	struct afc_ops *afcops = gpmuic->regmapdesc->afcops;

	pr_info("%s:AFC Restart attached_dev = 0x%x\n", __func__, gpmuic->attached_dev);

	if (!gpmuic->is_afc_device) {
		pr_info("%s: gpmuic->attached_dev[0x%02x] return \n", __func__, gpmuic->attached_dev);
		return 0;
	}

	msleep(120); // 120ms delay
	gpmuic->attached_dev = ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC;
	muic_notifier_attach_attached_dev(ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC);

	// voltage(9.0V) + current(1.65A) setting : 0x
	value = 0x46;
	ret = muic_i2c_write_byte(i2c, REG_AFCTXD, value);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err write AFC_TXD(%d)\n", __func__, ret);
	pr_info("%s:AFC_TXD [0x%02x]\n", __func__, value);

	// ENAFC set '1'
	afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_ENAFC, 1);

	return 0;
}

static void muic_afc_restart_work(struct work_struct *work)
{
	struct i2c_client *i2c = gpmuic->i2c;
	int ret, value;
	struct afc_ops *afcops = gpmuic->regmapdesc->afcops;

	pr_info("%s:AFC Restart\n", __func__);
	msleep(120); // 120ms delay
	gpmuic->attached_dev = ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC;
	muic_notifier_attach_attached_dev(ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC);

	// voltage(9.0V) + current(1.65A) setting : 0x
	value = 0x46;
	ret = muic_i2c_write_byte(i2c, REG_AFCTXD, value);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s: err write AFC_TXD(%d)\n", __func__, ret);
	pr_info("%s:AFC_TXD [0x%02x]\n", __func__, value);

	// ENAFC set '1'
	afcops->afc_ctrl_reg(gpmuic->regmapdesc, AFCCTRL_ENAFC, 1);
	afc_work_state = 0;

}

void muic_init_afc_state(muic_data_t *pmuic)
{
	gpmuic = pmuic;
	gpmuic->is_flash_on = 0;
	gpmuic->is_afc_device = 0;
	INIT_DELAYED_WORK(&afc_restart_work, muic_afc_restart_work);

	pr_info("%s:attached_dev = %d\n", __func__, gpmuic->attached_dev);
}

MODULE_DESCRIPTION("MUIC driver");
MODULE_AUTHOR("<jryu.kim@samsung.com>");
MODULE_LICENSE("GPL");
