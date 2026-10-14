/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Time zones for the clock display. The system clock stays on UTC (FT8,
 *  logs and the RTC rely on it); only what the clock shows is converted.
 *  The image ships no tz database, so daylight-saving rules are built in.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Number of zones; indices are stored in settings, so only append to the list */
#define TZ_ZONE_COUNT 21

/* "\n"-separated zone names for an LVGL dropdown; with_off prepends " Off " */
const char *tz_dropdown_options(bool with_off);

/* Convert a UTC time to wall-clock time in a zone. abbr gets e.g. "EDT". */
void tz_convert(time_t utc, int zone, struct tm *out, const char **abbr);

#ifdef __cplusplus
}
#endif
