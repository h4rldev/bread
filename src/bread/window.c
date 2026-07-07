#include <bread/backend.h>
#include <bread/input.h>
#include <bread/log.h>
#include <bread/types.h>
#include <bread/window.h>

#ifdef BREAD_WAYLAND
#include <bread/wayland/wayland.h>
extern const bread_backend_vtable_t bread_wayland_backend;
#endif

#ifdef BREAD_X11
#include <bread/x11/x11.h>
extern const bread_backend_vtable_t bread_x11_backend;
#endif

static const bread_backend_vtable_t *get_backend_vtable(void) {
#if BREAD_WAYLAND
  if (!bread_wayland_backend.destroy || !bread_wayland_backend.init ||
      !bread_wayland_backend.poll_events ||
      !bread_wayland_backend.should_close || !bread_wayland_backend.set_title ||
      !bread_wayland_backend.set_min_size || !bread_wayland_backend.get_surface)
    return null;

  return &bread_wayland_backend;
#elif BREAD_X11
  if (!bread_x11_backend.destroy || !bread_x11_backend.init ||
      !bread_x11_backend.poll_events || !bread_x11_backend.should_close ||
      !bread_x11_backend.set_title || !bread_x11_backend.set_min_size ||
      !bread_x11_backend.get_surface)
    return null;

  return &bread_x11_backend;
#else
#error "No windowing system defined, what are you using?"
#endif
}

void bread_window_init(bread_window_t *window) {
  if (!window) {
    bread_log_error("Missing values, can't init window");
    return;
  }

  bread_log_debug("Initializing window");
  get_backend_vtable()->init(window);
  bread_cursor_init(window);
}

void bread_window_set_title(bread_window_t *window, const char *title) {
  if (!window || !title || !*title) {
    bread_log_error("Missing values, can't set title");
    return;
  }

  bread_log_debug("Setting window title to '%s'", title);
  get_backend_vtable()->set_title(window, title);
}

void bread_window_set_min_size(bread_window_t *window, u16 width, u16 height) {
  if (!window) {
    bread_log_error("Missing values, can't set min size");
    return;
  }

  bread_log_debug("Setting window min size to %ux%u", width, height);
  get_backend_vtable()->set_min_size(window, width, height);
}

void bread_window_poll(bread_window_t *window) {
  if (!window) {
    bread_log_error("Missing values, can't poll");
    return;
  }

  get_backend_vtable()->poll_events(window);
}

b32 bread_window_should_close(bread_window_t *window) {
  if (!window) {
    bread_log_error("Missing values, can't check if should close");
    return false;
  }

  return get_backend_vtable()->should_close(window);
}

void bread_window_destroy(bread_window_t *window) {
  if (!window) {
    bread_log_error("Missing values, can't destroy window");
    return;
  }

  bread_log_debug("Destroying window");
  get_backend_vtable()->destroy(window);
  bread_cursor_cleanup(window);
}

void bread_window_clamp_size(bread_window_t *window, u32 *width, u32 *height) {
  if (!window || !width || !height) {
    bread_log_error("Missing values, can't clamp size");
    return;
  }

  bread_log_debug("Clamping window size");
  if (*width < window->min_width)
    *width = window->min_width;
  if (*height < window->min_height)
    *height = window->min_height;
}

bread_surface_t bread_window_get_surface(bread_window_t *window) {
  if (!window) {
    bread_log_error("Missing values, can't get surface");
    return (bread_surface_t){0};
  }

  bread_log_debug("Getting surface");
  return get_backend_vtable()->get_surface(window);
}

bread_backend_type_t bread_get_backend_type(void) {
  bread_log_debug("Getting backend type");
  return get_backend_vtable()->backend_type;
}
