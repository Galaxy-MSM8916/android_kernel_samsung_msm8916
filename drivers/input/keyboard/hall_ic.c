#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/spinlock.h>
#include <linux/wakelock.h>
#include <linux/hall.h>

extern struct device *sec_key;
struct device *sec_device_create(void *drvdata, const char *fmt);

struct hall_drvdata {
	struct input_dev *input;
	int gpio_flip_cover;
	int irq_flip_cover;
#if defined(CONFIG_SENSORS_HALL_REAR)
	int gpio_flip_cover_rear;
	int irq_flip_cover_rear;
#endif
	struct work_struct work;
	struct delayed_work flip_cover_dwork;
	struct delayed_work flip_cover_rear_dwork;
	struct wake_lock flip_wake_lock;
};

static bool flip_cover = 1;
#if defined(CONFIG_SENSORS_HALL_REAR)
static bool flip_cover_rear = 0;
#endif
static ssize_t hall_detect_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (flip_cover) {
		sprintf(buf, "OPEN");
	} else {
		sprintf(buf, "CLOSE");
	}

	return strlen(buf);
}
static DEVICE_ATTR(hall_detect, 0444, hall_detect_show, NULL);
#if defined(CONFIG_SENSORS_HALL_REAR)
static ssize_t certify_hall_detect_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	if (flip_cover_rear) {
		sprintf(buf, "CLOSE");
	} else {
		sprintf(buf, "OPEN");
	}

	return strlen(buf);
}
static DEVICE_ATTR(certify_hall_detect, 0444, certify_hall_detect_show, NULL);
#endif
#ifdef CONFIG_SEC_FACTORY
static void flip_cover_work(struct work_struct *work)
{
	bool first,second;
	struct hall_drvdata *ddata =
		container_of(work, struct hall_drvdata,
				flip_cover_dwork.work);

	first = gpio_get_value(ddata->gpio_flip_cover);

	printk("keys:%s #1 : %d\n", __func__, first);

	msleep(50);

	second = gpio_get_value(ddata->gpio_flip_cover);

	printk("keys:%s #2 : %d\n", __func__, second);

	if(first == second) {
		flip_cover = first;
		input_report_switch(ddata->input, SW_FLIP, flip_cover);
		input_sync(ddata->input);
	}
}
#if defined(CONFIG_SENSORS_HALL_REAR)
static void flip_cover_rear_work(struct work_struct *work)
{
	bool first,second;
	struct hall_drvdata *ddata =
		container_of(work, struct hall_drvdata,
				flip_cover_rear_dwork.work);

	first = !gpio_get_value(ddata->gpio_flip_cover_rear);

	printk("keys:%s #1 : %d\n", __func__, first);

	msleep(50);

	second = !gpio_get_value(ddata->gpio_flip_cover_rear);

	printk("keys:%s #2 : %d\n", __func__, second);

	if(first == second) {
		flip_cover_rear = first;
		input_report_switch(ddata->input, SW_COVER_ATTACH, flip_cover_rear);
		input_sync(ddata->input);
	}
}
#endif
#else
static void flip_cover_work(struct work_struct *work)
{
	bool first;
	struct hall_drvdata *ddata =
		container_of(work, struct hall_drvdata,
				flip_cover_dwork.work);

	first = gpio_get_value(ddata->gpio_flip_cover);

	printk("keys:%s #1 : %d\n", __func__, first);

	flip_cover = first;
	input_report_switch(ddata->input,
			SW_FLIP, flip_cover);
	input_sync(ddata->input);
}
#if defined(CONFIG_SENSORS_HALL_REAR)
static void flip_cover_rear_work(struct work_struct *work)
{
	bool first;
	struct hall_drvdata *ddata =
		container_of(work, struct hall_drvdata,
				flip_cover_rear_dwork.work);

	first = !gpio_get_value(ddata->gpio_flip_cover_rear);

	printk("keys:%s #1 : %d\n", __func__, first);

	flip_cover_rear = first;
	input_report_switch(ddata->input,
			SW_COVER_ATTACH, flip_cover_rear);
	input_sync(ddata->input);
}
#endif
#endif

static void __flip_cover_detect(struct hall_drvdata *ddata, bool flip_status)
{
	cancel_delayed_work_sync(&ddata->flip_cover_dwork);
#ifdef CONFIG_SEC_FACTORY
	schedule_delayed_work(&ddata->flip_cover_dwork, HZ / 20);
#else
	if(flip_status)	{
		wake_lock_timeout(&ddata->flip_wake_lock, HZ * 5 / 100); /* 50ms */
		schedule_delayed_work(&ddata->flip_cover_dwork, HZ * 1 / 100); /* 10ms */
	} else {
		wake_unlock(&ddata->flip_wake_lock);
		schedule_delayed_work(&ddata->flip_cover_dwork, 0);
	}
#endif
}

