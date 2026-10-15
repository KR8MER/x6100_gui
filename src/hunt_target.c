/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "hunt_target.h"

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

static struct {
    bool   active;
    char   call[32];    // base call
    char   shown[32];   // as spotted
    char   sig[8];
    char   ref[24];
    time_t set_at;
} hunt;

void hunt_base_call(const char *call, char *out, size_t out_len) {
    const char *best = call, *p = call;
    size_t      best_len = 0;

    while (*p) {
        const char *end = strchr(p, '/');
        size_t      len = end ? (size_t)(end - p) : strlen(p);
        if (len > best_len) {
            best     = p;
            best_len = len;
        }
        if (!end) break;
        p = end + 1;
    }
    if (best_len >= out_len) best_len = out_len - 1;
    for (size_t i = 0; i < best_len; i++) {
        out[i] = (char)toupper((unsigned char)best[i]);
    }
    out[best_len] = 0;
}

void hunt_target_set(const char *activator, const char *sig, const char *ref, time_t now) {
    pthread_mutex_lock(&mux);
    hunt.active = true;
    hunt_base_call(activator, hunt.call, sizeof(hunt.call));
    snprintf(hunt.shown, sizeof(hunt.shown), "%s", activator);
    snprintf(hunt.sig, sizeof(hunt.sig), "%s", sig);
    snprintf(hunt.ref, sizeof(hunt.ref), "%s", ref);
    hunt.set_at = now;
    pthread_mutex_unlock(&mux);
}

void hunt_target_clear(void) {
    pthread_mutex_lock(&mux);
    hunt.active = false;
    pthread_mutex_unlock(&mux);
}

static bool live(time_t now) {
    if (hunt.active && now - hunt.set_at > HUNT_TTL_SEC) {
        hunt.active = false;
    }
    return hunt.active;
}

/* A QSO logged a little before HUNT was pressed still counts */
#define HUNT_SLACK_SEC 60

bool hunt_target_match(const char *call, time_t qso_time, char *sig, size_t sig_len, char *ref, size_t ref_len) {
    char base[32];
    bool hit = false;

    hunt_base_call(call, base, sizeof(base));
    pthread_mutex_lock(&mux);
    if (hunt.active && qso_time >= hunt.set_at - HUNT_SLACK_SEC && qso_time - hunt.set_at <= HUNT_TTL_SEC &&
        strcmp(base, hunt.call) == 0) {
        snprintf(sig, sig_len, "%s", hunt.sig);
        snprintf(ref, ref_len, "%s", hunt.ref);
        hit = true;
    }
    pthread_mutex_unlock(&mux);
    return hit;
}

void hunt_target_done(const char *call) {
    char base[32];

    hunt_base_call(call, base, sizeof(base));
    pthread_mutex_lock(&mux);
    if (strcmp(base, hunt.call) == 0) {
        hunt.active = false;
    }
    pthread_mutex_unlock(&mux);
}

bool hunt_target_peek(time_t now, char *activator, size_t act_len, char *ref, size_t ref_len) {
    bool on;

    pthread_mutex_lock(&mux);
    on = live(now);
    if (on) {
        snprintf(activator, act_len, "%s", hunt.shown);
        snprintf(ref, ref_len, "%s", hunt.ref);
    }
    pthread_mutex_unlock(&mux);
    return on;
}
