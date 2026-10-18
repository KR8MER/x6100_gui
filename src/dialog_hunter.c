/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  POTA hunter, after Nexus's: live activator spots over Wi-Fi, filtered by
 *  band and mode, with NEW on parks not yet in your log. HUNT tunes to the
 *  spot, opens the right cockpit (CW, Phone or Digital) and remembers the
 *  park, so the next QSO you log with that activator is tagged for POTA.
 *
 *  The spot list is read-only and only fetched while this window is open,
 *  at most every 90 seconds: the POTA API runs on volunteers' goodwill.
 *  (SOTA is not included: its API terms require prior approval for apps.)
 */

#include "dialog_hunter.h"

#include <curl/curl.h>

#include "net_fetch.h"
#include <ft8lib/constants.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "cell_columns.h"
#include "cockpit.h"
#include "events.h"
#include "hunt_target.h"
#include "keyboard.h"
#include "lock_manager.h"
#include "main.h"
#include "main_screen.h"
#include "msg.h"
#include "pota_spots.h"
#include "pota_watch.h"
#include "qso_log.h"
#include "radio.h"
#include "scheduler.h"
#include "styles.h"

#include "aether_radio/x6100_control/control.h"

#define SPOTS_URL     "https://api.pota.app/spot/activator"
#define MAX_SPOTS     120
#define REFRESH_MS    (90 * 1000)
#define MIN_GAP_SEC   30            // manual refresh no faster than this
#define MAX_BODY      (512 * 1024)

typedef enum { FILTER_ALL_MODES, FILTER_CW, FILTER_PHONE, FILTER_DATA, FILTER_MODES } mode_filter_t;

static void construct_cb(lv_obj_t *parent);
static void destruct_cb(void);
static void key_cb(lv_event_t *e);

static void refresh_cb(button_data_t *b);
static void band_filter_cb(button_data_t *b);
static void mode_filter_cb(button_data_t *b);
static void hunt_cb(button_data_t *b);
static void alerts_cb(button_data_t *b);
static const char *band_label(void);
static const char *mode_label(void);

static button_data_t btn_p1      = {.type = BTN_TEXT, .label = "(HUNT 1:2)", .press = button_next_page_cb};
static button_data_t btn_refresh = {.type = BTN_TEXT, .label = "Refresh", .press = refresh_cb};
static button_data_t btn_band    = {.type = BTN_TEXT_FN, .label_fn = band_label, .press = band_filter_cb};
static button_data_t btn_mode    = {.type = BTN_TEXT_FN, .label_fn = mode_label, .press = mode_filter_cb};
static button_data_t btn_hunt    = {.type = BTN_TEXT, .label = "HUNT", .press = hunt_cb};

static button_data_t btn_p2      = {.type = BTN_TEXT, .label = "(HUNT 2:2)", .press = button_next_page_cb};
static button_data_t btn_alerts  = {.type = BTN_TEXT_FN, .label_fn = pota_watch_label, .press = alerts_cb};

static buttons_page_t page_1 = {{&btn_p1, &btn_refresh, &btn_band, &btn_mode, &btn_hunt}};
static buttons_page_t page_2 = {{&btn_p2, &btn_alerts}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page_1,
    .key_cb       = NULL,
};

dialog_t *dialog_hunter = &dialog;

static lv_obj_t   *status;
static lv_obj_t   *table;
static lv_timer_t *timer;

static pota_spot_t spots[MAX_SPOTS];
static int         spots_n;
static int         shown[MAX_SPOTS];   // spots index per table row
static int         shown_n;

static bool          this_band_only = true;
static mode_filter_t mode_filter    = FILTER_ALL_MODES;

static time_t last_fetch;
static bool   fetching;
static int    generation;              // ignores a reply that lands after close

typedef struct {
    int   generation;
    char *body;      // NULL on failure
    char  error[96];
} fetch_result_t;

/* ---- fetch (worker thread: no LVGL here) ---- */

