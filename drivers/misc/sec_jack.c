/*  drivers/misc/sec_jack.c
 *
 *  Copyright (C) 2012 Samsung Electronics Co.Ltd
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
 */
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/switch.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/sec_jack.h>
#include <linux/of_gpio.h>
#include <linux/qpnp/qpnp-adc.h>
#if defined (SEC_HEADSET_ADC_ADJUST)
#include <linux/uaccess.h>
#include <linux/file.h>
#endif /* SEC_HEADSET_ADC_ADJUST */

#define NUM_INPUT_DEVICE_ID	2
#define MAX_ZONE_LIMIT		10
#define SEND_KEY_CHECK_TIME_MS	30		/* 30ms */
#define DET_CHECK_TIME_MS	   100		/* 100ms */
#define DET_CHECK_TIME_MS_WITH_FSA 50		/* 50ms */
#define WAKE_LOCK_TIME		(HZ * 5)	/* 5 sec */
#define ADC_SAMPLE_CNT		1   /* Ear key ADC chekc count */

struct sec_jack_info {
	struct platform_device *client;
	struct sec_jack_platform_data *pdata;
	struct delayed_work jack_detect_work;
	struct work_struct buttons_work;
	struct work_struct detect_work;
	struct workqueue_struct *queue;
	struct workqueue_struct *buttons_queue;
	struct timer_list timer;
	struct input_dev *input_dev;
	struct wake_lock det_wake_lock;
	struct sec_jack_zone *zone;
	struct input_handler handler;
	struct input_handle handle;
	struct input_device_id ids[NUM_INPUT_DEVICE_ID];
	int det_irq;
	int dev_id;
	int pressed;
	int pressed_code;
	bool buttons_enable;
	struct platform_device *send_key_dev;
	unsigned int cur_jack_type;
	struct timespec ts; 		/* Get Current time for KSND */
	struct timespec ts_after;	/* Get Current time After Event */
};

/* with some modifications like moving all the gpio structs inside
 * the platform data and getting the name for the switch and
 * gpio_event from the platform data, the driver could support more than
 * one headset jack, but currently user space is looking only for
 * one key file and switch for a headset so it'd be overkill and
 * untestable so we limit to one instantiation for now.
 */
static atomic_t instantiated = ATOMIC_INIT(0);

/* sysfs name HeadsetObserver.java looks for to track headset state
 */
struct switch_dev switch_jack_detection = {
	.name = "h2w",
};

/* Samsung factory test application looks for to track button state
 */
struct switch_dev switch_sendend = {
	.name = "send_end", /* /sys/class/switch/send_end/state */
};

static struct gpio_event_direct_entry sec_jack_key_map[] = {
	{
		.code	= KEY_UNKNOWN,
	},
};

static struct gpio_event_input_info sec_jack_key_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,
	.type = EV_KEY,
	.debounce_time.tv64 = SEND_KEY_CHECK_TIME_MS * NSEC_PER_MSEC,
	.keymap = sec_jack_key_map,
	.keymap_size = ARRAY_SIZE(sec_jack_key_map)
};

static struct gpio_event_info *sec_jack_input_info[] = {
	&sec_jack_key_info.info,
};

static struct gpio_event_platform_data sec_jack_input_data = {
	.name = "sec_jack",
	.info = sec_jack_input_info,
	.info_count = ARRAY_SIZE(sec_jack_input_info),
};

#if defined (SEC_HEADSET_ADC_ADJUST)
#define ADC_PATH "data/data/com.sec.ksndtestmode/files/"
#define ADC_NAME "headset_adc.dat"
#define FILE_BUFFER_SIZE  256

enum {
	POLE3_MAX_ADC,
	POLE4_LOW_ADC,
	POLE4_MAX_ADC,
	KEY_MEDIA_ADC,
	KEY_VOICECOMMAND_ADC,
	KEY_VOLUME_UP_ADC,
	KEY_VOLUME_DOWN_ADC,
	MAX_ARRAY
} sed_jack_adc_type;

static char file_path[64];
static bool is_init = false;

static int sec_jack_read_data(struct sec_jack_info *hi, char * filename);
static int sec_jack_write_data(struct sec_jack_info *hi, char * filename );
#endif /* SEC_HEADSET_ADC_ADJUST */

int ear_adc_value = 0;

/*Enabling Ear Mic Bias of WCD Codec*/
extern void msm8x16_enable_ear_micbias(bool state);

#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
static int atof(const char *str)
{
	int result = 0;
	int count = 0;

	while (str[count] != 0	/* NULL */
		&& ((str[count] >= '0' && str[count] <= '9') || str[count] == '.')) {
		if (str[count] != '.')
			result = result * 10 + str[count] - '0';

		++count;
	}

	return result;
}
#endif
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
extern void set_earjack_state(void);

bool is_sec_earjack_on = false;
bool sec_jack_onoff(void)
{
    return is_sec_earjack_on;
}
#endif

static void sec_jack_gpio_init(struct sec_jack_platform_data *pdata)
{
	int ret;

	if(pdata->ear_micbias_gpio > 0) {
		ret = gpio_request(pdata->ear_micbias_gpio, "ear_micbias_en");
		if (ret) {
			pr_err("%s : gpio_request failed for %d\n", __func__,
				pdata->ear_micbias_gpio);
			return;
		}
		gpio_direction_output(pdata->ear_micbias_gpio, 0);
	}

	if (pdata->fsa_en_gpio > 0) {
		ret = gpio_request(pdata->fsa_en_gpio, "fsa_en");
		if (ret) {
			pr_err("%s : gpio_request failed for %d\n", __func__,
				pdata->fsa_en_gpio);
			return;
		}
		gpio_direction_output(pdata->fsa_en_gpio, 1);
	}

}

