#include <bread/types.h>
#include <bread/window.h>

int main(void) {
  arena_t *arena = arena_new(MiB(1), KiB(64));

  bread_window_t window = {
      .width = 100,
      .height = 100,
      .title = string_from_cstr(arena, "Hello, World!"),
  };

  arena_free(arena);
}
