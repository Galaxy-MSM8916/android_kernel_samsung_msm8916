/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  2013_0523_01
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/i2c/mxts.h>
#include <asm/unaligned.h>
#include <linux/firmware.h>
#include <linux/string.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>

#if (CHECK_ANTITOUCH |CHECK_ANTITOUCH_SERRANO |CHECK_ANTITOUCH_GOLDEN)
#define MXT_T61_TIMER_ONESHOT			0
#define MXT_T61_TIMER_REPEAT			1
#define MXT_T61_TIMER_CMD_START		1
#define MXT_T61_TIMER_CMD_STOP		2
#endif

#define MXT_BOOT_ADDRESS		0x24

#define MAX_RETRY 3
#define USE_OPEN_CLOSE

#if ENABLE_TOUCH_KEY
int tsp_keycodes[NUMOFKEYS] = {
	KEY_MENU,
	KEY_BACK,
};
char *tsp_keyname[NUMOFKEYS] = {
	"Menu",
	"Back",
};
static u16 tsp_keystatus;
#endif

#ifdef TSP_BOOSTER
static void set_dvfs_lock(struct mxt_data *data, int on);
#endif
static int mxt_wait_for_chg(struct mxt_data *data, u16 time);

#ifdef CONFIG_OF
struct regulator *i2c_vddo_vreg = NULL;
static int mxt_parse_dt(struct device *dev,
			struct mxt_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	u32 coords[4];
	int ret;

	/* reset, irq gpio info */
	pdata->tsp_en = of_get_named_gpio(np, "mxts,tsppwr_en", 0);
	if(pdata->tsp_en < 0){
		pr_err("unable to get tsp_en\n");
	}	
	pdata->tsp_en1 = of_get_named_gpio(np, "mxts,tsppwr_en1", 0);
	if(pdata->tsp_en1 < 0){
		pr_err("unable to get tsp_en2\n");
	}		
	pdata->tsp_int = of_get_named_gpio(np, "mxts,irq-gpio", 0);
	if(pdata->tsp_int < 0){
		pr_err("unable to get tsp_int\n");
	}	
	
	pr_err("%s tsp_en= %d, tsp_en1= %d, tsp_int= %d\n",
			__func__, pdata->tsp_en, pdata->tsp_en1, pdata->tsp_int);

	ret = of_property_read_u32_array(np, "mxts,tsp_coord", coords, 4);
	if (ret && (ret != -EINVAL)) {
		printk(KERN_ERR "%s: Unable to read mxts,tsp_coord\n", __func__);
		return ret;
	}
 	pdata->num_xnode = coords[0];
 	pdata->num_ynode = coords[1];
 	pdata->max_x = coords[2];
 	pdata->max_y = coords[3];
 	pdata->boot_address = MXT_BOOT_ADDRESS;

 	pr_err("%s num_xnode= %d, num_ynode= %d, max_x= %d, max_y= %d, boot_addr= 0x%02x\n",
 			__func__, pdata->num_xnode, pdata->num_ynode,
 			pdata->max_x, pdata->max_y, pdata->boot_address);	

	return 0;
}
#else
static int mxt_parse_dt(struct device *dev,
			struct mxt_platform_data *pdata)
{
	return -ENODEV;
}
#endif

static int mxt_request_gpio(struct mxt_data *data)
{
	int ret;
	printk(KERN_INFO "%s\n", __func__);

	if(data->pdata->tsp_en > 0){
		ret = gpio_request(data->pdata->tsp_en, "mxts,tsppwr_en");
		if (ret) {
			pr_err("%s: unable to request tsppwer_en [%d]\n",
					__func__, data->pdata->tsp_en);
			return ret;
		}
	}

	if(data->pdata->tsp_en1 > 0){	
		ret = gpio_request(data->pdata->tsp_en1, "mxts,tsppwr_en1");
		if (ret) {
			pr_err("%s: unable to request tsppwer_en1 [%d]\n",
					__func__, data->pdata->tsp_en1);
			return ret;
		}
	}

	ret = gpio_request(data->pdata->tsp_int, "mxts,tsp_int");
	if (ret) {
		pr_err("%s: unable to request tsp_int [%d]\n",
				__func__, data->pdata->tsp_int);
		return ret;
	}

	return ret;
}

static int mxt_power_onoff(struct mxt_data *data, bool enable)
{
	int ret = 0;

	printk(KERN_INFO "%s enable : %d\n", __func__, enable);
	
	if(i2c_vddo_vreg != NULL){
		if(enable){
				ret = regulator_enable(i2c_vddo_vreg);
		}else{
				ret = regulator_disable(i2c_vddo_vreg);
		}
	}else{
			pr_err("%s: i2c_vddo_vreg is null  vdd en:%d\n",	__func__, enable);
	}

	ret = gpio_direction_output(data->pdata->tsp_en, enable);
	if (ret) {
		pr_err("%s: unable to set_direction for tsp_en [%d]\n",
			 __func__, data->pdata->tsp_en);
		return -EINVAL;
	}
	ret = gpio_direction_output(data->pdata->tsp_en1, enable);
	if (ret) {
		pr_err("%s: unable to set_direction for tsp_en1 [%d]\n",
			 __func__, data->pdata->tsp_en1);
		return -EINVAL;
	}

	msleep(30);

	return 0;
}

static int mxt_read_mem(struct mxt_data *data, u16 reg, u8 len, void *buf)
{
	int ret = 0, i = 0, retry = 0;
	u16 le_reg = cpu_to_le16(reg);
	struct i2c_msg msg[2] = {
		{
			.addr = data->client->addr,
			.flags = 0,
			.len = 2,
			.buf = (u8 *)&le_reg,
		},
		{
			.addr = data->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

#if defined(CONFIG_MACH_GOLDEN_VZW) || defined(CONFIG_MACH_GOLDEN_ATT)
	err_retry:
#endif
		for (i = 0; i < 3 ; i++) {
			ret = i2c_transfer(data->client->adapter, msg, 2);
			
			if (ret < 0)
				dev_err(&data->client->dev, "%s fail[%d] address[0x%x], retry%d\n",
					__func__, ret, le_reg, retry);
			else
				break;
		}
	
#if defined(CONFIG_MACH_GOLDEN_VZW) || defined(CONFIG_MACH_GOLDEN_ATT)
		if (ret < 0 && retry < MAX_RETRY) {
			data->pdata->power_off(); 
			msleep(10); 
			data->pdata->power_on(); 
	
			mxt_wait_for_chg(data, MXT_HW_RESET_TIME);
	
			dev_err(&data->client->dev, "i2c trasfer failed 3 times. so executed H/W reset \n"); 
			retry++;
	
			goto err_retry;
		}
#endif


	if (ret == 2)
		return 0;
	else {
		dev_err(&data->client->dev, "i2c transfer 2 msgs. but msg executed %d\n", ret);
		return -EIO;
	}
}

static int mxt_write_mem(struct mxt_data *data,
		u16 reg, u8 len, const u8 *buf)
{
	int ret = 0, i = 0;
	u8 tmp[len + 2];

#if TSP_USE_ATMELDBG
	if (data->atmeldbg.block_access)
		return 0;
#endif

	put_unaligned_le16(cpu_to_le16(reg), tmp);
	memcpy(tmp + 2, buf, len);

	for (i = 0; i < 3 ; i++) {
		ret = i2c_master_send(data->client, tmp, sizeof(tmp));
		if (ret < 0)
			dev_err(&data->client->dev,	"%s %d times write error on address[0x%x,0x%x]\n",
				__func__, i, tmp[1], tmp[0]);
		else
			break;
	}

	return ret == sizeof(tmp) ? 0 : -EIO;
}

static struct mxt_object *
	mxt_get_object(struct mxt_data *data, u8 type)
{
	struct mxt_object *object;
	int i;

	if (!data->objects) {
		dev_err(&data->client->dev, "Objects is null\n");
		return NULL;
	}

	for (i = 0; i < data->info.object_num; i++) {
		object = data->objects + i;
		if (object->type == type)
			return object;
	}

	dev_err(&data->client->dev, "Invalid object type T%d\n",
		type);

	return NULL;
}

static int mxt_read_message(struct mxt_data *data,
				 struct mxt_message *message)
{
	struct mxt_object *object;

	object = mxt_get_object(data, MXT_GEN_MESSAGEPROCESSOR_T5);
	if (!object)
		return -EINVAL;

	return mxt_read_mem(data, object->start_address,
			sizeof(struct mxt_message), message);
}

#if !(DUAL_CFG)
static int mxt_read_message_reportid(struct mxt_data *data,
	struct mxt_message *message, u8 reportid)
{
	int try = 0;
	int error;
	int fail_count;

	fail_count = data->max_reportid * 2;

	while (++try < fail_count) {
		error = mxt_read_message(data, message);
		if (error)
			return error;

		if (message->reportid == 0xff)
			continue;

		if (message->reportid == reportid)
			return 0;
	}

	return -EINVAL;
}
#endif

static int mxt_read_object(struct mxt_data *data,
				u8 type, u8 offset, u8 *val)
{
	struct mxt_object *object;
	int error = 0;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	error = mxt_read_mem(data, object->start_address + offset, 1, val);
	if (error)
		dev_err(&data->client->dev, "Error to read T[%d] offset[%d] val[%d]\n",
			type, offset, *val);
	return error;
}

static int mxt_write_object(struct mxt_data *data,
				 u8 type, u8 offset, u8 val)
{
	struct mxt_object *object;
	int error = 0;
	u16 reg;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if (offset >= object->size * object->instances) {
		dev_err(&data->client->dev, "Tried to write outside object T%d offset:%d, size:%d\n",
			type, offset, object->size);
		return -EINVAL;
	}
	reg = object->start_address;
	error = mxt_write_mem(data, reg + offset, 1, &val);
	if (error)
		dev_err(&data->client->dev, "Error to write T[%d] offset[%d] val[%d]\n",
			type, offset, val);

	return error;
}

static u32 mxt_make_crc24(u32 crc, u8 byte1, u8 byte2)
{
	static const u32 crcpoly = 0x80001B;
	u32 res;
	u16 data_word;

	data_word = (((u16)byte2) << 8) | byte1;
	res = (crc << 1) ^ (u32)data_word;

	if (res & 0x1000000)
		res ^= crcpoly;

	return res;
}

static int mxt_calculate_infoblock_crc(struct mxt_data *data,
		u32 *crc_pointer)
{
	u32 crc = 0;
	u8 mem[7 + data->info.object_num * 6];
	int ret;
	int i;

	ret = mxt_read_mem(data, 0, sizeof(mem), mem);

	if (ret)
		return ret;

	for (i = 0; i < sizeof(mem) - 1; i += 2)
		crc = mxt_make_crc24(crc, mem[i], mem[i + 1]);

	*crc_pointer = mxt_make_crc24(crc, mem[i], 0) & 0x00FFFFFF;

	return 0;
}

static int mxt_read_info_crc(struct mxt_data *data, u32 *crc_pointer)
{
	u16 crc_address;
	u8 msg[3];
	int ret;

	/* Read Info block CRC address */
	crc_address = MXT_OBJECT_TABLE_START_ADDRESS +
			data->info.object_num * MXT_OBJECT_TABLE_ELEMENT_SIZE;

	ret = mxt_read_mem(data, crc_address, 3, msg);
	if (ret)
		return ret;

	*crc_pointer = msg[0] | (msg[1] << 8) | (msg[2] << 16);

	return 0;
}

#if !(DUAL_CFG)
static int mxt_read_config_crc(struct mxt_data *data, u32 *crc)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	struct mxt_object *object;
	int error;

	object = mxt_get_object(data, MXT_GEN_COMMANDPROCESSOR_T6);
	if (!object)
		return -EIO;

	/* Try to read the config checksum of the existing cfg */
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
		MXT_COMMAND_REPORTALL, 1);

	/* Read message from command processor, which only has one report ID */
	error = mxt_read_message_reportid(data, &message, object->max_reportid);
	if (error) {
		dev_err(dev, "Failed to retrieve CRC\n");
		return error;
	}

	/* Bytes 1-3 are the checksum. */
	*crc = message.message[1] | (message.message[2] << 8) |
		(message.message[3] << 16);

	return 0;
}
#endif

#if (CHECK_ANTITOUCH |CHECK_ANTITOUCH_SERRANO |CHECK_ANTITOUCH_GOLDEN)
void mxt_t61_timer_set(struct mxt_data *data, int num, u8 mode, u8 cmd, u16 msPeriod)					
{
	struct mxt_object *object;														
	int ret = 0;																
	u16 reg;																	
	u8 buf[5] = {3, 0, 0, 0, 0};

	buf[1] = cmd;
	buf[2] = mode;
	buf[3] = msPeriod & 0xFF;
	buf[4] = (msPeriod >> 8) & 0xFF;

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);
	reg = object->start_address;
	
	if (num == 2)
	reg = reg+5;

	ret = mxt_write_mem(data, reg+0, 5,(const u8*)&buf);

	dev_info(&data->client->dev, "Timer%d Enabled with [%d msec]\n",
		num, msPeriod);
}

void mxt_t61_timer2_set(struct mxt_data *data, u8 mode, u8 cmd, u16 msPeriod)					
{
	struct mxt_object *object;														
	int ret = 0;																
	u16 reg;																	
	u8 buf[5] = {3, 0, 0, 0, 0};

	buf[1] = cmd;
	buf[2] = mode;
	buf[3] = msPeriod & 0xFF;
	buf[4] = (msPeriod >> 8) & 0xFF;

	object = mxt_get_object(data, MXT_SPT_TIMER_T61);
	reg = object->start_address;
	ret = mxt_write_mem(data, reg+5, 5,(const u8*)&buf);

	pr_info("[TSP] T61 Timer2 Enabled %d\n", msPeriod);
}

void mxt_t8_cal_set(struct mxt_data *data, u8 mstime)
{
	if (mstime)
		data->pdata->check_autocal = 1;					
	else
		data->pdata->check_autocal = 0;

	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8,4, mstime);
}
#endif

#if CHECK_ANTITOUCH
static int diff_two_point(s16 x, s16 y, s16 oldx, s16 oldy)
{
	s16 diffx, diffy;
	s16 distance;

	diffx = x-oldx;
	diffy = y-oldy;
	distance = abs(diffx) + abs(diffy);

	return distance;
}

static void mxt_check_coordinate(struct mxt_data *data,
				u8 detect, u8 id,
				s16 x, s16 y)
{
	if (detect) {
		data->tcount[id] = 0;
		data->distance[id] = 0;
	} else {
		data->distance[id] = diff_two_point(x, y,
		data->touchbx[id], data->touchby[id]);
	}

	if (data->tcount[id] > 20000)
		data->tcount[id] = 1;
	else	
		data->tcount[id]++;

	data->touchbx[id] = x;
	data->touchby[id] = y;

	if (id >= data->old_id)
		data->max_id = id;
	else
		data->max_id = data->old_id;

	data->old_id = id;
}
#endif	/* CHECK_ANTITOUCH */

static int mxt_check_instance(struct mxt_data *data, u8 type)
{
	int i;

	for (i = 0; i < data->info.object_num; i++) {
		if (data->objects[i].type == type)
			return(data->objects[i].instances - 1);
	}
	return 0;
}

static int mxt_init_write_config(struct mxt_data *data,
		u8 type, const u8 *cfg)
{
	struct mxt_object *object;
	u8 *temp;
	int ret;

	object = mxt_get_object(data, type);
	if (!object)
		return -EINVAL;

	if ((object->size == 0) || (object->start_address == 0)) {
		dev_err(&data->client->dev,	"%s error T%d\n",
			 __func__, type);
		return -ENODEV;
	}

	ret = mxt_write_mem(data, object->start_address,
			object->size, cfg);
	if (ret) {
		dev_err(&data->client->dev,	"%s write error T%d address[0x%x]\n",
			__func__, type, object->start_address);
		return ret;
	}

	if (mxt_check_instance(data, type)) {
		temp = kzalloc(object->size, GFP_KERNEL);

		if (temp == NULL)
			return -ENOMEM;

		ret |= mxt_write_mem(data, object->start_address + object->size,
			object->size, temp);
		kfree(temp);
	}

	return ret;
}

static int mxt_write_config_from_pdata(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	u8 **tsp_config = (u8 **)data->pdata->config;
	u8 i;
	int ret;

	if (!tsp_config) {
		dev_info(dev, "No cfg data in pdata\n");
		return 0;
	}

	for (i = 0; tsp_config[i][0] != MXT_RESERVED_T255; i++) {
		ret = mxt_init_write_config(data, tsp_config[i][0],
							tsp_config[i] + 1);
		if (ret)
			return ret;
	}
	return ret;
}



#if CHECK_ANTITOUCH_SERRANO  //0617
/* Defined of SERRANO's workaround status related with Golden reference.
 * 0 : intial status(there is no touch after IC reset).
 * 1 : waiting that there is calm on the screen to enter prime status of Golden reference.
 * 2 : Getting Golden reference. during this step if the calibration is occured, the step return to the 0.
 * 3 : Finished to get Golden reference.
 * 4 : It is final step and idle step after wake up....
 */
#define MXT_GR_GDC_STATUS_INIT		0
#define MXT_GR_GDC_STATUS_WAITTING	1
#define MXT_GR_GDC_STATUS_GETTING	2
#define MXT_GR_GDC_STATUS_AQUIRED	3
#define	MXT_GR_GDC_STATUS_FINISH		4

static const char * const GF_GDC_STATUS[] = {
	"Init",
	"Waiting",
	"Getting",
	"Aquired",
	"Finish",
};
static inline void mxt_set_gdc_status(struct mxt_data *data, u8 value){

	dev_err(&data->client->dev, "[GDC] changed gdc_value %s -> %s\n",
		GF_GDC_STATUS[data->GoodConditionStep], GF_GDC_STATUS[value]);

	data->GoodConditionStep = value;
}

static void mxt_set_golden_reference(struct mxt_data *data,
				 bool en)
{
	int error = 0;

	dev_info(&data->client->dev, "[GDC] GF is %s\n", en ? "Enabled" : "Disabled");

	if (en)
		error = mxt_write_object(data, MXT_SPT_GOLDENREFERENCES_T66, 0, (data->T66_CtrlVal | 0x3));
	else
		error = mxt_write_object(data, MXT_SPT_GOLDENREFERENCES_T66, 0, (data->T66_CtrlVal & 0xFE));

	if (error)
		dev_err(&data->client->dev, "%s : Error to write golden reference\n", __func__);
}


static void mxt_gdc_init_config(struct mxt_data *data)
{
    mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_INIT);
    data->check_after_wakeup = 1;
