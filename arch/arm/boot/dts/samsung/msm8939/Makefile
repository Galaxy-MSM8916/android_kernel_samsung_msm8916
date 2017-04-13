ifeq ($(CONFIG_OF),y)

dtb-$(CONFIG_ARCH_MSM8974) += msm8974-v1-cdp.dtb \
	msm8974-v1-fluid.dtb \
	msm8974-v1-liquid.dtb \
	msm8974-v1-mtp.dtb \
	msm8974-v1-rumi.dtb \
	msm8974-v1-sim.dtb \
	msm8974-v2.0-1-cdp.dtb \
	msm8974-v2.0-1-fluid.dtb \
	msm8974-v2.0-1-liquid.dtb \
	msm8974-v2.0-1-mtp.dtb \
	apq8074-v2.0-1-cdp.dtb \
	apq8074-v2.0-1-liquid.dtb \
	apq8074-v2.0-1-dragonboard.dtb \
	apq8074-v2.2-cdp.dtb \
	apq8074-v2.2-liquid.dtb \
	apq8074-v2.2-dragonboard.dtb \
	msm8974-v2.2-cdp.dtb \
	msm8974-v2.2-fluid.dtb \
	msm8974-v2.2-liquid.dtb \
	msm8974-v2.2-mtp.dtb \
	msm8974pro-ab-pm8941-cdp.dtb \
	msm8974pro-ab-pm8941-fluid.dtb \
	msm8974pro-ab-pm8941-liquid.dtb \
	msm8974pro-ab-pm8941-mtp.dtb \
	msm8974pro-ac-pm8941-cdp.dtb \
	msm8974pro-ac-pm8941-liquid.dtb \
	msm8974pro-ac-pm8941-mtp.dtb \
	msm8974pro-ac-pma8084-pm8941-mtp.dtb
dtb-$(CONFIG_ARCH_MSM8909) += msm8909-sim.dtb \
	msm8909-rumi.dtb \
	msm8909-qrd-skua.dtb \
	msm8909-qrd-skuc.dtb \
	msm8909-qrd-skue.dtb \
	msm8909-cdp.dtb \
	msm8909-mtp.dtb \
	msm8909-1gb-qrd-skua.dtb \
	msm8909-1gb-cdp.dtb \
	msm8909-1gb-mtp.dtb
ifeq ($(CONFIG_SEC_A7_PROJECT),y)
	ifeq ($(CONFIG_MACH_A7_EUR_OPEN),y)
		dtb-y += msm8939-sec-a7-eur-r01.dtb
		dtb-y += msm8939-sec-a7-eur-r03.dtb
		dtb-y += msm8939-sec-a7-eur-r04.dtb
		dtb-y += msm8939-sec-a7-eur-r05.dtb
		dtb-y += msm8939-sec-a7-eur-r06.dtb
	else ifeq ($(CONFIG_MACH_A73G_EUR_OPEN),y)
		dtb-y += msm8939-sec-a73g-eur-r00.dtb
		dtb-y += msm8939-sec-a73g-eur-r01.dtb
		dtb-y += msm8939-sec-a73g-eur-r03.dtb
		dtb-y += msm8939-sec-a73g-eur-r05.dtb
		dtb-y += msm8939-sec-a73g-eur-r06.dtb
		dtb-y += msm8939-sec-a73g-eur-r07.dtb
		dtb-y += msm8939-sec-a73g-eur-r09.dtb
	else ifeq ($(CONFIG_MACH_A7_CHN_CTC),y)
		dtb-y += msm8939-sec-a7-ctc-r04.dtb
		dtb-y += msm8939-sec-a7-ctc-r05.dtb
		dtb-y += msm8939-sec-a7-ctc-r06.dtb
	else ifeq ($(CONFIG_MACH_A7_KOR_OPEN),y)
		dtb-y += msm8939-sec-a7-kor-r05.dtb
		dtb-y += msm8939-sec-a7-kor-r06.dtb
		dtb-y += msm8939-sec-a7-kor-r08.dtb
	else ifeq ($(CONFIG_MACH_A7_CHN_TW),y)
		dtb-y += msm8939-sec-a7-tw-r00.dtb
	else
		dtb-y += msm8939-sec-a7-chn-r01.dtb
		dtb-y += msm8939-sec-a7-chn-r03.dtb
		dtb-y += msm8939-sec-a7-chn-r04.dtb
		dtb-y += msm8939-sec-a7-chn-r05.dtb
		dtb-y += msm8939-sec-a7-chn-r06.dtb
	endif
endif
ifeq ($(CONFIG_SEC_K5_PROJECT),y)
	ifeq ($(CONFIG_MACH_K5_CHN_OPEN),y)
		dtb-y += msm8939-sec-k5-chn-r00.dtb
	endif
endif

