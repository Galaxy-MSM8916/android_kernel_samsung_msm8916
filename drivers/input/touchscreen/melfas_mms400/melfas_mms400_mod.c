/*
 * MELFAS MMS400 Touchscreen
 *
 * Copyright (C) 2014 MELFAS Inc.
 *
 *
 * Model dependent functions
 *
 */

#include "melfas_mms400.h"

#ifdef USE_TSP_TA_CALLBACKS
static bool ta_connected = 0;
#endif

/**
 * Control power supply
 */
int mms_power_control(struct mms_ts_info *info, int enable)
{
	int ret;
	struct i2c_client *client = info->client;

	if (gpio_is_valid(info->dtdata->gpio_vdd))
		gpio_direction_output(info->dtdata->gpio_vdd, enable);

	if (gpio_is_valid(info->dtdata->gpio_vio)) {
		/* using gpio en */
		gpio_direction_output(info->dtdata->gpio_vio, enable);
	} else if (!IS_ERR_OR_NULL(info->dtdata->vreg_vio)) {
		/* using regulator */
		if (enable) {
			ret = regulator_enable(info->dtdata->vreg_vio);
			if (ret)
				dev_err(&client->dev,
						"%s [ERROR] regulator_enable [%d]\n",
						__func__, ret);
		} else {
			ret = regulator_disable(info->dtdata->vreg_vio);
			if (ret)
				dev_err(&client->dev,
						"%s [ERROR] regulator_disable [%d]\n",
						__func__, ret);
		}
	}

	if (!enable)
		usleep_range(10 * 1000, 11 * 1000);
	else
		msleep(50);

	pr_info("%s: %s", __func__, enable? "on":"off");
	if (gpio_is_valid(info->dtdata->gpio_vdd))
		pr_cont(" vdd:%d", gpio_get_value(info->dtdata->gpio_vdd));
	if (gpio_is_valid(info->dtdata->gpio_vio))
		pr_cont(" vio:%d", gpio_get_value(info->dtdata->gpio_vio));
	else if (!IS_ERR_OR_NULL(info->dtdata->vreg_vio))
		pr_cont(" vio_reg:%d",
				regulator_is_enabled(info->dtdata->vreg_vio));
	pr_cont("\n");

	return 0;
}

/**
 * Clear touch input events
 */
void mms_clear_input(struct mms_ts_info *info)
{
	int i;

	input_report_key(info->input_dev, BTN_TOUCH, 0);
	input_report_key(info->input_dev, BTN_TOOL_FINGER, 0);

	for (i = 0; i< MAX_FINGER_NUM; i++) {
		info->finger_state[i] = 0;
		input_mt_slot(info->input_dev, i);
		input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, false);
	}

	info->touch_count = 0;

	input_sync(info->input_dev);

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, -1);
#endif
}

/**
 * Input event handler - Report touch input event
 */
