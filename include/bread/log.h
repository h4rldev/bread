#ifndef BUTTER_LOG_H
#define BUTTER_LOG_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

#include <bread/types.h>

void bread_log(bread_log_level_t level, cstr *fmt, ...);

#define bread_log_debug(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define bread_log_info(fmt, ...)                                               \
  bread_log(BREAD_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define bread_log_warn(fmt, ...)                                               \
  bread_log(BREAD_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define bread_log_error(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define bread_log_fatal(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)

#endif // !BUTTER_LOG_H
