/*
 * include/linux/sec_param.h
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

struct sec_param_data {
	unsigned int debuglevel;
	unsigned int uartsel;
	unsigned int rory_control;
	unsigned int movinand_checksum_done;
	unsigned int movinand_checksum_pass;
	unsigned int cp_debuglevel;
#ifdef CONFIG_GSM_MODEM_SPRD6500
	unsigned int update_cp_bin;
#endif
#ifdef CONFIG_SEC_MONITOR_BATTERY_REMOVAL
	unsigned int normal_poweroff;
#endif
#ifdef CONFIG_RESTART_REASON_SEC_PARAM
	unsigned int param_restart_reason;
#endif
#ifdef CONFIG_BARCODE_PAINTER
	char param_barcode_info[1024];
#endif
};

enum sec_param_index {
	param_index_debuglevel,
	param_index_uartsel,
	param_rory_control,
	param_index_movinand_checksum_done,
	param_index_movinand_checksum_pass,
	param_cp_debuglevel,
#ifdef CONFIG_GSM_MODEM_SPRD6500
	param_update_cp_bin,
#endif
#ifdef CONFIG_SEC_MONITOR_BATTERY_REMOVAL
	param_index_normal_poweroff,
#endif
#ifdef CONFIG_RESTART_REASON_SEC_PARAM
	param_index_restart_reason,
#endif
#ifdef CONFIG_BARCODE_PAINTER
	param_index_barcode_info,
#endif
};

extern bool sec_open_param(void);
extern bool sec_get_param(enum sec_param_index index, void *value);
extern bool sec_set_param(enum sec_param_index index, void *value);
