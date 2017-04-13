/* drivers/mfd/rt5033_irq.c
 * RT5033 Multifunction Device Driver
 * Charger / Buck / LDOs / FlashLED
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/mfd/rt5033.h>
#include <linux/mfd/rt5033_irq.h>
#include <linux/gpio.h>
#include <linux/device.h>

#define ALIAS_NAME "rt5033_mfd_irq"

#define RT5033_CHG_IRQ          0x60
#define RT5033_LED_IRQ          0x66
#define RT5033_PMIC_IRQ         0x68
#define RT5033_OFF_EVENT        0x6B

#define RT5033_CHG_IRQ_CTRL     0x63
#define RT5033_CHG_IRQ1_CTRL    RT5033_CHG_IRQ_CTRL
#define RT5033_CHG_IRQ2_CTRL    (RT5033_CHG_IRQ_CTRL + 1)
#define RT5033_CHG_IRQ3_CTRL    (RT5033_CHG_IRQ_CTRL + 2)
#define RT5033_LED_IRQ_CTRL     0x67
#define RT5033_PMIC_IRQ_CTRL    0x69

#define IRQ_NAME(irq) [irq] = irq##_NAME
static const char *rt5033_irq_names[] = {
	IRQ_NAME(RT5033_ADPBAD_IRQ),
	IRQ_NAME(RT5033_PPBATLV_IRQ),
	IRQ_NAME(RT5033_CHTMRFI_IRQ),
	IRQ_NAME(RT5033_VINOVPI_IRQ),
	IRQ_NAME(RT5033_TSDI_IRQ),
	IRQ_NAME(RT5033_CHMIVRI_IRQ),
	IRQ_NAME(RT5033_CHTREGI_IRQ),
	IRQ_NAME(RT5033_IEOC_IRQ),
	IRQ_NAME(RT5033_CHRCHGI_IRQ),
	IRQ_NAME(RT5033_CHTERMI_IRQ),
	IRQ_NAME(RT5033_CHBATOVI_IRQ),
	IRQ_NAME(RT5033_CHRVPI_IRQ),
	IRQ_NAME(RT5033_BSTLOWVI_IRQ),
	IRQ_NAME(RT5033_BSTOLI_IRQ),
	IRQ_NAME(RT5033_BSTVMIDOVP_IRQ),
	IRQ_NAME(RT5033_OVPR_IRQ),
	IRQ_NAME(RT5033_VF_L_IRQ),
	IRQ_NAME(RT5033_LEDCS2_SHORT_IRQ),
	IRQ_NAME(RT5033_LEDCS1_SHORT_IRQ),
	IRQ_NAME(RT5033_BUCK_OCP_IRQ),
	IRQ_NAME(RT5033_BUCK_LV_IRQ),
	IRQ_NAME(RT5033_SAFE_LDO_LV_IRQ),
	IRQ_NAME(RT5033_LDO_LV_IRQ),
	IRQ_NAME(RT5033_OT_IRQ),
	IRQ_NAME(RT5033_VDDA_UV_IRQ),
};

const char *rt5033_get_irq_name_by_index(int index)
{
	return rt5033_irq_names[index];
}
EXPORT_SYMBOL(rt5033_get_irq_name_by_index);

enum RT5033_IRQ_OFFSET {
	RT5033_CHG_IRQ_OFFSET = 0,
	RT5033_CHG_IRQ1_OFFSET = 0,
	RT5033_CHG_IRQ2_OFFSET,
	RT5033_CHG_IRQ3_OFFSET,
	RT5033_LED_IRQ_OFFSET,
	RT5033_PMIC_IRQ_OFFSET,
};

struct rt5033_irq_data {
	int mask;
	int offset;
};

static const u8 rt5033_mask_reg[] = {
	RT5033_CHG_IRQ1_CTRL,
	RT5033_CHG_IRQ2_CTRL,
	RT5033_CHG_IRQ3_CTRL,
	RT5033_LED_IRQ_CTRL,
	RT5033_PMIC_IRQ_CTRL,
};

#define DECLARE_IRQ_CTRL(idx, _offset, _mask)		\
	[(idx)] = { .offset = (_offset), .mask = (_mask) }

static const struct rt5033_irq_data rt5033_irqs[] = {
	DECLARE_IRQ_CTRL(RT5033_ADPBAD_IRQ, 0, 1 << 0),
	DECLARE_IRQ_CTRL(RT5033_PPBATLV_IRQ, 0, 1 << 4),
	DECLARE_IRQ_CTRL(RT5033_CHTERMI_IRQ, 0, 1 << 5),
	DECLARE_IRQ_CTRL(RT5033_VINOVPI_IRQ, 0, 1 << 6),
	DECLARE_IRQ_CTRL(RT5033_TSDI_IRQ, 0, 1 << 7),
	DECLARE_IRQ_CTRL(RT5033_CHMIVRI_IRQ, 1, 1 << 0),
	DECLARE_IRQ_CTRL(RT5033_CHTREGI_IRQ, 1, 1 << 1),
	DECLARE_IRQ_CTRL(RT5033_CHTMRFI_IRQ, 1, 1 << 2),
	DECLARE_IRQ_CTRL(RT5033_CHRCHGI_IRQ, 1, 1 << 3),
	DECLARE_IRQ_CTRL(RT5033_IEOC_IRQ, 1, 1 << 4),
	DECLARE_IRQ_CTRL(RT5033_CHBATOVI_IRQ, 1, 1 << 5),
	DECLARE_IRQ_CTRL(RT5033_CHRVPI_IRQ, 1, 1 << 7),

	DECLARE_IRQ_CTRL(RT5033_BSTLOWVI_IRQ, 2, 1 << 5),
	DECLARE_IRQ_CTRL(RT5033_BSTOLI_IRQ, 2, 1 << 6),
	DECLARE_IRQ_CTRL(RT5033_BSTVMIDOVP_IRQ, 2, 1 << 7),

	DECLARE_IRQ_CTRL(RT5033_OVPR_IRQ, 3, 1 << 2),
	DECLARE_IRQ_CTRL(RT5033_VF_L_IRQ, 3, 1 << 3),
	DECLARE_IRQ_CTRL(RT5033_LEDCS2_SHORT_IRQ, 3, 1 << 6),
	DECLARE_IRQ_CTRL(RT5033_LEDCS1_SHORT_IRQ, 3, 1 << 7),

	DECLARE_IRQ_CTRL(RT5033_BUCK_OCP_IRQ, 4, 1 << 2),
	DECLARE_IRQ_CTRL(RT5033_BUCK_LV_IRQ, 4, 1 << 3),
	DECLARE_IRQ_CTRL(RT5033_SAFE_LDO_LV_IRQ, 4, 1 << 4),
	DECLARE_IRQ_CTRL(RT5033_LDO_LV_IRQ, 4, 1 << 5),
	DECLARE_IRQ_CTRL(RT5033_OT_IRQ, 4, 1 << 6),
	DECLARE_IRQ_CTRL(RT5033_VDDA_UV_IRQ, 4, 1 << 7),
};


static void rt5033_irq_lock(struct irq_data *data)
{
	rt5033_mfd_chip_t *chip = irq_get_chip_data(data->irq);
	mutex_lock(&chip->irq_lock);
}

static void rt5033_irq_sync_unlock(struct irq_data *data)
{
	rt5033_mfd_chip_t *chip = irq_get_chip_data(data->irq);
	rt5033_block_write_device(chip->i2c_client,
			RT5033_CHG_IRQ_CTRL,
			RT5033_CHG_IRQ_REGS_NR,
			chip->irq_masks_cache +
			RT5033_CHG_IRQ_OFFSET);

	rt5033_block_write_device(chip->i2c_client,
			RT5033_LED_IRQ_CTRL,
			RT5033_LED_IRQ_REGS_NR,
			chip->irq_masks_cache +
			RT5033_LED_IRQ_OFFSET);

	rt5033_block_write_device(chip->i2c_client,
			RT5033_PMIC_IRQ_CTRL,
			RT5033_PMIC_IRQ_REGS_NR,
			chip->irq_masks_cache +
			RT5033_PMIC_IRQ_OFFSET);

	mutex_unlock(&chip->irq_lock);
}

static const inline struct rt5033_irq_data *
	irq_to_rt5033_irq(rt5033_mfd_chip_t *chip, int irq)
{
	return &rt5033_irqs[irq - chip->irq_base];
}

static void rt5033_irq_mask(struct irq_data *data)
{
	rt5033_mfd_chip_t *chip = irq_get_chip_data(data->irq);
	const struct rt5033_irq_data *irq_data = irq_to_rt5033_irq(chip,
			data->irq);

	chip->irq_masks_cache[irq_data->offset] |= irq_data->mask;
}


static void rt5033_irq_unmask(struct irq_data *data)
{
	rt5033_mfd_chip_t *chip = irq_get_chip_data(data->irq);
	const struct rt5033_irq_data *irq_data = irq_to_rt5033_irq(chip,
			data->irq);

	chip->irq_masks_cache[irq_data->offset] &= ~irq_data->mask;
}

static struct irq_chip rt5033_irq_chip = {
	.name			= "rt5033",
	.irq_bus_lock		= rt5033_irq_lock,
	.irq_bus_sync_unlock	= rt5033_irq_sync_unlock,
	.irq_mask		= rt5033_irq_mask,
	.irq_unmask		= rt5033_irq_unmask,
};

rt5033_irq_status_t *rt5033_get_irq_status(rt5033_mfd_chip_t *mfd_chip,
		rt5033_irq_status_sel_t sel)
{

	int index;

	switch(sel) {
	case RT5033_PREV_STATUS:
		index = mfd_chip->irq_status_index^0x01;
		break;
	case RT5033_NOW_STATUS:
	default:
		index = mfd_chip->irq_status_index;
	}

	return &mfd_chip->irq_status[index];
}
EXPORT_SYMBOL(rt5033_get_irq_status);

static int rt5033_read_irq_status(rt5033_mfd_chip_t *chip)
{
	int ret;
	struct i2c_client *iic = chip->i2c_client;
	rt5033_irq_status_t *now_irq_status;

	now_irq_status = rt5033_get_irq_status(chip, RT5033_NOW_STATUS);

	ret = rt5033_block_read_device(iic, RT5033_CHG_IRQ,
				       sizeof(now_irq_status->chg_irq_status),
				       now_irq_status->chg_irq_status);
	if (ret < 0) {
		dev_err(chip->dev,
			"Failed on reading CHG irq status\n");
		return ret;
	}

	printk("charger irq = 0x%x 0x%x 0x%x\n", (int)now_irq_status->chg_irq_status[0],
	       (int)now_irq_status->chg_irq_status[1],
	       now_irq_status->chg_irq_status[2]);
	ret = rt5033_block_read_device(iic, RT5033_LED_IRQ,
				       sizeof(now_irq_status->fled_irq_status),
				       now_irq_status->fled_irq_status);
	if (ret < 0) {
		dev_err(chip->dev,
			"Failed on reading FlashLED irq status\n");
		return ret;
	}
	printk("fled irq = 0x%x\n", (int)now_irq_status->fled_irq_status[0]);

	ret = rt5033_block_read_device(iic, RT5033_PMIC_IRQ,
				       sizeof(now_irq_status->pmic_irq_status),
				       now_irq_status->pmic_irq_status);
	if (ret < 0) {
		dev_err(chip->dev,
			"Failed on reading PMIC irq status\n");
		return ret;
	}
	printk("regulator irq = 0x%x\n", (int)now_irq_status->pmic_irq_status[0]);

	return 0;
}

static void rt5033_irq_work(rt5033_mfd_chip_t *chip)
{
	int ret;
	int i;
	rt5033_irq_status_t *status;
	pr_info("%s : handle IRQ event\n", __func__);
	ret = rt5033_read_irq_status(chip);
	if (ret < 0) {
		pr_err("%s :Error : can't read irq status (%d)\n",
		       __func__, ret);
		return;
	}
	status = rt5033_get_irq_status(chip, RT5033_NOW_STATUS);

	for (i = 0; i < RT5033_IRQ_REGS_NR; i++)
		status->regs[i] &= ~chip->irq_masks_cache[i];

	for (i = 0; i < RT5033_IRQS_NR; i++) {
		if (status->regs[rt5033_irqs[i].offset] & rt5033_irqs[i].mask) {
			pr_info("%s : Trigger IRQ %s, irq : %d \n",
				__func__, rt5033_get_irq_name_by_index(i),
				chip->irq_base + i);

			handle_nested_irq(chip->irq_base + i);
		}
	}

	chip->irq_status_index ^= 0x01; // exchange irq index;
}
static irqreturn_t rt5033_irq_handler(int irq, void *data)
{
	rt5033_mfd_chip_t *chip = data;


	printk("RT5033 IRQ triggered\n");
	wake_lock_timeout(&chip->irq_wake_lock, msecs_to_jiffies(500));
	mutex_lock(&chip->suspend_flag_lock);
	if (chip->suspend_flag) {
		printk("I2C host controller might not be ready,"
			" pend this IRQ event...\n");
		chip->pending_irq = true;
	}
	else
		rt5033_irq_work(chip);
	mutex_unlock(&chip->suspend_flag_lock);;
	return IRQ_HANDLED;
}

static int rt5033_irq_ctrl_regs[] = {
	RT5033_CHG_IRQ1_CTRL,
	RT5033_CHG_IRQ2_CTRL,
	RT5033_CHG_IRQ3_CTRL,
	RT5033_LED_IRQ_CTRL,
	RT5033_PMIC_IRQ_CTRL,
};

static uint8_t rt5033_irqs_ctrl_mask_all_val[] = {
	0xf1,
	0xbf,
	0xf0,
	0xc8,
	0xfc,
};

static int rt5033_mask_all_irqs(struct i2c_client *iic)
{
	int rc;
	int i;

	rt5033_mfd_chip_t *chip = i2c_get_clientdata(iic);

	for (i=0;i<ARRAY_SIZE(rt5033_irq_ctrl_regs);i++) {
		rc = rt5033_reg_write(iic, rt5033_irq_ctrl_regs[i],
				rt5033_irqs_ctrl_mask_all_val[i]);
		chip->irq_masks_cache[i] = rt5033_irqs_ctrl_mask_all_val[i];
		if (rc<0) {
			pr_info("Error : can't write reg[0x%x] = 0x%x\n",
					rt5033_irq_ctrl_regs[i],
					rt5033_irqs_ctrl_mask_all_val[i]);
			return rc;
		}
	}

	return 0;
}

static int rt5033_irq_init_read(rt5033_mfd_chip_t *chip)
{
	int ret;
	ret = rt5033_read_irq_status(chip);
	chip->irq_status_index ^= 0x01;

	return ret;
}

int rt5033_init_irq(rt5033_mfd_chip_t *chip)
{
	int i, ret, curr_irq;
	ret = rt5033_mask_all_irqs(chip->i2c_client);

	if (ret < 0) {
		pr_err("%s : Can't mask all irqs(%d)\n", __func__, ret);
		goto err_mask_all_irqs;
	}

	rt5033_irq_init_read(chip);
	mutex_init(&chip->irq_lock);

	/* Register with genirq */
	for (i = 0; i < RT5033_IRQS_NR; i++) {
		curr_irq = i + chip->irq_base;
		irq_set_chip_data(curr_irq, chip);
		irq_set_chip_and_handler(curr_irq, &rt5033_irq_chip,
				handle_simple_irq);
		irq_set_nested_thread(curr_irq, 1);
#ifdef CONFIG_ARM
		set_irq_flags(curr_irq, IRQF_VALID);
#else
		irq_set_noprobe(curr_irq);
#endif
	}

	ret = gpio_request(chip->pdata->irq_gpio, "rt5033_mfd_irq");
	if (ret < 0) {
		pr_err("%s : Request GPIO %d failed\n",
			__func__, (int)chip->pdata->irq_gpio);
		goto err_gpio_request;
	}

	ret = gpio_direction_input(chip->pdata->irq_gpio);
	if (ret < 0) {
		pr_err("Set GPIO direction to input : failed\n");
		goto err_set_gpio_input;
	}

	chip->irq = gpio_to_irq(chip->pdata->irq_gpio);
	ret = request_threaded_irq(chip->irq, NULL, rt5033_irq_handler,
			/* IRQF_TRIGGER_FALLING */
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT | IRQF_NO_SUSPEND,
			"rt5033", chip);
	if (ret <0) {
		pr_err("%s : Failed : request IRQ (%d)\n", __func__, ret);
		goto err_request_irq;

	}

	ret = enable_irq_wake(chip->irq);
	if (ret < 0) {
		pr_info("%s : enable_irq_wake(%d) failed for (%d)\n",
				__func__, chip->irq, ret);
	}


	return ret;
