/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/crc32.h>
#include <linux/lzo.h>
#include "msm_sd.h"
#include "msm_cci.h"
#if defined(CONFIG_SR544)
#include "msm_otp_sr544.h"
#elif defined(CONFIG_SR552)
#include "msm_otp_sr552.h"
#else
#include "msm_otp_s5k5e3yx.h"
#endif
#include <media/msm_cam_sensor.h>

//#define MSM_EEPROM_DEBUG 1

#undef CDBG
#ifdef MSM_EEPROM_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) pr_debug(fmt, ##args)
#endif

#undef EEPROM_MMAP_DEBUG

uint8_t *map_data = NULL;

DEFINE_MSM_MUTEX(msm_eeprom_mutex);

static int msm_eeprom_match_id(struct msm_eeprom_ctrl_t *e_ctrl);
static int read_eeprom_memory(struct msm_eeprom_ctrl_t *e_ctrl,
			      struct msm_eeprom_memory_block_t *block);


static int msm_eeprom_get_dt_data(struct msm_eeprom_ctrl_t *e_ctrl);
/**
  * msm_eeprom_verify_sum - verify crc32 checksum
  * @mem:	data buffer
  * @size:	size of data buffer
  * @sum:	expected checksum
  *
  * Returns 0 if checksum match, -EINVAL otherwise.
  */
static int msm_eeprom_verify_sum(const char *mem, uint32_t size, uint32_t sum)
{
	uint32_t crc = ~0UL;

	/* check overflow */
	if (size > crc - sizeof(uint32_t))
		return -EINVAL;

	crc = crc32_le(crc, mem, size);
	if (~crc != sum) {
		pr_err("%s: expect 0x%x, result 0x%x\n", __func__, sum, ~crc);
		pr_err("Check eeprom or interface");
		return -EINVAL;
	}
	pr_err("%s: checksum pass 0x%x\n", __func__, sum);
	return 0;
}

/**
  * msm_eeprom_match_crc - verify multiple regions using crc
  * @data:	data block to be verified
  *
  * Iterates through all regions stored in @data.  Regions with odd index
  * are treated as data, and its next region is treated as checksum.  Thus
  * regions of even index must have valid_size of 4 or 0 (skip verification).
  * Returns a bitmask of verified regions, starting from LSB.  1 indicates
  * a checksum match, while 0 indicates checksum mismatch or not verified.
  */
static uint32_t msm_eeprom_match_crc(struct msm_eeprom_memory_block_t *data)
{
	int j, rc;
	uint32_t *sum;
	uint32_t ret = 0;
	uint8_t *memptr, *memptr_crc;
	struct msm_eeprom_memory_map_t *map;

	if (!data) {
		pr_err("%s data is NULL", __func__);
		return -EINVAL;
	}
	map = data->map;

	for (j = 0; j + 1 < data->num_map; j += 2) {
		memptr = data->mapdata + map[j].mem.addr;
		memptr_crc = data->mapdata + map[j+1].mem.addr;

		/* empty table or no checksum */
		if (!map[j].mem.valid_size || !map[j+1].mem.valid_size) {
			continue;
		}
		if (map[j+1].mem.valid_size != sizeof(uint32_t)) {
			pr_err("%s: malformatted data mapping\n", __func__);
			return -EINVAL;
		}
		sum = (uint32_t *) (memptr_crc);
		rc = msm_eeprom_verify_sum(memptr, map[j].mem.valid_size, *sum);
		if (!rc)
		{
			ret |= 1 << (j/2);
		}
	}
	return ret;
}

static int msm_eeprom_get_cmm_data(struct msm_eeprom_ctrl_t *e_ctrl,
				       struct msm_eeprom_cfg_data *cdata)
{
	int rc = 0;
	struct msm_eeprom_cmm_t *cmm_data = &e_ctrl->eboard_info->cmm_data;
	cdata->cfg.get_cmm_data.cmm_support = cmm_data->cmm_support;
	cdata->cfg.get_cmm_data.cmm_compression = cmm_data->cmm_compression;
	cdata->cfg.get_cmm_data.cmm_size = cmm_data->cmm_size;
	return rc;
}

/**
  * msm_eeprom_power_up() - power up eeprom if it's not on
  * @e_ctrl:	control struct
  * @down:	output to indicate whether power down is needed later
  *
  * This function powers up EEPROM only if it's not already on.  If power
  * up is performed here, @down will be set to true.  Caller should power
  * down EEPROM after transaction if @down is true.
  */
static int msm_eeprom_power_up(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0;
	pr_warn("%s : E", __func__);

	rc = msm_camera_power_up(&e_ctrl->eboard_info->power_info,
		e_ctrl->eeprom_device_type, &e_ctrl->i2c_client);

	return rc;
}
/**
  * msm_eeprom_power_down() - power down eeprom
  * @e_ctrl:	control struct
  * @down:	indicate whether kernel powered up eeprom before
  *
  * This function powers down EEPROM only if it's powered on by calling
  * msm_eeprom_power_up() before.  If @down is false, no action will be
  * taken.  Otherwise, eeprom will be powered down.
  */
static int msm_eeprom_power_down(struct msm_eeprom_ctrl_t *e_ctrl)
{
	pr_warn("%s : E", __func__);
	return msm_camera_power_down(&e_ctrl->eboard_info->power_info,
		e_ctrl->eeprom_device_type, &e_ctrl->i2c_client);
}

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
static int prepare_read_write_data(struct msm_eeprom_ctrl_t *e_ctrl, uint8_t addr_h, uint8_t addr_l, enum msm_camera_i2c_data_type data_type, int isWrite)
{
	int rc=0;

	isWrite = !(!isWrite)+1;

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &init_otp);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
			&(e_ctrl->i2c_client), r_otp_addr_h,
			addr_h, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
			&(e_ctrl->i2c_client), r_otp_addr_l,
			addr_l, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
			&(e_ctrl->i2c_client), r_otp_cmd,
			isWrite, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	return rc;
}
#endif

static int eeprom_config_read_cal_data(struct msm_eeprom_ctrl_t *e_ctrl,
				       struct msm_eeprom_cfg_data *cdata)
{
	int rc;

	/* check range */
	if (cdata->cfg.read_data.num_bytes > e_ctrl->cal_data.num_data) {
		pr_err("%s: Invalid size. exp %u, req %u\n", __func__,
		     e_ctrl->cal_data.num_data,
		     cdata->cfg.read_data.num_bytes);
		return -EINVAL;
	}
	if (!e_ctrl->cal_data.mapdata) {
    pr_err("%s : is NULL", __func__);
		return -EFAULT;
  }
	CDBG("%s:%d: subdevid: %d",__func__,__LINE__,e_ctrl->subdev_id);
	rc = copy_to_user(cdata->cfg.read_data.dbuffer,
		e_ctrl->cal_data.mapdata,
		cdata->cfg.read_data.num_bytes);
      /* this below code is giving probelm in device -> encrypt -> came launch
      Ideally we should not free this data until dtor. so commentiong out */

	return rc;
}

static int eeprom_config_read_data(struct msm_eeprom_ctrl_t *e_ctrl,
				   struct msm_eeprom_cfg_data *cdata)
{
	char *buf;
	int rc = 0;
	uint16_t data;
	int i;
#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	uint8_t addr_h, addr_l;
#endif

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	return rc;
#endif

	buf = kmalloc(cdata->cfg.read_data.num_bytes+1, GFP_KERNEL);
	if (!buf) {
		pr_err("%s : buf is NULL", __func__);
		return -ENOMEM;
    }

	rc = msm_eeprom_power_up(e_ctrl);
	if (rc < 0) {
		pr_err("%s: failed to power on otp\n", __func__);
		goto FREE;
	}

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	addr_h = ((cdata->cfg.read_data.addr)>>8)&0xFF;
	addr_l = cdata->cfg.read_data.addr&0xFF;

	rc = prepare_read_write_data(e_ctrl, addr_h, addr_l, MSM_CAMERA_I2C_BYTE_DATA, 0);
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		goto POWER_DOWN;
	}
#else
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &init_read_otp);
	if (rc < 0) {
		pr_err("%s:(%d) init_read_otp failed\n", __func__, __LINE__);
		goto POWER_DOWN;
	}
#endif

	for(i=0; i<cdata->cfg.read_data.num_bytes; i++) {
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
				&(e_ctrl->i2c_client),
#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
				r_otp_rdata,
#else
				cdata->cfg.read_data.addr+i,
#endif
				&data, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			pr_err("%s:(%d) read failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}
		buf[i]=data;
	}

#if !defined(CONFIG_SR544) && !defined(CONFIG_SR552)
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &finish_read_otp);
	if (rc < 0) {
		pr_err("%s:(%d) finish_read_otp failed\n", __func__, __LINE__);
		goto POWER_DOWN;
	}
#endif
	buf[cdata->cfg.read_data.num_bytes] = '\0';
	pr_err("[sjlee] %s : buf[%s]\n", __func__, buf);

	rc = copy_to_user(cdata->cfg.read_data.dbuffer, buf,
			  cdata->cfg.read_data.num_bytes);
