/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  DX cluster spots from HamQTH (parsing only; no network, no LVGL).
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DX_SPOTS_LIMIT_MAX 200      // the API's own cap on `limit`

typedef struct {
    char    call[20];       // the DX station
    char    spotter[20];
    int32_t freq_hz;
    char    band[8];        // as HamQTH names it: "20M"
    char    mode[8];        // "CW", "SSB", "DIGI"; "" when the cluster gave none
    char    comment[96];
    time_t  time;           // UTC
    char    country[40];
    char    continent[4];
    bool    lotw;
} dx_spot_t;

/* How to set the radio up for a spot */
typedef enum {
    DX_TUNE_ONLY,           // just the frequency: the mode is left alone
    DX_TUNE_CW,             // CW cockpit
    DX_TUNE_PHONE,          // Phone cockpit
    DX_TUNE_FT8,            // Digital cockpit, FT8
    DX_TUNE_FT4,            // Digital cockpit, FT4
} dx_tune_t;

/* Parse a reply of /api/v2/spots and keep the newest max in out, newest
 * first. Spots without a call or outside 1000-60000 kHz are dropped.
 * Returns the count kept, or -1 when the text is not a spot list. */
int dx_spots_parse(const char *json, dx_spot_t *out, int max);

/* Amateur band name in HamQTH's form ("40M"); false outside the HF and
 * 6 m bands */
bool dx_band_name(int32_t freq_hz, char *out, size_t n);

/* The spots URL. band ("20M") and mode ("CW", "SSB", "DIGI") filter on the
 * server; NULL or "" means all. limit is clamped to 1..DX_SPOTS_LIMIT_MAX. */
void dx_spots_url(char *buf, size_t n, const char *band, const char *mode, int limit);

/* CW and SSB by their mode; data only when the comment names FT8 or FT4 */
dx_tune_t dx_spot_tune(const dx_spot_t *spot);

#ifdef __cplusplus
}
#endif
