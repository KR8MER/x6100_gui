#include "settings_widgets.h"

extern "C" {
#include "../atu_log.h"
#include "../main.h"
#include "../splash.h"
#include "../radio.h"
}

/***** PAGE *****/

void make_info_page(SettingsPage &page) {
    page.destroy();

    lv_obj_t *grid = lv_obj_create(page.dialog->obj);

    page.grid = grid;

    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_add_style(grid, &style.text_base_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(grid, 30, 0);
    lv_obj_set_style_pad_ver(grid, 40, 0);

    lv_obj_t *label = lv_label_create(grid);
    lv_obj_set_style_text_line_space(label, 20, LV_PART_MAIN);
    lv_label_set_text_fmt(label,
                          "%s %s\n"
                          "App version: %s\nBASE version: %s\n"
                          "Built on the X6100 GUI by R1CBU and OpenX6100",
                          BRAND_CALL, BRAND_EDITION, VERSION, x6100_control_get_fw_version_str());

    /* The tuner's setting for this frequency, raw: see atu_log.h */
    lv_obj_t *tuner = lv_label_create(grid);
    lv_obj_set_style_text_line_space(tuner, 20, LV_PART_MAIN);
    lv_obj_align_to(tuner, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);
    if (cfg_atu_is_loaded()) {
        char bits[40];
        atu_log_format_bits(cfg_atu_get_network(), bits, sizeof(bits));
        lv_label_set_text_fmt(tuner, "Tuner here: 0x%08X\n%s\nEvery tune is logged to atu_log.csv",
                              (unsigned)cfg_atu_get_network(), bits);
    } else {
        lv_label_set_text(tuner, "Tuner here: no saved tune\nEvery tune is logged to atu_log.csv");
    }
}