POWER_DOWN:
	msm_eeprom_power_down(e_ctrl);
FREE:
	kfree(buf);
	return rc;
}

static int eeprom_config_read_fw_version(struct msm_eeprom_ctrl_t *e_ctrl,
				   struct msm_eeprom_cfg_data *cdata)
{
	char *buf;
	int rc = 0;

	buf = kmalloc(cdata->cfg.read_data.num_bytes, GFP_KERNEL);
	if (!buf) {
		pr_err("%s : buf is NULL", __func__);
		return -ENOMEM;
	}
	memset(buf, 0, cdata->cfg.read_data.num_bytes);
	if(e_ctrl->cal_data.num_data < (cdata->cfg.read_data.addr + cdata->cfg.read_data.num_bytes)) {
		pr_err("%s : requested data size was mismatched! addr:size[0x%x:%d]\n", __func__, cdata->cfg.read_data.addr, cdata->cfg.read_data.num_bytes);
		rc = -ENOMEM;
		goto FREE;
	}

	memcpy(buf, &map_data[cdata->cfg.read_data.addr], cdata->cfg.read_data.num_bytes);

	rc = copy_to_user(cdata->cfg.read_data.dbuffer, buf,
			  cdata->cfg.read_data.num_bytes);

FREE:

	kfree(buf);
	return rc;
}

static int eeprom_config_read_compressed_data(struct msm_eeprom_ctrl_t *e_ctrl,
	struct msm_eeprom_cfg_data *cdata)
{
	int rc = 0;
	uint8_t *buf_comp = NULL;
	uint8_t *buf_decomp = NULL;
	uint32_t decomp_size;
	uint16_t data;
	int i;
#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	uint8_t addr_h, addr_l;
#endif

	pr_err("%s: address (0x%x) comp_size (%d) after decomp (%d)", __func__,
	cdata->cfg.read_data.addr,
	cdata->cfg.read_data.comp_size, cdata->cfg.read_data.num_bytes);

	buf_comp = kmalloc(cdata->cfg.read_data.comp_size, GFP_KERNEL);
	buf_decomp = kmalloc(cdata->cfg.read_data.num_bytes, GFP_KERNEL);
	if (!buf_decomp || !buf_comp) {
		pr_err("%s: kmalloc fail", __func__);
		rc = -ENOMEM;
		goto FREE;
	}

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	addr_h = ((cdata->cfg.read_data.addr)>>8)&0xFF;
	addr_l = cdata->cfg.read_data.addr&0xFF;

	rc = prepare_read_write_data(e_ctrl, addr_h, addr_l, MSM_CAMERA_I2C_BYTE_DATA, 0);
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		goto POWER_DOWN;
	}
#else
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &init_read_otp);
	if (rc < 0) {
		pr_err("%s:(%d) init_read_otp failed\n", __func__, __LINE__);
		goto POWER_DOWN;
	}
#endif

	for(i=0; i<cdata->cfg.read_data.comp_size; i++) {
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
				&(e_ctrl->i2c_client),
#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
				r_otp_rdata,
#else
				cdata->cfg.read_data.addr+i,
#endif
				&data, MSM_CAMERA_I2C_BYTE_DATA);
		if (rc < 0) {
			pr_err("%s:(%d) read failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}
		buf_comp[i]=data;
	}

#if !defined(CONFIG_SR544) && !defined(CONFIG_SR552)
	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &finish_read_otp);
	if (rc < 0) {
		pr_err("%s:(%d) finish_read_otp failed\n", __func__, __LINE__);
		goto POWER_DOWN;
	}
#endif

	pr_err("%s: crc = 0x%08X\n", __func__, *(uint32_t*)&buf_comp[cdata->cfg.read_data.comp_size-4]);
	//  compressed data(buf_comp) contains uncompressed crc32 value.
	rc = msm_eeprom_verify_sum(buf_comp, cdata->cfg.read_data.comp_size-4,
		*(uint32_t*)&buf_comp[cdata->cfg.read_data.comp_size-4]);

	if (rc < 0) {
		pr_err("%s: crc check error, rc %d\n", __func__, rc);
		goto POWER_DOWN;
	}

	decomp_size = cdata->cfg.read_data.num_bytes;
	rc = lzo1x_decompress_safe(buf_comp, cdata->cfg.read_data.comp_size-4,
	                           buf_decomp, &decomp_size);
	if (rc != LZO_E_OK) {
		pr_err("%s: decompression failed %d", __func__, rc);
		goto POWER_DOWN;
	}
	rc = copy_to_user(cdata->cfg.read_data.dbuffer, buf_decomp, decomp_size);

	if (rc < 0) {
		pr_err("%s: failed to copy to user\n", __func__);
		goto POWER_DOWN;
	}

	pr_info("%s: done", __func__);

POWER_DOWN:
#if 0 //  just once to power up when load lib
	msm_eeprom_power_down(e_ctrl);
#endif

	FREE:
	if (buf_comp) kfree(buf_comp);
	if (buf_decomp) kfree(buf_decomp);

	return rc;
}

static int eeprom_config_write_data(struct msm_eeprom_ctrl_t *e_ctrl,
				    struct msm_eeprom_cfg_data *cdata)
{
	int rc = 0;

	char *buf = NULL;
	void *work_mem = NULL;
	uint8_t *compressed_buf = NULL;
	uint32_t compressed_size = 0;
	uint32_t crc = ~0UL;
	int i;
#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	uint8_t addr_h, addr_l;
#endif