#if CHECK_PALM //0617
    data->PalmFlag = 0;
#endif
   data->TimerSet = 0;
    data->GoodStep1_AllReleased  = 0;
    data->Wakeup_Reset_Check_Press = 0;
    data->GoldenBadCheckCnt = 0;
    data->check_antitouch = 0;
    
     /* Disable Gloden reference. */
	mxt_set_golden_reference(data, false);
//0927	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 5);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 2, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 3, 1);

	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 6, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 7, 60);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 226);

	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 13, 1);
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 44, 70);
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 30, 20);
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 34, 0); 
	
	mxt_write_object(data, MXT_PROCI_EXTRATOUCHSCREENDATA_T57, 2, 0); //0924 New
	
	if(data->clear_cover_enable != 1){
		mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);//0923_2
	}
	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 1, 6);//0620
	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 10, 3);//0620
	mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 0xFF);
	
	mxt_t61_timer_set(data, 1,
			MXT_T61_TIMER_ONESHOT,
			MXT_T61_TIMER_CMD_STOP, 0);
			
	mxt_t61_timer_set(data, 2,
			MXT_T61_TIMER_ONESHOT,
			MXT_T61_TIMER_CMD_STOP, 0);
						
}

static void mxt_gdc_acquired_config(struct mxt_data *data)
{
	mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_AQUIRED);
	data->GoodStep1_AllReleased  = 0;
	data->Wakeup_Reset_Check_Press = 1;
	data->GoldenBadCheckCnt = 0;

	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 2, 5);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 3, 2);

	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 0);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 6, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 7, 60);
#if NO_GR_MODE
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 0);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 13, 1);
#else
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 226);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 13, 0);
#endif
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 44, 70);
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 30, 20);
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 34, 0);

	mxt_write_object(data, MXT_PROCI_EXTRATOUCHSCREENDATA_T57, 2, 0); //0924 New
    
	if(data->clear_cover_enable != 1){
		mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);//0923_2
		mxt_write_object(data, MXT_PROCI_STYLUS_T47, 7, 160); //0620
	}
	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 1, 1);
	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 10, 1);

	mxt_t61_timer_set(data, 1,
		MXT_T61_TIMER_ONESHOT,
		MXT_T61_TIMER_CMD_START, 400);
}

static void mxt_gdc_finish_config(struct mxt_data *data)
{
#if NO_GR_MODE
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 6, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 7, 60);	
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 226);
#else
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 6, 1);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 7, 60);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 0);
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 0);
#endif
	if(data->clear_cover_enable != 1)//0923_2
		mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);
}				

static void mxt_GR_Caputre_Prime_Process(struct mxt_data *data)
{		    
#if NO_GR_MODE
    mxt_gdc_acquired_config(data);
    mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 0xFD);  		    
#else					
    mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 8, 1);
    mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 0);

    if((data->T72_State== 0x02) ) {
       /* Time Out && All release Status */
	   mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 40, 8);//0620 Stable to Noisy Auto transaction disable
	   mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 60, 8);//0620 Noisy to Very Noisy Auto transaction disable	   
        data->GoodStep1_AllReleased = 0x02;
		mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 0xFD);  		    
		mxt_write_object(data, MXT_SPT_GOLDENREFERENCES_T66,
			0, MXT_FCALCMD(MXT_FCALCMD_PRIME) | data->T66_CtrlVal|0x83);//0614
    } else {
    	/* Go to waiting */
		dev_info(&data->client->dev, "But T72 status was not Stabel, Goto MXT_GR_GDC_STATUS_WAITTING \n");
		mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_WAITTING);
		data->GoodStep1_AllReleased = 0;
    }
#endif
}
#endif



#if DUAL_CFG
static int mxt_write_config(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_object *object;
	struct mxt_cfg_data *cfg_data;
	u8 i, val = 0;
	u16 reg, index;
	int ret;
	
	u32 cfg_length = data->cfg_len = fw_info->cfg_len / 2 ;

	if (!fw_info->ta_cfg_raw_data && !fw_info->batt_cfg_raw_data) {
		dev_info(dev, "No cfg data in file\n");
		ret = mxt_write_config_from_pdata(data);
		return ret;
	}

	/* Check Version information */
	if (fw_info->fw_ver != data->info.version) {
		dev_err(dev, "Warning: version mismatch! %s\n", __func__);
		return 0;
	}
	if (fw_info->build_ver != data->info.build) {
		dev_err(dev, "Warning: build num mismatch! %s\n", __func__);
		return 0;
	}

	dev_info(dev, "Writing Config:[CRC 0x%06X]\n", fw_info->cfg_crc);

	/* Get the address of configuration data */
	data->batt_cfg_raw_data = fw_info->batt_cfg_raw_data;
	data->ta_cfg_raw_data = fw_info->ta_cfg_raw_data =
		fw_info->batt_cfg_raw_data + cfg_length;

	/* Write config info */
	for (index = 0; index < cfg_length;) {
		if (index + sizeof(struct mxt_cfg_data) >= cfg_length) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d)!!\n",
				index + sizeof(struct mxt_cfg_data),
				cfg_length);
			return -EINVAL;
		}

		/* Get the info about each object */
		if (data->charging_mode)
			cfg_data = (struct mxt_cfg_data *)
					(&fw_info->ta_cfg_raw_data[index]);
		else
			cfg_data = (struct mxt_cfg_data *)
					(&fw_info->batt_cfg_raw_data[index]);

		index += sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > cfg_length) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d) in T%d object!!\n",
				index, cfg_length, cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			dev_err(dev, "T%d is Invalid object type\n",
				cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > object->size) {
			dev_err(dev, "T%d Object length exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= object->instances) {
			dev_err(dev, "T%d Object instances exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}

		dev_dbg(dev, "Writing config for obj %d len %d instance %d (%d/%d)\n",
			cfg_data->type, object->size,
			cfg_data->instance, index, cfg_length);

		reg = object->start_address + object->size * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size,
					 cfg_data->register_val);
		if (ret) {
			dev_err(dev, "Write T%d Object failed\n",
				object->type);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < object->size) {
			dev_err(dev, "Warning: zeroing %d byte(s) in T%d\n",
				 object->size - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < object->size; i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}
	dev_info(dev, "Updated configuration\n");
#if CHECK_ANTITOUCH_SERRANO
	ret = mxt_read_object(data, MXT_SPT_GOLDENREFERENCES_T66, 0, &val);
	if (!ret)
		data->T66_CtrlVal = val;
	dev_info(&data->client->dev, "data->T66_CtrlVal = %d\n",data->T66_CtrlVal);
#endif
	return ret;
}
#else
static int mxt_write_config(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_object *object;
	struct mxt_cfg_data *cfg_data;
	u32 current_crc;
	u8 i, val = 0;
	u16 reg, index;
	int ret;

	if (!fw_info->cfg_raw_data) {
		dev_info(dev, "No cfg data in file\n");
		ret = mxt_write_config_from_pdata(data);
		return ret;
	}

	/* Get config CRC from device */
	ret = mxt_read_config_crc(data, &current_crc);
	if (ret)
		return ret;

	/* Check Version information */
	if (fw_info->fw_ver != data->info.version) {
		dev_err(dev, "Warning: version mismatch! %s\n", __func__);
		return 0;
	}
	if (fw_info->build_ver != data->info.build) {
		dev_err(dev, "Warning: build num mismatch! %s\n", __func__);
		return 0;
	}

	/* Check config CRC */
	if (current_crc == fw_info->cfg_crc) {
		dev_info(dev, "Skip writing Config:[CRC 0x%06X]\n",
			current_crc);
		return 0;
	}

	dev_info(dev, "Writing Config:[CRC 0x%06X!=0x%06X]\n",
		current_crc, fw_info->cfg_crc);

	/* Write config info */
	for (index = 0; index < fw_info->cfg_len;) {

		if (index + sizeof(struct mxt_cfg_data) >= fw_info->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d)!!\n",
				index + sizeof(struct mxt_cfg_data),
				fw_info->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each object */
		cfg_data = (struct mxt_cfg_data *)
					(&fw_info->cfg_raw_data[index]);

		index += sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > fw_info->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d) in T%d object!!\n",
				index, fw_info->cfg_len, cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			dev_err(dev, "T%d is Invalid object type\n",
				cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > object->size) {
			dev_err(dev, "T%d Object length exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= object->instances) {
			dev_err(dev, "T%d Object instances exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}

		dev_dbg(dev, "Writing config for obj %d len %d instance %d (%d/%d)\n",
			cfg_data->type, object->size,
			cfg_data->instance, index, fw_info->cfg_len);

		reg = object->start_address + object->size * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size,
					 cfg_data->register_val);
		if (ret) {
			dev_err(dev, "Write T%d Object failed\n",
				object->type);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < object->size) {
			dev_err(dev, "Warning: zeroing %d byte(s) in T%d\n",
				 object->size - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < object->size; i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}
	dev_info(dev, "Updated configuration\n");

	return ret;
}
#endif

#if (CHECK_ANTITOUCH |CHECK_ANTITOUCH_SERRANO |CHECK_ANTITOUCH_GOLDEN)
static int mxt_command_calibration(struct mxt_data *data) ;
#endif

#if CLEAR_COVER
static int mxt_clear_cover_config_setting(struct mxt_data *data);
#endif
/* TODO TEMP_ADONIS: need to inspect below functions */

#if CHECK_PALM 
static void mxt_release_all_finger(struct mxt_data *data);
#endif

#if TSP_INFORM_CHARGER
static int set_charger_config(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	struct mxt_object *object;
	struct mxt_cfg_data *cfg_data;
	u8 i, val = 0;
	u16 reg, index;
	int ret;
	
	
	dev_info(&data->client->dev, "Current state is %s",
		data->charging_mode ? "Charging mode" : "Battery mode");

#if CHECK_PALM //0617
	if(data->PalmFlag == 1){
		data->PalmFlag = 0;
		mxt_release_all_finger(data);
	}
#endif

/* if you need to change configuration depend on chager detection,
 * please insert below line.
 */
	
	dev_dbg(dev, "set_charger_config data->cfg_len = %d\n", data->cfg_len);

	for (index = 0; index < data->cfg_len;) {
		if (index + sizeof(struct mxt_cfg_data) >= data->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d)!!\n",
				index + sizeof(struct mxt_cfg_data),
				data->cfg_len);
			return -EINVAL;
		}

		/* Get the info about each object */
		if (data->charging_mode)
			cfg_data = (struct mxt_cfg_data *)
					(&data->ta_cfg_raw_data[index]);
		else
			cfg_data = (struct mxt_cfg_data *)
					(&data->batt_cfg_raw_data[index]);

		index += sizeof(struct mxt_cfg_data) + cfg_data->size;
		if (index > data->cfg_len) {
			dev_err(dev, "index(%d) of cfg_data exceeded total size(%d) in T%d object!!\n",
				index, data->cfg_len, cfg_data->type);
			return -EINVAL;
		}

		object = mxt_get_object(data, cfg_data->type);
		if (!object) {
			dev_err(dev, "T%d is Invalid object type\n",
				cfg_data->type);
			return -EINVAL;
		}

		/* Check and compare the size, instance of each object */
		if (cfg_data->size > object->size) {
			dev_err(dev, "T%d Object length exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}
		if (cfg_data->instance >= object->instances) {
			dev_err(dev, "T%d Object instances exceeded!\n",
				cfg_data->type);
			return -EINVAL;
		}

		dev_dbg(dev, "Writing config for obj %d len %d instance %d (%d/%d)\n",
			cfg_data->type, object->size,
			cfg_data->instance, index, data->cfg_len);

		reg = object->start_address + object->size * cfg_data->instance;

		/* Write register values of each object */
		ret = mxt_write_mem(data, reg, cfg_data->size,
					 cfg_data->register_val);
		if (ret) {
			dev_err(dev, "Write T%d Object failed\n",
				object->type);
			return ret;
		}

		/*
		 * If firmware is upgraded, new bytes may be added to end of
		 * objects. It is generally forward compatible to zero these
		 * bytes - previous behaviour will be retained. However
		 * this does invalidate the CRC and will force a config
		 * download every time until the configuration is updated.
		 */
		if (cfg_data->size < object->size) {
			dev_err(dev, "Warning: zeroing %d byte(s) in T%d\n",
				 object->size - cfg_data->size, cfg_data->type);

			for (i = cfg_data->size + 1; i < object->size; i++) {
				ret = mxt_write_mem(data, reg + i, 1, &val);
				if (ret)
					return ret;
			}
		}
	}
#if CLEAR_COVER
	mxt_clear_cover_config_setting(data);
#endif

#if CHECK_ANTITOUCH_SERRANO //0617
    switch(data->GoodConditionStep) {
    case MXT_GR_GDC_STATUS_INIT:
    case MXT_GR_GDC_STATUS_WAITTING:
    case MXT_GR_GDC_STATUS_GETTING:
    		dev_info(&data->client->dev, "set_charger_ <<<MXT_GR_GDC_STATUS_AQUIRED(0x%04x)\n",data->FcalSeqdoneNum);//0913
    		mxt_gdc_init_config(data);//0615_
    		mxt_command_calibration(data);
            break;
    case MXT_GR_GDC_STATUS_AQUIRED:
    case MXT_GR_GDC_STATUS_FINISH:
    		dev_info(&data->client->dev, "set_charger_ >>>>MXT_GR_GDC_STATUS_AQUIRED(0x%04x)\n", data->FcalSeqdoneNum);//0913
    		if (((data->WakeupPowerOn == 1 )&& (data->Exist_Stylus != 0)&&(data->clear_cover_enable != 1))|| (data->Exist_Stylus ==100)) {//0913
    			dev_info(&data->client->dev, "set_charger_ Existed Stylus touch when phone had gone sleep\n");
#if 0 //0925_2
				if(data->Exist_Stylus ==100){
					mxt_write_object(data, MXT_PROCI_STYLUS_T47, 2, 145);
					mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 40);
				}
#endif
    			data->Exist_Stylus = 0;
    			mxt_gdc_init_config(data);//0615_
    			data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x100);//0913
    			mxt_command_calibration(data);
    		} else {
    			mxt_gdc_acquired_config(data);
    #if NO_GR_MODE
    			mxt_set_golden_reference(data, false);
    			mxt_command_calibration(data);
    #endif
		}   
        default :
            break;
    }
    data->WakeupPowerOn = 0; //0912
#endif
#if	CHECK_ANTITOUCH
	mxt_command_calibration(data);
#endif
#if	CHECK_ANTITOUCH_GOLDEN //0719
	mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 6, 4);//0725
	mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T62, 0, 125);

#if defined(CONFIG_MACH_GOLDEN_VZW) || defined(CONFIG_MACH_GOLDEN_ATT)
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 24, 30);//0910
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 25, 30);//0910
#endif
#endif
	return ret;
}

static void inform_charger(struct tsp_callbacks *cb,
	bool en)
{
	struct mxt_data *data = container_of(cb,
			struct mxt_data, callbacks);

	if (!data->mxt_enabled)
		return;

	cancel_delayed_work_sync(&data->noti_dwork);
	data->charging_mode = en;
	schedule_delayed_work(&data->noti_dwork, HZ / 5);
}

static void charger_noti_dwork(struct work_struct *work)
{
	struct mxt_data *data =
		container_of(work, struct mxt_data,
		noti_dwork.work);
#if CHECK_ANTITOUCH_GOLDEN//0723
	int i;//0723
#endif

	if (!data->mxt_enabled) {
		schedule_delayed_work(&data->noti_dwork, HZ / 5);
		return ;
	}

	dev_info(&data->client->dev, "%s mode\n",
		data->charging_mode ? "charging" : "battery");
	
#if	CHECK_ANTITOUCH
	data->Press_cnt = 0;
	data->Release_cnt = 0;
	data->Press_Release_check = 1;
#endif

#if CHECK_ANTITOUCH_GOLDEN//0723
	if(data->AfterProbe == 1){
		data->AfterProbe = 0;
		for (i = 0; i < MXT_MAX_FINGER; i++) {
			if ((data->fingers[i].state != \
				MXT_STATE_INACTIVE) &&
				(data->fingers[i].state != \
				MXT_STATE_RELEASE))
				data->Report_touch_number++;
		}
		if(data->Report_touch_number)
			mxt_command_calibration(data);
	}
#endif
	set_charger_config(data);
}

static void inform_charger_init(struct mxt_data *data)
{
	INIT_DELAYED_WORK(&data->noti_dwork, charger_noti_dwork);
}
#endif

#if CLEAR_COVER
static int mxt_clear_cover_config_setting(struct mxt_data *data)
{
	int ret = 0;
	struct mxt_object *object;
	u8 on_batt_t47[9]={0,}, on_ta_t47[9]={0,}, off_t47[9]={0,};
	u8 on_t80[4]={0,}, off_batt_t80[4]={0,}, off_ta_t80[4]={0,};
	u8 t72_conf[6]={0,};
	int error;
	
	/*to do setting config */
	if (!data->mxt_enabled)
		return -1;

	dev_err(&data->client->dev, "555clear_cover_enable = %d \n", data->clear_cover_enable);

	object = mxt_get_object(data, MXT_SPT_DYNAMICCONFIGURATIONCONTAINER_T71);
	error = mxt_read_mem(data, object->start_address + 50, 9, &on_batt_t47[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read on_batt_t47[%d]\n",
			*on_batt_t47);

	error = mxt_read_mem(data, object->start_address + 60, 9, &on_ta_t47[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read on_ta_t47[%d]\n",
			*on_ta_t47);


	error = mxt_read_mem(data, object->start_address + 70, 9, &off_t47[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read  off_t47[%d]\n",
			 *off_t47);

	error = mxt_read_mem(data, object->start_address + 80, 4, &on_t80[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read  on_t80[%d]\n",
			 *on_t80);

	error = mxt_read_mem(data, object->start_address + 85, 4, &off_batt_t80[0]);	
	if (error)
		dev_err(&data->client->dev, "Error to read  off_batt_t80[%d]\n",
			 *off_batt_t80);

	error = mxt_read_mem(data, object->start_address + 90, 4, &off_ta_t80[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read  off_ta_t80[%d]\n",
			 *off_ta_t80);

	error = mxt_read_mem(data, object->start_address + 95, 6, &t72_conf[0]);
	if (error)
		dev_err(&data->client->dev, "Error to read  off_ta_t80[%d]\n",
			 *t72_conf);

	if (data->clear_cover_enable == 1) {//clear coverd
		data->Exist_Stylus = 0;//0620
		mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 10, t72_conf[0]);
		mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 13, t72_conf[1]);
		mxt_init_write_config(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80, &on_t80[0]);
		object = mxt_get_object(data, MXT_PROCI_STYLUS_T47);
#if TSP_INFORM_CHARGER
		if (data->charging_mode == 1) {//ta mode
			mxt_write_mem(data, object->start_address+1,9, &on_ta_t47[0]);
		} else 
#endif
		{
			mxt_write_mem(data, object->start_address+1,9, &on_batt_t47[0]);
		}
	} else {
#if TSP_INFORM_CHARGER	
		if(data->charging_mode){
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 10, t72_conf[2]);
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 13, t72_conf[3]);
			mxt_init_write_config(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80, &off_ta_t80[0]);
		}else
#endif
		{
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 10, t72_conf[4]);
			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 13, t72_conf[5]);
			mxt_init_write_config(data, MXT_PROCI_RETRANSMISSIONCOMPENSATION_T80, &off_batt_t80[0]);
		}
		object = mxt_get_object(data, MXT_PROCI_STYLUS_T47);
		mxt_write_mem(data, object->start_address+1,9, &off_t47[0]);

    	mxt_write_object(data, MXT_PROCI_STYLUS_T47, 7, 160); //HOVERSUP
		switch(data->GoodConditionStep) {
			case MXT_GR_GDC_STATUS_INIT:
			case MXT_GR_GDC_STATUS_WAITTING:
			case MXT_GR_GDC_STATUS_GETTING:
    				mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);  //0923_2
			break;
			case(MXT_GR_GDC_STATUS_AQUIRED):
    				mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);  //0923_2
		       break;
			case(MXT_GR_GDC_STATUS_FINISH):
    				mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);  //0923_2
		       break;
			default:
			    break;
		}
		
	}
	return ret;
}
#endif

