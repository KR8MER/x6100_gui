/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Pure rules behind the log strip (unit tested).
 */

#pragma once

#include "qso_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Log mode for a radio mode (x6100_mode_t) in a cockpit (cockpit_t) */
qso_log_mode_t log_mode_for(int x6100_mode, int cockpit);

/* 59 for phone, 599 for CW and RTTY */
int log_default_rst(qso_log_mode_t mode);

/* "POTA" for a park reference (US-1234, K-1234, VE-0012), "SOTA" for a
 * summit (W8O/NE-001), "" when it is neither */
const char *log_sig_for_ref(const char *ref);

const char *log_mode_name(qso_log_mode_t mode);

#ifdef __cplusplus
}
#endif
