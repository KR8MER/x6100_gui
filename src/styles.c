/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#include "styles.h"

#include <stdlib.h>
#include "globals.h"
#include "cfg/cfg_api.h"

#define PATH "A:/dev/shm/"

const uint32_t wf_palette_legacy[] = {
#include "palettes/legacy.inc"
};

const uint32_t wf_palette_gauss[] = {
#include "palettes/gauss.inc"
};

const uint32_t wf_palette_lightning[] = {
#include "palettes/lightning.inc"
};

/* Skin API */
typedef struct {
    const uint32_t *wf_palette;
    lv_color_t base_text_color;
    lv_color_t spectrum_color;
    lv_color_t mark_color;

    lv_color_t wf_middle_line_color;
    lv_coord_t wf_middle_line_min_width;

    // Highlight for checked buttons and the focused dialog item
    lv_color_t accent_color;
    lv_opa_t   accent_opa;
    lv_color_t focus_border_color;
    // Fill for the focused/edited dialog item (otherwise the widget's own bg shows through)
    bool       focus_fill;
    lv_color_t focus_bg_color;
    lv_color_t edited_bg_color;

    ui_colors_t ui;

    // Widget chrome: badges, readouts and the indicator row
    struct {
        lv_color_t mode_bg;
        lv_color_t mode_text;
        lv_color_t vfo_bg;
        lv_color_t vfo_text;
        lv_opa_t   badge_opa;
        lv_color_t bg_freq;
        lv_color_t ind_text;
        lv_color_t ind_active_bg;
        lv_color_t ind_active_text;
        lv_color_t ind_sep;
        lv_color_t readout_bg;
    } chrome;

    // Top blocks while transmitting. NULL images: no change on TX.
    struct {
        void      *clock;
        void      *freq_info;
        void      *tx_info;
        lv_color_t flash;
    } tx;

    struct {
        lv_color_t noise;
        lv_color_t low;
        lv_color_t mid;
        lv_color_t high;
        lv_color_t peak;
    } s_meter;

    // Background images. Can be *char or *lv_img_dsc_t or NULL
    struct {
        void *btn;
        void *msg;
        void *msg_tiny;
        void *clock;
        void *s_meter;
        void *tx_info;
        void *freq_info;
        void *dialog;
    } bg_img;

    // Background colors (RGBA). Alpha 0 can be used to disable.
    struct {
        lv_color_t btn;
        lv_color_t s_meter;
        lv_color_t tx_info;
        lv_color_t freq_info;
        lv_color_t clock;
    } bg_color;

    // Panel background render parameters (the bitmap itself is re-rendered on resize).
    struct {
        lv_grad_dsc_t bg_grad;
        lv_grad_dsc_t border_grad;
        lv_opa_t      opa;
        lv_coord_t    border_width;
        lv_coord_t    radius;
    } panel;
} skin_t;


styles_t style;
colors_t colors;

static uint32_t rng_state=1;

static skin_t skin_default;
static skin_t skin_flat;
static skin_t skin_black;
static skin_t skin_lightning;

static skin_t *skin_current;

static lv_img_dsc_t panel_bg_dsc;

static bool      tx_active;
static lv_obj_t *tx_targets[3];   // top blocks that flash on key-down
static lv_coord_t   panel_bg_w = DIALOG_WIDTH;
static lv_coord_t   panel_bg_h = 182;


static void setup_skin_default(skin_t *skin);
static void setup_skin_flat(skin_t *skin);
static void setup_skin_black(skin_t *skin);
static void setup_skin_lightning(skin_t *skin);

static void set_skin(skin_t *skin);
static void apply_tx_art(skin_t *skin);

static void render_panel_bg(skin_t *skin, lv_coord_t w, lv_coord_t h);

static void update_spectrum_color_cb(Subject *subj, void *user_data);