	pr_warn("%s: compress ? %d size %d", __func__,
		cdata->cfg.write_data.compress, cdata->cfg.write_data.num_bytes);
	buf = kmalloc(cdata->cfg.write_data.num_bytes, GFP_KERNEL);
	if (!buf) {
		pr_err("%s: allocation failed 1", __func__);
		return -ENOMEM;
	}
	rc = copy_from_user(buf, cdata->cfg.write_data.dbuffer,
			    cdata->cfg.write_data.num_bytes);
	if (rc < 0) {
		pr_err("%s: failed to copy write data\n", __func__);
		goto FREE;
	}
	/* compress */
	if (cdata->cfg.write_data.compress) {
		compressed_buf = kmalloc(cdata->cfg.write_data.num_bytes +
			cdata->cfg.write_data.num_bytes / 16 + 64 + 3, GFP_KERNEL);
		if (!compressed_buf) {
			pr_err("%s: allocation failed 2", __func__);
			rc = -ENOMEM;
			goto FREE;
		}
		work_mem = kmalloc(LZO1X_1_MEM_COMPRESS, GFP_KERNEL);
		if (!work_mem) {
			pr_err("%s: allocation failed 3", __func__);
			rc = -ENOMEM;
			goto FREE;
		}
		if (lzo1x_1_compress(buf, cdata->cfg.write_data.num_bytes,
				compressed_buf, &compressed_size, work_mem) != LZO_E_OK) {
			pr_err("%s: compression failed", __func__);
			goto FREE;
		}

		crc = crc32_le(crc, compressed_buf, compressed_size);
		crc = ~crc;

		pr_err("%s: compressed size %d, crc=0x%0X", __func__, compressed_size, crc);
		*cdata->cfg.write_data.write_size = compressed_size + 4;  //  include CRC size
	}
	rc = msm_eeprom_power_up(e_ctrl);
	if (rc < 0) {
		pr_err("%s: failed to power on eeprom\n", __func__);
		goto FREE;
	}

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	if (cdata->cfg.write_data.compress) {

		addr_h = ((cdata->cfg.write_data.addr)>>8)&0xFF;
		addr_l = cdata->cfg.write_data.addr&0xFF;

		rc = prepare_read_write_data(e_ctrl, addr_h, addr_l, MSM_CAMERA_I2C_BYTE_DATA, 1);
		if (rc < 0) {
			pr_err("%s: failed to prepare read/write on otp\n", __func__);
			goto POWER_DOWN;
		}

		for(i=0; i<compressed_size; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), r_otp_wdata,
					compressed_buf[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}

		addr_h = ((cdata->cfg.write_data.addr+compressed_size)>>8)&0xFF;
		addr_l = (cdata->cfg.write_data.addr+compressed_size)&0xFF;

		rc = prepare_read_write_data(e_ctrl, addr_h, addr_l, MSM_CAMERA_I2C_BYTE_DATA, 1);
		if (rc < 0) {
			pr_err("%s: failed to prepare read/write on otp\n", __func__);
			goto POWER_DOWN;
		}

		//  write CRC32 for compressed data
		for(i=0; i<4; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), r_otp_wdata,
					((uint8_t *)crc)[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}
	} else {
		addr_h = ((cdata->cfg.write_data.addr)>>8)&0xFF;
		addr_l = cdata->cfg.write_data.addr&0xFF;

		rc = prepare_read_write_data(e_ctrl, addr_h, addr_l, MSM_CAMERA_I2C_BYTE_DATA, 1);
		if (rc < 0) {
			pr_err("%s: failed to prepare read/write on otp\n", __func__);
			goto POWER_DOWN;
		}

		for(i=0; i<cdata->cfg.write_data.num_bytes; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), r_otp_wdata,
					buf[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}
	}
#else
	if (cdata->cfg.write_data.compress) {
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &init_write_otp);
		if (rc < 0) {
			pr_err("%s:(%d) init_write_otp failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}

		for(i=0; i<compressed_size; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), cdata->cfg.write_data.addr+i,
					compressed_buf[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &init_write_otp);
		if (rc < 0) {
			pr_err("%s:(%d) init_write_otp failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}

		//  write CRC32 for compressed data
		for(i=0; i<4; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), cdata->cfg.write_data.addr+compressed_size+i,
					((uint8_t *)crc)[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &finish_write_otp);
		if (rc < 0) {
			pr_err("%s:(%d) finish_write_otp failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}
	} else {
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &init_write_otp);
		if (rc < 0) {
			pr_err("%s:(%d) init_write_otp failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}

		for(i=0; i<cdata->cfg.write_data.num_bytes; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
					&(e_ctrl->i2c_client), cdata->cfg.write_data.addr+i,
					buf[i], MSM_CAMERA_I2C_BYTE_DATA);
			if (rc < 0) {
				pr_err("%s:(%d) write failed\n", __func__, __LINE__);
				goto POWER_DOWN;
			}
		}
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &finish_write_otp);
		if (rc < 0) {
			pr_err("%s:(%d) finish_write_otp failed\n", __func__, __LINE__);
			goto POWER_DOWN;
		}
	}
#endif

	if (rc < 0) {
		pr_err("%s: failed to write data, rc %d\n", __func__, rc);
		goto POWER_DOWN;
	}
	CDBG("%s: done", __func__);
POWER_DOWN:
	msm_eeprom_power_down(e_ctrl);
FREE:
	if (buf) kfree(buf);
	if (compressed_buf) kfree(compressed_buf);
	if (work_mem) kfree(work_mem);
	return rc;
}
static int eeprom_config_erase(struct msm_eeprom_ctrl_t *e_ctrl,
			       struct msm_eeprom_cfg_data *cdata)
{

	int rc = 0;
#if 0
	pr_warn("%s: erasing addr 0x%x, size %u\n", __func__,
	     cdata->cfg.erase_data.addr, cdata->cfg.erase_data.num_bytes);
	rc = msm_eeprom_power_up(e_ctrl);
	if (rc < 0) {
		pr_err("%s: failed to power on eeprom\n", __func__);
		return rc;
	}
	rc = msm_camera_spi_erase(&e_ctrl->i2c_client,
		cdata->cfg.erase_data.addr, cdata->cfg.erase_data.num_bytes);
	if (rc < 0)
		pr_err("%s: failed to erase eeprom\n", __func__);
	msm_eeprom_power_down(e_ctrl);
#endif
	return rc;
}

static int32_t msm_eeprom_read_eeprom_data(struct msm_eeprom_ctrl_t *e_ctrl)
{
    int32_t rc = 0;

    /* power up */
    rc = msm_camera_power_up(&e_ctrl->eboard_info->power_info,
                           e_ctrl->eeprom_device_type,
                           &e_ctrl->i2c_client);
    if (rc) {
        pr_err("failed rc %d\n", rc);
    return rc;
    }

    /* read cal data */
    rc = read_eeprom_memory(e_ctrl, &e_ctrl->cal_data);
    if (rc < 0) {
        pr_err("%s read_eeprom_memory failed\n", __func__);
        goto POWER_DOWN;
    }

    /* update support info */
    e_ctrl->is_supported = (msm_eeprom_match_crc(&e_ctrl->cal_data) << 1) | 1;
    pr_err("%s:%d is_supported = 0x%X\n", __func__, __LINE__, e_ctrl->is_supported);

POWER_DOWN:
    /* power down */
    if (msm_camera_power_down(&e_ctrl->eboard_info->power_info,
                            e_ctrl->eeprom_device_type,
                            &e_ctrl->i2c_client) < 0)
    pr_err("%s:%d pwoer down failed\n", __func__, __LINE__);
    pr_err("%s:%d Exit\n", __func__, __LINE__);

    return rc;
}

static int msm_eeprom_config(struct msm_eeprom_ctrl_t *e_ctrl,
			     void __user *argp)
{
	struct msm_eeprom_cfg_data *cdata =
		(struct msm_eeprom_cfg_data *)argp;
	int rc = 0;

	CDBG("%s:%d: subdevid: %d, cfgtype: %d\n",__func__,__LINE__,e_ctrl->subdev_id, cdata->cfgtype);
	switch (cdata->cfgtype) {
	case CFG_EEPROM_GET_INFO:
		CDBG("%s E CFG_EEPROM_GET_INFO\n", __func__);
		cdata->is_supported = e_ctrl->is_supported;
		memcpy(cdata->cfg.eeprom_name,
			e_ctrl->eboard_info->eeprom_name,
			sizeof(cdata->cfg.eeprom_name));
		break;
	case CFG_EEPROM_GET_CAL_DATA:
		CDBG("%s E CFG_EEPROM_GET_CAL_DATA\n", __func__);
		cdata->cfg.get_data.num_bytes =
			e_ctrl->cal_data.num_data;
		break;
	case CFG_EEPROM_READ_CAL_DATA:
		CDBG("%s E CFG_EEPROM_READ_CAL_DATA\n", __func__);
		rc = eeprom_config_read_cal_data(e_ctrl, cdata);
		break;
	case CFG_EEPROM_READ_DATA:
		CDBG("%s E CFG_EEPROM_READ_DATA\n", __func__);
		rc = eeprom_config_read_data(e_ctrl, cdata);
		break;
	case CFG_EEPROM_READ_COMPRESSED_DATA:
		rc = eeprom_config_read_compressed_data(e_ctrl, cdata);
    if (rc < 0)
      pr_err("%s : eeprom_config_read_compressed_data failed", __func__);
		break;
	case CFG_EEPROM_WRITE_DATA:
		pr_warn("%s E CFG_EEPROM_WRITE_DATA\n", __func__);
		rc = eeprom_config_write_data(e_ctrl, cdata);
		break;
	case CFG_EEPROM_READ_DATA_FROM_HW:
		e_ctrl->is_supported = 0x01;
		pr_err ("kernel is supported before : %X\n",e_ctrl->is_supported);
		rc = msm_eeprom_read_eeprom_data(e_ctrl);
		pr_err ("kernel is supported after : %X\n",e_ctrl->is_supported);
		cdata->is_supported = e_ctrl->is_supported;
		if (rc < 0) {
			pr_err("%s:%d failed rc %d\n", __func__, __LINE__,  rc);
			break;
		}
		rc = copy_to_user(cdata->cfg.read_data.dbuffer,
			e_ctrl->cal_data.mapdata,
			cdata->cfg.read_data.num_bytes);
		break;
	case CFG_EEPROM_GET_MM_INFO:
		CDBG("%s E CFG_EEPROM_GET_MM_INFO\n", __func__);
		rc = msm_eeprom_get_cmm_data(e_ctrl, cdata);
		break;

	case CFG_EEPROM_ERASE:
		pr_warn("%s E CFG_EEPROM_ERASE\n", __func__);
		rc = eeprom_config_erase(e_ctrl, cdata);
		break;
	case CFG_EEPROM_POWER_ON:
		rc = msm_eeprom_power_up(e_ctrl);
		if (rc < 0)
			pr_err("%s : msm_eeprom_power_up failed", __func__);
		break;
	case CFG_EEPROM_POWER_OFF:
		rc = msm_eeprom_power_down(e_ctrl);
		if (rc < 0)
			pr_err("%s : msm_eeprom_power_down failed", __func__);
		break;
	case CFG_EEPROM_GET_FW_VERSION_INFO:
		CDBG("%s E CFG_EEPROM_GET_FW_VERSION_INFO\n", __func__);
		rc = eeprom_config_read_fw_version(e_ctrl, cdata);
		break;
	default:
		break;
	}

	pr_err("%s X rc: %d\n", __func__, rc);
	return rc;
}

static int msm_eeprom_get_subdev_id(struct msm_eeprom_ctrl_t *e_ctrl,
				    void *arg)
{
	uint32_t *subdev_id = (uint32_t *)arg;
	CDBG("%s E\n", __func__);
	if (!subdev_id) {
		pr_err("%s failed\n", __func__);
		return -EINVAL;
	}
	*subdev_id = e_ctrl->subdev_id;
	CDBG("subdev_id %d\n", *subdev_id);
	CDBG("%s X\n", __func__);
	return 0;
}

static long msm_eeprom_subdev_ioctl(struct v4l2_subdev *sd,
		unsigned int cmd, void *arg)
{
	struct msm_eeprom_ctrl_t *e_ctrl = v4l2_get_subdevdata(sd);
	void __user *argp = (void __user *)arg;
	CDBG("%s E\n", __func__);
	CDBG("%s:%d a_ctrl %p argp %p\n", __func__, __LINE__, e_ctrl, argp);
	switch (cmd) {
	case VIDIOC_MSM_SENSOR_GET_SUBDEV_ID:
		return msm_eeprom_get_subdev_id(e_ctrl, argp);
	case VIDIOC_MSM_EEPROM_CFG:
		return msm_eeprom_config(e_ctrl, argp);
	default:
		return -ENOIOCTLCMD;
	}

	pr_err("%s X\n", __func__);
}

static struct msm_camera_i2c_fn_t msm_eeprom_cci_func_tbl = {
	.i2c_read = msm_camera_cci_i2c_read,
	.i2c_read_seq = msm_camera_cci_i2c_read_seq,
	.i2c_write = msm_camera_cci_i2c_write,
	.i2c_write_seq = msm_camera_cci_i2c_write_seq,
	.i2c_write_table = msm_camera_cci_i2c_write_table,
	.i2c_write_seq_table = msm_camera_cci_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
	msm_camera_cci_i2c_write_table_w_microdelay,
	.i2c_util = msm_sensor_cci_i2c_util,
	.i2c_poll = msm_camera_cci_i2c_poll,
};

static struct msm_camera_i2c_fn_t msm_eeprom_qup_func_tbl = {
	.i2c_read = msm_camera_qup_i2c_read,
	.i2c_read_seq = msm_camera_qup_i2c_read_seq,
	.i2c_write = msm_camera_qup_i2c_write,
	.i2c_write_table = msm_camera_qup_i2c_write_table,
	.i2c_write_seq_table = msm_camera_qup_i2c_write_seq_table,
	.i2c_write_table_w_microdelay =
	msm_camera_qup_i2c_write_table_w_microdelay,
};

static struct msm_camera_i2c_fn_t msm_eeprom_spi_func_tbl = {
	.i2c_read = msm_camera_spi_read,
	.i2c_read_seq = msm_camera_spi_read_seq,
};

static int msm_eeprom_open(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh) {
	int rc = 0;
	struct msm_eeprom_ctrl_t *e_ctrl =  v4l2_get_subdevdata(sd);
	CDBG("%s E\n", __func__);
	if (!e_ctrl) {
		pr_err("%s failed e_ctrl is NULL\n", __func__);
		return -EINVAL;
	}
	CDBG("%s X\n", __func__);
	return rc;
}

static int msm_eeprom_close(struct v4l2_subdev *sd,
	struct v4l2_subdev_fh *fh) {
	int rc = 0;
	struct msm_eeprom_ctrl_t *e_ctrl =  v4l2_get_subdevdata(sd);
	CDBG("%s E\n", __func__);
	if (!e_ctrl) {
		pr_err("%s failed e_ctrl is NULL\n", __func__);
		return -EINVAL;
	}
	CDBG("%s X\n", __func__);
	return rc;
}

static const struct v4l2_subdev_internal_ops msm_eeprom_internal_ops = {
	.open = msm_eeprom_open,
	.close = msm_eeprom_close,
};

uint8_t* get_eeprom_data_addr()
{
	return map_data;
}

/**
  * read_eeprom_memory() - read map data into buffer
  * @e_ctrl:	eeprom control struct
  * @block:	block to be read
  *
  * This function iterates through blocks stored in block->map, reads each
  * region and concatenate them into the pre-allocated block->mapdata
  */
#if defined(CONFIG_SR544)
//  compare dtsi file to GTA when L-OS upgrade for rossa/core-prime.
static int read_eeprom_memory(struct msm_eeprom_ctrl_t *e_ctrl,
			      struct msm_eeprom_memory_block_t *block)
{
	int rc = 0;
	struct msm_eeprom_memory_map_t *emap = block->map;
	struct msm_eeprom_board_info *eb_info;
	uint8_t *memptr = block->mapdata;
	enum msm_camera_i2c_data_type data_type = MSM_CAMERA_I2C_BYTE_DATA;
	uint16_t OTP_Bank=0, OTP_Data=0;
	int i, j;
	uint16_t start_addr;
	uint16_t version;

	if (!e_ctrl) {
		pr_err("%s e_ctrl is NULL", __func__);
		return -EINVAL;
	}

	eb_info = e_ctrl->eboard_info;

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &sr544_init);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	rc = prepare_read_write_data(e_ctrl, 0x00, 0x20, MSM_CAMERA_I2C_BYTE_DATA, 0);
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
			&(e_ctrl->i2c_client), r_otp_rdata,
			&version, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) read failed\n", __func__, __LINE__);
		return rc;
	}
	pr_err("%s:(%d) cra_version(0x%02X)",__func__,__LINE__,version);

	rc = prepare_read_write_data(e_ctrl, 0x06, 0x80, MSM_CAMERA_I2C_BYTE_DATA, 0);
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
			&(e_ctrl->i2c_client), r_otp_rdata,
			&OTP_Bank, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) read failed\n", __func__, __LINE__);
		return rc;
	}

	pr_err("%s: read OTP_Bank: %d\n", __func__, OTP_Bank);

	switch(OTP_Bank) {
	case 0:
	case 1:
		start_addr = 0x0690;
		break;
	case 3:
		start_addr = 0x0EE0;
		break;
	case 7:
		start_addr = 0x1730;
		break;
	default:
		pr_err("%s: Bank error : Bank(%d)\n", __func__, OTP_Bank);
		return -EINVAL;
	}

	for (j = 0; j < block->num_map; j++) {
		if (emap[j].saddr.addr) {
			eb_info->i2c_slaveaddr = emap[j].saddr.addr;
			e_ctrl->i2c_client.cci_client->sid =
					eb_info->i2c_slaveaddr >> 1;
			pr_err("qcom,slave-addr = 0x%X\n",
				eb_info->i2c_slaveaddr);
		}
	}

	pr_err("%s:(%d) e_ctrl->cal_data.num_data : %x\n", __func__, __LINE__, e_ctrl->cal_data.num_data);

	if (e_ctrl->cal_data.num_data)
	{
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_addr_h,
				((start_addr & 0xFF00) >> 8), data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_addr_l,
				(start_addr & 0x00FF), data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_cmd,
				0x01, data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		for (i=0; i<e_ctrl->cal_data.num_data; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
					&(e_ctrl->i2c_client), r_otp_rdata,
					&OTP_Data, data_type);
			if (rc < 0) {
				pr_err("%s:(%d) read failed\n", __func__, __LINE__);
				return rc;
			}
			memptr[i] = OTP_Data;
		}
