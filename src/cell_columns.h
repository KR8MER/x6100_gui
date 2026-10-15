/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Columns inside a one-column lv_table. The encoder steps a one-column
 *  table row by row, but the UI fonts are proportional (only digits are
 *  fixed width), so padded strings drift. Instead each row's cell holds a
 *  placeholder and its fields are drawn at fixed x offsets, in the colour the
 *  table chose for that cell (so the selected row keeps its highlight ink).
 */

#pragma once

#include "lvgl/lvgl.h"

typedef const char *(*cell_columns_get_t)(uint16_t row, uint8_t col);

typedef struct {
    const lv_coord_t  *x;       // left edge of each column, relative to the cell
    uint8_t            count;
    cell_columns_get_t get;
} cell_columns_t;

/* Add to the table: draws `cols` for every row. `cols` must outlive it. */
void cell_columns_attach(lv_obj_t *table, const cell_columns_t *cols);
