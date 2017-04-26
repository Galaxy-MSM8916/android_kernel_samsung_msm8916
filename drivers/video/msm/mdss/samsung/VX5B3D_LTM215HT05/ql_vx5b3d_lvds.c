#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include <linux/qpnp/pin.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/qpnp/pwm.h>
#include <linux/err.h>
#include <linux/lcd.h>

#if defined(CONFIG_LCD_CLASS_DEVICE)
#include <linux/lcd.h>
#include <linux/of_platform.h>
#endif /* CONFIG_LCD_CLASS_DEVICE */

#include "../../mdss_dsi.h"
#include "../../mdss_fb.h"

#include  <mach/gpio.h>
#include <linux/clk.h>

#include "../ss_dsi_panel_common.h"


/*struct i2c_client *lvds_i2c_client;*/
struct vx5b3d_i2c_platform_data {
	unsigned	 int gpio_backlight_en;
	u32 en_gpio_flags;
	int gpio_sda;
	u32 sda_gpio_flags;
	int gpio_scl;
	u32 scl_gpio_flags;
};

struct vx5b3d_i2c_info {
	struct i2c_client			*client;
	struct vx5b3d_i2c_platform_data	*pdata;
};

static struct vx5b3d_i2c_info *i2c_info;

static int WriteRegister(u16 addr, u32 w_data)
{
	int ret;
	u8 tx_data[7];

	struct i2c_msg msg[] = {
		{i2c_info->client->addr, 0, 7, tx_data }
	};

	/* NOTE: Register address big-endian, data little-endian. */
	tx_data[0] = 0xA;

	tx_data[1] = (uint8_t)(addr >> 8) & 0xff;
	tx_data[2] = (uint8_t)addr & 0xff; ;
	tx_data[3] = w_data & 0xff;
	tx_data[4] = (w_data >> 8) & 0xff;
	tx_data[5] = (w_data >> 16) & 0xff;
	tx_data[6] = (w_data >> 24) & 0xff;

	ret = i2c_transfer(i2c_info->client->adapter, msg, ARRAY_SIZE(msg));
	if (unlikely(ret < 0)) {
		pr_err("%s: i2c write failed reg 0x%04x val 0x%08x error,retry %d\n",
				__func__ ,addr, w_data, ret);
		ret = i2c_transfer(i2c_info->client->adapter, msg, ARRAY_SIZE(msg));
		if(ret)
			pr_err("%s: i2c write ok reg 0x%04x val 0x%08x  %d\n",
				__func__ ,addr, w_data, ret);

		else
			pr_err("%s: i2c write fail reg 0x%04x val 0x%08x  %d\n",
				__func__ ,addr, w_data, ret);
		}

	return ret;
}

static int vx5b3d_i2c_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct vx5b3d_i2c_platform_data *pdata;
	struct vx5b3d_i2c_info *info;

	struct samsung_display_driver_data *vdd = samsung_get_vdd();


	int error = 0;

	dev_info(&client->dev, "%s:vx5b3d probe called\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	if (client->dev.of_node) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct vx5b3d_i2c_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&client->dev, "Failed to allocate memory\n");
			return -ENOMEM;
		}

	} else
		pdata = client->dev.platform_data;



	i2c_info = info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}


	info->client = client;
	info->pdata = pdata;

	i2c_set_clientdata(client, info);

	vdd->panel_func.samsung_lvds_write_reg = WriteRegister;

	return error;
}

static int vx5b3d_i2c_remove(struct i2c_client *client)
{

	return 0;
}

static const struct i2c_device_id vx5b3d_i2c_id[] = {
	{"lvds", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, vx5b3d_i2c_id);

static struct of_device_id vx5b3d_i2c_match_table[] = {
	{ .compatible = "lvds",},
	{ },
};

MODULE_DEVICE_TABLE(of, vx5b3d_i2c_match_table);


static struct i2c_driver vx5b3d_i2c_driver = {
	.driver = {
		.name = "lvds_vx5b3d",
		.owner = THIS_MODULE,
		.of_match_table = vx5b3d_i2c_match_table,
	},
	.probe	= vx5b3d_i2c_probe,
	.remove = vx5b3d_i2c_remove,
	.id_table = vx5b3d_i2c_id,
};


static int __init vx5b3d_i2c_init(void)
{

	int ret = 0;

	pr_err("%s", __func__);

	ret = i2c_add_driver(&vx5b3d_i2c_driver);
	if (ret) {
		pr_err("vx5b3dx_i2c_init registration failed. ret= %d\n",ret);
	}

	return ret;
}
module_init(vx5b3d_i2c_init);

static void __exit vx5b3d_i2c_exit(void)
{
	pr_err("%s", __func__);

	i2c_del_driver(&vx5b3d_i2c_driver);
}
module_exit(vx5b3d_i2c_exit);


