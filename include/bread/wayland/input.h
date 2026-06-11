#ifndef BREAD_WAYLAND_INPUT_H
#define BREAD_WAYLAND_INPUT_H

#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

void bread_wayland_seat_init(wl_state_t *state);
void bread_wayland_seat_bind(wl_state_t *state, wl_registry_t *registry,
                             u32 name, u32 version);
void bread_wayland_seat_cleanup(wl_state_t *state);

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_INPUT_H