#if FLIP_COVER
static int mxt_flip_cover_config_setting(struct mxt_data *data)
{
	int ret = 0;

	/*to do setting config */
	if (!data->mxt_enabled)
		return -1;


	dev_err(&data->client->dev, "removed flip_cover_enable = %d \n", data->flip_cover_enable);

	if (data->flip_cover_enable == 1) {//flip coverd
		if (data->charging_mode == 1) { //ta mode
			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 55);
		}
		else { // battery mode
			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 55);
		}
	}
	else { // No covered
		if (data->charging_mode == 1) {
			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 55);
		}
		else {
			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 55);
		}
	}

	return ret;
}
#endif

#if CHECK_ANTITOUCH_GOLDEN//0616

static int tcount_finger[MXT_MAX_FINGER] = {0,0,0,0,0,0,0,0,0,0};
static int touchbx[MXT_MAX_FINGER] = {0,0,0,0,0,0,0,0,0,0};
static int touchby[MXT_MAX_FINGER] = {0,0,0,0,0,0,0,0,0,0};
static int touchbx_init[MXT_MAX_FINGER] = {0,0,0,0,0,0,0,0,0,0};
static int touchby_init[MXT_MAX_FINGER] = {0,0,0,0,0,0,0,0,0,0};
static int GhostId = 100;;
static int ResetInitCorCnt = 0;//0619

static void clear_tcount(struct mxt_data *data)
{
	int i;
	for(i=0;i<MXT_MAX_FINGER;i++){
		tcount_finger[i] = 0;
		touchbx[i] = 0;
		touchby[i] = 0;
		touchbx_init[i] = 0;
		touchby_init[i] = 0;
	}
	GhostId = 100;
	ResetInitCorCnt = 0;//0619
	mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 35, 0);//0708

}



static int diff_two_point(u16 x, u16 y, u16 oldx, u16 oldy, int id)
{
	s16 diffx,diffy;//0617
	u16 distance;
	
	diffx = x-oldx;
	diffy = y-oldy;
	distance = abs(diffx) + abs(diffy);

	if((touchbx_init[id] != 0)&&(touchby_init[id] != 0)){
		diffx = x-touchbx_init[id];
		diffy = y-touchby_init[id];
		printk(KERN_ERR"[TSP] adbx=%d,  adby=%d\n", (int)abs(diffx),(int)abs(diffy));//0619
	}
	if((touchbx_init[id] == 0)&&(touchby_init[id] == 0)){
		printk(KERN_ERR"[TSP] x=%d,  y=%d\n",x,y);
		touchbx_init[id] = x;
		touchby_init[id] = y;
		ResetInitCorCnt = 0;
	}else if((distance == 0)&&((abs(diffx) > 10) ||(abs(diffy) > 10))){//0619
		ResetInitCorCnt++;
	}
	if(ResetInitCorCnt > 5){//0619
		printk(KERN_ERR"[TSP] Reset Init Cor. x=%d,  y=%d\n",x,y);
		touchbx_init[id] = x;
		touchby_init[id] = y;
		ResetInitCorCnt = 0;
	}
	
		printk(KERN_ERR" [TSP] %d, %d, %d, %d, distance %d\n",(int)x,(int)y,(int)oldx,(int)oldy,(int)distance);//0617 
	if((distance < PATTERN_TRACKING_DISTANCE)&&(abs(diffx)<10)&&(abs(diffy)<10)) {
		return 1;
	}
	else return 0;
}
static void tsp_pattern_tracking(struct mxt_data *data, int fingerindex, u16 x, u16 y, int num)
{
	int i;

	for( i = 0; i< MXT_MAX_FINGER; i++)	{
		if( i == fingerindex){
			if(diff_two_point(x,y, touchbx[i], touchby[i], i)){
				tcount_finger[i] = tcount_finger[i]+1;
				dev_info(&data->client->dev, "[TSP]mov_ tcount_finger[%d]=%d\n",i,tcount_finger[i]);//0617 
			}else{
				tcount_finger[i] = 0;
				dev_info(&data->client->dev, "[TSP] rel_tcount_finger[%d]=%d\n",i,tcount_finger[i]);//0617 
			}

			touchbx[i] = x;
			touchby[i] = y;

			if(num == 1){
				if(tcount_finger[i]> MAX_GHOSTTOUCH_COUNT){
					dev_info(&data->client->dev, "[TSP] Occured Ghost Touch\n");
					clear_tcount(data);
					mxt_command_calibration(data);
				}
			}else if (num > 1){
				if((tcount_finger[i]> MAX_GHOSTTOUCH_COUNT) && GhostId != i && GhostId != 100){//0618
					dev_info(&data->client->dev, "[TSP] Occured 2/2 Ghost Touch  tcount_finger[%d] GhostId %d \n", i, GhostId);//0618
					clear_tcount(data);
					mxt_command_calibration(data);
				}
				if(tcount_finger[i]> MAX_GHOSTTOUCH_COUNT){
					dev_info(&data->client->dev, "[TSP] Occured Ghost 1/2Touch %d\n",GhostId);//0618
					GhostId = i; 
					mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
						  MXT_T61_TIMER_CMD_STOP, 1000);//0619
					if(tcount_finger[i] > 200) { //0719
						clear_tcount(data);
						mxt_command_calibration(data);
					}
				}
			}
		}
	}
}
#endif

static void mxt_report_input_data(struct mxt_data *data)
{
	int i;
	int count = 0;
	int report_count = 0;

	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;

		input_mt_slot(data->input_dev, i);
		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			input_mt_report_slot_state(data->input_dev,
					MT_TOOL_FINGER, false);
		} else {
			input_mt_report_slot_state(data->input_dev,
					MT_TOOL_FINGER, true);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					data->fingers[i].x);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					data->fingers[i].y);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					data->fingers[i].w);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 data->fingers[i].z);
#if TSP_USE_SHAPETOUCH
			/* Currently revision G firmware do not support it */
//			input_report_abs(data->input_dev,
//				ABS_MT_COMPONENT,
//				data->fingers[i].component);
			input_report_abs(data->input_dev,
				ABS_MT_SUMSIZE, data->sumsize);
#endif
		}
		report_count++;

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			dev_info(&data->client->dev, "[P][%d]: T[%d][%d] X[%d],Y[%d]",
				i, data->fingers[i].type,
				data->fingers[i].event,
				data->fingers[i].x, data->fingers[i].y);
#if TSP_USE_SHAPETOUCH
			pr_cont(",COMP[%d],SUM[%d],AREA[%d]\n",
				data->fingers[i].component, data->sumsize,data->fingers[i].stylus);//0605
#else
			pr_cont("\n");
#endif
		}
#else
		if (data->fingers[i].state == MXT_STATE_PRESS) {
			dev_info(&data->client->dev, "[P][%d]: T[%d][%d]",
				i, data->fingers[i].type,
				data->fingers[i].event);
#if TSP_USE_SHAPETOUCH
			pr_cont(",COMP[%d],SUM[%d],AREA[%d]\n",
				data->fingers[i].component, data->sumsize,data->fingers[i].stylus);
#else
			pr_cont("\n");
#endif
			
		}
#endif
		else if (data->fingers[i].state == MXT_STATE_RELEASE)
			dev_info(&data->client->dev, "[R][%d]: T[%d][%d] M[%d]\n",
				i, data->fingers[i].type,
				data->fingers[i].event,
				data->fingers[i].mcount);

		if (data->fingers[i].state == MXT_STATE_RELEASE) {
			data->fingers[i].state = MXT_STATE_INACTIVE;
			data->fingers[i].mcount = 0;
		} else {
			data->fingers[i].state = MXT_STATE_MOVE;
			count++;
		}
#if CHECK_ANTITOUCH_GOLDEN//0616
	if(data->AntiTouchGoTrackingNum == 1)
		tsp_pattern_tracking(data, i, data->fingers[i].x, data->fingers[i].y, 1);
	else if(data->AntiTouchGoTrackingNum == 2)
		tsp_pattern_tracking(data,i, data->fingers[i].x, data->fingers[i].y, 2);
#endif

	}

	if (report_count > 0) {
#if TSP_USE_ATMELDBG
		if (!data->atmeldbg.stop_sync)
#endif
			input_sync(data->input_dev);
	}


#if CHECK_ANTITOUCH_SERRANO
	if(data->mxt_enabled == 1){//0912
		data->Old_Report_touch_number =data->Report_touch_number;//0913_3
		data->Report_touch_number = 0;
		if((data->WakeupPowerOn == 0) && (data->Exist_Stylus != 100)&&(data->clear_cover_enable != 1)) { //0912
			data->Exist_Stylus = 0; 
		}
		for (i = 0; i < MXT_MAX_FINGER; i++) {
			if ((data->fingers[i].state != \
				MXT_STATE_INACTIVE) &&
				(data->fingers[i].state != \
				MXT_STATE_RELEASE)){
				data->Report_touch_number++;
				if(/*(data->fingers[i].x < 20) || (data->fingers[i].x > (539-20)) ||*/\
				(data->fingers[i].y < 20) || (data->fingers[i].y > (959-20))) {
					if(data->fingers[i].stylus > 15){
						data->Exist_EdgeTouch++;
						if(data->Exist_EdgeTouch > 4) {
							data->Exist_EdgeTouch = 0;
							if (data->GoodConditionStep < MXT_GR_GDC_STATUS_AQUIRED) {
								mxt_gdc_init_config(data);
								data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x200);//0913
								mxt_command_calibration(data);
							}
						}
					}
				}
				if((data->fingers[i].stylus == 0)&&(data->Exist_Stylus != 100)&&(data->clear_cover_enable != 1)){//0912
					data->Exist_Stylus++;
				}
			}
		}
		if((data->Exist_Stylus > 1)&&(data->Exist_Stylus != 100)&&(data->clear_cover_enable != 1)){//0912
			dev_info(&data->client->dev, "data->Exist_Stylus = 0 -->100\n");
			data->Exist_Stylus = 100; //for one time set T47, T9
//0925_2			mxt_write_object(data, MXT_PROCI_STYLUS_T47, 2, 1);
//0925_2			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 50);
		}

		if (data->Report_touch_number){
			data->GoodStep1_AllReleased  = 0;/*re-touched*/
			data->Exist_EdgeTouch = 0;//0615
		} else {
			if(data->Wakeup_Reset_Check_Press == 1){//0912
				dev_info(&data->client->dev, "[GDC] Sequence is finished by Released all finger\n");
				data->Wakeup_Reset_Check_Press = 2;
				data->GoldenBadCheckCnt = 0;
				mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_FINISH);
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_START, 2000);
			}
			if((data->Exist_Stylus == 100)&&(data->clear_cover_enable != 1)){//0912
				dev_info(&data->client->dev, "data->Exist_Stylus = 100 --> 0\n");
				data->Exist_Stylus = 0;
//0925_2				mxt_write_object(data, MXT_PROCI_STYLUS_T47, 2, 145);
//0925_2				mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 7, 40);
			}
		}
	}
#endif

#if CHECK_ANTITOUCH_GOLDEN//0616
	data->Report_touch_number = 0;
	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != \
			MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != \
			MXT_STATE_RELEASE))
			data->Report_touch_number++;
	}
	
	if(data->Report_touch_number==0){
		if(data->AntiTouchGoTrackingNum){
			data->AntiTouchGoTrackingNum =0;
			data->LongTouchFlag = 0; //1001
			clear_tcount(data);
		}
		pr_info("[TSP] report data->TimerSet = %d\n",data->TimerSet);
		if(data->TimerSet == 2){ //auto calibration on status  //0724
			mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 0);
			data->TimerSet = 3;
			mxt_t61_timer_set(data, 2, MXT_T61_TIMER_ONESHOT,
				  MXT_T61_TIMER_CMD_START, 4000);
		}
	}else{//0618
		if(GhostId != 100){
			if ((data->fingers[GhostId].state == \
				MXT_STATE_INACTIVE) ||
				(data->fingers[GhostId].state == \
				MXT_STATE_RELEASE)){
					dev_info(&data->client->dev, "[TSP] Released ID %d\n",GhostId);//0618
					tcount_finger[GhostId] = 0;
					GhostId = 100;
				}
		}
	}
#endif

#if TSP_USE_SHAPETOUCH
	/* all fingers are released */
	if (count == 0)
		data->sumsize = 0;
#endif
#ifdef TSP_BOOSTER
	set_dvfs_lock(data, count);
#endif
	data->finger_mask = 0;
}

static void mxt_release_all_finger(struct mxt_data *data)
{
	int i;
	int count = 0;
#if CHECK_ANTITOUCH_SERRANO
	if(data->Exist_Stylus != 100)//0912
		data->Exist_Stylus = 0;
#endif
	for (i = 0; i < MXT_MAX_FINGER; i++) {
#if CHECK_ANTITOUCH_SERRANO//0613
	if (data->mxt_enabled == false) {
		if ((data->fingers[i].state != \
			MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != \
			MXT_STATE_RELEASE)&&
			(data->fingers[i].stylus == 0)&&(data->Exist_Stylus != 100)){//0912
				data->Exist_Stylus ++;
			}
		}
#endif
		if (data->fingers[i].state == MXT_STATE_INACTIVE)
			continue;
		data->fingers[i].z = 0;
		data->fingers[i].state = MXT_STATE_RELEASE;
		count++;
	}
	if (count) {
		dev_err(&data->client->dev, "%s\n", __func__);
		mxt_report_input_data(data);
	}
}

#if TSP_HOVER_WORKAROUND
static void mxt_current_calibration(struct mxt_data *data)
{
	dev_info(&data->client->dev, "%s\n", __func__);

	mxt_write_object(data, MXT_SPT_SELFCAPHOVERCTECONFIG_T102, 1, 1);
}
#endif

#if	CHECK_ANTITOUCH
static int mxt_dist_check(struct mxt_data *data)
{
	int i;
	u16 dist_sum = 0;

	for (i = 0; i <= data->max_id; i++) {
		if (data->distance[i] < 3)
			dist_sum++;
		else
			dist_sum = 0;
	}

	for (i = data->max_id + 1; i < MAX_USING_FINGER_NUM; i++)
		data->distance[i] = 0;

	return dist_sum;
}

static void mxt_tch_atch_area_check(struct mxt_data *data,
		int tch_area, int atch_area, int touch_area)
{
	u16 dist_sum = 0;
	unsigned char touch_num;

	touch_num = data->Report_touch_number;
	if (tch_area) {
		/* First Touch After Calibration */
		if (data->pdata->check_timer == 0) {
			data->coin_check = 0;
			mxt_t61_timer_set(data, 1,
					MXT_T61_TIMER_ONESHOT,
					MXT_T61_TIMER_CMD_START,
					2000);
			data->pdata->check_timer = 1;
		}
	}

	if ((tch_area == 0) && (atch_area > 0)) {
		pr_info("[TSP] T57_Abnormal Status, tch=%d, atch=%d\n"
		, data->tch_value
		, data->atch_value);
		mxt_command_calibration(data);
		return;
	}
	dist_sum = mxt_dist_check(data);
	if (touch_num > 1 && tch_area <= 45) {
		if (touch_num == 2) {
			if (tch_area < atch_area-3) {
				pr_info("[TSP] T57_Two touch  Cal_Bad : tch area < atch_area-3 !!!\n");
				mxt_command_calibration(data);
			} 
		}
		else if (tch_area <= (touch_num * 4 + 2)) {
			
			if (!data->coin_check) {
				if (dist_sum == (data->max_id + 1)) {
					if (touch_area < T_AREA_LOW_MT) {
						if (data->t_area_l_cnt >= 7) {
								pr_info("[TSP] T57_Multi touch Cal maybe bad contion : Set autocal = 5\n");
							mxt_t8_cal_set(data, 5);
							data->coin_check = 1;
							data->t_area_l_cnt = 0;
						} else {
							data->t_area_l_cnt++;
						}

						data->t_area_cnt = 0;
					} else {
						data->t_area_cnt = 0;
						data->t_area_l_cnt = 0;
					}
				}
			}

		} else {
			if (tch_area < atch_area-8) {
				pr_info("[TSP] T57_Multi touch  Cal_Bad : tch area < atch_area-8 !!!\n");						
				mxt_command_calibration(data);
			}
		}
	} else if (touch_num  > 1 && tch_area > 48) {
		if (tch_area > atch_area) {
			pr_info("[TSP] T57_Multi touch  Cal_Bad : tch area > atch_area !!!\n");
			mxt_command_calibration(data);
		} else {
			pr_info("[TSP] T57_Multi touch Cal maybe good contion : tch area <= atch_area\n");
		}

	} else if (touch_num == 1) {
		/* single Touch */
		dist_sum = data->distance[0];
		if ((tch_area < 7) &&
			(atch_area <= 1)) {
			if (!data->coin_check) {
				if (data->distance[0] < 3) {
					if (touch_area < T_AREA_LOW_ST) {
						if (data->t_area_l_cnt >= 7) {
								pr_info("[TSP] T57_Single Floating metal Wakeup suspection :Set autocal = 5\n");
							mxt_t8_cal_set(data, 5);
							data->coin_check = 1;
							data->t_area_l_cnt = 0;
						} else {
							data->t_area_l_cnt++;
						}
						data->t_area_cnt = 0;

					} else if (touch_area < \
							T_AREA_HIGH_ST) {
						if (data->t_area_cnt >= 7) {
								pr_info("[TSP] T57_Single Floating metal Wakeup suspection :Set autocal = 5\n");
							mxt_t8_cal_set(data, 5);
							data->coin_check = 1;
							data->t_area_cnt = 0;
						} else {
							data->t_area_cnt++;
						}
						data->t_area_l_cnt = 0;
					} else {
						data->t_area_cnt = 0;
						data->t_area_l_cnt = 0;
					}
				}
			}
		} else if (tch_area > 25) {
			pr_info("[TSP] tch_area > 25\n");
			mxt_command_calibration(data);
		}
	}
}
#endif

