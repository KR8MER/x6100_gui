/*******************************************************************************
 * Size: 7 px
 * Bpp: 4
 * Opts: --bpp 4 --size 7 --no-compress --lv-include lvgl/lvgl.h --format lvgl --font fonts_src/B612-Bold.ttf --range 0x20-0x7E,0xB0 --font lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff --range 0xF1EB,0xF1E6,0xF240-0xF244,0xF077,0xF078,0xF0E7=>0xE55B -o src/fonts/sony_10.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef SONY_10
#define SONY_10 1
#endif

#if SONY_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0xa6, 0xa6, 0xa6, 0x52, 0x84,

    /* U+0022 "\"" */
    0x8c, 0x55, 0x83,

    /* U+0023 "#" */
    0x2, 0x51, 0x60, 0x8, 0xb5, 0xe0, 0xae, 0xee,
    0xf8, 0xc, 0x79, 0xa0, 0xaf, 0xde, 0xe7, 0xf,
    0x3c, 0x60, 0x4, 0x4, 0x10,

    /* U+0024 "$" */
    0x6, 0x50, 0x7f, 0xe3, 0xdb, 0x70, 0x3e, 0xc1,
    0x9, 0xc9, 0x9d, 0xe6, 0x2b, 0x70,

    /* U+0025 "%" */
    0x9d, 0x55, 0xc0, 0xe, 0x6c, 0xe3, 0x0, 0x7c,
    0xcd, 0xc8, 0x0, 0x1e, 0xb7, 0xf0, 0xa, 0x84,
    0xda, 0x0,

    /* U+0026 "&" */
    0xc, 0xd7, 0x1, 0xf1, 0x20, 0x6e, 0xb4, 0x4b,
    0x6a, 0xf6, 0x3d, 0xee, 0x60,

    /* U+0027 "'" */
    0xa4, 0x50,

    /* U+0028 "(" */
    0x3d, 0x27, 0xa0, 0x8a, 0x8, 0xa0, 0x8a, 0x7,
    0xa0, 0x5d, 0x10, 0x20,

    /* U+0029 ")" */
    0x1d, 0x40, 0x98, 0x9, 0x90, 0x99, 0x9, 0x90,
    0x98, 0x1c, 0x60, 0x20,

    /* U+002A "*" */
    0x4, 0x10, 0x8c, 0xb1, 0x3b, 0x90, 0x1, 0x20,

    /* U+002B "+" */
    0x7, 0x50, 0x8d, 0xc6, 0x7, 0x50, 0x4, 0x30,

    /* U+002C "," */
    0xa6, 0x85, 0x20,

    /* U+002D "-" */
    0x7d, 0x80,

    /* U+002E "." */
    0x21, 0xa6,

    /* U+002F "/" */
    0x0, 0xc, 0x70, 0x2, 0xf1, 0x0, 0x9a, 0x0,
    0xf, 0x30, 0x6, 0xc0, 0x0, 0xd6, 0x0,

    /* U+0030 "0" */
    0x3d, 0xd4, 0xb, 0x75, 0xd0, 0xd5, 0x2f, 0xa,
    0x84, 0xe0, 0x2d, 0xe5, 0x0,

    /* U+0031 "1" */
    0x0, 0x60, 0x1b, 0xf0, 0x36, 0xf0, 0x2, 0xf0,
    0x2, 0xf0, 0xc, 0xf9,

    /* U+0032 "2" */
    0x2c, 0xd5, 0x22, 0x5c, 0x1, 0xd4, 0x1d, 0x60,
    0x9f, 0xba,

    /* U+0033 "3" */
    0x4b, 0xd8, 0x8, 0xd1, 0x7, 0xca, 0x0, 0x8c,
    0x5c, 0xa2,

    /* U+0034 "4" */
    0x1, 0x60, 0x0, 0x99, 0x0, 0x1f, 0x20, 0x8,
    0xaa, 0x40, 0xbb, 0xec, 0x0, 0xc, 0x50,

    /* U+0035 "5" */
    0x8e, 0xb6, 0x8f, 0xd4, 0x11, 0x6c, 0x0, 0xb9,
    0x6d, 0x80,

    /* U+0036 "6" */
    0x7, 0xc6, 0x5, 0xeb, 0x70, 0xa9, 0x1f, 0x19,
    0xa2, 0xf0, 0x2c, 0xd6, 0x0,

    /* U+0037 "7" */
    0x6d, 0xef, 0x0, 0xb, 0x80, 0x4, 0xe0, 0x0,
    0xa8, 0x0, 0xc, 0x60, 0x0,

    /* U+0038 "8" */
    0x2d, 0xd6, 0x5, 0xd8, 0xb0, 0x3e, 0xd8, 0xa,
    0x71, 0xf1, 0x4d, 0xd9, 0x0,

    /* U+0039 "9" */
    0x3d, 0xd5, 0xa, 0x73, 0xe0, 0xa8, 0x3f, 0x2,
    0xbc, 0xd0, 0x4, 0xc4, 0x0, 0x62, 0x0,

    /* U+003A ":" */
    0x0, 0xa6, 0x42, 0xa6,

    /* U+003B ";" */
    0x0, 0xb6, 0x31, 0xb7, 0x83, 0x0,

    /* U+003C "<" */
    0x0, 0x4, 0x4, 0xca, 0x7e, 0x30, 0x19, 0xd5,
    0x0, 0x29,

    /* U+003D "=" */
    0x8c, 0xc1, 0x8c, 0xc1,

    /* U+003E ">" */
    0x31, 0x0, 0x4d, 0x81, 0x0, 0xae, 0x1a, 0xc4,
    0x65, 0x0,

    /* U+003F "?" */
    0x7d, 0xd4, 0x20, 0xb8, 0x8, 0xb0, 0x7, 0x20,
    0xa, 0x20,

    /* U+0040 "@" */
    0x4, 0xcd, 0xc3, 0x2, 0xe8, 0xcd, 0xe1, 0x8a,
    0x9c, 0xec, 0x59, 0x9f, 0xf, 0xb6, 0x7b, 0x99,
    0xcd, 0x21, 0xe3, 0x1, 0x0, 0x3, 0xbd, 0x90,
    0x0,

    /* U+0041 "A" */
    0x0, 0x0, 0x0, 0x8e, 0x0, 0xd, 0xb5, 0x2,
    0xe6, 0xb0, 0x7e, 0xdf, 0xd, 0x50, 0xd5,

    /* U+0042 "B" */
    0x9e, 0xc5, 0x9, 0xa6, 0xb0, 0x9e, 0xe7, 0x9,
    0xa2, 0xf0, 0x9e, 0xd9, 0x0,

    /* U+0043 "C" */
    0x2c, 0xda, 0xb, 0x80, 0x0, 0xe5, 0x0, 0xb,
    0x90, 0x0, 0x2c, 0xda, 0x0,

    /* U+0044 "D" */
    0x9e, 0xd7, 0x8, 0xa1, 0xe4, 0x8b, 0xc, 0x68,
    0xb1, 0xf3, 0x8f, 0xd6, 0x0,

    /* U+0045 "E" */
    0xae, 0xd8, 0x9a, 0x0, 0x9e, 0xc5, 0x9a, 0x0,
    0x9e, 0xd9,

    /* U+0046 "F" */
    0xae, 0xd9, 0x9a, 0x0, 0x9e, 0xc5, 0x9a, 0x0,
    0x9a, 0x0,

    /* U+0047 "G" */
    0x2c, 0xda, 0xb, 0x80, 0x0, 0xe5, 0xad, 0x4b,
    0x90, 0xd5, 0x2c, 0xdb, 0x10,

    /* U+0048 "H" */
    0x0, 0x0, 0xa, 0x90, 0xd6, 0x99, 0xd, 0x69,
    0xed, 0xf6, 0x99, 0xd, 0x69, 0x90, 0xd6,

    /* U+0049 "I" */
    0x44, 0xaa, 0x9a, 0x9a, 0x9a, 0x9a,

    /* U+004A "J" */
    0x4, 0xfa, 0x0, 0x9a, 0x0, 0x9a, 0x0, 0xa9,
    0xd, 0xe3,

    /* U+004B "K" */
    0x0, 0x0, 0x9, 0xa6, 0xd0, 0x9e, 0xe1, 0x9,
    0xf5, 0x0, 0x9c, 0xe3, 0x9, 0xa5, 0xe1,

    /* U+004C "L" */
    0x34, 0x0, 0x9a, 0x0, 0x9a, 0x0, 0x9a, 0x0,
    0x9a, 0x0, 0x9e, 0xd9,

    /* U+004D "M" */
    0x44, 0x0, 0x44, 0xae, 0x1, 0xf9, 0xae, 0x68,
    0xe9, 0xaa, 0xcd, 0xa9, 0xa7, 0xdb, 0x99, 0xa7,
    0x42, 0x99,

    /* U+004E "N" */
    0x34, 0x3, 0x49, 0xf2, 0x8a, 0x8e, 0xb8, 0xa8,
    0xae, 0xba, 0x8a, 0x5f, 0xa8, 0xa0, 0xca,

    /* U+004F "O" */
    0x1a, 0xed, 0x40, 0x9b, 0x4, 0xf0, 0xb7, 0x0,
    0xf3, 0x8b, 0x4, 0xf0, 0xa, 0xed, 0x40,

    /* U+0050 "P" */
    0x9e, 0xc7, 0x9a, 0x5d, 0x9e, 0xc5, 0x9a, 0x0,
    0x9a, 0x0,

    /* U+0051 "Q" */
    0x1a, 0xed, 0x40, 0x9b, 0x4, 0xf0, 0xb7, 0x0,
    0xf2, 0x8b, 0x69, 0xf0, 0xa, 0xee, 0xb1, 0x0,
    0x0, 0x60,

    /* U+0052 "R" */
    0x9e, 0xc7, 0x9, 0xa4, 0xe0, 0x9d, 0xc6, 0x9,
    0xac, 0x50, 0x9a, 0x2e, 0x20,

    /* U+0053 "S" */
    0x6d, 0xd3, 0xc8, 0x0, 0x19, 0xc2, 0x10, 0x99,
    0x9e, 0xd3,

    /* U+0054 "T" */
    0xce, 0xed, 0x0, 0x9a, 0x0, 0x9, 0xa0, 0x0,
    0x9a, 0x0, 0x9, 0xa0, 0x0,

    /* U+0055 "U" */
    0x0, 0x0, 0xa, 0x90, 0xd6, 0x99, 0xd, 0x69,
    0x90, 0xd6, 0x8b, 0xe, 0x41, 0xce, 0xa0,

    /* U+0056 "V" */
    0x0, 0x0, 0x0, 0xe, 0x40, 0xc5, 0x9, 0x81,
    0xf1, 0x4, 0xd5, 0xb0, 0x0, 0xfa, 0x60, 0x0,
    0xaf, 0x20, 0x0, 0x26, 0x0,

    /* U+0057 "W" */
    0x0, 0x0, 0x0, 0xd, 0x56, 0xd0, 0xd4, 0x99,
    0xaf, 0x3f, 0x5, 0xcd, 0xca, 0xc0, 0x1f, 0xe8,
    0xe7, 0x0, 0xdb, 0x4f, 0x30,

    /* U+0058 "X" */
    0x0, 0x0, 0x0, 0x9, 0xa2, 0xe2, 0x1, 0xec,
    0x70, 0x0, 0x9f, 0x0, 0x3, 0xea, 0x80, 0xc,
    0x61, 0xf3,

    /* U+0059 "Y" */
    0x0, 0x0, 0x0, 0xc, 0x64, 0xe0, 0x4, 0xda,
    0x50, 0x0, 0xbc, 0x0, 0x0, 0x9a, 0x0, 0x0,
    0x9a, 0x0,

    /* U+005A "Z" */
    0x9, 0xde, 0xb0, 0x0, 0x1f, 0x20, 0x0, 0xb9,
    0x0, 0x4, 0xe1, 0x0, 0xd, 0xdd, 0xc0,

    /* U+005B "[" */
    0xdd, 0x7c, 0x60, 0xc6, 0xc, 0x60, 0xc6, 0xc,
    0x60, 0xbd, 0x70,

    /* U+005C "\\" */
    0xd, 0x60, 0x0, 0x6c, 0x0, 0x0, 0xf3, 0x0,
    0x9, 0x90, 0x0, 0x3f, 0x0, 0x0, 0xc6,

    /* U+005D "]" */
    0x9e, 0xa0, 0x99, 0x9, 0x90, 0x99, 0x9, 0x90,
    0x99, 0x9d, 0x90,

    /* U+005E "^" */
    0x0, 0x40, 0x0, 0x7, 0xfb, 0x0, 0x6d, 0x2a,
    0xa0,

    /* U+005F "_" */
    0xee, 0xec,

    /* U+0060 "`" */
    0x1, 0x0, 0xd3,

    /* U+0061 "a" */
    0x5c, 0xd1, 0x5c, 0xf5, 0xd3, 0xc5, 0x7c, 0xe5,

    /* U+0062 "b" */
    0x77, 0x0, 0x89, 0x0, 0x8d, 0xd6, 0x89, 0x4d,
    0x89, 0x5c, 0x7e, 0xd4,

    /* U+0063 "c" */
    0x4d, 0xc1, 0xc5, 0x0, 0xc5, 0x0, 0x3d, 0xc1,

    /* U+0064 "d" */
    0x0, 0x69, 0x0, 0x7b, 0x2c, 0xdb, 0x98, 0x7b,
    0xa7, 0x7b, 0x3d, 0xcb,

    /* U+0065 "e" */
    0x4c, 0xc3, 0xcc, 0xc7, 0xc5, 0x0, 0x4d, 0xc3,

    /* U+0066 "f" */
    0x3e, 0xa7, 0xa0, 0xde, 0x77, 0xa0, 0x7a, 0x7,
    0xa0,

    /* U+0067 "g" */
    0x0, 0x0, 0x3c, 0xda, 0xb7, 0x7b, 0xb7, 0x7b,
    0x3d, 0xeb, 0x0, 0x8a, 0x4d, 0xd2,

    /* U+0068 "h" */
    0x77, 0x0, 0x99, 0x0, 0x9d, 0xd5, 0x99, 0x7a,
    0x99, 0x7a, 0x99, 0x7a,

    /* U+0069 "i" */
    0x77, 0x44, 0x98, 0x98, 0x98,

    /* U+006A "j" */
    0x7, 0x70, 0x34, 0x8, 0x90, 0x89, 0x8, 0x90,
    0x88, 0x4e, 0x40,

    /* U+006B "k" */
    0x77, 0x0, 0x88, 0x0, 0x89, 0xd3, 0x8f, 0x50,
    0x8d, 0xc0, 0x88, 0x99,

    /* U+006C "l" */
    0x78, 0x8, 0x90, 0x89, 0x8, 0x90, 0x89, 0x4,
    0xe4,

    /* U+006D "m" */
    0x0, 0x0, 0x0, 0x8d, 0xdc, 0xd7, 0x89, 0x7b,
    0x5c, 0x89, 0x7b, 0x5c, 0x89, 0x7b, 0x5c,

    /* U+006E "n" */
    0x0, 0x0, 0x8d, 0xe7, 0x89, 0x7b, 0x89, 0x7b,
    0x89, 0x7b,

    /* U+006F "o" */
    0x3c, 0xd5, 0xb7, 0x3e, 0xa7, 0x3e, 0x2c, 0xd5,

    /* U+0070 "p" */
    0x0, 0x0, 0x7d, 0xd6, 0x7a, 0x2e, 0x7a, 0x3d,
    0x7e, 0xc4, 0x7a, 0x0, 0x7a, 0x0,

    /* U+0071 "q" */
    0x3c, 0xd8, 0xb6, 0x7a, 0xb5, 0x7a, 0x4d, 0xda,
    0x0, 0x7a, 0x0, 0x7a,

    /* U+0072 "r" */
    0x0, 0x8, 0xd9, 0x99, 0x9, 0x90, 0x99, 0x0,

    /* U+0073 "s" */
    0x9d, 0x7a, 0xb2, 0x5, 0xea, 0xd9,

    /* U+0074 "t" */
    0x1, 0x5, 0x90, 0xee, 0x77, 0xa0, 0x7a, 0x3,
    0xe4,

    /* U+0075 "u" */
    0x0, 0x0, 0x89, 0x6a, 0x89, 0x7b, 0x89, 0x7b,
    0x4e, 0xcb,

    /* U+0076 "v" */
    0x0, 0x0, 0xc4, 0x88, 0x89, 0xd3, 0x3d, 0xd0,
    0xe, 0x80, 0x0, 0x0,

    /* U+0077 "w" */
    0x0, 0x0, 0x0, 0xb5, 0xe4, 0xe0, 0x7a, 0xea,
    0xb0, 0x2e, 0x9e, 0x60, 0xe, 0x4f, 0x20,

    /* U+0078 "x" */
    0x0, 0x0, 0x0, 0xa8, 0x98, 0x1, 0xdc, 0x0,
    0x2d, 0xe1, 0xc, 0x59, 0xa0,

    /* U+0079 "y" */
    0xa, 0x23, 0x80, 0xa8, 0xa7, 0x4, 0xbe, 0x10,
    0xd, 0xb0, 0x0, 0xb5, 0x0, 0x9b, 0x0,

    /* U+007A "z" */
    0x8b, 0xe0, 0xb, 0x70, 0x4d, 0x0, 0xdc, 0xb2,

    /* U+007B "{" */
    0x3, 0xe8, 0x8, 0xa0, 0x9, 0x90, 0x3f, 0x40,
    0x9, 0xa0, 0x8, 0xa0, 0x3, 0xc8,

    /* U+007C "|" */
    0xa3, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4,

    /* U+007D "}" */
    0x1d, 0xb0, 0x1, 0xf1, 0x1, 0xf2, 0x0, 0xca,
    0x1, 0xf1, 0x2, 0xf1, 0x1c, 0x90,

    /* U+007E "~" */
    0x3d, 0xff, 0x81, 0x22, 0x10,

    /* U+00B0 "°" */
    0x89, 0x8, 0x90,

    /* U+E55B "" */
    0x0, 0x0, 0x0, 0x9, 0xff, 0x0, 0xb, 0xfa,
    0x0, 0xd, 0xfe, 0xd3, 0xd, 0xef, 0xc0, 0x0,
    0x3f, 0x30, 0x0, 0x6a, 0x0, 0x0, 0x82, 0x0,

    /* U+F077 "" */
    0x0, 0x12, 0x0, 0x0, 0x2e, 0xe3, 0x0, 0x2e,
    0x86, 0xe3, 0xc, 0x80, 0x6, 0xe0, 0x10, 0x0,
    0x1, 0x0,

    /* U+F078 "" */
    0x40, 0x0, 0x4, 0xb, 0xb0, 0xa, 0xc0, 0xb,
    0xba, 0xc1, 0x0, 0xb, 0xc1, 0x0, 0x0, 0x0,
    0x0, 0x0,

    /* U+F1E6 "" */
    0x0, 0x0, 0x0, 0x1b, 0x7, 0x50, 0x1a, 0x6,
    0x50, 0xef, 0xff, 0xe3, 0x8f, 0xff, 0xc0, 0x3f,
    0xff, 0x70, 0x3, 0xe5, 0x0, 0x0, 0xb0, 0x0,

    /* U+F1EB "" */
    0x0, 0x25, 0x75, 0x10, 0x2, 0xbf, 0xba, 0xcf,
    0x90, 0xc7, 0x2, 0x41, 0x1a, 0x80, 0x2d, 0xfd,
    0xfb, 0x0, 0x1, 0x60, 0x1, 0x70, 0x0, 0x0,
    0x5f, 0x20, 0x0, 0x0, 0x2, 0x80, 0x0, 0x0,

    /* U+F240 "" */
    0xbe, 0xee, 0xee, 0xed, 0x2e, 0x58, 0x88, 0x88,
    0x7c, 0xea, 0xff, 0xff, 0xf1, 0xee, 0x57, 0x77,
    0x77, 0x8a, 0xac, 0xcc, 0xcc, 0xcc, 0x10,

    /* U+F241 "" */
    0xbe, 0xee, 0xee, 0xed, 0x2e, 0x58, 0x88, 0x60,
    0x6c, 0xea, 0xff, 0xfc, 0x0, 0xee, 0x57, 0x77,
    0x51, 0x7a, 0xac, 0xcc, 0xcc, 0xcc, 0x10,

    /* U+F242 "" */
    0xbe, 0xee, 0xee, 0xed, 0x2e, 0x58, 0x83, 0x0,
    0x6c, 0xea, 0xff, 0x70, 0x0, 0xee, 0x57, 0x74,
    0x11, 0x7a, 0xac, 0xcc, 0xcc, 0xcc, 0x10,

    /* U+F243 "" */
    0xbe, 0xee, 0xee, 0xed, 0x2e, 0x58, 0x0, 0x0,
    0x6c, 0xea, 0xf1, 0x0, 0x0, 0xee, 0x57, 0x21,
    0x11, 0x7a, 0xac, 0xcc, 0xcc, 0xcc, 0x10,

    /* U+F244 "" */
    0xbe, 0xee, 0xee, 0xed, 0x2e, 0x0, 0x0, 0x0,
    0x6d, 0xe0, 0x0, 0x0, 0x0, 0xee, 0x11, 0x11,
    0x11, 0x7c, 0xac, 0xcc, 0xcc, 0xcc, 0x20
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 39, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 39, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 5, .adv_w = 45, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 8, .adv_w = 95, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 29, .adv_w = 62, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 43, .adv_w = 101, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 78, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 22, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 76, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 88, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 100, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 108, .adv_w = 67, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 28, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 119, .adv_w = 50, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 121, .adv_w = 34, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 62, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 73, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 73, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 73, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 73, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 73, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 73, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 262, .adv_w = 34, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 28, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 272, .adv_w = 73, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 56, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 286, .adv_w = 73, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 112, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 331, .adv_w = 73, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 67, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 395, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 34, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 50, .box_w = 4, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 67, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 62, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 101, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 90, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 90, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 552, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 67, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 603, .adv_w = 73, .box_w = 6, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 624, .adv_w = 106, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 73, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 67, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 681, .adv_w = 67, .box_w = 6, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 696, .adv_w = 50, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 707, .adv_w = 62, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 722, .adv_w = 56, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 733, .adv_w = 84, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 742, .adv_w = 62, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 744, .adv_w = 62, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 747, .adv_w = 59, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 755, .adv_w = 67, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 767, .adv_w = 56, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 67, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 787, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 795, .adv_w = 45, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 804, .adv_w = 67, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 818, .adv_w = 67, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 830, .adv_w = 34, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 835, .adv_w = 34, .box_w = 3, .box_h = 7, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 846, .adv_w = 62, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 39, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 867, .adv_w = 101, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 882, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 892, .adv_w = 67, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 900, .adv_w = 67, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 914, .adv_w = 67, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 926, .adv_w = 45, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 934, .adv_w = 50, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 940, .adv_w = 45, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 959, .adv_w = 62, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 971, .adv_w = 84, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 986, .adv_w = 62, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 999, .adv_w = 62, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 1014, .adv_w = 56, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1022, .adv_w = 73, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1036, .adv_w = 22, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1043, .adv_w = 67, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1057, .adv_w = 84, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1062, .adv_w = 34, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 1065, .adv_w = 70, .box_w = 6, .box_h = 8, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1089, .adv_w = 98, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1107, .adv_w = 98, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1125, .adv_w = 84, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1149, .adv_w = 140, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1181, .adv_w = 140, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1204, .adv_w = 140, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1227, .adv_w = 140, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1250, .adv_w = 140, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1273, .adv_w = 140, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0xe4ab, 0xefc7, 0xefc8, 0xf136, 0xf13b, 0xf190, 0xf191,
    0xf192, 0xf193, 0xf194
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 176, .range_length = 61845, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 11, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    16, 16,
    35, 48,
    35, 53,
    35, 55,
    35, 56,
    35, 67,
    35, 73,
    35, 74,
    35, 75,
    35, 76,
    35, 78,
    35, 81,
    35, 86,
    35, 90,
    36, 48,
    37, 36,
    37, 85,
    38, 48,
    38, 55,
    39, 34,
    39, 66,
    39, 67,
    39, 68,
    39, 69,
    39, 70,
    39, 72,
    39, 73,
    39, 74,
    39, 75,
    39, 76,
    39, 78,
    39, 79,
    39, 80,
    39, 81,
    39, 82,
    39, 83,
    39, 84,
    39, 85,
    39, 86,
    39, 87,
    39, 88,
    39, 90,
    40, 59,
    40, 66,
    40, 70,
    40, 80,
    40, 82,
    45, 48,
    45, 50,
    45, 53,
    45, 55,
    45, 56,
    45, 58,
    45, 87,
    46, 86,
    48, 37,
    48, 53,
    48, 55,
    48, 56,
    48, 57,
    48, 58,
    48, 67,
    48, 73,
    48, 74,
    48, 75,
    49, 34,
    49, 45,
    49, 55,
    49, 57,
    49, 58,
    49, 66,
    49, 68,
    49, 69,
    49, 70,
    49, 72,
    49, 73,
    49, 74,
    49, 75,
    49, 78,
    49, 79,
    49, 80,
    49, 81,
    49, 83,
    49, 86,
    49, 89,
    50, 55,
    50, 56,
    51, 48,
    51, 53,
    51, 55,
    51, 56,
    51, 69,
    52, 55,
    53, 34,
    53, 48,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 72,
    53, 76,
    53, 78,
    53, 79,
    53, 80,
    53, 81,
    53, 82,
    53, 83,
    53, 84,
    53, 86,
    53, 87,
    53, 88,
    53, 89,
    53, 90,
    54, 34,
    55, 48,
    55, 66,
    55, 69,
    55, 70,
    55, 72,
    55, 74,
    55, 75,
    55, 76,
    55, 78,
    55, 79,
    55, 80,
    55, 81,
    55, 82,
    55, 83,
    55, 86,
    55, 87,
    55, 88,
    55, 89,
    55, 90,
    55, 91,
    56, 34,
    56, 48,
    56, 50,
    56, 66,
    56, 69,
    56, 70,
    56, 72,
    56, 73,
    56, 74,
    56, 75,
    56, 76,
    56, 78,
    56, 79,
    56, 80,
    56, 81,
    56, 82,
    56, 83,
    56, 86,
    56, 87,
    56, 88,
    56, 89,
    56, 90,
    56, 91,
    57, 48,
    58, 34,
    58, 48,
    58, 66,
    58, 67,
    58, 68,
    58, 69,
    58, 70,
    58, 72,
    58, 74,
    58, 75,
    58, 76,
    58, 78,
    58, 79,
    58, 81,
    58, 82,
    58, 83,
    58, 86,
    58, 87,
    58, 88,
    58, 89,
    58, 90,
    58, 91,
    59, 48,
    66, 53,
    66, 58,
    66, 84,
    68, 53,
    69, 69,
    70, 53,
    70, 88,
    71, 67,
    71, 68,
    71, 69,
    71, 70,
    71, 71,
    71, 72,
    71, 77,
    71, 78,
    71, 79,
    71, 80,
    71, 81,
    71, 82,
    71, 86,
    71, 88,
    72, 67,
    73, 53,
    73, 58,
    77, 77,
    77, 84,
    79, 69,
    79, 75,
    79, 78,
    80, 53,
    80, 55,
    80, 58,
    81, 53,
    81, 77,
    82, 53,
    82, 58,
    82, 77,
    83, 52,
    83, 71,
    83, 84,
    83, 85,
    83, 87,
    83, 90,
    84, 53,
    84, 70,
    85, 53,
    85, 85,
    86, 53,
    86, 58,
    86, 75,
    87, 53,
    87, 69,
    87, 80,
    88, 69,
    90, 53,
    91, 48,
    91, 53,
    91, 58
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -11, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, 6,
    6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, 6, 3, 6, 6, 6, -6,
    -6, -6, -8, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, 6,
    -6, -6, -6, -3, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, 6, -3,
    -6, -6, -11, -6, -6, -6, -6, -6,
    -6, -6, 6, 6, 6, 6, 6, 6,
    -6, 3, -6, -6, -6, -6, -6, -6,
    -6, -6, -6, -6, -6, -6, -6
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 239,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 4,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t sony_10 = {
#else
lv_font_t sony_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if SONY_10*/

