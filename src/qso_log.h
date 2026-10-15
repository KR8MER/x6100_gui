/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2024 Georgy Dyuldin aka R2RFE
 */

#pragma once

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BAND_OTHER  = 0,
    BAND_6M     = 6,
    BAND_10M    = 10,
    BAND_12M    = 12,
    BAND_15M    = 15,
    BAND_17M    = 17,
    BAND_20M    = 20,
    BAND_30M    = 30,
    BAND_40M    = 40,
    BAND_60M    = 60,
    BAND_80M    = 80,
    BAND_160M   = 160,
} qso_log_band_t;


typedef enum {
    MODE_OTHER,
    MODE_SSB,
    MODE_AM,
    MODE_FM,
    MODE_CW,
    MODE_FT8,
    MODE_FT4,
    MODE_RTTY,
    MODE_JS8 = 8,       /* stored in qso_log.db: never reused or changed */
} qso_log_mode_t;

typedef struct {
    char local_call[32];
    char remote_call[32];
    time_t time;
    qso_log_mode_t mode;
    int rsts;
    int rstr;
    float freq_mhz;
    qso_log_band_t band;
    char name[64];
    char qth[64];
    char local_grid[8];
    char remote_grid[8];
    char sig[8];            // ADIF SIG: "POTA", "SOTA" (empty: none)
    char sig_info[24];      // ADIF SIG_INFO: park or summit reference
} qso_log_record_t;


typedef enum {
    SEARCH_WORKED_NO,
    SEARCH_WORKED_YES,
    SEARCH_WORKED_SAME_MODE,
} qso_log_search_worked_t;


bool qso_log_init();

int qso_log_record_save(qso_log_record_t qso);

void qso_log_import_adif(const char *path);

/* Newest QSOs first, at most max. Returns how many were written to out. */
int qso_log_recent(qso_log_record_t *out, int max);

/* Write the whole log as ADIF (a new file; an old one is replaced).
 * Returns the QSO count, or -1 on error. */
int qso_log_export_adif(const char *path);

/* A QSO with the row id that names it in the log */
typedef struct {
    int64_t          id;
    qso_log_record_t rec;
} qso_log_row_t;

/* QSOs not yet uploaded to the QRZ Logbook, oldest first, at most max.
 * Returns how many were written to out, -1 on error. */
int qso_log_qrz_unsent(qso_log_row_t *out, int max);

/* How many QSOs are not yet uploaded to QRZ, -1 on error */
int qso_log_qrz_unsent_count(void);

/* The QSO logged at this time with this call */
bool qso_log_find(time_t ts, const char *remote_call, qso_log_row_t *out);

/* Uploaded to QRZ (or QRZ already had it) */
bool qso_log_mark_qrz_sent(int64_t id);

/* True when a QSO with this POTA/SOTA reference is already in the log */
bool qso_log_has_ref(const char *ref);

/**
 * Create qso log recort struct.
 * Required params: `local_call`, `remote_call`, qso_time`, `mode`, `rsts`, `rstr`,  and `freq_mhz`
 */
qso_log_record_t qso_log_record_create(const char *local_call, const char *remote_call,
                                       time_t qso_time, qso_log_mode_t mode, int rsts, int rstr, uint64_t freq_hz,
                                       const char *name, const char *qth,
                                       const char *local_grid, const char *remote_grid);

/**
 * Search callsign in log.
 */
qso_log_search_worked_t qso_log_search_worked(const char *callsign, qso_log_mode_t mode, qso_log_band_t band);


qso_log_band_t qso_log_freq_to_band(uint64_t freq_hz);