ifeq ($(CONFIG_SEC_H7_PROJECT),y)
	ifeq ($(CONFIG_MACH_H7_CHN_OPEN),y)
		dtb-y += msm8939-sec-h7-chn-r01.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A8_PROJECT),y)
	ifeq ($(CONFIG_MACH_A8_CHN_OPEN),y)
		dtb-y += msm8939-sec-a8-chn-r02.dtb
		dtb-y += msm8939-sec-a8-chn-r05.dtb
		dtb-y += msm8939-sec-a8-chn-r06.dtb
		dtb-y += msm8939-sec-a8-chn-r07.dtb
		dtb-y += msm8939-sec-a8-chn-r08.dtb
		dtb-y += msm8939-sec-a8-chn-r13.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A5X_PROJECT),y)
	ifeq ($(CONFIG_MACH_A5X_CHN_OPEN),y)
		dtb-y += msm8939-sec-a5x-chn-r00.dtb
		dtb-y += msm8939-sec-a5x-chn-r01.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A7X_PROJECT),y)
	ifeq ($(CONFIG_MACH_A7X_CHN_OPEN),y)
		dtb-y += msm8939-sec-a7x-chn-r00.dtb
		dtb-y += msm8939-sec-a7x-chn-r01.dtb
		dtb-y += msm8939-sec-a7x-chn-r02.dtb
		dtb-y += msm8939-sec-a7x-chn-r03.dtb
		dtb-y += msm8939-sec-a7x-chn-r04.dtb
		dtb-y += msm8939-sec-a7x-chn-r05.dtb
	endif
endif

dtb-$(CONFIG_ARCH_MSM8226) += msm8226-sim.dtb \
	msm8226-fluid.dtb \
	msm8226-v1-cdp.dtb \
	msm8226-v1-w-cdp.dtb \
	msm8226-v1-mtp.dtb \
	msm8226-v1-w-mtp.dtb \
	msm8226-v1-qrd-evt.dtb \
	msm8226-v1-qrd-dvt.dtb \
	msm8226-v2-720p-cdp.dtb \
	msm8226-v2-320p-w-cdp.dtb \
	msm8226-v2-1080p-cdp.dtb \
	msm8226-v2-720p-mtp.dtb \
	msm8226-v2-320p-w-mtp.dtb \
	msm8226-v2-1080p-mtp.dtb \
	msm8226-v2-qrd-evt.dtb \
	msm8226-v2-qrd-dvt.dtb \
	msm8926-v1-720p-cdp.dtb \
	msm8926-v1-1080p-cdp.dtb \
	msm8926-v1-720p-mtp.dtb \
	msm8926-v1-1080p-mtp.dtb \
	msm8926-v1-qrd.dtb \
	msm8926-v1-qrd-skug.dtb \
	msm8926-v1-qrd-skug-pvt.dtb \
	msm8926-v2-720p-cdp.dtb \
	msm8926-v2-1080p-cdp.dtb \
	msm8926-v2-1080p-ext-buck-cdp.dtb \
	msm8926-v2-720p-mtp.dtb \
	msm8926-v2-1080p-mtp.dtb \
	msm8926-v2-1080p-ext-buck-mtp.dtb \
	msm8926-v2-qrd.dtb \
	msm8926-v2-qrd-skug.dtb \
	msm8926-v2-qrd-skug-pvt.dtb \
	msm8226-v1-qrd-skuf.dtb \
	msm8226-v2-qrd-skuf.dtb \
	apq8026-v1-xpm.dtb \
	apq8026-v1-cdp.dtb \
	apq8026-v1-mtp.dtb \
	apq8026-v1-w-mtp.dtb \
	apq8026-v2-xpm.dtb \
	apq8026-v2-720p-cdp.dtb \
	apq8026-v2-320p-w-cdp.dtb \
	apq8026-v2-1080p-cdp.dtb \
	apq8026-v2-720p-mtp.dtb \
	apq8026-v2-320p-w-mtp.dtb \
	apq8026-v2-1080p-mtp.dtb
dtb-$(CONFIG_ARCH_MSM8610) += msm8610-rumi.dtb \
	msm8610-v1-cdp.dtb \
	msm8610-v2-cdp.dtb \
	msm8610-v1-mtp.dtb \
	msm8610-v2-mtp.dtb \
	msm8610-sim.dtb \
	msm8610-v1-qrd-skuaa.dtb \
	msm8610-v1-qrd-skuab.dtb \
	msm8610-v2-qrd-skuaa.dtb \
	msm8610-v2-qrd-skuab.dtb
dtb-$(CONFIG_ARCH_FSM9900) += fsm9900-rumi.dtb \
	fsm9900-sim.dtb \
	fsm9900-cdp.dtb \
	fsm9900-mtp-v1.0.dtb \
	fsm9900-mtp-v1.1.dtb
dtb-$(CONFIG_ARCH_FSM9010) += fsm9010-rumi.dtb
dtb-$(CONFIG_ARCH_MDM9630) += mdm9630-sim.dtb \
	mdm9630-v1-cdp.dtb \
	mdm9630-v1-mtp.dtb \
	mdm9630-v2-cdp.dtb \
	mdm9630-v2-mtp.dtb \
	mdm9630-v2-tsmc-cdp.dtb \
	mdm9630-v2-gf-cdp.dtb  \
	mdm9630-v2-tsmc-mtp.dtb \
	mdm9630-v2-gf-mtp.dtb
