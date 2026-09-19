#ifndef BREAD_X11_H
#define BREAD_X11_H

#ifdef BREAD_X11
#define BREAD_X11 1

/*************************************************/

#include <htils/basictypes.h>
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
 * @details Holds every handle the XCB/XKB backend needs: the connection and
 *   window, the resolved screen and refresh rate, XKB keymap/state, the cursor
 *   context and cached cursors, the input snapshot, and the CLIPBOARD ownership
 *   buffers. One instance per window, owned by @ref bread_window_t.
 *
 * @param window Owning bread window (back-pointer).
 * @param connection XCB connection to the display.
 * @param screen Default screen of the connection.
 * @param xcb_window The created X11 window.
 * @param width Current window width in pixels.
 * @param height Current window height in pixels.
 * @param refresh_mhz Current monitor refresh rate in millihertz.
 * @param running True while the window should keep running.
 * @param wm_protocols Cached WM_PROTOCOLS atom.
 * @param wm_delete_window Cached WM_DELETE_WINDOW atom.
 * @param net_wm_name Cached _NET_WM_NAME atom.
 * @param xkb_context XKB context.
 * @param xkb_keymap Compiled keymap.
 * @param xkb_state Live keyboard state.
 * @param xkb_device_id Core keyboard device id.
 * @param cursor_context XCB cursor context.
 * @param current_cursor Currently applied cursor.
 * @param cursors Cached cursors indexed by @ref bread_cursor_type_t.
 * @param input Latest input snapshot (pointer/keys).
 * @param clipboard_atom Cached CLIPBOARD atom.
 * @param utf8_atom Cached UTF8_STRING atom.
 * @param targets_atom Cached TARGETS atom.
 * @param prop_atom Scratch property atom used for clipboard transfer.
 * @param clipboard Owned copy of the text we publish as selection owner.
 * @param clipboard_len Length of @ref clipboard in bytes.
 * @param owns_clipboard True while this window owns the CLIPBOARD selection.
 * @param clipboard_read Owned buffer holding text read from another owner.
 */
typedef struct {
  bread_window_t *window;
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t xcb_window;

  u16 width;
  u16 height;
  u32 refresh_mhz;
  b32 running;

  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_window;
  xcb_atom_t net_wm_name;

  xkb_context_t *xkb_context;
  xkb_keymap_t *xkb_keymap;
  xkb_state_t *xkb_state;
  i32 xkb_device_id;

  xcb_cursor_context_t *cursor_context;
  xcb_cursor_t current_cursor;
  xcb_cursor_t cursors[BREAD_CURSOR_MAX];

  bread_input_state_t input;

  xcb_atom_t clipboard_atom;
  xcb_atom_t utf8_atom;
  xcb_atom_t targets_atom;
  xcb_atom_t prop_atom;
  char *clipboard;
  u32 clipboard_len;
  b32 owns_clipboard;
  char *clipboard_read;
} x11_state_t;

#endif // !BREAD_X11
#endif // !BREAD_X11_H
