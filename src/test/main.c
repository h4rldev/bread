#include <stdio.h>
#include <unistd.h>

#include <bread/event.h>
#include <bread/types.h>
#include <bread/window.h>

void bread_event_callback(bread_event_t *event, void *userdata) {
  fprintf(stderr, "Got event of type %d\n", event->type);
}

int main(void) {
  arena_t *arena = arena_new(MiB(1), KiB(64));

  bread_window_t window = {
      .width = 100,
      .height = 100,
      .arena = arena,
  };

  bread_window_set_event_callback(&window, bread_event_callback, NULL);
  bread_window_init(&window);

  bread_window_set_title(&window, "Test Window");
  bread_window_set_min_size(&window, 100, 100);

  while (bread_window_should_close(&window) == false) {
    bread_window_poll(&window);
  }

  bread_window_destroy(&window);
  arena_free(arena);
}
