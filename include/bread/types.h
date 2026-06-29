#ifndef BREAD_TYPES_H
#define BREAD_TYPES_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

/**
 * @brief The window surface.
 *
 * @param handle The handle of the window surface.
 * @param display The display of the window surface.
 */
typedef struct bread_surface {
  void *handle;
  void *display;
} bread_surface_t;

/**
 * @brief A bread key map.
 */
typedef enum bread_key {
  BREAD_KEY_UNKNOWN = 0,

  BREAD_KEY_A,
  BREAD_KEY_B,
  BREAD_KEY_C,
  BREAD_KEY_D,
  BREAD_KEY_E,
  BREAD_KEY_F,
  BREAD_KEY_G,
  BREAD_KEY_H,
  BREAD_KEY_I,
  BREAD_KEY_J,
  BREAD_KEY_K,
  BREAD_KEY_L,
  BREAD_KEY_M,
  BREAD_KEY_N,
  BREAD_KEY_O,
  BREAD_KEY_P,
  BREAD_KEY_Q,
  BREAD_KEY_R,
  BREAD_KEY_S,
  BREAD_KEY_T,
  BREAD_KEY_U,
  BREAD_KEY_V,
  BREAD_KEY_W,
  BREAD_KEY_X,
  BREAD_KEY_Y,
  BREAD_KEY_Z,

  BREAD_KEY_0,
  BREAD_KEY_1,
  BREAD_KEY_2,
  BREAD_KEY_3,
  BREAD_KEY_4,
  BREAD_KEY_5,
  BREAD_KEY_6,
  BREAD_KEY_7,
  BREAD_KEY_8,
  BREAD_KEY_9,

  BREAD_KEY_F1,
  BREAD_KEY_F2,
  BREAD_KEY_F3,
  BREAD_KEY_F4,
  BREAD_KEY_F5,
  BREAD_KEY_F6,
  BREAD_KEY_F7,
  BREAD_KEY_F8,
  BREAD_KEY_F9,
  BREAD_KEY_F10,
  BREAD_KEY_F11,
  BREAD_KEY_F12,

  BREAD_KEY_UP,
  BREAD_KEY_DOWN,
  BREAD_KEY_LEFT,
  BREAD_KEY_RIGHT,
  BREAD_KEY_HOME,
  BREAD_KEY_END,
  BREAD_KEY_PAGE_UP,
  BREAD_KEY_PAGE_DOWN,
  BREAD_KEY_INSERT,
  BREAD_KEY_DELETE,

  BREAD_KEY_BACKSPACE,
  BREAD_KEY_TAB,
  BREAD_KEY_ENTER,
  BREAD_KEY_ESCAPE,
  BREAD_KEY_SPACE,

  BREAD_KEY_LEFT_SHIFT,
  BREAD_KEY_RIGHT_SHIFT,
  BREAD_KEY_LEFT_CTRL,
  BREAD_KEY_RIGHT_CTRL,
  BREAD_KEY_LEFT_ALT,
  BREAD_KEY_RIGHT_ALT,
  BREAD_KEY_LEFT_SUPER,
  BREAD_KEY_RIGHT_SUPER,
  BREAD_KEY_CAPS_LOCK,
  BREAD_KEY_NUM_LOCK,

  BREAD_KEY_GRAVE,
  BREAD_KEY_MINUS,
  BREAD_KEY_EQUAL,
  BREAD_KEY_LEFT_BRACKET,
  BREAD_KEY_RIGHT_BRACKET,
  BREAD_KEY_BACKSLASH,
  BREAD_KEY_SEMICOLON,
  BREAD_KEY_APOSTROPHE,
  BREAD_KEY_COMMA,
  BREAD_KEY_PERIOD,
  BREAD_KEY_SLASH,

  BREAD_KEY_KP_0,
  BREAD_KEY_KP_1,
  BREAD_KEY_KP_2,
  BREAD_KEY_KP_3,
  BREAD_KEY_KP_4,
  BREAD_KEY_KP_5,
  BREAD_KEY_KP_6,
  BREAD_KEY_KP_7,
  BREAD_KEY_KP_8,
  BREAD_KEY_KP_9,
  BREAD_KEY_KP_ENTER,
  BREAD_KEY_KP_PLUS,
  BREAD_KEY_KP_MINUS,
  BREAD_KEY_KP_MULTIPLY,
  BREAD_KEY_KP_DIVIDE,
  BREAD_KEY_KP_PERIOD,
  BREAD_KEY_KP_NUMLOCK,

  BREAD_KEY_PRINT_SCREEN,
  BREAD_KEY_SCROLL_LOCK,
  BREAD_KEY_PAUSE_BREAK,
  BREAD_KEY_MENU,

  BREAD_KEY_NON_US_BACKSLASH,

  BREAD_KEY_ZENKAKU_HANKAKU,
  BREAD_KEY_HENKAN,
  BREAD_KEY_MUHENKAN,
  BREAD_KEY_KATAKANA_HIRAGANA,
  BREAD_KEY_YEN,

  BREAD_KEY_HANGEUL,
  BREAD_KEY_HANJA,

  BREAD_KEY_KP_COMMA,

  BREAD_KEY_MAX
} bread_key_t;