static int sec_jack_get_adc_value(struct sec_jack_info *hi, int count)
{
	struct qpnp_vadc_result result;
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct qpnp_vadc_chip *earjack_vadc;
	int retVal;
	int maxVal = 0, minVal = 0xffff, totalVal = 0, adcVal;
	int i;
	uint32_t mpp_ch;

	/* Initialize mpp_ch default setting
	* default mpp scale is  < 4 1 3 >
	*/
	mpp_ch = pdata->mpp_ch_scale[0] + P_MUX1_1_3 - 1;

	/* To get proper mpp_ch,
	* If scaling is 1:1 then add (P_MUX1_1_1 - 1)
	* If scaling is 1:3 then add (P_MUX1_1_3 - 1)
	*/
	if (pdata->mpp_ch_scale[2] == 1)
		mpp_ch = pdata->mpp_ch_scale[0] + P_MUX1_1_1 - 1;
	else if (pdata->mpp_ch_scale[2] == 3)
		mpp_ch = pdata->mpp_ch_scale[0] + P_MUX1_1_3 - 1;
	else
		pr_err("%s - invalid channel scale=%d\n", __func__, pdata->mpp_ch_scale[2]);

	earjack_vadc = qpnp_get_vadc(&hi->client->dev, "earjack-read");

	if(count < 3) {
		qpnp_vadc_read(earjack_vadc,  mpp_ch, &result);
		// Get voltage in microvolts
		retVal = ((int)result.physical)/1000;
	} else {
		for (i = 0; i < count; i++) {
			qpnp_vadc_read(earjack_vadc,  mpp_ch, &result);
			// Get voltage in microvolts
			adcVal = ((int)result.physical)/1000;
			if (i != 0) {
				if (adcVal > maxVal)
					maxVal = adcVal;
				else if (adcVal < minVal)
					minVal = adcVal;
			} else {
				maxVal = adcVal;
				minVal = adcVal;
			}
			totalVal += adcVal;
		}
		retVal = (totalVal - maxVal - minVal) / (count - 2);
	}
	return retVal;
}

static void set_sec_micbias_state(struct sec_jack_info *hi, bool state)
{
	struct sec_jack_platform_data *pdata = hi->pdata;

	if(pdata->ear_micbias_gpio > 0)
		gpio_set_value_cansleep(pdata->ear_micbias_gpio, state); /*Uses external Mic Bias*/
	else
		msm8x16_enable_ear_micbias(state); /* Uses WCD Mic Bias*/
}

/* gpio_input driver does not support to read adc value.
 * We use input filter to support 3-buttons of headset
 * without changing gpio_input driver.
 */
static bool sec_jack_buttons_filter(struct input_handle *handle,
	unsigned int type, unsigned int code, int value)
{
	struct sec_jack_info *hi = handle->handler->private;

	if (type != EV_KEY || code != KEY_UNKNOWN)
		return false;

	hi->pressed = value;

	/* This is called in timer handler of gpio_input driver.
	 * We use workqueue to read adc value.
	 */
	queue_work(hi->buttons_queue, &hi->buttons_work);

	return true;
}

static int sec_jack_buttons_connect(struct input_handler *handler,
	struct input_dev *dev, const struct input_device_id *id)
{
	struct sec_jack_info *hi;
	struct sec_jack_platform_data *pdata;
	struct sec_jack_buttons_zone *btn_zones;
	int err;
	int i;
	int num_buttons_zones;

	/* bind input_handler to input device related to only sec_jack */
	if (dev->name != sec_jack_input_data.name)
		return -ENODEV;

	hi = handler->private;
	pdata = hi->pdata;
	btn_zones = pdata->jack_buttons_zones;
	num_buttons_zones = ARRAY_SIZE(pdata->jack_buttons_zones);

	hi->input_dev = dev;
	hi->handle.dev = dev;
	hi->handle.handler = handler;
	hi->handle.open = 0;
	hi->handle.name = "sec_jack_buttons";

	err = input_register_handle(&hi->handle);
	if (err) {
		pr_err("%s: Failed to register handle, error %d\n",
			__func__, err);
		goto err_register_handle;
	}

	err = input_open_device(&hi->handle);
	if (err) {
		pr_err("%s: Failed to open input device, error %d\n",
			__func__, err);
		goto err_open_device;
	}

	for (i = 0; i < num_buttons_zones; i++)
		input_set_capability(dev, EV_KEY, btn_zones[i].code);

	return 0;

err_open_device:
	input_unregister_handle(&hi->handle);
err_register_handle:

	return err;
}

static void sec_jack_buttons_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
}

static void sec_jack_set_type(struct sec_jack_info *hi, int jack_type)
{
	//struct sec_jack_platform_data *pdata = hi->pdata;

	/* this can happen during slow inserts where we think we identified
	 * the type but then we get another interrupt and do it again
	 */
	if (jack_type == hi->cur_jack_type) {
		if (jack_type != SEC_HEADSET_4POLE)
			set_sec_micbias_state(hi, false);
		return;
	}

	if (jack_type == SEC_HEADSET_4POLE) {
		/* for a 4 pole headset, enable detection of send/end key */
		if (hi->send_key_dev == NULL)
			/* enable to get events again */
			hi->send_key_dev = platform_device_register_data(NULL,
				GPIO_EVENT_DEV_NAME,
				hi->dev_id,
				&sec_jack_input_data,
				sizeof(sec_jack_input_data));
		mod_timer(&hi->timer,jiffies + msecs_to_jiffies(1000));
#if defined (SEC_HEADSET_ADC_ADJUST)
		/* Update and inform the adc date file */
		if( is_init ){
			sec_jack_read_data(hi, NULL);
		} else {
			sec_jack_write_data(hi, NULL);
		}
#endif /* SEC_HEADSET_ADC_ADJUST */
	} else {
		/* for all other jacks, disable send/end key detection */
		if (hi->send_key_dev != NULL) {
			/* disable to prevent false events on next insert */
			platform_device_unregister(hi->send_key_dev);
			hi->send_key_dev = NULL;
			del_timer_sync(&hi->timer);
			hi->buttons_enable = false;
		}
		/* micbias is left enabled for 4pole and disabled otherwise */
		set_sec_micbias_state(hi, false);
	}

	hi->cur_jack_type = jack_type;
	pr_info("%s : jack_type = %d\n", __func__, jack_type);
	switch_set_state(&switch_jack_detection, jack_type);

	/* Estimate Headset detection time */  //KSND
	if( jack_type == SEC_HEADSET_4POLE || jack_type == SEC_HEADSET_3POLE ) {
		hi->ts_after = current_kernel_time();
		hi->ts.tv_nsec = hi->ts_after.tv_nsec - hi->ts.tv_nsec;
		if( hi->ts.tv_nsec < 0) {
			--hi->ts.tv_sec;
			hi->ts.tv_nsec += 1000000000L;
		}
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
		is_sec_earjack_on = true;
		set_earjack_state();
#endif
		pr_info("%s: detect time : %d ms\n", __func__, (int)hi->ts.tv_nsec/1000000 );
	}

}

