#ifndef BREAD_WAYLAND_H
#define BREAD_WAYLAND_H

#ifdef BREAD_WAYLAND
#define BREAD_WAYLAND 1

/*************************************************/

#include <time.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <wayland/text-input-unstable-v3-client-protocol.h>
#include <wayland/xdg-decoration-client-protocol.h>
#include <wayland/xdg-shell-client-protocol.h>

#include <htils/basictypes.h>

#include <bread/input.h>

/*************************************************/

/** @brief Monotonic time in seconds, for autorepeat timing. */
static inline f64 bread_wayland_now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (f64)ts.tv_sec + (f64)ts.tv_nsec / 1e9;
}

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
typedef struct wl_surface_listener wl_surface_listener_t;

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

typedef struct zwp_text_input_manager_v3 zwp_text_input_manager_v3_t;
typedef struct zwp_text_input_v3 zwp_text_input_v3_t;
typedef struct zwp_text_input_v3_listener zwp_text_input_v3_listener_t;

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
 *
 * @details Holds every protocol object the backend binds (display, registry,
 * compositor, shm, xdg shell, decorations, seat/keyboard/pointer), XKB
 * keymap/state, cursor theme, and cached cursors, the input snapshot, and the
 * data-device clipboard objects and buffers. One instance per window, owned
 * by @ref bread_window_t.
 */
typedef struct {
  bread_window_t *window; /**< Owning bread window (back-pointer). */

  wl_display_t *display;       /**< The wl_display connection. */
  wl_registry_t *registry;     /**< The wl_registry. */
  wl_compositor_t *compositor; /**< The wl_compositor. */
  wl_shm_t *shm;               /**< The wl_shm pool allocator. */
  wl_surface_t *wl_surface;    /**< The window's wl_surface. */

  zwp_text_input_manager_v3_t *text_input_manager; /**< IME manager. */
  zwp_text_input_v3_t *text_input;                 /**< Seat text input. */
  b32 text_input_enabled; /**< Whether text input is enabled. */

  f32 text_input_x;        /**< Last caret rect x (logical px). */
  f32 text_input_y;        /**< Last caret rect y. */
  f32 text_input_w;        /**< Last caret rect width. */
  f32 text_input_h;        /**< Last caret rect height. */
  b32 text_input_rect_set; /**< Whether the caret rect is valid. */

  cstr text_input_surrounding[4001]; /**< Last surrounding text (UTF-8). */
  u32 text_input_cursor;             /**< Last caret byte offset. */
  u32 text_input_anchor;             /**< Last anchor byte offset. */
  b32 text_input_surrounding_set;    /**< Whether surrounding text is valid. */

  xdg_surface_t *xdg_surface;   /**< The xdg_surface role object. */
  xdg_toplevel_t *xdg_toplevel; /**< The xdg_toplevel role object. */
  xdg_wm_base_t *xdg_wm_base;   /**< The xdg_wm_base shell. */

  zxdg_decoration_manager_v1_t *decoration_manager; /**< Decoration manager. */
  zxdg_toplevel_decoration_v1_t *decoration;        /**< Toplevel decoration. */

  wl_seat_t *seat;         /**< The wl_seat. */
  wl_keyboard_t *keyboard; /**< The seat keyboard. */
  wl_pointer_t *pointer;   /**< The seat pointer. */

  xkb_context_t *xkb_context; /**< XKB context. */
  xkb_keymap_t *xkb_keymap;   /**< Compiled keymap. */
  xkb_state_t *xkb_state;     /**< Live keyboard state. */

  u16 width;        /**< Current window width in logical pixels. */
  u16 height;       /**< Current window height in logical pixels. */
  u32 refresh_mhz;  /**< Current monitor refresh rate in millihertz. */
  i32 output_scale; /**< Current output scale factor. */
  b32 running;      /**< True while the window should keep running. */

  bread_input_state_t input; /**< Latest input snapshot (pointer/keys). */

  bread_key_t repeat_key; /**< Held key for autorepeat, else BREAD_KEY_MAX. */
  i32 repeat_rate;        /**< Repeats per second from repeat_info. */
  i32 repeat_delay;       /**< ms before the first repeat. */
  f64 repeat_next;        /**< CLOCK_MONOTONIC time (s) of the next repeat. */
  i32 repeat_raw_keycode; /**< Raw evdev code of the held key. */

  wl_cursor_theme_t *cursor_theme;        /**< Loaded cursor theme. */
  wl_cursor_t *cursors[BREAD_CURSOR_MAX]; /**< Cached cursors by type. */
  wl_cursor_t *current_cursor;            /**< Currently applied cursor. */
  wl_surface_t *cursor_surface; /**< Surface used to set the cursor. */
  u32 pointer_serial;           /**< Last pointer-enter serial. */

  wl_data_device_manager_t *data_device_manager; /**< wl_data_device_manager. */
  wl_data_device_t *data_device;                 /**< The seat's data device. */
  wl_data_source_t *data_source; /**< Source while owning the selection. */
  wl_data_offer_t *data_offer;   /**< Offer for another client's selection. */

  cstr *offer_mime;     /**< Best text mime the current offer supports. */
  u32 input_serial;     /**< Last seat-event serial. */
  char *clipboard;      /**< Owned copy of the published selection. */
  u32 clipboard_len;    /**< Length of clipboard in bytes. */
  char *clipboard_read; /**< Owned buffer for text read from an owner. */
} wl_state_t;

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_H
