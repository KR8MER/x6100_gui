/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "pota_activation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cfg/cfg_api.h"
#include "msg.h"
#include "lvgl/lvgl.h"
#include "net_fetch.h"

#define PARK_URL "https://api.pota.app/park/"
#define SPOT_URL "https://api.pota.app/spot"
#define NAMES_MAX 8

static char parks[POTA_PARKS_MAX][POTA_REF_LEN];
static int  park_count;

/* ---- the activation ---- */

static void load(void) {
    char text[PARAM_TEXT_MAX];

    param_t_get_into(cfg.pota_parks(), text, sizeof(text));
    park_count = pota_refs_parse(text, parks, POTA_PARKS_MAX, NULL, 0);
    if (park_count < 0) {
        park_count = 0;
    }
}

/* Moved off the spotted frequency and settled there: offer a re-spot. A
 * spot is never sent without a press (pota.app's spots come from people). */
#define QSY_CHECK_MS  10000
#define QSY_MIN_HZ    1000

static uint64_t last_spot_freq;
static uint64_t qsy_seen;       // frequency at the last check
static uint64_t qsy_told;       // frequency already told about

static void qsy_check(lv_timer_t *t) {
    (void)t;
    if (park_count == 0 || !last_spot_freq) {
        qsy_seen = 0;
        return;
    }
    uint64_t freq  = (uint64_t)cparam_i_get(cfg.cur.fg_freq());
    uint64_t moved = freq > last_spot_freq ? freq - last_spot_freq : last_spot_freq - freq;
    bool     still = freq == qsy_seen;

    qsy_seen = freq;
    if (moved >= QSY_MIN_HZ && still && freq != qsy_told) {
        qsy_told = freq;
        msg_schedule_text_fmt("New frequency: APP > Activate POTA > Spot Me to re-spot");
    }
}

void pota_activation_init(void) {
    static lv_timer_t *timer;

    load();
    if (!timer) {
        timer = lv_timer_create(qsy_check, QSY_CHECK_MS, NULL);
    }
}

int pota_activation_parks(char out[][POTA_REF_LEN], int max) {
    int n = park_count < max ? park_count : max;

    for (int i = 0; i < n; i++) {
        snprintf(out[i], POTA_REF_LEN, "%s", parks[i]);
    }
    return n;
}

bool pota_activation_active(void) {
    return park_count > 0;
}

static time_t   last_spot_time;
static int      last_spot_mode = -1;

bool pota_activation_set(const char *text, char *why, size_t why_len) {
    char refs[POTA_PARKS_MAX][POTA_REF_LEN];
    char bad[POTA_REF_LEN];
    int  n = pota_refs_parse(text, refs, POTA_PARKS_MAX, bad, sizeof(bad));

    if (n < 0) {
        if (why && why_len) {
            snprintf(why, why_len, "%s is not a park (like US-1234), or more than %i parks", bad,
                     POTA_PARKS_MAX);
        }
        return false;
    }
    char joined[PARAM_TEXT_MAX];
    pota_refs_join(refs, n, joined, sizeof(joined));
    param_t_set(cfg.pota_parks(), joined);
    memcpy(parks, refs, sizeof(parks));
    park_count     = n;
    last_spot_freq = 0;
    last_spot_time = 0;
    last_spot_mode = -1;
    return true;
}

void pota_activation_tag(qso_log_record_t *qso) {
    if (park_count == 0 || !qso) {
        return;
    }
    snprintf(qso->my_sig, sizeof(qso->my_sig), "POTA");
    pota_refs_join(parks, park_count, qso->my_sig_info, sizeof(qso->my_sig_info));
}

static bool ensure_dir(void) {
    return mkdir(POTA_UPLOAD_DIR, 0755) == 0 || errno == EEXIST;
}

int pota_activation_write_files(time_t day) {
    char call[PARAM_TEXT_MAX];
    int  written = 0;

    if (park_count == 0 || !ensure_dir()) {
        return 0;
    }
    param_t_get_into(cfg.callsign(), call, sizeof(call));
    for (int i = 0; i < park_count; i++) {
        char name[96], path[160];
        pota_upload_name(call, parks[i], day, name, sizeof(name));
        snprintf(path, sizeof(path), "%s/%s", POTA_UPLOAD_DIR, name);
        if (qso_log_write_activation(parks[i], day, path) > 0) {
            written++;
        }
    }
    return written;
}

void pota_activation_saved(const qso_log_record_t *qso) {
    if (park_count == 0 || !qso || !qso->my_sig_info[0]) {
        return;
    }
    if (pota_activation_write_files(qso->time) < park_count) {
        msg_schedule_text_fmt("POTA: upload file not written (SD card?)");
        return;
    }
    int n = qso_log_activation_count(parks[0], qso->time);
    if (n < 0) {
        return;
    }
    const char *more = park_count > 1 ? " (each park)" : "";
    if (n < POTA_QSOS_NEEDED) {
        msg_schedule_text_fmt("%s: %i/%i%s", parks[0], n, POTA_QSOS_NEEDED, more);
    } else if (n == POTA_QSOS_NEEDED) {
        msg_schedule_text_fmt("%s: %i QSOs, a valid activation!%s", parks[0], n, more);
    } else {
        msg_schedule_text_fmt("%s: %i QSOs%s", parks[0], n, more);
    }
}

int pota_activation_count_today(void) {
    return park_count ? qso_log_activation_count(parks[0], time(NULL)) : -1;
}

