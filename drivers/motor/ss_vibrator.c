
/* Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Portions Copyright (c) 2013 The CyanogenMod Project
 *                        Daniel Hillenbrand <codeworkx@cyanogenmod.com>
 *                        Dan Pasanen <dan.pasanen@gmail.com>
 *                        Shareef Ali <shareefalis@cyanogenmod.org>
 *
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

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include "../staging/android/timed_output.h"

#include "ss_vibrator.h"

#define LEVEL_MAX           100
#define LEVEL_MIN           0
#define LEVEL_DEFAULT       50
#define LEVEL_THRESHOLD     75

static unsigned long pwm_val = 100;

/* default timeout */
#define VIB_DEFAULT_TIMEOUT 10000

struct ss_vib {
	struct device *dev;
	struct hrtimer vib_timer;
	struct timed_output_dev timed_dev;
	struct work_struct work;
	struct workqueue_struct *queue;
	struct mutex lock;

	int state;
	int timeout;
	int intensity;
	int timevalue;

	unsigned int vib_pwm_gpio;	/* gpio number for vibrator pwm */
	unsigned int vib_en_gpio;	/* gpio number of vibrator enable */
	unsigned int flag_en_gpio;
	enum driver_chip chip_model;
	unsigned int gp_clk;
	unsigned int m_default;
	unsigned int n_default;
	unsigned int motor_strength;
	void (*power_onoff)(int onoff);
};

void vibe_set_intensity(int intensity)
{
	if (0 == intensity)
		vibe_pwm_onoff(0);
	else {
		if (MAX_INTENSITY == intensity)
			intensity = 1;
		else if (0 != intensity) {
			int tmp = MAX_INTENSITY - intensity;
			intensity = (tmp / 79);	// 79 := 10000 / 127
		}
		vibe_set_pwm_freq(intensity);
		vibe_pwm_onoff(1);
	}
}

int32_t vibe_set_pwm_freq(int intensity)
{
	int32_t calc_d;

	/* Put the MND counter in reset mode for programming */
	HWIO_OUTM(GPx_CFG_RCGR, HWIO_GP_SRC_SEL_VAL_BMSK,
				0 << HWIO_GP_SRC_SEL_VAL_SHFT); //SRC_SEL = 000(cxo)
	HWIO_OUTM(GPx_CFG_RCGR, HWIO_GP_SRC_DIV_VAL_BMSK,
				31 << HWIO_GP_SRC_DIV_VAL_SHFT); //SRC_DIV = 11111 (Div 16)
	HWIO_OUTM(GPx_CFG_RCGR, HWIO_GP_MODE_VAL_BMSK,
				2 << HWIO_GP_MODE_VAL_SHFT); //Mode Select 10
	//M value
	HWIO_OUTM(GPx_M_REG, HWIO_GP_MD_REG_M_VAL_BMSK,
		g_nlra_gp_clk_m << HWIO_GP_MD_REG_M_VAL_SHFT);

	if (intensity > 0){
		calc_d = g_nlra_gp_clk_n - (((intensity * g_nlra_gp_clk_pwm_mul) >> 8));
		calc_d = calc_d * motor_strength / 100;
		if(calc_d < motor_min_strength)
			calc_d = motor_min_strength;
	} else {
		calc_d = ((intensity * g_nlra_gp_clk_pwm_mul) >> 8) + g_nlra_gp_clk_d;
		if(g_nlra_gp_clk_n - calc_d > g_nlra_gp_clk_n * motor_strength / 100)
			calc_d = g_nlra_gp_clk_n - g_nlra_gp_clk_n * motor_strength / 100;
	}
	// D value
	HWIO_OUTM(GPx_D_REG, HWIO_GP_MD_REG_D_VAL_BMSK,
	 (~((int16_t)calc_d << 1)) << HWIO_GP_MD_REG_D_VAL_SHFT);

	//N value
	HWIO_OUTM(GPx_N_REG, HWIO_GP_N_REG_N_VAL_BMSK,
	 ~(g_nlra_gp_clk_n - g_nlra_gp_clk_m) << 0);

	return VIBRATION_SUCCESS;
}

