/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  On-line services for the log: HamQTH lookup, QRZ Logbook upload.
 *
 *  HamQTH (https://www.hamqth.com/developers): log in with the operator's
 *  own account for a session id, good for an hour; we reuse it for 55
 *  minutes and log in again once when HamQTH says it expired.
 *
 *  QRZ Logbook (https://www.qrz.com/docs/logbook/QRZLogbookAPI.html): a form
 *  POST of KEY, ACTION=INSERT and one ADIF record per QSO. Never
 *  OPTION=REPLACE, which would overwrite QSOs already confirmed. A QSO QRZ
 *  reports as a duplicate is already there, so it counts as sent.
 */

#include "log_online.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adif.h"
#include "cfg/cfg_api.h"
#include "net_fetch.h"
#include "qso_log.h"

#define HAMQTH_URL      "https://www.hamqth.com/xml.php"
#define HAMQTH_PRG      "X6100-KR8MER"
#define HAMQTH_REUSE_S  (55 * 60)
#define QRZ_URL         "https://logbook.qrz.com/api"
#define QRZ_GIVE_UP     3       // failures in a row that end a batch

/* Passwords and keys do not linger in freed memory */
static void wipe(void *p, size_t n) {
    volatile unsigned char *v = p;
    while (n--) *v++ = 0;
}

/* A setting without surrounding white space (typed keys often carry some) */
static void setting(ParamText *p, char *buf, size_t n) {
    param_t_get_into(p, buf, n);

    char *s = buf;
    while (*s == ' ' || *s == '\t') s++;
    memmove(buf, s, strlen(s) + 1);
    size_t len = strlen(buf);
    while (len && (buf[len - 1] == ' ' || buf[len - 1] == '\t')) buf[--len] = 0;
}

/* ---- HamQTH ---- */

/* Session cache, LVGL thread only */
static struct {
    char   id[64];
    char   user[PARAM_TEXT_MAX];
    time_t when;
} session;

typedef struct {
    char                     user[PARAM_TEXT_MAX];
    char                     pass[PARAM_TEXT_MAX];
    char                     session[64];   // in: cached (or ""); out: the one in use
    bool                     session_new;
    char                     call[16];
    callbook_status_t        status;
    callbook_info_t          info;
    char                     error[128];
    log_online_lookup_done_t done;
    void                    *done_user;
} lookup_job_t;

bool log_online_hamqth_ready(void) {
    char user[PARAM_TEXT_MAX];

    setting(cfg.hamqth_user(), user, sizeof(user));
    return user[0] != 0;
}

void log_online_hamqth_forget(void) {
    memset(&session, 0, sizeof(session));
}

/* GET a HamQTH URL; false (with the reason in job->error) on network trouble */
static bool hamqth_get(lookup_job_t *job, const char *url, net_result_t *res) {
    if (!net_get(url, res)) {
        snprintf(job->error, sizeof(job->error), "%s", res->error);
        job->status = CALLBOOK_ERROR;
        return false;
    }
    return true;
}

static void hamqth_login(lookup_job_t *job) {
    char        *u = net_escape(job->user);
    char        *p = net_escape(job->pass);
    net_result_t res;

    job->session[0] = 0;
    if (!u || !p) {
        snprintf(job->error, sizeof(job->error), "Out of memory");
        job->status = CALLBOOK_ERROR;
        goto done;
    }

    size_t n   = strlen(HAMQTH_URL) + strlen(u) + strlen(p) + 16;
    char  *url = malloc(n);
    if (!url) {
        snprintf(job->error, sizeof(job->error), "Out of memory");
        job->status = CALLBOOK_ERROR;
        goto done;
    }
    snprintf(url, n, HAMQTH_URL "?u=%s&p=%s", u, p);
    if (hamqth_get(job, url, &res)) {
        job->status = hamqth_parse_session(res.body, job->session, sizeof(job->session), job->error,
                                           sizeof(job->error));
        job->session_new = job->status == CALLBOOK_OK;
        net_result_free(&res);
    }
    wipe(url, n);
    free(url);

done:
    if (p) wipe(p, strlen(p));
    free(u);
    free(p);
}

static void hamqth_search(lookup_job_t *job) {
    char        *call = net_escape(job->call);
    char         url[256];
    net_result_t res;

    if (!call) {
        snprintf(job->error, sizeof(job->error), "Out of memory");
        job->status = CALLBOOK_ERROR;
        return;
    }
    snprintf(url, sizeof(url), HAMQTH_URL "?id=%s&callsign=%s&prg=" HAMQTH_PRG, job->session, call);
    free(call);
    if (hamqth_get(job, url, &res)) {
        job->status = hamqth_parse_search(res.body, &job->info, job->error, sizeof(job->error));
        net_result_free(&res);
    }
}

/* Worker thread */
static void lookup_work(void *ctx) {
    lookup_job_t *job = ctx;

    if (!job->session[0]) {
        hamqth_login(job);
        if (job->status != CALLBOOK_OK) return;
    }
    if (!job->call[0]) {
        job->status = CALLBOOK_OK;  // a login check: done
        return;
    }
    hamqth_search(job);

    /* a cached session HamQTH has dropped: once more with a new one */
    if (job->status == CALLBOOK_EXPIRED && !job->session_new) {
        hamqth_login(job);
        if (job->status == CALLBOOK_OK) {
            hamqth_search(job);
        }
    }
}

/* LVGL thread */
static void lookup_done(void *ctx) {
    lookup_job_t *job = ctx;

    if (job->session_new && job->status != CALLBOOK_EXPIRED && job->status != CALLBOOK_BAD_LOGIN) {
        snprintf(session.id, sizeof(session.id), "%s", job->session);
        snprintf(session.user, sizeof(session.user), "%s", job->user);
        session.when = time(NULL);
    } else if (job->status == CALLBOOK_EXPIRED || job->status == CALLBOOK_BAD_LOGIN) {
        log_online_hamqth_forget();
    }
    if (job->done) {
        job->done(job->call, job->status, &job->info, job->error, job->done_user);
    }
    wipe(job, sizeof(*job));
    free(job);
}

log_online_start_t log_online_lookup(const char *call, log_online_lookup_done_t done, void *user) {
    lookup_job_t *job = calloc(1, sizeof(*job));

    if (!job) {
        return LOG_ONLINE_FAILED;
    }
    setting(cfg.hamqth_user(), job->user, sizeof(job->user));
    setting(cfg.hamqth_pass(), job->pass, sizeof(job->pass));
    if (!job->user[0] || !job->pass[0]) {
        wipe(job, sizeof(*job));
        free(job);
        return LOG_ONLINE_NO_ACCOUNT;
    }
    snprintf(job->call, sizeof(job->call), "%s", call ? call : "");
    job->done      = done;
    job->done_user = user;

    /* a login check always logs in; a lookup reuses a fresh session */
    time_t now = time(NULL);
    if (job->call[0] && session.id[0] && strcmp(session.user, job->user) == 0 && now >= session.when &&
        now - session.when < HAMQTH_REUSE_S) {
        snprintf(job->session, sizeof(job->session), "%s", session.id);
    }

    if (!net_async(lookup_work, lookup_done, job)) {
        wipe(job, sizeof(*job));
        free(job);
        return LOG_ONLINE_FAILED;
    }
    return LOG_ONLINE_STARTED;
}

/* ---- QRZ Logbook ---- */

typedef struct {
    int64_t      id;
    char         call[32];
    char        *adif;
    bool         tried;
    qrz_result_t result;
    char         reason[128];
} upload_item_t;

typedef struct {
    char                     key[PARAM_TEXT_MAX];
    int                      n;
    upload_item_t           *items;
    log_online_upload_done_t done;
    void                    *done_user;
} upload_job_t;

static bool upload_busy;    // one upload at a time (LVGL thread only)

bool log_online_qrz_ready(void) {
    char key[PARAM_TEXT_MAX];

    setting(cfg.qrz_key(), key, sizeof(key));
    bool ready = key[0] != 0;
    wipe(key, sizeof(key));
    return ready;
}

/* One INSERT; never OPTION=REPLACE. False on network trouble. */
static bool qrz_insert(const char *key, upload_item_t *item) {
    char        *k    = net_escape(key);
    char        *adif = net_escape(item->adif);
    net_result_t res;
    bool         net_ok = false;

    item->tried  = true;
    item->result = QRZ_RESULT_FAIL;
    if (!k || !adif) {
        snprintf(item->reason, sizeof(item->reason), "Out of memory");
        goto done;
    }

    size_t n    = strlen(k) + strlen(adif) + 32;
    char  *form = malloc(n);
    if (!form) {
        snprintf(item->reason, sizeof(item->reason), "Out of memory");
        goto done;
    }
    snprintf(form, n, "KEY=%s&ACTION=INSERT&ADIF=%s", k, adif);
    net_ok = net_post_form(QRZ_URL, form, &res);
    if (net_ok) {
        item->result = qrz_parse_reply(res.body, NULL, 0, item->reason, sizeof(item->reason));
        net_result_free(&res);
    } else {
        snprintf(item->reason, sizeof(item->reason), "%s", res.error);
    }
    wipe(form, n);
    free(form);

done:
    if (k) wipe(k, strlen(k));
    free(k);
    free(adif);
    return net_ok;
}

/* Worker thread */
static void upload_work(void *ctx) {
    upload_job_t *job   = ctx;
    int           fails = 0;

    for (int i = 0; i < job->n; i++) {
        /* a dead network or a refused key fails every QSO the same way */
        if (!qrz_insert(job->key, &job->items[i])) break;
        fails = job->items[i].result == QRZ_RESULT_FAIL ? fails + 1 : 0;
        if (fails >= QRZ_GIVE_UP) break;
    }
}

/* LVGL thread */
static void upload_done(void *ctx) {
    upload_job_t       *job = ctx;
    log_online_upload_t res = {.result = QRZ_RESULT_FAIL};

    for (int i = 0; i < job->n; i++) {
        upload_item_t *it = &job->items[i];

        if (!it->tried) continue;
        switch (it->result) {
            case QRZ_RESULT_OK:
                res.uploaded++;
                qso_log_mark_qrz_sent(it->id);
                break;
            case QRZ_RESULT_DUPLICATE:
                res.duplicates++;
                qso_log_mark_qrz_sent(it->id);
                break;
            default:
                if (!res.failed++) {
                    snprintf(res.reason, sizeof(res.reason), "%s", it->reason);
                }
                break;
        }
    }
    if (job->n > 0) {
        res.result = job->items[0].result;
        snprintf(res.call, sizeof(res.call), "%s", job->items[0].call);
    }
    res.remaining = qso_log_qrz_unsent_count();
    upload_busy   = false;

    if (job->done) {
        job->done(&res, job->done_user);
    }
    for (int i = 0; i < job->n; i++) {
        free(job->items[i].adif);
    }
    free(job->items);
    wipe(job->key, sizeof(job->key));
    free(job);
}

/* Takes the rows (ADIF is made here, on the LVGL thread) and starts the job */
static log_online_start_t upload_start(const qso_log_row_t *rows, int n, log_online_upload_done_t done,
                                       void *user) {
    upload_job_t *job = calloc(1, sizeof(*job));

    if (!job) {
        return LOG_ONLINE_FAILED;
    }
    setting(cfg.qrz_key(), job->key, sizeof(job->key));
    job->items = calloc(n, sizeof(*job->items));
    if (!job->items) {
        goto fail;
    }
    for (int i = 0; i < n; i++) {
        upload_item_t *it = &job->items[i];

        it->id   = rows[i].id;
        it->adif = adif_record_string(&rows[i].rec);
        snprintf(it->call, sizeof(it->call), "%s", rows[i].rec.remote_call);
        job->n = i + 1;     // so a failure frees what is made so far
        if (!it->adif) {
            goto fail;
        }
    }
    job->done      = done;
    job->done_user = user;

    if (!net_async(upload_work, upload_done, job)) {
        goto fail;
    }
    upload_busy = true;
    return LOG_ONLINE_STARTED;

fail:
    for (int i = 0; i < job->n; i++) {
        free(job->items[i].adif);
    }
    free(job->items);
    wipe(job->key, sizeof(job->key));
    free(job);
    return LOG_ONLINE_FAILED;
}

log_online_start_t log_online_qrz_upload_qso(time_t ts, const char *call, log_online_upload_done_t done,
                                             void *user) {
    qso_log_row_t row;

    if (!log_online_qrz_ready()) {
        return LOG_ONLINE_NO_ACCOUNT;
    }
    if (upload_busy) {
        return LOG_ONLINE_BUSY;
    }
    if (!qso_log_find(ts, call, &row)) {
        return LOG_ONLINE_FAILED;
    }
    return upload_start(&row, 1, done, user);
}

log_online_start_t log_online_qrz_upload_unsent(log_online_upload_done_t done, void *user) {
    if (!log_online_qrz_ready()) {
        return LOG_ONLINE_NO_ACCOUNT;
    }
    if (upload_busy) {
        return LOG_ONLINE_BUSY;
    }

    qso_log_row_t *rows = malloc(LOG_ONLINE_QRZ_BATCH * sizeof(*rows));
    if (!rows) {
        return LOG_ONLINE_FAILED;
    }
    int n = qso_log_qrz_unsent(rows, LOG_ONLINE_QRZ_BATCH);

    log_online_start_t started;
    if (n < 0) {
        started = LOG_ONLINE_FAILED;
    } else if (n == 0) {
        started = LOG_ONLINE_NOTHING;
    } else {
        started = upload_start(rows, n, done, user);
    }
    free(rows);
    return started;
}
