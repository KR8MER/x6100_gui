/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Start-up splash: KR8MER Lightning Edition, in the EAS Station style.
 *
 *  Timeline (ms): a forked bolt strikes the left end of the accent rule
 *  (350), a spark of charge runs along it (750), the operator is greeted (1500),
 *  a second bolt strikes the end of the rule (2250), then the splash fades.
 *  Every bolt is generated afresh, so no two start-ups look the same.
 */

#include "splash.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cfg/cfg_api.h"
#include "globals.h"
#include "main.h"
#include "styles.h"
#include "tz_display.h"

#define SPLASH_HOLD_MS  2750
#define SPLASH_FADE_MS  450

#define STRIKE1_MS      350
#define SPARK_MS        750
#define SPARK_TIME      700
#define GREET_MS        1500
#define STRIKE2_MS      2250
#define STRIKE_TIME     450

#define BOLT_W 96
#define BOLT_H 140

/* Main channel: 2^5 segments; branches: 2^3 */
#define MAIN_PTS        33
#define BRANCH_PTS      9
#define MAX_BRANCHES    3

#define GLOW_W          18

/* The boot logo (KR8MER Buildroot) as an 8-bit indexed LVGL image body:
 * 256 x ARGB8888 palette followed by one index per pixel. */
#ifndef LOGO_PATH
#define LOGO_PATH  "/usr/share/x6100/images/splash_logo.bin"
#endif
#define LOGO_SIZE  (256 * 4 + SCREEN_WIDTH * SCREEN_HEIGHT)

typedef struct {
    lv_point_t main[MAIN_PTS];
    lv_point_t branch[MAX_BRANCHES][BRANCH_PTS];
    uint8_t    n_branch;
    lv_opa_t   level;
    lv_area_t  bbox;
} strike_t;

/* Where the bolts land and the spark runs, for the logo and the fallback */
typedef struct {
    lv_coord_t rule_x1, rule_x2, rule_y;
} splash_geom_t;

static const splash_geom_t geom_logo     = {162, 637, 271};
static const splash_geom_t geom_fallback = {190, 610, 273};

static lv_obj_t     *splash;
static lv_obj_t     *flash;
static lv_img_dsc_t  logo_dsc;
static uint8_t      *logo_data;
static strike_t      strikes[2];
static int32_t       spark_pos = -1;   /* 0..1000 along the rule, -1 hidden */
static splash_geom_t geom;
static uint32_t      rng;

/* Lightning bolt icon in a BOLT_W x BOLT_H box, as two convex halves:
 * LVGL fills convex polygons only. */
static const lv_point_t bolt_upper[] = {{60, 4}, {84, 4}, {58, 60}, {42, 80}, {8, 80}};
static const lv_point_t bolt_lower[] = {{58, 60}, {90, 60}, {24, 138}, {42, 80}};

/* Own generator, so the splash never disturbs anyone's rand() sequence */
static int32_t rnd(int32_t lo, int32_t hi) {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return lo + (int32_t)(rng % (uint32_t)(hi - lo + 1));
}

/* Midpoint displacement: n = 2^k + 1 points from p[0] to p[n-1], each
 * midpoint pushed sideways by up to rough% of its segment's length */
static void bolt_path(lv_point_t *p, int n, int rough) {
    for (int step = n - 1; step > 1; step /= 2) {
        int half = step / 2;
        for (int i = 0; i + step < n; i += step) {
            float dx  = p[i + step].x - p[i].x;
            float dy  = p[i + step].y - p[i].y;
            float len = sqrtf(dx * dx + dy * dy);
            float off = len * rnd(-rough, rough) / 100.0f;
            if (len < 1.0f) {
                len = 1.0f;
            }
            p[i + half].x = (p[i].x + p[i + step].x) / 2 - (lv_coord_t)(dy * off / len);
            p[i + half].y = (p[i].y + p[i + step].y) / 2 + (lv_coord_t)(dx * off / len);
        }
    }
}

