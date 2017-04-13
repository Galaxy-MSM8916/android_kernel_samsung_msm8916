/* sec_thermistor.c
 *
 * Copyright (C) 2014 Samsung Electronics
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/sec_thermistor.h>
#include <linux/qpnp/qpnp-adc.h>
#include <linux/of.h>

#define ADC_SAMPLING_CNT	7

static struct qpnp_vadc_chip	*therm_vadc_dev;
static int ap_thm_chan;
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
static int flash_thm_chan;
#endif

struct sec_therm_info {
	struct device *dev;
	struct sec_therm_platform_data *pdata;
	struct delayed_work polling_work;
	struct qpnp_vadc_chip	*vadc_dev;
	int curr_temperature;
	int curr_temp_adc;
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	int curr_temperature_flash_led;
	int curr_temp_adc_flash_led;
#endif
};

static int sec_therm_get_adc_data(struct sec_therm_info *info);
static int convert_adc_to_temper(struct sec_therm_info *info, unsigned int adc);
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
static int sec_therm_get_adc_data_flash_led(struct sec_therm_info *info);
static int convert_adc_flash_to_temper(struct sec_therm_info *info, unsigned int adc);
#endif
extern struct sec_therm_platform_data * fill_therm_pdata(struct platform_device *);

static ssize_t sec_therm_show_temperature(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int adc;
	int temper;

	struct sec_therm_info *info = dev_get_drvdata(dev);

	adc = sec_therm_get_adc_data(info);
	temper = convert_adc_to_temper(info, adc);

	return sprintf(buf, "%d\n", temper);
}

static ssize_t sec_therm_show_temp_adc(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int adc;
	struct sec_therm_info *info = dev_get_drvdata(dev);

	adc = sec_therm_get_adc_data(info);

	return sprintf(buf, "%d\n", adc);
}

#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
static ssize_t sec_therm_show_temperature_flash_led(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int adc;
	int temper;

	struct sec_therm_info *info = dev_get_drvdata(dev);

	adc = sec_therm_get_adc_data_flash_led(info);
	temper = convert_adc_flash_to_temper(info, adc);

	dev_info(info->dev, "%s: adc_flash=%d\n", __func__, adc);
	return sprintf(buf, "%d\n", temper);
}

static ssize_t sec_therm_show_temp_adc_flash_led(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int adc;
	struct sec_therm_info *info = dev_get_drvdata(dev);

	adc = sec_therm_get_adc_data_flash_led(info);

	return sprintf(buf, "%d\n", adc);
}
#endif

static DEVICE_ATTR(temperature, S_IRUGO, sec_therm_show_temperature, NULL);
static DEVICE_ATTR(temp_adc, S_IRUGO, sec_therm_show_temp_adc, NULL);
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
static DEVICE_ATTR(temperature_flash, S_IRUGO, sec_therm_show_temperature_flash_led, NULL);
static DEVICE_ATTR(temp_adc_flash, S_IRUGO, sec_therm_show_temp_adc_flash_led, NULL);
#endif

static struct attribute *sec_therm_attributes[] = {
	&dev_attr_temperature.attr,
	&dev_attr_temp_adc.attr,
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	&dev_attr_temperature_flash.attr,
	&dev_attr_temp_adc_flash.attr,
#endif
	NULL
};

static const struct attribute_group sec_therm_group = {
	.attrs = sec_therm_attributes,
};

static int sec_therm_get_adc_data(struct sec_therm_info *info)
{
	int rc = 0;
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i, adc_data;

	struct qpnp_vadc_result results;

	for (i = 0; i < ADC_SAMPLING_CNT; i++) {
		rc = qpnp_vadc_read(therm_vadc_dev, ap_thm_chan, &results);

		if (rc) {
			pr_err("error reading ap_thm %d, rc = %d\n", ap_thm_chan, rc);
			goto err;
		}
		adc_data = results.adc_code;

		if (i == 0) {
			pr_err("reading ap_thm[%d] [rc = %d] [adc_code = %d]\n", ap_thm_chan, rc, results.adc_code);
		}

		if (i != 0) {
			if (adc_data > adc_max)
				adc_max = adc_data;
			else if (adc_data < adc_min)
				adc_min = adc_data;
		} else {
			adc_max = adc_data;
			adc_min = adc_data;
		}

		adc_total += adc_data;
	}

	return (adc_total - adc_max - adc_min) / (ADC_SAMPLING_CNT - 2);

err:
	return rc;

}

static int convert_adc_to_temper(struct sec_therm_info *info, unsigned int adc)
{
	int low = 0;
	int high = 0;
	int mid = 0;
	int temp = 0;
	int temp2 = 0;

	if (!info->pdata->adc_table || !info->pdata->adc_arr_size) {
		/* using fake temp */
		return 300;
	}

	high = info->pdata->adc_arr_size - 1;

	if (info->pdata->adc_table[low].adc >= adc) {
		temp = info->pdata->adc_table[low].temperature;
		goto convert_adc_to_temp_goto;
	} else if (info->pdata->adc_table[high].adc <= adc) {
		temp = info->pdata->adc_table[high].temperature;
		goto convert_adc_to_temp_goto;
	}

	while (low <= high) {
		mid = (low + high) / 2;
		if (info->pdata->adc_table[mid].adc > adc) {
			high = mid - 1;
		} else if (info->pdata->adc_table[mid].adc < adc) {
			low = mid + 1;
		} else {
			temp = info->pdata->adc_table[mid].temperature;
			goto convert_adc_to_temp_goto;
		}
	}

	temp = info->pdata->adc_table[high].temperature;

	temp2 = (info->pdata->adc_table[low].temperature -
			info->pdata->adc_table[high].temperature) *
			(adc - info->pdata->adc_table[high].adc);

	temp += temp2 /
		(info->pdata->adc_table[low].adc -
			info->pdata->adc_table[high].adc);

