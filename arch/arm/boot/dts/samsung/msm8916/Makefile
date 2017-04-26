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
dtb-$(CONFIG_ARCH_MSMFERRUM) += msmferrum-sim.dtb \
	msmferrum-rumi.dtb \
	msmferrum-qrd-skua.dtb \
	msmferrum-cdp.dtb \
	msmferrum-mtp.dtb \
	msmferrum-1gb-qrd-skua.dtb \
	msmferrum-1gb-cdp.dtb \
	msmferrum-1gb-mtp.dtb
ifeq ($(CONFIG_ARCH_MSM8916),y)
	ifeq ($(CONFIG_SEC_A5_EUR_PROJECT),y)
		dtb-y += msm8916-sec-a5-eur-r02.dtb
		dtb-y += msm8916-sec-a5-eur-r03.dtb
		dtb-y += msm8916-sec-a5-eur-r04.dtb
		dtb-y += msm8916-sec-a5-eur-r05.dtb
		dtb-y += msm8916-sec-a5-eur-r06.dtb
		dtb-y += msm8916-sec-a5-eur-r07.dtb
		dtb-y += msm8916-sec-a5-eur-r08.dtb
	endif
	ifeq ($(CONFIG_SEC_A53G_EUR_PROJECT),y)
      dtb-y += msm8916-sec-a53g-eur-r03.dtb
      dtb-y += msm8916-sec-a53g-eur-r04.dtb
      dtb-y += msm8916-sec-a53g-eur-r05.dtb
      dtb-y += msm8916-sec-a53g-eur-r06.dtb
      dtb-y += msm8916-sec-a53g-eur-r07.dtb
      dtb-y += msm8916-sec-a53g-eur-r08.dtb
	endif
ifeq ($(CONFIG_SEC_E7_PROJECT),y)
	ifeq ($(CONFIG_MACH_E7_EUR_OPEN),y)
		dtb-y += msm8916-sec-e7-eur-r00.dtb
		dtb-y += msm8916-sec-e7-eur-r01.dtb
		dtb-y += msm8916-sec-e7-eur-r02.dtb
		dtb-y += msm8916-sec-e7-eur-r03.dtb
		dtb-y += msm8916-sec-e7-eur-r04.dtb
		dtb-y += msm8916-sec-e7-eur-r05.dtb
		dtb-y += msm8916-sec-e7-eur-r06.dtb
	else ifeq ($(CONFIG_MACH_E7_CHN_OPEN),y)
		dtb-y += msm8916-sec-e7-chnopen-r00.dtb
		dtb-y += msm8916-sec-e7-chnopen-r01.dtb
	else ifeq ($(CONFIG_MACH_E7_CHN_CTC),y)
		dtb-y += msm8916-sec-e7-ctc-r00.dtb
		dtb-y += msm8916-sec-e7-ctc-r01.dtb
	else ifeq ($(CONFIG_MACH_E73G_SEA_OPEN),y)
		dtb-y += msm8916-sec-e73g-sea-r01.dtb
		dtb-y += msm8916-sec-e73g-sea-r02.dtb
	else ifeq ($(CONFIG_MACH_E73G_EUR_OPEN),y)
		dtb-y += msm8916-sec-e73g-eur-r01.dtb
		dtb-y += msm8916-sec-e73g-eur-r02.dtb
		dtb-y += msm8916-sec-e73g-eur-r03.dtb
		dtb-y += msm8916-sec-e73g-eur-r04.dtb
		dtb-y += msm8916-sec-e73g-eur-r05.dtb
		dtb-y += msm8916-sec-e73g-eur-r06.dtb
	endif
