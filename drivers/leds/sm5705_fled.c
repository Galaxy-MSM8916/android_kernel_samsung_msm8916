/*
 * Flash-LED device driver for SM5705
 *
 * Copyright (C) 2015 Silicon Mitus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/workqueue.h>
#include <linux/leds-sm5705.h>
#include <linux/mfd/sm5705/sm5705.h>
#include <linux/muic/muic_afc.h>
#include <linux/battery/charger/sm5705_charger_oper.h>
#if defined(CONFIG_MUIC_UNIVERSAL_SM5705_AFC)
#define DISABLE_AFC
#endif

enum {
    SM5705_FLED_OFF_MODE                    = 0x0,
    SM5705_FLED_ON_MOVIE_MODE               = 0x1,
    SM5705_FLED_ON_FLASH_MODE               = 0x2,
    SM5705_FLED_ON_EXTERNAL_CONTROL_MODE    = 0x3,
};

struct sm5705_fled_info {
    struct device *dev;
    struct i2c_client *i2c;

    struct sm5705_fled_platform_data *pdata;
    struct device *rear_fled_dev;

    /* for Flash VBUS check */
    struct workqueue_struct *wqueue;
    struct delayed_work fled0_vbus_check_work;
    struct delayed_work fled1_vbus_check_work;
};

extern struct class *camera_class; /*sys/class/camera*/
bool assistive_light = false;
#ifdef DISABLE_AFC
static DEFINE_MUTEX(lock);
#else
static DEFINE_SPINLOCK(fled_lock);
#endif
static struct sm5705_fled_info *g_sm5705_fled;

static inline int __get_revision_number(void)
{
    return 2;
}

/**
 * SM5705 Flash-LEDs device register control functions
 */

static int sm5705_FLEDx_mode_enable(struct sm5705_fled_info *sm5705_fled, int index, unsigned char FLEDxEN)
{
    int ret;

    ret = sm5705_update_reg(sm5705_fled->i2c, SM5705_REG_FLED1CNTL1 + (index * 4), (FLEDxEN & 0x3), 0x3);
    if (IS_ERR_VALUE(ret)) {
        dev_err(sm5705_fled->dev, "%s: fail to update REG:FLED%dEN (value=%d)\n", __func__, index, FLEDxEN);
        return ret;
    }

    dev_info(sm5705_fled->dev, "%s: FLED[%d] set mode = %d\n", __func__, index, FLEDxEN);

    return 0;
}

#if 0
static inline unsigned char _calc_oneshot_time_offset_to_ms(unsigned short ms)
{
    if (ms < 100) {
        return 0;
    } else {
        return (((ms - 100) / 100) & 0xF);
    }
}

static int sm5705_FLEDx_oneshot_config(struct sm5705_fled_info *sm5705_fled, int index, bool enable, unsigned short timer_ms)
{
    int ret;
    unsigned char reg_val;

    reg_val = (((!enable) & 0x1) << 4) | (_calc_oneshot_time_offset_to_ms(timer_ms) & 0xF);
    ret = sm5705_write_reg(sm5705_fled->i2c, SM5705_REG_FLED1CNTL2 + (index * 4), reg_val);
    if (IS_ERR_VALUE(ret)) {
        dev_err(sm5705_fled->dev, "%s: fail to write REG:FLED%dCNTL2 (value=%d)\n", __func__, index, reg_val);
        return ret;
    }

    return 0;
}
#endif

static inline unsigned char _calc_flash_current_offset_to_mA(unsigned short current_mA)
{
    return current_mA < 700 ? (((current_mA - 300) / 25) & 0x1F) : ((((current_mA - 700) / 50) + 0xF) & 0x1F);
}

static int sm5705_FLEDx_set_flash_current(struct sm5705_fled_info *sm5705_fled, int index, unsigned short current_mA)
{
    int ret;
    unsigned char reg_val;

    reg_val = _calc_flash_current_offset_to_mA(current_mA);
    ret = sm5705_write_reg(sm5705_fled->i2c, SM5705_REG_FLED1CNTL3 + (index * 4), reg_val);
    if (IS_ERR_VALUE(ret)) {
        dev_err(sm5705_fled->dev, "%s: fail to write REG:FLED%dCNTL3 (value=%d)\n", __func__, index, reg_val);
        return ret;
    }

    return 0;
}

