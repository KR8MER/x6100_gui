/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  DX cluster spots window: the latest cluster spots from HamQTH over Wi-Fi,
 *  filtered by band and mode on the server, with NEW on calls not yet in
 *  your log. TUNE goes to the spot and opens the cockpit for its mode.
 *
 *  The list is read-only and only fetched while this window is open, every
 *  30 seconds and never faster than every 15: HamQTH's terms for the API.
 */

#include "dialog_cluster.h"

#include <ft8lib/constants.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "cell_columns.h"
#include "cockpit.h"
#include "dx_spots.h"
#include "events.h"
#include "keyboard.h"
#include "lock_manager.h"
#include "main_screen.h"
#include "msg.h"
#include "net_fetch.h"
#include "qso_log.h"
#include "radio.h"
#include "styles.h"

#define MAX_SPOTS       100     // asked of the server, and kept
#define REFRESH_MS      (30 * 1000)
#define MIN_GAP_SEC     15      // HamQTH: not more often than this
#define FETCH_STALE_SEC 60      // give up on a reply that never came
#define URL_LEN         192
#define BAND_LEN        8
#define CELL_LEN        40
#define ATTRIBUTION     "Data provided by HamQTH.com"

typedef enum { FILTER_ALL_MODES, FILTER_CW, FILTER_PHONE, FILTER_DATA, FILTER_MODES } mode_filter_t;

static void construct_cb(lv_obj_t *parent);
static void destruct_cb(void);
static void key_cb(lv_event_t *e);

static void refresh_cb(button_data_t *b);
static void band_filter_cb(button_data_t *b);
static void mode_filter_cb(button_data_t *b);
static void tune_cb(button_data_t *b);
static const char *band_label(void);
static const char *mode_label(void);

static button_data_t btn_title   = {.type = BTN_TEXT, .label = "(CLUSTER)"};
static button_data_t btn_refresh = {.type = BTN_TEXT, .label = "Refresh", .press = refresh_cb};
static button_data_t btn_band    = {.type = BTN_TEXT_FN, .label_fn = band_label, .press = band_filter_cb};
static button_data_t btn_mode    = {.type = BTN_TEXT_FN, .label_fn = mode_label, .press = mode_filter_cb};
static button_data_t btn_tune    = {.type = BTN_TEXT, .label = "TUNE", .press = tune_cb};

static buttons_page_t page = {{&btn_title, &btn_refresh, &btn_band, &btn_mode, &btn_tune}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page,
    .key_cb       = NULL,
};

dialog_t *dialog_cluster = &dialog;

static lv_obj_t   *status;
static lv_obj_t   *table;
static lv_timer_t *timer;

static dx_spot_t spots[MAX_SPOTS];
static int       spots_n;
static int       shown[MAX_SPOTS];   // spots index per table row
static int       shown_n;

static bool          this_band_only = true;
static mode_filter_t mode_filter    = FILTER_ALL_MODES;

static time_t   last_fetch;
static bool     fetching;
static unsigned fetch_serial;        // tells a given-up fetch's late reply apart
static int      generation;          // ignores a reply that lands after close

typedef struct {
    int           generation;
    unsigned      serial;
    char          url[URL_LEN];
    char          band[BAND_LEN];    // the filters asked for, "" for all bands
    mode_filter_t mode;
    net_result_t  res;
} fetch_t;

/* The server's names for the mode filters (NULL: all) */
static const char *mode_param(mode_filter_t f) {
    switch (f) {
        case FILTER_CW:
            return "CW";
        case FILTER_PHONE:
            return "SSB";
        case FILTER_DATA:
            return "DIGI";
        default:
            return NULL;
    }
}

/* The band filter in force: "" for all bands, or when the radio is not on
 * an amateur band */
static void wanted_band(char *out, size_t n) {
    out[0] = 0;
    if (this_band_only) {
        dx_band_name(cparam_i_get(cfg.cur.fg_freq()), out, n);
    }
}

static void show_status(void) {
    char band[BAND_LEN];

    wanted_band(band, sizeof(band));
    lv_label_set_text_fmt(status, "%i of %i spots   |   %s, %s   |   " ATTRIBUTION, shown_n, spots_n,
                          band[0] ? band : "all bands", mode_label());
}

/* ---- fetch ---- */

static void fetch_work(void *arg) {   // worker thread: no LVGL, no settings
    fetch_t *f = arg;
    net_get(f->url, &f->res);
}

static void fetch_done(void *arg);

