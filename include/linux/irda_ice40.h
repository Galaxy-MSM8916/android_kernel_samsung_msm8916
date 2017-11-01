/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _IRDA_ICE40_H_
#define _IRDA_ICE40_H_

#define GPIO_LEVEL_LOW        0
#define GPIO_LEVEL_HIGH       1

#define SEC_FPGA_MAX_FW_PATH    255
#define SEC_FPGA_FW_FILENAME    "i2c_top_bitmap.bin"

#define SNPRINT_BUF_SIZE	255
#define FW_VER_ADDR		0x80

#define NO_PIN_DETECTED		-2
#define FIRMWARE_MAX_RETRY	2

#define IRDA_I2C_ADDR		0x50
#define FREQ_24MH		24000000
#define PWR_3_3V		3300000
#define IRDA_TEST_CODE_SIZE	144
#define IRDA_TEST_CODE_ADDR	0x00
#define MAX_SIZE		4096
#define READ_LENGTH		8

#define POWER_ON		1
#define POWER_OFF		0
#define SEND_SUCCESS		0
#define SEND_FAIL		-1

struct irda_ice40_data {
	struct i2c_client		*client;
	struct workqueue_struct		*firmware_dl;
	struct delayed_work		fw_dl;
	const struct firmware		*fw;
	struct mutex			mutex;
	struct {
		unsigned char		addr;
		unsigned char		data[MAX_SIZE];
	} i2c_block_transfer;
	int				length;
	int				count;
	int				operation;
	int				dev_id;
	int				ir_freq;
	int				ir_sum;
	int				on_off;
};

struct irda_ice40_platform_data {
	int fw_ver;
	int spi_clk;
	int spi_si;
	int cresetb;
	int rst_n;
	int irda_irq;
	struct pinctrl *pinctrl;
};

enum irda_tx_operation_type {
	IRDA_SINGLE,
	IRDA_REPEAT,
	IRDA_STOP,
	IRDA_LEARN,
};
extern struct class *sec_class;

#endif /* _IRDA_ICE40_H_ */
