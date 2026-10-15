/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "log_rules.h"

#include <ctype.h>
#include <string.h>

#include "cockpit.h"

/* x6100_mode_t values (aether_radio/x6100_control/low/control.h) */
enum { M_LSB, M_LSB_DIG, M_USB, M_USB_DIG, M_CW, M_CWR, M_AM, M_NFM };

qso_log_mode_t log_mode_for(int x6100_mode, int cockpit) {
    switch (x6100_mode) {
        case M_LSB:
        case M_USB:
            return MODE_SSB;
        case M_CW:
        case M_CWR:
            return MODE_CW;
        case M_AM:
            return MODE_AM;
        case M_NFM:
            return MODE_FM;
        case M_LSB_DIG:
        case M_USB_DIG:
            return cockpit == COCKPIT_RTTY ? MODE_RTTY : MODE_OTHER;
        default:
            return MODE_OTHER;
    }
}

int log_default_rst(qso_log_mode_t mode) {
    switch (mode) {
        case MODE_CW:
        case MODE_RTTY:
            return 599;
        default:
            return 59;
    }
}

/* PREFIX-NNNN: 1-4 letters/digits, a dash, 4 or 5 digits */
static bool is_park(const char *ref) {
    const char *dash = strchr(ref, '-');
    if (!dash || dash == ref || dash - ref > 4 || strchr(ref, '/')) {
        return false;
    }
    for (const char *p = ref; p < dash; p++) {
        if (!isalnum((unsigned char)*p)) return false;
    }
    size_t digits = strlen(dash + 1);
    if (digits < 4 || digits > 5) {
        return false;
    }
    for (const char *p = dash + 1; *p; p++) {
        if (!isdigit((unsigned char)*p)) return false;
    }
    return true;
}

/* ASSOC/RR-NNN: association with an optional digit, region, number */
static bool is_summit(const char *ref) {
    const char *slash = strchr(ref, '/');
    if (!slash || slash == ref) {
        return false;
    }
    const char *dash = strchr(slash, '-');
    if (!dash || dash - slash < 2) {
        return false;
    }
    size_t digits = strlen(dash + 1);
    if (digits != 3) {
        return false;
    }
    for (const char *p = dash + 1; *p; p++) {
        if (!isdigit((unsigned char)*p)) return false;
    }
    return true;
}

const char *log_sig_for_ref(const char *ref) {
    if (!ref || !ref[0]) {
        return "";
    }
    if (is_park(ref)) {
        return "POTA";
    }
    if (is_summit(ref)) {
        return "SOTA";
    }
    return "";
}

const char *log_mode_name(qso_log_mode_t mode) {
    switch (mode) {
        case MODE_SSB:  return "SSB";
        case MODE_AM:   return "AM";
        case MODE_FM:   return "FM";
        case MODE_CW:   return "CW";
        case MODE_FT8:  return "FT8";
        case MODE_FT4:  return "FT4";
        case MODE_RTTY: return "RTTY";
        case MODE_JS8:  return "JS8";
        default:        return "DATA";
    }
}
