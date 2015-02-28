/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/qpnp/power-on.h>
#include <linux/of_address.h>

#include <asm/cacheflush.h>
#include <asm/system_misc.h>

#include <soc/qcom/scm.h>
#include <soc/qcom/restart.h>
#include <soc/qcom/watchdog.h>
#ifdef CONFIG_SEC_DEBUG
#include <linux/sec_debug.h>
#include <linux/notifier.h>
#include <linux/ftrace.h>
#endif

#define EMERGENCY_DLOAD_MAGIC1    0x322A4F99
#define EMERGENCY_DLOAD_MAGIC2    0xC67E4350
#define EMERGENCY_DLOAD_MAGIC3    0x77777777

#define SCM_IO_DISABLE_PMIC_ARBITER	1
#define SCM_IO_DEASSERT_PS_HOLD		2
#define SCM_WDOG_DEBUG_BOOT_PART	0x9
#define SCM_DLOAD_MODE			0X10
#define SCM_EDLOAD_MODE			0X01
#define SCM_DLOAD_CMD			0x10


static int restart_mode;
#ifndef CONFIG_SEC_DEBUG
void *restart_reason;
#endif
static bool scm_pmic_arbiter_disable_supported;
static bool scm_deassert_ps_hold_supported;
/* Download mode master kill-switch */
static void __iomem *msm_ps_hold;
static phys_addr_t tcsr_boot_misc_detect;

#ifdef CONFIG_MSM_DLOAD_MODE
#define EDL_MODE_PROP "qcom,msm-imem-emergency_download_mode"
#define DL_MODE_PROP "qcom,msm-imem-download_mode"

static int in_panic;
static void *dload_mode_addr;
static bool dload_mode_enabled;
static void *emergency_dload_mode_addr;
static bool scm_dload_supported;

static int dload_set(const char *val, struct kernel_param *kp);
static int download_mode = 1;
module_param_call(download_mode, dload_set, param_get_int,
			&download_mode, 0644);
static int panic_prep_restart(struct notifier_block *this,
			      unsigned long event, void *ptr)
{
	in_panic = 1;
	return NOTIFY_DONE;
}

static struct notifier_block panic_blk = {
	.notifier_call	= panic_prep_restart,
};

int scm_set_dload_mode(int arg1, int arg2)
{
	struct scm_desc desc = {
		.args[0] = arg1,
		.args[1] = arg2,
		.arginfo = SCM_ARGS(2),
	};

	if (!scm_dload_supported) {
		if (tcsr_boot_misc_detect)
			return scm_io_write(tcsr_boot_misc_detect, arg1);

		return 0;
	}

	if (!is_scm_armv8())
		return scm_call_atomic2(SCM_SVC_BOOT, SCM_DLOAD_CMD, arg1,
					arg2);

	return scm_call2_atomic(SCM_SIP_FNID(SCM_SVC_BOOT, SCM_DLOAD_CMD),
				&desc);
}

void set_dload_mode(int on)
{
	int ret;

	if (dload_mode_addr) {
		__raw_writel(on ? 0xE47B337D : 0, dload_mode_addr);
		__raw_writel(on ? 0xCE14091A : 0,
		       dload_mode_addr + sizeof(unsigned int));
		mb();
	}

	ret = scm_set_dload_mode(on ? SCM_DLOAD_MODE : 0, 0);
	if (ret)
		pr_err("Failed to set secure DLOAD mode: %d\n", ret);

	dload_mode_enabled = on;
#ifdef CONFIG_SEC_DEBUG
	pr_err("set_dload_mode <%d> ( %x )\n", on,
			(unsigned int) CALLER_ADDR0);
#endif
}
EXPORT_SYMBOL(set_dload_mode);
#ifdef CONFIG_QCOM_HARDREBOOT_IMPLEMENTATION
static bool get_dload_mode(void)
{
	return dload_mode_enabled;
}
#endif