static void mxt_treat_T6_object(struct mxt_data *data,
		struct mxt_message *message)
{
	/* Normal mode */
	if (message->message[0] == 0x00) {
		dev_info(&data->client->dev, "T6 All Cleared\n");

#if	CHECK_ANTITOUCH
        if (data->cal_busy)
		data->cal_busy = 0;
#endif
#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed.
*/
		if (data->pdata->revision == MXT_REVISION_I
			&& data->cur_cal_status) {
			mxt_current_calibration(data);
			data->cur_cal_status = false;
		}
#endif
	}
	/* I2C checksum error */
	if (message->message[0] & 0x04)
		dev_err(&data->client->dev, "I2C checksum error\n");
	/* Config error */
	if (message->message[0] & 0x08)
		dev_err(&data->client->dev, "Config error\n");
	/* Calibration */
	if (message->message[0] & 0x10){
#if CHECK_ANTITOUCH_SERRANO		
		dev_info(&data->client->dev, "Calibration is on going  %d!!\n",data->GoodConditionStep);//0609
#else
		dev_info(&data->client->dev, "Calibration is on going !!\n");
#endif
#if CHECK_ANTITOUCH
		/* After Calibration */
		data->coin_check = 0;
		mxt_t8_cal_set(data, 0);
		data->pdata->check_antitouch = 1;
		mxt_t61_timer_set(data, 1,
				MXT_T61_TIMER_ONESHOT,
				MXT_T61_TIMER_CMD_STOP, 0);
		data->pdata->check_timer = 0;
		data->pdata->check_calgood = 0;
		data->finger_area = 0;

		if (!data->Press_Release_check) {
			pr_info("[TSP] Second Cal check\n");
			data->Press_Release_check = 1;
			data->Press_cnt = 0;
			data->Release_cnt = 0;
		}
		data->cal_busy = 1;
#elif CHECK_ANTITOUCH_SERRANO
		data->T9_msg_cnt = 0; //0924 New

       switch(data->GoodConditionStep) {
            case(MXT_GR_GDC_STATUS_INIT) :
            case(MXT_GR_GDC_STATUS_WAITTING) :
            case(MXT_GR_GDC_STATUS_GETTING) :
				data->check_antitouch = 0;
				data->TimerSet = 0;
				data->check_after_wakeup = 1;
				data->T72_State = 0;
				mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_INIT);
				mxt_set_golden_reference(data, false); //1008                // Disable golden referece
				mxt_t61_timer_set(data, 1,
					MXT_T61_TIMER_ONESHOT,
					MXT_T61_TIMER_CMD_STOP, 0);
                break;
            case (MXT_GR_GDC_STATUS_AQUIRED):
            case (MXT_GR_GDC_STATUS_FINISH):
    			if (data->GoldenBadCheckCnt == 0) {
    				mxt_t61_timer_set(data, 2,
    						MXT_T61_TIMER_ONESHOT,
    						MXT_T61_TIMER_CMD_START, 1200);
				if(data->clear_cover_enable != 1){//0725
					mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);//0923_3
				}    			
			}
    			data->GoldenBadCheckCnt ++;
    			if (data->GoldenBadCheckCnt >= 6) {//0912
    				dev_info(&data->client->dev, "T6 Disable GR Because Cal 6 times \n");//0912
    				mxt_gdc_init_config(data);
				data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x400);//0913
				mxt_command_calibration(data);//0617
    			}
                break;
            default:
                break;
		}
		
#elif CHECK_ANTITOUCH_GOLDEN //0619
		if(data->AntiTouchGoTrackingNum != 0){
			clear_tcount(data);
			data->AntiTouchGoTrackingNum = 0;
			data->LongTouchFlag = 0; //1001
		}
		data->TimerSet = 1;//0724
#endif
	}
	/* Signal error */
	if (message->message[0] & 0x20)
		dev_err(&data->client->dev, "Signal error\n");
	/* Overflow */
	if (message->message[0] & 0x40)
		dev_err(&data->client->dev, "Overflow detected\n");
	/* Reset */
	if (message->message[0] & 0x80) {
		dev_info(&data->client->dev, "Reset is ongoing\n");
#if TSP_INFORM_CHARGER
		set_charger_config(data);
#endif

#if	CHECK_ANTITOUCH
		data->Press_Release_check = 1;
		data->Press_cnt = 0;
		data->Release_cnt = 0;
#endif

#if TSP_HOVER_WORKAROUND
/* TODO HOVER : Below commands should be removed.
 * it added just for hover. Current firmware shoud set the acqusition mode
 * with free-run and run current calibration after receive reset command
 * to support hover functionality.
 * it is bug of firmware. and it will be fixed in firmware level.
 */
		if (data->pdata->revision == MXT_REVISION_I) {
			int error = 0;
			u8 value = 0;

			error = mxt_read_object(data,
				MXT_SPT_TOUCHSCREENHOVER_T101, 0, &value);

			if (error) {
				dev_err(&data->client->dev, "Error read hover enable status[%d]\n"
					, error);
			} else {
				if (value)
					data->cur_cal_status = true;
			}
		}
#endif
	}
#if CHECK_ANTITOUCH_GOLDEN
	/* all finished */
	if (message->message[0] == 0x00) {
//0724		data->TimerSet = 0;//golden
		data->check_antitouch = 0;
	//               data->check_after_wakeup = 1;
	}
#endif
}

#if ENABLE_TOUCH_KEY
static void mxt_release_all_keys(struct mxt_data *data)
{
	if (tsp_keystatus != TOUCH_KEY_NULL) {
		switch (tsp_keystatus) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU,
								KEY_RELEASE);
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK,
								KEY_RELEASE);
			break;
		default:
			break;
		}
		dev_info(&data->client->dev, "[TSP_KEY] r %s\n",
						tsp_keyname[tsp_keystatus - 1]);
		tsp_keystatus = TOUCH_KEY_NULL;
	}
}


static void mxt_treat_T15_object(struct mxt_data *data,
						struct mxt_message *message)
{
	struct	input_dev *input;
	input = data->input_dev;

	/* single key configuration*/
	if (message->message[MXT_MSG_T15_STATUS] & MXT_MSGB_T15_DETECT) {

		/* defence code, if there is any Pressed key, force release!! */
		if (tsp_keystatus != TOUCH_KEY_NULL)
			mxt_release_all_keys(data);

		switch (message->message[MXT_MSG_T15_KEYSTATE]) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU, KEY_PRESS);
			tsp_keystatus = TOUCH_KEY_MENU;
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK, KEY_PRESS);
			tsp_keystatus = TOUCH_KEY_BACK;
			break;
		default:
			dev_err(&data->client->dev, "[TSP_KEY] abnormal P [%d %d]\n",
				message->message[0], message->message[1]);
			return;
		}

		dev_info(&data->client->dev, "[TSP_KEY] P %s\n",
						tsp_keyname[tsp_keystatus - 1]);
	} else {
		switch (tsp_keystatus) {
		case TOUCH_KEY_MENU:
			input_report_key(data->input_dev, KEY_MENU,
								KEY_RELEASE);
			break;
		case TOUCH_KEY_BACK:
			input_report_key(data->input_dev, KEY_BACK,
								KEY_RELEASE);
			break;
		default:
			dev_err(&data->client->dev, "[TSP_KEY] abnormal R [%d %d]\n",
				message->message[0], message->message[1]);
			return;
		}
		dev_info(&data->client->dev, "[TSP_KEY] R %s\n",
						tsp_keyname[tsp_keystatus - 1]);
		tsp_keystatus = TOUCH_KEY_NULL;
	}
	input_sync(data->input_dev);
	return;
}
#endif

static void mxt_treat_T9_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int id;
	u8 *msg = message->message;
#if CHECK_ANTITOUCH_GOLDEN	
	int i;
#endif
	id = data->reportids[message->reportid].index;
#if CHECK_ANTITOUCH_SERRANO
	data->T9_msg_cnt++;	//0924 New
#endif
	/* If not a touch event, return */
	if (id >= MXT_MAX_FINGER) {
		dev_err(&data->client->dev, "MAX_FINGER exceeded!\n");
		return;
	}
	if (msg[0] & MXT_RELEASE_MSG_MASK) {
		data->fingers[id].z = 0;
		data->fingers[id].w = msg[4];
		data->fingers[id].stylus = msg[4];
		data->fingers[id].state = MXT_STATE_RELEASE;
#if 	CHECK_ANTITOUCH
		if (data->Press_Release_check)
			data->Release_cnt++;
		else
			data->Release_cnt = 0;
		pr_info("[TSP] Release: id[%d],mc=%d\n", \
				id, data->fingers[id].mcount);
		pr_info("[TSP] Release_cnt =%d\n", data->Release_cnt);


		data->tcount[id] = 0;
		data->distance[id] = 0;
#endif
	
#if CHECK_PALM //0617
    		data->PressEventCheck = 0;//0923_3
		if(data->PalmFlag == 0)
#endif
			mxt_report_input_data(data);
	} else if ((msg[0] & MXT_DETECT_MSG_MASK)
		&& (msg[0] & (MXT_PRESS_MSG_MASK | MXT_MOVE_MSG_MASK| MXT_VECTOR_MSG_MASK))) {
		data->fingers[id].x = (msg[1] << 4) | (msg[3] >> 4);
		data->fingers[id].y = (msg[2] << 4) | (msg[3] & 0xF);
		data->fingers[id].w = msg[4];
		data->fingers[id].stylus = msg[4];
		data->fingers[id].z = msg[5];
#if TSP_USE_SHAPETOUCH
		data->fingers[id].component = msg[6];
#endif
		if (data->pdata->max_x < 1024)
			data->fingers[id].x = data->fingers[id].x >> 2;
		if (data->pdata->max_y < 1024)
			data->fingers[id].y = data->fingers[id].y >> 2;

		data->finger_mask |= 1U << id;
#if CHECK_PALM
		data->PressEventCheck = 0;//0923_2
#endif
		if (msg[0] & MXT_PRESS_MSG_MASK) {
			data->fingers[id].state = MXT_STATE_PRESS;
			data->fingers[id].mcount = 0;
			
#if 	CHECK_ANTITOUCH
			if (data->Press_Release_check) {
				data->Press_cnt++;
				data->finger_area = data->fingers[id].z;
			} else
				data->Press_cnt = 0;
			pr_info("[TSP] Press: id[%d],w=%d\n", \
				id, data->fingers[id].w);
			pr_info("[TSP] Press cnt =%d\n", data->Press_cnt);

			mxt_check_coordinate(data, 1, id,
				data->fingers[id].x,
				data->fingers[id].y);		
#elif CHECK_ANTITOUCH_SERRANO
            switch(data->GoodConditionStep) {
            case MXT_GR_GDC_STATUS_INIT:
			if (data->check_after_wakeup) {
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_START, 900);
				 data->check_after_wakeup = 0;
			}
			break;
            case MXT_GR_GDC_STATUS_WAITTING:
#if 0 //0927
			if (data->TimerSet == 0) {
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_START, 600);
				data->TimerSet = 1;
			}
#endif
			break;
           case MXT_GR_GDC_STATUS_GETTING:
			break;
            case MXT_GR_GDC_STATUS_AQUIRED:
            case MXT_GR_GDC_STATUS_FINISH:
			if (data->Wakeup_Reset_Check_Press == 1) {
				dev_info(&data->client->dev, "T9 Press Press \n");
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_STOP, 0);
			}
			break;
            default:
			break;
            }
  	    data->T9_area = data->fingers[id].stylus;//0923_4 Modify
  	    data->T9_amp = data->fingers[id].z; //0924 New
#if CHECK_PALM //0617
		data->PressEventCheck = 1;
#endif
#elif CHECK_ANTITOUCH_GOLDEN

			data->Report_touch_number = 0;	
			
			for (i = 0; i < MXT_MAX_FINGER; i++) {
				if ((data->fingers[i].state != \
					MXT_STATE_INACTIVE) &&
					(data->fingers[i].state != \
					MXT_STATE_RELEASE))
					data->Report_touch_number++;
			}
			pr_info("[TSP] data->Report_touch_number = %d\n",data->Report_touch_number);
			if(data->check_after_wakeup==1)
                        {
								if(data->Report_touch_number > 1)
                                {
                                        pr_info("[TSP]data->check_after_wakeup!!!!!!!!!!!!!!\r\n");
                                        mxt_command_calibration(data);//0619 0724
                                        mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
                                                          MXT_T61_TIMER_CMD_START, 1000);

//0619                                        data->check_after_wakeup = 0;
                                }
                                else
                                {
                                        mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
                                                          MXT_T61_TIMER_CMD_START,500);
//0619                                        data->check_after_wakeup = 1;
                                }
                        }
			pr_info("[TSP] data->TimerSet = %d\n",data->TimerSet);
			if(data->TimerSet == 1){//0724
				mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 5);
				data->TimerSet = 2;
			}
#endif

		} else if (msg[0] & MXT_MOVE_MSG_MASK) {
			data->fingers[id].mcount += 1;

#if	CHECK_ANTITOUCH
			mxt_check_coordinate(data, 0, id,
				data->fingers[id].x,
				data->fingers[id].y);
#endif
		}
	} else if ((msg[0] & MXT_SUPPRESS_MSG_MASK)
		&& (data->fingers[id].state != MXT_STATE_INACTIVE)) {
		data->fingers[id].z = 0;
		data->fingers[id].w = msg[4];
		data->fingers[id].stylus= msg[4];
		data->fingers[id].state = MXT_STATE_RELEASE;
		data->finger_mask |= 1U << id;
#if CHECK_PALM//0923_3
		data->PressEventCheck = 0;//0923_2
#endif
	} else {
		/* ignore changed amplitude and vector messsage */
		if (!((msg[0] & MXT_DETECT_MSG_MASK)
				&& (msg[0] & MXT_AMPLITUDE_MSG_MASK)))
			dev_err(&data->client->dev, "Unknown state %#02x %#02x\n",
				msg[0], msg[1]);
#if CHECK_PALM//0923_3
    		data->PressEventCheck = 0;//0923_2
#endif
	}
}

static void mxt_treat_T42_object(struct mxt_data *data,
		struct mxt_message *message)
{
	if (message->message[0] & 0x01) {
		/* Palm Press */
		dev_info(&data->client->dev, "palm touch detected\n");
	} else {
		/* Palm release */
		dev_info(&data->client->dev, "palm touch released\n");
	}
}

static void mxt_treat_T57_object(struct mxt_data *data,
		struct mxt_message *message)
{

#if CHECK_ANTITOUCH
	u16 tch_area = 0; u16 atch_area = 0; u16 touch_area_T57 = 0;  u8 i;

	touch_area_T57 = message->message[0] | (message->message[1] << 8);
	tch_area = message->message[2] | (message->message[3] << 8);
	atch_area = message->message[4] | (message->message[5] << 8);	

	data->tch_value  = tch_area;
	data->atch_value = atch_area;
	data->T57_touch = touch_area_T57;
	data->Report_touch_number = 0;	
	
	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != \
			MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != \
			MXT_STATE_RELEASE))
			data->Report_touch_number++;
	}

	if (data->pdata->check_antitouch) {
		mxt_tch_atch_area_check(data,
		tch_area, atch_area, touch_area_T57);
	}

	if (data->pdata->check_calgood == 1) {
		if ((atch_area - tch_area) > 15) {
			if (tch_area < 25) {
				dev_info(&data->client->dev, "Cal Not Good1 ,tch:%d, atch:%d, t57tch:%d\n"
				, tch_area, atch_area
				, touch_area_T57);
				mxt_command_calibration(data);
			}
		}
		if ((tch_area - atch_area) > 48) {
			dev_info(&data->client->dev, "Cal Not Good 2 ,tch:%d, atch:%d, t57tch:%d\n"
			, tch_area, atch_area
			, touch_area_T57);
			mxt_command_calibration(data);
		}
	}
