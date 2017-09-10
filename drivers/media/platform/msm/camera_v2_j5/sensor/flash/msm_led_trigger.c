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
#if defined(CONFIG_FLED_SM5701)
#include <linux/mfd/sm5701_core.h>
#endif
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
#include <linux/leds/rtfled.h>
#endif
#if defined(CONFIG_FLED_SM5703_EXT_GPIO) || defined(CONFIG_FLED_SM5703)
#include <linux/leds/smfled.h>
#endif
#ifdef CONFIG_FLED_SM5703
#include <linux/leds/flashlight.h>
#endif
#include "msm_led_flash.h"

#define FLASH_NAME "camera-led-flash"

#if defined(CONFIG_FLED_LM3632)
extern void ssflash_led_turn_on(void);
extern void ssflash_led_turn_off(void);
#endif
#if defined(CONFIG_FLED_KTD2692)
extern void ktd2692_flash_on(unsigned data);
#endif

extern int system_rev;


/*#define CONFIG_MSMB_CAMERA_DEBUG*/
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

extern int32_t msm_led_torch_create_classdev(
				struct platform_device *pdev, void *data);
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
extern void rt5033_fled_strobe_critial_section_lock(struct rt_fled_info *fled_info);
extern void rt5033_fled_strobe_critial_section_unlock(struct rt_fled_info *fled_info);
extern bool assistive_light;
#endif

#if defined(CONFIG_FLED_SM5701)
extern bool assistive_light;
#endif

#ifdef CONFIG_FLED_SM5703_EXT_GPIO
extern bool assistive_light;
extern int32_t sm5703_fled_notification(struct sm_fled_info *info);
static int led_prev_mode = 0;
#endif

static enum flash_type flashtype;
static struct msm_led_flash_ctrl_t fctrl;
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
static bool lock_state = false;
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

#if defined(CONFIG_FLED_SM5703) && defined(CONFIG_FRONT_FLASH)
static int32_t msm_led_trigger_config(struct msm_led_flash_ctrl_t *fctrl,
		void *data)
{
	int rc = 0;
	struct msm_camera_led_cfg_t *cfg = (struct msm_camera_led_cfg_t *)data;
	int flash_id = 0;
	sm_fled_info_t *fled_info = sm_fled_get_info_by_name(NULL);

	CDBG("called led_state %d\n", cfg->cfgtype);

	if (!fctrl) {
		pr_err("[%s:%d]failed\n", __func__, __LINE__);
		return -EINVAL;
	}

	pr_err("[CAM_LED]FLASH ID:%d\n", cfg->torch_current);
	switch (cfg->torch_current) {
		case 0:
			flash_id = BACK_CAMERA_B;
			break;

		case 1:
			flash_id = FRONT_CAMERA_B;
			break;

		case 3:
			flash_id = 4;	//INIT&RELEASE
			break;

		default:
			pr_err("[CAM_LED]default is back:%d\n", cfg->torch_current);
			flash_id = 0;
			break;
	}

	switch (cfg->cfgtype) {
		case MSM_CAMERA_LED_OFF:
			pr_err("[CAM_LED]LED STATE OFF.\n");
			if(flash_id == FRONT_CAMERA_B){
#if defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				if(system_rev < 5){
					pr_err("ssflaash led turn on msm_led_trigger\n");
					ssflash_led_turn_off();
				}else{
					ktd2692_flash_on(0);
					CDBG("Ktd2692 led turn on msm_led_trigger\n");
				}
				break;
#elif !defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				pr_err("ssflaash led turn on msm_led_trigger\n");
				ssflash_led_turn_off();
				break;
#endif
			}else if(flash_id == BACK_CAMERA_B){
#if defined(CONFIG_FLED_SM5703)
				if (assistive_light == true) {
					pr_err("When assistive light, Not control flash\n");
					return 0;
				}
				if (fled_info) {
					flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_OFF);
					flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
					sm5703_fled_notification(fled_info);
				}
#endif
				break;
			}

		case MSM_CAMERA_LED_LOW:
			pr_err("[CAM_LED]LED STATE LOW.\n");
			if (flash_id == FRONT_CAMERA_B) {
#if defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				if(system_rev < 5){
					ssflash_led_turn_on();
				}else{
					ktd2692_flash_on(1);
				}
				break;
