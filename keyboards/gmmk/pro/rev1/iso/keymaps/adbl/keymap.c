/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*

# TODO

- [] red light bootloader
- [] enable debug key

-   [] [Caps Word](https://docs.qmk.fm/#/feature_caps_word)

 */

// clang-format off

#include QMK_KEYBOARD_H
#include "print.h"
#include "color.h"

#include "rgb_matrix_map.h"

enum custom_keycodes { 
    KC_ENCODER = SAFE_RANGE,
};
#define KC_MCTL KC_MISSION_CONTROL
#define KC_LPAD KC_LAUNCHPAD

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

//      ESC      F1       F2       F3       F4       F5       F6       F7       F8       F9       F10      F11      F12	     Prt           Rotary(Mute)
//      ~        1        2        3        4        5        6        7        8        9        0         -       (=)	     BackSpc           Del
//      Tab      Q        W        E        R        T        Y        U        I        O        P        [        ]                          PgUp
//      Caps     A        S        D        F        G        H        J        K        L        ;        "        #        Enter             PgDn
//      Sh_L     /        Z        X        C        V        B        N        M        ,        .        ?                 Sh_R     Up       End
//      Ct_L     Win_L    Alt_L                               SPACE                               Alt_R    FN       Ct_R     Left     Down     Right

    [0] = LAYOUT(
        KC_ESC,  KC_BRMD, KC_BRMU, KC_MCTL, KC_LPAD, RGB_VAD, RGB_VAI, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, KC_INSERT,        KC_ENCODER,
        KC_NUBS, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_PGUP,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC,                   KC_PGDN,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NUHS, KC_ENT,           KC_HOME,
        KC_LSFT, KC_GRV,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_END,
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, MO(1),   KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [1] = LAYOUT(
        QK_RBT,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,           _______,
        _______, RGB_TOG, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, QK_BOOT,          _______,
        _______, _______, RGB_VAI, RGB_HUI, RGB_SAI, _______, _______, _______, _______, _______, _______, _______, _______,                   _______,
        _______, _______, RGB_VAD, RGB_HUD, RGB_SAD, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, NK_TOGG, _______, _______, _______, _______,          _______, RGB_MOD, _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, RGB_SPD, RGB_RMOD, RGB_SPI
    ),

};

enum encoder_mode { 
    KEYS_RGB_BRIGHTNESS,
    KEYS_RGB_HUE,
    KEYS_RGB_SATURATION,
    SIDES_RGB_BRIGHTNESS,
    SIDES_RGB_HUE,
    SIDES_RGB_SATURATION,

    CONFIRM,
    NORMAL,
};

enum reset_status {
    NONE,
    AVAILABLE,
    PRELIMINARY,
};

enum encoder_mode encoderMode = NORMAL;
bool encoderPressed = false;
enum reset_status resetStatus = NONE;

HSV sides = {0, 255, 255};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_MISSION_CONTROL:
            if (record->event.pressed) {
                host_consumer_send(0x29F);
            } else {
                host_consumer_send(0);
            }
            return false;
        case KC_LAUNCHPAD:
            if (record->event.pressed) {
                host_consumer_send(0x2A0);
            } else {
                host_consumer_send(0);
            }
            return false;
        case KC_ENCODER:
            encoderPressed = record->event.pressed;
            if (!encoderPressed) {
                HSV *resetColor = NULL;
                if (resetStatus == PRELIMINARY) { 
                    #ifdef RGB_MATRIX_ENABLE
                    HSV keysColor = rgb_matrix_get_hsv();
                    resetColor = &keysColor;
                    dprintf("reset %d\n", encoderMode);
                    #endif
                }

                switch(encoderMode) {
                    case NORMAL:
                        tap_code(KC_MPLY);
                        break;
                    case CONFIRM:
                        encoderMode = NORMAL;
                        break;
                    case SIDES_RGB_BRIGHTNESS:
                        if (resetColor != NULL) {
                            sides.v = resetColor->v;
                        }
                        break;
                    case SIDES_RGB_HUE:
                        if (resetColor != NULL) {
                            sides.h = resetColor->h;
                        }
                        break;
                    case SIDES_RGB_SATURATION:
                        if (resetColor != NULL) {
                            sides.s = resetColor->s;
                        }
                        break;
                    default:
                        break;
                }
            }
            else if (resetStatus == AVAILABLE) {
                resetStatus = PRELIMINARY;
            }
            return false;
        default:
            return true;
    }
}

