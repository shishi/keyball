#include QMK_KEYBOARD_H

#include "quantum.h"

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"
void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif

layer_state_t layer_state_set_user(layer_state_t state) {
    // if just include layer2, set scroll mode
    // keyball_set_scroll_mode(layer_state_is(2));

    // Auto enable scroll mode when the highest layer is 3
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    return state;
}

// 2回Shiftを入力する関数
void double_shift(void) {
    tap_code(KC_LSFT);
    tap_code(KC_LSFT);
}

// begin of smtd
// https://github.com/stasmarkin/sm_td/blob/main/docs/010_installation_guide.md
enum custom_keycodes {
    SMTD_KEYCODES_BEGIN = SAFE_RANGE,
    CKC_A, // reads as C(ustom) + KC_A, but you may give any name here
    CKC_B,
    CKC_C,
    CKC_D,
    CKC_E,
    CKC_F,
    CKC_G,
    CKC_H,
    CKC_I,
    CKC_J,
    CKC_K,
    CKC_L,
    CKC_M,
    CKC_N,
    CKC_O,
    CKC_P,
    CKC_Q,
    CKC_R,
    CKC_S,
    CKC_T,
    CKC_U,
    CKC_V,
    CKC_W,
    CKC_X,
    CKC_Y,
    CKC_Z,
    CKC_BACKSPACE,
    CKC_COMMA,
    CKC_DOT,
    CKC_ENTER,
    CKC_ESCAPE,
    CKC_SEMICOLON,
    CKC_SLASH,
    CKC_SPACE,
    CKC_TAB,
    DOUBLE_SHIFT_KEY,
    SMTD_KEYCODES_END,
};
#include "sm_td.h"

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_smtd(keycode, record)) {
        return false;
    }

    switch (keycode) {
        case DOUBLE_SHIFT_KEY:
            if (record->event.pressed) {
                double_shift();
            }
            return false;
    }

    // your rest code here
    return true;
}

// タップ後ホールドを検出するための静的変数
static bool key_was_tapped = false;

// 単一キー用マクロ
// 使い分け: タップでキー、ホールドで別の単一キー、タップ後ホールドでリピート
// 例: SMTD_TAP_HOLD_REPEAT(KC_T, KC_LBRC)
//     タップ: t, ホールド: [, タップ後ホールド: tリピート
#define SMTD_TAP_HOLD_REPEAT(tap_key, hold_key) \
    switch (action) { \
        case SMTD_ACTION_TOUCH: \
            break; \
        case SMTD_ACTION_TAP: \
            tap_code16(tap_key); \
            key_was_tapped = true; \
            break; \
        case SMTD_ACTION_HOLD: \
            switch (tap_count) { \
                case 0: \
                case 1: \
                    register_code16(tap_key); \
                    break; \
                default: \
                    register_code16(hold_key); \
                    break; \
            } \
            break; \
        case SMTD_ACTION_RELEASE: \
            switch (tap_count) { \
                case 0: \
                case 1: \
                    unregister_code16(tap_key); \
                    break; \
                default: \
                    unregister_code16(hold_key); \
                    break; \
            } \
            break; \
    }

// 複数キー用マクロ（修飾キー + キー）
// 使い分け: タップでキー、ホールドで修飾キー+キーの組み合わせ
// 例: SMTD_TAP_HOLD_REPEAT_COMBO(KC_C, KC_LCTL, KC_C)
//     タップ: c, ホールド: Ctrl+C, タップ後ホールド: cリピート
// 例: SMTD_TAP_HOLD_REPEAT_COMBO(KC_V, KC_LCTL, KC_V)
//     タップ: v, ホールド: Ctrl+V, タップ後ホールド: vリピート
#define SMTD_TAP_HOLD_REPEAT_COMBO(tap_key, hold_mod, hold_key) \
    switch (action) { \
        case SMTD_ACTION_TOUCH: \
            break; \
        case SMTD_ACTION_TAP: \
            tap_code16(tap_key); \
            key_was_tapped = true; \
            break; \
        case SMTD_ACTION_HOLD: \
            if (!key_was_tapped) { \
                register_mods(MOD_BIT(hold_mod)); \
                register_code16(hold_key); \
            } else { \
                register_code16(tap_key); \
            } \
            break; \
        case SMTD_ACTION_RELEASE: \
            unregister_code16(tap_key); \
            unregister_mods(MOD_BIT(hold_mod)); \
            unregister_code16(hold_key); \
            key_was_tapped = false; \
            break; \
    }