endif
ifeq ($(CONFIG_SEC_GT58_PROJECT),y)
	ifeq ($(CONFIG_MACH_GT58_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt58lte-eur-r00.dtb
		dtb-y += msm8916-sec-gt58lte-eur-r01.dtb
	else ifeq ($(CONFIG_MACH_GT58WIFI_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt58wifi-eur-r00.dtb
		dtb-y += msm8916-sec-gt58wifi-eur-r01.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE8_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt5note8lte-eur-r00.dtb
		dtb-y += msm8916-sec-gt5note8lte-eur-r01.dtb
		dtb-y += msm8916-sec-gt5note8lte-eur-r02.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE8WIFI_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt5note8wifi-eur-r02.dtb
	else ifeq ($(CONFIG_MACH_GT58_USA_VZW),y)
		dtb-y += msm8916-sec-gt58lte-vzw-r01.dtb
	else ifeq ($(CONFIG_MACH_GT58_USA_TMO),y)
		dtb-y += msm8916-sec-gt58lte-tmo-r05.dtb
		dtb-y += msm8916-sec-gt58lte-tmo-r06.dtb
	else ifeq ($(CONFIG_MACH_GT58_CAN_BMC),y)
		dtb-y += msm8916-sec-gt58lte-canbmc-r05.dtb
		dtb-y += msm8916-sec-gt58lte-canbmc-r06.dtb
	else ifeq ($(CONFIG_MACH_GT58_SEA_XSA),y)
		dtb-y += msm8916-sec-gt58lte-aus-r00.dtb
	endif
endif

ifeq ($(CONFIG_SEC_GT510_PROJECT),y)
	ifeq ($(CONFIG_MACH_GT510_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt510lte-eur-r00.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r01.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r02.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r03.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r04.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r06.dtb
		dtb-y += msm8916-sec-gt510lte-eur-r07.dtb
	else ifeq ($(CONFIG_MACH_GT510WIFI_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt510wifi-eur-r00.dtb
		dtb-y += msm8916-sec-gt510wifi-eur-r03.dtb
		dtb-y += msm8916-sec-gt510wifi-eur-r06.dtb
		dtb-y += msm8916-sec-gt510wifi-eur-r07.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE10_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt5note10lte-eur-r00.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r01.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r02.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r03.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r04.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r05.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r06.dtb
		dtb-y += msm8916-sec-gt5note10lte-eur-r07.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE10WIFI_EUR_OPEN),y)
		dtb-y += msm8916-sec-gt5note10wifi-eur-r03.dtb
		dtb-y += msm8916-sec-gt5note10wifi-eur-r04.dtb
		dtb-y += msm8916-sec-gt5note10wifi-eur-r05.dtb
		dtb-y += msm8916-sec-gt5note10wifi-eur-r06.dtb
		dtb-y += msm8916-sec-gt5note10wifi-eur-r07.dtb
	else ifeq ($(CONFIG_MACH_GT510_USA_VZW),y)
		dtb-y += msm8916-sec-gt510lte-vzw-r03.dtb
	else ifeq ($(CONFIG_MACH_GT510_CHN_OPEN),y)
		dtb-y += msm8916-sec-gt510lte-chnopen-r04.dtb
		dtb-y += msm8916-sec-gt510lte-chnopen-r07.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE10_CHN_OPEN),y)
		dtb-y += msm8916-sec-gt5note10lte-chn-r04.dtb
		dtb-y += msm8916-sec-gt5note10lte-chn-r05.dtb
		dtb-y += msm8916-sec-gt5note10lte-chn-r07.dtb
	else ifeq ($(CONFIG_MACH_GT5NOTE10_KOR_OPEN),y)
		dtb-y += msm8916-sec-gt5note10lte-kor-r00.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r01.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r02.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r03.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r04.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r05.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r06.dtb
		dtb-y += msm8916-sec-gt5note10lte-kor-r07.dtb
	endif
endif

ifeq ($(CONFIG_SEC_SIGNUM_PROJECT),y)
	dtb-y += msm8916-sec-signumlte-eur-r00.dtb
endif

ifeq ($(CONFIG_SEC_O7_PROJECT),y)
	ifeq ($(CONFIG_MACH_O7_CHN_OPEN),y)
		dtb-y += msm8916-sec-o7lte-chn-r00.dtb
		dtb-y += msm8916-sec-o7lte-chn-r01.dtb
		dtb-y += msm8916-sec-o7lte-chn-r02.dtb
		dtb-y += msm8916-sec-o7lte-chn-r03.dtb
	else ifeq ($(CONFIG_MACH_O7_SWA_OPEN),y)
		dtb-y += msm8916-sec-o7lte-swa-r01.dtb
		dtb-y += msm8916-sec-o7lte-swa-r02.dtb
		dtb-y += msm8916-sec-o7lte-swa-r03.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J3_PROJECT),y)
	ifeq ($(CONFIG_MACH_J3LTE_CHN_CTC),y)
		dtb-y += msm8916-sec-j3lte-chnctc-r00.dtb
		dtb-y += msm8916-sec-j3lte-chnctc-r02.dtb
		dtb-y += msm8916-sec-j3lte-chnctc-r05.dtb
	else ifeq ($(CONFIG_MACH_J3LTE_USA_SPR),y)
		dtb-y += msm8916-sec-j3lte-spr-r00.dtb
		dtb-y += msm8916-sec-j3lte-spr-r01.dtb
	else ifeq ($(CONFIG_MACH_J3LTE_USA_VZW),y)
		dtb-y += msm8916-sec-j3lte-vzw-r00.dtb
		dtb-y += msm8916-sec-j3lte-vzw-r02.dtb
		dtb-y += msm8916-sec-j3lte-vzw-r03.dtb
	else ifeq ($(CONFIG_MACH_J3LTE_USA_USC),y)
		dtb-y += msm8916-sec-j3lte-usc-r00.dtb
	else ifeq ($(CONFIG_MACH_J3LTE_KOR_OPEN),y)
		dtb-y += msm8916-sec-j3lte-kor-r03.dtb
	endif
endif

ifeq ($(CONFIG_SEC_E5_PROJECT),y)
	ifeq ($(CONFIG_MACH_E5_EUR_OPEN),y)
		dtb-y += msm8916-sec-e5-r00.dtb
		dtb-y += msm8916-sec-e5-r01.dtb
		dtb-y += msm8916-sec-e5-r02.dtb
		dtb-y += msm8916-sec-e5-r03.dtb
		dtb-y += msm8916-sec-e5-r04.dtb
	else ifeq ($(CONFIG_MACH_E5_USA_TFN),y)
		dtb-y += msm8916-sec-e5-tfn-r00.dtb
		dtb-y += msm8916-sec-e5-tfn-r01.dtb
		dtb-y += msm8916-sec-e5-tfn-r02.dtb
	else ifeq ($(CONFIG_MACH_E53G_EUR_OPEN),y)
		dtb-y += msm8916-sec-e53g-r03.dtb
		dtb-y += msm8916-sec-e53g-r04.dtb
		dtb-y += msm8916-sec-e53g-r06.dtb
		dtb-y += msm8916-sec-e53g-r07.dtb
		dtb-y += msm8916-sec-e53g-r08.dtb
		dtb-y += msm8916-sec-e53g-r09.dtb
		dtb-y += msm8916-sec-e53g-r10.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A3_PROJECT),y)
	ifeq ($(CONFIG_MACH_A3U_EUR_OPEN),y)
		dtb-y += msm8916-sec-a3u-eur-r01.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A3_EUR_PROJECT),y)
	ifeq ($(CONFIG_MACH_A3_EUR_OPEN),y)
		dtb-y += msm8916-sec-a3-eur-r00.dtb
		dtb-y += msm8916-sec-a3-eur-r01.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A33G_EUR_PROJECT),y)
	ifeq ($(CONFIG_MACH_A33G_EUR_OPEN),y)
		dtb-y += msm8916-sec-a33g-eur-r00.dtb
		dtb-y += msm8916-sec-a33g-eur-r01.dtb
		dtb-y += msm8916-sec-a33g-eur-r03.dtb
		dtb-y += msm8916-sec-a33g-eur-r04.dtb
	endif
