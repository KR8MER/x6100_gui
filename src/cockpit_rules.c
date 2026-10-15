/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Pure rules behind the cockpits, kept apart so they can be unit tested.
 */

#include "cockpit.h"

/* x6100_mode_t values (aether_radio/x6100_control/low/control.h) */
#define MODE_LSB 0
#define MODE_USB 2

int cockpit_phone_sideband(int32_t freq_hz) {
    /* 60 m channels are USB by rule, though they sit below 10 MHz */
    if (freq_hz >= 5250000 && freq_hz <= 5450000) {
        return MODE_USB;
    }
    return freq_hz < 10000000 ? MODE_LSB : MODE_USB;
}

const char *cockpit_name(cockpit_t c) {
    switch (c) {
        case COCKPIT_PHONE:   return "Phone";
        case COCKPIT_CW:      return "CW";
        case COCKPIT_DIGITAL: return "Digital";
        case COCKPIT_RTTY:    return "RTTY";
        default:              return "None";
    }
}
