/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Operating cockpits, after Nexus (KD9TAW): pick how you are operating and
 *  the radio sets itself up for it: mode, sideband by band, and the controls
 *  on the buttons.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stored in settings: append only */
typedef enum {
    COCKPIT_NONE = 0,
    COCKPIT_PHONE,
    COCKPIT_CW,
    COCKPIT_DIGITAL,
    COCKPIT_RTTY,
} cockpit_t;

void        cockpit_init(void);
void        cockpit_enter(cockpit_t c);
cockpit_t   cockpit_current(void);
const char *cockpit_name(cockpit_t c);

/* Phone sideband for a frequency: LSB below 10 MHz, USB above, and USB on
 * 60 m where it is required. Returns an x6100_mode_t. */
int         cockpit_phone_sideband(int32_t freq_hz);

#ifdef __cplusplus
}
#endif
