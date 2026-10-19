/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2024 Georgy Dyuldin aka R2RFE
 */

#include "qso_log.h"

#include "util.h"
#include "msg.h"
#include "adif.h"
#include "hunt_target.h"

#include <lvgl/src/misc/lv_log.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifndef QSO_LOG_DB_PATH
#define QSO_LOG_DB_PATH "/mnt/qso_log.db"
#endif

static sqlite3_stmt     *search_callsign_stmt=NULL;
static sqlite3          *db = NULL;


static bool create_tables();
static void* import_adif_thread(void* args);


bool qso_log_init() {
    int rc = sqlite3_open(QSO_LOG_DB_PATH, &db);

    if (rc != SQLITE_OK) {
        LV_LOG_ERROR("Can't open qso_log.db");
        return false;
    }
    return create_tables();
}

void qso_log_destruct() {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

void qso_log_import_adif(const char * path) {
    pthread_t thr;

    if (access(path, F_OK) != 0) {
        LV_LOG_USER("No ADI file to import");
        return;
    }
    if(pthread_create(&thr, NULL, import_adif_thread, (void*)path) != 0) {
        LV_LOG_ERROR("Import adif thread start failed");
    }
}

qso_log_band_t qso_log_freq_to_band(uint64_t freq_hz)
{
    uint32_t freq_khz = freq_hz  / 1000;

    switch (freq_khz)
    {
    case 1800 ... 2000:
        return BAND_160M;
        break;
    case 3500 ... 4000:
        return BAND_80M;
        break;
    case 5351 ... 5367:
        return BAND_60M;
        break;
    case 7000 ... 7300:
        return BAND_40M;
        break;
    case 10100 ... 10150:
        return BAND_30M;
        break;
    case 14000 ... 14350:
        return BAND_20M;
        break;
    case 18068 ... 18168:
        return BAND_17M;
        break;
    case 21000 ... 21450:
        return BAND_15M;
        break;
    case 24890 ... 24990:
        return BAND_12M;
        break;
    case 28000 ... 29700:
        return BAND_10M;
        break;
    case 50000 ... 54000:
        return BAND_6M;
        break;
    default:
        return BAND_OTHER;
        break;
    }
}

qso_log_record_t qso_log_record_create(const char * local_call, const char * remote_call,
    time_t qso_time, qso_log_mode_t mode, int rsts, int rstr, uint64_t freq_hz,
    const char * name, const char * qth, const char *local_grid, const char * remote_grid)
{
    qso_log_record_t rec = {
        .time = qso_time,
        .mode = mode,
        .rsts = rsts,
        .rstr = rstr,
        .freq_mhz = (float) freq_hz / 1000000,
        .band = qso_log_freq_to_band(freq_hz)
    };
    strncpy(rec.local_call, local_call, sizeof(rec.local_call) - 1);
    rec.local_call[sizeof(rec.local_call) - 1] = 0;

    strncpy(rec.remote_call, remote_call, sizeof(rec.remote_call) - 1);
    rec.remote_call[sizeof(rec.remote_call) - 1] = 0;

    if (name) {
        strncpy(rec.name, name, sizeof(rec.name) - 1);
        rec.name[sizeof(rec.name) - 1] = 0;
    }

    if (qth) {
        strncpy(rec.qth, qth, sizeof(rec.qth) - 1);
        rec.qth[sizeof(rec.qth) - 1] = 0;
    }

    if (local_grid) {
        strncpy(rec.local_grid, local_grid, sizeof(rec.local_grid) - 1);
        rec.local_grid[sizeof(rec.local_grid) - 1] = 0;
    }

    if (remote_grid) {
        strncpy(rec.remote_grid, remote_grid, sizeof(rec.remote_grid) - 1);
        rec.remote_grid[sizeof(rec.remote_grid) - 1] = 0;
    }
    return rec;
}


static inline int bind_optional_text(sqlite3_stmt * stmt, int pos, const char * val) {
    if (!val) {
        return sqlite3_bind_null(stmt, pos);
    } else {
        return sqlite3_bind_text(stmt, pos, val, strlen(val), 0);
    }
}

int qso_log_record_save(qso_log_record_t qso) {
    sqlite3_stmt    *stmt;
    int             rc;

    if (strlen(qso.local_call) == 0) {
        LV_LOG_ERROR("Local callsign is required");
        return -1;
    }
    if (strlen(qso.remote_call) == 0) {
        LV_LOG_ERROR("Remote callsign is required");
        return -1;
    }

    /* A pending POTA/SOTA hunt tags the next QSO with its activator */
    bool hunted = !qso.sig[0] &&
        hunt_target_match(qso.remote_call, qso.time, qso.sig, sizeof(qso.sig), qso.sig_info, sizeof(qso.sig_info));

    rc = sqlite3_prepare_v2(
        db, "INSERT OR IGNORE INTO qso_log ("
                "ts, freq, band, mode, local_callsign, remote_callsign, rsts, rstr, "
                "local_grid, remote_grid, op_name, canonized_remote_callsign, sig, sig_info, my_sig, my_sig_info"
            ") VALUES (datetime(:ts, 'unixepoch'), :freq, :band, :mode, :local_callsign, :remote_callsign, "
                ":rsts, :rstr, :local_grid, :remote_grid, :op_name, :canonized_remote_callsign, :sig, :sig_info, "
                ":my_sig, :my_sig_info)",
                       -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR("Error in prepairing query");
        return -1;
    }
    rc = sqlite3_bind_int64(stmt, sqlite3_bind_parameter_index(stmt, ":ts"), qso.time);
    if (rc != SQLITE_OK) {
        printf("check point: %i\n", rc);
        printf("column_id: %i\n", sqlite3_bind_parameter_index(stmt, ":ts"));
        fflush(stdout);
        goto fail;
    }
    rc = sqlite3_bind_double(stmt, sqlite3_bind_parameter_index(stmt, ":freq"), (double) qso.freq_mhz);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":band"), qso.band);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":mode"), qso.mode);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":local_callsign"), qso.local_call, strlen(qso.local_call), 0);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_text(stmt, sqlite3_bind_parameter_index(stmt, ":remote_callsign"), qso.remote_call, strlen(qso.remote_call), 0);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":rsts"), qso.rsts);
    if (rc != SQLITE_OK) goto fail;
    rc = sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, ":rstr"), qso.rstr);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":local_grid"), qso.local_grid);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":remote_grid"), qso.remote_grid);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":op_name"), qso.name);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":sig"), qso.sig[0] ? qso.sig : NULL);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":sig_info"), qso.sig_info[0] ? qso.sig_info : NULL);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":my_sig"), qso.my_sig[0] ? qso.my_sig : NULL);
    if (rc != SQLITE_OK) goto fail;
    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":my_sig_info"),
                            qso.my_sig_info[0] ? qso.my_sig_info : NULL);
    if (rc != SQLITE_OK) goto fail;

    char * canonized_remote_callsign = util_canonize_callsign(qso.remote_call, true);

    rc = bind_optional_text(stmt, sqlite3_bind_parameter_index(stmt, ":canonized_remote_callsign"), canonized_remote_callsign);
    if (rc != SQLITE_OK) {
        free(canonized_remote_callsign);
        goto fail;
    }

    if(sqlite3_step(stmt) != SQLITE_DONE) {
        char *sql = sqlite3_expanded_sql(stmt);
        printf("Error during execute: `%s`\n", sql ? sql : "?");
        sqlite3_free(sql);
        free(canonized_remote_callsign);
        goto fail;
    }

    int changed = sqlite3_changes(db);
    if (changed == 0) {
        char *sql = sqlite3_expanded_sql(stmt);
        printf("Not inserted `%s`\n", sql ? sql : "?");
        sqlite3_free(sql);
    }

    free(canonized_remote_callsign);

    sqlite3_finalize(stmt);

    /* Only a QSO that went in uses the hunt up, not an ignored duplicate */
    if (hunted && changed > 0) {
        hunt_target_done(qso.remote_call);
        msg_schedule_text_fmt("Logged %s at %s", qso.remote_call, qso.sig_info);
    }
    return changed;

