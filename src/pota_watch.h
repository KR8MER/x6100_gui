/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  POTA alerts: when turned on (Hunt POTA, page 2), the activator spots are
 *  read every few minutes in the background and a message names any park
 *  that is not in the log yet, on this band or on any band. Each activation
 *  is announced once. Nothing is fetched while the hunter window is open
 *  (it fetches for itself) or while alerts are off.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POTA_ALERTS_OFF,
    POTA_ALERTS_THIS_BAND,
    POTA_ALERTS_ALL_BANDS,
    POTA_ALERTS_COUNT
} pota_alerts_t;

/* Start the watch (LVGL thread, once, after the log is open) */
void pota_watch_init(void);

/* "Alerts:\nOff" and so on, for a button */
const char *pota_watch_label(void);

/* Step to the next setting and say what it does */
void pota_watch_cycle(void);

#ifdef __cplusplus
}
#endif
