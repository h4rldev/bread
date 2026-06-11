#include <xkbcommon/xkbcommon.h>

#include <bread/input.h>
#include <bread/window.h>

#if BREAD_WAYLAND
#include <bread/wayland/wayland.h>
#endif

#if BREAD_X11
#include <bread/x11/x11.h>
#endif

#include <bread/event.h>

static const bread_key_t evdev_to_bread[] = {
    [0] = BREAD_KEY_UNKNOWN,
    [1] = BREAD_KEY_ESCAPE,
    [2] = BREAD_KEY_1,
    [3] = BREAD_KEY_2,
    [4] = BREAD_KEY_3,
    [5] = BREAD_KEY_4,
    [6] = BREAD_KEY_5,
    [7] = BREAD_KEY_6,
    [8] = BREAD_KEY_7,
    [9] = BREAD_KEY_8,
    [10] = BREAD_KEY_9,
    [11] = BREAD_KEY_0,
    [12] = BREAD_KEY_MINUS,
    [13] = BREAD_KEY_EQUAL,
    [14] = BREAD_KEY_BACKSPACE,
    [15] = BREAD_KEY_TAB,
    [16] = BREAD_KEY_Q,
    [17] = BREAD_KEY_W,
    [18] = BREAD_KEY_E,
    [19] = BREAD_KEY_R,
    [20] = BREAD_KEY_T,
    [21] = BREAD_KEY_Y,
    [22] = BREAD_KEY_U,
    [23] = BREAD_KEY_I,
    [24] = BREAD_KEY_O,
    [25] = BREAD_KEY_P,
    [26] = BREAD_KEY_LEFT_BRACKET,
    [27] = BREAD_KEY_RIGHT_BRACKET,
    [28] = BREAD_KEY_ENTER,
    [29] = BREAD_KEY_LEFT_CTRL,
    [30] = BREAD_KEY_A,
    [31] = BREAD_KEY_S,
    [32] = BREAD_KEY_D,
    [33] = BREAD_KEY_F,
    [34] = BREAD_KEY_G,
    [35] = BREAD_KEY_H,
    [36] = BREAD_KEY_J,
    [37] = BREAD_KEY_K,
    [38] = BREAD_KEY_L,
    [39] = BREAD_KEY_SEMICOLON,
    [40] = BREAD_KEY_APOSTROPHE,
    [41] = BREAD_KEY_GRAVE,
    [42] = BREAD_KEY_LEFT_SHIFT,
    [43] = BREAD_KEY_BACKSLASH,
    [44] = BREAD_KEY_Z,
    [45] = BREAD_KEY_X,
    [46] = BREAD_KEY_C,
    [47] = BREAD_KEY_V,
    [48] = BREAD_KEY_B,
    [49] = BREAD_KEY_N,
    [50] = BREAD_KEY_M,
    [51] = BREAD_KEY_COMMA,
    [52] = BREAD_KEY_PERIOD,
    [53] = BREAD_KEY_SLASH,
    [54] = BREAD_KEY_RIGHT_SHIFT,
    [55] = BREAD_KEY_KP_MULTIPLY,
    [56] = BREAD_KEY_LEFT_ALT,
    [57] = BREAD_KEY_SPACE,
    [58] = BREAD_KEY_CAPS_LOCK,
    [59] = BREAD_KEY_F1,
    [60] = BREAD_KEY_F2,
    [61] = BREAD_KEY_F3,
    [62] = BREAD_KEY_F4,
    [63] = BREAD_KEY_F5,
    [64] = BREAD_KEY_F6,
    [65] = BREAD_KEY_F7,
    [66] = BREAD_KEY_F8,
    [67] = BREAD_KEY_F9,
    [68] = BREAD_KEY_F10,
    [69] = BREAD_KEY_NUM_LOCK,
    [70] = BREAD_KEY_SCROLL_LOCK,
    [71] = BREAD_KEY_KP_7,
    [72] = BREAD_KEY_KP_8,
    [73] = BREAD_KEY_KP_9,
    [74] = BREAD_KEY_KP_MINUS,
    [75] = BREAD_KEY_KP_4,
    [76] = BREAD_KEY_KP_5,
    [77] = BREAD_KEY_KP_6,
    [78] = BREAD_KEY_KP_PLUS,
    [79] = BREAD_KEY_KP_1,
    [80] = BREAD_KEY_KP_2,
    [81] = BREAD_KEY_KP_3,
    [82] = BREAD_KEY_KP_0,
    [83] = BREAD_KEY_KP_PERIOD,
    [86] = BREAD_KEY_NON_US_BACKSLASH,
    [87] = BREAD_KEY_F11,
    [88] = BREAD_KEY_F12,
    [85] = BREAD_KEY_ZENKAKU_HANKAKU,
    [89] = BREAD_KEY_YEN, /* KEY_RO */
    [90] = BREAD_KEY_KATAKANA_HIRAGANA,
    [91] = BREAD_KEY_HENKAN,
    [92] = BREAD_KEY_KATAKANA_HIRAGANA,
    [93] = BREAD_KEY_HENKAN,
    [94] = BREAD_KEY_MUHENKAN,
    [95] = BREAD_KEY_KP_COMMA,
    [96] = BREAD_KEY_ENTER, /* KEY_KPENTER */
    [97] = BREAD_KEY_RIGHT_CTRL,
    [98] = BREAD_KEY_KP_DIVIDE,
    [99] = BREAD_KEY_PRINT_SCREEN,
    [100] = BREAD_KEY_RIGHT_ALT,
    [102] = BREAD_KEY_HOME,
    [103] = BREAD_KEY_UP,
    [104] = BREAD_KEY_PAGE_UP,
    [105] = BREAD_KEY_LEFT,
    [106] = BREAD_KEY_RIGHT,
    [107] = BREAD_KEY_END,
    [108] = BREAD_KEY_DOWN,
    [109] = BREAD_KEY_PAGE_DOWN,
    [110] = BREAD_KEY_INSERT,
    [111] = BREAD_KEY_DELETE,
    [119] = BREAD_KEY_PAUSE_BREAK,
    [121] = BREAD_KEY_KP_COMMA,
    [122] = BREAD_KEY_HANGEUL,
    [123] = BREAD_KEY_HANJA,
    [125] = BREAD_KEY_LEFT_SUPER,
    [126] = BREAD_KEY_RIGHT_SUPER,
    [127] = BREAD_KEY_MENU,
};

