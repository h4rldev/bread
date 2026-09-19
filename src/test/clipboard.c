/*************************************************/

#include <stdio.h>

#include <bread/event.h>
#include <bread/types.h>
#include <bread/window.h>

#if BREAD_WAYLAND
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <bread/wayland/wayland.h>
#endif

/*************************************************/

#if BREAD_WAYLAND

static void *test_pixels = null;
static u32 test_pixels_size = 0;
static wl_buffer_t *test_buffer = null;

/**
 * @brief Attaches a temporary software buffer to the window surface.
 *
 * @details Wayland only gives a surface keyboard focus (and therefore an input
 * serial) once it is mapped, which needs a buffer. bread never attaches one, so
 * the test attaches a solid-colour shared-memory buffer itself; no renderer is
 * involved. Reuses bread's already-bound @c wl_shm.
 *
 * @param window The window to attach the buffer to.
 */
static void software_surface_attach(bread_window_t *window) {
  wl_state_t *state = window->backend;
  if (!state || !state->shm || !state->wl_surface) {
    return;
  }

  u32 w = window->width;
  u32 h = window->height;
  u32 stride = w * 4;
  test_pixels_size = stride * h;

  char name[64];
  snprintf(name, sizeof(name), "/bread-clipboard-%d", (i32)getpid());

  int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
  shm_unlink(name);
  if (fd < 0) {
    return;
  }
  if (ftruncate(fd, (off_t)test_pixels_size) < 0) {
    close(fd);
    return;
  }

  test_pixels =
      mmap(null, test_pixels_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (test_pixels == MAP_FAILED) {
    test_pixels = null;
    close(fd);
    return;
  }
  memset(test_pixels, 0x1c, test_pixels_size);

  struct wl_shm_pool *pool =
      wl_shm_create_pool(state->shm, fd, (i32)test_pixels_size);
  test_buffer = wl_shm_pool_create_buffer(pool, 0, (i32)w, (i32)h, (i32)stride,
                                          WL_SHM_FORMAT_XRGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);

  wl_surface_attach(state->wl_surface, test_buffer, 0, 0);
  wl_surface_damage(state->wl_surface, 0, 0, (i32)w, (i32)h);
  wl_surface_commit(state->wl_surface);
  wl_display_roundtrip(state->display);
}

//
//
//

/**
 * @brief Releases the temporary software buffer.
 */
static void software_surface_cleanup(void) {
  if (test_buffer) {
    wl_buffer_destroy(test_buffer);
    test_buffer = null;
  }
  if (test_pixels) {
    munmap(test_pixels, test_pixels_size);
    test_pixels = null;
  }
}

#endif

//
//
//

/**
 * @brief Handles the clipboard test's window events.
 *
 * @details On a C key press the clipboard is set to a fixed string; on a V key
 * press the clipboard is read back and printed to stderr.
 *
 * @param event The event that fired.
 * @param userdata The window the event belongs to.
 */
static void clipboard_event_callback(bread_event_t *event, void *userdata) {
  bread_window_t *window = userdata;

  if (event->type != BREAD_EVENT_KEY_PRESS) {
    return;
  }

  if (event->data.key.key == BREAD_KEY_C) {
    bread_clipboard_set(window, "bread clipboard test");
    fprintf(stderr, "[clipboard] copied: bread clipboard test\n");
  } else if (event->data.key.key == BREAD_KEY_V) {
    const cstr *text = bread_clipboard_get(window);
    fprintf(stderr, "[clipboard] pasted: %s\n", text ? text : "(empty)");
  }
}

//
//
//

int main(void) {
  arena_t *arena = arena_new(MiB(1), KiB(64));

  bread_window_t window = {
      .width = 400,
      .height = 300,
      .arena = arena,
  };

  bread_window_set_event_callback(&window, clipboard_event_callback, &window);
  bread_window_init(&window);

  bread_window_set_title(&window, "bread clipboard test");
  bread_window_set_min_size(&window, 200, 150);

#if BREAD_WAYLAND
  software_surface_attach(&window);
#endif

  fprintf(stderr,
          "[clipboard] focus the window, press C to copy, V to paste.\n");

  while (bread_window_should_close(&window) == false) {
    bread_window_poll(&window);
  }

#if BREAD_WAYLAND
  software_surface_cleanup();
#endif

  bread_window_destroy(&window);
  arena_free(arena);
}
