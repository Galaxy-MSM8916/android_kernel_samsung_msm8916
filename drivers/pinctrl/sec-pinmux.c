/* Copyright (c) 2010,2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/pinctrl/sec-pinmux.h>
#ifdef CONFIG_SEC_PM_DEBUG
#include <linux/gpio.h>
#include <linux/debugfs.h>
#endif
#ifdef CONFIG_SEC_GPIO_DVS
#include <linux/errno.h>
#include <linux/secgpio_dvs.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinconf-generic.h>
#endif

#ifdef CONFIG_SEC_PM_DEBUG
static DEFINE_SPINLOCK(gpiomux_lock);
#endif
static unsigned msm_gpiomux_ngpio;
#ifdef CONFIG_SEC_GPIO_DVS
/****************************************************************/
/* Define value in accordance with
	the specification of each BB vendor. */
#define AP_GPIO_COUNT   122
/****************************************************************/

/* config translations */
#define dir_to_inout_val(dir)   (dir << 1)
#define GPIO_NO_PULL          0
#define GPIO_PULL_DOWN        1
#define GPIO_PULL_UP          3

enum {
	GPIO_IN_BIT  = 0,
	GPIO_OUT_BIT = 1
};

#define GET_RESULT_GPIO(a, b, c)	\
	((a<<4 & 0xF0) | (b<<1 & 0xE) | (c & 0x1))

#define GET_GPIO_IO(value)	\
	(unsigned char)((0xF0 & (value)) >> 4)
#define GET_GPIO_PUPD(value)	\
	(unsigned char)((0xE & (value)) >> 1)
#define GET_GPIO_LH(value)	\
	(unsigned char)(0x1 & (value))

/****************************************************************/
/* Pre-defined variables. (DO NOT CHANGE THIS!!) */
static unsigned char checkgpiomap_result[GDVS_PHONE_STATUS_MAX][AP_GPIO_COUNT];
static struct gpiomap_result gpiomap_result = {
	.init = checkgpiomap_result[PHONE_INIT],
	.sleep = checkgpiomap_result[PHONE_SLEEP]
};
/****************************************************************/
#ifdef SECGPIO_SLEEP_DEBUGGING
static struct sleepdebug_gpiotable sleepdebug_table;
#endif

static unsigned __msm_gpio_get_inout_lh(unsigned gpio)
{
	return msm_tlmm_v4_get_gp_input_value(gpio);
}

static void msm8916_check_gpio_status(unsigned char phonestate)
{
	struct gpiomux_setting val;

	u32 i;

	u8 temp_io = 0, temp_pdpu = 0, temp_lh = 0;

	pr_info("[secgpio_dvs][%s] state : %s\n", __func__,
		(phonestate == PHONE_INIT) ? "init" : "sleep");

	for (i = 0; i < AP_GPIO_COUNT; i++) {
#ifdef ENABLE_SENSORS_FPRINT_SECURE
		if (i >= CONFIG_SENSORS_FP_SPI_GPIO_START && i <= CONFIG_SENSORS_FP_SPI_GPIO_END)
			continue;
#endif
#ifdef CONFIG_MST_LDO
		if (i == MST_GPIO_D_MINUS || i == MST_GPIO_D_PLUS)
			continue;
#endif
#if (defined (CONFIG_SEC_A8_PROJECT) && defined (CONFIG_ESE_P61)) \
	|| (defined (CONFIG_SEC_A7X_PROJECT) && defined (CONFIG_ESE_P3)) \
	|| (defined (CONFIG_SEC_A5X_PROJECT) && defined (CONFIG_ESE_P3))
		if (i >= 0 && i <= 3)
			continue;
#endif
		msm_tlmm_v4_get_gp_cfg(i, &val);
		if (val.func == GPIOMUX_FUNC_GPIO) {
			if (val.dir == GPIOMUX_IN)
				temp_io = 0x01;	/* GPIO_IN */
			else if (val.dir == GPIOMUX_OUT_HIGH ||
					val.dir == GPIOMUX_OUT_LOW)
				temp_io = 0x02;	/* GPIO_OUT */
			else {
				temp_io = 0xF;	/* not alloc. */
				pr_err("[secgpio_dvs] gpio : %d, val.dir : %d, temp_io = 0x3",
					i, val.dir);
			}
		} else {
			temp_io = 0x0;		/* FUNC */
		}
		if (val.pull  == GPIOMUX_PULL_NONE)
			temp_pdpu = 0x00;
		else if (val.pull  == GPIOMUX_PULL_DOWN)
			temp_pdpu = 0x01;
		else if (val.pull == GPIOMUX_PULL_UP)
			temp_pdpu = 0x02;
		else if (val.pull == GPIOMUX_PULL_KEEPER)
			temp_pdpu = 0x03;
		else {
			temp_pdpu = 0x07;
			pr_err("[secgpio_dvs] gpio : %d, val.pull : %d, temp_pdpu : %d",
				i, val.pull, temp_pdpu);
		}
		if (val.func == GPIOMUX_FUNC_GPIO) {
			if (val.dir == GPIOMUX_OUT_LOW)
				temp_lh = 0x00;
			else if (val.dir == GPIOMUX_OUT_HIGH)
				temp_lh = 0x01;
			else if (val.dir == GPIOMUX_IN)
				temp_lh = __msm_gpio_get_inout_lh(i);
		} else
			temp_lh = 0;

		checkgpiomap_result[phonestate][i] =
			GET_RESULT_GPIO(temp_io, temp_pdpu, temp_lh);
	}

	pr_info("[secgpio_dvs][%s]-\n", __func__);

	return;
}

