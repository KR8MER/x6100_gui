/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Start-up splash: KR8MER Lightning Edition, in the EAS Station style.
 */

#include "splash.h"

#include "globals.h"
#include "main.h"
#include "styles.h"

#define SPLASH_HOLD_MS 2200
#define SPLASH_FADE_MS 450

#define BOLT_W 96
#define BOLT_H 140

static lv_obj_t     *splash;

/* Lightning bolt in a BOLT_W x BOLT_H box, as two convex halves:
 * LVGL fills convex polygons only. */
static const lv_point_t bolt_upper[] = {{60, 4}, {84, 4}, {58, 60}, {42, 80}, {8, 80}};
static const lv_point_t bolt_lower[] = {{58, 60}, {90, 60}, {24, 138}, {42, 80}};

static void draw_shape(lv_draw_ctx_t *ctx, const lv_area_t *box, const lv_point_t *shape, uint32_t n,
                       lv_coord_t dx, lv_coord_t dy, const lv_draw_rect_dsc_t *dsc) {
    lv_point_t pts[8];
    for (uint32_t i = 0; i < n; i++) {
        pts[i].x = box->x1 + shape[i].x + dx;
        pts[i].y = box->y1 + shape[i].y + dy;
    }
    lv_draw_polygon(ctx, dsc, pts, n);
}

static void bolt_draw_cb(lv_event_t *e) {
    lv_obj_t      *obj = lv_event_get_target(e);
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    lv_area_t      box;
    lv_obj_get_coords(obj, &box);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);

    /* Glow: the same shape, faint, nudged around the bolt */
    static const lv_point_t nudge[] = {{-4, 0}, {4, 0}, {0, -4}, {0, 4}};

    dsc.bg_color = lv_color_hex(0x40c4ff);
    dsc.bg_opa   = LV_OPA_20;
    for (uint32_t k = 0; k < 4; k++) {
        draw_shape(ctx, &box, bolt_upper, 5, nudge[k].x, nudge[k].y, &dsc);
        draw_shape(ctx, &box, bolt_lower, 4, nudge[k].x, nudge[k].y, &dsc);
    }

    dsc.bg_color = lv_color_hex(0xffeb3b);
    dsc.bg_opa   = LV_OPA_COVER;
    draw_shape(ctx, &box, bolt_upper, 5, 0, 0, &dsc);
    draw_shape(ctx, &box, bolt_lower, 4, 0, 0, &dsc);
}

static void flicker_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void fade_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void fade_done_cb(lv_anim_t *a) {
    lv_obj_del(splash);
    splash = NULL;
}

static lv_obj_t *label(lv_obj_t *parent, const char *text, const lv_font_t *font, lv_color_t color,
                       lv_coord_t spacing) {
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, color, 0);
    lv_obj_set_style_text_letter_space(l, spacing, 0);
    return l;
}

void splash_show(void) {
    if (splash) {
        return;
    }

    splash = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(splash);
    lv_obj_set_size(splash, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(splash, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(splash, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(splash, lv_color_hex(0x14224d), 0);
    lv_obj_set_style_bg_grad_color(splash, lv_color_hex(0x060d1f), 0);
    lv_obj_set_style_bg_grad_dir(splash, LV_GRAD_DIR_VER, 0);

    /* Wordmark row: bolt + call sign */
    lv_obj_t *row = lv_obj_create(splash);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 18, 0);
    lv_obj_align(row, LV_ALIGN_CENTER, 0, -60);

    lv_obj_t *bolt = lv_obj_create(row);
    lv_obj_remove_style_all(bolt);
    lv_obj_set_size(bolt, BOLT_W, BOLT_H);
    lv_obj_add_event_cb(bolt, bolt_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    /* One flicker, like the bolts on the EAS Station Lightning theme */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bolt);
    lv_anim_set_exec_cb(&a, flicker_cb);
    lv_anim_set_values(&a, LV_OPA_40, LV_OPA_COVER);
    lv_anim_set_time(&a, 160);
    lv_anim_set_delay(&a, 700);
    lv_anim_set_repeat_count(&a, 2);
    lv_anim_start(&a);

    label(row, BRAND_CALL, &sony_60, lv_color_white(), 6);

    /* Yellow-to-cyan rule, as on the EAS Station card headers */
    lv_obj_t *rule = lv_obj_create(splash);
    lv_obj_remove_style_all(rule);
    lv_obj_set_size(rule, 420, 6);
    lv_obj_set_style_radius(rule, 3, 0);
    lv_obj_set_style_bg_opa(rule, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(rule, lv_color_hex(0xffeb3b), 0);
    lv_obj_set_style_bg_grad_color(rule, lv_color_hex(0x40c4ff), 0);
    lv_obj_set_style_bg_grad_dir(rule, LV_GRAD_DIR_HOR, 0);
    lv_obj_align(rule, LV_ALIGN_CENTER, 0, 30);

    lv_obj_t *ed = label(splash, "X6100   " BRAND_EDITION_CAPS, &sony_30, lv_color_hex(0x40c4ff), 3);
    lv_obj_align(ed, LV_ALIGN_CENTER, 0, 70);

    lv_obj_t *credit = label(splash, VERSION "  -  built on the X6100 GUI by R1CBU and OpenX6100",
                             &sony_20, lv_color_hex(0x8fa3c8), 0);
    lv_obj_align(credit, LV_ALIGN_BOTTOM_MID, 0, -18);

    /* Hold, then fade out and clean up */
    lv_anim_t f;
    lv_anim_init(&f);
    lv_anim_set_var(&f, splash);
    lv_anim_set_exec_cb(&f, fade_cb);
    lv_anim_set_values(&f, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&f, SPLASH_FADE_MS);
    lv_anim_set_delay(&f, SPLASH_HOLD_MS);
    lv_anim_set_ready_cb(&f, fade_done_cb);
    lv_anim_start(&f);
}