convert_adc_to_temp_goto:

	return temp;
}

#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
static int sec_therm_get_adc_data_flash_led(struct sec_therm_info *info)
{
	int rc = 0;
	int adc_max = 0;
	int adc_min = 0;
	int adc_total = 0;
	int i, adc_data;

	struct qpnp_vadc_result results;

	for (i = 0; i < ADC_SAMPLING_CNT; i++) {

		rc = qpnp_vadc_read(therm_vadc_dev, flash_thm_chan, &results);

		if (rc) {
			pr_err("error reading flash_thm %d, rc = %d\n", flash_thm_chan, rc);
			goto err;
		}
		adc_data = results.adc_code;

		if (i == 0) {
			pr_err("reading flash_thm[%d] [rc = %d] [adc_code = %d]\n", flash_thm_chan, rc, results.adc_code);
		}

		if (i != 0) {
			if (adc_data > adc_max)
				adc_max = adc_data;
			else if (adc_data < adc_min)
				adc_min = adc_data;
		} else {
			adc_max = adc_data;
			adc_min = adc_data;
		}

		adc_total += adc_data;
	}

	return (adc_total - adc_max - adc_min) / (ADC_SAMPLING_CNT - 2);

err:
	return rc;

}

static int convert_adc_flash_to_temper(struct sec_therm_info *info, unsigned int adc)
{
	int low = 0;
	int high = 0;
	int mid = 0;
	int temp = 0;
	int temp2 = 0;

	if (!info->pdata->adc_table_flash || !info->pdata->adc_flash_arr_size) {
		/* using fake temp */
		return 300;
	}

	high = info->pdata->adc_flash_arr_size - 1;

	if (info->pdata->adc_table_flash[low].adc >= adc) {
		temp = info->pdata->adc_table_flash[low].temperature;
		goto convert_adc_to_temp_goto;
	} else if (info->pdata->adc_table_flash[high].adc <= adc) {
		temp = info->pdata->adc_table_flash[high].temperature;
		goto convert_adc_to_temp_goto;
	}

	while (low <= high) {
		mid = (low + high) / 2;
		if (info->pdata->adc_table_flash[mid].adc > adc) {
			high = mid - 1;
		} else if (info->pdata->adc_table_flash[mid].adc < adc) {
			low = mid + 1;
		} else {
			temp = info->pdata->adc_table_flash[mid].temperature;
			goto convert_adc_to_temp_goto;
		}
	}

	temp = info->pdata->adc_table_flash[high].temperature;

	temp2 = (info->pdata->adc_table_flash[low].temperature -
			info->pdata->adc_table_flash[high].temperature) *
			(adc - info->pdata->adc_table_flash[high].adc);

	temp += temp2 /
		(info->pdata->adc_table_flash[low].adc -
			info->pdata->adc_table_flash[high].adc);

convert_adc_to_temp_goto:

	return temp;
}
#endif

