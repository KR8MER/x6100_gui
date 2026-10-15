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

#ifdef __cplusplus
extern "C" {
#endif

/* Indices are stored in settings, so only append to the list */
#define TIME_SERVER_COUNT 8

#ifndef TIME_SERVER_FILE
#define TIME_SERVER_FILE  "/mnt/ntp_server"
#endif

/* "\n"-separated server names for an LVGL dropdown; 0 is the default */
const char *time_server_dropdown_options(void);

/* Host name for a setting index; NULL for the default */
const char *time_server_host(int index);

/* Save the choice for the boot scripts, then set the clock from it now */
void time_server_apply(int index);

/* At start-up: make the file match the setting, without syncing */
void time_server_init(int index);

#ifdef __cplusplus
}
#endif