#elif CHECK_ANTITOUCH_SERRANO
	u16 total_area=0; u16 tch_area = 0; u16 atch_area = 0;
	int id; u8 i;
	
	id = data->reportids[message->reportid].index;
	total_area =  message->message[0] | (message->message[1] << 8);
	tch_area = message->message[2] | (message->message[3] << 8);
	atch_area = message->message[4] | (message->message[5] << 8);

	data->T57_touch = total_area;
	data->tch_value  = tch_area;
	data->atch_value = atch_area;

	data->Report_touch_number = 0;
	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != \
			MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != \
			MXT_STATE_RELEASE))
			data->Report_touch_number++;
	}

	if(data->Report_touch_number  > 1)
	{
	    /* change lens bending config for multi touch ( more than two touch) */
	    if(data->TwoTouchLensBending == 1) {
	        dev_info(&data->client->dev,  "[TSP] T65: GRADTHR = 6, LPFILTCOEF = 3 for TouchNum = %d \n", data->Report_touch_number);
	    	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 1, 6);//0619
	    	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 10, 3);//0619 
	    	data->TwoTouchLensBending = 0;
	    }
	} else {
	    if(data->TwoTouchLensBending == 0) {
	        dev_info(&data->client->dev,  "[TSP] T65: GRADTHR = 1, LPFILTCOEF = 1 for TouchNum = %d \n", data->Report_touch_number);//0913
	    	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 1, 1);//0619
	    	mxt_write_object(data, MXT_PROCI_LENSBENDING_T65, 10, 1);//0619 
	    	data->TwoTouchLensBending = 1;
	    }
	}

    switch(data->GoodConditionStep) {
    case MXT_GR_GDC_STATUS_INIT:
			if (data->Report_touch_number > 1) {  //0927  Auto calibration Enable with multi finger touch
				mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 25);
			}
						
    	    /* 0923 avoid GR Capture process through abnormal touch conditions */
		if (data->Report_touch_number > 0) {
			data->init_tchnum = data->Report_touch_number;	//0925_1
			data->init_t57sum = data->T57_touch; 		//0925_1	
			data->init_t57tch = data->tch_value;		//0925_1	
			data->init_t57atch = data->atch_value;		//0925_1	
			data->init_t9area = data->T9_area;			//0925_1	

			if ((atch_area > 0) && (data->check_antitouch == 0)) {
				data->check_antitouch = 1;
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
				  MXT_T61_TIMER_CMD_STOP, 0);
			}
				
	    		if ((data->Report_touch_number==1)&&(data->Wakeup_Reset_Check_Press < 3)) {//0923_1
				if ((total_area > 40 && data->T9_area < 3)\
					||(tch_area > 40 && atch_area < 3) \
					|| (tch_area < 3 && atch_area > 40)) {
	    				dev_info(&data->client->dev, "T57 Stay at init mode Because size information \n");
	     				dev_info(&data->client->dev,  "[TSP] T57: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
							data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924 Modify
					data->check_antitouch = 1;
					mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
							  MXT_T61_TIMER_CMD_STOP, 0);
					mxt_command_calibration(data);
					mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
		    		}
	    		}
				
			if((data->Old_Report_touch_number <= data->Report_touch_number)&&(data->PressEventCheck == 1)) {//0923_1
				if ((data->Report_touch_number > 0) \
					 && (data->Report_touch_number <=2) \
					 &&(data->T9_area > 0) \
					 && (data->T9_area < 4) \
					 && (total_area > (data->T9_area*8) )){//0923_1 
					 	
					dev_info(&data->client->dev, "T57 Disable GR Because large sum size with single or two touch  \n"); //0924 Modify
			    	dev_info(&data->client->dev,  "[TSP] T57: total touch count = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
						data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924
					data->check_antitouch = 1;
					mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
						  MXT_T61_TIMER_CMD_STOP, 0);
					mxt_command_calibration(data);				  
					mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch			  
				}
			}
				
			if((data->Report_touch_number>1) \
				&& (atch_area < 3) \
				&& (data->T9_area == 0 ) \
				&& (total_area > 11)\
				&& (data->Exist_Stylus == 100)){//0925_2
					
				dev_info(&data->client->dev, "T57 Stay at init mode because multi stylus type ghost touch \n");
				dev_info(&data->client->dev,  "[TSP] T57: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
							data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924 Modify
				data->check_antitouch = 1;
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_STOP, 0);
				mxt_command_calibration(data);
			    mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
			} 
    	    }
    	     
		if(atch_area == 0 && data->check_antitouch == 1){
			dev_info(&data->client->dev, "T57 No anti touch  \n");
			data->check_antitouch = 0;
			mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_START, 1300);
    		}
			break;
    case MXT_GR_GDC_STATUS_WAITTING: //0927
		 
		if ((tch_area > 40 && atch_area < 5) || (tch_area < 10 && atch_area > 40)\
			|| (tch_area > 20 && atch_area > 20)) {
			data->TimerSet = 0;
			mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
				 MXT_T61_TIMER_CMD_STOP, 0);
		}
		
		if (data->Report_touch_number > 1) {	//0927  Auto calibration Enable with multi finger touch
			mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 25);
		}
			
		 if(data->Report_touch_number > 0) {
				data->wait_tchnum = data->Report_touch_number;
				data->wait_t57sum = data->T57_touch; 		
				data->wait_t57tch = data->tch_value;			
				data->wait_t57atch = data->atch_value;		
				data->wait_t9area = data->T9_area;				
				if (data->TimerSet == 1) {
    			data->TimerSet = 0;
    			mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
    				 MXT_T61_TIMER_CMD_STOP, 0);
    		}
				
				data->GoodStep1_AllReleased  = 0;/*re-touched*/
		 } else {
 #if  MaxStartup_Set    		
        		if ((tch_area == 0) && (atch_area == 0)&&(data->Exist_Stylus != 100) ) {//0912
        			data->GoodStep1_AllReleased  = 1;/* released status and not yet timer out */
        			dev_info(&data->client->dev, "T57: GoodStep1_AllReleased =%d\n", data->GoodStep1_AllReleased);
        		} else {
        			data->GoodStep1_AllReleased  = 0;/* released status and not yet timer out */
        			dev_info(&data->client->dev, "T57: Unstable GoodStep1_AllReleased =%d\n", data->GoodStep1_AllReleased);
        		}
#endif
			if (data->TimerSet == 0) {
				mxt_t61_timer_set(data, 1, MXT_T61_TIMER_ONESHOT,
					  MXT_T61_TIMER_CMD_START, 600);
				data->TimerSet = 1;
			}
    	    }
		  

			break;
    case MXT_GR_GDC_STATUS_GETTING:
#if  MaxStartup_Set /* 20130403 */
    	    if (data->Report_touch_number == 0) {
        		if ((tch_area == 0) && (atch_area == 0)&&(data->clear_cover_enable == 0)&&(data->Exist_Stylus != 100)) {//0912
				dev_info(&data->client->dev, "T57 : All Touch Released at MXT_GR_GDC_STATUS_GETTING \n");
				dev_info(&data->client->dev, "T66 Start Golden References in T57 0x%x \n", data->T66_CtrlVal);
				mxt_GR_Caputre_Prime_Process(data);                                     
			} else if (data->GoodStep1_AllReleased  < 2) { /* Befor PRIME Command */
        			dev_info(&data->client->dev, "Not Good Status Retry!! \n");
        			mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_WAITTING);
        			data->GoodStep1_AllReleased = 0;
        			mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 0, 0xFF);
        			mxt_command_calibration(data);
					mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
        		}
    	    }
#endif
            break;
    case MXT_GR_GDC_STATUS_AQUIRED:
		if( data->Old_Report_touch_number <= data->Report_touch_number){
			if((data->Report_touch_number < 3) \
				 && (data->T9_area < 4) \
				 && (total_area > 50 )){//0923_1
				dev_info(&data->client->dev, "T57 Disable GR Because large sum size with multi stylus  \n");
		    		dev_info(&data->client->dev,  "[TSP] T57: total touch count = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
					data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924
				mxt_gdc_init_config(data);
				data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x4000);
				mxt_command_calibration(data);
				mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
			}
		}
    case MXT_GR_GDC_STATUS_FINISH:
#if CHECK_PALM
    		if (data->PalmFlag == 0) {
    			if ((total_area > 110) && (data->Report_touch_number >= 3)) {
    				dev_info(&data->client->dev, "T57: Palm Reported\n");
    				data->PalmFlag = 1;
    			}
    		} else { /*data->PalmFlag == 1*/
    			if (data->Report_touch_number  == 0) {
    				dev_info(&data->client->dev, "T57: Palm Cleared. Disable GR\n");
    				data->PalmFlag = 0;
    				mxt_release_all_finger(data);
    			}
    		}
#endif
    		if ((data->Report_touch_number==1)&&(data->Wakeup_Reset_Check_Press < 3)){//0923_1
				if ((total_area > 40 && data->T9_area < 3) \
					|| (tch_area > 40 && atch_area < 3) \
				|| (tch_area < 3 && atch_area > 40)) {
	    				dev_info(&data->client->dev, "T57 Disable GR Because size information \n");
	    				dev_info(&data->client->dev,  "[TSP] T57: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
							data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924 Modify
	    				mxt_gdc_init_config(data);
					data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x800);//0913
	    				mxt_command_calibration(data);
						mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
	    		 }
    		 }
		if((data->Old_Report_touch_number <= data->Report_touch_number)&&(data->PressEventCheck == 1)){//0923_1
			if((data->Report_touch_number > 0)\
				 && (data->Report_touch_number <=2) \
				 &&(data->T9_area > 0) \
				 && (data->T9_area < 4) \
				 && (total_area > (data->T9_area*8) )){//0923_1
				 	
				dev_info(&data->client->dev, "T57 Disable GR Because large sum size with single or two touch  \n"); //0924 Modify
				dev_info(&data->client->dev,  "[TSP] T57: total touch count = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
					data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924
				mxt_gdc_init_config(data);
				data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x1000);
				mxt_command_calibration(data);
				mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
			}

			if((data->Report_touch_number>1) \
				&& (atch_area < 3) \
				&& (data->T9_area == 0 ) \
				&& (total_area > 11)\
				&& (data->Exist_Stylus == 100)){//0925_2		
				dev_info(&data->client->dev, "T57 Disable GR Because multi stylus type ghost touch \n");
				dev_info(&data->client->dev,  "[TSP] T57: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
					data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924 Modify
				mxt_gdc_init_config(data);
				data->FcalSeqdoneNum = (data->FcalSeqdoneNum | 0x2000);
				mxt_command_calibration(data);
				mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 4, 50); //0927  Auto calibration Enable cause cmd cal during touch
			}    		
		}
#if 0//NO_GR_MODE  //0923_2
    		if (((tch_area < 2) &&(total_area > 20)) ||(((tch_area+atch_area) <10) &&(total_area > 50))) {
    			dev_info(&data->client->dev, "T57: Calibration because of area size 1\n");
    				mxt_command_calibration(data);
    		}
    
    		if ((total_area  > tch_area*2) && (atch_area > 0)) {
    			dev_info(&data->client->dev, "T57: Calibration because of area size 3\n");
    				mxt_command_calibration(data);
    		}
    
    		if ((data->Report_touch_number > 1)&&(total_area < 6)) {
			dev_info(&data->client->dev, "T57: Calibration because of area size 2\n");
				mxt_command_calibration(data);
    		}
#endif
            break;
        default:
            break;
    }
	
#if (defined(CONFIG_MACH_SERRANO_SPR) || defined(CONFIG_MACH_SERRANO_USC))//0925_1
	if((data->PressEventCheck == 1) || (data->T9_msg_cnt > 100)) {//0923_4
    		dev_info(&data->client->dev,  "[TSP] T57: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d, t9_amp = %d \n", \
			data->Report_touch_number,  total_area, tch_area,  atch_area,  data->T9_area, data->T9_amp);  //0924 Modify

		if(data->T9_msg_cnt > 100) {
			dev_info(&data->client->dev,  "[TSP] GR INIT: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d\n", \
				data->init_tchnum,  data->init_t57sum, data->init_t57tch, data->init_t57atch,  data->init_t9area);  //0925 ! New
    
			dev_info(&data->client->dev,  "[TSP] GR WAIT: tchnum = %d,t57_sumsize= %d, t57_tch =%d, t57 atch=%d, t9_area=%d\n", \
				data->wait_tchnum,  data->wait_t57sum, data->wait_t57tch, data->wait_t57atch,  data->wait_t9area);  //0925 ! New
    
			data->T9_msg_cnt = 0;
		}
	}
#else
	data->T9_msg_cnt = 0;
#endif

#if CHECK_PALM//0923_3
    data->PressEventCheck = 0;//0923_1
#endif
#elif CHECK_ANTITOUCH_GOLDEN
	int id; u16 tch_area = 0; u16 atch_area = 0; 
	int i = 0;

	id = data->reportids[message->reportid].index;
	tch_area = message->message[2] | (message->message[3] << 8);
	atch_area = message->message[4] | (message->message[5] << 8);

	data->Report_touch_number = 0;	
	
	for (i = 0; i < MXT_MAX_FINGER; i++) {
		if ((data->fingers[i].state != \
			MXT_STATE_INACTIVE) &&
			(data->fingers[i].state != \
			MXT_STATE_RELEASE))
			data->Report_touch_number++;
		if((data->TimerSet >=4)&&(data->fingers[i].mcount > 120)){//0820
			data->TimerSet = 2;
		}else if(data->fingers[i].mcount > 240){//1001
			data->LongTouchFlag = 1;//1001
		}
		
	}
	if(data->TimerSet < 4){//0724
		if ((atch_area)||((data->Report_touch_number>3)&&(atch_area == 0))\
			||((data->Report_touch_number>1)&&(data->LongTouchFlag == 1))){//1001
			pr_info("[TSP]66Change T57: tchnum= %d, tch=%d, atch=%d\n", data->Report_touch_number, tch_area, atch_area);//1001
			mxt_write_object(data, MXT_TOUCH_MULTITOUCHSCREEN_T9, 35, 2);//0708

			if(data->Report_touch_number == 1)
					data->AntiTouchGoTrackingNum = 1;
			else if(data->Report_touch_number  > 1)
					data->AntiTouchGoTrackingNum = 2;
		}
		else{//1018
			if((data->Report_touch_number < 2)&&(data->AntiTouchGoTrackingNum==2))
				data->AntiTouchGoTrackingNum = 1;
		}
	}

	if(data->check_after_wakeup == 1){//0619
	        if((tch_area == 0) && (atch_area == 0) && (data->Report_touch_number > 0))
	        {
	                pr_info("[TSP] T57: remove cal tch_area==%d, atch_area==%d  !!!!!!\n", tch_area, atch_area);
	                mxt_command_calibration(data);
	                data->check_after_wakeup = 0;
//0725 mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 151);//0717
	        }else{//0619
			if (atch_area)
				mxt_t8_cal_set(data, 3);
	       }
	 }

#endif	/* CHECK_ANTITOUCH */

#if TSP_USE_SHAPETOUCH
	data->sumsize = message->message[0] + (message->message[1] << 8);
	data->fingers[id].w = tch_area;
#endif	/* TSP_USE_SHAPETOUCH */

}
static void mxt_treat_T61_object(struct mxt_data *data,
		struct mxt_message *message)
{

#if  CHECK_ANTITOUCH
	u8 i;
	u8 distance_check_cnt = 0;
	u8 distance_backup[10];

