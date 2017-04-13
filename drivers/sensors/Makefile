#
# Makefile for the sensors drivers.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSORS_CORE)          += sensors_core.o
obj-$(CONFIG_SENSORS_SSC)           += sensors_ssc.o

# accelerometer_sensor
obj-$(CONFIG_SENSORS_K2HH)          += k2hh.o
# optical_sensor
obj-$(CONFIG_SENSORS_CM36672P)      += cm36672p.o
obj-$(CONFIG_SENSORS_CM36652)           += cm36652.o
obj-$(CONFIG_SENSORS_CM36655)           += cm36655.o
obj-$(CONFIG_SENSORS_GP2A002)	    += gp2a002.o
obj-$(CONFIG_SENSORS_TCS3490)	    += tcs3490.o
obj-$(CONFIG_SENSORS_CM3323)	    += cm3323.o
obj-$(CONFIG_SENSORS_TMD3782S)	+= taos_tmd3782.o
obj-$(CONFIG_SENSORS_STK3013)      += stk3013.o
obj-$(CONFIG_SENSORS_CM36686)	+= cm36686.o
# grip_sensor
obj-$(CONFIG_SENSORS_SX9310)        +=sx9310.o
obj-$(CONFIG_SENSORS_SX9500)        +=sx9500.o
obj-$(CONFIG_SENSORS_SX9500_WIFI)   +=sx9500_wifi.o
obj-$(CONFIG_SENSORS_SX9500_ALWAYS) +=sx9500_always.o
# magnetic_sensor
obj-$(CONFIG_SENSORS_BMA2X2)	+= bma2x2.o
obj-$(CONFIG_SENSORS_BMM050)	+= bmm050_driver.o bmm050.o
obj-$(CONFIG_SENSORS_AK09911C)	+= ak09911c.o
obj-$(CONFIG_SENSORS_AK09916C)	+= ak09916c.o