#ifdef EEPROM_MMAP_DEBUG
		printk("OTP data : ");
		for (i=0; i<e_ctrl->cal_data.num_data; i++) {
			printk("[%d:%x], ", i, memptr[i]);
			if(i%16 == 15)
				printk("\n");
		}
		printk("\n");
#endif

		// copy CRA information which is read from OTP 0x20 to cal data [0x50] address.
		memptr[0x50] = version & 0xFF;
		pr_err("%s: %d version = 0x%02X, memptr[0x50] = 0x%02X\n", __func__, __LINE__ , version, memptr[0x50]);

		memptr += e_ctrl->cal_data.num_data;
		pr_err("%s: %d   memptr after addition = %p\n", __func__, __LINE__ , memptr);
	}
	return rc;
}
#elif defined(CONFIG_SR552)
static int read_eeprom_memory(struct msm_eeprom_ctrl_t *e_ctrl,
			      struct msm_eeprom_memory_block_t *block)
{
	int rc = 0;
	struct msm_eeprom_memory_map_t *emap = block->map;
	struct msm_eeprom_board_info *eb_info;
	uint8_t *memptr = block->mapdata;
	enum msm_camera_i2c_data_type data_type = MSM_CAMERA_I2C_BYTE_DATA;
	uint16_t OTP_Bank=0, OTP_Data=0;
	int i, j;
	uint16_t start_addr;
	uint16_t version;

	if (!e_ctrl) {
		pr_err("%s e_ctrl is NULL", __func__);
		return -EINVAL;
	}

