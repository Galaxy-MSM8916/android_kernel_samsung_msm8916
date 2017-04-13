# When adding new entries keep the list in alphabetical order
CFLAGS_scm.o :=$(call as-instr,.arch_extension sec,-DREQUIRES_SEC=1)

obj-$(CONFIG_SEC_DEBUG) += sec_debug.o sec_gaf.o sec_getlog.o
obj-$(CONFIG_SEC_DEBUG_DOUBLE_FREE) += sec_debug-dfd.o

obj-$(CONFIG_SEC_BSP) += sec_bsp.o

obj-$(CONFIG_SEC_THERMISTOR) += sec_thermistor.o msm8916-thermistor.o
