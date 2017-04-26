/*
 * max77843-irq.c - Interrupt controller support for MAX77843
 *
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * SangYoung Son <hello.son@samsung.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This driver is based on max77843-irq.c
 */

#include <linux/err.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/mfd/max77843.h>
#include <linux/mfd/max77843-private.h>

static const u8 max77843_mask_reg[] = {
	[MAX77843_TOPSYS_INT] = MAX77843_PMIC_REG_SYS_INT_MASK,
	[MAX77843_CHG_INT] = MAX77843_CHG_REG_INT_MASK,
	[MAX77843_MUIC_INT1] = MAX77843_MUIC_REG_INTMASK1,
	[MAX77843_MUIC_INT2] = MAX77843_MUIC_REG_INTMASK2,
	[MAX77843_MUIC_INT3] = MAX77843_MUIC_REG_INTMASK3,
};

static struct i2c_client *get_i2c(struct max77843_dev *max77843,
				enum max77843_irq_source src)
{
	switch (src) {
	case MAX77843_TOPSYS_INT:
		return max77843->i2c;
	case MAX77843_CHG_INT:
		return max77843->charger;
	case MAX77843_FUEL_INT:
		return max77843->fuelgauge;
	case MAX77843_MUIC_INT1 ... MAX77843_MUIC_INT3:
		return max77843->muic;
	default:
		return ERR_PTR(-EINVAL);
	}
}

struct max77843_irq_data {
	int mask;
	enum max77843_irq_source group;
};

#define DECLARE_IRQ(idx, _group, _mask)		\
	[(idx)] = { .group = (_group), .mask = (_mask) }
static const struct max77843_irq_data max77843_irqs[] = {
	DECLARE_IRQ(MAX77843_CHG_IRQ_BYP_I,	MAX77843_CHG_INT, 1 << 0),
	DECLARE_IRQ(MAX77843_CHG_IRQ_BATP_I,	MAX77843_CHG_INT, 1 << 2),
	DECLARE_IRQ(MAX77843_CHG_IRQ_BAT_I,	MAX77843_CHG_INT, 1 << 3),
	DECLARE_IRQ(MAX77843_CHG_IRQ_CHG_I,	MAX77843_CHG_INT, 1 << 4),
	DECLARE_IRQ(MAX77843_CHG_IRQ_WCIN_I,	MAX77843_CHG_INT, 1 << 5),
	DECLARE_IRQ(MAX77843_CHG_IRQ_CHGIN_I,	MAX77843_CHG_INT, 1 << 6),
	DECLARE_IRQ(MAX77843_CHG_IRQ_AICL_I,	MAX77843_CHG_INT, 1 << 7),

	DECLARE_IRQ(MAX77843_FG_IRQ_ALERT, MAX77843_FUEL_INT, 1 << 1),

	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT1_ADC,		MAX77843_MUIC_INT1, 1 << 0),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT1_ADCERR,	MAX77843_MUIC_INT1, 1 << 2),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT1_ADC1K,	MAX77843_MUIC_INT1, 1 << 3),

	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT2_CHGTYP,	MAX77843_MUIC_INT2, 1 << 0),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT2_CHGDETREUN,	MAX77843_MUIC_INT2, 1 << 1),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT2_DCDTMR,	MAX77843_MUIC_INT2, 1 << 2),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT2_DXOVP,	MAX77843_MUIC_INT2, 1 << 3),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT2_VBVOLT,	MAX77843_MUIC_INT2, 1 << 4),

	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_VBADC,	MAX77843_MUIC_INT3, 1 << 0),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_VDNMON,	MAX77843_MUIC_INT3, 1 << 1),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_DNRES,	MAX77843_MUIC_INT3, 1 << 2),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_MPNACK,	MAX77843_MUIC_INT3, 1 << 3),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_MRXBUFOW,	MAX77843_MUIC_INT3, 1 << 4),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_MRXTRF,	MAX77843_MUIC_INT3, 1 << 5),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_MRXPERR,	MAX77843_MUIC_INT3, 1 << 6),
	DECLARE_IRQ(MAX77843_MUIC_IRQ_INT3_MRXRDY,	MAX77843_MUIC_INT3, 1 << 7),
};

static void max77843_irq_lock(struct irq_data *data)
{
	struct max77843_dev *max77843 = irq_get_chip_data(data->irq);

	mutex_lock(&max77843->irqlock);
}

static void max77843_irq_sync_unlock(struct irq_data *data)
{
	struct max77843_dev *max77843 = irq_get_chip_data(data->irq);
	int i;

	for (i = 0; i < MAX77843_IRQ_GROUP_NR; i++) {
		u8 mask_reg = max77843_mask_reg[i];
		struct i2c_client *i2c = get_i2c(max77843, i);

		if (mask_reg == MAX77843_REG_INVALID ||
				IS_ERR_OR_NULL(i2c))
			continue;
		max77843->irq_masks_cache[i] = max77843->irq_masks_cur[i];

		max77843_write_reg(i2c, max77843_mask_reg[i],
				max77843->irq_masks_cur[i]);
	}

	mutex_unlock(&max77843->irqlock);
}