static void determine_jack_type(struct sec_jack_info *hi)
{
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct sec_jack_zone *zones = pdata->jack_zones;
	int size = ARRAY_SIZE(pdata->jack_zones);
	int count[MAX_ZONE_LIMIT] = {0};
	int adc;
	int i;
	unsigned npolarity = !pdata->det_active_high;

	pr_info("%s - detect adc range - [%d %d] [%d %d]\n", __func__,
		zones[0].adc_high, zones[1].adc_high, zones[2].adc_high, zones[3].adc_high) ;

	/* set mic bias to enable adc */
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
	/* There is an issue detect adc is low because Micbias is 2.2V.
	   Micbias is 2.2V when detect irq is detected
	   while 4-pole headset is connected. */
	msm8x16_wcd_dynamic_control_micbias(MIC_BIAS_V2P80V);
#else
	set_sec_micbias_state(hi, true);
#endif

	while (gpio_get_value(pdata->det_gpio) ^ npolarity) {
		adc = sec_jack_get_adc_value(hi, 1);
		pr_info("%s: adc = %d\n", __func__, adc);

		/* determine the type of headset based on the
		 * adc value.  An adc value can fall in various
		 * ranges or zones.  Within some ranges, the type
		 * can be returned immediately.  Within others, the
		 * value is considered unstable and we need to sample
		 * a few more types (up to the limit determined by
		 * the range) before we return the type for that range.
		 */

		for (i = 0; i < size; i++) {
			if (adc <= zones[i].adc_high) {
				if (++count[i] > zones[i].check_count) {
					sec_jack_set_type(hi,
						zones[i].jack_type);
					return;
				}
				if (zones[i].delay_us > 0)
					usleep_range(zones[i].delay_us, zones[i].delay_us);
				break;
			}
		}
	}

	/* jack removed before detection complete */
	pr_debug("%s : jack removed before detection complete\n", __func__);
	sec_jack_set_type(hi, SEC_JACK_NO_DEVICE);
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
	is_sec_earjack_on = false;
    set_earjack_state();
#endif
	set_sec_micbias_state(hi, false);
}

static ssize_t key_state_onoff_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int value = 0;

	if (hi->pressed && hi->pressed_code == KEY_MEDIA)
		value = 1;

	return snprintf(buf, 4, "%d\n", value);
}

static DEVICE_ATTR(key_state, 0444 , key_state_onoff_show, NULL);

static ssize_t earjack_state_onoff_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int value = 0;

	if (hi->cur_jack_type == SEC_HEADSET_4POLE)
		value = 1;

	return snprintf(buf, 4, "%d\n", value);
}

static DEVICE_ATTR(state, 0444 , earjack_state_onoff_show, NULL);

static ssize_t mic_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int value = 0;

	value = ear_adc_value;
	return snprintf(buf, 10, "%d\n", value);
}

static DEVICE_ATTR(mic_adc, 0444 , mic_adc_show, NULL);

#if defined (CONFIG_EARJACK_ADC_SYSFS)
static ssize_t jack_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int val[4] = {0,};

	val[0] = hi->pdata->jack_zones[0].adc_high;
	val[1] = hi->pdata->jack_zones[1].adc_high;
	val[2] = hi->pdata->jack_zones[2].adc_high;
	val[3] = hi->pdata->jack_zones[3].adc_high;

	return sprintf(buf, "%d %d %d %d\n",val[0],val[1],val[2],val[3]);

}

static ssize_t jack_adc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{

	struct sec_jack_info *hi = dev_get_drvdata(dev);
	char *buffer = (char*)buf;
	char *token;
	int val[4] = {0,};

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[0]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[1]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[2]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[3]) != 0)
		return -EINVAL;

	hi->pdata->jack_zones[0].adc_high = val[0];
	hi->pdata->jack_zones[1].adc_high = val[1];
	hi->pdata->jack_zones[2].adc_high = val[2];
	hi->pdata->jack_zones[3].adc_high = val[3];


	return count;
	}


static DEVICE_ATTR(jack_adc, 0666, jack_adc_show,
	jack_adc_store);

static ssize_t send_end_btn_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int val[2]  = {0,};

	val[0] = hi->pdata->jack_buttons_zones[0].adc_low;
	val[1] = hi->pdata->jack_buttons_zones[0].adc_high;

	return sprintf(buf, "%d %d\n",val[0],val[1]);
	}

static ssize_t send_end_btn_adc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	char *buffer = (char*)buf;
	char *token;
	int val[2] = {0,};

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[0]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[1]) != 0)
		return -EINVAL;

	hi->pdata->jack_buttons_zones[0].adc_low = val[0];
	hi->pdata->jack_buttons_zones[0].adc_high = val[1];

	return count;
}
static DEVICE_ATTR(send_end_btn_adc, 0666, send_end_btn_adc_show,
	send_end_btn_adc_store);

static ssize_t voicecommand_btn_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int val[2]  = {0,};

	val[0] = hi->pdata->jack_buttons_zones[1].adc_low;
	val[1] = hi->pdata->jack_buttons_zones[1].adc_high;

	return sprintf(buf, "%d %d\n",val[0],val[1]);
}

static ssize_t voicecommand_btn_adc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	char *buffer = (char*)buf;
	char *token;
	int val[2] = {0,};

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[0]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[1]) != 0)
		return -EINVAL;

	hi->pdata->jack_buttons_zones[1].adc_low = val[0];
	hi->pdata->jack_buttons_zones[1].adc_high = val[1];

	return count;
}
static DEVICE_ATTR(voicecommand_btn_adc, 0666, voicecommand_btn_adc_show,
	voicecommand_btn_adc_store);

