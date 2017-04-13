/*
 *
 * $Id: stk3013.h
 *
 * Copyright (C) 2012~2015 Lex Hsieh     <lex_hsieh@sensortek.com.tw>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 */
#ifndef __STK3013_H__
#define __STK3013_H__

/* platform data */
struct stk3013_platform_data
{
	uint8_t state_reg;
	uint8_t psctrl_reg;
	uint8_t alsctrl_reg;
	uint8_t ledctrl_reg;
	uint8_t wait_reg;
	uint16_t ps_thd_h;
	uint16_t ps_thd_l;
	uint16_t ps_cancel_thd_h;
	uint16_t ps_cancel_thd_l;
	uint16_t ps_cal_skip_adc;
	uint16_t ps_cal_fail_adc;
	uint16_t ps_default_offset;
	int int_pin;
	uint32_t transmittance;
	uint32_t int_flags;
};
#endif // __STK3013_H__