void styles_init(themes_t theme) {
    /* * */
    lv_style_t *s;

    lv_style_init(&style.text_base_color);
    lv_style_init(&style.text_muted_color);

    lv_style_init(&style.freq_bounds);
    lv_style_set_text_font(&style.freq_bounds, &mono_22);
    lv_style_set_pad_all(&style.freq_bounds, 3);
    lv_style_set_text_align(&style.freq_bounds, LV_TEXT_ALIGN_CENTER);
    lv_style_set_bg_color(&style.freq_bounds, lv_color_black());
    lv_style_set_bg_opa(&style.freq_bounds, LV_OPA_40);
    lv_style_set_radius(&style.freq_bounds, 5);

    lv_style_init(&style.badge_mode);
    lv_style_init(&style.badge_vfo);
    lv_style_init(&style.bg_freq);
    lv_style_init(&style.ind_text);
    lv_style_init(&style.ind_active);
    lv_style_init(&style.ind_sep);

    /* Buttons */
    lv_style_init(&style.btn.base);
    lv_style_set_text_font(&style.btn.base, &sony_30);
    lv_style_set_bg_img_opa(&style.btn.base, LV_OPA_COVER);
    lv_style_set_border_width(&style.btn.base, 0);
    lv_style_set_radius(&style.btn.base, 0);
    lv_style_set_width(&style.btn.base, BTN_WIDTH);
    lv_style_set_height(&style.btn.base, BTN_HEIGHT);

    lv_style_init(&style.btn.active);

    lv_style_init(&style.btn.disabled);
    lv_style_set_bg_img_recolor(&style.btn.disabled, lv_color_hex(0x000000));
    lv_style_set_bg_img_recolor_opa(&style.btn.disabled, LV_OPA_20);
    lv_style_set_text_color(&style.btn.disabled, lv_color_hex(0x101010));

    s = &style.btn.mark;
    lv_style_set_width(s, 24),
    lv_style_set_height(s, 24),
    lv_style_set_radius(s, 12);
    lv_style_set_bg_color(s, lv_color_hex(0x808080));
    lv_style_set_blend_mode(s, LV_BLEND_MODE_ADDITIVE);
    lv_style_set_outline_width(s, 0);
    lv_style_set_border_width(s, 2);
    lv_style_set_border_color(s, lv_color_hex(0x909090));
    lv_style_set_opa(s, LV_OPA_30);

    s = &style.btn.mark_assigned;
    lv_style_init(s);
    lv_style_set_opa(s, LV_OPA_40);
    lv_style_set_bg_color(s, lv_color_hex(0x80ff80));

    /* Message style */
    lv_style_init(&style.msg);
    lv_style_set_pad_hor(&style.msg, 10);
    lv_style_set_text_font(&style.msg, &sony_38);
    lv_style_set_width(&style.msg, 603);
    lv_style_set_height(&style.msg, 66);
    lv_style_set_text_align(&style.msg, LV_TEXT_ALIGN_CENTER);
    lv_style_set_radius(&style.msg, 0);
    lv_style_set_bg_img_opa(&style.msg, LV_OPA_COVER);

    lv_style_init(&style.msg_tiny);
    lv_style_set_text_font(&style.msg_tiny, &sony_60);
    lv_style_set_width(&style.msg_tiny, 324);
    lv_style_set_height(&style.msg_tiny, 66);
    lv_style_set_radius(&style.msg_tiny, 0);
    lv_style_set_pad_ver(&style.msg_tiny, 12);

    /* Panel */
    lv_style_init(&style.panels.base);
    lv_style_set_text_font(&style.panels.base, &sony_38);
    lv_style_set_width(&style.panels.base, DIALOG_WIDTH);
    // TODO: dynamic height
    lv_style_set_height(&style.panels.base, 182);
    lv_style_set_align(&style.panels.base, LV_ALIGN_BOTTOM_MID);
    lv_style_set_translate_y(&style.panels.base, -BTN_HEIGHT - PANEL_GAP_BOTTOM);
    lv_style_set_pad_ver(&style.panels.base, 10);
    lv_style_set_pad_hor(&style.panels.base, 10);
    lv_style_set_radius(&style.panels.base, 0);
    lv_style_set_bg_img_opa(&style.panels.base, LV_OPA_COVER);

    lv_style_init(&style.panels.info);
    lv_style_set_align(&style.panels.info, LV_ALIGN_TOP_LEFT);
    lv_style_set_x(&style.panels.info, 10);
    lv_style_set_y(&style.panels.info, -38);
    lv_style_set_text_font(&style.panels.info, &sony_28);
    lv_style_set_text_color(&style.panels.info, lv_color_hex(0xaaaaaa));

    lv_style_init(&style.dialog.base);
    lv_style_set_text_font(&style.dialog.base, &sony_36);
    lv_style_set_width(&style.dialog.base, DIALOG_WIDTH);
    lv_style_set_height(&style.dialog.base, DIALOG_HEIGHT);
    lv_style_set_x(&style.dialog.base, (SCREEN_WIDTH - DIALOG_WIDTH) / 2);
    lv_style_set_y(&style.dialog.base, TOP_BLOCK_SMALL_HEIGHT + DIALOG_SPACING);
    lv_style_set_radius(&style.dialog.base, 0);
    lv_style_set_bg_img_opa(&style.dialog.base, LV_OPA_COVER);
    lv_style_set_pad_ver(&style.dialog.base, 0);
    lv_style_set_pad_hor(&style.dialog.base, 0);

    lv_style_init(&style.dialog.item);
    lv_style_set_bg_opa(&style.dialog.item, LV_OPA_TRANSP);

    lv_style_init(&style.dialog.item_focus);
    lv_style_set_text_color(&style.dialog.item_focus, lv_color_black());
    lv_style_set_border_width(&style.dialog.item_focus, 2);

    lv_style_init(&style.dialog.item_edited);
    lv_style_set_bg_opa(&style.dialog.item_edited, LV_OPA_COVER);
    lv_style_set_text_color(&style.dialog.item_edited, lv_color_black());

    lv_style_init(&style.dialog.dropdown);
    lv_style_set_text_font(&style.dialog.dropdown, &sony_30);

    /* Waterfall elements */
    lv_style_init(&style.waterfall_middle_line);
    lv_style_set_line_opa(&style.waterfall_middle_line, LV_OPA_60);
    lv_style_set_pad_all(&style.waterfall_middle_line, 0);

    /* Clock */
    lv_style_init(&style.clock);
    lv_style_set_radius(&style.clock, 0);
    lv_style_set_bg_img_opa(&style.clock, LV_OPA_COVER);
    lv_style_set_width(&style.clock, CLOCK_WIDTH);
    lv_style_set_height(&style.clock, TOP_BLOCK_SMALL_HEIGHT);

    /* Knobs */
    lv_style_init(&style.knobs);
    lv_style_set_text_font(&style.knobs, &sony_24);
    lv_style_set_radius(&style.knobs, 8);
    lv_style_set_bg_opa(&style.knobs, LV_OPA_60);
    lv_style_set_bg_color(&style.knobs, lv_color_black());
    lv_style_set_border_width(&style.knobs, 0);
    lv_style_set_pad_hor(&style.knobs, 5);
    lv_style_set_pad_ver(&style.knobs, 3);

    /* Freq info */
    lv_style_init(&style.freq_info);
    lv_style_set_pad_all(&style.freq_info, 0);
    lv_style_set_radius(&style.freq_info, 0);
    lv_style_set_bg_img_opa(&style.freq_info, LV_OPA_COVER);
    lv_style_set_border_width(&style.freq_info, 0);
    lv_style_set_bg_opa(&style.freq_info, LV_OPA_0);
    lv_style_set_width(&style.freq_info, FREQ_INFO_WIDTH);
    lv_style_set_height(&style.freq_info, TOP_BLOCK_SMALL_HEIGHT);

    /* Meter */
    lv_style_init(&style.s_meter);
    lv_style_set_radius(&style.s_meter, 0);
    lv_style_set_align(&style.s_meter, LV_ALIGN_TOP_MID);
    lv_style_set_border_width(&style.s_meter, 0);
    lv_style_set_bg_img_opa(&style.s_meter, LV_OPA_COVER);
    lv_style_set_bg_opa(&style.s_meter, LV_OPA_0);
    lv_style_set_width(&style.s_meter, METER_WIDTH);
    lv_style_set_height(&style.s_meter, TOP_BLOCK_SMALL_HEIGHT);
    lv_style_set_pad_all(&style.s_meter, 10);

    /* TX info */
    lv_style_init(&style.tx_info);
    lv_style_set_radius(&style.tx_info, 0);
    lv_style_set_align(&style.tx_info, LV_ALIGN_TOP_MID);
    lv_style_set_border_width(&style.tx_info, 0);
    lv_style_set_bg_img_opa(&style.tx_info, LV_OPA_COVER);
    lv_style_set_bg_opa(&style.tx_info, LV_OPA_0);
    lv_style_set_width(&style.tx_info, METER_WIDTH);
    lv_style_set_height(&style.tx_info, TOP_BLOCK_BIG_HEIGHT);
    lv_style_set_pad_all(&style.tx_info, 10);

    /* CW tune */
    lv_style_init(&style.cw_tune);
    lv_style_set_radius(&style.cw_tune, 5);
    lv_style_set_bg_color(&style.cw_tune, lv_color_black());
    lv_style_set_border_width(&style.cw_tune, 0);
    lv_style_set_opa(&style.cw_tune, LV_OPA_50);
    lv_style_set_align(&style.cw_tune, LV_ALIGN_TOP_LEFT);
    lv_style_set_translate_x(&style.cw_tune, 90);
    lv_style_set_translate_y(&style.cw_tune, 5 + TOP_BLOCK_SMALL_HEIGHT);

    /* RGB Picker Styles */
    lv_style_init(&style.rgb.preview_cont);
    lv_style_set_bg_opa(&style.rgb.preview_cont, LV_OPA_TRANSP);
    lv_style_set_pad_top(&style.rgb.preview_cont, 10);

    lv_style_init(&style.rgb.preview_rect);
    lv_style_set_radius(&style.rgb.preview_rect, 12);
    lv_style_set_border_color(&style.rgb.preview_rect, lv_color_white());
    lv_style_set_border_width(&style.rgb.preview_rect, 1);
    lv_style_set_bg_color(&style.rgb.preview_rect, lv_color_hex(0xAAAAAA));

    lv_style_init(&style.rgb.preview_hex);
    lv_style_set_text_font(&style.rgb.preview_hex, &sony_26);

    lv_style_init(&style.rgb.slider_panel);
    lv_style_set_bg_opa(&style.rgb.slider_panel, LV_OPA_TRANSP);
    lv_style_set_pad_row(&style.rgb.slider_panel, 12);

    lv_style_init(&style.rgb.slider_row);
    lv_style_set_pad_column(&style.rgb.slider_row, 10);
    lv_style_set_pad_top(&style.rgb.slider_row, 5);

    lv_style_init(&style.rgb.letter);
    lv_style_set_text_font(&style.rgb.letter, &sony_26);
    lv_style_set_pad_top(&style.rgb.letter, -2);

    lv_style_init(&style.rgb.slider);
    lv_style_set_border_width(&style.rgb.slider, 2);
    lv_style_set_border_color(&style.rgb.slider, lv_color_white());
    lv_style_set_border_opa(&style.rgb.slider, LV_OPA_COVER);
    lv_style_set_radius(&style.rgb.slider, 10);

    lv_style_init(&style.rgb.slider_focused);
    lv_style_set_border_width(&style.rgb.slider_focused, 3);
    lv_style_set_border_color(&style.rgb.slider_focused, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_init(&style.rgb.val_label);
    lv_style_set_text_font(&style.rgb.val_label, &sony_26);
    lv_style_set_pad_top(&style.rgb.val_label, -2);

    setup_skin_default(&skin_default);
    setup_skin_flat(&skin_flat);
    setup_skin_black(&skin_black);
    setup_skin_lightning(&skin_lightning);

    styles_set_theme(theme);

    subject_subscribe_delayed((Subject *)cfg.ui.spectrum_use_custom_color(), update_spectrum_color_cb, NULL);
    subject_subscribe_delayed((Subject *)cfg.ui.spectrum_color(), update_spectrum_color_cb, NULL);
}

void styles_update_meter_colors(meter_color_t mc)
{
    if (mc == METER_COLORED) {
        style.colors.s_meter.noise = lv_color_hex(0x228B22);
        style.colors.s_meter.low = lv_color_hex(0x00CC00);
        style.colors.s_meter.mid = lv_color_hex(0xFFFF00);
        style.colors.s_meter.high = lv_color_hex(0xAA0000);
        style.colors.s_meter.peak = lv_color_hex(0xFFFF00);
    } else {
        style.colors.s_meter.noise = skin_current->s_meter.noise;
        style.colors.s_meter.low = skin_current->s_meter.low;
        style.colors.s_meter.mid = skin_current->s_meter.mid;
        style.colors.s_meter.high = skin_current->s_meter.high;
        style.colors.s_meter.peak = skin_current->s_meter.peak;
    }
}

void styles_set_theme(themes_t theme) {
    switch (theme) {
        case THEME_BLACK:
            skin_current = &skin_black;
            break;
        case THEME_FLAT:
            skin_current = &skin_flat;
            break;
        case THEME_LIGHTNING:
            skin_current = &skin_lightning;
            break;
        case THEME_SIMPLE:
        default:
            skin_current = &skin_default;
            break;
        }
        set_skin(skin_current);
}

static void set_spectrum_color(lv_color_t color) {
    lv_color_hsv_t hsv = lv_color_to_hsv(color);
    style.colors.spectrum.mid = color;

    // Low darker and bit more saturated
    style.colors.spectrum.low = lv_color_hsv_to_rgb(
        hsv.h,
        LV_CLAMP(0, hsv.s * 1.2f, 100),
        hsv.v * 0.3f);

    // High is bright and bit less saturates
    style.colors.spectrum.high = lv_color_hsv_to_rgb(
        hsv.h,
        hsv.s * 0.4f,
        100);

    // style.colors.spectrum.line = color;
    style.colors.spectrum.line = lv_color_hsv_to_rgb(hsv.h, 12, 85);
    style.colors.spectrum.peak = lv_color_hsv_to_rgb(hsv.h, 12, 30);
}

static void update_spectrum_color(skin_t *skin) {
    if (param_i_get(cfg.ui.spectrum_use_custom_color())) {
        lv_color_t col;
        col.full = param_i_get(cfg.ui.spectrum_color());
        set_spectrum_color(col);
    } else {
        set_spectrum_color(skin->spectrum_color);
    }
}

static void update_spectrum_color_cb(Subject *subj, void *user_data) {
    update_spectrum_color(skin_current);
}

static bool style_get_size(lv_style_t *style, lv_coord_t *w, lv_coord_t *h) {
    lv_style_value_t prop;
    lv_style_res_t res;
    res = lv_style_get_prop(style, LV_STYLE_WIDTH, &prop);
    if (res != LV_STYLE_RES_FOUND) {
        return false;
    }
    *w = prop.num;
    res = lv_style_get_prop(style, LV_STYLE_HEIGHT, &prop);
    if (res != LV_STYLE_RES_FOUND) {
        return false;
    }
    *h = prop.num;
    return true;
}

static inline uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static inline int get_gaussian_noise_approx(uint32_t *rng_state) {
    int r1 = ((int)(xorshift32(rng_state) & 7)) - 4;
    int r2 = ((int)(xorshift32(rng_state) & 7)) - 4;
    int r3 = ((int)(xorshift32(rng_state) & 7)) - 4;

    return (r1 + r2 + r3) / 3;
}

static inline void add_noise(uint8_t *val) {
    int noise = get_gaussian_noise_approx(&rng_state);
    int pixel_val = *val + noise;

    if (pixel_val < 0) {
        *val = 0;
    } else if (pixel_val > 255) {
        *val = 255;
    } else {
        *val = (uint8_t)pixel_val;
    }
}

static void render_grad_bg_with_border(lv_coord_t w, lv_coord_t h, lv_img_dsc_t *bg_dsc, lv_opa_t opa,
                                       lv_coord_t border_width, lv_coord_t radius, const lv_grad_dsc_t *bg_grad,
                                       const lv_grad_dsc_t *border_grad) {
    lv_coord_t outer_radius = radius;
    lv_coord_t inner_radius = outer_radius - border_width;

    if (inner_radius < 0)
        inner_radius = 0;

    if (bg_dsc->data) {
        free((void *)bg_dsc->data);
        bg_dsc->data      = NULL;
        bg_dsc->data_size = 0;
    }

    uint32_t cf = LV_IMG_CF_TRUE_COLOR_ALPHA;

    bg_dsc->header.always_zero = 0;
    bg_dsc->header.w           = w;
    bg_dsc->header.h           = h;
    bg_dsc->header.cf          = cf;
    bg_dsc->data_size          = w * h * 4;
    bg_dsc->data               = malloc(w * h * 4);

    uint8_t  *canvas_buffer = (uint8_t *)bg_dsc->data;
    lv_obj_t *canvas        = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, canvas_buffer, w, h, cf);

    // 2. Clear canvas with a base color
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_TRANSP);

    lv_draw_rect_dsc_t draw_dsc;

    // Draw border
    if (border_width && border_grad) {
        lv_draw_rect_dsc_init(&draw_dsc);
        draw_dsc.bg_grad = *border_grad;
        draw_dsc.radius  = outer_radius;

        lv_canvas_draw_rect(canvas, 0, 0, w, h, &draw_dsc);
    }

    // Draw inner
    lv_draw_rect_dsc_init(&draw_dsc);
    draw_dsc.bg_grad = *bg_grad;
    draw_dsc.radius  = inner_radius;

    lv_canvas_draw_rect(canvas, border_width, border_width, w - 2 * border_width, h - 2 * border_width, &draw_dsc);

    lv_obj_del(canvas);

    // dithering + alpha
    for (int i = 0; i < w * h; i++) {
        int j = i * 4;
        // Add noise for all channels
        add_noise(&canvas_buffer[j]);
        add_noise(&canvas_buffer[j + 1]);
        add_noise(&canvas_buffer[j + 2]);
        canvas_buffer[j + 3] = ((uint16_t)canvas_buffer[j + 3] * opa + 128) >> 8;
    }
}