static inline unsigned char _calc_torch_current_offset_to_mA(unsigned short current_mA)
{
    return (((current_mA - 10) / 10) & 0x1F);
}

static inline unsigned short _calc_torch_current_mA_to_offset(unsigned char offset)
{
    return (((offset & 0x1F) + 1) * 10);
}

static int sm5705_FLEDx_set_torch_current(struct sm5705_fled_info *sm5705_fled, int index, unsigned short current_mA)
{
    int ret;
    unsigned char reg_val;

    reg_val = _calc_torch_current_offset_to_mA(current_mA);
    ret = sm5705_write_reg(sm5705_fled->i2c, SM5705_REG_FLED1CNTL4 + (index * 4), reg_val);
    if (IS_ERR_VALUE(ret)) {
        dev_err(sm5705_fled->dev, "%s: fail to write REG:FLED%dCNTL4 (value=%d)\n", __func__, index, reg_val);
        return ret;
    }

    return 0;
}


//#define MUIC_DEF
/**
 * SM5705 Flash-LED to MUIC interface functions
 */
static inline int sm5705_fled_muic_flash_work_on(struct sm5705_fled_info *sm5705_fled)
{
#ifdef MUIC_DEF
    /* MUIC 9V -> 5V function */
    muic_check_afc_state(1);
    //sm5705_muic_DP_RESET();
#endif
    return 0;
}

static inline int sm5705_fled_muic_flash_work_off(struct sm5705_fled_info *sm5705_fled)
{
#ifdef MUIC_DEF
    /* MUIC 5V -> 9V function */
    muic_check_afc_state(0);
    //sm5705_muic_AFC_restart();
#endif
    return 0;
}

static inline bool sm5705_fled_check_valid_vbus_from_MUIC(void)
{
#ifdef MUIC_DEF
    if (muic_check_afc_state(1) == 1) {
        return true;
    }
    return false;
#else
    return true;
#endif
}

/**
 * SM5705 Flash-LED operation control functions
 */
static int sm5705_fled_initialize(struct sm5705_fled_info *sm5705_fled)
{
    struct device *dev = sm5705_fled->dev;
    struct sm5705_fled_platform_data *pdata = sm5705_fled->pdata;
    int i, ret;

    for (i=0; i < SM5705_FLED_MAX; ++i) {
        if (pdata->led[i].used_gpio) {
            ret = gpio_request(pdata->led[i].flash_en_pin, "sm5705_fled");
            if (IS_ERR_VALUE(ret)) {
                dev_err(dev, "%s: fail to request flash gpio pin = %d (ret=%d)\n", __func__, pdata->led[i].flash_en_pin, ret);
                return ret;
            }
            gpio_direction_output(pdata->led[i].flash_en_pin, 0);

            ret = gpio_request(pdata->led[i].torch_en_pin, "sm5705_fled");
            if (IS_ERR_VALUE(ret)) {
                dev_err(dev, "%s: fail to request torch gpio pin = %d (ret=%d)\n", __func__, pdata->led[i].torch_en_pin, ret);
                return ret;
            }
            gpio_direction_output(pdata->led[i].torch_en_pin, 0);

            dev_info(dev, "SM5705 FLED[%d] used External GPIO control Mode (Flash pin=%d, Torch pin=%d)\n",
                i, pdata->led[i].flash_en_pin, pdata->led[i].torch_en_pin);
        } else {
            dev_info(dev, "SM5705 FLED[%d] used I2C control Mode\n", i);
        }
        ret = sm5705_FLEDx_mode_enable(sm5705_fled, i, SM5705_FLED_OFF_MODE);
        if (IS_ERR_VALUE(ret)) {
            dev_err(dev, "%s: fail to set FLED[%d] external control mode\n", __func__, i);
            return ret;
        }
    }

    return 0;
}

static void sm5705_fled_deinitialize(struct sm5705_fled_info *sm5705_fled)
{
    struct device *dev = sm5705_fled->dev;
    struct sm5705_fled_platform_data *pdata = sm5705_fled->pdata;
    int i;

    for (i=0; i < SM5705_FLED_MAX; ++i) {
        if (pdata->led[i].used_gpio) {
            gpio_free(pdata->led[i].flash_en_pin);
            gpio_free(pdata->led[i].torch_en_pin);
        }
        sm5705_FLEDx_mode_enable(sm5705_fled, i, SM5705_FLED_OFF_MODE);
    }

    dev_info(dev, "%s: FLEDs de-initialize done.\n", __func__);
}