static void enable_emergency_dload_mode(void)
{
	int ret;

	if (emergency_dload_mode_addr) {
		__raw_writel(EMERGENCY_DLOAD_MAGIC1,
				emergency_dload_mode_addr);
		__raw_writel(EMERGENCY_DLOAD_MAGIC2,
				emergency_dload_mode_addr +
				sizeof(unsigned int));
		__raw_writel(EMERGENCY_DLOAD_MAGIC3,
				emergency_dload_mode_addr +
				(2 * sizeof(unsigned int)));

		/* Need disable the pmic wdt, then the emergency dload mode
		 * will not auto reset. */
		qpnp_pon_wd_config(0);
		mb();
	}

	ret = scm_set_dload_mode(SCM_EDLOAD_MODE, 0);
	if (ret)
		pr_err("Failed to set secure EDLOAD mode: %d\n", ret);
}

static int dload_set(const char *val, struct kernel_param *kp)
{
	int ret;
	int old_val = download_mode;

	ret = param_set_int(val, kp);

	if (ret)
		return ret;

	/* If download_mode is not zero or one, ignore. */
	if (download_mode >> 1) {
		download_mode = old_val;
		return -EINVAL;
	}

	set_dload_mode(download_mode);

	return 0;
}
#else
#define set_dload_mode(x) do {} while (0)

static void enable_emergency_dload_mode(void)
{
	pr_err("dload mode is not enabled on target\n");
}

#ifdef CONFIG_QCOM_HARDREBOOT_IMPLEMENTATION
static bool get_dload_mode(void)
{
	return false;
}
#endif
#endif

void msm_set_restart_mode(int mode)
{
	restart_mode = mode;
}
EXPORT_SYMBOL(msm_set_restart_mode);

/*
 * Force the SPMI PMIC arbiter to shutdown so that no more SPMI transactions
 * are sent from the MSM to the PMIC.  This is required in order to avoid an
 * SPMI lockup on certain PMIC chips if PS_HOLD is lowered in the middle of
 * an SPMI transaction.
 */
static void halt_spmi_pmic_arbiter(void)
{
	struct scm_desc desc = {
		.args[0] = 0,
		.arginfo = SCM_ARGS(1),
	};

	if (scm_pmic_arbiter_disable_supported) {
		pr_crit("Calling SCM to disable SPMI PMIC arbiter\n");
		if (!is_scm_armv8())
			scm_call_atomic1(SCM_SVC_PWR,
					 SCM_IO_DISABLE_PMIC_ARBITER, 0);
		else
			scm_call2_atomic(SCM_SIP_FNID(SCM_SVC_PWR,
				  SCM_IO_DISABLE_PMIC_ARBITER), &desc);
	}
}