fail:
    sqlite3_finalize(stmt);
    return -1;
}


qso_log_search_worked_t qso_log_search_worked(const char *callsign, qso_log_mode_t mode, qso_log_band_t band)
{
    int                         rc;
    qso_log_search_worked_t     worked = SEARCH_WORKED_NO;

    if (!search_callsign_stmt) {
        rc = sqlite3_prepare_v3(db, "SELECT DISTINCT band, mode FROM qso_log WHERE canonized_remote_callsign LIKE ?",
                        -1, SQLITE_PREPARE_PERSISTENT, &search_callsign_stmt, 0);
        if (rc != SQLITE_OK) {
            return -1;
        }
    } else {
        sqlite3_reset(search_callsign_stmt);
        sqlite3_clear_bindings(search_callsign_stmt);
    }

    char * canonized_callsign = util_canonize_callsign(callsign, true);
    if (!canonized_callsign) {
        canonized_callsign = strdup(callsign);
    }
    rc = sqlite3_bind_text(search_callsign_stmt, 1, canonized_callsign, strlen(canonized_callsign), 0);
    if (rc != SQLITE_OK) {
        free(canonized_callsign);
        return -1;
    }

    while (sqlite3_step(search_callsign_stmt) != SQLITE_DONE) {
        worked = SEARCH_WORKED_YES;
        if ((sqlite3_column_int(search_callsign_stmt, 0) == band) &&
            (sqlite3_column_int(search_callsign_stmt, 1) == mode))
        {
            worked = SEARCH_WORKED_SAME_MODE;
            break;
        }
    }

    free(canonized_callsign);
    return worked;
}


