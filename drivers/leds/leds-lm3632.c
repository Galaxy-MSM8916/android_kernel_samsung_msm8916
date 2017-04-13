/*
 * TI LM3632 Backlight Driver
 *
 * Copyright (C) 2015 Texas Instruments
 *
 * Author: Daniel Jeong <daniel.jeong@ti.com>
 *			Ldd-Mlp <ldd-mlp@list.ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/mfd/lm3632.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <linux/of_gpio.h>

#define FLED_PINCTRL_STATE_DEFAULT "front_fled_default"
#define FLED_PINCTRL_STATE_SLEEP "front_fled_sleep"

/* operation mode */
enum led_opmode {
	OPMODE_SHDN = 0x00,
	OPMODE_TORCH = 0x01,
	OPMODE_FLASH = 0x03,
};

/*
 * register data
 * @reg : register
 * @mask : mask bits
 * @shift : bit shift of data
 */
struct ctrl_reg {
	unsigned int reg;
	unsigned int mask;
	unsigned int shift;
};

/*
 * unit data
 * @min : min value of brightness or timeout
 *        brightness : uA
 *		  timeout    : ms
 * @step : step value of brightness or timeout
 *        brightness : uA
 *		  timeout    : ms
 * @knee: knee point of step of brightness/timeout
 *        brightness : uA
 *		  timeout    : ms
 * @knee_step : step value of brightness or timeout after knee point
 *        brightness : uA
 *		  timeout    : ms
 * @max : max value of brightness or timeout
 *        brightness : uA
 *		  timeout    : ms
 * @ctrl : register info to control brightness or timeout
 */
struct ssflash_config {
	unsigned int min;
	unsigned int step;
	unsigned int knee;
	unsigned int knee_step;
	unsigned int max;
	struct ctrl_reg ctrl;
};

/*
 * @reg : fault register
 * @mask : fault mask bit
 * @v4l2_fault : bit mapping to V4L2_FLASH_FAULT_
 *               refer to include//uapi/linux/v4l2-controls.h
 */
struct ssflash_fault {
	unsigned int reg;
	unsigned int mask;
	unsigned int v4l2_fault;
};

#define NUM_V4L2_FAULT 9

/*
 * ssflash data
 * @name: device name
 * @mode: operation mode control data
 * @flash_br: flash brightness register and bit data
 * @timeout: timeout control data
 * @strobe: strobe control data
 * @torch_br: torch brightness register and bit data
 * @fault: fault data
 * @func: initialize function
 */
struct ssflash_data {
	char *name;
	struct ctrl_reg mode;
	struct ssflash_config flash_br;
	struct ssflash_config timeout;
	struct ctrl_reg strobe;

	struct ssflash_config torch_br;
	struct ssflash_fault fault[NUM_V4L2_FAULT];

	int (*func)(struct lm3632 *lm3632);
};

/*
 * struct ssflash_flash
 * @dev: device
 * @ctrls_led: V4L2 contols
 * @subdev_led: V4L2 subdev
 * @data : chip control data
 */
struct ssflash_flash {
	struct device *dev;
	struct lm3632 *lm3632;
	struct v4l2_ctrl_handler ctrls_led;
	struct v4l2_subdev subdev_led;
	const struct ssflash_data *data;
};


struct lm3632_ffl {
	struct device *dev;
	struct lm3632 *lm3632;
	struct lm3632_flash_platform_data *pdata;
};

struct flash_info {
	struct i2c_client *client;
	struct lm3632_flash_platform_data	*pdata;
	struct lm3632 *lm3632;
};

struct flash_info *ffl_info;
static const char *ffl_name;

extern struct class *camera_class;
extern struct device *flash_dev;

struct lm3632_flash_platform_data *pdata;
struct lm3632_ffl *lm3632_ffl;
struct flash_info *info;
void ssflash_led_turn_on(void);
void ssflash_led_turn_off(void);