	if ((message->message[0] & 0xa0) == 0xa0) {	
		if (data->pdata->check_calgood == 1) {
			if (data->Press_cnt == \
			data->Release_cnt) {
				if ((data->tch_value == 0)\
				&& (data->atch_value == 0)) {
					if (data->FirstCal_tch == 0\
					&& data->FirstCal_atch == 0) {
						if (data->FirstCal_t57tch\
						== data->T57_touch) {
							if (\
						data->T57_touch == 0\
						|| data->T57_touch > 12) {
								pr_info("[TSP] CalFail_1 SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
							, data->tch_value
							, data->atch_value
							, data->T57_touch);
							mxt_command_calibration(data);
				} else {
					data->pdata->check_calgood = 0;
					data->Press_Release_check = 0;
					data->pdata->check_afterCalgood = 1;
					pr_info("[TSP] CalGood SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
					, data->tch_value
					, data->atch_value
					, data->T57_touch);
					}
				} else {
					data->pdata->check_calgood = 0;
					data->Press_Release_check = 0;
					data->pdata->check_afterCalgood = 1;
					pr_info("[TSP] CalGood SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
					, data->tch_value
					, data->atch_value
					, data->T57_touch);
					}
				} else {
					data->pdata->check_calgood = 0;
					data->Press_Release_check = 0;
					data->pdata->check_afterCalgood = 1;
					pr_info("[TSP] CalGood SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
					, data->tch_value
					, data->atch_value
					, data->T57_touch);
					}
				} else {
				       if(data->atch_value !=0 ){
						mxt_command_calibration(data);
						pr_info("[TSP] CalFail_2 SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
						, data->tch_value
						, data->atch_value
						, data->T57_touch);
				       }
					else{
						data->pdata->check_calgood = 0;
						data->Press_Release_check = 0;
						data->pdata->check_afterCalgood = 1;
						pr_info("[TSP] CalGood SPT_TIMER_T61 Stop 3sec, tch=%d, atch=%d, t57tch=%d\n"
						, data->tch_value
						, data->atch_value
						, data->T57_touch);

					 }
					}
				} else {
				if (data->atch_value == 0) {
					if (data->finger_area < 35) {									
						mxt_command_calibration(data);
				pr_info("[TSP] CalFail_3 Press_cnt Fail, tch=%d, atch=%d, t57tch=%d\n"
				, data->tch_value
				, data->atch_value
				, data->T57_touch);
				} else {
				pr_info("[TSP] CalGood Press_cnt Fail, tch=%d, atch=%d, t57tch=%d\n"
				, data->tch_value
				, data->atch_value
				, data->T57_touch);
				data->pdata->check_afterCalgood = 1;
				data->pdata->check_calgood = 0;
				data->Press_Release_check = 0;
				}
			}
			else if (data->atch_value < data->tch_value \
				&& data->Report_touch_number < 4) {
				if (data->Report_touch_number == 2 && \
					data->tch_value > 12 && \
					data->T57_touch >=1) {
					pr_info("[TSP] CalGood Press_two touch, tch=%d, atch=%d, num=%d, t57tch=%d\n"
					, data->tch_value
					, data->atch_value
					, data->Report_touch_number
					, data->T57_touch);
					data->pdata->check_calgood = 0;
					data->Press_Release_check = 0;
					data->pdata->check_afterCalgood = 1;
				} else if (data->Report_touch_number == 3 \
					&&	data->tch_value > 18\
					&& data->T57_touch > 8) {
						pr_info("[TSP] CalGood Press_three touch, tch=%d, atch=%d, num=%d, t57tch=%d\n"
				, data->tch_value, data->atch_value
				, data->Report_touch_number, data->T57_touch);
				data->pdata->check_calgood = 0;
				data->Press_Release_check = 0;
				data->pdata->check_afterCalgood = 1;
				} else {
				mxt_command_calibration(data);
				pr_info("[TSP] CalFail_4 Press_cnt Fail, tch=%d, atch=%d, num=%d, t57tch=%d\n"
				, data->tch_value, data->atch_value, \
				data->Report_touch_number, \
				data->T57_touch);
				}
				} else {
				mxt_command_calibration(data);
				pr_info("[TSP] CalFail_5 Press_cnt Fail, tch=%d, atch=%d, num=%d, t57tch=%d\n"
				, data->tch_value, data->atch_value,\
				data->Report_touch_number,\
				data->T57_touch);
				}
				}
		} else if (data->pdata->check_antitouch) {
			if (data->pdata->check_autocal == 1) {
				pr_info("[TSP] Auto cal is on going - 1sec time restart, tch=%d, atch=%d, t57tch=%d\n"
				, data->tch_value, data->atch_value\
				, data->T57_touch);
				data->pdata->check_timer = 0;
				data->coin_check = 0;
				mxt_t8_cal_set(data, 0);
				mxt_t61_timer_set(data, 1,
				MXT_T61_TIMER_ONESHOT,
				MXT_T61_TIMER_CMD_START,
				1000);
			} else {
				data->pdata->check_antitouch = 0;
				data->pdata->check_timer = 0;
				mxt_t8_cal_set(data, 0);
				data->pdata->check_calgood = 1;
				data->coin_check = 0;
				pr_info("[TSP] First Check Good, tch=%d, atch=%d, t57tch=%d\n"
				, data->tch_value, data->atch_value
				, data->T57_touch);
				data->FirstCal_tch = data->tch_value;
				data->FirstCal_atch = data->atch_value;
				data->FirstCal_t57tch = data->T57_touch;
				mxt_t61_timer_set(data, 1,
				MXT_T61_TIMER_ONESHOT,
				MXT_T61_TIMER_CMD_START,
				3000);
		}
	}
		if (!data->Press_Release_check) {
			if (data->pdata->check_afterCalgood) {
				pr_info("[TSP] CalGood 3sec START\n");
				data->AfterCal_cnt = 0;
				data->calgood_anti_cnt = 0;
				data->pdata->check_afterCalgood = 0;
				mxt_t61_timer_set(data, 1,
				MXT_T61_TIMER_ONESHOT,
				MXT_T61_TIMER_CMD_START,
				3000);
			} else {																							
				if(data->AfterCal_cnt >= 40 ){
					data->AfterCal_cnt =0;
					if(data->calgood_anti_cnt >= 34 ){
						mxt_command_calibration(data);
#if DEBUG_TSP
						pr_info("[TSP] CalFail_7 Press_cnt Fail, tch=%d, atch=%d, num=%d, t57tch=%d,calgood_anti_cnt=%d\n"
						, data->tch_value, data->atch_value, \
						data->Report_touch_number, \
						data->T57_touch,data->calgood_anti_cnt);
#endif
					}
					else if(data->calgood_anti_cnt >= 28) {
						data->calgood_anti_cnt = 0;
						pr_info("[TSP] CalGood 6sec Second Check\n");
						mxt_t61_timer_set(data, 1,
						MXT_T61_TIMER_ONESHOT,
						MXT_T61_TIMER_CMD_START,
						3000);
					}
					else{
						pr_info("[TSP] CalGood 6sec STOP\n");
					}
				}
				else{
					data->AfterCal_cnt++;
					if((data->Report_touch_number == data->old_touchnum) && data->Report_touch_number > 0 && data->atch_value!=0){
						 for(i=0; i < data->Report_touch_number; i++){
						 	distance_backup[i]=diff_two_point(data->touchbx[i],data->touchby[i],data->touchbx_backup[i],data->touchby_backup[i]);
							if(distance_backup[i]<3){
								distance_check_cnt++;
							}
							else distance_check_cnt = 0;
#if DEBUG_TSP
							pr_info("[TSP] distance_backup[%d] =%d\n",i, distance_backup[i] );
#endif					
						 }
						 if( distance_check_cnt == data->Report_touch_number) data->calgood_anti_cnt++;
					}
					data->old_touchnum = data->Report_touch_number;

					for( i =0; i < data->Report_touch_number; i++){
						data->touchbx_backup[i] = data->touchbx[i];
						data->touchby_backup[i] = data->touchby[i];
					}
#if DEBUG_TSP
					pr_info("[TSP] AfterCal 100msec cnt=%d, tch=%d, atch=%d, t57tch=%d, num=%d, dis_check=%d, calgood_anti_cnt=%d\n",data->AfterCal_cnt, data->tch_value, data->atch_value
					, data->T57_touch, data->Report_touch_number,distance_check_cnt
					, data->calgood_anti_cnt );
#endif					
					mxt_t61_timer_set(data, 1,
					MXT_T61_TIMER_ONESHOT,
					MXT_T61_TIMER_CMD_START,
					100);
				}

			}
		}

	}
#elif CHECK_ANTITOUCH_SERRANO
	char tag[] = "T61";

#if DEBUG_TSP
	dev_info(&data->client->dev, " %s: %d: datas: %d, %#x, %#x, %#x\n", tag,  //0725
		data->check_antitouch, message->reportid,message->message[0], message->message[1], message->message[2]);
#endif
	if(message->reportid == 15){
		if ((message->message[0] & 0xa0) == 0xa0) {
			switch(data->GoodConditionStep) {
	        case MXT_GR_GDC_STATUS_INIT:
				data->check_antitouch = 0;
				data->TimerSet = 0;
				dev_info(&data->client->dev, "No cfg data in file\n");
				mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_WAITTING);
				break;
			case MXT_GR_GDC_STATUS_WAITTING:
				data->check_antitouch = 0;
				data->TimerSet = 0;
				mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_GETTING);

				if ((data->GoodStep1_AllReleased == 1)&&(data->clear_cover_enable == 0)) {/* already all released status */
					dev_info(&data->client->dev, "T61 : All Touch Released at MXT_GR_GDC_STATUS_WAITTING \n");
					dev_info(&data->client->dev, "T66 Start Golden References in T61 0x%x \n", data->T66_CtrlVal);
					mxt_GR_Caputre_Prime_Process(data);
				}
				break;
             case MXT_GR_GDC_STATUS_AQUIRED:
				if(data->Wakeup_Reset_Check_Press == 1){
					dev_info(&data->client->dev, "[GDC] Timer Out and go to Finish\n");
					data->Wakeup_Reset_Check_Press = 2;
					data->GoldenBadCheckCnt = 0;
					mxt_set_gdc_status(data, MXT_GR_GDC_STATUS_FINISH);
					mxt_t61_timer_set(data, 1,
						MXT_T61_TIMER_ONESHOT,
						MXT_T61_TIMER_CMD_START, 2000);
				}
				break;
			case MXT_GR_GDC_STATUS_FINISH:
				dev_info(&data->client->dev, "[GDC] Sequence is finished in 2 sec\n");
				data->Wakeup_Reset_Check_Press = 3;//0614
				data->GoldenBadCheckCnt = 0;
				mxt_gdc_finish_config(data);
				break;
			default:
				break;
		    }
		}
	} else if(message->reportid == 16) {
		switch(data->GoodConditionStep) {
		case MXT_GR_GDC_STATUS_AQUIRED:
		case MXT_GR_GDC_STATUS_FINISH: //0701
			if ((message->message[0] & 0xa0) == 0xa0) {
				data->GoldenBadCheckCnt = 0;
				if(data->clear_cover_enable != 1){//0725
					mxt_write_object(data, MXT_PROCI_STYLUS_T47, 1, 60);//0923_3
				}    			
			}
				break;
			default :
				break;
		}
	}
#elif CHECK_ANTITOUCH_GOLDEN
	pr_info("[TSP] T61: %d: datas: %d, %#x, %#x, %#x\n",
			data->check_antitouch,message->reportid, message->message[0], message->message[1],  //0725
		 message->message[2]);

	if ((message->message[0] & 0xa0) == 0xa0) {
		if(message->reportid == 20){//0725
			data->check_antitouch = 0;
	//0724		data->TimerSet = 0;//golden
			if(data->TimerSet == 0)//0724
				mxt_t8_cal_set(data, 0);

			if(data->check_after_wakeup == 1){
	                        data->check_after_wakeup = 0;
	//mxt_write_object(data, MXT_GEN_ACQUISITIONCONFIG_T8, 9, 151);//0717
			}

			if(data->AntiTouchGoTrackingNum != 0){
					printk(KERN_ERR"[TSP] Time Out Force Calibration with Ghost\n");//0619
				mxt_command_calibration(data);
			}
		}else if(message->reportid == 21){//0725
			if(data->TimerSet == 3){
				if(data->Report_touch_number)//0805
					data->TimerSet = 2;
				else
					data->TimerSet = 4;
			}
			pr_info("[TSP]T61  data->TimerSet = %d\n",data->TimerSet);
		}
	}

#endif
}

#if MaxStartup_Set
static int mxt_command_backup(struct mxt_data *data, u8 value);

static void mxt_treat_T66_object(struct mxt_data *data,
		struct mxt_message *message)
{
	int error = 0;

	dev_info(&data->client->dev, " FCAL : state[%s]\n",
		 MXT_FCALSTATE(message->message[0]) ? (MXT_FCALSTATE(message->message[0]) == 1 ?
		 "PRIMED" : "GENERATED") : "IDLE");

	switch (MXT_FCALSTATE(message->message[0])) {
	case MXT_FCALSTATE_IDLE:
    mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 40, 9);//0620 Stable to Noisy Auto transaction Enable
    mxt_write_object(data, MXT_PROCG_NOISESUPPRESSION_T72, 60, 9);//0620 Noisy to Very Noisy Auto transaction Enable
		if (message->message[0] & MXT_FCALSTATUS_SEQTO) {
			dev_info(&data->client->dev, "Sequence Timeout:[0x%x]\n", message->message[0]);
		}
		if (message->message[0] & MXT_FCALSTATUS_SEQERR) {
			dev_info(&data->client->dev, "Sequence Error:[0x%x]\n", message->message[0]);
		}
		if (message->message[0] & MXT_FCALSTATUS_SEQDONE) {
			dev_info(&data->client->dev, "Sequence Done:[0x%x]\n", message->message[0]);
			if(data->GoodConditionStep == MXT_GR_GDC_STATUS_GETTING){
				mxt_gdc_acquired_config(data);
				if((data->FcalSeqdoneNum & 0xFF) == 0xFF){//0913
					data->FcalSeqdoneNum = (data->FcalSeqdoneNum & 0xFF00); 
				}else{
					data->FcalSeqdoneNum++;
				}
			}
		}
		break;
	case MXT_FCALSTATE_PRIMED:
		dev_info(&data->client->dev, " data->T66_CtrlVal[0x%x]\n",data->T66_CtrlVal);
		mxt_write_object(data, MXT_SPT_GOLDENREFERENCES_T66,
				0, MXT_FCALCMD(MXT_FCALCMD_GENERATE) | data->T66_CtrlVal |0x83);
		if (error) {
			dev_err(&data->client->dev, "Failed to write FALCMD_GENERATE\n");
		}
		break;

	case MXT_FCALSTATE_GENERATED:
		mxt_write_object(data, MXT_SPT_GOLDENREFERENCES_T66,
				0, MXT_FCALCMD(MXT_FCALCMD_STORE) | data->T66_CtrlVal|0x03);
		if (error) {
			dev_err(&data->client->dev, "Failed to write FALCMD_STORE\n");
		}
		break;

	default:
		dev_info(&data->client->dev, " Invaild Factory Calibration status[0x%x]\n",
				 message->message[0]);
	}
}
#endif

#if CHECK_ANTITOUCH_SERRANO
static void mxt_treat_T72_object(struct mxt_data *data,
                                struct mxt_message *message)
{
	u8 *msg = message->message;

    dev_info(&data->client->dev, "T72 MSG: %02X %02X %02X \n",
    	msg[0], msg[1], msg[2]);
	data->T72_State =  message->message[1] & 0x0F;
}
#endif
static void mxt_treat_T100_object(struct mxt_data *data,
		struct mxt_message *message)
{
	u8 id, index;
	u8 *msg = message->message;
	u8 touch_type = 0, touch_event = 0, touch_detect = 0;

	index = data->reportids[message->reportid].index;

	/* Treate screen messages */
	if (index < MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID) {
		if (index == MXT_T100_SCREEN_MSG_FIRST_RPT_ID)
			/* TODO: Need to be implemeted after fixed protocol
			 * This messages will indicate TCHAREA, ATCHAREA
			 */
			dev_dbg(&data->client->dev, "SCRSTATUS:[%02X] %02X %04X %04X %04X\n",
				 msg[0], msg[1], (msg[3] << 8) | msg[2],
				 (msg[5] << 8) | msg[4],
				 (msg[7] << 8) | msg[6]);
#if TSP_USE_SHAPETOUCH
			data->sumsize = (msg[3] << 8) | msg[2];
#endif	/* TSP_USE_SHAPETOUCH */
		return;
	}

	/* Treate touch status messages */
	id = index - MXT_T100_SCREEN_MESSAGE_NUM_RPT_ID;
	touch_detect = msg[0] >> MXT_T100_DETECT_MSG_MASK;
	touch_type = (msg[0] & 0x70) >> 4;
	touch_event = msg[0] & 0x0F;

	dev_dbg(&data->client->dev, "TCHSTATUS [%d] : DETECT[%d] TYPE[%d] EVENT[%d] %d,%d,%d,%d,%d\n",
		id, touch_detect, touch_type, touch_event,
		msg[1] | (msg[2] << 8),	msg[3] | (msg[4] << 8),
		msg[5], msg[6], msg[7]);

	switch (touch_type)	{
	case MXT_T100_TYPE_FINGER:
	case MXT_T100_TYPE_PASSIVE_STYLUS:
	case MXT_T100_TYPE_HOVERING_FINGER:
		/* There are no touch on the screen */
		if (!touch_detect) {
			if (touch_event == MXT_T100_EVENT_UP
				|| touch_event == MXT_T100_EVENT_SUPPESS) {

				data->fingers[id].w = 0;
				data->fingers[id].z = 0;
				data->fingers[id].state = MXT_STATE_RELEASE;
				data->fingers[id].type = touch_type;
				data->fingers[id].event = touch_event;

				mxt_report_input_data(data);
			} else {
				dev_err(&data->client->dev, "Untreated Undetectd touch : type[%d], event[%d]\n",
					touch_type, touch_event);
			}
			break;
		}

		/* There are touch on the screen */
		if (touch_event == MXT_T100_EVENT_DOWN
			|| touch_event == MXT_T100_EVENT_UNSUPPRESS
			|| touch_event == MXT_T100_EVENT_MOVE
			|| touch_event == MXT_T100_EVENT_NONE) {

			data->fingers[id].x = msg[1] | (msg[2] << 8);
			data->fingers[id].y = msg[3] | (msg[4] << 8);

			/* AUXDATA[n]'s order is depended on which values are
			 * enabled or not.
			 */
#if TSP_USE_SHAPETOUCH
			data->fingers[id].component = msg[5];
#endif
			data->fingers[id].z = msg[6];
			data->fingers[id].w = msg[7];

			if (touch_type == MXT_T100_TYPE_HOVERING_FINGER) {
				data->fingers[id].w = 0;
				data->fingers[id].z = 0;
			}

			if (touch_event == MXT_T100_EVENT_DOWN
				|| touch_event == MXT_T100_EVENT_UNSUPPRESS) {
				data->fingers[id].state = MXT_STATE_PRESS;
				data->fingers[id].mcount = 0;
			} else {
				data->fingers[id].state = MXT_STATE_MOVE;
				data->fingers[id].mcount += 1;
			}
			data->fingers[id].type = touch_type;
			data->fingers[id].event = touch_event;

			mxt_report_input_data(data);
		} else {
			dev_err(&data->client->dev, "Untreated Detectd touch : type[%d], event[%d]\n",
				touch_type, touch_event);
		}
		break;
	case MXT_T100_TYPE_ACTIVE_STYLUS:
		break;
	}
}

static int mxt_command_reset(struct mxt_data *data, u8 value);

static irqreturn_t mxt_irq_thread(int irq, void *ptr)
{
	struct mxt_data *data = ptr;
	struct mxt_message message;
	struct device *dev = &data->client->dev;
	u8 reportid, type;
	int i = 0;

	do {
		if (mxt_read_message(data, &message)) {
			dev_err(dev, "Failed to read message\n");
			dev_err(dev, "[report id =0x%x messgae=0x%x]\n",message.reportid, message.message[0]);
			for (i = 0; i < 5; i++) {
				if (mxt_read_message(data, &message))
					dev_err(dev, "[report id =0x%x messgae=0x%x]\n",message.reportid, message.message[0]);
				else
					break;
			}
			if (i==5) {
				mxt_release_all_finger(data);
				dev_err(dev, "i2c failed.. ic soft reset\n");
				if (mxt_command_reset(data, MXT_RESET_VALUE))
					dev_err(dev, "Failed Reset IC\n");
				goto end;
			}
		}

#if TSP_USE_ATMELDBG
		if (data->atmeldbg.display_log) {
			print_hex_dump(KERN_INFO, "MXT MSG:",
				DUMP_PREFIX_NONE, 16, 1,
				&message,
				sizeof(struct mxt_message), false);
		}
#endif
		reportid = message.reportid;

		if (reportid > data->max_reportid)
			goto end;

		type = data->reportids[reportid].type;

		switch (type) {
		case MXT_RESERVED_T0:
			goto end;
			break;
		case MXT_GEN_COMMANDPROCESSOR_T6:
			mxt_treat_T6_object(data, &message);
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T9:
			mxt_treat_T9_object(data, &message);
			break;
#if ENABLE_TOUCH_KEY
		case MXT_TOUCH_KEYARRAY_T15:
			mxt_treat_T15_object(data, &message);
			break;
#endif
		case MXT_SPT_SELFTEST_T25:
			dev_err(dev, "Self test fail [0x%x 0x%x 0x%x 0x%x]\n",
				message.message[0], message.message[1],
				message.message[2], message.message[3]);
			if(message.message[0] == 0x12){
				mxt_command_reset(data, MXT_RESET_VALUE);
			}
			break;
		case MXT_PROCI_TOUCHSUPPRESSION_T42:
			mxt_treat_T42_object(data, &message);
			break;
		case MXT_PROCI_EXTRATOUCHSCREENDATA_T57:
			mxt_treat_T57_object(data, &message);
			data->t57update=true;//0924_3
			break;
		case MXT_SPT_TIMER_T61:
			mxt_treat_T61_object(data, &message);
			break;
		case MXT_PROCG_NOISESUPPRESSION_T62:
			break;
#if MaxStartup_Set			
		case MXT_SPT_GOLDENREFERENCES_T66:
			mxt_treat_T66_object(data, &message);
			break;
#endif
#if CHECK_ANTITOUCH_SERRANO
		case MXT_PROCG_NOISESUPPRESSION_T72://0614
			mxt_treat_T72_object(data, &message);
			break;
#endif			
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			mxt_treat_T100_object(data, &message);
			break;

		default:
			dev_dbg(dev, "Untreated Object type[%d]\tmessage[0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x]\n",
				type, message.message[0],
				message.message[1], message.message[2],
				message.message[3], message.message[4],
				message.message[5], message.message[6]);
			break;
		}
	} while (!gpio_get_value(data->pdata->tsp_int));

#if CHECK_PALM //0617
	if((data->PalmFlag == 0)||(data->PressEventCheck ==1)){
		if(data->t57update==true){//0924_3
			if (data->finger_mask){
				mxt_report_input_data(data);
	   			data->t57update=false;
			}
		}
	}
#else
	if(data->t57update==true) {//0924_3
		if (data->finger_mask) {
		  mxt_report_input_data(data);
		  data->t57update=false;//0924_3
  		 }
	}
#endif

end:
	return IRQ_HANDLED;
}

