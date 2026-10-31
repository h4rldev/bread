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

#define XIM_ABSOLUTE_POSITION 10

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
 * @brief Converts a UTF-8 character index to a byte offset.
 *
 * @details XIM reports pre-edit positions in characters while the text itself
 * is UTF-8. Walks lead bytes, so an index past the end clamps to the length.
 *
 * @param s The UTF-8 string.
 * @param len The string length in bytes.
 * @param chars The character index to reach.
 *
 * @return The byte offset of the given character index.
 *
 * @pre @p s holds @p len readable bytes.
 */
static u32 xim_utf8_offset(const cstr *s, u32 len, u32 chars) {
  u32 i = 0;
  while (i < len && chars > 0) {
    i++;
    while (i < len && ((u8)s[i] & 0xC0) == 0x80)
      i++;
    chars--;
  }
  return i;
}

//
//
//

/**
 * @brief Fires the current pre-edit state.
 *
 * @details The text points at the state's owned buffer and is valid only
 * during the callback.
 *
 * @param state The x11_state_t.
 * @param begin The selection start byte offset.
 * @param end The selection end byte offset; equals @p begin for a plain caret.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_fire(x11_state_t *state, u32 begin, u32 end) {
  bread_event_t ev = {0};
  ev.type = BREAD_EVENT_TEXT_PREEDIT;
  ev.data.text_preedit.text = state->text_preedit ? state->text_preedit : "";
  ev.data.text_preedit.cursor_begin = begin;
  ev.data.text_preedit.cursor_end = end;
  bread_fire_event(state->window, &ev);
}

//
//
//

/**
 * @brief Clears the pre-edit buffer and reports an empty pre-edit.
 *
 * @details The IM does not always send a final empty draw when a composition
 * ends (focus loss, disable), so the app is told the pre-edit is gone here.
 *
 * @param state The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_reset(x11_state_t *state) {
  b32 had = state->text_preedit_len > 0;
  free(state->text_preedit);
  state->text_preedit = null;
  state->text_preedit_len = 0;
  state->text_preedit_cursor = 0;
  if (had && state->window)
    xim_preedit_fire(state, 0, 0);
}

//
//
//

/**
 * @brief Starts a pre-edit sequence.
 *
 * @details Drops any stale buffer; the following draw event supplies the text.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_start(xcb_xim_t *im, xcb_xic_t ic, void *user_data) {
  (void)im;
  (void)ic;
  x11_state_t *state = user_data;
  if (state)
    xim_preedit_reset(state);
}

//
//
//

/**
 * @brief Applies an incremental pre-edit update and reports the result.
 *
 * @details XIM positions (caret, change start and length) are character
 * indices; the buffer is UTF-8, so they are converted to byte offsets. The
 * changed span is replaced in place, the caret is stored, and the highlighted
 * span is derived from the per-character feedback array. Bitmap pre-edits carry
 * no text and are ignored, as is a pre-edit in a non-UTF-8 encoding.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param frame The pre-edit draw frame.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_draw(xcb_xim_t *im, xcb_xic_t ic,
                             xcb_im_preedit_draw_fr_t *frame, void *user_data) {
  (void)ic;
  x11_state_t *state = user_data;
  if (!state || !state->window || !frame)
    return;
  if (xcb_xim_get_encoding(im) != XCB_XIM_UTF8_STRING)
    return;
  if (frame->status != XCB_IM_TextType)
    return;

  const cstr *old = state->text_preedit ? state->text_preedit : "";
  u32 old_len = state->text_preedit_len;
  u32 first = xim_utf8_offset(old, old_len, frame->chg_first);
  u32 last =
      xim_utf8_offset(old, old_len, frame->chg_first + frame->chg_length);
  u32 new_len =
      frame->preedit_string ? (u32)frame->length_of_preedit_string : 0;

  u32 len = old_len - (last - first) + new_len;
  cstr *buf = malloc((size_t)len + 1);
  if (!buf)
    return;

  memcpy(buf, old, first);
  if (new_len)
    memcpy(buf + first, frame->preedit_string, new_len);
  memcpy(buf + first + new_len, old + last, old_len - last);
  buf[len] = '\0';

  free(state->text_preedit);
  state->text_preedit = buf;
  state->text_preedit_len = len;
  state->text_preedit_cursor = xim_utf8_offset(buf, len, frame->caret);

  u32 begin = state->text_preedit_cursor;
  u32 end = begin;
  u32 run_first = 0;
  u32 run_last = 0;
  b32 in_run = false;
  for (u32 i = 0; frame->feedback_array.items && i < frame->feedback_array.size;
       i++) {
    if (frame->feedback_array.items[i] & XCB_XIM_HIGHLIGHT) {
      if (!in_run) {
        run_first = i;
        in_run = true;
      }
      run_last = i + 1;
    } else if (in_run) {
      break;
    }
  }
  if (in_run) {
    begin = xim_utf8_offset(buf, len, frame->chg_first + run_first);
    end = xim_utf8_offset(buf, len, frame->chg_first + run_last);
  }

  xim_preedit_fire(state, begin, end);
}

//
//
//

/**
 * @brief Moves the pre-edit caret.
 *
 * @details Only an absolute position is meaningful; relative movement
 * commands are ignored. The position is a character index.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param frame The pre-edit caret frame.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_caret(xcb_xim_t *im, xcb_xic_t ic,
                              xcb_im_preedit_caret_fr_t *frame,
                              void *user_data) {
  (void)im;
  (void)ic;
  x11_state_t *state = user_data;
  if (!state || !state->window || !frame || !state->text_preedit)
    return;
  if (frame->direction != XIM_ABSOLUTE_POSITION)
    return;

  state->text_preedit_cursor = xim_utf8_offset(
      state->text_preedit, state->text_preedit_len, frame->position);
  xim_preedit_fire(state, state->text_preedit_cursor,
                   state->text_preedit_cursor);
}

//
//
//

/**
 * @brief Ends a pre-edit sequence.
 *
 * @details Clears the buffer and reports an empty pre-edit, so the app stops
 * drawing the composition.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_preedit_done(xcb_xim_t *im, xcb_xic_t ic, void *user_data) {
  (void)im;
  (void)ic;
  x11_state_t *state = user_data;
  if (state)
    xim_preedit_reset(state);
}

//
//
//

/**
 * @brief Handles committed IME text.
 *
 * @details Copies the UTF-8 commit string into the state and fires
 * BREAD_EVENT_TEXT_COMMIT. The event text is valid during the callback. Only
 * a XCB_XIM_UTF8_STRING negotiation is accepted; the client offers UTF-8
 * alone, so a commit in any other encoding is ignored rather than decoded
 * incorrectly.
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
 * @brief Requests an input context with the given style.
 *
 * @details The spot location starts at the origin and is moved by
 * @ref bread_x11_text_input_enable. The pre-edit attributes always carry a spot
 * location because the IM may draw the pre-edit when it refuses callbacks.
 *
 * @param state The x11_state_t.
 * @param style The XIM input style to request.
 *
 * @pre @p state holds an open XIM.
 */
