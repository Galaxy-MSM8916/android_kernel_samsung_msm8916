#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

struct gpio_sett {
	struct platform_device *pdev;

	struct pinctrl *ts_pinctrl;
	struct pinctrl_state *gpio_state_active;
	struct pinctrl_state *gpio_state_suspend;
};

static int pinctrl_select(struct gpio_sett *data, bool on)
{
	int ret;
	struct pinctrl_state *pins_state;

	pins_state = on ? data->gpio_state_active
		: data->gpio_state_suspend;
	if (!IS_ERR_OR_NULL(pins_state)) {
		ret = pinctrl_select_state(data->ts_pinctrl, pins_state);
		if (ret) {
			dev_err(&data->pdev->dev,
				"can not set %s pins\n",
				on ? "sec_gpio_active" : "sec_gpio_suspend");
			return ret;
		}
	} else
		dev_err(&data->pdev->dev,
			"not a valid '%s' pinstate\n",
				on ? "sec_gpio_active" : "sec_gpio_suspend");

	return 0;
}

static int pinctrl_init(struct gpio_sett *data)
{
	int retval;


	data->ts_pinctrl = devm_pinctrl_get(&(data->pdev->dev));
	if (IS_ERR_OR_NULL(data->ts_pinctrl)) {
		dev_dbg(&data->pdev->dev,
			"Target does not use pinctrl\n");
		retval = PTR_ERR(data->ts_pinctrl);
		data->ts_pinctrl = NULL;
		return retval;
	}

	data->gpio_state_active
		= pinctrl_lookup_state(data->ts_pinctrl, "sec_gpio_active");
	if (IS_ERR_OR_NULL(data->gpio_state_active)) {
		dev_dbg(&data->pdev->dev,
			"Can not get ts default pinstate\n");
		retval = PTR_ERR(data->gpio_state_active);
		data->ts_pinctrl = NULL;
		return retval;
	}

	data->gpio_state_suspend
		= pinctrl_lookup_state(data->ts_pinctrl, "sec_gpio_suspend");
	if (IS_ERR_OR_NULL(data->gpio_state_suspend)) {
		dev_dbg(&data->pdev->dev,
			"Can not get ts sleep pinstate\n");
		retval = PTR_ERR(data->gpio_state_suspend);
		data->ts_pinctrl = NULL;
		return retval;
	}

	return 0;
}

static int gpio_sett_probe(struct platform_device *pdev)
{
	struct gpio_sett *data;
	int ret;

	data = kzalloc(sizeof(struct gpio_sett), GFP_KERNEL);
	if (!data) {
		dev_err(&pdev->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}
	data->pdev = pdev;
	dev_set_drvdata(&pdev->dev, data);

	ret = pinctrl_init(data);
	if (!ret && data->ts_pinctrl) {
		ret = pinctrl_select(data, true);
		if (ret < 0)
			goto err_gpio_config;
	}

	return 0;

err_gpio_config:
	kfree(data);
	return ret;
}

static int gpio_sett_resume(struct device *dev)
{
	struct gpio_sett *data = dev_get_drvdata(dev);
	int ret;

	if (data->ts_pinctrl) {
		ret = pinctrl_select(data, true);
		if (ret < 0)
			dev_err(dev, "Cannot get idle pinctrl state\n");
	}

	return 0;
}

static int gpio_sett_suspend(struct device *dev)
{
	struct gpio_sett *data = dev_get_drvdata(dev);
	int ret;

	if (data->ts_pinctrl) {
		ret = pinctrl_select(data, false);
		if (ret < 0)
			dev_err(dev, "Cannot get default pinctrl state\n");
	}

	return 0;
}

static SIMPLE_DEV_PM_OPS(gpio_sett_pm_ops, gpio_sett_suspend, gpio_sett_resume);

static const struct of_device_id gpio_sett_match[] = {
	{	.compatible = "sec_gpio_sett",
	},
	{}
};

static int gpio_sett_remove(struct platform_device *pdev)
{
	int ret;
	struct gpio_sett *data = dev_get_drvdata(&pdev->dev);

	if (data->ts_pinctrl) {
		ret = pinctrl_select(data, false);
		if (ret < 0)
			dev_err(&pdev->dev, "Cannot get default pinctrl state\n");
	}

	kfree(data);
	return 0;
}

static struct platform_driver gpio_settings_platdrv = {
	.driver =
	{
		.name = "sec_gpio_sett",
		.owner = THIS_MODULE,
		.of_match_table = gpio_sett_match,
		.pm	= &gpio_sett_pm_ops,
	},
	.probe = gpio_sett_probe,
	.remove = gpio_sett_remove,
};

static int __init gpio_sett_i2c_init(void)
{
	return platform_driver_register(&gpio_settings_platdrv);
}

static void __exit gpio_sett_i2c_exit(void)
{
	platform_driver_unregister(&gpio_settings_platdrv);
}

module_init(gpio_sett_i2c_init);
module_exit(gpio_sett_i2c_exit);
MODULE_DESCRIPTION("Samsung gpio settings driver");
MODULE_AUTHOR("Chaitanya Vadrevu <v.chaitanya@samsung.com>");
MODULE_LICENSE("GPL");
