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

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_INPUT_H