static ssize_t vol_up_btn_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int val[2]  = {0,};

	if(hi->pdata->jack_buttons_zones[2].adc_low == hi->pdata->jack_buttons_zones[3].adc_low) {
		val[0] = hi->pdata->jack_buttons_zones[1].adc_low;
		val[1] = hi->pdata->jack_buttons_zones[1].adc_high;
	} else {
		val[0] = hi->pdata->jack_buttons_zones[2].adc_low;
		val[1] = hi->pdata->jack_buttons_zones[2].adc_high;
	}

	return sprintf(buf, "%d %d\n",val[0],val[1]);
}

static ssize_t vol_up_btn_adc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	char *buffer = (char*)buf;
	char *token;
	int val[2] = {0,};

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[0]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[1]) != 0)
		return -EINVAL;

	if(hi->pdata->jack_buttons_zones[2].adc_low == hi->pdata->jack_buttons_zones[3].adc_low) {
		hi->pdata->jack_buttons_zones[1].adc_low = val[0];
		hi->pdata->jack_buttons_zones[1].adc_high = val[1];
	} else {
		hi->pdata->jack_buttons_zones[2].adc_low = val[0];
		hi->pdata->jack_buttons_zones[2].adc_high = val[1];
	}

	return count;
}
static DEVICE_ATTR(vol_up_btn_adc, 0666, vol_up_btn_adc_show,
	vol_up_btn_adc_store);

static ssize_t vol_down_btn_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	int val[2]  = {0,};

	if(hi->pdata->jack_buttons_zones[2].adc_low == hi->pdata->jack_buttons_zones[3].adc_low) {
		val[0] = hi->pdata->jack_buttons_zones[2].adc_low;
		val[1] = hi->pdata->jack_buttons_zones[2].adc_high;
	} else {
		val[0] = hi->pdata->jack_buttons_zones[3].adc_low;
		val[1] = hi->pdata->jack_buttons_zones[3].adc_high;
	}

	return sprintf(buf, "%d %d\n",val[0],val[1]);
}

static ssize_t vol_down_btn_adc_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct sec_jack_info *hi = dev_get_drvdata(dev);
	char *buffer = (char*)buf;
	char *token;
	int val[2] = {0,};

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[0]) != 0)
		return -EINVAL;

	token = strsep(&buffer, " ");
	if(kstrtoint(token,0,&val[1]) != 0)
		return -EINVAL;

	if(hi->pdata->jack_buttons_zones[2].adc_low == hi->pdata->jack_buttons_zones[3].adc_low) {
		hi->pdata->jack_buttons_zones[2].adc_low = val[0];
		hi->pdata->jack_buttons_zones[2].adc_high = val[1];
	} else {
		hi->pdata->jack_buttons_zones[3].adc_low = val[0];
		hi->pdata->jack_buttons_zones[3].adc_high = val[1];
	}

	return count;
}
static DEVICE_ATTR(vol_down_btn_adc, 0666, vol_down_btn_adc_show,
	vol_down_btn_adc_store);
#endif /* CONFIG_EARJACK_ADC_SYSFS */

#if defined (SEC_HEADSET_ADC_ADJUST)
static int sec_jack_read_data( struct sec_jack_info *hi, char * filename )
{
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct file	*flip;
	char *bufs, *token;
	int i=0,size,ret;
	unsigned long adc_array[MAX_ARRAY];

	// kernel memory access setting
	mm_segment_t oldfs = get_fs();
	set_fs(KERNEL_DS);

	// set default name if we don't set any data file name
	if(filename == NULL){ 
		filename = file_path;
	}

	flip = filp_open(filename, O_RDWR, S_IRUSR|S_IWUSR);
	if(IS_ERR(flip)){
		pr_err("%s : File not found or open error!\n",__func__);
		return -1;
	}

	//file size
	size = flip->f_dentry->d_inode->i_size;

	bufs = kmalloc( size, GFP_KERNEL);
	if( bufs == NULL ){
		pr_err("%s : kmalloc error!\n", __func__);
		return -1;
	}
	ret = vfs_read(flip, bufs, size, &flip->f_pos);

	//set to the array
	while ((token = strsep(&bufs, ",")) != NULL) {
		if ( i == MAX_ARRAY ||(strict_strtoul(token , 10, &adc_array[i]) < 0))
			break;
		i++;
	}

	/* Close the file */
	if( flip ){
		filp_close(flip, NULL);
	}
	set_fs(oldfs);
	kfree(bufs);

	//check the data buffer has all 0 or not.
	for( i=0;i<MAX_ARRAY;i++ ){
		if( adc_array[i] != 0 ){
			goto read_data;
		}
	}
	goto rewrite_data;

rewrite_data:
	is_init = false;
	return 0;

read_data:
	pdata->jack_zones[1].adc_high = adc_array[POLE3_MAX_ADC];
	pdata->jack_zones[2].adc_high = adc_array[POLE4_LOW_ADC];
	pdata->jack_zones[3].adc_high = adc_array[POLE4_MAX_ADC];
	pdata->jack_buttons_zones[0].adc_high = adc_array[KEY_MEDIA_ADC];
	pdata->jack_buttons_zones[1].adc_high = adc_array[KEY_VOICECOMMAND_ADC];
	pdata->jack_buttons_zones[2].adc_high = adc_array[KEY_VOLUME_UP_ADC];
	pdata->jack_buttons_zones[3].adc_high = adc_array[KEY_VOLUME_DOWN_ADC];

	pr_info("%s : Read the ADC data file. size:%d\n", __func__, ret);
	return 0;
}

