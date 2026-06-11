#include <bread/event.h>
#include <bread/types.h>
#include <bread/window.h>

void bread_window_set_event_callback(bread_window_t *window,
                                     bread_event_callback_t callback,
                                     void *userdata) {
  window->event_callback = callback;
  window->event_userdata = userdata;
}

void fire_event(bread_window_t *window, bread_event_t *event) {
  if (window->event_callback)
    window->event_callback(event, window->event_userdata);
}