	eb_info = e_ctrl->eboard_info;

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
			&(e_ctrl->i2c_client), &sr552_init);
	if (rc < 0) {
		pr_err("%s:(%d) write failed\n", __func__, __LINE__);
		return rc;
	}

	rc = prepare_read_write_data(e_ctrl, 0x00, 0x20, MSM_CAMERA_I2C_BYTE_DATA, 0);//version
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
			&(e_ctrl->i2c_client), r_otp_rdata,
			&version, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) read failed\n", __func__, __LINE__);
		return rc;
	}
	pr_err("%s:(%d) cra_version(0x%02X)",__func__,__LINE__,version);

	rc = prepare_read_write_data(e_ctrl, 0x06, 0x80, MSM_CAMERA_I2C_BYTE_DATA, 0);//bank
	if (rc < 0) {
		pr_err("%s: failed to prepare read/write on otp\n", __func__);
		return rc;
	}

	rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
			&(e_ctrl->i2c_client), r_otp_rdata,
			&OTP_Bank, data_type);
	if (rc < 0) {
		pr_err("%s:(%d) read failed\n", __func__, __LINE__);
		return rc;
	}

	pr_err("%s: read OTP_Bank: %d\n", __func__, OTP_Bank);

	switch(OTP_Bank) {
	case 0:
	case 1:
		start_addr = 0x0690;
		break;
	case 3:
		start_addr = 0x0EE0;
		break;
	case 7:
		start_addr = 0x1730;
		break;
	default:
		pr_err("%s: Bank error : Bank(%d)\n", __func__, OTP_Bank);
		return -EINVAL;
	}

	for (j = 0; j < block->num_map; j++) {
		if (emap[j].saddr.addr) {
			eb_info->i2c_slaveaddr = emap[j].saddr.addr;
			e_ctrl->i2c_client.cci_client->sid =
					eb_info->i2c_slaveaddr >> 1;
			pr_err("qcom,slave-addr = 0x%X\n",
				eb_info->i2c_slaveaddr);
		}
	}

	pr_err("%s:(%d) e_ctrl->cal_data.num_data : %x\n", __func__, __LINE__, e_ctrl->cal_data.num_data);

	if (e_ctrl->cal_data.num_data)
	{
		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_addr_h,
				((start_addr & 0xFF00) >> 8), data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_addr_l,
				(start_addr & 0x00FF), data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write(
				&(e_ctrl->i2c_client), r_otp_cmd,
				0x01, data_type);
		if (rc < 0) {
			pr_err("%s:(%d) write failed\n", __func__, __LINE__);
			return rc;
		}

		for (i=0; i<e_ctrl->cal_data.num_data; i++) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
					&(e_ctrl->i2c_client), r_otp_rdata,
					&OTP_Data, data_type);
			if (rc < 0) {
				pr_err("%s:(%d) read failed\n", __func__, __LINE__);
				return rc;
			}
			memptr[i] = OTP_Data;
		}
#ifdef EEPROM_MMAP_DEBUG
		printk("OTP data : ");
		for (i=0; i<e_ctrl->cal_data.num_data; i++) {
			printk("[%d:%x], ", i, memptr[i]);
			if(i%16 == 15)
				printk("\n");
		}
		printk("\n");
#endif

		// copy CRA information which is read from OTP 0x20 to cal data [0x50] address.
		memptr[0x50] = version & 0xFF;
		pr_err("%s: %d version = 0x%02X, memptr[0x50] = 0x%02X\n", __func__, __LINE__ , version, memptr[0x50]);

		memptr += e_ctrl->cal_data.num_data;
		pr_err("%s: %d   memptr after addition = %p\n", __func__, __LINE__ , memptr);
	}
	return rc;
}
#else
static int read_eeprom_memory(struct msm_eeprom_ctrl_t *e_ctrl,
			      struct msm_eeprom_memory_block_t *block)
{
	int rc = 0;
	struct msm_eeprom_memory_map_t *emap = block->map;
	struct msm_eeprom_board_info *eb_info;
	uint8_t *memptr = block->mapdata;
	enum msm_camera_i2c_data_type data_type = MSM_CAMERA_I2C_BYTE_DATA;
	uint16_t OTP_Bank=0, OTP_Data=0;
	int i, j;
	uint8_t page;

	if (!e_ctrl) {
		pr_err("%s e_ctrl is NULL", __func__);
		return -EINVAL;
	}

	eb_info = e_ctrl->eboard_info;

	for (j = 0; j < block->num_map; j++) {
		if (emap[j].saddr.addr) {
			eb_info->i2c_slaveaddr = emap[j].saddr.addr;
			e_ctrl->i2c_client.cci_client->sid =
					eb_info->i2c_slaveaddr >> 1;
			pr_err("qcom,slave-addr = 0x%X\n",
				eb_info->i2c_slaveaddr);
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &init_read_otp);
		if (rc < 0) {
			pr_err("%s:(%d) init_read_otp failed\n", __func__, __LINE__);
			return rc;
		}

		rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
				&(e_ctrl->i2c_client), 0x0A04,
				&OTP_Bank, data_type);
		if (rc < 0) {
			pr_err("%s:(%d) read failed\n", __func__, __LINE__);
			return rc;
		}

		pr_err("%s: read OTP_Bank: %d\n", __func__, OTP_Bank);

		switch(OTP_Bank) {
		case 0:
		case 1:
			page = 2;
			break;
		case 3:
			page = 3;
			break;
		case 7:
			page = 4;
			break;
		case 0xF:
			page = 5;
			break;
		default:
			pr_err("%s: Bank error : Bank(%d)\n", __func__, OTP_Bank);
			return -EINVAL;
		}
		init_read_otp.reg_setting[1].reg_data = page;
		init_write_otp.reg_setting[7].reg_data = page;

		if (emap[j].mem.valid_size) {
			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
				&(e_ctrl->i2c_client), &init_read_otp);
			if (rc < 0) {
				pr_err("%s:(%d) init_read_otp failed\n", __func__, __LINE__);
				return rc;
			}

			for (i=0x0A08; i<=0x0A41; i++) {
				rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_read(
					&(e_ctrl->i2c_client), i,
					&OTP_Data, data_type);
				if (rc < 0) {
					pr_err("%s:(%d) read failed\n", __func__, __LINE__);
					return rc;
				}
				memptr[i-0x0A08] = OTP_Data;
			}

			rc = e_ctrl->i2c_client.i2c_func_tbl->i2c_write_table(
					&(e_ctrl->i2c_client), &finish_read_otp);
			if (rc < 0) {
				pr_err("%s:(%d) finish_read_otp failed\n", __func__, __LINE__);
				return rc;
			}
#ifdef EEPROM_MMAP_DEBUG
			for (i=0; i<emap[j].mem.valid_size; i+=8)
				pr_err("memptr[%d]: %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd",i,
					memptr[i],memptr[i+1],memptr[i+2],memptr[i+3],
					memptr[i+4],memptr[i+5],memptr[i+6],memptr[i+7]);
#endif
			memptr += emap[j].mem.valid_size;
			pr_err("%s: %d   memptr after addition = %p\n", __func__, __LINE__ , memptr);

		}
	}
	return rc;
}
#endif

/**
  * msm_eeprom_parse_memory_map() - parse memory map in device node
  * @of:	device node
  * @data:	memory block for output
  *
  * This functions parses @of to fill @data.  It allocates map itself, parses
  * the @of node, calculate total data length, and allocates required buffer.
  * It only fills the map, but does not perform actual reading.
  */
static int msm_eeprom_parse_memory_map(struct device_node *of,
				       struct msm_eeprom_memory_block_t *data)
{
	int i, rc = 0;
	char property[PROPERTY_MAXSIZE];
	uint32_t count = 6;
	struct msm_eeprom_memory_map_t *map;
	uint32_t total_size = 0;

	snprintf(property, PROPERTY_MAXSIZE, "qcom,num-blocks");
	rc = of_property_read_u32(of, property, &data->num_map);
	pr_err("%s::%d  %s %d\n", __func__, __LINE__, property, data->num_map);
	if (rc < 0) {
		pr_err("%s::%d failed rc %d\n", __func__,__LINE__,rc);
		return rc;
	}

	map = kzalloc((sizeof(*map) * data->num_map), GFP_KERNEL);
	if (!map) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		return -ENOMEM;
	}
	data->map = map;

	for (i = 0; i < data->num_map; i++) {
		pr_err("%s, %d: i = %d\n", __func__, __LINE__, i);

		snprintf(property, PROPERTY_MAXSIZE, "qcom,page%d", i);
		rc = of_property_read_u32_array(of, property,
				(uint32_t *) &map[i].page, count);
		if (rc < 0) {
			pr_err("%s: failed %d\n", __func__, __LINE__);
			goto ERROR;
		}

		snprintf(property, PROPERTY_MAXSIZE, "qcom,poll%d", i);
		rc = of_property_read_u32_array(of, property,
				(uint32_t *) &map[i].poll, count);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto ERROR;
		}

		snprintf(property, PROPERTY_MAXSIZE, "qcom,mem%d", i);
		rc = of_property_read_u32_array(of, property,
				(uint32_t *) &map[i].mem, count);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto ERROR;
		}
		data->num_data += map[i].mem.valid_size;
	}
	pr_err("%s::%d valid size = %d\n", __func__,__LINE__, data->num_data);

	total_size = 0;

#if defined(CONFIG_SR544) || defined(CONFIG_SR552)
	// if total-size is defined at dtsi file.
	// set num_data as total-size (refer dtsi file of GTA)
	snprintf(property, PROPERTY_MAXSIZE, "qcom,total-size");
	rc = of_property_read_u32(of, property, &total_size);
	pr_err("%s::%d  %s %d\n", __func__,__LINE__,property, total_size);

	// if "qcom,total-size" propoerty exists.
	if (rc >= 0) {
		pr_err("%s::%d set num_data as total-size in order to use same address at cal map(total : %d, valid : %d)\n",
			__func__,__LINE__, total_size, data->num_data);
		data->num_data = total_size;
	}
#endif

	data->mapdata = kzalloc(data->num_data, GFP_KERNEL);
	if (!data->mapdata) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto ERROR;
	}
	return rc;

ERROR:
	kfree(data->map);
	memset(data, 0, sizeof(*data));
	return rc;
}

static struct msm_cam_clk_info cam_8960_clk_info[] = {
	[SENSOR_CAM_MCLK] = {"cam_clk", 24000000},
};