static void * import_adif_thread(void* args) {

    char *path = (char* )args;

    pthread_detach(pthread_self());

    qso_log_record_t * records = NULL;
    int cnt = adif_read(path, &records);
    if (cnt < 0) {
        cnt = 0;   // unreadable: nothing to import
    }
    char * canonized;
    size_t updated_rows = 0;
    size_t c = 0;
    for (size_t i = 0; i < cnt; i++) {
        int changed = qso_log_record_save(records[i]);
        c++;
        if (changed > 0) {
            updated_rows += changed;
        }
        if (c >= 10) {
            c = 0;
            msg_update_text_fmt("Importing QSO: %zu/%zu", i + 1, cnt);
        }
    }
    free(records);

    char new_path[128] = {0};
    snprintf(new_path, sizeof(new_path), "%s.bak", path);
    rename(path, new_path);
    msg_update_text_fmt("Imported %u QSOs from %i", updated_rows, cnt);
    pthread_exit(NULL);
}



static bool has_column(const char *table, const char *column) {
    sqlite3_stmt *stmt;
    char          sql[64];
    bool          found = false;

    snprintf(sql, sizeof(sql), "PRAGMA table_info(%s)", table);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        if (name && strcmp(name, column) == 0) {
            found = true;
            break;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

static void copy_col(char *dst, size_t len, sqlite3_stmt *stmt, int col) {
    const char *v = (const char *)sqlite3_column_text(stmt, col);
    snprintf(dst, len, "%s", v ? v : "");
}

static void read_row(sqlite3_stmt *stmt, qso_log_record_t *r) {
    memset(r, 0, sizeof(*r));
    r->time     = (time_t)sqlite3_column_int64(stmt, 0);
    r->freq_mhz = (float)sqlite3_column_double(stmt, 1);
    r->band     = sqlite3_column_int(stmt, 2);
    r->mode     = sqlite3_column_int(stmt, 3);
    copy_col(r->local_call, sizeof(r->local_call), stmt, 4);
    copy_col(r->remote_call, sizeof(r->remote_call), stmt, 5);
    r->rsts = sqlite3_column_int(stmt, 6);
    r->rstr = sqlite3_column_int(stmt, 7);
    copy_col(r->local_grid, sizeof(r->local_grid), stmt, 8);
    copy_col(r->remote_grid, sizeof(r->remote_grid), stmt, 9);
    copy_col(r->name, sizeof(r->name), stmt, 10);
    copy_col(r->sig, sizeof(r->sig), stmt, 11);
    copy_col(r->sig_info, sizeof(r->sig_info), stmt, 12);
    copy_col(r->my_sig, sizeof(r->my_sig), stmt, 13);
    copy_col(r->my_sig_info, sizeof(r->my_sig_info), stmt, 14);
}

#define ROW_COLUMNS \
    "strftime('%s', ts), freq, band, mode, local_callsign, remote_callsign, rsts, rstr, " \
    "local_grid, remote_grid, op_name, sig, sig_info, my_sig, my_sig_info"
#define SELECT_ROWS     "SELECT " ROW_COLUMNS " FROM qso_log "
/* the same, with the row id as column ROWID_COL for the QRZ upload */
#define SELECT_ROWS_ID  "SELECT " ROW_COLUMNS ", rowid FROM qso_log "
#define ROWID_COL       15

int qso_log_recent(qso_log_record_t *out, int max) {
    sqlite3_stmt *stmt;
    int           n = 0;

    if (!db || sqlite3_prepare_v2(db, SELECT_ROWS "ORDER BY ts DESC LIMIT ?", -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, max);
    while (n < max && sqlite3_step(stmt) == SQLITE_ROW) {
        read_row(stmt, &out[n++]);
    }
    sqlite3_finalize(stmt);
    return n;
}

int qso_log_history(const char *call, qso_log_record_t *last) {
    sqlite3_stmt *stmt;
    int           n     = -1;
    char         *canon = util_canonize_callsign(call, true);

    if (!canon) {
        return -1;
    }
    if (db && sqlite3_prepare_v2(db,
                                 "SELECT " ROW_COLUMNS ", (SELECT COUNT(*) FROM qso_log "
                                 "WHERE canonized_remote_callsign = ?1 COLLATE NOCASE) FROM qso_log "
                                 "WHERE canonized_remote_callsign = ?1 COLLATE NOCASE ORDER BY ts DESC LIMIT 1",
                                 -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_bind_text(stmt, 1, canon, -1, SQLITE_TRANSIENT) == SQLITE_OK) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                if (last) read_row(stmt, last);
                n = sqlite3_column_int(stmt, ROWID_COL);   // the count follows the row columns
            } else if (rc == SQLITE_DONE) {
                n = 0;
            }
        }
        sqlite3_finalize(stmt);
    }
    free(canon);
    return n;
}

bool qso_log_has_ref(const char *ref) {
    sqlite3_stmt *stmt;
    bool          found = false;

    if (!db || sqlite3_prepare_v2(db, "SELECT 1 FROM qso_log WHERE sig_info = ? COLLATE NOCASE LIMIT 1", -1, &stmt,
                                  NULL) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, ref, -1, SQLITE_TRANSIENT);
    found = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return found;
}

bool qso_log_worked_at_ref_today(const char *call, const char *ref, time_t now) {
    sqlite3_stmt *stmt;
    bool          found = false;
    char         *canon = util_canonize_callsign(call, true);

    if (!canon) {
        return false;
    }
    if (!db || sqlite3_prepare_v2(db,
                                  "SELECT 1 FROM qso_log WHERE canonized_remote_callsign = ? COLLATE NOCASE "
                                  "AND sig_info = ? COLLATE NOCASE AND date(ts) = date(?, 'unixepoch') LIMIT 1",
                                  -1, &stmt, NULL) != SQLITE_OK) {
        free(canon);
        return false;
    }
    if (sqlite3_bind_text(stmt, 1, canon, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 2, ref, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_int64(stmt, 3, (sqlite3_int64)now) == SQLITE_OK) {
        found = sqlite3_step(stmt) == SQLITE_ROW;
    }
    sqlite3_finalize(stmt);
    free(canon);
    return found;
}

/* ---- POTA activation ---- */

/* The QSO was made at `park` (one of the comma-separated MY_SIG_INFO
 * references) on the UTC day of the bound time */
#define ACT_WHERE \
    "WHERE instr(',' || UPPER(REPLACE(IFNULL(my_sig_info, ''), ' ', '')) || ',', ',' || UPPER(?1) || ',') > 0 " \
    "AND date(ts) = date(?2, 'unixepoch') "
/* POTA's mode groups: a call is a dupe on the same band in the same group */
#define ACT_GROUP "CASE WHEN mode IN (1, 2, 3) THEN 1 WHEN mode = 4 THEN 0 ELSE 2 END"

qso_log_mode_group_t qso_log_mode_group(qso_log_mode_t mode) {
    switch (mode) {
        case MODE_SSB:
        case MODE_AM:
        case MODE_FM:
            return MODE_GROUP_PHONE;
        case MODE_CW:
            return MODE_GROUP_CW;
        default:
            return MODE_GROUP_DATA;
    }
}

static bool act_bind(sqlite3_stmt *stmt, const char *park, time_t day) {
    return sqlite3_bind_text(stmt, 1, park, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
           sqlite3_bind_int64(stmt, 2, (sqlite3_int64)day) == SQLITE_OK;
}

int qso_log_activation_count(const char *park, time_t day) {
    sqlite3_stmt *stmt;
    int           n = -1;

    if (!db || !park || !park[0] ||
        sqlite3_prepare_v2(db,
                           "SELECT COUNT(*) FROM (SELECT DISTINCT canonized_remote_callsign, band, " ACT_GROUP
                           " FROM qso_log " ACT_WHERE ")",
                           -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    if (act_bind(stmt, park, day) && sqlite3_step(stmt) == SQLITE_ROW) {
        n = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return n;
}

bool qso_log_activation_dupe(const char *call, qso_log_band_t band, qso_log_mode_t mode, const char *park,
                             time_t day) {
    sqlite3_stmt *stmt;
    bool          dupe  = false;
    char         *canon = util_canonize_callsign(call, true);

    if (!canon) {
        return false;
    }
    if (db && park && park[0] &&
        sqlite3_prepare_v2(db,
                           "SELECT 1 FROM qso_log " ACT_WHERE
                           "AND canonized_remote_callsign = ?3 COLLATE NOCASE AND band = ?4 AND " ACT_GROUP
                           " = ?5 LIMIT 1",
                           -1, &stmt, NULL) == SQLITE_OK) {
        if (act_bind(stmt, park, day) && sqlite3_bind_text(stmt, 3, canon, -1, SQLITE_TRANSIENT) == SQLITE_OK &&
            sqlite3_bind_int(stmt, 4, band) == SQLITE_OK &&
            sqlite3_bind_int(stmt, 5, qso_log_mode_group(mode)) == SQLITE_OK) {
            dupe = sqlite3_step(stmt) == SQLITE_ROW;
        }
        sqlite3_finalize(stmt);
    }
    free(canon);
    return dupe;
}

int qso_log_activation_summary(const char *park, time_t day, qso_log_band_count_t *out, int max) {
    sqlite3_stmt *stmt;
    int           n = 0;

    if (!db || !park || !park[0] || max <= 0 ||
        sqlite3_prepare_v2(db,
                           "SELECT band, grp, COUNT(*) FROM (SELECT DISTINCT canonized_remote_callsign, band, "
                           ACT_GROUP " AS grp FROM qso_log " ACT_WHERE ") GROUP BY band, grp ORDER BY band DESC, grp",
                           -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    if (act_bind(stmt, park, day)) {
        while (n < max && sqlite3_step(stmt) == SQLITE_ROW) {
            out[n].band  = sqlite3_column_int(stmt, 0);
            out[n].group = sqlite3_column_int(stmt, 1);
            out[n].count = sqlite3_column_int(stmt, 2);
            n++;
        }
    }
    sqlite3_finalize(stmt);
    return n;
}

int qso_log_write_activation(const char *park, time_t day, const char *path) {
    sqlite3_stmt *stmt;
    int           n = 0;
    char          tmp[256];

    if (!db || !park || !park[0] || snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp) ||
        sqlite3_prepare_v2(db, SELECT_ROWS ACT_WHERE "ORDER BY ts", -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    if (!act_bind(stmt, park, day)) {
        sqlite3_finalize(stmt);
        return -1;
    }
    unlink(tmp);
    adif_log l = adif_log_init(tmp);
    if (!l) {
        sqlite3_finalize(stmt);
        return -1;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        qso_log_record_t r;
        read_row(stmt, &r);
        /* POTA takes one park per file: this one */
        snprintf(r.my_sig, sizeof(r.my_sig), "POTA");
        snprintf(r.my_sig_info, sizeof(r.my_sig_info), "%s", park);
        adif_add_qso(l, r);
        n++;
    }
    adif_log_close(l);
    free(l);
    sqlite3_finalize(stmt);
    if (n == 0) {
        unlink(tmp);
        unlink(path);   // no QSOs there (any more): no upload file
        return 0;
    }
    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }
    return n;
}

/* A P2P with a 2-fer is one QSO in two rows (one per park): QRZ gets it once */
#define QRZ_ONE_PER_QSO "SELECT MIN(rowid) FROM qso_log WHERE qrz_sent = 0 GROUP BY ts, remote_callsign"

int qso_log_qrz_unsent(qso_log_row_t *out, int max) {
    sqlite3_stmt *stmt;
    int           n  = 0;
    int           rc = SQLITE_DONE;

    if (!db || max <= 0) {
        return 0;
    }
    if (sqlite3_prepare_v2(db, SELECT_ROWS_ID "WHERE qrz_sent = 0 AND rowid IN (" QRZ_ONE_PER_QSO ") ORDER BY ts LIMIT ?", -1, &stmt, NULL) !=
        SQLITE_OK) {
        LV_LOG_ERROR("Can't read unsent QSOs: %s", sqlite3_errmsg(db));
        return -1;
    }
    if (sqlite3_bind_int(stmt, 1, max) != SQLITE_OK) goto fail;

    while (n < max && (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        read_row(stmt, &out[n].rec);
        out[n].id = sqlite3_column_int64(stmt, ROWID_COL);
        n++;
    }
    if (n < max && rc != SQLITE_DONE) goto fail;
    sqlite3_finalize(stmt);
    return n;

fail:
    sqlite3_finalize(stmt);
    return -1;
}

int qso_log_qrz_unsent_count(void) {
    sqlite3_stmt *stmt;
    int           n = -1;

    if (!db || sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM qso_log WHERE qrz_sent = 0 AND rowid IN (" QRZ_ONE_PER_QSO ")", -1, &stmt, NULL) !=
                   SQLITE_OK) {
        return -1;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        n = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return n;
}

bool qso_log_find(time_t ts, const char *remote_call, qso_log_row_t *out) {
    sqlite3_stmt *stmt;
    bool          found = false;

    if (!db || !remote_call) {
        return false;
    }
    /* (ts, remote_callsign) is the log's unique key */
    if (sqlite3_prepare_v2(db, SELECT_ROWS_ID "WHERE ts = datetime(?, 'unixepoch') AND remote_callsign = ? LIMIT 1",
                           -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    if (sqlite3_bind_int64(stmt, 1, ts) != SQLITE_OK) goto done;
    if (sqlite3_bind_text(stmt, 2, remote_call, -1, SQLITE_TRANSIENT) != SQLITE_OK) goto done;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        read_row(stmt, &out->rec);
        out->id = sqlite3_column_int64(stmt, ROWID_COL);
        found   = true;
    }

done:
    sqlite3_finalize(stmt);
    return found;
}

bool qso_log_mark_qrz_sent(int64_t id) {
    sqlite3_stmt *stmt;
    bool          ok = false;

    if (!db || sqlite3_prepare_v2(db, "UPDATE qso_log SET qrz_sent = 1 WHERE rowid IN (SELECT b.rowid FROM qso_log a JOIN qso_log b "
                                  "ON a.ts = b.ts AND a.remote_callsign = b.remote_callsign WHERE a.rowid = ?1)", -1, &stmt, NULL) !=
                   SQLITE_OK) {
        return false;
    }
    if (sqlite3_bind_int64(stmt, 1, id) == SQLITE_OK && sqlite3_step(stmt) == SQLITE_DONE) {
        ok = sqlite3_changes(db) >= 1;   // a 2-fer P2P: every row of it
    }
    if (!ok) {
        LV_LOG_ERROR("Can't mark QSO %lld as sent to QRZ: %s", (long long)id, sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
    return ok;
}

int qso_log_export_adif(const char *path) {
    sqlite3_stmt *stmt;
    int           n = 0;

    if (!db || sqlite3_prepare_v2(db, SELECT_ROWS "ORDER BY ts", -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    /* Written aside and renamed over: a power cut mid-write leaves the
     * previous file, not half of one */
    char tmp[256];
    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp)) {
        sqlite3_finalize(stmt);
        return -1;
    }
    unlink(tmp);
    adif_log l = adif_log_init(tmp);
    if (!l) {
        sqlite3_finalize(stmt);
        return -1;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        qso_log_record_t r;
        read_row(stmt, &r);
        adif_add_qso(l, r);
        n++;
    }
    adif_log_close(l);
    free(l);
    sqlite3_finalize(stmt);
    if (rename(tmp, path) != 0) {
        unlink(tmp);
        return -1;
    }
    return n;
}

int qso_log_update_adif(void) {
    return qso_log_export_adif(QSO_LOG_ADIF_PATH);
}

static bool create_tables() {
    char    *err = 0;
    int     rc;

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS qso_log( "
            "ts              TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "freq            REAL CHECK ( freq > 0 ), "
            "band            INT NOT NULL, "
            "mode            INT NOT NULL, "
            "local_callsign  TEXT NOT NULL, "
            "remote_callsign TEXT NOT NULL, "
            "canonized_remote_callsign TEXT NOT NULL, "
            "rsts            INTEGER NOT NULL, "
            "rstr            INTEGER NOT NULL, "
            "local_qth       TEXT, "
            "remote_qth      TEXT, "
            "local_grid      TEXT, "
            "remote_grid     TEXT, "
            "op_name         TEXT, "
            "comment         TEXT "
        ")",
        NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR(err);
        return false;
    }

    /* Columns added later (POTA/SOTA references, QRZ upload state): add them
     * to old logs. A NOT NULL column needs its default here, which is also
     * what the rows already in the log get: old QSOs count as not uploaded. */
    static const struct {
        const char *name;
        const char *type;
    } added[] = {
        {"sig", "TEXT"},
        {"sig_info", "TEXT"},
        {"qrz_sent", "INTEGER NOT NULL DEFAULT 0"},
        {"my_sig", "TEXT"},
        {"my_sig_info", "TEXT"},
    };
    for (size_t i = 0; i < sizeof(added) / sizeof(added[0]); i++) {
        if (!has_column("qso_log", added[i].name)) {
            char sql[96];
            snprintf(sql, sizeof(sql), "ALTER TABLE qso_log ADD COLUMN %s %s", added[i].name, added[i].type);
            rc = sqlite3_exec(db, sql, NULL, NULL, &err);
            if (rc != SQLITE_OK) {
                LV_LOG_ERROR(err);
                sqlite3_free(err);
                return false;
            }
        }
    }

    rc = sqlite3_exec(db,
        "CREATE INDEX IF NOT EXISTS qso_log_idx_canonized_remote_callsign ON qso_log(canonized_remote_callsign COLLATE NOCASE)",
        NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR(err);
        return false;
    }
    rc = sqlite3_exec(db,
        "CREATE INDEX IF NOT EXISTS qso_log_idx_mode ON qso_log(mode)",
        NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR(err);
        return false;
    }
    rc = sqlite3_exec(db,
        "CREATE INDEX IF NOT EXISTS qso_log_idx_ts ON qso_log(ts)",
        NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR(err);
        return false;
    }

    /* One QSO per time and call, except one entry per park when the other
     * station is in two or more (POTA logs a 2-fer P2P as two entries that
     * differ only in SIG_INFO). The older index was on time and call. */
    rc = sqlite3_exec(db,
        "DROP INDEX IF EXISTS qso_log_idx_ts_call; "
        "CREATE UNIQUE INDEX IF NOT EXISTS qso_log_idx_ts_call_ref "
        "ON qso_log(ts, remote_callsign, IFNULL(sig_info, ''))",
        NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        LV_LOG_ERROR(err);
        return false;
    }

    return true;
}