static void render_panel_bg(skin_t *skin, lv_coord_t w, lv_coord_t h) {
    render_grad_bg_with_border(w, h, &panel_bg_dsc, skin->panel.opa, skin->panel.border_width, skin->panel.radius,
                               &skin->panel.bg_grad,
                               skin->panel.border_width ? &skin->panel.border_grad : NULL);
}

void styles_panel_set_height(lv_coord_t h) {
    panel_bg_h = h;

    lv_style_set_height(&style.panels.base, panel_bg_h);

    if (skin_current) {
        render_panel_bg(skin_current, panel_bg_w, panel_bg_h);
        lv_style_set_bg_img_src(&style.panels.base, &panel_bg_dsc);
    }

    lv_obj_invalidate(lv_scr_act());
}

static void setup_skin_default(skin_t *skin) {
    static lv_img_dsc_t btn_bg_dsc;
    static lv_img_dsc_t dialog_bg_dsc;
    static lv_img_dsc_t clock_bg_dsc;
    static lv_img_dsc_t freq_info_bg_dsc;
    static lv_img_dsc_t s_meter_bg_dsc;
    static lv_img_dsc_t tx_info_bg_dsc;
    static lv_img_dsc_t msg_bg_dsc;
    static lv_img_dsc_t msg_tiny_bg_dsc;

    skin->wf_palette = wf_palette_gauss;

    skin->base_text_color = lv_color_white();
    // skin->base_text_color = lv_color_hex(0x00ffaa);

    skin->mark_color     = lv_color_hex(0x576d82);
    skin->spectrum_color = lv_color_hex(0xEAC345);

    skin->wf_middle_line_color     = lv_color_hex(0xAAAAAA);
    skin->wf_middle_line_min_width = 2;

    skin->accent_color = lv_color_hex(0x00FF00);
    skin->accent_opa   = LV_OPA_20;
    skin->focus_border_color = lv_color_white();
    skin->focus_fill         = false;

    skin->ui.spectrum_glow       = lv_color_black();
    skin->ui.spectrum_glow_opa   = LV_OPA_TRANSP;
    skin->ui.spectrum_center     = lv_color_hex(0xAAAAAA);
    skin->ui.freq_step_mark      = lv_color_hex(0x4CAF50);
    skin->ui.tx_pwr              = lv_color_hex(0xAAAAAA);
    skin->ui.swr_ok              = lv_color_hex(0x00CC00);
    skin->ui.swr_ok_gray         = lv_color_hex(0xAAAAAA);
    skin->ui.swr_warn            = lv_color_hex(0xAAAA00);
    skin->ui.swr_bad             = lv_color_hex(0xAA0000);
    skin->ui.finder.filter       = lv_color_hex(0xFF5050);
    skin->ui.finder.cursor       = lv_color_hex(0x50FF50);
    skin->ui.ft8.bg              = lv_color_black();
    skin->ui.ft8.border          = lv_color_white();
    skin->ui.ft8.text            = lv_color_hex(0xC0C0C0);
    skin->ui.ft8.info            = lv_color_hex(0x303030);
    skin->ui.ft8.cq_new          = lv_color_hex(0x00DD00);
    skin->ui.ft8.cq_worked       = lv_color_hex(0x2e5a00);
    skin->ui.ft8.cq_worked_mode  = lv_color_hex(0x224400);
    skin->ui.ft8.to_me           = lv_color_hex(0xFF0000);
    skin->ui.ft8.tx              = lv_color_hex(0x0000FF);
    skin->ui.chart.grid          = lv_color_hex(0xAAAAAA);
    skin->ui.chart.label         = lv_color_white();
    skin->ui.chart.line          = lv_color_white();
    skin->ui.level               = lv_palette_main(LV_PALETTE_BLUE);

    skin->chrome.mode_bg         = lv_color_hex(0x4CAF50);
    skin->chrome.mode_text       = skin->base_text_color;
    skin->chrome.vfo_bg          = lv_color_hex(0xD32F2F);
    skin->chrome.vfo_text        = skin->base_text_color;
    skin->chrome.badge_opa       = LV_OPA_70;
    skin->chrome.bg_freq         = lv_color_hex(0xF9A851);
    skin->chrome.ind_text        = lv_color_hex(0x8A8A8A);
    skin->chrome.ind_active_bg   = lv_color_hex(0x12526F);
    skin->chrome.ind_active_text = lv_color_hex(0xFFFFFF);
    skin->chrome.ind_sep         = lv_color_hex(0x282828);
    skin->chrome.readout_bg      = lv_color_black();

    skin->tx.clock     = NULL;
    skin->tx.freq_info = NULL;
    skin->tx.tx_info   = NULL;
    skin->tx.flash     = lv_color_black();

    skin->s_meter.noise = lv_color_hex(0x777777);
    skin->s_meter.low   = lv_color_hex(0xAAAAAA);
    skin->s_meter.mid   = lv_color_hex(0xAAAA00);
    skin->s_meter.high  = lv_color_hex(0xAA0000);
    skin->s_meter.peak  = lv_color_hex(0xAAAAAA);


    /* Setup background colors (transparent by default) */
    lv_color_t bg_fill_color  = {.full = 0};

    skin->bg_color.btn = bg_fill_color;
    skin->bg_color.clock = bg_fill_color;
    skin->bg_color.freq_info = bg_fill_color;
    skin->bg_color.s_meter = bg_fill_color;
    skin->bg_color.tx_info = bg_fill_color;

    /* Setup background images */

    lv_coord_t border_width = 1;
    lv_coord_t radius       = 8;
    lv_coord_t w, h;

    // Button
    lv_color_t    btn_bg1_color     = lv_color_hex(0x4f4742);
    lv_color_t    btn_bg2_color     = lv_color_hex(0x1c150d);
    lv_color_t    btn_border1_color = lv_color_hex(0xa7a7a7);
    lv_color_t    btn_border2_color = lv_color_hex(0x1d1d1d);
    lv_grad_dsc_t btn_bg_grad       = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = btn_bg1_color, .frac = 0},
                        [1] = {.color = lv_color_mix(btn_bg1_color, btn_bg2_color, 127), .frac = 160},
                        [2] = {.color = btn_bg2_color, .frac = 255},
                        }
    };
    lv_grad_dsc_t btn_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {
                        [0] = {.color = btn_border1_color, .frac = 0},
                        [1] = {.color = btn_border2_color, .frac = 255},
                        }
    };
    if (style_get_size(&style.btn.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &btn_bg_dsc, LV_OPA_90, border_width, radius, &btn_bg_grad, &btn_border_grad);
        skin->bg_img.btn = &btn_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown btn style size");
        skin->bg_img.btn = NULL;
    }

    // Dialog

    lv_color_t dialog_bg1_color     = lv_color_hex(0x334452);
    lv_color_t dialog_bg2_color     = lv_color_hex(0x1c242b);
    lv_color_t dialog_border1_color = lv_color_hex(0x363636);
    lv_color_t dialog_border2_color = lv_color_hex(0xffffff);

    lv_grad_dsc_t dialog_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = dialog_border1_color, .frac = 0},
                        [1] = {.color = dialog_border2_color, .frac = 100},
                        [2] = {.color = dialog_border1_color, .frac = 255},
                        }
    };

    lv_grad_dsc_t dialog_bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = dialog_bg2_color, .frac = 0},
                        [1] = {.color = dialog_bg1_color, .frac = 100},
                        [2] = {.color = dialog_bg2_color, .frac = 255},
                        }
    };
    if (style_get_size(&style.dialog.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &dialog_bg_dsc, 255 * 95 / 100, border_width, radius, &dialog_bg_grad,
                                   &dialog_border_grad);
        skin->bg_img.dialog = &dialog_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown dialog style size");
        skin->bg_img.dialog = NULL;
    }

    // Panel
    skin->panel.bg_grad      = dialog_bg_grad;
    skin->panel.border_grad  = dialog_border_grad;
    skin->panel.opa          = LV_OPA_80;
    skin->panel.border_width = border_width;
    skin->panel.radius       = radius;

    // Top panels
    lv_color_t    top_block_bg1_color     = lv_color_hex(0x5f7e97);
    lv_color_t    top_block_bg2_color     = lv_color_hex(0x333333);
    lv_color_t    top_block_border1_color = lv_color_hex(0xffffff);
    lv_color_t    top_block_border2_color = lv_color_hex(0x3e3d3d);
    uint8_t       top_block_opa           = LV_OPA_60;
    lv_grad_dsc_t top_border_grad         = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {
                        [0] = {.color = top_block_border1_color, .frac = 0},
                        [1] = {.color = top_block_border2_color, .frac = 255},
                        }
    };

    lv_grad_dsc_t top_bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = top_block_bg1_color, .frac = 0},
                        [1] = {.color = lv_color_mix(top_block_bg1_color, top_block_bg2_color, 127), .frac = 160},
                        [2] = {.color = top_block_bg2_color, .frac = 255},
                        }
    };

    // Clock
    if (style_get_size(&style.clock, &w, &h)) {
        render_grad_bg_with_border(w, h, &clock_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.clock = &clock_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown clock style size");
        skin->bg_img.clock = NULL;
    }

    // Freq info
    if (style_get_size(&style.freq_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &freq_info_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.freq_info = &freq_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown freq_info style size");
        skin->bg_img.freq_info = NULL;
    }

    // Meter
    if (style_get_size(&style.s_meter, &w, &h)) {
        render_grad_bg_with_border(w, h, &s_meter_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.s_meter = &s_meter_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown s_meter style size");
        skin->bg_img.s_meter = NULL;
    }

    // TX info

    // Recalculate color to preserve value
    lv_grad_dsc_t tx_db_grad = top_bg_grad;
    for (uint8_t i = 0; i < tx_db_grad.stops_count; i++)
        tx_db_grad.stops[i].color = lv_color_darken(tx_db_grad.stops[i].color, 255-top_block_opa);
    lv_grad_dsc_t tx_border_grad = top_border_grad;
    for (uint8_t i = 0; i < tx_border_grad.stops_count; i++)
        tx_border_grad.stops[i].color = lv_color_darken(tx_border_grad.stops[i].color, 255-top_block_opa);
    if (style_get_size(&style.tx_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &tx_info_bg_dsc, LV_OPA_COVER, border_width, radius, &tx_db_grad,
                                   &tx_border_grad);
        skin->bg_img.tx_info = &tx_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown tx_info style size");
        skin->bg_img.tx_info = NULL;
    }

    lv_color_t    msg_bg1_color     = lv_color_hex(0x486175);
    lv_color_t    msg_bg2_color     = lv_color_hex(0x293e4f);
    lv_color_t    msg_border1_color = lv_color_hex(0x5d5d5d);
    lv_color_t    msg_border2_color = lv_color_hex(0xc4c4c4);
    lv_grad_dsc_t msg_grad       = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = msg_bg2_color, .frac = 0},
                        [1] = {.color = msg_bg1_color, .frac = 100},
                        [2] = {.color = msg_bg2_color, .frac = 255},
                        }
    };
    lv_grad_dsc_t msg_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = msg_border1_color, .frac = 0},
                        [1] = {.color = msg_border2_color, .frac = 100},
                        [2] = {.color = msg_border1_color, .frac = 255},
                        }
    };
    if (style_get_size(&style.msg, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_bg_dsc, LV_OPA_90, border_width, radius, &msg_grad, &msg_border_grad);
        skin->bg_img.msg = &msg_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg style size");
        skin->bg_img.msg = NULL;
    }

    /* msg_tiny */
    if (style_get_size(&style.msg_tiny, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_tiny_bg_dsc, LV_OPA_COVER, border_width, radius, &msg_grad, &msg_border_grad);
        skin->bg_img.msg_tiny = &msg_tiny_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg_tiny style size");
        skin->bg_img.msg_tiny = NULL;
    }
}

