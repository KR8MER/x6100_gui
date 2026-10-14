/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 */

#pragma once

#include <stdbool.h>

/* Restyle stock LVGL widgets (dropdown lists, switches, sliders, keyboard,
 * text fields) for the Lightning skin. Takes effect for widgets created
 * afterwards; dialogs are built each time they open. */
void theme_lightning_enable(bool on);