static void request_fetch(void) {
    time_t now   = time(NULL);
    time_t since = now - last_fetch;

    if (since < 0) {
        since = MIN_GAP_SEC;   // the clock was set back
    }
    if (fetching && since < FETCH_STALE_SEC) return;
    if (since < MIN_GAP_SEC) {
        /* too soon for the server: the timer fetches when the gap is up */
        lv_timer_set_period(timer, (MIN_GAP_SEC - since) * 1000);
        lv_timer_reset(timer);
        return;
    }

    fetch_t *f = calloc(1, sizeof(*f));
    if (!f) {
        lv_label_set_text(status, "Can't start the spot fetch");
        return;
    }
    f->generation = generation;
    f->serial     = ++fetch_serial;
    f->mode       = mode_filter;
    wanted_band(f->band, sizeof(f->band));
    dx_spots_url(f->url, sizeof(f->url), f->band, mode_param(f->mode), MAX_SPOTS);

    fetching   = true;
    last_fetch = now;
    lv_label_set_text(status, "Fetching DX spots from HamQTH.com...");

    if (!net_async(fetch_work, fetch_done, f)) {
        fetching = false;
        free(f);
        lv_label_set_text(status, "Can't start the spot fetch");
    }
}

/* ---- list ---- */

static bool spot_matches(const dx_spot_t *s, const char *band) {
    const char *mode = mode_param(mode_filter);

    if (band[0] && strcmp(s->band, band) != 0) return false;
    return !mode || !strcmp(s->mode, mode);
}

static qso_log_mode_t log_mode(const dx_spot_t *s) {
    switch (dx_spot_tune(s)) {
        case DX_TUNE_CW:
            return MODE_CW;
        case DX_TUNE_PHONE:
            return MODE_SSB;
        case DX_TUNE_FT8:
            return MODE_FT8;
        case DX_TUNE_FT4:
            return MODE_FT4;
        default:
            return MODE_OTHER;
    }
}

/* NEW: never worked; SLOT: worked, but not on this band and mode */
static const char *worked_mark(const dx_spot_t *s) {
    switch (qso_log_search_worked(s->call, log_mode(s), qso_log_freq_to_band(s->freq_hz))) {
        case SEARCH_WORKED_NO:
            return "NEW";
        case SEARCH_WORKED_YES:
            return "SLOT";
        default:
            return "";
    }
}

static void format_freq(char *buf, size_t n, int32_t freq_hz) {
    snprintf(buf, n, "%i.%03i.%01i", freq_hz / 1000000, (freq_hz / 1000) % 1000, (freq_hz / 100) % 10);
}

/* One string per column for each shown spot */
enum { COL_FREQ, COL_MODE, COL_CALL, COL_COUNTRY, COL_WORKED, COL_AGE, COL_COMMENT, COLS };

static char        cells[MAX_SPOTS][COLS][CELL_LEN];
static const char *empty_text = "";

static const char *cell_get(uint16_t row, uint8_t col) {
    if (!shown_n) return col == 0 ? empty_text : NULL;
    return row < shown_n ? cells[row][col] : NULL;
}

static const lv_coord_t     col_x[COLS] = {0, 112, 180, 310, 445, 505, 560};
static const cell_columns_t columns     = {.x = col_x, .count = COLS, .get = cell_get};

static void rebuild_table(void) {
    char   band[BAND_LEN];
    time_t now = time(NULL);

    wanted_band(band, sizeof(band));
    shown_n = 0;
    for (int i = 0; i < spots_n; i++) {
        if (spot_matches(&spots[i], band)) {
            shown[shown_n++] = i;
        }
    }

    empty_text = spots_n ? "No spots match the filters" : "";
    for (int r = 0; r < shown_n; r++) {
        dx_spot_t *s   = &spots[shown[r]];
        int        age = s->time ? (int)((now - s->time) / 60) : 0;
        char (*c)[CELL_LEN] = cells[r];

        format_freq(c[COL_FREQ], CELL_LEN, s->freq_hz);
        snprintf(c[COL_MODE], CELL_LEN, "%s", s->mode);
        snprintf(c[COL_CALL], CELL_LEN, "%s", s->call);
        snprintf(c[COL_COUNTRY], CELL_LEN, "%s", s->country);
        snprintf(c[COL_WORKED], CELL_LEN, "%s", worked_mark(s));
        snprintf(c[COL_AGE], CELL_LEN, "%im", age < 0 ? 0 : age);
        snprintf(c[COL_COMMENT], CELL_LEN, "%.*s", CELL_LEN - 1, s->comment);
    }

    /* the cells hold placeholders: the columns are drawn by cell_columns */
    lv_table_set_row_cnt(table, shown_n ? shown_n : 1);
    for (int r = 0; r < (shown_n ? shown_n : 1); r++) {
        lv_table_set_cell_value(table, r, 0, " ");
    }
    lv_obj_invalidate(table);
}