uint8_t add_clamped(uint8_t value, int8_t increment) {
    const int16_t result = value + increment;
    if (result > 0xFF) {
        return 0xFF;
    }
    else if (result < 0) {
        return 0;
    }
    return result;
}

#ifdef ENCODER_ENABLE

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index != 0) {
        return true;
    }

    const uint8_t mods = get_mods();
    const uint8_t shiftMods = (mods & MOD_BIT(KC_RSFT)) | (mods & MOD_BIT(KC_LSFT));
    const uint8_t cmdMods = (mods & MOD_BIT(KC_LGUI)) | (mods & MOD_BIT(KC_RGUI));

    if (encoderPressed) {
        if (clockwise && ++encoderMode > CONFIRM) {
            encoderMode = 0;
        }
        else if (!clockwise && --encoderMode > CONFIRM) {
            encoderMode = CONFIRM;
        }
        resetStatus = 
            (encoderMode >= SIDES_RGB_BRIGHTNESS && encoderMode <= SIDES_RGB_SATURATION)
            ? AVAILABLE : NONE;
    }
    else if (encoderMode < CONFIRM) {
        #ifdef RGB_MATRIX_ENABLE
        bool settingKeys = encoderMode < SIDES_RGB_BRIGHTNESS;

        HSV *current;
        if (settingKeys) {
            HSV keysColor = rgb_matrix_get_hsv();
            current = &keysColor;
        } 
        else {
            current = &sides;
        }
        const int8_t increment = (clockwise ? 1 : -1) * (shiftMods ? 1 : 8);
        switch (encoderMode) {
            case KEYS_RGB_BRIGHTNESS:
            case SIDES_RGB_BRIGHTNESS:
                current->v = add_clamped(current->v, increment);
                break;
            case KEYS_RGB_HUE:
            case SIDES_RGB_HUE:
                current->h = add_clamped(current->h, increment);
                break;
            case KEYS_RGB_SATURATION:
            case SIDES_RGB_SATURATION:
                current->s = add_clamped(current->s, increment);
                break;
            default:
                break;
        }
        if (settingKeys) {
            rgb_matrix_sethsv(current->h, current->s, current->v);
        }
        #endif
    }
    else if (shiftMods) {
        unregister_mods(shiftMods);
        if (clockwise) {
            tap_code(KC_MEDIA_NEXT_TRACK);
        } else {
            tap_code(KC_MEDIA_PREV_TRACK);
        }
        register_mods(shiftMods);
    }
    else if (cmdMods) {
        unregister_mods(cmdMods);
        if (clockwise) {
            tap_code(KC_WH_U);
        } else {
            tap_code(KC_WH_D);
        }
        register_mods(cmdMods);
    }
    else {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    dprintf("encoder_update_user: encoderMode %d, resetStatus %d\n", encoderMode, resetStatus);
    return false;
}
#endif

#define ARRAYSIZE(arr) sizeof(arr) / sizeof(arr[0])
#ifdef RGB_MATRIX_ENABLE
const uint8_t NUMBER_LEDS[] = {LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8};

uint8_t value_indication = 0xFF;

void housekeeping_task_user(void) {
    static uint32_t last_draw = 0;
    if (encoderMode != NORMAL) {
        if (timer_elapsed32(last_draw) > 3) {
            last_draw = timer_read32();
            if (++value_indication == 0) {
                value_indication = 64;
            }
        }
    }
}