// カスタム関数呼び出し用マクロ
// 使い分け: タップでキー、ホールドで任意のカスタム動作
// 例: SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_A, tap_code16(LALT(KC_F4)), do_nothing())
//     タップ: a, ホールド: Alt+F, タップ後ホールド: aリピート
// 例: SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_L, layer_on(3), layer_off(3))
//     タップ: l, ホールド: レイヤー3有効, タップ後ホールド: lリピート
#define SMTD_TAP_HOLD_REPEAT_CUSTOM(tap_key, hold_action, release_action) \
    switch (action) { \
        case SMTD_ACTION_TOUCH: \
            break; \
        case SMTD_ACTION_TAP: \
            tap_code16(tap_key); \
            key_was_tapped = true; \
            break; \
        case SMTD_ACTION_HOLD: \
            if (!key_was_tapped) { \
                hold_action; \
            } else { \
                register_code16(tap_key); \
            } \
            break; \
        case SMTD_ACTION_RELEASE: \
            unregister_code16(tap_key); \
            release_action; \
            key_was_tapped = false; \
            break; \
    }

// 複数キー順次入力用マクロ（文字列送信）
// 使い分け: タップでキー、ホールドで文字列を順次入力
// 例: SMTD_TAP_HOLD_REPEAT_STRING(KC_M, "hello@example.com")
//     タップ: m, ホールド: "hello@example.com"を順次入力
// 例: SMTD_TAP_HOLD_REPEAT_STRING(KC_E, "echo 'Hello World'")
//     タップ: e, ホールド: "echo 'Hello World'"を順次入力
#define SMTD_TAP_HOLD_REPEAT_STRING(tap_key, hold_string) \
    switch (action) { \
        case SMTD_ACTION_TOUCH: \
            break; \
        case SMTD_ACTION_TAP: \
            tap_code16(tap_key); \
            key_was_tapped = true; \
            break; \
        case SMTD_ACTION_HOLD: \
            if (!key_was_tapped) { \
                send_string(hold_string); \
            } else { \
                register_code16(tap_key); \
            } \
            break; \
        case SMTD_ACTION_RELEASE: \
            unregister_code16(tap_key); \
            key_was_tapped = false; \
            break; \
    }

// 複数キー順次入力用マクロ（キーコード配列）
// 使い分け: タップでキー、ホールドで複数キーコードを順次実行
// 例: SMTD_TAP_HOLD_REPEAT_SEQUENCE(KC_A, KC_H, KC_E, KC_L, KC_L, KC_O)
//     タップ: a, ホールド: h-e-l-l-oを順次入力
// 例: SMTD_TAP_HOLD_REPEAT_SEQUENCE(KC_B, LCTL(KC_C), LCTL(KC_V))
//     タップ: b, ホールド: Ctrl+C → Ctrl+V を順次実行
// 例: SMTD_TAP_HOLD_REPEAT_SEQUENCE(KC_S, KC_ESCAPE, LSFT(KC_SEMICOLON), KC_Q, KC_ENTER)
//     タップ: s, ホールド: Esc → : → q → Enter (vim終了コマンド)
#define SMTD_TAP_HOLD_REPEAT_SEQUENCE(tap_key, ...) \
    switch (action) { \
        case SMTD_ACTION_TOUCH: \
            break; \
        case SMTD_ACTION_TAP: \
            tap_code16(tap_key); \
            key_was_tapped = true; \
            break; \
        case SMTD_ACTION_HOLD: \
            if (!key_was_tapped) { \
                uint16_t keys[] = {__VA_ARGS__, KC_NO}; \
                for (int i = 0; keys[i] != KC_NO; i++) { \
                    tap_code16(keys[i]); \
                } \
            } else { \
                register_code16(tap_key); \
            } \
            break; \
        case SMTD_ACTION_RELEASE: \
            unregister_code16(tap_key); \
            key_was_tapped = false; \
            break; \
    }

