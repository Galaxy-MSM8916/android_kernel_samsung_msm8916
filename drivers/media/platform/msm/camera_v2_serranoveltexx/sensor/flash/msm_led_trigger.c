/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
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

#define pr_fmt(fmt) "%s:%d " fmt, __func__, __LINE__

#include <linux/module.h>
#include <linux/of_gpio.h>
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
#include <linux/leds/rtfled.h>
#endif
#include "msm_led_flash.h"

#define FLASH_NAME "camera-led-flash"

/*#define CONFIG_MSMB_CAMERA_DEBUG*/
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif
// Implementation KTD2692 flashIC
#if defined(CONFIG_FLED_KTD2692)
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#endif

extern int32_t msm_led_torch_create_classdev(
				struct platform_device *pdev, void *data);
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
extern void rt5033_fled_strobe_critial_section_lock(struct rt_fled_info *fled_info);
extern void rt5033_fled_strobe_critial_section_unlock(struct rt_fled_info *fled_info);
extern bool assistive_light;
#endif

static enum flash_type flashtype;
static struct msm_led_flash_ctrl_t fctrl;
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
static bool lock_state = false;
#endif

#if defined(CONFIG_FLED_KTD2692)
bool is_torch_enabled;

extern struct class *camera_class; /*sys/class/camera*/
struct device *flash_dev;

/* KTD2692 : command time delay(us) */
#define T_DS		15	//	12
#define T_EOD_H		1000 //	350
#define T_EOD_L		10
#define T_H_LB		10
#define T_L_LB		3*T_H_LB
#define T_L_HB		10
#define T_H_HB		7*T_L_HB
#define T_RESET		800	//	700
#define T_RESET2	1000
/* KTD2692 : command address(A2:A0) */
#define LVP_SETTING		0x0 << 5
#define FLASH_TIMEOUT	0x1 << 5
#define MIN_CURRENT		0x2 << 5
#define MOVIE_CURRENT	0x3 << 5
#define FLASH_CURRENT	0x4 << 5
#define MODE_CONTROL	0x5 << 5

static DEFINE_SPINLOCK(flash_ctrl_lock);
void KTD2692_set_flash(struct msm_led_flash_ctrl_t *fctrl, unsigned int ctl_cmd)
{
	int i=0;
	int j = 0;
	int k = 0;
	unsigned long flags;
	unsigned int ctl_cmd_buf;
	spin_lock_irqsave(&flash_ctrl_lock, flags);
	if ( MODE_CONTROL == (MODE_CONTROL & ctl_cmd) )
		k = 8;
	else
		k = 1;
	for(j = 0; j < k; j++) {
		CDBG("[cmd::0x%2X][MODE_CONTROL&cmd::0x%2X][k::%d]\n", ctl_cmd, (MODE_CONTROL & ctl_cmd), k);
		gpio_set_value(fctrl->led_irq_gpio1, 1);
		udelay(T_DS);

		ctl_cmd_buf = ctl_cmd;
		for(i = 0; i < 8; i++) {
			if(ctl_cmd_buf & 0x80) { /* set bit to 1 */
				gpio_set_value(fctrl->led_irq_gpio1, 0);
				gpio_set_value(fctrl->led_irq_gpio1, 1);
				udelay(T_H_HB);
			} else { /* set bit to 0 */
				gpio_set_value(fctrl->led_irq_gpio1, 0);
				udelay(T_L_LB);
				gpio_set_value(fctrl->led_irq_gpio1, 1);
			}
			ctl_cmd_buf = ctl_cmd_buf << 1;
		}

		gpio_set_value(fctrl->led_irq_gpio1, 0);
		udelay(T_EOD_L);
		gpio_set_value(fctrl->led_irq_gpio1, 1);
		udelay(T_EOD_H);
	}
	spin_unlock_irqrestore(&flash_ctrl_lock, flags);
}

static ssize_t ktd2692_flash(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t size)
{
	ssize_t ret = -EINVAL;
	char *after;
	unsigned long state = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;
	if (count == size) {
		ret = count;
		if (state == 0) {
			KTD2692_set_flash(&fctrl, MODE_CONTROL | 0x00);
			gpio_set_value(fctrl.led_irq_gpio1, 0);
			is_torch_enabled = false;
		} else {
			KTD2692_set_flash(&fctrl, LVP_SETTING | 0x00);
			KTD2692_set_flash(&fctrl, MOVIE_CURRENT | 0x03);
			KTD2692_set_flash(&fctrl, MODE_CONTROL | 0x01); /* Movie mode */
			is_torch_enabled = true;
		}
	}

	return ret;
}

