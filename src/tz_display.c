/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "tz_display.h"

#include <stdio.h>
#include <string.h>

typedef enum {
    DST_NONE = 0,
    DST_US,     // 2nd Sun Mar 02:00 local -> 1st Sun Nov 02:00 local
    DST_EU,     // last Sun Mar 01:00 UTC  -> last Sun Oct 01:00 UTC
    DST_AU,     // 1st Sun Oct 02:00 local -> 1st Sun Apr 03:00 local (southern)
    DST_NZ,     // last Sun Sep 02:00 local -> 1st Sun Apr 03:00 local (southern)
} dst_rule_t;

typedef struct {
    const char *name;
    const char *std;
    const char *dst;
    int16_t     offset_min;   // standard time, east of UTC
    dst_rule_t  rule;
} tz_zone_t;

/* Settings store the index: append only */
static const tz_zone_t zones[TZ_ZONE_COUNT] = {
    {"UTC",              "UTC",  "UTC",  0,    DST_NONE},
    {"US Eastern",       "EST",  "EDT",  -300, DST_US},
    {"US Central",       "CST",  "CDT",  -360, DST_US},
    {"US Mountain",      "MST",  "MDT",  -420, DST_US},
    {"Arizona",          "MST",  "MST",  -420, DST_NONE},
    {"US Pacific",       "PST",  "PDT",  -480, DST_US},
    {"Alaska",           "AKST", "AKDT", -540, DST_US},
    {"Hawaii",           "HST",  "HST",  -600, DST_NONE},
    {"Atlantic",         "AST",  "ADT",  -240, DST_US},
    {"Newfoundland",     "NST",  "NDT",  -210, DST_US},
    {"UK / Ireland",     "GMT",  "BST",  0,    DST_EU},
    {"Central Europe",   "CET",  "CEST", 60,   DST_EU},
    {"Eastern Europe",   "EET",  "EEST", 120,  DST_EU},
    {"Moscow",           "MSK",  "MSK",  180,  DST_NONE},
    {"India",            "IST",  "IST",  330,  DST_NONE},
    {"China",            "CST",  "CST",  480,  DST_NONE},
    {"Japan",            "JST",  "JST",  540,  DST_NONE},
    {"Sydney",           "AEST", "AEDT", 600,  DST_AU},
    {"Auckland",         "NZST", "NZDT", 720,  DST_NZ},
    {"Brazil",           "BRT",  "BRT",  -180, DST_NONE},
    {"South Africa",     "SAST", "SAST", 120,  DST_NONE},
};

/* Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm) */
static int64_t days_from_civil(int y, int m, int d) {
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const int64_t yoe = y - era * 400;
    const int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static int days_in_month(int y, int m) {
    static const int dm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    return (m == 2 && leap) ? 29 : dm[m - 1];
}

/* Day of month of the n-th Sunday (n = -1: last) */
static int sunday(int y, int m, int n) {
    if (n > 0) {
        int wday = (int)((days_from_civil(y, m, 1) + 4) % 7);   // 1970-01-01 was a Thursday
        int first = 1 + (7 - wday) % 7;
        return first + (n - 1) * 7;
    }
    int last = days_in_month(y, m);
    int wday = (int)((days_from_civil(y, m, last) + 4) % 7);
    return last - wday;
}

/* UTC seconds of a wall-clock moment given its UTC offset in minutes */
static int64_t at(int y, int m, int d, int hour, int offset_min) {
    return days_from_civil(y, m, d) * 86400 + hour * 3600 - offset_min * 60;
}

static bool in_dst(const tz_zone_t *z, int64_t t, int year) {
    int std = z->offset_min;
    int dst = std + 60;

    switch (z->rule) {
        case DST_US:
            return t >= at(year, 3, sunday(year, 3, 2), 2, std) && t < at(year, 11, sunday(year, 11, 1), 2, dst);
        case DST_EU:
            return t >= at(year, 3, sunday(year, 3, -1), 1, 0) && t < at(year, 10, sunday(year, 10, -1), 1, 0);
        case DST_AU:
            return t >= at(year, 10, sunday(year, 10, 1), 2, std) || t < at(year, 4, sunday(year, 4, 1), 3, dst);
        case DST_NZ:
            return t >= at(year, 9, sunday(year, 9, -1), 2, std) || t < at(year, 4, sunday(year, 4, 1), 3, dst);
        default:
            return false;
    }
}

void tz_convert(time_t utc, int zone, struct tm *out, const char **abbr) {
    if (zone < 0 || zone >= TZ_ZONE_COUNT) {
        zone = 0;
    }
    const tz_zone_t *z = &zones[zone];

    struct tm u;
    gmtime_r(&utc, &u);

    /* The UTC year is right for every rule except within hours of New Year,
     * where only the southern rules apply and they are in DST either way. */
    bool dst = in_dst(z, (int64_t)utc, u.tm_year + 1900);
    time_t local = utc + (time_t)(z->offset_min + (dst ? 60 : 0)) * 60;

    gmtime_r(&local, out);
    out->tm_isdst = dst;
    if (abbr) {
        *abbr = dst ? z->dst : z->std;
    }
}

const char *tz_dropdown_options(bool with_off) {
    static char buf[2][512];
    char *b = buf[with_off ? 1 : 0];

    if (b[0]) {
        return b;
    }
    size_t n = 0;
    if (with_off) {
        n += snprintf(b + n, sizeof(buf[0]) - n, " Off ");
    }
    for (int i = 0; i < TZ_ZONE_COUNT; i++) {
        n += snprintf(b + n, sizeof(buf[0]) - n, "%s %s ", (n ? "\n" : ""), zones[i].name);
    }
    return b;
}