void mms_input_event_handler(struct mms_ts_info *info, u8 sz, u8 *buf)
{
	struct i2c_client *client = info->client;
	int i;

	dev_dbg(&client->dev, "%s [START]\n", __func__);
	dev_dbg(&client->dev, "%s - sz[%d] buf[0x%02X]\n", __func__, sz, buf[0]);

	for (i = 1; i < sz; i += info->event_size) {
		u8 *tmp = &buf[i];

		int id = (tmp[0] & MIP_EVENT_INPUT_ID) - 1;
		int x = tmp[2] | ((tmp[1] & 0xf) << 8);
		int y = tmp[3] | (((tmp[1] >> 4) & 0xf) << 8);

		/* old protocal		int touch_major = tmp[4];
		   int pressure = tmp[5];  */

		int pressure = tmp[4];
		//int size = tmp[5];		// sumsize
		int touch_major = tmp[6];
		int touch_minor = tmp[7];

		int palm = (tmp[0] & MIP_EVENT_INPUT_PALM) >> 4;

#ifdef CONFIG_SEC_FACTORY
#ifdef APPLY_RESOLUTION
		if (!info->dtdata->fix_resolution && info->config_ver_ic <= 0x10) {
			x *= 2;
			y *= 2;
		}
#endif
#endif

		// Report input data
#if MMS_USE_TOUCHKEY
		if ((tmp[0] & MIP_EVENT_INPUT_SCREEN) == 0) {
			//Touchkey Event
			int key = tmp[0] & 0xf;
			int key_state = (tmp[0] & MIP_EVENT_INPUT_PRESS) ? 1 : 0;
			int key_code = 0;

			//Report touchkey event
			switch (key) {
			case 1:
				key_code = KEY_MENU;
				//dev_dbg(&client->dev, "Key : KEY_MENU\n");
				break;
			case 2:
				key_code = KEY_BACK;
				//dev_dbg(&client->dev, "Key : KEY_BACK\n");
				break;
			default:
				dev_err(&client->dev,
						"%s [ERROR] Unknown key code [%d]\n",
						__func__, key);
				continue;
				break;
			}

			input_report_key(info->input_dev, key_code, key_state);
			input_sync(info->input_dev);

			dev_dbg(&client->dev, "%s - Key : ID[%d] Code[%d] State[%d]\n",
					__func__, key, key_code, key_state);
		} else
#endif
		{
			//Report touchscreen event
			if ((tmp[0] & MIP_EVENT_INPUT_PRESS) == 0) {
				//Release
				input_mt_slot(info->input_dev, id);
#ifdef CONFIG_SEC_FACTORY
				input_report_abs(info->input_dev, ABS_MT_PRESSURE, 0);
#endif
				input_mt_report_slot_state(info->input_dev,
						MT_TOOL_FINGER, false);
				if (info->finger_state[id] != 0) {
					info->touch_count--;
					if (!info->touch_count) {
						input_report_key(info->input_dev, BTN_TOUCH, 0);
						input_report_key(info->input_dev,
								BTN_TOOL_FINGER, 0);
					}
					info->finger_state[id] = 0;
				}
				input_sync(info->input_dev);

				dev_err(&client->dev,
						"R[%d] V[%02x%02x%02x] tc:%d g:%d\n",
						id, info->boot_ver_ic, info->core_ver_ic,
						info->config_ver_ic, info->touch_count,
						info->glove_mode);
				continue;
			}

			//Press or Move
			input_mt_slot(info->input_dev, id);
			input_mt_report_slot_state(info->input_dev, MT_TOOL_FINGER, true);
			input_report_key(info->input_dev, BTN_TOUCH, 1);
			input_report_key(info->input_dev, BTN_TOOL_FINGER, 1);
			input_report_abs(info->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(info->input_dev, ABS_MT_POSITION_Y, y);
#ifdef CONFIG_SEC_FACTORY
			if (pressure)
				input_report_abs(info->input_dev, ABS_MT_PRESSURE, pressure);
			else
				input_report_abs(info->input_dev, ABS_MT_PRESSURE, 1);
#endif
			input_report_abs(info->input_dev, ABS_MT_TOUCH_MAJOR, touch_major);
			input_report_abs(info->input_dev, ABS_MT_TOUCH_MINOR, touch_minor);
			input_report_abs(info->input_dev, ABS_MT_PALM, palm);

			input_sync(info->input_dev);

			if (info->finger_state[id] == 0) {
				info->finger_state[id] = 1;
				info->touch_count++;
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
				dev_info(&client->dev,
						"P[%d] z:%d p:%d m:%d,%d tc:%d g:%d\n",
						id, pressure, palm, touch_major, touch_minor,
						info->touch_count, info->glove_mode);
#else
				dev_err(&client->dev,
						"P[%d] (%d, %d) z:%d p:%d m:%d,%d tc:%d g:%d\n",
						id, x, y, pressure, palm, touch_major,
						touch_minor, info->touch_count, info->glove_mode);
#endif
			}
		}
	}

#ifdef CONFIG_INPUT_BOOSTER
	if (info->booster && info->booster->dvfs_set)
		info->booster->dvfs_set(info->booster, info->touch_count);
#endif


	dev_dbg(&client->dev, "%s [DONE]\n", __func__);
	return;
}

int mms_pinctrl_configure(struct mms_ts_info *info, int active)
{
	struct pinctrl_state *set_state_i2c;
	int retval;

	dev_dbg(&info->client->dev, "%s: %d\n", __func__, active);

	if (active) {
		set_state_i2c =	pinctrl_lookup_state(info->pinctrl, "tsp_gpio_active");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev,
					"%s: cannot get pinctrl(i2c) active state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}
	} else {
		set_state_i2c =	pinctrl_lookup_state(info->pinctrl, "tsp_gpio_suspend");
		if (IS_ERR(set_state_i2c)) {
			dev_err(&info->client->dev,
					"%s: cannot get pinctrl(i2c) suspend state\n", __func__);
			return PTR_ERR(set_state_i2c);
		}
	}
	retval = pinctrl_select_state(info->pinctrl, set_state_i2c);
	if (retval) {
		dev_err(&info->client->dev,
				"%s: cannot set pinctrl(i2c) %d state\n", __func__, active);
		return retval;
	}

	return 0;
}


#if MMS_USE_DEVICETREE
/**
 * Parse device tree
 */
int mms_parse_devicetree(struct device *dev, struct mms_ts_info *info)
{
	struct device_node *np = dev->of_node;

	dev_dbg(dev, "%s [START]\n", __func__);

	info->dtdata->gpio_intr = of_get_named_gpio(np, "melfas,irq-gpio", 0);
	gpio_request(info->dtdata->gpio_intr, "irq-gpio");
	gpio_direction_input(info->dtdata->gpio_intr);
	info->client->irq = gpio_to_irq(info->dtdata->gpio_intr);

	info->dtdata->gpio_scl = of_get_named_gpio(np, "melfas,scl-gpio", 0);
	gpio_request(info->dtdata->gpio_scl, "melfas_scl_gpio");
	info->dtdata->gpio_sda = of_get_named_gpio(np, "melfas,sda-gpio", 0);
	gpio_request(info->dtdata->gpio_sda, "melfas_sda_gpio");

	info->dtdata->gpio_vdd = of_get_named_gpio(np, "melfas,vdd_en", 0);
	if (gpio_is_valid(info->dtdata->gpio_vdd))
		gpio_request(info->dtdata->gpio_vdd, "melfas_vdd_gpio");

	info->dtdata->gpio_vio = of_get_named_gpio(np, "melfas,io_en", 0);
	if (gpio_is_valid(info->dtdata->gpio_vio)) {
		gpio_request(info->dtdata->gpio_vio, "melfas_io_gpio");
	} else {
		info->dtdata->vreg_vio = regulator_get(dev, "vdd-io");
		if (IS_ERR(info->dtdata->vreg_vio))
			dev_err(dev, "%s: vdd-io get failed\n", __func__);
	}

	of_property_read_string(np, "melfas,fw_path", &info->dtdata->fw_path);

	info->dtdata->fix_resolution =
		of_property_read_bool(np, "melfas,fix_resolution");
	info->dtdata->support_glove_mode =
		of_property_read_bool(np, "melfas,support-glove-mode");
	info->dtdata->thr_read_from_ic =
		of_property_read_bool(np, "melfas,thd-read");

	dev_info(dev, "%s: int:%d irq:%d sda:%d scl:%d vdd:%d gpio_vio:%d fw_path:%s\n"
			"%s: fix_resolution:%d support_glove_mode:%d\n",
			__func__, info->dtdata->gpio_intr, info->client->irq,
			info->dtdata->gpio_sda, info->dtdata->gpio_scl,
			info->dtdata->gpio_vdd, info->dtdata->gpio_vio,
			info->dtdata->fw_path,
			__func__, info->dtdata->fix_resolution,
			info->dtdata->support_glove_mode);

	return 0;
}
#endif

/**
 * Config input interface
 */
void mms_config_input(struct mms_ts_info *info)
{
	struct input_dev *input_dev = info->input_dev;

	dev_dbg(&info->client->dev, "%s [START]\n", __func__);

	//Screen
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);

	set_bit(BTN_TOUCH, input_dev->keybit);
	set_bit(BTN_TOOL_FINGER, input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_mt_init_slots(input_dev, MAX_FINGER_NUM, INPUT_MT_DIRECT);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, info->max_x -1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, info->max_y -1, 0, 0);
#ifdef CONFIG_SEC_FACTORY
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, INPUT_PRESSURE_MAX, 0, 0);
#endif
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, INPUT_TOUCH_MAJOR_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, INPUT_TOUCH_MINOR_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PALM, 0, 1, 0, 0);

	//Key
	set_bit(EV_KEY, input_dev->evbit);
