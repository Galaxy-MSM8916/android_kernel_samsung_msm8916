/*
 * leds-sm5701.c
 *
 * Copyright (c) 2014 Silicon Mitus Co., Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/mfd/sm5701_core.h>
#include <linux/of_gpio.h>
//#include <linux/platform_data/leds-sm5701.h>
#include <linux/kernel.h>
#include <linux/leds/rtfled.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/pinctrl/consumer.h>
#include "../pinctrl/core.h"
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/machine.h>

#define FLED_PINCTRL_STATE_DEFAULT "fled_default"
#define FLED_PINCTRL_STATE_SLEEP "fled_sleep"

enum sm5701_oper_mode {
        SUSPEND_MODE = 0,
        CHARGING_OFF_MODE,
        CHARGING_ON_MODE,
        CHARGING_FLASH_BOOST_MODE
};

typedef struct SM5701_fled_platform_data {
        struct pinctrl *fled_pinctrl;
        struct pinctrl_state *gpio_state_active;
        struct pinctrl_state *gpio_state_suspend;
} SM5701_fled_platform_data_t;

struct SM5701_leds_data {
        struct device *dev;
        struct SM5701_dev *iodev;
        const SM5701_fled_platform_data_t *pdata;

        struct led_classdev cdev_flash;
        struct led_classdev cdev_movie;

        struct work_struct work_flash;
        struct work_struct work_movie;

        u8 br_flash; //IFLED Current in Flash Mode
        u8 br_movie; //IMLED Current in Movie Mode

        //struct sm5701_platform_data *pdata;

        struct mutex lock;
};

static SM5701_fled_platform_data_t sm5701_default_fled_pdata;
static struct i2c_client * leds_sm5701_client = NULL;

extern struct class *camera_class;

#define ENABSTMR_SHIFT  4
void sm5701_set_enabstmr(int abstmr_enable)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s abstmr_enable = %d\n",__func__,abstmr_enable);

    SM5701_reg_read(client, SM5701_FLEDCNTL1, &data);

    data = (data & (~SM5701_FLEDCNTL1_ENABSTMR)) | (abstmr_enable << ENABSTMR_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL1,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL1, data);
    
}
EXPORT_SYMBOL(sm5701_set_enabstmr);

#define ABSTMR_SHIFT  2
void sm5701_set_abstmr(int abstmr_sec)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s abstmr_sec = %d\n",__func__,abstmr_sec);

    SM5701_reg_read(client, SM5701_FLEDCNTL1, &data);

    data = (data & (~SM5701_FLEDCNTL1_ABSTMR)) | (abstmr_sec << ABSTMR_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL1,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL1, data);
    
}
EXPORT_SYMBOL(sm5701_set_abstmr);

void sm5701_set_fleden(int fled_enable)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s fled_enable = %d\n",__func__,fled_enable);

    SM5701_reg_read(client, SM5701_FLEDCNTL1, &data);
    
    data = (data & (~SM5701_FLEDCNTL1_FLEDEN)) | fled_enable;

    SM5701_reg_write(client,SM5701_FLEDCNTL1,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL1, data);
    
}
EXPORT_SYMBOL(sm5701_set_fleden);

#define nENSAFET_SHIFT  7
void sm5701_set_nensafet(int nensafet_enable)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s nensafet_enable = %d\n",__func__,nensafet_enable);

    SM5701_reg_read(client, SM5701_FLEDCNTL2, &data);

    data = (data & (~SM5701_FLEDCNTL2_nENSAFET)) | (nensafet_enable << nENSAFET_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL2,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL2, data);
    
}
EXPORT_SYMBOL(sm5701_set_nensafet);

#define SAFET_SHIFT  5
void sm5701_set_safet(int safet_us)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s safet_us = %d\n",__func__,safet_us);

    SM5701_reg_read(client, SM5701_FLEDCNTL2, &data);

    data = (data & (~SM5701_FLEDCNTL2_SAFET)) | (safet_us << SAFET_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL2,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL2, data);
    
}
EXPORT_SYMBOL(sm5701_set_safet);

#define nONESHOT_SHIFT  4
void sm5701_set_noneshot(int noneshot_enable)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s noneshot_enable = %d\n",__func__,noneshot_enable);

    SM5701_reg_read(client, SM5701_FLEDCNTL2, &data);

    data = (data & (~SM5701_FLEDCNTL2_nONESHOT)) | (noneshot_enable << nONESHOT_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL2,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL2, data);
    
}
EXPORT_SYMBOL(sm5701_set_noneshot);

void sm5701_set_onetimer(int onetimer_ms)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s onetimer_ms = %d\n",__func__,onetimer_ms);

    SM5701_reg_read(client, SM5701_FLEDCNTL2, &data);

    data = (data & (~SM5701_FLEDCNTL2_ONETIMER)) | onetimer_ms;

    SM5701_reg_write(client,SM5701_FLEDCNTL2,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL2, data);
    
}
EXPORT_SYMBOL(sm5701_set_onetimer);

#define IFLED_MAX   0x1F
#define IFLED_MIN   0x0
void sm5701_set_ifled(int ifled_ma)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s ifled_ma = %d\n",__func__,ifled_ma);

    if(ifled_ma < IFLED_MIN)
        ifled_ma = IFLED_MIN;
    else if (ifled_ma > IFLED_MAX)
        ifled_ma = IFLED_MAX;
    
    SM5701_reg_read(client, SM5701_FLEDCNTL3, &data);

    data = (data & (~SM5701_FLEDCNTL3_IFLED)) | ifled_ma;

    SM5701_reg_write(client,SM5701_FLEDCNTL3,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL3, data);
    
}
EXPORT_SYMBOL(sm5701_set_ifled);

#define IMLED_MAX   0x1F
#define IMLED_MIN   0x0
void sm5701_set_imled(int imled_ma)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s imled_ma = %d\n",__func__,imled_ma);

    if(imled_ma < IMLED_MIN) 
        imled_ma = IMLED_MIN;
    else if(imled_ma > IMLED_MAX) 
        imled_ma = IMLED_MAX;

    SM5701_reg_read(client, SM5701_FLEDCNTL4, &data);

    data = (data & (~SM5701_FLEDCNTL4_IMLED)) | imled_ma;

    SM5701_reg_write(client,SM5701_FLEDCNTL4,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL4, data);
    
}
EXPORT_SYMBOL(sm5701_set_imled);

#define ENLOWBATT_SHIFT  7
void sm5701_set_enlowbatt(int enlowbatt_enable)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s enlowbatt_enable = %d\n",__func__,enlowbatt_enable);

    SM5701_reg_read(client, SM5701_FLEDCNTL5, &data);

    data = (data & (~SM5701_FLEDCNTL5_ENLOWBATT)) | (enlowbatt_enable << ENLOWBATT_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL5,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL5, data);
    
}
EXPORT_SYMBOL(sm5701_set_enlowbatt);

#define LBRSTIMER_SHIFT  5
void sm5701_set_lbrstimer(int lbrstimer_us)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s lbrstimer_us = %d\n",__func__,lbrstimer_us);

    SM5701_reg_read(client, SM5701_FLEDCNTL5, &data);

    data = (data & (~SM5701_FLEDCNTL5_LBRSTIMER)) | (lbrstimer_us << LBRSTIMER_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL5,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL5, data);
    
}
EXPORT_SYMBOL(sm5701_set_lbrstimer);

#define LOWBATT_SHIFT  2
void sm5701_set_lowbatt(int lowbatt_v)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s lowbatt_v = %d\n",__func__,lowbatt_v);

    SM5701_reg_read(client, SM5701_FLEDCNTL5, &data);

    data = (data & (~SM5701_FLEDCNTL5_LOWBATT)) | (lowbatt_v << LOWBATT_SHIFT);

    SM5701_reg_write(client,SM5701_FLEDCNTL5,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL5, data);
    
}
EXPORT_SYMBOL(sm5701_set_lowbatt);

void sm5701_set_lbdhys(int lbdhys_mv)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s lbdhys_mv = %d\n",__func__,lbdhys_mv);

    SM5701_reg_read(client, SM5701_FLEDCNTL5, &data);

    data = (data & (~SM5701_FLEDCNTL5_LBDHYS))| lbdhys_mv;

    SM5701_reg_write(client,SM5701_FLEDCNTL5,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL5, data);
    
}
EXPORT_SYMBOL(sm5701_set_lbdhys);

void SM5701_set_bstout(int bstout_mv)
{
    struct i2c_client * client;
    u8 data = 0;

    client = leds_sm5701_client;
  
    if( !client) return;

    pr_info("%s bstout_mv = %d\n",__func__,bstout_mv);

    SM5701_reg_read(client, SM5701_FLEDCNTL6, &data);

    data = (data & (~SM5701_FLEDCNTL6_BSTOUT))| bstout_mv;

    SM5701_reg_write(client,SM5701_FLEDCNTL6,data);    
  
    pr_info("write SM5701 addr : 0x%02x data : 0x%02x\n", SM5701_FLEDCNTL6, data);
    
}
EXPORT_SYMBOL(SM5701_set_bstout);

/* brightness control */
static int sm5701_brightness_control(struct SM5701_leds_data *chip,
                          u8 brightness, enum sm5701_flash_mode flash_mode)
{
        switch (flash_mode) {
        case NONE_MODE:
                break;

        case FLASH_MODE:            
                sm5701_set_ifled(brightness);
                break;

        case MOVIE_MODE:
                sm5701_set_imled(brightness);
                break;

        default:
                break;
        }
        
        return flash_mode;
}