#define to_ssflash_flash(_ctrl)	\
	container_of(_ctrl->handler, struct ssflash_flash, ctrls_led)

static int ssflash_lm3632_init(struct lm3632 *lm3632)
{
	int rval;
	/* disable flash enable  */
	rval = lm3632_update_bits(lm3632, 0x0a, 0x06, 0x00);
	/* torch 150mA, flash 200mA */
	rval |= lm3632_update_bits(lm3632, 0x06, 0xff, 0x51);
	return rval;
}

static const struct ssflash_data flash_lm3632 = {
	.name = "lm3632_flash",
	.mode = {
		.reg = 0x0a, .mask = 0x06, .shift = 1
	},
	.flash_br = {
		.min = 100000, .step = 100000, .max = 1500000,
		.ctrl = {
			.reg = 0x06, .mask = 0x0f, .shift = 0
		},
	},
	.timeout = {
		.min = 32,	.step = 32, .max = 1024,
		.ctrl = {
			.reg = 0x07, .mask = 0x1f, .shift = 0
		}
	},
	.torch_br = {
		.min = 25000, .step = 25000, .max = 375000,
		.ctrl = {
			.reg = 0x06, .mask = 0xf0, .shift = 4
		}
	},
	.strobe = {
		.reg = 0x09, .mask = 0x10, .shift = 4
	},
	.fault = {
		[0] = {
			.reg = 0x0b, .mask = 0x02,
			.v4l2_fault = V4L2_FLASH_FAULT_TIMEOUT,
		},
		[1] = {
			.reg = 0x0b, .mask = 0x02,
			.v4l2_fault = V4L2_FLASH_FAULT_OVER_TEMPERATURE,
		},
		[2] = {
			.reg = 0x0b, .mask = 0x04,
			.v4l2_fault = V4L2_FLASH_FAULT_SHORT_CIRCUIT,
		},
		[3] = {
			.reg = 0x0b, .mask = 0x08,
			.v4l2_fault = V4L2_FLASH_FAULT_OVER_VOLTAGE,
		},
	},
	.func = ssflash_lm3632_init
};

static u8 ssflash_conv_val_to_reg(unsigned int val,
	unsigned int min, unsigned int step,
	unsigned int knee, unsigned int knee_step,
	unsigned int max)
{
	if (val >= max)
		return 0xff;

	if (knee <= min)
		return (u8)(val < min ? 0 : (val - min)/step);
	return (u8)val < min ? 0 :
			(val < knee ? (val-min)/step :
				(val-knee)/knee_step + (knee-min)/step);
}

/* mode control */
static int ssflash_mode_ctrl(struct ssflash_flash *flash,
			    enum v4l2_flash_led_mode v4l_led_mode)
{
	const struct ssflash_data *data = flash->data;
	enum led_opmode opmode;

	switch (v4l_led_mode) {
	case V4L2_FLASH_LED_MODE_NONE:
		opmode = OPMODE_SHDN;
	break;
	case V4L2_FLASH_LED_MODE_TORCH:
		opmode = OPMODE_TORCH;
	break;
	case V4L2_FLASH_LED_MODE_FLASH:
		opmode = OPMODE_FLASH;
	break;
	default:
		return -EINVAL;
	}

	return lm3632_update_bits(flash->lm3632, data->mode.reg,
				data->mode.mask, opmode << data->mode.shift);
}

/* torch brightness control */
static int ssflash_torch_brt_ctrl
	(struct ssflash_flash *flash, unsigned int brt)
{
	const struct ssflash_data *data = flash->data;
	int rval;
	u8 br_bits;

	br_bits = ssflash_conv_val_to_reg(brt,
				data->torch_br.min, data->torch_br.step,
				data->torch_br.knee, data->torch_br.knee_step,
				data->torch_br.max);

	rval = lm3632_update_bits(flash->lm3632,
			data->torch_br.ctrl.reg,
			data->torch_br.ctrl.mask,
			br_bits << data->torch_br.ctrl.shift);

	return rval;
}

