#ifndef BREAD_X11_CLIPBOARD_H
#define BREAD_X11_CLIPBOARD_H

/*************************************************/

#include <bread/x11/x11.h>

/*************************************************/

#if BREAD_X11

/**
 * @brief Take ownership of the CLIPBOARD selection and set @c text.
 * @details Copies @c text and claims the selection on the window. Passing null
 * or an empty string releases ownership.
 *
 * @param state The X11 state.
 * @param text The text to copy, or null to clear.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_x11_clipboard_set(x11_state_t *state, const cstr *text);

//
//
//

/**
 * @brief Read the CLIPBOARD selection text.
 * @details Returns our own copy while we own the selection; otherwise converts
 * the selection and waits for the owner's reply. The result is owned by @c
 * state and valid until the next call or @ref bread_x11_clipboard_cleanup. The
 * wait consumes events on the connection, so pending input arriving mid-read is
 * dropped.
 *
 * @param state The X11 state.
 *
 * @return The text, or null when unavailable.
 *
 * @pre @c state must be a valid pointer.
 */
const cstr *bread_x11_clipboard_get(x11_state_t *state);

//
//
//

/**
 * @brief Release the owned text buffers and drop ownership.
 *
 * @param state The X11 state.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_x11_clipboard_cleanup(x11_state_t *state);

//
//
//

/**
 * @brief Answer a SelectionRequest from another client.
 * @details Writes the requested data (the text, or the TARGETS list) into the
 * requestor's property and replies with a SelectionNotify.
 *
 * @param state The X11 state.
 * @param request The SelectionRequest event.
 *
 * @pre @c state and @c request must be valid pointers.
 */
void bread_x11_clipboard_request(x11_state_t *state,
                                 xcb_selection_request_event_t *request);

//
//
//

/**
 * @brief Drop ownership when another client takes the selection.
 *
 * @param state The X11 state.
 * @param clear The SelectionClear event.
 *
 * @pre @c state and @c clear must be valid pointers.
 */
void bread_x11_clipboard_clear(x11_state_t *state,
                               xcb_selection_clear_event_t *clear);

#endif // BREAD_X11

#endif // !BREAD_X11_CLIPBOARD_H
