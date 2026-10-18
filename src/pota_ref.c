/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "pota_ref.h"

#include <ctype.h>
#include <jansson.h>
#include <stdio.h>
#include <string.h>

/* pota.app's validReferenceRegex: ^[A-Z0-9]{1,2}-[0-9]{4,5}$ */
bool pota_ref_valid(const char *ref) {
    size_t n = ref ? strlen(ref) : 0;
    size_t i = 0;

    while (i < n && i < 3 && ((ref[i] >= 'A' && ref[i] <= 'Z') || isdigit((unsigned char)ref[i]))) {
        i++;
    }
    if (i < 1 || i > 2 || i >= n || ref[i] != '-') {
        return false;
    }
    size_t digits = n - i - 1;
    if (digits < 4 || digits > 5) {
        return false;
    }
    for (size_t k = i + 1; k < n; k++) {
        if (!isdigit((unsigned char)ref[k])) {
            return false;
        }
    }
    return true;
}

/* pota.app's checks on a spot's callsign: validCallsignRegex
 *   ^(?:[A-Z\d]{1,4}/)?[A-Z\d]{1,3}\d[A-Z\d]*(?:/[A-Z\d]{1,4})?$
 * and no empty part between slashes, the main part (the longest, or the
 * one shaped like a call) having a digit and ending in a letter */
static bool all_alnum(const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) return false;
    }
    return true;
}

static bool base_ok(const char *s, size_t n) {
    /* [A-Z\d]{1,3}\d[A-Z\d]*: a digit at position 1..3 */
    if (n < 2 || !all_alnum(s, n)) return false;
    for (size_t i = 1; i <= 3 && i < n; i++) {
        if (isdigit((unsigned char)s[i])) return true;
    }
    return false;
}

static bool callish(const char *s, size_t n) {
    /* ^[A-Z]{1,2}\d{1,2}[A-Z]{1,4}$ */
    size_t i = 0, a = 0, d = 0, b = 0;
    while (i < n && isupper((unsigned char)s[i]) && a < 3) { i++; a++; }
    while (i < n && isdigit((unsigned char)s[i]) && d < 3) { i++; d++; }
    while (i < n && isupper((unsigned char)s[i]) && b < 5) { i++; b++; }
    return i == n && a >= 1 && a <= 2 && d >= 1 && d <= 2 && b >= 1 && b <= 4;
}

bool pota_call_valid(const char *call) {
    const char *part[3];
    size_t      len[3];
    int         parts = 0;
    const char *p     = call ? call : "";

    if (!*p) return false;
    while (parts < 3) {
        const char *slash = strchr(p, '/');
        part[parts] = p;
        len[parts]  = slash ? (size_t)(slash - p) : strlen(p);
        if (len[parts] == 0) return false;   // an empty part
        parts++;
        if (!slash) break;
        p = slash + 1;
        if (parts == 3) return false;        // more than two slashes
    }
    for (int i = 0; i < parts; i++) {
        if (!all_alnum(part[i], len[i])) return false;
    }
    /* the regex: [prefix/]base[/suffix] */
    int base;
    if (parts == 1) {
        base = 0;
    } else if (parts == 3) {
        if (len[0] > 4 || len[2] > 4) return false;
        base = 1;
    } else if (len[1] <= 4 && base_ok(part[0], len[0])) {
        base = 0;   // CALL/P
    } else if (len[0] <= 4) {
        base = 1;   // VE3/CALL
    } else {
        return false;
    }
    if (!base_ok(part[base], len[base])) return false;
    /* the main part: shaped like a call, else the longest */
    int main = -1;
    for (int i = 0; i < parts && main < 0; i++) {
        if (callish(part[i], len[i])) main = i;
    }
    if (main < 0) {
        main = 0;
        for (int i = 1; i < parts; i++) {
            if (len[i] > len[main]) main = i;
        }
    }
    const char *m = part[main];
    size_t      n = len[main];
    bool        digit = false;
    for (size_t i = 0; i < n; i++) digit |= isdigit((unsigned char)m[i]) != 0;
    return digit && isupper((unsigned char)m[n - 1]);
}