bool pota_activation_is_dupe(const char *call, qso_log_band_t band, qso_log_mode_t mode) {
    time_t now = time(NULL);

    for (int i = 0; i < park_count; i++) {
        if (qso_log_activation_dupe(call, band, mode, parks[i], now)) {
            return true;
        }
    }
    return false;
}

/* ---- park names ---- */

static struct {
    char ref[POTA_REF_LEN];
    char name[64];
} names[NAMES_MAX];
static int names_next;

const char *pota_park_name(const char *ref) {
    for (int i = 0; ref && i < NAMES_MAX; i++) {
        if (names[i].ref[0] && strcasecmp(names[i].ref, ref) == 0) {
            return names[i].name;
        }
    }
    return "";
}

typedef struct {
    char         ref[POTA_REF_LEN];
    net_result_t res;
    bool         ok;
    void (*done)(const char *ref, bool ok);
} lookup_t;

static bool looking_up;

static void lookup_work(void *ctx) {
    lookup_t *job = ctx;
    char      url[64];

    snprintf(url, sizeof(url), PARK_URL "%s", job->ref);
    job->ok = net_get(url, &job->res);
}

static void lookup_done(void *ctx) {
    lookup_t   *job = ctx;
    pota_park_t park;
    bool        ok = job->ok && pota_park_parse(job->res.body, &park);

    looking_up = false;
    if (ok) {
        snprintf(names[names_next].ref, sizeof(names[0].ref), "%s", job->ref);
        if (park.location[0]) {
            snprintf(names[names_next].name, sizeof(names[0].name), "%.40s, %.20s", park.name, park.location);
        } else {
            snprintf(names[names_next].name, sizeof(names[0].name), "%s", park.name);
        }
        names_next = (names_next + 1) % NAMES_MAX;
    }
    if (job->done) {
        job->done(job->ref, ok);
    }
    net_result_free(&job->res);
    free(job);
}

bool pota_park_lookup(const char *ref, void (*done)(const char *ref, bool ok)) {
    if (looking_up || !pota_ref_valid(ref)) {
        return false;
    }
    lookup_t *job = calloc(1, sizeof(*job));
    if (!job) {
        return false;
    }
    snprintf(job->ref, sizeof(job->ref), "%s", ref);
    job->done = done;
    if (!net_async(lookup_work, lookup_done, job)) {
        free(job);
        return false;
    }
    looking_up = true;
    return true;
}

/* ---- self-spots ---- */

typedef struct {
    char        *body;
    net_result_t res;
    bool         ok;
    char         park[POTA_REF_LEN];
    uint64_t     freq_hz;
    int          mode;
} spot_t;

static bool spotting;

static void spot_work(void *ctx) {
    spot_t *job = ctx;
    job->ok     = net_post_json(SPOT_URL, job->body, &job->res);
}

static void spot_done(void *ctx) {
    spot_t *job = ctx;

    spotting = false;
    if (job->ok) {
        last_spot_freq = job->freq_hz;
        last_spot_mode = job->mode;
        last_spot_time = time(NULL);
        msg_schedule_text_fmt("Spotted at %s on %llu.%llu kHz", job->park,
                              (unsigned long long)(job->freq_hz / 1000),
                              (unsigned long long)(job->freq_hz % 1000 / 100));
    } else {
        msg_schedule_text_fmt("Spot not sent: %s", job->res.error);
    }
    net_result_free(&job->res);
    free(job->body);
    free(job);
}

pota_spot_start_t pota_self_spot(uint64_t freq_hz, qso_log_mode_t mode, const char *comment) {
    char call[PARAM_TEXT_MAX];

    if (park_count == 0) {
        return POTA_SPOT_NOT_ACTIVATING;
    }
    param_t_get_into(cfg.callsign(), call, sizeof(call));
    for (char *c = call; *c; c++) {
        if (*c >= 'a' && *c <= 'z') *c -= 'a' - 'A';
    }
    if (!call[0]) {
        return POTA_SPOT_NO_CALL;
    }
    /* what pota.app's own spot form checks before it posts */
    if (!pota_call_valid(call)) {
        return POTA_SPOT_BAD_CALL;
    }
    if (freq_hz <= 1000000 || qso_log_freq_to_band(freq_hz) == BAND_OTHER) {
        return POTA_SPOT_OUT_OF_BAND;
    }
    if (!pota_spot_mode(mode)[0]) {
        return POTA_SPOT_BAD_MODE;
    }
    if (spotting) {
        return POTA_SPOT_BUSY;
    }
    /* the same frequency and mode again: not before the gap */
    uint64_t moved = freq_hz > last_spot_freq ? freq_hz - last_spot_freq : last_spot_freq - freq_hz;
    if (last_spot_time && moved < 1000 && (int)mode == last_spot_mode &&
        time(NULL) - last_spot_time < POTA_SPOT_GAP_SEC) {
        return POTA_SPOT_TOO_SOON;
    }
    spot_t *job = calloc(1, sizeof(*job));
    if (!job) {
        return POTA_SPOT_FAILED;
    }
    job->body = pota_spot_json(call, parks[0], freq_hz, mode, comment, "X6100-KR8MER");
    if (!job->body) {
        free(job);
        return POTA_SPOT_FAILED;
    }
    snprintf(job->park, sizeof(job->park), "%s", parks[0]);
    job->freq_hz = freq_hz;
    job->mode    = mode;
    if (!net_async(spot_work, spot_done, job)) {
        free(job->body);
        free(job);
        return POTA_SPOT_FAILED;
    }
    spotting = true;
    return POTA_SPOT_STARTED;
}

uint64_t pota_last_spot_freq(void) {
    return last_spot_freq;
}