endif

ifeq ($(CONFIG_SEC_MATISSEVE_PROJECT),y)
	ifeq ($(CONFIG_MACH_MATISSEVEWIFI_EUR_OPEN),y)
		dtb-y += msm8916-sec-matissevewifi-eur-r00.dtb
		dtb-y += msm8916-sec-matissevewifi-eur-r02.dtb
	endif
endif

ifeq ($(CONFIG_SEC_A5_PROJECT),y)
	ifeq ($(CONFIG_MACH_A5U_EUR_OPEN),y)
		dtb-y += msm8916-sec-a5u-eur-r01.dtb
	else ifeq ($(CONFIG_MACH_A5U_CAN_BMC),y)
		dtb-y += msm8916-sec-a5u-canbmc-r01.dtb
	else ifeq ($(CONFIG_MACH_A5U_KOR_OPEN),y)
		dtb-y += msm8916-sec-a5u-kor-r00.dtb
		dtb-y += msm8916-sec-a5u-kor-r02.dtb
	endif
endif
ifeq ($(CONFIG_SEC_GTEL_PROJECT),y)
	ifeq ($(CONFIG_MACH_GTEL_USA_VZW),y)
		dtb-y += msm8916-sec-gtel-vzw-r00.dtb
		dtb-y += msm8916-sec-gtel-vzw-r01.dtb
		dtb-y += msm8916-sec-gtel-vzw-r02.dtb
		dtb-y += msm8916-sec-gtel-vzw-r03.dtb
		dtb-y += msm8916-sec-gtel-vzw-r04.dtb
		dtb-y += msm8916-sec-gtel-vzw-r05.dtb
	else ifeq ($(CONFIG_MACH_GTELWIFI_USA_OPEN),y)
		dtb-y += msm8916-sec-gtelwifi-usa-r00.dtb
		dtb-y += msm8916-sec-gtelwifi-usa-r01.dtb
		dtb-y += msm8916-sec-gtelwifi-usa-r02.dtb
		dtb-y += msm8916-sec-gtelwifi-usa-r03.dtb
		dtb-y += msm8916-sec-gtelwifi-usa-r04.dtb
		dtb-y += msm8916-sec-gtelwifi-usa-r05.dtb
	endif