static int sec_jack_write_data(struct sec_jack_info *hi, char * filename )
{
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct file 	*flip;
	unsigned int adc_array[MAX_ARRAY];
	char 	*result;
	int 	i;
	char 	buf[128];
	mm_segment_t 	oldfs = get_fs();

	// kernel memory access setting
    set_fs(KERNEL_DS);

	// set default name if we don't set any data file name
	if(filename == NULL){ 
		filename = file_path;
	}

	//open the adc data file from user fs
	flip = 	filp_open(filename, O_RDWR, S_IRUSR|S_IWUSR);
	if(IS_ERR(flip)){
		flip = NULL;
		pr_err("%s : File not found or open error!\n", __func__);
		return -1;
	}

	//init buffer to write adc data to the file
	adc_array[POLE3_MAX_ADC] = pdata->jack_zones[1].adc_high;
	adc_array[POLE4_LOW_ADC] = pdata->jack_zones[2].adc_high;
	adc_array[POLE4_MAX_ADC] = pdata->jack_zones[3].adc_high;
	adc_array[KEY_MEDIA_ADC] = pdata->jack_buttons_zones[0].adc_high;
	adc_array[KEY_VOICECOMMAND_ADC] = pdata->jack_buttons_zones[1].adc_high;
	adc_array[KEY_VOLUME_UP_ADC] = pdata->jack_buttons_zones[2].adc_high;
	adc_array[KEY_VOLUME_DOWN_ADC] = pdata->jack_buttons_zones[3].adc_high;

	result = kmalloc( FILE_BUFFER_SIZE, GFP_KERNEL);
	if( result == NULL ){
		pr_err("%s : kmalloc error!\n", __func__);
		return -1;
	}

	//make the data format with adc data.
	for(i=0; i < MAX_ARRAY; i++){
		sprintf(buf, "%d", adc_array[i] );
		strcat(result, buf);
		strcat(result, ",");
	}
	pr_info("%s : data : %s \n", __func__, result);

	vfs_write(flip, result, strlen(result), &flip->f_pos);
	set_fs(oldfs);
	kfree(result);

	// Close  the adc data file from user fs
	if( flip ){
		filp_close(flip, NULL);
	}
	is_init = true;

	pr_info("%s : Write the ADC data file\n", __func__);
	return 0;
}

static void sec_jack_adcData_init (void)
{
	//get the file path name
	strcpy(file_path, ADC_PATH);
	strcat(file_path, ADC_NAME);

	is_init = false;
	pr_info("%s : file path : %s \n", __func__, file_path);
}
#endif /* SEC_HEADSET_ADC_ADJUST */

static void sec_jack_timer_handler(unsigned long data)
{
	struct sec_jack_info *hi = (struct sec_jack_info *)data;

	hi->buttons_enable = true;

}
/* thread run whenever the headset detect state changes (either insertion
 * or removal).
 */
static irqreturn_t sec_jack_detect_irq(int irq, void *dev_id)
{
	struct sec_jack_info *hi = dev_id;

	queue_work(hi->queue, &hi->detect_work);

	/* Get Current time of detecting headset */
	hi->ts = current_kernel_time();

	return IRQ_HANDLED;
}

void sec_jack_detect_work(struct work_struct *work)
{
	struct sec_jack_info *hi =
		container_of(work, struct sec_jack_info, detect_work);
	struct sec_jack_platform_data *pdata = hi->pdata;
	unsigned npolarity = !hi->pdata->det_active_high;
	int time_left_ms;

#ifdef CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND
        disable_irq(hi->det_irq);
#endif /* CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND */
	if (pdata->fsa_en_gpio < 0)
		time_left_ms = DET_CHECK_TIME_MS;
	else
		time_left_ms = DET_CHECK_TIME_MS_WITH_FSA;

	/* prevent suspend to allow user space to respond to switch */
	wake_lock_timeout(&hi->det_wake_lock, WAKE_LOCK_TIME);

	pr_info("%s: detect_irq(%d)\n", __func__,
		gpio_get_value(pdata->det_gpio) ^ npolarity);

	/* debounce headset jack.  don't try to determine the type of
	 * headset until the detect state is true for a while.
	 */
	while (time_left_ms > 0) {
		if (!(gpio_get_value(hi->pdata->det_gpio) ^ npolarity)) {
			/* jack not detected. */
			sec_jack_set_type(hi, SEC_JACK_NO_DEVICE);
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
			/* restore micbias to 2.8V */
			msm8x16_wcd_dynamic_control_micbias(MIC_BIAS_V2P80V);
#endif
#if defined(CONFIG_MACH_KOR_EARJACK_WR)
			is_sec_earjack_on = false;
			set_earjack_state();
#endif
			set_sec_micbias_state(hi, false);
#ifdef CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND
                       enable_irq(hi->det_irq);
#endif /* CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND */
			return;
		}
		usleep_range(10000, 10000);
		time_left_ms -= 10;
	}

	/* jack presence was detected the whole time, figure out which type */
	determine_jack_type(hi);
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
	/* Mic bias default dynamic voltage */
	if (hi->cur_jack_type == SEC_HEADSET_4POLE)
		msm8x16_wcd_dynamic_control_micbias(pdata->dynamic_micb_ctrl_voltage);
#endif

#ifdef CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND
       enable_irq(hi->det_irq);
#endif /* CONFIG_SAMSUNG_JACK_WATCH_DOG_RESET_WORK_AROUND */
}

/* thread run whenever the button of headset is pressed or released */
void sec_jack_buttons_work(struct work_struct *work)
{
	struct sec_jack_info *hi =
		container_of(work, struct sec_jack_info, buttons_work);
	struct sec_jack_platform_data *pdata = hi->pdata;
	struct sec_jack_buttons_zone *btn_zones = pdata->jack_buttons_zones;
	int num_buttons_zones = ARRAY_SIZE(pdata->jack_buttons_zones);
	int adc;
	int i;

	ear_adc_value = 0;

	pr_info("%s - BTN adc range - [%d %d] [%d %d] [%d %d] [%d %d]\n", __func__,
		btn_zones[0].adc_low, btn_zones[0].adc_high,
		btn_zones[1].adc_low, btn_zones[1].adc_high,
		btn_zones[2].adc_low, btn_zones[2].adc_high,
		btn_zones[3].adc_low, btn_zones[3].adc_high);

	if (!hi->buttons_enable) {
		pr_info("%s: BTN %d is skipped\n", __func__,
			hi->pressed_code);
		return;
	}
	/* prevent suspend to allow user space to respond to switch */
	wake_lock_timeout(&hi->det_wake_lock, WAKE_LOCK_TIME);

	/* when button is released */
	if (hi->pressed == 0) {
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
		/* Mic bias must keep 2.8V when mic is enalbed */
		if (!is_mic_enable())
			msm8x16_wcd_dynamic_control_micbias(pdata->dynamic_micb_ctrl_voltage);
#endif
		input_report_key(hi->input_dev, hi->pressed_code, 0);
		input_sync(hi->input_dev);
		switch_set_state(&switch_sendend, 0);
		pr_info("%s: BTN %d is released\n", __func__,
			hi->pressed_code);
		return;
	}

	/* when button is pressed */
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
	/* Mic bias is alreay 2.8V due to mic is enalbed */
	if (!is_mic_enable())
		msm8x16_wcd_dynamic_control_micbias(MIC_BIAS_V2P80V);
#endif

	adc = sec_jack_get_adc_value(hi, pdata->btn_adc_read_count);
	ear_adc_value = adc;

	for (i = 0; i < num_buttons_zones; i++)
		if (adc >= btn_zones[i].adc_low &&
			adc <= btn_zones[i].adc_high) {
			hi->pressed_code = btn_zones[i].code;
			input_report_key(hi->input_dev, btn_zones[i].code, 1);
			input_sync(hi->input_dev);
			switch_set_state(&switch_sendend, 1);
			pr_info("%s: adc = %d, BTN %d is pressed\n", __func__,
				adc, btn_zones[i].code);
			return;
		}

	pr_warn("%s: key is skipped. ADC value is %d\n", __func__, adc);
}