#if defined(CONFIG_SENSORS_HALL_REAR)
static void __flip_cover_rear_detect(struct hall_drvdata *ddata, bool flip_status)
{
	cancel_delayed_work_sync(&ddata->flip_cover_rear_dwork);
#ifdef CONFIG_SEC_FACTORY
	schedule_delayed_work(&ddata->flip_cover_rear_dwork, HZ / 20);
#else
	if(flip_status)	{
		schedule_delayed_work(&ddata->flip_cover_rear_dwork, HZ * 1 / 100); /* 10ms */
	} else {
		schedule_delayed_work(&ddata->flip_cover_rear_dwork, 0);
	}
#endif
}
#endif

static irqreturn_t flip_cover_detect(int irq, void *dev_id)
{
	bool flip_status;
	struct hall_drvdata *ddata = dev_id;

	flip_status = gpio_get_value(ddata->gpio_flip_cover);

	printk(KERN_DEBUG "keys:%s flip_status : %d\n",
		 __func__, flip_status);

	__flip_cover_detect(ddata, flip_status);

	return IRQ_HANDLED;
}

#if defined(CONFIG_SENSORS_HALL_REAR)
static irqreturn_t flip_cover_rear_detect(int irq, void *dev_id)
{
	bool flip_status;
	struct hall_drvdata *ddata = dev_id;

	flip_status = !gpio_get_value(ddata->gpio_flip_cover_rear);

	printk(KERN_DEBUG "keys:%s flip_status : %d\n",
		 __func__, flip_status);

	__flip_cover_rear_detect(ddata, flip_status);

	return IRQ_HANDLED;
}
#endif

static int hall_open(struct input_dev *input)
{
	struct hall_drvdata *ddata = input_get_drvdata(input);
	/* update the current status */
	schedule_delayed_work(&ddata->flip_cover_dwork, HZ / 2);
	/* Report current state of buttons that are connected to GPIOs */
	input_sync(input);

	return 0;
}

static void hall_close(struct input_dev *input)
{
}


static void init_hall_ic_irq(struct input_dev *input)
{
	struct hall_drvdata *ddata = input_get_drvdata(input);

	int ret = 0;
	int irq = ddata->irq_flip_cover;
#if defined(CONFIG_SENSORS_HALL_REAR)
	int irq_rear = ddata->irq_flip_cover_rear;
#endif

	flip_cover = gpio_get_value(ddata->gpio_flip_cover);
#if defined(CONFIG_SENSORS_HALL_REAR)
	flip_cover_rear = !gpio_get_value(ddata->gpio_flip_cover_rear);
#endif

	INIT_DELAYED_WORK(&ddata->flip_cover_dwork, flip_cover_work);
#if defined(CONFIG_SENSORS_HALL_REAR)
	INIT_DELAYED_WORK(&ddata->flip_cover_rear_dwork, flip_cover_rear_work);
#endif

	ret =
		request_threaded_irq(
		irq, NULL,
		flip_cover_detect,
		IRQF_DISABLED | IRQF_TRIGGER_RISING |
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"flip_cover", ddata);
	if (ret < 0) {
		printk(KERN_ERR
		"keys: failed to request flip cover irq %d gpio %d\n",
		irq, ddata->gpio_flip_cover);
	} else {
		pr_info("%s : success\n", __func__);
	}

#if defined(CONFIG_SENSORS_HALL_REAR)
	ret =
		request_threaded_irq(
		irq_rear, NULL,
		flip_cover_rear_detect,
		IRQF_DISABLED | IRQF_TRIGGER_RISING |
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"flip_cover_rear", ddata);
	if (ret < 0) {
		printk(KERN_ERR
		"keys: failed to request flip cover irq %d gpio %d\n",
		irq, ddata->gpio_flip_cover);
	} else {
		pr_info("%s : success\n", __func__);
	}
#endif
}

#ifdef CONFIG_OF
static int of_hall_data_parsing_dt(struct device *dev,struct hall_drvdata *ddata)
{
	struct device_node *np_haptic= dev->of_node;
	int gpio;
	enum of_gpio_flags flags;

	gpio = of_get_named_gpio_flags(np_haptic, "hall,gpio_flip_cover", 0, &flags);
	ddata->gpio_flip_cover = gpio;

	gpio = gpio_to_irq(gpio);
	ddata->irq_flip_cover = gpio;

#if defined(CONFIG_SENSORS_HALL_REAR)
	gpio = of_get_named_gpio_flags(np_haptic, "hall,gpio_flip_cover_rear", 0, &flags);
	ddata->gpio_flip_cover_rear = gpio;

	gpio = gpio_to_irq(gpio);
	ddata->irq_flip_cover_rear = gpio;
#endif
	return 0;
}
#endif

static int hall_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hall_drvdata *ddata;
	struct input_dev *input;
	struct pinctrl *hall_pinctrl;
	int error;
	int wakeup = 0;
	printk(KERN_CRIT "%s called", __func__);
	ddata = kzalloc(sizeof(struct hall_drvdata), GFP_KERNEL);
	if (!ddata) {
		dev_err(dev, "failed to allocate state\n");
		return -ENOMEM;
	}