static void setup_skin_flat(skin_t *skin) {
    static lv_img_dsc_t btn_bg_dsc;

    // Copy default and override
    *skin = skin_default;

    skin->wf_middle_line_color = lv_color_hex(0xFF0000);
    skin->mark_color = lv_color_hex(0x36454F);

    lv_color_t color = lv_color_hex(0x374a58);
    color.ch.alpha = LV_OPA_90;

    skin->bg_img.btn = NULL;
    skin->bg_color.btn = color;

    skin->bg_img.s_meter = NULL;
    skin->bg_color.s_meter = color;

    skin->bg_img.tx_info = NULL;
    skin->bg_color.tx_info = color;

    skin->bg_img.freq_info = NULL;
    skin->bg_color.freq_info = color;

    skin->bg_img.clock = NULL;
    skin->bg_color.clock = color;

    // ? same images like on default
    // skin->bg_img.btn = PATH "images/dialog_dark.bin";
    // skin->bg_img.msg = PATH "images/msg_dark.bin";
    // //     lv_style_set_width(&style.btn.base, 795);
    // //     lv_style_set_height(&style.btn.base, 61);

    // skin->bg_img.clock = PATH "images/dialog_dark.bin";
    // skin->bg_img.freq_info = PATH "images/dialog_dark.bin";
    // skin->bg_img.s_meter = PATH "images/dialog_dark.bin";

    // skin->bg_img.panel = PATH "images/panel_dark.bin";
    // skin->bg_img.msg_tiny = PATH "images/msg_tiny_dark.bin";
    // skin->bg_img.dialog = PATH "images/dialog_dark.bin";
    // skin->bg_img.tx_info = PATH "images/dialog_dark.bin";
}