int pota_refs_parse(const char *text, char out[][POTA_REF_LEN], int max, char *bad, size_t bad_len) {
    int         n = 0;
    const char *p = text ? text : "";

    if (bad && bad_len) {
        bad[0] = 0;
    }
    while (*p) {
        char   ref[POTA_REF_LEN];
        size_t len = 0;

        while (*p == ',' || *p == ' ' || *p == ';') p++;
        if (!*p) break;
        while (*p && *p != ',' && *p != ' ' && *p != ';') {
            if (len < sizeof(ref) - 1) {
                ref[len++] = (char)toupper((unsigned char)*p);
            } else {
                len = sizeof(ref);   // too long: not a reference
            }
            p++;
        }
        if (len >= sizeof(ref)) {
            len = sizeof(ref) - 1;
            ref[len] = 0;
            if (bad && bad_len) snprintf(bad, bad_len, "%s", ref);
            return -1;
        }
        ref[len] = 0;
        if (!pota_ref_valid(ref)) {
            if (bad && bad_len) snprintf(bad, bad_len, "%s", ref);
            return -1;
        }
        bool seen = false;
        for (int i = 0; i < n; i++) {
            seen |= strcmp(out[i], ref) == 0;
        }
        if (!seen) {
            if (n >= max) {
                if (bad && bad_len) snprintf(bad, bad_len, "%s", ref);
                return -1;
            }
            snprintf(out[n++], POTA_REF_LEN, "%s", ref);
        }
    }
    return n;
}

void pota_refs_join(char refs[][POTA_REF_LEN], int n, char *out, size_t len) {
    size_t used = 0;

    if (!len) return;
    out[0] = 0;
    for (int i = 0; i < n && used < len; i++) {
        int w = snprintf(out + used, len - used, "%s%s", i ? "," : "", refs[i]);
        if (w < 0) break;
        used += (size_t)w;
    }
}

void pota_upload_name(const char *call, const char *park, time_t day, char *out, size_t len) {
    char      safe[32];
    size_t    k = 0;
    struct tm tm;

    for (const char *c = call ? call : ""; *c && k < sizeof(safe) - 1; c++) {
        safe[k++] = *c == '/' ? '_' : (char)toupper((unsigned char)*c);
    }
    safe[k] = 0;
    gmtime_r(&day, &tm);
    snprintf(out, len, "%s@%s-%04d%02d%02d.adi", safe, park ? park : "", tm.tm_year + 1900, tm.tm_mon + 1,
             tm.tm_mday);
}

const char *pota_spot_mode(qso_log_mode_t mode) {
    switch (mode) {
        case MODE_SSB: return "SSB";
        case MODE_AM: return "AM";
        case MODE_FM: return "FM";
        case MODE_CW: return "CW";
        case MODE_FT8: return "FT8";
        case MODE_FT4: return "FT4";
        case MODE_RTTY: return "RTTY";
        case MODE_JS8: return "JS8";
        default: return "";
    }
}

char *pota_spot_json(const char *call, const char *park, uint64_t freq_hz, qso_log_mode_t mode,
                     const char *comments, const char *source) {
    char khz[24];
    /* kHz with up to 2 decimals, trailing zeros stripped, as pota.app's
     * spot form sends it: 14074, 7185.5, 14074.25. Cut to 10 Hz, not
     * rounded: rounding could put a signal at a band edge outside it. */
    uint64_t cents = freq_hz / 10;
    unsigned dec   = (unsigned)(cents % 100);

    if (dec == 0) {
        snprintf(khz, sizeof(khz), "%llu", (unsigned long long)(cents / 100));
    } else if (dec % 10 == 0) {
        snprintf(khz, sizeof(khz), "%llu.%u", (unsigned long long)(cents / 100), dec / 10);
    } else {
        snprintf(khz, sizeof(khz), "%llu.%02u", (unsigned long long)(cents / 100), dec);
    }

    json_t *o = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s}", "activator", call ? call : "", "spotter",
                          call ? call : "", "frequency", khz, "reference", park ? park : "", "mode",
                          pota_spot_mode(mode), "source", source ? source : "", "comments",
                          comments && comments[0] ? comments : POTA_SPOT_DEFAULT_COMMENT);
    if (!o) {
        return NULL;
    }
    char *s = json_dumps(o, JSON_COMPACT);
    json_decref(o);
    return s;
}

static void copy_field(char *dst, size_t n, json_t *obj, const char *key) {
    const char *v = json_string_value(json_object_get(obj, key));
    snprintf(dst, n, "%s", v ? v : "");
}

bool pota_park_parse(const char *json, pota_park_t *out) {
    json_error_t err;
    json_t      *root = json_loads(json ? json : "", 0, &err);

    memset(out, 0, sizeof(*out));
    if (!json_is_object(root)) {
        json_decref(root);
        return false;
    }
    copy_field(out->name, sizeof(out->name), root, "name");
    copy_field(out->type, sizeof(out->type), root, "parktypeDesc");
    copy_field(out->location, sizeof(out->location), root, "locationDesc");
    copy_field(out->grid, sizeof(out->grid), root, "grid6");
    json_decref(root);
    return out->name[0] != 0;
}
