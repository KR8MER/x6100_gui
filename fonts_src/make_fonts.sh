#!/bin/sh
# Regenerates src/fonts/*.c from B612 Bold (fonts_src/B612-Bold.ttf, SIL OFL,
# fonts_src/B612-OFL.txt) and LVGL's Font Awesome 5 for the status icons.
#
# The C names stay sony_N / mono_N (N: the size the screens were laid out
# for); sizes.txt gives the B612 pixel size for each, chosen so text is no
# wider and at most 10 % taller than with the old font. B612's digits are
# all one width, so the mono_* fonts are the same face.
#
# Needs lv_font_conv 1.5.2:  npm install lv_font_conv@1.5.2
set -e
cd "$(dirname "$0")/.."
CONV=${LV_FONT_CONV:-lv_font_conv}
FA=lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff
# wifi, plug, battery 0-4, chevron up/down; the bolt stands in for the
# charging plug at U+E55B (SYMBOL_PLUG_CHARGE), which Font Awesome 5 lacks
ICONS=0xF1EB,0xF1E6,0xF240-0xF244,0xF077,0xF078,0xF0E7=\>0xE55B

while read name size; do
    $CONV --bpp 4 --size "$size" --no-compress --lv-include lvgl/lvgl.h --format lvgl \
        --font fonts_src/B612-Bold.ttf --range 0x20-0x7E,0xB0 \
        --font "$FA" --range "$ICONS" \
        -o "src/fonts/$name.c"
    # lv_font_conv names the font after the output file: already sony_N / mono_N
done < fonts_src/sizes.txt
