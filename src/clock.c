/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#include "clock.h"

#include "settings_types.h"
#include "cfg/cfg_api.h"

#include "styles.h"
#include "radio.h"
#include "util.h"
#include "display.h"
#include "voice.h"
#include "msg.h"
#include "tz_display.h"

#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>

typedef enum {
    CLOCK_TIME = 0,
    CLOCK_POWER
} clock_state_t;

static lv_obj_t         *obj;
static lv_obj_t         *zone_label;    // zone under the time, when not plain UTC

typedef enum {
    TIME_LAYOUT_NONE = 0,
    TIME_LAYOUT_UTC,        // "18:42:07", as it always was
    TIME_LAYOUT_ZONE,       // "14:42:07" over "EDT"
    TIME_LAYOUT_DUAL,       // "18:42 UTC" over "14:42 EDT"
} time_layout_t;

static time_layout_t    time_layout = TIME_LAYOUT_NONE;
static pthread_mutex_t  power_mux;

static clock_state_t    state = CLOCK_TIME;
static uint64_t         timeout;

static lv_timer_t       *timer_time_sync;
static time_t           last_time_sync=0;

static float            v_ext;
static float            v_bat;
static uint8_t          cap_bat;
static bool             charging;

static char             str[32];

static void check_time_sync_cb(lv_timer_t * t);
static time_t get_last_sync_time();

static void set_state(clock_state_t new_state) {
    state = new_state;

    switch (state) {
        case CLOCK_TIME:
            time_layout = TIME_LAYOUT_NONE;     // show_time() picks the layout
            break;

        case CLOCK_POWER:
            lv_obj_set_style_text_font(obj, &sony_30, 0);
            lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
            if (zone_label) {
                lv_obj_add_flag(zone_label, LV_OBJ_FLAG_HIDDEN);
            }
            break;
    }
}

static void set_time_layout(time_layout_t layout) {
    if (layout == time_layout) {
        return;
    }
    time_layout = layout;

    switch (layout) {
        case TIME_LAYOUT_ZONE:
            lv_obj_set_style_text_font(obj, &sony_34, 0);
            lv_obj_set_style_text_line_space(obj, 0, 0);
            lv_obj_align(obj, LV_ALIGN_CENTER, 0, -9);
            lv_obj_clear_flag(zone_label, LV_OBJ_FLAG_HIDDEN);
            break;

        case TIME_LAYOUT_DUAL:
            lv_obj_set_style_text_font(obj, &sony_26, 0);
            lv_obj_set_style_text_line_space(obj, -2, 0);
            lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
            lv_obj_add_flag(zone_label, LV_OBJ_FLAG_HIDDEN);
            break;

        default:
            lv_obj_set_style_text_font(obj, &sony_38, 0);
            lv_obj_set_style_text_line_space(obj, 0, 0);
            lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
            lv_obj_add_flag(zone_label, LV_OBJ_FLAG_HIDDEN);
            break;
    }
}

/* Time in the chosen zone(s); the system clock itself is UTC */
static void show_zoned_time(time_t now) {
    int32_t     zone  = param_i_get(cfg.ui.clock_tz());
    int32_t     zone2 = param_i_get(cfg.ui.clock_tz2()) - 1;   // -1: off
    struct tm   t;
    const char  *abbr;

    tz_convert(now, zone, &t, &abbr);

    if (zone2 >= 0) {
        struct tm   t2;
        const char  *abbr2;

        tz_convert(now, zone2, &t2, &abbr2);
        set_time_layout(TIME_LAYOUT_DUAL);
        snprintf(str, sizeof(str), "%02i:%02i %s\n%02i:%02i %s", t.tm_hour, t.tm_min, abbr, t2.tm_hour, t2.tm_min,
                 abbr2);
    } else if (zone != 0) {
        set_time_layout(TIME_LAYOUT_ZONE);
        snprintf(str, sizeof(str), "%02i:%02i:%02i", t.tm_hour, t.tm_min, t.tm_sec);
        lv_label_set_text(zone_label, abbr);
    } else {
        set_time_layout(TIME_LAYOUT_UTC);
        snprintf(str, sizeof(str), "%02i:%02i:%02i", t.tm_hour, t.tm_min, t.tm_sec);
    }
}

