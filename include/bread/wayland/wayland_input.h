#ifndef BREAD_WAYLAND_INPUT_H
#define BREAD_WAYLAND_INPUT_H

/*************************************************/

#include <htils/basictypes.h>

#include <bread/wayland/wayland.h>

/*************************************************/

#if BREAD_WAYLAND

/**
 * @brief Initializes the wayland seat.
 *
 * @details By creating a new xkb_context for the current wl_state.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param state The wl_state to initialize.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_seat_init(wl_state_t *state);

//
//
//

/**
 * @brief Bind the wayland seat.
 *
 * @details Binds the seat to the wayland registry, and adds the seat listener
 * to it.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param state The wl_state to bind.
 * @param registry The wayland registry to bind to.
 * @param name The name of the seat to bind to.
 * @param version The version of the seat to bind to.
 *
 * @pre
 * - @c state must be a valid pointer and properly initialized through @ref
 * bread_wayland_seat_init().
 * - @c registry must be a valid pointer to the current process's wayland
 * registry.
 *
 * @see @ref bread_wayland_seat_init()
 */
void bread_wayland_seat_bind(wl_state_t *state, wl_registry_t *registry,
                             u32 name, u32 version);

//
//
//

/**
 * @brief Cleans up the wayland seat.
 *
 * @details Releasing the keyboard, mouse pointer, and seat if they're non-null,
 * then unrefs the entire xkb context.
 *
 * @param state The wl_state to clean up.
 *
 * @pre @c state must be a valid pointer and properly initialized through @ref
 * bread_wayland_seat_init().
 *
 * @see @ref bread_wayland_seat_init()
 */
void bread_wayland_seat_cleanup(wl_state_t *state);

//
//
//

/**
 * @brief Enables Wayland text input.
 *
 * @details Lazily creates the seat's text input, enables it, anchors the IME
 * candidate window at the given rectangle, and commits the state.
 * Lazily creates the seat's text input, enables it, retains the caret
 * rectangle for the focus-enter restore, anchors the IME candidate window at
 * the given rectangle, and commits the state.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param state The wl_state to enable text input for.
 * @param x The caret rectangle x, in logical surface coordinates.
 * @param y The caret rectangle y, in logical surface coordinates.
 * @param w The caret rectangle width, in logical surface coordinates.
 * @param h The caret rectangle height, in logical surface coordinates.
 *
 * @pre @c state must be a valid pointer and initialized through @ref
 * bread_wayland_seat_bind().
 *
 * @see bread_wayland_text_input_disable()
 */
void bread_wayland_text_input_enable(wl_state_t *state, f32 x, f32 y, f32 w,
                                     f32 h);

//
//
//

/**
 * @brief Disables Wayland text input.
 *
 * @details Disables the seat's text input and clears the retained caret
 * rectangle and surrounding text.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param state The wl_state to disable text input for.
 *
 * @pre @c state must be a valid pointer.
 *
 * @see bread_wayland_text_input_enable()
 */
void bread_wayland_text_input_disable(wl_state_t *state);

//
//
//

/**
 * @brief Sets the surrounding text for the compositor's IME.
 *
 * @details Sends the editing buffer (excluding preedit) with the caret and
 * selection anchor as byte offsets, clamped to the protocol's 4000-byte text
 * limit, and commits it. No-op unless text input is enabled.
 *
 * @note For internal use, no real reason to use this when using the library.
 *
 * @param state The wl_state to set surrounding text for.
 * @param text The editing buffer, excluding preedit, in UTF-8.
 * @param cursor The caret byte offset into @p text.
 * @param anchor The selection anchor byte offset into @p text.
 *
 * @pre @c state must be a valid pointer.
 *
 * @see bread_wayland_text_input_enable()
 */
void bread_wayland_text_input_set_surrounding(wl_state_t *state,
                                              const cstr *text, u32 cursor,
                                              u32 anchor);

//
//
//

/**
 * @brief Initializes the wayland cursor.
 *
 * @details Finds the cursor theme, cursor size, and loads them, then loads each
 * cursor and image from that cursor, then creates a cursor surface, and sets
 * the current cursor to the default cursor.
 *
 * @param state The wl_state to initialize.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_cursor_init(wl_state_t *state);

//
//
//

/**
 * @brief Cleans up the wayland cursor.
 *
 * @details Cleans up the cursor theme, and cursor surface.
 *
 * @param state The wl_state to clean up.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_cursor_cleanup(wl_state_t *state);

//
//
//

/**
 * @brief Sets the cursor to the given cursor type.
 *
 * @details Gets the cursor for the cursor type, gets the image, buffer of the
 * cursor, then setting the cursor to the image coordinates, attaches the buffer
 * to the surface, damages the surface with the coordinates, and then commits
 * it.
 *
 * @param state The wl_state to set the cursor for.
 * @param cursor The cursor type to set.
 *
 * @pre @c state must be a valid pointer.
 */
void bread_wayland_set_cursor(wl_state_t *state, bread_cursor_type_t cursor);

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_INPUT_H