#if MMS_USE_TOUCHKEY
	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
#endif
#if MMS_USE_NAP_MODE
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(KEY_POWER, input_dev->keybit);
#endif
	dev_dbg(&info->client->dev, "%s [DONE]\n", __func__);
	return;
}

/**
 * Callback - get charger status
 */
#ifdef USE_TSP_TA_CALLBACKS
void mms_charger_status_cb(struct tsp_callbacks *cb, int status)
{
	pr_err("%s: TA %s\n",
			__func__, status ? "connected" : "disconnected");

	if (status)
		ta_connected = true;
	else
		ta_connected = false;

	/* not yet defined functions */
}

void mms_register_callback(struct tsp_callbacks *cb)
{
	charger_callbacks = cb;
	pr_info("%s\n", __func__);
}
#endif

#ifdef CHARGER_NOTIFIER
static struct extcon_tsp_ta_cable support_cable_list[] = {
	{ .cable_type = EXTCON_USB, },
	{ .cable_type = EXTCON_TA, },
};

static struct mms_ts_info *tsp_driver = NULL;
void mms_set_tsp_info(struct mms_ts_info *tsp_data)
{
	if (tsp_data != NULL)
		tsp_driver = tsp_data;
	else
		pr_info("%s : tsp info is null\n", __func__);
}
static struct mms_ts_info * mms_get_tsp_info(void)
{
	return tsp_driver;
}