static void show_time() {
    time_t      now;
    struct tm   *t;

    int32_t clock_view = param_i_get(cfg.ui.clock_view());
    if (clock_view == CLOCK_TIME_ALLWAYS) {
        set_state(CLOCK_TIME);
    } else if (clock_view == CLOCK_POWER_ALLWAYS) {
        set_state(CLOCK_POWER);
    } else if (clock_view == CLOCK_TIME_POWER) {
        uint64_t    ms = get_time();

        if (radio_get_state() == RADIO_RX) {
            if (ms > timeout) {
                switch (state) {
                    case CLOCK_TIME:
                        set_state(CLOCK_POWER);
                        timeout = ms + param_i_get(cfg.ui.clock_power_timeout()) * 1000;
                        break;

                    case CLOCK_POWER:
                        set_state(CLOCK_TIME);
                        timeout = ms + param_i_get(cfg.ui.clock_time_timeout()) * 1000;
                        break;
                }
            }
        } else {
            set_state(CLOCK_POWER);
            timeout = ms + param_i_get(cfg.ui.clock_tx_timeout()) * 1000;
        }
    }

    switch (state) {
        case CLOCK_TIME:
            now = time(NULL);
            show_zoned_time(now);
            break;

        case CLOCK_POWER:
            pthread_mutex_lock(&power_mux);
            const char * bat_sym;
            const char * ext_sym;
            switch ((uint8_t)(v_bat * 10)) {
                case 0 ... 63:
                    bat_sym = LV_SYMBOL_BATTERY_EMPTY;
                    break;
                case 64 ... 69:
                    bat_sym = LV_SYMBOL_BATTERY_1;
                    break;
                case 70 ... 75:
                    bat_sym = LV_SYMBOL_BATTERY_2;
                    break;
                case 76 ... 81:
                    bat_sym = LV_SYMBOL_BATTERY_3;
                    break;
                case 82 ... 100:
                    bat_sym = LV_SYMBOL_BATTERY_FULL;
                    break;
            }
            if (charging) {
                ext_sym = SYMBOL_PLUG_CHARGE;
            } else {
                ext_sym = SYMBOL_PLUG;
            }

            if (v_ext < 3.0f) {
                snprintf(str, sizeof(str), "%s %.1fv\n%i%%", bat_sym, v_bat, cap_bat);
            } else {
                snprintf(str, sizeof(str), "%s %.1fv\n%s %.1fv", bat_sym, v_bat, ext_sym, v_ext);
            }

            pthread_mutex_unlock(&power_mux);
            break;
    }

    lv_label_set_text(obj, str);
}

static void clock_power_cb(const radio_power_t *power) {
    pthread_mutex_lock(&power_mux);
    v_ext = power->vext;
    v_bat = power->vbat;
    cap_bat = power->cap;
    charging = power->charging;
    pthread_mutex_unlock(&power_mux);
}

static void on_clock_param_change(Subject *subj, void *user_data) {
    timeout = get_time();
}

lv_obj_t * clock_init(lv_obj_t * parent) {
    pthread_mutex_init(&power_mux, NULL);
    radio_power_set_cb(clock_power_cb);

    lv_obj_t * container = lv_obj_create(parent);
    lv_obj_remove_style_all(container);
    lv_obj_add_style(container, &style.clock, 0);
    styles_tx_flash_add(container);

    obj = lv_label_create(container);
    lv_obj_remove_style_all(obj);
    lv_obj_center(obj);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);

    zone_label = lv_label_create(container);
    lv_obj_remove_style_all(zone_label);
    lv_obj_add_style(zone_label, &style.text_muted_color, 0);
    lv_obj_set_style_text_font(zone_label, &sony_20, 0);
    lv_obj_set_style_text_letter_space(zone_label, 2, 0);
    lv_obj_align(zone_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_flag(zone_label, LV_OBJ_FLAG_HIDDEN);

    set_state(CLOCK_TIME);
    timeout = get_time() + param_i_get(cfg.ui.clock_time_timeout()) * 1000;

    show_time();
    lv_timer_create(show_time, 500, NULL);

    last_time_sync = get_last_sync_time();
    timer_time_sync = lv_timer_create(check_time_sync_cb, 200, NULL);

    subject_subscribe_delayed((Subject *)cfg.ui.clock_view(), on_clock_param_change, NULL);
    subject_subscribe_delayed((Subject *)cfg.ui.clock_time_timeout(), on_clock_param_change, NULL);
    subject_subscribe_delayed((Subject *)cfg.ui.clock_power_timeout(), on_clock_param_change, NULL);
    subject_subscribe_delayed((Subject *)cfg.ui.clock_tx_timeout(), on_clock_param_change, NULL);
}

void clock_say_bat_info() {
    voice_sure();
    voice_say_float("Battery voltage|", v_bat);
}

static void check_time_sync_cb(lv_timer_t * t) {
    time_t new_time = get_last_sync_time();
    if (last_time_sync != new_time) {
        last_time_sync = new_time;
        msg_schedule_text_fmt("Time is synchronized with NTP");
    }
}

static time_t get_last_sync_time() {
    struct stat attr;
    if (stat("/var/run/time-sync", &attr) == 0) {
        return attr.st_atime;
    }
    return 0;
}
