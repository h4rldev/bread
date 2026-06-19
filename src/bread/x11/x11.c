#include <bread/x11/x11.h>

#if BREAD_X11

#include <stdlib.h>
#include <string.h>

#include <htils/arena.h>
#include <htils/string.h>

#include <bread/backend.h>
#include <bread/log.h>
#include <bread/types.h>
#include <bread/window.h>
#include <bread/x11/x11_input.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>

static xcb_atom_t intern_atom(xcb_connection_t *conn, const cstr *name) {
  bread_log_debug("Interning atom");

  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(conn, 0, strlen(name), name);
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(conn, cookie, NULL);

  xcb_atom_t atom = XCB_ATOM_NONE;
  if (reply) {
    bread_log_debug("Got atom");
    atom = reply->atom;
    free(reply);
  }

  return atom;
}

static void x11_state_cleanup(x11_state_t *state) {
  bread_log_debug("Cleaning up x11 state");
  if (state->xcb_window) {
    bread_log_debug("Destroying window");
    xcb_destroy_window(state->connection, state->xcb_window);
  }
  if (state->connection) {
    bread_log_debug("Disconnecting from X server");
    xcb_disconnect(state->connection);
  }
}

static void x11_init(bread_window_t *window) {
  bread_log_debug("Initializing x11 window");
  arena_t *arena = window->arena;
  x11_state_t *state = arena_alloc(arena, x11_state_t, 1);
  window->backend = state;
  state->window = window;
  string *class = {0};

  if (window->class) {
    class = string_new(arena, window->class->len * 2 + 2);
    memcpy(class->base, window->class->base, window->class->len);
    memset(class->base + window->class->len, 0, 1);
    memcpy(class->base + window->class->len + 1, window->class->base,
           window->class->len);
    memset(class->base + window->class->len + 1 + window->class->len, 0, 1);
  } else {
    class = string_new(arena, 12);
    memcpy(class->base, "bread", 5);
    memset(class->base + 5, 0, 1);
    memcpy(class->base + 6, "bread", 5);
    memset(class->base + 11, 0, 1);
  }

  int screen_num = 0;
  bread_log_debug("Connecting to X server");
  state->connection = xcb_connect(NULL, &screen_num);
  int has_error = xcb_connection_has_error(state->connection);
  if (has_error) {
    bread_log_error("Failed to connect to X server: %d", has_error);
    xcb_disconnect(state->connection);
    window->backend = NULL;
    return;
  }

  bread_log_debug("Getting screen info");
  const xcb_setup_t *setup = xcb_get_setup(state->connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  for (int i = 0; i < screen_num; i++) {
    xcb_screen_next(&iter);
  }

  bread_log_debug("Setting up window");
  state->screen = iter.data;
  state->width = window->width;
  state->height = window->height;
  state->xcb_window = xcb_generate_id(state->connection);

  bread_log_debug("Creating window with mask");
  u32 value_mask = XCB_CW_EVENT_MASK;
  u32 value_list[] = {
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
      XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
      XCB_EVENT_MASK_POINTER_MOTION};

  xcb_create_window(state->connection, XCB_COPY_FROM_PARENT, state->xcb_window,
                    state->screen->root, 0, 0, state->width, state->height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, state->screen->root_visual,
                    value_mask, value_list);

  if (window->title) {
    bread_log_debug("Setting window title to %s",
                    string_to_cstr(window->title));
    xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE,
                        state->xcb_window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        window->title->len, window->title->base);
  }

  bread_log_debug("Setting intern WM_PROTOCOLS");
  state->wm_protocols = intern_atom(state->connection, "WM_PROTOCOLS");
  bread_log_debug("Setting intern WM_DELETE_WINDOW");
  state->wm_delete_window = intern_atom(state->connection, "WM_DELETE_WINDOW");
  bread_log_debug("WM_DELETE_WINDOW: %d", state->wm_delete_window);

  bread_log_debug("Setting WM_PROTOCOLS");
  xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE,
                      state->xcb_window, state->wm_protocols, XCB_ATOM_ATOM, 32,
                      1, &state->wm_delete_window);

  bread_log_debug("Mapping window");
  xcb_map_window(state->connection, state->xcb_window);
  bread_log_debug("Reapplying mask");
  xcb_change_window_attributes(state->connection, state->xcb_window, value_mask,
                               value_list);
  bread_log_debug("Flushing X11 connection");
  xcb_flush(state->connection);

  xcb_change_property(state->connection, XCB_PROP_MODE_REPLACE,
                      state->xcb_window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8,
                      class->len, class->base);

  bread_x11_xkb_init(state);

  bread_log_debug("Setting running to true");
  state->running = true;
}

