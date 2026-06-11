#ifndef BREAD_X11_INPUT_H
#define BREAD_X11_INPUT_H

#include <bread/types.h>
#include <bread/x11/x11.h>

#if BREAD_X11

#include <xcb/xcb.h>

b32 bread_x11_xkb_init(x11_state_t *state);
void bread_x11_xkb_cleanup(x11_state_t *state);
bread_mouse_button_t xcb_button_to_bread(u8 detail);

void bread_x11_handle_key_press(x11_state_t *state,
                                xcb_key_press_event_t *event);
void bread_x11_handle_key_release(x11_state_t *state,
                                  xcb_key_release_event_t *event);
void bread_x11_handle_button_press(x11_state_t *state,
                                   xcb_button_press_event_t *event);
void bread_x11_handle_button_release(x11_state_t *state,
                                     xcb_button_release_event_t *event);
void bread_x11_handle_motion(x11_state_t *state,
                             xcb_motion_notify_event_t *event);

#endif // !BREAD_X11
#endif // !BREAD_X11_INPUT_H