static DEVICE_ATTR(rear_flash, S_IWUSR|S_IWGRP|S_IROTH,
	NULL, ktd2692_flash);


int create_flash_sysfs(void)
{
    int err = -ENODEV;

    CDBG("flash_sysfs: sysfs test!!!! (%s)\n",__func__);

    if (IS_ERR_OR_NULL(camera_class)) {
        pr_err("flash_sysfs: error, camera class not exist");
        return -ENODEV;
    }

    flash_dev = device_create(camera_class, NULL, 0, NULL, "flash");
    if (IS_ERR(flash_dev)) {
        pr_err("flash_sysfs: failed to create device(flash)\n");
        return -ENODEV;
    }

    err = device_create_file(flash_dev, &dev_attr_rear_flash);
    if (unlikely(err < 0)) {
        pr_err("flash_sysfs: failed to create device file, %s\n",
                dev_attr_rear_flash.attr.name);
    }
    return 0;
}

#endif

static int32_t msm_led_trigger_get_subdev_id(struct msm_led_flash_ctrl_t *fctrl,
	void *arg)
{
	uint32_t *subdev_id = (uint32_t *)arg;
	if (!subdev_id) {
		pr_err("%s:%d failed\n", __func__, __LINE__);
		return -EINVAL;
	}
	*subdev_id = fctrl->pdev->id;
	CDBG("%s:%d subdev_id %d\n", __func__, __LINE__, *subdev_id);
	return 0;
}

