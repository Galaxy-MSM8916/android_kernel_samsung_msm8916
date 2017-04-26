#
# Makefile for Samsung System LSI - nfc devices
#

ifeq ($(CONFIG_SEC_NFC_SENN3AB),y)
obj-y += sec_nfc.o
obj-y += sec_nfc_fn.o
# bu80003gul driver
obj-y += bu80003gul.o

ccflags-y += -DCONFIG_NFC_DEBUG
ccflags-y += -DCONFIG_SEC_NFC_LDO_JPN_CONTROL
ccflags-y += -DCONFIG_NFC_EDC_TUNING

endif