typedef struct {
    char  *buf;
    size_t len;
} body_t;

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *user) {
    body_t *b = user;
    size_t  n = size * nmemb;
    if (b->len + n > MAX_BODY) return 0;
    char *grown = realloc(b->buf, b->len + n + 1);
    if (!grown) return 0;
    b->buf = grown;
    memcpy(b->buf + b->len, ptr, n);
    b->len += n;
    b->buf[b->len] = 0;
    return n;
}

static void fetch_done(void *arg);

static void *fetch_thread(void *arg) {
    fetch_result_t *res  = arg;
    body_t          body = {0};
    CURL           *curl = curl_easy_init();

    pthread_detach(pthread_self());

    if (!curl) {
        snprintf(res->error, sizeof(res->error), "Network unavailable");
    } else {
        curl_easy_setopt(curl, CURLOPT_URL, SPOTS_URL);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "X6100-GUI-KR8MER/" VERSION);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, NET_CA_BUNDLE);   // none built in
        curl_easy_setopt(curl, CURLOPT_CAPATH, NET_CA_PATH);

        CURLcode rc   = curl_easy_perform(curl);
        long     http = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
        if (rc != CURLE_OK) {
            snprintf(res->error, sizeof(res->error), "No spots: %s (Wi-Fi on?)", curl_easy_strerror(rc));
            free(body.buf);
        } else if (http != 200) {
            snprintf(res->error, sizeof(res->error), "No spots: server said %ld", http);
            free(body.buf);
        } else {
            res->body = body.buf;
        }
        curl_easy_cleanup(curl);
    }

    scheduler_put(fetch_done, &res, sizeof(res));
    return NULL;
}

/* A fetch is given up on after this long, should its reply never arrive
 * (curl itself times out well before) */
#define FETCH_STALE_SEC 60

static void start_fetch(void) {
    if (fetching && time(NULL) - last_fetch < FETCH_STALE_SEC) return;

    fetch_result_t *res = calloc(1, sizeof(*res));
    pthread_t       thr;

    res->generation = generation;
    fetching        = true;
    last_fetch      = time(NULL);
    lv_label_set_text(status, "Fetching POTA spots...");

    if (pthread_create(&thr, NULL, fetch_thread, res) != 0) {
        fetching = false;
        free(res);
        lv_label_set_text(status, "Can't start the spot fetch");
    }
}

/* ---- list (LVGL thread) ---- */

static bool mode_matches(const char *mode) {
    switch (mode_filter) {
        case FILTER_CW:
            return strcmp(mode, "CW") == 0;
        case FILTER_PHONE:
            return !strcmp(mode, "SSB") || !strcmp(mode, "USB") || !strcmp(mode, "LSB") || !strcmp(mode, "AM") ||
                   !strcmp(mode, "FM");
        case FILTER_DATA:
            return !strcmp(mode, "FT8") || !strcmp(mode, "FT4") || !strcmp(mode, "RTTY") || !strcmp(mode, "DATA") ||
                   !strcmp(mode, "PSK31") || !strcmp(mode, "JS8");
        default:
            return true;
    }
}

/* One string per column for each shown spot */
enum { COL_FREQ, COL_MODE, COL_CALL, COL_REF, COL_NEW, COL_AGE, COL_PARK, COLS };

static char        cells[MAX_SPOTS][COLS][40];
static const char *empty_text = "";

static const char *cell_get(uint16_t row, uint8_t col) {
    if (!shown_n) return col == 0 ? empty_text : NULL;
    return row < shown_n ? cells[row][col] : NULL;
}

static const lv_coord_t     col_x[COLS] = {0, 112, 180, 310, 425, 490, 545};
static const cell_columns_t columns     = {.x = col_x, .count = COLS, .get = cell_get};