static void setup_skin_black(skin_t *skin) {
    static lv_img_dsc_t btn_bg_dsc;
    static lv_img_dsc_t msg_bg_dsc;
    static lv_img_dsc_t msg_tiny_bg_dsc;
    static lv_img_dsc_t clock_bg_dsc;
    static lv_img_dsc_t freq_info_bg_dsc;
    static lv_img_dsc_t s_meter_bg_dsc;
    static lv_img_dsc_t tx_info_bg_dsc;
    static lv_img_dsc_t dialog_bg_dsc;

    // Copy default and override
    *skin = skin_default;

    skin->wf_middle_line_color = lv_color_hex(0xFF0000);
    skin->mark_color           = lv_color_hex(0x36454F);

    // All: no border
    lv_coord_t w, h;
    lv_color_t c2     = lv_color_black();
    lv_color_t c1     = lv_color_lighten(c2, LV_OPA_10);
    lv_coord_t radius = 7;

    lv_grad_dsc_t bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {[0] = {.color = c1, .frac = 0}, [1] = {.color = c2, .frac = 255}}
    };

    /* Button */
    if (style_get_size(&style.btn.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &btn_bg_dsc, LV_OPA_90, 0, radius, &bg_grad, NULL);
        skin->bg_img.btn = &btn_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown btn style size");
        skin->bg_img.btn = NULL;
    }

    /* Msg */
    if (style_get_size(&style.msg, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_bg_dsc, LV_OPA_90, 0, radius, &bg_grad, NULL);
        skin->bg_img.msg = &msg_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg style size");
        skin->bg_img.msg = NULL;
    }

    /* msg_tiny */
    if (style_get_size(&style.msg_tiny, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_tiny_bg_dsc, LV_OPA_COVER, 0, radius, &bg_grad, NULL);
        skin->bg_img.msg_tiny = &msg_tiny_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg_tiny style size");
        skin->bg_img.msg_tiny = NULL;
    }

    /* panel */
    skin->panel.bg_grad      = bg_grad;
    skin->panel.border_grad  = bg_grad;
    skin->panel.opa          = LV_OPA_90;
    skin->panel.border_width = 0;
    skin->panel.radius       = radius;

    // Clock
    lv_opa_t top_block_opa = LV_OPA_80;
    if (style_get_size(&style.clock, &w, &h)) {
        render_grad_bg_with_border(w, h, &clock_bg_dsc, top_block_opa, 0, radius, &bg_grad, NULL);
        skin->bg_img.clock = &clock_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown clock style size");
        skin->bg_img.clock = NULL;
    }

    // Freq info
    if (style_get_size(&style.freq_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &freq_info_bg_dsc, top_block_opa, 0, radius, &bg_grad, NULL);
        skin->bg_img.freq_info = &freq_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown freq_info style size");
        skin->bg_img.freq_info = NULL;
    }

    // Meter
    if (style_get_size(&style.s_meter, &w, &h)) {
        render_grad_bg_with_border(w, h, &s_meter_bg_dsc, top_block_opa, 0, radius, &bg_grad, NULL);
        skin->bg_img.s_meter = &s_meter_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown s_meter style size");
        skin->bg_img.s_meter = NULL;
    }

    // TX info
    if (style_get_size(&style.tx_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &tx_info_bg_dsc, top_block_opa, 0, radius, &bg_grad, NULL);
        skin->bg_img.tx_info = &tx_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown tx_info style size");
        skin->bg_img.tx_info = NULL;
    }

    // Dialog
    bg_grad.stops_count = 3;
    bg_grad.stops[0] = (lv_gradient_stop_t){.color=c2, .frac=0};
    bg_grad.stops[1] = (lv_gradient_stop_t){.color=c1, .frac=15};
    bg_grad.stops[2] = (lv_gradient_stop_t){.color=c2, .frac=255};
    if (style_get_size(&style.dialog.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &dialog_bg_dsc, 255 * 95 / 100, 0, radius, &bg_grad, NULL);
        skin->bg_img.dialog = &dialog_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown dialog style size");
        skin->bg_img.dialog = NULL;
    }

    // skin->bg_img.dialog = PATH "images/dialog_black.bin";
}

