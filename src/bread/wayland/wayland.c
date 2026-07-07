#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

#include <errno.h>
#include <poll.h>
#include <string.h>

#include <htils/basictypes.h>
#include <htils/string.h>

#include <bread/backend.h>
#include <bread/event.h>
#include <bread/input.h>
#include <bread/log.h>
#include <bread/window.h>

#include <bread/wayland/wayland_input.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <wayland/xdg-decoration-client-protocol.h>
#include <wayland/xdg-shell-client-protocol.h>

/**
 * @brief Ping the wm_base.
 *
 * @details Pings the wm_base, and responds with a pong.
 *
 * @param data The data pointer.
 * @param wm The wm_base.
 * @param serial The serial.
 *
 * @pre @c wm and @c serial must be valid.
 */
static void xdg_wm_base_ping(void *data, xdg_wm_base_t *wm, u32 serial) {
  bread_log_debug("Got wm_base ping");
  xdg_wm_base_pong(wm, serial);
}

/**
 * @brief The wm_base listener.
 *
 * @details Which is used to listen for pings.
 */
static const xdg_wm_base_listener_t xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

/**
 * @brief Close the toplevel.
 *
 * @details Closes the toplevel, and sets the running flag to false.
 *
 * @param data The data pointer.
 * @param toplevel The toplevel.
 *
 * @pre @c data, and @c toplevel must be valid.
 */
static void xdg_toplevel_close(void *data, xdg_toplevel_t *toplevel) {
  bread_log_debug("Got toplevel close");
  wl_state_t *state = data;
  state->running = false;

  bread_event_t event = {.type = BREAD_EVENT_WINDOW_CLOSE};
  fire_event(state->window, &event);
}

/**
 * @brief Configure the toplevel.
 *
 * @details Fires an event if the window size is changed.
 *
 * @param data The data pointer.
 * @param toplevel The toplevel.
 *
 * @pre @c data, and @c toplevel must be valid.
 */
static void xdg_toplevel_configure(void *data, xdg_toplevel_t *toplevel,
                                   i32 width, i32 height, wl_array_t *states) {
  bread_log_debug("Got toplevel configure");
  wl_state_t *state = data;
  if (width > 0 && height > 0) {
    bread_log_debug("Setting window size to %d x %d", width, height);
    state->width = (u16)(width > 65535 ? 65535 : width);
    state->height = (u16)(height > 65535 ? 65535 : height);

    bread_event_t event = {0};
    event.type = BREAD_EVENT_WINDOW_RESIZE;
    event.data.resize.width = state->width;
    event.data.resize.height = state->height;
    fire_event(state->window, &event);
  }
}

/**
 * @brief The toplevel listener.
 *
 * @details Which is used to listen for toplevel events such as close and
 * configure.
 */
static const xdg_toplevel_listener_t xdg_toplevel_listener = {
    .close = xdg_toplevel_close,
    .configure = xdg_toplevel_configure,
};

/**
 * @brief Configure the surface.
 *
 * @details Acks the configure, and updates the surface.
 *
 * @param data The data pointer.
 * @param surface The surface.
 * @param serial The serial.
 *
 * @pre @c data, @c surface, and @c serial must be valid.
 */
static void xdg_surface_configure(void *data, xdg_surface_t *surface,
                                  u32 serial) {
  bread_log_debug("Got surface configure");
  wl_state_t *state = data;
  if (!state) {
    bread_log_fatal("No state, can't update surface");
    return;
  }

  xdg_surface_ack_configure(surface, serial);
  wl_surface_commit(state->wl_surface);
}

/**
 * @brief The surface listener.
 *
 * @details Which is used to listen for configure events.
 */
static const xdg_surface_listener_t xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

/**
 * @brief Handes registry binds.
 *
 * @details Binds the compositor, shm, seat, and SSD manager. Binds and listens
 * the wm_base.
 *
 * @param data The data pointer.
 * @param registry The registry.
 * @param id The id.
 * @param interface The interface.
 * @param version The version.
 *
 * @pre @c data, @c registry, @c id, @c interface, and @c version must be valid.
 */
