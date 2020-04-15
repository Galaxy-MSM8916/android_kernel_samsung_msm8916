#!/bin/bash
for variant in a33g a3_eur a3u a5_chnctc a5_chnopen fortuna3g_eur fortuna_sea fortunave3g_eur gt510lte_eur gt510wifi_eur gt58lte_tmo gt58wifi_eur gtelwifi_usa gtes_spr j53g_eur j5lte_chncmcc j5lte_eur j5nlte_eur j5xlte_eur j5xnlte_eur o7lte_swaopen rossa_spr serranove3g_eur serranovelte_eur
do
	cat "./arch/arm/configs/msm8916_sec_defconfig" >> "./arch/arm/configs/${variant}_defconfig"
	cat "./arch/arm/configs/msm8916_sec_${variant}_defconfig" >> "./arch/arm/configs/${variant}_defconfig"
	make O=out ARCH=arm ${variant}_defconfig
	rm "./arch/arm/configs/${variant}_defconfig"
	cp "./out/.config" "./arch/arm/configs/${variant}_defconfig"
	rm "./out/.config"
done
