#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

#include <xkbcommon/xkbcommon.h>

#include <bread/event.h>
#include <bread/input.h>
#include <bread/log.h>
#include <bread/wayland/wayland_input.h>

static void keyboard_keymap(void *data, wl_keyboard_t *keyboard, u32 format,
                            i32 fd, u32 size) {
  bread_log_debug("Handling keyboard keymap");
  wl_state_t *state = data;

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    bread_log_debug("Got unknown keymap format");
    close(fd);
    return;
  }

  char *map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  close(fd);

  if (map == MAP_FAILED) {
    bread_log_fatal("Failed to mmap keymap");
    return;
  }

  bread_log_debug("Creating keymap from map, with XKB_V1 format");
  xkb_keymap_t *keymap = xkb_keymap_new_from_string(
      state->xkb_context, map, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map, size);

  if (!keymap) {
    bread_log_fatal("Failed to create keymap");
    return;
  }

  bread_log_debug("Creating xkb state");
  xkb_state_t *xkb_state = xkb_state_new(keymap);
  bread_log_debug("Unrefing old keymap");
  xkb_keymap_unref(state->xkb_keymap);
  bread_log_debug("Unrefing old xkb state");
  xkb_state_unref(state->xkb_state);

  bread_log_debug("Setting new keymap");
  state->xkb_keymap = keymap;
  bread_log_debug("Setting new xkb state");
  state->xkb_state = xkb_state;
}

static void keyboard_enter(void *data, wl_keyboard_t *keyboard, u32 serial,
                           wl_surface_t *surface, wl_array_t *keys) {}

static void keyboard_leave(void *data, wl_keyboard_t *keyboard, u32 serial,
                           wl_surface_t *surface) {
  bread_log_debug("Handling keyboard leave");
  wl_state_t *state = data;
  memset(state->input.keys, 0, sizeof(state->input.keys));
}

static void keyboard_key(void *data, wl_keyboard_t *keyboard, u32 serial,
                         u32 time, u32 key, u32 keystate) {
  bread_log_debug("Handling keyboard key");
  wl_state_t *state = data;

  bread_log_debug("Converting key to bread key");
  bread_key_t bread_key = bread_evdev_to_key(key);

  bread_log_debug("Getting pressed state");
  b32 pressed = keystate == WL_KEYBOARD_KEY_STATE_PRESSED;
  state->input.keys[bread_key] = pressed;

  bread_log_debug("Emitting key press");
  bread_event_t event = {0};
  event.type = BREAD_EVENT_KEY_PRESS;
  event.type = pressed ? BREAD_EVENT_KEY_PRESS : BREAD_EVENT_KEY_RELEASE;
  event.data.key.key = bread_key;
  event.data.key.raw_keycode = key;
  fire_event(state->window, &event);
}

static void keyboard_modifiers(void *data, wl_keyboard_t *keyboard, u32 serial,
                               u32 mods_depressed, u32 mods_latched,
                               u32 mods_locked, u32 group) {
  bread_log_debug("Handling keyboard modifiers");
  wl_state_t *state = data;
  if (state->xkb_state) {
    bread_log_debug("Updating xkb state mask");
    xkb_state_update_mask(state->xkb_state, mods_depressed, mods_latched,
                          mods_locked, 0, 0, group);
  }
}

static void keyboard_repeat_info(void *data, wl_keyboard_t *keyboard, i32 rate,
                                 i32 delay) {}

static const wl_keyboard_listener_t keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

static void pointer_enter(void *data, wl_pointer_t *pointer, u32 serial,
                          wl_surface_t *surface, wl_fixed_t surface_x,
                          wl_fixed_t surface_y) {
  bread_log_debug("Handling pointer enter");
  wl_state_t *state = data;
  state->input.mouse_x = wl_fixed_to_double(surface_x);
  state->input.mouse_y = wl_fixed_to_double(surface_y);
  bread_log_debug("Pointer entered is now at (%f, %f)", state->input.mouse_x,
                  state->input.mouse_y);
}

static void pointer_leave(void *data, wl_pointer_t *pointer, u32 serial,
                          wl_surface_t *surface) {}

static void pointer_motion(void *data, wl_pointer_t *pointer, u32 time,
                           wl_fixed_t surface_x, wl_fixed_t surface_y) {
  bread_log_debug("Handling pointer motion");
  wl_state_t *state = data;
  state->input.mouse_x = wl_fixed_to_double(surface_x);
  state->input.mouse_y = wl_fixed_to_double(surface_y);
  bread_log_debug("Pointer moved to (%f, %f)", state->input.mouse_x,
                  state->input.mouse_y);

  bread_log_debug("Emitting mouse move");
  bread_event_t event = {
      .type = BREAD_EVENT_MOUSE_MOVE,
  };
  event.data.mouse_move.x = state->input.mouse_x;
  event.data.mouse_move.y = state->input.mouse_y;
  fire_event(state->window, &event);
}