static void msm_restart_prepare(const char *cmd)
{
#ifdef CONFIG_QCOM_HARDREBOOT_IMPLEMENTATION
	bool need_warm_reset = false;
#endif
#ifndef CONFIG_QCOM_HARDREBOOT_IMPLEMENTATION
	unsigned long value;
	unsigned int warm_reboot_set = 0;
#endif
#ifndef CONFIG_SEC_DEBUG
#ifdef CONFIG_MSM_DLOAD_MODE

	/* Write download mode flags if we're panic'ing
	 * Write download mode flags if restart_mode says so
	 * Kill download mode if master-kill switch is set
	 */

	set_dload_mode(download_mode &&
			(in_panic || restart_mode == RESTART_DLOAD));
#endif
#endif
#ifdef CONFIG_SEC_DEBUG_LOW_LOG
#ifdef CONFIG_MSM_DLOAD_MODE
#ifdef CONFIG_SEC_DEBUG
	if (sec_debug_is_enabled()
	&& ((restart_mode == RESTART_DLOAD) || in_panic))
		set_dload_mode(1);
	else
		set_dload_mode(0);
#else
	set_dload_mode(0);
	set_dload_mode(in_panic);
	if (restart_mode == RESTART_DLOAD)
		set_dload_mode(1);
#endif
#endif
#endif
/* Qualcomm has provided support to implement PMIC warm reboot for recovery/fastboot/RTC cases.
However, Samsung implemation already supports more usecases including nvrestore, nvbackup, EDL, LPM etc.
Hence Qualcomm's PMIC hard reboot implementation has been taken, but disabled. */
#ifdef CONFIG_QCOM_HARDREBOOT_IMPLEMENTATION
	if (qpnp_pon_check_hard_reset_stored()) {
		/* Set warm reset as true when device is in dload mode
		 *  or device doesn't boot up into recovery, bootloader or rtc.
		 */
		if (get_dload_mode() ||
			((cmd != NULL && cmd[0] != '\0') &&
			strcmp(cmd, "recovery") &&
			strcmp(cmd, "bootloader") &&
			strcmp(cmd, "rtc")))
			need_warm_reset = true;
	} else {
		need_warm_reset = (get_dload_mode() ||
				(cmd != NULL && cmd[0] != '\0'));
	}

#ifdef CONFIG_MSM_PRESERVE_MEM
	need_warm_reset = true;
#endif

	/* Hard reset the PMIC unless memory contents must be maintained. */
	if (need_warm_reset) {
		qpnp_pon_system_pwr_off(PON_POWER_OFF_WARM_RESET);
	} else {
		qpnp_pon_system_pwr_off(PON_POWER_OFF_HARD_RESET);
	}
	 if (cmd != NULL) {
                if (!strncmp(cmd, "bootloader", 10)) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_BOOTLOADER);
                        __raw_writel(0x77665500, restart_reason);
                } else if (!strncmp(cmd, "recovery", 8)) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_RECOVERY);
                        __raw_writel(0x77665502, restart_reason);
                } else if (!strcmp(cmd, "rtc")) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_RTC);
                        __raw_writel(0x77665503, restart_reason);
                } else if (!strcmp(cmd, "dm-verity device corrupted")) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_DMVERITY_CORRUPTED);
                        __raw_writel(0x77665508, restart_reason);
                } else if (!strcmp(cmd, "dm-verity enforcing")) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_DMVERITY_ENFORCE);
                        __raw_writel(0x77665509, restart_reason);
                } else if (!strcmp(cmd, "keys clear")) {
                        qpnp_pon_set_restart_reason(
                                PON_RESTART_REASON_KEYS_CLEAR);
                        __raw_writel(0x7766550a, restart_reason);
                } else if (!strncmp(cmd, "oem-", 4)) {
                        unsigned long code;
                        int ret;
                        ret = kstrtoul(cmd + 4, 16, &code);
                        if (!ret)
                                __raw_writel(0x6f656d00 | (code & 0xff),
                                             restart_reason);
                } else if (!strncmp(cmd, "edl", 3)) {
                        enable_emergency_dload_mode();
                } else if (!strncmp(cmd, "download", 8)) {
                        __raw_writel(0x12345671, restart_reason);
                } else {
                        __raw_writel(0x77665501, restart_reason);
                }
        }