static void global_registry_handler(void *data, wl_registry_t *registry, u32 id,
                                    const cstr *interface, u32 version) {
  bread_log_debug("Handling registry event");
  wl_state_t *state = (wl_state_t *)data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    bread_log_debug("Binding compositor");
    u32 compos_version = (version < 4) ? version : 4;
    state->compositor = wl_registry_bind(registry, id, &wl_compositor_interface,
                                         compos_version);
  }

  else if (strcmp(interface, wl_shm_interface.name) == 0) {
    bread_log_debug("Binding shm");
    u32 shm_version = (version < 1) ? version : 1;
    state->shm = wl_registry_bind(registry, id, &wl_shm_interface, shm_version);
  }

  else if (strcmp(interface, wl_seat_interface.name) == 0) {
    bread_log_debug("Binding seat");
    bread_wayland_seat_bind(state, registry, id, version);
  }

  else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
    bread_log_debug("Binding SSD manager");
    u32 decoration_version = (version < 1) ? version : 1;
    state->decoration_manager =
        wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface,
                         decoration_version);
  }

  else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    bread_log_debug("Binding wm_base");
    u32 wm_version = (version < 1) ? version : 1;
    state->xdg_wm_base =
        wl_registry_bind(registry, id, &xdg_wm_base_interface, wm_version);
    xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
  }
}

/**
 * @brief Removes the registry.
 * @details Noop because the registry is cleaned up in @ref wl_state_cleanup().
 *
 * @see @ref wl_state_cleanup().
 */
static void global_registry_remover(void *data, wl_registry_t *registry,
                                    u32 id) {}

/**
 * @brief Cleans up the wayland state.
 *
 * @details Cleans up the seat, decoration, decoration manager, xdg_toplevel,
 * xdg_surface, wl_surface, wm_base, shm, compositor, registry, and display if
 * they exist.
 *
 * @param state The wayland state.
 *
 * @pre @c state must be valid.
 */
static void wl_state_cleanup(wl_state_t *state) {
  if (state->seat) {
    bread_log_debug("Cleaning up seat");
    bread_wayland_seat_cleanup(state);
    state->seat = null;
  }

  if (state->decoration) {
    bread_log_debug("Destroying decoration");
    zxdg_toplevel_decoration_v1_destroy(state->decoration);
    state->decoration = null;
  }

  if (state->decoration_manager) {
    bread_log_debug("Destroying decoration manager");
    zxdg_decoration_manager_v1_destroy(state->decoration_manager);
    state->decoration_manager = null;
  }

  if (state->xdg_toplevel) {
    bread_log_debug("Destroying toplevel");
    xdg_toplevel_destroy(state->xdg_toplevel);
    state->xdg_toplevel = null;
  }

  if (state->xdg_surface) {
    bread_log_debug("Destroying surface");
    xdg_surface_destroy(state->xdg_surface);
    state->xdg_surface = null;
  }

  if (state->wl_surface) {
    bread_log_debug("Destroying surface");
    wl_surface_destroy(state->wl_surface);
    state->wl_surface = null;
  }

  if (state->xdg_wm_base) {
    bread_log_debug("Destroying wm_base");
    xdg_wm_base_destroy(state->xdg_wm_base);
    state->xdg_wm_base = null;
  }

  if (state->shm) {
    bread_log_debug("Destroying shm");
    wl_shm_destroy(state->shm);
    state->shm = null;
  }

  if (state->compositor) {
    bread_log_debug("Destroying compositor");
    wl_compositor_destroy(state->compositor);
    state->compositor = null;
  }

  if (state->registry) {
    bread_log_debug("Destroying registry");
    wl_registry_destroy(state->registry);
    state->registry = null;
  }

  if (state->display) {
    bread_log_debug("Disconnecting from display");
    wl_display_disconnect(state->display);
    state->display = null;
  }
}

/**
 * @brief The registry listener.
 *
 * @details Which is used to listen for global resitry events.
 */
static const wl_registry_listener_t registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remover,
};

/**
 * @brief Initializes the wayland window.
 *
 * @details Connects to the display socket, gets the screen, and creates the
 * window, sets the app-id, title and minimum size to what's specified if
 * they're defined in @c window.
 *
 * @param window The window to initialize.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 */