static inline int _fled_turn_on_torch(struct sm5705_fled_info *sm5705_fled, int index)
{
    struct sm5705_fled_platform_data *pdata = sm5705_fled->pdata;
    struct device *dev = sm5705_fled->dev;
    int ret;

    if (pdata->led[index].used_gpio) {
        ret = sm5705_FLEDx_mode_enable(sm5705_fled, index, SM5705_FLED_ON_EXTERNAL_CONTROL_MODE);
        if (IS_ERR_VALUE(ret)) {
            dev_err(dev, "%s: fail to set FLED[%d] External control mode\n", __func__, index);
            return ret;
        }
        gpio_set_value(pdata->led[index].flash_en_pin, 0);
        gpio_set_value(pdata->led[index].torch_en_pin, 1);
    } else {
        ret = sm5705_FLEDx_mode_enable(sm5705_fled, index, SM5705_FLED_ON_MOVIE_MODE);
        if (IS_ERR_VALUE(ret)) {
            dev_err(dev, "%s: fail to set FLED[%d] Movie mode\n", __func__, index);
            return ret;
        }
    }
    sm5705_charger_oper_push_event(SM5705_CHARGER_OP_EVENT_TORCH, 1);

    dev_info(dev, "%s: FLED[%d] Torch turn-on done.\n", __func__, index);

    return 0;
}

static int sm5705_fled_turn_on_torch(struct sm5705_fled_info *sm5705_fled, int index, unsigned short current_mA)
{
    struct device *dev = sm5705_fled->dev;
    int ret;

    sm5705_fled_muic_flash_work_on(sm5705_fled);

    ret = sm5705_FLEDx_set_torch_current(sm5705_fled, index, current_mA);
    if (IS_ERR_VALUE(ret)) {
        sm5705_fled_muic_flash_work_off(sm5705_fled);
        dev_err(dev, "%s: fail to set FLED[%d] torch current (current_mA=%d)\n", __func__, index, current_mA);
        return ret;
    }

    if (sm5705_fled_check_valid_vbus_from_MUIC() == true) {
        _fled_turn_on_torch(sm5705_fled, index);
    } else {
        if (index == SM5705_FLED_0) {
            queue_delayed_work(sm5705_fled->wqueue, &sm5705_fled->fled0_vbus_check_work, msecs_to_jiffies(10));
        } else {
            queue_delayed_work(sm5705_fled->wqueue, &sm5705_fled->fled1_vbus_check_work, msecs_to_jiffies(10));
        }
    }

    return 0;
}

static int sm5705_fled_turn_on_flash(struct sm5705_fled_info *sm5705_fled, int index, unsigned short current_mA)
{
    struct device *dev = sm5705_fled->dev;
    struct sm5705_fled_platform_data *pdata = sm5705_fled->pdata;
    int ret;

    ret = sm5705_FLEDx_set_flash_current(sm5705_fled, index, current_mA);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to set FLED[%d] flash current (current_mA=%d)\n", __func__, index, current_mA);
        return ret;
    }

    if (pdata->led[index].used_gpio) {
        ret = sm5705_FLEDx_mode_enable(sm5705_fled, index, SM5705_FLED_ON_EXTERNAL_CONTROL_MODE);
        if (IS_ERR_VALUE(ret)) {
            dev_err(dev, "%s: fail to set FLED[%d] External control mode\n", __func__, index);
            return ret;
        }
        gpio_set_value(pdata->led[index].torch_en_pin, 0);
        gpio_set_value(pdata->led[index].flash_en_pin, 1);
    } else {
        ret = sm5705_FLEDx_mode_enable(sm5705_fled, index, SM5705_FLED_ON_FLASH_MODE);
        if (IS_ERR_VALUE(ret)) {
            dev_err(dev, "%s: fail to set FLED[%d] Flash mode\n", __func__, index);
            return ret;
        }
    }
    sm5705_charger_oper_push_event(SM5705_CHARGER_OP_EVENT_FLASH, 1);

    dev_info(dev, "%s: FLED[%d] Flash turn-on done.\n", __func__, index);

    return 0;
}