void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
    switch ((uint16_t)keycode) {
        case CKC_A:
            SMTD_TAP_HOLD_REPEAT(KC_A, KC_LEFT_GUI);
            break;
        case CKC_B:
            SMTD_TAP_HOLD_REPEAT(KC_B, KC_LEFT_CURLY_BRACE);
            break;
        case CKC_C:
            SMTD_TAP_HOLD_REPEAT_COMBO(KC_C, KC_LCTL, KC_C);
            break;
        case CKC_D:
            SMTD_TAP_HOLD_REPEAT(KC_D, KC_LEFT_SHIFT)
            break;
        case CKC_E:
            SMTD_TAP_HOLD_REPEAT(KC_E, KC_ESCAPE);
            break;
        case CKC_F:
            SMTD_TAP_HOLD_REPEAT(KC_F, KC_LEFT_CTRL);
            break;
        case CKC_G:
            SMTD_TAP_HOLD_REPEAT(KC_G, KC_LEFT_PAREN);
            break;
        case CKC_H:
            SMTD_TAP_HOLD_REPEAT(KC_H, KC_RIGHT_PAREN);
            break;
        case CKC_I:
            SMTD_TAP_HOLD_REPEAT(KC_I, KC_ESCAPE);
            break;
        case CKC_J:
            SMTD_TAP_HOLD_REPEAT(KC_J, KC_RIGHT_CTRL);
            break;
        case CKC_K:
            SMTD_TAP_HOLD_REPEAT(KC_K, KC_RIGHT_SHIFT);
            break;
        case CKC_L:
            SMTD_TAP_HOLD_REPEAT(KC_L, KC_RIGHT_ALT);
            break;
        case CKC_M:
            SMTD_TAP_HOLD_REPEAT(KC_M, KC_MINUS);
            break;
        case CKC_N:
            SMTD_TAP_HOLD_REPEAT(KC_N, KC_RIGHT_CURLY_BRACE);
            break;
        case CKC_O:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_O, layer_on(3), layer_off(3))
            break;
        case CKC_P:
            SMTD_TAP_HOLD_REPEAT(KC_P, KC_GRAVE);
            break;
        case CKC_R:
            SMTD_TAP_HOLD_REPEAT_COMBO(KC_R, KC_LCTL, KC_R);
            break;
        case CKC_S:
            SMTD_TAP_HOLD_REPEAT(KC_S, KC_LEFT_ALT);
            break;
        case CKC_T:
            SMTD_TAP_HOLD_REPEAT(KC_T, KC_LEFT_BRACKET);
            break;
        case CKC_U:
            SMTD_TAP_HOLD_REPEAT(KC_U, KC_QUOTE);
            break;
        case CKC_V:
            SMTD_TAP_HOLD_REPEAT_COMBO(KC_V, KC_LCTL, KC_V);
            break;
        case CKC_X:
            SMTD_TAP_HOLD_REPEAT_COMBO(KC_X, KC_LCTL, KC_X);
            break;
        case CKC_Y:
            SMTD_TAP_HOLD_REPEAT(KC_Y, KC_RIGHT_BRACKET);
            break;
        case CKC_Z:
            SMTD_TAP_HOLD_REPEAT_COMBO(KC_Z, KC_LCTL, KC_Z);
            break;
        case CKC_BACKSPACE:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_BACKSPACE, layer_on(2), layer_off(2))
            break;
        case CKC_COMMA:
            SMTD_TAP_HOLD_REPEAT(KC_COMMA, KC_EQUAL);
            break;
        case CKC_ENTER:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_ENTER, layer_on(1), layer_off(1))
            break;
        case CKC_ESCAPE:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_ESCAPE, layer_on(3), layer_off(3))
            break;
        case CKC_DOT:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_DOT, layer_on(2), layer_off(2))
            break;
        case CKC_SEMICOLON:
            SMTD_TAP_HOLD_REPEAT(KC_SEMICOLON, KC_RIGHT_GUI);
            break;
        case CKC_SLASH:
            SMTD_TAP_HOLD_REPEAT(KC_SLASH, KC_BACKSLASH);
            break;
        case CKC_SPACE:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_SPACE, layer_on(2), layer_off(2))
            break;
        case CKC_TAB:
            SMTD_TAP_HOLD_REPEAT_CUSTOM(KC_TAB, layer_on(1), layer_off(1))
            break;
        default:
            break;
    }
}

