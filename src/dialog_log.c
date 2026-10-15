/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Log a QSO in any mode, after Nexus's log strip: band, frequency, mode and
 *  the usual report are filled in from the radio, a pending POTA/SOTA hunt
 *  fills in the activator and reference, and the entry goes into the same log
 *  FT8 writes to. Below the form, the latest QSOs; Export writes the whole
 *  log to /mnt/qso_log.adi for LoTW, QRZ, POTA or a desktop logger.
 */

#include "dialog_log.h"

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
#include "log_rules.h"
#include "msg.h"
#include "qso_log.h"
#include "radio.h"
#include "styles.h"
#include "textarea_window.h"
#include "tz_display.h"

#define RECENT_ROWS 40
#define EXPORT_PATH "/mnt/qso_log.adi"

static void construct_cb(lv_obj_t *parent);
static void destruct_cb(void);
static void key_cb(lv_event_t *e);

static void edit_call_cb(button_data_t *b);
static void edit_rsts_cb(button_data_t *b);
static void edit_rstr_cb(button_data_t *b);
static void save_cb(button_data_t *b);
static void edit_name_cb(button_data_t *b);
static void edit_ref_cb(button_data_t *b);
static void export_cb(button_data_t *b);
static void clear_cb(button_data_t *b);

static button_data_t btn_p1   = {.type = BTN_TEXT, .label = "(LOG 1:2)", .press = button_next_page_cb};
static button_data_t btn_call = {.type = BTN_TEXT, .label = "Call", .press = edit_call_cb};
static button_data_t btn_rsts = {.type = BTN_TEXT, .label = "Sent", .press = edit_rsts_cb};
static button_data_t btn_rstr = {.type = BTN_TEXT, .label = "Rcvd", .press = edit_rstr_cb};
static button_data_t btn_save = {.type = BTN_TEXT, .label = "Save", .press = save_cb};

static button_data_t btn_p2     = {.type = BTN_TEXT, .label = "(LOG 2:2)", .press = button_next_page_cb};
static button_data_t btn_name   = {.type = BTN_TEXT, .label = "Name", .press = edit_name_cb};
static button_data_t btn_ref    = {.type = BTN_TEXT, .label = "Park /\nSummit", .press = edit_ref_cb};
static button_data_t btn_export = {.type = BTN_TEXT, .label = "Export\nADIF", .press = export_cb};
static button_data_t btn_clear  = {.type = BTN_TEXT, .label = "Clear", .press = clear_cb};

static buttons_page_t page_1 = {{&btn_p1, &btn_call, &btn_rsts, &btn_rstr, &btn_save}};
static buttons_page_t page_2 = {{&btn_p2, &btn_name, &btn_ref, &btn_export, &btn_clear}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page_1,
    .key_cb       = NULL,
};

dialog_t *dialog_log = &dialog;

static lv_obj_t *header;
static lv_obj_t *form;
static lv_obj_t *table;

/* The entry being edited */
static struct {
    char call[16];
    char rsts[8];
    char rstr[8];
    char name[32];
    char ref[24];
} entry;

/* Which field the text window is editing */
static char  *editing;
static size_t editing_len;

static qso_log_mode_t current_mode(void) {
    return log_mode_for(cparam_i_get(cfg.cur.mode()), cockpit_current());
}

static int32_t current_freq(void) {
    return cparam_i_get(cfg.cur.fg_freq());
}

static void refresh_header(void) {
    int32_t f = current_freq();
    lv_label_set_text_fmt(header, "%i.%03i.%03i   %s   %s cockpit", f / 1000000, (f / 1000) % 1000, f % 1000,
                          log_mode_name(current_mode()), cockpit_name(cockpit_current()));
}

static void refresh_form(void) {
    const char *sig = log_sig_for_ref(entry.ref);

    lv_label_set_text_fmt(form, "#ffeb3b %s#   %s / %s   %s   %s%s%s", entry.call[0] ? entry.call : "(call)",
                          entry.rsts, entry.rstr, entry.name, sig[0] ? sig : "", sig[0] ? " " : "", entry.ref);
}