static void wayland_init(bread_window_t *window) {
  bread_log_debug("Initializing wayland window");

  arena_t *arena = window->arena;
  wl_state_t *state = arena_alloc(arena, wl_state_t, 1);
  state->window = window;

  state->display = wl_display_connect(NULL);
  if (!state->display) {
    bread_log_fatal("Failed to connect to display socket");
    window->backend = null;
    return;
  }

  bread_log_debug("Setting window size to %d x %d", window->width,
                  window->height);
  state->width = window->width;
  state->height = window->height;

  bread_log_debug("Getting registry");
  state->registry = wl_display_get_registry(state->display);
  bread_log_debug("Adding registry listener");
  wl_registry_add_listener(state->registry, &registry_listener, state);

  bread_log_debug("Initializing seat");
  bread_wayland_seat_init(state);

  bread_log_debug("Flushing display");
  if (wl_display_roundtrip(state->display) == -1) {
    bread_log_fatal("Failed to roundtrip display");
    goto fail;
  }

  if (!state->compositor) {
    bread_log_fatal("No wl_compositor found");
    goto fail;
  }

  bread_log_debug("Creating surface");
  state->wl_surface = wl_compositor_create_surface(state->compositor);
  if (!state->wl_surface) {
    bread_log_fatal("Failed to create surface");
    goto fail;
  }

  if (!state->xdg_wm_base) {
    bread_log_fatal("xdg_wm_base is null");
    goto fail;
  }

  bread_log_debug("Getting xdg_surface");
  state->xdg_surface =
      xdg_wm_base_get_xdg_surface(state->xdg_wm_base, state->wl_surface);
  if (!state->xdg_surface) {
    bread_log_fatal("Failed to get xdg_surface");
    goto fail;
  }

  bread_log_debug("Adding xdg_surface listener");
  xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);

  bread_log_debug("Getting toplevel");
  state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
  if (!state->xdg_toplevel) {
    bread_log_fatal("Failed to get toplevel");
    goto fail;
  }

  bread_log_debug("Adding toplevel listener");
  xdg_toplevel_add_listener(state->xdg_toplevel, &xdg_toplevel_listener, state);

  if (window->min_width > 0 && window->min_height > 0) {
    xdg_toplevel_set_min_size(state->xdg_toplevel, window->min_width,
                              window->min_height);
  }

  if (window->title) {
    bread_log_debug("Setting window title to %s",
                    string_to_cstr(window->title));
    xdg_toplevel_set_title(state->xdg_toplevel, string_to_cstr(window->title));
  }

  if (state->decoration_manager) {
    bread_log_debug("Getting decoration");
    state->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
        state->decoration_manager, state->xdg_toplevel);
    zxdg_toplevel_decoration_v1_set_mode(
        state->decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  } else
    bread_log_debug("No decoration manager, disabling decoration");

  if (!window->class) {
    xdg_toplevel_set_app_id(state->xdg_toplevel, "bread-app");
  } else {
    xdg_toplevel_set_app_id(state->xdg_toplevel, string_to_cstr(window->class));
  }

  bread_log_debug("Committing surface");
  wl_surface_commit(state->wl_surface);
  bread_log_debug("Flushing display");

  if (wl_display_roundtrip(state->display) == -1) {
    bread_log_fatal("Failed to roundtrip display");
    goto fail;
  }

  bread_log_debug("Setting running to true");
  state->running = true;
  bread_log_debug("Setting window backend");
  window->backend = state;
  return;

fail:
  wl_state_cleanup(state);
  window->backend = null;
}

/**
 * @brief Polls for events on the wayland window.
 *
 * @details Uses poll to poll the display file descriptor for events, and if
 * there's any it reads them, and dispatches them.
 *
 * @param window The window to poll for events.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 */
static void wayland_poll_events(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_fatal("No state, can't poll events");
    return;
  }

  wl_display_flush(state->display);

  struct pollfd fds = {
      .fd = wl_display_get_fd(state->display),
      .events = POLLIN,
  };

  int ret = poll(&fds, 1, 0);
  if (ret < 0) {
    bread_log_error("poll failed: %s", strerror(errno));
    return;
  }

  if (ret > 0) {
    if (fds.revents & (POLLERR | POLLHUP)) {
      bread_log_error("Wayland socket error, closing");
      state->running = false;
      return;
    }

    if (fds.revents & POLLIN) {
      bread_log_debug("Found event, processing");
      while (wl_display_prepare_read(state->display) != 0)
        wl_display_dispatch_pending(state->display);

      wl_display_flush(state->display);
      wl_display_read_events(state->display);
      wl_display_dispatch_pending(state->display);
    } else {
      bread_log_debug("No event, flushing");
      wl_display_dispatch_pending(state->display);
      wl_display_flush(state->display);
    }
  }
}

