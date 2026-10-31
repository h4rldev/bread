/*************************************************/

#include <bread/x11/x11_input.h>

#if BREAD_X11

#include <stdlib.h>
#include <string.h>

#include <xcb-imdkit/imclient.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xkb.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xkbcommon/xkbcommon.h>

#include <bread/event.h>
#include <bread/input.h>
#include <bread/log.h>
#include <bread/window.h>

#include <bread/x11/x11.h>

/*************************************************/

/**
 * @brief Handles a key the IM forwarded back to the client.
 *
 * @details Keys are handed to the IM with synchronous forwarding; the IM
 * commits the ones it consumed and forwards the rest back here. Runs the same
 * path as a normal key event (state update plus the key event), so the client
 * sees each key exactly once.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param event The key event.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_forward_event(xcb_xim_t *im, xcb_xic_t ic,
                              xcb_key_press_event_t *event, void *user_data) {
  (void)im;
  (void)ic;
  x11_state_t *state = user_data;
  if (!state || !state->window || !event)
    return;

  u8 type = event->response_type & ~0x80;
  bread_event_t ev = {0};
  if (type == XCB_KEY_PRESS) {
    bread_x11_handle_key_press(state, event);
    ev.type = BREAD_EVENT_KEY_PRESS;
  } else if (type == XCB_KEY_RELEASE) {
    bread_x11_handle_key_release(state, (xcb_key_release_event_t *)event);
    ev.type = BREAD_EVENT_KEY_RELEASE;
  } else {
    return;
  }
  ev.data.key.key = bread_evdev_to_key(event->detail - 8);
  ev.data.key.raw_keycode = event->detail;
  bread_fire_event(state->window, &ev);
}

//
//
//

/**
 * @brief Handles committed IME text.
 *
 * @details Copies the UTF-8 commit string into the state and fires
 * BREAD_EVENT_TEXT_COMMIT. The event text is valid during the callback.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param flag The commit flag.
 * @param str The committed text.
 * @param length The text length in bytes.
 * @param keysym Unused keysyms.
 * @param n_keysym Number of keysyms.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_commit_string(xcb_xim_t *im, xcb_xic_t ic, u32 flag, char *str,
                              u32 length, u32 *keysym, size_t n_keysym,
                              void *user_data) {
  (void)ic;
  (void)flag;
  (void)keysym;
  (void)n_keysym;

  x11_state_t *state = user_data;
  if (!state || !state->window || !str || length == 0)
    return;
  if (xcb_xim_get_encoding(im) != XCB_XIM_UTF8_STRING)
    return;

  free(state->text_commit);
  state->text_commit = malloc((size_t)length + 1);
  if (!state->text_commit)
    return;

  memcpy(state->text_commit, str, length);
  state->text_commit[length] = '\0';

  bread_event_t ev = {0};
  ev.type = BREAD_EVENT_TEXT_COMMIT;
  ev.data.text_commit.text = state->text_commit;
  bread_fire_event(state->window, &ev);
}

//
//
//

/**
 * @brief Handles the IM disconnecting.
 *
 * @param im The XIM client.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_disconnected(xcb_xim_t *im, void *user_data) {
  (void)im;
  x11_state_t *state = user_data;
  if (state)
    state->xim_ic = 0;
}

//
//
//

/**
 * @brief Stores the created input context and focuses it if enabled.
 *
 * @param im The XIM client.
 * @param ic The new input context, or 0 on failure.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_create_ic_callback(xcb_xim_t *im, xcb_xic_t ic,
                                   void *user_data) {
  x11_state_t *state = user_data;
  if (!state)
    return;

  state->xim_ic = ic;
  if (ic && state->text_input_enabled)
    xcb_xim_set_ic_focus(im, ic);
}

//
//
//

/**
 * @brief Creates the input context once the IM is open.
 *
 * @details Uses PreeditPosition | StatusArea so the IM draws its own pre-edit
 * and status window at the spot location; the client only receives commits.
 *
 * @param im The XIM client.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_open_callback(xcb_xim_t *im, void *user_data) {
  x11_state_t *state = user_data;
  if (!state)
    return;

  u32 input_style = XCB_IM_PreeditPosition | XCB_IM_StatusArea;
  xcb_point_t spot = {0, 0};
  xcb_xim_nested_list nested =
      xcb_xim_create_nested_list(im, XCB_XIM_XNSpotLocation, &spot, null);

  xcb_xim_create_ic(im, xim_create_ic_callback, state, XCB_XIM_XNInputStyle,
                    &input_style, XCB_XIM_XNClientWindow, &state->xcb_window,
                    XCB_XIM_XNFocusWindow, &state->xcb_window,
                    XCB_XIM_XNPreeditAttributes, &nested, null);
  free(nested.data);
}

//
//
//

/**
 * @brief The XIM client callbacks.
 *
 * @details Handles commits, forwarded keys, and IM disconnection.
 */
