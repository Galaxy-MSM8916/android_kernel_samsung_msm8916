
/* Copyright (c) 2016, The Linux Foundation. All rights reserved.
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

#ifndef _VIBRATOR_H
#define _VIBRATOR_H

enum driver_chip {
	CHIP_MAXIM,
	CHIP_DRVXXX,
	CHIP_ISAXXX,
};

/* Error and Return value codes */
#define VIBRATION_SUCCESS	0	/* Success */
#define VIBRATION_FAIL		-1	/* Generic error */
#define VIBRATION_ON		1
#define VIBRATION_OFF		0

#define MAX_INTENSITY		10000

#if defined(CONFIG_SEC_GT510_PROJECT)
#define MOTOR_STRENGTH			98	/*MOTOR_STRENGTH 98 %*/
#elif defined(CONFIG_MACH_FORTUNA_TMO)
#define MOTOR_STRENGTH			98	/*MOTOR_STRENGTH 98 %*/
#elif defined(CONFIG_SEC_A3_PROJECT)
#define MOTOR_STRENGTH			90	/*MOTOR_STRENGTH 90 %*/
#elif defined(CONFIG_SEC_A7_PROJECT)
#define MOTOR_STRENGTH                  95      /*MOTOR_STRENGTH 95 %*/
#elif defined(CONFIG_MACH_A8_CHN_OPEN)
#define MOTOR_STRENGTH			84	/*MOTOR_STRENGTH 84 %*/
#else
#define MOTOR_STRENGTH			98	/*MOTOR_STRENGTH 98 %*/
#endif

#if defined(CONFIG_SEC_GT510_PROJECT)
#define GP_CLK_M_DEFAULT			3
#define GP_CLK_N_DEFAULT                        140
#define GP_CLK_D_DEFAULT			70  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER			140
#elif defined(CONFIG_MACH_FORTUNA_TMO)
#define GP_CLK_M_DEFAULT			3
#define GP_CLK_N_DEFAULT                        138
#define GP_CLK_D_DEFAULT			69  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER			138
#elif defined(CONFIG_SEC_A3_PROJECT)
#define GP_CLK_M_DEFAULT			3
#define GP_CLK_N_DEFAULT                        121
#define GP_CLK_D_DEFAULT			61  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER			121
#elif defined(CONFIG_SEC_A7_PROJECT)
#define GP_CLK_M_DEFAULT                        3
#define GP_CLK_N_DEFAULT                        119
#define GP_CLK_D_DEFAULT                        61  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER                      119
#elif defined(CONFIG_MACH_A8_CHN_OPEN)
#define GP_CLK_M_DEFAULT			3
#define GP_CLK_N_DEFAULT                        121
#define GP_CLK_D_DEFAULT			61  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER			121
#else
#define GP_CLK_M_DEFAULT			3
#define GP_CLK_N_DEFAULT                        121
#define GP_CLK_D_DEFAULT			61  /* 50% duty cycle */
#define IMM_PWM_MULTIPLIER			121
#endif

#define MOTOR_MIN_STRENGTH			54
/*
 * ** Global variables for LRA PWM M,N and D values.
 * */
int32_t g_nlra_gp_clk_m = GP_CLK_M_DEFAULT;
int32_t g_nlra_gp_clk_n = GP_CLK_N_DEFAULT;
int32_t g_nlra_gp_clk_d = GP_CLK_D_DEFAULT;
int32_t g_nlra_gp_clk_pwm_mul = IMM_PWM_MULTIPLIER;
int32_t motor_strength = MOTOR_STRENGTH;
int32_t motor_min_strength;

#define __inp(port) ioread8(port)

#define __inpw(port) ioread16(port)

#define __inpdw(port) ioread32(port)

#define __outp(port, val) iowrite8(val, port)

#define __outpw(port, val) iowrite16(val, port)

#define __outpdw(port, val) iowrite32(val, port)

#define in_dword(addr)              (__inpdw(addr))
#define in_dword_masked(addr, mask) (__inpdw(addr) & (mask))
#define out_dword(addr, val)        __outpdw(addr, val)
#define out_dword_masked(io, mask, val, shadow)  \
	(void) out_dword(io, \
	((shadow & (unsigned int)(~(mask))) | ((unsigned int)((val) & (mask)))))
#define out_dword_masked_ns(io, mask, val, current_reg_content) \
	(void) out_dword(io, \
	((current_reg_content & (unsigned int)(~(mask))) \
	| ((unsigned int)((val) & (mask)))))

static void __iomem *virt_mmss_gp1_base;
#define MSM_GCC_GPx_BASE		0x01809000

