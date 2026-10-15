/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Space weather window: solar flux and K index.
 *
 *  Plain values from NOAA SWPC with the official G scale, each with the UTC
 *  time it applies to. No band-condition predictions are made from them.
 *  The last good reply is kept in memory, so reopening the window soon after
 *  shows it without asking SWPC again.
 */

#include "dialog_spacewx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "events.h"
#include "keyboard.h"
#include "msg.h"
#include "net_fetch.h"
#include "radio.h"
#include "space_weather.h"
#include "styles.h"

#define CACHE_SEC       (15 * 60)   // reopening within this shows the cached data
#define MIN_GAP_SEC     60          // Refresh no faster than this
#define FETCH_STALE_SEC 90          // give up on a reply that never came (two 20 s requests)
#define TICK_MS         (30 * 1000) // keeps the age current, refetches once the cache is old

#define PAD       16
#define COL_2_X   (DIALOG_WIDTH / 2 + PAD)

static void construct_cb(lv_obj_t *parent);
static void destruct_cb(void);
static void key_cb(lv_event_t *e);
static void refresh_cb(button_data_t *b);

static button_data_t btn_title   = {.type = BTN_TEXT, .label = "(SPACE WX)"};
static button_data_t btn_refresh = {.type = BTN_TEXT, .label = "Refresh", .press = refresh_cb};

static buttons_page_t page = {{&btn_title, &btn_refresh}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page,
    .key_cb       = key_cb,
};

dialog_t *dialog_spacewx = &dialog;

static lv_obj_t   *status;
static lv_obj_t   *flux_value;
static lv_obj_t   *flux_time;
static lv_obj_t   *kp_value;
static lv_obj_t   *kp_scale;
static lv_obj_t   *kp_time;
static lv_obj_t   *a_line;
static lv_obj_t   *max_line;
static lv_timer_t *timer;

static sw_data_t cache;
static time_t    last_good;        // last fetch that brought both feeds
static time_t    last_try;
static char      last_error[160];
static bool      fetching;
static int       generation;       // tells a reply for a closed window apart

typedef struct {
    int       generation;
    sw_data_t data;
    bool      flux_ok;
    bool      kp_ok;
    char      error[160];
} fetch_job_t;

/* ---- fetch (worker thread: no LVGL here) ---- */

static bool fetch_one(const char *url, bool (*parse)(const char *, sw_data_t *), fetch_job_t *job,
                      const char *what) {
    net_result_t res;
    bool         ok = false;
    size_t       used = strlen(job->error);
    const char  *sep  = used ? "; " : "";

    if (!net_get(url, &res)) {
        snprintf(job->error + used, sizeof(job->error) - used, "%s%s: %s", sep, what, res.error);
    } else if (!(ok = parse(res.body, &job->data))) {
        snprintf(job->error + used, sizeof(job->error) - used, "%s%s: unexpected reply", sep, what);
    }
    net_result_free(&res);
    return ok;
}

static void fetch_work(void *ctx) {
    fetch_job_t *job = ctx;

    job->flux_ok = fetch_one(SW_FLUX_URL, sw_parse_flux, job, "Solar flux");
    job->kp_ok   = fetch_one(SW_KP_URL, sw_parse_kp, job, "K index");
}

/* ---- readout (LVGL thread) ---- */

static void utc_text(time_t t, char *buf, size_t n) {
    struct tm tm;

    gmtime_r(&t, &tm);
    strftime(buf, n, "%Y-%m-%d %H:%M UTC", &tm);
}

static void age_text(time_t since, char *buf, size_t n) {
    long min = (long)(time(NULL) - since) / 60;

    if (min < 1) {
        snprintf(buf, n, "under a minute");
    } else if (min < 60) {
        snprintf(buf, n, "%li min", min);
    } else {
        snprintf(buf, n, "%li h %li min", min / 60, min % 60);
    }
}

static void show_status(void) {
    char age[32];

    if (last_good) {
        age_text(last_good, age, sizeof(age));
    }
    if (fetching) {
        lv_label_set_text(status, "Fetching from NOAA SWPC...");
    } else if (last_error[0] && last_good) {
        lv_label_set_text_fmt(status, "%s (showing data fetched %s ago)", last_error, age);
    } else if (last_error[0]) {
        lv_label_set_text(status, last_error);
    } else if (last_good) {
        lv_label_set_text_fmt(status, "Fetched %s ago", age);
    } else {
        lv_label_set_text(status, "");
    }
}

static void show_data(void) {
    char when[32];

    if (cache.has_flux) {
        lv_label_set_text_fmt(flux_value, "%i sfu", cache.flux);
        utc_text(cache.flux_time, when, sizeof(when));
        lv_label_set_text(flux_time, when);
    } else {
        lv_label_set_text(flux_value, "--");
        lv_label_set_text(flux_time, "");
    }

    if (cache.has_kp) {
        int g = sw_g_level(cache.kp);

        lv_label_set_text_fmt(kp_value, "%.2f", cache.kp);
        if (g) {
            lv_label_set_text_fmt(kp_scale, "G%i %s geomagnetic storm", g, sw_g_name(g));
        } else {
            lv_label_set_text(kp_scale, "Below storm level");
        }
        utc_text(cache.kp_time, when, sizeof(when));
        lv_label_set_text_fmt(kp_time, "3 h from %s", when);

        if (cache.a_index >= 0) {
            lv_label_set_text_fmt(a_line, "A index (running): %i, %s", cache.a_index, when);
        } else {
            lv_label_set_text(a_line, "A index (running): --");
        }

        g = sw_g_level(cache.kp_max_24h);
        utc_text(cache.kp_max_time, when, sizeof(when));
        if (g) {
            lv_label_set_text_fmt(max_line, "Highest Kp, last 24 h: %.2f (G%i %s), %s", cache.kp_max_24h, g,
                                  sw_g_name(g), when);
        } else {
            lv_label_set_text_fmt(max_line, "Highest Kp, last 24 h: %.2f, %s", cache.kp_max_24h, when);
        }
    } else {
        lv_label_set_text(kp_value, "--");
        lv_label_set_text(kp_scale, "");
        lv_label_set_text(kp_time, "");
        lv_label_set_text(a_line, "A index (running): --");
        lv_label_set_text(max_line, "Highest Kp, last 24 h: --");
    }
}