/* flash brightness control */
static int ssflash_flash_brt_ctrl(struct ssflash_flash *flash,
				 unsigned int brt)
{
	const struct ssflash_data *data = flash->data;
	int rval;
	u8 br_bits;

	br_bits = ssflash_conv_val_to_reg(brt,
					data->flash_br.min,
					data->flash_br.step,
					data->flash_br.knee,
					data->flash_br.knee_step,
					data->flash_br.max);

	rval = lm3632_update_bits(flash->lm3632,
				data->flash_br.ctrl.reg,
				data->flash_br.ctrl.mask,
				br_bits << data->flash_br.ctrl.shift);

	return rval;
}

/* fault status */
static int ssflash_get_ctrl(struct v4l2_ctrl *ctrl)
{
	return -EINVAL;
}

static int ssflash_set_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ssflash_flash *flash = to_ssflash_flash(ctrl);
	const struct ssflash_data *data = flash->data;
	u8 tout_bits, reg_val;
	int rval = -EINVAL;

	switch (ctrl->id) {
	case V4L2_CID_FLASH_LED_MODE:
		if (ctrl->val != V4L2_FLASH_LED_MODE_FLASH)
			return ssflash_mode_ctrl(flash, ctrl->val);
		/* switch to SHDN mode before flash strobe on */
		rval = ssflash_mode_ctrl(flash, V4L2_FLASH_LED_MODE_NONE);
		rval |= ssflash_mode_ctrl(flash, ctrl->val);
		return rval;

	case V4L2_CID_FLASH_TORCH_INTENSITY:
		return ssflash_torch_brt_ctrl(flash, ctrl->val);

	case V4L2_CID_FLASH_INTENSITY:
		return ssflash_flash_brt_ctrl(flash, ctrl->val);

	case V4L2_CID_FLASH_STROBE_SOURCE:
		return lm3632_update_bits(flash->lm3632,
					data->strobe.reg, data->strobe.mask,
					(ctrl->val) << data->strobe.shift);

	case V4L2_CID_FLASH_STROBE:
		/* read and check current mode of chip to start flash */
		rval = lm3632_read_byte(flash->lm3632,
						data->mode.reg, &reg_val);
		if (rval < 0 ||
				(((reg_val & data->mode.mask)>>data->mode.shift)
				!= OPMODE_SHDN))
			return rval;
		/* flash on */
		return ssflash_mode_ctrl(flash,
					V4L2_FLASH_LED_MODE_FLASH);

	case V4L2_CID_FLASH_STROBE_STOP:
		/*
		 * flash mode will be turned automatically
		 * from FLASH mode to SHDN mode after flash duration timeout
		 * read and check current mode of chip to stop flash
		 */
		rval = lm3632_read_byte(flash->lm3632,
						data->mode.reg, &reg_val);
		if (rval < 0)
			return rval;
		if (((reg_val & data->mode.mask)
				>> data->mode.shift) == OPMODE_FLASH)
			return ssflash_mode_ctrl(flash,
						V4L2_FLASH_LED_MODE_NONE);
		return rval;

	case V4L2_CID_FLASH_TIMEOUT:
		tout_bits = ssflash_conv_val_to_reg(ctrl->val,
						data->timeout.min,
						data->timeout.step,
						data->timeout.knee,
						data->timeout.knee_step,
						data->timeout.max);

		rval = lm3632_update_bits(flash->lm3632,
					data->timeout.ctrl.reg,
					data->timeout.ctrl.mask,
					tout_bits << data->timeout.ctrl.shift);
	}
	return rval;
}

static int flash_parse_dt(struct device *dev,
			struct lm3632_flash_platform_data *pdata)
{
	struct device_node *np = dev->of_node;