/* movie */

/* movie config for sm5701*/
static ssize_t sm5701_movie_store(struct device *dev,
                                      struct device_attribute *attr,
                                      const char *buf, size_t size)
{
        ssize_t ret;
     /*   struct led_classdev *led_cdev = dev_get_drvdata(dev);
        struct SM5701_leds_data *chip =
            container_of(led_cdev, struct SM5701_leds_data, work_movie);*/
        unsigned int state;

        ret = kstrtouint(buf, 10, &state);
        if (ret)
                goto out_strtoint;

        if (state == 0){
                sm5701_led_ready(LED_DISABLE);
                sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
	}
        else if (state == 1) {
                sm5701_led_ready(MOVIE_MODE);
                sm5701_set_fleden(SM5701_FLEDEN_ON_MOVIE);
	}
        else {
                sm5701_led_ready(LED_DISABLE);
                sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
	}

        //sm5701_dump_register();

        return size;

out_strtoint:
        /*dev_err(chip->dev, "%s: fail to change str to int\n", __func__);*/
        return ret;
}

static DEVICE_ATTR(rear_flash, S_IWUSR, NULL, sm5701_movie_store);

static void sm5701_deferred_movie_brightness_set(struct work_struct *work)
{
    struct SM5701_leds_data *chip =
        container_of(work, struct SM5701_leds_data, work_movie);

        mutex_lock(&chip->lock);
        sm5701_brightness_control(chip, chip->br_movie, MOVIE_MODE);
        mutex_unlock(&chip->lock);
}