void brightness_leds(const HSV current) {
    for (uint8_t i=0; i<8; i++) {
        HSV color = {current.h, current.s, 31 + i * 32};
        if (current.v <= color.v && current.v > (color.v - 32)) {
            color.v = value_indication;
        }
        RGB rgb = hsv_to_rgb(color);
        rgb_matrix_set_color(NUMBER_LEDS[i], rgb.r, rgb.g, rgb.b);
    }
}

void hue_leds(const HSV current) {
    for (uint8_t i=0; i<8; i++) {
        // TODO saturation from current?
        HSV color = {31 + i * 32, 255, 255};
        if (current.h <= color.h && current.h > (color.h - 32)) {
            color.v = value_indication;
        }
        RGB rgb = hsv_to_rgb(color);
        rgb_matrix_set_color(NUMBER_LEDS[i], rgb.r, rgb.g, rgb.b);
    }
}

void saturation_leds(const HSV current) {
    for (uint8_t i=0; i<8; i++) {
        HSV color = {current.h, 31 + i * 32, 255};
        if (current.s <= color.s && current.s > (color.s - 32)) {
            color.v = value_indication;
        }
        RGB rgb = hsv_to_rgb(color);
        rgb_matrix_set_color(NUMBER_LEDS[i], rgb.r, rgb.g, rgb.b);
    }
}

uint8_t top_tint_value(const HSV color) {
    const uint8_t top = 0xFF - value_indication;
    if (top < 8) {
        return color.v * (1.0/16 * (top + 8));
    }
    return 0;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    HSV keysColor = rgb_matrix_get_hsv();
    HSV sidesColor = sides;

    if (encoderMode <= KEYS_RGB_SATURATION) {
        uint8_t top_tint = top_tint_value(keysColor);
        if (top_tint != 0) {
            HSV tintColor = keysColor;
            tintColor.v = top_tint;
            RGB rgb = hsv_to_rgb(tintColor);
            rgb_matrix_set_color_all(rgb.r, rgb.g, rgb.b);
        }
    }

    // TODO skip value indication if sides not set

    switch (encoderMode) {
        case KEYS_RGB_BRIGHTNESS:
            brightness_leds(keysColor);
            break;
        case KEYS_RGB_HUE:
            hue_leds(keysColor);
            break;
        case KEYS_RGB_SATURATION:
            saturation_leds(keysColor);
            break;
        case SIDES_RGB_BRIGHTNESS:
            brightness_leds(sidesColor);
            break;
        case SIDES_RGB_HUE:
            hue_leds(sidesColor);
            break;
        case SIDES_RGB_SATURATION:
            saturation_leds(sidesColor);
            break;
        default:
            break;
    }

     if (encoderMode > KEYS_RGB_SATURATION && encoderMode <= SIDES_RGB_SATURATION) {
        uint8_t top_tint = top_tint_value(sidesColor);
        if (top_tint != 0) {
            sidesColor.v = top_tint;
        }
    }

    const RGB sidesRGB = hsv_to_rgb(sidesColor);

    for (uint8_t i=0; i<ARRAYSIZE(LED_SIDE_RIGHT); i++) {
        rgb_matrix_set_color(LED_SIDE_RIGHT[i], sidesRGB.r, sidesRGB.g, sidesRGB.b);
    }
    for (uint8_t i=0; i<ARRAYSIZE(LED_SIDE_LEFT); i++) {
        rgb_matrix_set_color(LED_SIDE_LEFT[i], sidesRGB.r, sidesRGB.g, sidesRGB.b);
    }
    return true;
}
#endif

void keyboard_post_init_user(void) {
#ifdef RGB_MATRIX_ENABLE
    sides = rgb_matrix_get_hsv();
    sides.v = 0xFF;
#endif
}

// void suspend_power_down_user(void) {
//     uprintf("suspend_power_down_user\n", timer_read());
//     // code will run multiple times while keyboard is suspended
// }
// void suspend_wakeup_init_user(void) {
//     // code will run on keyboard wakeup
// }

// clang-format on