/*
 * Lightning: storm-cloud navy surfaces with electric-yellow and cyan edges,
 * after the EAS Station "Lightning" web theme.
 */
static void setup_skin_lightning(skin_t *skin) {
    static lv_img_dsc_t btn_bg_dsc;
    static lv_img_dsc_t msg_bg_dsc;
    static lv_img_dsc_t msg_tiny_bg_dsc;
    static lv_img_dsc_t clock_bg_dsc;
    static lv_img_dsc_t freq_info_bg_dsc;
    static lv_img_dsc_t s_meter_bg_dsc;
    static lv_img_dsc_t tx_info_bg_dsc;
    static lv_img_dsc_t dialog_bg_dsc;
    static lv_img_dsc_t clock_tx_dsc;
    static lv_img_dsc_t freq_info_tx_dsc;
    static lv_img_dsc_t tx_info_tx_dsc;

    // Copy default and override
    *skin = skin_default;

    lv_color_t night   = lv_color_hex(0x060d1f);
    lv_color_t deep    = lv_color_hex(0x0d1b2a);
    lv_color_t storm   = lv_color_hex(0x14224d);
    lv_color_t cloud   = lv_color_hex(0x1c2e5e);
    lv_color_t haze    = lv_color_hex(0x5973a8);
    lv_color_t bolt    = lv_color_hex(0xffeb3b);
    lv_color_t spark   = lv_color_hex(0x40c4ff);

    skin->wf_palette      = wf_palette_lightning;
    skin->base_text_color = lv_color_white();
    skin->spectrum_color  = bolt;
    skin->mark_color      = lv_color_hex(0x24407a);

    skin->wf_middle_line_color     = bolt;
    skin->wf_middle_line_min_width = 2;

    skin->accent_color       = spark;
    skin->accent_opa         = LV_OPA_40;
    skin->focus_border_color = bolt;
    skin->focus_fill         = true;
    skin->focus_bg_color     = bolt;
    skin->edited_bg_color    = lv_color_hex(0xfff176);

    skin->s_meter.noise = haze;
    skin->s_meter.low   = spark;
    skin->s_meter.mid   = bolt;
    skin->s_meter.high  = lv_color_hex(0xff5252);
    skin->s_meter.peak  = lv_color_hex(0xfff176);

    lv_color_t ink   = lv_color_hex(0x0d1b2a);   // dark text on bright fills
    lv_color_t alarm = lv_color_hex(0xff5252);

    skin->ui.spectrum_glow      = bolt;
    skin->ui.spectrum_glow_opa  = LV_OPA_30;
    skin->ui.spectrum_center    = lv_color_hex(0xfff176);
    skin->ui.freq_step_mark     = bolt;
    skin->ui.tx_pwr             = spark;
    skin->ui.swr_ok             = lv_color_hex(0x00e676);
    skin->ui.swr_ok_gray        = spark;
    skin->ui.swr_warn           = bolt;
    skin->ui.swr_bad            = alarm;
    skin->ui.finder.filter      = bolt;
    skin->ui.finder.cursor      = spark;
    skin->ui.ft8.bg             = night;
    skin->ui.ft8.border         = haze;
    skin->ui.ft8.text           = lv_color_hex(0xeef4ff);
    skin->ui.ft8.info           = storm;
    skin->ui.ft8.cq_new         = lv_color_hex(0x00c853);
    skin->ui.ft8.cq_worked      = lv_color_hex(0x1b5e20);
    skin->ui.ft8.cq_worked_mode = lv_color_hex(0x123d17);
    skin->ui.ft8.to_me          = lv_color_hex(0xd50000);
    skin->ui.ft8.tx             = lv_color_hex(0x0091ea);
    skin->ui.chart.grid         = haze;
    skin->ui.chart.label        = lv_color_hex(0xeef4ff);
    skin->ui.chart.line         = bolt;
    skin->ui.level              = spark;

    skin->chrome.mode_bg         = spark;
    skin->chrome.mode_text       = ink;
    skin->chrome.vfo_bg          = bolt;
    skin->chrome.vfo_text        = ink;
    skin->chrome.badge_opa       = LV_OPA_COVER;
    skin->chrome.bg_freq         = lv_color_hex(0x80d8ff);
    skin->chrome.ind_text        = lv_color_hex(0x8fa3c8);
    skin->chrome.ind_active_bg   = spark;
    skin->chrome.ind_active_text = ink;
    skin->chrome.ind_sep         = lv_color_hex(0x24365f);
    skin->chrome.readout_bg      = deep;

    lv_coord_t w, h;
    lv_coord_t border_width = 2;
    lv_coord_t radius       = 10;

    /* Buttons: bolt-lit top edge fading into the storm */
    lv_grad_dsc_t btn_bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {
                        [0] = {.color = cloud, .frac = 0},
                        [1] = {.color = deep, .frac = 255},
                        }
    };
    lv_grad_dsc_t btn_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = bolt, .frac = 0},
                        [1] = {.color = haze, .frac = 70},
                        [2] = {.color = storm, .frac = 255},
                        }
    };
    if (style_get_size(&style.btn.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &btn_bg_dsc, LV_OPA_90, border_width, radius, &btn_bg_grad,
                                   &btn_border_grad);
        skin->bg_img.btn = &btn_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown btn style size");
        skin->bg_img.btn = NULL;
    }

    /* Top blocks: cyan spark along the top edge */
    lv_grad_dsc_t top_bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {
                        [0] = {.color = cloud, .frac = 0},
                        [1] = {.color = night, .frac = 255},
                        }
    };
    lv_grad_dsc_t top_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = spark, .frac = 0},
                        [1] = {.color = haze, .frac = 90},
                        [2] = {.color = storm, .frac = 255},
                        }
    };
    lv_opa_t top_block_opa = LV_OPA_80;

    if (style_get_size(&style.clock, &w, &h)) {
        render_grad_bg_with_border(w, h, &clock_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.clock = &clock_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown clock style size");
        skin->bg_img.clock = NULL;
    }

    if (style_get_size(&style.s_meter, &w, &h)) {
        render_grad_bg_with_border(w, h, &s_meter_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.s_meter = &s_meter_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown s_meter style size");
        skin->bg_img.s_meter = NULL;
    }

    if (style_get_size(&style.tx_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &tx_info_bg_dsc, LV_OPA_COVER, border_width, radius, &top_bg_grad,
                                   &top_border_grad);
        skin->bg_img.tx_info = &tx_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown tx_info style size");
        skin->bg_img.tx_info = NULL;
    }

    /* On the air: the same blocks with a red-hot edge */
    lv_grad_dsc_t tx_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = alarm, .frac = 0},
                        [1] = {.color = lv_color_hex(0xb71c1c), .frac = 110},
                        [2] = {.color = lv_color_hex(0x4a1020), .frac = 255},
                        }
    };
    lv_grad_dsc_t tx_bg_grad = top_bg_grad;
    tx_bg_grad.stops[0].color = lv_color_hex(0x3a1c3e);
    skin->tx.flash = alarm;

    if (style_get_size(&style.clock, &w, &h)) {
        render_grad_bg_with_border(w, h, &clock_tx_dsc, top_block_opa, border_width, radius, &tx_bg_grad,
                                   &tx_border_grad);
        skin->tx.clock = &clock_tx_dsc;
    }
    if (style_get_size(&style.freq_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &freq_info_tx_dsc, top_block_opa, border_width, radius, &tx_bg_grad,
                                   &tx_border_grad);
        skin->tx.freq_info = &freq_info_tx_dsc;
    }
    if (style_get_size(&style.tx_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &tx_info_tx_dsc, LV_OPA_COVER, border_width, radius, &tx_bg_grad,
                                   &tx_border_grad);
        skin->tx.tx_info = &tx_info_tx_dsc;
    }

    /* Frequency gets the yellow bolt: it's the headline number */
    lv_grad_dsc_t freq_border_grad = btn_border_grad;
    if (style_get_size(&style.freq_info, &w, &h)) {
        render_grad_bg_with_border(w, h, &freq_info_bg_dsc, top_block_opa, border_width, radius, &top_bg_grad,
                                   &freq_border_grad);
        skin->bg_img.freq_info = &freq_info_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown freq_info style size");
        skin->bg_img.freq_info = NULL;
    }

    /* Dialogs and panels: storm surface, bolt flash on the edges */
    lv_grad_dsc_t dialog_bg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = cloud, .frac = 0},
                        [1] = {.color = storm, .frac = 60},
                        [2] = {.color = night, .frac = 255},
                        }
    };
    lv_grad_dsc_t dialog_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = bolt, .frac = 0},
                        [1] = {.color = haze, .frac = 40},
                        [2] = {.color = storm, .frac = 255},
                        }
    };
    if (style_get_size(&style.dialog.base, &w, &h)) {
        render_grad_bg_with_border(w, h, &dialog_bg_dsc, 255 * 95 / 100, border_width, radius, &dialog_bg_grad,
                                   &dialog_border_grad);
        skin->bg_img.dialog = &dialog_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown dialog style size");
        skin->bg_img.dialog = NULL;
    }

    skin->panel.bg_grad      = dialog_bg_grad;
    skin->panel.border_grad  = dialog_border_grad;
    skin->panel.opa          = LV_OPA_90;
    skin->panel.border_width = border_width;
    skin->panel.radius       = radius;

    /* Messages: bolt rim all round */
    lv_grad_dsc_t msg_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 2,
        .stops       = {
                        [0] = {.color = cloud, .frac = 0},
                        [1] = {.color = storm, .frac = 255},
                        }
    };
    lv_grad_dsc_t msg_border_grad = {
        .dir         = LV_GRAD_DIR_VER,
        .stops_count = 3,
        .stops       = {
                        [0] = {.color = bolt, .frac = 0},
                        [1] = {.color = lv_color_hex(0xfbc02d), .frac = 128},
                        [2] = {.color = bolt, .frac = 255},
                        }
    };
    if (style_get_size(&style.msg, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_bg_dsc, LV_OPA_90, border_width, radius, &msg_grad, &msg_border_grad);
        skin->bg_img.msg = &msg_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg style size");
        skin->bg_img.msg = NULL;
    }

    if (style_get_size(&style.msg_tiny, &w, &h)) {
        render_grad_bg_with_border(w, h, &msg_tiny_bg_dsc, LV_OPA_COVER, border_width, radius, &msg_grad,
                                   &msg_border_grad);
        skin->bg_img.msg_tiny = &msg_tiny_bg_dsc;
    } else {
        LV_LOG_ERROR("Unknown msg_tiny style size");
        skin->bg_img.msg_tiny = NULL;
    }
}