endif
ifeq ($(CONFIG_SEC_GTES_PROJECT),y)
	ifeq ($(CONFIG_MACH_GTES_USA_SPR),y)
		dtb-y += msm8916-sec-gteslte-spr-r00.dtb
		dtb-y += msm8916-sec-gteslte-spr-r01.dtb
		dtb-y += msm8916-sec-gteslte-spr-r04.dtb
	endif
	ifeq ($(CONFIG_MACH_GTES_USA_USC),y)
		dtb-y += msm8916-sec-gteslte-usc-r00.dtb
		dtb-y += msm8916-sec-gteslte-usc-r01.dtb
		dtb-y += msm8916-sec-gteslte-usc-r04.dtb
	endif
endif

endif

ifeq ($(CONFIG_SEC_FORTUNA_PROJECT),y)
	ifeq ($(CONFIG_MACH_FORTUNA_TFN),y)
		dtb-y += msm8916-sec-fortuna-tfn-r00.dtb
		dtb-y += msm8916-sec-fortuna-tfn-r02.dtb
	else ifeq ($(CONFIG_MACH_FORTUNA_USC),y)
		dtb-y += msm8916-sec-fortuna-usc-r00.dtb
		dtb-y += msm8916-sec-fortuna-usc-r01.dtb
		dtb-y += msm8916-sec-fortuna-usc-r03.dtb
	else ifeq ($(CONFIG_MACH_FORTUNA_TMO),y)
                dtb-y += msm8916-sec-fortuna-tmo-r00.dtb
                dtb-y += msm8916-sec-fortuna-tmo-r02.dtb
	else ifeq ($(CONFIG_MACH_FORTUNA_SPR),y)
                dtb-y += msm8916-sec-fortuna-spr-r00.dtb
	else ifeq ($(CONFIG_MACH_FORTUNA_ACG),y)
                dtb-y += msm8916-sec-fortuna-acg-r00.dtb
        endif

endif

ifeq ($(CONFIG_SEC_ROSSA_PROJECT),y)
	ifeq ($(CONFIG_MACH_ROSSA_TMO),y)
		dtb-y += msm8916-sec-rossatmo-r00.dtb
	endif
endif

