/*
 * A new exposure driver based on SDE dim layer for OLED devices
 *
 * Copyright (C) 2012-2014, The Linux Foundation. All rights reserved.
 * Copyright (C) 2019, Devries <therkduan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "dsi_display.h"
#include "sde_crtc.h"
#include "sde_expo_dim_layer.h"

extern dc_dim_alpha;

static int interpolate(int x, int xa, int xb, int ya, int yb)
{
	int bf, factor, plus;
	int sub = 0;

	bf = 2 * (yb - ya) * (x - xa) / (xb - xa);
	factor = bf / 2;
	plus = bf % 2;
	if ((xa - xb) && (yb - ya))
		sub = 2 * (x - xa) * (x - xb) / (yb - ya) / (xa - xb);

	return ya + factor + plus + sub;
}

static int brightness_to_alpha(uint8_t brightness)
{
	int level = ARRAY_SIZE(brightness_alpha_lut);
	int index, alpha;

	for (index = 0; index < ARRAY_SIZE(brightness_alpha_lut); index++) {
		if (brightness_alpha_lut[index][BRIGHTNESS] >= brightness)
			break;
	}

	if (index == 0)
		alpha = brightness_alpha_lut[0][ALPHA];
	else if (index == level)
		alpha = brightness_alpha_lut[level - 1][ALPHA];
	else
		alpha = interpolate(brightness,
			brightness_alpha_lut[index - 1][BRIGHTNESS],
			brightness_alpha_lut[index][BRIGHTNESS],
			brightness_alpha_lut[index - 1][ALPHA],
			brightness_alpha_lut[index][ALPHA]);

	return alpha;
}

int expo_map_dim_level(int level, struct dsi_display *display, bool skip_update)
{
	int override_level;
	uint8_t brightness;

	if (level / LEVEL_BR_SCALE > U8_MAX) {
		brightness = U8_MAX;
	} else {
		brightness = level / LEVEL_BR_SCALE;
	}

	if (level < DIM_THRES_LEVEL) {
		override_level = DIM_THRES_LEVEL;
	} else {
		override_level = level;
	}
	
	if (!skip_update) {
		dc_dim_alpha = brightness_to_alpha(brightness);
	}

	return override_level;
}