int32_t vibe_pwm_onoff(u8 onoff)
{
	if (onoff) {
		HWIO_OUTM(GPx_CMD_RCGR,HWIO_UPDATE_VAL_BMSK,
					1 << HWIO_UPDATE_VAL_SHFT);//UPDATE ACTIVE
		HWIO_OUTM(GPx_CMD_RCGR,HWIO_ROOT_EN_VAL_BMSK,
					1 << HWIO_ROOT_EN_VAL_SHFT);//ROOT_EN
		HWIO_OUTM(CAMSS_GPx_CBCR, HWIO_CLK_ENABLE_VAL_BMSK,
					1 << HWIO_CLK_ENABLE_VAL_SHFT); //CLK_ENABLE
	} else {

		HWIO_OUTM(GPx_CMD_RCGR,HWIO_UPDATE_VAL_BMSK,
					0 << HWIO_UPDATE_VAL_SHFT);
		HWIO_OUTM(GPx_CMD_RCGR,HWIO_ROOT_EN_VAL_BMSK,
					0 << HWIO_ROOT_EN_VAL_SHFT);
		HWIO_OUTM(CAMSS_GPx_CBCR, HWIO_CLK_ENABLE_VAL_BMSK,
					0 << HWIO_CLK_ENABLE_VAL_SHFT);
	}
	return VIBRATION_SUCCESS;
}

static void set_vibrator(struct ss_vib *vib)
{
	struct pinctrl *motor_pinctrl;

	pr_info("[VIB]: %s, value[%d]\n", __func__, vib->state);
	if (vib->state) {
		motor_pinctrl = devm_pinctrl_get_select(vib->dev, "tlmm_motor_active");
		if (IS_ERR(motor_pinctrl)) {
			if (PTR_ERR(motor_pinctrl) == -EPROBE_DEFER)
				pr_err("[VIB]: Error %d\n", -EPROBE_DEFER);

			pr_debug("[VIB]: Target does not use pinctrl\n");
			motor_pinctrl = NULL;
		}
		if (vib->power_onoff)
			vib->power_onoff(1);
		if (vib->flag_en_gpio)
			gpio_set_value(vib->vib_en_gpio, VIBRATION_ON);

		gpio_set_value(vib->vib_pwm_gpio, VIBRATION_ON);
		hrtimer_start(&vib->vib_timer, ktime_set(vib->timevalue / 1000, (vib->timevalue % 1000) * 1000000),HRTIMER_MODE_REL);
	} else {
		motor_pinctrl = devm_pinctrl_get_select(vib->dev, "tlmm_motor_suspend");
		if (IS_ERR(motor_pinctrl)) {
			if (PTR_ERR(motor_pinctrl) == -EPROBE_DEFER)
				pr_err("[VIB]: Error %d\n", -EPROBE_DEFER);

			pr_debug("[VIB]: Target does not use pinctrl\n");
			motor_pinctrl = NULL;
		}
		gpio_set_value(vib->vib_pwm_gpio, VIBRATION_OFF);

		if (vib->flag_en_gpio)
			gpio_set_value(vib->vib_en_gpio, VIBRATION_OFF);
		if (vib->power_onoff)
			vib->power_onoff(0);
	}
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	struct ss_vib *vib = container_of(dev, struct ss_vib, timed_dev);

	mutex_lock(&vib->lock);
	hrtimer_cancel(&vib->vib_timer);

	if (value == 0) {
		pr_info("[VIB]: OFF\n");
		vib->state = 0;
		vib->timevalue = 0;
	} else {
		pr_info("[VIB]: ON, Duration : %d msec, intensity : %d\n", value, vib->intensity);
		vib->state = 1;
		vib->timevalue = value;
	}
	mutex_unlock(&vib->lock);
	queue_work(vib->queue, &vib->work);
}