// SMTD_MT definition
// void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
//     switch (keycode) {
//         case emulate_mt_macro_key: {
//             switch (action) {
//                 case SMTD_ACTION_TOUCH:
//                     break;

//                 case SMTD_ACTION_TAP:
//                     tap_code16(KEY);
//                     break;

//                 case SMTD_ACTION_HOLD:
//                     switch (tap_count) {
//                         case 0:
//                         case 1:
//                             register_mods(MOD_BIT(MOD));
//                             break;
//                         default:
//                             register_code16(KEY);
//                             break;
//                     }
//                     break;

//                 case SMTD_ACTION_RELEASE:
//                     switch (tap_count) {
//                         case 0:
//                         case 1:
//                             unregister_mods(MOD_BIT(MOD));
//                             break;
//                         default:
//                             unregister_code16(KEY);
//                             break;
//                     }
//                     break;
//               } // end of switch (keycode)
// } // end of on_smtd_action function

// SMTD_LT definition
// void on_smtd_action(uint16_t keycode, smtd_action action, uint8_t tap_count) {
//     switch (keycode) {
//         case emulate_lt_macro_key: {
//             switch (action) {
//                 case SMTD_ACTION_TOUCH:
//                     break;

//                 case SMTD_ACTION_TAP:
//                     tap_code(KEY);
//                     break;

//                 case SMTD_ACTION_HOLD:
//                     switch (tap_count) {
//                         case 0:
//                         case 1:
//                             layer_move(LAYER);
//                             break;
//                         default:
//                             register_code(KEY);
//                             break;
//                     }
//                     break;

//                 case SMTD_ACTION_RELEASE:
//                     switch (tap_count) {
//                         case 0:
//                         case 1:
//                             layer_move(0);
//                             break;
//                         default:
//                             unregister_code(KEY);
//                             break;
//                     }
//                     break;
//             } // end of switch (action)
//             break;

//     } // end of switch (keycode)
// } // end of on_smtd_action function

