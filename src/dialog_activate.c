/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Activate POTA window: set the park(s) you are activating, watch the
 *  day's count towards 10, see the QSOs per band and mode, spot yourself on
 *  pota.app, and end the activation. The QSOs themselves are logged as
 *  usual (Log QSO, FT8/FT4): while activating they carry your park(s), and
 *  the upload files in pota/ on the DATA partition are kept current.
 */

#include "dialog_activate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "cockpit.h"
#include "events.h"
#include "keyboard.h"
#include "log_rules.h"
#include "msg.h"
#include "pota_activation.h"
#include "radio.h"
#include "styles.h"
#include "textarea_window.h"

#define PAD       16
#define TICK_MS   2000
#define SUMMARY_ROWS 12

static void construct_cb(lv_obj_t *parent);
static void destruct_cb(void);
static void key_cb(lv_event_t *e);
static void parks_cb(button_data_t *b);
static void spot_cb(button_data_t *b);
static void comment_cb(button_data_t *b);
static void end_cb(button_data_t *b);

static button_data_t btn_title   = {.type = BTN_TEXT, .label = "(POTA ACT)"};
static button_data_t btn_parks   = {.type = BTN_TEXT, .label = "My\nParks", .press = parks_cb};
static button_data_t btn_spot    = {.type = BTN_TEXT, .label = "Spot\nMe", .press = spot_cb};
static button_data_t btn_comment = {.type = BTN_TEXT, .label = "Spot\nComment", .press = comment_cb};
static button_data_t btn_end     = {.type = BTN_TEXT, .label = "End", .press = end_cb};

static buttons_page_t page = {{&btn_title, &btn_parks, &btn_spot, &btn_comment, &btn_end}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page,
    .key_cb       = key_cb,
};

dialog_t *dialog_activate = &dialog;

static lv_obj_t   *status;
static lv_obj_t   *count_label;
static lv_obj_t   *names_label;
static lv_obj_t   *summary;
static lv_obj_t   *files_label;
static lv_obj_t   *spot_label;
static lv_timer_t *timer;

static char spot_comment[48];

static enum { EDIT_NONE, EDIT_PARKS, EDIT_COMMENT } editing;

static qso_log_mode_t current_mode(void) {
    return log_mode_for(cparam_i_get(cfg.cur.mode()), cockpit_current());
}

static const char *group_name(qso_log_mode_group_t g) {
    switch (g) {
        case MODE_GROUP_CW: return "CW";
        case MODE_GROUP_PHONE: return "Phone";
        default: return "Data";
    }
}

static void names_found(const char *ref, bool ok);