/**
 * @brief A bread mouse button map.
 *
 * @note BUTTON_4 and BUTTON_5 are usually the side buttons on a mouse.
 */
typedef enum bread_mouse_button {
  BREAD_MOUSE_BUTTON_LEFT,
  BREAD_MOUSE_BUTTON_RIGHT,
  BREAD_MOUSE_BUTTON_MIDDLE,
  BREAD_MOUSE_BUTTON_4,
  BREAD_MOUSE_BUTTON_5,
  BREAD_MOUSE_BUTTON_MAX
} bread_mouse_button_t;

/**
 * @brief The input state of the window.
 *
 * @param keys The state of the keys.
 * @param mouse_buttons The state of the mouse buttons.
 * @param mouse_x The current mouse x position.
 * @param mouse_y The current mouse y position.
 * @param scroll_x The current scroll x position.
 * @param scroll_y The current scroll y position.
 */
typedef struct {
  b32 keys[BREAD_KEY_MAX];
  b32 mouse_buttons[BREAD_MOUSE_BUTTON_MAX];
  f64 mouse_x;
  f64 mouse_y;
  f64 scroll_x;
  f64 scroll_y;
} bread_input_state_t;

/**
 * @brief The cursor type.
 *
 * @param BREAD_CURSOR_DEFAULT The default cursor (pointer).
 * @param BREAD_CURSOR_POINTER The pointer cursor (default).
 * @param BREAD_CURSOR_HAND The hand cursor.
 * @param BREAD_CURSOR_TEXT The text cursor.
 * @param BREAD_CURSOR_MOVE The move cursor.
 * @param BREAD_CURSOR_RESIZE_EW The horizontal resize cursor (east-west).
 * @param BREAD_CURSOR_RESIZE_NS The vertical resize cursor (north-south).
 * @param BREAD_CURSOR_RESIZE_NESW The diagonal resize cursor
 * (northeast-southwest).
 * @param BREAD_CURSOR_RESIZE_NWSE The anti-diagonal resize cu<rsor
 * (northwest-southeast).
 * @param BREAD_CURSOR_NOT_ALLOWED The not allowed cursor.
 * @param BREAD_CURSOR_WAIT The wait cursor.
 * @param BREAD_CURSOR_MAX The maximum cursor type, used for bounds checking.
 */
typedef enum bread_cursor_type {
  BREAD_CURSOR_DEFAULT = 0,
  BREAD_CURSOR_POINTER = 0,
  BREAD_CURSOR_HAND,
  BREAD_CURSOR_TEXT,
  BREAD_CURSOR_MOVE,
  BREAD_CURSOR_RESIZE_EW,
  BREAD_CURSOR_RESIZE_NS,
  BREAD_CURSOR_RESIZE_NESW,
  BREAD_CURSOR_RESIZE_NWSE,
  BREAD_CURSOR_NOT_ALLOWED,
  BREAD_CURSOR_WAIT,
  BREAD_CURSOR_MAX
} bread_cursor_type_t;

/**
 * @brief The event type of the window.
 *
 * @param BREAD_EVENT_NONE No event.
 * @param BREAD_EVENT_WINDOW_CLOSE The window was closed.
 * @param BREAD_EVENT_WINDOW_RESIZE The window was resized.
 * @param BREAD_EVENT_KEY_PRESS A key was pressed.
 * @param BREAD_EVENT_KEY_RELEASE A key was released.
 * @param BREAD_EVENT_MOUSE_MOVE The mouse was moved.
 * @param BREAD_EVENT_MOUSE_PRESS A mouse button was pressed.
 * @param BREAD_EVENT_MOUSE_RELEASE A mouse button was released.
 * @param BREAD_EVENT_MOUSE_SCROLL The mouse was scrolled.
 *
 * @note If any other events are needed, they'll be added here.
 */
