#ifndef BREAD_TYPES_H
#define BREAD_TYPES_H

/*************************************************/

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

/*************************************************/

/**
 * @brief The window surface.
 */
typedef struct bread_surface {
  void *handle;  /**< The handle of the window surface. */
  void *display; /**< The display of the window surface. */
} bread_surface_t;

//
//
//

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

//
//
//

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

//
//
//

/**
 * @brief A bitmask of active keyboard modifiers.
 */
typedef enum bread_mod {
  BREAD_MOD_SHIFT = 1 << 0,
  BREAD_MOD_CTRL = 1 << 1,
  BREAD_MOD_META = 1 << 2,
  BREAD_MOD_SUPER = 1 << 3,
} bread_mod_t;

//
//
//

/**
 * @brief The input state of the window.
 */
typedef struct {
  b32 keys[BREAD_KEY_MAX];                   /**< The state of the keys. */
  b32 mouse_buttons[BREAD_MOUSE_BUTTON_MAX]; /**< The state of the mouse
                                                buttons. */
  f64 mouse_x;  /**< The current mouse x position. */
  f64 mouse_y;  /**< The current mouse y position. */
  f64 scroll_x; /**< The current scroll x position. */
  f64 scroll_y; /**< The current scroll y position. */
} bread_input_state_t;

//
//
//

/**
 * @brief The cursor type.
 */
typedef enum bread_cursor_type {
  BREAD_CURSOR_DEFAULT = 0, /**< The default cursor (pointer). */
  BREAD_CURSOR_POINTER,     /**< The pointer cursor (default). */
  BREAD_CURSOR_HAND,        /**< The hand cursor. */
  BREAD_CURSOR_TEXT,        /**< The text cursor. */
  BREAD_CURSOR_MOVE,        /**< The move cursor. */
  BREAD_CURSOR_RESIZE_EW,   /**< The horizontal resize cursor (east-west). */
  BREAD_CURSOR_RESIZE_NS,   /**< The vertical resize cursor (north-south). */
  BREAD_CURSOR_RESIZE_NESW, /**< The diagonal resize cursor
                             * (northeast-southwest). */
  BREAD_CURSOR_RESIZE_NWSE, /**< The anti-diagonal resize cursor
                               (northwest-southeast). */
  BREAD_CURSOR_NOT_ALLOWED, /**< The not allowed cursor */
  BREAD_CURSOR_WAIT,        /**< The wait cursor. */
  BREAD_CURSOR_MAX /**< The maximum cursor type, used for bounds checking. */
} bread_cursor_type_t;

//
//
//

/**
 * @brief The event type of the window.
 *
 * @note If any other events are needed, they'll be added here.
 */
typedef enum {
  BREAD_EVENT_NONE,          /**< No event. */
  BREAD_EVENT_WINDOW_CLOSE,  /**< The window was closed. */
  BREAD_EVENT_WINDOW_RESIZE, /**< The window was resized. */
  BREAD_EVENT_KEY_PRESS,     /**< A key was pressed. */
  BREAD_EVENT_KEY_RELEASE,   /**< A key was released. */
  BREAD_EVENT_MOUSE_MOVE,    /**< The mouse was moved. */
  BREAD_EVENT_MOUSE_PRESS,   /**< A mouse button was pressed. */
  BREAD_EVENT_MOUSE_RELEASE, /**< A mouse button was released. */
  BREAD_EVENT_MOUSE_SCROLL,  /**< The mouse was scrolled. */
  BREAD_EVENT_TEXT_PREEDIT,  /**< The IME pre-edit string changed. */
  BREAD_EVENT_TEXT_COMMIT,   /**< The IME committed text. */
  BREAD_EVENT_TEXT_DELETE,   /**< The IME requested a delete. */
} bread_event_type_t;

//
//
//

/**
 * @brief An event of the window.
 */