int sec_jack_get_button_type(int arg)
{
	switch(arg) {
	case 1:
		return KEY_VOLUMEUP;
	case 2:
		return KEY_VOLUMEDOWN;
	case 3:
		return KEY_VOICECOMMAND;
	case 0:
	default:
		return KEY_MEDIA;
	}
}

static struct sec_jack_platform_data *sec_jack_populate_dt_pdata(struct device *dev)
{
	struct sec_jack_platform_data *pdata;
	struct of_phandle_args args;
	int i = 0;
	int ret = 0;
#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
	const char *voltage_str;
#endif
	pdata =  devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		pr_err("%s : could not allocate memory for platform data\n", __func__);
		goto alloc_err;
	}

	pdata->det_gpio = of_get_named_gpio(dev->of_node, "qcom,earjack-detect-gpio", 0);
	if (pdata->det_gpio < 0) {
		pr_err("%s : can not find the earjack-detect-gpio in the dt\n", __func__);
	} else
		pr_info("%s : earjack-detect-gpio =%d\n", __func__, pdata->det_gpio);

	pdata->send_end_gpio = of_get_named_gpio(dev->of_node, "qcom,earjack-sendend-gpio", 0);
	if (pdata->send_end_gpio < 0) {
		pr_err("%s : can not find the earjack-detect-gpio in the dt\n", __func__);
	} else
		pr_info("%s : earjack-sendend-gpio =%d\n", __func__, pdata->send_end_gpio);

	pdata->ear_micbias_gpio = of_get_named_gpio(dev->of_node, "qcom,earjack-micbias-gpio", 0);
	if (pdata->ear_micbias_gpio < 0)
		of_property_read_u32(dev->of_node, "qcom,earjack-micbias-expander-gpio", &pdata->ear_micbias_gpio);
	if (pdata->ear_micbias_gpio < 0) {
		pr_err("%s : can not find the earjack-micbias-gpio in the dt\n", __func__);
	} else
		pr_info("%s : earjack-micbias-gpio =%d\n", __func__, pdata->ear_micbias_gpio);	
			
	pdata->fsa_en_gpio = of_get_named_gpio(dev->of_node, "qcom,earjack-fsa_en-gpio", 0);
	if (pdata->fsa_en_gpio < 0) 
		of_property_read_u32(dev->of_node, "qcom,earjack-fsa_en-expander-gpio", &pdata->fsa_en_gpio);
	if (pdata->fsa_en_gpio < 0)
		pr_info("%s : No support FSA8038 chip\n", __func__);
	else
		pr_info("%s : earjack-fsa_en-gpio =%d\n", __func__, pdata->fsa_en_gpio);

	for( i = 0; i < 4; i++)
	{
		of_parse_phandle_with_args(dev->of_node, "det-zones-list","#list-det-cells", i, &args);
		pdata->jack_zones[i].adc_high = args.args[0];
		pdata->jack_zones[i].delay_us = args.args[1];
		pdata->jack_zones[i].check_count = args.args[2];
		pdata->jack_zones[i].jack_type = args.args[3]==0?SEC_HEADSET_3POLE:SEC_HEADSET_4POLE;
		pr_info("%s : %d, %d, %d, %d, %d \n",
				__func__, args.args_count, args.args[0],
				args.args[1], args.args[2],args.args[3]);
	}
	for( i = 0; i < 4; i++)
	{
		of_parse_phandle_with_args(dev->of_node, "but-zones-list","#list-but-cells", i, &args);
		pdata->jack_buttons_zones[i].code = sec_jack_get_button_type(args.args[0]);
		pdata->jack_buttons_zones[i].adc_low = args.args[1];
		pdata->jack_buttons_zones[i].adc_high = args.args[2];
		pr_info("%s : %d, %d, %d, %d\n",
				__func__, args.args_count, args.args[0],
				args.args[1], args.args[2]);
	}

	if (of_property_read_u32(dev->of_node, "btn-adc-read-count", &pdata->btn_adc_read_count))
		pdata->btn_adc_read_count = ADC_SAMPLE_CNT;
	pr_info("%s - btn-adc-read-count - %d\n", __func__, pdata->btn_adc_read_count);

	if (of_find_property(dev->of_node, "qcom,send-end-active-high", NULL))
		pdata->send_end_active_high = true;

	ret = of_property_read_u32_array(dev->of_node, "mpp-channel-scaling", pdata->mpp_ch_scale, 3);
	if (ret < 0) {
		pr_err("%s : cannot find mpp-channel-scaling in the dt - using default MPP6_1_3\n",
		__func__);
		pdata->mpp_ch_scale[0] = 6;
		pdata->mpp_ch_scale[1] = 1;
		pdata->mpp_ch_scale[2] = 3;
	}
	pr_info("%s - mpp-channel-scaling - %d %d %d\n", __func__, pdata->mpp_ch_scale[0], pdata->mpp_ch_scale[1], pdata->mpp_ch_scale[2]);