static void set_skin(skin_t *skin) {
    style.wf_palette = skin->wf_palette;
    style.colors.mark = skin->mark_color;

    /* Text colors */
    lv_color_t muted_text_color;
    if (lv_color_brightness(skin->base_text_color) > 64) {
        // Bright color, muted should be darker
        muted_text_color = lv_color_darken(skin->base_text_color, LV_OPA_50);
    } else {
        muted_text_color = lv_color_lighten(skin->base_text_color, LV_OPA_50);
    }
    colors.base_text_color = skin->base_text_color;

    lv_style_set_text_color(&style.text_base_color, skin->base_text_color);
    lv_style_set_text_color(&style.btn.base, skin->base_text_color);
    lv_style_set_text_color(&style.freq_bounds, lv_color_darken(skin->base_text_color, LV_OPA_30));
    lv_style_set_text_color(&style.msg, skin->base_text_color);
    lv_style_set_text_color(&style.msg_tiny, skin->base_text_color);
    lv_style_set_text_color(&style.panels.base, skin->base_text_color);
    lv_style_set_text_color(&style.dialog.base, skin->base_text_color);
    lv_style_set_text_color(&style.dialog.item, skin->base_text_color);
    lv_style_set_text_color(&style.clock, skin->base_text_color);
    lv_style_set_text_color(&style.knobs, skin->base_text_color);
    lv_style_set_text_color(&style.rgb.letter, skin->base_text_color);
    lv_style_set_text_color(&style.rgb.val_label, skin->base_text_color);

    lv_style_set_text_color(&style.text_muted_color, muted_text_color);

    /* Accent */
    lv_style_set_bg_img_recolor(&style.btn.active, skin->accent_color);
    lv_style_set_bg_img_recolor_opa(&style.btn.active, skin->accent_opa);
    lv_style_set_border_color(&style.dialog.item_focus, skin->focus_border_color);
    if (skin->focus_fill) {
        lv_style_set_bg_opa(&style.dialog.item_focus, LV_OPA_90);
        lv_style_set_bg_color(&style.dialog.item_focus, skin->focus_bg_color);
        lv_style_set_bg_color(&style.dialog.item_edited, skin->edited_bg_color);
    } else {
        lv_style_set_bg_opa(&style.dialog.item_focus, 128);
        lv_style_remove_prop(&style.dialog.item_focus, LV_STYLE_BG_COLOR);
        lv_style_remove_prop(&style.dialog.item_edited, LV_STYLE_BG_COLOR);
    }

    /* Spectrum */
    update_spectrum_color(skin);

    /* S-meter */
    styles_update_meter_colors((meter_color_t)param_i_get(cfg.ui.meter_color()));

    /* Waterfall */
    lv_style_set_line_color(&style.waterfall_middle_line, skin->wf_middle_line_color);
    lv_style_set_line_width(&style.waterfall_middle_line, skin->wf_middle_line_min_width);

    /* Background images */
    lv_style_set_bg_img_src(&style.btn.base, skin->bg_img.btn);
    lv_style_set_bg_img_src(&style.s_meter, skin->bg_img.s_meter);
    lv_style_set_bg_img_src(&style.tx_info, skin->bg_img.tx_info);
    lv_style_set_bg_img_src(&style.freq_info, skin->bg_img.freq_info);
    lv_style_set_bg_img_src(&style.clock, skin->bg_img.clock);
    render_panel_bg(skin, panel_bg_w, panel_bg_h);
    lv_style_set_bg_img_src(&style.panels.base, &panel_bg_dsc);
    lv_style_set_bg_img_src(&style.msg, skin->bg_img.msg);
    lv_style_set_bg_img_src(&style.msg_tiny, skin->bg_img.msg_tiny);
    lv_style_set_bg_img_src(&style.dialog.base, skin->bg_img.dialog);

    /* Background colors */
    lv_style_set_bg_opa(&style.btn.base, skin->bg_color.btn.ch.alpha);
    lv_style_set_bg_color(&style.btn.base, skin->bg_color.btn);
    lv_style_set_bg_opa(&style.s_meter, skin->bg_color.s_meter.ch.alpha);
    lv_style_set_bg_color(&style.s_meter, skin->bg_color.s_meter);
    lv_style_set_bg_opa(&style.tx_info, skin->bg_color.tx_info.ch.alpha);
    lv_style_set_bg_color(&style.tx_info, skin->bg_color.tx_info);
    lv_style_set_bg_opa(&style.freq_info, skin->bg_color.freq_info.ch.alpha);
    lv_style_set_bg_color(&style.freq_info, skin->bg_color.freq_info);
    lv_style_set_bg_opa(&style.clock, skin->bg_color.clock.ch.alpha);
    lv_style_set_bg_color(&style.clock, skin->bg_color.clock);

    /* Widget colours */
    style.colors.ui = skin->ui;

    lv_style_set_bg_color(&style.badge_mode, skin->chrome.mode_bg);
    lv_style_set_bg_opa(&style.badge_mode, skin->chrome.badge_opa);
    lv_style_set_text_color(&style.badge_mode, skin->chrome.mode_text);
    lv_style_set_bg_color(&style.badge_vfo, skin->chrome.vfo_bg);
    lv_style_set_bg_opa(&style.badge_vfo, skin->chrome.badge_opa);
    lv_style_set_text_color(&style.badge_vfo, skin->chrome.vfo_text);
    lv_style_set_text_color(&style.bg_freq, skin->chrome.bg_freq);
    lv_style_set_text_color(&style.ind_text, skin->chrome.ind_text);
    lv_style_set_bg_color(&style.ind_active, skin->chrome.ind_active_bg);
    lv_style_set_text_color(&style.ind_active, skin->chrome.ind_active_text);
    lv_style_set_bg_color(&style.ind_sep, skin->chrome.ind_sep);
    lv_style_set_bg_color(&style.knobs, skin->chrome.readout_bg);
    lv_style_set_bg_color(&style.freq_bounds, skin->chrome.readout_bg);

    apply_tx_art(skin);

    lv_obj_report_style_change(NULL);
    lv_obj_invalidate(lv_scr_act());
}

