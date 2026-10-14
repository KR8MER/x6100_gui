/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#pragma once

#include "lvgl/lvgl.h"

/* Brand shown on the splash and in the startup message */
#define BRAND_CALL    "KR8MER"
#define BRAND_EDITION "Lightning Edition"
#define BRAND_EDITION_CAPS "LIGHTNING EDITION"

/* Show the start-up splash on the top layer; it fades out by itself */
void splash_show(void);
