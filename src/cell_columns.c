/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#include "cell_columns.h"

static void draw_cb(lv_event_t *e) {
    lv_obj_draw_part_dsc_t *dsc  = lv_event_get_draw_part_dsc(e);
    const cell_columns_t   *cols = lv_event_get_user_data(e);

    if (dsc->part != LV_PART_ITEMS) {
        return;
    }

    if (lv_event_get_code(e) == LV_EVENT_DRAW_PART_BEGIN) {
        /* the cell's own text is only a placeholder for the row height */
        dsc->label_dsc->opa = LV_OPA_TRANSP;
        return;
    }

    lv_obj_t *table = lv_event_get_target(e);
    uint16_t  row   = dsc->id / lv_table_get_col_cnt(table);

    lv_draw_label_dsc_t label;
    lv_draw_label_dsc_init(&label);
    label.font  = lv_obj_get_style_text_font(table, LV_PART_ITEMS);
    label.color = dsc->label_dsc->color;   // already resolved for the cell's state
    label.flag  = LV_TEXT_FLAG_EXPAND;     // one line; the table clips the end

    lv_coord_t pad_left = lv_obj_get_style_pad_left(table, LV_PART_ITEMS);
    lv_coord_t pad_top  = lv_obj_get_style_pad_top(table, LV_PART_ITEMS);

    for (uint8_t c = 0; c < cols->count; c++) {
        const char *text = cols->get(row, c);
        if (!text || !text[0]) continue;

        lv_area_t area = *dsc->draw_area;
        area.x1 += pad_left + cols->x[c];
        area.y1 += pad_top;
        if (c + 1 < cols->count) {
            area.x2 = dsc->draw_area->x1 + pad_left + cols->x[c + 1] - 6;
        }
        if (area.x1 >= area.x2) continue;
        lv_draw_label(dsc->draw_ctx, &label, &area, text, NULL);
    }
}

void cell_columns_attach(lv_obj_t *table, const cell_columns_t *cols) {
    lv_obj_add_event_cb(table, draw_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)cols);
    lv_obj_add_event_cb(table, draw_cb, LV_EVENT_DRAW_PART_END, (void *)cols);
}
