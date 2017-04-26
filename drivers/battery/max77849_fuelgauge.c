/*
 *  max77849_fuelgauge.c
 *  Samsung MAX77849 Fuel Gauge Driver
 *
 *  Copyright (C) 2015 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define DEBUG

#include <linux/battery/fuelgauge/max77849_fuelgauge.h>

int fg_alert_init(struct i2c_client *client, int soc);

static struct device_attribute max77849_fg_attrs[] = {
	MAX77849_FG_ATTR(reg),
	MAX77849_FG_ATTR(data),
	MAX77849_FG_ATTR(regs),
};

static enum power_supply_property max77849_fuelgauge_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_ENERGY_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_AMBIENT,
	POWER_SUPPLY_PROP_ENERGY_FULL,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
};

static int fg_i2c_read(struct i2c_client *client,
				u8 reg, u8 *data, u8 length)
{
	s32 value;

	value = i2c_smbus_read_i2c_block_data(client, reg, length, data);
	if (value < 0 || value != length) {
		dev_err(&client->dev, "%s: Error(%d)\n",
			__func__, value);
		return -1;
	}

	return 0;
}

static int fg_i2c_write(struct i2c_client *client,
				u8 reg, u8 *data, u8 length)
{
	s32 value;

	value = i2c_smbus_write_i2c_block_data(client, reg, length, data);
	if (value < 0) {
		dev_err(&client->dev, "%s: Error(%d)\n",
			__func__, value);
		return -1;
	}

	return 0;
}

static int fg_read_register(struct i2c_client *client,
				u8 addr)
{
	u8 data[2];

	if (fg_i2c_read(client, addr, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read addr(0x%x)\n",
			__func__, addr);
		return -1;
	}

	return (data[1] << 8) | data[0];
}

static int fg_write_register(struct i2c_client *client,
				u8 addr, u16 w_data)
{
	u8 data[2];

	data[0] = w_data & 0xFF;
	data[1] = w_data >> 8;

	if (fg_i2c_write(client, addr, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to write addr(0x%x)\n",
			__func__, addr);
		return -1;
	}

	return 0;
}

static void fg_write_and_verify_register(struct i2c_client *client,
				u8 addr, u16 w_data)
{
	u16 r_data;
	u8 retry_cnt = 2;

	while (retry_cnt) {
		fg_write_register(client, addr, w_data);
		r_data = fg_read_register(client, addr);

		if (r_data != w_data) {
			dev_err(&client->dev,
				"%s: verification failed (addr: 0x%x, w_data: 0x%x, r_data: 0x%x)\n",
				__func__, addr, w_data, r_data);
			retry_cnt--;
		} else
			break;
	}
}

static void fg_test_print(struct i2c_client *client)
{
	u8 data[2];
	u32 average_vcell;
	u16 w_data;
	u32 temp;
	u32 temp2;
	u16 reg_data;

	if (fg_i2c_read(client, AVR_VCELL_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read VCELL\n", __func__);
		return;
	}

	w_data = (data[1]<<8) | data[0];

	temp = (w_data & 0xFFF) * 78125;
	average_vcell = temp / 1000000;

	temp = ((w_data & 0xF000) >> 4) * 78125;
	temp2 = temp / 1000000;
	average_vcell += (temp2 << 4);

	dev_info(&client->dev, "%s: AVG_VCELL(%d), data(0x%04x)\n", __func__,
		average_vcell, (data[1]<<8) | data[0]);

	reg_data = fg_read_register(client, FULLCAP_REG);
	dev_info(&client->dev, "%s: FULLCAP(%d), data(0x%04x)\n", __func__,
		reg_data/2, reg_data);

	reg_data = fg_read_register(client, REMCAP_REP_REG);
	dev_info(&client->dev, "%s: REMCAP_REP(%d), data(0x%04x)\n", __func__,
		reg_data/2, reg_data);

	reg_data = fg_read_register(client, REMCAP_MIX_REG);
	dev_info(&client->dev, "%s: REMCAP_MIX(%d), data(0x%04x)\n", __func__,
		reg_data/2, reg_data);

	reg_data = fg_read_register(client, REMCAP_AV_REG);
	dev_info(&client->dev, "%s: REMCAP_AV(%d), data(0x%04x)\n", __func__,
		reg_data/2, reg_data);
}

static void fg_periodic_read(struct i2c_client *client)
{
	u8 reg;
	int i;
	int data[0x10];
	char *str = NULL;

	str = kzalloc(sizeof(char)*1500, GFP_KERNEL);
	if (!str)
		return;

	for (i = 0; i < 16; i++) {
		for (reg = 0; reg < 0x10; reg++) {
			data[reg] = fg_read_register(client, reg + i * 0x10);
			if (data[reg] < 0) {
				pr_err("%s: fg read register failed(%d)\n", __func__, data[reg]);
				goto periodic_error;
			}
		}

		sprintf(str+strlen(str),
			"%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,",
			data[0x00], data[0x01], data[0x02], data[0x03],
			data[0x04], data[0x05], data[0x06], data[0x07]);
		sprintf(str+strlen(str),
			"%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,%04xh,",
			data[0x08], data[0x09], data[0x0a], data[0x0b],
			data[0x0c], data[0x0d], data[0x0e], data[0x0f]);
		if (i == 4)
			i = 13;
	}

	dev_info(&client->dev, "%s", str);
periodic_error:
	kfree(str);
}

static void fg_read_regs(struct i2c_client *client, char *str)
{
	int data = 0;
	u32 addr = 0;

	for (addr = 0; addr <= 0x4f; addr++) {
		data = fg_read_register(client, addr);
		sprintf(str+strlen(str), "0x%04x, ", data);
	}

	/* "#" considered as new line in application */
	sprintf(str+strlen(str), "#");

	for (addr = 0xe0; addr <= 0xff; addr++) {
		data = fg_read_register(client, addr);
		sprintf(str+strlen(str), "0x%04x, ", data);
	}
}