static void ss_vibrator_update(struct work_struct *work)
{
	struct ss_vib *vib = container_of(work, struct ss_vib, work);

	set_vibrator(vib);
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	struct ss_vib *vib = container_of(dev, struct ss_vib, timed_dev);

	if (hrtimer_active(&vib->vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&vib->vib_timer);
		return (int)ktime_to_us(r);
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct ss_vib *vib = container_of(timer, struct ss_vib, vib_timer);

	vib->state = 0;
	queue_work(vib->queue, &vib->work);

	return HRTIMER_NORESTART;
}

#ifdef CONFIG_PM
static int ss_vibrator_suspend(struct device *dev)
{
	struct ss_vib *vib = dev_get_drvdata(dev);

	pr_info("[VIB]: %s\n", __func__);

	hrtimer_cancel(&vib->vib_timer);
	cancel_work_sync(&vib->work);
	/* turn-off vibrator */
	vib->state = 0;
	set_vibrator(vib);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(vibrator_pm_ops, ss_vibrator_suspend, NULL);

static int vibrator_parse_dt(struct ss_vib *vib)
{
	struct device_node *np = vib->dev->of_node;
	int rc;

	vib->vib_pwm_gpio = of_get_named_gpio(np, "samsung,vib_pwm", 0);
	if (!gpio_is_valid(vib->vib_pwm_gpio)) {
		pr_err("%s:%d, reset gpio not specified\n", __func__, __LINE__);
	}

	vib->vib_en_gpio = of_get_named_gpio(np, "samsung,vib_en", 0);
	if (!gpio_is_valid(vib->vib_en_gpio)) {
		vib->flag_en_gpio = 0;
		pr_info("%s:%d, en gpio not specified\n", __func__, __LINE__);
	} else {
		vib->flag_en_gpio = 1;
	}

	rc = of_property_read_u32(np, "samsung,chip_model", &vib->chip_model);
	if (rc) {
		pr_info("chip_model not specified so using MAXIM\n");
		vib->chip_model = CHIP_ISAXXX;
	}

	rc = of_property_read_u32(np, "samsung,gp_clk", &vib->gp_clk);
	if (rc) {
		pr_info("gp_clk not specified so using default address\n");
		vib->gp_clk = MSM_GCC_GPx_BASE;
		rc = 0;
	}

	rc = of_property_read_u32(np, "samsung,m_default", &vib->m_default);
	if (rc) {
		pr_info("m_default not specified so using default address\n");
		vib->m_default = GP_CLK_M_DEFAULT;
		rc = 0;
	}

	rc = of_property_read_u32(np, "samsung,n_default", &vib->n_default);
	if (rc) {
		pr_info("n_default not specified so using default address\n");
		vib->n_default = GP_CLK_N_DEFAULT;
		rc = 0;
	}

	rc = of_property_read_u32(np, "samsung,motor_strength", &vib->motor_strength);
	if (rc) {
		pr_info("motor_strength not specified so using default address\n");
		vib->motor_strength = MOTOR_STRENGTH;
		rc = 0;
	}
	return rc;
}

static struct device *vib_dev;
extern struct class *sec_class;

static ssize_t pwm_max_show(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "%d\n", LEVEL_MAX);
	pr_info("vibrator: pwm max value: %d\n", LEVEL_MAX);

	return count;
}

static DEVICE_ATTR(pwm_max, S_IRUGO | S_IWUSR,
                   pwm_max_show, NULL);

static ssize_t pwm_min_show(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "%d\n", LEVEL_MIN);
	pr_info("vibrator: pwm min value: %d\n", LEVEL_MIN);

	return count;
}

static DEVICE_ATTR(pwm_min, S_IRUGO | S_IWUSR,
                   pwm_min_show, NULL);

static ssize_t pwm_default_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "%d\n", LEVEL_DEFAULT);
	pr_info("vibrator: pwm default value: %d\n", LEVEL_DEFAULT);

	return count;
}

static DEVICE_ATTR(pwm_default, S_IRUGO | S_IWUSR,
                   pwm_default_show, NULL);

static ssize_t pwm_threshold_show(struct device *dev,
                                  struct device_attribute *attr, char *buf)
{
	int count;

	count = sprintf(buf, "%d\n", LEVEL_THRESHOLD);
	pr_info("vibrator: pwm threshold value: %d\n", LEVEL_THRESHOLD);

	return count;
}

static DEVICE_ATTR(pwm_threshold, S_IRUGO | S_IWUSR,
                   pwm_threshold_show, NULL);

static ssize_t pwm_value_show(struct device *dev, struct device_attribute *attr,
                              char *buf)
{
	int count;

	count = sprintf(buf, "%lu\n", pwm_val);
	pr_debug("[VIB] pwm_val: %lu\n", pwm_val);

	return count;
}

