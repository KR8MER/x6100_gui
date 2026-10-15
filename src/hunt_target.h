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

/* True (and the hunt is used up) when `call` is the hunted activator. The
 * base call is compared, so W1AW/P matches a hunt for W1AW. */
bool hunt_target_take(const char *call, time_t now, char *sig, size_t sig_len, char *ref, size_t ref_len);

/* What is being hunted, for display; false when nothing is */
bool hunt_target_peek(time_t now, char *activator, size_t act_len, char *ref, size_t ref_len);

/* Base call: the longest '/'-separated part (W1AW/P -> W1AW, VE3/W1AW -> W1AW) */
void hunt_base_call(const char *call, char *out, size_t out_len);

#ifdef __cplusplus
}
#endif
