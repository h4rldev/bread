#ifndef BREAD_EVENT_H
#define BREAD_EVENT_H

#include <htils/basictypes.h>

#include <bread/types.h>
#include <bread/window.h>

void bread_window_set_event_callback(bread_window_t *window,
                                     bread_event_callback_t callback,
                                     void *userdata);

// Internal API, do not use
void fire_event(bread_window_t *window, bread_event_t *event);

#endif
