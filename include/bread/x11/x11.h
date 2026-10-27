#ifndef BREAD_X11_H
#define BREAD_X11_H

#ifdef BREAD_X11
#define BREAD_X11 1

/*************************************************/

#include <htils/basictypes.h>
#include <xcb-imdkit/imclient.h>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <bread/input.h>

/*************************************************/

typedef struct xkb_context xkb_context_t;
typedef struct xkb_keymap xkb_keymap_t;
typedef struct xkb_state xkb_state_t;

/**
 * @brief X11 backend state.
 *
 * @details Holds every handle the XCB/XKB backend needs: the connection and
 * window, the resolved screen and refresh rate, XKB keymap/state, the cursor
 * context and cached cursors, the input snapshot, and the CLIPBOARD ownership
 * buffers. One instance per window, owned by @ref bread_window_t.
 */
typedef struct {
  bread_window_t *window;       /**< Owning bread window (back-pointer). */
  xcb_connection_t *connection; /**< XCB connection to the display. */
  xcb_screen_t *screen;         /**< Default screen of the connection. */
  xcb_window_t xcb_window;      /**< The created X11 window. */

  u16 width;       /**< Current window width in logical pixels. */
  u16 height;      /**< Current window height in logical pixels. */
  f32 scale;       /**< Drawable pixels per logical pixel (Xft.dpi / 96). */
  u32 refresh_mhz; /**< Current monitor refresh rate in millihertz. */
  b32 running;     /**< True while the window should keep running. */

  xcb_atom_t wm_protocols;     /**< Cached WM_PROTOCOLS atom. */
  xcb_atom_t wm_delete_window; /**< Cached WM_DELETE_WINDOW atom. */
  xcb_atom_t net_wm_name;      /**< Cached _NET_WM_NAME atom. */

  xkb_context_t *xkb_context; /**< XKB context. */
  xkb_keymap_t *xkb_keymap;   /**< Compiled keymap. */
  xkb_state_t *xkb_state;     /**< Live keyboard state. */
  i32 xkb_device_id;          /**< Core keyboard device id. */

  xcb_cursor_context_t *cursor_context;   /**< XCB cursor context. */
  xcb_cursor_t current_cursor;            /**< Currently applied cursor. */
  xcb_cursor_t cursors[BREAD_CURSOR_MAX]; /**< Cached cursors by type. */

  bread_input_state_t input; /**< Latest input snapshot (pointer/keys). */

  xcb_atom_t clipboard_atom; /**< Cached CLIPBOARD atom. */
  xcb_atom_t utf8_atom;      /**< Cached UTF8_STRING atom. */
  xcb_atom_t targets_atom;   /**< Cached TARGETS atom. */
  xcb_atom_t prop_atom; /**< Scratch property atom for clipboard transfer. */

  char *clipboard;      /**< Owned copy of the published selection. */
  u32 clipboard_len;    /**< Length of clipboard in bytes. */
  b32 owns_clipboard;   /**< True while we own the CLIPBOARD selection. */
  char *clipboard_read; /**< Owned buffer for text read from an owner. */

  i32 screen_number;      /**< Screen number passed to xcb_connect. */
  xcb_xim_t *xim;         /**< XIM client, or null when unavailable. */
  xcb_xic_t xim_ic;       /**< XIM input context, or 0 before creation. */
  b32 text_input_enabled; /**< Whether text input is currently enabled. */
  cstr *text_commit;      /**< Owned NULL-terminated commit buffer. */
} x11_state_t;

#endif // !BREAD_X11
#endif // !BREAD_X11_H
