#ifndef BREAD_WINDOW_H
#define BREAD_WINDOW_H

#include <bread/types.h>

/**
 * @brief Initializes a window.
 *
 * @details Initializes a window based on the target backend.
 *
 * @param window The window to initialize.
 *
 * @pre
 *  - @c window must be a valid pointer.
 *  - @c window must have a valid arena initialized and populated into the
 * struct.
 *  - @c window must have an initial width and height set.
 *
 *  @see @ref wayland_window_init() and @ref x11_window_init().
 */
void bread_window_init(bread_window_t *window);

/**
 * @brief Poll for window events.
 *
 * @details Polls for keyboard, mouse, close and resize events and fires them to
 * the @ref bread_event_callback passed on @ref
 * bread_window_set_event_callback().
 *
 * @param window The window to poll.
 *
 * @pre @c Window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 *
 * @see @ref bread_window_set_event_callback(), @ref wayland_poll_events(), @ref
 * x11_poll_events().
 */
void bread_window_poll(bread_window_t *window);

/**
 * @brief Sets the window title
 *
 * @details Doesn't matter where you're calling this, and the window title will
 * be updated based on the target backend.
 *
 * @param window The window to set the title for.
 * @param title The title to set.
 *
 * @pre
 * - @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 * - @c title must be a valid pointer to a null-terminated C-String.
 *
 * @see @ref wayland_window_set_title() and @ref x11_window_set_title()
 */
void bread_window_set_title(bread_window_t *window, const cstr *title);

/**
 * @brief Sets the minimum size of the window.
 *
 * @details Doesn't matter where you're calling this, and the minimum size will
 * be updated accordingly.
 *
 * @param window The window to set the minimum size for.
 * @param width The minimum width of the window.
 * @param height The minimum height of the window.
 *
 * @pre
 * - @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 * - @c height and @c width can be 0, but then what's the point?
 */
void bread_window_set_min_size(bread_window_t *window, u16 width, u16 height);

/**
 * @brief Checks if the window should close.
 *
 * @details Useful for making a window event loop, and it's the expected way to
 * do so.
 *
 * @param window The window to check.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 *
 * @return true if the window should close, false otherwise.
 *
 * @see @ref wayland_should_close() and @ref x11_should_close().
 */
b32 bread_window_should_close(bread_window_t *window);

/**
 * @brief Destroys a window.
 *
 * @details Fully cleans up the window and the target backend, although it
 * doesn't free the arena, so be sure to free it yourself, or don't, it's
 * mmap()'d so the kernel usually handles it.
 *
 * @param window The window to destroy.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 *
 * @see @ref wayland_destroy() and @ref x11_destroy().
 */
void bread_window_destroy(bread_window_t *window);

/**
 * @brief Clamps the sizes passed to the minimum size.
 *
 * @details If the passed sizes is smaller the minimum, it clamps it.
 *
 * @param window The window to clamp the size for.
 * @param width The width to clamp.
 * @param height The height to clamp.
 *
 * @pre
 * - @c window must be a valid pointer and properly initialized through @ref
 * - @c width and @c height must be valid pointers.
 */

void bread_window_clamp_size(bread_window_t *window, u32 *width, u32 *height);

//
//
//

/**
 * @brief Gets the surface of the window to render to.
 *
 * @param window The window to get the surface for.
 *
 * @pre @c window must be a valid pointer and properly initialized through @ref
 * bread_window_init().
 *
 * @return The surface of the window, based on the target backend.
 *
 * @see @ref wayland_get_surface() and @ref x11_get_surface().
 */
bread_surface_t bread_window_get_surface(bread_window_t *window);

/**
 * @brief Gets the backend type of the bread build.
 *
 * @details Which can be either @ref BREAD_BACKEND_WAYLAND or @ref
 * BREAD_BACKEND_X11.
 *
 * @return The backend type of the bread build.
 */
bread_backend_type_t bread_get_backend_type(void);

#endif