static int sm5705_fled_turn_off(struct sm5705_fled_info *sm5705_fled, int index)
{
    struct device *dev = sm5705_fled->dev;
    struct sm5705_fled_platform_data *pdata = sm5705_fled->pdata;
    int ret;

    if (pdata->led[index].used_gpio) {
        gpio_set_value(pdata->led[index].flash_en_pin, 0);
        gpio_set_value(pdata->led[index].torch_en_pin, 0);
    }

    ret = sm5705_FLEDx_mode_enable(sm5705_fled, index, SM5705_FLED_OFF_MODE);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to set FLED[%d] OFF mode\n", __func__, index);
        return ret;
    }

    sm5705_fled_muic_flash_work_off(sm5705_fled);

    ret = sm5705_FLEDx_set_flash_current(sm5705_fled, index, 0);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to set FLED[%d] flash current\n", __func__, index);
        return ret;
    }

    ret = sm5705_FLEDx_set_torch_current(sm5705_fled, index, 0);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to set FLED[%d] torch current\n", __func__, index);
        return ret;
    }

    sm5705_charger_oper_push_event(SM5705_CHARGER_OP_EVENT_FLASH, 0);
    sm5705_charger_oper_push_event(SM5705_CHARGER_OP_EVENT_TORCH, 0);

    dev_info(dev, "%s: FLED[%d] turn-off done.\n", __func__, index);

    return 0;
}

/**
 *  For Export Flash control functions (external GPIO control)
 */
static bool fimc_is_activated = 0;

int sm5705_fled_prepare_flash(unsigned char index)
{
    if (fimc_is_activated == 1) {
        /* skip to overlapping function calls */
        return 0;
    }

    dev_info(g_sm5705_fled->dev, "%s: check - GPIO used, set - Torch/Flash current\n", __func__);

    if (g_sm5705_fled == NULL) {
        pr_err("sm5705-fled: %s: invalid g_sm5705_fled, maybe not registed fled device driver\n", __func__);
        return -ENXIO;
    }

    if (g_sm5705_fled->pdata->led[index].used_gpio == 0) {
        pr_err("sm5705-fled: %s: can't used external GPIO control, check device tree\n", __func__);
        return -ENOENT;
    }

    sm5705_fled_muic_flash_work_on(g_sm5705_fled);

    sm5705_FLEDx_set_torch_current(g_sm5705_fled, index, g_sm5705_fled->pdata->led[index].torch_current_mA);
    sm5705_FLEDx_set_flash_current(g_sm5705_fled, index, g_sm5705_fled->pdata->led[index].flash_current_mA);

    fimc_is_activated = 1;

    return 0;
}
EXPORT_SYMBOL(sm5705_fled_prepare_flash);

int sm5705_fled_torch_on(unsigned char index)
{
    if (assistive_light == false) {
        dev_info(g_sm5705_fled->dev, "%s: Torch - ON\n", __func__);
        sm5705_fled_turn_on_torch(g_sm5705_fled, index,g_sm5705_fled->pdata->led[index].torch_current_mA);
    }

    return 0;
}
EXPORT_SYMBOL(sm5705_fled_torch_on);

int sm5705_fled_flash_on(unsigned char index)
{

    if (assistive_light == false) {
        /* CAUTION: SM5705 Flash-LED can't driving 9V_VBUS, MUST be "VBUS < 6V" */
        if (sm5705_fled_check_valid_vbus_from_MUIC() == false) {
            dev_err(g_sm5705_fled->dev, "%s: Can't used Flash_dev, now VBUS=9V\n", __func__);
            return -EBUSY;
        }

        dev_info(g_sm5705_fled->dev, "%s: Flash - ON\n", __func__);
        sm5705_fled_turn_on_flash(g_sm5705_fled, index, g_sm5705_fled->pdata->led[index].flash_current_mA);
    }

    return 0;
}
EXPORT_SYMBOL(sm5705_fled_flash_on);

int sm5705_fled_led_off(unsigned char index)
{
    /* zero2lte - test temp code */
    if (assistive_light == false) {
        dev_info(g_sm5705_fled->dev, "%s: LED - OFF\n", __func__);
        sm5705_fled_turn_off(g_sm5705_fled, index);
    }

    return 0;
}
EXPORT_SYMBOL(sm5705_fled_led_off);

