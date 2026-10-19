/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Log a QSO in any mode, after Nexus's log strip: band, frequency, mode and
 *  the usual report are filled in from the radio, a pending POTA/SOTA hunt
 *  fills in the activator and reference, and the entry goes into the same log
 *  FT8 writes to. Below the form, the latest QSOs. The whole log is kept as
 *  /mnt/qso_log.adi (rewritten after every QSO, and by Export) for LoTW,
 *  QRZ, POTA or a desktop logger.
 *
 *  On line (page 3), with the operator's own accounts: a call entered is
 *  looked up on HamQTH (name, QTH, grid, country), and a saved QSO goes
 *  straight to the QRZ Logbook; Upload QRZ sends whatever is still unsent.
 */

#include "dialog_log.h"

#include <stdint.h>
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
#include "log_online.h"
#include "log_rules.h"
#include "pota_activation.h"
#include "msg.h"
#include "qso_log.h"
#include "radio.h"
#include "styles.h"
#include "textarea_window.h"
#include "tz_display.h"

#define RECENT_ROWS 40

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
static void hamqth_login_cb(button_data_t *b);
static void qrz_key_cb(button_data_t *b);
static void upload_cb(button_data_t *b);
static void lookup_cb(button_data_t *b);

static button_data_t btn_p1   = {.type = BTN_TEXT, .label = "(LOG 1:3)", .press = button_next_page_cb};
static button_data_t btn_call = {.type = BTN_TEXT, .label = "Call", .press = edit_call_cb};
static button_data_t btn_rsts = {.type = BTN_TEXT, .label = "Sent", .press = edit_rsts_cb};
static button_data_t btn_rstr = {.type = BTN_TEXT, .label = "Rcvd", .press = edit_rstr_cb};
static button_data_t btn_save = {.type = BTN_TEXT, .label = "Save", .press = save_cb};

static button_data_t btn_p2     = {.type = BTN_TEXT, .label = "(LOG 2:3)", .press = button_next_page_cb};
static button_data_t btn_name   = {.type = BTN_TEXT, .label = "Name", .press = edit_name_cb};
static button_data_t btn_ref    = {.type = BTN_TEXT, .label = "Park /\nSummit", .press = edit_ref_cb};
static button_data_t btn_export = {.type = BTN_TEXT, .label = "Export\nADIF", .press = export_cb};
static button_data_t btn_clear  = {.type = BTN_TEXT, .label = "Clear", .press = clear_cb};

static button_data_t btn_p3     = {.type = BTN_TEXT, .label = "(LOG 3:3)", .press = button_next_page_cb};
static button_data_t btn_hamqth = {.type = BTN_TEXT, .label = "HamQTH\nLogin", .press = hamqth_login_cb};
static button_data_t btn_qrzkey = {.type = BTN_TEXT, .label = "QRZ\nKey", .press = qrz_key_cb};
static button_data_t btn_upload = {.type = BTN_TEXT, .label = "Upload\nQRZ", .press = upload_cb};
static button_data_t btn_lookup = {.type = BTN_TEXT, .label = "Look up", .press = lookup_cb};

static buttons_page_t page_1 = {{&btn_p1, &btn_call, &btn_rsts, &btn_rstr, &btn_save}};
static buttons_page_t page_2 = {{&btn_p2, &btn_name, &btn_ref, &btn_export, &btn_clear}};
static buttons_page_t page_3 = {{&btn_p3, &btn_hamqth, &btn_qrzkey, &btn_upload, &btn_lookup}};

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
    .destruct_cb  = destruct_cb,
    .btn_page     = &page_1,
    .key_cb       = NULL,
};

dialog_t *dialog_log = &dialog;

static lv_obj_t *header;
static lv_obj_t *info;      // HamQTH lookup result, right of the header
static lv_obj_t *form;
static lv_obj_t *table;

/* The entry being edited */
static struct {
    char call[16];
    char rsts[8];
    char rstr[8];
    char name[32];
    char ref[48];   // one park or summit, or the other station's parks "US-1,US-2"
    char grid[8];   // from the lookup, saved as the station's grid
} entry;

/* A lookup reply counts only while its call is still the one entered:
 * every change of call (and closing the window) moves this on */
static uint32_t lookup_gen;
static char     looked_up_name[sizeof(entry.name)];   // the Name the lookup filled in

