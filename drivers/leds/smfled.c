/* drivers/leds/smfled.c
 * Siliconmitus Flash LED Universal Architecture
 *
 * Copyright (C) 2013 Siliconmitus Technology Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/leds/smfled.h>
#include <linux/init.h>
#include <linux/version.h>

#define SMFLED_INFO(format, args...) \
    printk(KERN_INFO "%s:%s() line-%d: " format, \
            ALIAS_NAME, __FUNCTION__, __LINE__, ## args)
#define SMFLED_WARN(format, args...) \
    printk(KERN_WARNING "%s:%s() line-%d: " format, \
            ALIAS_NAME, __FUNCTION__, __LINE__, ## args)
#define SMFLED_ERR(format, args...) \
    printk(KERN_ERR "%s:%s() line-%d: " format, \
            ALIAS_NAME, __FUNCTION__, __LINE__, ## args)

#define SM_FLED_DEVICE  "sm-flash-led"
#define ALIAS_NAME SM_FLED_DEVICE



sm_fled_info_t *sm_fled_get_info_by_name(char *name)
{
	struct flashlight_device *flashlight_dev;
	flashlight_dev = find_flashlight_by_name(name ? name : SM_FLED_DEVICE);
	if (flashlight_dev == NULL)
		return (sm_fled_info_t *)NULL;
	return flashlight_get_data(flashlight_dev);
}
EXPORT_SYMBOL(sm_fled_get_info_by_name);

static int smfled_set_movie_brightness(struct flashlight_device *flashlight_dev,
				       int brightness_sel)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	return info->hal->fled_set_movie_current_sel(info, brightness_sel);
}

static int smfled_set_flash_brightness(struct flashlight_device
					*flashlight_dev,
					int brightness_sel)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	return info->hal->fled_set_flash_current_sel(info, brightness_sel);
}

static int smfled_set_mode(struct flashlight_device *flashlight_dev, int mode)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	return info->hal->fled_set_mode(info, mode);
}

#if 1
static int smfled_flash(struct flashlight_device *flashlight_dev, int turn_way)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	return info->hal->fled_strobe(info,turn_way);
}
#else
static int smfled_flash(struct flashlight_device *flashlight_dev)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	return info->hal->fled_strobe(info);
}
#endif
static int smfled_set_color_temperature(struct flashlight_device
					*flashlight_dev,
					int color_temp)
{
	/* Doesn't support color temperature */
	return -EINVAL;
}

static int smfled_list_color_temperature(struct flashlight_device
		*flashlight_dev,
		int selector)
{
	/* Doesn't support color temperature */
	return -EINVAL;
}
static int smfled_suspend(struct flashlight_device *flashlight_dev,
			  pm_message_t state)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	if (info->hal->fled_suspend)
		return info->hal->fled_suspend(info, state);
	return 0;
}
static int smfled_resume(struct flashlight_device *flashlight_dev)
{
	sm_fled_info_t *info = flashlight_get_data(flashlight_dev);
	if (info->hal->fled_resume)
		return info->hal->fled_resume(info);
	return 0;
}

static struct flashlight_ops smfled_impl_ops = {
	.set_torch_brightness = smfled_set_movie_brightness,
	.set_strobe_brightness = smfled_set_flash_brightness,
	//.set_flash_timeout = smfled_set_flash_timeout,
	//.list_flash_timeout = smfled_list_flash_timeout,
	.set_mode = smfled_set_mode,
	.strobe = smfled_flash,
	.set_color_temperature = smfled_set_color_temperature,
	.list_color_temperature = smfled_list_color_temperature,
	.suspend = smfled_suspend,
	.resume = smfled_resume,
};


static void smled_shutdown(struct platform_device *pdev)
{
	struct sm_fled_info *info = platform_get_drvdata(pdev);
	if (info->hal->fled_shutdown)
		info->hal->fled_shutdown(info);
}

static int smled_impl_set_movie_current(struct sm_fled_info *info,
					int min_uA, int max_uA, int *selector)
{
	int sel = 0;
	int rc;
	for (sel = 0; ; sel++) {
		rc = info->hal->fled_movie_current_list(info, sel);
		if (rc < 0)
			return rc;
		if (rc >= min_uA && rc <= max_uA) {
			*selector = sel;
			return info->hal->fled_set_movie_current_sel(info, sel);
		}
	}
	return -EINVAL;
}