ifeq ($(CONFIG_SEC_XCOVER3_PROJECT),y)
	ifeq ($(CONFIG_MACH_XCOVER3_DCM),y)
		dtb-y += msm8916-sec-xcover3-dcm-r01.dtb
		dtb-y += msm8916-sec-xcover3-dcm-r02.dtb
		dtb-y += msm8916-sec-xcover3-dcm-r03.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J1_PROJECT),y)
	ifeq ($(CONFIG_MACH_J1_VZW),y)
		dtb-y += msm8916-sec-j1vzw-r00.dtb
		dtb-y += msm8916-sec-j1vzw-r02.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J1X_PROJECT),y)
	ifeq ($(CONFIG_MACH_J1XLTE_USA_SPR),y)
		dtb-y += msm8916-sec-j1xlte-spr-r00.dtb
	else ifeq ($(CONFIG_MACH_J1XLTE_USA_TFNVZW),y)
		dtb-y += msm8916-sec-j1xlte-tfnvzw-r00.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J5X_PROJECT),y)
	ifeq ($(CONFIG_MACH_J5XLTE_CHN_CMCC),y)
		dtb-y += msm8916-sec-j5xlte-chn-r00.dtb
	else ifeq ($(CONFIG_MACH_J5XLTE_EUR_OPEN),y)
		dtb-y += msm8916-sec-j5xlte-eur-r00.dtb
		dtb-y += msm8916-sec-j5xlte-eur-r01.dtb
		dtb-y += msm8916-sec-j5xlte-eur-r02.dtb
		dtb-y += msm8916-sec-j5xlte-eur-r04.dtb
	else ifeq ($(CONFIG_MACH_J5XLTE_KOR_OPEN),y)
		dtb-y += msm8916-sec-j5xlte-kor-r01.dtb
		dtb-y += msm8916-sec-j5xlte-kor-r02.dtb
		dtb-y += msm8916-sec-j5xlte-kor-r04.dtb
	else ifeq ($(CONFIG_MACH_J5X3G_MEA_JV),y)
		dtb-y += msm8916-sec-j5x3g-mea-r00.dtb
		dtb-y += msm8916-sec-j5x3g-mea-r01.dtb
		dtb-y += msm8916-sec-j5x3g-mea-r02.dtb
		dtb-y += msm8916-sec-j5x3g-mea-r04.dtb
	else ifeq ($(CONFIG_MACH_J5XLTE_MEA_JV),y)
		dtb-y += msm8916-sec-j5xlte-mea-r00.dtb
		dtb-y += msm8916-sec-j5xlte-mea-r01.dtb
		dtb-y += msm8916-sec-j5xlte-mea-r02.dtb
		dtb-y += msm8916-sec-j5xlte-mea-r04.dtb
	else ifeq ($(CONFIG_MACH_J5XLTE_MEA_TUR),y)
		dtb-y += msm8916-sec-j5xlte-tur-r00.dtb
		dtb-y += msm8916-sec-j5xlte-tur-r01.dtb
		dtb-y += msm8916-sec-j5xlte-tur-r02.dtb
		dtb-y += msm8916-sec-j5xlte-tur-r04.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J5_PROJECT),y)
	ifeq ($(CONFIG_MACH_J5LTE_EUR_OPEN),y)
		dtb-y += msm8916-sec-j5lte-eur-r00.dtb
		dtb-y += msm8916-sec-j5lte-eur-r01.dtb
		dtb-y += msm8916-sec-j5lte-eur-r05.dtb
	else ifeq ($(CONFIG_MACH_J53G_EUR_OPEN),y)
		dtb-y += msm8916-sec-j53g-eur-r00.dtb
		dtb-y += msm8916-sec-j53g-eur-r01.dtb
		dtb-y += msm8916-sec-j53g-eur-r02.dtb
		dtb-y += msm8916-sec-j53g-eur-r05.dtb
	else ifeq ($(CONFIG_MACH_J5LTE_CHN_CMCC),y)
		dtb-y += msm8916-sec-j5lte-chn-r00.dtb
		dtb-y += msm8916-sec-j5lte-chn-r01.dtb
	else ifeq ($(CONFIG_MACH_J5LTE_KOR_OPEN),y)
		dtb-y += msm8916-sec-j5lte-kor-r01.dtb
		dtb-y += msm8916-sec-j5lte-kor-r05.dtb
	else ifeq ($(CONFIG_MACH_J5LTE_LTN_DTV),y)
		dtb-y += msm8916-sec-j5lte-ltn-dtv-r04.dtb
		dtb-y += msm8916-sec-j5lte-ltn-dtv-r05.dtb
	else ifeq ($(CONFIG_MACH_J5YLTE_SEA_NZC),y)
		dtb-y += msm8916-sec-j5ylte-sea-r00.dtb
		dtb-y += msm8916-sec-j5ylte-sea-r01.dtb
		dtb-y += msm8916-sec-j5ylte-sea-r05.dtb
	endif
endif

ifeq ($(CONFIG_SEC_J5N_PROJECT),y)
	ifeq ($(CONFIG_MACH_J5NLTE_EUR_OPEN),y)
		dtb-y += msm8916-sec-j5nlte-eur-r04.dtb
		dtb-y += msm8916-sec-j5nlte-eur-r05.dtb
	endif
endif

ifeq ($(CONFIG_SEC_ON7N_PROJECT),y)
	ifeq ($(CONFIG_MACH_ON7NLTE_KOR_SKT),y)
		dtb-y += msm8916-sec-on7nlte-kor-r00.dtb
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
	mdmferrum-rumi.dtb
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
