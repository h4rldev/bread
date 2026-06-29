#ifndef BREAD_WAYLAND_INPUT_H
#define BREAD_WAYLAND_INPUT_H

#include <bread/wayland/wayland.h>

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
