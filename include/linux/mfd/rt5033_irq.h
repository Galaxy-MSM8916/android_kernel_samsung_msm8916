#ifndef LINUX_MFD_RT5033_IRQ_H
#define LINUX_MFD_RT5033_IRQ_H

enum {
    RT5033_ADPBAD_IRQ = 0,
    RT5033_PPBATLV_IRQ,
    RT5033_CHTERMI_IRQ,
    RT5033_VINOVPI_IRQ,
    RT5033_TSDI_IRQ,
    RT5033_CHMIVRI_IRQ,
    RT5033_CHTREGI_IRQ,
    RT5033_CHTMRFI_IRQ,
    RT5033_CHRCHGI_IRQ,
    RT5033_IEOC_IRQ,
    RT5033_CHBATOVI_IRQ,
    RT5033_CHRVPI_IRQ,
    RT5033_BSTLOWVI_IRQ,
    RT5033_BSTOLI_IRQ,
    RT5033_BSTVMIDOVP_IRQ,
    RT5033_OVPR_IRQ,
    RT5033_VF_L_IRQ,
    RT5033_LEDCS2_SHORT_IRQ,
    RT5033_LEDCS1_SHORT_IRQ,
    RT5033_BUCK_OCP_IRQ,
    RT5033_BUCK_LV_IRQ,
    RT5033_SAFE_LDO_LV_IRQ,
    RT5033_LDO_LV_IRQ,
    RT5033_OT_IRQ,
    RT5033_VDDA_UV_IRQ,
    RT5033_IRQS_NR,
};

#define RT5033_ADPBAD_IRQ_NAME "chg_adp_bad"
#define RT5033_PPBATLV_IRQ_NAME "chg_ppbat_lv"
#define RT5033_CHTERMI_IRQ_NAME "chg_termination"
#define RT5033_VINOVPI_IRQ_NAME "chg_vin_ovp"
#define RT5033_TSDI_IRQ_NAME "chg_thermal_shutdown"
#define RT5033_CHMIVRI_IRQ_NAME "chg_mivr"
#define RT5033_CHTREGI_IRQ_NAME "chg_thermal_regulation"
#define RT5033_CHTMRFI_IRQ_NAME "chg_timeout"
#define RT5033_CHRCHGI_IRQ_NAME "chg_recharge_req"
#define RT5033_IEOC_IRQ_NAME "chg_ieoc"
#define RT5033_CHBATOVI_IRQ_NAME "chg_bat_ovp"
#define RT5033_CHRVPI_IRQ_NAME "chg_reverse_protection"
#define RT5033_BSTLOWVI_IRQ_NAME "bst_low_v"
#define RT5033_BSTOLI_IRQ_NAME "bst_over_load"
#define RT5033_BSTVMIDOVP_IRQ_NAME "bst_vmid_ovp"

#define RT5033_OVPR_IRQ_NAME "ovp_release"
#define RT5033_VF_L_IRQ_NAME "led_vf_l"
#define RT5033_LEDCS2_SHORT_IRQ_NAME "led_cs2_short"
#define RT5033_LEDCS1_SHORT_IRQ_NAME "led_cs1_short"

#define RT5033_BUCK_OCP_IRQ_NAME "pmic_buck_ocp"
#define RT5033_BUCK_LV_IRQ_NAME "pmic_buck_lv"
#define RT5033_SAFE_LDO_LV_IRQ_NAME "pmic_safe_ldo_lv"
#define RT5033_LDO_LV_IRQ_NAME "pmic_ldo_lv"
#define RT5033_OT_IRQ_NAME "pmic_ot"
#define RT5033_VDDA_UV_IRQ_NAME "pmic_vdda_uv"

const char *rt5033_get_irq_name_by_index(int index);

#endif /* LINUX_MFD_RT5033_IRQ_H */
