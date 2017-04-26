/*
 * Copyright (C) 2015 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _TZISR_H_
#define _TZISR_H_

#include "tzdev.h"
#include "tzirs_ioctl.h"

#if defined(CONFIG_ARCH_MSM) && defined(CONFIG_MSM_SCM)
#if defined(CONFIG_ARCH_APQ8084) || defined(CONFIG_ARCH_MSM8939)
#include <soc/qcom/scm.h>
#elif defined(CONFIG_ARCH_MSM8974)
#include <mach/scm.h>
#endif
#endif

#ifdef TZIRS_DEBUG
#define DBG(...)		pr_info( "[tzirs] DBG : " __VA_ARGS__)
#define ERR(...)		pr_alert("[tzirs] ERR : " __VA_ARGS__)
#else
#define DBG(...)
#define ERR(...)		pr_alert("[tzirs] ERR : " __VA_ARGS__)
#endif

/* Set appropriate command value */
#define SMC_IRS_CMD_RAW	(0x0000000d)

#ifdef CONFIG_TZDEV_USE_ARM_CALLING_CONVENTION
#define SMC_IRS_CMD		CREATE_SMC_CMD(SMC_TYPE_FAST, SMC_AARCH_32, SMC_OEM_SERVICE_MASK, SMC_IRS_CMD_RAW)
#else
#define SMC_IRS_CMD		SMC_IRS_CMD_RAW
#endif

#if defined(CONFIG_ARCH_MSM) && defined(CONFIG_MSM_SCM)
#define SCM_BLOW_SW_FUSE_ID     0x01
#define SCM_IS_SW_FUSE_BLOWN_ID 0x02
#define tzirs_smc(id, value, func_cmd)		tzirs_smc_cmd(SCM_SVC_FUSE, (uint32_t)id, (uint32_t)value , (uint32_t)func_cmd)
#define tzirs_smc_cmd(p0, p1, p2, p3)		__tzirs_smc_cmd((uint32_t) p0, (uint32_t*) p1, (uint32_t*)p2, (uint32_t*)p3)
#else
#define tzirs_smc(id, value, func_cmd)		tzirs_smc_cmd(SMC_IRS_CMD, (unsigned long)id, (unsigned long) value , (unsigned long) func_cmd)
#define tzirs_smc_cmd(p0, p1, p2, p3)		__tzirs_smc_cmd((unsigned long) p0, (unsigned long*) p1, (unsigned long *)p2, (unsigned long*)p3)
#endif

typedef enum {
	IRS_FAIL			= -100,		/*Fail result*/
	IRS_UNKNOWN_ID,					/*Unknown flag id*/
	IRS_UNKNOWN_INT_CMD,				/*Unknown internal command*/
	IRS_INCORRECT_FLAG_TYPE,			/*Incorrect flag type (can be boolean, value or counter )*/
	IRS_RT_FLAGS_EMPTY,				/*List of run-time flags is empty*/
	IRS_RT_FLAGS_FULL,
	IRS_INCORRECT_RT_ID,
	IRS_DENY_READ_FROM_SMC,
	IRS_DENY_WRITE_FROM_SMC,
	IRS_DENY_DELETE_FROM_SMC,
	IRS_SUCCESS			= 0		/*Success result*/
} TZ_ISR_ERR;

#if defined(CONFIG_ARCH_MSM) && defined(CONFIG_MSM_SCM)
#if defined(CONFIG_QCOM_SCM_ARMV8)
static inline unsigned long __tzirs_smc_cmd(uint32_t p0, uint32_t *p1, uint32_t *p2, uint32_t *p3)
{
	unsigned int ret;

	struct scm_desc desc = {0};
	desc.args[0] = *p1;
	desc.arginfo = SCM_ARGS(1);

	switch(*p3) {
	case IRS_SET_FLAG_CMD:
		ret = scm_call2(SCM_SIP_FNID(p0, SCM_BLOW_SW_FUSE_ID), &desc);
		DBG("[SET_FLAG_CMD] : scm_call2 returned 0x%08x\n", ret);
		break;

	case IRS_GET_FLAG_VAL_CMD:
		ret = scm_call2(SCM_SIP_FNID(p0, SCM_IS_SW_FUSE_BLOWN_ID), &desc);
		if (ret) {
			ERR("[GET_FLAG_CMD] : scm_call2 failed ret = 0x%08x\n", ret);
			break;
		}
		DBG("[GET_FLAG_CMD] : desc.ret[0] = 0x%08x\n", (unsigned int) desc.ret[0]);
		*p2 = (uint32_t) desc.ret[0];
		break;

	default:
		ERR("Wrong SCM command ID\n");
		ret = IRS_INCORRECT_FLAG_TYPE;
		break;
	}

	return ret;
}
#else /* defined(CONFIG_QCOM_SCM_ARMV8) */
static inline unsigned long __tzirs_smc_cmd(uint32_t p0, uint32_t *p1, uint32_t *p2, uint32_t *p3)
{
	int ret;

	switch(*p3) {
	case IRS_SET_FLAG_CMD:
		ret = scm_call(p0, SCM_BLOW_SW_FUSE_ID, (void *)p1, sizeof(*p1), NULL, 0);
		break;

	case IRS_GET_FLAG_VAL_CMD:
		ret = scm_call(p0, SCM_IS_SW_FUSE_BLOWN_ID, (void *)p1, sizeof(*p1), (void *)p2, sizeof(*p2));
		break;

	default:
		ERR("Wrong SCM command ID\n");
		ret = IRS_INCORRECT_FLAG_TYPE;
		break;
	}

	return ret;
}
#endif /* defined (CONFIG_QCOM_SCM_ARMV8) */
#else /* defined(CONFIG_ARCH_MSM) && defined(CONFIG_MSM_SCM) */
static inline long __tzirs_smc_cmd(unsigned long p0, unsigned long *p1, unsigned long *p2, unsigned long *p3)
{
	register unsigned long _a0 __asm__(REGISTERS_NAME "0") = p0 | TZDEV_SMC_MAGIC;
	register unsigned long _a1 __asm__(REGISTERS_NAME "1") = *p1;
	register unsigned long _a2 __asm__(REGISTERS_NAME "2") = *p2;
	register unsigned long _a3 __asm__(REGISTERS_NAME "3") = *p3;
	register unsigned long _r0 __asm__(REGISTERS_NAME "0");
	register unsigned long _r1 __asm__(REGISTERS_NAME "1");
	register unsigned long _r2 __asm__(REGISTERS_NAME "2");
	register unsigned long _r3 __asm__(REGISTERS_NAME "3");

	__asm__ __volatile__(ARCH_EXTENSION SMC(0): "=r"(_r0) , "=r" (_r1) , "=r" (_r2), "=r" (_r3) : "0"(_a0),"r"(_a1), "r"(_a2), "r"(_a3) : "memory");

	*p3 = _r3;
	*p2 = _r2;
	*p1 = _r1;

	return _r0;
}
#endif /* defined(CONFIG_ARCH_MSM) && defined(CONFIG_MSM_SCM) */
#endif /* _TZISR_H_ */
