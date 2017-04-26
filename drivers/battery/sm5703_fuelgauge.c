/* drivers/battery/sm5703_fuelgauge.c
 * SM5703 Voltage Tracking Fuelgauge Driver
 *
 * Copyright (C) 2013
 * Author: Dongik Sin <dongik.sin@samsung.com>
 * Modified by SW Jung
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/battery/fuelgauge/sm5703_fuelgauge.h>
#include <linux/battery/fuelgauge/sm5703_fuelgauge_impl.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/math64.h>
#include <linux/compiler.h>
#include <linux/of_gpio.h>

#define SM5703_FG_DEVICE_NAME "sm5703-fg"
#define ALIAS_NAME "sm5703-fuelgauge"

#define FG_DET_BAT_PRESENT 1
#define MINVAL(a, b) ((a <= b) ? a : b)

enum battery_table_type {
	DISCHARGE_TABLE = 0,
	CHARGE_TABLE,
	Q_TABLE,
	TABLE_MAX,
};

static struct device_attribute sec_fg_attrs[] = {
	SEC_FG_ATTR(reg),
	SEC_FG_ATTR(data),
	SEC_FG_ATTR(regs),
};

static enum power_supply_property sm5703_fuelgauge_props[] = {
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
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
};

static inline int sm5703_fg_read_device(struct i2c_client *client,
				      int reg, int bytes, void *dest)
{
	int ret;

	if (bytes > 1)
		ret = i2c_smbus_read_i2c_block_data(client, reg, bytes, dest);
	else {
		ret = i2c_smbus_read_byte_data(client, reg);
		if (ret < 0)
			return ret;
		*(unsigned char *)dest = (unsigned char)ret;
	}
	return ret;
}

static int32_t sm5703_fg_i2c_read_word(struct i2c_client *client,
                        uint8_t reg_addr)
{
	uint16_t data = 0;
	int ret;
	ret = sm5703_fg_read_device(client, reg_addr, 2, &data);
	//dev_info(&client->dev, "%s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);

	if (ret < 0)
		return ret;
	else
		return data;

	// not use big endian
	//return (int32_t)be16_to_cpu(data);
}


static int32_t sm5703_fg_i2c_write_word(struct i2c_client *client,
                            uint8_t reg_addr,uint16_t data)
{
	int ret;

	// not use big endian
	//data = cpu_to_be16(data);
	ret = i2c_smbus_write_i2c_block_data(client, reg_addr, 2, (uint8_t *)&data);
//	dev_info(&client->dev, "%s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);

	return ret;
}

static int32_t sm5703_fg_i2c_verified_write_word(struct i2c_client *client,
		uint8_t reg_addr,uint16_t data)
{
	int ret;

	// not use big endian
	//data = cpu_to_be16(data);
	ret = i2c_smbus_write_i2c_block_data(client, reg_addr, 2, (uint8_t *)&data);
	if(ret<0)
	{
		msleep(50);
		dev_info(&client->dev, "1st fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);
		ret = i2c_smbus_write_i2c_block_data(client, reg_addr, 2, (uint8_t *)&data);
		if(ret<0)
		{
			msleep(50);
			dev_info(&client->dev, "2nd fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);
			ret = i2c_smbus_write_i2c_block_data(client, reg_addr, 2, (uint8_t *)&data);
			if(ret<0)
			{
				dev_info(&client->dev, "3rd fail i2c write %s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);
			}
		}
	}
	//	dev_info(&client->dev, "%s: ret = %d, addr = 0x%x, data = 0x%x\n", __func__, ret, reg_addr, data);

	return ret;
}

static unsigned int sm5703_get_vbat(struct i2c_client *client);
static unsigned int sm5703_get_ocv(struct i2c_client *client);
static int sm5703_get_curr(struct i2c_client *client);
static int sm5703_get_temperature(struct i2c_client *client);

static void sm5703_pr_ver_info(struct i2c_client *client)
{
	dev_info(&client->dev, "SM5703 Fuel-Gauge Ver %s\n", FG_DRIVER_VER);
}

static unsigned int sm5703_get_ocv(struct i2c_client *client)
{
	int ret;
	unsigned int ocv;// = 3500; /*3500 means 3500mV*/
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_OCV);
	if (ret<0) {
		pr_err("%s: read ocv reg fail\n", __func__);
		ocv = 4000;
	} else {
		ocv = ((ret&0x0700)>>8) * 1000; //integer bit;
		ocv = ocv + (((ret&0x00ff)*1000)/256); // integer + fractional bit
	}

	fuelgauge->info.batt_ocv = ocv;
	dev_info(&client->dev, "%s: read = 0x%x, ocv = %d\n", __func__, ret, ocv);

	return ocv;
}

static unsigned int sm5703_get_vbat(struct i2c_client *client)
{
	int ret;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	unsigned int vbat;/* = 3500; 3500 means 3500mV*/
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_VOLTAGE);
	if (ret<0) {
		pr_err("%s: read vbat reg fail", __func__);
		vbat = 4000;
	} else {
		vbat = ((ret&0x0700)>>8) * 1000; //integer bit
		vbat = vbat + (((ret&0x00ff)*1000)/256); // integer + fractional bit
	}
	fuelgauge->info.batt_voltage = vbat;

	//cal avgvoltage
	fuelgauge->info.batt_avgvoltage = ((fuelgauge->info.batt_avgvoltage*2) + (fuelgauge->info.p_batt_voltage+vbat))/4;

	dev_info(&client->dev, "%s: read = 0x%x, vbat = %d\n", __func__, ret, vbat);
	dev_info(&client->dev, "%s: batt_avgvoltage = %d\n", __func__, fuelgauge->info.batt_avgvoltage);

	if ((fuelgauge->volt_alert_flag == true) && vbat > 3400) {
		fuelgauge->volt_alert_flag = false;
		sec_hal_fg_fuelalert_init(client,
				fuelgauge->pdata->fuel_alert_soc);
		dev_info(&client->dev, "%s : volt_alert_flag = %d \n", __func__, fuelgauge->volt_alert_flag);
	}

	return vbat;
}

static int sm5703_get_curr(struct i2c_client *client)
{
	int ret;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	int curr;/* = 1000; 1000 means 1000mA*/
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CURRENT);
	if (ret<0) {
		pr_err("%s: read curr reg fail", __func__);
		curr = 0;
	} else {
		curr = ((ret&0x0700)>>8) * 1000; //integer bit
		curr = curr + (((ret&0x00ff)*1000)/256); // integer + fractional bit
		if(ret&0x8000) {
			curr *= -1;
		}
	}
	fuelgauge->info.batt_current = curr;

	//cal avgcurr
	fuelgauge->info.batt_avgcurrent = ((fuelgauge->info.batt_avgcurrent*2) + (fuelgauge->info.p_batt_current+curr))/4;

	dev_info(&client->dev, "%s: read = 0x%x, curr = %d\n", __func__, ret, curr);
	dev_info(&client->dev, "%s: batt_avgcurrent = %d\n", __func__, fuelgauge->info.batt_avgcurrent);

	return curr;
}

static int sm5703_get_temperature(struct i2c_client *client)
{
	int ret;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	int temp;/* = 250; 250 means 25.0oC*/
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_TEMPERATURE);
	if (ret<0) {
		pr_err("%s: read temp reg fail", __func__);
		temp = 0;
	} else {
		temp = ((ret&0x7F00)>>8) * 10; //integer bit
		temp = temp + (((ret&0x00ff)*10)/256); // integer + fractional bit
		if(ret&0x8000) {
			temp *= -1;
		}
	}
	fuelgauge->info.temp_fg = temp;

	dev_dbg(&client->dev, "%s: read = 0x%x, temp_fg = %d\n", __func__, ret, temp);

	return temp;
}

static unsigned int sm5703_get_device_id(struct i2c_client *client)
{
	int ret;
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_DEVICE_ID);
	//ret &= 0x00ff;
	dev_info(&client->dev, "%s: device_id = 0x%x\n", __func__, ret);

	return ret;
}

static bool sm5703_fg_check_reg_init_need(struct i2c_client *client)
{
	int ret;

	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_FG_OP_STATUS);

	if((ret & INIT_CHECK_MASK) == DISABLE_RE_INIT)
	{
		dev_dbg(&client->dev, "%s: return 0\n", __func__);
		return 0;
	}
	else
	{
		dev_dbg(&client->dev, "%s: return 1\n", __func__);
		return 1;
	}
}

static bool sm5703_fg_get_batt_present(struct i2c_client *client)
{
	// SM5703 is not suport batt present
	dev_dbg(&client->dev, "%s: sm5703_fg_get_batt_present\n", __func__);

	return true;
}

