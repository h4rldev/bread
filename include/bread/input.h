#ifndef BREAD_INPUT_H
#define BREAD_INPUT_H

#include <htils/basictypes.h>

#include <bread/event.h>
#include <bread/types.h>
#include <bread/window.h>

/**
 * @brief Get the current input state of the window.
 *
 * @details Usually used if you think getting inputs through BREAD_EVENT isn't
 * good enough.
 *
 * @param window The window to get the input state for.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init(), and you must have polled the window through @ref
 * bread_window_poll() at least once.
 *
 * @return The input state of the window.
 */
bread_input_state_t bread_window_get_input(bread_window_t *window);

/**
 * @brief Convert a bread event key to a unicode codepoint.
 *
 * @details Usually used if you wanna represent a BREAD_KEY as unicode to then
 * convert into text for UI or something similar, really only used within the
 * event callback.
 *
 * @param window The window to get the xkb state for.
 * @param event The event to get the key from.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init(), and you must have polled the window through @ref
 * bread_window_poll() at least once.
 *
 * @return The unicode codepoint of the key, or 0 if the event wasn't a key
 * press or release.
 */
u32 bread_event_key_to_unicode(bread_window_t *window, bread_event_t *event);

/**
 * @brief Convert a bread event key to a C-string.
 *
 * @details Usually used if you wanna represent a BREAD_KEY as a C-String to
 * then enter into text for UI, less flexible than @ref
 * bread_event_key_to_unicode(), but it should suffice for a lot of use cases,
 * should be used in the event callback.
 *
 * @param window The window to get the xkb state for.
 * @param event The event to get the key from.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init(), and you must have polled the window through @ref
 * bread_window_poll() at least once.
 *
 * @return The C-string representation of the key, or nullif the key wasn't a
 * valid key.
 *
 * @see bread_event_key_to_unicode().
 */
cstr *bread_event_key_to_cstr(bread_window_t *window, bread_event_t *event);

void bread_cursor_init(bread_window_t *window);

void bread_cursor_cleanup(bread_window_t *window);

/**
 * @brief Set the current pointer cursor
 *
 * @details Changes the cursor's icon based on platform, for either X11 of
 * Wayland.
 *
 * @param window The window to set the cursor for.
 * @param cursor The cursor to set.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 *
 * @note The cursor is set to the default cursor if the cursor type is invalid.
 */
void bread_set_cursor(bread_window_t *window, bread_cursor_type_t cursor);

//
//
//

/**
 * @brief Convert an evdev code to a bread key.
 *
 * @details Uses an internal lookup table to convert an evdev key to a bread
 * key.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param evdev_key The evdev key to convert.
 *
 * @pre @c evdev_key must be a valid evdev key.
 *
 * @return The bread key equivalent of the evdev key.
 */
bread_key_t bread_evdev_to_key(u32 evdev_key);

/**
 * @brief Convert an evdev code to a bread mouse button.
 *
 * @details Uses an internal lookup table to convert an evdev button to a bread
 * mouse button.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param button The evdev code to convert.
 *
 * @pre @c button must be a valid evdev code.
 *
 * @return The bread mouse button equivalent of the evdev button.
 */
bread_mouse_button_t bread_evdev_to_mouse_button(u32 button);

#endif