/* Recent QSOs, one string per column */
enum { COL_TIME, COL_CALL, COL_BAND, COL_MODE, COL_RST, COL_REF, COLS };

static char       cells[RECENT_ROWS][COLS][28];
static int        rows_n;
static const char *empty_text;

static const char *cell_get(uint16_t row, uint8_t col) {
    if (!rows_n) return col == 0 ? empty_text : NULL;
    return row < rows_n ? cells[row][col] : NULL;
}

static const lv_coord_t     col_x[COLS] = {0, 170, 330, 400, 480, 600};
static const cell_columns_t columns     = {.x = col_x, .count = COLS, .get = cell_get};

static void refresh_table(void) {
    static qso_log_record_t rows[RECENT_ROWS];

    rows_n = qso_log_recent(rows, RECENT_ROWS);
    empty_text = "No QSOs yet";

    for (int i = 0; i < rows_n; i++) {
        struct tm t;

        gmtime_r(&rows[i].time, &t);
        snprintf(cells[i][COL_TIME], sizeof(cells[i][0]), "%02i-%02i %02i:%02iZ", t.tm_mon + 1, t.tm_mday, t.tm_hour,
                 t.tm_min);
        snprintf(cells[i][COL_CALL], sizeof(cells[i][0]), "%s", rows[i].remote_call);
        snprintf(cells[i][COL_BAND], sizeof(cells[i][0]), "%im", (int)rows[i].band);
        snprintf(cells[i][COL_MODE], sizeof(cells[i][0]), "%s", log_mode_name(rows[i].mode));
        snprintf(cells[i][COL_RST], sizeof(cells[i][0]), "%i/%i", rows[i].rsts, rows[i].rstr);
        snprintf(cells[i][COL_REF], sizeof(cells[i][0]), "%s", rows[i].sig_info);
    }

    /* the cells hold placeholders: the columns are drawn by cell_columns */
    lv_table_set_row_cnt(table, rows_n ? rows_n : 1);
    for (int i = 0; i < (rows_n ? rows_n : 1); i++) {
        lv_table_set_cell_value(table, i, 0, " ");
    }
    lv_obj_invalidate(table);
}

static void reset_entry(void) {
    int rst = log_default_rst(current_mode());

    memset(&entry, 0, sizeof(entry));
    snprintf(entry.rsts, sizeof(entry.rsts), "%i", rst);
    snprintf(entry.rstr, sizeof(entry.rstr), "%i", rst);

    /* A pending hunt prefills the activator and the reference */
    char who[32], ref[24];
    if (hunt_target_peek(time(NULL), who, sizeof(who), ref, sizeof(ref))) {
        snprintf(entry.call, sizeof(entry.call), "%s", who);
        snprintf(entry.ref, sizeof(entry.ref), "%s", ref);
    }
}

/* Back to the list with the encoder after the text window */
static bool window_closed(void) {
    lv_group_add_obj(keyboard_group, table);
    lv_group_set_editing(keyboard_group, true);
    editing = NULL;
    return true;
}

static bool edit_ok(void) {
    if (editing) {
        snprintf(editing, editing_len, "%s", textarea_window_get());
        /* calls and references are capitals */
        if (editing == entry.call || editing == entry.ref) {
            for (char *p = editing; *p; p++) {
                if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
            }
        }
    }
    refresh_form();
    return window_closed();
}

static void edit(char *field, size_t len, const char *title, const char *chars) {
    editing     = field;
    editing_len = len;
    textarea_window_open_w_label(edit_ok, window_closed, title);
    lv_obj_t *text = textarea_window_text();
    if (chars) {
        lv_textarea_set_accepted_chars(text, chars);
    }
    lv_textarea_set_max_length(text, len - 1);
    textarea_window_set(field);
}

#define CALL_CHARS "0123456789/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