#else
	pr_info("preparing for restart now\n");
	warm_reboot_set = 0;

	if (cmd != NULL) {
		printk(KERN_NOTICE " Reboot cmd=%s\n",cmd);
		if (!strncmp(cmd, "bootloader", 10)) {
			__raw_writel(0x77665500, restart_reason);
			warm_reboot_set = 1;
		} else if (!strncmp(cmd, "recovery", 8)) {
			__raw_writel(0x77665502, restart_reason);
		} else if (!strcmp(cmd, "rtc")) {
			__raw_writel(0x77665503, restart_reason);
                } else if (!strcmp(cmd, "dm-verity device corrupted")) {
                        __raw_writel(0x77665508, restart_reason);
                } else if (!strcmp(cmd, "dm-verity enforcing")) {
                        __raw_writel(0x77665509, restart_reason);
                } else if (!strcmp(cmd, "keys clear")) {
                        __raw_writel(0x7766550a, restart_reason);
		} else if (!strncmp(cmd, "oem-", 4)) {
			unsigned long code;
			int ret;
			ret = kstrtoul(cmd + 4, 16, &code);
			if (!ret)
				__raw_writel(0x6f656d00 | (code & 0xff),
					     restart_reason);
#ifdef CONFIG_SEC_DEBUG
		} else if (!strncmp(cmd, "sec_debug_hw_reset", 18)) {
			__raw_writel(0x776655ee, restart_reason);
			warm_reboot_set = 1;
#endif
		} else if (!strncmp(cmd, "download", 8)) {
		    __raw_writel(0x12345671, restart_reason);
                    warm_reboot_set = 1;
		} else if (!strncmp(cmd, "nvbackup", 8)) {
				__raw_writel(0x77665511, restart_reason);
				warm_reboot_set = 1;
		} else if (!strncmp(cmd, "nvrestore", 9)) {
				__raw_writel(0x77665512, restart_reason);
				warm_reboot_set = 1;
		} else if (!strncmp(cmd, "nverase", 7)) {
				__raw_writel(0x77665514, restart_reason);
				warm_reboot_set = 1;
		} else if (!strncmp(cmd, "nvrecovery", 10)) {
				__raw_writel(0x77665515, restart_reason);
				warm_reboot_set = 1;
		} else if (!strncmp(cmd, "sud", 3)) {
				__raw_writel(0xabcf0000 | (cmd[3] - '0'),
								restart_reason);
		} else if (!strncmp(cmd, "debug", 5)
						&& !kstrtoul(cmd + 5, 0, &value)) {
				__raw_writel(0xabcd0000 | value, restart_reason);
		} else if (!strncmp(cmd, "cpdebug", 7) /*  set cp debug level */
						&& !kstrtoul(cmd + 7, 0, &value)) {
				__raw_writel(0xfedc0000 | value, restart_reason);
#if defined(CONFIG_MUIC_SUPPORT_RUSTPROOF)
		} else if (!strncmp(cmd, "swsel", 5) /* set switch value */
		&& !kstrtoul(cmd + 5, 0, &value)) {
		__raw_writel(0xabce0000 | value, restart_reason);
#endif
		} else if (!strncmp(cmd, "edl", 3)) {
			enable_emergency_dload_mode();
				warm_reboot_set = 1;
		} else if (strlen(cmd) == 0) {
		    printk(KERN_NOTICE "%s : value of cmd is NULL.\n", __func__);
		        __raw_writel(0x12345678, restart_reason);
#ifdef CONFIG_SEC_PERIPHERAL_SECURE_CHK
		} else if (!strncmp(cmd, "peripheral_hw_reset", 19)) {
			__raw_writel(0x77665507, restart_reason);
			warm_reboot_set = 1;
#endif
		} else {
			__raw_writel(0x77665501, restart_reason);
		}
		printk(KERN_NOTICE "%s : restart_reason = 0x%x\n",
				__func__, __raw_readl(restart_reason));
		pr_err("%s : restart_reason = 0x%x\n",
				__func__, __raw_readl(restart_reason));
	}
#ifdef CONFIG_SEC_DEBUG
	else {
		printk(KERN_NOTICE "%s: clear reset flag\n", __func__);
		warm_reboot_set = 1;
		__raw_writel(0x12345678, restart_reason);
	}
#endif
	printk(KERN_NOTICE "%s : restart_reason = 0x%x\n",
			__func__, __raw_readl(restart_reason));
	printk(KERN_NOTICE "%s : warm_reboot_set = %d\n",
			__func__, warm_reboot_set);
