#
# Makefile for Inertial Measurement Units
#

adis16400-y             := adis16400_core.o
adis16400-$(CONFIG_IIO_BUFFER) += adis16400_buffer.o
obj-$(CONFIG_ADIS16400) += adis16400.o
obj-$(CONFIG_ADIS16480) += adis16480.o

adis_lib-y += adis.o
adis_lib-$(CONFIG_IIO_ADIS_LIB_BUFFER) += adis_trigger.o
adis_lib-$(CONFIG_IIO_ADIS_LIB_BUFFER) += adis_buffer.o
obj-$(CONFIG_IIO_ADIS_LIB) += adis_lib.o

obj-y += inv_mpu6050/

obj-$(CONFIG_ST_LSM6DS3_IIO) += st_lsm6ds3.o
st_lsm6ds3-objs := st_lsm6ds3_core.o

obj-$(CONFIG_ST_LSM6DS3_IIO) 	+= st_lsm6ds3_i2c.o
st_lsm6ds3-$(CONFIG_IIO_BUFFER) += st_lsm6ds3_buffer.o
st_lsm6ds3-$(CONFIG_IIO_TRIGGER)+= st_lsm6ds3_trigger.o
