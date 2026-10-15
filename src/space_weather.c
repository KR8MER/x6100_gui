/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Space weather from NOAA SWPC
 */

#include "space_weather.h"

#include <jansson.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DAY_SEC      (24 * 60 * 60)
#define KP_MAX       9.0
#define G_LEVELS     5
#define KP_THIRD     (1.0 / 3.0)
#define KP_ROUNDING  0.01       // Kp thirds (4.67, 5.33) arrive as rounded decimals

bool sw_parse_time(const char *text, time_t *out) {
    struct tm tm = {0};
    char      sep;
    int       used = 0;

    if (!text) {
        return false;
    }
    if (sscanf(text, "%4d-%2d-%2d%c%2d:%2d:%2d%n", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &sep, &tm.tm_hour,
               &tm.tm_min, &tm.tm_sec, &used) != 7 ||
        (sep != 'T' && sep != ' ')) {
        return false;
    }
    if (tm.tm_mon < 1 || tm.tm_mon > 12 || tm.tm_mday < 1 || tm.tm_mday > 31 || tm.tm_hour > 23 || tm.tm_min > 59 ||
        tm.tm_sec > 60) {
        return false;
    }
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    *out = timegm(&tm);
    return true;
}

/* A number, or a numeric string as older SWPC products sent */
static bool get_number(json_t *obj, const char *key, double *out) {
    json_t *v = json_object_get(obj, key);

    if (json_is_number(v)) {
        *out = json_number_value(v);
    } else if (json_is_string(v)) {
        char *end;
        *out = strtod(json_string_value(v), &end);
        if (end == json_string_value(v) || *end) {
            return false;
        }
    } else {
        return false;
    }
    return isfinite(*out);
}

static bool get_time(json_t *obj, time_t *out) {
    return sw_parse_time(json_string_value(json_object_get(obj, "time_tag")), out);
}

bool sw_parse_flux(const char *json, sw_data_t *data) {
    json_t *root  = json ? json_loads(json, 0, NULL) : NULL;
    bool    found = false;
    double  flux  = 0;
    time_t  when  = 0;
    size_t  i;
    json_t *item;

    if (!json_is_array(root)) {
        json_decref(root);
        return false;
    }
    /* normally one entry; take the newest usable one */
    json_array_foreach(root, i, item) {
        double f;
        time_t t;
        if (get_number(item, "flux", &f) && f >= 0 && get_time(item, &t)) {
            flux  = f;
            when  = t;
            found = true;
        }
    }
    json_decref(root);
    if (found) {
        data->has_flux  = true;
        data->flux      = (int)lround(flux);
        data->flux_time = when;
    }
    return found;
}

bool sw_parse_kp(const char *json, sw_data_t *data) {
    json_t *root = json ? json_loads(json, 0, NULL) : NULL;
    json_t *item;
    size_t  i, n, last = 0;
    bool    found = false;
    double  kp    = 0, kp_max = 0, a;
    time_t  when  = 0, max_time = 0;

    if (!json_is_array(root)) {
        json_decref(root);
        return false;
    }
    n = json_array_size(root);

    /* the feed is oldest first: the latest Kp is the last usable entry */
    for (i = n; i-- > 0;) {
        item = json_array_get(root, i);
        if (get_number(item, "Kp", &kp) && kp >= 0 && kp <= KP_MAX && get_time(item, &when)) {
            last  = i;
            found = true;
            break;
        }
    }
    if (!found) {
        json_decref(root);
        return false;
    }

    data->has_kp  = true;
    data->kp      = kp;
    data->kp_time = when;
    data->a_index = get_number(json_array_get(root, last), "a_running", &a) && a >= 0 ? (int)lround(a) : -1;

    /* 24 h of data ending with the latest interval, not of wall-clock time */
    kp_max   = kp;
    max_time = when;
    for (i = 0; i < last; i++) {
        double k;
        time_t t;
        item = json_array_get(root, i);
        if (get_number(item, "Kp", &k) && k >= 0 && k <= KP_MAX && get_time(item, &t) && t > when - DAY_SEC &&
            t <= when && k > kp_max) {
            kp_max   = k;
            max_time = t;
        }
    }
    data->kp_max_24h  = kp_max;
    data->kp_max_time = max_time;

    json_decref(root);
    return true;
}

/*
 * NOAA Space Weather Scales (https://www.swpc.noaa.gov/noaa-scales-explanation):
 * G1 Minor "Kp = 5", G2 "Kp = 6", G3 "Kp = 7", G4 Severe "Kp = 8, including
 * a 9-", G5 Extreme "Kp = 9". A whole Kp covers its thirds (5- = 4.67 is
 * Kp 5), and 9- is named as G4, so G5 starts at 9o. Below G1 there is no
 * name (https://www.swpc.noaa.gov/products/planetary-k-index).
 */
int sw_g_level(double kp) {
    static const double start[G_LEVELS] = {5 - KP_THIRD, 6 - KP_THIRD, 7 - KP_THIRD, 8 - KP_THIRD, 9};
    int                 level           = 0;

    if (!isfinite(kp)) {
        return 0;
    }
    for (int i = 0; i < G_LEVELS; i++) {
        if (kp + KP_ROUNDING >= start[i]) {
            level = i + 1;
        }
    }
    return level;
}

const char *sw_g_name(int level) {
    static const char *names[] = {"", "Minor", "Moderate", "Strong", "Severe", "Extreme"};

    return level >= 0 && level <= G_LEVELS ? names[level] : "";
}