int calculate_iocv(struct i2c_client *client)
{
	int i;
	int max=0, min=0, sum=0, l_avg=0, s_avg=0, l_minmax_offset=0;
	int ret=0;

	for (i = SM5703_REG_IOCV_B_L_MIN; i <= SM5703_REG_IOCV_B_L_MAX; i++)
	{
		ret = sm5703_fg_i2c_read_word(client, i);
		if (i == SM5703_REG_IOCV_B_L_MIN)
		{
			max = ret;
			min = ret;
			sum = ret;
		}
		else
		{
			if(ret > max)
				max = ret;
			else if(ret < min)
				min = ret;
			sum = sum + ret;
		}
	}
	sum = sum - max - min;
	l_minmax_offset = max - min;
	l_avg = sum / (SM5703_REG_IOCV_B_L_MAX-SM5703_REG_IOCV_B_L_MIN-1);
	dev_info(&client->dev, "%s: iocv_l_max=0x%x, iocv_l_min=0x%x, iocv_l_sum=0x%x, iocv_l_avg=0x%x \n", __func__, max, min, sum, l_avg);

	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_END_V_IDX);
	pr_info("%s: iocv_status_read = addr : 0x%x , data : 0x%x\n", __func__, SM5703_REG_END_V_IDX, ret);

	if((ret & 0x0030) == 0x0030)
	{
		for (i = SM5703_REG_IOCV_B_S_MIN; i <= SM5703_REG_IOCV_B_S_MAX; i++)
		{
			ret = sm5703_fg_i2c_read_word(client, i);
			if (i == SM5703_REG_IOCV_B_S_MIN)
			{
				max = ret;
				min = ret;
				sum = ret;
			}
			else
			{
				if(ret > max)
					max = ret;
				else if(ret < min)
					min = ret;
				sum = sum + ret;
			}
		}
		sum = sum - max - min;
		s_avg = sum / (SM5703_REG_IOCV_B_S_MAX-SM5703_REG_IOCV_B_S_MIN-1);
		dev_info(&client->dev, "%s: iocv_s_max=0x%x, iocv_s_min=0x%x, iocv_s_sum=0x%x, iocv_s_avg=0x%x \n", __func__, max, min, sum, s_avg);
	}

	if(((abs(l_avg - s_avg) > 0x29) && (l_minmax_offset < 0x200)) || (s_avg == 0))
	{
		ret = l_avg;
	}
	else
	{
		ret = s_avg;
	}

	return ret;
}

static bool sm5703_fg_reg_init(struct i2c_client *client, int is_surge)
{
	int i, j, value, ret;
	uint8_t table_reg;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	dev_info(&client->dev, "%s: sm5703_fg_reg_init START!!\n", __func__);

	// start first param_ctrl unlock
	sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_CTRL, SM5703_FG_PARAM_UNLOCK_CODE);

	// RCE write
	for (i = 0; i < 3; i++)
	{
		sm5703_fg_i2c_write_word(client, SM5703_REG_RCE0+i, fuelgauge->info.rce_value[i]);
		dev_dbg(&client->dev, "%s: RCE write RCE%d = 0x%x : 0x%x\n", __func__,  i, SM5703_REG_RCE0+i, fuelgauge->info.rce_value[i]);
	}

	// DTCD write
	sm5703_fg_i2c_write_word(client, SM5703_REG_DTCD, fuelgauge->info.dtcd_value);
	dev_dbg(&client->dev, "%s: DTCD write DTCD = 0x%x : 0x%x\n", __func__, SM5703_REG_DTCD, fuelgauge->info.dtcd_value);

	// RS write
	sm5703_fg_i2c_write_word(client, SM5703_REG_RS, fuelgauge->info.rs_value[0]);
	dev_dbg(&client->dev, "%s: RS write RS = 0x%x : 0x%x\n", __func__, SM5703_REG_RS, fuelgauge->info.rs_value[0]);


	// VIT_PERIOD write
	sm5703_fg_i2c_write_word(client, SM5703_REG_VIT_PERIOD, fuelgauge->info.vit_period);
	dev_dbg(&client->dev, "%s: VIT_PERIOD write VIT_PERIOD = 0x%x : 0x%x\n", __func__, SM5703_REG_VIT_PERIOD, fuelgauge->info.vit_period);

	// TABLE_LEN write & pram unlock
	sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_CTRL, SM5703_FG_PARAM_UNLOCK_CODE | SM5703_FG_TABLE_LEN);

	for (i=0; i < 3; i++)
	{
		table_reg = SM5703_REG_TABLE_START + (i<<4);
		for(j=0; j <= SM5703_FG_TABLE_LEN; j++)
		{
			sm5703_fg_i2c_write_word(client, (table_reg + j), fuelgauge->info.battery_table[i][j]);
			//dev_dbg(&client->dev, "%s: TABLE write table[%d][%d] = 0x%x : 0x%x\n", __func__, i, j, (table_reg + j), fuelgauge->info.battery_table[i][j]);
		}
	}

	// MIX_MODE write
	sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MIX_FACTOR, fuelgauge->info.rs_value[1]);
	sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAX, fuelgauge->info.rs_value[2]);
	sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MIN, fuelgauge->info.rs_value[3]);
	sm5703_fg_i2c_write_word(client, SM5703_REG_MIX_RATE, fuelgauge->info.mix_value[0]);
	sm5703_fg_i2c_write_word(client, SM5703_REG_MIX_INIT_BLANK, fuelgauge->info.mix_value[1]);

	dev_dbg(&client->dev, "%s: RS_MIX_FACTOR = 0x%x, RS_MAX = 0x%x, RS_MIN = 0x%x, MIX_RATE = 0x%x, MIX_INIT_BLANK = 0x%x\n",
		__func__, fuelgauge->info.rs_value[1], fuelgauge->info.rs_value[2], fuelgauge->info.rs_value[3], fuelgauge->info.mix_value[0], fuelgauge->info.mix_value[1]);


	// CAL write
	sm5703_fg_i2c_write_word(client, SM5703_REG_VOLT_CAL, fuelgauge->info.volt_cal);
	sm5703_fg_i2c_write_word(client, SM5703_REG_CURR_CAL, fuelgauge->info.curr_cal);

	dev_dbg(&client->dev, "%s: VOLT_CAL = 0x%x, CURR_CAL = 0x%x\n", __func__, fuelgauge->info.volt_cal, fuelgauge->info.curr_cal);

	// TOPOFF SOC
	sm5703_fg_i2c_write_word(client, SM5703_REG_TOPOFFSOC, fuelgauge->info.topoff_soc);

	// INIT_last -  control register set
	value = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
	value &= 0xDFFF;
	value |= ENABLE_MIX_MODE | ENABLE_TEMP_MEASURE | (fuelgauge->info.enable_topoff_soc << 13);
	value |= ENABLE_MANUAL_OCV;

	pr_info("%s: SM5703_REG_CNTL reg : 0x%x\n", __func__, value);

	ret = sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, value);
	if (ret < 0)
		dev_info(&client->dev, "%s: fail control register set(%d)\n", __func__, ret);

	dev_info(&client->dev, "%s: LAST SM5703_REG_CNTL = 0x%x : 0x%x\n", __func__, SM5703_REG_CNTL, value);

	// LOCK
	value = SM5703_FG_PARAM_LOCK_CODE | SM5703_FG_TABLE_LEN;
	sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_CTRL, value);
	dev_info(&client->dev, "%s: LAST PARAM CTRL VALUE = 0x%x : 0x%x\n", __func__, SM5703_REG_PARAM_CTRL, value);

	// surge reset defence
	if(is_surge)
	{
		value = ((fuelgauge->info.batt_ocv<<8)/125);
	} else {
		value = calculate_iocv(client);
	}

	sm5703_fg_i2c_write_word(client, SM5703_REG_IOCV_MAN, value);
	dev_info(&client->dev, "%s: IOCV_MAN_WRITE = %d : 0x%x\n", __func__, fuelgauge->info.batt_ocv, value);

	return 1;
}