static int32_t msm_led_trigger_config(struct msm_led_flash_ctrl_t *fctrl,
	void *data)
{
	int rc = 0;
	struct msm_camera_led_cfg_t *cfg = (struct msm_camera_led_cfg_t *)data;
	uint32_t i;
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
	rt_fled_info_t *fled_info = rt_fled_get_info_by_name(NULL);
#endif
#if 0
	uint32_t curr_l, max_curr_l;
#endif
	CDBG("called led_state %d\n", cfg->cfgtype);

#if defined(CONFIG_FLED_KTD2692)
	if (is_torch_enabled == true) {
		return rc;
	}
#endif
	if (!fctrl) {
		pr_err("failed\n");
		return -EINVAL;
	}

	switch (cfg->cfgtype) {
	case MSM_CAMERA_LED_OFF:
		pr_err("MSM_CAMERA_LED_OFF\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
#endif
#if defined(CONFIG_FLED_KTD2692)
		KTD2692_set_flash(fctrl, MODE_CONTROL | 0x00);
#else
		gpio_request(fctrl->led_irq_gpio1, NULL);
		gpio_request(fctrl->led_irq_gpio2, NULL);
		gpio_direction_output(fctrl->led_irq_gpio1, 0);
		gpio_direction_output(fctrl->led_irq_gpio2, 0);
		gpio_free(fctrl->led_irq_gpio1);
		gpio_free(fctrl->led_irq_gpio2);
#endif
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (lock_state) {
			if (fled_info) {
				rt5033_fled_strobe_critial_section_unlock(fled_info);
				lock_state = false;
			}
		}
#endif
		break;
#if 0
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i], 0);
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		break;
#endif

	case MSM_CAMERA_LED_LOW:
		pr_err("MSM_CAMERA_LED_LOW\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
#endif
#if defined(CONFIG_FLED_KTD2692)
		gpio_set_value(fctrl->led_irq_gpio1, 0);
		udelay(T_RESET);
		gpio_set_value(fctrl->led_irq_gpio1, 1);
		udelay(T_RESET2);
		KTD2692_set_flash(fctrl, LVP_SETTING | 0x00);
		KTD2692_set_flash(fctrl, MOVIE_CURRENT | 0x03);
		KTD2692_set_flash(fctrl, MODE_CONTROL | 0x01);
#else
		gpio_request(fctrl->led_irq_gpio1, NULL);
		gpio_direction_output(fctrl->led_irq_gpio1, 1);
		gpio_free(fctrl->led_irq_gpio1);
#endif
		break;
#if 0
		if (fctrl->torch_trigger) {
			max_curr_l = fctrl->torch_max_current;
			if (cfg->torch_current > 0 &&
				cfg->torch_current < max_curr_l) {
				curr_l = cfg->torch_current;
			} else {
				curr_l = fctrl->torch_op_current;
				pr_debug("LED current clamped to %d\n",
					curr_l);
			}
			led_trigger_event(fctrl->torch_trigger,
				curr_l);
		}
		break;
#endif

	case MSM_CAMERA_LED_HIGH:
		pr_err("MSM_CAMERA_LED_HIGH\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
		if (fled_info) {
			rt5033_fled_strobe_critial_section_lock(fled_info);
			lock_state = true;
		}
#endif
#if defined(CONFIG_FLED_KTD2692)
		gpio_set_value(fctrl->led_irq_gpio1, 0);
		udelay(T_RESET);
		gpio_set_value(fctrl->led_irq_gpio1, 1);
		udelay(T_RESET2);
		KTD2692_set_flash(fctrl, LVP_SETTING | 0x00);
		KTD2692_set_flash(fctrl, FLASH_CURRENT | 0x0C);
		KTD2692_set_flash(fctrl, MODE_CONTROL | 0x02);
#else
		gpio_request(fctrl->led_irq_gpio2, NULL);
		gpio_direction_output(fctrl->led_irq_gpio2, 1);
		gpio_free(fctrl->led_irq_gpio2);
#endif
		break;
#if 0
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i]) {
				max_curr_l = fctrl->flash_max_current[i];
				if (cfg->flash_current[i] > 0 &&
					cfg->flash_current[i] < max_curr_l) {
					curr_l = cfg->flash_current[i];
				} else {
					curr_l = fctrl->flash_op_current[i];
					pr_debug("LED current clamped to %d\n",
						curr_l);
				}
				led_trigger_event(fctrl->flash_trigger[i],
					curr_l);
			}
		break;
#endif
	case MSM_CAMERA_LED_INIT:
	case MSM_CAMERA_LED_RELEASE:
		CDBG("MSM_CAMERA_LED_INIT\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
		} else {
			gpio_request(fctrl->led_irq_gpio1, NULL);
			gpio_request(fctrl->led_irq_gpio2, NULL);
			gpio_direction_output(fctrl->led_irq_gpio1, 0);
			gpio_direction_output(fctrl->led_irq_gpio2, 0);
			gpio_free(fctrl->led_irq_gpio1);
			gpio_free(fctrl->led_irq_gpio2);
			if (lock_state) {
				if (fled_info) {
					rt5033_fled_strobe_critial_section_unlock(fled_info);
					lock_state = false;
				}
			}
		}
#endif
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i], 0);
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		break;

	default:
		pr_err("LED state error!\n");
		rc = -EFAULT;
		break;
	}
	CDBG("flash_set_led_state: return %d\n", rc);
	return rc;
}

static const struct of_device_id msm_led_trigger_dt_match[] = {
	{.compatible = "qcom,camera-led-flash"},
	{}
};

MODULE_DEVICE_TABLE(of, msm_led_trigger_dt_match);

static struct platform_driver msm_led_trigger_driver = {
	.driver = {
		.name = FLASH_NAME,
		.owner = THIS_MODULE,
		.of_match_table = msm_led_trigger_dt_match,
	},
};

