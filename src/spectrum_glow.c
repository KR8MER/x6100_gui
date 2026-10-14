/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "spectrum_glow.h"

/* Rows either side that a peak's halo spreads into */
#define GLOW_SPREAD 3

static inline uint32_t blend_xrgb(uint32_t dst, uint32_t src, uint8_t a) {
    uint32_t rb = ((dst & 0x00FF00FFu) * (255u - a) + (src & 0x00FF00FFu) * a + 0x00800080u);
    uint32_t g  = ((dst & 0x0000FF00u) * (255u - a) + (src & 0x0000FF00u) * a + 0x00008000u);
    return ((rb >> 8) & 0x00FF00FFu) | ((g >> 8) & 0x0000FF00u);
}

void spectrum_glow_render(uint32_t *buf, int stride, int rows, const int16_t *x_top, lv_color_t color,
                          lv_opa_t opa) {
    if (opa == LV_OPA_TRANSP) {
        return;
    }

    for (int row = 0; row < rows; row++) {
        /* Envelope: the highest trace point among the neighbouring rows */
        int env = stride;
        for (int k = -GLOW_SPREAD; k <= GLOW_SPREAD; k++) {
            int r = row + k;
            if (r < 0 || r >= rows || x_top[r] < 0) {
                continue;
            }
            /* Neighbours count a little less, so the halo tapers sideways */
            int x = x_top[r] + (k < 0 ? -k : k) * 2;
            if (x < env) {
                env = x;
            }
        }
        if (env >= stride) {
            continue;
        }

        int own = x_top[row] >= 0 ? x_top[row] : stride;
        int from = env - SPECTRUM_GLOW_WIDTH;
        if (from < 0) {
            from = 0;
        }

        uint32_t *p = &buf[row * stride];
        for (int x = from; x < own && x < stride; x++) {
            /* Brightest at the envelope, fading out above it */
            int d = env - x;
            int f = SPECTRUM_GLOW_WIDTH - d;   /* quadratic falloff */
            int a = d <= 0 ? opa : opa * f * f / (SPECTRUM_GLOW_WIDTH * SPECTRUM_GLOW_WIDTH);
            if (a > 0) {
                p[x] = blend_xrgb(p[x], color.full, (uint8_t)a);
            }
        }
    }
}