static void bbox_add(lv_area_t *a, const lv_point_t *p, int n) {
    for (int i = 0; i < n; i++) {
        a->x1 = LV_MIN(a->x1, p[i].x - GLOW_W);
        a->y1 = LV_MIN(a->y1, p[i].y - GLOW_W);
        a->x2 = LV_MAX(a->x2, p[i].x + GLOW_W);
        a->y2 = LV_MAX(a->y2, p[i].y + GLOW_W);
    }
}

static void strike_make(strike_t *s, lv_point_t target, lv_coord_t x_lo, lv_coord_t x_hi) {
    lv_point_t *m = s->main;

    m[0].x = rnd(x_lo, x_hi);
    m[0].y = -10;
    m[MAIN_PTS - 1] = target;
    bolt_path(m, MAIN_PTS, 22);

    s->bbox.x1 = s->bbox.y1 = LV_COORD_MAX;
    s->bbox.x2 = s->bbox.y2 = LV_COORD_MIN;
    bbox_add(&s->bbox, m, MAIN_PTS);

    /* Forks peel off the upper two thirds, angled away from the channel */
    s->n_branch = rnd(2, MAX_BRANCHES);
    for (int b = 0; b < s->n_branch; b++) {
        lv_point_t *br   = s->branch[b];
        int         from = rnd(4, MAIN_PTS * 2 / 3);
        float       dx   = m[from + 2].x - m[from].x;
        float       dy   = m[from + 2].y - m[from].y;
        float       ang  = atan2f(dy, dx) + (rnd(0, 1) ? 1 : -1) * rnd(25, 50) * 0.01745f;
        float       len  = rnd(50, 120);

        br[0]              = m[from];
        br[BRANCH_PTS - 1].x = m[from].x + (lv_coord_t)(cosf(ang) * len);
        br[BRANCH_PTS - 1].y = m[from].y + (lv_coord_t)(sinf(ang) * len);
        bolt_path(br, BRANCH_PTS, 30);
        bbox_add(&s->bbox, br, BRANCH_PTS);
    }
    s->level = 0;
}

static void draw_polyline(lv_draw_ctx_t *ctx, const lv_point_t *p, int n, lv_coord_t w, lv_color_t c,
                          lv_opa_t opa) {
    if (opa <= LV_OPA_MIN) {
        return;
    }
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.width       = w;
    dsc.color       = c;
    dsc.opa         = opa;
    dsc.round_start = 1;
    dsc.round_end   = 1;
    for (int i = 0; i + 1 < n; i++) {
        lv_draw_line(ctx, &dsc, &p[i], &p[i + 1]);
    }
}

static void strike_draw_cb(lv_event_t *e) {
    strike_t      *s   = lv_event_get_user_data(e);
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);
    uint16_t       k   = s->level;

    if (!k) {
        return;
    }
    /* Wide cyan aura, pale-blue body, white-hot core; forks a little dimmer */
    draw_polyline(ctx, s->main, MAIN_PTS, GLOW_W, lv_color_hex(0x40c4ff), k * 45 / 255);
    for (int b = 0; b < s->n_branch; b++) {
        draw_polyline(ctx, s->branch[b], BRANCH_PTS, 8, lv_color_hex(0x40c4ff), k * 35 / 255);
        draw_polyline(ctx, s->branch[b], BRANCH_PTS, 3, lv_color_hex(0xb3e5fc), k * 150 / 255);
        draw_polyline(ctx, s->branch[b], BRANCH_PTS, 1, lv_color_white(), k * 200 / 255);
    }
    draw_polyline(ctx, s->main, MAIN_PTS, 8, lv_color_hex(0x80d8ff), k * 110 / 255);
    draw_polyline(ctx, s->main, MAIN_PTS, 3, lv_color_white(), k);
}

/* Return stroke: on, a dark beat, on again, then the afterglow fades */
static lv_opa_t strike_level(int32_t t) {
    if (t < 50) {
        return LV_OPA_COVER;
    }
    if (t < 95) {
        return LV_OPA_30;
    }
    if (t < 150) {
        return LV_OPA_COVER;
    }
    int32_t left = STRIKE_TIME - t;
    return left <= 0 ? 0 : (lv_opa_t)(255 * left * left / ((STRIKE_TIME - 150) * (STRIKE_TIME - 150)));
}

