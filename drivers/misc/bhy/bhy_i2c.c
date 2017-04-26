/*!
* @section LICENSE
 * (C) Copyright 2011~2015 Bosch Sensortec GmbH All Rights Reserved
 *
 * This software program is licensed subject to the GNU General
 * Public License (GPL).Version 2,June 1991,
 * available at http://www.fsf.org/copyleft/gpl.html
*
* @filename bhy_i2c.c
* @date     "Fri Feb 13 14:57:45 2015 +0800"
* @id       "a51313e"
*
* @brief
* The implementation file for BHy I2C bus driver
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/input.h>

#include "bhy_core.h"
#include "bs_log.h"

#define BHY_MAX_RETRY_I2C_XFER		10
#define BHY_I2C_WRITE_DELAY_TIME	1000
#define BHY_I2C_MAX_BURST_WRITE_LEN	64

static s32 bhy_i2c_read_internal(struct i2c_client *client,
		u8 reg, u8 *data, u16 len)
{
	int ret, retry;

	struct i2c_msg msg[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		{
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = data,
		},
	};

	for (retry = 0; retry < BHY_MAX_RETRY_I2C_XFER; retry++) {
		ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
		if (ret >= 0)
			break;
		usleep_range(BHY_I2C_WRITE_DELAY_TIME,
				BHY_I2C_WRITE_DELAY_TIME);
	}

	return ret;
	/*int ret;
	if ((ret = i2c_master_send(client, &reg, 1)) < 0)
		return ret;
	return i2c_master_recv(client, data, len);*/
}

static s32 bhy_i2c_write_internal(struct i2c_client *client,
		u8 reg, u8 *data, u16 len)
{
	int ret, retry;
	u8 buf[BHY_I2C_MAX_BURST_WRITE_LEN + 1];
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
	};

	if (len > BHY_I2C_MAX_BURST_WRITE_LEN)
		return -EINVAL;

	buf[0] = reg;
	memcpy(&buf[1], data, len);
	msg.len = len + 1;
	msg.buf = buf;

	for (retry = 0; retry < BHY_MAX_RETRY_I2C_XFER; retry++) {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (ret >= 0)
			break;
		usleep_range(BHY_I2C_WRITE_DELAY_TIME,
				BHY_I2C_WRITE_DELAY_TIME);
	}

	return ret;
}

static s32 bhy_i2c_read(struct device *dev, u8 reg, u8 *data, u16 len)
{
	struct i2c_client *client;
	client = to_i2c_client(dev);
	return bhy_i2c_read_internal(client, reg, data, len);
}

static s32 bhy_i2c_write(struct device *dev, u8 reg, u8 *data, u16 len)
{
	struct i2c_client *client;
	client = to_i2c_client(dev);
	return bhy_i2c_write_internal(client, reg, data, len);
}

#ifdef CONFIG_PM
static int bhy_pm_op_suspend(struct device *dev)
{
	return bhy_suspend(dev);
}

static int bhy_pm_op_resume(struct device *dev)
{
	return bhy_resume(dev);
}

static const struct dev_pm_ops bhy_pm_ops = {
	.suspend = bhy_pm_op_suspend,
	.resume = bhy_pm_op_resume,
};
#endif

/*!
 * @brief	bhy version of i2c_probe
 */
static int bhy_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *dev_id) {
	struct bhy_data_bus data_bus = {
		.read = bhy_i2c_read,
		.write = bhy_i2c_write,
		.dev = &client->dev,
		.irq = client->irq,
		.bus_type = BUS_I2C,
	};

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		PERR("i2c_check_functionality error!");
		return -EIO;
	}

	return bhy_probe(&data_bus);
}

static void bhy_i2c_shutdown(struct i2c_client *client)
{
}

static int bhy_i2c_remove(struct i2c_client *client)
{
	return bhy_remove(&client->dev);
}

static const struct i2c_device_id bhy_i2c_id[] = {
	{ "bhy", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, bhy_i2c_id);

static const struct of_device_id device_of_match[] = {
	{ .compatible = "bhy", },
	{},
};

static struct i2c_driver bhy_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "bhy",
		.of_match_table = of_match_ptr(device_of_match),
#ifdef CONFIG_PM
		.pm = &bhy_pm_ops,
#endif
	},
	.id_table = bhy_i2c_id,
	.probe = bhy_i2c_probe,
	.shutdown = bhy_i2c_shutdown,
	.remove = bhy_i2c_remove,
};

module_i2c_driver(bhy_i2c_driver);

MODULE_AUTHOR("Contact <contact@bosch-sensortec.com>");
MODULE_DESCRIPTION("BHY I2C DRIVER");
MODULE_LICENSE("GPL v2");
