/*
 * sm5705-muic-afc.c - SM5705 AFC micro USB switch device driver
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
#include <linux/muic/sm5705-muic.h>

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


int set_afc_ctrl_reg(struct sm5705_muic_data *muic_data, int shift, bool on)
{
	struct i2c_client *i2c = muic_data->i2c;
	u8 reg_val;
	int ret = 0;

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_CTRL);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s(%d)\n", __func__, ret);
	if (on)
		reg_val = ret | (0x1 << shift);
	else
		reg_val = ret & ~(0x1 << shift);

	if (reg_val ^ ret) {
		printk(KERN_DEBUG "[muic] %s reg_val(0x%x)!=AFC_CTRL reg(0x%x), update reg\n",
			__func__, reg_val, ret);

		ret = sm5705_i2c_write_byte(i2c, SM5705_MUIC_REG_AFC_CTRL,
				reg_val);
		if (ret < 0)
			printk(KERN_ERR "[muic] %s err write AFC_CTRL(%d)\n",
					__func__, ret);
	} else {
		printk(KERN_DEBUG "[muic] %s (0x%x), just return\n",
				__func__, ret);
		return 0;
	}

	ret = sm5705_i2c_read_byte(i2c, SM5705_MUIC_REG_AFC_CTRL);
	if (ret < 0)
		printk(KERN_ERR "[muic] %s err read AFC_CTRL(%d)\n",
			__func__, ret);
	else
		printk(KERN_DEBUG "[muic] %s AFC_CTRL reg after change(0x%x)\n",
			__func__, ret);

	return ret;
}

int set_afc_ctrl_enafc(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = AFC_ENAFC_SHIFT;
	int ret = 0;

	ret = set_afc_ctrl_reg(muic_data, shift, on);

	return ret;
}

int set_afc_vbus_read(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = AFC_VBUS_READ_SHIFT;
	int ret = 0;

	ret = set_afc_ctrl_reg(muic_data, shift, on);

	return ret;
}

int set_afc_dm_reset(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = AFC_DM_RESET_SHIFT;
	int ret = 0;

	ret = set_afc_ctrl_reg(muic_data, shift, on);

	return ret;
}

int set_afc_dp_reset(struct sm5705_muic_data *muic_data, bool on)
{
	int shift = AFC_DP_RESET_SHIFT;
	int ret = 0;

	ret = set_afc_ctrl_reg(muic_data, shift, on);

	return ret;
}