static const xcb_xim_im_callback xim_callbacks = {
    .forward_event = xim_forward_event,
    .commit_string = xim_commit_string,
    .disconnected = xim_disconnected,
};

//
//
//

b32 bread_x11_xkb_init(x11_state_t *state) {
  bread_log_debug("Initializing X11 xkb");
  state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!state->xkb_context) {
    bread_log_fatal("Failed to create xkb context");
    return false;
  }

  u8 unused = 0;
  if (!xkb_x11_setup_xkb_extension(
          state->connection, XKB_X11_MIN_MAJOR_XKB_VERSION,
          XKB_X11_MIN_MINOR_XKB_VERSION, XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
          null, null, &unused, &unused)) {
    bread_log_fatal("Failed to setup xkb extension");
    xkb_context_unref(state->xkb_context);
    state->xkb_context = null;
    return false;
  }

  state->xkb_device_id = xkb_x11_get_core_keyboard_device_id(state->connection);
  if (state->xkb_device_id < 0) {
    bread_log_fatal("Failed to get xkb device id");
    xkb_context_unref(state->xkb_context);

    state->xkb_context = null;
    return false;
  }

  state->xkb_keymap = xkb_x11_keymap_new_from_device(
      state->xkb_context, state->connection, state->xkb_device_id,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (!state->xkb_keymap) {
    bread_log_fatal("Failed to create xkb keymap");
    xkb_context_unref(state->xkb_context);

    state->xkb_context = null;
    return false;
  }

  state->xkb_state = xkb_x11_state_new_from_device(
      state->xkb_keymap, state->connection, state->xkb_device_id);
  if (!state->xkb_state) {
    bread_log_fatal("Failed to create xkb state");
    xkb_keymap_unref(state->xkb_keymap);
    xkb_context_unref(state->xkb_context);

    state->xkb_keymap = null;
    state->xkb_context = null;
    return false;
  }

  bread_log_debug("Successfully initialized X11 xkb");
  return true;
}

bread_mouse_button_t xcb_button_to_bread(u8 detail) {
  switch (detail) {
  case 1:
    bread_log_debug("Got mouse button left");
    return BREAD_MOUSE_BUTTON_LEFT;
  case 2:
    bread_log_debug("Got mouse button middle");
    return BREAD_MOUSE_BUTTON_MIDDLE;
  case 3:
    bread_log_debug("Got mouse button right");
    return BREAD_MOUSE_BUTTON_RIGHT;
  case 4:
    bread_log_debug("Got mouse button 4");
    return BREAD_MOUSE_BUTTON_4;
  case 5:
    bread_log_debug("Got mouse button 5");
    return BREAD_MOUSE_BUTTON_5;
  case 8:
    bread_log_debug("Got mouse button 4");
    return BREAD_MOUSE_BUTTON_4;
  case 9:
    bread_log_debug("Got mouse button 5");
    return BREAD_MOUSE_BUTTON_5;
  default:
    bread_log_debug("Got unknown mouse button");
    return BREAD_MOUSE_BUTTON_MAX;
  }
}

void bread_x11_handle_key_press(x11_state_t *state,
                                xcb_key_press_event_t *event) {
  bread_log_debug("Handling key press");

  u32 evdev = event->detail - 8;
  bread_key_t bread_key = bread_evdev_to_key(evdev);
  if (bread_key >= BREAD_KEY_MAX) {
    bread_log_debug("Got unknown key, ignoring");
    return;
  }

  state->input.keys[bread_key] = true;
  if (state->xkb_state) {
    bread_log_debug("Updating xkb state to DOWN");
    xkb_state_update_key(state->xkb_state, event->detail, XKB_KEY_DOWN);
  }
}