static int32_t msm_led_trigger_probe(struct platform_device *pdev)
{
	int32_t rc = 0, rc_1 = 0, i = 0;
	struct device_node *of_node = pdev->dev.of_node;
	struct device_node *flash_src_node = NULL;
	uint32_t count = 0;
	struct led_trigger *temp = NULL;

	CDBG("called\n");

	if (!of_node) {
		pr_err("of_node NULL\n");
		return -EINVAL;
	}

	fctrl.pdev = pdev;
	fctrl.num_sources = 0;

	rc = of_property_read_u32(of_node, "cell-index", &pdev->id);
	if (rc < 0) {
		pr_err("failed\n");
		return -EINVAL;
	}
	CDBG("pdev id %d\n", pdev->id);

	rc = of_property_read_u32(of_node,
			"qcom,flash-type", &flashtype);
	if (rc < 0) {
		pr_err("flash-type: read failed\n");
		return -EINVAL;
	}

	if (of_get_property(of_node, "qcom,flash-source", &count)) {
		count /= sizeof(uint32_t);
		CDBG("count %d\n", count);
		if (count > MAX_LED_TRIGGERS) {
			pr_err("invalid count\n");
			return -EINVAL;
		}
		fctrl.num_sources = count;
		for (i = 0; i < count; i++) {
			flash_src_node = of_parse_phandle(of_node,
				"qcom,flash-source", i);
			if (!flash_src_node) {
				pr_err("flash_src_node NULL\n");
				continue;
			}

			rc = of_property_read_string(flash_src_node,
				"linux,default-trigger",
				&fctrl.flash_trigger_name[i]);
			if (rc < 0) {
				pr_err("default-trigger: read failed\n");
				of_node_put(flash_src_node);
				continue;
			}

			CDBG("default trigger %s\n",
				fctrl.flash_trigger_name[i]);

			if (flashtype == GPIO_FLASH) {
				/* use fake current */
				fctrl.flash_op_current[i] = LED_FULL;
			} else {
				rc = of_property_read_u32(flash_src_node,
					"qcom,current",
					&fctrl.flash_op_current[i]);
				rc_1 = of_property_read_u32(flash_src_node,
					"qcom,max-current",
					&fctrl.flash_max_current[i]);
				if ((rc < 0) || (rc_1 < 0)) {
					pr_err("current: read failed\n");
					of_node_put(flash_src_node);
					continue;
				}
			}

			of_node_put(flash_src_node);

			CDBG("max_current[%d] %d\n",
				i, fctrl.flash_op_current[i]);

			led_trigger_register_simple(fctrl.flash_trigger_name[i],
				&fctrl.flash_trigger[i]);

			if (flashtype == GPIO_FLASH)
				if (fctrl.flash_trigger[i])
					temp = fctrl.flash_trigger[i];
		}

		/* Torch source */
		flash_src_node = of_parse_phandle(of_node, "qcom,torch-source",
			0);
		if (flash_src_node) {
			rc = of_property_read_string(flash_src_node,
				"linux,default-trigger",
				&fctrl.torch_trigger_name);
			if (rc < 0) {
				pr_err("default-trigger: read failed\n");
				goto torch_failed;
			}

			CDBG("default trigger %s\n",
				fctrl.torch_trigger_name);

			if (flashtype == GPIO_FLASH) {
				/* use fake current */
				fctrl.torch_op_current = LED_HALF;
				if (temp)
					fctrl.torch_trigger = temp;
				else
					led_trigger_register_simple(
						fctrl.torch_trigger_name,
						&fctrl.torch_trigger);
			} else {
				rc = of_property_read_u32(flash_src_node,
					"qcom,current",
					&fctrl.torch_op_current);
				rc_1 = of_property_read_u32(flash_src_node,
					"qcom,max-current",
					&fctrl.torch_max_current);

				if ((rc < 0) || (rc_1 < 0)) {
					pr_err("current: read failed\n");
					goto torch_failed;
				}

				CDBG("torch max_current %d\n",
					fctrl.torch_op_current);

				led_trigger_register_simple(
					fctrl.torch_trigger_name,
					&fctrl.torch_trigger);
			}
torch_failed:
			of_node_put(flash_src_node);
		}
	}

	rc = msm_led_flash_create_v4lsubdev(pdev, &fctrl);
	if (!rc) {
		msm_led_torch_create_classdev(pdev, &fctrl);
	}
#if defined(CONFIG_FLED_KTD2692)
	create_flash_sysfs();
#endif

	return rc;
}

static int __init msm_led_trigger_add_driver(void)
{
	CDBG("called\n");
	return platform_driver_probe(&msm_led_trigger_driver,
		msm_led_trigger_probe);
}

static struct msm_flash_fn_t msm_led_trigger_func_tbl = {
	.flash_get_subdev_id = msm_led_trigger_get_subdev_id,
	.flash_led_config = msm_led_trigger_config,
};

static struct msm_led_flash_ctrl_t fctrl = {
	.func_tbl = &msm_led_trigger_func_tbl,
};

module_init(msm_led_trigger_add_driver);
MODULE_DESCRIPTION("LED TRIGGER FLASH");
MODULE_LICENSE("GPL v2");
