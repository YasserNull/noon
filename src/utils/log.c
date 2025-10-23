// utils/log.c
// This file manages the logging system for errors, warnings, and info messages.
// It includes functions for formatting, printing, saving, and sorting logs
// to provide clear feedback to the user.

#include "utils/log.h"
#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "utils/strings.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prints a final summary of the total number of warnings and errors generated.
void print_summary(void) {
  debug_func("");
  int first = 1;

  // Print warning count if any.
  if (ctx->total_warnings > 0) {
    printf("%d warning%s", ctx->total_warnings, ctx->total_warnings > 1 ? "s" : "");
    first = 0;
  }

  // Print error count if any.
  if (ctx->total_errors > 0) {
    if (!first)
      printf(" and ");
    printf("%d error%s", ctx->total_errors, ctx->total_errors > 1 ? "s" : "");
    first = 0;
  }

  // Print info count if any.
  if (ctx->total_infos > 0) {
    if (!first)
      printf(" and ");
    printf("%d info%s", ctx->total_infos, ctx->total_infos > 1 ? "s" : "");
  }

  if (ctx->total_errors + ctx->total_warnings + ctx->total_infos > 0)
    printf(" generated.\n");
}

// Formats and prints a single log message to stderr, including source code
// context.
void print_log(int log_type, const char *fmt, LogPosition log_position, const char *symbol_str, ...) {
  debug_func("log_type:%d, fmt:%s,{line:%zu, index:%zu}, symbol:%s", log_type, fmt, log_position.log_line, log_position.log_index, symbol_str);
  const char *log_type_string;
  const char *color;
  // Determine log type string and color.
  switch (log_type) {
  case LOG_ERROR:
    log_type_string = "error";
    color = COLOR_RED;
    ctx->total_errors++;
    break;
  case LOG_WARNING:
    log_type_string = "warning";
    color = COLOR_PURPLE;
    ctx->total_warnings++;
    break;
  case LOG_INFO:
    log_type_string = "info";
    color = COLOR_CYAN;
    ctx->total_infos++;
    break;
  default: return;
  }

  // Format the main message string.
  va_list args;
  va_start(args, symbol_str);
  int msg = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char *msg_buf = malloc(msg + 1);
  if (!msg_buf)
    return;
  va_start(args, symbol_str);
  vsnprintf(msg_buf, msg + 1, fmt, args);
  va_end(args);

  // Create the caret string (e.g., "^~~~") to underline the symbol.
  size_t sym = symbol_str ? strlen(symbol_str) : 0;
  size_t caret = (sym <= 1) ? 1 : sym;
  char *caret_buf = malloc(caret + 1);
  if (!caret_buf) {
    free(msg_buf);
    return;
  }
  caret_buf[0] = '^';
  for (size_t i = 1; i < caret; ++i)
    caret_buf[i] = '~';
  caret_buf[caret] = '\0';

  // Get the line of code where the log occurred.
  const char *line_text = "";
  if (ctx->lines != NULL && log_position.log_line > 0 && log_position.log_line <= ctx->line_number && ctx->lines[log_position.log_line - 1])
    line_text = ctx->lines[log_position.log_line - 1];
  else
    log_position.log_index = 0;

  int caret_index = (int)(log_position.log_index > 0 ? log_position.log_index - 1 : 0);
  int num_digits = number_count((int)log_position.log_line);

  log_position.log_index = (log_position.log_index == 0) ? 1 : log_position.log_index;

  // Print the fully formatted log message.
  fprintf(stderr,
          "%s%s:%zu:%zu: %s%s:%s%s %s%s\n"
          "%*zu | %s\n"
          "%*s | %*s%s%s%s%s\n",
          COLOR_BOLD,
          ni->input,
          log_position.log_line,
          log_position.log_index,
          color,
          log_type_string,
          COLOR_RESET,
          COLOR_BOLD,
          msg_buf,
          COLOR_RESET,
          num_digits,
          log_position.log_line,
          line_text,
          num_digits,
          "",
          caret_index,
          "",
          COLOR_BOLD,
          COLOR_GREEN,
          caret_buf,
          COLOR_RESET);

  free(msg_buf);
  free(caret_buf);
}