ssize_t pwm_value_store(struct device *dev, struct device_attribute *attr,
                        const char *buf, size_t size)
{
	struct timed_output_dev *t_dev = dev_get_drvdata(dev);
	struct ss_vib *vib = container_of(t_dev, struct ss_vib, timed_dev);

	int set_intensity = 0;

	if (kstrtoul(buf, 0, &pwm_val))
		pr_err("[VIB] %s: error on storing pwm_val\n", __func__);

	pr_info("[VIB] %s: pwm_val=%lu\n", __func__, pwm_val);

	/* make sure new pwm duty is in range */
	if(pwm_val > 100)
		pwm_val = 100;
	else if (pwm_val < 0)
		pwm_val = 0;

	/*calculate the new intensity value */
	set_intensity = MAX_INTENSITY * pwm_val / 100;

	/* set the new intensity value */
	vibe_set_intensity(set_intensity);
	vib->intensity = set_intensity;

	return size;
}

static DEVICE_ATTR(pwm_value, S_IRUGO | S_IWUSR,
    pwm_value_show, pwm_value_store);


static ssize_t show_vib_tuning(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	sprintf(buf, "gp_m %d, gp_n %d, gp_d %d, pwm_mul %d, strength %d, min_str %d\n",
			g_nlra_gp_clk_m, g_nlra_gp_clk_n, g_nlra_gp_clk_d,
			g_nlra_gp_clk_pwm_mul, motor_strength, motor_min_strength);
	return strlen(buf);
}

static ssize_t store_vib_tuning(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int retval;
	int temp_m, temp_n, temp_str;

	retval = sscanf(buf, "%d %d %d", &temp_m, &temp_n, &temp_str);
	if (retval == 0) {
		pr_info("[VIB]: %s, fail to get vib_tuning value\n", __func__);
		return count;
	}

	g_nlra_gp_clk_m = temp_m;
	g_nlra_gp_clk_n = temp_n;
	g_nlra_gp_clk_d = temp_n / 2;
	g_nlra_gp_clk_pwm_mul = temp_n;
	motor_strength = temp_str;
	motor_min_strength = g_nlra_gp_clk_n*MOTOR_MIN_STRENGTH/100;

	pr_info("[VIB]: %s gp_m %d, gp_n %d, gp_d %d, pwm_mul %d, strength %d, min_str %d\n", __func__,
			g_nlra_gp_clk_m, g_nlra_gp_clk_n, g_nlra_gp_clk_d,
			g_nlra_gp_clk_pwm_mul, motor_strength, motor_min_strength);

	return count;                                                              
}                                                                                  

static DEVICE_ATTR(vib_tuning, 0660, show_vib_tuning, store_vib_tuning);           

static ssize_t intensity_store(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	struct timed_output_dev *t_dev = dev_get_drvdata(dev);
	struct ss_vib *vib = container_of(t_dev, struct ss_vib, timed_dev); 
	int ret = 0, set_intensity = 0; 

	ret = kstrtoint(buf, 0, &set_intensity);

	if ((set_intensity < 0) || (set_intensity > MAX_INTENSITY)) {
		pr_err("[VIB]: %sout of rage\n", __func__);
		return -EINVAL;
	}

	vibe_set_intensity(set_intensity);
	vib->intensity = set_intensity;

	return count;
}

static ssize_t intensity_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct timed_output_dev *t_dev = dev_get_drvdata(dev);
	struct ss_vib *vib = container_of(t_dev, struct ss_vib, timed_dev); 

	return sprintf(buf, "intensity: %u\n", vib->intensity);
}

static DEVICE_ATTR(intensity, 0660, intensity_show, intensity_store);