static void strike_anim_cb(void *obj, int32_t t) {
    strike_t *s = lv_obj_get_user_data(obj);
    s->level    = strike_level(t);
    lv_obj_invalidate_area(obj, &s->bbox);
    if (flash) {
        lv_obj_set_style_opa(flash, s->level * 70 / 255, 0);
    }
}

static void strike_start(strike_t *s, lv_point_t target, lv_coord_t x_lo, lv_coord_t x_hi, uint32_t delay) {
    strike_make(s, target, x_lo, x_hi);

    lv_obj_t *obj = lv_obj_create(splash);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(obj, s);
    lv_obj_add_event_cb(obj, strike_draw_cb, LV_EVENT_DRAW_MAIN, s);

    /* The object is the anim var, so deleting the splash stops the anim */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, strike_anim_cb);
    lv_anim_set_values(&a, 0, STRIKE_TIME);
    lv_anim_set_time(&a, STRIKE_TIME);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_early_apply(&a, false);
    lv_anim_start(&a);
}

/* Spark of charge running along the accent rule, with a fading trail */
static void spark_draw_cb(lv_event_t *e) {
    lv_draw_ctx_t *ctx = lv_event_get_draw_ctx(e);

    if (spark_pos < 0) {
        return;
    }
    lv_coord_t x = geom.rule_x1 + (geom.rule_x2 - geom.rule_x1) * spark_pos / 1000;
    lv_coord_t y = geom.rule_y;

    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.width = 4;
    line.color = lv_color_white();
    for (int i = 0; i < 10; i++) {
        lv_point_t p1 = {LV_MAX(x - (i + 1) * 14, geom.rule_x1), y};
        lv_point_t p2 = {LV_MAX(x - i * 14, geom.rule_x1), y};
        if (p1.x >= p2.x) {
            break;
        }
        line.opa = (lv_opa_t)(210 * (10 - i) / 10);
        lv_draw_line(ctx, &line, &p1, &p2);
    }

    static const struct {
        lv_coord_t r;
        uint32_t   color;
        lv_opa_t   opa;
    } head[] = {{18, 0x40c4ff, LV_OPA_20}, {11, 0x80d8ff, LV_OPA_50}, {6, 0xe1f5fe, LV_OPA_80}, {3, 0xffffff, LV_OPA_COVER}};

    lv_draw_rect_dsc_t dot;
    lv_draw_rect_dsc_init(&dot);
    dot.radius = LV_RADIUS_CIRCLE;
    for (uint32_t i = 0; i < sizeof(head) / sizeof(head[0]); i++) {
        lv_area_t a = {x - head[i].r, y - head[i].r, x + head[i].r, y + head[i].r};
        dot.bg_color = lv_color_hex(head[i].color);
        dot.bg_opa   = head[i].opa;
        lv_draw_rect(ctx, &dot, &a);
    }
}

static void spark_invalidate(lv_obj_t *obj) {
    lv_area_t a = {geom.rule_x1 - 20, geom.rule_y - 20, geom.rule_x2 + 20, geom.rule_y + 20};
    lv_obj_invalidate_area(obj, &a);
}

static void spark_anim_cb(void *obj, int32_t v) {
    spark_pos = v;
    spark_invalidate(obj);
}

static void spark_done_cb(lv_anim_t *a) {
    spark_pos = -1;
    spark_invalidate(a->var);
}

static void spark_start(void) {
    lv_obj_t *obj = lv_obj_create(splash);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(obj, spark_draw_cb, LV_EVENT_DRAW_MAIN, NULL);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, spark_anim_cb);
    lv_anim_set_values(&a, 0, 1000);
    lv_anim_set_time(&a, SPARK_TIME);
    lv_anim_set_delay(&a, SPARK_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_early_apply(&a, false);
    lv_anim_set_ready_cb(&a, spark_done_cb);
    lv_anim_start(&a);
}