// Iterates through all saved logs and prints them.
void print_logs(void) {
  debug_func("");
  for (size_t i = 0; i < ctx->logs_count; i++) {
    print_log(ctx->logs[i].log_type, ctx->logs[i].log_msg, ctx->logs[i].log_position, ctx->logs[i].log_symbol);
  }
}

// Saves a log entry to a dynamic array to be printed later.
void save_log(int log_type, const char *fmt, LogPosition log_position, const char *symbol_str, ...) {
  debug_func("log_type:%d, fmt:%s,{line:%zu, index:%zu}, symbol:%s", log_type, fmt, log_position.log_line, log_position.log_index, symbol_str);
  // Format the message string.
  va_list args;
  va_start(args, symbol_str);
  int msg = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  char *msg_buf = malloc(msg + 1);
  if (!msg_buf)
    return;

  va_start(args, symbol_str);
  vsnprintf(msg_buf, msg + 1, fmt, args);
  va_end(args);

  /* Expand the array if capacity is reached */
  if (ctx->logs_count >= ctx->logs_capacity) {
    size_t new_cap = ctx->logs_capacity ? ctx->logs_capacity * 2 : 16;
    LogEntry *new_logs = realloc(ctx->logs, new_cap * sizeof(LogEntry));
    if (!new_logs) {
      // Safely exit on realloc failure.
      fprintf(stderr, "fatal: out of memory saving logs\n");
      free(msg_buf);
      exit(EXIT_FAILURE);
      return;
    }
    ctx->logs = new_logs;
    ctx->logs_capacity = new_cap;
  }

  // Store the new log entry.
  ctx->logs[ctx->logs_count].log_position = log_position;
  ctx->logs[ctx->logs_count].log_msg = msg_buf;
  ctx->logs[ctx->logs_count].log_symbol = symbol_str ? safe_strdup(symbol_str) : NULL;
  ctx->logs[ctx->logs_count].log_type = log_type;
  ctx->logs_count++;
}

// Comparison function for qsort to sort logs by line and then by index.
int compare_logs(const void *a, const void *b) {
  debug_func("a:%p, b:%p", a, b);
  const LogEntry *logA = (const LogEntry *)a;
  const LogEntry *logB = (const LogEntry *)b;

  // Compare by line number first.
  if (logA->log_position.log_line < logB->log_position.log_line)
    return -1;
  if (logA->log_position.log_line > logB->log_position.log_line)
    return 1;

  // If lines are the same, compare by index (column).
  if (logA->log_position.log_index < logB->log_position.log_index)
    return -1;
  if (logA->log_position.log_index > logB->log_position.log_index)
    return 1;

  return 0;
}

// Sorts the global array of logs.
void sort_logs(void) {
  debug_func("");
  qsort(ctx->logs, ctx->logs_count, sizeof(LogEntry), compare_logs);
}

// Resets the logging system to a clean initial state.
void reset_logs(void) {
  debug_func("");
  // Free existing log data.
  for (size_t i = 0; i < ctx->logs_count; i++) {
    free(ctx->logs[i].log_msg);
    free(ctx->logs[i].log_symbol);
  }

  // Free the old buffer completely.
  free(ctx->logs);

  // Allocate a new clean log buffer with initial capacity.
  ctx->logs = safe_calloc(INITIAL_CAPACITY, sizeof(LogEntry));
  ctx->logs_capacity = INITIAL_CAPACITY;

  // Reset counters.
  ctx->logs_count = 0;
  ctx->total_errors = 0;
  ctx->total_warnings = 0;
  ctx->total_infos = 0;
}
