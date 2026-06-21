#ifndef BREAD_BACKEND_H
#define BREAD_BACKEND_H

#include <htils/basictypes.h>
#include <htils/string.h>

#include <bread/types.h>
#include <bread/window.h>

/**
 * @brief The backend_vtable.
 *
 * @details this exposes the backend vtable set in either wayland or x11 for
 * use.
 */
extern const bread_backend_vtable_t *bread_current_backend;

#endif