#define HWIO_CAMSS_GPx_CBCR_ADDR	((void __iomem *)(virt_mmss_gp1_base + 0x0))	// MMSS_CC_CAMSS_GPx_CBCR
#define HWIO_GPx_CMD_RCGR_ADDR		((void __iomem *)(virt_mmss_gp1_base + 0x4))	// MMSS_CC_GPx_CMD_RCGR
#define HWIO_GPx_CFG_RCGR_ADDR		((void __iomem *)(virt_mmss_gp1_base + 0x8))	// MMSS_CC_GPx_CFG_RCGR
#define HWIO_GPx_M_REG_ADDR		((void __iomem *)(virt_mmss_gp1_base + 0xc))	// MMSS_CC_GPx_M
#define HWIO_GPx_N_REG_ADDR		((void __iomem *)(virt_mmss_gp1_base + 0x10))	// MMSS_CC_GPx_N
#define HWIO_GPx_D_REG_ADDR		((void __iomem *)(virt_mmss_gp1_base + 0x14))	// MMSS_CC_GPx_D

#define HWIO_GP_MD_REG_RMSK		0xffffffff
#define HWIO_GP_N_REG_RMSK		0xffffffff

#define HWIO_GP_MD_REG_M_VAL_BMSK		0xff
#define HWIO_GP_MD_REG_M_VAL_SHFT		0
#define HWIO_GP_MD_REG_D_VAL_BMSK		0xff
#define HWIO_GP_MD_REG_D_VAL_SHFT		0
#define HWIO_GP_N_REG_N_VAL_BMSK		0xff
#define HWIO_GP_SRC_SEL_VAL_BMSK		0x700
#define HWIO_GP_SRC_SEL_VAL_SHFT		8
#define HWIO_GP_SRC_DIV_VAL_BMSK		0x1f
#define HWIO_GP_SRC_DIV_VAL_SHFT		0
#define HWIO_GP_MODE_VAL_BMSK			0x3000
#define HWIO_GP_MODE_VAL_SHFT			12

#define HWIO_CLK_ENABLE_VAL_BMSK		0x1
#define HWIO_CLK_ENABLE_VAL_SHFT		0
#define HWIO_UPDATE_VAL_BMSK			0x1
#define HWIO_UPDATE_VAL_SHFT			0
#define HWIO_ROOT_EN_VAL_BMSK			0x2
#define HWIO_ROOT_EN_VAL_SHFT			1

#define HWIO_GPx_CMD_RCGR_IN		\
		in_dword_masked(HWIO_GPx_CMD_RCGR_ADDR, HWIO_GP_N_REG_RMSK)
#define HWIO_GPx_CMD_RCGR_OUTM(m, v)	\
	out_dword_masked_ns(HWIO_GPx_CMD_RCGR_ADDR, m, v, HWIO_GPx_CMD_RCGR_IN)

#define HWIO_GPx_CFG_RCGR_IN		\
		in_dword_masked(HWIO_GPx_CFG_RCGR_ADDR, HWIO_GP_N_REG_RMSK)
#define HWIO_GPx_CFG_RCGR_OUTM(m, v)	\
	out_dword_masked_ns(HWIO_GPx_CFG_RCGR_ADDR, m, v, HWIO_GPx_CFG_RCGR_IN)

#define HWIO_CAMSS_GPx_CBCR_IN		\
		in_dword_masked(HWIO_CAMSS_GPx_CBCR_ADDR, HWIO_GP_N_REG_RMSK)
#define HWIO_CAMSS_GPx_CBCR_OUTM(m, v)	\
	out_dword_masked_ns(HWIO_CAMSS_GPx_CBCR_ADDR, m, v, HWIO_CAMSS_GPx_CBCR_IN)

#define HWIO_GPx_D_REG_IN		\
		in_dword_masked(HWIO_GPx_D_REG_ADDR, HWIO_GP_MD_REG_RMSK)

#define HWIO_GPx_D_REG_OUTM(m, v)\
	out_dword_masked_ns(HWIO_GPx_D_REG_ADDR, m, v, HWIO_GPx_D_REG_IN)

#define HWIO_GPx_M_REG_IN		\
		in_dword_masked(HWIO_GPx_M_REG_ADDR, HWIO_GP_MD_REG_RMSK)
#define HWIO_GPx_M_REG_OUTM(m, v)\
	out_dword_masked_ns(HWIO_GPx_M_REG_ADDR, m, v, HWIO_GPx_M_REG_IN)

#define HWIO_GPx_N_REG_IN		\
		in_dword_masked(HWIO_GPx_N_REG_ADDR, HWIO_GP_N_REG_RMSK)
#define HWIO_GPx_N_REG_OUTM(m, v)	\
	out_dword_masked_ns(HWIO_GPx_N_REG_ADDR, m, v, HWIO_GPx_N_REG_IN)

#define __msmhwio_outm(hwiosym, mask, val)  HWIO_##hwiosym##_OUTM(mask, val)
#define HWIO_OUTM(hwiosym, mask, val)	__msmhwio_outm(hwiosym, mask, val)
int32_t vibe_pwm_onoff(u8 onoff);
int32_t vibe_set_pwm_freq(int intensity);

#endif  /* _VIBRATOR_H */