#ifdef CONFIG_RESTART_REASON_SEC_PARAM
	//fixme : Enabling Hard reset
	/* Memory contents will be lost when when PMIC is configured for HARD RESET */
	if (warm_reboot_set == 1) {
		qpnp_pon_system_pwr_off(PON_POWER_OFF_WARM_RESET);
		printk(KERN_NOTICE "Configure as WARM RESET\n");
	}
	else {
		qpnp_pon_system_pwr_off(PON_POWER_OFF_HARD_RESET);
		printk(KERN_NOTICE "Configure as HARD RESET\n");
	}
#else
		qpnp_pon_system_pwr_off(PON_POWER_OFF_WARM_RESET);
#endif
#endif
	flush_cache_all();

	/*outer_flush_all is not supported by 64bit kernel*/
#ifndef CONFIG_ARM64
	outer_flush_all();
#endif

}

/*
 * Deassert PS_HOLD to signal the PMIC that we are ready to power down or reset.
 * Do this by calling into the secure environment, if available, or by directly
 * writing to a hardware register.
 *
 * This function should never return.
 */
static void deassert_ps_hold(void)
{
	struct scm_desc desc = {
		.args[0] = 0,
		.arginfo = SCM_ARGS(1),
	};

	if (scm_deassert_ps_hold_supported) {
		/* This call will be available on ARMv8 only */
		scm_call2_atomic(SCM_SIP_FNID(SCM_SVC_PWR,
				 SCM_IO_DEASSERT_PS_HOLD), &desc);
	}

	/* Fall-through to the direct write in case the scm_call "returns" */
	__raw_writel(0, msm_ps_hold);
}

#ifdef CONFIG_SEC_DEBUG
void do_msm_restart(enum reboot_mode reboot_mode, const char *cmd)
#else
static void do_msm_restart(enum reboot_mode reboot_mode, const char *cmd)
#endif
{
	int ret;
	struct scm_desc desc = {
		.args[0] = 1,
		.args[1] = 0,
		.arginfo = SCM_ARGS(2),
	};

	pr_notice("Going down for restart now\n");

	msm_restart_prepare(cmd);

#ifdef CONFIG_MSM_DLOAD_MODE
	/*
	 * Trigger a watchdog bite here and if this fails,
	 * device will take the usual restart path.
	 */

	if (WDOG_BITE_ON_PANIC && in_panic)
		msm_trigger_wdog_bite();
#endif

	/* Needed to bypass debug image on some chips */
	if (!is_scm_armv8())
		ret = scm_call_atomic2(SCM_SVC_BOOT,
			       SCM_WDOG_DEBUG_BOOT_PART, 1, 0);
	else
		ret = scm_call2_atomic(SCM_SIP_FNID(SCM_SVC_BOOT,
			  SCM_WDOG_DEBUG_BOOT_PART), &desc);
	if (ret)
		pr_err("Failed to disable secure wdog debug: %d\n", ret);

	halt_spmi_pmic_arbiter();
	deassert_ps_hold();

	mdelay(10000);
}

static void do_msm_poweroff(void)
{
	int ret;
	struct scm_desc desc = {
		.args[0] = 1,
		.args[1] = 0,
		.arginfo = SCM_ARGS(2),
	};

	pr_notice("Powering off the SoC\n");
#ifdef CONFIG_MSM_DLOAD_MODE
	set_dload_mode(0);
#endif
	qpnp_pon_system_pwr_off(PON_POWER_OFF_SHUTDOWN);
	/* Needed to bypass debug image on some chips */
	if (!is_scm_armv8())
		ret = scm_call_atomic2(SCM_SVC_BOOT,
			       SCM_WDOG_DEBUG_BOOT_PART, 1, 0);
	else
		ret = scm_call2_atomic(SCM_SIP_FNID(SCM_SVC_BOOT,
			  SCM_WDOG_DEBUG_BOOT_PART), &desc);
	if (ret)
		pr_err("Failed to disable wdog debug: %d\n", ret);

	halt_spmi_pmic_arbiter();
	deassert_ps_hold();

	mdelay(10000);
	pr_err("Powering off has failed\n");
	return;
}

