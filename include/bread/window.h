#ifndef BREAD_WINDOW_H
#define BREAD_WINDOW_H

#include <bread/types.h>

void bread_window_init(bread_window_t *window);
void bread_window_poll(bread_window_t *window);
b32 bread_window_should_close(bread_window_t *window);
void bread_window_destroy(bread_window_t *window);
bread_surface_t bread_window_get_surface(bread_window_t *window);
bread_backend_type_t bread_get_backend_type(void);

#endif