#ifdef CONFIG_DYNAMIC_MICBIAS_CONTROL
	ret = of_property_read_string(dev->of_node, "dynamic-micbias-ctrl-voltage", &voltage_str);
	if (ret < 0) {
		pr_err("%s : cannot find dynamic-micbias-ctrl-voltage in the dt - using default voltage (2.8V)\n",
		__func__);
		pdata->dynamic_micb_ctrl_voltage = MIC_BIAS_V2P80V;
	}
	else {
		pdata->dynamic_micb_ctrl_voltage = set_dynamic_micb_ctrl_voltage(atof(voltage_str));
	}
	pr_info("%s - dynamic-micbias-ctrl-voltage - %d \n", __func__, pdata->dynamic_micb_ctrl_voltage);
#endif
	return pdata;

alloc_err:
	devm_kfree(dev, pdata);
	return NULL;
}

static int sec_jack_pinctrl_configure(struct sec_jack_platform_data *pdata, bool active)
{
	struct pinctrl_state *set_state;
	int retval;

	pr_info("%s - active = %d\n", __func__, active);
	if (active) {
		set_state = pinctrl_lookup_state(pdata->jack_pinctrl, "earjack_gpio_active");
		if (IS_ERR(set_state)) {
			pr_err("%s: cannot get earjack pinctrl active state\n", __func__);
			return PTR_ERR(set_state);
		}
	} else {
		set_state = pinctrl_lookup_state(pdata->jack_pinctrl, "earjack_gpio_suspend");
		if (IS_ERR(set_state)) {
			pr_err("%s: cannot get earjack pinctrl sleep state\n", __func__);
			return PTR_ERR(set_state);
		}
	}
	retval = pinctrl_select_state(pdata->jack_pinctrl, set_state);
	if (retval) {
		pr_err("%s: cannot set earjack pinctrl active state\n", __func__);
		return retval;
	}
	return 0;
}


extern bool is_codec_probe_done(void);

static int sec_jack_probe(struct platform_device *pdev)
{
	struct sec_jack_info *hi;
	struct sec_jack_platform_data *pdata;
	struct class *audio;
	struct device *earjack;
	int ret;

	if(!is_codec_probe_done()) {
		pr_err("%s - defer as codec_probe_done is false\n", __func__);
		return -EPROBE_DEFER;
	}

	pr_info("%s : Registering jack driver\n", __func__);

	pdata = sec_jack_populate_dt_pdata(&pdev->dev);
	if (!pdata) {
		pr_err("%s : pdata is NULL.\n", __func__);
		return -ENODEV;
	} else
		sec_jack_gpio_init(pdata);

	if (atomic_xchg(&instantiated, 1)) {
		pr_err("%s : already instantiated, can only have one\n",
			__func__);
		return -ENODEV;
	}

	sec_jack_key_map[0].gpio = pdata->send_end_gpio;

	hi = kzalloc(sizeof(struct sec_jack_info), GFP_KERNEL);
	if (hi == NULL) {
		pr_err("%s : Failed to allocate memory.\n", __func__);
		ret = -ENOMEM;
		goto err_kzalloc;
	}

	hi->pdata = pdata;

	/* make the id of our gpi_event device the same as our platform device,
	 * which makes it the responsiblity of the board file to make sure
	 * it is unique relative to other gpio_event devices
	 */
	hi->dev_id = pdev->id;
	/* to read the vadc */
	hi->client = pdev;	
	
	if(pdata->det_gpio > 0) {
		ret = gpio_request(pdata->det_gpio, "ear_jack_detect");
		if (ret) {
			pr_err("%s : gpio_request failed for %d\n",
				__func__, pdata->det_gpio);
			goto err_gpio_request;
		}
	}

	pdata->jack_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pdata->jack_pinctrl)) {
		if (PTR_ERR(pdata->jack_pinctrl) == -EPROBE_DEFER)
		{
			kfree(hi);
			atomic_set(&instantiated, 0);
			return -EPROBE_DEFER;
		}

		pr_err("%s: Target does not use pinctrl\n", __func__);
		pdata->jack_pinctrl = NULL;
	}

	if (pdata->jack_pinctrl) {
		ret = sec_jack_pinctrl_configure(pdata, true);
		if (ret) {
			pr_err("%s: cannot set earjack pinctrl active state\n", __func__);
			goto err_gpio_request;
		}
	}

	ret = switch_dev_register(&switch_jack_detection);
	if (ret < 0) {
		pr_err("%s : Failed to register switch device\n", __func__);
		goto err_switch_dev_register;
	}

	ret = switch_dev_register(&switch_sendend);
	if (ret < 0) {
		pr_err("%s : Failed to register switch device\n", __func__);
		goto err_switch_dev_register;
	}

	wake_lock_init(&hi->det_wake_lock, WAKE_LOCK_SUSPEND, "sec_jack_det");

	audio = class_create(THIS_MODULE, "audio");
	if (IS_ERR(audio))
		pr_err("Failed to create class(audio)!\n");

	earjack = device_create(audio, NULL, 0, NULL, "earjack");
	if (IS_ERR(earjack))
		pr_err("Failed to create device(earjack)!\n");

	ret = device_create_file(earjack, &dev_attr_key_state);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_key_state.attr.name);

	ret = device_create_file(earjack, &dev_attr_state);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_state.attr.name);

	ret = device_create_file(earjack, &dev_attr_mic_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_key_state.attr.name);

#if defined (CONFIG_EARJACK_ADC_SYSFS)
	ret = device_create_file(earjack, &dev_attr_jack_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_jack_adc.attr.name);

	ret = device_create_file(earjack, &dev_attr_send_end_btn_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_send_end_btn_adc.attr.name);

	ret = device_create_file(earjack, &dev_attr_voicecommand_btn_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_voicecommand_btn_adc.attr.name);

	ret = device_create_file(earjack, &dev_attr_vol_up_btn_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_vol_up_btn_adc.attr.name);

	ret = device_create_file(earjack, &dev_attr_vol_down_btn_adc);
	if (ret)
		pr_err("Failed to create device file in sysfs entries(%s)!\n",
			dev_attr_vol_down_btn_adc.attr.name);
