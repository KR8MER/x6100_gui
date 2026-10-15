/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "pota_spots.h"

#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copy_str(char *dst, size_t len, json_t *obj, const char *key) {
    const char *v = json_string_value(json_object_get(obj, key));
    snprintf(dst, len, "%s", v ? v : "");
}

/* "2026-10-15T00:13:23" (UTC) */
static time_t parse_time(const char *s) {
    struct tm t = {0};
    if (!s || sscanf(s, "%d-%d-%dT%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) {
        return 0;
    }
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    return timegm(&t);
}

static int newest_first(const void *a, const void *b) {
    time_t ta = ((const pota_spot_t *)a)->spot_time, tb = ((const pota_spot_t *)b)->spot_time;
    return ta < tb ? 1 : ta > tb ? -1 : 0;
}

int pota_spots_parse(const char *json, pota_spot_t *out, int max) {
    json_error_t err;
    json_t      *root = json_loads(json, 0, &err);
    size_t       i;
    json_t      *item;
    int          n = 0;

    if (!root || !json_is_array(root)) {
        json_decref(root);
        return -1;
    }

    json_array_foreach(root, i, item) {
        if (n >= max) break;
        if (!json_is_object(item)) continue;

        /* frequency is kHz as a string ("14055.5"); some feeds send a number */
        json_t *jf = json_object_get(item, "frequency");
        double  khz = json_is_string(jf) ? atof(json_string_value(jf)) : json_number_value(jf);
        if (khz < 1000 || khz > 60000) continue;   // the radio covers HF and 6 m

        pota_spot_t *s = &out[n];
        memset(s, 0, sizeof(*s));
        s->freq_hz = (int32_t)(khz * 1000.0 + 0.5);
        copy_str(s->activator, sizeof(s->activator), item, "activator");
        copy_str(s->ref, sizeof(s->ref), item, "reference");
        copy_str(s->park, sizeof(s->park), item, "name");
        copy_str(s->location, sizeof(s->location), item, "locationDesc");
        copy_str(s->mode, sizeof(s->mode), item, "mode");
        s->spot_time = parse_time(json_string_value(json_object_get(item, "spotTime")));

        if (s->activator[0] && s->ref[0]) {
            n++;
        }
    }
    json_decref(root);

    qsort(out, n, sizeof(out[0]), newest_first);
    return n;
}
