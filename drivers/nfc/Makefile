#
# Makefile for nfc devices
#

obj-$(CONFIG_NFC_PN544)		+= pn544/
obj-$(CONFIG_SEC_NFC_SENN3AB)	+= felica/
obj-$(CONFIG_NFC_WILINK)	+= nfcwilink.o
obj-$(CONFIG_NFC_MEI_PHY)	+= mei_phy.o
obj-$(CONFIG_NFC_QNCI)		+= nfc-nci.o
obj-$(CONFIG_NFC_NQ)		+= nq-nci.o
obj-$(CONFIG_NFC_PN547)		+= pn547.o
obj-$(CONFIG_SEC_NFC)		+= sec_nfc.o
obj-$(CONFIG_ESE_P3)           += ese_p3.o

obj-$(CONFIG_ESE_P61)		+= p61.o
obj-$(CONFIG_BCM2079X_NFC_I2C)	+= bcm2079x-i2c.o

ccflags-$(CONFIG_NFC_DEBUG) := -DDEBUG