#elif !defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				ssflash_led_turn_on();
				break;
#endif
			}else if(flash_id == BACK_CAMERA_B){
#if defined(CONFIG_FLED_SM5703)
				if (assistive_light == true) {
					pr_err("When assistive light, Not control flash\n");
					return 0;
				}
				if (fled_info) {
					flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_TORCH);
					sm5703_fled_notification(fled_info);
					flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
				}
#endif
				break;
			}

		case MSM_CAMERA_LED_HIGH:
			pr_err("[CAM_LED]LED STATE HIGH.\n");
			if(flash_id == BACK_CAMERA_B){
#if defined(CONFIG_FLED_SM5703)
				if (assistive_light == true) {
					pr_err("When assistive light, Not control flash\n");
					return 0;
				}
				if (fled_info) {
					flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_FLASH);
					sm5703_fled_notification(fled_info);
					flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
				}
#endif
				break;
			}

		case MSM_CAMERA_LED_INIT:
		case MSM_CAMERA_LED_RELEASE:
			CDBG("[CAM_LED]LED STATE INIT/RELEASE.\n");
#if defined(CONFIG_FLED_SM5703)
			if (assistive_light == true) {
				pr_err("When assistive light, Not control flash\n");
			} else if (fled_info) {
				flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_OFF);
				flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
				sm5703_fled_notification(fled_info);
#endif
#if defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				if(system_rev >= 5)
					ktd2692_flash_on(0);
				else
					ssflash_led_turn_off();
#elif !defined(CONFIG_FLED_KTD2692) && defined(CONFIG_FLED_LM3632)
				ssflash_led_turn_off();
#endif
			}
			break;

		default:
			pr_err("[CAM_LED]LED STATE error!\n");
			rc = -EFAULT;
			break;

	}
	CDBG("flash_set_led_state: return %d\n", rc);
	return rc;
}
#else
static int32_t msm_led_trigger_config(struct msm_led_flash_ctrl_t *fctrl,
	void *data)
{
	int rc = 0;
	struct msm_camera_led_cfg_t *cfg = (struct msm_camera_led_cfg_t *)data;
#if !defined(CONFIG_FLED_SM5703)
	uint32_t i;
#endif

#ifdef CONFIG_FLED_RT5033_EXT_GPIO
	rt_fled_info_t *fled_info = rt_fled_get_info_by_name(NULL);
#endif
#if defined(CONFIG_FLED_SM5703_EXT_GPIO) || defined(CONFIG_FLED_SM5703)
	sm_fled_info_t *fled_info = sm_fled_get_info_by_name(NULL);
#endif

	CDBG("called led_state %d\n", cfg->cfgtype);

	if (!fctrl) {
		pr_err("[%s:%d]failed\n", __func__, __LINE__);
		return -EINVAL;
	}

#ifdef CONFIG_SEC_NOVEL_PROJECT
	pr_err("MSM-Not control flash %d\n", cfg->cfgtype);
	return 0;
#endif

	switch (cfg->cfgtype) {
	case MSM_CAMERA_LED_OFF:
		pr_err("[CAM_LED]LED STATE OFF.\n");

#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
#endif
#ifdef CONFIG_FLED_SM5703_EXT_GPIO
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
		if (fled_info) {
			flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_OFF);
			flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
			sm5703_fled_notification(fled_info);
		}
		break;
#endif
#if defined(CONFIG_FLED_SM5701)
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
		sm5701_led_ready(LED_DISABLE);
		sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
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
	case MSM_CAMERA_LED_LOW:
		pr_err("[CAM_LED]LED STATE LOW.\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			pr_err("When assistive light, Not control flash\n");
			return 0;
		}
#endif
#ifdef CONFIG_FLED_SM5703_EXT_GPIO
		if (assistive_light == true) {
			pr_err("When assistive light, Not control flash\n");
			return 0;
		}
		if (fled_info) {
		flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_TORCH);
		sm5703_fled_notification(fled_info);
		flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
		}
		break;

