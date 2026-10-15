/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "atu_log.h"

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "msg.h"
#include "scheduler.h"

#define BITS_LEN 36   // 32 bits, 3 spaces, NUL

void atu_log_format_bits(uint32_t network, char *out, size_t n) {
    char   bits[BITS_LEN];
    size_t k = 0;

    for (int i = 31; i >= 0; i--) {
        bits[k++] = (network >> i) & 1 ? '1' : '0';
        if (i % 8 == 0 && i) {
            bits[k++] = ' ';
        }
    }
    bits[k] = 0;
    snprintf(out, n, "%s", bits);
}

/* LVGL thread: settings and messages are safe here */
static void tuned_cb(void *arg) {
    uint32_t network = *(uint32_t *)arg;
    int32_t  freq    = cparam_i_get(cfg.cur.fg_freq());
    int32_t  ant     = param_i_get(cfg.ant_id());
    char     bits[BITS_LEN];
    char     when[24];
    time_t   now = time(NULL);
    struct tm tm;
    struct stat st;

    atu_log_format_bits(network, bits, sizeof(bits));
    gmtime_r(&now, &tm);
    strftime(when, sizeof(when), "%Y-%m-%dT%H:%M:%SZ", &tm);

    bool  fresh = stat(ATU_LOG_PATH, &st) != 0;
    FILE *f     = fopen(ATU_LOG_PATH, "a");
    if (f) {
        if (fresh) {
            fprintf(f, "utc,antenna,freq_hz,network_hex,network_bits\n");
        }
        fprintf(f, "%s,%d,%d,0x%08X,%s\n", when, (int)ant, (int)freq, (unsigned)network, bits);
        fclose(f);
    }
    msg_schedule_text_fmt("Tuned: network 0x%08X%s", (unsigned)network, f ? "" : " (log not written)");
}

void atu_log_tuned(uint32_t network) {
    scheduler_put(tuned_cb, &network, sizeof(network));
}