static int fg_read_soc(struct i2c_client *client);
static int fg_read_vcell(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data[2];
	u32 vcell;
	u16 w_data;
	u32 temp;
	u32 temp2;

	if (fg_i2c_read(client, VCELL_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read VCELL\n", __func__);
		return -1;
	}

	w_data = (data[1]<<8) | data[0];

	temp = (w_data & 0xFFF) * 78125;
	vcell = temp / 1000000;

	temp = ((w_data & 0xF000) >> 4) * 78125;
	temp2 = temp / 1000000;
	vcell += (temp2 << 4);

	if (!(fuelgauge->pr_cnt % PRINT_COUNT)) {
		dev_info(&client->dev, "%s: VCELL(%d), data(0x%04x)\n",
			__func__, vcell, (data[1]<<8) | data[0]);
		fg_read_soc(client);
	}

	if ((fuelgauge->sw_v_empty == MAX77849_VEMPTY_MODE) && vcell > 3600) {
		fuelgauge->sw_v_empty = MAX77849_VEMPTY_RECOVERY_MODE;
		fg_alert_init(fuelgauge->client,
			fuelgauge->pdata->fuel_alert_soc);
		pr_info("%s : SW V EMPTY DISABLE\n", __func__);
	}

	return vcell;
}

static int fg_read_vfocv(struct i2c_client *client)
{
	u8 data[2];
	u32 vfocv = 0;
	u16 w_data;
	u32 temp;
	u32 temp2;

	if (fg_i2c_read(client, VFOCV_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read VFOCV\n", __func__);
		return -1;
	}

	w_data = (data[1]<<8) | data[0];

	temp = (w_data & 0xFFF) * 78125;
	vfocv = temp / 1000000;

	temp = ((w_data & 0xF000) >> 4) * 78125;
	temp2 = temp / 1000000;
	vfocv += (temp2 << 4);

	return vfocv;
}

static int fg_read_avg_vcell(struct i2c_client *client)
{
	u8 data[2];
	u32 avg_vcell = 0;
	u16 w_data;
	u32 temp;
	u32 temp2;

	if (fg_i2c_read(client, AVR_VCELL_REG, data, 2) < 0) {
		dev_err(&client->dev,
			"%s: Failed to read AVG_VCELL\n", __func__);
		return -1;
	}

	w_data = (data[1]<<8) | data[0];

	temp = (w_data & 0xFFF) * 78125;
	avg_vcell = temp / 1000000;

	temp = ((w_data & 0xF000) >> 4) * 78125;
	temp2 = temp / 1000000;
	avg_vcell += (temp2 << 4);

	return avg_vcell;
}

static int fg_check_battery_present(struct i2c_client *client)
{
	u8 status_data[2];
	int ret = 1;

	/* 1. Check Bst bit */
	if (fg_i2c_read(client, STATUS_REG, status_data, 2) < 0) {
		dev_err(&client->dev,
			"%s: Failed to read STATUS_REG\n", __func__);
		return 0;
	}

	if (status_data[0] & (0x1 << 3)) {
		dev_info(&client->dev,
			"%s: addr(0x01), data(0x%04x)\n", __func__,
			(status_data[1]<<8) | status_data[0]);
		dev_info(&client->dev, "%s: battery is absent!!\n", __func__);
		ret = 0;
	}

	return ret;
}

static int max77849_fg_low_temp_compensation(struct i2c_client *client, bool en)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u16 read_data;
	u8 valrt_data[2];

	if (!fuelgauge->using_temp_compensation)
		return 0;

	if (en) {
		/* VALRT Threshold setting */
		valrt_data[1] = 0xFF;
		valrt_data[0] = (u8)fuelgauge->low_temp_valrt;
		if (fg_i2c_write(client, VALRT_THRESHOLD_REG, valrt_data, 2) < 0) {
			dev_info(&client->dev,
				"%s: Failed to write VALRT_THRESHOLD_REG\n", __func__);
			return -1;
		}

		read_data = fg_read_register(client, (u8)VALRT_THRESHOLD_REG);
		if (read_data != (u16)(0xFF00 | fuelgauge->low_temp_valrt))
			dev_err(&client->dev,
				"%s: VALRT_THRESHOLD_REG is not valid (0x%x)\n",
				__func__, read_data);
	} else {
		/* VALRT Threshold setting */
		valrt_data[1] = 0xFF;
		valrt_data[0] = 0xA5;
		if (fg_i2c_write(client, VALRT_THRESHOLD_REG, valrt_data, 2) < 0) {
			dev_info(&client->dev,
				"%s: Failed to write VALRT_THRESHOLD_REG\n", __func__);
			return -1;
		}

		read_data = fg_read_register(client, (u8)VALRT_THRESHOLD_REG);
		if (read_data != 0xFFA5)
		dev_err(&client->dev,
			"%s: VALRT_THRESHOLD_REG is not valid (0x%x)\n",
			__func__, read_data);
		}
	return 0;
}

static int fg_adjust_temp (struct i2c_client *client, enum power_supply_property psp, int value)
{
	int temp = 0;
	int temp_adc;
	int low = 0;
	int high = 0;
	int mid = 0;
	static int count = 0;
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	const sec_bat_adc_table_data_t *temp_adc_table;
	unsigned int temp_adc_table_size;

	temp_adc = value;

	switch (psp) {
	case POWER_SUPPLY_PROP_TEMP:
		if (fuelgauge->pdata->temp_adc_table) {
			temp_adc_table = fuelgauge->pdata->temp_adc_table;
			temp_adc_table_size = fuelgauge->pdata->temp_adc_table_size;
		} else {
			return temp_adc;
		}
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		if (fuelgauge->pdata->temp_amb_adc_table) {
			temp_adc_table = fuelgauge->pdata->temp_amb_adc_table;
			temp_adc_table_size =
				fuelgauge->pdata->temp_amb_adc_table_size;
		} else {
			return temp_adc;
		}
		break;
	default:
		return temp_adc;
	}

	if (temp_adc_table[0].adc <= temp_adc) {
		temp = temp_adc_table[0].data;
		goto finish;
	} else if (temp_adc_table[temp_adc_table_size-1].adc >= temp_adc) {
		temp = temp_adc_table[temp_adc_table_size-1].data;
		goto finish;
	}

	high = temp_adc_table_size - 1;

	while (low <= high) {
		mid = (low + high) / 2;
		if (temp_adc_table[mid].adc > temp_adc)
			low = mid + 1;
		else if (temp_adc_table[mid].adc < temp_adc)
			high = mid - 1;
		else {
			temp = temp_adc_table[mid].data;
			goto finish;
		}
	}

	temp = temp_adc_table[high].data;
	temp +=
		((temp_adc_table[low].data -
		temp_adc_table[high].data) *
		(temp_adc - temp_adc_table[high].adc)) /
		(temp_adc_table[low].adc - temp_adc_table[high].adc);

finish:
	if (!(count++ % PRINT_COUNT)) {
		dev_dbg(&client->dev,
			"%s: Temp_org(%d) -> Temp_adj(%d)\n",
			__func__, temp_adc, temp);
		count = 1;
	}
	return temp;
}

static int fg_read_temp(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data[2] = {0, 0};
	int temper = 0;
	/*int i;*/

	if (fg_check_battery_present(client)) {
		if (fg_i2c_read(client, TEMPERATURE_REG, data, 2) < 0) {
			dev_err(&client->dev,
				"%s: Failed to read TEMPERATURE_REG\n",
				__func__);
			return -1;
		}

		if (data[1]&(0x1 << 7)) {
			temper = ((~(data[1]))&0xFF)+1;
			temper *= (-1000);
			temper -= ((~((int)data[0]))+1) * 39 / 10;
		} else {
			temper = data[1] & 0x7f;
			temper *= 1000;
			temper += data[0] * 39 / 10;
		}
	} else
		temper = 20000;

	if (!(fuelgauge->pr_cnt % PRINT_COUNT))
		dev_info(&client->dev, "%s: TEMPERATURE(%d), data(0x%04x)\n",
			__func__, temper, (data[1]<<8) | data[0]);

	return temper/100;
}

/* soc should be 0.1% unit */
static int fg_read_vfsoc(struct i2c_client *client)
{
	u8 data[2];
	int soc;

	if (fg_i2c_read(client, VFSOC_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read VFSOC\n", __func__);
		return -1;
	}

	soc = ((data[1] * 100) + (data[0] * 100 / 256)) / 10;

	return min(soc, 1000);
}

/* soc should be 0.1% unit */
static int fg_read_avsoc(struct i2c_client *client)
{
	u8 data[2];
	int soc;

	if (fg_i2c_read(client, SOCAV_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read AVSOC\n", __func__);
		return -1;
	}

	soc = ((data[1] * 100) + (data[0] * 100 / 256)) / 10;

	return min(soc, 1000);
}

/* soc should be 0.1% unit */
static int fg_read_soc(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data[2];
	int soc;
	int rep_soc;
	int vf_soc;

	if (fg_i2c_read(client, SOCREP_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read SOCREP\n", __func__);
		return -1;
	}

	soc = ((data[1] * 100) + (data[0] * 100 / 256)) / 10;
	rep_soc = min(soc, 1000);
	vf_soc = fg_read_vfsoc(client);

	dev_dbg(&client->dev, "%s: raw capacity (0.1%%) (%d)\n", __func__, soc);

	if (!(fuelgauge->pr_cnt % PRINT_COUNT)) {
		dev_dbg(&client->dev, "%s: raw capacity (%d), data(0x%04x)\n",
			__func__, soc, (data[1]<<8) | data[0]);
		dev_dbg(&client->dev, "%s: RepSOC (%d), VFSOC (%d), data(0x%04x)\n",
			__func__, rep_soc/10, vf_soc/10, (data[1]<<8) | data[0]);
	}

	return rep_soc;
}

/* soc should be 0.01% unit */
static int fg_read_rawsoc(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data[2];
	int soc;

	if (fg_i2c_read(client, SOCREP_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read SOCREP\n", __func__);
		return -1;
	}

	soc = (data[1] * 100) + (data[0] * 100 / 256);

	dev_dbg(&client->dev, "%s: raw capacity (0.01%%) (%d)\n",
		__func__, soc);

	if (!(fuelgauge->pr_cnt % PRINT_COUNT))
		dev_dbg(&client->dev, "%s: raw capacity (%d), data(0x%04x)\n",
			__func__, soc, (data[1]<<8) | data[0]);

	return min(soc, 10000);
}

static int fg_read_fullcap(struct i2c_client *client)
{
	u8 data[2];
	int ret;

	if (fg_i2c_read(client, FULLCAP_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read FULLCAP\n", __func__);
		return -1;
	}

	ret = (data[1] << 8) + data[0];

	return ret;
}

static int fg_read_mixcap(struct i2c_client *client)
{
	u8 data[2];
	int ret;

	if (fg_i2c_read(client, REMCAP_MIX_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read REMCAP_MIX_REG\n",
			__func__);
		return -1;
	}

	ret = (data[1] << 8) + data[0];

	return ret;
}

static int fg_read_avcap(struct i2c_client *client)
{
	u8 data[2];
	int ret;

	if (fg_i2c_read(client, REMCAP_AV_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read REMCAP_AV_REG\n",
			__func__);
		return -1;
	}

	ret = (data[1] << 8) + data[0];

	return ret;
}

static int fg_read_repcap(struct i2c_client *client)
{
	u8 data[2];
	int ret;

	if (fg_i2c_read(client, REMCAP_REP_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read REMCAP_REP_REG\n",
			__func__);
		return -1;
	}

	ret = (data[1] << 8) + data[0];

	return ret;
}

static int fg_read_current(struct i2c_client *client, int unit)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data1[2], data2[2];
	u32 temp, sign;
	s32 i_current;
	s32 avg_current;

	if (fg_i2c_read(client, CURRENT_REG, data1, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read CURRENT\n",
			__func__);
		return -1;
	}

	if (fg_i2c_read(client, AVG_CURRENT_REG, data2, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read AVERAGE CURRENT\n",
			__func__);
		return -1;
	}

	temp = ((data1[1]<<8) | data1[0]) & 0xFFFF;
	if (temp & (0x1 << 15)) {
		sign = NEGATIVE;
		temp = (~temp & 0xFFFF) + 1;
	} else
		sign = POSITIVE;

	/* 1.5625uV/0.01Ohm(Rsense) = 156.25uA */
	switch (unit) {
	case SEC_BATTEY_CURRENT_UA:
		i_current = temp * 15625 / 100;
		break;
	case SEC_BATTEY_CURRENT_MA:
	default:
		i_current = temp * 15625 / 100000;
	}

	if (sign)
		i_current *= -1;

	temp = ((data2[1]<<8) | data2[0]) & 0xFFFF;
	if (temp & (0x1 << 15)) {
		sign = NEGATIVE;
		temp = (~temp & 0xFFFF) + 1;
	} else
		sign = POSITIVE;

	/* 1.5625uV/0.01Ohm(Rsense) = 156.25uA */
	avg_current = temp * 15625 / 100000;
	if (sign)
		avg_current *= -1;

	if (!(fuelgauge->pr_cnt++ % PRINT_COUNT)) {
		fg_test_print(client);
		dev_info(&client->dev, "%s: CURRENT(%dmA), AVG_CURRENT(%dmA)\n",
			__func__, i_current, avg_current);
		fuelgauge->pr_cnt = 1;
		/* Read max77849's all registers every 5 minute. */
		fg_periodic_read(client);
	}

	return i_current;
}

static int fg_read_avg_current(struct i2c_client *client, int unit)
{
	u8  data2[2];
	u32 temp, sign;
	s32 avg_current;

	if (fg_i2c_read(client, AVG_CURRENT_REG, data2, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read AVERAGE CURRENT\n",
			__func__);
		return -1;
	}

	temp = ((data2[1]<<8) | data2[0]) & 0xFFFF;
	if (temp & (0x1 << 15)) {
		sign = NEGATIVE;
		temp = (~temp & 0xFFFF) + 1;
	} else
		sign = POSITIVE;

	/* 1.5625uV/0.01Ohm(Rsense) = 156.25uA */
	switch (unit) {
	case SEC_BATTEY_CURRENT_UA:
		avg_current = temp * 15625 / 100;
		break;
	case SEC_BATTEY_CURRENT_MA:
	default:
		avg_current = temp * 15625 / 100000;
	}

	if (sign)
		avg_current *= -1;

	return avg_current;
}

int fg_reset_soc(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 data[2];
	int vfocv, fullcap;

	/* delay for current stablization */
	msleep(500);

	dev_info(&client->dev,
		"%s: Before quick-start - VCELL(%d), VFOCV(%d), VfSOC(%d), RepSOC(%d)\n",
		__func__, fg_read_vcell(client), fg_read_vfocv(client),
		fg_read_vfsoc(client), fg_read_soc(client));
	dev_info(&client->dev,
		"%s: Before quick-start - current(%d), avg current(%d)\n",
		__func__, fg_read_current(client, SEC_BATTEY_CURRENT_MA),
		fg_read_avg_current(client, SEC_BATTEY_CURRENT_MA));

	if (!sec_bat_check_jig_status()) {
		dev_info(&client->dev,
			"%s : Return by No JIG_ON signal\n", __func__);
		return 0;
	} else if (fuelgauge->pdata->jig_irq &&
			!gpio_get_value(fuelgauge->pdata->jig_irq)) {
		dev_info(&client->dev,
			"%s : Return by No JIG_ON signal\n", __func__);
		return 0;
	}

	fg_write_register(client, CYCLES_REG, 0);

	if (fg_i2c_read(client, MISCCFG_REG, data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read MiscCFG\n", __func__);
		return -1;
	}

	data[1] |= (0x1 << 2);
	if (fg_i2c_write(client, MISCCFG_REG, data, 2) < 0) {
		dev_err(&client->dev,
			"%s: Failed to write MiscCFG\n", __func__);
		return -1;
	}

	msleep(250);
	fg_write_register(client, FULLCAP_REG,
		fuelgauge->battery_data->Capacity);
	msleep(500);

	dev_info(&client->dev,
		"%s: After quick-start - VCELL(%d), VFOCV(%d), VfSOC(%d), RepSOC(%d)\n",
		__func__, fg_read_vcell(client), fg_read_vfocv(client),
		fg_read_vfsoc(client), fg_read_soc(client));
	dev_info(&client->dev,
		"%s: After quick-start - current(%d), avg current(%d)\n",
		__func__, fg_read_current(client, SEC_BATTEY_CURRENT_MA),
		fg_read_avg_current(client, SEC_BATTEY_CURRENT_MA));
	fg_write_register(client, CYCLES_REG, 0x00a0);

/* P8 is not turned off by Quickstart @3.4V
 * (It's not a problem, depend on mode data)
 * Power off for factory test(File system, etc..) */
	vfocv = fg_read_vfocv(client);
	if (vfocv < POWER_OFF_VOLTAGE_LOW_MARGIN) {
		dev_info(&client->dev, "%s: Power off condition(%d)\n",
			__func__, vfocv);

		fullcap = fg_read_register(client, FULLCAP_REG);
		/* FullCAP * 0.009 */
		fg_write_register(client, REMCAP_REP_REG,
			(u16)(fullcap * 9 / 1000));
		msleep(200);
		dev_info(&client->dev, "%s: new soc=%d, vfocv=%d\n", __func__,
			fg_read_soc(client), vfocv);
	}

	dev_info(&client->dev,
		"%s: Additional step - VfOCV(%d), VfSOC(%d), RepSOC(%d)\n",
		__func__, fg_read_vfocv(client),
		fg_read_vfsoc(client), fg_read_soc(client));

	return 0;
}


int fg_reset_capacity_by_jig_connection(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	union power_supply_propval value;

	dev_info(&client->dev,
		"%s: DesignCap = Capacity - 1 (Jig Connection)\n", __func__);
	/* If JIG is attached, the voltage is set as 1079 */
	value.intval = 1079;
	psy_do_property("battery", set,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, value);

	return fg_write_register(client, DESIGNCAP_REG,
		fuelgauge->battery_data->Capacity-1);
}

void fg_low_batt_compensation(struct i2c_client *client, u32 level)
{
	int read_val;
	u32 temp;

	dev_info(&client->dev, "%s: Adjust SOCrep to %d!!\n",
		__func__, level);

	read_val = fg_read_register(client, FULLCAP_REG);
	/* RemCapREP (05h) = FullCap(10h) x 0.0090 */
	temp = read_val * (level*90) / 10000;
	fg_write_register(client, REMCAP_REP_REG, (u16)temp);
}

static int fg_check_status_reg(struct i2c_client *client)
{
	u8 status_data[2];
	int ret = 0;

	/* 1. Check Smn was generatedread */
	if (fg_i2c_read(client, STATUS_REG, status_data, 2) < 0) {
		dev_err(&client->dev, "%s: Failed to read STATUS_REG\n",
			__func__);
		return -1;
	}
	dev_info(&client->dev, "%s: addr(0x00), data(0x%04x)\n", __func__,
		(status_data[1]<<8) | status_data[0]);

	if (status_data[1] & (0x1 << 2))
		ret = 1;

	/* 2. clear Status reg */
	status_data[1] = 0;
	if (fg_i2c_write(client, STATUS_REG, status_data, 2) < 0) {
		dev_info(&client->dev, "%s: Failed to write STATUS_REG\n",
			__func__);
		return -1;
	}

	return ret;
}

int get_fuelgauge_value(struct i2c_client *client, int data)
{
	int ret;

	switch (data) {
	case FG_LEVEL:
		ret = fg_read_soc(client);
		break;

	case FG_TEMPERATURE:
		ret = fg_read_temp(client);
		break;

	case FG_VOLTAGE:
		ret = fg_read_vcell(client);
		break;

	case FG_CURRENT:
		ret = fg_read_current(client, SEC_BATTEY_CURRENT_MA);
		break;

	case FG_CURRENT_AVG:
		ret = fg_read_avg_current(client, SEC_BATTEY_CURRENT_MA);
		break;

	case FG_CHECK_STATUS:
		ret = fg_check_status_reg(client);
		break;

	case FG_RAW_SOC:
		ret = fg_read_rawsoc(client);
		break;

	case FG_VF_SOC:
		ret = fg_read_vfsoc(client);
		break;

	case FG_AV_SOC:
		ret = fg_read_avsoc(client);
		break;

	case FG_FULLCAP:
		ret = fg_read_fullcap(client);
		break;

	case FG_MIXCAP:
		ret = fg_read_mixcap(client);
		break;

	case FG_AVCAP:
		ret = fg_read_avcap(client);
		break;

	case FG_REPCAP:
		ret = fg_read_repcap(client);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

int fg_alert_init(struct i2c_client *client, int soc)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	u8 misccgf_data[2];
	u8 salrt_data[2];
	u8 config_data[2];
	u8 valrt_data[2];
	u8 talrt_data[2];
	u16 read_data = 0;

	/* Using RepSOC */
	if (fg_i2c_read(client, MISCCFG_REG, misccgf_data, 2) < 0) {
		dev_err(&client->dev,
			"%s: Failed to read MISCCFG_REG\n", __func__);
		return -1;
	}
	misccgf_data[0] = misccgf_data[0] & ~(0x03);

	if (fg_i2c_write(client, MISCCFG_REG, misccgf_data, 2) < 0) {
		dev_info(&client->dev,
			"%s: Failed to write MISCCFG_REG\n", __func__);
		return -1;
	}

	/* SALRT Threshold setting */
	salrt_data[1] = 0xff;
	salrt_data[0] = soc;
	if (fg_i2c_write(client, SALRT_THRESHOLD_REG, salrt_data, 2) < 0) {
		dev_info(&client->dev,
			"%s: Failed to write SALRT_THRESHOLD_REG\n", __func__);
		return -1;
	}

	/* Reset VALRT Threshold setting */
	if (fuelgauge->using_temp_compensation){
		valrt_data[1] = 0xFF;
		valrt_data[0] = 0xA5;
		if (fg_i2c_write(client, VALRT_THRESHOLD_REG, valrt_data, 2) < 0) {
			dev_info(&client->dev,
				"%s: Failed to write VALRT_THRESHOLD_REG\n", __func__);
			return -1;
		}
	} else {
		valrt_data[1] = 0xFF;
		valrt_data[0] = 0x00;
		if (fg_i2c_write(client, VALRT_THRESHOLD_REG, valrt_data, 2) < 0) {
			dev_info(&client->dev,
				"%s: Failed to write VALRT_THRESHOLD_REG\n", __func__);
			return -1;
		}
	}

	read_data = fg_read_register(client, (u8)VALRT_THRESHOLD_REG);
	if (read_data != 0xff00)
		dev_err(&client->dev,
			"%s: VALRT_THRESHOLD_REG is not valid (0x%x)\n",
			__func__, read_data);

	/* Reset TALRT Threshold setting (disable) */
	talrt_data[1] = 0x7F;
	talrt_data[0] = 0x80;
	if (fg_i2c_write(client, TALRT_THRESHOLD_REG, talrt_data, 2) < 0) {
		dev_info(&client->dev,
			"%s: Failed to write TALRT_THRESHOLD_REG\n", __func__);
		return -1;
	}

	read_data = fg_read_register(client, (u8)TALRT_THRESHOLD_REG);
	if (read_data != 0x7f80)
		dev_err(&client->dev,
			"%s: TALRT_THRESHOLD_REG is not valid (0x%x)\n",
			__func__, read_data);

	/*mdelay(100);*/

	/* Enable SOC alerts */
	if (fg_i2c_read(client, CONFIG_REG, config_data, 2) < 0) {
		dev_err(&client->dev,
			"%s: Failed to read CONFIG_REG\n", __func__);
		return -1;
	}
	config_data[0] = config_data[0] | (0x1 << 2);

	if (fg_i2c_write(client, CONFIG_REG, config_data, 2) < 0) {
		dev_info(&client->dev,
			"%s: Failed to write CONFIG_REG\n", __func__);
		return -1;
	}

	return 1;
}

static void display_low_batt_comp_cnt(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	pr_info("Check Array(%s): [%d, %d], [%d, %d], ",
			fuelgauge->battery_data->type_str,
			fuelgauge->low_batt_comp_cnt[0][0],
			fuelgauge->low_batt_comp_cnt[0][1],
			fuelgauge->low_batt_comp_cnt[1][0],
			fuelgauge->low_batt_comp_cnt[1][1]);
	pr_info("[%d, %d], [%d, %d], [%d, %d]\n",
			fuelgauge->low_batt_comp_cnt[2][0],
			fuelgauge->low_batt_comp_cnt[2][1],
			fuelgauge->low_batt_comp_cnt[3][0],
			fuelgauge->low_batt_comp_cnt[3][1],
			fuelgauge->low_batt_comp_cnt[4][0],
			fuelgauge->low_batt_comp_cnt[4][1]);
}

static void add_low_batt_comp_cnt(struct i2c_client *client,
				int range, int level)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int i;
	int j;

	/* Increase the requested count value, and reset others. */
	fuelgauge->low_batt_comp_cnt[range-1][level/2]++;

	for (i = 0; i < LOW_BATT_COMP_RANGE_NUM; i++) {
		for (j = 0; j < LOW_BATT_COMP_LEVEL_NUM; j++) {
			if (i == range-1 && j == level/2)
				continue;
			else
				fuelgauge->low_batt_comp_cnt[i][j] = 0;
		}
	}
}

void prevent_early_poweroff(struct i2c_client *client,
	int vcell, int *fg_soc)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int soc = 0;
	int read_val;

	soc = fg_read_soc(client);

	/* No need to write REMCAP_REP in below normal cases */
	if (soc > POWER_OFF_SOC_HIGH_MARGIN || vcell > fuelgauge->battery_data->low_battery_comp_voltage)
		return;

	dev_info(&client->dev, "%s: soc=%d, vcell=%d\n", __func__,
		soc, vcell);

	if (vcell > POWER_OFF_VOLTAGE_HIGH_MARGIN) {
		read_val = fg_read_register(client, FULLCAP_REG);
		/* FullCAP * 0.013 */
		fg_write_register(client, REMCAP_REP_REG,
		(u16)(read_val * 13 / 1000));
		msleep(200);
		*fg_soc = fg_read_soc(client);
		dev_info(&client->dev, "%s: new soc=%d, vcell=%d\n",
			__func__, *fg_soc, vcell);
	}
}

void reset_low_batt_comp_cnt(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	memset(fuelgauge->low_batt_comp_cnt, 0,
		sizeof(fuelgauge->low_batt_comp_cnt));
}

static int check_low_batt_comp_condition(
				struct i2c_client *client, int *nLevel)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int i;
	int j;
	int ret = 0;

	for (i = 0; i < LOW_BATT_COMP_RANGE_NUM; i++) {
		for (j = 0; j < LOW_BATT_COMP_LEVEL_NUM; j++) {
			if (fuelgauge->low_batt_comp_cnt[i][j] >=
				MAX_LOW_BATT_CHECK_CNT) {
				display_low_batt_comp_cnt(client);
				ret = 1;
				*nLevel = j*2 + 1;
				break;
			}
		}
	}

	return ret;
}

static int get_low_batt_threshold(struct i2c_client *client,
				int range, int nCurrent, int level)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int ret = 0;

	ret = fuelgauge->battery_data->low_battery_table[range][OFFSET] +
		((nCurrent *
		fuelgauge->battery_data->low_battery_table[range][SLOPE]) /
		1000);

	return ret;
}

int low_batt_compensation(struct i2c_client *client,
		int fg_soc, int fg_vcell, int fg_current)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int fg_avg_current = 0;
	int fg_min_current = 0;
	int new_level = 0;
	int i, table_size;

	/* Not charging, Under low battery comp voltage */
	if (fg_vcell <= fuelgauge->battery_data->low_battery_comp_voltage) {
		fg_avg_current = fg_read_avg_current(client,
			SEC_BATTEY_CURRENT_MA);
		fg_min_current = min(fg_avg_current, fg_current);

		table_size =
			sizeof(fuelgauge->battery_data->low_battery_table) /
			(sizeof(s16)*TABLE_MAX);

		for (i = 1; i < CURRENT_RANGE_MAX_NUM; i++) {
			if ((fg_min_current >= fuelgauge->battery_data->
				low_battery_table[i-1][RANGE]) &&
				(fg_min_current < fuelgauge->battery_data->
				low_battery_table[i][RANGE])) {
				if (fg_soc >= 10 && fg_vcell <
					get_low_batt_threshold(client,
					i, fg_min_current, 1)) {
					add_low_batt_comp_cnt(
						client, i, 1);
				} else {
					reset_low_batt_comp_cnt(client);
				}
			}
		}

		if (check_low_batt_comp_condition(client, &new_level)) {
			fg_low_batt_compensation(client, new_level);
			reset_low_batt_comp_cnt(client);

			/* Do not update soc right after
			 * low battery compensation
			 * to prevent from powering-off suddenly
			 */
			dev_info(&client->dev,
				"%s: SOC is set to %d by low compensation!!\n",
				__func__, fg_read_soc(client));
		}
	}

	/* Prevent power off over 3500mV */
	prevent_early_poweroff(client, fg_vcell, &fg_soc);

	return fg_soc;
}

static bool fuelgauge_recovery_handler(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	if (fuelgauge->soc < LOW_BATTERY_SOC_REDUCE_UNIT)
		fuelgauge->is_low_batt_alarm = false;
	else {
		dev_err(&client->dev,
			"%s: Reduce the Reported SOC by 1%%\n",
			__func__);
		fuelgauge->soc -=
			LOW_BATTERY_SOC_REDUCE_UNIT;
		dev_err(&client->dev,
			"%s: New Reduced RepSOC (%d)\n",
			__func__, fuelgauge->soc);
	}

	return fuelgauge->is_low_batt_alarm;
}

static int get_fuelgauge_soc(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	union power_supply_propval value;
	int fg_soc = 0;
	int fg_vfsoc;
	int fg_vcell;
	int fg_current;
	int avg_current;

	if (fuelgauge->is_low_batt_alarm)
		if (fuelgauge_recovery_handler(client)) {
			fg_soc = fuelgauge->soc;
			goto return_soc;
		}

	fg_soc = get_fuelgauge_value(client, FG_LEVEL);
	if (fg_soc < 0) {
		dev_info(&client->dev, "Can't read soc!!!");
		fg_soc = fuelgauge->soc;
	}

	fg_vcell = get_fuelgauge_value(client, FG_VOLTAGE);
	fg_current = get_fuelgauge_value(client, FG_CURRENT);
	avg_current = get_fuelgauge_value(client, FG_CURRENT_AVG);
	fg_vfsoc = get_fuelgauge_value(client, FG_VF_SOC);

	psy_do_property("battery", get,
		POWER_SUPPLY_PROP_STATUS, value);

	/*  Checks vcell level and tries to compensate SOC if needed.*/
	if (fuelgauge->pdata->jig_irq) {
		if (!gpio_get_value(fuelgauge->pdata->jig_irq) &&
				(value.intval == POWER_SUPPLY_STATUS_DISCHARGING))
			fg_soc = low_batt_compensation(
					client, fg_soc, fg_vcell, fg_current);
	} else {
		if(value.intval == POWER_SUPPLY_STATUS_DISCHARGING)
			fg_soc = low_batt_compensation(
					client, fg_soc, fg_vcell, fg_current);
	}

	if (fuelgauge->is_first_check)
		fuelgauge->is_first_check = false;

	fuelgauge->soc = fg_soc;

return_soc:
	dev_dbg(&client->dev, "%s: soc(%d), low_batt_alarm(%d)\n",
		__func__, fuelgauge->soc,
		fuelgauge->is_low_batt_alarm);

	return fg_soc;
}

static irqreturn_t sec_jig_irq_thread(int irq, void *irq_data)
{
	struct max77849_fuelgauge_info *fuelgauge = irq_data;

	if (gpio_get_value(fuelgauge->pdata->jig_irq))
		fg_reset_capacity_by_jig_connection(fuelgauge->client);
	else
		dev_info(&fuelgauge->client->dev,
			"%s: jig removed\n", __func__);

	return IRQ_HANDLED;
}

bool fg_init(struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge =
				i2c_get_clientdata(client);
	ktime_t	current_time;
	struct timespec ts;

	current_time = ktime_get_boottime();
	ts = ktime_to_timespec(current_time);

	board_fuelgauge_init(fuelgauge);

	fuelgauge->fullcap_check_interval = ts.tv_sec;

	fuelgauge->is_low_batt_alarm = false;
	fuelgauge->is_first_check = true;

	/*fg_read_model_data(client);*/
	fg_periodic_read(client);

	if (sec_bat_check_jig_status())
		fg_reset_capacity_by_jig_connection(client);
	else {
		if (fuelgauge->pdata->jig_irq > 0) {
			int ret;
			ret = request_threaded_irq(
				gpio_to_irq(fuelgauge->pdata->jig_irq),
				NULL, sec_jig_irq_thread,
				fuelgauge->pdata->jig_irq_attr,
				"jig-irq", fuelgauge);
			if (ret) {
				dev_info(&fuelgauge->client->dev,
					"%s: Failed to Reqeust IRQ\n",
					__func__);
			}
		}

	}

	return true;
}

bool sec_hal_fg_is_fuelalerted(struct i2c_client *client)
{
	if (get_fuelgauge_value(client, FG_CHECK_STATUS) > 0)
		return true;
	else
		return false;
}

bool fg_fuelalert_process(void *irq_data, bool is_fuel_alerted)
{
	struct max77849_fuelgauge_info *fuelgauge =
		(struct max77849_fuelgauge_info *)irq_data;
	union power_supply_propval value;
	u8 config_data[2];
	u8 status_data[2];
	int overcurrent_limit_in_soc;
	int current_soc =
		get_fuelgauge_value(fuelgauge->client, FG_LEVEL);
#if defined(FUELALERT_CHECK_VOLTAGE_FEATURE)
	int fg_vcell = get_fuelgauge_value(fuelgauge->client, FG_VOLTAGE);
#endif

	if (fg_i2c_read(fuelgauge->client, CONFIG_REG, config_data, 2) < 0) {
		dev_err(&fuelgauge->client->dev,
			"%s: Failed to read CONFIG_REG\n", __func__);
		return -1;
	}
	config_data[0] &= ~ALERT_EN;
	dev_info(&fuelgauge->client->dev, "%s: CONIFG(0x%02x%02x)\n", __func__, config_data[1], config_data[0]);

	if (fg_i2c_write(fuelgauge->client, CONFIG_REG, config_data, 2) < 0) {
		dev_info(&fuelgauge->client->dev,
			"%s: Failed to write CONFIG_REG\n", __func__);
		return -1;
	}

	if (fg_i2c_read(fuelgauge->client, STATUS_REG, status_data, 2) < 0) {
		dev_err(&fuelgauge->client->dev,
			"%s: Failed to read STATUS_REG\n", __func__);
		return -1;
	}

	if (status_data[1] & 0x01) {
		pr_info("%s : Battery Voltage is Very Low!! SW V EMPTY ENABLE\n", __func__);
		fuelgauge->sw_v_empty = MAX77849_VEMPTY_MODE;
	}

	psy_do_property("battery", get,
		POWER_SUPPLY_PROP_STATUS, value);
	if (value.intval ==
		POWER_SUPPLY_STATUS_CHARGING)
		return true;

	if ((int)fuelgauge->soc - current_soc <= STABLE_LOW_BATTERY_DIFF)
		overcurrent_limit_in_soc = STABLE_LOW_BATTERY_DIFF_LOWBATT;
	else
		overcurrent_limit_in_soc = STABLE_LOW_BATTERY_DIFF;

	if (((int)fuelgauge->soc - current_soc) >
		overcurrent_limit_in_soc) {
		dev_info(&fuelgauge->client->dev,
			"%s: Abnormal Current Consumption jump by %d units\n",
			__func__, (((int)fuelgauge->soc - current_soc)));
		dev_info(&fuelgauge->client->dev,
			"%s: Last Reported SOC (%d).\n",
			__func__, fuelgauge->soc);

		fuelgauge->is_low_batt_alarm = true;

		if (fuelgauge->soc >=
			LOW_BATTERY_SOC_REDUCE_UNIT)
			return true;
	}

	if (value.intval ==
			POWER_SUPPLY_STATUS_DISCHARGING) {
#if defined(FUELALERT_CHECK_VOLTAGE_FEATURE)
		if (fg_vcell >= POWER_OFF_VOLTAGE_HIGH_MARGIN) {
			dev_info(&fuelgauge->client->dev,
				"%s: skip setting battery level as 0 (voltage: %d)\n",
				__func__, fg_vcell);
			return true;
		}
#endif
		dev_err(&fuelgauge->client->dev,
			"Set battery level as 0, power off.\n");
		fuelgauge->soc = 0;
		value.intval = 0;
		psy_do_property("battery", set,
			POWER_SUPPLY_PROP_CAPACITY, value);
	}

	return true;
}

/* capacity is  0.1% unit */
static void max77849_fg_get_scaled_capacity(
				struct max77849_fuelgauge_info *fuelgauge,
				union power_supply_propval *val)
{
	val->intval = (val->intval < fuelgauge->pdata->capacity_min) ?
		0 : ((val->intval - fuelgauge->pdata->capacity_min) * 1000 /
		(fuelgauge->capacity_max - fuelgauge->pdata->capacity_min));

	dev_dbg(&fuelgauge->client->dev,
		"%s: scaled capacity (%d.%d)\n",
		__func__, val->intval/10, val->intval%10);
}

/* capacity is integer */
static void max77849_fg_get_atomic_capacity(
				struct max77849_fuelgauge_info *fuelgauge,
				union power_supply_propval *val)
{
	if (fuelgauge->pdata->capacity_calculation_type &
		SEC_FUELGAUGE_CAPACITY_TYPE_ATOMIC) {
		if (fuelgauge->capacity_old < val->intval)
			val->intval = fuelgauge->capacity_old + 1;
		else if (fuelgauge->capacity_old > val->intval)
			val->intval = fuelgauge->capacity_old - 1;
	}

	/* keep SOC stable in abnormal status */
	if (fuelgauge->pdata->capacity_calculation_type &
		SEC_FUELGAUGE_CAPACITY_TYPE_SKIP_ABNORMAL) {
		if (!fuelgauge->is_charging &&
			fuelgauge->capacity_old < val->intval) {
			dev_err(&fuelgauge->client->dev,
				"%s: capacity (old %d : new %d)\n",
				__func__, fuelgauge->capacity_old, val->intval);
			val->intval = fuelgauge->capacity_old;
		}
	}

	/* updated old capacity */
	fuelgauge->capacity_old = val->intval;
}

static int max77849_fg_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max77849_fuelgauge_info *fuelgauge =
		container_of(psy, struct max77849_fuelgauge_info, psy_fg);
	int soc_type = val->intval;

	switch (psp) {
		/* Cell voltage (VCELL, mV) */
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = get_fuelgauge_value(fuelgauge->client, FG_VOLTAGE);
		break;
		/* Additional Voltage Information (mV) */
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		switch (val->intval) {
		case SEC_BATTEY_VOLTAGE_OCV:
			val->intval = fg_read_vfocv(fuelgauge->client);
			break;
		case SEC_BATTEY_VOLTAGE_AVERAGE:
		default:
			val->intval = fg_read_avg_vcell(fuelgauge->client);
			break;
		}
		break;
		/* Current */
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		switch (val->intval) {
		case SEC_BATTEY_CURRENT_UA:
			val->intval =
				fg_read_current(fuelgauge->client, SEC_BATTEY_CURRENT_UA);
			break;
		case SEC_BATTEY_CURRENT_MA:
		default:
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_CURRENT);
			break;
		}
		break;
		/* Average Current */
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		switch (val->intval) {
		case SEC_BATTEY_CURRENT_UA:
			val->intval =
				fg_read_avg_current(fuelgauge->client,
				SEC_BATTEY_CURRENT_UA);
			break;
		case SEC_BATTEY_CURRENT_MA:
		default:
			val->intval =
				get_fuelgauge_value(fuelgauge->client, FG_CURRENT_AVG);
			break;
		}
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		val->intval = get_fuelgauge_value(fuelgauge->client, FG_FULLCAP) * 100 / fuelgauge->battery_data->Capacity;
		break;
		/* Full Capacity */
	case POWER_SUPPLY_PROP_ENERGY_NOW:
		switch (val->intval) {
		case SEC_BATTEY_CAPACITY_DESIGNED:
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_FULLCAP);
			break;
		case SEC_BATTEY_CAPACITY_ABSOLUTE:
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_MIXCAP);
			break;
		case SEC_BATTEY_CAPACITY_TEMPERARY:
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_AVCAP);
			break;
		case SEC_BATTEY_CAPACITY_CURRENT:
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_REPCAP);
			break;
		}
		break;
		/* SOC (%) */
	case POWER_SUPPLY_PROP_CAPACITY:
		if (val->intval == SEC_FUELGAUGE_CAPACITY_TYPE_RAW)
			val->intval = get_fuelgauge_value(fuelgauge->client, FG_RAW_SOC);
		else
			val->intval = get_fuelgauge_soc(fuelgauge->client);

		if (soc_type == SEC_FUELGAUGE_CAPACITY_TYPE_RAW)
			break;

		if (fuelgauge->pdata->capacity_calculation_type &
				(SEC_FUELGAUGE_CAPACITY_TYPE_SCALE |
				 SEC_FUELGAUGE_CAPACITY_TYPE_DYNAMIC_SCALE))
			max77849_fg_get_scaled_capacity(fuelgauge, val);

		/* capacity should be between 0% and 100%
		 * (0.1% degree)
		 */
		if (val->intval > 1000)
			val->intval = 1000;
		if (val->intval < 0)
			val->intval = 0;

		/* get only integer part */
		val->intval /= 10;

		if (!fuelgauge->is_charging && (fuelgauge->sw_v_empty == MAX77849_VEMPTY_MODE)) {
			pr_info("%s : SW V EMPTY. Decrease SOC\n", __func__);
			val->intval = 0;
		} else if ((fuelgauge->sw_v_empty == MAX77849_VEMPTY_RECOVERY_MODE) &&
				(val->intval == fuelgauge->capacity_old)) {
			fuelgauge->sw_v_empty = MAX77849_NORMAL_MODE;
		}

		/* check whether doing the wake_unlock */
		if ((val->intval > fuelgauge->pdata->fuel_alert_soc) &&
				fuelgauge->is_fuel_alerted) {
			wake_unlock(&fuelgauge->fuel_alert_wake_lock);
			fg_alert_init(fuelgauge->client,
					fuelgauge->pdata->fuel_alert_soc);
		}

		/* (Only for atomic capacity)
		 * In initial time, capacity_old is 0.
		 * and in resume from sleep,
		 * capacity_old is too different from actual soc.
		 * should update capacity_old
		 * by val->intval in booting or resume.
		 */
		if ((fuelgauge->initial_update_of_soc) &&
			(fuelgauge->sw_v_empty == MAX77849_NORMAL_MODE)){
			/* updated old capacity */
			fuelgauge->capacity_old = val->intval;
			fuelgauge->initial_update_of_soc = false;
			break;
		}

		if (fuelgauge->pdata->capacity_calculation_type &
				(SEC_FUELGAUGE_CAPACITY_TYPE_ATOMIC |
				 SEC_FUELGAUGE_CAPACITY_TYPE_SKIP_ABNORMAL))
			max77849_fg_get_atomic_capacity(fuelgauge, val);
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		val->intval = fuelgauge->capacity_max;
		break;
	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		val->intval = get_fuelgauge_value(fuelgauge->client, FG_TEMPERATURE);
		if (psp == POWER_SUPPLY_PROP_TEMP){
			val->intval = fg_adjust_temp(fuelgauge->client, psp, val->intval);

			if (!fuelgauge->low_temp_compensation_en &&
				(val->intval <= (int)fuelgauge->low_temp_limit)) {
				max77849_fg_low_temp_compensation(fuelgauge->client, true);
				fuelgauge->low_temp_compensation_en = true;
			} else if (fuelgauge->low_temp_compensation_en &&
				(val->intval >= (int)fuelgauge->low_temp_recovery)) {
				max77849_fg_low_temp_compensation(fuelgauge->client, false);
				fuelgauge->low_temp_compensation_en = false;
			}
		}

		break;
	case POWER_SUPPLY_PROP_STATUS:
	case POWER_SUPPLY_PROP_CHARGE_FULL:
	case POWER_SUPPLY_PROP_PRESENT:
		return -ENODATA;
	default:
		return -EINVAL;
	}
	return 0;
}