int sm5705_fled_close_flash(unsigned char index)
{
    if (fimc_is_activated == 0) {
        /* skip to overlapping function calls */
        return 0;
    }

    dev_info(g_sm5705_fled->dev, "%s: Close Process\n", __func__);

    if (g_sm5705_fled == NULL) {
        pr_err("sm5705-fled: %s: invalid g_sm5705_fled, maybe not registed fled device driver\n", __func__);
        return -ENXIO;
    }

    sm5705_fled_muic_flash_work_off(g_sm5705_fled);

    fimc_is_activated = 0;

    return 0;
}
EXPORT_SYMBOL(sm5705_fled_close_flash);

/**
 * For Camera-class Rear Flash device file support functions
 */
#define REAR_FLASH_INDEX    SM5705_FLED_0

static ssize_t sm5705_rear_flash_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct sm5705_fled_info *sm5705_fled = dev_get_drvdata(dev->parent);
#ifdef DISABLE_AFC
    int retry;
#else
    unsigned long flags;
#endif
    int ret, value_u32;

    if ((buf == NULL) || kstrtouint(buf, 10, &value_u32)) {
        return -1;
    }

    /** temp error canceling code */
    if (sm5705_fled != g_sm5705_fled) {
        dev_info(dev, "%s: sm5705_fled handler mismatched (g_handle:%p , l_handle:%p)\n", __func__, g_sm5705_fled, sm5705_fled);
        sm5705_fled = g_sm5705_fled;
    }

    dev_info(dev, "%s: value=%d\n", __func__, value_u32);
#ifdef DISABLE_AFC
    mutex_lock(&lock);
#else
    spin_lock_irqsave(&fled_lock, flags);
#endif

    switch (value_u32) {
    case 0:
#ifdef DISABLE_AFC
        /* MUIC 5V -> 9V function */
       muic_torch_prepare(0);
#endif
        /* Turn off Torch */
        ret = sm5705_fled_turn_off(sm5705_fled, REAR_FLASH_INDEX);
        assistive_light = false;
        break;
    case 1:
           /* Turn on Torch */
#ifdef DISABLE_AFC
	for (retry = 0; retry < 3; retry++) {
		if(muic_torch_prepare(1) == 1)
			break;

		pr_err("%s:%d ERROR: AFC disable unsuccessfull retrying after 30ms\n", __func__, __LINE__);
		msleep(30);
	}

	if (retry == 3) {
		pr_err("%s:%d ERROR: AFC disable failed\n", __func__, __LINE__);
		mutex_unlock(&lock);
		return -1;
	}
        /* MUIC 9V -> 5V function */
#endif
        ret = sm5705_fled_turn_on_torch(sm5705_fled, REAR_FLASH_INDEX, 60);
        assistive_light = true;
        break;
    case 100:
          /* Factory mode Turn on Torch */
 #ifdef DISABLE_AFC
	for (retry = 0; retry < 3; retry++) {
		if(muic_torch_prepare(1) == 1)
			break;

		pr_err("%s:%d ERROR: AFC disable unsuccessfull retrying after 30ms\n", __func__, __LINE__);
		msleep(30);
	}

	if (retry == 3) {
		pr_err("%s:%d ERROR: AFC disable failed\n", __func__, __LINE__);
		mutex_unlock(&lock);
		return -1;
	}
        /* MUIC 9V -> 5V function */
#endif

        ret = sm5705_fled_turn_on_torch(sm5705_fled, REAR_FLASH_INDEX, 240);
        assistive_light = true;
        break;
    default:
        if (value_u32 > 1000 && value_u32 < (1000 + 32)) {
            /* Turn on Torch : 20mA ~ 320mA */
            ret = sm5705_fled_turn_on_torch(sm5705_fled, REAR_FLASH_INDEX, _calc_torch_current_mA_to_offset(value_u32 - 1000));
            assistive_light = true;
        } else {
            dev_err(dev, "%s: can't process, invalid value=%d\n", __func__, value_u32);
            ret = -EINVAL;
        }
        break;
    }
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to rear flash file operation:store (value=%d, ret=%d)\n", __func__, value_u32, ret);
    }