static void edit_call_cb(button_data_t *b) { edit(entry.call, sizeof(entry.call), "Call", CALL_CHARS); }
static void edit_rsts_cb(button_data_t *b) { edit(entry.rsts, sizeof(entry.rsts), "Report sent", "+-0123456789"); }
static void edit_rstr_cb(button_data_t *b) { edit(entry.rstr, sizeof(entry.rstr), "Report rcvd", "+-0123456789"); }
static void edit_name_cb(button_data_t *b) { edit(entry.name, sizeof(entry.name), "Name", NULL); }
static void edit_ref_cb(button_data_t *b) {
    edit(entry.ref, sizeof(entry.ref), "Park or summit", CALL_CHARS "-");
}

static void save_cb(button_data_t *b) {
    const char *mycall = PARAM_T_GET(cfg.callsign());

    if (!mycall[0]) {
        msg_update_text_fmt("Set your callsign first (APP > Callsign)");
        return;
    }
    if (!entry.call[0]) {
        msg_update_text_fmt("Enter the station's call");
        return;
    }

    qso_log_record_t qso = qso_log_record_create(mycall, entry.call, time(NULL), current_mode(), atoi(entry.rsts),
                                                 atoi(entry.rstr), current_freq(), entry.name, NULL,
                                                 PARAM_T_GET(cfg.qth()), NULL);
    const char *sig = log_sig_for_ref(entry.ref);
    if (sig[0]) {
        snprintf(qso.sig, sizeof(qso.sig), "%s", sig);
        snprintf(qso.sig_info, sizeof(qso.sig_info), "%s", entry.ref);
    }

    if (qso_log_record_save(qso) > 0) {
        msg_update_text_fmt("Logged %s", entry.call);
        reset_entry();
        refresh_form();
        refresh_table();
    } else {
        msg_update_text_fmt("Not logged (already in the log?)");
    }
}

static void export_cb(button_data_t *b) {
    int n = qso_log_export_adif(EXPORT_PATH);
    if (n < 0) {
        msg_update_text_fmt("Export failed");
    } else {
        msg_update_text_fmt("%i QSOs written to qso_log.adi", n);
    }
}

static void clear_cb(button_data_t *b) {
    reset_entry();
    refresh_form();
}

static void construct_cb(lv_obj_t *parent) {
    dialog.obj = dialog_init(parent);

    page_1.items[0]->next = &page_2;
    page_1.items[0]->prev = &page_2;
    page_2.items[0]->next = &page_1;
    page_2.items[0]->prev = &page_1;

    header = lv_label_create(dialog.obj);
    lv_obj_add_style(header, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(header, &sony_22, 0);
    lv_obj_set_pos(header, 16, 12);

    form = lv_label_create(dialog.obj);
    lv_label_set_recolor(form, true);
    lv_obj_set_style_text_font(form, &sony_30, 0);
    lv_obj_set_pos(form, 16, 40);

    table = lv_table_create(dialog.obj);
    lv_obj_remove_style(table, NULL, LV_STATE_ANY | LV_PART_MAIN);
    lv_obj_set_size(table, DIALOG_WIDTH - 24, DIALOG_HEIGHT - 96);
    lv_obj_set_pos(table, 12, 88);
    lv_table_set_col_cnt(table, 1);
    lv_table_set_col_width(table, 0, DIALOG_WIDTH - 40);
    lv_obj_set_style_text_font(table, &mono_22, LV_PART_ITEMS);
    lv_obj_set_style_border_width(table, 0, LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(table, LV_OPA_TRANSP, LV_PART_ITEMS);
    lv_obj_add_style(table, &style.text_base_color, LV_PART_ITEMS);
    lv_obj_set_style_pad_ver(table, 3, LV_PART_ITEMS);
    lv_obj_set_style_pad_hor(table, 4, LV_PART_ITEMS);
    lv_obj_add_style(table, &style.dialog.item_focus, LV_PART_ITEMS | LV_STATE_EDITED);
    cell_columns_attach(table, &columns);
    lv_obj_add_event_cb(table, key_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(keyboard_group, table);
    lv_group_set_editing(keyboard_group, true);

    reset_entry();
    refresh_header();
    refresh_form();
    refresh_table();
    lm_set_mode(true);
}

static void destruct_cb(void) {
    textarea_window_close();
    lm_set_mode(false);
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