typedef struct {
  bread_event_type_t type; /**< The type of the event. */
  union {
    struct {
      u16 width;
      u16 height;
    } resize;
    struct {
      bread_key_t key;
      u32 raw_keycode;
      b32 repeat;
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
    struct {
      const cstr *text; /**< Valid only during the callback. */
      u32 cursor;       /**< IME cursor byte offset inside text. */
    } text_preedit;
    struct {
      const cstr *text; /**< Valid only during the callback. */
    } text_commit;
    struct {
      u32 before_length; /**< Bytes to delete before the cursor. */
      u32 after_length;  /**< Bytes to delete after the cursor. */
    } text_delete;
  } data; /**< The data of the event. */
} bread_event_t;

//
//
//

/**
 * @brief The backend type of the bread build.
 *
 * @param BREAD_BACKEND_UNKNOWN The window is using an unknown backend, meaning
 * that either it was built incorrectly or the backend is not supported.
 * @param BREAD_BACKEND_WAYLAND The window is using the wayland backend.
 * @param BREAD_BACKEND_X11 The window is using the X11 backend.
 * @param BREAD_BACKEND_MAX The maximum backend type, used for bounds checking.
 */
typedef enum bread_backend {
  BREAD_BACKEND_UNKNOWN =
      -1, /**< The window is using an unknown backend, meaning that either it
             was build incorrectly or the backend is not supported. */
  BREAD_BACKEND_WAYLAND, /**< The window is using the wayland backend. */
  BREAD_BACKEND_X11,     /**< The window is using the X11 backend. */
  BREAD_BACKEND_MAX /**< The maximum backend type, used for bounds checking. */
} bread_backend_type_t;

//
//
//

/**
 * @brief The callback function signature for the window events.
 *
 * @param event The event that fired.
 * @param userdata The userdata passed to the callback.
 *
 * @note This is passed to the @ref bread_window_set_event_callback() function.
 */
typedef void (*bread_event_callback_t)(bread_event_t *event, void *userdata);

//
//
//

/**
 * @brief The window struct.
 *
 * @details Carries data required to initialize and maintain a window in
 * bread.
 */
typedef struct bread_window {
  u16 width;  /**< The width of the window. */
  u16 height; /**< The height of the window. */

  u16 min_width;  /**< The minimum width of the window. */
  u16 min_height; /**< The minimum height of the window. */

  string *title; /**< The title of the window. */
  string *class; /**< The class/app-id of the window. */

  void *backend;  /**< The current backend of the window. */
  arena_t *arena; /**< The shared arena of the window. */

  bread_event_callback_t
      event_callback;   /**< The callback function for the window's events. */
  void *event_userdata; /**< The userdata passed to the callback function. */
} bread_window_t;

//
//
//

/**
 * @brief The backend vtable of the window, exposing the functionality of each
 * backend, based on build.
 */
typedef struct {
  void (*init)(bread_window_t *window);
  void (*poll_events)(bread_window_t *window);
  void (*wait_events)(bread_window_t *window, i32 timeout_ms);
  b32 (*should_close)(bread_window_t *window);
  void (*destroy)(bread_window_t *window);
  bread_surface_t (*get_surface)(bread_window_t *window);
  void (*set_title)(bread_window_t *window, const char *title);
  void (*set_min_size)(bread_window_t *window, u16 width, u16 height);
  void (*clipboard_set)(bread_window_t *window, const cstr *text);
  const cstr *(*clipboard_get)(bread_window_t *window);
  bread_backend_type_t backend_type;
} bread_backend_vtable_t;

//
//
//

/**
 * @brief The log level of the bread build.
 */
typedef enum bread_log_level {
  BREAD_LOG_LEVEL_DEBUG,
  BREAD_LOG_LEVEL_INFO,
  BREAD_LOG_LEVEL_WARN,
  BREAD_LOG_LEVEL_ERROR,
  BREAD_LOG_LEVEL_FATAL,
} bread_log_level_t;

#endif // !BREAD_TYPES_H
