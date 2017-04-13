#!/bin/bash
# MSM8916 KK kernel build script v0.5

SELINUX_DEFCONFIG=selinux_defconfig
SELINUX_LOG_DEFCONFIG=selinux_log_defconfig

BUILD_COMMAND=$1
if [ "$BUILD_COMMAND" == "a3u_eur" ]; then
	PRODUCT_NAME=a3ulte;
	SIGN_MODEL=SM-A300FU_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "a5u_eur" ]; then
	PRODUCT_NAME=a5ulte;
	SIGN_MODEL=SM-A500FU_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "e5_eur" ]; then
	PRODUCT_NAME=e5lte;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "e53g_eur" ]; then
	PRODUCT_NAME=e53g;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "e7_eur" ]; then
	PRODUCT_NAME=e7lte;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "e7_chn_open" ]; then
	PRODUCT_NAME=e7ltezm;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "e7_ctc" ]; then
	PRODUCT_NAME=e7ltectc;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "e73g_sea" ]; then
	PRODUCT_NAME=e73gdx;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt58lte_eur" ]; then
	PRODUCT_NAME=gt58lte;
	SIGN_MODEL=SM-T355_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "gt58lte_aus" ]; then
	PRODUCT_NAME=gt58ltedo;
	SIGN_MODEL=SM-T355Y_SEA_XSA_ROOT0;
elif [ "$BUILD_COMMAND" == "gt58lte_vzw" ]; then
	PRODUCT_NAME=gt58ltevzw;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt58lte_tmo" ]; then
	PRODUCT_NAME=gt58ltetmo;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt58wifi_eur" ]; then
	PRODUCT_NAME=gt58wifi;
	SIGN_MODEL=SM-T350_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "gt510lte_eur" ]; then
	PRODUCT_NAME=gt510lte;
	SIGN_MODEL=SM-T555_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "gt510lte_vzw" ]; then
	PRODUCT_NAME=gt510ltevzw;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt510lte_chnopen" ]; then
	PRODUCT_NAME=gt510ltechnopen;
	SIGN_MODEL=SM-T555C_CHN_CHN_ROOT0
elif [ "$BUILD_COMMAND" == "gt510wifi_eur" ]; then
	PRODUCT_NAME=gt510wifi;
	SIGN_MODEL=SM-T550_EUR_XX_ROOT0;
elif [ "$BUILD_COMMAND" == "gt5note8lte_eur" ]; then
	PRODUCT_NAME=gt5note8lte;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt5note8wifi_eur" ]; then
	PRODUCT_NAME=gt5note8wifi;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt5note10lte_eur" ]; then
	PRODUCT_NAME=gt5note10lte;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt5note10lte_chn" ]; then
	PRODUCT_NAME=gt5note10ltechn;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt5note103g_eur" ]; then
	PRODUCT_NAME=gt5note103g;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gt5note10wifi_eur" ]; then
	PRODUCT_NAME=gt5note10wifi;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "signumlte_eur" ]; then
	PRODUCT_NAME=signumltexx;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "fortuna_tfn" ]; then
	PRODUCT_NAME=gprimeltetfnvzw;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "fortuna_usc" ]; then
	PRODUCT_NAME=gprimelteusc;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "fortuna_tmo" ]; then
	PRODUCT_NAME=gprimeltetmo;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "fortuna_spr" ]; then
	PRODUCT_NAME=gprimeltespr;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "rossa_tmo" ]; then
	PRODUCT_NAME=rossaltetmo;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "xcover3_dcm" ]; then
	PRODUCT_NAME=xcover3ltedcm;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j1_vzw" ]; then
	PRODUCT_NAME=j1qltevzw;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gtes_spr" ]; then
	PRODUCT_NAME=gtesqltespr;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "gtes_usc" ]; then
	PRODUCT_NAME=gtesqlteusc;
	SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "o7lte_chnopen" ]; then
        PRODUCT_NAME=o7ltezc;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "o7lte_swaopen" ]; then
        PRODUCT_NAME=o7ltedd;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j3lte_chnctc" ]; then
        PRODUCT_NAME=j3ltechnctc;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j3lte_tw" ]; then
        PRODUCT_NAME=j3ltetw;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j3lte_spr" ]; then
        PRODUCT_NAME=j3ltespr;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j3lte_vzw" ]; then
        PRODUCT_NAME=j3ltevzw;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j3lte_usc" ]; then
        PRODUCT_NAME=j3lteusc;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j1xqlte_spr" ]; then
        PRODUCT_NAME=j1xqltespr;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j1xqlte_tfnvzw" ]; then
        PRODUCT_NAME=j1xqltetfnvzw;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j5lte_eur" ]; then
        PRODUCT_NAME=j5xlte;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "j5x3g_mea" ]; then
        PRODUCT_NAME=j5x3gjv;
        SIGN_MODEL=
