#ifndef BREAD_WAYLAND_H
#define BREAD_WAYLAND_H

#ifdef BREAD_WAYLAND
#define BREAD_WAYLAND 1

/*************************************************/

#include <htils/basictypes.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <wayland/xdg-decoration-client-protocol.h>
#include <wayland/xdg-shell-client-protocol.h>

#include <bread/input.h>

/*************************************************/

typedef struct wl_display wl_display_t;
typedef struct wl_registry wl_registry_t;
typedef struct wl_compositor wl_compositor_t;
typedef struct wl_shm wl_shm_t;

typedef struct wl_data_device_manager wl_data_device_manager_t;
typedef struct wl_data_device wl_data_device_t;
typedef struct wl_data_source wl_data_source_t;
typedef struct wl_data_offer wl_data_offer_t;
typedef struct wl_data_source_listener wl_data_source_listener_t;
typedef struct wl_data_offer_listener wl_data_offer_listener_t;
typedef struct wl_data_device_listener wl_data_device_listener_t;

typedef struct wl_surface wl_surface_t;

typedef struct wl_buffer wl_buffer_t;
typedef struct wl_registry wl_registry_t;
typedef struct wl_seat wl_seat_t;
typedef struct wl_keyboard wl_keyboard_t;
typedef struct wl_pointer wl_pointer_t;
typedef struct wl_cursor_theme wl_cursor_theme_t;
typedef struct wl_cursor wl_cursor_t;
typedef struct wl_cursor_image wl_cursor_image_t;
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

/**
 * @brief Wayland backend state.
 * @details Holds every protocol object the backend binds (display, registry,
 *   compositor, shm, xdg shell, decorations, seat/keyboard/pointer), XKB
 *   keymap/state, cursor theme and cached cursors, the input snapshot, and the
 *   data-device clipboard objects and buffers. One instance per window, owned
 *   by @ref bread_window_t.
 *
 * @param window Owning bread window (back-pointer).
 * @param display The wl_display connection.
 * @param registry The wl_registry.
 * @param compositor The wl_compositor.
 * @param shm The wl_shm pool allocator.
 * @param wl_surface The window's wl_surface.
 * @param xdg_surface The xdg_surface role object.
 * @param xdg_toplevel The xdg_toplevel role object.
 * @param xdg_wm_base The xdg_wm_base shell.
 * @param decoration_manager The xdg-decoration manager.
 * @param decoration The toplevel decoration object.
 * @param seat The wl_seat.
 * @param keyboard The seat keyboard.
 * @param pointer The seat pointer.
 * @param xkb_context XKB context.
 * @param xkb_keymap Compiled keymap.
 * @param xkb_state Live keyboard state.
 * @param width Current window width in pixels.
 * @param height Current window height in pixels.
 * @param refresh_mhz Current monitor refresh rate in millihertz.
 * @param output_scale Current output scale factor.
 * @param running True while the window should keep running.
 * @param input Latest input snapshot (pointer/keys).
 * @param cursor_theme Loaded cursor theme.
 * @param cursors Cached cursors indexed by @ref bread_cursor_type_t.
 * @param current_cursor Currently applied cursor.
 * @param cursor_surface Surface used to set the pointer cursor.
 * @param pointer_serial Last pointer-enter serial (set_cursor needs it).
 * @param data_device_manager The wl_data_device_manager.
 * @param data_device The seat's data device.
 * @param data_source Source we created while owning the selection, else null.
 * @param data_offer Offer received for another client's selection, else null.
 * @param offer_mime Best text mime the current offer supports, else null.
 * @param input_serial Last seat-event serial (set_selection needs it).
 * @param clipboard Owned copy of the text we publish as selection owner.
 * @param clipboard_len Length of @ref clipboard in bytes.
 * @param clipboard_read Owned buffer holding text read from another owner.
 */
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

  wl_cursor_theme_t *cursor_theme;
  wl_cursor_t *cursors[BREAD_CURSOR_MAX];
  wl_cursor_t *current_cursor;
  wl_surface_t *cursor_surface;
  u32 pointer_serial;

  wl_data_device_manager_t *data_device_manager;
  wl_data_device_t *data_device;
  wl_data_source_t *data_source;
  wl_data_offer_t *data_offer;

  cstr *offer_mime; // best text mime the current offer supports
  u32 input_serial; // last seat-event serial (set_selection needs it)
  char *clipboard;  // owned copy for `send`
  u32 clipboard_len;
  char *clipboard_read; // owned buffer returned by `get`
} wl_state_t;

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_H
