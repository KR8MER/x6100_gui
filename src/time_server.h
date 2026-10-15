/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  The internet time server. The choice is written to the DATA partition,
 *  where the boot scripts read it: ntpd uses it in place of pool.ntp.org, and
 *  the sync on Wi-Fi connect uses it in place of time.nist.gov. A USB GPS,
 *  when one is plugged in, keeps feeding ntpd whatever server is chosen.
 */

#pragma once

#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Indices are stored in settings, so only append to the list */
#define TIME_SERVER_COUNT    9
#define TIME_SERVER_CUSTOM   8      // a host typed by the operator
#define TIME_SERVER_HOST_MAX 64

#ifndef TIME_SERVER_FILE
#define TIME_SERVER_FILE  "/mnt/ntp_server"
#endif

/* Characters a host name or IPv4 address may contain */
#define TIME_SERVER_HOST_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-"

/* A host name or IPv4 address, nothing else: it ends up on a command line.
 * Starts and ends with a letter or digit, no empty labels. */
static inline bool time_server_valid_host(const char *host) {
    size_t n = host ? strlen(host) : 0;

    if (n == 0 || n >= TIME_SERVER_HOST_MAX) {
        return false;
    }
    for (size_t i = 0; i < n; i++) {
        char c     = host[i];
        bool alnum = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');

        if (!alnum && c != '.' && c != '-') {
            return false;
        }
        if ((i == 0 || i == n - 1) && !alnum) {
            return false;
        }
        if (c == '.' && host[i + 1] == '.') {
            return false;
        }
    }
    return true;
}

/* "\n"-separated server names for an LVGL dropdown; 0 is the default */
const char *time_server_dropdown_options(void);

/* Host name for a setting index; NULL for the default (and for Custom
 * before a valid host is set) */
const char *time_server_host(int index);

/* The host for TIME_SERVER_CUSTOM; ignored unless valid */
void time_server_set_custom(const char *host);

/* Save the choice for the boot scripts, then set the clock from it now */
void time_server_apply(int index);

/* At start-up: make the file match the setting, without syncing */
void time_server_init(int index, const char *custom_host);

#ifdef __cplusplus
}
#endif