static void notify_change_of_temperature(struct sec_therm_info *info)
{
	char temp_buf[20];
	char siop_buf[20];
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	char *envp[4];
#else
	char *envp[3];
#endif
	int env_offset = 0;
	int siop_level = -1;
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	char temp_buf_flash[20];

	snprintf(temp_buf, sizeof(temp_buf), "SUBTEMPERATURE=%d",
		 info->curr_temperature);
	envp[env_offset++] = temp_buf;

	snprintf(temp_buf_flash, sizeof(temp_buf_flash), "FLASH_TEMP=%d",
		 info->curr_temperature_flash_led);
	envp[env_offset++] = temp_buf_flash;
#else
	snprintf(temp_buf, sizeof(temp_buf), "TEMPERATURE=%d",
		 info->curr_temperature);
	envp[env_offset++] = temp_buf;
#endif

	if (info->pdata->get_siop_level)
		siop_level =
		    info->pdata->get_siop_level(info->curr_temperature);
	if (siop_level >= 0) {
		snprintf(siop_buf, sizeof(siop_buf), "SIOP_LEVEL=%d",
			 siop_level);
		envp[env_offset++] = siop_buf;
		dev_info(info->dev, "%s: uevent: %s\n", __func__, siop_buf);
	} else {
		envp[env_offset++] = NULL;
	}

	envp[env_offset] = NULL;

	dev_info(info->dev, "%s: siop_level=%d\n", __func__, siop_level);
	dev_info(info->dev, "%s: uevent: %s\n", __func__, temp_buf);
	kobject_uevent_env(&info->dev->kobj, KOBJ_CHANGE, envp);
}

static void sec_therm_polling_work(struct work_struct *work)
{
	struct sec_therm_info *info =
		container_of(work, struct sec_therm_info, polling_work.work);
	int adc;
	int temper;
#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	int temper_flash;
	int adc_flash;
#endif

	adc = sec_therm_get_adc_data(info);
	dev_info(info->dev, "%s: adc=%d\n", __func__, adc);

	if (adc < 0)
		goto out;

	temper = convert_adc_to_temper(info, adc);
	dev_info(info->dev, "%s: temper=%d\n", __func__, temper);

#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	adc_flash = sec_therm_get_adc_data_flash_led(info);
	dev_info(info->dev, "%s: adc_flash=%d\n", __func__, adc_flash);

	if (adc_flash < 0)
		goto out;

	temper_flash= convert_adc_flash_to_temper(info, adc_flash);
	dev_info(info->dev, "%s: temper_flash=%d\n", __func__, temper_flash);

	/* if temperature was changed, notify to framework */
	if (info->curr_temperature != temper || info->curr_temperature_flash_led!= temper_flash) {
		info->curr_temp_adc = adc;
		info->curr_temperature = temper;
		info->curr_temp_adc_flash_led = adc_flash;
		info->curr_temperature_flash_led = temper_flash;
		notify_change_of_temperature(info);
	}
#else
	/* if temperature was changed, notify to framework */
	if (info->curr_temperature != temper) {
		info->curr_temp_adc = adc;
		info->curr_temperature = temper;
		notify_change_of_temperature(info);
	}
#endif

out:
	schedule_delayed_work(&info->polling_work,
			msecs_to_jiffies(info->pdata->polling_interval));
}

