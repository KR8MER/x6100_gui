/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#pragma once

#include "settings_types.h"
#include "globals.h"

#include <unistd.h>
#include "lvgl/lvgl.h"

/* Size */
#define INDICATORS_HEIGHT 28
#define TOP_BLOCK_SMALL_HEIGHT 70
#define TOP_BLOCK_BIG_HEIGHT (TOP_BLOCK_SMALL_HEIGHT + 2 * INDICATORS_HEIGHT)
#define CLOCK_WIDTH 160
#define FREQ_INFO_WIDTH 280
#define METER_WIDTH (SCREEN_WIDTH - CLOCK_WIDTH - FREQ_INFO_WIDTH)
#define BTN_HEIGHT 61
#define BTN_WIDTH (SCREEN_WIDTH / 5)
#define DIALOG_SPACING 2
#define DIALOG_WIDTH (SCREEN_WIDTH - DIALOG_SPACING * 2)
#define DIALOG_HEIGHT (SCREEN_HEIGHT - TOP_BLOCK_SMALL_HEIGHT - BTN_HEIGHT - DIALOG_SPACING * 2)
#define BAND_INFO_HEIGHT 24
#define BAND_INFO_OFFSET_Y 6
#define PANEL_GAP_BOTTOM 6

// COLORS

#define COLOR_LIGHT_GREEN (0x80FF80)
#define COLOR_LIGHT_YELLOW (0xFFFF30)
#define COLOR_LIGHT_RED (0xFF8080)

// Symbols

#define SYMBOL_PLUG "\xEF\x87\xA6"  // F1E6
#define SYMBOL_PLUG_CHARGE "\xEE\x95\x9B"  // E55B
#define SYMBOL_NORTH_WEST_ARROW "\xE2\x86\x96"
#define SYMBOL_SOUTH_WEST_ARROW "\xE2\x86\x99"

/* Theme colours read by individual widgets (copied from the active skin) */
typedef struct {
    /* Spectrum: soft halo above the trace (alpha 0 = off) */
    lv_color_t spectrum_glow;
    lv_opa_t   spectrum_glow_opa;
    lv_color_t spectrum_center;     // center and RTTY marker lines

    lv_color_t freq_step_mark;      // underline of the tuning-step digit
    lv_color_t tx_pwr;
    lv_color_t swr_ok;
    lv_color_t swr_ok_gray;
    lv_color_t swr_warn;
    lv_color_t swr_bad;

    struct {
        lv_color_t filter;
        lv_color_t cursor;
    } finder;

    struct {
        lv_color_t bg;
        lv_color_t border;
        lv_color_t text;
        lv_color_t info;
        lv_color_t cq_new;
        lv_color_t cq_worked;
        lv_color_t cq_worked_mode;
        lv_color_t to_me;
        lv_color_t tx;
    } ft8;

    struct {
        lv_color_t grid;
        lv_color_t label;
        lv_color_t line;
    } chart;

    lv_color_t level;               // recorder level bar
} ui_colors_t;

typedef struct {
    lv_style_t waterfall_middle_line;

    lv_style_t msg;
    lv_style_t msg_tiny;
    lv_style_t clock;
    lv_style_t knobs;
    lv_style_t freq_info;
    lv_style_t s_meter;
    lv_style_t tx_info;
    lv_style_t cw_tune;

    lv_style_t text_base_color;
    lv_style_t text_muted_color;

    lv_style_t freq_bounds;

    /* Theme-coloured pieces of individual widgets */
    lv_style_t badge_mode;      // mode label on the frequency block
    lv_style_t badge_vfo;       // VFO label on the frequency block
    lv_style_t bg_freq;         // the other VFO's frequency
    lv_style_t ind_text;        // indicator row: idle
    lv_style_t ind_active;      // indicator row: on
    lv_style_t ind_sep;         // indicator row: group separator

    struct {
        lv_style_t base;
        lv_style_t active;
        lv_style_t disabled;
        lv_style_t mark;
        lv_style_t mark_assigned;
    } btn;

    struct {
        lv_style_t base;
        lv_style_t info;
    } panels;

    struct {
        lv_style_t base;
        lv_style_t item;
        lv_style_t item_focus;
        lv_style_t item_edited;
        lv_style_t dropdown;
    } dialog;

    struct {
        lv_style_t preview_cont;
        lv_style_t preview_rect;
        lv_style_t preview_hex;
        lv_style_t slider_panel;
        lv_style_t slider_row;
        lv_style_t letter;
        lv_style_t slider;
        lv_style_t slider_focused;
        lv_style_t val_label;
    } rgb;

    /* Actual colors */
    struct {
        struct {
            lv_color_t low;
            lv_color_t mid;
            lv_color_t high;
            lv_color_t line;
            lv_color_t peak;
        } spectrum;

        struct {
            lv_color_t noise;
            lv_color_t low;
            lv_color_t mid;
            lv_color_t high;
            lv_color_t peak;
        } s_meter;

        lv_color_t mark;
        lv_color_t wf_middle_line;

        ui_colors_t ui;

    } colors;

    const uint32_t *wf_palette;
} styles_t;


typedef struct {
    lv_color_t base_text_color;
} colors_t;

extern styles_t style;
extern colors_t colors;

/* Fonts */

extern lv_font_t    sony_14;
extern lv_font_t    sony_16;
extern lv_font_t    sony_18;
extern lv_font_t    sony_20;
extern lv_font_t    sony_22;
extern lv_font_t    sony_24;
extern lv_font_t    sony_26;
extern lv_font_t    sony_28;
extern lv_font_t    sony_30;
extern lv_font_t    sony_32;
extern lv_font_t    sony_34;
extern lv_font_t    sony_36;
extern lv_font_t    sony_38;
extern lv_font_t    sony_40;
extern lv_font_t    sony_42;
extern lv_font_t    sony_44;
extern lv_font_t    sony_48;
extern lv_font_t    sony_60;

extern lv_font_t    mono_22;
extern lv_font_t    mono_28;
extern lv_font_t    mono_30;
extern lv_font_t    mono_48;

void styles_init(themes_t theme);

void styles_set_theme(themes_t theme);

void styles_update_meter_colors(meter_color_t mc);

/* Top blocks follow TX: skins with TX artwork swap it in and flash once */
void styles_set_tx(bool tx);
void styles_tx_flash_add(lv_obj_t *obj);

/* Resize the panel background: updates the shared panel style geometry and
 * re-renders the pre-computed gradient bitmap for the current skin. */
void styles_panel_set_height(lv_coord_t h);
