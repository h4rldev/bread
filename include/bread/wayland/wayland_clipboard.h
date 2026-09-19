#ifndef BREAD_WAYLAND_CLIPBOARD_H
#define BREAD_WAYLAND_CLIPBOARD_H

/*************************************************/

#include <bread/wayland/wayland.h>

/*************************************************/

#if BREAD_WAYLAND

/**
 * @brief Create the window's data device.
 * @details Needs both the seat and the data-device manager; call once the
 * registry roundtrip has bound them. A no-op when either is missing or the
 * device already exists.
 *
 * @param state The wayland state.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_clipboard_init(wl_state_t *state);

//
//
//

/**
 * @brief Release the data source, data device, and owned text.
 *
 * @param state The wayland state.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_clipboard_cleanup(wl_state_t *state);

//
//
//

/**
 * @brief Take ownership of the clipboard and offer @c text.
 * @details Copies @c text, creates a data source offering the text mime types,
 * and sets it as the selection using the last input serial. Passing null or an
 * empty string drops ownership.
 *
 * @param state The wayland state.
 * @param text The text to copy, or null to clear.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_clipboard_set(wl_state_t *state, const cstr *text);

//
//
//

/**
 * @brief Read the clipboard text.
 * @details Returns our own copy while we own the selection, otherwise receives
 * the current offer's best text mime. The result is owned by @c state and valid
 * until the next call or @ref bread_wayland_clipboard_cleanup.
 *
 * @param state The wayland state.
 *
 * @return The text, or null when unavailable.
 *
 * @pre @c state must be a valid pointer.
 */
const cstr *bread_wayland_clipboard_get(wl_state_t *state);

#endif // BREAD_WAYLAND

#endif // !BREAD_WAYLAND_CLIPBOARD_H