static void sm5701_movie_brightness_set(struct led_classdev *cdev,
                                        enum led_brightness brightness)
{
        struct SM5701_leds_data *chip =
            container_of(cdev, struct SM5701_leds_data, cdev_movie);
        
        if(brightness >= 0 && brightness <= cdev->max_brightness )
        {
            chip->br_movie = brightness;
            schedule_work(&chip->work_movie);
        }
}

/* flash */

/* flash config for sm5701*/
static ssize_t sm5701_flash_store(struct device *dev,
                                       struct device_attribute *attr,
                                       const char *buf, size_t size)
{
        ssize_t ret;
 /*       struct led_classdev *led_cdev = dev_get_drvdata(dev);
        struct SM5701_leds_data *chip =
            container_of(led_cdev, struct SM5701_leds_data, work_flash);*/
        unsigned int state;

        ret = kstrtouint(buf, 10, &state);
        if (ret)
                goto out_strtoint;

        if (state == 0) {
                sm5701_led_ready(LED_DISABLE);
                sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
	}
        else if (state == 1) {
                sm5701_led_ready(FLASH_MODE);
                sm5701_set_fleden(SM5701_FLEDEN_ON_FLASH);
	}
        else {
                sm5701_led_ready(LED_DISABLE);
                sm5701_set_fleden(SM5701_FLEDEN_DISABLED);
	}

        //sm5701_dump_register();

        return size;

out_strtoint:
        /*dev_err(chip->dev, "%s: fail to change str to int\n", __func__);*/
        return ret;
}