#ifdef SECGPIO_SLEEP_DEBUGGING
/****************************************************************/
/* Define this function in accordance with the specification of each BB vendor */
void setgpio_for_sleepdebug(int gpionum, uint16_t  io_pupd_lh)
{
	unsigned char temp_io, temp_pupd, temp_lh;
	unsigned int temp_data;

	pr_info("[secgpio_dvs][%s] gpionum=%d, io_pupd_lh=0x%x\n",
		__func__, gpionum, io_pupd_lh);

	temp_io = GET_GPIO_IO(io_pupd_lh);
	temp_pupd = GET_GPIO_PUPD(io_pupd_lh);
	temp_lh = GET_GPIO_LH(io_pupd_lh);

	pr_info("[secgpio_dvs][%s] io=%d, pupd=%d, lh=%d\n",
		__func__, temp_io, temp_pupd, temp_lh);

	/* in case of 'INPUT', set PD/PU */
	if (temp_io == GDVS_IO_IN) {
		/* 0x0:NP, 0x1:PD, 0x2:PU */
		if (temp_pupd == GDVS_PUPD_NP)
			temp_data = PIN_CONFIG_BIAS_DISABLE;
		else if (temp_pupd == GDVS_PUPD_PD)
			temp_data = PIN_CONFIG_BIAS_PULL_DOWN;
		else if (temp_pupd == GDVS_PUPD_PU)
			temp_data = PIN_CONFIG_BIAS_PULL_UP;
		else		/* It should be not runned */
			temp_data = PIN_CONFIG_BIAS_DISABLE;

		msm_tlmm_v4_set_gp_cfg(gpionum, temp_data, 0);
	}
	/* in case of 'OUTPUT', set L/H */

	else if (temp_io == GDVS_IO_OUT) {
		pr_info("[secgpio_dvs][%s] %d gpio set %d\n",
			__func__, gpionum, temp_lh);
		temp_data = PIN_CONFIG_OUTPUT;
		msm_tlmm_v4_set_gp_cfg(gpionum, temp_data, temp_lh);
	}
	else
	{
		pr_info("[secgpio_dvs][%s] %d gpio set %d NOT VALID\n",
			__func__, gpionum, temp_lh);
	}
}
/****************************************************************/

/****************************************************************/
/* Define this function in accordance with the specification of each BB vendor */
static void undo_sleepgpio(void)
{
	int i;

	pr_info("[secgpio_dvs][%s] ++\n", __func__);

	for (i = 0; i < sleepdebug_table.gpio_count; i++) {
		int gpio_num;
		gpio_num = sleepdebug_table.gpioinfo[i].gpio_num;
		/*
		 * << Caution >>
		 * If it's necessary,
		 * change the following function to another appropriate one
		 * or delete it
		 */
		setgpio_for_sleepdebug(gpio_num, gpiomap_result.sleep[gpio_num]);
	}

	pr_info("[secgpio_dvs][%s] --\n", __func__);
	return;
}
/****************************************************************/
#endif