static bool sm5703_fg_init(struct i2c_client *client, bool is_surge)
{
	int ret;
	int ta_exist,reg_val;
	union power_supply_propval value;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	fuelgauge->info.is_FG_initialised = 0;
	fuelgauge->info.iocv_error_count = 0;

	board_fuelgauge_init(fuelgauge);

	//SM5703 i2c read check
	ret = sm5703_get_device_id(client);
	if (ret < 0)
	{
		dev_dbg(&client->dev, "%s: fail to do i2c read(%d)\n", __func__, ret);
		return false;
	}
	/* enable_topoff set */
	reg_val = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
	reg_val &= 0xDFFF;
	reg_val |= (fuelgauge->info.enable_topoff_soc << 13);

	pr_info("%s: SM5703_REG_CNTL reg : 0x%x\n", __func__, reg_val);

	sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, reg_val);

	if(sm5703_fg_check_reg_init_need(client))
	{
		sm5703_fg_reg_init(client, is_surge);
	}

	value.intval = POWER_SUPPLY_HEALTH_UNKNOWN;
	psy_do_property("sm5703-charger", get,
			POWER_SUPPLY_PROP_HEALTH, value);
	dev_dbg(&client->dev, "%s: get POWER_SUPPLY_PROP_HEALTH = 0x%x\n", __func__, value.intval);

	ta_exist = ((value.intval == POWER_SUPPLY_HEALTH_GOOD) |
		fuelgauge->is_charging) && (fuelgauge->info.batt_current > 30);

	dev_dbg(&client->dev, "%s: is_charging = %d, ta_exist = %d\n", __func__, fuelgauge->is_charging, ta_exist);

	// get first voltage measure to avgvoltage
	fuelgauge->info.batt_avgvoltage = sm5703_get_vbat(client);

	// get first temperature of fuelgauge
	fuelgauge->info.temp_fg= sm5703_get_temperature(client);

	// get first OCV
	fuelgauge->info.batt_ocv = sm5703_get_ocv(client);

	ret = sm5703_fg_i2c_read_word(client, 0x30);
	dev_info(&client->dev, "%s: sm5703 FG 0x30 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x31);
	dev_info(&client->dev, "%s: sm5703 FG 0x31 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x32);
	dev_info(&client->dev, "%s: sm5703 FG 0x32 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x33);
	dev_info(&client->dev, "%s: sm5703 FG 0x33 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x34);
	dev_info(&client->dev, "%s: sm5703 FG 0x34 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x35);
	dev_info(&client->dev, "%s: sm5703 FG 0x35 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x36);
	dev_info(&client->dev, "%s: sm5703 FG 0x36 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x37);
	dev_info(&client->dev, "%s: sm5703 FG 0x37 = 0x%x \n", __func__, ret);

	ret = sm5703_fg_i2c_read_word(client, 0x40);
	dev_info(&client->dev, "%s: sm5703 FG 0x40 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x41);
	dev_info(&client->dev, "%s: sm5703 FG 0x41 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x42);
	dev_info(&client->dev, "%s: sm5703 FG 0x42 = 0x%x \n", __func__, ret);
	ret = sm5703_fg_i2c_read_word(client, 0x43);
	dev_info(&client->dev, "%s: sm5703 FG 0x43 = 0x%x \n", __func__, ret);

	fuelgauge->info.is_FG_initialised = 1;

	return true;
}

static void sm5703_fg_test_read(struct i2c_client *client)
{
	int ret1, ret2, ret3, ret4;

	ret1 = sm5703_fg_i2c_read_word(client, 0xAC);
	ret2 = sm5703_fg_i2c_read_word(client, 0xAD);
	ret3 = sm5703_fg_i2c_read_word(client, 0xAE);
	ret4 = sm5703_fg_i2c_read_word(client, 0xAF);
	pr_info("0xAC=0x%04x, 0xAD=0x%04x, 0xAE=0x%04x, 0xAF=0x%04x \n", ret1, ret2, ret3, ret4);

	ret1 = sm5703_fg_i2c_read_word(client, 0xBC);
	ret2 = sm5703_fg_i2c_read_word(client, 0xBD);
	ret3 = sm5703_fg_i2c_read_word(client, 0xBE);
	ret4 = sm5703_fg_i2c_read_word(client, 0xBF);
	pr_info("0xBC=0x%04x, 0xBD=0x%04x, 0xBE=0x%04x, 0xBF=0x%04x \n", ret1, ret2, ret3, ret4);

	ret1 = sm5703_fg_i2c_read_word(client, 0xCC);
	ret2 = sm5703_fg_i2c_read_word(client, 0xCD);
	ret3 = sm5703_fg_i2c_read_word(client, 0xCE);
	ret4 = sm5703_fg_i2c_read_word(client, 0xCF);
	pr_info("0xCC=0x%04x, 0xCD=0x%04x, 0xCE=0x%04x, 0xCF=0x%04x \n", ret1, ret2, ret3, ret4);

	ret1 = sm5703_fg_i2c_read_word(client, 0x85);
	ret2 = sm5703_fg_i2c_read_word(client, 0x86);
	ret3 = sm5703_fg_i2c_read_word(client, 0x87);
	ret4 = sm5703_fg_i2c_read_word(client, 0x28);
	pr_info("0x85=0x%04x, 0x86=0x%04x, 0x87=0x%04x, 0x28=0x%04x \n", ret1, ret2, ret3, ret4);
}

void fg_abnormal_reset_check(struct i2c_client *client)
{
	int ret;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	// abnormal case.... SW reset
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_FG_OP_STATUS);
	if(((ret & INIT_CHECK_MASK) != DISABLE_RE_INIT) && (fuelgauge->info.is_FG_initialised == 1))
	{
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
		dev_info(&client->dev, "%s: SM5703 FG abnormal case!!!! SM5703_REG_CNTL : 0x%x\n", __func__, ret);
		if(ret == CNTL_REG_DEFAULT_VALUE)
		{
			// SW reset code
			if(sm5703_fg_i2c_verified_write_word(client, SM5703_REG_MISC, SW_RESET_CODE) < 0)
			{
				dev_info(&client->dev, "%s: Warning!!!! SM5703 FG abnormal case.... SW reset FAIL \n", __func__);
			}
			else
			{
				dev_info(&client->dev, "%s: SM5703 FG abnormal case.... SW reset OK\n", __func__);
			}
			// delay 200ms
			msleep(200);
			// init code
			sm5703_fg_init(client, true);
		}
	}
}

void fg_vbatocv_check(struct i2c_client *client, int ta_exist)
{
	int ret;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	// iocv error case cover start
	if((abs(fuelgauge->info.batt_current)<40) || ((ta_exist) && (abs(fuelgauge->info.batt_current)<fuelgauge->info.min_charge_curr)))
	{
		if(abs(fuelgauge->info.batt_ocv-fuelgauge->info.batt_voltage)>30) // 30mV over
		{
			fuelgauge->info.iocv_error_count ++;
		}
		if(fuelgauge->info.iocv_error_count > 5) // prevent to overflow
			fuelgauge->info.iocv_error_count = 6;
	}
	else
	{
		fuelgauge->info.iocv_error_count = 0;
	}

	dev_info(&client->dev, "%s: iocv_error_count (%d)\n", __func__, fuelgauge->info.iocv_error_count);

	if(fuelgauge->info.iocv_error_count > 5)
	{
		dev_info(&client->dev, "%s: p_v - v = (%d)\n", __func__, fuelgauge->info.p_batt_voltage - fuelgauge->info.batt_voltage);
		if(abs(fuelgauge->info.p_batt_voltage - fuelgauge->info.batt_voltage)>15) // 15mV over
		{
			fuelgauge->info.iocv_error_count = 0;
		}
		else
		{
			// mode change to mix RS manual mode
			dev_info(&client->dev, "%s: mode change to mix RS manual mode\n", __func__);
			// RS manual value write
			sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAN, fuelgauge->info.rs_value[0]);
			// run update
			sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 0);
			sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 1);
			// mode change
			ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
			ret = (ret | ENABLE_MIX_MODE) | ENABLE_RS_MAN_MODE; // +RS_MAN_MODE
			sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);
		}
	}
	else
	{
		if((fuelgauge->info.temperature/10) > 15)
		{
			if((fuelgauge->info.p_batt_voltage < fuelgauge->info.n_tem_poff) && (fuelgauge->info.batt_voltage < fuelgauge->info.n_tem_poff) && (!ta_exist))
			{
				dev_info(&client->dev, "%s: mode change to normal tem mix RS manual mode\n", __func__);
				// mode change to mix RS manual mode
				// RS manual value write
				if((fuelgauge->info.p_batt_voltage < (fuelgauge->info.n_tem_poff - fuelgauge->info.n_tem_poff_offset)) && (fuelgauge->info.batt_voltage < (fuelgauge->info.n_tem_poff - fuelgauge->info.n_tem_poff_offset)))
				{
					sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAN, fuelgauge->info.rs_value[0]>>1);
				}
				else
				{
					sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAN, fuelgauge->info.rs_value[0]);
				}
				// run update
				sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 0);
				sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 1);

				// mode change
				ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
				ret = (ret | ENABLE_MIX_MODE) | ENABLE_RS_MAN_MODE; // +RS_MAN_MODE
				sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);
			}
			else
			{
				dev_info(&client->dev, "%s: mode change to mix RS auto mode\n", __func__);

				// mode change to mix RS auto mode
				ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
				ret = (ret | ENABLE_MIX_MODE) & ~ENABLE_RS_MAN_MODE; // -RS_MAN_MODE
				sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);
			}
		}
		else
		{
			if((fuelgauge->info.p_batt_voltage < fuelgauge->info.l_tem_poff) && (fuelgauge->info.batt_voltage < fuelgauge->info.l_tem_poff) && (!ta_exist))
			{
				dev_info(&client->dev, "%s: mode change to normal tem mix RS manual mode\n", __func__);
				// mode change to mix RS manual mode
				// RS manual value write
				if((fuelgauge->info.p_batt_voltage < (fuelgauge->info.l_tem_poff - fuelgauge->info.l_tem_poff_offset)) && (fuelgauge->info.batt_voltage < (fuelgauge->info.l_tem_poff - fuelgauge->info.l_tem_poff_offset)))
				{
					sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAN, fuelgauge->info.rs_value[0]>>1);
				}
				else
				{
					sm5703_fg_i2c_write_word(client, SM5703_REG_RS_MAN, fuelgauge->info.rs_value[0]);
				}
				// run update
				sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 0);
				sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 1);

				// mode change
				ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
				ret = (ret | ENABLE_MIX_MODE) | ENABLE_RS_MAN_MODE; // +RS_MAN_MODE
				sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);
			}
			else
			{
				dev_info(&client->dev, "%s: mode change to mix RS auto mode\n", __func__);

				// mode change to mix RS auto mode
				ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
				ret = (ret | ENABLE_MIX_MODE) & ~ENABLE_RS_MAN_MODE; // -RS_MAN_MODE
				sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);
			}
		}
	}
	fuelgauge->info.p_batt_voltage = fuelgauge->info.batt_voltage;
	fuelgauge->info.p_batt_current = fuelgauge->info.batt_current;
	// iocv error case cover end
}

