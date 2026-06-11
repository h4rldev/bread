#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

#include <string.h>

#include <htils/basictypes.h>
#include <htils/string.h>

#include <bread/backend.h>
#include <bread/event.h>
#include <bread/input.h>
#include <bread/surface.h>
#include <bread/wayland/input.h>
#include <bread/window.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland/xdg-shell-client-protocol.h>

static void xdg_wm_base_ping(void *data, xdg_wm_base_t *wm, u32 serial) {
  xdg_wm_base_pong(wm, serial);
}

static const xdg_wm_base_listener_t xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void xdg_toplevel_close(void *data, xdg_toplevel_t *toplevel) {
  wl_state_t *state = data;
  state->running = false;

  bread_event_t event = {.type = BREAD_EVENT_WINDOW_CLOSE};
  fire_event(state->window, &event);
}

static void xdg_toplevel_configure(void *data, xdg_toplevel_t *toplevel,
                                   i32 width, i32 height, wl_array_t *states) {
  wl_state_t *state = data;
  if (width > 0 && height > 0) {
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
  xdg_surface_ack_configure(surface, serial);
}

static const xdg_surface_listener_t xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void global_registry_handler(void *data, wl_registry_t *registry, u32 id,
                                    const cstr *interface, u32 version) {
  wl_state_t *state = (wl_state_t *)data;

  if (strcmp(interface, wl_compositor_interface.name) == 0)
    state->compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 4);

  else if (strcmp(interface, wl_shm_interface.name) == 0)
    state->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);

  else if (strcmp(interface, wl_seat_interface.name) == 0)
    bread_wayland_seat_bind(state, registry, id, version);

  else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    state->xdg_wm_base =
        wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
  }
}

static void global_registry_remover(void *data, wl_registry_t *registry,
                                    u32 id) {}

static void wl_state_cleanup(wl_state_t *state) {
  if (state->xdg_toplevel)
    xdg_toplevel_destroy(state->xdg_toplevel);

  if (state->xdg_surface)
    xdg_surface_destroy(state->xdg_surface);

  if (state->wl_surface)
    wl_surface_destroy(state->wl_surface);

  if (state->xdg_wm_base)
    xdg_wm_base_destroy(state->xdg_wm_base);

  if (state->shm)
    wl_shm_destroy(state->shm);

  if (state->compositor)
    wl_compositor_destroy(state->compositor);

  if (state->registry)
    wl_registry_destroy(state->registry);

  if (state->display)
    wl_display_disconnect(state->display);

  if (state->seat)
    bread_wayland_seat_cleanup(state);
}

static const wl_registry_listener_t registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remover,
};

static void wayland_init(bread_window_t *window) {
  arena_t *arena = window->arena;

  wl_state_t *state = arena_alloc(arena, wl_state_t, 1);
  window->backend = state;

  state->display = wl_display_connect(NULL);
  if (!state->display) {
    fprintf(stderr, "bread-wayland: Failed to connect to display socket.\n");
    return;
  }

  state->width = window->width;
  state->height = window->height;

  state->registry = wl_display_get_registry(state->display);
  wl_registry_add_listener(state->registry, &registry_listener, state);

  bread_wayland_seat_init(state);

  wl_display_roundtrip(state->display);
  if (!state->compositor) {
    fprintf(stderr, "bread-wayland: No wl_compositor found.\n");
    return;
  }

  state->wl_surface = wl_compositor_create_surface(state->compositor);
  state->xdg_surface =
      xdg_wm_base_get_xdg_surface(state->xdg_wm_base, state->wl_surface);
  xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);

  state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
  xdg_toplevel_add_listener(state->xdg_toplevel, &xdg_toplevel_listener, state);

  if (window->title) {
    xdg_toplevel_set_title(state->xdg_toplevel, string_to_cstr(window->title));
  }

  wl_surface_commit(state->wl_surface);
  wl_display_roundtrip(state->display);

  state->window = window;
  state->running = true;
}

static void wayland_poll_events(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state)
    return;

  while (wl_display_prepare_read(state->display) != 0)
    wl_display_dispatch_pending(state->display);

  wl_display_flush(state->display);
  wl_display_cancel_read(state->display);
  wl_display_dispatch_pending(state->display);
}

static b32 wayland_should_close(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state)
    return true;
  return !state->running;
}

static void wayland_destroy(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state)
    return;
  wl_state_cleanup(state);
  window->backend = NULL;
}

static bread_surface_t wayland_get_surface(bread_window_t *window) {
  wl_state_t *state = window->backend;
  return (bread_surface_t){.handle = state->wl_surface,
                           .display = state->display};
}

const bread_backend_vtable_t bread_wayland_backend = {
    .init = wayland_init,
    .poll_events = wayland_poll_events,
    .should_close = wayland_should_close,
    .destroy = wayland_destroy,
    .get_surface = wayland_get_surface,
    .backend_type = BREAD_BACKEND_WAYLAND,
};

#undef BREAD_INTERNAL
#endif // BREAD_WAYLAND