#ifdef DISABLE_AFC
    mutex_unlock(&lock);
#else
    spin_unlock_irqrestore(&fled_lock, flags);
#endif
    return count;
}

static ssize_t sm5705_rear_flash_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char offset = _calc_torch_current_offset_to_mA(320);

    dev_info(dev, "%s: SM5705 Movie mode max current = 320mA(offset:%d)\n", __func__, offset);

    return sprintf(buf, "%d\n", offset);
}

static DEVICE_ATTR(rear_flash, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, sm5705_rear_flash_show, sm5705_rear_flash_store);

/**
 * SM5705 Flash-LED device driver management functions
 */

static void sm5705_fled0_wait_valid_vbus_work(struct work_struct *work)
{
    if (sm5705_fled_check_valid_vbus_from_MUIC() == true) {
        _fled_turn_on_torch(g_sm5705_fled, 0);
    } else {
        queue_delayed_work(g_sm5705_fled->wqueue, &g_sm5705_fled->fled0_vbus_check_work, msecs_to_jiffies(10));
    }
}

static void sm5705_fled1_wait_valid_vbus_work(struct work_struct *work)
{
    if (sm5705_fled_check_valid_vbus_from_MUIC() == true) {
        _fled_turn_on_torch(g_sm5705_fled, 1);
    } else {
        queue_delayed_work(g_sm5705_fled->wqueue, &g_sm5705_fled->fled1_vbus_check_work, msecs_to_jiffies(10));
    }
}

#ifdef CONFIG_OF
static int sm5705_fled_parse_dt(struct sm5705_dev *sm5705, struct sm5705_fled_platform_data *pdata)
{
    struct device_node *nproot = sm5705->dev->of_node;
    struct device_node *np;// *c_np;
    unsigned int temp;
    int ret = 0, index;

    if (!nproot) {
        pr_err("<%s> could not find led sub-node led_np\n", __func__);
        return -ENODEV;
    }

    np = of_find_node_by_name(nproot, "sm5705_fled");
    if (!np) {
        pr_err("%s : could not find led sub-node np\n", __func__);
        return -EINVAL;
    }

    ret = of_property_read_u32(np, "id", &temp);
    if (ret) {
        pr_err("%s : could not find led id\n", __func__);
        return ret;
    }
    index = temp;

    ret = of_property_read_u32(np, "flash-mode-current-mA", &temp);
    if (ret) {
        pr_err("%s: fail to get dt:flash-mode-current-mA\n", __func__);
        return ret;
    }
    pdata->led[index].flash_current_mA = temp;


    ret = of_property_read_u32(np, "torch-mode-current-mA", &temp);
    if (ret) {
        pr_err("%s: fail to get dt:torch-mode-current-mA\n", __func__);
        return ret;
    }
    pdata->led[index].torch_current_mA = temp;

    ret = of_property_read_u32(np, "used-gpio-control", &temp);
    if (ret) {
        pr_err("%s: fail to get dt:used-gpio-control\n", __func__);
        return ret;
    }
    pdata->led[index].used_gpio = (bool)(temp & 0x1);

        if (pdata->led[index].used_gpio) {
        ret = of_get_named_gpio(np, "flash-en-gpio", 0);
        if (ret < 0) {
            pr_err("%s: fail to get dt:flash-en-gpio (ret=%d)\n", __func__, ret);
            return ret;
        }
        pdata->led[index].flash_en_pin = ret;

        ret = of_get_named_gpio(np, "torch-en-gpio", 0);
        if (ret < 0) {
            pr_err("%s: fail to get dt:torch-en-gpio (ret=%d)\n", __func__, ret);
            return ret;
            }
        pdata->led[index].torch_en_pin = ret;
    }

#if 0
    for_each_child_of_node(np, c_np) {
        ret = of_property_read_u32(c_np, "id", &temp);
        if (ret) {
            dev_err(dev, "%s: fail to get a id\n", __func__);
            return ret;
        }
        index = temp;

        ret = of_property_read_u32(c_np, "flash-mode-current-mA", &temp);
        if (ret) {
            dev_err(dev, "%s: fail to get dt:flash-mode-current-mA\n", __func__);
            return ret;
        }
        pdata->led[index].flash_current_mA = temp;

        ret = of_property_read_u32(c_np, "torch-mode-current-mA", &temp);
        if (ret) {
            dev_err(dev, "%s: fail to get dt:torch-mode-current-mA\n", __func__);
            return ret;
        }
        pdata->led[index].torch_current_mA = temp;

        ret = of_property_read_u32(c_np, "used-gpio-control", &temp);
        if (ret) {
            dev_err(dev, "%s: fail to get dt:used-gpio-control\n", __func__);
            return ret;
        }
        pdata->led[index].used_gpio = (bool)(temp & 0x1);

        if (pdata->led[index].used_gpio) {
            ret = of_get_named_gpio(c_np, "flash-en-gpio", 0);
            if (ret < 0) {
                dev_err(dev, "%s: fail to get dt:flash-en-gpio (ret=%d)\n", __func__, ret);
                return ret;
            }
            pdata->led[index].flash_en_pin = ret;

            ret = of_get_named_gpio(c_np, "torch-en-gpio", 0);
            if (ret < 0) {
                dev_err(dev, "%s: fail to get dt:torch-en-gpio (ret=%d)\n", __func__, ret);
                return ret;
            }
            pdata->led[index].torch_en_pin = ret;
        }
    }

#endif
    return ret;
}
#endif

