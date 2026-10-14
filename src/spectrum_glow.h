/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#pragma once

#include <stdint.h>

#include "lvgl/lvgl.h"

/* Width of the halo, in pixels of amplitude */
#define SPECTRUM_GLOW_WIDTH 10

/*
 * Paint a soft halo above the spectrum trace into the rotated spectrum buffer
 * (XRGB8888, row = bin, column = amplitude, column 0 = top of the screen).
 *
 * x_top[row] is the trace's column for each row, or -1 for no data. The halo
 * follows the envelope of neighbouring rows, so a narrow carrier glows on its
 * sides as well as above its tip. Call before the trace and fill are drawn.
 */
void spectrum_glow_render(uint32_t *buf, int stride, int rows, const int16_t *x_top, lv_color_t color,
                          lv_opa_t opa);