static int mxt_get_bootloader_version(struct i2c_client *client, u8 val)
{
	u8 buf[3];

	if (val & MXT_BOOT_EXTENDED_ID) {
		if (i2c_master_recv(client, buf, sizeof(buf)) != sizeof(buf)) {
			dev_err(&client->dev, "%s: i2c recv failed\n",
				 __func__);
			return -EIO;
		}
		dev_info(&client->dev, "Bootloader ID:%d Version:%d",
			 buf[1], buf[2]);
	} else {
		dev_info(&client->dev, "Bootloader ID:%d",
			 val & MXT_BOOT_ID_MASK);
	}
	return 0;
}

static int mxt_check_bootloader(struct i2c_client *client,
				     unsigned int state)
{
	u8 val;

recheck:
	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	switch (state) {
	case MXT_WAITING_BOOTLOAD_CMD:
		if (mxt_get_bootloader_version(client, val))
			return -EIO;
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_WAITING_FRAME_DATA:
	case MXT_APP_CRC_FAIL:
		val &= ~MXT_BOOT_STATUS_MASK;
		break;
	case MXT_FRAME_CRC_PASS:
		if (val == MXT_FRAME_CRC_CHECK)
			goto recheck;
		if (val == MXT_FRAME_CRC_FAIL) {
			dev_err(&client->dev, "Bootloader CRC fail\n");
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	if (val != state) {
		dev_err(&client->dev,
			 "Invalid bootloader mode state 0x%X\n", val);
		return -EINVAL;
	}

	return 0;
}

static int mxt_unlock_bootloader(struct i2c_client *client)
{
	u8 buf[2] = {MXT_UNLOCK_CMD_LSB, MXT_UNLOCK_CMD_MSB};

	if (i2c_master_send(client, buf, 2) != 2) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);

		return -EIO;
	}

	return 0;
}

#ifdef TSP_BRING_UP
static int mxt_probe_bootloader(struct i2c_client *client)
{
	u8 val;

	if (i2c_master_recv(client, &val, 1) != 1) {
		dev_err(&client->dev, "%s: i2c recv failed\n", __func__);
		return -EIO;
	}

	if (val & (~MXT_BOOT_STATUS_MASK)) {
		if (val & MXT_APP_CRC_FAIL)
			dev_err(&client->dev, "Application CRC failure\n");
		else
			dev_err(&client->dev, "Device in bootloader mode\n");
	} else {
		dev_err(&client->dev, "%s: Unknow status\n", __func__);
		return -EIO;
	}
	return 0;
}
#endif
static int mxt_fw_write(struct i2c_client *client,
				const u8 *frame_data, unsigned int frame_size)
{
	if (i2c_master_send(client, frame_data, frame_size) != frame_size) {
		dev_err(&client->dev, "%s: i2c send failed\n", __func__);
		return -EIO;
	}

	return 0;
}

#if DUAL_CFG
int mxt_verify_fw(struct mxt_fw_info *fw_info, const struct firmware *fw)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_fw_image *fw_img;

	if (!fw) {
		dev_err(dev, "could not find firmware file\n");
		return -ENOENT;
	}

	fw_img = (struct mxt_fw_image *)fw->data;

	if (le32_to_cpu(fw_img->magic_code) != MXT_FW_MAGIC) {
		/* In case, firmware file only consist of firmware */
		dev_info(dev, "Firmware file only consist of raw firmware\n");
		fw_info->fw_len = fw->size;
		fw_info->fw_raw_data = fw->data;
	} else {
		/*
		 * In case, firmware file consist of header,
		 * configuration, firmware.
		 */
		dev_info(dev, "Firmware file consist of header, configuration, firmware\n");
		fw_info->fw_ver = fw_img->fw_ver;
		fw_info->build_ver = fw_img->build_ver;
		fw_info->hdr_len = le32_to_cpu(fw_img->hdr_len);
		fw_info->cfg_len = le32_to_cpu(fw_img->cfg_len);
		fw_info->fw_len = le32_to_cpu(fw_img->fw_len);
		fw_info->cfg_crc = le32_to_cpu(fw_img->cfg_crc);

		/* Check the firmware file with header */
		if (fw_info->hdr_len != sizeof(struct mxt_fw_image)
			|| fw_info->hdr_len + fw_info->cfg_len
				+ fw_info->fw_len != fw->size) {
			dev_err(dev, "Firmware file is invaild !!hdr size[%d] cfg,fw size[%d,%d] filesize[%d]\n",
				fw_info->hdr_len, fw_info->cfg_len,
				fw_info->fw_len, fw->size);
			return -EINVAL;
		}

		if (!fw_info->cfg_len) {
			dev_err(dev, "Firmware file dose not include configuration data\n");
			return -EINVAL;
		}
		if (!fw_info->fw_len) {
			dev_err(dev, "Firmware file dose not include raw firmware data\n");
			return -EINVAL;
		}

		/* Get the address of configuration data */
		data->cfg_len = fw_info->cfg_len / 2;
		data->batt_cfg_raw_data = fw_info->batt_cfg_raw_data
			= fw_img->data;
		data->ta_cfg_raw_data = fw_info->ta_cfg_raw_data
			= fw_img->data +  (fw_info->cfg_len / 2) ;

		/* Get the address of firmware data */
		fw_info->fw_raw_data = fw_img->data + fw_info->cfg_len;
#if SUPPORT_CONFIG_VER
		memcpy(data->fw_config_ver, &(data->batt_cfg_raw_data[3]), 8);
		data->ic_config_ver[8] = '\0';
		dev_info(dev, "fw_config_ver =%s\n", data->fw_config_ver);
#endif
#if TSP_SEC_FACTORY
		data->fdata->fw_ver = fw_info->fw_ver;
		data->fdata->build_ver = fw_info->build_ver;
#endif
	}

	return 0;
}
#else
int mxt_verify_fw(struct mxt_fw_info *fw_info, const struct firmware *fw)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	struct mxt_fw_image *fw_img;

	if (!fw) {
		dev_err(dev, "could not find firmware file\n");
		return -ENOENT;
	}

	fw_img = (struct mxt_fw_image *)fw->data;

	if (le32_to_cpu(fw_img->magic_code) != MXT_FW_MAGIC) {
		/* In case, firmware file only consist of firmware */
		dev_dbg(dev, "Firmware file only consist of raw firmware\n");
		fw_info->fw_len = fw->size;
		fw_info->fw_raw_data = fw->data;
	} else {
		/*
		 * In case, firmware file consist of header,
		 * configuration, firmware.
		 */
		dev_info(dev, "Firmware file consist of header, configuration, firmware\n");
		fw_info->fw_ver = fw_img->fw_ver;
		fw_info->build_ver = fw_img->build_ver;
		fw_info->hdr_len = le32_to_cpu(fw_img->hdr_len);
		fw_info->cfg_len = le32_to_cpu(fw_img->cfg_len);
		fw_info->fw_len = le32_to_cpu(fw_img->fw_len);
		fw_info->cfg_crc = le32_to_cpu(fw_img->cfg_crc);

		/* Check the firmware file with header */
		if (fw_info->hdr_len != sizeof(struct mxt_fw_image)
			|| fw_info->hdr_len + fw_info->cfg_len
				+ fw_info->fw_len != fw->size) {
			dev_err(dev, "Firmware file is invaild !!hdr size[%d] cfg,fw size[%d,%d] filesize[%d]\n",
				fw_info->hdr_len, fw_info->cfg_len,
				fw_info->fw_len, fw->size);
			return -EINVAL;
		}

		if (!fw_info->cfg_len) {
			dev_err(dev, "Firmware file dose not include configuration data\n");
			return -EINVAL;
		}
		if (!fw_info->fw_len) {
			dev_err(dev, "Firmware file dose not include raw firmware data\n");
			return -EINVAL;
		}

		/* Get the address of configuration data */
		fw_info->cfg_raw_data = fw_img->data;

		/* Get the address of firmware data */
		fw_info->fw_raw_data = fw_img->data + fw_info->cfg_len;

#if TSP_SEC_FACTORY
		data->fdata->fw_ver = fw_info->fw_ver;
		data->fdata->build_ver = fw_info->build_ver;
#endif
	}

	return 0;
}
#endif

static int mxt_wait_for_chg(struct mxt_data *data, u16 time)
{
	int timeout_counter = 0;

	msleep(time);

	if (data->pdata->tsp_int) {
		while (gpio_get_value(data->pdata->tsp_int)
			&& timeout_counter++ <= 20) {

			msleep(MXT_RESET_INTEVAL_TIME);
			dev_err(&data->client->dev, "Spend %d time waiting for chg_high\n",
				(MXT_RESET_INTEVAL_TIME * timeout_counter)
				 + time);
		}
	}

	return 0;
}

static int mxt_command_reset(struct mxt_data *data, u8 value)
{
	int error;

	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_RESET, value);

	error = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (error)
		dev_err(&data->client->dev, "Not respond after reset command[%d]\n",
			value);

	return error;
}

static int mxt_command_calibration(struct mxt_data *data)
{
	int ret = 0;

	dev_info(&data->client->dev, "Command Calibration!!\n");//0608
#if CHECK_ANTITOUCH
	if (data->cal_busy)
		return ret;
#endif
	ret = mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
						MXT_COMMAND_CALIBRATE, 1);

#if CHECK_ANTITOUCH
	/* set flag for calibration lockup
	recovery if cal command was successful */
	data->cal_busy = 1;
#endif
	return ret;
}

static int mxt_command_backup(struct mxt_data *data, u8 value)
{
	mxt_write_object(data, MXT_GEN_COMMANDPROCESSOR_T6,
			MXT_COMMAND_BACKUPNV, value);

	msleep(MXT_BACKUP_TIME);

	return 0;
}

static int mxt_flash_fw(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct i2c_client *client = data->client_boot;
	struct device *dev = &data->client->dev;
	const u8 *fw_data = fw_info->fw_raw_data;
	size_t fw_size = fw_info->fw_len;
	unsigned int frame_size;
	unsigned int pos = 0;
	int ret;

	if (!fw_data) {
		dev_err(dev, "firmware data is Null\n");
		return -ENOMEM;
	}

	ret = mxt_check_bootloader(client, MXT_WAITING_BOOTLOAD_CMD);
	if (ret) {
		/*may still be unlocked from previous update attempt */
		ret = mxt_check_bootloader(client, MXT_WAITING_FRAME_DATA);
		if (ret)
			goto out;
	} else {
		dev_info(dev, "Unlocking bootloader\n");
		/* Unlock bootloader */
		mxt_unlock_bootloader(client);
	}
	while (pos < fw_size) {
		ret = mxt_check_bootloader(client,
					MXT_WAITING_FRAME_DATA);
		if (ret) {
			dev_err(dev, "Fail updating firmware. wating_frame_data err\n");
			goto out;
		}

		frame_size = ((*(fw_data + pos) << 8) | *(fw_data + pos + 1));

		/*
		* We should add 2 at frame size as the the firmware data is not
		* included the CRC bytes.
		*/

		frame_size += 2;

		/* Write one frame to device */
		mxt_fw_write(client, fw_data + pos, frame_size);

		ret = mxt_check_bootloader(client,
						MXT_FRAME_CRC_PASS);
		if (ret) {
			dev_err(dev, "Fail updating firmware. frame_crc err\n");
			goto out;
		}

		pos += frame_size;

		dev_dbg(dev, "Updated %d bytes / %zd bytes\n",
				pos, fw_size);

		msleep(20);
	}

	ret = mxt_wait_for_chg(data, MXT_SW_RESET_TIME);
	if (ret) {
		dev_err(dev, "Not respond after F/W  finish reset\n");
		goto out;
	}

	dev_info(dev, "success updating firmware\n");
out:
	return ret;
}

static void mxt_handle_init_data(struct mxt_data *data)
{
/*
 * Caution : This function is called before backup NV. So If you write
 * register vaules directly without config file in this function, it can
 * be a cause of that configuration CRC mismatch or unintended values are
 * stored in Non-volatile memory in IC. So I would recommed do not use
 * this function except for bring up case. Please keep this in your mind.
 */
	return;
}

static int mxt_read_id_info(struct mxt_data *data)
{
	int ret = 0;
	u8 id[MXT_INFOMATION_BLOCK_SIZE];

	/* Read IC information */
	ret = mxt_read_mem(data, 0, MXT_INFOMATION_BLOCK_SIZE, id);
	if (ret) {
		dev_err(&data->client->dev, "Read fail. IC information\n");
		goto out;
	} else {
		dev_info(&data->client->dev,
			"family: 0x%x variant: 0x%x version: 0x%x"
			" build: 0x%x matrix X,Y size:  %d,%d"
			" number of obect: %d\n"
			, id[0], id[1], id[2], id[3], id[4], id[5], id[6]);
		data->info.family_id = id[0];
		data->info.variant_id = id[1];
		data->info.version = id[2];
		data->info.build = id[3];
		data->info.matrix_xsize = id[4];
		data->info.matrix_ysize = id[5];
		data->info.object_num = id[6];
	}

out:
	return ret;
}

static int mxt_get_object_table(struct mxt_data *data)
{
	int error;
	int i;
	u16 reg;
	u8 reportid = 0;
	u8 buf[MXT_OBJECT_TABLE_ELEMENT_SIZE];

	for (i = 0; i < data->info.object_num; i++) {
		struct mxt_object *object = data->objects + i;

		reg = MXT_OBJECT_TABLE_START_ADDRESS +
				MXT_OBJECT_TABLE_ELEMENT_SIZE * i;
		error = mxt_read_mem(data, reg,
				MXT_OBJECT_TABLE_ELEMENT_SIZE, buf);
		if (error)
			return error;

		object->type = buf[0];
		object->start_address = (buf[2] << 8) | buf[1];
		/* the real size of object is buf[3]+1 */
		object->size = buf[3] + 1;
		/* the real instances of object is buf[4]+1 */
		object->instances = buf[4] + 1;
		object->num_report_ids = buf[5];

		dev_dbg(&data->client->dev,
			"Object:T%d\t\t\t Address:0x%x\tSize:%d\tInstance:%d\tReport Id's:%d\n",
			object->type, object->start_address, object->size,
			object->instances, object->num_report_ids);

		if (object->num_report_ids) {
			reportid += object->num_report_ids * object->instances;
			object->max_reportid = reportid;
		}
	}

	/* Store maximum reportid */
	data->max_reportid = reportid;
	dev_dbg(&data->client->dev, "maXTouch: %d report ID\n",
			data->max_reportid);

	return 0;
}

static void mxt_make_reportid_table(struct mxt_data *data)
{
	struct mxt_object *objects = data->objects;
	struct mxt_reportid *reportids = data->reportids;
	int i, j;
	int id = 0;

	for (i = 0; i < data->info.object_num; i++) {
		for (j = 0; j < objects[i].num_report_ids *
				objects[i].instances; j++) {
			id++;

			reportids[id].type = objects[i].type;
			reportids[id].index = j;

			dev_dbg(&data->client->dev, "Report_id[%d]:\tT%d\tIndex[%d]\n",
				id, reportids[id].type, reportids[id].index);
		}
	}
}

static int mxt_initialize(struct mxt_data *data)
{
	struct i2c_client *client = data->client;

	u32 read_info_crc, calc_info_crc;
	int ret;

	ret = mxt_read_id_info(data);
	if (ret)
		return ret;

	data->objects = kcalloc(data->info.object_num,
				sizeof(struct mxt_object),
				GFP_KERNEL);
	if (!data->objects) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Get object table infomation */
	ret = mxt_get_object_table(data);
	if (ret)
		goto out;

	data->reportids = kcalloc(data->max_reportid + 1,
			sizeof(struct mxt_reportid),
			GFP_KERNEL);
	if (!data->reportids) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}

	/* Make report id table */
	mxt_make_reportid_table(data);

	/* Verify the info CRC */
	ret = mxt_read_info_crc(data, &read_info_crc);
	if (ret)
		goto out;

	ret = mxt_calculate_infoblock_crc(data, &calc_info_crc);
	if (ret)
		goto out;

	if (read_info_crc != calc_info_crc) {
		dev_err(&data->client->dev, "Infomation CRC error :[CRC 0x%06X!=0x%06X]\n",
				read_info_crc, calc_info_crc);
		ret = -EFAULT;
		goto out;
	}
	return 0;

out:
	return ret;
}

static int  mxt_rest_initialize(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int ret;

	/* Restore memory and stop event handing */
	ret = mxt_command_backup(data, MXT_DISALEEVT_VALUE);
	if (ret) {
		dev_err(dev, "Failed Restore NV and stop event\n");
		goto out;
	}

	/* Write config */
	ret = mxt_write_config(fw_info);
	if (ret) {
		dev_err(dev, "Failed to write config from file\n");
		goto out;
	}

	/* Handle data for init */
	mxt_handle_init_data(data);

	/* Backup to memory */
	ret = mxt_command_backup(data, MXT_BACKUP_VALUE);
	if (ret) {
		dev_err(dev, "Failed backup NV data\n");
		goto out;
	}

	/* Soft reset */
	ret = mxt_command_reset(data, MXT_RESET_VALUE);
	if (ret) {
		dev_err(dev, "Failed Reset IC\n");
		goto out;
	}
out:
	return ret;
}
static int mxt_power_on(struct mxt_data *data)
{
/*
 * If do not turn off the power during suspend, you can use deep sleep
 * or disable scan to use T7, T9 Object. But to turn on/off the power
 * is better.
 */
	int error = 0;

	if (data->mxt_enabled)
		return 0;

	error = mxt_power_onoff(data,1);
	if (error) {
		dev_err(&data->client->dev, "Failed to power on\n");
		goto out;
	}

	error = mxt_wait_for_chg(data, MXT_HW_RESET_TIME);
	if (error)
		dev_err(&data->client->dev, "Not respond after H/W reset\n");

	data->mxt_enabled = true;

out:
	return error;
}

static int mxt_power_off(struct mxt_data *data)
{
	int error = 0;

	if (!data->mxt_enabled)
		return 0;

	error = mxt_power_onoff(data,0);
	if (error) {
		dev_err(&data->client->dev, "Failed to power off\n");
		goto out;
	}

	data->mxt_enabled = false;

out:
	return error;
}

