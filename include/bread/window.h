#ifndef BREAD_WINDOW_H
#define BREAD_WINDOW_H

#include <bread/types.h>

void bread_window_init(bread_window_t *window);
void bread_window_poll(bread_window_t *window);
b32 bread_window_should_close(bread_window_t *window);
void bread_window_destroy(bread_window_t *window);
bread_surface_t bread_window_get_surface(bread_window_t *window);
bread_backend_type_t bread_get_backend_type(void);

/* Returns the refresh rate reported by the compositor.
   This may NOT reflect the actual refresh rate on high-Hz or VRR monitors.
   The renderer should use swapchain presentation timing for accurate frame
   pacing. */
u16 bread_window_get_refresh_rate(bread_window_t *window);

#endif
