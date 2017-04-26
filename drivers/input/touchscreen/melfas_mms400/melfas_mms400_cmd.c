/*
 * MELFAS MMS400 Touchscreen
 *
 * Copyright (C) 2014 MELFAS Inc.
 *
 *
 * Command Functions (Optional)
 *
 */

#include "melfas_mms400.h"

#if MMS_USE_CMD_MODE

#define NAME_OF_UNKNOWN_CMD "not_support_cmd"

enum CMD_STATUS {
	CMD_STATUS_WAITING = 0,
	CMD_STATUS_RUNNING,
	CMD_STATUS_OK,
	CMD_STATUS_FAIL,
	CMD_STATUS_NONE,
};

/**
 * Clear command result
 */
static void cmd_clear_result(struct mms_ts_info *info)
{
	char delim = ':';
	memset(info->cmd_result, 0x00, sizeof(u8) * 4096);
	memcpy(info->cmd_result, info->cmd, strnlen(info->cmd, CMD_LEN));
	strncat(info->cmd_result, &delim, 1);
}

/**
 * Set command result
 */
static void cmd_set_result(struct mms_ts_info *info, char *buf, int len)
{
	strncat(info->cmd_result, buf, len);
}

/**
 * Command : Update firmware
 */
static void cmd_fw_update(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int fw_location = info->cmd_param[0];

	cmd_clear_result(info);

	switch (fw_location) {
	case 0:
		if (mms_fw_update_from_kernel(info, true, false)) {
			goto ERROR;
		}
		break;
	case 1:
		if (mms_fw_update_from_storage(info, true)) {
			goto ERROR;
		}
		break;
	case 2:
		/* FFU - urgent fw update */
		if (mms_fw_update_from_kernel(info, true, true)) {
			goto ERROR;
		}
		break;
	default:
		goto ERROR;
		break;
	}

	sprintf(buf, "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;
	goto EXIT;

ERROR:
	sprintf(buf, "%s", "NG");
	info->cmd_state = CMD_STATUS_FAIL;
	goto EXIT;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get firmware version from MFSB file
 */
static void cmd_get_fw_ver_bin(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	const char *fw_name = INTERNAL_FW_PATH;
	const struct firmware *fw;
	struct mms_bin_hdr *fw_hdr;
	struct mms_fw_img **img;
	u8 ver_file[MMS_FW_MAX_SECT_NUM * 2];
	int i = 0;
	int offset = sizeof(struct mms_bin_hdr);

	cmd_clear_result(info);

	if (info->dtdata->fw_path)
		request_firmware(&fw, info->dtdata->fw_path, &info->client->dev);
	else
		request_firmware(&fw, fw_name, &info->client->dev);

	if (!fw) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	fw_hdr = (struct mms_bin_hdr *)fw->data;
	img = kzalloc(sizeof(*img) * fw_hdr->section_num, GFP_KERNEL);

	for (i = 0; i < fw_hdr->section_num; i++, offset += sizeof(struct mms_fw_img)) {
		img[i] = (struct mms_fw_img *)(fw->data + offset);
		ver_file[i * 2] = ((img[i]->version) >> 8) & 0xFF;
		ver_file[i * 2 + 1] = (img[i]->version) & 0xFF;
	}

	release_firmware(fw);

	sprintf(buf, "ME00%02X%02X\n", ver_file[3],ver_file[5]);
	info->cmd_state = CMD_STATUS_OK;

	kfree(img);

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get firmware version from IC
 */
static void cmd_get_fw_ver_ic(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 rbuf[64];

	cmd_clear_result(info);

	if (mms_get_fw_version(info, rbuf)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	info->boot_ver_ic = rbuf[1];
	info->core_ver_ic = rbuf[3];
	info->config_ver_ic = rbuf[5];

	sprintf(buf, "ME00%02X%02X\n", info->core_ver_ic, info->config_ver_ic);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get chip vendor
 */
static void cmd_get_chip_vendor(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	//u8 rbuf[64];

	cmd_clear_result(info);

	sprintf(buf, "MELFAS");
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get chip name
 */
static void cmd_get_chip_name(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	//u8 rbuf[64];

	cmd_clear_result(info);

	sprintf(buf, CHIP_NAME);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

static void cmd_get_config_ver(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	cmd_clear_result(info);
	sprintf(buf, "%s_ME_%02d%02d",
			info->product_name, info->fw_month, info->fw_date);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get X ch num
 */
static void cmd_get_x_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 rbuf[64];
	u8 wbuf[64];
	int val;

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_NODE_NUM_X;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 1)) {
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	val = rbuf[0];

	sprintf(buf, "%d", val);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

EXIT:
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get Y ch num
 */
static void cmd_get_y_num(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 rbuf[64];
	u8 wbuf[64];
	int val;

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_NODE_NUM_Y;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 1)) {
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	val = rbuf[0];

	sprintf(buf, "%d", val);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

EXIT:
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get X resolution
 */
static void cmd_get_max_x(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 rbuf[64];
	u8 wbuf[64];
	int val;

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_RESOLUTION_X;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 2)) {
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	//val = (rbuf[0] << 8) | rbuf[1];
	val = (rbuf[0]) | (rbuf[1] << 8);

	sprintf(buf, "%d", val);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

EXIT:
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get Y resolution
 */
static void cmd_get_max_y(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 rbuf[64];
	u8 wbuf[64];
	int val;

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_RESOLUTION_Y;
	if (mms_i2c_read(info, wbuf, 2, rbuf, 2)) {
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	//val = (rbuf[0] << 8) | rbuf[1];
	val = (rbuf[0]) | (rbuf[1] << 8);

	sprintf(buf, "%d", val);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	info->cmd_state = CMD_STATUS_OK;

EXIT:
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Power off
 */
static void cmd_module_off_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	cmd_clear_result(info);

	mms_power_control(info, 0);

	sprintf(buf, "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Power on
 */
static void cmd_module_on_master(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	cmd_clear_result(info);

	mms_power_control(info, 1);

	sprintf(buf, "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Read intensity image
 */
static void cmd_read_intensity(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int min = 999999;
	int max = -999999;
	int i = 0;

	cmd_clear_result(info);

	if (mms_get_image(info, MIP_IMG_TYPE_INTENSITY)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	for (i = 0; i < (info->node_x * info->node_y); i++) {
		if (info->image_buf[i] > max) {
			max = info->image_buf[i];
		}
		if (info->image_buf[i] < min) {
			min = info->image_buf[i];
		}
	}

	sprintf(buf, "%d,%d", min, max);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get intensity data
 */
static void cmd_get_intensity(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int x = info->cmd_param[0];
	int y = info->cmd_param[1];
	int idx = 0;

	cmd_clear_result(info);

	if ((x < 0) || (x >= info->node_x) || (y < 0) || (y >= info->node_y)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	idx = x + y * info->node_x;

	sprintf(buf, "%d", info->image_buf[idx]);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Read rawdata image
 */
static void cmd_read_rawdata(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int min = 999999;
	int max = -999999;
	int i = 0;

	cmd_clear_result(info);

	if (mms_get_image(info, MIP_IMG_TYPE_RAWDATA)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	for (i = 0; i < (info->node_x * info->node_y); i++) {
		if (info->image_buf[i] > max) {
			max = info->image_buf[i];
		}
		if (info->image_buf[i] < min) {
			min = info->image_buf[i];
		}
	}

	sprintf(buf, "%d,%d", min, max);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get rawdata
 */
static void cmd_get_rawdata(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int x = info->cmd_param[0];
	int y = info->cmd_param[1];
	int idx = 0;

	cmd_clear_result(info);

	if ((x < 0) || (x >= info->node_x) || (y < 0) || (y >= info->node_y)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	idx = x + y * info->node_x;

	sprintf(buf, "%d", info->image_buf[idx]);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Run cm delta test
 */
static void cmd_run_test_cm_delta(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int min = 999999;
	int max = -999999;
	int i = 0;

	cmd_clear_result(info);

	if (mms_run_test(info, MIP_TEST_TYPE_CM_DELTA)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	for (i = 0; i < (info->node_x * info->node_y); i++) {
		if (info->image_buf[i] > max) {
			max = info->image_buf[i];
		}
		if (info->image_buf[i] < min) {
			min = info->image_buf[i];
		}
	}

	sprintf(buf, "%d,%d", min, max);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get result of cm delta test
 */
static void cmd_get_cm_delta(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int x = info->cmd_param[0];
	int y = info->cmd_param[1];
	int idx = 0;

	cmd_clear_result(info);

	if ((x < 0) || (x >= info->node_x) || (y < 0) || (y >= info->node_y)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	idx = x + y * info->node_x;

	sprintf(buf, "%d", info->image_buf[idx]);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Run cm abs test
 */
static void cmd_run_test_cm_abs(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int min = 999999;
	int max = -999999;
	int i = 0;

	cmd_clear_result(info);

	if (mms_run_test(info, MIP_TEST_TYPE_CM_ABS)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	for (i = 0; i < (info->node_x * info->node_y); i++) {
		if (info->image_buf[i] > max) {
			max = info->image_buf[i];
		}
		if (info->image_buf[i] < min) {
			min = info->image_buf[i];
		}
	}

	sprintf(buf, "%d,%d", min, max);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

/**
 * Command : Get result of cm abs test
 */
static void cmd_get_cm_abs(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };

	int x = info->cmd_param[0];
	int y = info->cmd_param[1];
	int idx = 0;

	cmd_clear_result(info);

	if ((x < 0) || (x >= info->node_x) || (y < 0) || (y >= info->node_y)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	idx = x + y * info->node_x;

	sprintf(buf, "%d", info->image_buf[idx]);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

static void cmd_get_threshold(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 wbuf[4];
	u8 rbuf[4];

	cmd_clear_result(info);

	if (info->dtdata->thr_read_from_ic) {
		wbuf[0] = MIP_R0_INFO;
		wbuf[1] = MIP_R1_INFO_IC_CONTACT_ON_THD;
		if (mms_i2c_read(info, wbuf, 2, rbuf, 1)) {
			info->cmd_state = CMD_STATUS_FAIL;
			sprintf(buf, "%s", "NG");
			goto EXIT;
		}
		dev_info(&info->client->dev,
				"%s: read from IC, %d\n",
				__func__, rbuf[0]);
		sprintf(buf, "%d", rbuf[0]);
	} else {
		sprintf(buf, "55");
	}

	info->cmd_state = CMD_STATUS_OK;
EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

static void dead_zone_enable(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 wbuf[4];

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_CTRL;
	wbuf[1] = MIP_R1_CTRL_DISABLE_EDGE_EXPAND;
	wbuf[2] = 0;

	if (info->cmd_param[0] == 0) {
		wbuf[2] = 2;
	} else if (info->cmd_param[0] == 1) {
		wbuf[2] = 0;
	} else {
		sprintf(buf, "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		goto EXIT;
	}

	if (mms_i2c_write(info, wbuf, 3)) {
		dev_err(&info->client->dev, "%s [ERROR] mms_i2c_write\n", __func__);
		sprintf(buf, "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		sprintf(buf, "OK");
		info->cmd_state = CMD_STATUS_OK;
	}

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));


	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

static void get_checksum_data(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 wbuf[4];
	u8 rbuf[12];

	cmd_clear_result(info);

	wbuf[0] = MIP_R0_INFO;
	wbuf[1] = MIP_R1_INFO_BUILD_DATE;

	if (mms_i2c_read(info, wbuf, 2, rbuf, 12)) {
		info->cmd_state = CMD_STATUS_FAIL;
		sprintf(buf, "NG");
		goto EXIT;
	}
	snprintf(buf, sizeof(buf), "%02X%02X", rbuf[11], rbuf[10]);
	info->cmd_state = CMD_STATUS_OK;

EXIT:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));
	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}



#ifdef CONFIG_INPUT_BOOSTER
static void cmd_boost_level(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	struct i2c_client *client = info->client;
	int stage;
	char buf[64] = { 0 };

	cmd_clear_result(info);

	stage = 1 << info->cmd_param[0];

	dev_err(&client->dev, "%s,%d(%x)\n", __func__, info->cmd_param[0], stage);

	if (!(info->booster)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		dev_err(&info->client->dev," %s, booster is null \n", __func__);
		goto out;
	}

	if (!(info->booster->dvfs_stage & stage)) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
		dev_err(&info->client->dev,
				"%s: %d is not in supported stage[%x].\n",
				__func__, info->cmd_param[0], info->booster->dvfs_stage);
		goto out;
	}

	info->booster->dvfs_boost_mode = stage;
	input_booster_set_level_change(info->cmd_param[0]);
	sprintf(buf, "%s", "OK");
	info->cmd_state = CMD_STATUS_OK;

	if (info->booster->dvfs_boost_mode == DVFS_STAGE_NONE) {
		if (info->booster->dvfs_set)
			info->booster->dvfs_set(info->booster, -1);
	}

out:
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;
}
#endif

static void get_intensity_all_data(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	int ret;
	int length;

	cmd_clear_result(info);

	info->read_all_data = true;

	ret = mms_get_image(info, MIP_IMG_TYPE_INTENSITY);
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed to read intensity, %d\n", __func__, ret);
		sprintf(info->print_buf, "%s", "NG");
		goto out;
	}

	info->cmd_state = CMD_STATUS_OK;

	length = strlen(info->print_buf);
	dev_err(&info->client->dev, "%s: length is %d\n", __func__, length);

out:
	cmd_set_result(info, info->print_buf, length);

	info->read_all_data = false;

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;
}

static void get_rawdata_all_data(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	int ret;
	int length;

	cmd_clear_result(info);

	info->read_all_data = true;

	ret = mms_get_image(info, MIP_IMG_TYPE_RAWDATA);
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed to read raw data, %d\n", __func__, ret);
		sprintf(info->print_buf, "%s", "NG");
		goto out;
	}

	info->cmd_state = CMD_STATUS_OK;
	length = strlen(info->print_buf);
	dev_err(&info->client->dev, "%s: length is %d\n", __func__, length);

out:
	cmd_set_result(info, info->print_buf, length);

	info->read_all_data = false;

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;

}

static void get_cm_delta_all_data(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	int ret;
	int length;

	cmd_clear_result(info);

	info->read_all_data = true;

	ret = mms_run_test(info, MIP_TEST_TYPE_CM_DELTA);
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed to read cm delta, %d\n", __func__, ret);
		sprintf(info->print_buf, "%s", "NG");
		goto out;
	}

	info->cmd_state = CMD_STATUS_OK;
	length = strlen(info->print_buf);
	dev_err(&info->client->dev, "%s: length is %d\n", __func__, length);

out:
	cmd_set_result(info, info->print_buf, length);

	info->read_all_data = false;

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;


}

static void get_cm_abs_all_data(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	int ret;
	int length;

	cmd_clear_result(info);

	info->read_all_data = true;

	ret = mms_run_test(info, MIP_TEST_TYPE_CM_ABS);
	if (ret < 0) {
		dev_err(&info->client->dev, "%s: failed to read cm abs, %d\n", __func__, ret);
		sprintf(info->print_buf, "%s", "NG");
		goto out;
	}

	info->cmd_state = CMD_STATUS_OK;
	length = strlen(info->print_buf);
	dev_err(&info->client->dev, "%s: length is %d\n", __func__, length);

out:
	cmd_set_result(info, info->print_buf, length);

	info->read_all_data = false;

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;

}

/**
 * Command : Glove mode
 */
static void cmd_glove_mode(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[64] = { 0 };
	u8 val = info->cmd_param[0];
	int ret;

	cmd_clear_result(info);

	ret = mms_enable_glove_mode(info, val);
	if (ret < 0) {
		sprintf(buf, "%s", "NG");
		info->cmd_state = CMD_STATUS_FAIL;
	} else {
		sprintf(buf, "%s", "OK");
		info->cmd_state = CMD_STATUS_OK;
	}

	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;
}

/**
 * Command : Unknown cmd
 */
static void cmd_unknown_cmd(void *device_data)
{
	struct mms_ts_info *info = (struct mms_ts_info *)device_data;
	char buf[16] = { 0 };

	cmd_clear_result(info);

	snprintf(buf, sizeof(buf), "%s", NAME_OF_UNKNOWN_CMD);
	cmd_set_result(info, buf, strnlen(buf, sizeof(buf)));

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_NONE;

	dev_dbg(&info->client->dev, "%s - cmd[%s] len[%d] state[%d]\n",
			__func__, buf, strnlen(buf, sizeof(buf)), info->cmd_state);
}

#define MMS_CMD(name, func)	.cmd_name = name, .cmd_func = func

/**
 * Info of command function
 */
struct mms_cmd {
	struct list_head list;
	const char *cmd_name;
	void (*cmd_func) (void *device_data);
};

/**
 * List of command functions
 */
static struct mms_cmd mms_commands[] = {
	{MMS_CMD("fw_update", cmd_fw_update),},
	{MMS_CMD("get_fw_ver_bin", cmd_get_fw_ver_bin),},
	{MMS_CMD("get_fw_ver_ic", cmd_get_fw_ver_ic),},
	{MMS_CMD("get_chip_vendor", cmd_get_chip_vendor),},
	{MMS_CMD("get_chip_name", cmd_get_chip_name),},
	{MMS_CMD("get_x_num", cmd_get_x_num),},
	{MMS_CMD("get_y_num", cmd_get_y_num),},
	{MMS_CMD("get_max_x", cmd_get_max_x),},
	{MMS_CMD("get_max_y", cmd_get_max_y),},
	{MMS_CMD("module_off_master", cmd_module_off_master),},
	{MMS_CMD("module_on_master", cmd_module_on_master),},
	{MMS_CMD("run_intensity_read", cmd_read_intensity),},
	{MMS_CMD("get_intensity", cmd_get_intensity),},
	{MMS_CMD("run_rawdata_read", cmd_read_rawdata),},
	{MMS_CMD("get_rawdata", cmd_get_rawdata),},
	{MMS_CMD("run_inspection_read", cmd_run_test_cm_delta),},
	{MMS_CMD("get_inspection", cmd_get_cm_delta),},
	{MMS_CMD("run_cm_delta_read", cmd_run_test_cm_delta),},
	{MMS_CMD("get_cm_delta", cmd_get_cm_delta),},
	{MMS_CMD("run_cm_abs_read", cmd_run_test_cm_abs),},
	{MMS_CMD("get_cm_abs", cmd_get_cm_abs),},
	{MMS_CMD("get_config_ver", cmd_get_config_ver),},
	{MMS_CMD("get_threshold", cmd_get_threshold),},
#ifdef CONFIG_INPUT_BOOSTER
	{MMS_CMD("boost_level", cmd_boost_level),},
#endif
	{MMS_CMD("get_intensity_all_data", get_intensity_all_data),},
	{MMS_CMD("get_rawdata_all_data", get_rawdata_all_data),},
	{MMS_CMD("get_cm_delta_all_data", get_cm_delta_all_data),},
	{MMS_CMD("get_cm_abs_all_data", get_cm_abs_all_data),},
	{MMS_CMD("module_off_slave", cmd_unknown_cmd),},
	{MMS_CMD("module_on_slave", cmd_unknown_cmd),},
	{MMS_CMD("dead_zone_enable", dead_zone_enable),},
	{MMS_CMD("get_checksum_data", get_checksum_data),},
	{MMS_CMD("glove_mode", cmd_glove_mode),},
	{MMS_CMD(NAME_OF_UNKNOWN_CMD, cmd_unknown_cmd),},
};

/**
 * Sysfs - recv command
 */
static ssize_t mms_sys_cmd(struct device *dev, struct device_attribute *devattr,
		const char *buf, size_t count)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	int ret;
	char *cur, *start, *end;
	char cbuf[CMD_LEN] = { 0 };
	int len, i;
	struct mms_cmd *mms_cmd_ptr = NULL;
	char delim = ',';
	bool cmd_found = false;
	int param_cnt = 0;

	if (!info) {
		pr_err("%s [ERROR] mms_ts_info not found\n", __func__);
		ret = -EINVAL;
		goto ERROR;
	}

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);
	dev_dbg(&info->client->dev, "%s - input [%s]\n", __func__, buf);

	if (!info->input_dev) {
		dev_err(&info->client->dev,
				"%s [ERROR] input_dev not found\n", __func__);
		ret = -EINVAL;
		goto ERROR;
	}

	if (info->cmd_busy == true) {
		dev_err(&info->client->dev,
				"%s [ERROR] previous command is not ended\n", __func__);
		ret = -1;
		goto ERROR;
	}

	mutex_lock(&info->lock);
	info->cmd_busy = true;
	mutex_unlock(&info->lock);

	info->cmd_state = 1;
	for (i = 0; i < ARRAY_SIZE(info->cmd_param); i++) {
		info->cmd_param[i] = 0;
	}

	len = (int)count;
	if (*(buf + len - 1) == '\n') {
		len--;
	}

	memset(info->cmd, 0x00, ARRAY_SIZE(info->cmd));
	memcpy(info->cmd, buf, len);
	cur = strchr(buf, (int)delim);
	if (cur)
		memcpy(cbuf, buf, cur - buf);
	else
		memcpy(cbuf, buf, len);

	dev_dbg(&info->client->dev, "%s - command [%s]\n", __func__, cbuf);

	//command
	list_for_each_entry(mms_cmd_ptr, &info->cmd_list_head, list) {
		if (!strncmp(cbuf, mms_cmd_ptr->cmd_name, CMD_LEN)) {
			cmd_found = true;
			break;
		}
	}
	if (!cmd_found) {
		list_for_each_entry(mms_cmd_ptr, &info->cmd_list_head, list) {
			if (!strncmp(NAME_OF_UNKNOWN_CMD, mms_cmd_ptr->cmd_name, CMD_LEN)) {
				break;
			}
		}
	}

	//parameter
	if (cur && cmd_found) {
		cur++;
		start = cur;
		memset(cbuf, 0x00, ARRAY_SIZE(cbuf));

		do {
			if (*cur == delim || cur - buf == len) {
				end = cur;
				memcpy(cbuf, start, end - start);
				*(cbuf + strnlen(cbuf, ARRAY_SIZE(cbuf))) = '\0';
				if (kstrtoint(cbuf, 10, info->cmd_param + param_cnt) < 0) {
					goto ERROR;
				}
				start = cur + 1;
				memset(cbuf, 0x00, ARRAY_SIZE(cbuf));
				param_cnt++;
			}
			cur++;
		} while (cur - buf <= len);
	}

	//print
	dev_dbg(&info->client->dev, "%s - cmd [%s]\n", __func__, mms_cmd_ptr->cmd_name);
	for (i = 0; i < param_cnt; i++) {
		dev_dbg(&info->client->dev,
				"%s - param #%d [%d]\n", __func__, i, info->cmd_param[i]);
	}

	//execute
	mms_cmd_ptr->cmd_func(info);

	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);
	return count;

ERROR:
	pr_err("%s [ERROR]\n", __func__);
	return count;
}
static DEVICE_ATTR(cmd, 0666, NULL, mms_sys_cmd);

/**
 * Sysfs - print command status
 */
static ssize_t mms_sys_cmd_status(struct device *dev,
		struct device_attribute *devattr,char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	int ret;
	char cbuf[32] = {0};

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	dev_dbg(&info->client->dev, "%s - status [%d]\n", __func__, info->cmd_state);

	if (info->cmd_state == CMD_STATUS_WAITING) {
		snprintf(cbuf, sizeof(cbuf), "WAITING");
	} else if (info->cmd_state == CMD_STATUS_RUNNING) {
		snprintf(cbuf, sizeof(cbuf), "RUNNING");
	} else if (info->cmd_state == CMD_STATUS_OK) {
		snprintf(cbuf, sizeof(cbuf), "OK");
	} else if (info->cmd_state == CMD_STATUS_FAIL) {
		snprintf(cbuf, sizeof(cbuf), "FAIL");
	} else if (info->cmd_state == CMD_STATUS_NONE) {
		snprintf(cbuf, sizeof(cbuf), "NOT_APPLICABLE");
	}

	ret = snprintf(buf, PAGE_SIZE, "%s\n", cbuf);
	//memset(info->print_buf, 0, 4096);

	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);

	return ret;
}
static DEVICE_ATTR(cmd_status, 0666, mms_sys_cmd_status, NULL);

/**
 * Sysfs - print command result
 */
static ssize_t mms_sys_cmd_result(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	int ret;

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	dev_dbg(&info->client->dev, "%s - result [%s]\n", __func__, info->cmd_result);

	mutex_lock(&info->lock);
	info->cmd_busy = false;
	mutex_unlock(&info->lock);

	info->cmd_state = CMD_STATUS_WAITING;

	//EXIT:
	ret = snprintf(buf, PAGE_SIZE, "%s\n", info->cmd_result);
	//memset(info->print_buf, 0, 4096);

	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);

	return ret;
}
static DEVICE_ATTR(cmd_result, 0666, mms_sys_cmd_result, NULL);

/**
 * Sysfs - print command list
 */
static ssize_t mms_sys_cmd_list(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mms_ts_info *info = dev_get_drvdata(dev);
	int ret;
	int i = 0;
	char buffer[info->cmd_buffer_size];
	char buffer_name[CMD_LEN];

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	snprintf(buffer, 30, "== Command list ==\n");
	while (strncmp(mms_commands[i].cmd_name, NAME_OF_UNKNOWN_CMD, CMD_LEN) != 0) {
		snprintf(buffer_name, CMD_LEN, "%s\n", mms_commands[i].cmd_name);
		strcat(buffer, buffer_name);
		i++;
	}

	dev_dbg(&info->client->dev, "%s - length [%u / %d]\n",
			__func__, strlen(buffer), info->cmd_buffer_size);

	ret = snprintf(buf, PAGE_SIZE, "%s\n", buffer);
	//memset(info->print_buf, 0, 4096);

	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);

	return ret;
}
static DEVICE_ATTR(cmd_list, 0666, mms_sys_cmd_list, NULL);

/**
 * Sysfs - cmd attr info
 */
static struct attribute *mms_cmd_attr[] = {
	&dev_attr_cmd.attr,
	&dev_attr_cmd_status.attr,
	&dev_attr_cmd_result.attr,
	&dev_attr_cmd_list.attr,
	NULL,
};

/**
 * Sysfs - cmd attr group info
 */
static const struct attribute_group mms_cmd_attr_group = {
	.attrs = mms_cmd_attr,
};

/**
 * Create sysfs command functions
 */
int mms_sysfs_cmd_create(struct mms_ts_info *info)
{
	struct i2c_client *client = info->client;
	int i = 0;

	info->cmd_result = kzalloc(sizeof(u8) * 4096, GFP_KERNEL);
	//init cmd list
	INIT_LIST_HEAD(&info->cmd_list_head);
	info->cmd_buffer_size = 0;

	for (i = 0; i < ARRAY_SIZE(mms_commands); i++) {
		list_add_tail(&mms_commands[i].list, &info->cmd_list_head);
		if (mms_commands[i].cmd_name) {
			info->cmd_buffer_size += strlen(mms_commands[i].cmd_name) + 1;
		}
	}

	info->cmd_busy = false;
	info->print_buf = kzalloc(sizeof(u8) * 4096, GFP_KERNEL);

	//create sysfs
	if (sysfs_create_group(&client->dev.kobj, &mms_cmd_attr_group)) {
		dev_err(&client->dev, "%s [ERROR] sysfs_create_group\n", __func__);
		return -EAGAIN;
	}

	//create class
	//info->cmd_class = class_create(THIS_MODULE, "melfas");
	//info->cmd_dev = device_create(info->cmd_class, NULL, info->cmd_dev_t, NULL, "touchscreen");
	info->cmd_dev = device_create(sec_class, NULL, 0, info, "tsp");
	if (sysfs_create_group(&info->cmd_dev->kobj, &mms_cmd_attr_group)) {
		dev_err(&client->dev, "%s [ERROR] sysfs_create_group\n", __func__);
		return -EAGAIN;
	}

	if (sysfs_create_link(&info->cmd_dev->kobj, &info->input_dev->dev.kobj, "input")) {
		dev_err(&client->dev, "%s [ERROR] sysfs_create_link\n", __func__);
		return -EAGAIN;
	}

	return 0;
}

/**
 * Remove sysfs command functions
 */
void mms_sysfs_cmd_remove(struct mms_ts_info *info)
{
	sysfs_remove_group(&info->client->dev.kobj, &mms_cmd_attr_group);

	kfree(info->cmd_result);
	kfree(info->print_buf);

	return;
}

#endif
