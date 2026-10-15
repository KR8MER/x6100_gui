/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Operating cockpits, after Nexus (KD9TAW). Entering one sets the radio up
 *  for it in one step: Phone picks the sideband for the band (and keeps it
 *  right when you change band), CW turns on the decoder, Digital opens FT8
 *  in USB-D, RTTY opens the RTTY panel in LSB-D. Phone and CW bring their
 *  own button pages: log, messages, recorder, hunter and the controls that
 *  matter for that mode.
 */

#include "cockpit.h"

#include "buttons.h"
#include "cfg/cfg_api.h"
#include "main_screen.h"
#include "msg.h"
#include "voice.h"

#include "aether_radio/x6100_control/control.h"

static int last_rule = -1;   // sideband rule for the last frequency seen

static void load_group_first(buttons_page_t **group) {
    buttons_unload_page();
    buttons_load_page(group[0]);
}

/* Phone: follow the sideband when the band changes. Only a change of the
 * rule's answer switches it, so picking the other sideband by hand on a band
 * sticks until you leave that band. */
static void on_freq_change(Subject *subj, void *user_data) {
    (void)subj;
    (void)user_data;

    int32_t freq = cparam_i_get(cfg.cur.fg_freq());
    int     rule = cockpit_phone_sideband(freq);

    if (rule == last_rule) {
        return;
    }
    last_rule = rule;

    if (param_i_get(cfg.radio.cockpit()) != COCKPIT_PHONE) {
        return;
    }
    x6100_mode_t mode = cparam_i_get(cfg.cur.mode());
    if ((mode == x6100_mode_lsb || mode == x6100_mode_usb) && mode != (x6100_mode_t)rule) {
        cparam_i_set(cfg.cur.mode(), rule);
    }
}

void cockpit_init(void) {
    last_rule = cockpit_phone_sideband(cparam_i_get(cfg.cur.fg_freq()));
    subject_subscribe_delayed((Subject *)cfg.cur.fg_freq(), on_freq_change, NULL);
}

cockpit_t cockpit_current(void) {
    return (cockpit_t)param_i_get(cfg.radio.cockpit());
}

void cockpit_enter(cockpit_t c) {
    int32_t freq = cparam_i_get(cfg.cur.fg_freq());

    param_i_set(cfg.radio.cockpit(), c);

    switch (c) {
        case COCKPIT_PHONE:
            main_screen_start_app(ACTION_NONE);     // close any app
            last_rule = cockpit_phone_sideband(freq);
            cparam_i_set(cfg.cur.mode(), last_rule);
            load_group_first(buttons_group_phone);
            break;

        case COCKPIT_CW:
            main_screen_start_app(ACTION_NONE);
            cparam_i_set(cfg.cur.mode(), x6100_mode_cw);
            param_i_set(cfg.cw.decoder(), true);
            load_group_first(buttons_group_cw);
            break;

        case COCKPIT_DIGITAL:
            cparam_i_set(cfg.cur.mode(), x6100_mode_usb_dig);
            main_screen_start_app(ACTION_APP_FT8);
            break;

        case COCKPIT_RTTY:
            cparam_i_set(cfg.cur.mode(), x6100_mode_lsb_dig);
            main_screen_start_app(ACTION_APP_RTTY);
            break;

        default:
            return;
    }

    msg_update_text_fmt("%s cockpit", cockpit_name(c));
    voice_say_text_fmt("%s cockpit", cockpit_name(c));
}
