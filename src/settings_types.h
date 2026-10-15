/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#pragma once

// Plain settings enums shared by UI/service code and the cfg parameter layer.
// Kept free of any params/ dependency so headers such as styles.h / display.h
// can expose these types without pulling the legacy params module.

typedef enum {
    BUTTONS_DARK = 0,
    BUTTONS_LIGHT,
    BUTTONS_TEMPORARILY
} buttons_light_t;

typedef enum {
    ACTION_NONE = 0,
    ACTION_SCREENSHOT,
    ACTION_RECORDER,
    ACTION_MUTE,
    ACTION_STEP_UP,
    ACTION_STEP_DOWN,
    ACTION_VOICE_MODE,
    ACTION_BAT_INFO,
    ACTION_NR_TOGGLE,
    ACTION_NB_TOGGLE,

    /* The ACTION_APP_* values are saved on the card (long-press keys), so
     * each is written out and never reused or changed; a new app takes
     * the next free number. 109 WeFax, 110 NavTex, 111 SSTV (Murus),
     * 112 JS8 are taken. */
    ACTION_APP_RTTY = 100,
    ACTION_APP_FT8,
    ACTION_APP_SWRSCAN,
    ACTION_APP_GPS,
    ACTION_APP_SETTINGS,
    ACTION_APP_RECORDER,
    ACTION_APP_QTH,
    ACTION_APP_CALLSIGN,
    ACTION_APP_WIFI,
    ACTION_APP_BT = 116,

    /* KR8MER: Nexus-style operating. 113-115 left free for upstream. */
    ACTION_APP_LOG = 120,       // log a QSO / the logbook
    ACTION_APP_HUNTER = 121,    // POTA/SOTA hunter
    ACTION_COCKPIT_PHONE = 122,
    ACTION_COCKPIT_CW = 123,
    ACTION_COCKPIT_DIGITAL = 124,
    ACTION_COCKPIT_RTTY = 125,
    ACTION_MESSAGES = 126,      // the CW or voice message keyer for the mode
    ACTION_APP_CLUSTER = 127,   // DX cluster spots
    ACTION_APP_SPACEWX = 128,   // space weather: solar flux, K index
} press_action_t;

typedef enum {
    FREQ_ACCEL_NONE = 0,
    FREQ_ACCEL_LITE,
    FREQ_ACCEL_STRONG,
} freq_accel_t;

/* Themes */
typedef enum {
    THEME_SIMPLE,
    THEME_BLACK,
    THEME_FLAT,
    THEME_LIGHTNING,
    THEME_FACTORY,
} themes_t;

/* Meter Color */
typedef enum {
    METER_GRAY,
    METER_COLORED,
} meter_color_t;

/* SWR Color */
typedef enum {
    SWR_GRAY,
    SWR_COLORED,
} swr_color_t;

/* Clock view */
typedef enum {
    CLOCK_TIME_ALLWAYS = 0,
    CLOCK_TIME_POWER,
    CLOCK_POWER_ALLWAYS
} clock_view_t;

/* Voice */
#define VOICES_NUM 4

typedef enum {
    VOICE_OFF = 0,
    VOICE_LCD,
    VOICE_ALWAYS
} voice_mode_t;