static void rebuild_table(void) {
    qso_log_band_t band = qso_log_freq_to_band(cparam_i_get(cfg.cur.fg_freq()));
    time_t         now  = time(NULL);

    shown_n = 0;
    for (int i = 0; i < spots_n; i++) {
        if (this_band_only && qso_log_freq_to_band(spots[i].freq_hz) != band) continue;
        if (!mode_matches(spots[i].mode)) continue;
        shown[shown_n++] = i;
    }

    empty_text = spots_n ? "No spots match the filters" : "";
    for (int r = 0; r < shown_n; r++) {
        pota_spot_t *s   = &spots[shown[r]];
        int          age = s->spot_time ? (int)((now - s->spot_time) / 60) : 0;
        char (*c)[40]    = cells[r];

        snprintf(c[COL_FREQ], 40, "%i.%03i.%01i", s->freq_hz / 1000000, (s->freq_hz / 1000) % 1000,
                 (s->freq_hz / 100) % 10);
        snprintf(c[COL_MODE], 40, "%s", s->mode);
        snprintf(c[COL_CALL], 40, "%s", s->activator);
        snprintf(c[COL_REF], 40, "%s", s->ref);
        /* WKD: this activator is already in the log at this park today
         * (POTA counts one per day); NEW: a park not in the log at all */
        snprintf(c[COL_NEW], 40, "%s", qso_log_worked_at_ref_today(s->activator, s->ref, now) ? "WKD"
                                       : qso_log_has_ref(s->ref)                                ? ""
                                                                                                : "NEW");
        snprintf(c[COL_AGE], 40, "%im", age < 0 ? 0 : age);
        snprintf(c[COL_PARK], 40, "%s", s->park);
    }

    /* the cells hold placeholders: the columns are drawn by cell_columns */
    lv_table_set_row_cnt(table, shown_n ? shown_n : 1);
    for (int r = 0; r < (shown_n ? shown_n : 1); r++) {
        lv_table_set_cell_value(table, r, 0, " ");
    }
    lv_obj_invalidate(table);
}

static void fetch_done(void *arg) {
    fetch_result_t *res = *(fetch_result_t **)arg;

    fetching = false;
    if (dialog.run && res->generation != generation) {
        /* Reply for a closed-and-reopened Hunter: fetch for this one now */
        start_fetch();
    } else if (dialog.run) {
        if (!res->body) {
            lv_label_set_text(status, res->error);
        } else {
            int n = pota_spots_parse(res->body, spots, MAX_SPOTS);
            if (n < 0) {
                lv_label_set_text(status, "No spots: unexpected reply");
            } else {
                spots_n = n;
                rebuild_table();
                lv_label_set_text_fmt(status, "%i POTA spots, %i shown   |   %s, %s", spots_n, shown_n,
                                      this_band_only ? "this band" : "all bands", mode_label());
            }
        }
    }
    free(res->body);
    free(res);
}

static void timer_cb(lv_timer_t *t) {
    start_fetch();
}

/* ---- buttons ---- */

static const char *band_label(void) {
    return this_band_only ? "Band:\nThis" : "Band:\nAll";
}

static const char *mode_label(void) {
    static const char *names[] = {"All modes", "CW", "Phone", "Data"};
    return names[mode_filter];
}

static void refresh_cb(button_data_t *b) {
    if (time(NULL) - last_fetch < MIN_GAP_SEC) {
        msg_update_text_fmt("Spots refresh every %i s at most", MIN_GAP_SEC);
        return;
    }
    start_fetch();
}

static void band_filter_cb(button_data_t *b) {
    this_band_only = !this_band_only;
    buttons_refresh(&btn_band);
    rebuild_table();
}

static void mode_filter_cb(button_data_t *b) {
    mode_filter = (mode_filter + 1) % FILTER_MODES;
    buttons_refresh(&btn_mode);
    rebuild_table();
}