unsigned int sm5703_get_soc(struct i2c_client *client)
{
	int ret;
	unsigned int soc;
	int ta_exist;
	int curr_cal;
	int temp_cal_fact;
	int temp_gap=0;
	int high_temp_cal_fact=0, low_temp_cal_fact=0;
	union power_supply_propval value;
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	value.intval = POWER_SUPPLY_HEALTH_UNKNOWN;
	psy_do_property("sm5703-charger", get,
			POWER_SUPPLY_PROP_HEALTH, value);
	dev_dbg(&client->dev, "%s: get POWER_SUPPLY_PROP_HEALTH = 0x%x\n", __func__, value.intval);

	ta_exist = ((value.intval == POWER_SUPPLY_HEALTH_GOOD) | fuelgauge->is_charging) && (fuelgauge->info.batt_current > 30);
	dev_info(&client->dev, "%s: curr_cal = 0x%x , ta_exist = %d\n", __func__, fuelgauge->info.curr_cal, ta_exist);

	fg_abnormal_reset_check(client);
	fg_vbatocv_check(client, ta_exist);

	if(ta_exist)
		curr_cal = fuelgauge->info.curr_cal+(fuelgauge->info.charge_offset_cal<<8);
	else
		curr_cal = fuelgauge->info.curr_cal;

	temp_cal_fact = fuelgauge->info.temp_std - (fuelgauge->info.temp_fg/10);
	temp_cal_fact = temp_cal_fact / fuelgauge->info.temp_offset;
	temp_cal_fact = temp_cal_fact * fuelgauge->info.temp_offset_cal;
	curr_cal = curr_cal + (temp_cal_fact<<8);

	temp_gap = (fuelgauge->info.temperature/10) - fuelgauge->info.temp_std;
	if (fuelgauge->info.en_high_temp_cal && (temp_gap > 0))
	{
		if (ta_exist)
			high_temp_cal_fact = fuelgauge->info.high_temp_p_cal_fact;
		else
			high_temp_cal_fact = fuelgauge->info.high_temp_n_cal_fact;
		curr_cal = curr_cal + (((temp_gap / fuelgauge->info.high_temp_cal_denom)*high_temp_cal_fact)<<8);
	}
	else if (fuelgauge->info.en_low_temp_cal && (temp_gap < 0))
	{
		if (ta_exist)
			low_temp_cal_fact = fuelgauge->info.low_temp_p_cal_fact;
		else
			low_temp_cal_fact = fuelgauge->info.low_temp_n_cal_fact;
		temp_gap = -temp_gap;
		curr_cal = curr_cal + (((temp_gap / fuelgauge->info.low_temp_cal_denom)*low_temp_cal_fact)<<8);
	}

	dev_info(&client->dev, "%s: temp_fg = %d , batt_temp = %d, volt_cal = 0x%x, curr_cal = 0x%x, high_temp_cal_fact = %d, low_temp_cal_fact = %d\n",
		__func__, fuelgauge->info.temp_fg, fuelgauge->info.temperature, fuelgauge->info.volt_cal, curr_cal, high_temp_cal_fact, low_temp_cal_fact);

	sm5703_fg_i2c_write_word(client, SM5703_REG_CURR_CAL, curr_cal);

	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_SOC);
	if (ret<0) {
		pr_err("%s: Warning!!!! read soc reg fail\n", __func__);
		soc = 500;
	} else {
		soc = ((ret&0xff00)>>8) * 10; //integer bit;
		soc = soc + (((ret&0x00ff)*10)/256); // integer + fractional bit
	}

