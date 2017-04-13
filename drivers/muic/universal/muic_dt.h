#ifndef _MUIC_DT_
#define _MUIC_DT_

extern struct of_device_id muic_i2c_dt_ids[];

extern int of_update_supported_list(struct i2c_client *i2c,
				struct muic_platform_data *pdata);
extern int of_muic_dt(struct i2c_client *i2c, struct muic_platform_data *pdata);
#if defined(CONFIG_MUIC_PINCTRL)
extern int of_muic_pinctrl(struct i2c_client *i2c);
#endif
extern int muic_set_gpio_uart_sel(int uart_sel);
#endif
