/*
 * STMicroelectronics lsm6ds3 platform-data driver
 *
 * Copyright 2014 STMicroelectronics Inc.
 *
 * Denis Ciocca <denis.ciocca@st.com>
 *
 * Licensed under the GPL-2.
 */

#ifndef ST_LSM6DS3_PDATA_H
#define ST_LSM6DS3_PDATA_H

/**
 * struct st_lsm6ds3_platform_data - Platform data for the ST lsm6ds3 sensor
 * @drdy_int_pin: Redirect DRDY on pin 1 (1) or pin 2 (2).
 */
struct st_lsm6ds3_platform_data {
	u8 drdy_int_pin;
};

#endif /* ST_LSM6DS3_PDATA_H */