static void hunt_cb(button_data_t *b) {
    uint16_t row = 0, col = 0;

    lv_table_get_selected_cell(table, &row, &col);
    if (!shown_n || row == LV_TABLE_CELL_NONE || row >= shown_n) {
        msg_update_text_fmt("Pick a spot first");
        return;
    }
    pota_spot_t s = spots[shown[row]];

    if (!radio_check_freq(s.freq_hz)) {
        msg_update_text_fmt("%i kHz is outside the radio's range", s.freq_hz / 1000);
        return;
    }

    hunt_target_set(s.activator, "POTA", s.ref, time(NULL));

    /* Tune first, then let the cockpit set the mode for that frequency */
    dialog_destruct(&dialog);
    main_screen_set_freq(s.freq_hz);

    if (!strcmp(s.mode, "CW")) {
        cockpit_enter(COCKPIT_CW);
    } else if (!strcmp(s.mode, "FT8") || !strcmp(s.mode, "FT4")) {
        /* The FT8 app tunes to its own protocol's nearest calling frequency */
        param_i_set(cfg.ft8.protocol(), strcmp(s.mode, "FT4") ? FTX_PROTOCOL_FT8 : FTX_PROTOCOL_FT4);
        cockpit_enter(COCKPIT_DIGITAL);
    } else if (!strcmp(s.mode, "FM")) {
        cparam_i_set(cfg.cur.mode(), x6100_mode_nfm);
    } else if (!strcmp(s.mode, "AM")) {
        cparam_i_set(cfg.cur.mode(), x6100_mode_am);
    } else {
        cockpit_enter(COCKPIT_PHONE);
    }
    msg_update_text_fmt("Hunting %s at %s", s.activator, s.ref);
}

static void alerts_cb(button_data_t *b) {
    pota_watch_cycle();
    buttons_refresh(&btn_alerts);
}

/* ---- dialog ---- */

static void construct_cb(lv_obj_t *parent) {
    static bool curl_ready;
    if (!curl_ready) {
        curl_global_init(CURL_GLOBAL_DEFAULT);   // once, before any worker thread
        curl_ready = true;
    }

    dialog.obj = dialog_init(parent);
    generation++;

    page_1.items[0]->next = &page_2;
    page_1.items[0]->prev = &page_2;
    page_2.items[0]->next = &page_1;
    page_2.items[0]->prev = &page_1;

    status = lv_label_create(dialog.obj);
    lv_obj_add_style(status, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(status, &sony_22, 0);
    lv_obj_set_pos(status, 16, 12);

    table = lv_table_create(dialog.obj);
    lv_obj_remove_style(table, NULL, LV_STATE_ANY | LV_PART_MAIN);
    lv_obj_set_size(table, DIALOG_WIDTH - 24, DIALOG_HEIGHT - 52);
    lv_obj_set_pos(table, 12, 44);
    lv_table_set_col_cnt(table, 1);
    lv_table_set_col_width(table, 0, DIALOG_WIDTH - 40);
    lv_obj_set_style_text_font(table, &mono_22, LV_PART_ITEMS);
    lv_obj_set_style_border_width(table, 0, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(table, LV_OPA_TRANSP, LV_PART_ITEMS);
    lv_obj_add_style(table, &style.text_base_color, LV_PART_ITEMS);
    lv_obj_set_style_pad_ver(table, 3, LV_PART_ITEMS);
    lv_obj_set_style_pad_hor(table, 4, LV_PART_ITEMS);
    /* the selected spot */
    lv_obj_add_style(table, &style.dialog.item_focus, LV_PART_ITEMS | LV_STATE_EDITED);
    cell_columns_attach(table, &columns);
    lv_obj_add_event_cb(table, key_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(keyboard_group, table);
    lv_group_set_editing(keyboard_group, true);

    rebuild_table();   // the last list (if any) while the new one loads

    start_fetch();
    timer = lv_timer_create(timer_cb, REFRESH_MS, NULL);
    lm_set_mode(true);
}

static void destruct_cb(void) {
    if (timer) {
        lv_timer_del(timer);
        timer = NULL;
    }
    generation++;
    lm_set_mode(false);
}

static void key_cb(lv_event_t *e) {
    uint32_t key = *((uint32_t *)lv_event_get_param(e));

    switch (key) {
        case LV_KEY_ESC:
            dialog_destruct(&dialog);
            break;

        case LV_KEY_ENTER:
            hunt_cb(NULL);
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