// end of smtd

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    XXXXXXX  , KC_1         , KC_2         , KC_3         , KC_4        , KC_5         ,                                          KC_6          , KC_7         , KC_8         , KC_9         , KC_0            , XXXXXXX ,
    XXXXXXX  , KC_Q         , KC_W         , CKC_E        , CKC_R       , CKC_T        ,                                          CKC_Y         , CKC_U        , CKC_I        , CKC_O        , CKC_P           , XXXXXXX ,
    XXXXXXX  , CKC_A        , CKC_S        , CKC_D        , CKC_F       , CKC_G        ,                                          CKC_H         , CKC_J        , CKC_K        , CKC_L        , CKC_SEMICOLON   , XXXXXXX ,
    XXXXXXX  , CKC_Z        , CKC_X        , CKC_C        , CKC_V       , CKC_B        , KC_LNG1 ,                    KC_LNG2   , CKC_N         , CKC_M        , CKC_COMMA    , CKC_DOT      , CKC_SLASH        , XXXXXXX ,
    XXXXXXX  , TO(3)        , TO(2)        , TO(1)        , CKC_ESCAPE  , CKC_SPACE    , CKC_TAB ,                    CKC_ENTER , CKC_BACKSPACE , XXXXXXX      , XXXXXXX      , XXXXXXX      , XXXXXXX         , XXXXXXX
  ),
  [1] = LAYOUT_universal(
    XXXXXXX , XXXXXXX  , KC_PEQL , KC_PSLS , KC_PAST , KC_NUM  ,                                            XXXXXXX , XXXXXXX  , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX ,
    XXXXXXX , XXXXXXX  , KC_P7   , KC_P8   , KC_P9   , KC_PMNS ,                                            XXXXXXX , KC_HOME  , KC_END  , KC_PGUP , KC_GRV  , XXXXXXX ,
    XXXXXXX , XXXXXXX  , KC_P4   , KC_P5   , KC_P6   , KC_PPLS ,                                            KC_LEFT , KC_DOWN  , KC_UP   , KC_RGHT , KC_QUOT , XXXXXXX ,
    XXXXXXX , XXXXXXX  , KC_P1   , KC_P2   , KC_P3   , KC_PENT , TO(0)   ,            TO(0)               , XXXXXXX , KC_MINUS , KC_EQL  , KC_PGDN , KC_BSLS , XXXXXXX ,
    XXXXXXX , XXXXXXX  , KC_0    , KC_PDOT , KC_PCMM , XXXXXXX , XXXXXXX ,            DOUBLE_SHIFT_KEY    , KC_DEL  , XXXXXXX  , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX
  ),
  [2] = LAYOUT_universal(
    XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX , XXXXXXX ,                                  XXXXXXX    , XXXXXXX    , XXXXXXX     , XXXXXXX , XXXXXXX , XXXXXXX ,
    XXXXXXX , KC_F1   , KC_F2   , KC_F3   , KC_F4   , XXXXXXX ,                                  A(KC_F4)   , A(KC_LEFT) , A(KC_RIGHT) , XXXXXXX , KC_GRV  , XXXXXXX ,
    XXXXXXX , KC_F5   , KC_F6   , KC_F7   , KC_F8   , XXXXXXX ,                                  C(KC_W)    , KC_BTN1    , KC_BTN3     , KC_BTN2 , KC_QUOT , XXXXXXX ,
    XXXXXXX , KC_F9   , KC_F10  , KC_F11  , KC_F12  , XXXXXXX , TO(0)   ,             TO(0)    , C(S(KC_T)) , KC_PGUP    , KC_PGDN     , XXXXXXX , KC_BSLS , XXXXXXX ,
    XXXXXXX , KC_F13  , KC_F14  , KC_F15  , XXXXXXX , XXXXXXX , XXXXXXX ,             XXXXXXX  , XXXXXXX    , XXXXXXX    , XXXXXXX     , XXXXXXX , XXXXXXX , XXXXXXX
  ),
  [3] = LAYOUT_universal(
    XXXXXXX , XXXXXXX  , XXXXXXX , XXXXXXX  , XXXXXXX  , XXXXXXX ,                              XXXXXXX  , XXXXXXX    , XXXXXXX    , XXXXXXX , XXXXXXX , XXXXXXX ,
    XXXXXXX , KBC_RST  , XXXXXXX , SSNP_HOR , SSNP_VRT , XXXXXXX ,                              KC_INS   , KC_LBRC    , KC_RBRC    , KC_APP  , KC_GRV  , XXXXXXX ,
    XXXXXXX , KBC_SAVE , XXXXXXX , CPI_D100 , CPI_I100 , QK_BOOT ,                              KC_SCRL  , S(KC_9)    , S(KC_0)    , KC_PSCR , KC_QUOT , XXXXXXX ,
    XXXXXXX , XXXXXXX  , XXXXXXX , SCRL_DVD , SCRL_DVI , EE_CLR  , TO(0)   ,          TO(0)   , KC_PAUSE , S(KC_LBRC) , S(KC_RBRC) , XXXXXXX , KC_BSLS , XXXXXXX ,
    XXXXXXX , XXXXXXX  , XXXXXXX , AML_TO   , XXXXXXX  , KC_SPACE  , XXXXXXX ,          XXXXXXX , XXXXXXX  , XXXXXXX    , XXXXXXX    , XXXXXXX , XXXXXXX , XXXXXXX
  ),
};
// clang-format on