static int max77849_fg_calculate_dynamic_scale(
				struct max77849_fuelgauge_info *fuelgauge,
				int capacity)
{
	union power_supply_propval raw_soc_val;

	raw_soc_val.intval = get_fuelgauge_value(fuelgauge->client, FG_RAW_SOC);
	raw_soc_val.intval /= 10;

	if (raw_soc_val.intval <
		fuelgauge->pdata->capacity_max -
		fuelgauge->pdata->capacity_max_margin) {
		fuelgauge->capacity_max =
			fuelgauge->pdata->capacity_max -
			fuelgauge->pdata->capacity_max_margin;
		dev_dbg(&fuelgauge->client->dev, "%s: capacity_max (%d)\n",
			__func__, fuelgauge->capacity_max);
	} else {
		fuelgauge->capacity_max =
			(raw_soc_val.intval >
			fuelgauge->pdata->capacity_max +
			fuelgauge->pdata->capacity_max_margin) ?
			(fuelgauge->pdata->capacity_max +
			fuelgauge->pdata->capacity_max_margin) :
			raw_soc_val.intval;
		dev_dbg(&fuelgauge->client->dev, "%s: raw soc (%d)\n",
			__func__, fuelgauge->capacity_max);
	}

	if (capacity != 100) {
		fuelgauge->capacity_max =
			(fuelgauge->capacity_max * 100 / capacity);
	} else  {
		fuelgauge->capacity_max =
			(fuelgauge->capacity_max * 99 / 100);
	}
	/* update capacity_old for sec_fg_get_atomic_capacity algorithm */
	fuelgauge->capacity_old = capacity;

	dev_info(&fuelgauge->client->dev, "%s: %d is used for capacity_max\n",
		__func__, fuelgauge->capacity_max);

	return fuelgauge->capacity_max;
}