	pdata->gpio_flash_en = of_get_named_gpio_flags(np, "flash-en-gpio",
				0, &pdata->flash_en_gpio_flags);
	pdata->gpio_flash_int = of_get_named_gpio_flags(np, "flash-int-gpio",
				0, &pdata->flash_int_gpio_flags);
	ffl_name = of_get_property(np, "fflash-name", NULL);
	if (!ffl_name) {
		pr_err("%s:%d, flash name not specified\n",__func__, __LINE__);
	} else {
		pr_err("%s: flash Name = %s\n", __func__,ffl_name);
	}
	return 0;
}


int flash_request_gpio(struct lm3632_flash_platform_data *pdata)
{
	int ret;
	if(!pdata){
		printk("flash_Request_gpio pdata is null!!\n");
		return -EINVAL;
	}

	if(!pdata->gpio_flash_en || !pdata->gpio_flash_int){
		printk("flash_Request_gpio is null!!\n");
		return -EINVAL;
	}

	if (gpio_is_valid(pdata->gpio_flash_en)) {
		ret = gpio_request(pdata->gpio_flash_en, "flash-en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_flash_en [%d]\n",
				__func__, pdata->gpio_flash_en);
		}
	}
	if (gpio_is_valid(pdata->gpio_flash_int)) {
		ret = gpio_request(pdata->gpio_flash_int, "flash-int");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_flash_int [%d]\n",
				__func__, pdata->gpio_flash_int);
		}
	}
	return 0;
}EXPORT_SYMBOL_GPL(flash_request_gpio);


static int ssflash_led_get_ctrl(struct v4l2_ctrl *ctrl)
{
	return ssflash_get_ctrl(ctrl);
}

static int ssflash_led_set_ctrl(struct v4l2_ctrl *ctrl)
{
	return ssflash_set_ctrl(ctrl);
}

static const struct v4l2_ctrl_ops ssflash_led_ctrl_ops = {
	 .g_volatile_ctrl = ssflash_led_get_ctrl,
	 .s_ctrl = ssflash_led_set_ctrl,
};

void ssflash_led_turn_on()
{
	int ret = 0;
	ret = flash_request_gpio(pdata);
	if(ret < 0){
		printk("Need to not to use ssflash_led_turn_on");
		return;
	}

	pr_err("ssflash_led_turn_on \n");

	if (gpio_is_valid(info->pdata->gpio_flash_int)) {
		gpio_set_value(info->pdata->gpio_flash_int,1);
		gpio_direction_output(info->pdata->gpio_flash_int,1);
	}
	msleep(5);

	lm3632_update_bits(info->lm3632, 0x06, 0xF0, 0x10);
	lm3632_update_bits(info->lm3632, 0x09, 0x10, 0x10);
	lm3632_update_bits(info->lm3632, 0x0A, 0x06, 0x02);

	if (gpio_is_valid(info->pdata->gpio_flash_en)){
		gpio_set_value(info->pdata->gpio_flash_en,1);
		gpio_direction_output(info->pdata->gpio_flash_en,1);
	}
	msleep(5);

}EXPORT_SYMBOL_GPL(ssflash_led_turn_on);

void ssflash_led_turn_off()
{
	int ret = 0;
	ret = flash_request_gpio(pdata);
	if(ret < 0){
		printk("Need to not to use ssflash_led_turn_on");
		return;
	}

	pr_err("ssflash_led_turn_off\n");

	if (gpio_is_valid(info->pdata->gpio_flash_int)) {
		gpio_set_value(info->pdata->gpio_flash_int,0);
		gpio_direction_output(info->pdata->gpio_flash_int,0);
	}
	msleep(5);

	if (gpio_is_valid(info->pdata->gpio_flash_en)){
		gpio_set_value(info->pdata->gpio_flash_en,0);
		gpio_direction_output(info->pdata->gpio_flash_en,0);
	}
	msleep(5);
	gpio_free(pdata->gpio_flash_en);
	gpio_free(pdata->gpio_flash_int);

}EXPORT_SYMBOL_GPL(ssflash_led_turn_off);