static inline struct sm5705_fled_platform_data *_get_sm5705_fled_platform_data(struct device *dev, struct sm5705_dev *sm5705)
{
    struct sm5705_fled_platform_data *pdata;
    int i, ret;

#ifdef CONFIG_OF
    pdata = devm_kzalloc(dev, sizeof(struct sm5705_fled_platform_data), GFP_KERNEL);
    if (unlikely(!pdata)) {
        dev_err(dev, "%s: fail to allocate memory for sm5705_fled_platform_data\n", __func__);
        goto out_p;
    }

    ret = sm5705_fled_parse_dt(sm5705, pdata);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to parse dt for sm5705 flash-led (ret=%d)\n", __func__, ret);
        goto out_kfree_p;
    }
#else
    pdata = sm5705->pdata->fled_platform_data;
    if (unlikely(!pdata)) {
        dev_err(dev, "%s: fail to get sm5705_fled_platform_data\n", __func__);
        goto out_p;
    }
#endif

    dev_info(dev, "sm5705 flash-LED device platform data info, \n");
    for (i=0; i < SM5705_FLED_MAX; ++i) {
        dev_info(dev, "[FLED-%d] Flash: %dmA, Torch: %dmA, used_gpio=%d, GPIO_PIN(%d, %d)\n", i, pdata->led[i].flash_current_mA, \
                        pdata->led[i].torch_current_mA, pdata->led[i].used_gpio, pdata->led[i].flash_en_pin, pdata->led[i].torch_en_pin);
    }

    return pdata;

out_kfree_p:
    devm_kfree(dev, pdata);
out_p:
    return NULL;
}