typedef enum {
  BREAD_EVENT_NONE = 0,
  BREAD_EVENT_WINDOW_CLOSE,
  BREAD_EVENT_WINDOW_RESIZE,
  BREAD_EVENT_KEY_PRESS,
  BREAD_EVENT_KEY_RELEASE,
  BREAD_EVENT_MOUSE_MOVE,
  BREAD_EVENT_MOUSE_PRESS,
  BREAD_EVENT_MOUSE_RELEASE,
  BREAD_EVENT_MOUSE_SCROLL,
} bread_event_type_t;

/**
 * @brief An event of the window.
 *
 * @param type The type of the event.
 * @param data The data of the event.
 */
typedef struct {
  bread_event_type_t type;
  union {
    struct {
      u16 width;
      u16 height;
    } resize;
    struct {
      bread_key_t key;
      u32 raw_keycode;
    } key;
    struct {
      bread_mouse_button_t button;
    } mouse_button;
    struct {
      f64 x;
      f64 y;
    } mouse_move;
    struct {
      f64 dx;
      f64 dy;
    } mouse_scroll;
  } data;
} bread_event_t;

/**
 * @brief The backend type of the bread build.
 *
 * @param BREAD_BACKEND_WAYLAND The window is using the wayland backend.
 * @param BREAD_BACKEND_X11 The window is using the X11 backend.
 */
typedef enum bread_backend {
  BREAD_BACKEND_WAYLAND,
  BREAD_BACKEND_X11,
} bread_backend_type_t;

/**
 * @brief The callback function signature for the window events.
 *
 * @param event The event that fired.
 * @param userdata The userdata passed to the callback.
 *
 * @note This is passed to the @ref bread_window_set_event_callback() function.
 */
typedef void (*bread_event_callback_t)(bread_event_t *event, void *userdata);

/**
 * @brief The window struct.
 *
 * @param width The width of the window.
 * @param height The height of the window.
 *
 * @param min_width The minimum width of the window.
 * @param min_height The minimum height of the window.
 *
 * @param title The title of the window.
 * @param class The class/app-id of the window.
 *
 * @param backend The current backend of the window.
 * @param arena The arena of the window.
 *
 * @param event_callback The callback function for the window events.
 * @param event_userdata The userdata passed to the callback function.
 */
typedef struct bread_window {
  u16 width;
  u16 height;

  u16 min_width;
  u16 min_height;

  string *title;
  string *class;
  void *backend;
  arena_t *arena;

  bread_event_callback_t event_callback;
  void *event_userdata;
} bread_window_t;

/**
 * @brief The backend vtable of the window, exposing the functionality of each
 * backend, based on build.
 *
 * @param init The init function of the backend.
 * @param poll_events The poll_events function of the backend.
 * @param should_close The should_close function of the backend.
 * @param destroy The destroy function of the backend.
 * @param get_surface The get_surface function of the backend.
 * @param set_title The set_title function of the backend.
 * @param set_min_size The set_min_size function of the backend.
 * @param backend_type The backend type of the bread build.
 */
typedef struct {
  void (*init)(bread_window_t *window);
  void (*poll_events)(bread_window_t *window);
  b32 (*should_close)(bread_window_t *window);
  void (*destroy)(bread_window_t *window);
  bread_surface_t (*get_surface)(bread_window_t *window);
  void (*set_title)(bread_window_t *window, const char *title);
  void (*set_min_size)(bread_window_t *window, u16 width, u16 height);
  bread_backend_type_t backend_type;
} bread_backend_vtable_t;

/**
 * @brief The log level of the bread build.
 *
 * @param BREAD_LOG_LEVEL_DEBUG Debug level.
 * @param BREAD_LOG_LEVEL_INFO Info level.
 * @param BREAD_LOG_LEVEL_WARN Warn level.
 * @param BREAD_LOG_LEVEL_ERROR Error level.
 * @param BREAD_LOG_LEVEL_FATAL Fatal level.
 */
typedef enum bread_log_level {
  BREAD_LOG_LEVEL_DEBUG,
  BREAD_LOG_LEVEL_INFO,
  BREAD_LOG_LEVEL_WARN,
  BREAD_LOG_LEVEL_ERROR,
  BREAD_LOG_LEVEL_FATAL,
} bread_log_level_t;

#endif // !BREAD_TYPES_H