#if (defined(CONFIG_SEC_J5_PROJECT) || defined(CONFIG_SEC_J5N_PROJECT)) && !defined(CONFIG_MACH_J5LTE_CHN_CMCC)  /* only for J5 LDO1 noise */
	// for charger setting
	if(fuelgauge->is_charging) //During charging
	{
		if (fuelgauge->info.batt_ocv >= 4200)
		{
			value.intval = 0;
			psy_do_property("sm5703-charger", set,
					POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		}
		else if (fuelgauge->info.batt_ocv >= 4100)
		{
			value.intval = 1;
			psy_do_property("sm5703-charger", set,
					POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		}
		else if (fuelgauge->info.batt_ocv >= 4080)
		{
			value.intval = 2;
			psy_do_property("sm5703-charger", set,
					POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		}
		else if (fuelgauge->info.batt_ocv >= 4000)
		{
			value.intval = 3;
			psy_do_property("sm5703-charger", set,
					POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		}
		else if (fuelgauge->info.batt_ocv >= 3980)
		{
			value.intval = 4;
			psy_do_property("sm5703-charger", set,
					POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, value);
		}
	}
#endif

	dev_info(&client->dev, "%s: read = 0x%x, soc = %d\n", __func__, ret, soc);
	fuelgauge->info.batt_soc = soc;

	//temp for SM5703 FG debug
	sm5703_fg_test_read(client);

	// for low temp power off test
	if(fuelgauge->info.volt_alert_flag && (fuelgauge->info.temperature < -100))
	{
		dev_info(&client->dev, "%s: volt_alert_flag is TRUE!!!! SOC make force ZERO!!!!\n", __func__);
		fuelgauge->info.batt_soc = 0;
		return 0;
	}

	return soc;
}

#ifdef CONFIG_OF
static int get_battery_id(struct sec_fuelgauge_info *fuelgauge)
{
	// sm5703fg does not support this function
	return 0;
}
#define PROPERTY_NAME_SIZE 128

#define PINFO(format, args...) \
	printk(KERN_INFO "%s() line-%d: " format, \
		__func__, __LINE__, ## args)

#define DECL_PARAM_PROP(_id, _name) {.id = _id, .name = _name,}

static int sm5703_fg_parse_dt(struct sec_fuelgauge_info *fuelgauge)
{
	struct device *dev = &fuelgauge->client->dev;
	struct device_node *np = dev->of_node;
	char prop_name[PROPERTY_NAME_SIZE];
	int battery_id = -1;
	int table[16];
	int rce_value[3];
	int rs_value[4];
	int mix_value[2];
	int topoff_soc[2];
	int ext_temp_cal[8] = {1, 7, 1, 1, 1, 10, -1, -1};
	int set_temp_poff[4] = {3350,70,3300,70};

	int ret;
	int i, j;

	BUG_ON(dev == 0);
	BUG_ON(np == 0);

	// get battery_params node
	np = of_find_node_by_name(of_node_get(np), "battery_params");
	if (np == NULL) {
		PINFO("Cannot find child node \"battery_params\"\n");
		return -EINVAL;
	}

	// get battery_id
	if (of_property_read_u32(np, "battery,id", &battery_id) < 0)
		PINFO("not battery,id property\n");
	if (battery_id == -1)
		battery_id = get_battery_id(fuelgauge);
	PINFO("battery id = %d\n", battery_id);

	// get battery_table
	for (i = DISCHARGE_TABLE; i < TABLE_MAX; i++) {
		snprintf(prop_name, PROPERTY_NAME_SIZE,
			 "battery%d,%s%d", battery_id, "battery_table", i);

		ret = of_property_read_u32_array(np, prop_name, table, 16);
		if (ret < 0) {
			PINFO("Can get prop %s (%d)\n", prop_name, ret);
		}
		for (j = 0; j <= SM5703_FG_TABLE_LEN; j++)
		{
			fuelgauge->info.battery_table[i][j] = table[j];
//			PINFO("%s = <table[%d][%d] 0x%x>\n", prop_name, i, j, table[j]);
		}
	}

	// get rce
	for (i = 0; i < 3; i++) {
		snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "rce_value");
		ret = of_property_read_u32_array(np, prop_name, rce_value, 3);
		if (ret < 0) {
			PINFO("Can get prop %s (%d)\n", prop_name, ret);
		}
        fuelgauge->info.rce_value[i] = rce_value[i];
	}
	PINFO("%s = <0x%x 0x%x 0x%x>\n", prop_name, rce_value[0], rce_value[1], rce_value[2]);

	// get dtcd_value
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "dtcd_value");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.dtcd_value, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n",prop_name, fuelgauge->info.dtcd_value);

	// get rs_value
	for (i = 0; i < 4; i++) {
		snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "rs_value");
		ret = of_property_read_u32_array(np, prop_name, rs_value, 4);
		if (ret < 0) {
			PINFO("Can get prop %s (%d)\n", prop_name, ret);
		}
        fuelgauge->info.rs_value[i] = rs_value[i];
	}
	PINFO("%s = <0x%x 0x%x 0x%x 0x%x>\n", prop_name, rs_value[0], rs_value[1], rs_value[2], rs_value[3]);

	// get vit_period
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "vit_period");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.vit_period, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n",prop_name, fuelgauge->info.vit_period);

	// get mix_value
	for (i = 0; i < 2; i++) {
		snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "mix_value");
		ret = of_property_read_u32_array(np, prop_name, mix_value, 2);
		if (ret < 0) {
			PINFO("Can get prop %s (%d)\n", prop_name, ret);
		}
        fuelgauge->info.mix_value[i] = mix_value[i];
	}
	PINFO("%s = <0x%x 0x%x>\n", prop_name, mix_value[0], mix_value[1]);

	// battery_type
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "battery_type");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.battery_type, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <%d>\n", prop_name, fuelgauge->info.battery_type);

	// TOP OFF SOC
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "topoff_soc");
	ret = of_property_read_u32_array(np, prop_name, topoff_soc, 2);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);

	fuelgauge->info.enable_topoff_soc = topoff_soc[0];
	fuelgauge->info.topoff_soc = topoff_soc[1];
	PINFO("%s = <0x%x 0x%x>\n", prop_name, fuelgauge->info.enable_topoff_soc, fuelgauge->info.topoff_soc);

	// VOL & CURR CAL
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "volt_cal");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.volt_cal, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n", prop_name, fuelgauge->info.volt_cal);

	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "curr_cal");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.curr_cal, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n", prop_name, fuelgauge->info.curr_cal);

	// temp_std
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "temp_std");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.temp_std, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <%d>\n", prop_name, fuelgauge->info.temp_std);

	// temp_offset
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "temp_offset");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.temp_offset, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <%d>\n", prop_name, fuelgauge->info.temp_offset);

	// temp_offset_cal
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "temp_offset_cal");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.temp_offset_cal, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n", prop_name, fuelgauge->info.temp_offset_cal);

	// charge_offset_cal
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "charge_offset_cal");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.charge_offset_cal, 1);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	PINFO("%s = <0x%x>\n", prop_name, fuelgauge->info.charge_offset_cal);

	// min_charge_curr
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "min_charge_curr");
	ret = of_property_read_u32_array(np, prop_name, &fuelgauge->info.min_charge_curr, 1);
	if (ret < 0){
		fuelgauge->info.min_charge_curr = 200;
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	}
	PINFO("%s = <%d>\n", prop_name, fuelgauge->info.min_charge_curr);

	// ext_temp_calc
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "ext_temp_cal");
	ret = of_property_read_u32_array(np, prop_name, ext_temp_cal, 8);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->info.en_high_temp_cal = ext_temp_cal[0];
	fuelgauge->info.high_temp_cal_denom = ext_temp_cal[1];
	fuelgauge->info.high_temp_p_cal_fact = ext_temp_cal[2];
	fuelgauge->info.high_temp_n_cal_fact = ext_temp_cal[3];
	fuelgauge->info.en_low_temp_cal = ext_temp_cal[4];
	fuelgauge->info.low_temp_cal_denom = ext_temp_cal[5];
	fuelgauge->info.low_temp_p_cal_fact = ext_temp_cal[6];
	fuelgauge->info.low_temp_n_cal_fact = ext_temp_cal[7];
	PINFO("%s = <%d, %d, %d, %d, %d, %d, %d, %d>\n", prop_name, fuelgauge->info.en_high_temp_cal, fuelgauge->info.high_temp_cal_denom, fuelgauge->info.high_temp_p_cal_fact, fuelgauge->info.high_temp_n_cal_fact, fuelgauge->info.en_low_temp_cal, fuelgauge->info.low_temp_cal_denom, fuelgauge->info.low_temp_p_cal_fact, fuelgauge->info.low_temp_n_cal_fact);

	// tem poff level
	snprintf(prop_name, PROPERTY_NAME_SIZE, "battery%d,%s", battery_id, "tem_poff");
	ret = of_property_read_u32_array(np, prop_name, set_temp_poff, 4);
	if (ret < 0)
		PINFO("Can get prop %s (%d)\n", prop_name, ret);
	fuelgauge->info.n_tem_poff = set_temp_poff[0];
	fuelgauge->info.n_tem_poff_offset = set_temp_poff[1];
	fuelgauge->info.l_tem_poff = set_temp_poff[2];
	fuelgauge->info.l_tem_poff_offset = set_temp_poff[3];

	PINFO("%s = <%d, %d, %d, %d>\n", prop_name, fuelgauge->info.n_tem_poff, fuelgauge->info.n_tem_poff_offset, fuelgauge->info.l_tem_poff, fuelgauge->info.l_tem_poff_offset);

	return 0;
}
#else
static int sm5703_fg_parse_dt(struct sec_fuelgauge_info *fuelgauge)
{
	return 0;
}
#endif

bool sec_hal_fg_init(struct i2c_client *client)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	struct battery_data_t *battery_data;
	dev_info(&client->dev, "sm5703 sec_hal_fg_init...\n");
	mutex_init(&fuelgauge->info.param_lock);
	mutex_lock(&fuelgauge->info.param_lock);
	if (client->dev.of_node) {
	    // Load battery data from DTS
	    sm5703_fg_parse_dt(fuelgauge);

	} else {
		// Copy battery data from platform data
		battery_data = &get_battery_data(fuelgauge);
		fuelgauge->info.battery_type =
			battery_data->battery_type;
	}
	//struct battery_data_t *battery_data = &get_battery_data(fuelgauge);

	sm5703_fg_init(client, false);
	sm5703_pr_ver_info(client);
	fuelgauge->info.temperature = 250;

	mutex_unlock(&fuelgauge->info.param_lock);
	dev_info(&client->dev, "sm5703 hal fg init OK\n");
	return true;
}

bool sec_hal_fg_suspend(struct i2c_client *client)
{
	dev_dbg(&client->dev, "%s: sec_hal_fg_suspend\n", __func__);

	return true;
}

bool sec_hal_fg_resume(struct i2c_client *client)
{
	dev_dbg(&client->dev, "%s: sec_hal_fg_resume\n", __func__);

	return true;
}

bool sec_hal_fg_fuelalert_init(struct i2c_client *client, int soc)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int ret;
	int value_v_alarm, value_soc_alarm;

	if (soc >= 0)
	{
		dev_info(&client->dev, "%s: sec_hal_fg_fuelalert_init\n", __func__);

		// remove interrupt
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_INTFG);

		// check status
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_STATUS);

		// remove all mask
		sm5703_fg_i2c_write_word(client,SM5703_REG_INTFG_MASK, 0x0000);

		/* enable volt alert only, other alert mask*/
		ret = MASK_L_SOC_INT|MASK_H_TEM_INT|MASK_L_TEM_INT;
		sm5703_fg_i2c_write_word(client,SM5703_REG_INTFG_MASK,ret);
		fuelgauge->info.irq_ctrl = ~(ret);

		/* set volt and soc alert threshold */
		value_v_alarm = 0x02B4; // 2700mV
		sm5703_fg_i2c_write_word(client, SM5703_REG_V_ALARM, value_v_alarm);
		value_soc_alarm = 0x0100; // 1.00%
		sm5703_fg_i2c_write_word(client, SM5703_REG_SOC_ALARM, value_soc_alarm);

		/* update parameters */
		sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 0);
		sm5703_fg_i2c_write_word(client, SM5703_REG_PARAM_RUN_UPDATE, 1);

		// enabel volt alert control, other alert disable
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_CNTL);
		ret = ret | ENABLE_V_ALARM;
		ret = ret & (~ENABLE_SOC_ALARM & ~ENABLE_T_H_ALARM & ~ENABLE_T_L_ALARM);
		sm5703_fg_i2c_write_word(client, SM5703_REG_CNTL, ret);

		dev_info(&client->dev, "%s: irq_ctrl=0x%x, REG_CNTL=0x%x, V_ALARM=0x%x, SOC_ALARM=0x%x \n", __func__, fuelgauge->info.irq_ctrl, ret, value_v_alarm, value_soc_alarm);
	}

	/* alert flag init*/
	fuelgauge->info.soc_alert_flag = false;
	fuelgauge->is_fuel_alerted = false;

	return true;
}

