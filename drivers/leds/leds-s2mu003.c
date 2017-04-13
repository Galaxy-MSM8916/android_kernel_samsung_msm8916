/*
 * leds-s2mu003.c - LED class driver for S2MU003 LEDs.
 *
 * Copyright (C) 2014 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/mfd/samsung/s2mu003.h>
#include <linux/mfd/samsung/s2mu003-private.h>
#include <linux/leds-s2mu003.h>
#include <linux/platform_device.h>

#define FLED_PINCTRL_STATE_DEFAULT "fled_default"
#define FLED_PINCTRL_STATE_SLEEP "fled_sleep"

extern struct class *camera_class;
struct device *s2mu003_dev;
struct s2mu003_led_data *global_led_datas[S2MU003_LED_MAX];
struct s2mu003_led_data {
	struct s2mu003_mfd_chip *iodev;
	struct led_classdev cdev;
	struct s2mu003_led *data;
	struct notifier_block batt_nb;
	struct i2c_client *i2c;
	struct work_struct work;
	struct mutex lock;
	spinlock_t value_lock;
	int brightness;
	int test_brightness;
	int attach_ta;
	bool enable;
	int torch_pin;
	int flash_pin;
};

static u8 leds_mask[S2MU003_LED_MAX] = {
	S2MU003_FLASH_ENABLE_MASK,
	S2MU003_TORCH_ENABLE_MASK,
};

static u8 leds_shift[S2MU003_LED_MAX] = {
	S2MU003_BOOST_FLASH_MODE_MASK,
	S2MU003_FLASH_TORCH_OFF,
};

u32 original_brightness;
extern bool assistive_light;

#ifdef CONFIG_MUIC_NOTIFIER
static void attach_cable_check(muic_attached_dev_t attached_dev,
					int *attach_ta)
{
	switch (attached_dev) {
	case ATTACHED_DEV_TA_MUIC:
	case ATTACHED_DEV_SMARTDOCK_TA_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_TA_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_TA_MUIC:
	case ATTACHED_DEV_CDP_MUIC:
	case ATTACHED_DEV_UNOFFICIAL_ID_CDP_MUIC:
		*attach_ta = 1;
		break;
	default:
		*attach_ta = 0;
		break;
	}
}

static int ta_notification(struct notifier_block *nb,
		unsigned long action, void *data)
{
	muic_attached_dev_t attached_dev = *(muic_attached_dev_t *)data;
	u8 temp;
	int ret;
	struct s2mu003_led_data *led_data =
		container_of(nb, struct s2mu003_led_data, batt_nb);

	switch (action) {
	case MUIC_NOTIFY_CMD_DETACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_DETACH:
		pr_info("%s : DETACH\n", __func__);
		if (!led_data->attach_ta)
			goto err;

		pr_info("%s : TA DETACH\n", __func__);
		ret = s2mu003_assign_bits(led_data->i2c,
			S2MU003_FLED_CTRL1, 0x80, 0x00);
		if (ret < 0)
			goto err;

		temp = s2mu003_reg_read(led_data->i2c,
				S2MU003_FLED_CH1_CTRL4);
		if ((temp & 0x0C) == 0x0C) {
			ret = s2mu003_assign_bits(led_data->i2c,
				S2MU003_FLED_CH1_CTRL4,
				S2MU003_TORCH_ENABLE_MASK,
				S2MU003_FLASH_TORCH_OFF);

			pr_info("%s : LED OFF\n", __func__);
			if (ret < 0)
				goto err;
			ret = s2mu003_assign_bits(led_data->i2c,
				S2MU003_FLED_CH1_CTRL4,
				S2MU003_TORCH_ENABLE_MASK,
				S2MU003_TORCH_ON_I2C);

			pr_info("%s : LED ON\n", __func__);
			if (ret < 0)
				goto err;
		}
		break;

	case MUIC_NOTIFY_CMD_ATTACH:
	case MUIC_NOTIFY_CMD_LOGICALLY_ATTACH:
		pr_info("%s : ATTACH", __func__);
		led_data->attach_ta = 0;
		attach_cable_check(attached_dev, &led_data->attach_ta);
		if (led_data->attach_ta) {
			ret = s2mu003_assign_bits(led_data->i2c,
				S2MU003_FLED_CTRL1, 0x80, 0x80);

			pr_info("%s : TA ATTACH\n", __func__);
			if (ret < 0)
				goto err;
		}
		return 0;
	default:
		goto err;
		break;
	}

	pr_info("%s : complete TA detached\n", __func__);
	return 0;
err:
	pr_err("%s : abandond access %d\n", __func__, led_data->attach_ta);
	return 0;
}
#endif

static void s2mu003_led_set(struct led_classdev *led_cdev,
			enum led_brightness value)
{
	unsigned long flags;
	struct s2mu003_led_data *led_data =
		container_of(led_cdev, struct s2mu003_led_data, cdev);
	u8 max;

	max = led_cdev->max_brightness;

	pr_info("%s value = %d, max = %d\n", __func__, value, max);

	spin_lock_irqsave(&led_data->value_lock, flags);
	led_data->data->brightness = min_t(int, (int)value, (int)max);
	spin_unlock_irqrestore(&led_data->value_lock, flags);

	schedule_work(&led_data->work);
	return;
}

static void led_set(struct s2mu003_led_data *led_data)
{
	int ret, boost_mode;
	struct s2mu003_led *data = led_data->data;
	int id = data->id;
	u8 mask = 0, reg = 0;

#ifdef CONFIG_S2MU003_LEDS_I2C
	u8 enable_mask, value;
#else
	int gpio_pin;
#endif
	if (id == S2MU003_FLASH_LED) {
		pr_info("%s led mode is flash\n", __func__);
		reg = S2MU003_FLED_CH1_CTRL0;
		mask = S2MU003_FLASH_IOUT_MASK;
		boost_mode = 0x0;
#ifndef CONFIG_S2MU003_LEDS_I2C
		pr_info("%s gpio_flash mode\n", __func__);
		gpio_pin = led_data->flash_pin;
#endif
	} else {
		pr_info("%s led mode is torch\n", __func__);
		reg = S2MU003_FLED_CH1_CTRL1;
		mask = S2MU003_TORCH_IOUT_MASK;
		boost_mode = 0x4;
#ifndef CONFIG_S2MU003_LEDS_I2C
		pr_info("%s gpio_torch mode\n", __func__);
		gpio_pin = led_data->torch_pin;
#endif
	}


#ifndef CONFIG_S2MU003_LEDS_I2C
	if (gpio_is_valid(gpio_pin)) {
		ret = devm_gpio_request(led_data->cdev.dev, gpio_pin,
				"s2mu003_gpio");
		if (ret) {
			pr_err("%s : fail to assignment gpio\n", __func__);
			goto gpio_free_data;
		}
	}
#endif
	pr_info("%s start led_set\n", __func__);

	if (global_led_datas[S2MU003_TORCH_LED]->data->brightness == LED_OFF) {
		ret = s2mu003_assign_bits(led_data->i2c, reg,
				mask, led_data->data->brightness);
		if (ret < 0)
			goto error_set_bits;

#ifdef CONFIG_S2MU003_LEDS_I2C
		value = S2MU003_FLASH_TORCH_OFF;
#else
		gpio_direction_output(gpio_pin, 0);
		goto gpio_free_data;
#endif
	} else {
		pr_info("%s led on\n", __func__);

		ret = s2mu003_assign_bits(led_data->i2c,
			S2MU003_FLED_CTRL1,
			S2MU003_BOOST_FLASH_MODE_MASK, boost_mode);
		if (ret < 0)
			goto error_set_bits;

		ret = s2mu003_assign_bits(led_data->i2c,
				reg, mask, led_data->data->brightness);
		if (ret < 0)
			goto error_set_bits;

#ifdef CONFIG_S2MU003_LEDS_I2C
		value = id ? S2MU003_TORCH_ON_I2C : S2MU003_FLASH_ON_I2C;
#else
		gpio_direction_output(gpio_pin, 1);
		goto gpio_free_data;

#endif
	}
#ifdef CONFIG_S2MU003_LEDS_I2C
	enable_mask = id ? S2MU003_TORCH_ENABLE_MASK :
		S2MU003_FLASH_ENABLE_MASK;

	ret = s2mu003_assign_bits(led_data->i2c,
		S2MU003_FLED_CH1_CTRL4,
		enable_mask, value);

	if (ret < 0)
		goto error_set_bits;
#endif
	return;

#ifndef CONFIG_S2MU003_LEDS_I2C
gpio_free_data:
	gpio_free(gpio_pin);
	pr_info("%s : gpio free\n", __func__);
	return;
#endif
error_set_bits:
	pr_err("%s: can't set led level %d\n", __func__, ret);
	return;
}

static void s2mu003_led_work(struct work_struct *work)
{
	struct s2mu003_led_data *led_data
		= container_of(work, struct s2mu003_led_data, work);

	pr_debug("%s [led]\n", __func__);

	mutex_lock(&led_data->lock);
	led_set(led_data);
	mutex_unlock(&led_data->lock);
}

static int s2mu003_led_setup(struct s2mu003_led_data *led_data)
{
	int ret = 0;
#ifdef CONFIG_S2MU003_LEDS_I2C
	int mask, value;
#endif
	ret = s2mu003_assign_bits(led_data->i2c, 0x89, 0x08, 0x08);
	if (ret < 0)
		goto out;

	ret = s2mu003_assign_bits(led_data->i2c, S2MU003_FLED_CTRL2,
			S2MU003_EN_CHANNEL_SHARE_MASK, 0x80);
	if (ret < 0)
		goto out;

	ret = s2mu003_assign_bits(led_data->i2c,
			S2MU003_FLED_CH1_CTRL3, 0x80, 0x80);
	if (ret < 0)
		goto out;

	ret = s2mu003_assign_bits(led_data->i2c, S2MU003_FLED_CH1_CTRL3,
			S2MU003_TIMEOUT_MAX, S2MU003_FLASH_TIMEOUT_992MS);
	if (ret < 0)
		goto out;

	ret = s2mu003_assign_bits(led_data->i2c, S2MU003_FLED_CH1_CTRL2,
			S2MU003_TIMEOUT_MAX, S2MU003_TORCH_TIMEOUT_15728MS);
	if (ret < 0)
		goto out;

	ret = s2mu003_assign_bits(led_data->i2c, S2MU003_FLED_CTRL0,
			S2MU003_EN_CHANNEL_SHARE_MASK, 0x0);
	if (ret < 0)
		goto out;

#ifdef CONFIG_S2MU003_LEDS_I2C
	value =	S2MU003_FLASH_TORCH_OFF;
	mask = S2MU003_TORCH_ENABLE_MASK | S2MU003_FLASH_ENABLE_MASK;
	ret = s2mu003_assign_bits(led_data->i2c, S2MU003_FLED_CH1_CTRL4,
		mask, value);
#endif
	if (ret < 0)
		goto out;

	pr_info("%s : led setup complete\n", __func__);
	return ret;

out:
	pr_err("%s : led setup fail\n", __func__);
	return ret;
}

static ssize_t rear_flash_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if(global_led_datas[S2MU003_TORCH_LED] == NULL) {
		pr_err("<%s> global_led_datas[S2MU003_TORCH_LED] is NULL\n", __func__);
		return -1;
	}
	pr_info("[LED] %s , MAX STEP TORCH_LED:%d\n", __func__, S2MU003_TORCH_OUT_I_MAX);
	return sprintf(buf, "%d\n", S2MU003_TORCH_OUT_I_MAX);
}

static ssize_t rear_flash_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	int value = 0;

	if ((buf == NULL) || kstrtouint(buf, 10, &value)) {
		return -1;
	}

	if(global_led_datas[S2MU003_TORCH_LED] == NULL) {
		pr_err("<%s> global_led_datas[S2MU003_TORCH_LED] is NULL\n", __func__);
		return -1;
	}

	pr_err("[LED]%s , value:%d\n", __func__, value);
	if (value == 0) {
		// Turn off Torch
		assistive_light = false;
		global_led_datas[S2MU003_TORCH_LED]->data->brightness = LED_OFF;
		led_set(global_led_datas[S2MU003_TORCH_LED]);
	} else if (value == 1) {
		// Turn on Torch
		assistive_light = true;
		global_led_datas[S2MU003_TORCH_LED]->data->brightness = S2MU003_TORCH_OUT_I_75MA;
		led_set(global_led_datas[S2MU003_TORCH_LED]);
	} else if (value == 100) {
		// Factory mode Turn on Torch
		assistive_light = true;
		global_led_datas[S2MU003_TORCH_LED]->data->brightness = S2MU003_TORCH_OUT_I_250MA;
		led_set(global_led_datas[S2MU003_TORCH_LED]);
	} else {
		pr_info("[LED]%s , Invalid value:%d\n", __func__, value);
	}

	if (value <= 0) {
		s2mu003_assign_bits(global_led_datas[S2MU003_TORCH_LED]->i2c, S2MU003_FLED_CTRL1,
				leds_mask[global_led_datas[S2MU003_TORCH_LED]->data->id],
				original_brightness << leds_shift[global_led_datas[S2MU003_TORCH_LED]->data->id]);
		global_led_datas[S2MU003_TORCH_LED]->data->brightness = original_brightness;
	}
	return size;
}

static DEVICE_ATTR(rear_flash, 0644, rear_flash_show, rear_flash_store);

#if defined(CONFIG_OF)
static int s2mu003_led_dt_parse_pdata(struct s2mu003_mfd_chip *iodev,
				struct s2mu003_fled_platform_data *pdata)
{
	struct device_node *led_np, *np, *c_np;
	int ret;
	u32 temp;
	const char *temp_str;
	int index;

	led_np = iodev->dev->of_node;
	if (!led_np) {
		pr_err("<%s> could not find led sub-node led_np\n", __func__);
		return -ENODEV;
	}

	np = of_find_node_by_name(led_np, "s2mu003_fled");
	if (!np) {
		pr_err("%s : could not find led sub-node np\n", __func__);
		return -EINVAL;
	}

	ret = pdata->torch_pin = of_get_named_gpio(np, "s2mu003,torch-gpio", 0);
	if (ret < 0) {
		pr_err("%s : can't get torch-gpio\n", __func__);
		return ret;
	}

	ret = pdata->flash_pin = of_get_named_gpio(np, "s2mu003,flash-gpio", 0);
	if (ret < 0) {
		pr_err("%s : can't get flash-gpio\n", __func__);
		return ret;
	}

	pdata->num_leds = of_get_child_count(np);

	for_each_child_of_node(np, c_np) {
		ret = of_property_read_u32(c_np, "id", &temp);
		if (ret < 0)
			goto dt_err;
		index = temp;
		pdata->leds[index].id = temp;

		ret = of_property_read_string(c_np, "ledname", &temp_str);
		if (ret)
			goto dt_err;
		pdata->leds[index].name = temp_str;

		ret = of_property_read_u32(c_np, "brightness", &temp);
		if (ret)
			goto dt_err;
		if (temp > leds_cur_max[index])
			temp = leds_cur_max[index];
		pdata->leds[index].brightness = temp;
		original_brightness = temp;

		ret = of_property_read_u32(c_np, "timeout", &temp);
		if (ret)
			goto dt_err;
		if (temp > leds_time_max[index])
			temp = leds_time_max[index];
		pdata->leds[index].timeout = temp;

	}
	return 0;
dt_err:
	pr_err("%s failed to get a timeout\n", __func__);
	return ret;
}
#endif /* CONFIG_OF */