static const inline struct max77843_irq_data *
irq_to_max77843_irq(struct max77843_dev *max77843, int irq)
{
	return &max77843_irqs[irq - max77843->irq_base];
}

static void max77843_irq_mask(struct irq_data *data)
{
	struct max77843_dev *max77843 = irq_get_chip_data(data->irq);
	const struct max77843_irq_data *irq_data =
	    irq_to_max77843_irq(max77843, data->irq);

	if (irq_data->group >= MAX77843_IRQ_GROUP_NR)
		return;

	if (irq_data->group >= MAX77843_MUIC_INT1 && irq_data->group <= MAX77843_MUIC_INT3)
		max77843->irq_masks_cur[irq_data->group] &= ~irq_data->mask;
	else
		max77843->irq_masks_cur[irq_data->group] |= irq_data->mask;
}

static void max77843_irq_unmask(struct irq_data *data)
{
	struct max77843_dev *max77843 = irq_get_chip_data(data->irq);
	const struct max77843_irq_data *irq_data =
	    irq_to_max77843_irq(max77843, data->irq);

	if (irq_data->group >= MAX77843_IRQ_GROUP_NR)
		return;

	if (irq_data->group >= MAX77843_MUIC_INT1 && irq_data->group <= MAX77843_MUIC_INT3)
		max77843->irq_masks_cur[irq_data->group] |= irq_data->mask;
	else
		max77843->irq_masks_cur[irq_data->group] &= ~irq_data->mask;
}

static void max77843_irq_ack(struct irq_data *data)
{
}
static struct irq_chip max77843_irq_chip = {
	.name			= "max77843",
	.irq_bus_lock		= max77843_irq_lock,
	.irq_bus_sync_unlock	= max77843_irq_sync_unlock,
	.irq_mask		= max77843_irq_mask,
	.irq_unmask		= max77843_irq_unmask,
	.irq_ack		= max77843_irq_ack,
	.irq_disable		= max77843_irq_mask,
};

static irqreturn_t max77843_irq_thread(int irq, void *data)
{
	struct max77843_dev *max77843 = data;
	u8 irq_reg[MAX77843_IRQ_GROUP_NR] = {0};
	u8 irq_src;
	int ret;
	int i;
	pr_debug("%s: irq gpio pre-state(0x%02x)\n", __func__,
		gpio_get_value(max77843->irq_gpio));

clear_retry:
	ret = max77843_read_reg(max77843->i2c,
		MAX77843_PMIC_REG_INTSRC, &irq_src);
	if (ret < 0) {
		dev_err(max77843->dev, "Failed to read interrupt source: %d\n",
				ret);
		return IRQ_NONE;
	}
	pr_info("%s: interrupt source(0x%02x)\n", __func__, irq_src);

	if (irq_src & MAX77843_IRQSRC_TOP) {
		/* TOPSYS_INT */
		ret = max77843_read_reg(max77843->i2c,
				MAX77843_PMIC_REG_TOPSYS_INT,
				&irq_reg[MAX77843_TOPSYS_INT]);
		pr_info("%s: topsys interrupt(0x%02x)\n",
			__func__, irq_reg[MAX77843_TOPSYS_INT]);
	}

	if (irq_src & MAX77843_IRQSRC_CHG) {
		/* CHG_INT */
		ret = max77843_read_reg(max77843->charger, MAX77843_CHG_REG_INT,
				&irq_reg[MAX77843_CHG_INT]);
		pr_info("%s: charger interrupt(0x%02x)\n",
			__func__, irq_reg[MAX77843_CHG_INT]);
		/* mask chgin to prevent chgin infinite interrupt
		 * chgin is unmasked chgin isr
		 */
		if (irq_reg[MAX77843_CHG_INT] & max77843_irqs[MAX77843_CHG_IRQ_CHGIN_I].mask) {
			max77843_update_reg(max77843->i2c,
				MAX77843_CHG_REG_INT_MASK, MAX77843_CHGIN_IM, MAX77843_CHGIN_IM);
		}
	}

	if (irq_src & MAX77843_IRQSRC_FG) {
		pr_info("%s: fuelgauge interrupt, IRQ_BASE(%d), NESTED_IRQ(%d)\n", __func__,
			max77843->irq_base, max77843->irq_base + MAX77843_FG_IRQ_ALERT);
		handle_nested_irq(max77843->irq_base + MAX77843_FG_IRQ_ALERT);
		return IRQ_HANDLED;
	}

	if (irq_src & MAX77843_IRQSRC_MUIC) {
		/* MUIC INT1 ~ INT3 */
		max77843_bulk_read(max77843->muic,
		MAX77843_MUIC_REG_INT1,
		MAX77843_NUM_IRQ_MUIC_REGS,
				&irq_reg[MAX77843_MUIC_INT1]);
		pr_info("%s: muic interrupt(0x%02x, 0x%02x, 0x%02x)\n",
			__func__, irq_reg[MAX77843_MUIC_INT1],
			irq_reg[MAX77843_MUIC_INT2], irq_reg[MAX77843_MUIC_INT3]);
	}

	pr_debug("%s: irq gpio post-state(0x%02x)\n", __func__,
		gpio_get_value(max77843->irq_gpio));

	if (gpio_get_value(max77843->irq_gpio) == 0) {
		pr_warn("%s: irq_gpio is not High!\n", __func__);
		goto clear_retry;
	}
#if 0
	/* Apply masking */
	for (i = 0; i < MAX77843_IRQ_GROUP_NR; i++) {
		if (i >= MAX77843_MUIC_INT1 && i <= MAX77843_MUIC_INT3)
			irq_reg[i] &= max77843->irq_masks_cur[i];
		else
			irq_reg[i] &= ~max77843->irq_masks_cur[i];
	}
#endif
	/* Report */
	for (i = 0; i < MAX77843_IRQ_NR; i++) {
		if (irq_reg[max77843_irqs[i].group] & max77843_irqs[i].mask)
			handle_nested_irq(max77843->irq_base + i);
	}

	return IRQ_HANDLED;
}

