/**
 * Copyright (c) 2022 Halex
 * MIT License
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

// -------------------- OPERATION ----------------------
#define APPEND    "a"
#define WRITE     "w"

// -------------------- COLOR ----------------------
#define BLK       "\x1B[30m"
#define RED       "\x1B[31m"
#define GRN       "\x1B[32m"
#define YEL       "\x1B[33m"
#define BLU       "\x1B[34m"
#define MAG       "\x1B[35m"
#define CYN       "\x1B[36m"
#define WHT       "\x1B[37m"

// -------------------- COLOR BACKGROUND ----------------------
#define RED_BACK  "\x1b[41m"
#define GRN_BACK  "\x1b[42m"
#define YEL_BACK  "\x1b[43m"
#define BLU_BACK  "\x1b[44m"
#define MAG_BACK  "\x1b[45m"
#define CYN_BACK  "\x1b[46m"

#define LOG_VERSION "1.0.0"
#define SIZEOF_ARRAY( arr ) sizeof( arr ) / sizeof( arr[0] )

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file; //who called the log 
  void *udata;
  int line;
} log_event;

typedef struct {
  bool thread_safe;
  bool file;
} log_config;

enum { LOG_STATS, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_stats(...) mylog(LOG_STATS, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) mylog(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  mylog(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  mylog(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) mylog(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) mylog(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

/**
 * Given a path to a config file tries 
 * to read the server configuration from the file.
 * 
 * EXIT_FAILURE on error
 */
void init_log_file(const char* path, const char* mode);

/**
 * Function to close logger
 */
void close_logger();

/**
 * Enable or disable thread-safe
 */
void log_threadsafe(bool value);

/**
 * @brief Set a determinate color of one level
 * 
 * @param level level of log
 * @param color color what do u want
 */
void log_setColor(int level, const char* color);

/**
 * @brief Set if the level can write on stdout or not
 * 
 * @param level level of log
 * @param cond true if write, false otherwise
 */
void log_setConsole(int level, bool cond);

/**
 * @brief Set if the level write the line code where it was called
 * 
 * @param level level of log
 * @param cond true if write, false otherwise
 */
void log_setWriteLine(int level, bool cond);

/**
 * Function main to write into stderr and file (under certain conditions)
 */
void mylog(int level, const char *file, int line, const char *str, ...);
#endif