/**
 * @brief Checks if the wayland window should close.
 *
 * @details Useful for an application's event loop, and it's designed with that
 * in mind.
 *
 * @param window The window to check.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 *
 * @return true if the window should close, false if not.
 */
static b32 wayland_should_close(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_fatal("Window already destroyed");
    return true;
  }
  return !state->running;
}

/**
 * @brief Destroys the wayland window.
 *
 * @details Cleans up the wayland state with @ref wl_state_cleanup(), and sets
 * the backend to null.
 *
 * @param window The window to destroy.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 *
 * @see @ref bread_window_new(), @ref wl_state_cleanup().
 */
static void wayland_destroy(bread_window_t *window) {
  bread_log_debug("Destroying wayland window");
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_debug("Window already destroyed");
    return;
  }
  wl_state_cleanup(state);
  window->backend = null;
}

/**
 * @brief Gets the wayland surface.
 *
 * @details Which is the wl_surface, and the wl_display.
 *
 * @param window The window to get the surface from.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 *
 * @return The surface of the wayland window as a bread surface.
 */
static bread_surface_t wayland_get_surface(bread_window_t *window) {
  bread_log_debug("Getting wayland surface");
  wl_state_t *state = window->backend;
  if (!state)
    return (bread_surface_t){0};

  return (bread_surface_t){
      .handle = state->wl_surface,
      .display = state->display,
  };
}

/**
 * @brief Sets the title of the wayland window.
 *
 * @details Updates the internal title and sets the xdg_toplevel's title, then
 * commits the surface.
 *
 * @param window The window to set the title of.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 */
static void wayland_set_title(bread_window_t *window, const char *title) {
  if (!window || !window->backend || !title) {
    bread_log_error("Missing values, can't set title");
    return;
  }

  wl_state_t *state = window->backend;
  if (!state->xdg_toplevel) {
    bread_log_error("No xdg_toplevel, can't set title");
    return;
  }

  window->title = string_from_cstr(window->arena, title);
  if (!window->title || window->title->len == 0) {
    bread_log_error("Failed to set title, invalid title");
    return;
  }

  xdg_toplevel_set_title(state->xdg_toplevel, title);
  wl_surface_commit(state->wl_surface);
}

/**
 * @brief Sets the minimum size of the wayland window.
 *
 * @details Updates the internal minimum size and sets the xdg_toplevel's
 * minimum width, and height, then commits the surface. If width and height
 * passed are equal to the already set minimum size or 0 then this function does
 * nothing.
 *
 * @param window The window to set the minimum size of.
 * @param width The minimum width of the window.
 * @param height The minimum height of the window.
 *
 * @pre @c window must be valid and created by @ref bread_window_new().
 */
static void wayland_set_min_size(bread_window_t *window, u16 width,
                                 u16 height) {
  wl_state_t *state = window->backend;
  if (!state || !state->xdg_toplevel)
    return;

  if ((width == window->min_width && height == window->min_height) ||
      (width == 0 && height == 0))
    return;

  window->min_width = width;
  window->min_height = height;

  xdg_toplevel_set_min_size(state->xdg_toplevel, width, height);
  wl_surface_commit(state->wl_surface);
}

/**
 * @brief Get the wayland backend.
 *
 * @param init The init function.
 * @param poll_events The poll events function.
 * @param should_close The should close function.
 * @param destroy The destroy function.
 * @param get_surface The get surface function.
 * @param set_title The set title function.
 * @param set_min_size The set min size function.
 * @param backend_type The backend type which is BREAD_BACKEND_WAYLAND.
 */
const bread_backend_vtable_t bread_wayland_backend = {
    .init = wayland_init,
    .poll_events = wayland_poll_events,
    .should_close = wayland_should_close,
    .destroy = wayland_destroy,
    .get_surface = wayland_get_surface,
    .set_title = wayland_set_title,
    .set_min_size = wayland_set_min_size,
    .backend_type = BREAD_BACKEND_WAYLAND,
};
#endif // BREAD_WAYLAND