static int s2mu003_led_probe(struct platform_device *pdev)
{
	int ret = 0, i = 0;

	struct s2mu003_mfd_chip *s2mu003;
	struct s2mu003_mfd_platform_data *s2mu003_pdata;
	struct s2mu003_fled_platform_data *pdata;
	struct s2mu003_led_data *led_data;
	struct s2mu003_led *data;
	struct s2mu003_led_data **led_datas;

	pr_info("[%s] s2mu003_fled start\n", __func__);

	s2mu003 = dev_get_drvdata(pdev->dev.parent);

	if (!s2mu003) {
		dev_err(&pdev->dev, "drvdata->dev.parent not supplied\n");
		return -ENODEV;
	}

	s2mu003_pdata = s2mu003->pdata;

	if (!s2mu003_pdata) {
		dev_err(&pdev->dev, "platform data not supplied\n");
		return -ENODEV;
	}
#ifdef CONFIG_OF

	pdata = kzalloc(sizeof(struct s2mu003_fled_platform_data), GFP_KERNEL);
	if (!pdata) {
		pr_err("%s: failed to allocate driver data\n", __func__);
		return -ENOMEM;
	}

	if (s2mu003->dev->of_node) {
		ret = s2mu003_led_dt_parse_pdata(s2mu003, pdata);
		if (ret < 0) {
			pr_err("s2mu003-leds : %s not found leds dt! ret[%d]\n",
				__func__, ret);
			kfree(pdata);
			return -1;
		}
	}
#else
	pdata = s2mu003_pdata->fled_platform_data;
	if (!pdata) {
		pr_err("[%s] no platform data for this led is found\n",
				__func__);
		return -EFAULT;
	}
#endif

	led_datas = devm_kzalloc(s2mu003->dev,
			sizeof(struct s2mu003_led_data *) *
			S2MU003_LED_MAX, GFP_KERNEL);
	if (!led_datas) {
		pr_err("[%s] memory allocation error led_datas", __func__);
		kfree(pdata);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, led_datas);

	pr_info("%s %d leds\n", __func__, pdata->num_leds);

	for (i = 0; i != pdata->num_leds; ++i) {
		pr_info("%s led%d setup ...\n", __func__, i);

		data = devm_kzalloc(s2mu003->dev, sizeof(struct s2mu003_led),
				GFP_KERNEL);
		if (!data) {
			pr_err("[%s] memory allocation error data\n",
					__func__);
			ret = -ENOMEM;
			continue;
		}

		memcpy(data, &(pdata->leds[i]), sizeof(struct s2mu003_led));
		led_data = devm_kzalloc(&pdev->dev,
				sizeof(struct s2mu003_led_data), GFP_KERNEL);
		global_led_datas[i] = led_data;
		led_datas[i] = led_data;

		if (!led_data) {
			pr_err("[%s] memory allocation error led_data\n",
					__func__);
			return -ENOMEM;
			kfree(data);
			continue;
		}

		led_data->iodev = s2mu003;
		led_data->i2c = s2mu003->i2c_client;
		led_data->data = data;
		led_data->cdev.name = data->name;
		led_data->cdev.brightness_set = s2mu003_led_set;
		led_data->cdev.flags = 0;
		led_data->cdev.brightness = data->brightness;
		led_data->cdev.max_brightness = led_data->data->id ?
		S2MU003_TORCH_OUT_I_400MA : S2MU003_FLASH_OUT_I_900MA;

		mutex_init(&led_data->lock);
		spin_lock_init(&led_data->value_lock);
		INIT_WORK(&led_data->work, s2mu003_led_work);
		ret = led_classdev_register(&pdev->dev, &led_data->cdev);
		if (ret < 0) {
			pr_err("unable to register LED\n");
			cancel_work_sync(&led_data->work);
			mutex_destroy(&led_data->lock);
			kfree(data);
			kfree(led_data);
			global_led_datas[i] = NULL;
			led_datas[i] = NULL;
			ret = -EFAULT;
			continue;
		}
		if (led_data->data->id == S2MU003_TORCH_LED) {
			s2mu003_dev = device_create(camera_class, NULL, 3, NULL, "flash");
			if (IS_ERR(s2mu003_dev)) {
				pr_err("Failed to create device(flash)!\n");
			}
			ret = device_create_file(s2mu003_dev,
					&dev_attr_rear_flash);
			if (ret < 0)
				pr_err("%s :unable to create file\n", __func__);
		}
	}
#ifndef CONFIG_S2MU003_LEDS_I2C
	if (gpio_is_valid(pdata->torch_pin) &&
			gpio_is_valid(pdata->flash_pin)) {
		if (ret < 0) {
			pr_err("%s : s2mu003 fled gpio allocation error\n",
								__func__);
		} else {
			led_data->torch_pin = pdata->torch_pin;
			led_data->flash_pin = pdata->torch_pin;
		}
	}
#endif

    pdata->fled_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR_OR_NULL(pdata->fled_pinctrl)) {
		pr_err("%s:%d Getting pinctrl handle failed\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	pdata->gpio_state_active = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(pdata->gpio_state_active)) {
		pr_err("%s:%d Failed to get the active state pinctrl handle\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	pdata->gpio_state_suspend = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(pdata->gpio_state_suspend)) {
		pr_err("%s:%d Failed to get the active state pinctrl handle\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	ret = pinctrl_select_state(pdata->fled_pinctrl, pdata->gpio_state_suspend);
	if (ret) {
		pr_err("%s:%d cannot set pin to active state", __func__, __LINE__);
		return -EINVAL;
	}

#ifdef CONFIG_MUIC_NOTIFIER
	muic_notifier_register(&led_data->batt_nb,
			ta_notification,
			MUIC_NOTIFY_DEV_CHARGER);
#endif
	ret = s2mu003_led_setup(led_data);
	if (ret < 0)
		goto err;

	return 0;
err:
	pr_err("%s : failed s2mu003 led reg init\n", __func__);
	return ret;
}

static int s2mu003_led_remove(struct platform_device *pdev)
{
	struct s2mu003_led_data **led_datas = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i != S2MU003_LED_MAX; ++i) {
		if (led_datas[i] == NULL)
			continue;

		if (led_datas[i]->data->id == S2MU003_TORCH_LED)
			device_remove_file(led_datas[i]->cdev.dev,
					&dev_attr_rear_flash);
		cancel_work_sync(&led_datas[i]->work);
		mutex_destroy(&led_datas[i]->lock);
		led_classdev_unregister(&led_datas[i]->cdev);
		kfree(led_datas[i]->data);
		kfree(led_datas[i]);
		kfree(global_led_datas[i]);
	}
	kfree(led_datas);

	return 0;
}

static struct of_device_id s2mu003_led_match_table[] = {
	{ .compatible = "samsung,s2mu003-leds",},
	{},
};

static struct platform_driver s2mu003_led_driver = {
	.probe  = s2mu003_led_probe,
	.remove = s2mu003_led_remove,	
	.driver = {
		.name  = "s2mu003-leds",
		.owner = THIS_MODULE,
		.of_match_table = s2mu003_led_match_table,
		},
};

static int __init s2mu003_led_driver_init(void)
{
	return platform_driver_register(&s2mu003_led_driver);
}
module_init(s2mu003_led_driver_init);

static void __exit s2mu003_led_driver_exit(void)
{
	platform_driver_unregister(&s2mu003_led_driver);
}
module_exit(s2mu003_led_driver_exit);

MODULE_AUTHOR("SUJI LEE <suji0908.lee@samsung.com>");
MODULE_DESCRIPTION("SAMSUNG s2mu003 LED Driver");
MODULE_LICENSE("GPL");