static void show(void) {
    char parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n   = pota_activation_parks(parks, POTA_PARKS_MAX);
    time_t now = time(NULL);

    if (n == 0) {
        lv_label_set_text(status, "Not activating. Press My Parks and enter your park, e.g. US-1234");
        lv_label_set_text(count_label, "");
        lv_label_set_text(names_label, "");
        lv_label_set_text(summary, "");
        lv_label_set_text(files_label, "");
        lv_label_set_text(spot_label, "");
        return;
    }

    struct tm tm;
    gmtime_r(&now, &tm);
    lv_label_set_text_fmt(status, "Activating today (UTC %04d-%02d-%02d). Log QSOs as usual: Log QSO or FT8.",
                          tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    /* "US-1234  7/10    US-5678  7/10" */
    char line[160] = "", names[200] = "";
    size_t used = 0, nused = 0;
    for (int i = 0; i < n; i++) {
        int c = qso_log_activation_count(parks[i], now);
        if (c < 0) c = 0;
        used += snprintf(line + used, sizeof(line) - used, "%s%s %i/%i%s", i ? "   " : "", parks[i], c,
                         POTA_QSOS_NEEDED, c >= POTA_QSOS_NEEDED ? " OK" : "");
        if (used >= sizeof(line)) used = sizeof(line) - 1;
        const char *name = pota_park_name(parks[i]);
        if (name[0]) {
            nused += snprintf(names + nused, sizeof(names) - nused, "%s%s", nused ? ";  " : "", name);
            if (nused >= sizeof(names)) nused = sizeof(names) - 1;
        }
    }
    lv_label_set_text(count_label, line);
    lv_label_set_text(names_label, names);

    /* QSOs per band and mode group, for the first park (all parks share them) */
    qso_log_band_count_t rows[SUMMARY_ROWS];
    int  rn = qso_log_activation_summary(parks[0], now, rows, SUMMARY_ROWS);
    char sum[400] = "";
    size_t su = 0;
    for (int i = 0; i < rn; i++) {
        bool new_band = i == 0 || rows[i].band != rows[i - 1].band;
        if (new_band) {
            su += snprintf(sum + su, sizeof(sum) - su, "%s%im:", i ? "\n" : "", (int)rows[i].band);
        }
        if (su >= sizeof(sum)) break;
        su += snprintf(sum + su, sizeof(sum) - su, "  %s %i", group_name(rows[i].group), rows[i].count);
        if (su >= sizeof(sum)) break;
    }
    lv_label_set_text(summary, rn > 0 ? sum : "No QSOs at this park yet today");

    char call[PARAM_TEXT_MAX], file[96];
    param_t_get_into(cfg.callsign(), call, sizeof(call));
    pota_upload_name(call, parks[0], now, file, sizeof(file));
    lv_label_set_text_fmt(files_label, "Upload to pota.app: pota/%s%s", file, n > 1 ? " (and one per park)" : "");

    uint64_t spot = pota_last_spot_freq();
    uint64_t freq = (uint64_t)cparam_i_get(cfg.cur.fg_freq());
    if (!spot) {
        lv_label_set_text(spot_label, "Not spotted yet: Spot Me sends this frequency and mode to pota.app");
    } else if ((freq > spot ? freq - spot : spot - freq) >= 1000) {
        lv_label_set_text_fmt(spot_label, "#ffeb3b You moved from %llu.%llu kHz: press Spot Me to re-spot#",
                              (unsigned long long)(spot / 1000), (unsigned long long)(spot % 1000 / 100));
    } else {
        lv_label_set_text_fmt(spot_label, "Spotted on %llu.%llu kHz", (unsigned long long)(spot / 1000),
                              (unsigned long long)(spot % 1000 / 100));
    }
}

static void names_found(const char *ref, bool ok) {
    if (!dialog.run) return;
    show();
    /* the next park without a name, one look-up at a time */
    char parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n = pota_activation_parks(parks, POTA_PARKS_MAX);
    for (int i = 0; ok && i < n; i++) {
        if (!pota_park_name(parks[i])[0] && strcmp(parks[i], ref) != 0) {
            pota_park_lookup(parks[i], names_found);
            break;
        }
    }
}

static void look_up_names(void) {
    char parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n = pota_activation_parks(parks, POTA_PARKS_MAX);
    for (int i = 0; i < n; i++) {
        if (!pota_park_name(parks[i])[0]) {
            pota_park_lookup(parks[i], names_found);
            return;
        }
    }
}

static void tick_cb(lv_timer_t *t) {
    (void)t;
    show();
}

static lv_obj_t *add_label(const lv_font_t *font, bool muted, lv_coord_t y, lv_coord_t h) {
    lv_obj_t *l = lv_label_create(dialog.obj);

    lv_obj_add_style(l, muted ? &style.text_muted_color : &style.text_base_color, 0);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_pos(l, PAD, y);
    lv_obj_set_width(l, DIALOG_WIDTH - PAD * 2);
    if (h > 0) {
        lv_obj_set_height(l, h);
        lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
    }
    lv_label_set_text(l, "");
    return l;
}

static void construct_cb(lv_obj_t *parent) {
    dialog.obj = dialog_init(parent);

    lv_group_add_obj(keyboard_group, dialog.obj);
    lv_obj_add_event_cb(dialog.obj, key_cb, LV_EVENT_KEY, NULL);

    status      = add_label(&sony_22, true, 12, lv_font_get_line_height(&sony_22));
    count_label = add_label(&sony_44, false, 44, lv_font_get_line_height(&sony_44));
    names_label = add_label(&sony_22, true, 100, lv_font_get_line_height(&sony_22));
    summary     = add_label(&sony_24, false, 136, 0);
    lv_obj_set_height(summary, DIALOG_HEIGHT - 136 - 76);
    lv_label_set_long_mode(summary, LV_LABEL_LONG_CLIP);
    files_label = add_label(&sony_20, true, DIALOG_HEIGHT - 66, lv_font_get_line_height(&sony_20));
    spot_label  = add_label(&sony_20, false, DIALOG_HEIGHT - 36, lv_font_get_line_height(&sony_20));
    lv_label_set_recolor(spot_label, true);

    editing = EDIT_NONE;
    show();
    look_up_names();
    timer = lv_timer_create(tick_cb, TICK_MS, NULL);
}

static void destruct_cb(void) {
    if (timer) {
        lv_timer_del(timer);
        timer = NULL;
    }
    if (editing != EDIT_NONE) {
        textarea_window_close();
        editing = EDIT_NONE;
    }
}

/* ---- text entry ---- */

static bool text_closed(void) {
    editing = EDIT_NONE;
    lv_group_add_obj(keyboard_group, dialog.obj);
    lv_group_focus_obj(dialog.obj);
    return true;
}

static bool text_ok(void) {
    const char *text = textarea_window_get();

    if (editing == EDIT_PARKS) {
        char why[96];
        if (!pota_activation_set(text, why, sizeof(why))) {
            msg_update_text_fmt("%s", why);
            return false;   // keep the window open to correct it
        }
        if (pota_activation_active()) {
            pota_activation_write_files(time(NULL));
            msg_update_text_fmt("Activating %s. QSOs you log now carry it.", PARAM_T_GET(cfg.pota_parks()));
            look_up_names();
        } else {
            msg_update_text_fmt("Not activating");
        }
    } else if (editing == EDIT_COMMENT) {
        snprintf(spot_comment, sizeof(spot_comment), "%s", text);
        msg_update_text_fmt(spot_comment[0] ? "Spot comment set" : "Spot comment: " POTA_SPOT_DEFAULT_COMMENT);
    }
    show();
    return text_closed();
}

static void open_text(const char *title, const char *value, const char *chars, int max) {
    lv_group_remove_obj(dialog.obj);
    textarea_window_open_w_label(text_ok, text_closed, title);
    lv_obj_t *text = textarea_window_text();
    if (chars) {
        lv_textarea_set_accepted_chars(text, chars);
    }
    lv_textarea_set_max_length(text, max);
    textarea_window_set(value);
    textarea_window_focus_keys();
}

#define REF_CHARS "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-,"

static void parks_cb(button_data_t *b) {
    (void)b;
    msg_update_text_fmt("Your park, or parks for an n-fer: US-1234,US-5678 (empty ends)");
    editing = EDIT_PARKS;
    open_text("My parks", PARAM_T_GET(cfg.pota_parks()), REF_CHARS, PARAM_TEXT_MAX - 1);
}

static void comment_cb(button_data_t *b) {
    (void)b;
    msg_update_text_fmt("Your spot's comment (empty: \"" POTA_SPOT_DEFAULT_COMMENT "\"). QRT in it ends your spot.");
    editing = EDIT_COMMENT;
    open_text("Comment", spot_comment, NULL, sizeof(spot_comment) - 1);
}

/* ---- spot, end ---- */

static void spot_cb(button_data_t *b) {
    (void)b;
    char comment[96];
    char parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n = pota_activation_parks(parks, POTA_PARKS_MAX);

    /* an n-fer is spotted at the first park, the others named in the comment */
    if (n > 1) {
        char others[64];
        pota_refs_join(&parks[1], n - 1, others, sizeof(others));
        snprintf(comment, sizeof(comment), "%d-fer with %s%s%s", n, others, spot_comment[0] ? " " : "",
                 spot_comment);
    } else {
        snprintf(comment, sizeof(comment), "%s", spot_comment);
    }
    switch (pota_self_spot((uint64_t)cparam_i_get(cfg.cur.fg_freq()), current_mode(), comment)) {
        case POTA_SPOT_STARTED:
            msg_update_text_fmt("Spotting on pota.app...");
            break;
        case POTA_SPOT_NOT_ACTIVATING:
            msg_update_text_fmt("Set your park first (My Parks)");
            break;
        case POTA_SPOT_NO_CALL:
            msg_update_text_fmt("Set your callsign first (APP > Callsign)");
            break;
        case POTA_SPOT_BAD_CALL:
            msg_update_text_fmt("pota.app would not take your callsign as set (APP > Callsign)");
            break;
        case POTA_SPOT_OUT_OF_BAND:
            msg_update_text_fmt("Not on an amateur band: no spot");
            break;
        case POTA_SPOT_BAD_MODE:
            msg_update_text_fmt("Pick a cockpit (Phone, CW, FT8) so the spot has a mode");
            break;
        case POTA_SPOT_TOO_SOON:
            msg_update_text_fmt("Already spotted here: again in %i min at most once", POTA_SPOT_GAP_SEC / 60);
            break;
        case POTA_SPOT_BUSY:
            msg_update_text_fmt("A spot is on its way");
            break;
        default:
            msg_update_text_fmt("Could not spot (Wi-Fi on?)");
            break;
    }
}

static void end_cb(button_data_t *b) {
    (void)b;
    char parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n = pota_activation_parks(parks, POTA_PARKS_MAX);

    if (n == 0) {
        msg_update_text_fmt("Not activating");
        return;
    }
    time_t now   = time(NULL);
    int    files = pota_activation_write_files(now);
    int    count = qso_log_activation_count(parks[0], now);
    pota_activation_set("", NULL, 0);
    show();
    if (count >= POTA_QSOS_NEEDED) {
        msg_update_text_fmt("%s: %i QSOs, valid. %i upload file(s) in pota/", parks[0], count, files);
    } else {
        msg_update_text_fmt("%s: %i QSOs (10 needed). %i upload file(s) in pota/", parks[0], count < 0 ? 0 : count,
                            files);
    }
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
