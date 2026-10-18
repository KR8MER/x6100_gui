/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  POTA references, upload file names, the self-spot request and the park
 *  look-up reply: the parts of POTA activating that need no radio or
 *  network, kept apart so they can be tested on a PC.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "qso_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POTA_REF_LEN   16      /* "US-12345" and room to spare */
#define POTA_PARKS_MAX 4       /* a 4-fer at most */

/* "US-1234", "VE-12345": as pota.app checks it, 1 or 2 capitals or digits,
 * a dash, 4 or 5 digits */
bool pota_ref_valid(const char *ref);

/* A callsign pota.app accepts on a spot (KR8MER, KR8MER/P, VE3/KR8MER),
 * upper case */
bool pota_call_valid(const char *call);

/* pota.app requires a comment on every spot */
#define POTA_SPOT_DEFAULT_COMMENT "CQ POTA"

/* Parks from text typed as "us-1234, US-5678" or "US-1234 US-5678": stored
 * upper case, duplicates dropped. Returns how many, 0 for none, -1 when one
 * is not a park reference (bad gets it, cut to bad_len). */
int pota_refs_parse(const char *text, char out[][POTA_REF_LEN], int max, char *bad, size_t bad_len);

/* "US-1234,US-5678" */
void pota_refs_join(char refs[][POTA_REF_LEN], int n, char *out, size_t len);

/* POTA's upload file name, "KR8MER@US-1234-20261018.adi" for the UTC day
 * of `day`. A / in the call (KR8MER/P) becomes _ (no / in file names). */
void pota_upload_name(const char *call, const char *park, time_t day, char *out, size_t len);

/* The mode POTA's spot list shows: "SSB", "CW", "FT8", ... */
const char *pota_spot_mode(qso_log_mode_t mode);

/* The self-spot request body for api.pota.app/spot, as pota.app sends it.
 * Frequency in kHz. The caller frees it; NULL when out of memory. */
char *pota_spot_json(const char *call, const char *park, uint64_t freq_hz, qso_log_mode_t mode,
                     const char *comments, const char *source);

typedef struct {
    char name[64];       /* "Pearl Lake" */
    char type[32];       /* "State Park" */
    char location[24];   /* "US-CO" */
    char grid[8];        /* "DN60ns" */
} pota_park_t;

/* api.pota.app/park/<ref> reply */
bool pota_park_parse(const char *json, pota_park_t *out);

#ifdef __cplusplus
}
#endif