#ifdef CONFIG_SEC_DEBUG
static int dload_mode_normal_reboot_handler(struct notifier_block *nb,
				unsigned long l, void *p)
{
	set_dload_mode(0);
	return 0;
}

static struct notifier_block dload_reboot_block = {
	.notifier_call = dload_mode_normal_reboot_handler
};
#endif

static int msm_restart_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *mem;
	struct device_node *np;
	int ret = 0;

#ifdef CONFIG_MSM_DLOAD_MODE
	if (scm_is_call_available(SCM_SVC_BOOT, SCM_DLOAD_CMD) > 0)
		scm_dload_supported = true;

	atomic_notifier_chain_register(&panic_notifier_list, &panic_blk);
#ifdef CONFIG_SEC_DEBUG
	register_reboot_notifier(&dload_reboot_block);
#endif
	np = of_find_compatible_node(NULL, NULL, DL_MODE_PROP);
	if (!np) {
		pr_err("unable to find DT imem DLOAD mode node\n");
	} else {
		dload_mode_addr = of_iomap(np, 0);
		if (!dload_mode_addr)
			pr_err("unable to map imem DLOAD offset\n");
	}

	np = of_find_compatible_node(NULL, NULL, EDL_MODE_PROP);
	if (!np) {
		pr_err("unable to find DT imem EDLOAD mode node\n");
	} else {
		emergency_dload_mode_addr = of_iomap(np, 0);
		if (!emergency_dload_mode_addr)
			pr_err("unable to map imem EDLOAD mode offset\n");
	}

#endif
#ifndef CONFIG_SEC_DEBUG
	np = of_find_compatible_node(NULL, NULL,
				"qcom,msm-imem-restart_reason");
	if (!np) {
		pr_err("unable to find DT imem restart reason node\n");
	} else {
		restart_reason = of_iomap(np, 0);
		if (!restart_reason) {
			pr_err("unable to map imem restart reason offset\n");
			ret = -ENOMEM;
			goto err_restart_reason;
		}
	}
#endif
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	msm_ps_hold = devm_ioremap_resource(dev, mem);
	if (IS_ERR(msm_ps_hold))
		return PTR_ERR(msm_ps_hold);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (mem)
		tcsr_boot_misc_detect = mem->start;

	pm_power_off = do_msm_poweroff;
	arm_pm_restart = do_msm_restart;

	if (scm_is_call_available(SCM_SVC_PWR, SCM_IO_DISABLE_PMIC_ARBITER) > 0)
		scm_pmic_arbiter_disable_supported = true;

	if (scm_is_call_available(SCM_SVC_PWR, SCM_IO_DEASSERT_PS_HOLD) > 0)
		scm_deassert_ps_hold_supported = true;

	set_dload_mode(download_mode);

#ifdef CONFIG_SEC_DEBUG_LOW_LOG
	if (!sec_debug_is_enabled()) {
		set_dload_mode(0);
	}
#endif

	return 0;
#ifndef CONFIG_SEC_DEBUG
err_restart_reason:
#endif
#ifdef CONFIG_MSM_DLOAD_MODE
	iounmap(emergency_dload_mode_addr);
	iounmap(dload_mode_addr);
#endif
	return ret;
}

static const struct of_device_id of_msm_restart_match[] = {
	{ .compatible = "qcom,pshold", },
	{},
};
MODULE_DEVICE_TABLE(of, of_msm_restart_match);

static struct platform_driver msm_restart_driver = {
	.probe = msm_restart_probe,
	.driver = {
		.name = "msm-restart",
		.of_match_table = of_match_ptr(of_msm_restart_match),
	},
};

static int __init msm_restart_init(void)
{
	return platform_driver_register(&msm_restart_driver);
}
device_initcall(msm_restart_init);