int max77843_irq_resume(struct max77843_dev *max77843)
{
	int ret = 0;
	if (max77843->irq && max77843->irq_base)
		ret = max77843_irq_thread(max77843->irq_base, max77843);

	dev_info(max77843->dev, "%s: irq_resume ret=%d", __func__, ret);

	return ret >= 0 ? 0 : ret;
}

int max77843_irq_init(struct max77843_dev *max77843)
{
	int i;
	int cur_irq;
	int ret;
	u8 i2c_data;

	pr_info("func: %s, irq_gpio: %d, irq_base: %d\n", __func__,
			max77843->irq_gpio, max77843->irq_base);
	if (!max77843->irq_gpio) {
		dev_warn(max77843->dev, "No interrupt specified.\n");
		max77843->irq_base = 0;
		return 0;
	}

	if (!max77843->irq_base) {
		dev_err(max77843->dev, "No interrupt base specified.\n");
		return 0;
	}

	mutex_init(&max77843->irqlock);

	max77843->irq = gpio_to_irq(max77843->irq_gpio);
	ret = gpio_request(max77843->irq_gpio, "if_pmic_irq");
	if (ret) {
		dev_err(max77843->dev, "%s: failed requesting gpio %d\n",
			__func__, max77843->irq_gpio);
		return ret;
	}
	gpio_direction_input(max77843->irq_gpio);
	gpio_free(max77843->irq_gpio);

	/* Mask individual interrupt sources */
	for (i = 0; i < MAX77843_IRQ_GROUP_NR; i++) {
		struct i2c_client *i2c;
		/* MUIC IRQ  0:MASK 1:NOT MASK */
		/* Other IRQ 1:MASK 0:NOT MASK */
		if (i >= MAX77843_MUIC_INT1 && i <= MAX77843_MUIC_INT3) {
			max77843->irq_masks_cur[i] = 0x00;
			max77843->irq_masks_cache[i] = 0x00;
		} else {
			max77843->irq_masks_cur[i] = 0xff;
			max77843->irq_masks_cache[i] = 0xff;
		}
		i2c = get_i2c(max77843, i);

		if (IS_ERR_OR_NULL(i2c))
			continue;
		if (max77843_mask_reg[i] == MAX77843_REG_INVALID)
			continue;
		if (i >= MAX77843_MUIC_INT1 && i <= MAX77843_MUIC_INT3)
			max77843_write_reg(i2c, max77843_mask_reg[i], 0x00);
		else
			max77843_write_reg(i2c, max77843_mask_reg[i], 0xff);
	}

	/* Register with genirq */
	for (i = 0; i < MAX77843_IRQ_NR; i++) {
		cur_irq = i + max77843->irq_base;
		irq_set_chip_data(cur_irq, max77843);
		irq_set_chip_and_handler(cur_irq, &max77843_irq_chip,
					 handle_edge_irq);
		irq_set_nested_thread(cur_irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(cur_irq, IRQF_VALID);
#else
		irq_set_noprobe(cur_irq);
#endif
	}

	/* Unmask max77843 interrupt */
	ret = max77843_read_reg(max77843->i2c, MAX77843_PMIC_REG_INTSRC_MASK,
			  &i2c_data);
	if (ret) {
		dev_err(max77843->dev, "%s: fail to read muic reg\n", __func__);
		return ret;
	}

	i2c_data &= ~(MAX77843_IRQSRC_CHG);	/* Unmask charger interrupt */
	i2c_data &= ~(MAX77843_IRQSRC_FG);	/* Unmask fuelgauge interrupt */
	i2c_data &= ~(MAX77843_IRQSRC_MUIC);	/* Unmask muic interrupt */
	max77843_write_reg(max77843->i2c, MAX77843_PMIC_REG_INTSRC_MASK,
			   i2c_data);

	ret = request_threaded_irq(max77843->irq, NULL, max77843_irq_thread,
				   IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
				   "max77843-irq", max77843);

	if (ret) {
		dev_err(max77843->dev, "Failed to request IRQ %d: %d\n",
			max77843->irq, ret);
		return ret;
	}

	return 0;
}

void max77843_irq_exit(struct max77843_dev *max77843)
{
	if (max77843->irq)
		free_irq(max77843->irq, max77843);
}
