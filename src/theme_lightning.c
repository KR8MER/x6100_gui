/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Lightning look for the stock LVGL widgets, layered on the default theme.
 */

#include "theme_lightning.h"

#include "lvgl/lvgl.h"

#define C_NIGHT lv_color_hex(0x060d1f)
#define C_DEEP  lv_color_hex(0x0d1b2a)
#define C_STORM lv_color_hex(0x14224d)
#define C_CLOUD lv_color_hex(0x1c2e5e)
#define C_HAZE  lv_color_hex(0x5973a8)
#define C_BOLT  lv_color_hex(0xffeb3b)
#define C_SPARK lv_color_hex(0x40c4ff)
#define C_INK   lv_color_hex(0x0d1b2a)

static lv_theme_t  theme;
static lv_theme_t *base;
static bool        ready;

static struct {
    lv_style_t panel;        // dropdown list, keyboard, text field
    lv_style_t selected;     // dropdown list selection
    lv_style_t key;          // keyboard keys
    lv_style_t key_checked;
    lv_style_t key_pressed;
    lv_style_t track;        // switch / slider track
    lv_style_t fill;         // switch / slider indicator
    lv_style_t knob;
    lv_style_t cursor;
} s;

static void init_styles(void) {
    lv_style_init(&s.panel);
    lv_style_set_bg_opa(&s.panel, LV_OPA_COVER);
    lv_style_set_bg_color(&s.panel, C_CLOUD);
    lv_style_set_bg_grad_color(&s.panel, C_NIGHT);
    lv_style_set_bg_grad_dir(&s.panel, LV_GRAD_DIR_VER);
    lv_style_set_border_color(&s.panel, C_HAZE);
    lv_style_set_border_width(&s.panel, 2);
    lv_style_set_radius(&s.panel, 10);
    lv_style_set_text_color(&s.panel, lv_color_white());

    lv_style_init(&s.selected);
    lv_style_set_bg_opa(&s.selected, LV_OPA_COVER);
    lv_style_set_bg_color(&s.selected, C_BOLT);
    lv_style_set_bg_grad_color(&s.selected, C_SPARK);
    lv_style_set_bg_grad_dir(&s.selected, LV_GRAD_DIR_HOR);
    lv_style_set_text_color(&s.selected, C_INK);

    lv_style_init(&s.key);
    lv_style_set_bg_opa(&s.key, LV_OPA_COVER);
    lv_style_set_bg_color(&s.key, lv_color_hex(0x2a4278));
    lv_style_set_bg_grad_color(&s.key, C_STORM);
    lv_style_set_bg_grad_dir(&s.key, LV_GRAD_DIR_VER);
    lv_style_set_border_color(&s.key, C_HAZE);
    lv_style_set_border_width(&s.key, 1);
    lv_style_set_border_opa(&s.key, LV_OPA_70);
    lv_style_set_radius(&s.key, 8);
    lv_style_set_text_color(&s.key, lv_color_white());

    lv_style_init(&s.key_checked);
    lv_style_set_bg_color(&s.key_checked, C_BOLT);
    lv_style_set_bg_grad_color(&s.key_checked, lv_color_hex(0xfbc02d));
    lv_style_set_text_color(&s.key_checked, C_INK);

    lv_style_init(&s.key_pressed);
    lv_style_set_bg_color(&s.key_pressed, C_SPARK);
    lv_style_set_bg_grad_color(&s.key_pressed, lv_color_hex(0x0091ea));
    lv_style_set_text_color(&s.key_pressed, C_INK);

    lv_style_init(&s.track);
    lv_style_set_bg_opa(&s.track, LV_OPA_COVER);
    lv_style_set_bg_color(&s.track, C_STORM);
    lv_style_set_border_color(&s.track, C_HAZE);
    lv_style_set_border_width(&s.track, 1);

    lv_style_init(&s.fill);
    lv_style_set_bg_opa(&s.fill, LV_OPA_COVER);
    lv_style_set_bg_color(&s.fill, C_SPARK);
    lv_style_set_bg_grad_color(&s.fill, C_BOLT);
    lv_style_set_bg_grad_dir(&s.fill, LV_GRAD_DIR_HOR);

    lv_style_init(&s.knob);
    lv_style_set_bg_opa(&s.knob, LV_OPA_COVER);
    lv_style_set_bg_color(&s.knob, lv_color_white());
    lv_style_set_border_color(&s.knob, C_BOLT);
    lv_style_set_border_width(&s.knob, 2);
    lv_style_set_shadow_width(&s.knob, 0);

    lv_style_init(&s.cursor);
    lv_style_set_border_color(&s.cursor, C_BOLT);
    lv_style_set_bg_color(&s.cursor, C_BOLT);
}

static void apply_cb(lv_theme_t *th, lv_obj_t *obj) {
    LV_UNUSED(th);

    if (lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &s.panel, 0);
        lv_obj_add_style(obj, &s.selected, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &s.selected, LV_PART_SELECTED | LV_STATE_PRESSED);
    } else if (lv_obj_check_type(obj, &lv_switch_class)) {
        lv_obj_add_style(obj, &s.track, 0);
        lv_obj_add_style(obj, &s.fill, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &s.knob, LV_PART_KNOB);
    } else if (lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_add_style(obj, &s.track, 0);
        lv_obj_add_style(obj, &s.fill, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &s.knob, LV_PART_KNOB);
    } else if (lv_obj_check_type(obj, &lv_keyboard_class)) {
        lv_obj_add_style(obj, &s.panel, 0);
        lv_obj_add_style(obj, &s.key, LV_PART_ITEMS);
        lv_obj_add_style(obj, &s.key_checked, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &s.key_pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
    } else if (lv_obj_check_type(obj, &lv_textarea_class)) {
        lv_obj_add_style(obj, &s.panel, 0);
        lv_obj_add_style(obj, &s.cursor, LV_PART_CURSOR | LV_STATE_FOCUSED);
    }
}

void theme_lightning_enable(bool on) {
    if (!ready) {
        base = lv_disp_get_theme(NULL);
        if (!base) {
            return;
        }
        init_styles();
        theme = *base;
        lv_theme_set_parent(&theme, base);
        lv_theme_set_apply_cb(&theme, apply_cb);
        ready = true;
    }
    lv_disp_set_theme(NULL, on ? &theme : base);
}
