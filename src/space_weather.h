/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Space weather from NOAA SWPC (parsing only; no network, no LVGL).
 */

#pragma once

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SW_FLUX_URL "https://services.swpc.noaa.gov/products/summary/10cm-flux.json"
#define SW_KP_URL   "https://services.swpc.noaa.gov/products/noaa-planetary-k-index.json"

typedef struct {
    bool   has_flux;
    int    flux;            // 10.7 cm solar flux, sfu
    time_t flux_time;       // UTC

    bool   has_kp;
    double kp;              // latest planetary K index
    time_t kp_time;         // start of its 3 h interval, UTC
    int    a_index;         // a_running of that interval, -1 if not given
    double kp_max_24h;      // highest Kp of the intervals in the 24 h up to kp_time
    time_t kp_max_time;
} sw_data_t;

/* "YYYY-MM-DDTHH:MM:SS" (or a space instead of T) read as UTC */
bool sw_parse_time(const char *text, time_t *out);

/* Fill the flux / Kp fields of *data from a feed reply; on false *data is
 * left untouched */
bool sw_parse_flux(const char *json, sw_data_t *data);
bool sw_parse_kp(const char *json, sw_data_t *data);

/* NOAA G scale for a Kp: 0 below storm level, 1 (G1) .. 5 (G5) */
int         sw_g_level(double kp);
/* Official name of a G level: "Minor" .. "Extreme", "" for 0 */
const char *sw_g_name(int level);

#ifdef __cplusplus
}
#endif