elif [ "$BUILD_COMMAND" == "on7nlte_kor" ]; then
        PRODUCT_NAME=on7nlteskt;
        SIGN_MODEL=
else
#default product
        PRODUCT_NAME=$BUILD_COMMAND;
	SIGN_MODEL=
fi

BUILD_WHERE=$(pwd)
BUILD_KERNEL_DIR=$BUILD_WHERE
BUILD_ROOT_DIR=$BUILD_KERNEL_DIR/../..
BUILD_KERNEL_OUT_DIR=$BUILD_ROOT_DIR/android/out/target/product/$PRODUCT_NAME/obj/KERNEL_OBJ
PRODUCT_OUT=$BUILD_ROOT_DIR/android/out/target/product/$PRODUCT_NAME


SECURE_SCRIPT=$BUILD_ROOT_DIR/buildscript/tools/signclient.jar
BUILD_CROSS_COMPILE=$BUILD_ROOT_DIR/android/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
BUILD_JOB_NUMBER=`grep processor /proc/cpuinfo|wc -l`

# Default Python version is 2.7
mkdir -p bin
ln -sf /usr/bin/python2.7 ./bin/python
export PATH=$(pwd)/bin:$PATH

KERNEL_DEFCONFIG=msm8916_sec_defconfig
DEBUG_DEFCONFIG=msm8916_sec_eng_defconfig
DMVERITY_DEFCONFIG=dmverity_defconfig

while getopts "w:t:" flag; do
	case $flag in
		w)
			BUILD_OPTION_HW_REVISION=$OPTARG
			echo "-w : "$BUILD_OPTION_HW_REVISION""
			;;
		t)
			TARGET_BUILD_VARIANT=$OPTARG
			echo "-t : "$TARGET_BUILD_VARIANT""
			;;
		*)
			echo "wrong 2nd param : "$OPTARG""
			exit -1
			;;
	esac
done

shift $((OPTIND-1))

BUILD_COMMAND=$1
SECURE_OPTION=
SEANDROID_OPTION=
if [ "$2" == "-B" ]; then
	SECURE_OPTION=$2
elif [ "$2" == "-E" ]; then
	SEANDROID_OPTION=$2
else
	NO_JOB=
fi

if [ "$3" == "-B" ]; then
	SECURE_OPTION=$3
elif [ "$3" == "-E" ]; then
	SEANDROID_OPTION=$3
else
	NO_JOB=
fi

MODEL=${BUILD_COMMAND%%_*}
TEMP=${BUILD_COMMAND#*_}
REGION=${TEMP%%_*}
CARRIER=${TEMP##*_}

VARIANT=k${CARRIER}
PROJECT_NAME=${VARIANT}
VARIANT_DEFCONFIG=msm8916_sec_${MODEL}_${CARRIER}_defconfig

CERTIFICATION=NONCERT

case $1 in
		clean)
		echo "Not support... remove kernel out directory by yourself"
		exit 1
		;;
		
		*)
		
		BOARD_KERNEL_BASE=0x80000000
		BOARD_KERNEL_PAGESIZE=2048
		BOARD_KERNEL_TAGS_OFFSET=0x01E00000
		BOARD_RAMDISK_OFFSET=0x02000000
		#BOARD_KERNEL_CMDLINE="console=ttyHSL0,115200,n8 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x37 ehci-hcd.park=3"
		BOARD_KERNEL_CMDLINE="console=ttyHSL0,115200,n8 androidboot.console=ttyHSL0 androidboot.hardware=qcom user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 androidboot.bootdevice=7824900.sdhci"