static void x11_poll_events(bread_window_t *window) {
  x11_state_t *state = window->backend;
  if (!state) {
    bread_log_fatal("No X11 state, can't poll events");
    return;
  }

  xcb_generic_event_t *event;
  while ((event = xcb_poll_for_event(state->connection))) {
    u8 type = event->response_type & ~0x80;
    bread_log_debug("Got event of type %d", type);

    switch (type) {
    case XCB_CLIENT_MESSAGE: {
      bread_log_debug("Got client message");
      xcb_client_message_event_t *client_message =
          (xcb_client_message_event_t *)event;
      if (client_message->data.data32[0] == state->wm_delete_window) {
        state->running = false;

        bread_event_t event = {
            .type = BREAD_EVENT_WINDOW_CLOSE,
        };
        fire_event(state->window, &event);
      }
    } break;

    case XCB_CONFIGURE_NOTIFY: {
      bread_log_debug("Got configure notify");
      xcb_configure_notify_event_t *cfg = (xcb_configure_notify_event_t *)event;
      state->width = cfg->width;
      state->height = cfg->height;

      bread_event_t event = {
          .type = BREAD_EVENT_WINDOW_RESIZE,
      };
      event.data.resize.width = state->width;
      event.data.resize.height = state->height;
      fire_event(state->window, &event);
    } break;

    case XCB_KEY_PRESS: {
      bread_log_debug("Got key press");
      xcb_key_press_event_t *key = (xcb_key_press_event_t *)event;
      bread_x11_handle_key_press(state, key);

      bread_event_t event = {
          .type = BREAD_EVENT_KEY_PRESS,
      };
      event.data.key.key = bread_evdev_to_key(key->detail - 8);
      event.data.key.raw_keycode = key->detail;
      fire_event(state->window, &event);
    } break;

    case XCB_KEY_RELEASE: {
      bread_log_debug("Got key release");
      xcb_key_release_event_t *key = (xcb_key_release_event_t *)event;
      bread_x11_handle_key_release(state, key);

      bread_event_t event = {
          .type = BREAD_EVENT_KEY_RELEASE,
      };

      event.data.key.key = bread_evdev_to_key(key->detail - 8);
      event.data.key.raw_keycode = key->detail;
      fire_event(state->window, &event);
    } break;

    case XCB_BUTTON_PRESS: {
      bread_log_debug("Got button press");
      xcb_button_press_event_t *button = (xcb_button_press_event_t *)event;
      bread_x11_handle_button_press(state, button);

      if (button->detail == 4 || button->detail == 5) {
        bread_log_debug("Emitting mouse scroll");
        bread_event_t event = {
            .type = BREAD_EVENT_MOUSE_SCROLL,
        };
        event.data.mouse_scroll.dx = 0.0;
        event.data.mouse_scroll.dy = (button->detail == 4) ? -1.0 : 1.0;
        fire_event(state->window, &event);
        break;
      }

      bread_log_debug("Emitting mouse press");
      bread_mouse_button_t mouse_button = xcb_button_to_bread(button->detail);
      bread_event_t event = {
          .type = BREAD_EVENT_MOUSE_PRESS,
      };
      event.data.mouse_button.button = mouse_button;
      fire_event(state->window, &event);
    } break;

    case XCB_BUTTON_RELEASE: {
      bread_log_debug("Got button release");
      xcb_button_release_event_t *button = (xcb_button_release_event_t *)event;
      bread_x11_handle_button_release(state, button);
      if (button->detail == 4 || button->detail == 5)
        break;

      bread_mouse_button_t mouse_button = xcb_button_to_bread(button->detail);
      bread_event_t event = {
          .type = BREAD_EVENT_MOUSE_RELEASE,
      };
      event.data.mouse_button.button = mouse_button;
      fire_event(state->window, &event);
    } break;

    case XCB_MOTION_NOTIFY: {
      bread_log_debug("Got motion notify");
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;
      bread_x11_handle_motion(state, motion);

      bread_event_t event = {
          .type = BREAD_EVENT_MOUSE_MOVE,
      };
      event.data.mouse_move.x = motion->event_x;
      event.data.mouse_move.y = motion->event_y;
      fire_event(state->window, &event);
    } break;

    case XCB_DESTROY_NOTIFY: {
      bread_log_debug("Got destroy notify");
      state->running = false;
    } break;

    default:
      bread_log_debug("Got unknown event");
      break;
    }

    free(event);
  }
}

static b32 x11_should_close(bread_window_t *window) {
  x11_state_t *state = window->backend;
  if (!state)
    return true;
  return !state->running;
}

static void x11_destroy(bread_window_t *window) {
  bread_log_debug("Destroying X11 window");
  x11_state_t *state = window->backend;
  if (!state) {
    bread_log_debug("Window already destroyed");
    return;
  }

  x11_state_cleanup(state);
  window->backend = NULL;
}

static bread_surface_t x11_get_surface(bread_window_t *window) {
  bread_log_debug("Getting X11 surface");
  x11_state_t *state = window->backend;
  return (bread_surface_t){
      .handle = (void *)(uintptr_t)state->xcb_window,
      .display = state->connection,
  };
}

const bread_backend_vtable_t bread_x11_backend = {
    .init = x11_init,
    .poll_events = x11_poll_events,
    .should_close = x11_should_close,
    .destroy = x11_destroy,
    .get_surface = x11_get_surface,
    .backend_type = BREAD_BACKEND_X11,
};
#endif // !BREAD_X11
