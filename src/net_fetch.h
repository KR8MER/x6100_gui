/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  HTTPS for the online features (DX cluster, space weather, call sign
 *  lookup, QRZ upload). Requests block, so they run on a worker thread:
 *  net_async() runs `work` on its own thread and then `done` on the LVGL
 *  thread, which is the only place either may touch the GUI.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char  *body;        // NUL-terminated reply, NULL on failure
    size_t len;
    long   http;        // HTTP status, 0 when no reply
    char   error[128];  // why it failed, for the operator
} net_result_t;

/* Blocking GET / form POST (worker thread only). True on HTTP 200. */
bool net_get(const char *url, net_result_t *res);
bool net_post_form(const char *url, const char *form, net_result_t *res);
void net_result_free(net_result_t *res);

/* Run work(ctx) on a new thread, then done(ctx) on the LVGL thread.
 * Call from the LVGL thread. False if the thread could not start (then
 * neither callback runs). */
bool net_async(void (*work)(void *ctx), void (*done)(void *ctx), void *ctx);

/* URL-encode text for a query string or form (caller frees) */
char *net_escape(const char *text);

/* "X6100-KR8MER/<version> (<call>)": services ask apps to identify themselves */
void net_user_agent(char *buf, size_t n);

#ifdef __cplusplus
}
#endif