void set_charger_config(struct mms_ts_info *tsp_data)
{
	// not yet.
	pr_err("%s mode\n",	tsp_data->charger_mode ? "charging" : "battery");

}

static void mms_charger_notify_work(struct work_struct *work)
{
	struct extcon_tsp_ta_cable *cable =
		container_of(work, struct extcon_tsp_ta_cable, work);
	struct mms_ts_info *tsp_data = mms_get_tsp_info();
	//int rc;
	if (!tsp_data) {
		pr_err("%s tsp driver is null\n", __func__);
		return;
	}

	tsp_data->charger_mode = cable->cable_state;

	if (!tsp_data->enabled) {
		pr_err("%s tsp is stopped\n", __func__);
		//	schedule_delayed_work(&tsp_data->noti_dwork, HZ / 5);
		return ;
	}

	pr_err("%s mode\n",
			tsp_data->charger_mode ? "charging" : "battery");

	set_charger_config(tsp_data);
}

static int mms_charger_notify(struct notifier_block *nb,
		unsigned long stat, void *ptr)
{
	struct extcon_tsp_ta_cable *cable =
		container_of(nb, struct extcon_tsp_ta_cable, nb);
	pr_info("%s, %ld\n", __func__, stat);
	cable->cable_state = stat;

	schedule_work(&cable->work);

	return NOTIFY_DONE;

}

static int __init mms_init_charger_notify(void)
{
	struct mms_ts_info *tsp_data = mms_get_tsp_info();
	struct extcon_tsp_ta_cable *cable;
	int ret;
	int i;

	if (!tsp_data) {
		pr_info("%s tsp driver is null\n", __func__);
		return 0;
	}
	pr_info("%s register extcon notifier for usb and ta\n", __func__);
	for (i = 0; i < ARRAY_SIZE(support_cable_list); i++) {
		cable = &support_cable_list[i];
		INIT_WORK(&cable->work, mms_charger_notify_work);
		cable->nb.notifier_call = mms_charger_notify;

		ret = extcon_register_interest(&cable->extcon_nb,
				EXTCON_DEV_NAME,
				extcon_cable_name[cable->cable_type],
				&cable->nb);
		if (ret)
			pr_err("%s: fail to register extcon notifier(%s, %d)\n",
					__func__, extcon_cable_name[cable->cable_type],
					ret);

		cable->edev = cable->extcon_nb.edev;
		if (!cable->edev)
			pr_err("%s: fail to get extcon device\n", __func__);
	}
	return 0;
}
device_initcall_sync(mms_init_charger_notify);
#endif

int mms_enable_glove_mode(struct mms_ts_info *info, int enable)
{
	u8 wbuf[8];

	if (!info->dtdata->support_glove_mode) {
		dev_err(&info->client->dev,
				"%s: not support glove mode\n", __func__);
		return -EINVAL;
	}

	if (enable != 1 && enable != 0) {
		dev_err(&info->client->dev,
				"%s: param %d not available\n", __func__, enable);
		return -EINVAL;
	}

	wbuf[0] = MIP_R0_CTRL;
	wbuf[1] = MIP_R1_CTRL_GLOVE_MODE;
	wbuf[2] = enable;

	if (mms_i2c_write(info, wbuf, 3)) {
		dev_err(&info->client->dev,
				"%s write fail\n", __func__);
		return -EIO;
	}

	info->glove_mode = enable;
	dev_info(&info->client->dev, "%s: %s\n", __func__, enable ? "on" : "off");
	return 0;
}