static int smled_impl_set_flash_current(struct sm_fled_info *info,
		int min_uA, int max_uA, int *selector)
{
	int sel = 0;
	int rc;
	for (sel = 0; ; sel++) {
		rc = info->hal->fled_flash_current_list(info, sel);
		if (rc < 0)
			return rc;
		if (rc >= min_uA && rc <= max_uA) {
			*selector = sel;
			return info->hal->fled_set_flash_current_sel(info, sel);
		}
	}
	return -EINVAL;
}
/*
static int smled_impl_set_flash_timeout(struct sm_fled_info *info,
		int min_ms, int max_ms, int *selector)
{
	int sel = 0;
	int rc;
	for (sel = 0; ; sel++) {
		rc = info->hal->fled_flash_timeout_list(info, sel);
		if (rc < 0)
			return rc;
		if (rc >= min_ms && rc <= max_ms) {
			*selector = sel;
			return info->hal->fled_set_flash_timeout_sel(info, sel);
		}
	}
	return -EINVAL;
}
*/
static int smled_impl_get_movie_current(struct sm_fled_info *info)
{
	int sel = info->hal->fled_get_movie_current_sel(info);
	if (sel < 0)
		return sel;
	return info->hal->fled_movie_current_list(info, sel);
}

static int smled_impl_get_flash_current(struct sm_fled_info *info)
{
	int sel = info->hal->fled_get_flash_current_sel(info);
	if (sel < 0)
		return sel;
	return info->hal->fled_flash_current_list(info, sel);
}

#define HAL_NOT_IMPLEMENTED(x) (hal->x == NULL)
#define CHECK_HAL_IMPLEMENTED(x) if (hal->x == NULL) return -EINVAL

static int smfled_check_hal_implement(struct sm_fled_hal *hal)
{
	if (HAL_NOT_IMPLEMENTED(fled_set_movie_current))
		hal->fled_set_movie_current = smled_impl_set_movie_current;
	if (HAL_NOT_IMPLEMENTED(fled_set_flash_current))
		hal->fled_set_flash_current = smled_impl_set_flash_current;
	if (HAL_NOT_IMPLEMENTED(fled_get_movie_current))
		hal->fled_get_movie_current = smled_impl_get_movie_current;
	if (HAL_NOT_IMPLEMENTED(fled_get_flash_current))
		hal->fled_get_flash_current = smled_impl_get_flash_current;
	CHECK_HAL_IMPLEMENTED(fled_set_mode);
	CHECK_HAL_IMPLEMENTED(fled_get_mode);
	CHECK_HAL_IMPLEMENTED(fled_strobe);
	CHECK_HAL_IMPLEMENTED(fled_movie_current_list);
	CHECK_HAL_IMPLEMENTED(fled_flash_current_list);
	CHECK_HAL_IMPLEMENTED(fled_set_movie_current_sel);
	CHECK_HAL_IMPLEMENTED(fled_set_flash_current_sel);
	CHECK_HAL_IMPLEMENTED(fled_get_movie_current_sel);
	CHECK_HAL_IMPLEMENTED(fled_get_flash_current_sel);
	return 0;
}


static int smfled_probe(struct platform_device *pdev)
{
	sm_fled_info_t *info = dev_get_drvdata(pdev->dev.parent);
	int rc;
	BUG_ON(info == NULL);
	BUG_ON(info->hal == NULL);

	SMFLED_INFO("Siliconmitus FlashLED Driver is probing\n");
	rc = smfled_check_hal_implement(info->hal);
	if (rc < 0) {
		SMFLED_ERR("HAL implemented incompletely\n");
		goto err_check_hal;
	}
	platform_set_drvdata(pdev, info);
	info->flashlight_dev = flashlight_device_register(
				       info->name ? info->name : SM_FLED_DEVICE,
				       &pdev->dev, info, &smfled_impl_ops,
				       info->init_props);
	if (info->hal->fled_init) {
		rc = info->hal->fled_init(info);
		if (rc < 0) {
			SMFLED_ERR("Initialization failed\n");
			goto err_init;
		}
	}
	SMFLED_INFO("Siliconmitus FlashLED Driver initialized successfully\n");

	return 0;
err_init:
	flashlight_device_unregister(info->flashlight_dev);
err_check_hal:
	return rc;
}

static int smfled_remove(struct platform_device *pdev)
{
	sm_fled_info_t *info = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);
	flashlight_device_unregister(info->flashlight_dev);
	return 0;
}

static struct platform_driver sm_flash_led_driver = {
	.driver		= {
		.name	= SM_FLED_DEVICE,
		.owner	= THIS_MODULE,
	},
	.shutdown   = smled_shutdown,
	.probe		= smfled_probe,
	.remove		= smfled_remove,
};

static int __init smfled_init(void)
{
	return platform_driver_register(&sm_flash_led_driver);
}
subsys_initcall(smfled_init);

static void __exit smfled_exit(void)
{
	platform_driver_unregister(&sm_flash_led_driver);
}
module_exit(smfled_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0_G");
MODULE_DESCRIPTION("Siliconmitus Flash LED Driver");

