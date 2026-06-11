#ifndef BREAD_X11_H
#define BREAD_X11_H
#ifdef BREAD_X11

#define BREAD_X11 1

#include <htils/basictypes.h>
#include <xcb/xcb.h>
#include <xkbcommon/xkbcommon.h>

#include <bread/input.h>

typedef struct xkb_context xkb_context_t;
typedef struct xkb_keymap xkb_keymap_t;
typedef struct xkb_state xkb_state_t;

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

  xkb_context_t *xkb_context;
  xkb_keymap_t *xkb_keymap;
  xkb_state_t *xkb_state;
  i32 xkb_device_id;

  bread_input_state_t input;
} x11_state_t;

#endif // !BREAD_X11
#endif // !BREAD_X11_H