static DEVICE_ATTR(flash, S_IWUSR, NULL, sm5701_flash_store);

static void sm5701_deferred_flash_brightness_set(struct work_struct *work)
{
    struct SM5701_leds_data *chip =
        container_of(work, struct SM5701_leds_data, work_flash);
    
        mutex_lock(&chip->lock);
        sm5701_brightness_control(chip, chip->br_flash, FLASH_MODE);
        mutex_unlock(&chip->lock);
}

static void sm5701_flash_brightness_set(struct led_classdev *cdev,
                                         enum led_brightness brightness)
{
        struct SM5701_leds_data *chip =
            container_of(cdev, struct SM5701_leds_data, cdev_flash);

        if(brightness >= 0 && brightness <= cdev->max_brightness )
        {
            chip->br_flash = brightness;
            schedule_work(&chip->work_flash);
        }
}

/* chip initialize */
static int sm5701_chip_init(struct SM5701_leds_data *chip)
{
        int ret = 0;

        chip->br_movie = 0x0B; //100mA
        sm5701_set_imled(chip->br_movie);


        chip->br_flash = 0x15; //600mA
        sm5701_set_ifled(chip->br_flash);

        //sm5701_dump_register();

        //disable ABSTMR
        sm5701_set_enabstmr(1);
        
        return ret;
}

#define flash_enable_gpio 214
#define flash_torch_gpio 219
static int leds_sm5701_probe(struct platform_device *pdev)
{
        struct SM5701_dev *iodev = dev_get_drvdata(pdev->dev.parent);
        struct SM5701_leds_data *chip;
        struct SM5701_fled_platform_data *pdata;

        int err;

        printk("******* %s *******\n",__func__);
        pdata = &sm5701_default_fled_pdata;

        chip = kzalloc(sizeof(struct SM5701_leds_data), GFP_KERNEL);
        if (!chip)
           return -ENOMEM;

        chip->dev = &pdev->dev;
        chip->iodev = iodev;
        platform_set_drvdata(pdev, chip);
        /*
        if (!(leds_sm5701_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
            return -ENOMEM;
        }
        memset(leds_sm5701_client, 0, sizeof(struct i2c_client));
        */
        leds_sm5701_client = chip->iodev->i2c;

        mutex_init(&chip->lock);

        if (camera_class ==NULL){
                camera_class = class_create(THIS_MODULE, "camera");
                if (IS_ERR(camera_class))
			pr_err("failed to create device cam_dev_rear!\n");
        }
        /* flash */
        INIT_WORK(&chip->work_flash, sm5701_deferred_flash_brightness_set);
        chip->cdev_flash.name = "flash";
        chip->cdev_flash.max_brightness = 32-1;//0x1f
        chip->cdev_flash.brightness_set = sm5701_flash_brightness_set;
        chip->cdev_flash.default_trigger = "flash";
        err = led_classdev_register((struct device *)
                                    chip->dev, &chip->cdev_flash);

        if (err < 0) {
                dev_err(chip->dev, "failed to register flash\n");
                goto err_create_flash_file;
        }
        err = device_create_file(chip->cdev_flash.dev, &dev_attr_flash);
        if (err < 0) {
                dev_err(chip->dev, "failed to create flash file\n");
                goto err_create_flash_pin_file;
        }

	/* movie */
	INIT_WORK(&chip->work_movie, sm5701_deferred_movie_brightness_set);
	chip->cdev_movie.name = "flash";
	chip->cdev_movie.max_brightness = 32-1;//0x1f
	chip->cdev_movie.brightness_set = sm5701_movie_brightness_set;
	chip->cdev_movie.default_trigger = "flash";

        chip->cdev_movie.dev = device_create(camera_class, NULL, 0, NULL, "flash");
        if (IS_ERR(chip->cdev_movie.dev)) {
                pr_err("flash_sysfs: failed to create device(flash)\n");
                goto err_create_movie_file;
        }
        err = device_create_file(chip->cdev_movie.dev, &dev_attr_rear_flash);
        if (err < 0) {
                dev_err(chip->dev, "failed to create movie file\n");
                goto err_create_movie_pin_file;
        }

        err = sm5701_chip_init(chip);
        if (err < 0)
                goto err_out;
    #if  defined (CONFIG_MACH_VIVALTO5MVE3G) || defined(CONFIG_MACH_VIVALTO3MVE3G_LTN)
        if (!gpio_is_valid(flash_enable_gpio))
        {
              printk("flash_enable_gpio gpio pin error");
               return 1;
        }
        gpio_request(flash_enable_gpio, "gpioFlashhigh");
        gpio_direction_output(flash_enable_gpio,0);

        if (!gpio_is_valid(flash_torch_gpio)) 
        {
              printk("flash_torch_gpio gpio pin error");
               return 1;
        }
        gpio_request(flash_torch_gpio, "gpioFlashlow");
        gpio_direction_output(flash_torch_gpio,0);
    #endif
        //sm5701_dump_register();
        pdata->fled_pinctrl = devm_pinctrl_get(&pdev->dev);
        if (IS_ERR_OR_NULL(pdata->fled_pinctrl)) {
              pr_err("%s:%d Getting pinctrl handle failed\n", __func__, __LINE__);
              return -EINVAL;
        }

        pdata->gpio_state_active = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_DEFAULT);
        if (IS_ERR_OR_NULL(pdata->gpio_state_active)) {
              pr_err("%s:%d Failed to get the active state pinctrl handle\n", __func__, __LINE__);
              return -EINVAL;
        }

        pdata->gpio_state_suspend = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_SLEEP);
        if (IS_ERR_OR_NULL(pdata->gpio_state_suspend)) {
              pr_err("%s:%d Failed to get the active state pinctrl handle\n", __func__, __LINE__);
              return -EINVAL;
        }

        err = pinctrl_select_state(pdata->fled_pinctrl, pdata->gpio_state_suspend);
        if (err) {
              pr_err("%s:%d cannot set pin to active state", __func__, __LINE__);
              return err;
        }
        pr_err("%s End : X\n", __func__);

        dev_info(chip->dev, "LEDs_SM5701 Probe Done\n");
        return 0;

