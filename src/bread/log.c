#include <stdarg.h>
#include <stdio.h>
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

#define BREAD_LOG_HISTORY_SIZE 256

typedef struct {
  u64 hash;
  u64 count;
} bread_log_entry_t;

static thread_local bread_log_entry_t history[BREAD_LOG_HISTORY_SIZE] = {0};
static thread_local u32 history_count = 0;
static thread_local u64 last_hash = 0;
static thread_local b32 line_active = false;

static u64 bread_log_hash(const cstr *level, const cstr *msg) {
  u64 h = 0xcbf29ce484222325ULL;
  for (const cstr *p = level; *p; p++)
    h = (h ^ (u64)(u8)*p) * 0x100000001b3ULL;
  for (const cstr *p = msg; *p; p++)
    h = (h ^ (u64)(u8)*p) * 0x100000001b3ULL;
  return h;
}

static bread_log_entry_t *bread_log_find(u64 hash) {
  for (u32 i = 0; i < history_count; i++) {
    if (history[i].hash == hash)
      return &history[i];
  }
  u32 idx = history_count;
  if (idx >= BREAD_LOG_HISTORY_SIZE)
    idx = 0;
  history[idx].hash = hash;
  history[idx].count = 0;
  if (idx == history_count)
    history_count++;
  return &history[idx];
}

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

  u64 hash = bread_log_hash(level_str, fmt_str);
  bread_log_entry_t *entry = bread_log_find(hash);
  entry->count++;

  if (hash == last_hash && line_active) {
    if (entry->count >= 2) {
      fprintf(stderr, "\r[BREAD] %s: %s [x%lu]", level_str, fmt_str,
              entry->count);
      fflush(stderr);
    }
    return;
  }

  if (line_active)
    fprintf(stderr, "\n");

  if (entry->count == 1)
    fprintf(stderr, "\r[BREAD] %s: %s", level_str, fmt_str);
  else
    fprintf(stderr, "\r[BREAD] %s: %s [x%lu]", level_str, fmt_str,
            entry->count);
  fflush(stderr);

  last_hash = hash;
  line_active = true;
}