static void pointer_button(void *data, wl_pointer_t *pointer, u32 serial,
                           u32 time, u32 button, u32 pointer_state) {
  bread_log_debug("Handling pointer button");
  wl_state_t *state = data;

  bread_mouse_button_t bread_button = bread_evdev_to_mouse_button(button);
  if (bread_button >= BREAD_MOUSE_BUTTON_MAX) {
    bread_log_debug("Got unknown mouse button");
    return;
  }

  bread_log_debug("Got mouse button %d", bread_button);
  b32 pressed = pointer_state == WL_POINTER_BUTTON_STATE_PRESSED;
  state->input.mouse_buttons[bread_button] = pressed;

  bread_log_debug("Emitting mouse button press");
  bread_event_t event = {
      .type = pressed ? BREAD_EVENT_MOUSE_PRESS : BREAD_EVENT_MOUSE_RELEASE,
  };
  event.data.mouse_button.button = bread_button;
  fire_event(state->window, &event);
}

static void pointer_axis(void *data, wl_pointer_t *pointer, u32 time, u32 axis,
                         wl_fixed_t value) {
  bread_log_debug("Handling pointer axis");
  wl_state_t *state = data;
  f64 dv = wl_fixed_to_double(value);

  bread_event_t event = {
      .type = BREAD_EVENT_MOUSE_SCROLL,
  };

  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
    bread_log_debug("Scrolling vertically");
    state->input.scroll_y += dv;
    event.data.mouse_scroll.dx = 0.0f;
    event.data.mouse_scroll.dy = dv;
  } else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
    bread_log_debug("Scrolling horizontally");
    state->input.scroll_x += dv;
    event.data.mouse_scroll.dx = dv;
    event.data.mouse_scroll.dy = 0.0f;
  }

  fire_event(state->window, &event);
}

static void pointer_frame(void *data, wl_pointer_t *pointer) {}

static void pointer_axis_source(void *data, wl_pointer_t *pointer,
                                u32 axis_source) {}
static void pointer_axis_stop(void *data, wl_pointer_t *pointer, u32 time,
                              u32 axis) {}
static void pointer_axis_discrete(void *data, wl_pointer_t *pointer, u32 axis,
                                  i32 discrete) {}

static const wl_pointer_listener_t pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
    .frame = pointer_frame,
    .axis_source = pointer_axis_source,
    .axis_stop = pointer_axis_stop,
    .axis_discrete = pointer_axis_discrete,
};

static void seat_capabilities(void *data, wl_seat_t *seat, u32 capabilities) {
  bread_log_debug("Handling seat capabilities");
  wl_state_t *state = data;

  b32 has_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
  b32 has_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

  if (has_pointer && !state->pointer) {
    bread_log_debug("Getting pointer");
    state->pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(state->pointer, &pointer_listener, state);
  } else if (!has_pointer && state->pointer) {
    bread_log_debug("Releasing pointer");
    wl_pointer_release(state->pointer);
    state->pointer = NULL;
  }

  if (has_keyboard && !state->keyboard) {
    bread_log_debug("Getting keyboard");
    state->keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(state->keyboard, &keyboard_listener, state);
  } else if (!has_keyboard && state->keyboard) {
    bread_log_debug("Releasing keyboard");
    wl_keyboard_release(state->keyboard);
    state->keyboard = NULL;
  }
}

static void seat_name(void *data, wl_seat_t *seat, const cstr *name) {}

static const wl_seat_listener_t seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

void bread_wayland_seat_init(wl_state_t *state) {
  bread_log_debug("Initializing wayland seat");
  state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

void bread_wayland_seat_bind(wl_state_t *state, wl_registry_t *registry,
                             u32 name, u32 version) {
  bread_log_debug("Binding wayland seat");
  state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
  bread_log_debug("Adding seat listener");
  wl_seat_add_listener(state->seat, &seat_listener, state);
}

void bread_wayland_seat_cleanup(wl_state_t *state) {
  if (state->keyboard) {
    bread_log_debug("Releasing keyboard");
    wl_keyboard_release(state->keyboard);
  }

  if (state->pointer) {
    bread_log_debug("Releasing pointer");
    wl_pointer_release(state->pointer);
  }

  if (state->seat) {
    bread_log_debug("Destroying seat");
    wl_seat_destroy(state->seat);
  }

  bread_log_debug("Unrefing xkb state");
  xkb_state_unref(state->xkb_state);
  bread_log_debug("Unrefing xkb keymap");
  xkb_keymap_unref(state->xkb_keymap);
  bread_log_debug("Unrefing xkb context");
  xkb_context_unref(state->xkb_context);
}
#endif // !BREAD_WAYLAND