static void xim_request_ic(x11_state_t *state, u32 style);

//
//
//

/**
 * @brief Checks whether the input context was granted pre-edit callbacks.
 *
 * @details The IM silently downgrades an unsupported style, so the granted
 * style is read back. When callbacks are missing a second context is created
 * with the IM-drawn pre-edit style, which keeps the composition and the
 * candidate window working. The downgraded context is left to the IM: it is
 * never focused, and issuing a second request here would sit in the client's
 * queue forever because a request issued while another is in flight is only
 * sent by the next call into the library.
 *
 * @param im The XIM client.
 * @param ic The input context.
 * @param reply The granted values reply.
 * @param user_data The x11_state_t.
 *
 * @pre All parameters must be valid.
 */
static void xim_ic_values_callback(xcb_xim_t *im, xcb_xic_t ic,
                                   xcb_im_get_ic_values_reply_fr_t *reply,
                                   void *user_data) {
  x11_state_t *state = user_data;
  if (!state || !reply || state->xim_style_checked)
    return;

  state->xim_style_checked = true;

  u32 style = 0;
  if (reply->ic_attribute.size > 0) {
    xcb_im_xicattribute_fr_t *item = &reply->ic_attribute.items[0];
    if (item->value && item->value_length >= sizeof style)
      memcpy(&style, item->value, sizeof style);
  }

  if (style & XCB_IM_PreeditCallbacks) {
    if (state->text_input_enabled)
      xcb_xim_set_ic_focus(im, ic);
    return;
  }

  xim_request_ic(state, XCB_IM_PreeditPosition | XCB_IM_StatusArea);
}

//
//
//

/**
 * @brief Stores the created input context and focuses it if enabled.
 *
 * @details The first context's granted style is validated before it is used,
 * because the IM may have downgraded the requested style.
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
  if (!ic)
    return;

  if (!state->xim_style_checked) {
    xcb_xim_get_ic_values(im, ic, xim_ic_values_callback, state,
                          XCB_XIM_XNInputStyle, null);
    return;
  }

  if (state->text_input_enabled)
    xcb_xim_set_ic_focus(im, ic);
}

//
//
//

/**
 * @brief Requests an input context with the given style.
 *
 * @details The spot location starts at the origin and is moved by
 * @ref bread_x11_text_input_enable. The pre-edit attributes always carry a spot
 * location because the IM may draw the pre-edit when it refuses callbacks.
 *
 * @param state The x11_state_t.
 * @param style The XIM input style to request.
 *
 * @pre @p state holds an open XIM.
 */
static void xim_request_ic(x11_state_t *state, u32 style) {
  xcb_point_t spot = {0, 0};
  xcb_xim_nested_list nested = xcb_xim_create_nested_list(
      state->xim, XCB_XIM_XNSpotLocation, &spot, null);

  xcb_xim_create_ic(
      state->xim, xim_create_ic_callback, state, XCB_XIM_XNInputStyle, &style,
      XCB_XIM_XNClientWindow, &state->xcb_window, XCB_XIM_XNFocusWindow,
      &state->xcb_window, XCB_XIM_XNPreeditAttributes, &nested, null);
  free(nested.data);
}

//
//
//

/**
 * @brief Creates the input context once the IM is open.
 *
 * @details Requests client-drawn pre-edit callbacks so the app renders the
 * composition inline, falling back to an IM-drawn pre-edit when the IM does not
 * grant the style.
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

  xim_request_ic(state, XCB_IM_PreeditCallbacks | XCB_IM_StatusArea);
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
    .preedit_start = xim_preedit_start,
    .preedit_draw = xim_preedit_draw,
    .preedit_caret = xim_preedit_caret,
    .preedit_done = xim_preedit_done,
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
  xcb_xim_set_use_compound_text(state->xim, false);
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
  else {
    xcb_xim_unset_ic_focus(state->xim, state->xim_ic);
    xim_preedit_reset(state);
  }
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
  free(state->text_preedit);

  state->text_commit = null;
  state->text_preedit = null;
  state->text_preedit_len = 0;
  state->text_preedit_cursor = 0;
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

  xim_preedit_reset(state);
}

#endif // !BREAD_X11
