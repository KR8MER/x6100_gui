/*******************************************************************************
 * Size: 6 px
 * Bpp: 4
 * Opts: --bpp 4 --size 6 --no-compress --lv-include lvgl/lvgl.h --format lvgl --font fonts_src/B612-Bold.ttf --range 0x20-0x7E,0xB0 --font lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff --range 0xF1EB,0xF1E6,0xF240-0xF244,0xF077,0xF078,0xF0E7=>0xE55B -o src/fonts/sony_8.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef SONY_8
#define SONY_8 1
#endif

#if SONY_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0xb3, 0xb3, 0x92, 0x71,

    /* U+0022 "\"" */
    0x9d, 0x2, 0x20,

    /* U+0023 "#" */
    0x0, 0x0, 0x0, 0xb5, 0xd3, 0x7f, 0xaf, 0x78,
    0xfa, 0xf7, 0x2b, 0x3a, 0x0,

    /* U+0024 "$" */
    0x0, 0x0, 0x3e, 0x60, 0xdc, 0x30, 0x3e, 0x90,
    0x3c, 0xd1, 0x5e, 0x40,

    /* U+0025 "%" */
    0xbb, 0x3d, 0x10, 0x8a, 0xa6, 0x0, 0x2, 0xda,
    0xa0, 0xc, 0x4c, 0xc1,

    /* U+0026 "&" */
    0x2c, 0xb1, 0x4, 0xe1, 0x0, 0xc8, 0xce, 0x6,
    0xbd, 0xb0,

    /* U+0027 "'" */
    0xb1, 0x10,

    /* U+0028 "(" */
    0x6a, 0x8, 0x60, 0x96, 0x9, 0x60, 0x87, 0x1,
    0x50,

    /* U+0029 ")" */
    0x2d, 0x10, 0xc2, 0xc, 0x30, 0xc3, 0xd, 0x22,
    0x50,

    /* U+002A "*" */
    0x1, 0x7, 0xc6, 0x4a, 0x30, 0x0,

    /* U+002B "+" */
    0xa, 0x0, 0x6d, 0x80, 0x8, 0x0,

    /* U+002C "," */
    0xb3, 0x60,

    /* U+002D "-" */
    0x59, 0x20,

    /* U+002E "." */
    0xa2,

    /* U+002F "/" */
    0x0, 0x2d, 0x0, 0x9, 0x60, 0x1, 0xe0, 0x0,
    0x87, 0x0, 0x7, 0x10, 0x0,

    /* U+0030 "0" */
    0x5c, 0xb0, 0xd3, 0xa6, 0xc3, 0x96, 0x4c, 0xb1,

    /* U+0031 "1" */
    0x0, 0x0, 0x1b, 0x90, 0x37, 0x90, 0x6, 0x90,
    0x1b, 0xc2,

    /* U+0032 "2" */
    0x5a, 0xb1, 0x0, 0xc2, 0xb, 0x50, 0x8d, 0x83,

    /* U+0033 "3" */
    0x49, 0xe0, 0x1e, 0x90, 0x0, 0xc4, 0x4a, 0x80,

    /* U+0034 "4" */
    0x4, 0x30, 0xd, 0x10, 0x79, 0x70, 0xa9, 0xf3,
    0x1, 0xe0,

    /* U+0035 "5" */
    0x9c, 0x80, 0x9c, 0xa0, 0x0, 0xc3, 0x5b, 0x70,

    /* U+0036 "6" */
    0x1a, 0x90, 0x9b, 0xa2, 0xb5, 0x78, 0x4b, 0xb2,

    /* U+0037 "7" */
    0x59, 0xe5, 0x3, 0xd0, 0xc, 0x40, 0xf, 0x0,

    /* U+0038 "8" */
    0x4b, 0xc1, 0x4d, 0xd1, 0xa7, 0xa6, 0x6b, 0xb3,

    /* U+0039 "9" */
    0x6b, 0xb1, 0xb4, 0x97, 0x3a, 0xd5, 0x9, 0x90,
    0x0, 0x0,

    /* U+003A ":" */
    0xa3, 0xa2,

    /* U+003B ";" */
    0xa2, 0x51, 0xa3, 0x10,

    /* U+003C "<" */
    0x0, 0x12, 0x18, 0xa2, 0x6c, 0x30, 0x1, 0x84,

    /* U+003D "=" */
    0x69, 0x66, 0x96,

    /* U+003E ">" */
    0x20, 0x0, 0x4b, 0x70, 0x5, 0xd4, 0x66, 0x0,

    /* U+003F "?" */
    0x8a, 0xc0, 0x4, 0xc0, 0xc, 0x10, 0x8, 0x0,

    /* U+0040 "@" */
    0x9, 0xaa, 0x80, 0x78, 0x8c, 0xc5, 0xa7, 0xac,
    0xb7, 0x6d, 0xaa, 0xd4, 0x6, 0xa9, 0x0,

    /* U+0041 "A" */
    0x0, 0x0, 0xb, 0xa0, 0x2b, 0xd0, 0x7c, 0xd5,
    0xd2, 0x4b,

    /* U+0042 "B" */
    0xab, 0xc1, 0xab, 0xd1, 0xa6, 0x87, 0xac, 0xc2,

    /* U+0043 "C" */
    0x4b, 0xa2, 0xd3, 0x0, 0xd3, 0x0, 0x4c, 0xa2,

    /* U+0044 "D" */
    0xac, 0xb3, 0x97, 0x5b, 0x97, 0x5b, 0x9c, 0xb2,

    /* U+0045 "E" */
    0xac, 0x91, 0xab, 0x80, 0xa6, 0x0, 0xac, 0x92,

    /* U+0046 "F" */
    0xac, 0x92, 0xab, 0x80, 0xa6, 0x0, 0xa6, 0x0,

    /* U+0047 "G" */
    0x4b, 0x92, 0xd4, 0xa8, 0xd3, 0x4b, 0x4c, 0xb6,

    /* U+0048 "H" */
    0x42, 0x14, 0xa6, 0x4c, 0xab, 0xbc, 0xa6, 0x4c,
    0xa6, 0x4c,

    /* U+0049 "I" */
    0x0, 0xb6, 0xa6, 0xa6, 0xa6,

    /* U+004A "J" */
    0x4, 0xe4, 0x0, 0xd4, 0x0, 0xd3, 0xa, 0xc1,

    /* U+004B "K" */
    0x0, 0x0, 0xa7, 0xc4, 0xad, 0x40, 0xae, 0x70,
    0xa7, 0xc5,

    /* U+004C "L" */
    0x0, 0x0, 0xa6, 0x0, 0xa6, 0x0, 0xa6, 0x0,
    0xac, 0x92,

    /* U+004D "M" */
    0x21, 0x0, 0x2b, 0xa0, 0x9c, 0xbd, 0x5d, 0xcb,
    0x8e, 0x7c, 0xb4, 0x63, 0xc0,

    /* U+004E "N" */
    0x0, 0x0, 0xab, 0xf, 0x9e, 0x7f, 0x97, 0xdf,
    0x96, 0x5f,

    /* U+004F "O" */
    0x3b, 0xaa, 0xb, 0x50, 0xa6, 0xb5, 0xa, 0x62,
    0xcb, 0xa0,

    /* U+0050 "P" */
    0xab, 0xb1, 0xa6, 0xb5, 0xac, 0x90, 0xa6, 0x0,

    /* U+0051 "Q" */
    0x3b, 0xaa, 0xb, 0x50, 0xa6, 0xb5, 0x4a, 0x52,
    0xcd, 0xe2, 0x0, 0x4, 0x20,

    /* U+0052 "R" */
    0xab, 0xb2, 0xa8, 0xc4, 0xad, 0xb0, 0xa6, 0x97,

    /* U+0053 "S" */
    0x8b, 0x80, 0x99, 0x20, 0x3, 0xe1, 0x8a, 0xb0,

    /* U+0054 "T" */
    0x9e, 0xb4, 0xc, 0x40, 0xc, 0x40, 0xc, 0x40,

    /* U+0055 "U" */
    0x0, 0x0, 0xb6, 0x4c, 0xa6, 0x4c, 0xa6, 0x4b,
    0x4c, 0xc5,

    /* U+0056 "V" */
    0x0, 0x0, 0x0, 0xd1, 0x4b, 0x8, 0x79, 0x50,
    0x2c, 0xd0, 0x0, 0xca, 0x0, 0x0, 0x0,

    /* U+0057 "W" */
    0x0, 0x0, 0x0, 0xd2, 0xc5, 0x95, 0x98, 0xf9,
    0xd1, 0x4d, 0xad, 0xc0, 0xf, 0x5d, 0x80,

    /* U+0058 "X" */
    0x0, 0x0, 0x0, 0x97, 0x97, 0x0, 0xdb, 0x0,
    0x2d, 0xc0, 0xc, 0x49, 0x80,

    /* U+0059 "Y" */
    0x0, 0x0, 0x0, 0xc4, 0xb4, 0x3, 0xcb, 0x0,
    0xc, 0x40, 0x0, 0xc4, 0x0,

    /* U+005A "Z" */
    0x7, 0xaf, 0x20, 0x9, 0x80, 0x3, 0xd0, 0x0,
    0xdb, 0x93,

    /* U+005B "[" */
    0xda, 0x1d, 0x30, 0xd3, 0xd, 0x30, 0xa9, 0x10,

    /* U+005C "\\" */
    0xc, 0x30, 0x0, 0x5b, 0x0, 0x0, 0xd2, 0x0,
    0x6, 0xa0, 0x0, 0x7, 0x0,

    /* U+005D "]" */
    0x7e, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0x7b, 0x30,

    /* U+005E "^" */
    0x5, 0xa0, 0x5, 0xb6, 0xb1,

    /* U+005F "_" */
    0xbb, 0xb3,

    /* U+0060 "`" */
    0x1a, 0x0,

    /* U+0061 "a" */
    0x5a, 0x97, 0x9d, 0xa8, 0xd0,

    /* U+0062 "b" */
    0x32, 0x0, 0x96, 0x0, 0x9b, 0xc1, 0x96, 0xa4,
    0x8b, 0xb0,

    /* U+0063 "c" */
    0x7a, 0x6d, 0x10, 0x6a, 0x60,

    /* U+0064 "d" */
    0x0, 0x41, 0x0, 0xc3, 0x4a, 0xe3, 0xb4, 0xc3,
    0x6b, 0xe3,

    /* U+0065 "e" */
    0x7a, 0xa0, 0xd8, 0xb0, 0x7a, 0x70,

    /* U+0066 "f" */
    0x6b, 0x3c, 0xc1, 0x86, 0x8, 0x60,

    /* U+0067 "g" */
    0x0, 0x0, 0x6b, 0xe2, 0xc3, 0xd2, 0x6b, 0xf2,
    0x39, 0xb0,

    /* U+0068 "h" */
    0x31, 0x0, 0xa5, 0x0, 0xaa, 0xc0, 0xa5, 0xd2,
    0xa5, 0xd2,

    /* U+0069 "i" */
    0x63, 0x84, 0xa5, 0xa5,

    /* U+006A "j" */
    0x6, 0x30, 0x85, 0x9, 0x50, 0x95, 0x2d, 0x30,

    /* U+006B "k" */
    0x32, 0x0, 0x95, 0x0, 0x9a, 0x90, 0x9d, 0x20,
    0x97, 0xc1,

    /* U+006C "l" */
    0x32, 0x9, 0x60, 0x96, 0x9, 0x60, 0x6b, 0x0,

    /* U+006D "m" */
    0x0, 0x0, 0x9, 0xbd, 0xab, 0x96, 0xd2, 0xf9,
    0x6d, 0x2f,

    /* U+006E "n" */
    0x0, 0x0, 0x9b, 0xd1, 0x96, 0xd2, 0x96, 0xd2,

    /* U+006F "o" */
    0x5b, 0xb1, 0xc3, 0x96, 0x5b, 0xb1,

    /* U+0070 "p" */
    0x0, 0x0, 0x8b, 0xb1, 0x86, 0x85, 0x8b, 0xb1,
    0x86, 0x0,

    /* U+0071 "q" */
    0x5a, 0xd1, 0xc2, 0xd2, 0x7a, 0xe2, 0x0, 0xd2,

    /* U+0072 "r" */
    0x0, 0x9, 0xb3, 0xa5, 0xa, 0x50,

    /* U+0073 "s" */
    0xaa, 0x25, 0xb4, 0x8b, 0x50,

    /* U+0074 "t" */
    0x55, 0xd, 0xc2, 0x86, 0x6, 0xb0,

    /* U+0075 "u" */
    0x0, 0x0, 0x95, 0xc2, 0x96, 0xd2, 0x6b, 0xe2,

    /* U+0076 "v" */
    0x0, 0x0, 0xc2, 0xd0, 0x7b, 0x90, 0x1f, 0x30,
    0x0, 0x0,

    /* U+0077 "w" */
    0x0, 0x0, 0xb, 0x6c, 0xa3, 0x6c, 0xbb, 0x1,
    0xe7, 0x90,

    /* U+0078 "x" */
    0x0, 0x0, 0x0, 0x97, 0xc0, 0x1, 0xf6, 0x0,
    0xb5, 0xd1,

    /* U+0079 "y" */
    0x3, 0x2, 0x10, 0xc4, 0xd0, 0x5, 0xb9, 0x0,
    0xe, 0x30, 0x8, 0x90, 0x0,

    /* U+007A "z" */
    0x6c, 0x72, 0xd0, 0xca, 0x60,

    /* U+007B "{" */
    0x8, 0xb1, 0xc, 0x30, 0x3e, 0x10, 0xb, 0x40,
    0x6, 0xa1,

    /* U+007C "|" */
    0x0, 0xe1, 0xe1, 0xe1, 0xe1, 0xe1,

    /* U+007D "}" */
    0x2b, 0x70, 0x5, 0xa0, 0x2, 0xd2, 0x6, 0xa0,
    0x2a, 0x40,

    /* U+007E "~" */
    0x4c, 0xca, 0x0,

    /* U+00B0 "°" */
    0x75, 0x76,

    /* U+E55B "" */
    0x2, 0x42, 0x0, 0xbf, 0x70, 0xd, 0xf8, 0x40,
    0xff, 0xf7, 0x1, 0x8d, 0x0, 0xa, 0x40, 0x0,
    0x60, 0x0,

    /* U+F077 "" */
    0x0, 0xa2, 0x0, 0xb, 0xad, 0x20, 0xb8, 0x4,
    0xd1, 0x20, 0x0, 0x20,

    /* U+F078 "" */
    0x92, 0x0, 0xa1, 0x4e, 0x2b, 0x80, 0x5, 0xf8,
    0x0, 0x0, 0x20, 0x0,

    /* U+F1E6 "" */
    0x13, 0x4, 0x4, 0x80, 0xc0, 0xab, 0x9c, 0x4a,
    0xff, 0xf2, 0x4f, 0xfc, 0x0, 0x4b, 0x0, 0x1,
    0x50, 0x0,

    /* U+F1EB "" */
    0x0, 0x0, 0x0, 0x0, 0x5, 0xbd, 0xc8, 0x20,
    0xba, 0x41, 0x26, 0xd4, 0x13, 0xbe, 0xd7, 0x10,
    0x4, 0x40, 0x17, 0x0, 0x0, 0xe, 0x60, 0x0,
    0x0, 0x3, 0x10, 0x0,

    /* U+F240 "" */
    0xdc, 0xcc, 0xcc, 0xd4, 0xcc, 0xff, 0xff, 0x6b,
    0xc7, 0x99, 0x99, 0x69, 0xac, 0xcc, 0xcc, 0xb0,

    /* U+F241 "" */
    0xdc, 0xcc, 0xcc, 0xd4, 0xcc, 0xff, 0xf0, 0x3b,
    0xc7, 0x99, 0x90, 0x59, 0xac, 0xcc, 0xcc, 0xb0,

    /* U+F242 "" */
    0xdc, 0xcc, 0xcc, 0xd4, 0xcc, 0xfc, 0x0, 0x3b,
    0xc7, 0x97, 0x0, 0x59, 0xac, 0xcc, 0xcc, 0xb0,

    /* U+F243 "" */
    0xdc, 0xcc, 0xcc, 0xd4, 0xcc, 0xa0, 0x0, 0x3b,
    0xc7, 0x60, 0x0, 0x59, 0xac, 0xcc, 0xcc, 0xb0,

    /* U+F244 "" */
    0xdc, 0xcc, 0xcc, 0xc2, 0xc0, 0x0, 0x0, 0x3b,
    0xc0, 0x0, 0x0, 0x5a, 0xac, 0xcc, 0xcc, 0xb0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 34, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 34, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 38, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 7, .adv_w = 82, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 53, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 32, .adv_w = 86, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 44, .adv_w = 67, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 19, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 56, .adv_w = 43, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 65, .adv_w = 43, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 74, .adv_w = 53, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 80, .adv_w = 58, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 24, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 88, .adv_w = 43, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 90, .adv_w = 29, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 53, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 104, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 190, .adv_w = 29, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 24, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 196, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 48, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 53, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 238, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 53, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 29, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 43, .box_w = 4, .box_h = 4, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 53, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 86, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 72, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 393, .adv_w = 62, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 53, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 67, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 62, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 442, .adv_w = 91, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 62, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 58, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 58, .box_w = 5, .box_h = 4, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 43, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 501, .adv_w = 53, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 514, .adv_w = 48, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 522, .adv_w = 72, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 527, .adv_w = 53, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 529, .adv_w = 53, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 531, .adv_w = 50, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 53, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 38, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 573, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 583, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 29, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 29, .box_w = 3, .box_h = 5, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 605, .adv_w = 53, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 34, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 86, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 633, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 641, .adv_w = 58, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 647, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 657, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 665, .adv_w = 38, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 43, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 38, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 58, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 53, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 700, .adv_w = 72, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 710, .adv_w = 53, .box_w = 5, .box_h = 4, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 53, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 733, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 62, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 748, .adv_w = 19, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 754, .adv_w = 58, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 764, .adv_w = 72, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 767, .adv_w = 29, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 769, .adv_w = 60, .box_w = 5, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 787, .adv_w = 84, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 799, .adv_w = 84, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 811, .adv_w = 72, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 829, .adv_w = 120, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 857, .adv_w = 120, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 120, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 120, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 120, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 921, .adv_w = 120, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0}
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
    -10, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, 5,
    5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, 5, 2, 5, 5, 5, -5,
    -5, -5, -7, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, 5,
    -5, -5, -5, -2, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, 5, -2,
    -5, -5, -10, -5, -5, -5, -5, -5,
    -5, -5, 5, 5, 5, 5, 5, 5,
    -5, 2, -5, -5, -5, -5, -5, -5,
    -5, -5, -5, -5, -5, -5, -5
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
const lv_font_t sony_8 = {
#else
lv_font_t sony_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 8,          /*The maximum line height required by the font*/
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



#endif /*#if SONY_8*/