static struct msm_cam_clk_info cam_8974_clk_info[] = {
	[SENSOR_CAM_MCLK] = {"cam_src_clk", 24000000},
	[SENSOR_CAM_CLK] = {"cam_clk", 0},
};

static struct v4l2_subdev_core_ops msm_eeprom_subdev_core_ops = {
	.ioctl = msm_eeprom_subdev_ioctl,
};

static struct v4l2_subdev_ops msm_eeprom_subdev_ops = {
	.core = &msm_eeprom_subdev_core_ops,
};

static int msm_eeprom_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int rc = 0;
	uint32_t temp = 0;
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_camera_power_ctrl_t *power_info = NULL;
	struct device_node *of_node = client->dev.of_node;
	int j;

	pr_err("%s E\n", __func__);

        if (!of_node) {
                pr_err("%s of_node NULL\n", __func__);
                return -EINVAL;
        }

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s i2c_check_functionality failed\n", __func__);
		goto probe_failure;
	}

	e_ctrl = kzalloc(sizeof(*e_ctrl), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto probe_failure;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;
	pr_err("%s client = 0x%p\n", __func__, client);

	//e_ctrl->eboard_info = (struct msm_eeprom_board_info *)(id->driver_data);
	e_ctrl->eboard_info = kzalloc(sizeof(
		struct msm_eeprom_board_info), GFP_KERNEL);
	if (!e_ctrl->eboard_info) {
		pr_err("%s:%d board info NULL\n", __func__, __LINE__);
		rc = -EINVAL;
		goto memdata_free;
	}

	rc = of_property_read_u32(of_node, "qcom,slave-addr", &temp);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		goto board_free;
	}

	rc = of_property_read_u32(of_node, "cell-index",
			&e_ctrl->subdev_id);
	pr_err("cell-index/subdev_id %d, rc %d\n", e_ctrl->subdev_id, rc);
	if (rc < 0) {
		pr_err("failed read, rc %d\n", rc);
		goto board_free;
	}

	power_info = &e_ctrl->eboard_info->power_info;
	e_ctrl->eboard_info->i2c_slaveaddr = temp;
	e_ctrl->i2c_client.client = client;
	e_ctrl->is_supported = 0;

	pr_err("%s:%d e_ctrl->eboard_info->i2c_slaveaddr = %d\n", __func__, __LINE__ , e_ctrl->eboard_info->i2c_slaveaddr);

	/* Set device type as I2C */
	e_ctrl->eeprom_device_type = MSM_CAMERA_I2C_DEVICE;
	e_ctrl->i2c_client.i2c_func_tbl = &msm_eeprom_qup_func_tbl;
	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;

	if (e_ctrl->eboard_info->i2c_slaveaddr != 0)
		e_ctrl->i2c_client.client->addr =
					e_ctrl->eboard_info->i2c_slaveaddr;
	power_info->clk_info = cam_8960_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8960_clk_info);
	power_info->dev = &client->dev;


	rc = of_property_read_string(of_node, "qcom,eeprom-name",
		&e_ctrl->eboard_info->eeprom_name);
	pr_err("%s qcom,eeprom-name %s, rc %d\n", __func__,
		e_ctrl->eboard_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}
	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc)
		goto board_free;
	rc = msm_eeprom_parse_memory_map(of_node, &e_ctrl->cal_data);
	if (rc < 0)
		pr_err("%s: no cal memory map\n", __func__);
	if (e_ctrl->cal_data.mapdata)
		map_data = e_ctrl->cal_data.mapdata;

	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto board_free;
	}
	rc = read_eeprom_memory(e_ctrl, &e_ctrl->cal_data);
	if (rc < 0) {
		pr_err("%s read_eeprom_memory failed\n", __func__);
		goto power_down;
	}
	for (j = 0; j < e_ctrl->cal_data.num_data; j++)
		CDBG("memory_data[%d] = 0x%X\n", j,
			e_ctrl->cal_data.mapdata[j]);

//	e_ctrl->is_supported |= msm_eeprom_match_crc(&e_ctrl->cal_data);

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto power_down;
	}

	if (0 > of_property_read_u32(of_node, "qcom,sensor-position", &temp)) {
		pr_err("%s:%d Fail position, Default sensor position\n", __func__, __LINE__);
		temp = 0;
	}
	pr_err("%s qcom,sensor-position %d\n", __func__,temp);

	/* Initialize sub device */
	v4l2_i2c_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->i2c_client.client,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.flags = temp;
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);
	e_ctrl->is_supported = 1;
	pr_err("%s success result=%d X\n", __func__, rc);
	return rc;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
board_free:
	if (e_ctrl->eboard_info)
		kfree(e_ctrl->eboard_info);
memdata_free:
	if (e_ctrl)
		kfree(e_ctrl);
probe_failure:
	pr_err("%s failed! rc = %d\n", __func__, rc);
	return rc;
}

static int msm_eeprom_i2c_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->cal_data.mapdata);
	kfree(e_ctrl->cal_data.map);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info);
	}
	kfree(e_ctrl);
	return 0;
}

#define msm_eeprom_spi_parse_cmd(spic, str, name, out, size)		\
	{								\
		if (of_property_read_u32_array(				\
			spic->spi_master->dev.of_node,			\
			str, out, size)) {				\
			return -EFAULT;					\
		} else {						\
			spic->cmd_tbl.name.opcode = out[0];		\
			spic->cmd_tbl.name.addr_len = out[1];		\
			spic->cmd_tbl.name.dummy_len = out[2];		\
		}							\
	}

static int msm_eeprom_spi_parse_of(struct msm_camera_spi_client *spic)
{
	int rc = -EFAULT;
	uint32_t tmp[3];
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,read", read, tmp, 3);
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,readseq", read_seq, tmp, 3);
	msm_eeprom_spi_parse_cmd(spic, "qcom,spiop,queryid", query_id, tmp, 3);

	rc = of_property_read_u32_array(spic->spi_master->dev.of_node,
					"qcom,eeprom-id", tmp, 2);
	if (rc) {
		pr_err("%s: Failed to get eeprom id\n", __func__);
		return rc;
	}
	spic->mfr_id = tmp[0];
	spic->device_id = tmp[1];

	return 0;
}

static int msm_eeprom_match_id(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc;
	struct msm_camera_i2c_client *client = &e_ctrl->i2c_client;
	uint8_t id[2];

	rc = msm_camera_spi_query_id(client, 0, &id[0], 2);
	if (rc < 0)
		return rc;
	pr_err("%s: read 0x%x 0x%x, check 0x%x 0x%x\n", __func__, id[0],
	     id[1], client->spi_client->mfr_id, client->spi_client->device_id);
	if (id[0] != client->spi_client->mfr_id
		    || id[1] != client->spi_client->device_id)
		return -ENODEV;

	return 0;
}

static int msm_eeprom_get_dt_data(struct msm_eeprom_ctrl_t *e_ctrl)
{
	int rc = 0, i = 0;
	struct msm_eeprom_board_info *eb_info;
	struct msm_camera_power_ctrl_t *power_info =
		&e_ctrl->eboard_info->power_info;
	struct device_node *of_node = NULL;
	struct msm_camera_gpio_conf *gconf = NULL;
	uint16_t gpio_array_size = 0;
	uint16_t *gpio_array = NULL;

	eb_info = e_ctrl->eboard_info;
	if (e_ctrl->eeprom_device_type == MSM_CAMERA_SPI_DEVICE)
		of_node = e_ctrl->i2c_client.
			spi_client->spi_master->dev.of_node;
	else if (e_ctrl->eeprom_device_type == MSM_CAMERA_PLATFORM_DEVICE)
		of_node = e_ctrl->pdev->dev.of_node;
        else if (e_ctrl->eeprom_device_type == MSM_CAMERA_I2C_DEVICE)
                of_node = e_ctrl->i2c_client.client->dev.of_node;
	rc = msm_camera_get_dt_vreg_data(of_node, &power_info->cam_vreg,
					     &power_info->num_vreg);
	if (rc < 0) {
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		return rc;
	}

	pr_err("msm_camera_get_dt_power_setting_data : %d\n", __LINE__);
	rc = msm_camera_get_dt_power_setting_data(of_node,
		power_info->cam_vreg, power_info->num_vreg,
		power_info);
	if (rc < 0) {
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		goto ERROR1;
	}

	power_info->gpio_conf = kzalloc(sizeof(struct msm_camera_gpio_conf),
					GFP_KERNEL);
	if (!power_info->gpio_conf) {
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		rc = -ENOMEM;
		goto ERROR2;
	}
	gconf = power_info->gpio_conf;
	gpio_array_size = of_gpio_count(of_node);
	pr_err("%s gpio count %d\n", __func__, gpio_array_size);

	if (gpio_array_size) {
		gpio_array = kzalloc(sizeof(uint16_t) * gpio_array_size,
			GFP_KERNEL);
		if (!gpio_array) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto ERROR3;
		}
		for (i = 0; i < gpio_array_size; i++) {
			gpio_array[i] = of_get_gpio(of_node, i);
			pr_err("%s gpio_array[%d] = %d\n", __func__, i,
				gpio_array[i]);
		}

		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		rc = msm_camera_get_dt_gpio_req_tbl(of_node, gconf,
			gpio_array, gpio_array_size);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto ERROR4;
		}

		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		rc = msm_camera_init_gpio_pin_tbl(of_node, gconf,
			gpio_array, gpio_array_size);
		if (rc < 0) {
			pr_err("%s failed %d\n", __func__, __LINE__);
			goto ERROR4;
		}
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
		kfree(gpio_array);
	}

	pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
	return rc;