/* HamQTH user name while the password is asked for */
static char login_user[PARAM_TEXT_MAX];

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
    char    act[48] = "";
    char    parks[POTA_PARKS_MAX][POTA_REF_LEN];
    int     n = pota_activation_parks(parks, POTA_PARKS_MAX);

    if (n > 0) {
        int count = pota_activation_count_today();
        snprintf(act, sizeof(act), "   POTA %s%s %i/%i", parks[0], n > 1 ? "+" : "", count < 0 ? 0 : count,
                 POTA_QSOS_NEEDED);
    }
    lv_label_set_text_fmt(header, "%i.%03i.%03i   %s   %s%s", f / 1000000, (f / 1000) % 1000, f % 1000,
                          log_mode_name(current_mode()), n > 0 ? "" : cockpit_name(cockpit_current()),
                          n > 0 ? act : " cockpit");
}

/* The form label colours text between '#' marks, and LVGL 8 cannot escape
 * one, so typed text is shown with '#' as a lookalike (it is saved as typed) */
static const char *no_recolor(const char *text, char *buf, size_t n) {
    snprintf(buf, n, "%s", text);
    for (char *p = buf; *p; p++) {
        if (*p == '#') *p = '=';
    }
    return buf;
}

static void refresh_form(void) {
    const char *sig = log_sig_for_ref(entry.ref);
    char        call[sizeof(entry.call)], name[sizeof(entry.name)], ref[sizeof(entry.ref)];

    /* activating: a call already worked today on this band and mode */
    bool dupe = entry.call[0] && pota_activation_active() &&
                pota_activation_is_dupe(entry.call, qso_log_freq_to_band(current_freq()), current_mode());

    if (!sig[0] && strchr(entry.ref, ',')) {
        sig = "POTA";   // the other station's parks, one QSO each
    }
    lv_label_set_text_fmt(form, "#ffeb3b %s#%s   %s / %s   %s   %s%s%s",
                          entry.call[0] ? no_recolor(entry.call, call, sizeof(call)) : "(call)",
                          dupe ? " #ff5252 DUPE#" : "", entry.rsts, entry.rstr,
                          no_recolor(entry.name, name, sizeof(name)), sig[0] ? sig : "", sig[0] ? " " : "",
                          no_recolor(entry.ref, ref, sizeof(ref)));
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
    looked_up_name[0] = 0;
    lookup_gen++;
    if (info) lv_label_set_text(info, "");
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

/* The text window takes the encoder: with the table still in the group and
 * in editing mode, the knob scrolled the list instead of the on-screen keys */
static void open_text_window(textarea_window_cb_t ok, textarea_window_cb_t cancel, const char *title) {
    lv_group_remove_obj(table);
    textarea_window_open_w_label(ok, cancel, title);
    textarea_window_focus_keys();
}

static void start_lookup(bool asked);

/* Worked before? Every QSO with this station, however the call was written */
static void show_history(const char *call) {
    qso_log_record_t last;
    int              n = qso_log_history(call, &last);
    char             buf[sizeof(entry.call)];

    if (n == 0) {
        msg_update_text_fmt("%s: first QSO", no_recolor(call, buf, sizeof(buf)));
    } else if (n > 0) {
        struct tm t;
        gmtime_r(&last.time, &t);
        msg_update_text_fmt("%s: worked %i time%s, last %04i-%02i-%02i %im %s", no_recolor(call, buf, sizeof(buf)),
                            n, n > 1 ? "s" : "", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, (int)last.band,
                            log_mode_name(last.mode));
    }
}

/* The park's name, from pota.app, as a message */
static void park_found(const char *ref, bool ok) {
    if (ok && dialog.run) {
        msg_update_text_fmt("%s: %s", ref, pota_park_name(ref));
    }
}

static void show_park(const char *ref) {
    const char *name = pota_park_name(ref);
    if (name[0]) {
        msg_update_text_fmt("%s: %s", ref, name);
    } else {
        pota_park_lookup(ref, park_found);
    }
}

static bool edit_ok(void) {
    char old_call[sizeof(entry.call)];
    bool call_edit = editing == entry.call;

    snprintf(old_call, sizeof(old_call), "%s", entry.call);
    if (editing) {
        snprintf(editing, editing_len, "%s", textarea_window_get());
        /* calls and references are capitals */
        if (editing == entry.call || editing == entry.ref) {
            for (char *p = editing; *p; p++) {
                if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
            }
        }
    }
    if (call_edit && strcmp(old_call, entry.call) != 0) {
        /* what the last lookup found belongs to the old call */
        lookup_gen++;
        entry.grid[0] = 0;
        if (looked_up_name[0] && strcmp(entry.name, looked_up_name) == 0) {
            entry.name[0] = 0;
        }
        looked_up_name[0] = 0;
        lv_label_set_text(info, "");
        if (entry.call[0]) {
            start_lookup(false);
            show_history(entry.call);
        }
    }
    if (editing == entry.ref && log_sig_for_ref(entry.ref)[0] && strcmp(log_sig_for_ref(entry.ref), "POTA") == 0) {
        show_park(entry.ref);
    }
    refresh_form();
    return window_closed();
}

static void edit(char *field, size_t len, const char *title, const char *chars) {
    editing     = field;
    editing_len = len;
    open_text_window(edit_ok, window_closed, title);
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
    msg_update_text_fmt("Their park or summit; a 2-fer as US-1234,US-5678");
    edit(entry.ref, sizeof(entry.ref), "Park or summit", CALL_CHARS "-,");
}

/* ---- on line ---- */

/* Server text goes through no_recolor() into messages; the info label
 * does not colour, so it takes text as it is */
static void lookup_done(const char *call, callbook_status_t status, const callbook_info_t *found,
                        const char *error, void *user) {
    char buf[128];

    /* the window was closed or the call changed since */
    if (!dialog.run || (uint32_t)(uintptr_t)user != lookup_gen) {
        return;
    }
    switch (status) {
        case CALLBOOK_OK: {
            char where[96];
            const char *sep = found->qth[0] && found->country[0] ? ", " : "";

            if (!entry.name[0] && found->nick[0]) {
                snprintf(entry.name, sizeof(entry.name), "%s", found->nick);
                snprintf(looked_up_name, sizeof(looked_up_name), "%s", entry.name);
            }
            /* a grid too long for the log is better left out than cut */
            if (strlen(found->grid) < sizeof(entry.grid)) {
                snprintf(entry.grid, sizeof(entry.grid), "%s", found->grid);
            }
            snprintf(where, sizeof(where), "%s%s%s", found->qth, sep, found->country);
            /* credit first: a long place name is cut at the end, never the credit */
            lv_label_set_text_fmt(info, "HamQTH.com: %s  %s", where, found->grid);
            refresh_form();
            break;
        }

        case CALLBOOK_NOT_FOUND:
            /* quiet: plenty of calls are not on HamQTH */
            lv_label_set_text_fmt(info, "%s not on HamQTH.com", call);
            break;

        case CALLBOOK_BAD_LOGIN:
            lv_label_set_text(info, "");
            msg_update_text_fmt("HamQTH: wrong user name or password (LOG 3:3)");
            break;

        default:
            lv_label_set_text(info, "");
            msg_update_text_fmt("HamQTH: %s", no_recolor(error, buf, sizeof(buf)));
            break;
    }
}

/* `asked`: the operator pressed Look up, so say why nothing happens */
static void start_lookup(bool asked) {
    if (!log_online_hamqth_ready()) {
        if (asked) msg_update_text_fmt("Set your HamQTH login first (HamQTH Login)");
        return;
    }
    if (!entry.call[0]) {
        if (asked) msg_update_text_fmt("Enter the station's call");
        return;
    }
    lookup_gen++;
    log_online_start_t st = log_online_lookup(entry.call, lookup_done, (void *)(uintptr_t)lookup_gen);
    if (st == LOG_ONLINE_STARTED) {
        lv_label_set_text_fmt(info, "Looking up %s...", entry.call);
    } else if (st == LOG_ONLINE_NO_ACCOUNT) {
        if (asked) msg_update_text_fmt("Set your HamQTH password first (HamQTH Login)");
    } else {
        msg_update_text_fmt("HamQTH: lookup did not start");
    }
}

static void lookup_cb(button_data_t *b) {
    start_lookup(true);
}

static void qrz_saved_done(const log_online_upload_t *res, void *user) {
    char buf[128];

    switch (res->result) {
        case QRZ_RESULT_OK:
            msg_update_text_fmt("QRZ: %s uploaded", res->call);
            break;
        case QRZ_RESULT_DUPLICATE:
            msg_update_text_fmt("QRZ: %s already in your logbook", res->call);
            break;
        default:
            msg_update_text_fmt("QRZ: %s", no_recolor(res->reason, buf, sizeof(buf)));
            break;
    }
}

static void qrz_batch_done(const log_online_upload_t *res, void *user) {
    char buf[128];
    char left[40] = "";

    if (res->remaining > 0) {
        snprintf(left, sizeof(left), ", %i not sent yet", res->remaining);
    }
    if (res->failed) {
        msg_update_text_fmt("QRZ: %i uploaded, %i already there, %i failed%s: %s", res->uploaded, res->duplicates,
                            res->failed, left, no_recolor(res->reason, buf, sizeof(buf)));
    } else {
        msg_update_text_fmt("QRZ: %i uploaded, %i already there%s", res->uploaded, res->duplicates, left);
    }
}

static void upload_cb(button_data_t *b) {
    switch (log_online_qrz_upload_unsent(qrz_batch_done, NULL)) {
        case LOG_ONLINE_STARTED:
            msg_update_text_fmt("QRZ: uploading...");
            break;
        case LOG_ONLINE_NO_ACCOUNT:
            msg_update_text_fmt("Set your QRZ Logbook API key first (QRZ Key)");
            break;
        case LOG_ONLINE_BUSY:
            msg_update_text_fmt("QRZ: an upload is already running");
            break;
        case LOG_ONLINE_NOTHING:
            msg_update_text_fmt("QRZ: every QSO is already uploaded");
            break;
        default:
            msg_update_text_fmt("QRZ: upload did not start");
            break;
    }
}

/* Settings windows. Nothing secret is put back on screen: the password
 * field starts empty and hides what is typed, the key shows its last 4 */

static void login_check_done(const char *call, callbook_status_t status, const callbook_info_t *found,
                             const char *error, void *user) {
    char buf[128];

    if (status == CALLBOOK_OK) {
        msg_update_text_fmt("HamQTH: logged in. Data provided by HamQTH.com");
    } else if (status == CALLBOOK_BAD_LOGIN) {
        msg_update_text_fmt("HamQTH: wrong user name or password");
    } else {
        msg_update_text_fmt("HamQTH: %s", no_recolor(error, buf, sizeof(buf)));
    }
}

static bool password_ok(void) {
    char pass[PARAM_TEXT_MAX];

    snprintf(pass, sizeof(pass), "%s", textarea_window_get());
    /* the window's own copy of the password goes with it */
    textarea_window_set("");
    if (!pass[0]) {
        msg_update_text_fmt("HamQTH login not changed");
        return window_closed();
    }
    param_t_set(cfg.hamqth_user(), login_user);
    param_t_set(cfg.hamqth_pass(), pass);
    memset(pass, 0, sizeof(pass));
    log_online_hamqth_forget();

    if (log_online_lookup("", login_check_done, NULL) == LOG_ONLINE_STARTED) {
        msg_update_text_fmt("HamQTH: logging in as %s...", login_user);
    }
    return window_closed();
}

static bool password_cancel(void) {
    textarea_window_set("");
    return window_closed();
}

/* After the user name window has closed */
static void ask_password(void *unused) {
    if (!dialog.run) return;
    open_text_window(password_ok, password_cancel, "Password");
    lv_obj_t *text = textarea_window_text();
    lv_textarea_set_password_mode(text, true);
    lv_textarea_set_max_length(text, PARAM_TEXT_MAX - 1);
    textarea_window_set("");
}

static bool login_user_ok(void) {
    snprintf(login_user, sizeof(login_user), "%s", textarea_window_get());
    if (!login_user[0]) {
        param_t_set(cfg.hamqth_user(), "");
        param_t_set(cfg.hamqth_pass(), "");
        log_online_hamqth_forget();
        lv_label_set_text(info, "");
        msg_update_text_fmt("HamQTH login removed");
    } else {
        /* open the next window once this one is gone */
        lv_async_call(ask_password, NULL);
    }
    return window_closed();
}

static void hamqth_login_cb(button_data_t *b) {
    msg_update_text_fmt("Your HamQTH.com user name (empty removes the login)");
    open_text_window(login_user_ok, window_closed, "HamQTH user");
    lv_obj_t *text = textarea_window_text();
    lv_textarea_set_accepted_chars(text, CALL_CHARS "-_.@");
    lv_textarea_set_max_length(text, PARAM_TEXT_MAX - 1);
    textarea_window_set(PARAM_T_GET(cfg.hamqth_user()));
}

static bool qrz_key_ok(void) {
    char key[PARAM_TEXT_MAX];

    snprintf(key, sizeof(key), "%s", textarea_window_get());
    textarea_window_set("");
    param_t_set(cfg.qrz_key(), key);
    if (key[0]) {
        size_t n = strlen(key);
        msg_update_text_fmt("QRZ key saved (...%s)", n > 4 ? key + n - 4 : "");
    } else {
        msg_update_text_fmt("QRZ key removed: QSOs are not uploaded");
    }
    memset(key, 0, sizeof(key));
    return window_closed();
}

static bool qrz_key_cancel(void) {
    textarea_window_set("");
    return window_closed();
}

static void qrz_key_cb(button_data_t *b) {
    char   key[PARAM_TEXT_MAX];
    size_t n;

    param_t_get_into(cfg.qrz_key(), key, sizeof(key));
    n = strlen(key);
    if (n) {
        msg_update_text_fmt("QRZ Logbook API key now ...%s. Type a new one, or none to remove it",
                            n > 4 ? key + n - 4 : "");
    } else {
        msg_update_text_fmt("Your QRZ Logbook API key (QRZ.com > Logbook > Settings)");
    }
    memset(key, 0, sizeof(key));
    open_text_window(qrz_key_ok, qrz_key_cancel, "QRZ key");
    lv_obj_t *text = textarea_window_text();
    lv_textarea_set_accepted_chars(text, CALL_CHARS "-");
    lv_textarea_set_max_length(text, PARAM_TEXT_MAX - 1);
    textarea_window_set("");
}

/* ---- buttons ---- */

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
                                                 PARAM_T_GET(cfg.qth()), entry.grid[0] ? entry.grid : NULL);
    pota_activation_tag(&qso);

    /* Their park(s): a 2-fer is logged as one QSO per park, the same in all
     * but SIG_INFO, which is how POTA credits a P2P with each park */
    char theirs[POTA_PARKS_MAX][POTA_REF_LEN];
    int  n_theirs = 0;
    if (strchr(entry.ref, ',')) {
        n_theirs = pota_refs_parse(entry.ref, theirs, POTA_PARKS_MAX, NULL, 0);
        if (n_theirs <= 0) {
            msg_update_text_fmt("Park list: references like US-1234,US-5678");
            return;
        }
    } else {
        const char *sig = log_sig_for_ref(entry.ref);
        if (sig[0]) {
            snprintf(qso.sig, sizeof(qso.sig), "%s", sig);
            snprintf(qso.sig_info, sizeof(qso.sig_info), "%s", entry.ref);
        }
    }

    int saved = 0;
    if (n_theirs > 0) {
        for (int i = 0; i < n_theirs; i++) {
            snprintf(qso.sig, sizeof(qso.sig), "POTA");
            snprintf(qso.sig_info, sizeof(qso.sig_info), "%s", theirs[i]);
            if (qso_log_record_save(qso) > 0) saved++;
        }
    } else if (qso_log_record_save(qso) > 0) {
        saved = 1;
    }

    if (saved > 0) {
        qso_log_update_adif();
        if (n_theirs > 1) {
            msg_update_text_fmt("Logged %s at %i parks", entry.call, saved);
        } else {
            msg_update_text_fmt("Logged %s", entry.call);
        }
        pota_activation_saved(&qso);   // says how far the activation is
        /* straight on to the QRZ Logbook when a key is set */
        if (log_online_qrz_ready()) {
            log_online_start_t st = log_online_qrz_upload_qso(qso.time, qso.remote_call, qrz_saved_done, NULL);
            if (st == LOG_ONLINE_BUSY) {
                msg_schedule_text_fmt("QRZ: busy, %s goes with the next Upload QRZ", qso.remote_call);
            } else if (st != LOG_ONLINE_STARTED) {
                msg_schedule_text_fmt("QRZ: %s not sent, use Upload QRZ", qso.remote_call);
            }
        }
        reset_entry();
        refresh_header();
        refresh_form();
        refresh_table();
    } else {
        msg_update_text_fmt("Not logged (already in the log?)");
    }
}

static void export_cb(button_data_t *b) {
    int n = qso_log_update_adif();
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
    page_1.items[0]->prev = &page_3;
    page_2.items[0]->next = &page_3;
    page_2.items[0]->prev = &page_1;
    page_3.items[0]->next = &page_1;
    page_3.items[0]->prev = &page_2;

    header = lv_label_create(dialog.obj);
    lv_obj_add_style(header, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(header, &sony_22, 0);
    lv_obj_set_pos(header, 16, 12);

    info = lv_label_create(dialog.obj);
    lv_obj_add_style(info, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(info, &sony_22, 0);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_long_mode(info, LV_LABEL_LONG_DOT);
    lv_obj_set_width(info, DIALOG_WIDTH / 2 - 24);
    /* one line: "..." needs a fixed height, or the label wraps over the form */
    lv_obj_set_height(info, lv_font_get_line_height(&sony_22));
    lv_obj_align(info, LV_ALIGN_TOP_RIGHT, -16, 12);
    lv_label_set_text(info, "");

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
    lookup_gen++;   // a lookup still running is ignored when it returns
    info = NULL;
    memset(login_user, 0, sizeof(login_user));
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