#		BOARD_KERNEL_CMDLINE="console=ttyHSL0,115200,n8 androidboot.hardware=qcom androidboot.bootdevice=soc.0/7824900.sdhci user_debug=31 msm_rtb.filter=0x3F ehci-hcd.park=3 video=vfb:640x400,bpp=32,memsize=3072000 earlyprintk"
		mkdir -p $BUILD_KERNEL_OUT_DIR
		;;

esac

KERNEL_ZIMG=$BUILD_KERNEL_OUT_DIR/arch/arm/boot/zImage
DTC=$BUILD_KERNEL_OUT_DIR/scripts/dtc/dtc

FUNC_CLEAN_DTB()
{
	if ! [ -d $BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts ] ; then
		echo "no directory : "$BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts""
	else
		echo "rm files in : "$BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts/*.dtb""
		rm $BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts/*.dtb
	fi
}

INSTALLED_DTIMAGE_TARGET=${PRODUCT_OUT}/dt.img
DTBTOOL=$BUILD_KERNEL_DIR/tools/dtbTool

FUNC_BUILD_DTIMAGE_TARGET()
{
	echo ""
	echo "================================="
	echo "START : FUNC_BUILD_DTIMAGE_TARGET"
	echo "================================="
	echo ""
	echo "DT image target : $INSTALLED_DTIMAGE_TARGET"
	
	if ! [ -e $DTBTOOL ] ; then
		if ! [ -d $BUILD_ROOT_DIR/android/out/host/linux-x86/bin ] ; then
			mkdir -p $BUILD_ROOT_DIR/android/out/host/linux-x86/bin
		fi
		cp $BUILD_ROOT_DIR/kernel/tools/dtbTool $DTBTOOL
	fi

	echo "$DTBTOOL -o $INSTALLED_DTIMAGE_TARGET -s $BOARD_KERNEL_PAGESIZE \
		-p $BUILD_KERNEL_OUT_DIR/scripts/dtc/ $BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts/"
		$DTBTOOL -o $INSTALLED_DTIMAGE_TARGET -s $BOARD_KERNEL_PAGESIZE \
		-p $BUILD_KERNEL_OUT_DIR/scripts/dtc/ $BUILD_KERNEL_OUT_DIR/arch/arm/boot/dts/

	chmod a+r $INSTALLED_DTIMAGE_TARGET

	echo ""
	echo "================================="
	echo "END   : FUNC_BUILD_DTIMAGE_TARGET"
	echo "================================="
	echo ""
}

FUNC_BUILD_KERNEL()
{
	echo ""
	echo "=============================================="
	echo "START : FUNC_BUILD_KERNEL"
	echo "=============================================="
	echo ""
	echo "build project="$PROJECT_NAME""
	echo "build common config="$KERNEL_DEFCONFIG ""
	echo "build variant config="$VARIANT_DEFCONFIG ""
	echo "build secure option="$SECURE_OPTION ""
	echo "build SEANDROID option="$SEANDROID_OPTION ""

        if [ "$BUILD_COMMAND" == "" ]; then
                SECFUNC_PRINT_HELP;
                exit -1;
        fi

	FUNC_CLEAN_DTB

	make -C $BUILD_KERNEL_DIR O=$BUILD_KERNEL_OUT_DIR -j$BUILD_JOB_NUMBER ARCH=arm \
			CROSS_COMPILE=$BUILD_CROSS_COMPILE \
			$KERNEL_DEFCONFIG VARIANT_DEFCONFIG=$VARIANT_DEFCONFIG \
			DEBUG_DEFCONFIG=$DEBUG_DEFCONFIG \
			SELINUX_DEFCONFIG=$SELINUX_DEFCONFIG \
			SELINUX_LOG_DEFCONFIG=$SELINUX_LOG_DEFCONFIG || exit -1

	make -C $BUILD_KERNEL_DIR O=$BUILD_KERNEL_OUT_DIR -j$BUILD_JOB_NUMBER ARCH=arm \
			CROSS_COMPILE=$BUILD_CROSS_COMPILE || exit -1

	FUNC_BUILD_DTIMAGE_TARGET
	
	echo ""
	echo "================================="
	echo "END   : FUNC_BUILD_KERNEL"
	echo "================================="
	echo ""
}

FUNC_MKBOOTIMG()
{
	echo ""
	echo "==================================="
	echo "START : FUNC_MKBOOTIMG"
	echo "==================================="
	echo ""
	MKBOOTIMGTOOL=$BUILD_ROOT_DIR/android/kernel/tools/mkbootimg

	if ! [ -e $MKBOOTIMGTOOL ] ; then
		if ! [ -d $BUILD_ROOT_DIR/android/out/host/linux-x86/bin ] ; then
			mkdir -p $BUILD_ROOT_DIR/android/out/host/linux-x86/bin
		fi
		cp $BUILD_ROOT_DIR/anroid/kernel/tools/mkbootimg $MKBOOTIMGTOOL
	fi

	echo "Making boot.img ..."
	echo "	$MKBOOTIMGTOOL --kernel $KERNEL_ZIMG \
			--ramdisk $PRODUCT_OUT/ramdisk.img \
			--output $PRODUCT_OUT/boot.img \
			--cmdline "$BOARD_KERNEL_CMDLINE" \
			--base $BOARD_KERNEL_BASE \
			--pagesize $BOARD_KERNEL_PAGESIZE \
			--ramdisk_offset $BOARD_RAMDISK_OFFSET \
			--tags_offset $BOARD_KERNEL_TAGS_OFFSET \
			--dt $INSTALLED_DTIMAGE_TARGET"
			
	$MKBOOTIMGTOOL --kernel $KERNEL_ZIMG \
			--ramdisk $PRODUCT_OUT/ramdisk.img \
			--output $PRODUCT_OUT/boot.img \
			--cmdline "$BOARD_KERNEL_CMDLINE" \
			--base $BOARD_KERNEL_BASE \
			--pagesize $BOARD_KERNEL_PAGESIZE \
			--ramdisk_offset $BOARD_RAMDISK_OFFSET \
			--tags_offset $BOARD_KERNEL_TAGS_OFFSET \
			--dt $INSTALLED_DTIMAGE_TARGET

	if [ "$SEANDROID_OPTION" == "-E" ] ; then
		FUNC_SEANDROID
	fi

	if [ "$SECURE_OPTION" == "-B" ]; then
		FUNC_SECURE_SIGNING
	fi

	cd $PRODUCT_OUT
	tar cvf boot_${MODEL}_${CARRIER}_${CERTIFICATION}.tar boot.img

	cd $BUILD_ROOT_DIR
	if ! [ -d output ] ; then
		mkdir -p output
	fi

        echo ""
	echo "================================================="
        echo "-->Note, copy to $BUILD_TOP_DIR/../output/ directory"
	echo "================================================="
	cp $PRODUCT_OUT/boot_${MODEL}_${CARRIER}_${CERTIFICATION}.tar $BUILD_ROOT_DIR/output/boot_${MODEL}_${CARRIER}_${CERTIFICATION}.tar || exit -1
        cd ~

	echo ""
	echo "==================================="
	echo "END   : FUNC_MKBOOTIMG"
	echo "==================================="
	echo ""
}

FUNC_SEANDROID()
{
	echo -n "SEANDROIDENFORCE" >> $PRODUCT_OUT/boot.img
}

FUNC_SECURE_SIGNING()
{
	echo "java -jar $SECURE_SCRIPT -model $SIGN_MODEL -runtype ss_openssl_sha -input $PRODUCT_OUT/boot.img -output $PRODUCT_OUT/signed_boot.img"
	openssl dgst -sha256 -binary $PRODUCT_OUT/boot.img > sig_32
	java -jar $SECURE_SCRIPT -runtype ss_openssl_sha -model $SIGN_MODEL -input sig_32 -output sig_256
	cat $PRODUCT_OUT/boot.img sig_256 > $PRODUCT_OUT/signed_boot.img

	mv -f $PRODUCT_OUT/boot.img $PRODUCT_OUT/unsigned_boot.img
	mv -f $PRODUCT_OUT/signed_boot.img $PRODUCT_OUT/boot.img

	CERTIFICATION=CERT
}

SECFUNC_PRINT_HELP()
{
	echo -e '\E[33m'
	echo "Help"
	echo "$0 \$1 \$2 \$3"
	echo "  \$1 : "
	echo "	for A3ULTE_EUR_OPEN use a3u_eur"
	echo "	for A5ULTE_EUR_OPEN use a5u_eur"
	echo "	for E5LTE_EUR_OPEN use e5_eur"
	echo "	for E53G_EUR_OPEN use e53g_eur"
	echo "	for E7LTE_EUR_OPEN use e7_eur"
	echo "	for E7LTE_CHNOPEN use e7_chn_open"
	echo "	for E7LTE_CTC use e7_ctc"
	echo "	for E73G_SEA_OPEN use e73g_sea"
	echo "	for GT58LTE_EUR_OPEN use gt58lte_eur"
	echo "	for GT58LTE_SEA_XSA use gt58lte_aus"
	echo "	for GT58LTE_USA_VZW use gt58lte_vzw"
	echo "	for GT58LTE_USA_TMO use gt58lte_tmo"
	echo "	for GT58WIFI_EUR_OPEN use gt58wifi_eur"
	echo "	for GT510LTE_EUR_OPEN use gt510lte_eur"
	echo "	for GT510LTE_USA_VZW use gt510lte_vzw"
	echo "	for GT510LTE_CHN_OPEN use gt510lte_chnopen"
	echo "	for GT510WIFI_EUR_OPEN use gt510wifi_eur"
	echo "	for GT5NOTE8LTE_EUR_OPEN use gt5note8lte_eur"
	echo "	for GT5NOTE8WIFI_EUR_OPEN use gt5note8wifi_eur"
	echo "	for GT5NOTE10LTE_EUR_OPEN use gt5note10lte_eur"
	echo "	for GT5NOTE10LTE_CHN_OPEN use gt5note10lte_chn"
	echo "	for GT5NOTE103G_EUR_OPEN use gt5note103g_eur"
	echo "	for GT5NOTE103G_EUR_OPEN use gt5note10wifi_eur"
	echo "	for SIGNUMLTE_EUR_OPEN use signumlte_eur"
	echo "	for FORTUNA_USA_TFN use fortuna_tfn"
	echo "	for FORTUNA_USA_USC use fortuna_usc"
	echo "	for FORTUNA_USA_TMO use fortuna_tmo"
	echo "	for FORTUNA_USA_SPR use fortuna_spr"
	echo "	for ROSSA_USA_TMO use rossa_tmo"
	echo "	for XCOVER3_JPN_DCM use xcover3_dcm"
    echo "	for J1QLTE_USA_VZW use j1_vzw"
	echo "	for J3LTE_CHN_CTC use j3lte_chnctc"
	echo "	for J3LTE_CHN_TW use j3lte_tw"
	echo "	for J3LTE_USA_SPR use j3lte_spr"
	echo "	for J3LTE_USA_VZW use j3lte_vzw"
	echo "	for J3LTE_USA_VZW use j3lte_usc"
	echo "	for J1XQLTE_USA_VZW use j1xqlte_spr"
    echo "	for GTESLTE_USA_SPR use gtes_spr"
    echo "	for GTESLTE_USA_USC use gtes_usc"
    echo "	for O7LTE_SWA_OPEN use o7lte_swaopen"
	echo "	for J5XLTE_EUR_OPEN use j5xlte_eur"
	echo "	for J5LTE_EUR use j5lte_eur"
	echo "  \$2 : "
	echo "	-B or Nothing  (-B : Secure Binary)"
	echo "  \$3 : "
	echo "	-E or Nothing  (-E : SEANDROID Binary)"
	echo -e '\E[0m'
}


# MAIN FUNCTION
rm -rf ./build.log
(
	START_TIME=`date +%s`

	FUNC_BUILD_KERNEL
	#FUNC_RAMDISK_EXTRACT_N_COPY
	FUNC_MKBOOTIMG

	END_TIME=`date +%s`

	let "ELAPSED_TIME=$END_TIME-$START_TIME"
	echo "Total compile time is $ELAPSED_TIME seconds"
) 2>&1	 | tee -a ./build.log