err_create_movie_file:
        device_remove_file(chip->cdev_movie.dev, &dev_attr_rear_flash);
err_create_movie_pin_file:
        led_classdev_unregister(&chip->cdev_movie);
err_create_flash_file:
        device_remove_file(chip->cdev_flash.dev, &dev_attr_flash);
err_create_flash_pin_file:
        led_classdev_unregister(&chip->cdev_flash);
err_out:
        return err;
}

static int leds_sm5701_remove(struct platform_device *pdev)
{
        struct SM5701_leds_data *chip = platform_get_drvdata(pdev);

        device_remove_file(chip->cdev_movie.dev, &dev_attr_rear_flash);
        led_classdev_unregister(&chip->cdev_movie);
        flush_work(&chip->work_movie);
        device_remove_file(chip->cdev_flash.dev, &dev_attr_flash);
        led_classdev_unregister(&chip->cdev_flash);
        flush_work(&chip->work_flash);
        return 0;
}

static const struct platform_device_id leds_sm5701_id[] = {
        {"leds_sm5701", 0},
        {}
};

MODULE_DEVICE_TABLE(platform, leds_sm5701_id);

#ifdef CONFIG_OF
static struct of_device_id leds_sm5701_match_table[] = {
	{ .compatible = "sm,leds_sm5701",},
	{},
};
#else
#define SM5701_match_table NULL
#endif

static struct platform_driver leds_sm5701_driver = {
        .driver = {
                   .name = "leds_sm5701",
                   .owner = THIS_MODULE,
                   .of_match_table = leds_sm5701_match_table,
                   },
        .probe = leds_sm5701_probe,
        .remove = leds_sm5701_remove,
        .id_table = leds_sm5701_id,
};

static int __init leds_sm5701_init(void)
{
    printk("******* %s *******\n",__func__);

	return platform_driver_register(&leds_sm5701_driver);
}

subsys_initcall(leds_sm5701_init);

static void __exit leds_sm5701_exit(void)
{
	platform_driver_unregister(&leds_sm5701_driver);
}
module_exit(leds_sm5701_exit);

/* Module information */
MODULE_DESCRIPTION("SILICONMITUS SM5701 Flash LED Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-sm5701");