static int ssflash_init_controls(struct ssflash_flash *flash)
{
	struct v4l2_ctrl *fault;
	struct v4l2_ctrl_handler *hdl = &flash->ctrls_led;
	const struct v4l2_ctrl_ops *ops = &ssflash_led_ctrl_ops;
	const struct ssflash_data *data = flash->data;
	s64 fault_max = 0;
	int icnt;

	v4l2_ctrl_handler_init(hdl, 8);
	/* flash mode */
	v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_FLASH_LED_MODE,
			       V4L2_FLASH_LED_MODE_TORCH, ~0x7,
			       V4L2_FLASH_LED_MODE_NONE);
	/* flash source */
	v4l2_ctrl_new_std_menu(hdl, ops, V4L2_CID_FLASH_STROBE_SOURCE,
			       0x1, ~0x3, V4L2_FLASH_STROBE_SOURCE_SOFTWARE);
	/* flash strobe */
	v4l2_ctrl_new_std(hdl, ops, V4L2_CID_FLASH_STROBE, 0, 0, 0, 0);
	/* flash strobe stop */
	v4l2_ctrl_new_std(hdl, ops, V4L2_CID_FLASH_STROBE_STOP, 0, 0, 0, 0);
	/* flash strobe timeout */
	v4l2_ctrl_new_std(hdl, ops, V4L2_CID_FLASH_TIMEOUT,
			  data->timeout.min,
			  data->timeout.max,
			  data->timeout.step,
			  data->timeout.max);
	/* flash brt */
	v4l2_ctrl_new_std(hdl, ops, V4L2_CID_FLASH_INTENSITY,
			  data->flash_br.min,
			  data->flash_br.max,
			  data->flash_br.step,
			  data->flash_br.max);
	/* torch brt */
	v4l2_ctrl_new_std(hdl, ops, V4L2_CID_FLASH_TORCH_INTENSITY,
			  data->torch_br.min,
			  data->torch_br.max,
			  data->torch_br.step,
			  data->torch_br.max);
	/* fault */
	for (icnt = 0; icnt < NUM_V4L2_FAULT; icnt++)
		fault_max |= data->fault[icnt].v4l2_fault;
	fault = v4l2_ctrl_new_std(hdl,
				ops, V4L2_CID_FLASH_FAULT, 0, fault_max, 0, 0);
	if (fault != NULL)
		fault->flags |= V4L2_CTRL_FLAG_VOLATILE;

	if (hdl->error)
		return hdl->error;

	flash->subdev_led.ctrl_handler = hdl;
	return 0;
}

/* initialize device */
static const struct v4l2_subdev_ops ssflash_ops = {
	.core = NULL,
};

static int ssflash_subdev_init(struct ssflash_flash *flash)
{
	int rval;

	v4l2_subdev_init(&flash->subdev_led, &ssflash_ops);
	flash->subdev_led.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	strcpy(flash->subdev_led.name, flash->data->name);

	rval = ssflash_init_controls(flash);
	if (rval)
		goto err_out;

	rval = media_entity_init(&flash->subdev_led.entity, 0, NULL, 0);
	if (rval < 0)
		goto err_out;

	flash->subdev_led.entity.type = MEDIA_ENT_T_V4L2_SUBDEV_FLASH;

	return rval;

err_out:
	v4l2_ctrl_handler_free(&flash->ctrls_led);
	return rval;
}

static int ssflash_init_device(struct ssflash_flash *flash)
{
	int rval;

	/* output disable */
	rval = ssflash_mode_ctrl(flash, V4L2_FLASH_LED_MODE_NONE);
	if (rval < 0)
		return rval;

	if (flash->data->func != NULL) {
		rval = flash->data->func(flash->lm3632);
		if (rval < 0)
			return rval;
	}

	return rval;
}

