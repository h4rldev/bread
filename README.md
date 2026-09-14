# bread

A platform/windowing layer in C, with backends for Wayland and X11 (XCB). Part
of the `htils` `bread` `butter` `cheese` stack; `butter` (Vulkan renderer) binds
its surface to a `bread` window, and `cheese` (immediate-mode UI) sits on top.

## Status

Usable for windowed 2D apps, still under development. X11 and Wayland both work;
the API may change.

## Backends

Chosen by which library is built (`BREAD_WAYLAND` / `BREAD_X11`):

- Wayland (via `xdg-shell`), with `xdg-decoration` support
- X11 (XCB)

`bread_get_backend_type()` reports the active backend, and `bread_current_backend`
exposes its vtable.

## Features

- **Windows**: create and tear down a window (`bread_window_init` /
  `bread_window_destroy`), set a title and minimum size
  (`bread_window_set_title`, `bread_window_set_min_size`), and hand the native
  surface to a renderer (`bread_window_get_surface`).
- **Event loop**: `bread_window_poll` + `bread_window_should_close` drive the
  loop; a callback (`bread_window_set_event_callback`) receives `bread_event_t`
  close, mouse-move, key-press and resize events.
- **Input**: key events to unicode/string (`bread_event_key_to_unicode`,
  `bread_event_key_to_cstr`), and the current keyboard/mouse state
  (`bread_window_get_input`).
- **Cursors**: a set of standard cursors (`bread_cursor_type_t`,
  `bread_set_cursor`) loaded from the user's cursor theme, set up and torn down
  with the window.
- **Backend abstraction**: a `bread_backend_vtable_t` so both platforms share one
  window/event/input API.

## Building

bread builds with [conjure](https://codeberg.org/h4rl/conjure):

```
conjure build -p wayland-release          # libbread-wayland.so
conjure build -p wayland-release-static   # libbread-wayland.a
conjure build -p wayland-debug            # libbread-wayland-debug.a
# ...and the x11-* equivalents
```

Profiles are `<wayland|x11>-<debug|release|release-static>`. Debug builds are
static archives; release builds are shared unless built with the `-static`
profile.

The Wayland build generates its `xdg-shell` and `xdg-decoration` protocol sources
with `wayland-scanner` before compiling.

A Nix flake provides the six library packages plus a dev shell:

```
nix build .#bread-wayland-release
nix develop
```

## Tests

`src/test` holds a small windowed example. Build it with
`conjure test -p wayland-debug` (or `x11-debug`).

## License

This project is licensed under the BSD 3-Clause License - See the
[LICENSE](LICENSE) file for details.
