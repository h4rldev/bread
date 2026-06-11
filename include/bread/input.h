#ifndef BREAD_INPUT_H
#define BREAD_INPUT_H

#include <htils/basictypes.h>

#include <bread/event.h>
#include <bread/types.h>
#include <bread/window.h>

bread_input_state_t bread_window_get_input(bread_window_t *window);
u32 bread_event_key_to_unicode(bread_window_t *window, bread_event_t *event);

/* ── Internal: used by backends, not part of the public API ── */
bread_key_t bread_evdev_to_key(u32 evdev_key);
bread_mouse_button_t bread_evdev_to_mouse_button(u32 button);

#endif
