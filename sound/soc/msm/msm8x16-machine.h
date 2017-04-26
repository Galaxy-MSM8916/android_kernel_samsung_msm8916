/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
#ifndef MSM8X16_MACHINE_H
#define MSM8X16_MACHINE_H

struct msm8916_asoc_mach_data {
	/* wcd 8916 */
	int codec_type;
	int ext_pa;
	int us_euro_gpio;
	int mclk_freq;
	int lb_mode;
	atomic_t mclk_rsc_ref;
	atomic_t mclk_enabled;
	struct mutex cdc_mclk_mutex;
	struct delayed_work disable_mclk_work;
	struct afe_digital_clk_cfg digital_cdc_clk;
	void __iomem *vaddr_gpio_mux_spkr_ctl;
	void __iomem *vaddr_gpio_mux_mic_ctl;	
	
	/* wm1814 */
	struct snd_soc_codec *codec;
	struct snd_soc_dai *aif[3];
	struct input_dev *input;
	int aif2_enable;

	unsigned int hp_impedance_step;
	bool ear_mic;

	int sysclk_rate;
	int asyncclk_rate;
	u32 imp_shift;
	u32 aif_format[3];	
};
#endif