/********************* Fixed Code Area !***************************/
#ifdef SECGPIO_SLEEP_DEBUGGING
static void set_sleepgpio(void)
{
	int i;
	uint16_t set_data;

	pr_info("[secgpio_dvs][%s] ++, cnt=%d\n",
		__func__, sleepdebug_table.gpio_count);

	for (i = 0; i < sleepdebug_table.gpio_count; i++) {
		int gpio_num;
		pr_info("[secgpio_dvs][%d] gpio_num(%d), io(%d), pupd(%d), lh(%d)\n",
			i, sleepdebug_table.gpioinfo[i].gpio_num,
			sleepdebug_table.gpioinfo[i].io,
			sleepdebug_table.gpioinfo[i].pupd,
			sleepdebug_table.gpioinfo[i].lh);

		gpio_num = sleepdebug_table.gpioinfo[i].gpio_num;

		// to prevent a human error caused by "don't care" value
		if( sleepdebug_table.gpioinfo[i].io == GDVS_IO_IN)		/* IN */
			sleepdebug_table.gpioinfo[i].lh =
				GET_GPIO_LH(gpiomap_result.sleep[gpio_num]);
		else if( sleepdebug_table.gpioinfo[i].io == GDVS_IO_OUT)		/* OUT */
			sleepdebug_table.gpioinfo[i].pupd =
				GET_GPIO_PUPD(gpiomap_result.sleep[gpio_num]);

		set_data = GET_RESULT_GPIO(
			sleepdebug_table.gpioinfo[i].io,
			sleepdebug_table.gpioinfo[i].pupd,
			sleepdebug_table.gpioinfo[i].lh);

		setgpio_for_sleepdebug(gpio_num, set_data);
	}
	pr_info("[secgpio_dvs][%s] --\n", __func__);
	return;
}
#endif

/****************************************************************/
/* Define appropriate variable in accordance with
	the specification of each BB vendor */
static struct gpio_dvs msm8916_gpio_dvs = {
	.result = &gpiomap_result,
	.check_gpio_status = msm8916_check_gpio_status,
	.count = AP_GPIO_COUNT,
	.check_sleep = false,
#ifdef SECGPIO_SLEEP_DEBUGGING
	.sdebugtable = &sleepdebug_table,
	.set_sleepgpio = set_sleepgpio,
	.undo_sleepgpio = undo_sleepgpio,
#endif
};
/****************************************************************/
#endif

#ifdef CONFIG_SEC_PM_DEBUG
static const char * const gpiomux_drv_str[] = {
	"DRV_2mA",
	"DRV_4mA",
	"DRV_6mA",
	"DRV_8mA",
	"DRV_10mA",
	"DRV_12mA",
	"DRV_14mA",
	"DRV_16mA",
};

static const char * const gpiomux_func_str[] = {
	"GPIO",
	"Func_1",
	"Func_2",
	"Func_3",
	"Func_4",
	"Func_5",
	"Func_6",
	"Func_7",
	"Func_8",
	"Func_9",
	"Func_a",
	"Func_b",
	"Func_c",
	"Func_d",
	"Func_e",
	"Func_f",
};

static const char * const gpiomux_pull_str[] = {
	"PULL_NONE",
	"PULL_DOWN",
	"PULL_KEEPER",
	"PULL_UP",
};

static const char * const gpiomux_dir_str[] = {
	"IN",
	"OUT_HIGH",
	"OUT_LOW",
};

static const char * const gpiomux_val_str[] = {
	"VAL_LOW",
	"NULL",
	"VAL_HIGH",
};

static const char * const gpiomux_in_val_str[] = {
	"VAL_LOW",
	"VAL_HIGH",
};

