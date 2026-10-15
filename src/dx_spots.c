/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  DX cluster spots from HamQTH
 */

#include "dx_spots.h"

#include <ctype.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define SPOTS_URL    "https://www.hamqth.com/api/v2/spots"
#define MIN_FREQ_KHZ 1000       // the radio covers HF and 6 m
#define MAX_FREQ_KHZ 60000

typedef struct {
    int32_t     lo, hi;         // Hz, inclusive
    const char *name;
} band_t;

/* Edges wide enough for every ITU region's allocation */
static const band_t bands[] = {
    {1800000, 2000000, "160M"},
    {3500000, 4000000, "80M"},
    {5250000, 5450000, "60M"},
    {7000000, 7300000, "40M"},
    {10100000, 10150000, "30M"},
    {14000000, 14350000, "20M"},
    {18068000, 18168000, "17M"},
    {21000000, 21450000, "15M"},
    {24890000, 24990000, "12M"},
    {28000000, 29700000, "10M"},
    {50000000, 54000000, "6M"},
};

/* A spot with its place in the feed, so equal times keep the feed's order */
typedef struct {
    dx_spot_t spot;
    size_t    index;
} ranked_t;

static void copy_str(char *dst, size_t len, json_t *obj, const char *key) {
    const char *v = json_string_value(json_object_get(obj, key));
    snprintf(dst, len, "%s", v ? v : "");
}

/* "2026-10-15T16:31:00Z" (UTC) */
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
    const ranked_t *ra = a, *rb = b;
    if (ra->spot.time != rb->spot.time) {
        return ra->spot.time < rb->spot.time ? 1 : -1;
    }
    return ra->index < rb->index ? -1 : ra->index > rb->index;
}

int dx_spots_parse(const char *json, dx_spot_t *out, int max) {
    json_error_t err;
    json_t      *root = json_loads(json ? json : "", 0, &err);
    json_t      *list = json_object_get(root, "spots");
    json_t      *item;
    size_t       i;
    int          n = 0;
    ranked_t    *all;

    if (!json_is_array(list)) {
        json_decref(root);
        return -1;
    }

    /* Parse every spot and keep the newest `max`, whatever the feed's order */
    all = malloc((json_array_size(list) + 1) * sizeof(*all));
    if (!all) {
        json_decref(root);
        return -1;
    }

    json_array_foreach(list, i, item) {
        if (!json_is_object(item)) continue;

        /* kHz as a number; accept a string too */
        json_t *jf  = json_object_get(item, "freq");
        double  khz = json_is_string(jf) ? atof(json_string_value(jf)) : json_number_value(jf);
        if (khz < MIN_FREQ_KHZ || khz > MAX_FREQ_KHZ) continue;

        dx_spot_t *s = &all[n].spot;
        memset(s, 0, sizeof(*s));
        all[n].index = i;
        s->freq_hz   = (int32_t)(khz * 1000.0 + 0.5);
        copy_str(s->call, sizeof(s->call), item, "dxcall");
        copy_str(s->spotter, sizeof(s->spotter), item, "spotter");
        copy_str(s->band, sizeof(s->band), item, "band");
        copy_str(s->mode, sizeof(s->mode), item, "mode");
        copy_str(s->comment, sizeof(s->comment), item, "comment");
        copy_str(s->country, sizeof(s->country), item, "country");
        copy_str(s->continent, sizeof(s->continent), item, "continent");
        s->lotw = json_is_true(json_object_get(item, "lotw"));
        s->time = parse_time(json_string_value(json_object_get(item, "time")));
        if (!s->band[0]) {
            dx_band_name(s->freq_hz, s->band, sizeof(s->band));
        }

        if (s->call[0]) {
            n++;
        }
    }
    json_decref(root);

    qsort(all, n, sizeof(all[0]), newest_first);
    if (n > max) {
        n = max < 0 ? 0 : max;
    }
    for (int k = 0; k < n; k++) {
        out[k] = all[k].spot;
    }
    free(all);
    return n;
}

bool dx_band_name(int32_t freq_hz, char *out, size_t n) {
    for (size_t i = 0; i < sizeof(bands) / sizeof(bands[0]); i++) {
        if (freq_hz >= bands[i].lo && freq_hz <= bands[i].hi) {
            snprintf(out, n, "%s", bands[i].name);
            return true;
        }
    }
    if (n) {
        out[0] = 0;
    }
    return false;
}

void dx_spots_url(char *buf, size_t n, const char *band, const char *mode, int limit) {
    int len;

    if (limit < 1) {
        limit = 1;
    } else if (limit > DX_SPOTS_LIMIT_MAX) {
        limit = DX_SPOTS_LIMIT_MAX;
    }
    len = snprintf(buf, n, SPOTS_URL "?format=json&limit=%i", limit);
    if (band && band[0] && len >= 0 && (size_t)len < n) {
        len += snprintf(buf + len, n - len, "&bands=%s", band);
    }
    if (mode && mode[0] && len >= 0 && (size_t)len < n) {
        snprintf(buf + len, n - len, "&modes=%s", mode);
    }
}

/* Where `word` first appears in text as a word of its own (any case), or NULL */
static const char *find_word(const char *text, const char *word) {
    size_t len = strlen(word);

    for (const char *p = text; *p; p++) {
        if (strncasecmp(p, word, len) != 0) continue;
        bool start = p == text || !isalnum((unsigned char)p[-1]);
        bool end   = !isalnum((unsigned char)p[len]);
        if (start && end) {
            return p;
        }
    }
    return NULL;
}

dx_tune_t dx_spot_tune(const dx_spot_t *spot) {
    if (!strcmp(spot->mode, "CW")) {
        return DX_TUNE_CW;
    }
    if (!strcmp(spot->mode, "SSB")) {
        return DX_TUNE_PHONE;
    }
    if (!strcmp(spot->mode, "DIGI")) {
        const char *ft8 = find_word(spot->comment, "FT8");
        const char *ft4 = find_word(spot->comment, "FT4");
        if (ft8 && (!ft4 || ft8 < ft4)) {
            return DX_TUNE_FT8;
        }
        if (ft4) {
            return DX_TUNE_FT4;
        }
    }
    return DX_TUNE_ONLY;
}