/* "Good evening, KR8MER" from the configured call and the clock's zone */
static void greeting_text(char *buf, size_t n) {
    char        call[32] = "";
    const char *part     = NULL;
    time_t      now      = time(NULL);

    param_t_get_into(cfg.callsign(), call, sizeof(call));

    /* Before the clock is set (year < 2024) there is no time of day */
    if (now > 1704067200) {
        struct tm   tm;
        const char *abbr;
        tz_convert(now, param_i_get(cfg.ui.clock_tz()), &tm, &abbr);
        if (tm.tm_hour >= 5 && tm.tm_hour < 12) {
            part = "Good morning";
        } else if (tm.tm_hour >= 12 && tm.tm_hour < 18) {
            part = "Good afternoon";
        } else if (tm.tm_hour >= 18 && tm.tm_hour < 23) {
            part = "Good evening";
        } else {
            part = "Late night on the bands";
        }
    }
    if (!part) {
        part = "Welcome back";
    }
    if (call[0]) {
        snprintf(buf, n, "%s, %s", part, call);
    } else {
        snprintf(buf, n, "%s", part);
    }
}

static void greet_anim_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa(obj, (lv_opa_t)(v * 255 / 100), 0);
    lv_obj_set_style_translate_y(obj, (100 - v) / 8, 0);
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

static void greet_start(void) {
    char buf[64];
    greeting_text(buf, sizeof(buf));

    lv_obj_t *l = label(splash, buf, &sony_30, lv_color_hex(0xe1f5fe), 1);
    lv_obj_align(l, LV_ALIGN_TOP_MID, 0, 352);
    lv_obj_set_style_opa(l, LV_OPA_TRANSP, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, l);
    lv_anim_set_exec_cb(&a, greet_anim_cb);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 380);
    lv_anim_set_delay(&a, GREET_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_early_apply(&a, false);
    lv_anim_start(&a);
}

static void bolt_icon_draw(lv_draw_ctx_t *ctx, const lv_area_t *box, const lv_point_t *shape, uint32_t n,
                           lv_coord_t dx, lv_coord_t dy, const lv_draw_rect_dsc_t *dsc) {
    lv_point_t pts[8];
    for (uint32_t i = 0; i < n; i++) {
        pts[i].x = box->x1 + shape[i].x + dx;
        pts[i].y = box->y1 + shape[i].y + dy;
    }
    lv_draw_polygon(ctx, dsc, pts, n);
}

static void bolt_icon_cb(lv_event_t *e) {
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
        bolt_icon_draw(ctx, &box, bolt_upper, 5, nudge[k].x, nudge[k].y, &dsc);
        bolt_icon_draw(ctx, &box, bolt_lower, 4, nudge[k].x, nudge[k].y, &dsc);
    }

    dsc.bg_color = lv_color_hex(0xffeb3b);
    dsc.bg_opa   = LV_OPA_COVER;
    bolt_icon_draw(ctx, &box, bolt_upper, 5, 0, 0, &dsc);
    bolt_icon_draw(ctx, &box, bolt_lower, 4, 0, 0, &dsc);
}

static void fade_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void fade_done_cb(lv_anim_t *a) {
    lv_obj_del(splash);
    splash    = NULL;
    flash     = NULL;
    spark_pos = -1;
    free(logo_data);
    logo_data = NULL;
}

static bool load_logo(void) {
    FILE *f = fopen(LOGO_PATH, "rb");
    if (!f) {
        return false;
    }
    logo_data = malloc(LOGO_SIZE);
    bool ok = logo_data && fread(logo_data, 1, LOGO_SIZE, f) == LOGO_SIZE;
    fclose(f);
    if (!ok) {
        free(logo_data);
        logo_data = NULL;
        return false;
    }
    logo_dsc.header.always_zero = 0;
    logo_dsc.header.cf          = LV_IMG_CF_INDEXED_8BIT;
    logo_dsc.header.w           = SCREEN_WIDTH;
    logo_dsc.header.h           = SCREEN_HEIGHT;
    logo_dsc.data_size          = LOGO_SIZE;
    logo_dsc.data               = logo_data;
    return true;
}