#endif
#if defined(CONFIG_FLED_SM5701)
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
		sm5701_led_ready(MOVIE_MODE);
		sm5701_set_fleden(SM5701_FLEDEN_ON_MOVIE);
		break;
#else
		gpio_request(fctrl->led_irq_gpio1, NULL);
		gpio_direction_output(fctrl->led_irq_gpio1, 1);
		gpio_free(fctrl->led_irq_gpio1);
		break;
#endif

	case MSM_CAMERA_LED_HIGH:
		pr_err("[CAM_LED]LED STATE HIGH.\n");
#ifdef CONFIG_FLED_RT5033_EXT_GPIO
		if (assistive_light == true) {
			pr_err("When assistive light, Not control flash\n");
			return 0;
		}
		if (fled_info) {
			rt5033_fled_strobe_critial_section_lock(fled_info);
			lock_state = true;
		}
#endif
#ifdef CONFIG_FLED_SM5703_EXT_GPIO
		if (assistive_light == true) {
			pr_err("When assistive light, Not control flash\n");
			return 0;
		}
		if (fled_info) {
			flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_FLASH);
			sm5703_fled_notification(fled_info);
			flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
		}
		break;
#endif
#if defined(CONFIG_FLED_SM5701)
		if (assistive_light == true) {
			pr_err("When assistive light, Not control flash\n");
			return 0;
		}
		sm5701_led_ready(FLASH_MODE);
		sm5701_set_fleden(SM5701_FLEDEN_ON_FLASH);
		break;
#else
		gpio_request(fctrl->led_irq_gpio2, NULL);
		gpio_direction_output(fctrl->led_irq_gpio2, 1);
		gpio_free(fctrl->led_irq_gpio2);
		break;
#endif
		break;
	case MSM_CAMERA_LED_INIT:
	case MSM_CAMERA_LED_RELEASE:
		CDBG("[CAM_LED]LED STATE INIT/RELEASE.\n");
#if defined(CONFIG_FLED_SM5703_EXT_GPIO)
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
		} else if (fled_info) {
			flashlight_set_mode(fled_info->flashlight_dev, FLASHLIGHT_MODE_OFF);
			flashlight_strobe(fled_info->flashlight_dev, TURN_WAY_GPIO);
			sm5703_fled_notification(fled_info);
		}
		break;
#endif
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
		break;
#endif
#if defined(CONFIG_FLED_SM5701)
		if (assistive_light == true) {
			CDBG("When assistive light, Not control flash\n");
			return 0;
		}
		sm5701_led_ready(LED_DISABLE);
		sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
		break;
#endif
#if !defined(CONFIG_FLED_SM5703_EXT_GPIO)
		for (i = 0; i < fctrl->num_sources; i++)
			if (fctrl->flash_trigger[i])
				led_trigger_event(fctrl->flash_trigger[i], 0);
		if (fctrl->torch_trigger)
			led_trigger_event(fctrl->torch_trigger, 0);
		break;
#endif
		break;
	default:
		pr_err("[CAM_LED]LED STATE error!\n");
		rc = -EFAULT;
		break;
	}
	CDBG("flash_set_led_state: return %d\n", rc);
	return rc;
}
#endif

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
#if defined(CONFIG_FLED_SM5703)
int set_led_flash(int mode)
{
    struct msm_camera_led_cfg_t cfg;
    int rc = 0;
    cfg.cfgtype = mode;

    if (led_prev_mode && mode)
        return -1;

    rc = msm_led_trigger_config(&fctrl, &mode);

    if (rc == 0)
        led_prev_mode = mode;

    return rc;
}
EXPORT_SYMBOL(set_led_flash);
#endif
module_init(msm_led_trigger_add_driver);
MODULE_DESCRIPTION("LED TRIGGER FLASH");
MODULE_LICENSE("GPL v2");
