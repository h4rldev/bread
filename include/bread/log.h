#ifndef BUTTER_LOG_H
#define BUTTER_LOG_H

#include <htils/arena.h>
#include <htils/basictypes.h>
#include <htils/string.h>

#include <bread/types.h>

/**
 * @brief The log function.
 *
 * @details Prints a message based on the log level onto stderr.
 *
 * @param level The log level of the message.
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre
 * - @c level must be a valid log level.
 * - @c fmt must be a valid pointer to a null-terminated C-String.
 */
void bread_log(bread_log_level_t level, cstr *fmt, ...);

/**
 * @brief Logs a debug message.
 *
 * @details Prints a debug message onto stderr.
 *
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre @c fmt must be a valid pointer to a null-terminated C-String.
 */
#define bread_log_debug(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

/**
 * @brief Logs an info message.
 *
 * @details Prints an info message onto stderr.
 *
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre @c fmt must be a valid pointer to a null-terminated C-String.
 */
#define bread_log_info(fmt, ...)                                               \
  bread_log(BREAD_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

/**
 * @brief Logs a warning message.
 *
 * @details Prints a warning message onto stderr.
 *
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre @c fmt must be a valid pointer to a null-terminated C-String.
 */
#define bread_log_warn(fmt, ...)                                               \
  bread_log(BREAD_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)

/**
 * @brief Logs an error message.
 *
 * @details Prints an error message onto stderr.
 *
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre @c fmt must be a valid pointer to a null-terminated C-String.
 */
#define bread_log_error(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

/**
 * @brief Logs a fatal message.
 *
 * @details Prints a fatal message onto stderr, usually used for when something
 * unrecoverable happens during runtime.
 *
 * @param fmt The format string of the message.
 * @param ... The arguments of the message.
 *
 * @pre @c fmt must be a valid pointer to a null-terminated C-String.
 */
#define bread_log_fatal(fmt, ...)                                              \
  bread_log(BREAD_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)

#endif // !BUTTER_LOG_H
