/*************************************************/

#include <bread/x11/x11_clipboard.h>

#if BREAD_X11

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <bread/log.h>

/*************************************************/

void bread_x11_clipboard_set(x11_state_t *state, const cstr *text) {
  if (!state || !state->connection)
    return;

  free(state->clipboard);
  state->clipboard = null;
  state->clipboard_len = 0;

  if (text && *text) {
    u32 len = (u32)strlen(text);
    state->clipboard = malloc(len + 1);
    if (!state->clipboard)
      return;
    memcpy(state->clipboard, text, len + 1);
    state->clipboard_len = len;
  }

  xcb_set_selection_owner(state->connection,
                          state->clipboard ? state->xcb_window : XCB_NONE,
                          state->clipboard_atom, XCB_CURRENT_TIME);
  state->owns_clipboard = state->clipboard != null;
  xcb_flush(state->connection);
}

const cstr *bread_x11_clipboard_get(x11_state_t *state) {
  if (!state || !state->connection)
    return null;

  if (state->owns_clipboard && state->clipboard)
    return state->clipboard;

  free(state->clipboard_read);
  state->clipboard_read = null;

  xcb_convert_selection(state->connection, state->xcb_window,
                        state->clipboard_atom, state->utf8_atom,
                        state->prop_atom, XCB_CURRENT_TIME);
  xcb_flush(state->connection);

  b32 ready = false;
  xcb_generic_event_t *event;
  while ((event = xcb_wait_for_event(state->connection))) {
    u8 type = event->response_type & ~0x80;
    if (type == XCB_SELECTION_NOTIFY) {
      xcb_selection_notify_event_t *notify =
          (xcb_selection_notify_event_t *)event;
      ready = notify->property != XCB_NONE &&
              notify->selection == state->clipboard_atom;
      free(event);
      break;
    }
    free(event);
  }

  if (!ready)
    return null;

  xcb_get_property_cookie_t cookie = xcb_get_property(
      state->connection, false, state->xcb_window, state->prop_atom,
      XCB_GET_PROPERTY_TYPE_ANY, 0, UINT32_MAX);
  xcb_get_property_reply_t *reply =
      xcb_get_property_reply(state->connection, cookie, null);
  if (!reply)
    return null;

  int len = xcb_get_property_value_length(reply);
  if (len > 0) {
    char *buf = malloc((size_t)len + 1);
    if (buf) {
      memcpy(buf, xcb_get_property_value(reply), (size_t)len);
      buf[len] = '\0';
      state->clipboard_read = buf;
    }
  }

  free(reply);
  xcb_delete_property(state->connection, state->xcb_window, state->prop_atom);
  return state->clipboard_read;
}

void bread_x11_clipboard_cleanup(x11_state_t *state) {
  if (!state)
    return;

  free(state->clipboard);
  state->clipboard = null;
  state->clipboard_len = 0;
  free(state->clipboard_read);
  state->clipboard_read = null;
  state->owns_clipboard = false;
}

void bread_x11_clipboard_request(x11_state_t *state,
                                 xcb_selection_request_event_t *request) {
  if (!state || !request)
    return;

  xcb_selection_notify_event_t notify = {
      .response_type = XCB_SELECTION_NOTIFY,
      .time = request->time,
      .requestor = request->requestor,
      .selection = request->selection,
      .target = request->target,
      .property = XCB_NONE,
  };

  if (state->clipboard && request->selection == state->clipboard_atom) {
    xcb_atom_t prop =
        request->property != XCB_NONE ? request->property : request->target;

    if (request->target == state->targets_atom) {
      xcb_atom_t targets[] = {state->targets_atom, state->utf8_atom,
                              XCB_ATOM_STRING};
      xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE,
                          request->requestor, prop, XCB_ATOM_ATOM, 32, 3,
                          targets);
      notify.property = prop;
    } else if (request->target == state->utf8_atom ||
               request->target == XCB_ATOM_STRING) {
      xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE,
                          request->requestor, prop, request->target, 8,
                          state->clipboard_len, state->clipboard);
      notify.property = prop;
    }
  }

  xcb_send_event(state->connection, false, request->requestor,
                 XCB_EVENT_MASK_NO_EVENT, (const char *)&notify);
  xcb_flush(state->connection);
}

void bread_x11_clipboard_clear(x11_state_t *state,
                               xcb_selection_clear_event_t *clear) {
  if (!state || !clear)
    return;

  if (clear->selection == state->clipboard_atom) {
    state->owns_clipboard = false;
    free(state->clipboard);
    state->clipboard = null;
    state->clipboard_len = 0;
  }
}

#endif // BREAD_X11