/* Need to be called by function that is blocked with mutex */
static int mxt_start(struct mxt_data *data)
{
	int error = 0;

	if (data->mxt_enabled) {
		dev_err(&data->client->dev,
			"%s. but touch already on\n", __func__);
		return error;
	}
	
#if CHECK_ANTITOUCH_SERRANO
       data->check_antitouch = 0;
#if CHECK_PALM //0617
	data->PalmFlag = 0;
#endif
	data->GoldenBadCheckCnt = 0;
	data->WakeupPowerOn = 1;//0614
#elif CHECK_ANTITOUCH_GOLDEN
	data->check_after_wakeup = 1;
#endif

	error = mxt_power_on(data);

	if (error) {
		dev_err(&data->client->dev, "Fail to start touch\n");
	} else {
//		if (system_rev == 0) {
//			mxt_command_calibration(data);
//			dev_err(&data->client->dev, "Force calibration\n");
//		}
		enable_irq(data->client->irq);
	}

	return error;
}

/* Need to be called by function that is blocked with mutex */
static int mxt_stop(struct mxt_data *data)
{
	int error = 0;

	if (!data->mxt_enabled) {
		dev_err(&data->client->dev,
			"%s. but touch already off\n", __func__);
		return error;
	}
	disable_irq(data->client->irq);

	error = mxt_power_off(data);
	if (error) {
		dev_err(&data->client->dev, "Fail to stop touch\n");
		goto err_power_off;
	}
	mxt_release_all_finger(data);

#if ENABLE_TOUCH_KEY
	mxt_release_all_keys(data);
#endif

#ifdef TSP_BOOSTER
	set_dvfs_lock(data, -1);
#endif
	return 0;

err_power_off:
	enable_irq(data->client->irq);
	return error;
}

#ifdef USE_OPEN_CLOSE
static int mxt_input_open(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);
	int ret;

#ifdef TSP_INIT_COMPLETE
	ret = wait_for_completion_interruptible_timeout(&data->init_done,
			msecs_to_jiffies(90 * MSEC_PER_SEC));

	if (ret < 0) {
		dev_err(&data->client->dev,
			"error while waiting for device to init (%d)\n", ret);
		ret = -ENXIO;
		goto err_open;
	}
	if (ret == 0) {
		dev_err(&data->client->dev,
			"timedout while waiting for device to init\n");
		ret = -ENXIO;
		goto err_open;
	}
#endif
	ret = mxt_start(data);
	if (ret)
		goto err_open;

	dev_dbg(&data->client->dev, "%s\n", __func__);

	return 0;

err_open:
	return ret;
}

static void mxt_input_close(struct input_dev *dev)
{
	struct mxt_data *data = input_get_drvdata(dev);

	mxt_stop(data);

	dev_dbg(&data->client->dev, "%s\n", __func__);
}
#endif

static int mxt_make_highchg(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	struct mxt_message message;
	int count = data->max_reportid * 2;
	int error;

	/* Read dummy message to make high CHG pin */
	do {
		error = mxt_read_message(data, &message);
		if (error)
			return error;
	} while (message.reportid != 0xff && --count);

	if (!count) {
		dev_err(dev, "CHG pin isn't cleared\n");
		return -EBUSY;
	}

	return 0;
}

static int mxt_touch_finish_init(struct mxt_data *data)
{
	struct i2c_client *client = data->client;
	int error;

	client->irq = gpio_to_irq(data->pdata->tsp_int);
	dev_info(&data->client->dev, "%s: tsp : gpio_to_irq : %d\n",
			__func__, client->irq);

	error = request_threaded_irq(client->irq,
				NULL, mxt_irq_thread,
				IRQF_TRIGGER_LOW | IRQF_ONESHOT,
				client->dev.driver->name, data);

	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		goto err_req_irq;
	}

	error = mxt_make_highchg(data);
	if (error) {
		dev_err(&client->dev, "Failed to clear CHG pin\n");
		goto err_req_irq;
	}

	dev_info(&client->dev,  "Mxt touch controller initialized\n");

#ifdef TSP_INIT_COMPLETE
	/*
	* to prevent unnecessary report of touch event
	* it will be enabled in open function
	*/
#ifdef USE_OPEN_CLOSE
	mxt_stop(data);
#endif

	/* for blocking to be excuted open function untile finishing ts init */
	complete_all(&data->init_done);
#endif
	return 0;

err_req_irq:
	return error;
}

static int mxt_touch_rest_init(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int error;

	error = mxt_initialize(data);
	if (error) {
		dev_err(dev, "Failed to initialize\n");
		goto err_free_mem;
	}

	error = mxt_rest_initialize(fw_info);
	if (error) {
		dev_err(dev, "Failed to rest initialize\n");
		goto err_free_mem;
	}

	error = mxt_touch_finish_init(data);
	if (error)
		goto err_free_mem;

	return 0;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;
	kfree(data->reportids);
	data->reportids = NULL;
	return error;
}

#ifdef TSP_BRING_UP
static int mxt_enter_bootloader(struct mxt_data *data)
{	int error;

#if SUPPORT_CONFIG_VER
	error = mxt_command_reset(data, MXT_BOOT_VALUE);
	return error;
#else
	struct device *dev = &data->client->dev;

	data->objects = kcalloc(data->info.object_num,
				     sizeof(struct mxt_object),
				     GFP_KERNEL);
	if (!data->objects) {
		dev_err(dev, "%s Failed to allocate memory\n",
			__func__);
		error = -ENOMEM;
		goto out;
	}

	/* Get object table information*/
	error = mxt_get_object_table(data);
	if (error)
		goto err_free_mem;

	/* Change to the bootloader mode */
	error = mxt_command_reset(data, MXT_BOOT_VALUE);
	if (error)
		goto err_free_mem;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;

out:
	return error;
#endif	
}
#endif

#if SUPPORT_CONFIG_VER
static int check_config_ver(struct mxt_data *data)
{
	struct device *dev = &data->client->dev;
	int error;
	struct mxt_object *user_object;

	data->objects = kcalloc(data->info.object_num,
				     sizeof(struct mxt_object),
				     GFP_KERNEL);
	if (!data->objects) {
		dev_err(dev, "%s Failed to allocate memory\n",
			__func__);
		error = -ENOMEM;
		goto out;
	}

	/* Get object table information*/
	error = mxt_get_object_table(data);
	if (error)
		goto err_free_mem;

	/* Get the config version from userdata */
	user_object = mxt_get_object(data, MXT_SPT_USERDATA_T38);
	if (!user_object) {
		dev_err(dev, "T38 fail to get object_info\n");
		error = -EINVAL;
		goto err_free_mem;
	}

	error = mxt_read_mem(data, user_object->start_address,
			8, data->ic_config_ver);
	data->ic_config_ver[8] = '\0';
	dev_info(dev, "ic_config_ver = %s\n",data->ic_config_ver);
	if (error)
		goto err_free_mem;
	else
		return 0;

err_free_mem:
	kfree(data->objects);
	data->objects = NULL;

out:
	return error;
}
#endif

#ifdef TSP_BRING_UP
static int mxt_flash_fw_on_probe(struct mxt_fw_info *fw_info)
{
	struct mxt_data *data = fw_info->data;
	struct device *dev = &data->client->dev;
	int error;

	error = mxt_read_id_info(data);

	if (error) {
		/* need to check IC is in boot mode */
		error = mxt_probe_bootloader(data->client_boot);
		if (error) {
			dev_err(dev, "Failed to verify bootloader's status\n");
			goto out;
		}

		dev_info(dev, "Updating firmware from boot-mode\n");
		goto load_fw;
	}

	/* compare the version to verify necessity of firmware updating */
#if SUPPORT_CONFIG_VER
	error = check_config_ver(data);
	if (error) {
		dev_err(dev, "failed check config ver\n");
		goto out;
	}

	if (data->info.version == fw_info->fw_ver && data->info.build == fw_info->build_ver) {
		if ((strcmp(data->ic_config_ver, data->fw_config_ver)) < 0)
			dev_info(dev, "config version need update!!!\n");
		else	{
			dev_info(dev, "config version is latest !!!\n");
			kfree(data->objects);
			data->objects = NULL;
			goto out;
		}
	} else
#endif
	if (data->info.version > fw_info->fw_ver ||
		(data->info.version == fw_info->fw_ver && data->info.build >= fw_info->build_ver)) {
		dev_info(dev, "Firmware version is latest. FW update does not need!!!\n");
		goto out;
	}

	dev_info(dev, "Updating firmware from app-mode : IC:0x%x,0x%x =! FW:0x%x,0x%x\n",
			data->info.version, data->info.build,
			fw_info->fw_ver, fw_info->build_ver);

	error = mxt_enter_bootloader(data);
	if (error) {
		dev_err(dev, "Failed updating firmware\n");
		goto out;
	}

load_fw:
	error = mxt_flash_fw(fw_info);
	if (error)
		dev_err(dev, "Failed updating firmware\n");
	else
		dev_info(dev, "succeeded updating firmware\n");
out:
	return error;
}
#endif
static int mxt_touch_init_firmware(const struct firmware *fw,
		void *context)
{
	struct mxt_data *data = context;
	struct mxt_fw_info fw_info;
	int error;

	memset(&fw_info, 0, sizeof(struct mxt_fw_info));
	fw_info.data = data;

#if FW_UPDATE_ENABLE
	error = mxt_verify_fw(&fw_info, fw);
	if (error)
		goto ts_rest_init;

	/* Skip update on boot up if firmware file does not have a header */
	if (!fw_info.hdr_len)
		goto ts_rest_init;
#ifdef TSP_BRING_UP
	error = mxt_flash_fw_on_probe(&fw_info);
	if (error)
		goto out;
#endif	
#endif

ts_rest_init:
	error = mxt_touch_rest_init(&fw_info);
#ifdef TSP_BRING_UP
out:
#ifdef TSP_INIT_COMPLETE	
	if (error)
		/* complete anyway, so open() doesn't get blocked */
		complete_all(&data->init_done);
#endif	
#if FW_UPDATE_ENABLE
	release_firmware(fw);
#endif
#endif
	return error;
}

static void mxt_request_firmware_work(const struct firmware *fw,
		void *context)
{
	struct mxt_data *data = context;
	mxt_touch_init_firmware(fw, data);
}

static int mxt_touch_init(struct mxt_data *data, bool nowait)
{
	struct i2c_client *client = data->client;
	const char *firmware_name =
		 data->pdata->firmware_name ?: MXT_DEFAULT_FIRMWARE_NAME;
	int ret = 0;

#if TSP_INFORM_CHARGER
	/* Register callbacks */
	/* To inform tsp , charger connection status*/
	data->callbacks.inform_charger = inform_charger;
	if (data->pdata->register_cb) {
		data->pdata->register_cb(&data->callbacks);
		inform_charger_init(data);
	}
#endif

	if (nowait) {
		const struct firmware *fw;
		char fw_path[MXT_MAX_FW_PATH];

		memset(&fw_path, 0, MXT_MAX_FW_PATH);

		snprintf(fw_path, MXT_MAX_FW_PATH, "%s%s",
			MXT_FIRMWARE_INKERNEL_PATH, firmware_name);

		dev_err(&client->dev, "%s\n", fw_path);

#if FW_UPDATE_ENABLE
		ret = request_firmware(&fw, fw_path, &client->dev);
		if (ret) {
			dev_err(&client->dev,
				"error requesting built-in firmware\n");
			goto out;
		}
#endif
		ret = mxt_touch_init_firmware(fw, data);
	} else {
		ret = request_firmware_nowait(THIS_MODULE, true, firmware_name,
				      &client->dev, GFP_KERNEL,
				      data, mxt_request_firmware_work);
		if (ret)
			dev_err(&client->dev,
				"cannot schedule firmware update (%d)\n", ret);
	}

out:
	return ret;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
#define mxt_suspend	NULL
#define mxt_resume	NULL

static void mxt_early_suspend(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
#if TSP_INFORM_CHARGER
	cancel_delayed_work_sync(&data->noti_dwork);
#endif

	mutex_lock(&data->input_dev->mutex);

	mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
}

static void mxt_late_resume(struct early_suspend *h)
{
	struct mxt_data *data = container_of(h, struct mxt_data,
								early_suspend);
	mutex_lock(&data->input_dev->mutex);

	mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
}
#else
static int mxt_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->input_dev->mutex);

	mxt_stop(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}

static int mxt_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mxt_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->input_dev->mutex);

	mxt_start(data);

	mutex_unlock(&data->input_dev->mutex);
	return 0;
}
#endif

/* Added for samsung dependent codes such as Factory test,
 * Touch booster, Related debug sysfs.
 */
#include "mxts_sec.c"

static int mxt_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct mxt_data *data;
	struct mxt_platform_data *pdata;
	struct input_dev *input_dev;
	u16 boot_address;
	int error = 0;

	printk(KERN_ERR "%s", __func__);

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct mxt_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev,
				"%s: Failed to allocate memory\n",
				__func__);
			return -ENOMEM;
		}
		error = mxt_parse_dt(&client->dev, pdata);

		if (error)
			return error;
	} else	{
		pdata = client->dev.platform_data;
		dev_err(&client->dev,
			"%s: TSP failed to align dtsi", __func__);
	}

	if (!pdata) {
		dev_err(&client->dev,
				"%s: Platform data is not proper\n", __func__);
		return -EINVAL;
	}

	data = kzalloc(sizeof(struct mxt_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	data->pdata = pdata;

	if(mxt_request_gpio(data) <0)
	{
		error =  -EINVAL;
		goto err_allocate_input_device;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		error = -ENOMEM;
		dev_err(&client->dev, "Input device allocation failed\n");
		goto err_allocate_input_device;
	}

	input_dev->name = "sec_touchscreen";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
#ifdef USE_OPEN_CLOSE	
	input_dev->open = mxt_input_open;
	input_dev->close = mxt_input_close;
#endif
	data->client = client;
	data->input_dev = input_dev;
	data->pdata = pdata;
#ifdef TSP_INIT_COMPLETE	
	init_completion(&data->init_done);
#endif

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

#if ENABLE_TOUCH_KEY
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_BACK, input_dev->keybit);
#endif


	input_mt_init_slots(input_dev, MXT_MAX_FINGER,0);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X,
				0, pdata->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
				0, pdata->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
				0, MXT_AREA_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE,
				0, MXT_AMPLITUDE_MAX, 0, 0);
#if TSP_USE_SHAPETOUCH
//	input_set_abs_params(input_dev, ABS_MT_COMPONENT,
//				0, MXT_COMPONENT_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_SUMSIZE,
				0, MXT_SUMSIZE_MAX, 0, 0);
#endif

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);

	if (data->pdata->boot_address) {
		boot_address = data->pdata->boot_address;
	} else {
		if (client->addr == MXT_APP_LOW)
			boot_address = MXT_BOOT_LOW;
		else
			boot_address = MXT_BOOT_HIGH;
	}
	data->client_boot = i2c_new_dummy(client->adapter, boot_address);
	if (!data->client_boot) {
		dev_err(&client->dev, "Fail to register sub client[0x%x]\n",
			 boot_address);
		error = -ENODEV;
		goto err_create_sub_client;
	}

	/* regist input device */
	error = input_register_device(input_dev);
	if (error)
		goto err_register_input_device;

	error = mxt_sysfs_init(client);
	if (error < 0) {
		dev_err(&client->dev, "Failed to create sysfs\n");
		goto err_sysfs_init;
	}

#if defined(CONFIG_OF)
	i2c_vddo_vreg = regulator_get(&data->client->dev,"vddo");
	if (IS_ERR(i2c_vddo_vreg)){
		i2c_vddo_vreg = NULL;
		dev_info(&data->client->dev," %s: i2c_vddo_vreg is error , %d \n", __func__, __LINE__);
	}
#endif	

	error = mxt_power_on(data);
	if (error) {
		dev_err(&client->dev, "Failed to power_on\n");
		goto err_power_on;
	}

#ifdef TSP_BOOSTER
	mxt_init_dvfs(data);
#endif

	error = mxt_touch_init(data, MXT_FIRMWARE_UPDATE_TYPE);
	if (error) {
		dev_err(&client->dev, "Failed to init driver\n");
		goto err_touch_init;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 2;
	data->early_suspend.suspend = mxt_early_suspend;
	data->early_suspend.resume = mxt_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

#if CHECK_ANTITOUCH_SERRANO
	mxt_gdc_init_config(data);//0617
	mxt_command_calibration(data);
	data->FcalSeqdoneNum =0;//0801
	dev_info(&data->client->dev, "Mxt 336S Probed\n");	
#elif CHECK_ANTITOUCH_GOLDEN
	data->check_after_wakeup = 1;
	data->AfterProbe = 1;//0723
#endif

	return 0;

err_touch_init:
	data->pdata->register_cb(NULL);
	mxt_power_off(data);
err_power_on:
	mxt_sysfs_remove(data);
err_sysfs_init:
	input_unregister_device(input_dev);
	input_dev = NULL;
err_register_input_device:
	i2c_unregister_device(data->client_boot);
err_create_sub_client:
	input_free_device(input_dev);
err_allocate_input_device:
	kfree(data);

	return error;
}

static int mxt_remove(struct i2c_client *client)
{
	struct mxt_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);
	kfree(data->objects);
	kfree(data->reportids);
	input_unregister_device(data->input_dev);
	i2c_unregister_device(data->client_boot);
	mxt_sysfs_remove(data);
	mxt_power_off(data);
	kfree(data);

	return 0;
}
#ifdef CONFIG_OF
static struct of_device_id mxts_match_table[] = {
	{ .compatible = "atmel,mxts-ts",},
	{ },
};
#else
#define mxts_match_table	NULL
#endif

static struct i2c_device_id mxt_idtable[] = {
	{MXT_DEV_NAME, 0},
};

MODULE_DEVICE_TABLE(i2c, mxt_idtable);

static const struct dev_pm_ops mxt_pm_ops = {
	.suspend = mxt_suspend,
	.resume = mxt_resume,
};

static struct i2c_driver mxt_i2c_driver = {
	.id_table = mxt_idtable,
	.probe = mxt_probe,
	.remove = mxt_remove,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= MXT_DEV_NAME,
#ifdef CONFIG_OF
		.of_match_table = mxts_match_table,
#endif
#ifdef CONFIG_PM
		.pm	= &mxt_pm_ops,
#endif
	},
};

static int __init mxt_i2c_init(void)
{
	return i2c_add_driver(&mxt_i2c_driver);
}

static void __exit mxt_i2c_exit(void)
{
	i2c_del_driver(&mxt_i2c_driver);
}

module_init(mxt_i2c_init);
module_exit(mxt_i2c_exit);

MODULE_DESCRIPTION("Atmel MaXTouch driver");
MODULE_AUTHOR("bumwoo.lee<bw365.lee@samsung.com>");
MODULE_LICENSE("GPL");
