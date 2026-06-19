#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

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

static void xdg_wm_base_ping(void *data, xdg_wm_base_t *wm, u32 serial) {
  bread_log_debug("Got wm_base ping");
  xdg_wm_base_pong(wm, serial);
}

static const xdg_wm_base_listener_t xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void xdg_toplevel_close(void *data, xdg_toplevel_t *toplevel) {
  bread_log_debug("Got toplevel close");
  wl_state_t *state = data;
  state->running = false;

  bread_event_t event = {.type = BREAD_EVENT_WINDOW_CLOSE};
  fire_event(state->window, &event);
}

static void xdg_toplevel_configure(void *data, xdg_toplevel_t *toplevel,
                                   i32 width, i32 height, wl_array_t *states) {
  bread_log_debug("Got toplevel configure");
  wl_state_t *state = data;
  if (width > 0 && height > 0) {
    bread_log_debug("Setting window size to %d x %d", width, height);
    state->width = (u16)width;
    state->height = (u16)height;

    bread_event_t event = {
        .type = BREAD_EVENT_WINDOW_RESIZE,
    };
    event.data.resize.width = state->width;
    event.data.resize.height = state->height;
    fire_event(state->window, &event);
  }
}

static const xdg_toplevel_listener_t xdg_toplevel_listener = {
    .close = xdg_toplevel_close,
    .configure = xdg_toplevel_configure,
};

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

static const xdg_surface_listener_t xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void global_registry_handler(void *data, wl_registry_t *registry, u32 id,
                                    const cstr *interface, u32 version) {
  bread_log_debug("Handling registry event");
  wl_state_t *state = (wl_state_t *)data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    bread_log_debug("Binding compositor");
    state->compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 4);
  }

  else if (strcmp(interface, wl_shm_interface.name) == 0) {
    bread_log_debug("Binding shm");
    state->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
  }

  else if (strcmp(interface, wl_seat_interface.name) == 0) {
    bread_log_debug("Binding seat");
    bread_wayland_seat_bind(state, registry, id, version);
  }

  else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
    bread_log_debug("Binding SSD manager");
    state->decoration_manager = wl_registry_bind(
        registry, id, &zxdg_decoration_manager_v1_interface, 1);
  }

  else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    bread_log_debug("Binding wm_base");
    state->xdg_wm_base =
        wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
  }
}

static void global_registry_remover(void *data, wl_registry_t *registry,
                                    u32 id) {}

static void wl_state_cleanup(wl_state_t *state) {
  if (state->seat) {
    bread_log_debug("Cleaning up seat");
    bread_wayland_seat_cleanup(state);
  }

  if (state->decoration) {
    bread_log_debug("Destroying decoration");
    zxdg_toplevel_decoration_v1_destroy(state->decoration);
  }

  if (state->decoration_manager) {
    bread_log_debug("Destroying decoration manager");
    zxdg_decoration_manager_v1_destroy(state->decoration_manager);
  }

  if (state->xdg_toplevel) {
    bread_log_debug("Destroying toplevel");
    xdg_toplevel_destroy(state->xdg_toplevel);
  }

  if (state->xdg_surface) {
    bread_log_debug("Destroying surface");
    xdg_surface_destroy(state->xdg_surface);
  }

  if (state->wl_surface) {
    bread_log_debug("Destroying surface");
    wl_surface_destroy(state->wl_surface);
  }

  if (state->xdg_wm_base) {
    bread_log_debug("Destroying wm_base");
    xdg_wm_base_destroy(state->xdg_wm_base);
  }

  if (state->shm) {
    bread_log_debug("Destroying shm");
    wl_shm_destroy(state->shm);
  }

  if (state->compositor) {
    bread_log_debug("Destroying compositor");
    wl_compositor_destroy(state->compositor);
  }

  if (state->registry) {
    bread_log_debug("Destroying registry");
    wl_registry_destroy(state->registry);
  }

  if (state->display) {
    bread_log_debug("Disconnecting from display");
    wl_display_disconnect(state->display);
  }
}

static const wl_registry_listener_t registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remover,
};

static void wayland_init(bread_window_t *window) {
  bread_log_debug("Initializing wayland window");

  arena_t *arena = window->arena;
  wl_state_t *state = arena_alloc(arena, wl_state_t, 1);
  state->window = window;

  state->display = wl_display_connect(NULL);
  if (!state->display) {
    bread_log_fatal("Failed to connect to display socket");
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
  wl_display_roundtrip(state->display);
  if (!state->compositor) {
    bread_log_fatal("No wl_compositor found");
    return;
  }

  bread_log_debug("Creating surface");
  state->wl_surface = wl_compositor_create_surface(state->compositor);
  bread_log_debug("Getting xdg_surface");
  state->xdg_surface =
      xdg_wm_base_get_xdg_surface(state->xdg_wm_base, state->wl_surface);
  bread_log_debug("Adding xdg_surface listener");
  xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);

  bread_log_debug("Getting toplevel");
  state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
  bread_log_debug("Adding toplevel listener");
  xdg_toplevel_add_listener(state->xdg_toplevel, &xdg_toplevel_listener, state);

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
  } else {
    bread_log_debug("No decoration manager, disabling decoration");
  }

  bread_log_debug("Committing surface");
  wl_surface_commit(state->wl_surface);
  bread_log_debug("Flushing display");
  wl_display_roundtrip(state->display);

  if (!window->class) {
    xdg_toplevel_set_app_id(state->xdg_toplevel, "bread-app");
  } else {
    xdg_toplevel_set_app_id(state->xdg_toplevel, string_to_cstr(window->class));
  }

  bread_log_debug("Setting running to true");
  state->running = true;
  bread_log_debug("Setting window backend");
  window->backend = state;
}

static void wayland_poll_events(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_fatal("No state, can't poll events");
    return;
  }

  struct pollfd fds = {
      .fd = wl_display_get_fd(state->display),
      .events = POLLIN,
  };

  int ret = poll(&fds, 1, 0);
  if (ret > 0 && (fds.revents & POLLIN)) {
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

static b32 wayland_should_close(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_fatal("Window already destroyed");
    return true;
  }
  return !state->running;
}

static void wayland_destroy(bread_window_t *window) {
  bread_log_debug("Destroying wayland window");
  wl_state_t *state = window->backend;
  if (!state) {
    bread_log_debug("Window already destroyed");
    return;
  }
  wl_state_cleanup(state);
  window->backend = NULL;
}

static bread_surface_t wayland_get_surface(bread_window_t *window) {
  bread_log_debug("Getting wayland surface");
  wl_state_t *state = window->backend;
  return (bread_surface_t){
      .handle = state->wl_surface,
      .display = state->display,
  };
}

const bread_backend_vtable_t bread_wayland_backend = {
    .init = wayland_init,
    .poll_events = wayland_poll_events,
    .should_close = wayland_should_close,
    .destroy = wayland_destroy,
    .get_surface = wayland_get_surface,
    .backend_type = BREAD_BACKEND_WAYLAND,
};
#endif // BREAD_WAYLAND
