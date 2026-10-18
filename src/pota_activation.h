/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  POTA activating. With your park(s) set (Activate POTA window), every QSO
 *  logged, by hand or by FT8/FT4, carries them as MY_SIG_INFO; after each
 *  one the day's upload file for each park is rewritten on the DATA
 *  partition (pota/KR8MER@US-1234-20261018.adi, the name pota.app expects)
 *  and the message says how far the activation is ("US-1234: 7/10").
 *
 *  Everything here runs on the LVGL thread.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "pota_ref.h"
#include "qso_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef POTA_UPLOAD_DIR
#define POTA_UPLOAD_DIR "/mnt/pota"
#endif

#define POTA_QSOS_NEEDED 10   /* a valid activation, per park and UTC day */

/* Read the parks from settings (after the settings are loaded) */
void pota_activation_init(void);

/* Parks being activated; 0 when not activating */
int pota_activation_parks(char out[][POTA_REF_LEN], int max);
bool pota_activation_active(void);

/* Start (or change) an activation from typed references; "" ends it.
 * Returns false, with the reason in why, when a reference is not valid. */
bool pota_activation_set(const char *text, char *why, size_t why_len);

/* Give a QSO about to be saved your park(s) as MY_SIG / MY_SIG_INFO */
void pota_activation_tag(qso_log_record_t *qso);

/* A QSO tagged by pota_activation_tag was saved: rewrite the upload files
 * and say how far each park is. */
void pota_activation_saved(const qso_log_record_t *qso);

/* Write the upload files for the UTC day of `day` now; returns the number
 * of files written. */
int pota_activation_write_files(time_t day);

/* The first park's QSO count today (dupes left out), -1 when not activating */
int pota_activation_count_today(void);

/* True when `call` is a dupe for this activation today on this band and
 * mode group, in any of your parks */
bool pota_activation_is_dupe(const char *call, qso_log_band_t band, qso_log_mode_t mode);

/* ---- network: park names and self-spots ---- */

/* The park's name, if already looked up ("" otherwise) */
const char *pota_park_name(const char *ref);

/* Look the park up on pota.app in the background; done(ref, ok) runs on
 * the LVGL thread. Returns false when it could not start (busy, no memory). */
bool pota_park_lookup(const char *ref, void (*done)(const char *ref, bool ok));

typedef enum {
    POTA_SPOT_STARTED,
    POTA_SPOT_NOT_ACTIVATING,
    POTA_SPOT_NO_CALL,
    POTA_SPOT_BAD_CALL,     /* pota.app would refuse the callsign */
    POTA_SPOT_OUT_OF_BAND,  /* not on an amateur band pota.app knows */
    POTA_SPOT_BAD_MODE,     /* a mode with no POTA name */
    POTA_SPOT_TOO_SOON,     /* the same spot again within POTA_SPOT_GAP_SEC */
    POTA_SPOT_BUSY,
    POTA_SPOT_FAILED,
} pota_spot_start_t;

#define POTA_SPOT_GAP_SEC 120

/* Spot yourself at your first park, on this frequency and mode, with the
 * comment. The result shows as a message. */
pota_spot_start_t pota_self_spot(uint64_t freq_hz, qso_log_mode_t mode, const char *comment);

/* Frequency and mode of the last spot sent, 0 when none this activation */
uint64_t pota_last_spot_freq(void);

#ifdef __cplusplus
}
#endif