static int max77849_fg_set_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	struct max77849_fuelgauge_info *fuelgauge =
		container_of(psy, struct max77849_fuelgauge_info, psy_fg);

	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		if (!fuelgauge->pdata->jig_irq)
			fg_reset_capacity_by_jig_connection(fuelgauge->client);
		break;
	case POWER_SUPPLY_PROP_STATUS:
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		if (fuelgauge->pdata->capacity_calculation_type &
				SEC_FUELGAUGE_CAPACITY_TYPE_DYNAMIC_SCALE) {
#if defined(CONFIG_PREVENT_SOC_JUMP)
			max77849_fg_calculate_dynamic_scale(fuelgauge, val->intval);
#else
			max77849_fg_calculate_dynamic_scale(fuelgauge, 100);
#endif
		}
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		fuelgauge->cable_type = val->intval;
		if (val->intval == POWER_SUPPLY_TYPE_BATTERY)
			fuelgauge->is_charging = false;
		else
			fuelgauge->is_charging = true;

			if (fuelgauge->sw_v_empty != MAX77849_NORMAL_MODE) {
				fuelgauge->sw_v_empty = MAX77849_NORMAL_MODE;
				fuelgauge->initial_update_of_soc = true;
				fg_alert_init(fuelgauge->client,
					fuelgauge->pdata->fuel_alert_soc);
			}

		if (val->intval != POWER_SUPPLY_TYPE_BATTERY) {
			if (fuelgauge->is_low_batt_alarm) {
				dev_info(&fuelgauge->client->dev,
					"%s: Reset low_batt_alarm\n",
					__func__);
				fuelgauge->is_low_batt_alarm = false;
			}

			reset_low_batt_comp_cnt(fuelgauge->client);
		}
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (val->intval == SEC_FUELGAUGE_CAPACITY_TYPE_RESET) {
			fuelgauge->initial_update_of_soc = true;
			if (!fg_reset_soc(fuelgauge->client))
				return -EINVAL;
			else
				break;
		}
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (!fuelgauge->low_temp_compensation_en &&
			(val->intval <= (int)fuelgauge->low_temp_limit)) {
				max77849_fg_low_temp_compensation(fuelgauge->client, true);
				fuelgauge->low_temp_compensation_en = true;
		} else if (fuelgauge->low_temp_compensation_en &&
			(val->intval >= (int)fuelgauge->low_temp_recovery)) {
				max77849_fg_low_temp_compensation(fuelgauge->client, false);
				fuelgauge->low_temp_compensation_en = false;
		}
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
	case POWER_SUPPLY_PROP_PRESENT:
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		dev_info(&fuelgauge->client->dev,
				"%s: capacity_max changed, %d -> %d\n",
				__func__, fuelgauge->capacity_max, val->intval);
		fuelgauge->capacity_max = val->intval;
		fuelgauge->initial_update_of_soc = true;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void max77849_fg_isr_work(struct work_struct *work)
{
	struct max77849_fuelgauge_info *fuelgauge =
		container_of(work, struct max77849_fuelgauge_info, isr_work.work);

	/* process for fuel gauge chip */
	fg_fuelalert_process(fuelgauge, fuelgauge->is_fuel_alerted);

	/* process for others */
	if (fuelgauge->pdata->fuelalert_process != NULL)
		fuelgauge->pdata->fuelalert_process(fuelgauge->is_fuel_alerted);
}