static int sm5705_fled_probe(struct platform_device *pdev)
{
    struct sm5705_dev *sm5705 = dev_get_drvdata(pdev->dev.parent);
    struct sm5705_fled_info *sm5705_fled;
    struct sm5705_fled_platform_data *sm5705_fled_pdata;
    struct device *dev = &pdev->dev;
    int i,ret;

    if (IS_ERR_OR_NULL(camera_class)) {
        dev_err(dev, "%s: can't find camera_class sysfs object, didn't used rear_flash attribute\n", __func__);
        return -ENOENT;
    }

    sm5705_fled = devm_kzalloc(dev, sizeof(struct sm5705_fled_info), GFP_KERNEL);
    if (unlikely(!sm5705_fled)) {
        dev_err(dev, "%s: fail to allocate memory for sm5705_fled_info\n", __func__);
        return -ENOMEM;
    }

    dev_info(dev, "SM5705(rev.%d) Flash-LED devic driver Probing..\n", __get_revision_number());

    sm5705_fled_pdata = _get_sm5705_fled_platform_data(dev, sm5705);
    if (unlikely(!sm5705_fled_pdata)) {
        dev_info(dev, "%s: fail to get platform data\n", __func__);
        goto fled_platfrom_data_err;
    }

    sm5705_fled->dev = dev;
    sm5705_fled->i2c = sm5705->i2c;
    sm5705_fled->pdata = sm5705_fled_pdata;
    platform_set_drvdata(pdev, sm5705_fled);
    g_sm5705_fled = sm5705_fled;

    ret = sm5705_fled_initialize(sm5705_fled);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s: fail to initialize SM5705 Flash-LED[%d] (ret=%d)\n", __func__, i, ret);
        goto fled_init_err;
    }

    sm5705_fled->wqueue = create_singlethread_workqueue(dev_name(dev));
    if (!sm5705_fled->wqueue) {
        dev_err(dev, "%s: fail to Create Workqueue\n", __func__);
        goto fled_deinit_err;
    }
    INIT_DELAYED_WORK(&sm5705_fled->fled0_vbus_check_work, sm5705_fled0_wait_valid_vbus_work);
    INIT_DELAYED_WORK(&sm5705_fled->fled1_vbus_check_work, sm5705_fled1_wait_valid_vbus_work);

    /* create camera_class rear_flash device */
    sm5705_fled->rear_fled_dev = device_create(camera_class, NULL, 3, NULL, "flash");
    if (IS_ERR(sm5705_fled->rear_fled_dev)) {
        dev_err(dev, "%s fail to create device for rear_flash\n", __func__);
        goto fled_deinit_err;
    }
    sm5705_fled->rear_fled_dev->parent = dev;

    ret = device_create_file(sm5705_fled->rear_fled_dev, &dev_attr_rear_flash);
    if (IS_ERR_VALUE(ret)) {
        dev_err(dev, "%s fail to create device file for rear_flash\n", __func__);
        goto fled_rear_device_err;
    }

    dev_info(dev, "%s: Probe done.\n", __func__);

    return 0;

fled_rear_device_err:
    device_destroy(camera_class, sm5705_fled->rear_fled_dev->devt);

fled_deinit_err:
    sm5705_fled_deinitialize(sm5705_fled);

fled_init_err:
    platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_OF
    devm_kfree(dev, sm5705_fled_pdata);
#endif

fled_platfrom_data_err:
    devm_kfree(dev, sm5705_fled);

    return ret;
}

static int sm5705_fled_remove(struct platform_device *pdev)
{
    struct sm5705_fled_info *sm5705_fled = platform_get_drvdata(pdev);
    struct device *dev = &pdev->dev;
    int i;

    device_remove_file(sm5705_fled->rear_fled_dev, &dev_attr_rear_flash);
    device_destroy(camera_class, sm5705_fled->rear_fled_dev->devt);

    for (i = 0; i != SM5705_FLED_MAX; ++i) {
        sm5705_fled_turn_off(sm5705_fled, i);
    }
    sm5705_fled_deinitialize(sm5705_fled);

    platform_set_drvdata(pdev, NULL);
#ifdef CONFIG_OF
    devm_kfree(dev, sm5705_fled->pdata);
#endif
    devm_kfree(dev, sm5705_fled);

    return 0;
}

static void sm5705_fled_shutdown(struct device *dev)
{
    struct sm5705_fled_info *sm5705_fled = dev_get_drvdata(dev);
    int i;

    for (i=0; i < SM5705_FLED_MAX; ++i) {
        sm5705_fled_turn_off(sm5705_fled, i);
    }
}

#ifdef CONFIG_OF
static struct of_device_id sm5705_fled_match_table[] = {
    { .compatible = "siliconmitus,sm5705-fled",},
    {},
};
#else
#define sm5705_fled_match_table NULL
#endif

static struct platform_driver sm5705_fled_driver = {
    .probe      = sm5705_fled_probe,
    .remove     = sm5705_fled_remove,
    .driver     = {
        .name   = "sm5705-fled",
        .owner  = THIS_MODULE,
        .shutdown   = sm5705_fled_shutdown,
        .of_match_table = sm5705_fled_match_table,
    },
};

static int __init sm5705_fled_init(void)
{
    printk("%s\n",__func__);
    return platform_driver_register(&sm5705_fled_driver);
}
module_init(sm5705_fled_init);

static void __exit sm5705_fled_exit(void)
{
    platform_driver_unregister(&sm5705_fled_driver);
}
module_exit(sm5705_fled_exit);

MODULE_DESCRIPTION("SM5705 FLASH-LED driver");
MODULE_ALIAS("platform:sm5705-flashLED");
MODULE_LICENSE("GPL");

