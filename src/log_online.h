/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  On-line services for the log: HamQTH lookup, QRZ Logbook upload.
 *
 *  Call everything here from the LVGL thread; the callbacks run there too.
 *  The requests themselves run on a worker thread (net_async), which gets
 *  copies of the settings and of the log rows it needs: it never reads cfg
 *  or the log database.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "callbook.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_ONLINE_STARTED,
    LOG_ONLINE_NO_ACCOUNT,  // no HamQTH login / no QRZ key set
    LOG_ONLINE_BUSY,        // an upload is already running
    LOG_ONLINE_NOTHING,     // nothing to upload
    LOG_ONLINE_FAILED,      // could not read the log or start the job
} log_online_start_t;

/* ---- HamQTH ---- */

/* `call` is the call looked up ("" for a login check); info is filled on
 * CALLBOOK_OK; error is HamQTH's (or the network's) message otherwise */
typedef void (*log_online_lookup_done_t)(const char *call, callbook_status_t status, const callbook_info_t *info,
                                         const char *error, void *user);

bool log_online_hamqth_ready(void);

/* Forget the cached session, e.g. after the login was changed */
void log_online_hamqth_forget(void);

/* Look a call up in the background. An empty call only logs in, to check
 * the user name and password. done runs unless this returns non-STARTED. */
log_online_start_t log_online_lookup(const char *call, log_online_lookup_done_t done, void *user);

/* ---- QRZ Logbook ---- */

#define LOG_ONLINE_QRZ_BATCH 200   // QSOs per Upload press

typedef struct {
    int          uploaded;
    int          duplicates;     // QRZ already had them: marked sent too
    int          failed;
    int          remaining;      // QSOs still not sent after this run, -1 unknown
    qrz_result_t result;         // a single QSO: what QRZ said
    char         call[32];       // a single QSO: its call
    char         reason[128];    // the first failure's reason
} log_online_upload_t;

typedef void (*log_online_upload_done_t)(const log_online_upload_t *res, void *user);

bool log_online_qrz_ready(void);

/* Upload the QSO logged at `ts` with `call` */
log_online_start_t log_online_qrz_upload_qso(time_t ts, const char *call, log_online_upload_done_t done,
                                             void *user);

/* Upload QSOs not sent yet, oldest first, LOG_ONLINE_QRZ_BATCH at most */
log_online_start_t log_online_qrz_upload_unsent(log_online_upload_done_t done, void *user);

#ifdef __cplusplus
}
#endif
