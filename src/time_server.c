/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "time_server.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>

#include "msg.h"

/* What the Wi-Fi connect script uses when nothing is chosen */
#define DEFAULT_SYNC_HOST "time.nist.gov"

/* Regenerates /etc/ntp.conf from the choice and restarts ntpd (Buildroot) */
#define NTPCONF_SCRIPT    "/etc/init.d/S48ntpconf"

/* clock.c announces a sync when this file's time changes */
#define SYNC_MARK         "/var/run/time-sync"

static const char *hosts[TIME_SERVER_COUNT] = {
    NULL,
    "time.nist.gov",
    "time.google.com",
    "time.cloudflare.com",
    "north-america.pool.ntp.org",
    "europe.pool.ntp.org",
    "asia.pool.ntp.org",
    "oceania.pool.ntp.org",
};

static atomic_bool syncing;

const char *time_server_dropdown_options(void) {
    return " Default (pool.ntp.org, NIST) \n"
           " time.nist.gov \n"
           " time.google.com \n"
           " time.cloudflare.com \n"
           " North America pool \n"
           " Europe pool \n"
           " Asia pool \n"
           " Oceania pool ";
}

const char *time_server_host(int index) {
    if (index < 0 || index >= TIME_SERVER_COUNT) {
        return NULL;
    }
    return hosts[index];
}

static bool write_choice(int index) {
    const char *host = time_server_host(index);

    if (!host) {
        return unlink(TIME_SERVER_FILE) == 0 || access(TIME_SERVER_FILE, F_OK) != 0;
    }

    FILE *f = fopen(TIME_SERVER_FILE, "w");
    if (!f) {
        return false;
    }
    bool ok = fprintf(f, "%s\n", host) > 0;
    return fclose(f) == 0 && ok;
}

static void *sync_thread(void *arg) {
    const char *host = arg ? (const char *)arg : DEFAULT_SYNC_HOST;
    char        cmd[128];

    /* Hosts come from the fixed list above, never from typed text */
    snprintf(cmd, sizeof(cmd), "ntpdate -u %s >/dev/null 2>&1", host);
    if (system(cmd) == 0) {
        system("hwclock -w -f /dev/rtc1 >/dev/null 2>&1");
        FILE *f = fopen(SYNC_MARK, "a");
        if (f) {
            fclose(f);
            utime(SYNC_MARK, NULL);
        }
        msg_schedule_text_fmt("Clock set from %s", host);
    } else {
        msg_schedule_text_fmt("Can't reach %s (Wi-Fi on?)", host);
    }

    /* ntpd keeps the clock right from the new choice from now on */
    if (access(NTPCONF_SCRIPT, X_OK) == 0) {
        system(NTPCONF_SCRIPT " restart >/dev/null 2>&1");
    }

    atomic_store(&syncing, false);
    return NULL;
}

void time_server_apply(int index) {
    if (!write_choice(index)) {
        msg_schedule_text_fmt("Can't save the time server");
        return;
    }
    if (atomic_exchange(&syncing, true)) {
        return;   /* a sync is already running; ntpd picks the choice up */
    }

    const char *host = time_server_host(index);
    pthread_t   thr;

    msg_schedule_text_fmt("Setting the clock from %s", host ? host : DEFAULT_SYNC_HOST);
    if (pthread_create(&thr, NULL, sync_thread, (void *)host) != 0) {
        atomic_store(&syncing, false);
        return;
    }
    pthread_detach(thr);
}

void time_server_init(int index) {
    write_choice(index);
}