static irqreturn_t max77849_fg_irq_thread(int irq, void *irq_data)
{
	struct max77849_fuelgauge_info *fuelgauge = irq_data;
	bool fuel_alerted;

	if (fuelgauge->pdata->fuel_alert_soc >= 0) {
		fuel_alerted =
			get_fuelgauge_value(fuelgauge->client, FG_CHECK_STATUS);

		dev_info(&fuelgauge->client->dev,
			"%s: Fuel-alert %salerted!\n",
			__func__, fuel_alerted ? "" : "NOT ");

		if (fuel_alerted == fuelgauge->is_fuel_alerted) {
			if (!fuelgauge->pdata->repeated_fuelalert) {
				dev_dbg(&fuelgauge->client->dev,
					"%s: Fuel-alert Repeated (%d)\n",
					__func__, fuelgauge->is_fuel_alerted);
				return IRQ_HANDLED;
			}
		}

		if (fuel_alerted)
			wake_lock(&fuelgauge->fuel_alert_wake_lock);
		else
			wake_unlock(&fuelgauge->fuel_alert_wake_lock);

		schedule_delayed_work(&fuelgauge->isr_work, 0);

		fuelgauge->is_fuel_alerted = fuel_alerted;
	}

	return IRQ_HANDLED;
}

static int max77849_fg_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(max77849_fg_attrs); i++) {
		rc = device_create_file(dev, &max77849_fg_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	dev_err(dev, "%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &max77849_fg_attrs[i]);
create_attrs_succeed:
	return rc;
}

ssize_t max77849_fg_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int i = 0;
	struct power_supply *psy = dev_get_drvdata(dev);
	struct max77849_fuelgauge_info *fg =
		container_of(psy, struct max77849_fuelgauge_info, psy_fg);
	const ptrdiff_t offset = attr - max77849_fg_attrs;
	char *str = NULL;

	switch (offset) {
	case FG_REG:
		break;
	case FG_DATA:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%02x%02x\n",
			fg->reg_data[1], fg->reg_data[0]);
		break;
	case FG_REGS:
		str = kzalloc(sizeof(char)*1024, GFP_KERNEL);
		if (!str)
			return -ENOMEM;

		fg_read_regs(fg->client, str);
		i += scnprintf(buf + i, PAGE_SIZE - i, "%s\n",
			str);

		kfree(str);
		break;
	default:
		i = -EINVAL;
		break;
	}

	return i;
}

