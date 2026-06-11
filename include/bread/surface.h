#ifndef BREAD_SURFACE_H
#define BREAD_SURFACE_H

#include <bread/window.h>

bread_surface_t bread_window_get_surface(bread_window_t *window);
bread_backend_type_t bread_window_get_backend_type(bread_window_t *window);

#endif