static void gpiomux_debug_print(struct seq_file *m)
{
	unsigned long flags;
	struct gpiomux_setting set;
	unsigned val = 0;
	unsigned gpio;
	unsigned begin = 0;

	spin_lock_irqsave(&gpiomux_lock, flags);

	for (gpio = begin; gpio < msm_gpiomux_ngpio; ++gpio) {
#ifdef ENABLE_SENSORS_FPRINT_SECURE
		if (gpio >= CONFIG_SENSORS_FP_SPI_GPIO_START && gpio <= CONFIG_SENSORS_FP_SPI_GPIO_END)
			continue;
#endif
#ifdef CONFIG_MST_LDO
		if (gpio == MST_GPIO_D_MINUS || gpio == MST_GPIO_D_PLUS)
			continue;
#endif
#if (defined (CONFIG_SEC_A8_PROJECT) && defined (CONFIG_ESE_P61)) \
	|| (defined (CONFIG_SEC_A7X_PROJECT) && defined (CONFIG_ESE_P3)) \
	|| (defined (CONFIG_SEC_A5X_PROJECT) && defined (CONFIG_ESE_P3))
		if (gpio >= 0 && gpio <= 3)
			continue;
#endif
		msm_tlmm_v4_get_gp_cfg(gpio, &set);
		if (set.dir) {
			val = msm_tlmm_v4_get_gp_value(gpio);
			if (IS_ERR_OR_NULL(m))
				pr_info("GPIO[%u] \t%s \t%s \t%s \t%s \t%s\n",
					gpio,
					gpiomux_func_str[set.func],
					gpiomux_dir_str[set.dir],
					gpiomux_pull_str[set.pull],
					gpiomux_drv_str[set.drv],
					gpiomux_val_str[val]);
			else
				seq_printf(m, "GPIO[%u] \t%s \t%s \t%s \t%s \t%s\n",
						gpio,
						gpiomux_func_str[set.func],
						gpiomux_dir_str[set.dir],
						gpiomux_pull_str[set.pull],
						gpiomux_drv_str[set.drv],
						gpiomux_val_str[val]);

		} else {
			val = msm_tlmm_v4_get_gp_input_value(gpio);
			if (IS_ERR_OR_NULL(m))
				pr_info("GPIO[%u] \t%s \t%s \t%s \t%s \t%s\n",
					gpio,
					gpiomux_func_str[set.func],
					gpiomux_dir_str[set.dir],
					gpiomux_pull_str[set.pull],
					gpiomux_drv_str[set.drv],
					gpiomux_in_val_str[val]);
			else
				seq_printf(m, "GPIO[%u] \t%s \t%s \t%s \t%s \t%s\n",
					gpio,
					gpiomux_func_str[set.func],
					gpiomux_dir_str[set.dir],
					gpiomux_pull_str[set.pull],
					gpiomux_drv_str[set.drv],
					gpiomux_in_val_str[val]);
		}

	}

	spin_unlock_irqrestore(&gpiomux_lock, flags);
}

void msm_gpio_print_enabled(void)
{
	gpiomux_debug_print(NULL);
}

static int gpiomux_debug_showall(struct seq_file *m, void *unused)
{
	gpiomux_debug_print(m);
	return 0;
}

static int gpiomux_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, gpiomux_debug_showall, inode->i_private);
}

static const struct file_operations gpiomux_operations = {
	.open		= gpiomux_debug_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int __init msm_gpiomux_debug_init(void)
{
	(void) debugfs_create_file("gpiomux", S_IFREG | S_IRUGO,
				NULL, NULL, &gpiomux_operations);
	return 0;
}
late_initcall(msm_gpiomux_debug_init);
#endif

static int __init msm_gpiomux_init(void)
{

	msm_gpiomux_ngpio = msm_tlmm_v4_get_gp_num();

	return 0;
}
late_initcall(msm_gpiomux_init);

#ifdef CONFIG_SEC_GPIO_DVS
static struct platform_device secgpio_dvs_device = {
	.name	= "secgpio_dvs",
	.id		= -1,
	/****************************************************************
	 * Designate appropriate variable pointer
	 * in accordance with the specification of each BB vendor.
	 ***************************************************************/
	.dev.platform_data = &msm8916_gpio_dvs,
};

static struct platform_device *secgpio_dvs_devices[] __initdata = {
	&secgpio_dvs_device,
};

static int __init secgpio_dvs_device_init(void)
{
	return platform_add_devices(
		secgpio_dvs_devices, ARRAY_SIZE(secgpio_dvs_devices));
}
arch_initcall(secgpio_dvs_device_init);
#endif