static int ss_vibrator_probe(struct platform_device *pdev)
{
	struct ss_vib *vib;
	int rc = 0;

	pr_info("[VIB]: %s\n",__func__);

	vib = devm_kzalloc(&pdev->dev, sizeof(*vib), GFP_KERNEL);
	if (!vib) {
		pr_err("[VIB]: %s : Failed to allocate memory\n", __func__);
		return -ENOMEM;
	}

	if (!pdev->dev.of_node) {
		pr_err("[VIB]: %s failed, DT is NULL", __func__);
		return -ENODEV;
	}

	vib->dev = &pdev->dev;
	rc = vibrator_parse_dt(vib);
	if(rc)
		return rc;

	virt_mmss_gp1_base = ioremap(vib->gp_clk, 0x28);
	if (!virt_mmss_gp1_base)
		panic("[VIB]: Unable to ioremap MSM_MMSS_GP1 memory!");

	vib->power_onoff = NULL;
	vib->intensity = MAX_INTENSITY;
	g_nlra_gp_clk_m = vib->m_default;
	g_nlra_gp_clk_n = vib->n_default;
	g_nlra_gp_clk_d = (vib->n_default)/2;
	g_nlra_gp_clk_pwm_mul = vib->n_default;
	motor_strength = vib->motor_strength;
	motor_min_strength = g_nlra_gp_clk_n*MOTOR_MIN_STRENGTH/100;

	vib->timeout = VIB_DEFAULT_TIMEOUT;

	vibe_set_intensity(vib->intensity);
	INIT_WORK(&vib->work, ss_vibrator_update);
	mutex_init(&vib->lock);

	vib->queue = create_singlethread_workqueue("ss_vibrator");
	if (!vib->queue) {
		pr_err("[VIB]: %s: can't create workqueue\n", __func__);
		return -ENOMEM;
	}

	hrtimer_init(&vib->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vib->vib_timer.function = vibrator_timer_func;

	vib->timed_dev.name = "vibrator";
	vib->timed_dev.get_time = vibrator_get_time;
	vib->timed_dev.enable = vibrator_enable;

	dev_set_drvdata(&pdev->dev, vib);

	rc = timed_output_dev_register(&vib->timed_dev);
	if (rc < 0) {
		pr_err("[VIB]: timed_output_dev_register fail (rc=%d)\n", rc);
		goto err_read_vib;
	}

	rc = sysfs_create_file(&vib->timed_dev.dev->kobj, &dev_attr_intensity.attr);
	if (rc < 0) {
		pr_err("[VIB]: Failed to register sysfs intensity: %d\n", rc);
	}

	vib_dev = device_create(sec_class, NULL, 0, NULL, "vib");
	if (IS_ERR(vib_dev)) {
		pr_info("[VIB]: Failed to create device for samsung vib\n");
	}

	rc = sysfs_create_file(&vib_dev->kobj, &dev_attr_vib_tuning.attr);
	if (rc) {
		pr_info("Failed to create sysfs group for samsung specific led\n");
	}

	rc = device_create_file(vib->timed_dev.dev, &dev_attr_pwm_value);
	if (rc < 0) {
		pr_err("[VIB]: device_create_file fail: pwm_value\n");
	}

	rc = device_create_file(vib->timed_dev.dev, &dev_attr_pwm_max);
	if (rc < 0) {
		pr_err("[VIB]: create sysfs fail: pwm_max\n");
	}

	rc = device_create_file(vib->timed_dev.dev, &dev_attr_pwm_min);
	if (rc < 0) {
		pr_err("[VIB]: create sysfs fail: pwm_min\n");
	}

	rc = device_create_file(vib->timed_dev.dev, &dev_attr_pwm_default);
	if (rc < 0) {
		pr_err("[VIB]: create sysfs fail: pwm_default\n");
	}

	rc = device_create_file(vib->timed_dev.dev, &dev_attr_pwm_threshold);
	if (rc < 0) {
		pr_err("[VIB]: create sysfs fail: pwm_threshold\n");
	}

	return 0;
err_read_vib:
	iounmap(virt_mmss_gp1_base);
	destroy_workqueue(vib->queue);
	mutex_destroy(&vib->lock);
	return rc;
}

static int ss_vibrator_remove(struct platform_device *pdev)
{
	struct ss_vib *vib = dev_get_drvdata(&pdev->dev);
	iounmap(virt_mmss_gp1_base);

	destroy_workqueue(vib->queue);
	mutex_destroy(&vib->lock);

	return 0;
}

static const struct of_device_id vib_motor_match[] = {
	{	.compatible = "samsung_vib",
	},
	{}
};

static struct platform_driver ss_vibrator_platdrv =
{
	.driver =
	{
		.name = "samsung_vib",
		.owner = THIS_MODULE,
		.of_match_table = vib_motor_match,
		.pm	= &vibrator_pm_ops,
	},
	.probe = ss_vibrator_probe,
	.remove = ss_vibrator_remove,
};

static int __init ss_timed_vibrator_init(void)
{
	return platform_driver_register(&ss_vibrator_platdrv);
}

void __exit ss_timed_vibrator_exit(void)
{
	platform_driver_unregister(&ss_vibrator_platdrv);
}
module_init(ss_timed_vibrator_init);
module_exit(ss_timed_vibrator_exit);

MODULE_AUTHOR("Samsung Corporation");
MODULE_DESCRIPTION("timed output vibrator device");
MODULE_LICENSE("GPL v2");
