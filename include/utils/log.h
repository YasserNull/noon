// utils/log.h
// Header file for the logging system. It defines the types of logs,
// the structures for log entries and positions, and declares the functions
// for saving, sorting, and printing logs. It also includes the debug macro.

#ifndef LOG_H
#define LOG_H

#include "input.h"
#include <stddef.h>
#include <stdio.h>

/* Log message types */
typedef enum { LOG_ERROR, LOG_WARNING, LOG_INFO } LogType;

// Struct to hold the position (line, column) of a log.
typedef struct {
  size_t log_line;
  size_t log_index;
} LogPosition;

/* Struct for a single log message entry. */
typedef struct {
  LogPosition log_position;
  char *log_msg;
  char *log_symbol;
  LogType log_type;
} LogEntry;

/* Sort logs by line and column. */
void sort_logs(void);

/* Print all saved logs. */
void print_logs(void);

/* Print the final summary. */
void print_summary(void);

/* Print a single log message immediately. */
void print_log(int log_type, const char *fmt, LogPosition log_position, const char *symbol_str, ...);
/* Save a log message to be printed later. */
void save_log(int log_type, const char *fmt, LogPosition log_position, const char *symbol_str, ...);
/* Resets the logging system to a clean initial state. */
void reset_logs(void);

/* Debug macro: prints function name and arguments if debug mode is enabled. */
#define debug_func(...)                                                                                                                                                                                \
  do {                                                                                                                                                                                                 \
    if (ni && ni->debug) {                                                                                                                                                                             \
      fprintf(stderr, "[%s] ", __func__);                                                                                                                                                              \
      fprintf(stderr, " " __VA_ARGS__);                                                                                                                                                                \
      fprintf(stderr, "\n");                                                                                                                                                                           \
    }                                                                                                                                                                                                  \
  } while (0)

#endif
