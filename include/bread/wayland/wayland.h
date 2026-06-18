#ifndef BREAD_WAYLAND_H
#define BREAD_WAYLAND_H
#ifdef BREAD_WAYLAND
#define BREAD_WAYLAND 1

#include <htils/basictypes.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <wayland/xdg-decoration-client-protocol.h>
#include <wayland/xdg-shell-client-protocol.h>

#include <bread/input.h>

typedef struct wl_display wl_display_t;
typedef struct wl_registry wl_registry_t;
typedef struct wl_compositor wl_compositor_t;
typedef struct wl_shm wl_shm_t;

typedef struct wl_surface wl_surface_t;

typedef struct wl_buffer wl_buffer_t;
typedef struct wl_registry wl_registry_t;
typedef struct wl_seat wl_seat_t;
typedef struct wl_keyboard wl_keyboard_t;
typedef struct wl_pointer wl_pointer_t;
typedef struct wl_array wl_array_t;
typedef struct wl_output wl_output_t;

typedef struct xkb_context xkb_context_t;
typedef struct xkb_keymap xkb_keymap_t;
typedef struct xkb_state xkb_state_t;

typedef struct xdg_wm_base_listener xdg_wm_base_listener_t;
typedef struct xdg_toplevel_listener xdg_toplevel_listener_t;
typedef struct xdg_surface_listener xdg_surface_listener_t;
typedef struct wl_buffer_listener wl_buffer_listener_t;
typedef struct wl_registry_listener wl_registry_listener_t;
typedef struct wl_keyboard_listener wl_keyboard_listener_t;
typedef struct wl_pointer_listener wl_pointer_listener_t;
typedef struct wl_seat_listener wl_seat_listener_t;
typedef struct wl_output_listener wl_output_listener_t;

typedef struct xdg_surface xdg_surface_t;
typedef struct xdg_toplevel xdg_toplevel_t;
typedef struct xdg_wm_base xdg_wm_base_t;

typedef struct zxdg_decoration_manager_v1 zxdg_decoration_manager_v1_t;
typedef struct zxdg_toplevel_decoration_v1 zxdg_toplevel_decoration_v1_t;

typedef struct {
  bread_window_t *window;

  wl_display_t *display;
  wl_registry_t *registry;
  wl_compositor_t *compositor;
  wl_shm_t *shm;
  wl_surface_t *wl_surface;

  xdg_surface_t *xdg_surface;
  xdg_toplevel_t *xdg_toplevel;
  xdg_wm_base_t *xdg_wm_base;

  zxdg_decoration_manager_v1_t *decoration_manager;
  zxdg_toplevel_decoration_v1_t *decoration;

  wl_seat_t *seat;
  wl_keyboard_t *keyboard;
  wl_pointer_t *pointer;

  xkb_context_t *xkb_context;
  xkb_keymap_t *xkb_keymap;
  xkb_state_t *xkb_state;

  u16 width;
  u16 height;
  u32 refresh_mhz;
  i32 output_scale;
  b32 running;

  bread_input_state_t input;
} wl_state_t;

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_H