bool sec_hal_fg_is_fuelalerted(struct i2c_client *client)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	int ret;

	dev_info(&client->dev, "%s: sec_hal_fg_is_fuelalerted\n", __func__);

	/* alert process */
	ret = sm5703_fg_i2c_read_word(client, SM5703_REG_INTFG);
	dev_info(&client->dev, "%s: SM5703_REG_INTFG(0x%x)\n",
		__func__, ret);

	if(ret & fuelgauge->info.irq_ctrl)
	{
		// check status
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_STATUS);
		dev_info(&client->dev, "%s: SM5703_REG_STATUS(0x%x)\n",
			__func__, ret);

		if(ret & fuelgauge->info.irq_ctrl)
		{
			return true;
		}
	}

	return false;
}

bool sec_hal_fg_fuelalert_process(void *irq_data, bool is_fuel_alerted)
{
	struct sec_fuelgauge_info *fuelgauge = irq_data;
	struct i2c_client *client = fuelgauge->client;
	int ret;

	dev_info(&client->dev, "%s: sec_hal_fg_fuelalert_process :: is_fuel_alerted=%d \n", __func__, is_fuel_alerted);

	if(is_fuel_alerted)
	{
		ret = sm5703_fg_i2c_read_word(client, SM5703_REG_STATUS);
		dev_info(&client->dev, "%s: SM5703_REG_STATUS(0x%x)\n",
			__func__, ret);

		/* not use SOC alarm
		if(ret & fuelgauge->info.irq_ctrl & ENABLE_SOC_ALARM) {
			fuelgauge->info.soc_alert_flag = true;
			// todo more action
		}
		*/

		if(ret & fuelgauge->info.irq_ctrl & ENABLE_V_ALARM) {
			fuelgauge->info.volt_alert_flag = true;
			// todo more action
		}
	}

	return true;
}

/* capacity is  0.1% unit */
static void sec_fg_get_scaled_capacity(
				struct sec_fuelgauge_info *fuelgauge,
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
static void sec_fg_get_atomic_capacity(
				struct sec_fuelgauge_info *fuelgauge,
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

static int sec_fg_check_capacity_max(
		struct sec_fuelgauge_info *fuelgauge, int capacity_max)
{
	int new_capacity_max = capacity_max;

	if (new_capacity_max < (fuelgauge->pdata->capacity_max -
				fuelgauge->pdata->capacity_max_margin - 10)) {
		new_capacity_max =
			(fuelgauge->pdata->capacity_max -
			 fuelgauge->pdata->capacity_max_margin);

		dev_info(&fuelgauge->client->dev, "%s: set capacity max(%d --> %d)\n",
				__func__, capacity_max, new_capacity_max);
	} else if (new_capacity_max > (fuelgauge->pdata->capacity_max +
				fuelgauge->pdata->capacity_max_margin)) {
		new_capacity_max =
			(fuelgauge->pdata->capacity_max +
			 fuelgauge->pdata->capacity_max_margin);

		dev_info(&fuelgauge->client->dev, "%s: set capacity max(%d --> %d)\n",
				__func__, capacity_max, new_capacity_max);
	}

	return new_capacity_max;
}

static int sec_fg_calculate_dynamic_scale(
				struct sec_fuelgauge_info *fuelgauge, int capacity)
{
	union power_supply_propval raw_soc_val;

	raw_soc_val.intval = SEC_FUELGAUGE_CAPACITY_TYPE_RAW;
	if (!sec_hal_fg_get_property(fuelgauge->client,
		POWER_SUPPLY_PROP_CAPACITY,
		&raw_soc_val))
		return -EINVAL;
	raw_soc_val.intval /= 10;

	if (raw_soc_val.intval <
		fuelgauge->pdata->capacity_max -
		fuelgauge->pdata->capacity_max_margin) {
		fuelgauge->capacity_max =
			fuelgauge->pdata->capacity_max -
			fuelgauge->pdata->capacity_max_margin;
		dev_dbg(&fuelgauge->client->dev, "%s: capacity_max (%d)",
			__func__, fuelgauge->capacity_max);
	} else {
		fuelgauge->capacity_max =
			(raw_soc_val.intval >
			fuelgauge->pdata->capacity_max +
			fuelgauge->pdata->capacity_max_margin) ?
			(fuelgauge->pdata->capacity_max +
			fuelgauge->pdata->capacity_max_margin) :
			raw_soc_val.intval;
		dev_dbg(&fuelgauge->client->dev, "%s: raw soc (%d)",
			__func__, fuelgauge->capacity_max);
	}

	if (capacity != 100) {
		fuelgauge->capacity_max = sec_fg_check_capacity_max(
				fuelgauge, (fuelgauge->capacity_max * 100 / capacity));
	} else {
		fuelgauge->capacity_max =
			(fuelgauge->capacity_max * 99 / 100);
	}
	/* update capacity_old for sec_fg_get_atomic_capacity algorithm */
	fuelgauge->capacity_old = capacity;

	dev_info(&fuelgauge->client->dev, "%s: %d is used for capacity_max\n",
		__func__, fuelgauge->capacity_max);

	return fuelgauge->capacity_max;
}

bool sec_hal_fg_full_charged(struct i2c_client *client)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	fuelgauge->info.flag_full_charge = 1;

	dev_dbg(&client->dev, "%s: full_charged\n", __func__);

	return true;
}

bool sm5703_fg_reset(struct i2c_client *client)
{
	dev_dbg(&client->dev, "%s: sec_hal_fg_reset\n", __func__);

	// SW reset code
	sm5703_fg_i2c_write_word(client, 0x90, 0x0008);
	// delay 200ms
	msleep(200);
	// init code
	sm5703_fg_init(client, false);

	return true;
}

static void sm5703_fg_reset_capacity_by_jig_connection(struct sec_fuelgauge_info *fuelgauge)
{
	union power_supply_propval value;
	int ret;

	ret = sm5703_fg_i2c_read_word(fuelgauge->client, SM5703_REG_CNTL);
	ret |= 0x0010;
	sm5703_fg_i2c_write_word(fuelgauge->client, SM5703_REG_CNTL, ret);
	/* If JIG is attached, the voltage is set as 1079 */
	value.intval = 1079;
	psy_do_property("battery", set,
			POWER_SUPPLY_PROP_VOLTAGE_NOW, value);
}

bool sec_hal_fg_get_property(struct i2c_client *client,
			     enum power_supply_property psp,
			     union power_supply_propval *val)
{
	union power_supply_propval value;
	struct sec_fuelgauge_info *fuelgauge =
		i2c_get_clientdata(client);

	psy_do_property("sm5703-charger", get,
			POWER_SUPPLY_PROP_STATUS, value);
	fuelgauge->info.flag_full_charge =
		(value.intval == POWER_SUPPLY_STATUS_FULL) ? 1 : 0;
	fuelgauge->info.flag_chg_status =
		(value.intval == POWER_SUPPLY_STATUS_CHARGING) ? 1 : 0;

//	dev_info(&client->dev, "%s: psp=%d, val->intval=%d\n", __func__, psp, val->intval);

	switch (psp) {
	/* Cell voltage (VCELL, mV) */
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = sm5703_get_vbat(client);
		break;
	/* Additional Voltage Information (mV) */
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		switch (val->intval) {
		case SEC_BATTEY_VOLTAGE_AVERAGE:
			sm5703_get_vbat(client);
			val->intval = fuelgauge->info.batt_avgvoltage;
			break;
		case SEC_BATTEY_VOLTAGE_OCV:
			val->intval = sm5703_get_ocv(client);
			break;
		}
		break;

		case POWER_SUPPLY_PROP_PRESENT:
                // SM5703 is not suport this prop
                sm5703_fg_get_batt_present(client);
			break;
			/* Current (mA) */
		case POWER_SUPPLY_PROP_CURRENT_NOW:
			val->intval = sm5703_get_curr(client);
			break;
			/* Average Current (mA) */
		case POWER_SUPPLY_PROP_CURRENT_AVG:
			sm5703_get_curr(client);
			val->intval = fuelgauge->info.batt_avgcurrent;
			break;
		case POWER_SUPPLY_PROP_CHARGE_FULL:
			val->intval =
				(fuelgauge->info.batt_soc >= 1000) ? true : false;
			break;
			/* SOC (%) */
		case POWER_SUPPLY_PROP_CAPACITY:
			/* SM5703 F/G unit is 0.1%, raw ==> convert the unit to 0.01% */
			if (val->intval == SEC_FUELGAUGE_CAPACITY_TYPE_RAW)
				val->intval = sm5703_get_soc(client) * 10;
			else
				val->intval = sm5703_get_soc(client);
			break;
			/* Battery Temperature */
		case POWER_SUPPLY_PROP_TEMP:
			/* Target Temperature */
		case POWER_SUPPLY_PROP_TEMP_AMBIENT:
			val->intval = sm5703_get_temperature(client);
			break;
		default:
			return false;
	}
	return true;
}

static int sm5703_fg_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct sec_fuelgauge_info *fuelgauge =
		container_of(psy, struct sec_fuelgauge_info, psy_fg);
	int soc_type = val->intval;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
	case POWER_SUPPLY_PROP_CURRENT_NOW:
	case POWER_SUPPLY_PROP_CURRENT_AVG:
	case POWER_SUPPLY_PROP_ENERGY_NOW:
	case POWER_SUPPLY_PROP_CAPACITY:
	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		if (!sec_hal_fg_get_property(fuelgauge->client, psp, val))
			return -EINVAL;
		if (psp == POWER_SUPPLY_PROP_CAPACITY) {
			if (soc_type == SEC_FUELGAUGE_CAPACITY_TYPE_RAW)
				break;

			if (fuelgauge->pdata->capacity_calculation_type &
				(SEC_FUELGAUGE_CAPACITY_TYPE_SCALE |
				 SEC_FUELGAUGE_CAPACITY_TYPE_DYNAMIC_SCALE))
				sec_fg_get_scaled_capacity(fuelgauge, val);

			/* capacity should be between 0% and 100%
			 * (0.1% degree)
			 */
			if (val->intval > 1000)
				val->intval = 1000;
			if (val->intval < 0)
				val->intval = 0;

			/* get only integer part */
			val->intval /= 10;

			/* check whether doing the wake_unlock */
			if ((val->intval > fuelgauge->pdata->fuel_alert_soc) &&
				fuelgauge->is_fuel_alerted) {
				wake_unlock(&fuelgauge->fuel_alert_wake_lock);
				sec_hal_fg_fuelalert_init(fuelgauge->client,
					fuelgauge->pdata->fuel_alert_soc);
			}

			/* (Only for atomic capacity)
			 * In initial time, capacity_old is 0.
			 * and in resume from sleep,
			 * capacity_old is too different from actual soc.
			 * should update capacity_old
			 * by val->intval in booting or resume.
			 */
			if (fuelgauge->initial_update_of_soc) {
				/* updated old capacity */
				fuelgauge->capacity_old = val->intval;
				fuelgauge->initial_update_of_soc = false;
				break;
			}

			if (fuelgauge->pdata->capacity_calculation_type &
				(SEC_FUELGAUGE_CAPACITY_TYPE_ATOMIC |
				 SEC_FUELGAUGE_CAPACITY_TYPE_SKIP_ABNORMAL))
				sec_fg_get_atomic_capacity(fuelgauge, val);
		}
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		val->intval = fuelgauge->capacity_max;
		break;
	case POWER_SUPPLY_PROP_STATUS:
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		return -ENODATA;
	default:
		return -EINVAL;
	}
	return 0;
}

