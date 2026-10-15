/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  POTA activator spots (api.pota.app/spot/activator), parsed with jansson.
 *  Read-only: the hunter never posts spots.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char    activator[16];
    char    ref[16];        // park reference, e.g. US-1391
    char    park[48];
    char    location[16];   // e.g. US-PA
    char    mode[8];        // as spotted: CW, SSB, FT8...
    int32_t freq_hz;
    time_t  spot_time;      // UTC
} pota_spot_t;

/* Parse the JSON array into out (at most max). Returns the count kept, or -1
 * when the text is not a spot list. Spots without a usable frequency are
 * dropped. Newest first. */
int pota_spots_parse(const char *json, pota_spot_t *out, int max);

#ifdef __cplusplus
}
#endif
