/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  The pending hunt, after Nexus's POTA/SOTA hunter: pressing HUNT on a spot
 *  remembers the activator and the park or summit. The next QSO logged with
 *  that activator, in any mode, is tagged with the reference. A hunt tags one
 *  QSO only and expires after four hours, so a stale reference can never land
 *  on an unrelated contact later.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HUNT_TTL_SEC (4 * 60 * 60)

/* sig: "POTA" or "SOTA"; ref: e.g. "US-1234" or "W8O/NE-001" */
void hunt_target_set(const char *activator, const char *sig, const char *ref, time_t now);
void hunt_target_clear(void);

/* True when a QSO with `call` at `qso_time` belongs to the hunt: the base
 * call matches (W1AW/P matches a hunt for W1AW) and the QSO was made after
 * the hunt started and before it expired, so an old QSO imported from ADIF
 * never takes the reference. Does not use the hunt up. */
bool hunt_target_match(const char *call, time_t qso_time, char *sig, size_t sig_len, char *ref, size_t ref_len);

/* Use the hunt up, once its QSO is actually in the log */
void hunt_target_done(const char *call);

/* What is being hunted, for display; false when nothing is */
bool hunt_target_peek(time_t now, char *activator, size_t act_len, char *ref, size_t ref_len);

/* Base call: the longest '/'-separated part (W1AW/P -> W1AW, VE3/W1AW -> W1AW) */
void hunt_base_call(const char *call, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