static int sec_therm_probe(struct platform_device *pdev)
{
	struct sec_therm_platform_data *pdata = fill_therm_pdata(pdev);
	struct sec_therm_info *info;
	int ret = 0;
	int channel;

	dev_info(&pdev->dev, "%s: SEC Thermistor Driver Loading\n", __func__);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->dev = &pdev->dev;
	info->pdata = pdata;
	info->vadc_dev = qpnp_get_vadc(info->dev, "therm");
	therm_vadc_dev = info->vadc_dev;
	if (IS_ERR(info->vadc_dev)) {
		ret = PTR_ERR(info->vadc_dev);
		pr_err("%s:ret=%d\n",__func__,ret);
		if (ret != -EPROBE_DEFER)
			pr_err("vadc property missing\n");
		else
			goto err_therm;
	}

	ap_thm_chan = P_MUX4_1_1; //default ADC channel
	ret = of_property_read_u32(info->dev->of_node, "sec-therm,vadc-ap-chan", &channel);
	if (ret)
		pr_err("ap_thm invalid channel num, using default value\n");
	else
	{
		ap_thm_chan = channel;
		pr_info("ap_thm channel num is %d\n", ap_thm_chan);
	}

#ifdef CONFIG_SEC_USE_FLASH_THERMISTOR
	flash_thm_chan = LR_MUX2_BAT_ID; //default ADC channel
	ret = of_property_read_u32(info->dev->of_node, "sec-therm,vadc-flash-chan", &channel);
	if (ret)
		pr_err("flash_thm invalid channel num, using default value\n");
	else
	{
		flash_thm_chan = channel;
		pr_info("flash_thm channel num is %d\n", flash_thm_chan);
	}
#endif

	dev_set_drvdata(&pdev->dev, info);

	ret = sysfs_create_group(&info->dev->kobj, &sec_therm_group);
	if (ret) {
		dev_err(info->dev,
			"failed to create sysfs attribute group\n");
	}

	if (!(pdata->no_polling)) {
		INIT_DEFERRABLE_WORK(&info->polling_work,
			sec_therm_polling_work);
		schedule_delayed_work(&info->polling_work,
			msecs_to_jiffies(info->pdata->polling_interval));
	}
	return ret;

err_therm:
	kfree(info);
	return ret;
}

static int sec_therm_remove(struct platform_device *pdev)
{
	struct sec_therm_info *info = platform_get_drvdata(pdev);

	if (!info)
		return 0;

	sysfs_remove_group(&info->dev->kobj, &sec_therm_group);

	if (!(info->pdata->no_polling))
		cancel_delayed_work(&info->polling_work);
	kfree(info);

	return 0;
}

#ifdef CONFIG_PM
static int sec_therm_suspend(struct device *dev)
{
	struct sec_therm_info *info = dev_get_drvdata(dev);

	if (!(info->pdata->no_polling))
		cancel_delayed_work(&info->polling_work);

	return 0;
}

static int sec_therm_resume(struct device *dev)
{
	struct sec_therm_info *info = dev_get_drvdata(dev);

	if (!(info->pdata->no_polling))
		schedule_delayed_work(&info->polling_work,
			msecs_to_jiffies(info->pdata->polling_interval));
	return 0;
}
#else
#define sec_therm_suspend	NULL
#define sec_therm_resume	NULL
#endif /* CONFIG_PM */

static const struct dev_pm_ops sec_thermistor_pm_ops = {
	.suspend = sec_therm_suspend,
	.resume = sec_therm_resume,
};

#ifdef CONFIG_OF
static const struct of_device_id sec_therm_dt_match[] = {
	{ .compatible = "sec,thermistor" },
	{ }
};
MODULE_DEVICE_TABLE(of, sec_therm_dt_match);
#endif

static struct platform_driver sec_thermistor_driver = {
	.driver = {
		   .name = "sec-thermistor",
		   .owner = THIS_MODULE,
		   .pm = &sec_thermistor_pm_ops,
#ifdef CONFIG_OF
		   .of_match_table = sec_therm_dt_match,
#endif
	},
	.probe = sec_therm_probe,
	.remove = sec_therm_remove,
};

static int __init sec_therm_init(void)
{
	return platform_driver_register(&sec_thermistor_driver);
}
module_init(sec_therm_init);

static void __exit sec_therm_exit(void)
{
	platform_driver_unregister(&sec_thermistor_driver);
}
module_exit(sec_therm_exit);

MODULE_AUTHOR("ms925.kim@samsung.com");
MODULE_DESCRIPTION("sec thermistor driver");
MODULE_LICENSE("GPL");
