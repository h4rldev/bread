#ifndef BREAD_EVENT_H
#define BREAD_EVENT_H

#include <htils/basictypes.h>

#include <bread/types.h>
#include <bread/window.h>

/**
 * @brief Set the event callback of the window.
 *
 * @details Which is called whenever an event is fired through @ref fire_event()
 * in @ref bread_window_poll().
 *
 * @param window The window to set the event callback for.
 * @param callback The callback function to call when an event is fired.
 * @param userdata The userdata passed to the callback function.
 *
 * @pre
 * - @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 * - @c callback must be a valid function pointer with the correct signature.
 * - @c userdata must be a valid pointer or null.
 *
 * @see @ref bread_window_poll(), @ref fire_event().
 */
void bread_window_set_event_callback(bread_window_t *window,
                                     bread_event_callback_t callback,
                                     void *userdata);

/**
 * @brief Fire an event.
 *
 * @details Sends an event and then runs the event callback if set.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param window The window to fire the event for.
 * @param event The event to fire.
 *
 * @pre
 * - @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 * - @c event must be a valid pointer to a valid event.
 */
void fire_event(bread_window_t *window, bread_event_t *event);

#endif
