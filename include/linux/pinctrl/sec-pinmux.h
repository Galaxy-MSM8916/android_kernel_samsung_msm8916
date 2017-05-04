/* Copyright (c) 2010-2011,2013-2014, The Linux Foundation. All rights reserved.
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
#ifndef __LINUX_PINCTRL_SEC_PINMUX_H
#define __LINUX_PINCTRL_SEC_PINMUX_H

#include <linux/bitops.h>

enum msm_gpiomux_setting {
	GPIOMUX_ACTIVE = 0,
	GPIOMUX_SUSPENDED,
	GPIOMUX_NSETTINGS
};

enum gpiomux_drv {
	GPIOMUX_DRV_2MA = 0,
	GPIOMUX_DRV_4MA,
	GPIOMUX_DRV_6MA,
	GPIOMUX_DRV_8MA,
	GPIOMUX_DRV_10MA,
	GPIOMUX_DRV_12MA,
	GPIOMUX_DRV_14MA,
	GPIOMUX_DRV_16MA,
};

enum gpiomux_func {
	GPIOMUX_FUNC_GPIO = 0,
	GPIOMUX_FUNC_1,
	GPIOMUX_FUNC_2,
	GPIOMUX_FUNC_3,
	GPIOMUX_FUNC_4,
	GPIOMUX_FUNC_5,
	GPIOMUX_FUNC_6,
	GPIOMUX_FUNC_7,
	GPIOMUX_FUNC_8,
	GPIOMUX_FUNC_9,
	GPIOMUX_FUNC_A,
	GPIOMUX_FUNC_B,
	GPIOMUX_FUNC_C,
	GPIOMUX_FUNC_D,
	GPIOMUX_FUNC_E,
	GPIOMUX_FUNC_F,
};

enum gpiomux_pull {
	GPIOMUX_PULL_NONE = 0,
	GPIOMUX_PULL_DOWN,
	GPIOMUX_PULL_KEEPER,
	GPIOMUX_PULL_UP,
};

/* Direction settings are only meaningful when GPIOMUX_FUNC_GPIO is selected.
 * This element is ignored for all other FUNC selections, as the output-
 * enable pin is not under software control in those cases.  See the SWI
 * for your target for more details.
 */
enum gpiomux_dir {
	GPIOMUX_IN = 0,
	GPIOMUX_OUT_HIGH,
	GPIOMUX_OUT_LOW,
};

struct gpiomux_setting {
	enum gpiomux_func func;
	enum gpiomux_drv  drv;
	enum gpiomux_pull pull;
	enum gpiomux_dir  dir;
};

void msm_tlmm_v4_get_gp_cfg(uint pin_no, struct gpiomux_setting *val);
int msm_tlmm_v4_set_gp_cfg(uint pin_no, uint id, bool level);
int msm_tlmm_v4_get_gp_value(uint pin_no);
int msm_tlmm_v4_get_gp_input_value(uint pin_no);
int msm_tlmm_v4_get_gp_num(void);

#endif