ERROR4:
	pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
	kfree(gpio_array);
ERROR3:
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
	kfree(power_info->gpio_conf);
ERROR2:
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
	kfree(power_info->cam_vreg);
ERROR1:
		pr_err("msm_eeprom_get_dt_data: %d\n", __LINE__);
	kfree(power_info->power_setting);
	return rc;
}


static int msm_eeprom_cmm_dts(struct msm_eeprom_board_info *eb_info,
				struct device_node *of_node)
{
	int rc = 0;
	struct msm_eeprom_cmm_t *cmm_data = &eb_info->cmm_data;

	cmm_data->cmm_support =
		of_property_read_bool(of_node, "qcom,cmm-data-support");
	if (!cmm_data->cmm_support){
                pr_err("%s::%d qcom,cmm-data-support failed ",__func__,__LINE__);
		return -EINVAL;
        }
	cmm_data->cmm_compression =
		of_property_read_bool(of_node, "qcom,cmm-data-compressed");
	if (!cmm_data->cmm_compression)
		CDBG("No MM compression data\n");

	rc = of_property_read_u32(of_node, "qcom,cmm-data-offset",
				  &cmm_data->cmm_offset);
	if (rc < 0)
		CDBG("No MM offset data\n");

	rc = of_property_read_u32(of_node, "qcom,cmm-data-size",
				  &cmm_data->cmm_size);
	if (rc < 0)
		CDBG("No MM size data\n");

	CDBG("cmm_support: cmm_compr %d, cmm_offset %d, cmm_size %d\n",
		cmm_data->cmm_compression,
		cmm_data->cmm_offset,
		cmm_data->cmm_size);
	return 0;
}

static int msm_eeprom_spi_setup(struct spi_device *spi)
{
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_camera_i2c_client *client = NULL;
	struct msm_camera_spi_client *spi_client;
	struct msm_eeprom_board_info *eb_info;
	struct msm_camera_power_ctrl_t *power_info = NULL;
	int rc = 0;
	uint32_t temp = 0;

	e_ctrl = kzalloc(sizeof(*e_ctrl), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;
	client = &e_ctrl->i2c_client;
	e_ctrl->is_supported = 0;

	spi_client = kzalloc(sizeof(*spi_client), GFP_KERNEL);
	if (!spi_client) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		kfree(e_ctrl);
		return -ENOMEM;
	}

	rc = of_property_read_u32(spi->dev.of_node, "cell-index",
				  &e_ctrl->subdev_id);
	CDBG("cell-index/subdev_id %d, rc %d\n", e_ctrl->subdev_id, rc);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		goto spi_free;
	}

	e_ctrl->eeprom_device_type = MSM_CAMERA_SPI_DEVICE;
	client->spi_client = spi_client;
	spi_client->spi_master = spi;
	client->i2c_func_tbl = &msm_eeprom_spi_func_tbl;
	client->addr_type = MSM_CAMERA_I2C_3B_ADDR;

	eb_info = kzalloc(sizeof(*eb_info), GFP_KERNEL);
	if (!eb_info)
		goto spi_free;
	e_ctrl->eboard_info = eb_info;
	rc = of_property_read_string(spi->dev.of_node, "qcom,eeprom-name",
		&eb_info->eeprom_name);
	CDBG("%s qcom,eeprom-name %s, rc %d\n", __func__,
		eb_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}

	rc = msm_eeprom_cmm_dts(e_ctrl->eboard_info, spi->dev.of_node);
	if (rc < 0)
		CDBG("%s MM data miss:%d\n", __func__, __LINE__);

	power_info = &eb_info->power_info;

	power_info->clk_info = cam_8974_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8974_clk_info);
	power_info->dev = &spi->dev;

	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc < 0)
		goto board_free;

	/* set spi instruction info */
	spi_client->retry_delay = 1;
	spi_client->retries = 0;

	rc = msm_eeprom_spi_parse_of(spi_client);
	if (rc < 0) {
		dev_err(&spi->dev,
			"%s: Error parsing device properties\n", __func__);
		goto board_free;
	}

	/* prepare memory buffer */
	rc = msm_eeprom_parse_memory_map(spi->dev.of_node,
					 &e_ctrl->cal_data);
	if (rc < 0)
		CDBG("%s: no cal memory map\n", __func__);

	/* power up eeprom for reading */
	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		goto caldata_free;
	}

	/* check eeprom id */
	rc = msm_eeprom_match_id(e_ctrl);
	if (rc < 0) {
		CDBG("%s: eeprom not matching %d\n", __func__, rc);
		goto power_down;
	}
	/* read eeprom */
	if (e_ctrl->cal_data.map) {
		rc = read_eeprom_memory(e_ctrl, &e_ctrl->cal_data);
		if (rc < 0) {
			pr_err("%s: read cal data failed\n", __func__);
			goto power_down;
		}
		e_ctrl->is_supported |= msm_eeprom_match_crc(
						&e_ctrl->cal_data);
	}

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		goto caldata_free;
	}

	if (0 > of_property_read_u32(spi->dev.of_node, "qcom,sensor-position", &temp)) {
		pr_err("%s:%d Fail position, Default sensor position\n", __func__, __LINE__);
		temp = 0;
	}

	/* initiazlie subdev */
	v4l2_spi_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->i2c_client.spi_client->spi_master,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.flags = temp;
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);
	e_ctrl->is_supported = (e_ctrl->is_supported << 1) | 1;
	CDBG("%s success result=%d supported=%x X\n", __func__, rc,
	     e_ctrl->is_supported);

	return 0;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
caldata_free:
	kfree(e_ctrl->cal_data.mapdata);
	kfree(e_ctrl->cal_data.map);
board_free:
	kfree(e_ctrl->eboard_info);
spi_free:
	kfree(spi_client);
	kfree(e_ctrl);
	return rc;
}

static int msm_eeprom_spi_probe(struct spi_device *spi)
{
	int irq, cs, cpha, cpol, cs_high;

	pr_err("%s\n", __func__);
	spi->bits_per_word = 8;
	spi->mode = SPI_MODE_0;
	spi_setup(spi);

	irq = spi->irq;
	cs = spi->chip_select;
	cpha = (spi->mode & SPI_CPHA) ? 1 : 0;
	cpol = (spi->mode & SPI_CPOL) ? 1 : 0;
	cs_high = (spi->mode & SPI_CS_HIGH) ? 1 : 0;
	pr_err("%s: irq[%d] cs[%x] CPHA[%x] CPOL[%x] CS_HIGH[%x]\n",
			__func__, irq, cs, cpha, cpol, cs_high);
	pr_err("%s: max_speed[%u]\n", __func__, spi->max_speed_hz);

	return msm_eeprom_spi_setup(spi);
}

static int msm_eeprom_spi_remove(struct spi_device *sdev)
{
	struct v4l2_subdev *sd = spi_get_drvdata(sdev);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->i2c_client.spi_client);
	kfree(e_ctrl->cal_data.mapdata);
	kfree(e_ctrl->cal_data.map);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info);
	}
	kfree(e_ctrl);
	return 0;
}

