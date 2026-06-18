#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>

#include <htils/arena.h>
#include <htils/path.h>
#include <htils/string.h>

#include <bread/log.h>

#define COLOR_RESET "\x1b[0m"
#define COLOR_DARK_RED "\x1b[31m"
#define COLOR_RED "\x1b[91m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_CYAN "\x1b[36m"

static thread_local cstr last_level[20] = {0};
static thread_local cstr last_msg[4096] = {0};
static thread_local u64 repeat_count = 0;
static thread_local b32 has_printed = false;

void bread_log(bread_log_level_t level, cstr *fmt, ...) {
#ifndef BREAD_DEBUG
  if (level == BREAD_LOG_LEVEL_DEBUG)
    return;
#endif

  static cstr level_str[20] = {0};
  static cstr fmt_str[4096] = {0};

  switch (level) {
  case BREAD_LOG_LEVEL_DEBUG:
    snprintf(level_str, 20, "%s[DEBUG]%s", COLOR_CYAN, COLOR_RESET);
    break;
  case BREAD_LOG_LEVEL_INFO:
    snprintf(level_str, 20, "%s[INFO]%s", COLOR_CYAN, COLOR_RESET);
    break;
  case BREAD_LOG_LEVEL_WARN:
    snprintf(level_str, 20, "%s[WARN]%s", COLOR_YELLOW, COLOR_RESET);
    break;
  case BREAD_LOG_LEVEL_ERROR:
    snprintf(level_str, 20, "%s[ERROR]%s", COLOR_RED, COLOR_RESET);
    break;
  case BREAD_LOG_LEVEL_FATAL:
    snprintf(level_str, 20, "%s[FATAL]%s", COLOR_DARK_RED, COLOR_RESET);
    break;
  }

  va_list args;
  va_start(args, fmt);
  vsnprintf(fmt_str, 4096, fmt, args);
  va_end(args);

  if (memcmp(last_level, level_str, 20) == 0 &&
      memcmp(last_msg, fmt_str, 4096) == 0) {
    repeat_count++;
    fprintf(stderr, "\r[BREAD] %s: %s [x%lu]", level_str, fmt_str,
            repeat_count + 1);
    fflush(stderr);
    has_printed = true;
    return;
  }

  if (has_printed) {
    fprintf(stderr, "\n");
    fflush(stderr);
    has_printed = false;
  }

  fprintf(stderr, "[BREAD] %s: %s\n", level_str, fmt_str);
  memcpy(last_level, level_str, 20);
  memcpy(last_msg, fmt_str, 4096);
}
