#ifndef BREAD_WAYLAND_OUTPUT_H
#define BREAD_WAYLAND_OUTPUT_H

#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

#include <htils/basictypes.h>

void bread_wayland_output_bind(wl_state_t *state, wl_registry_t *registry,
                               u32 name, u32 version);

#endif // !BREAD_WAYLAND
#endif // !BREAD_WAYLAND_OUTPUT_H