static int msm_eeprom_platform_probe(struct platform_device *pdev)
{
	int rc = 0;
	int j = 0;
	uint32_t temp;

	struct msm_camera_cci_client *cci_client = NULL;
	struct msm_eeprom_ctrl_t *e_ctrl = NULL;
	struct msm_eeprom_board_info *eb_info = NULL;
	struct device_node *of_node = pdev->dev.of_node;
	struct msm_camera_power_ctrl_t *power_info = NULL;

	pr_err("%s E\n", __func__);

	e_ctrl = kzalloc(sizeof(*e_ctrl), GFP_KERNEL);
	if (!e_ctrl) {
		pr_err("%s:%d kzalloc failed\n", __func__, __LINE__);
		return -ENOMEM;
	}
	e_ctrl->eeprom_v4l2_subdev_ops = &msm_eeprom_subdev_ops;
	e_ctrl->eeprom_mutex = &msm_eeprom_mutex;

	e_ctrl->is_supported = 0;
	if (!of_node) {
		pr_err("%s dev.of_node NULL\n", __func__);
		kfree(e_ctrl);
		return -EINVAL;
	}

	rc = of_property_read_u32(of_node, "cell-index",
		&pdev->id);
	CDBG("cell-index %d, rc %d\n", pdev->id, rc);
	if (rc < 0) {
		pr_err("failed rc %d\n", rc);
		kfree(e_ctrl);
		return rc;
	}
	e_ctrl->subdev_id = pdev->id;

	rc = of_property_read_u32(of_node, "qcom,cci-master",
		&e_ctrl->cci_master);
	CDBG("qcom,cci-master %d, rc %d\n", e_ctrl->cci_master, rc);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		kfree(e_ctrl);
		return rc;
	}
	rc = of_property_read_u32(of_node, "qcom,slave-addr",
		&temp);
	if (rc < 0) {
		pr_err("%s failed rc %d\n", __func__, rc);
		kfree(e_ctrl);
		return rc;
	}

	/* Set platform device handle */
	e_ctrl->pdev = pdev;
	/* Set device type as platform device */
	e_ctrl->eeprom_device_type = MSM_CAMERA_PLATFORM_DEVICE;
	e_ctrl->i2c_client.i2c_func_tbl = &msm_eeprom_cci_func_tbl;
	e_ctrl->i2c_client.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	e_ctrl->i2c_client.cci_client = kzalloc(sizeof(
		struct msm_camera_cci_client), GFP_KERNEL);
	if (!e_ctrl->i2c_client.cci_client) {
		pr_err("%s failed no memory\n", __func__);
		kfree(e_ctrl);
		return -ENOMEM;
	}

	e_ctrl->eboard_info = kzalloc(sizeof(
		struct msm_eeprom_board_info), GFP_KERNEL);
	if (!e_ctrl->eboard_info) {
		pr_err("%s failed line %d\n", __func__, __LINE__);
		rc = -ENOMEM;
		goto cciclient_free;
	}
	eb_info = e_ctrl->eboard_info;
	power_info = &eb_info->power_info;
	eb_info->i2c_slaveaddr = temp;

	power_info->clk_info = cam_8974_clk_info;
	power_info->clk_info_size = ARRAY_SIZE(cam_8974_clk_info);
	power_info->dev = &pdev->dev;

	CDBG("qcom,slave-addr = 0x%X\n", eb_info->i2c_slaveaddr);
	cci_client = e_ctrl->i2c_client.cci_client;
	cci_client->cci_subdev = msm_cci_get_subdev();
	cci_client->cci_i2c_master = e_ctrl->cci_master;
	cci_client->sid = eb_info->i2c_slaveaddr >> 1;
	cci_client->retries = 3;
	cci_client->id_map = 0;
	cci_client->i2c_freq_mode = I2C_FAST_MODE;

	rc = of_property_read_string(of_node, "qcom,eeprom-name",
		&eb_info->eeprom_name);
	CDBG("%s qcom,eeprom-name %s, rc %d\n", __func__,
		eb_info->eeprom_name, rc);
	if (rc < 0) {
		pr_err("%s failed %d\n", __func__, __LINE__);
		goto board_free;
	}

	rc = msm_eeprom_cmm_dts(e_ctrl->eboard_info, of_node);
	if (rc < 0){
		pr_err("%s MM data miss:%d\n", __func__, __LINE__);
        }

	rc = msm_eeprom_get_dt_data(e_ctrl);
	if (rc)
		goto board_free;

	rc = msm_eeprom_parse_memory_map(of_node, &e_ctrl->cal_data);
	if (rc < 0)
		goto board_free;

	if (e_ctrl->cal_data.mapdata)
		map_data = e_ctrl->cal_data.mapdata;
	rc = msm_camera_power_up(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memdata_free;
	}
	rc = read_eeprom_memory(e_ctrl, &e_ctrl->cal_data);
	if (rc < 0) {
		pr_err("%s read_eeprom_memory failed\n", __func__);
		goto power_down;
	}
	for (j = 0; j < e_ctrl->cal_data.num_data; j++)
		CDBG("memory_data[%d] = 0x%X\n", j,
			e_ctrl->cal_data.mapdata[j]);

	e_ctrl->is_supported |= msm_eeprom_match_crc(&e_ctrl->cal_data);

	rc = msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
	if (rc) {
		pr_err("failed rc %d\n", rc);
		goto memdata_free;
	}

	if (0 > of_property_read_u32(of_node, "qcom,sensor-position", &temp)) {
		pr_err("%s:%d Fail position, Default sensor position\n", __func__, __LINE__);
		temp = 0;
	}
	pr_err("%s qcom,sensor-position %d\n", __func__,temp);

	v4l2_subdev_init(&e_ctrl->msm_sd.sd,
		e_ctrl->eeprom_v4l2_subdev_ops);
	v4l2_set_subdevdata(&e_ctrl->msm_sd.sd, e_ctrl);
	platform_set_drvdata(pdev, &e_ctrl->msm_sd.sd);
	e_ctrl->msm_sd.sd.internal_ops = &msm_eeprom_internal_ops;
	e_ctrl->msm_sd.sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	snprintf(e_ctrl->msm_sd.sd.name,
		ARRAY_SIZE(e_ctrl->msm_sd.sd.name), "msm_otp");
	media_entity_init(&e_ctrl->msm_sd.sd.entity, 0, NULL, 0);
	e_ctrl->msm_sd.sd.entity.flags = temp;
	e_ctrl->msm_sd.sd.entity.type = MEDIA_ENT_T_V4L2_SUBDEV;
	e_ctrl->msm_sd.sd.entity.group_id = MSM_CAMERA_SUBDEV_EEPROM;
	msm_sd_register(&e_ctrl->msm_sd);

	e_ctrl->is_supported = (e_ctrl->is_supported << 1) | 1;
	pr_err("%s X\n", __func__);
	return rc;

power_down:
	msm_camera_power_down(power_info, e_ctrl->eeprom_device_type,
		&e_ctrl->i2c_client);
memdata_free:
	kfree(e_ctrl->cal_data.mapdata);
	kfree(e_ctrl->cal_data.map);
board_free:
	kfree(e_ctrl->eboard_info);
cciclient_free:
	kfree(e_ctrl->i2c_client.cci_client);
	kfree(e_ctrl);
	return rc;
}

static int msm_eeprom_platform_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct msm_eeprom_ctrl_t  *e_ctrl;
	if (!sd) {
		pr_err("%s: Subdevice is NULL\n", __func__);
		return 0;
	}

	e_ctrl = (struct msm_eeprom_ctrl_t *)v4l2_get_subdevdata(sd);
	if (!e_ctrl) {
		pr_err("%s: eeprom device is NULL\n", __func__);
		return 0;
	}

	kfree(e_ctrl->i2c_client.cci_client);
	kfree(e_ctrl->cal_data.mapdata);
	kfree(e_ctrl->cal_data.map);
	if (e_ctrl->eboard_info) {
		kfree(e_ctrl->eboard_info->power_info.gpio_conf);
		kfree(e_ctrl->eboard_info);
	}
	kfree(e_ctrl);
	return 0;
}

static const struct of_device_id msm_eeprom_dt_match[] = {
	{ .compatible = "qcom,otp" },
	{ }
};

MODULE_DEVICE_TABLE(of, msm_eeprom_dt_match);

static struct platform_driver msm_eeprom_platform_driver = {
	.driver = {
		.name = "qcom,otp",
		.owner = THIS_MODULE,
		.of_match_table = msm_eeprom_dt_match,
	},
	.remove = msm_eeprom_platform_remove,
};

static const struct of_device_id msm_eeprom_i2c_dt_match[] = {
	{ .compatible = "qcom,otp"},
	{ }
};

MODULE_DEVICE_TABLE(of, msm_eeprom_i2c_dt_match);

static const struct i2c_device_id msm_eeprom_i2c_id[] = {
	{ "qcom,otp", (kernel_ulong_t)NULL},
	{ }
};

static struct i2c_driver msm_eeprom_i2c_driver = {
	.id_table = msm_eeprom_i2c_id,
	.probe  = msm_eeprom_i2c_probe,
	.remove = msm_eeprom_i2c_remove,
	.driver = {
		.name = "qcom,otp",
		.owner = THIS_MODULE,
		.of_match_table = msm_eeprom_i2c_dt_match,
	},
};

static struct spi_driver msm_eeprom_spi_driver = {
	.driver = {
		.name = "qcom_otp",
		.owner = THIS_MODULE,
		.of_match_table = msm_eeprom_dt_match,
	},
	.probe = msm_eeprom_spi_probe,
	.remove = msm_eeprom_spi_remove,
};

static int __init msm_eeprom_init_module(void)
{
        int rc = 0 /*, spi_rc = 0*/;
	pr_err("%s E\n", __func__);
	rc = platform_driver_probe(&msm_eeprom_platform_driver,
		msm_eeprom_platform_probe);
        if(rc<0){
	   pr_err("%s:%d platform rc %d\n", __func__, __LINE__, rc);
           return rc;
        }
	rc = i2c_add_driver(&msm_eeprom_i2c_driver);
        if (rc < 0)
        pr_err("%s:%d probe failed\n", __func__, __LINE__);
	return rc;
}

static void __exit msm_eeprom_exit_module(void)
{
	platform_driver_unregister(&msm_eeprom_platform_driver);
	spi_unregister_driver(&msm_eeprom_spi_driver);
	i2c_del_driver(&msm_eeprom_i2c_driver);
}

module_init(msm_eeprom_init_module);
module_exit(msm_eeprom_exit_module);
MODULE_DESCRIPTION("MSM EEPROM driver");
MODULE_LICENSE("GPL v2");