static void fetch_done(void *ctx);

static void start_fetch(void) {
    if (fetching && time(NULL) - last_try < FETCH_STALE_SEC) {
        return;
    }

    fetch_job_t *job = calloc(1, sizeof(*job));

    if (!job) {
        return;
    }
    job->generation = generation;
    fetching        = true;
    last_try        = time(NULL);
    if (!net_async(fetch_work, fetch_done, job)) {
        fetching = false;
        free(job);
        snprintf(last_error, sizeof(last_error), "Can't start the fetch");
    }
    show_status();
}

static void fetch_done(void *ctx) {
    fetch_job_t *job = ctx;

    fetching = false;

    /* The data is good whichever window asked for it */
    if (job->flux_ok) {
        cache.has_flux  = true;
        cache.flux      = job->data.flux;
        cache.flux_time = job->data.flux_time;
    }
    if (job->kp_ok) {
        cache.has_kp      = true;
        cache.kp          = job->data.kp;
        cache.kp_time     = job->data.kp_time;
        cache.a_index     = job->data.a_index;
        cache.kp_max_24h  = job->data.kp_max_24h;
        cache.kp_max_time = job->data.kp_max_time;
    }
    if (job->flux_ok && job->kp_ok) {
        last_good     = time(NULL);
        last_error[0] = 0;
    } else {
        snprintf(last_error, sizeof(last_error), "%s", job->error);
    }

    if (dialog.run && job->generation != generation && !(job->flux_ok && job->kp_ok)) {
        /* A failed reply for a closed-and-reopened window: try again for this one */
        start_fetch();
    } else if (dialog.run) {
        show_data();
        show_status();
    }
    free(job);
}

static void timer_cb(lv_timer_t *t) {
    if (!fetching && time(NULL) - last_try >= CACHE_SEC) {
        start_fetch();
    }
    show_status();
}

/* ---- buttons ---- */

static void refresh_cb(button_data_t *b) {
    if (fetching) {
        return;
    }
    if (time(NULL) - last_try < MIN_GAP_SEC) {
        msg_update_text_fmt("Space weather refreshes every %i s at most", MIN_GAP_SEC);
        return;
    }
    start_fetch();
}

/* ---- dialog ---- */

static lv_obj_t *add_label(const lv_font_t *font, bool muted, lv_coord_t x, lv_coord_t y) {
    lv_obj_t *l = lv_label_create(dialog.obj);

    lv_obj_add_style(l, muted ? &style.text_muted_color : &style.text_base_color, 0);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_pos(l, x, y);
    lv_label_set_text(l, "");
    return l;
}

static void construct_cb(lv_obj_t *parent) {
    dialog.obj = dialog_init(parent);
    generation++;

    lv_group_add_obj(keyboard_group, dialog.obj);
    lv_obj_add_event_cb(dialog.obj, key_cb, LV_EVENT_KEY, NULL);

    status = add_label(&sony_22, true, PAD, 12);
    lv_label_set_long_mode(status, LV_LABEL_LONG_DOT);
    lv_obj_set_width(status, DIALOG_WIDTH - PAD * 2);

    lv_label_set_text(add_label(&sony_22, true, PAD, 48), "Solar flux (10.7 cm)");
    flux_value = add_label(&sony_60, false, PAD, 74);
    flux_time  = add_label(&sony_22, true, PAD, 146);

    lv_label_set_text(add_label(&sony_22, true, COL_2_X, 48), "Planetary K index");
    kp_value = add_label(&sony_60, false, COL_2_X, 74);
    kp_scale = add_label(&sony_24, false, COL_2_X, 146);
    kp_time  = add_label(&sony_22, true, COL_2_X, 176);

    a_line   = add_label(&sony_24, false, PAD, 218);
    max_line = add_label(&sony_24, false, PAD, 254);

    lv_label_set_text(add_label(&sony_20, true, PAD, DIALOG_HEIGHT - 34),
                      "Data: NOAA SWPC. Not endorsed by NOAA.");

    show_data();
    if (!fetching && (!last_good || time(NULL) - last_good >= CACHE_SEC)) {
        start_fetch();
    }
    show_status();
    timer = lv_timer_create(timer_cb, TICK_MS, NULL);
}

static void destruct_cb(void) {
    if (timer) {
        lv_timer_del(timer);
        timer = NULL;
    }
    generation++;
}

static void key_cb(lv_event_t *e) {
    uint32_t key = *((uint32_t *)lv_event_get_param(e));

    switch (key) {
        case LV_KEY_ESC:
            dialog_destruct(&dialog);
            break;

        case KEY_VOL_LEFT_EDIT:
        case KEY_VOL_LEFT_SELECT:
            radio_change_vol(-1);
            break;

        case KEY_VOL_RIGHT_EDIT:
        case KEY_VOL_RIGHT_SELECT:
            radio_change_vol(1);
            break;
    }
}