#define EVDEV_TO_BREAD_MAX 128

bread_key_t bread_evdev_to_bread(u32 evdev_key) {
  if (evdev_key < EVDEV_TO_BREAD_MAX)
    return evdev_to_bread[evdev_key];
  return BREAD_KEY_UNKNOWN;
}

bread_mouse_button_t bread_evdev_to_mouse_button(u32 button) {
  switch (button) {
  case 0x110:
    return BREAD_MOUSE_BUTTON_LEFT;
  case 0x111:
    return BREAD_MOUSE_BUTTON_RIGHT;
  case 0x112:
    return BREAD_MOUSE_BUTTON_MIDDLE;
  case 0x113:
    return BREAD_MOUSE_BUTTON_4;
  case 0x114:
    return BREAD_MOUSE_BUTTON_5;
  default:
    return BREAD_MOUSE_BUTTON_MAX;
  }
}

bread_input_state_t bread_window_get_input(bread_window_t *window) {
#if BREAD_WAYLAND
  wl_state_t *state = window->backend;
  return state->input;
#elif BREAD_X11
  x11_state_t *state = window->backend;
  return state->input;
#endif
}

u32 bread_event_key_to_unicode(bread_window_t *window, bread_event_t *event) {
  if (event->type != BREAD_EVENT_KEY_PRESS &&
      event->type != BREAD_EVENT_KEY_RELEASE)
    return 0;

#if BREAD_WAYLAND
  wl_state_t *state = window->backend;
  if (!state->xkb_state)
    return 0;

  xkb_keycode_t keycode = event->data.key.raw_keycode + 8;
  return xkb_keysym_to_utf32(
      xkb_state_key_get_one_sym(state->xkb_state, keycode));
#elif BREAD_X11
  x11_state_t *state = window->backend;
  if (!state->xkb_state)
    return 0;
  xkb_keycode_t keycode = event->data.key.raw_keycode;
  return xkb_keysym_to_utf32(
      xkb_state_key_get_one_sym(state->xkb_state, keycode));
#endif
}