void bread_x11_handle_key_release(x11_state_t *state,
                                  xcb_key_release_event_t *event) {
  bread_log_debug("Handling key release");
  u32 evdev = event->detail - 8;
  bread_key_t bread_key = bread_evdev_to_key(evdev);
  if (bread_key >= BREAD_KEY_MAX) {
    bread_log_debug("Got unknown key, ignoring");
    return;
  }

  state->input.keys[bread_key] = false;
  if (state->xkb_state) {
    bread_log_debug("Updating xkb state to UP");
    xkb_state_update_key(state->xkb_state, event->detail, XKB_KEY_UP);
  }
}

void bread_x11_handle_button_press(x11_state_t *state,
                                   xcb_button_press_event_t *event) {
  bread_log_debug("Handling mouse button press");
  if (event->detail == 4) {
    bread_log_debug("Scrolling up");
    state->input.scroll_y += 1.0f;
    return;
  }

  if (event->detail == 5) {
    bread_log_debug("Scrolling down");
    state->input.scroll_y -= 1.0f;
    return;
  }

  bread_log_debug("Emitting mouse press");
  bread_mouse_button_t bread_button = xcb_button_to_bread(event->detail);
  if (bread_button < BREAD_MOUSE_BUTTON_MAX) {
    bread_log_debug("Emitting mouse press for button %d", bread_button);
    state->input.mouse_buttons[bread_button] = true;
  }
}

void bread_x11_handle_button_release(x11_state_t *state,
                                     xcb_button_release_event_t *event) {
  if (event->detail == 4 || event->detail == 5) {
    bread_log_debug("Got scroll release, ignoring");
    return;
  }

  bread_mouse_button_t bread_button = xcb_button_to_bread(event->detail);
  if (bread_button < BREAD_MOUSE_BUTTON_MAX) {
    bread_log_debug("Emitting mouse release for button %d", bread_button);
    state->input.mouse_buttons[bread_button] = false;
  }
}

void bread_x11_handle_motion(x11_state_t *state,
                             xcb_motion_notify_event_t *event) {
  bread_log_debug("Handling motion");
  f64 scale = state->scale > 0.0f ? (f64)state->scale : 1.0;
  state->input.mouse_x = (f64)event->event_x / scale;
  state->input.mouse_y = (f64)event->event_y / scale;
}

void bread_x11_cursor_init(x11_state_t *state) {
  if (!state || !state->connection || !state->screen) {
    bread_log_error(
        "No state, X11 connection or screen, can't initialize cursor");
    return;
  }

  if (xcb_cursor_context_new(state->connection, state->screen,
                             &state->cursor_context) != 0) {
    bread_log_error(
        "Failed to create cursor context, not allowing cursor changes");
    state->cursor_context = null;
    return;
  }

  static const char *cursor_names[] = {
      [BREAD_CURSOR_DEFAULT] = "default",
      [BREAD_CURSOR_POINTER] = "pointer",
      [BREAD_CURSOR_HAND] = "grabbing",
      [BREAD_CURSOR_TEXT] = "text",
      [BREAD_CURSOR_MOVE] = "move",
      [BREAD_CURSOR_RESIZE_EW] = "ew-resize",
      [BREAD_CURSOR_RESIZE_NS] = "ns-resize",
      [BREAD_CURSOR_RESIZE_NESW] = "nesw-resize",
      [BREAD_CURSOR_RESIZE_NWSE] = "nwse-resize",
      [BREAD_CURSOR_NOT_ALLOWED] = "not-allowed",
      [BREAD_CURSOR_WAIT] = "wait",
  };

  static const char *cursor_fallbacks[] = {
      [BREAD_CURSOR_DEFAULT] = "left_ptr",
      [BREAD_CURSOR_POINTER] = "hand1",
      [BREAD_CURSOR_HAND] = "hand2",
      [BREAD_CURSOR_TEXT] = "xterm",
      [BREAD_CURSOR_MOVE] = "fleur",
      [BREAD_CURSOR_RESIZE_EW] = "sb_h_double_arrow",
      [BREAD_CURSOR_RESIZE_NS] = "sb_v_double_arrow",
      [BREAD_CURSOR_RESIZE_NESW] = "fd_double_arrow",
      [BREAD_CURSOR_RESIZE_NWSE] = "bd_double_arrow",
      [BREAD_CURSOR_NOT_ALLOWED] = "x_cursor",
      [BREAD_CURSOR_WAIT] = "watch",
  };

  for (u32 i = 0; i < BREAD_CURSOR_MAX; i++) {
    state->cursors[i] =
        xcb_cursor_load_cursor(state->cursor_context, cursor_names[i]);
    if (!state->cursors[i]) {
      bread_log_warn("Failed to load cursor %s, falling back to %s",
                     cursor_names[i], cursor_fallbacks[i]);
      state->cursors[i] =
          xcb_cursor_load_cursor(state->cursor_context, cursor_fallbacks[i]);
    }
  }

  state->current_cursor = state->cursors[BREAD_CURSOR_DEFAULT];
  u32 values[] = {state->current_cursor};

  xcb_change_window_attributes(state->connection, state->xcb_window,
                               XCB_CW_CURSOR, values);
  xcb_flush(state->connection);
}

