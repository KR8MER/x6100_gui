/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Tuner network readout. The base board reports each tune as a 32-bit
 *  relay setting whose meaning Xiegu has not published. Every tune is shown
 *  and appended to atu_log.csv on the DATA partition (time, antenna,
 *  frequency, the setting in hex and in bits), so tunes on known loads can
 *  be compared to work out which bits switch which inductor or capacitor.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ATU_LOG_PATH
#define ATU_LOG_PATH "/mnt/atu_log.csv"
#endif

/* "00000000 00000000 00000000 00000000", most significant byte first */
void atu_log_format_bits(uint32_t network, char *out, size_t n);

/* A tune finished (any thread): shows and logs it on the LVGL thread */
void atu_log_tuned(uint32_t network);

#ifdef __cplusplus
}
#endif
