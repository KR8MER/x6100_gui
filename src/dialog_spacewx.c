/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Space weather window: solar flux and K index.
 */

#include "dialog_spacewx.h"

#include "styles.h"

static void construct_cb(lv_obj_t *parent) {
    lv_obj_t *obj = dialog_init(parent);
    lv_obj_t *l   = lv_label_create(obj);
    lv_label_set_text(l, "Space weather");
    lv_obj_center(l);
}

static dialog_t dialog = {
    .run          = false,
    .construct_cb = construct_cb,
};

dialog_t *dialog_spacewx = &dialog;