#ifdef CONFIG_OF
	if(dev->of_node) {
		error = of_hall_data_parsing_dt(dev, ddata);
		if (error < 0) {
			pr_info("%s : fail to get the dt (HALL)\n", __func__);
			goto fail1;
		}
		hall_pinctrl = devm_pinctrl_get_select(dev, "pmx_hall_ic_pin");
		if (IS_ERR(hall_pinctrl)) {
			if (PTR_ERR(hall_pinctrl) == -EPROBE_DEFER){
				kfree(ddata);
				return -EPROBE_DEFER;
			}
			pr_debug("Target does not use pinctrl\n");
			hall_pinctrl = NULL;
		}
	}
#endif

	input = input_allocate_device();
	if (!input) {
		dev_err(dev, "failed to allocate state\n");
		error = -ENOMEM;
		goto fail1;
	}

	ddata->input = input;

	wake_lock_init(&ddata->flip_wake_lock, WAKE_LOCK_SUSPEND,
		"flip wake lock");

	platform_set_drvdata(pdev, ddata);
	input_set_drvdata(input, ddata);

	input->name = "hall";
	input->phys = "hall";
	input->dev.parent = &pdev->dev;

	input->evbit[0] |= BIT_MASK(EV_SW);

	input_set_capability(input, EV_SW, SW_FLIP);
#if defined(CONFIG_SENSORS_HALL_REAR)
	input_set_capability(input, EV_SW, SW_COVER_ATTACH);
#endif
	input->open = hall_open;
	input->close = hall_close;

	/* Enable auto repeat feature of Linux input subsystem */
	__set_bit(EV_REP, input->evbit);

	init_hall_ic_irq(input);

	if(ddata->gpio_flip_cover != 0) {
                error = device_create_file(sec_key, &dev_attr_hall_detect);
                if (error < 0) {
                        pr_err("Failed to create device file(%s)!, error: %d\n",
                                dev_attr_hall_detect.attr.name,error);
                }
        }

#if defined(CONFIG_SENSORS_HALL_REAR)
	if(ddata->gpio_flip_cover_rear != 0) {
                error = device_create_file(sec_key, &dev_attr_certify_hall_detect);
                if (error < 0) {
                        pr_err("Failed to create device file(%s)!, error: %d\n",
                                dev_attr_certify_hall_detect.attr.name, error);
                }
        }
#endif
	error = input_register_device(input);
	if (error) {
		dev_err(dev, "Unable to register input device, error: %d\n",
			error);
		goto fail1;
	}

	device_init_wakeup(&pdev->dev, wakeup);

	printk(KERN_CRIT "%s end", __func__);
	return 0;

 fail1:
	kfree(ddata);

	return error;
}

static int hall_remove(struct platform_device *pdev)
{
	struct hall_drvdata *ddata = platform_get_drvdata(pdev);
	struct input_dev *input = ddata->input;

	printk("%s start\n", __func__);

	device_init_wakeup(&pdev->dev, 0);

	input_unregister_device(input);

	wake_lock_destroy(&ddata->flip_wake_lock);

	kfree(ddata);

	return 0;
}

#if defined(CONFIG_OF)
static struct of_device_id hall_dt_ids[] = {
	{ .compatible = "hall" },
	{ },
};
MODULE_DEVICE_TABLE(of, hall_dt_ids);
#endif /* CONFIG_OF */

#ifdef CONFIG_PM_SLEEP
static int hall_suspend(struct device *dev)
{
	struct hall_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;

	printk("%s start\n", __func__);

	enable_irq_wake(ddata->irq_flip_cover);

	if (device_may_wakeup(dev)) {
		enable_irq_wake(ddata->irq_flip_cover);
	} else {
		mutex_lock(&input->mutex);
		if (input->users)
			hall_close(input);
		mutex_unlock(&input->mutex);
	}

	return 0;
}

static int hall_resume(struct device *dev)
{
	struct hall_drvdata *ddata = dev_get_drvdata(dev);
	struct input_dev *input = ddata->input;

	printk("%s start\n", __func__);
	input_sync(input);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(hall_pm_ops, hall_suspend, hall_resume);

static struct platform_driver hall_device_driver = {
	.probe		= hall_probe,
	.remove		= hall_remove,
	.driver		= {
		.name	= "hall",
		.owner	= THIS_MODULE,
		.pm	= &hall_pm_ops,
#if defined(CONFIG_OF)
		.of_match_table	= hall_dt_ids,
#endif /* CONFIG_OF */
	}
};

static int __init hall_init(void)
{
	printk("%s start\n", __func__);
	return platform_driver_register(&hall_device_driver);
}

static void __exit hall_exit(void)
{
	printk("%s start\n", __func__);
	platform_driver_unregister(&hall_device_driver);
}

late_initcall(hall_init);
module_exit(hall_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phil Blundell <pb@handhelds.org>");
MODULE_DESCRIPTION("Keyboard driver for GPIOs");
MODULE_ALIAS("platform:gpio-keys");