#endif /* CONFIG_EARJACK_ADC_SYSFS */
	setup_timer(&hi->timer, sec_jack_timer_handler, (unsigned long)hi);

	INIT_WORK(&hi->buttons_work, sec_jack_buttons_work);
	INIT_WORK(&hi->detect_work, sec_jack_detect_work);
	hi->queue = create_singlethread_workqueue("sec_jack_wq");
	if (hi->queue == NULL) {
		ret = -ENOMEM;
		pr_err("%s: Failed to create workqueue\n", __func__);
		goto err_create_wq_failed;
	}
	hi->buttons_queue = create_singlethread_workqueue("sec_jack_buttons_wq");
	if (hi->buttons_queue == NULL) {
		ret = -ENOMEM;
		pr_err("%s: Failed to create buttons workqueue\n", __func__);
		goto err_create_buttons_wq_failed;
	}
	queue_work(hi->queue, &hi->detect_work);

	hi->det_irq = gpio_to_irq(pdata->det_gpio);

	set_bit(EV_KEY, hi->ids[0].evbit);
	hi->ids[0].flags = INPUT_DEVICE_ID_MATCH_EVBIT;
	hi->handler.filter = sec_jack_buttons_filter;
	hi->handler.connect = sec_jack_buttons_connect;
	hi->handler.disconnect = sec_jack_buttons_disconnect;
	hi->handler.name = "sec_jack_buttons";
	hi->handler.id_table = hi->ids;
	hi->handler.private = hi;
	ret = input_register_handler(&hi->handler);
	if (ret) {
		pr_err("%s : Failed to register_handler\n", __func__);
		goto err_register_input_handler;
	}
	/* We are going to remove this code later */
	if (pdata->send_end_active_high == true)
		sec_jack_key_info.flags = 1;

	ret = request_irq(hi->det_irq, sec_jack_detect_irq,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
		IRQF_ONESHOT, "sec_headset_detect", hi);
	if (ret) {
		pr_err("%s : Failed to request_irq.\n", __func__);
		goto err_request_detect_irq;
	}

	/* to handle insert/removal when we're sleeping in a call */
	ret = enable_irq_wake(hi->det_irq);
	if (ret) {
		pr_err("%s : Failed to enable_irq_wake.\n", __func__);
		goto err_enable_irq_wake;
	}

	dev_set_drvdata(&pdev->dev, hi);
	dev_set_drvdata(earjack, hi);

#if defined (SEC_HEADSET_ADC_ADJUST)
	sec_jack_adcData_init();
#endif /* SEC_HEADSET_ADC_ADJUST */

	return 0;

err_enable_irq_wake:
	free_irq(hi->det_irq, hi);
err_request_detect_irq:
	input_unregister_handler(&hi->handler);
err_register_input_handler:
	destroy_workqueue(hi->buttons_queue);
err_create_buttons_wq_failed:
	destroy_workqueue(hi->queue);
err_create_wq_failed:
	device_remove_file(earjack, &dev_attr_state);
	device_remove_file(earjack, &dev_attr_key_state);
	device_remove_file(earjack, &dev_attr_mic_adc);
#if defined (CONFIG_EARJACK_ADC_SYSFS)
	device_remove_file(earjack, &dev_attr_jack_adc);
	device_remove_file(earjack, &dev_attr_send_end_btn_adc);
	device_remove_file(earjack, &dev_attr_send_end_btn_adc);
	device_remove_file(earjack, &dev_attr_vol_up_btn_adc);
	device_remove_file(earjack, &dev_attr_vol_down_btn_adc);
#endif /* CONFIG_EARJACK_ADC_SYSFS */
	device_destroy(audio, 0);
	class_destroy(audio);
	wake_lock_destroy(&hi->det_wake_lock);
	switch_dev_unregister(&switch_jack_detection);
	switch_dev_unregister(&switch_sendend);
err_switch_dev_register:
	gpio_free(pdata->det_gpio);
err_gpio_request:
	kfree(hi);
err_kzalloc:
	atomic_set(&instantiated, 0);

	return ret;
}

static int sec_jack_remove(struct platform_device *pdev)
{

	struct sec_jack_info *hi = dev_get_drvdata(&pdev->dev);
	pr_info("%s :\n", __func__);
	disable_irq_wake(hi->det_irq);
	free_irq(hi->det_irq, hi);
	destroy_workqueue(hi->queue);
	destroy_workqueue(hi->buttons_queue);
	if (hi->send_key_dev) {
		platform_device_unregister(hi->send_key_dev);
		hi->send_key_dev = NULL;
	}
	input_unregister_handler(&hi->handler);
	wake_lock_destroy(&hi->det_wake_lock);
	switch_dev_unregister(&switch_jack_detection);
	switch_dev_unregister(&switch_sendend);
	gpio_free(hi->pdata->det_gpio);
	kfree(hi);
	atomic_set(&instantiated, 0);

	return 0;
}

#ifdef CONFIG_PM
static int sec_jack_suspend(struct device *dev)
{
	return 0;
}

static int sec_jack_resume(struct device *dev)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(sec_jack_pm_ops, sec_jack_suspend, sec_jack_resume);
#endif /* CONFIG_PM */

static const struct of_device_id sec_jack_dt_match[] = {
	{ .compatible = "sec_jack" },
	{ }
};
MODULE_DEVICE_TABLE(of, sec_jack_dt_match);

static struct platform_driver sec_jack_driver = {
	.probe = sec_jack_probe,
	.remove = sec_jack_remove,
	.driver = {
		.name = "sec_jack",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm	= &sec_jack_pm_ops,
#endif /* CONFIG_PM */
		.of_match_table = sec_jack_dt_match,
	},
};

static int __init sec_jack_init(void)
{
	return platform_driver_register(&sec_jack_driver);
}

static void __exit sec_jack_exit(void)
{
	platform_driver_unregister(&sec_jack_driver);
}

late_initcall(sec_jack_init);
module_exit(sec_jack_exit);

MODULE_AUTHOR("ms17.kim@samsung.com");
MODULE_DESCRIPTION("Samsung Electronics Corp Ear-Jack detection driver");
MODULE_LICENSE("GPL");