bool sec_hal_fg_set_property(struct i2c_client *client,
			     enum power_supply_property psp,
			     const union power_supply_propval *val)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);
	dev_info(&client->dev, "%s: psp=%d\n", __func__, psp);

	switch (psp) {
		/* Battery Temperature */
	case POWER_SUPPLY_PROP_TEMP:
		fuelgauge->info.temperature = val->intval;
                break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
                break;
	default:
		return false;
	}
	return true;
}

ssize_t sec_hal_fg_show_attrs(struct device *dev,
				const ptrdiff_t offset, char *buf)
{
	struct power_supply *psy = dev_get_drvdata(dev);
	struct sec_fuelgauge_info *fg =
		container_of(psy, struct sec_fuelgauge_info, psy_fg);
	int i = 0;

    dev_dbg(dev, "%s: offset=%d\n", __func__, offset);

	switch (offset) {
	case FG_REG:
		break;
	case FG_DATA:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n",
			       fg->info.batt_soc);

		break;
	default:
		i = -EINVAL;
		break;
	}
	return i;
}

ssize_t sec_hal_fg_store_attrs(struct device *dev,
				const ptrdiff_t offset,
				const char *buf, size_t count)
{
	int ret = 0;

        dev_dbg(dev, "%s: offset=%d\n", __func__, offset);

	switch (offset) {
	case FG_REG:
		break;
	case FG_DATA:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int sm5703_fg_set_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	struct sec_fuelgauge_info *fuelgauge =
		container_of(psy, struct sec_fuelgauge_info, psy_fg);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		if (fuelgauge->pdata->capacity_calculation_type &
				SEC_FUELGAUGE_CAPACITY_TYPE_DYNAMIC_SCALE) {
#if defined(CONFIG_PREVENT_SOC_JUMP)
			sec_fg_calculate_dynamic_scale(fuelgauge, val->intval);
#else
			sec_fg_calculate_dynamic_scale(fuelgauge, 100);
#endif
		}
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		fuelgauge->cable_type = val->intval;
		if (val->intval == POWER_SUPPLY_TYPE_BATTERY)
			fuelgauge->is_charging = false;
		else
			fuelgauge->is_charging = true;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		if (val->intval == SEC_FUELGAUGE_CAPACITY_TYPE_RESET) {
			fuelgauge->initial_update_of_soc = true;
			if (!sm5703_fg_reset(fuelgauge->client))
				return -EINVAL;
			else
				break;
		}
	case POWER_SUPPLY_PROP_TEMP:
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		if (!sec_hal_fg_set_property(fuelgauge->client, psp, val))
			return -EINVAL;
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		dev_info(&fuelgauge->client->dev,
				"%s: capacity_max changed, %d -> %d\n",
				__func__, fuelgauge->capacity_max, val->intval);
		fuelgauge->capacity_max = sec_fg_check_capacity_max(fuelgauge, val->intval);
		fuelgauge->initial_update_of_soc = true;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		sm5703_fg_reset_capacity_by_jig_connection(fuelgauge);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void sm5703_fg_isr_work(struct work_struct *work)
{
	struct sec_fuelgauge_info *fuelgauge =
		container_of(work, struct sec_fuelgauge_info, isr_work.work);

	/* process for fuel gauge chip */
	sec_hal_fg_fuelalert_process(fuelgauge, fuelgauge->is_fuel_alerted);

	/* process for others */
	if (fuelgauge->pdata->fuelalert_process != NULL)
		fuelgauge->pdata->fuelalert_process(fuelgauge->is_fuel_alerted);
}

static irqreturn_t sm5703_fg_irq_thread(int irq, void *irq_data)
{
	struct sec_fuelgauge_info *fuelgauge = irq_data;
	bool fuel_alerted;

	if (fuelgauge->pdata->fuel_alert_soc >= 0) {
		fuel_alerted =
			sec_hal_fg_is_fuelalerted(fuelgauge->client);

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

		fuelgauge->is_fuel_alerted = fuel_alerted;

		schedule_delayed_work(&fuelgauge->isr_work, 0);
	}

	return IRQ_HANDLED;
}

static int sm5703_create_attrs(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(sec_fg_attrs); i++) {
		rc = device_create_file(dev, &sec_fg_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	dev_err(dev, "%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &sec_fg_attrs[i]);
create_attrs_succeed:
	return rc;
}

ssize_t sec_fg_show_attrs(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	const ptrdiff_t offset = attr - sec_fg_attrs;
	int i = 0;

	switch (offset) {
	case FG_REG:
	case FG_DATA:
	case FG_REGS:
		i = sec_hal_fg_show_attrs(dev, offset, buf);
		break;
	default:
		i = -EINVAL;
		break;
	}

	return i;
}

ssize_t sec_fg_store_attrs(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	const ptrdiff_t offset = attr - sec_fg_attrs;
	int ret = 0;

	switch (offset) {
	case FG_REG:
	case FG_DATA:
		ret = sec_hal_fg_store_attrs(dev, offset, buf, count);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_OF
static int fuelgauge_parse_dt(struct device *dev,
			struct sec_fuelgauge_info *fuelgauge)
{
	struct device_node *np = dev->of_node;
	sec_battery_platform_data_t *pdata = fuelgauge->pdata;
	int ret;

	/* reset, irq gpio info */
	if (np == NULL) {
		pr_err("%s np NULL\n", __func__);
	} else {
		ret = of_get_named_gpio(np, "fuelgauge,fuel_int", 0);
		if (ret > 0) {
			pdata->fg_irq = ret;
			pr_info("%s reading fg_irq = %d\n", __func__, ret);
		}

		ret = of_get_named_gpio(np, "fuelgauge,bat_int", 0);
		if (ret > 0) {
			pdata->bat_irq_gpio = ret;
			pdata->bat_irq = gpio_to_irq(ret);
			pr_info("%s reading bat_int_gpio = %d\n", __func__, ret);
		}

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

		pr_info("%s: fg_irq: %d, "
				"calculation_type: 0x%x, fuel_alert_soc: %d,\n"
				"repeated_fuelalert: %d\n", __func__, pdata->fg_irq,
				pdata->capacity_calculation_type,
				pdata->fuel_alert_soc, pdata->repeated_fuelalert
				);
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

static int sm5703_fuelgauge_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct sec_fuelgauge_info *fuelgauge;
	sec_battery_platform_data_t *pdata = NULL;
//	struct battery_data_t *battery_data = NULL;
	int ret = 0;
	union power_supply_propval raw_soc_val;

	dev_info(&client->dev,
		"%s: SM5703 Fuelgauge Driver Loading\n", __func__);

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
		error = fuelgauge_parse_dt(&client->dev, fuelgauge);
		if (error) {
			dev_err(&client->dev,
				"%s: Failed to get fuel_int\n", __func__);
		}
	} else	{
		dev_err(&client->dev,
			"%s: Failed to get of_node\n", __func__);
		fuelgauge->pdata = client->dev.platform_data;
	}
	i2c_set_clientdata(client, fuelgauge);

	if (fuelgauge->pdata->fg_gpio_init != NULL) {
		dev_err(&client->dev,
				"%s: @@@\n", __func__);
		if (!fuelgauge->pdata->fg_gpio_init()) {
			dev_err(&client->dev,
					"%s: Failed to Initialize GPIO\n", __func__);
			goto err_devm_free;
		}
	}

	if (!sec_hal_fg_init(fuelgauge->client)) {
		dev_err(&client->dev,
			"%s: Failed to Initialize Fuelgauge\n", __func__);
		goto err_devm_free;
	}

	fuelgauge->psy_fg.name		= "sm5703-fuelgauge";
	fuelgauge->psy_fg.type		= POWER_SUPPLY_TYPE_UNKNOWN;
	fuelgauge->psy_fg.get_property	= sm5703_fg_get_property;
	fuelgauge->psy_fg.set_property	= sm5703_fg_set_property;
	fuelgauge->psy_fg.properties	= sm5703_fuelgauge_props;
	fuelgauge->psy_fg.num_properties =
		ARRAY_SIZE(sm5703_fuelgauge_props);
        fuelgauge->capacity_max = fuelgauge->pdata->capacity_max;
        raw_soc_val.intval = SEC_FUELGAUGE_CAPACITY_TYPE_RAW;
        sec_hal_fg_get_property(fuelgauge->client,
                POWER_SUPPLY_PROP_CAPACITY, &raw_soc_val);
        raw_soc_val.intval /= 10;
        if(raw_soc_val.intval > fuelgauge->pdata->capacity_max)
                sec_fg_calculate_dynamic_scale(fuelgauge, 100);

	ret = power_supply_register(&client->dev, &fuelgauge->psy_fg);
	if (ret) {
		dev_err(&client->dev,
			"%s: Failed to Register psy_fg\n", __func__);
		goto err_free;
	}

	fuelgauge->is_fuel_alerted = false;
	if (fuelgauge->pdata->fuel_alert_soc >= 0) {
		if (sec_hal_fg_fuelalert_init(fuelgauge->client,
			fuelgauge->pdata->fuel_alert_soc))
			wake_lock_init(&fuelgauge->fuel_alert_wake_lock,
				WAKE_LOCK_SUSPEND, "fuel_alerted");
		else {
			dev_err(&client->dev,
				"%s: Failed to Initialize Fuel-alert\n",
				__func__);
			goto err_irq;
		}
	}

	if (fuelgauge->pdata->fg_irq > 0) {
		INIT_DELAYED_WORK(
			&fuelgauge->isr_work, sm5703_fg_isr_work);

		fuelgauge->fg_irq = gpio_to_irq(fuelgauge->pdata->fg_irq);
		dev_info(&client->dev,
			"%s: fg_irq = %d\n", __func__, fuelgauge->fg_irq);
		if (fuelgauge->fg_irq > 0) {
			ret = request_threaded_irq(fuelgauge->fg_irq,
					NULL, sm5703_fg_irq_thread,
					IRQF_TRIGGER_FALLING
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
	if (sec_bat_check_jig_status())
		sm5703_fg_reset_capacity_by_jig_connection(fuelgauge);

	ret = sm5703_create_attrs(fuelgauge->psy_fg.dev);
	if (ret) {
		dev_err(&client->dev,
			"%s : Failed to create_attrs\n", __func__);
		goto err_irq;
	}

	dev_info(&client->dev,
		"%s: SEC Fuelgauge Driver Loaded\n", __func__);
	return 0;

err_irq:
	if (fuelgauge->fg_irq > 0)
		free_irq(fuelgauge->fg_irq, fuelgauge);
	wake_lock_destroy(&fuelgauge->fuel_alert_wake_lock);
err_supply_unreg:
	power_supply_unregister(&fuelgauge->psy_fg);
err_devm_free:
	if(pdata)
		devm_kfree(&client->dev, pdata);
//	if(battery_data)
//		devm_kfree(&client->dev, battery_data);
err_free:
	mutex_destroy(&fuelgauge->fg_lock);
	kfree(fuelgauge);

	dev_info(&client->dev, "%s: Fuel gauge probe failed\n", __func__);
	return ret;
}

static int sm5703_fuelgauge_remove(
						struct i2c_client *client)
{
	struct sec_fuelgauge_info *fuelgauge = i2c_get_clientdata(client);

	if (fuelgauge->pdata->fuel_alert_soc >= 0)
		wake_lock_destroy(&fuelgauge->fuel_alert_wake_lock);

	return 0;
}

static int sm5703_fuelgauge_suspend(struct device *dev)
{
	struct sec_fuelgauge_info *fuelgauge = dev_get_drvdata(dev);

	if (!sec_hal_fg_suspend(fuelgauge->client))
		dev_err(&fuelgauge->client->dev,
			"%s: Failed to Suspend Fuelgauge\n", __func__);

	return 0;
}

static int sm5703_fuelgauge_resume(struct device *dev)
{
	struct sec_fuelgauge_info *fuelgauge = dev_get_drvdata(dev);

	if (!sec_hal_fg_resume(fuelgauge->client))
		dev_err(&fuelgauge->client->dev,
			"%s: Failed to Resume Fuelgauge\n", __func__);

	return 0;
}

static void sm5703_fuelgauge_shutdown(struct i2c_client *client)
{
}

static const struct i2c_device_id sm5703_fuelgauge_id[] = {
	{"sm5703-fuelgauge", 0},
	{}
};

static const struct dev_pm_ops sm5703_fuelgauge_pm_ops = {
	.suspend = sm5703_fuelgauge_suspend,
	.resume  = sm5703_fuelgauge_resume,
};

MODULE_DEVICE_TABLE(i2c, sm5703_fuelgauge_id);
static struct of_device_id fuelgague_i2c_match_table[] = {
	{ .compatible = "sm5703-fuelgauge,i2c", },
	{ },
};
MODULE_DEVICE_TABLE(i2c, fuelgague_i2c_match_table);

static struct i2c_driver sm5703_fuelgauge_driver = {
	.driver = {
		   .name = "sm5703-fuelgauge",
		   .owner = THIS_MODULE,
		   .of_match_table = fuelgague_i2c_match_table,
#ifdef CONFIG_PM
		   .pm = &sm5703_fuelgauge_pm_ops,
#endif
	},
	.probe	= sm5703_fuelgauge_probe,
	.remove	= sm5703_fuelgauge_remove,
	.shutdown   = sm5703_fuelgauge_shutdown,
	.id_table   = sm5703_fuelgauge_id,
};

static int __init sm5703_fuelgauge_init(void)
{
	pr_info("%s \n", __func__);

	return i2c_add_driver(&sm5703_fuelgauge_driver);
}

static void __exit sm5703_fuelgauge_exit(void)
{
	i2c_del_driver(&sm5703_fuelgauge_driver);
}

module_init(sm5703_fuelgauge_init);
module_exit(sm5703_fuelgauge_exit);

MODULE_DESCRIPTION("Samsung SM5703 Fuel Gauge Driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
