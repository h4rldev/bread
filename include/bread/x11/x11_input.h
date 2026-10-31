#ifndef BREAD_X11_INPUT_H
#define BREAD_X11_INPUT_H

/*************************************************/

#include <bread/types.h>
#include <bread/x11/x11.h>

#if BREAD_X11

#include <xcb/xcb.h>

/*************************************************/

/**
 * @brief Initializes the X11 xkb.
 *
 * @details Initializes a new xcb context, extension, device_id, keymap, and
 * state.
 *
 * @param state The state to initialize.
 *
 * @pre @c state must not be null.
 */
b32 bread_x11_xkb_init(x11_state_t *state);

//
//
//

/**
 * @brief Converts an xcb mouse press detail to a bread mouse button.
 *
 * @details Converts the detail field of an xcb mouse press event to a bread
 * mouse button field.
 *
 * @param detail The detail field of the event.
 *
 * @pre @c detail must be between 1 and 9.
 *
 * @return The bread mouse button.
 */
bread_mouse_button_t xcb_button_to_bread(u8 detail);

//
//
//

/**
 * @brief Handles a key press event.
 *
 * @details Updates the input state, fires a bread event, and updates the xkb
 * state if it's available.
 *
 * @param state The state to update.
 * @param event The event to handle.
 *
 * @pre @c state, and @c event must not be null.
 */
void bread_x11_handle_key_press(x11_state_t *state,
                                xcb_key_press_event_t *event);

//
//
//

/**
 * @brief Handles a key release event.
 *
 * @details Updates the input state, fires a bread event, and updates the xkb
 * state if it's available.
 *
 * @param state The state to update.
 * @param event The event to handle.
 *
 * @pre @c state, and @c event must not be null.
 */
void bread_x11_handle_key_release(x11_state_t *state,
                                  xcb_key_release_event_t *event);

//
//
//

/**
 * @brief Handles a mouse button press event.
 *
 * @details Updates the input state, and sends a bread event, if detail is 4 or
 * 5 it will increment or decrement the scroll_y field.
 *
 * @param state The state to update.
 * @param event The event to handle.
 *
 * @pre @c state, and @c event must not be null.
 */
void bread_x11_handle_button_press(x11_state_t *state,
                                   xcb_button_press_event_t *event);

//
//
//

/**
 * @brief Handles a mouse button release event.
 *
 * @details Updates the input state, and sends a bread event. If detail is 4 or
 * 5 it will be ignored.
 *
 * @param state The state to update.
 * @param event The event to handle.
 *
 * @pre @c state, and @c event must not be null.
 */
void bread_x11_handle_button_release(x11_state_t *state,
                                     xcb_button_release_event_t *event);

//
//
//

/**
 * @brief Handles a mouse motion event.
 *
 * @details Updates the input state, and sends a bread event.
 *
 * @param state The state to update
 * @param event The event to handle.
 *
 * @pre @c state, and @c event must not be null.
 */
void bread_x11_handle_motion(x11_state_t *state,
                             xcb_motion_notify_event_t *event);

//
//
//

/**
 * @brief Initializes the X11 cursor.
 *
 * @details Initializes the xcb cursor context, and loads the cursor theme, and
 * sets the cursor to default.
 *
 * @param state The state to initialize.
 *
 * @pre @c state must not be null.
 */
void bread_x11_cursor_init(x11_state_t *state);

//
//
//

/**
 * @brief Cleans up the X11 cursor.
 *
 * @details Cleans up the xcb cursor context.
 *
 * @param state The state to clean up.
 *
 * @pre @c state must not be null.
 */
void bread_x11_cursor_cleanup(x11_state_t *state);

//
//
//

/**
 * @brief Sets the cursor to the given cursor type.
 *
 * @details Sets the cursor to the given cursor type, and updates the cursor.
 *
 * @param state The state to set the cursor for.
 * @param cursor The cursor type to set.
 *
 * @pre @c state must not be null and have an initialized cursor context.
 */
void bread_x11_set_cursor(x11_state_t *state, bread_cursor_type_t cursor);

//
//
//

/**
 * @brief Initializes the X11 input method.
 *
 * @details Creates and opens the XIM client. The input context is created
 * asynchronously by xim_open_callback.
 *
 * @param state The x11 state to initialize.
 *
 * @return true on success, false when no IM is available.
 *
 * @pre @c state must be valid and connected.
 */
b32 bread_x11_ime_init(x11_state_t *state);

//
//
//

/**
 * @brief Filters an X event through the IM.
 *
 * @details Must run before normal dispatch. Returns true when the IM consumed
 * the event. While a text field is editing, key events are handed to the
 * input context and consumed here; the IM forwards the keys it does not use
 * back through the forward-event callback, which dispatches them normally.
 *
 * @param state The x11 state.
 * @param event The event to filter.
 *
 * @return true when the event was consumed by the IM.
 *
 * @pre @c state and @c event must be valid.
 */
b32 bread_x11_ime_filter(x11_state_t *state, xcb_generic_event_t *event);

//
//
//

/**
 * @brief Applies window focus to the IM.
 *
 * @param state The x11 state.
 * @param focused Whether the window gained focus.
 *
 * @pre @c state must be valid.
 */
void bread_x11_ime_focus(x11_state_t *state, b32 focused);

//
//
//

/**
 * @brief Cleans up the X11 input method.
 *
 * @details Destroys the input context, closes and destroys the XIM client, and
 * frees the commit buffer. Safe to call when the IM was never created.
 *
 * @param state The x11 state to clean up.
 *
 * @pre @c state must be valid.
 */
void bread_x11_ime_cleanup(x11_state_t *state);

//
//
//

/**
 * @brief Enables X11 text input.
 *
 * @details Focuses the input context and moves the IM spot to the caret.
 *
 * @param state The x11 state.
 * @param x The caret x in logical coordinates.
 * @param y The caret y in logical coordinates.
 * @param w Unused caret width.
 * @param h Unused caret height.
 *
 * @pre @c state must be valid.
 */
void bread_x11_text_input_enable(x11_state_t *state, f32 x, f32 y, f32 w,
                                 f32 h);

//
//
//

/**
 * @brief Disables X11 text input.
 *
 * @details Unfocuses the input context. Safe to call when it was never enabled.
 *
 * @param state The x11 state.
 *
 * @pre @c state must be valid.
 */
void bread_x11_text_input_disable(x11_state_t *state);

#endif // !BREAD_X11
#endif // !BREAD_X11_INPUT_H