dtb-$(CONFIG_ARCH_MSMZIRC) += msmzirc-sim.dtb \
	msmzirc-rumi.dtb \
	msmzirc-cdp.dtb \
	msmzirc-mtp.dtb
dtb-$(CONFIG_ARCH_MDMFERRUM) += mdmferrum-sim.dtb \
	mdmferrum-rumi.dtb \
	mdmferrum-cdp.dtb \
	mdmferrum-mtp.dtb
dtb-$(CONFIG_ARCH_APQ8084) += apq8084-v1.0-old-pwr-grid-liquid.dtb \
	apq8084-v1.1-old-pwr-grid-liquid.dtb \
	apq8084-v1.1-liquid.dtb \
	apq8084-v1.0-old-pwr-grid-smb1357-cdp.dtb \
	apq8084-v1.1-old-pwr-grid-smb1357-cdp.dtb \
	apq8084-v1.1-smb1357-cdp.dtb \
	apq8084-v1.0-old-pwr-grid-smb1357-mtp.dtb \
	apq8084-v1.1-old-pwr-grid-smb1357-mtp.dtb \
	apq8084-v1.1-smb1357-mtp.dtb \
	apq8084-v1.0-old-pwr-grid-smb1359-mdm9x35-alt-mtp.dtb \
	apq8084-v1.1-old-pwr-grid-smb1359-mdm9x35-alt-mtp.dtb \
	apq8084-v1.1-smb1359-mdm9x35-alt-mtp.dtb \
	apq8084-old-pwr-grid-smb349-cdp.dtb \
	apq8084-old-pwr-grid-smb349-mtp.dtb \
	apq8084-v1.0-old-pwr-grid-smb1357-mdm9x35-alt-rcm.dtb \
	apq8084-v1.1-old-pwr-grid-smb1357-mdm9x35-alt-rcm.dtb \
	apq8084-v1.1-smb1357-mdm9x35-alt-rcm.dtb \
	apq8084-v1.0-old-pwr-grid-smb1357-mdm9x35-mtp.dtb \
	apq8084-v1.1-old-pwr-grid-smb1357-mdm9x35-mtp.dtb \
	apq8084-v1.1-smb1357-mdm9x35-mtp.dtb \
	apq8084-v1.0-old-pwr-grid-smb1357-mdm9x35-cdp.dtb \
	apq8084-v1.1-old-pwr-grid-smb1357-mdm9x35-cdp.dtb \
	apq8084-v1.1-smb1357-mdm9x35-cdp.dtb \
	apq8084-v1.0-old-pwr-grid-mdm9x35-alt-cdp.dtb \
	apq8084-v1.1-old-pwr-grid-mdm9x35-alt-cdp.dtb \
	apq8084-v1.1-mdm9x35-alt-cdp.dtb \
	apq8084-v1.1-sbc.dtb
dtb-$(CONFIG_ARCH_MSM8994) += msm8994-v1-sim.dtb \
	msm8994-rumi.dtb \
	msm8994-v1-cdp.dtb \
	msm8994-pmi8994-pm8004-v1-cdp.dtb \
	msm8994-v1-mtp.dtb \
	msm8994-pmi8994-pm8004-v1-mtp.dtb \
	msm8994-v1-liquid.dtb \
	msm8994-v1-fluid.dtb \
	apq8094-v1-cdp.dtb \
	apq8094-v1-mtp.dtb \
	apq8094-v1-liquid.dtb \
	apq8094-v1-fluid.dtb \
	apq8094-v1-dragonboard.dtb \
	msm8994-v2-sim.dtb \
	msm8994-v2-cdp.dtb \
	msm8994-pmi8994-pm8004-v2-cdp.dtb \
	msm8994-v2-mtp.dtb \
	msm8994-pmi8994-pm8004-v2-mtp.dtb \
	msm8994-v2-liquid.dtb \
	msm8994-v2-fluid.dtb \
	apq8094-v2-cdp.dtb \
	apq8094-v2-mtp.dtb \
	apq8094-v2-liquid.dtb \
	apq8094-v2-fluid.dtb

dtb-$(CONFIG_ARCH_MSM8992) += msm8992-sim.dtb \
	msm8992-rumi.dtb

DTB_NAMES := $(subst $\",,$(CONFIG_BUILD_ARM_APPENDED_DTB_IMAGE_NAMES))
ifneq ($(DTB_NAMES),)
DTB_LIST := $(addsuffix .dtb,$(DTB_NAMES))
else
DTB_LIST := $(dtb-y)
endif

targets += dtbs
targets += $(addprefix ../, $(DTB_LIST))
endif

$(obj)/../../%.dtb: $(src)/%.dts FORCE
	$(call if_changed_dep,dtc)

dtbs: $(addprefix $(obj)/../../,$(DTB_LIST))

clean-files := *.dtb