/* Top-block artwork for the current TX state */
static void apply_tx_art(skin_t *skin) {
    bool tx = tx_active && skin->tx.clock;

    lv_style_set_bg_img_src(&style.clock, tx ? skin->tx.clock : skin->bg_img.clock);
    lv_style_set_bg_img_src(&style.freq_info, tx ? skin->tx.freq_info : skin->bg_img.freq_info);
    lv_style_set_bg_img_src(&style.tx_info, tx ? skin->tx.tx_info : skin->bg_img.tx_info);
}

static void tx_flash_cb(void *obj, int32_t v) {
    lv_obj_set_style_bg_img_recolor_opa(obj, (lv_opa_t)v, LV_PART_MAIN);
}

void styles_tx_flash_add(lv_obj_t *obj) {
    for (int i = 0; i < 3; i++) {
        if (!tx_targets[i]) {
            tx_targets[i] = obj;
            return;
        }
    }
    LV_LOG_WARN("No room for another TX flash target");
}

void styles_set_tx(bool tx) {
    tx_active = tx;

    if (!skin_current || !skin_current->tx.clock) {
        return;
    }
    apply_tx_art(skin_current);
    lv_obj_report_style_change(&style.clock);
    lv_obj_report_style_change(&style.freq_info);
    lv_obj_report_style_change(&style.tx_info);

    /* Key-down: one bright flash that fades out */
    for (int i = 0; i < 3; i++) {
        lv_obj_t *obj = tx_targets[i];
        if (!obj) {
            continue;
        }
        lv_anim_del(obj, tx_flash_cb);
        lv_obj_set_style_bg_img_recolor(obj, skin_current->tx.flash, LV_PART_MAIN);
        if (tx) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, obj);
            lv_anim_set_exec_cb(&a, tx_flash_cb);
            lv_anim_set_values(&a, LV_OPA_50, LV_OPA_TRANSP);
            lv_anim_set_time(&a, 450);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_start(&a);
        } else {
            tx_flash_cb(obj, LV_OPA_TRANSP);
        }
    }
}