/* Drawn fallback when the logo file is missing */
static void draw_wordmark(void) {
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
    lv_obj_add_event_cb(bolt, bolt_icon_cb, LV_EVENT_DRAW_MAIN, NULL);

    label(row, BRAND_CALL, &sony_60, lv_color_white(), 6);

    /* Yellow-to-cyan rule, as on the EAS Station card headers */
    lv_obj_t *rule = lv_obj_create(splash);
    lv_obj_remove_style_all(rule);
    lv_obj_set_size(rule, geom.rule_x2 - geom.rule_x1, 6);
    lv_obj_set_style_radius(rule, 3, 0);
    lv_obj_set_style_bg_opa(rule, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(rule, lv_color_hex(0xffeb3b), 0);
    lv_obj_set_style_bg_grad_color(rule, lv_color_hex(0x40c4ff), 0);
    lv_obj_set_style_bg_grad_dir(rule, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_pos(rule, geom.rule_x1, geom.rule_y - 3);

    lv_obj_t *ed = label(splash, "X6100   " BRAND_EDITION_CAPS, &sony_30, lv_color_hex(0x40c4ff), 3);
    lv_obj_align(ed, LV_ALIGN_CENTER, 0, 70);
}

void splash_show(void) {
    if (splash) {
        return;
    }
    rng = (uint32_t)time(NULL) * 2654435761u ^ (uint32_t)lv_tick_get();
    if (!rng) {
        rng = 0x2545f491;
    }

    splash = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(splash);
    lv_obj_set_size(splash, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(splash, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(splash, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(splash, lv_color_hex(0x14224d), 0);
    lv_obj_set_style_bg_grad_color(splash, lv_color_hex(0x060d1f), 0);
    lv_obj_set_style_bg_grad_dir(splash, LV_GRAD_DIR_VER, 0);

    if (load_logo()) {
        /* Same picture as the boot logo, so start-up runs straight on */
        geom          = geom_logo;
        lv_obj_t *img = lv_img_create(splash);
        lv_img_set_src(img, &logo_dsc);
        lv_obj_set_pos(img, 0, 0);
    } else {
        geom = geom_fallback;
        draw_wordmark();
    }

    lv_obj_t *credit = label(splash, VERSION "  -  built on the X6100 GUI by R1CBU and OpenX6100",
                             &sony_20, lv_color_hex(0x8fa3c8), 0);
    lv_obj_align(credit, LV_ALIGN_BOTTOM_MID, 0, -18);

    greet_start();
    spark_start();
    /* First bolt lands on the left end of the rule and sets the spark off;
     * the second lands on the far end as it arrives */
    lv_point_t left  = {geom.rule_x1 + 2, geom.rule_y - 3};
    lv_point_t right = {geom.rule_x2 - 6, geom.rule_y - 3};
    strike_start(&strikes[0], left, 20, 150, STRIKE1_MS);
    strike_start(&strikes[1], right, right.x - 90, SCREEN_WIDTH - 30, STRIKE2_MS);

    /* Sky lights up with each strike; transparent until one lands */
    flash = lv_obj_create(splash);
    lv_obj_remove_style_all(flash);
    lv_obj_set_size(flash, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(flash, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(flash, lv_color_hex(0xb3e5fc), 0);
    lv_obj_set_style_bg_opa(flash, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(flash, LV_OPA_TRANSP, 0);
    lv_obj_move_to_index(flash, 1);   /* over the logo, under the bolts */

    /* Hold, then fade out and clean up */
    lv_anim_t f;
    lv_anim_init(&f);
    lv_anim_set_var(&f, splash);
    lv_anim_set_exec_cb(&f, fade_cb);
    lv_anim_set_values(&f, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&f, SPLASH_FADE_MS);
    lv_anim_set_delay(&f, SPLASH_HOLD_MS);
    lv_anim_set_early_apply(&f, false);
    lv_anim_set_ready_cb(&f, fade_done_cb);
    lv_anim_start(&f);
}
