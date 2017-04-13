/* include/linux/leds/rt5033_fled.h
 * Header of Richtek RT5033 Flash LED Driver
 *
 * Copyright (C) 2013 Richtek Technology Corp.
 * Author: Patrick Chang <patrick_chang@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef LINUX_LEDS_RT5033_FLED_H
#define LINUX_LEDS_RT5033_FLED_H
#include <linux/kernel.h>
#include "rtfled.h"


#define RT5033_STROBE_CURRENT(mA) (((mA - 50) / 25) & 0x1f)
#define RT5033_TIMEOUT_LEVEL(mA) (((mA - 50) / 25) & 0x07)
#define RT5033_STROBE_TIMEOUT(ms) (((ms - 64) / 32) & 0x3f)
#define RT5033_TORCH_CURRENT(mA) (((mA * 10 - 125) / 125) & 0x0f)
#define RT5033_LV_PROTECTION(mV) (((mV - 3000) / 100) & 0x07)
#define RT5033_MID_REGULATION_LEVEL(mV) (((mV - 3625) / 25) & 0x3f)

typedef struct rt5033_fled_platform_data {
    unsigned int fled1_en : 1;
    unsigned int fled2_en : 1;
    unsigned int fled_mid_track_alive : 1;
    unsigned int fled_mid_auto_track_en : 1;
    unsigned int fled_timeout_current_level;
    unsigned int fled_strobe_current;
    unsigned int fled_strobe_timeout;
    unsigned int fled_torch_current;
    unsigned int fled_lv_protection;
    unsigned int fled_mid_level;
    struct pinctrl *fled_pinctrl;
    struct pinctrl_state *gpio_state_active;
    struct pinctrl_state *gpio_state_suspend;

} rt5033_fled_platform_data_t;

#define RT5033_FLED_RESET           0x21

#define RT5033_FLED_FUNCTION1       0x21
#define RT5033_FLED_FUNCTION2       0x22
#define RT5033_FLED_STROBE_CONTROL1 0x23
#define RT5033_FLED_STROBE_CONTROL2 0x24
#define RT5033_FLED_CONTROL1        0x25
#define RT5033_FLED_CONTROL2        0x26
#define RT5033_FLED_CONTROL3        0x27
#define RT5033_FLED_CONTROL4        0x28
#define RT5033_FLED_CONTROL5        0x29

#define RT5033_CHG_FLED_CTRL        0x67

extern int32_t rt5033_charger_notification(struct rt_fled_info *info, int32_t attach);
extern int32_t rt5033_boost_notification(struct rt_fled_info *info, int32_t on);
extern void rt5033_fled_lock(struct rt_fled_info *fled_info);
extern void rt5033_fled_unlock(struct rt_fled_info *fled_info);

#ifdef CONFIG_FLED_RT5033_EXT_GPIO
/* If you are using external GPIO to control torch and flash led,
 * you must call rt5033_fled_srobe_critial_section_lock() for camera shot,
 * and call rt5033_fled_srobe_critial_section_unlock() after camera mode.
 * example code :
 * struct camera_chip* chip;
 * if (chip->fled_info == NULL)
 *      chip->fled_info = rt_fled_get_info_by_name(NULL);
 * if (chip->fled_info)
 *      rt5033_fled_srobe_critial_section_lock(chip->fled_info);
 *  ...
 *  ... camera opertion
 *  ...
 *  ... when finished camera operation
 * if (chip->fled_info)
 *      rt5033_fled_srobe_critial_section_unlock(chip->fled_info);
 */
extern void rt5033_fled_strobe_critial_section_lock(struct rt_fled_info *fled_info);
extern void rt5033_fled_strobe_critial_section_unlock(struct rt_fled_info *fled_info);
#endif /* CONFIG_FLED_RT5033_EXT_GPIO */

#endif /* LINUX_LEDS_RT5033_FLED_H */
