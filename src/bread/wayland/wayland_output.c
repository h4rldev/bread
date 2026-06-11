#include <bread/wayland/wayland.h>

#if BREAD_WAYLAND

#include <htils/arena.h>
#include <htils/basictypes.h>

#include <bread/window.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

typedef struct {
  wl_state_t *state;
  u32 current_width;
  u32 current_height;
  u32 current_refresh_mhz;
  b32 has_current;
} output_data_t;

static void output_geometry(void *data, wl_output_t *output, i32 x, i32 y,
                            i32 physical_width, i32 physical_height,
                            i32 subpixel, const cstr *make, const cstr *model,
                            i32 transform) {}
static void output_mode(void *data, wl_output_t *output, u32 flags, i32 width,
                        i32 height, i32 refresh) {
  output_data_t *output_data = data;

  if (flags & WL_OUTPUT_MODE_CURRENT) {
    fprintf(stderr, "Found current, updating\n");
    output_data->current_width = width;
    output_data->current_height = height;
    output_data->current_refresh_mhz = refresh;
    output_data->has_current = true;
  }
}

static void output_done(void *data, wl_output_t *output) {
  output_data_t *output_data = data;
  if (output_data->has_current) {
    output_data->state->width = output_data->current_width;
    output_data->state->height = output_data->current_height;
    output_data->state->refresh_mhz = output_data->current_refresh_mhz;
  }
}

static void output_scale(void *data, wl_output_t *output, i32 scale) {
  output_data_t *output_data = data;
  output_data->state->output_scale = scale;
}

static const wl_output_listener_t output_listener = {
    .geometry = output_geometry,
    .mode = output_mode,
    .done = output_done,
    .scale = output_scale,
};

void bread_wayland_output_bind(wl_state_t *state, wl_registry_t *registry,
                               u32 name, u32 version) {
  wl_output_t *output = wl_registry_bind(registry, name, &wl_output_interface,
                                         version < 2 ? version : 2);

  output_data_t *output_data =
      arena_alloc(state->window->arena, output_data_t, 1);
  output_data->state = state;
  output_data->has_current = false;

  wl_output_add_listener(output, &output_listener, output_data);
}

#endif // !BREAD_WAYLAND
