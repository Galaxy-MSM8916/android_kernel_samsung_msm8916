/*
 * Copyright (c) 2017 The Lineage Project
 *                    Vincent Zvikaramba <zvikovincent@gmail.com>
 *                    Vladimir Bely <vlwwwwww@gmail.com>
 *                    Emery Tanghanwaye <emerytang@gmail.com>
 *                    Sean Hoyt <deadman96385@gmail.com>
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

#ifndef TOUCH_DISABLER_H
#define TOUCH_DISABLER_H

#define TOUCH_DISABLER_NAME "touch_disabler"

#define CONTROL_AUTO "auto"
#define CONTROL_MANUAL "manual"

typedef struct touch_disabler_data {
	struct class *disabler_class;
	struct class *touch_screen_class;
	struct class *touch_key_class;
	struct input_dev *ts_dev;
	struct input_dev *tk_dev;
	int tk_enabled; /* enable (1) or disable (0) touch key */
	int tk_control;    /* driver control, between auto (0) and manual (1) */
	int ts_enabled; /* enable (1) or disable (0) touch panel */
	int ts_control;    /* driver control, between auto (0) and manual (1) */
} touch_disabler_data_t;

void touch_disabler_set_tk_dev(struct input_dev *ts_dev);
void touch_disabler_set_touch_status(bool status);
void touch_disabler_set_ts_dev(struct input_dev *ts_dev);

#endif /* TOUCH_DISABLER_H */