static ssize_t flash_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int i, nValue=0, ret =0;

	for(i = 0; i < count; i++) {
		if(buf[i] < '0' || buf[i] > '9')
			break;
		nValue = nValue * 10 + (buf[i] - '0');
	}

	pr_err("%s gpio flash_en(%u) gpio_flash_int(%u) \n",__func__,
			ffl_info->pdata->gpio_flash_en, ffl_info->pdata->gpio_flash_int);
	if (gpio_is_valid(ffl_info->pdata->gpio_flash_en)) {
		ret = gpio_request(ffl_info->pdata->gpio_flash_en, "flash-en");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_flash_en [%d]\n",
					__func__, ffl_info->pdata->gpio_flash_en);
			//      return;
		}
	}
	if (gpio_is_valid(ffl_info->pdata->gpio_flash_int)) {
		ret = gpio_request(ffl_info->pdata->gpio_flash_int, "flash-int");
		if (ret) {
			printk(KERN_ERR "%s: unable to request gpio_flash_int [%d]\n",
					__func__, ffl_info->pdata->gpio_flash_int);
			//      return;
		}
	}

	switch(nValue) {
		case 0:
			pr_err("Torch OFF\n");
			if (gpio_is_valid(ffl_info->pdata->gpio_flash_int)) {
				gpio_direction_output(ffl_info->pdata->gpio_flash_int,0);
			}
			if (gpio_is_valid(ffl_info->pdata->gpio_flash_en)){
				gpio_direction_output(ffl_info->pdata->gpio_flash_en,0);
			}
			gpio_free(pdata->gpio_flash_int);
			gpio_free(pdata->gpio_flash_en);
			break;

		case 1:
			pr_err("Torch ON\n");
			if (gpio_is_valid(ffl_info->pdata->gpio_flash_int)) {
				gpio_direction_output(ffl_info->pdata->gpio_flash_int,1);
			}

			lm3632_write_byte(ffl_info->lm3632, 0x06, 0x10);
			lm3632_write_byte(ffl_info->lm3632, 0x09, 0x51);
			lm3632_write_byte(ffl_info->lm3632, 0x0A, 0x1B);

			if (gpio_is_valid(ffl_info->pdata->gpio_flash_en)){
				gpio_direction_output(ffl_info->pdata->gpio_flash_en,1);
			}
			gpio_free(pdata->gpio_flash_en);
			gpio_free(pdata->gpio_flash_int);
			break;

		case 100:
			pr_err("Torch ON-F\n");
			if (gpio_is_valid(ffl_info->pdata->gpio_flash_int)) {
				gpio_direction_output(ffl_info->pdata->gpio_flash_int,1);
			}

			lm3632_write_byte(ffl_info->lm3632, 0x06, 0x50);
			lm3632_write_byte(ffl_info->lm3632, 0x09, 0x51);
			lm3632_write_byte(ffl_info->lm3632, 0x0A, 0x1B);

			if (gpio_is_valid(ffl_info->pdata->gpio_flash_en)){
				gpio_direction_output(ffl_info->pdata->gpio_flash_en,1);
			}
			gpio_free(pdata->gpio_flash_en);
			gpio_free(pdata->gpio_flash_int);
			break;

		default:
			pr_err("Torch NC:%d\n", nValue);
			break;
	}
	return count;
}

static DEVICE_ATTR(front_flash, S_IWUSR|S_IWGRP, NULL, flash_store);

int create_front_flash_sysfs(void)
{
    int err = -ENODEV;

    if (IS_ERR_OR_NULL(camera_class)) {
        pr_err("flash_sysfs: error, camera class not exist");
        return -ENODEV;
    }

    if (IS_ERR_OR_NULL(flash_dev)) {
        pr_err("flash_sysfs: error, flash dev not exist");
        return -ENODEV;
    }

    err = device_create_file(flash_dev, &dev_attr_front_flash);
    if (unlikely(err < 0)) {
        pr_err("flash_sysfs: failed to create device file, %s\n",
                dev_attr_front_flash.attr.name);
    }
    return 0;
}

