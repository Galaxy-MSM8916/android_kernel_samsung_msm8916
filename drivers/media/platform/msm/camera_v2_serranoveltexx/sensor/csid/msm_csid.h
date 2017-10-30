/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef MSM_CSID_H
#define MSM_CSID_H

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <media/v4l2-subdev.h>
#include "../../include/media/msm_cam_sensor.h"
#include "msm_sd.h"

struct csid_reg_parms_t {
/* MIPI	CSID registers */
	uint32_t csid_hw_version_addr;
	uint32_t csid_core_ctrl_0_addr;
	uint32_t csid_core_ctrl_1_addr;
	uint32_t csid_rst_cmd_addr;
	uint32_t csid_cid_lut_vc_0_addr;
	uint32_t csid_cid_lut_vc_1_addr;
	uint32_t csid_cid_lut_vc_2_addr;
	uint32_t csid_cid_lut_vc_3_addr;
	uint32_t csid_cid_n_cfg_addr;
	uint32_t csid_irq_clear_cmd_addr;
	uint32_t csid_irq_mask_addr;
	uint32_t csid_irq_status_addr;
	uint32_t csid_captured_unmapped_long_pkt_hdr_addr;
	uint32_t csid_captured_mmaped_long_pkt_hdr_addr;
	uint32_t csid_captured_short_pkt_addr;
	uint32_t csid_captured_long_pkt_hdr_addr;
	uint32_t csid_captured_long_pkt_ftr_addr;
	uint32_t csid_pif_misr_dl0_addr;
	uint32_t csid_pif_misr_dl1_addr;
	uint32_t csid_pif_misr_dl2_addr;
	uint32_t csid_pif_misr_dl3_addr;
	uint32_t csid_stats_total_pkts_rcvd_addr;
	uint32_t csid_stats_ecc_addr;
	uint32_t csid_stats_crc_addr;
	uint32_t csid_tg_ctrl_addr;
	uint32_t csid_tg_vc_cfg_addr;
	uint32_t csid_tg_dt_n_cfg_0_addr;
	uint32_t csid_tg_dt_n_cfg_1_addr;
	uint32_t csid_tg_dt_n_cfg_2_addr;
	uint32_t csid_rst_done_irq_bitshift;
	uint32_t csid_rst_stb_all;
	uint32_t csid_dl_input_sel_shift;
	uint32_t csid_phy_sel_shift;
	uint32_t csid_version;
};

struct csid_ctrl_t {
	struct csid_reg_parms_t csid_reg;
};

enum msm_csid_state_t {
	CSID_POWER_UP,
	CSID_POWER_DOWN,
};

struct csid_device {
	struct platform_device *pdev;
	struct msm_sd_subdev msm_sd;
	struct resource *mem;
	struct resource *irq;
	struct resource *io;
	struct regulator *csi_vdd;
	void __iomem *base;
	struct mutex mutex;
	struct completion reset_complete;
	uint32_t hw_version;
	uint32_t hw_dts_version;
	enum msm_csid_state_t csid_state;
	struct csid_ctrl_t *ctrl_reg;
	uint32_t num_clk;
	uint32_t num_clk_src_info;

	struct clk *csid_clk[11];
};

#define VIDIOC_MSM_CSID_RELEASE \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 5, struct v4l2_subdev*)
#endif
