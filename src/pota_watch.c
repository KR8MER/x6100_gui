/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "pota_watch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "dialog_hunter.h"
#include "msg.h"
#include "net_fetch.h"
#include "pota_spots.h"
#include "qso_log.h"

#define SPOTS_URL     "https://api.pota.app/spot/activator"
#define PERIOD_MS     (5 * 60 * 1000)
#define MAX_SPOTS     200
#define SEEN_MAX      256           // activations already announced
#define SEEN_KEY      48
#define NAMED_MAX     1             // spots named per message (it scrolls past one)

typedef struct {
    net_result_t res;
    bool         ok;
} job_t;

static lv_timer_t *timer;
static bool        fetching;
static char        seen[SEEN_MAX][SEEN_KEY];
static int         seen_next;

static bool seen_before(const pota_spot_t *s) {
    char key[SEEN_KEY];

    snprintf(key, sizeof(key), "%s@%s", s->activator, s->ref);
    for (int i = 0; i < SEEN_MAX; i++) {
        if (strcmp(seen[i], key) == 0) {
            return true;
        }
    }
    snprintf(seen[seen_next], SEEN_KEY, "%s", key);
    seen_next = (seen_next + 1) % SEEN_MAX;
    return false;
}

static void work(void *ctx) {
    job_t *job = ctx;
    job->ok    = net_get(SPOTS_URL, &job->res);
}

static void done(void *ctx) {
    job_t        *job   = ctx;
    pota_spot_t  *spots = NULL;
    int           mode  = param_i_get(cfg.ui.pota_alerts());
    char          named[160] = "";
    int           count = 0;

    fetching = false;
    /* Off now, or the window opened meanwhile: drop it quietly */
    if (!job->ok || mode == POTA_ALERTS_OFF || dialog_type_is_run(dialog_hunter)) {
        goto out;
    }
    spots = malloc(sizeof(*spots) * MAX_SPOTS);
    int n = spots ? pota_spots_parse(job->res.body, spots, MAX_SPOTS) : -1;
    qso_log_band_t band = qso_log_freq_to_band(cparam_i_get(cfg.cur.fg_freq()));

    for (int i = 0; i < n; i++) {
        pota_spot_t *s = &spots[i];

        if (mode == POTA_ALERTS_THIS_BAND && qso_log_freq_to_band(s->freq_hz) != band) continue;
        if (qso_log_has_ref(s->ref)) continue;
        if (seen_before(s)) continue;

        if (count < NAMED_MAX) {
            size_t len = strlen(named);
            snprintf(named + len, sizeof(named) - len, "%s%s %s %i.%03i %s", count ? ", " : "", s->activator,
                     s->ref, s->freq_hz / 1000000, (s->freq_hz / 1000) % 1000, s->mode);
        }
        count++;
    }
    if (count > NAMED_MAX) {
        msg_schedule_long_text_fmt("New park: %s (+%i more)", named, count - NAMED_MAX);
    } else if (count) {
        msg_schedule_long_text_fmt("New park: %s", named);
    }
out:
    free(spots);
    net_result_free(&job->res);
    free(job);
}

static void tick(lv_timer_t *t) {
    (void)t;
    if (fetching || param_i_get(cfg.ui.pota_alerts()) == POTA_ALERTS_OFF || dialog_type_is_run(dialog_hunter)) {
        return;
    }
    job_t *job = calloc(1, sizeof(*job));
    if (job && net_async(work, done, job)) {
        fetching = true;
    } else {
        free(job);
    }
}

void pota_watch_init(void) {
    if (!timer) {
        timer = lv_timer_create(tick, PERIOD_MS, NULL);
    }
}

const char *pota_watch_label(void) {
    switch (param_i_get(cfg.ui.pota_alerts())) {
        case POTA_ALERTS_THIS_BAND:
            return "Alerts:\nThis band";
        case POTA_ALERTS_ALL_BANDS:
            return "Alerts:\nAll bands";
        default:
            return "Alerts:\nOff";
    }
}

void pota_watch_cycle(void) {
    int next = (param_i_get(cfg.ui.pota_alerts()) + 1) % POTA_ALERTS_COUNT;

    param_i_set(cfg.ui.pota_alerts(), next);
    switch (next) {
        case POTA_ALERTS_THIS_BAND:
            msg_update_text_fmt("Every 5 min: parks you need on this band");
            break;
        case POTA_ALERTS_ALL_BANDS:
            msg_update_text_fmt("Every 5 min: parks you need on any band");
            break;
        default:
            msg_update_text_fmt("POTA alerts off");
            break;
    }
}
