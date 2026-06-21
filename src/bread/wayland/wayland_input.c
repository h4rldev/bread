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

/**
 * @brief Initializes the keyboard keymap for wayland.
 *
 * @details Called when the keyboard keymap is ready to be used by the @ref
 * keyboard_listener.
 *
 * @param data The wl_state to initialize.
 * @param keyboard The keyboard to initialize.
 * @param format The format of the keymap.
 * @param fd The file descriptor of the keymap.
 * @param size The size of the keymap.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c format must be WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1.
 * - @c fd must be a valid file descriptor.
 * - @c size must be the size of the keymap.
 */
static void keyboard_keymap(void *data, wl_keyboard_t *keyboard, u32 format,
                            i32 fd, u32 size) {
  bread_log_debug("Initializing keyboard keymap");
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

/**
 * @brief Handles the keyboard enter event.
 * @note Noop because we don't need to handle keyboard_enter.
 */
static void keyboard_enter(void *data, wl_keyboard_t *keyboard, u32 serial,
                           wl_surface_t *surface, wl_array_t *keys) {}

/**
 * @brief Handles the keyboard leave event.
 *
 * @details Resets the keyboard state, called by the @ref keyboard_listener.
 *
 * @param data The wl_state.
 * @param keyboard The keyboard that's leaving.
 * @param serial The serial of the keyboard.
 * @param surface The current surface where the keyboard is leaving.
 *
 * @pre @c data must be a valid pointer.
 */
static void keyboard_leave(void *data, wl_keyboard_t *keyboard, u32 serial,
                           wl_surface_t *surface) {
  bread_log_debug("Handling keyboard leave");
  wl_state_t *state = data;
  memset(state->input.keys, 0, sizeof(state->input.keys));
}

/**
 * @brief Handles the keyboard key event.
 *
 * @details Converts the key to a bread key, sets the pressed state and fires a
 * bread event. Called by the @ref keyboard_listener.
 *
 * @param data The wl_state.
 * @param keyboard The keyboard that's emitting the key event.
 * @param serial The serial of the keyboard.
 * @param time The time of the key event.
 * @param key The evdev key of the key event.
 * @param keystate The state of the key event.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c key must be a valid evdev key.
 * - @c keystate must be a valid wayland keyboard key state.
 */
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

/**
 * @brief Handles the keyboard modifiers event.
 *
 * @details Updates the xkb state mask if the xkb state is available.
 *
 * @param data The wl_state.
 * @param keyboard The keyboard that's emitting the modifiers event.
 * @param serial The serial of the keyboard.
 * @param mods_depressed The modifiers depressed.
 * @param mods_latched The modifiers latched.
 * @param mods_locked The modifiers locked.
 * @param group The group of the modifiers.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c mods_depressed, @c mods_latched, and @c mods_locked must be valid
 * wayland modifier masks.
 * - @c group must be a valid wayland modifier group.
 */
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

/**
 * @brief Handles the keyboard repeat info event.
 * @details Noop because we don't need to handle keyboard_repeat_info.
 */
static void keyboard_repeat_info(void *data, wl_keyboard_t *keyboard, i32 rate,
                                 i32 delay) {}

/**
 * @brief The keyboard listener.
 *
 * @details Handles keymap, enter, leave, keypress, keyboard modifiers, and
 * keyboard repeat info.
 */
static const wl_keyboard_listener_t keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

/**
 * @brief Handles the pointer enter event.
 *
 * @details Updates the mouse position and fires a bread event.
 *
 * @param data The wl_state.
 * @param pointer The pointer that's entering.
 * @param serial The serial of the pointer.
 * @param surface The current surface where the pointer is entering.
 * @param surface_x The x position of the pointer in the surface.
 * @param surface_y The y position of the pointer in the surface.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c surface_x and @c surface_y must be valid wayland fixed coordinates.
 */
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

/**
 * @brief Handles the pointer leave event.
 * @details Noop because we don't need to handle pointer_leave.
 */
static void pointer_leave(void *data, wl_pointer_t *pointer, u32 serial,
                          wl_surface_t *surface) {}

/**
 * @brief Handles the pointer motion event.
 *
 * @details Updates the mouse position and fires a bread event.
 *
 * @param data The wl_state.
 * @param pointer The pointer that's emitting the motion event.
 * @param time The time of the motion event.
 * @param surface_x The x position of the pointer in the surface.
 * @param surface_y The y position of the pointer in the surface.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c surface_x and @c surface_y must be valid wayland fixed coordinates.
 */
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

/**
 * @brief Handles the pointer button event.
 *
 * @details Converts the button to a bread mouse button, sets the pressed state
 * and fires a bread event.
 *
 * @param data The wl_state.
 * @param pointer The pointer that's emitting the button event.
 * @param serial The serial of the pointer.
 * @param time The time of the button event.
 * @param button The evdev button of the button event.
 * @param pointer_state The state of the button event.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c button must be a valid evdev button.
 * - @c pointer_state must be a valid wayland pointer button state.
 */
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

/**
 * @brief Handles the pointer axis event.
 *
 * @details Converts the axis to a bread mouse axis, sets the value and fires an
 * event.
 *
 * @param data The wl_state.
 * @param pointer The pointer that's emitting the axis event.
 * @param time The time of the axis event.
 * @param axis The axis of the axis event.
 * @param value The value of the axis event.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c axis must be a valid wayland pointer axis.
 * - @c value must be a valid wayland fixed value.
 */
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

/**
 * @brief Handles the pointer frame event.
 * @details Noop because we don't need to handle pointer_frame.
 */
static void pointer_frame(void *data, wl_pointer_t *pointer) {}

/**
 * @brief Handles the pointer axis source event.
 * @details Noop because we don't need to handle pointer_axis_source.
 */
static void pointer_axis_source(void *data, wl_pointer_t *pointer,
                                u32 axis_source) {}

/**
 * @brief Handles the pointer axis stop event.
 * @details Noop because we don't need to handle pointer_axis_stop.
 */
static void pointer_axis_stop(void *data, wl_pointer_t *pointer, u32 time,
                              u32 axis) {}

/**
 * @brief Handles the pointer axis discrete event.
 * @details Noop because we don't need to handle pointer_axis_discrete.
 */
static void pointer_axis_discrete(void *data, wl_pointer_t *pointer, u32 axis,
                                  i32 discrete) {}

/**
 * @brief The pointer listener.
 *
 * @details Handles pointer enter, leave, motion, button, axis, frame, axis
 * source, axis stop, and axis discrete.
 */
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

/**
 * @brief Handles the seat capabilities event.
 *
 * @details Gets the pointer and keyboard if the capabilities include them.
 *
 * @param data The wl_state.
 * @param seat The seat that's emitting the capabilities event.
 * @param capabilities The capabilities of the seat.
 *
 * @pre
 * - @c data must be a valid pointer.
 * - @c seat must be a valid wayland seat.
 * - @c capabilities must be a valid wayland seat capabilities.
 */
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
    state->keyboard = null;
  }
}

/**
 * @brief Seat name.
 * @details Noop because we don't need to handle seat_name.
 */
static void seat_name(void *data, wl_seat_t *seat, const cstr *name) {}

/**
 * @brief The seat listener.
 *
 * @details Handles seat capabilities and seat_name.
 */
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