static int lm3632_flash_probe(struct platform_device *pdev)
{
	struct lm3632 *lm3632 = dev_get_drvdata(pdev->dev.parent);
	struct ssflash_flash *flash;
	int rval;
	int error = 0;
	int ret = 0;

	pdata = lm3632->pdata->fl_pdata;

	flash = devm_kzalloc(&pdev->dev, sizeof(*flash), GFP_KERNEL);
	if (flash == NULL)
		return -ENOMEM;

	flash->dev = &pdev->dev;
	flash->lm3632 = lm3632;
	flash->data = (const struct ssflash_data *)&flash_lm3632;
	platform_set_drvdata(pdev, flash);

	rval = ssflash_subdev_init(flash);
	if (rval < 0)
		return rval;

	rval = ssflash_init_device(flash);
	if (rval < 0)
		return rval;

	dev_info(&pdev->dev, "%s", __func__);

	lm3632_ffl = devm_kzalloc(&pdev->dev, sizeof(*lm3632_ffl), GFP_KERNEL);
	if (!lm3632_ffl)
		return -ENOMEM;

	lm3632_ffl->dev = &pdev->dev;
	lm3632_ffl->lm3632 = lm3632;
	lm3632_ffl->pdata = pdata;

	if (!lm3632_ffl->pdata) {
		pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct lm3632_flash_platform_data), GFP_KERNEL);
		if (!pdata) {
			dev_info(&pdev->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}
		if (IS_ENABLED(CONFIG_OF)) {
			error = flash_parse_dt(&pdev->dev, pdata);
		} else {
			return -ENODEV;
    }
	ffl_info = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_info(&pdev->dev, "%s: fail to memory allocation.\n", __func__);
		return -ENOMEM;
	}

	info->pdata = pdata;
	info->lm3632 = lm3632;

	create_front_flash_sysfs();

	pdata->fled_pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR_OR_NULL(pdata->fled_pinctrl)) {
		pr_err("%s:%d Getting pinctrl handle failed\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	pdata->gpio_state_active = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_DEFAULT);
	if (IS_ERR_OR_NULL(pdata->gpio_state_active)) {
		pr_err("%s:%d Failed to get the active state pinctrl handle\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	pdata->gpio_state_suspend = pinctrl_lookup_state(pdata->fled_pinctrl, FLED_PINCTRL_STATE_SLEEP);
	if (IS_ERR_OR_NULL(pdata->gpio_state_suspend)) {
		pr_err("%s:%d Failed to get the active state pinctrl handle\n",
				__func__, __LINE__);
		return -EINVAL;
	}

	ret = pinctrl_select_state(pdata->fled_pinctrl, pdata->gpio_state_suspend);
	if (ret) {
		pr_err("%s:%d cannot set pin to active state", __func__, __LINE__);
		return -EINVAL;
	}
	if (error)
			return error;
	}
	return 0;
}

static int lm3632_flash_remove(struct platform_device *pdev)
{
	struct ssflash_flash *flash = platform_get_drvdata(pdev);

	v4l2_device_unregister_subdev(&flash->subdev_led);
	v4l2_ctrl_handler_free(&flash->ctrls_led);
	media_entity_cleanup(&flash->subdev_led.entity);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id lm3632_flash_of_match[] = {
	{ .compatible = "ti,lm3632-flash", },
	{ }
};
MODULE_DEVICE_TABLE(of, lm3632_flash_of_match);
#endif

static struct platform_driver lm3632_flash_driver = {
	.probe = lm3632_flash_probe,
	.remove = lm3632_flash_remove,
	.driver = {
		.name = "lm3632-flash",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lm3632_flash_of_match),
	},
};
module_platform_driver(lm3632_flash_driver);

MODULE_DESCRIPTION("TI LM3632 Flash Driver");
MODULE_AUTHOR("Daniel Jeong");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:lm3632-flash");