static void fetch_done(void *arg) {
    fetch_t *f = arg;

    if (f->serial != fetch_serial) {
        /* a fetch given up on: a newer one is in flight, leave it be */
        net_result_free(&f->res);
        free(f);
        return;
    }

    fetching = false;
    if (!dialog.run) {
        /* closed: nothing to show */
    } else if (f->generation != generation) {
        /* reply for a closed-and-reopened window: fetch for this one now */
        request_fetch();
    } else if (!f->res.body) {
        lv_label_set_text_fmt(status, "No spots: %s", f->res.error);
    } else {
        int n = dx_spots_parse(f->res.body, spots, MAX_SPOTS);
        if (n < 0) {
            lv_label_set_text(status, "No spots: unexpected reply");
        } else {
            char band[BAND_LEN];

            spots_n = n;
            rebuild_table();
            show_status();

            /* the filters changed while this was on its way */
            wanted_band(band, sizeof(band));
            if (strcmp(band, f->band) != 0 || mode_filter != f->mode) {
                request_fetch();
            }
        }
    }
    net_result_free(&f->res);
    free(f);
}

static void timer_cb(lv_timer_t *t) {
    lv_timer_set_period(t, REFRESH_MS);   // after a deferred fetch
    request_fetch();
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
    time_t since = time(NULL) - last_fetch;

    if (since >= 0 && since < MIN_GAP_SEC) {
        msg_update_text_fmt("DX spots refresh every %i s at most", MIN_GAP_SEC);
        return;
    }
    request_fetch();
}

/* The list on hand is filtered at once; the server is asked again for the
 * new filters as soon as its gap allows */
static void filters_changed(void) {
    rebuild_table();
    show_status();
    request_fetch();
}

static void band_filter_cb(button_data_t *b) {
    this_band_only = !this_band_only;
    buttons_refresh(&btn_band);
    filters_changed();
}

static void mode_filter_cb(button_data_t *b) {
    mode_filter = (mode_filter + 1) % FILTER_MODES;
    buttons_refresh(&btn_mode);
    filters_changed();
}

static void tune_cb(button_data_t *b) {
    uint16_t row = 0, col = 0;
    char     freq[CELL_LEN];

    lv_table_get_selected_cell(table, &row, &col);
    if (!shown_n || row == LV_TABLE_CELL_NONE || row >= shown_n) {
        msg_update_text_fmt("Pick a spot first");
        return;
    }
    dx_spot_t s = spots[shown[row]];

    if (!radio_check_freq(s.freq_hz)) {
        msg_update_text_fmt("%i kHz is outside the radio's range", s.freq_hz / 1000);
        return;
    }

    /* Tune first, then let the cockpit set the mode for that frequency */
    dialog_destruct(&dialog);
    main_screen_set_freq(s.freq_hz);

    switch (dx_spot_tune(&s)) {
        case DX_TUNE_CW:
            cockpit_enter(COCKPIT_CW);
            break;
        case DX_TUNE_PHONE:
            cockpit_enter(COCKPIT_PHONE);
            break;
        case DX_TUNE_FT8:
        case DX_TUNE_FT4:
            /* The FT8 app tunes to its own protocol's nearest calling frequency */
            param_i_set(cfg.ft8.protocol(), dx_spot_tune(&s) == DX_TUNE_FT4 ? FTX_PROTOCOL_FT4 : FTX_PROTOCOL_FT8);
            cockpit_enter(COCKPIT_DIGITAL);
            break;
        default:
            break;
    }
    format_freq(freq, sizeof(freq), s.freq_hz);
    msg_update_text_fmt("Tuned to %s %s", s.call, freq);
}

/* ---- dialog ---- */

static void construct_cb(lv_obj_t *parent) {
    dialog.obj = dialog_init(parent);
    generation++;

    status = lv_label_create(dialog.obj);
    lv_obj_add_style(status, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(status, &sony_22, 0);
    lv_obj_set_width(status, DIALOG_WIDTH - 32);
    lv_label_set_long_mode(status, LV_LABEL_LONG_DOT);
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
    show_status();

    timer = lv_timer_create(timer_cb, REFRESH_MS, NULL);
    request_fetch();
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
            tune_cb(NULL);
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