void bread_x11_cursor_cleanup(x11_state_t *state) {
  if (!state) {
    bread_log_error("Missing values, can't cleanup cursor");
    return;
  }

  if (state->cursor_context)
    xcb_cursor_context_free(state->cursor_context);

  state->cursor_context = null;
}

void bread_x11_set_cursor(x11_state_t *state, bread_cursor_type_t cursor) {
  if (!state || !state->cursor_context || !state->connection ||
      !state->xcb_window) {
    bread_log_error("Missing values, can't set cursor");
    return;
  }

  if (cursor >= BREAD_CURSOR_MAX) {
    bread_log_error("Invalid cursor type, can't set cursor");
    return;
  }

  if (state->current_cursor == state->cursors[cursor])
    return;

  xcb_cursor_t new_cursor = state->cursors[cursor];
  u32 values[] = {new_cursor};
  xcb_change_window_attributes(state->connection, state->xcb_window,
                               XCB_CW_CURSOR, values);
  xcb_flush(state->connection);
  state->current_cursor = new_cursor;
}

b32 bread_x11_ime_init(x11_state_t *state) {
  if (!state || !state->connection)
    return false;

  state->xim = xcb_xim_create(state->connection, state->screen_number, null);
  if (!state->xim) {
    bread_log_error("Failed to create XIM");
    return false;
  }

  xcb_xim_set_im_callback(state->xim, &xim_callbacks, state);
  xcb_xim_set_use_utf8_string(state->xim, true);

  if (!xcb_xim_open(state->xim, xim_open_callback, true, state)) {
    bread_log_error("Failed to open XIM");
    xcb_xim_destroy(state->xim);
    state->xim = null;
    return false;
  }

  return true;
}

b32 bread_x11_ime_filter(x11_state_t *state, xcb_generic_event_t *event) {
  if (!state || !state->xim || !event)
    return false;

  if (xcb_xim_filter_event(state->xim, event))
    return true;

  u8 type = event->response_type & ~0x80;
  if (state->xim_ic && state->text_input_enabled &&
      (type == XCB_KEY_PRESS || type == XCB_KEY_RELEASE)) {
    xcb_xim_forward_event(state->xim, state->xim_ic,
                          (xcb_key_press_event_t *)event);
    return true;
  }
  return false;
}

void bread_x11_ime_focus(x11_state_t *state, b32 focused) {
  if (!state || !state->xim || !state->xim_ic)
    return;

  if (focused && state->text_input_enabled)
    xcb_xim_set_ic_focus(state->xim, state->xim_ic);
  else
    xcb_xim_unset_ic_focus(state->xim, state->xim_ic);
}

void bread_x11_ime_cleanup(x11_state_t *state) {
  if (!state)
    return;

  if (state->xim) {
    if (state->xim_ic)
      xcb_xim_destroy_ic(state->xim, state->xim_ic, null, null);
    xcb_xim_close(state->xim);
    xcb_xim_destroy(state->xim);
    state->xim = null;
    state->xim_ic = 0;
  }

  free(state->text_commit);
  state->text_commit = null;
}

void bread_x11_text_input_enable(x11_state_t *state, f32 x, f32 y, f32 w,
                                 f32 h) {
  (void)w;
  (void)h;
  if (!state)
    return;

  state->text_input_enabled = true;
  if (!state->xim || !state->xim_ic)
    return;

  xcb_xim_set_ic_focus(state->xim, state->xim_ic);
  xcb_xim_ext_move(state->xim, state->xim_ic, (i16)x, (i16)y);
}

void bread_x11_text_input_disable(x11_state_t *state) {
  if (!state)
    return;

  state->text_input_enabled = false;
  if (state->xim && state->xim_ic)
    xcb_xim_unset_ic_focus(state->xim, state->xim_ic);
}

#endif // !BREAD_X11