ssize_t max77849_fg_store_attrs(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct max77849_fuelgauge_info *fg =
		container_of(psy, struct max77849_fuelgauge_info, psy_fg);
	const ptrdiff_t offset = attr - max77849_fg_attrs;
	int ret = 0;
	int x = 0;

	switch (offset) {
	case FG_REG:
		if (sscanf(buf, "%x\n", &x) == 1) {
			fg->reg_addr = x;
			if (fg_i2c_read(fg->client,
				fg->reg_addr, fg->reg_data, 2) < 0) {
				dev_err(dev, "%s: Error in read\n", __func__);
				break;
			}
			dev_dbg(dev,
				"%s: (read) addr = 0x%x, data = 0x%02x%02x\n",
				__func__, fg->reg_addr,
				fg->reg_data[1], fg->reg_data[0]);
			ret = count;
		}
		break;
	case FG_DATA:
		if (sscanf(buf, "%x\n", &x) == 1) {
			dev_dbg(dev, "%s: (write) addr = 0x%x, data = 0x%04x\n",
				__func__, fg->reg_addr, x);
			fg_write_and_verify_register(fg->client,
				fg->reg_addr, (u16)x);
			ret = count;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_OF
static int fuelgauge_parse_dt(struct device *dev,
			struct max77849_fuelgauge_info *fuelgauge)
{
	struct device_node *np = dev->of_node;
	struct device_node *np_battery_data = of_find_node_by_name(NULL, "battery_data");
	sec_battery_platform_data_t *pdata = fuelgauge->pdata;
	int ret;
	int i;

	/* reset, irq gpio info */
	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
	} else {
		pdata->fg_irq = of_get_named_gpio(np, "fuelgauge,fuel_int", 0);
		if (pdata->fg_irq < 0)
			pr_err("%s error reading fg_irq = %d\n", __func__, pdata->fg_irq);

		ret = of_property_read_u32(np, "fuelgauge,capacity_calculation_type",
				&pdata->capacity_calculation_type);
		if (ret < 0)
			pr_err("%s error reading capacity_calculation_type %d\n",
					__func__, ret);
		ret = of_property_read_u32(np, "fuelgauge,fuel_alert_soc",
				&pdata->fuel_alert_soc);
		if (ret < 0)
			pr_err("%s error reading pdata->fuel_alert_soc %d\n",
					__func__, ret);
		pdata->repeated_fuelalert = of_property_read_bool(np,
				"fuelgaguge,repeated_fuelalert");

		fuelgauge->using_temp_compensation = of_property_read_bool(np,
				"fuelgauge,using_temp_compensation");
		if (fuelgauge->using_temp_compensation) {
			ret = of_property_read_u32(np, "fuelgauge,low_temp_valrt",
						   &fuelgauge->low_temp_valrt);
			if  (ret){
				pr_info("%s: low_temp_valrt is Empty\n", __func__);
				fuelgauge->low_temp_valrt = 0xA2 ;	/* Default VALRT is 3.24V */
			}

			ret = of_property_read_u32(np, "fuelgauge,low_temp_limit",
						   &fuelgauge->low_temp_limit);
			if (ret<0)
				pr_err("%s: error reading low_temp_limit %d\n", __func__, ret);

			ret = of_property_read_u32(np, "fuelgauge,low_temp_recovery",
						   &fuelgauge->low_temp_recovery);
			if (ret<0)
				pr_err("%s: error reading low_temp_recovery %d\n", __func__, ret);
		}

		pdata->jig_irq = of_get_named_gpio(np, "fuelgauge,jig_gpio", 0);
		if (pdata->jig_irq < 0) {
			pr_err("%s error reading jig_gpio = %d\n",
					__func__,pdata->jig_irq);
			pdata->jig_irq = 0;
		} else {
			pdata->jig_irq_attr = IRQF_TRIGGER_RISING;
		}

		pr_info("%s: fg_irq: %d, "
				"calculation_type: 0x%x, fuel_alert_soc: %d,"
				"repeated_fuelalert: %d, jig_irq : %d\n", __func__, pdata->fg_irq,
				pdata->capacity_calculation_type,
				pdata->fuel_alert_soc, pdata->repeated_fuelalert, pdata->jig_irq);

		if (np_battery_data == NULL) {
			pr_err("%s np_battery_data NULL\n", __func__);
		} else {
			/* battery data */
			ret = of_property_read_u32(np_battery_data, "battery_data,capacity",
					&fuelgauge->battery_data->Capacity);
			if (ret < 0)
				pr_err("%s error reading battery_data,capacity %d\n", __func__, ret);

			ret = of_property_read_u32(np_battery_data, "battery_data,low_battery_comp_voltage",
					&fuelgauge->battery_data->low_battery_comp_voltage);
			if (ret < 0)
				pr_err("%s error reading battery_data,low_battery_comp_voltage %d\n",
						__func__, ret);

			pr_info("%s: count : %d\n", __func__, CURRENT_RANGE_MAX_NUM * TABLE_MAX);
			for(i = 0; i < (CURRENT_RANGE_MAX_NUM * TABLE_MAX); i++) {
				ret = of_property_read_u32_index(np_battery_data, "battery_data,low_battery_table", i,
						&fuelgauge->battery_data->low_battery_table[i/3][i%3]);
				if (ret < 0)
					pr_err("%s error reading battery_data,low_battery_table %d\n",
							__func__, ret);
				else {
					pr_info("[%d]", fuelgauge->battery_data->low_battery_table[i/3][i%3]);

					if ((i%3) == 2)
						pr_info("\n");
				}
			}
			ret = of_property_read_string(np_battery_data,
					"battery_data,type_str", (char const **)&fuelgauge->battery_data->type_str);
			if (ret)
				pr_info("%s: battery_data,type_str is Empty\n", __func__);
		}
	}
	return 0;
}
#else
static int fuelgauge_parse_dt(struct device *dev,
			struct synaptics_rmi4_power_data *pdata)
{
	return -ENODEV;
}
#endif

static int max77849_fuelgauge_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct max77849_fuelgauge_info *fuelgauge;
	sec_battery_platform_data_t *pdata = NULL;

	int ret = 0;
	union power_supply_propval raw_soc_val;

	dev_info(&client->dev,
			"%s: MAX77849 Fuelgauge Driver Loading\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		return -EIO;

	fuelgauge = kzalloc(sizeof(*fuelgauge), GFP_KERNEL);
	if (!fuelgauge)
		return -ENOMEM;

	mutex_init(&fuelgauge->fg_lock);

	fuelgauge->client = client;

	if (client->dev.of_node) {
		int error;

		pdata = devm_kzalloc(&client->dev,
				sizeof(sec_battery_platform_data_t),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_free;
		}
		fuelgauge->pdata = pdata;

		fuelgauge->battery_data = kzalloc(sizeof(struct battery_data_t),
				GFP_KERNEL);
		if(!fuelgauge->battery_data) {
			pr_err("Failed to allocate memory\n");
			ret = -ENOMEM;
			goto err_pdata_free;
		}

		error = fuelgauge_parse_dt(&client->dev, fuelgauge);
		if (error) {
			dev_err(&client->dev,
					"%s: Failed to get fuel_int\n", __func__);
		}
	} else {
		dev_err(&client->dev,
				"%s: Failed to get of_node\n", __func__);
		fuelgauge->pdata = client->dev.platform_data;
	}
	i2c_set_clientdata(client, fuelgauge);

	if (fuelgauge->pdata->fg_gpio_init != NULL) {
		if (!fuelgauge->pdata->fg_gpio_init()) {
			dev_err(&client->dev,
					"%s: Failed to Initialize GPIO\n", __func__);
			goto err_pdata_free;
		}
	}

	if (!fg_init(fuelgauge->client)) {
		dev_err(&client->dev,
				"%s: Failed to Initialize Fuelgauge\n", __func__);
		goto err_pdata_free;
	}

	fuelgauge->psy_fg.name		= "max77849-fuelgauge";
	fuelgauge->psy_fg.type		= POWER_SUPPLY_TYPE_UNKNOWN;
	fuelgauge->psy_fg.get_property	= max77849_fg_get_property;
	fuelgauge->psy_fg.set_property	= max77849_fg_set_property;
	fuelgauge->psy_fg.properties	= max77849_fuelgauge_props;
	fuelgauge->psy_fg.num_properties =
		ARRAY_SIZE(max77849_fuelgauge_props);
	fuelgauge->capacity_max = fuelgauge->pdata->capacity_max;
	raw_soc_val.intval = get_fuelgauge_value(client, FG_RAW_SOC);
	raw_soc_val.intval /= 10;
	if(raw_soc_val.intval > fuelgauge->pdata->capacity_max)
		max77849_fg_calculate_dynamic_scale(fuelgauge, 100);

	ret = power_supply_register(&client->dev, &fuelgauge->psy_fg);
	if (ret) {
		dev_err(&client->dev,
				"%s: Failed to Register psy_fg\n", __func__);
		goto err_pdata_free;
	}

	fuelgauge->is_fuel_alerted = false;
	if (fuelgauge->pdata->fuel_alert_soc >= 0) {
		if (fg_alert_init(fuelgauge->client,
					fuelgauge->pdata->fuel_alert_soc) > 0)
			wake_lock_init(&fuelgauge->fuel_alert_wake_lock,
					WAKE_LOCK_SUSPEND, "fuel_alerted");
		else {
			dev_err(&client->dev,
					"%s: Failed to Initialize Fuel-alert\n",
					__func__);
			goto err_supply_unreg;
		}
	}

	if (fuelgauge->pdata->fg_irq > 0) {
		INIT_DELAYED_WORK(
				&fuelgauge->isr_work, max77849_fg_isr_work);

		fuelgauge->fg_irq = gpio_to_irq(fuelgauge->pdata->fg_irq);
		dev_info(&client->dev,
				"%s: fg_irq = %d\n", __func__, fuelgauge->fg_irq);
		if (fuelgauge->fg_irq > 0) {
			ret = request_threaded_irq(fuelgauge->fg_irq,
					NULL, max77849_fg_irq_thread,
					IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING
					| IRQF_ONESHOT,
					"fuelgauge-irq", fuelgauge);
			if (ret) {
				dev_err(&client->dev,
						"%s: Failed to Reqeust IRQ\n", __func__);
				goto err_supply_unreg;
			}

			ret = enable_irq_wake(fuelgauge->fg_irq);
			if (ret < 0)
				dev_err(&client->dev,
						"%s: Failed to Enable Wakeup Source(%d)\n",
						__func__, ret);
		} else {
			dev_err(&client->dev, "%s: Failed gpio_to_irq(%d)\n",
					__func__, fuelgauge->fg_irq);
			goto err_supply_unreg;
		}
	}

	fuelgauge->initial_update_of_soc = true;
	fuelgauge->low_temp_compensation_en = false;
	fuelgauge->sw_v_empty = MAX77849_NORMAL_MODE;

	ret = max77849_fg_create_attrs(fuelgauge->psy_fg.dev);
	if (ret) {
		dev_err(&client->dev,
				"%s : Failed to create_attrs\n", __func__);
		goto err_irq;
	}

	dev_info(&client->dev,
			"%s: MAX77849 Fuelgauge Driver Loaded\n", __func__);
	return 0;

err_irq:
	if (fuelgauge->fg_irq > 0)
		free_irq(fuelgauge->fg_irq, fuelgauge);
	wake_lock_destroy(&fuelgauge->fuel_alert_wake_lock);
err_supply_unreg:
	power_supply_unregister(&fuelgauge->psy_fg);
err_pdata_free:
	if(fuelgauge->battery_data)
		devm_kfree(&client->dev, fuelgauge->battery_data);
	if(pdata)
		devm_kfree(&client->dev, pdata);
err_free:
	mutex_destroy(&fuelgauge->fg_lock);
	kfree(fuelgauge);

	dev_info(&client->dev, "%s: MAX77849 Fuelgauge probe failed\n", __func__);
	return ret;
}

static int max77849_fuelgauge_remove(
						struct i2c_client *client)
{
	struct max77849_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	if (fuelgauge->pdata->fuel_alert_soc >= 0)
		wake_lock_destroy(&fuelgauge->fuel_alert_wake_lock);
	kfree(fuelgauge->pdata->battery_data);

	return 0;
}

static int max77849_fuelgauge_suspend(struct device *dev)
{
	return 0;
}

static int max77849_fuelgauge_resume(struct device *dev)
{
	return 0;
}

static void max77849_fuelgauge_shutdown(struct i2c_client *client)
{
}

static const struct i2c_device_id max77849_fuelgauge_id[] = {
	{"max77849-fuelgauge", 0},
	{}
};

static const struct dev_pm_ops max77849_fuelgauge_pm_ops = {
	.suspend = max77849_fuelgauge_suspend,
	.resume  = max77849_fuelgauge_resume,
};

MODULE_DEVICE_TABLE(i2c, max77849_fuelgauge_id);
static struct of_device_id fuelgague_i2c_match_table[] = {
	{ .compatible = "max77849-fuelgauge,i2c", },
	{ },
};
MODULE_DEVICE_TABLE(i2c, fuelgague_i2c_match_table);

static struct i2c_driver max77849_fuelgauge_driver = {
	.driver = {
		   .name = "max77849-fuelgauge",
		   .owner = THIS_MODULE,
		   .of_match_table = fuelgague_i2c_match_table,
#ifdef CONFIG_PM
		   .pm = &max77849_fuelgauge_pm_ops,
#endif
	},
	.probe	= max77849_fuelgauge_probe,
	.remove	= max77849_fuelgauge_remove,
	.shutdown   = max77849_fuelgauge_shutdown,
	.id_table   = max77849_fuelgauge_id,
};

static int __init max77849_fuelgauge_init(void)
{
	pr_info("%s \n", __func__);

	return i2c_add_driver(&max77849_fuelgauge_driver);
}

static void __exit max77849_fuelgauge_exit(void)
{
	i2c_del_driver(&max77849_fuelgauge_driver);
}

module_init(max77849_fuelgauge_init);
module_exit(max77849_fuelgauge_exit);

MODULE_DESCRIPTION("MAX77849 Fuel Gauge Driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