err_request_irq:
err_set_gpio_input:
	gpio_free(chip->pdata->irq_gpio);
err_gpio_request:
	for (curr_irq = chip->irq_base;
			curr_irq < chip->irq_base + RT5033_IRQS_NR;
			curr_irq++) {
#ifdef CONFIG_ARM
		set_irq_flags(curr_irq, 0);
#endif
		irq_set_chip_and_handler(curr_irq, NULL, NULL);
		irq_set_chip_data(curr_irq, NULL);
	}

	mutex_destroy(&chip->irq_lock);
err_mask_all_irqs:
	return ret;

}

int rt5033_exit_irq(rt5033_mfd_chip_t *chip)
{
	int curr_irq;

	for (curr_irq = chip->irq_base;
			curr_irq < chip->irq_base + RT5033_IRQS_NR;
			curr_irq++) {
#ifdef CONFIG_ARM
		set_irq_flags(curr_irq, 0);
#endif
		irq_set_chip_and_handler(curr_irq, NULL, NULL);
		irq_set_chip_data(curr_irq, NULL);
	}

	if (chip->irq)
		free_irq(chip->irq, chip);
	mutex_destroy(&chip->irq_lock);
	return 0;
}
#ifdef CONFIG_PM
int rt5033_irq_suspend(rt5033_mfd_chip_t *chip)
{
	mutex_lock(&chip->suspend_flag_lock);
	chip->suspend_flag = true;
	mutex_unlock(&chip->suspend_flag_lock);
	return 0;
}

int rt5033_irq_resume(rt5033_mfd_chip_t *chip)
{
	mutex_lock(&chip->suspend_flag_lock);
	if (chip->pending_irq) {
		pr_info("%s : there is pending IRQ event\n",
					__func__);
		rt5033_irq_work(chip);
		chip->pending_irq = false;
	}
	chip->suspend_flag = false;
	mutex_unlock(&chip->suspend_flag_lock);
	return 0;
}
#endif /* CONFIG_PM */
