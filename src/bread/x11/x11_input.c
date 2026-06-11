#include <bread/x11/x11.h>

#if BREAD_X11

#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

#include <bread/event.h>
#include <bread/input.h>
#include <bread/window.h>
#include <bread/x11/x11_input.h>

b32 bread_x11_xkb_init(x11_state_t *state) {
  state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!state->xkb_context)
    return false;

  u8 unused = 0;
  if (!xkb_x11_setup_xkb_extension(
          state->connection, XKB_X11_MIN_MAJOR_XKB_VERSION,
          XKB_X11_MIN_MINOR_XKB_VERSION, XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
          null, null, &unused, &unused)) {
    fprintf(stderr, "bread-x11: xkb x11 extension setup failed\n");
    return false;
  }

  state->xkb_device_id = xkb_x11_get_core_keyboard_device_id(state->connection);
  if (state->xkb_device_id < 0) {
    fprintf(stderr, "bread-x11: failed to get xkb device id\n");
    return false;
  }

  state->xkb_keymap = xkb_x11_keymap_new_from_device(
      state->xkb_context, state->connection, state->xkb_device_id,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (!state->xkb_keymap) {
    fprintf(stderr, "bread-x11: failed to create xkb keymap\n");
    return false;
  }

  state->xkb_state = xkb_x11_state_new_from_device(
      state->xkb_keymap, state->connection, state->xkb_device_id);
  if (!state->xkb_state) {
    fprintf(stderr, "bread-x11: failed to create xkb state\n");
    return false;
  }

  return true;
}

void bread_x11_xkb_cleanup(x11_state_t *state) {
  xkb_state_unref(state->xkb_state);
  xkb_keymap_unref(state->xkb_keymap);
  xkb_context_unref(state->xkb_context);
}

bread_mouse_button_t xcb_button_to_bread(u8 detail) {
  switch (detail) {
  case 1:
    return BREAD_MOUSE_BUTTON_LEFT;
  case 2:
    return BREAD_MOUSE_BUTTON_MIDDLE;
  case 3:
    return BREAD_MOUSE_BUTTON_RIGHT;
  case 4:
    return BREAD_MOUSE_BUTTON_4;
  case 5:
    return BREAD_MOUSE_BUTTON_5;
  case 8:
    return BREAD_MOUSE_BUTTON_4;
  case 9:
    return BREAD_MOUSE_BUTTON_5;
  default:
    return BREAD_MOUSE_BUTTON_MAX;
  }
}

void bread_x11_handle_key_press(x11_state_t *state,
                                xcb_key_press_event_t *event) {
  u32 evdev = event->detail - 8;
  bread_key_t bread_key = bread_evdev_to_key(evdev);

  state->input.keys[bread_key] = true;

  if (state->xkb_state)
    xkb_state_update_key(state->xkb_state, event->detail, XKB_KEY_DOWN);
}

void bread_x11_handle_key_release(x11_state_t *state,
                                  xcb_key_release_event_t *event) {
  u32 evdev = event->detail - 8;
  bread_key_t bread_key = bread_evdev_to_key(evdev);
  state->input.keys[bread_key] = false;

  if (state->xkb_state)
    xkb_state_update_key(state->xkb_state, event->detail, XKB_KEY_UP);
}

void bread_x11_handle_button_press(x11_state_t *state,
                                   xcb_button_press_event_t *event) {
  if (event->detail == 4) {
    state->input.scroll_y -= 1.0f;
    return;
  }

  if (event->detail == 5) {
    state->input.scroll_y += 1.0f;
    return;
  }

  bread_mouse_button_t bread_button = xcb_button_to_bread(event->detail);
  if (bread_button < BREAD_MOUSE_BUTTON_MAX)
    state->input.mouse_buttons[bread_button] = true;
}

void bread_x11_handle_button_release(x11_state_t *state,
                                     xcb_button_release_event_t *event) {
  if (event->detail == 4 || event->detail == 5)
    return;

  bread_mouse_button_t bread_button = xcb_button_to_bread(event->detail);
  if (bread_button < BREAD_MOUSE_BUTTON_MAX)
    state->input.mouse_buttons[bread_button] = false;
}

void bread_x11_handle_motion(x11_state_t *state,
                             xcb_motion_notify_event_t *event) {
  state->input.mouse_x = (f64)event->event_x;
  state->input.mouse_y = (f64)event->event_y;
}
#endif // !BREAD_X11
