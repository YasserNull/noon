#include "utilities.h"
#include "../world_variables/world_variables.h"
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cleanup(int exit_code) {
  if (lines) {
    for (size_t i = 0; i < line_count; i++) {
      if (lines[i]) {
        free(lines[i]);
        lines[i] = NULL;
      }
    }
    free(lines);
    lines = NULL;
  }
  if (bracket_stack) {
    free(bracket_stack);
    bracket_stack = NULL;
  }
  if (file && file != stdin) {
    fclose(file);
    file = NULL;
  }
  if (code != NULL) {
    free(code);
  }
  if (error_logs) {
    free(error_logs);
    error_logs = NULL;
  }
  if (exit_code != 0) {
    exit(exit_code);
  }
}

void handle_sigint() { cleanup(EXIT_FAILURE); }

int is_empty(const char *str) {
  if (str == NULL || *str == '\0')
    return 1;
  while (*str) {
    if (!isspace((unsigned char)*str)) {
      return 0;
    }
    str++;
  }
  return 1;
}

int number_count(int number) {
  if (number == 0)
    return 1;
  return (int)(log10(number) + 1);
}

void print_log(int type, const char *msg, size_t line, size_t col,
               const char *symbol_str) {
  if (line == 0 || line > line_count || col == 0 || !lines)
    return;

  int num_digits = number_count((int)line);

  const char *log_type;
  const char *color;
  switch (type) {
  case 0:
    log_type = "error";
    color = "\033[31m";
    break; // أحمر
  case 1:
    log_type = "warning";
    color = "\033[36m";
    break; // أزرق فاتح
  case 2:
    log_type = "info";
    color = "\033[35m";
    break; // بنفسجي
  default:
    return;
  }

  size_t sym_len = strlen(symbol_str);

  // إذا كان صفر أو 1، خلي الطول الفعلي 1 فقط
  size_t caret_len = (sym_len <= 1) ? 1 : sym_len;

  static char caret_buf[512];
  if (caret_len >= sizeof(caret_buf))
    caret_len = sizeof(caret_buf) - 1;

  // أول رمز دائماً ^
  caret_buf[0] = '^';

  // إذا كان فيه أكثر من رمز، عبي الباقي بـ ~
  for (size_t i = 1; i < caret_len; ++i)
    caret_buf[i] = '~';

  caret_buf[caret_len] = '\0';

  printf("\033[1m%s:%zu:%zu: %s%s:\033[0;1m %s %s\033[0m\n"
         "%*zu | %s\n"
         "%*s | %*s\033[32;1m%s\033[0m\n",
         input, line, col, color, log_type, msg, symbol_str, num_digits, line,
         lines[line - 1], num_digits, "", (int)(col > 0 ? col - 1 : 0), "",
         caret_buf);
}

void push_bracket_stack(Bracket bracket, size_t line, size_t col) {
  if (bracket_stack_size >= bracket_stack_capacity) {
    bracket_stack_capacity *= 2;
    bracket_stack = (BracketStackItem *)realloc(
        bracket_stack, bracket_stack_capacity * sizeof(BracketStackItem));
    if (!bracket_stack) {
      fprintf(stderr, "Failed to reallocate bracket_stack.\n");
      cleanup(EXIT_FAILURE);
    }
  }
  bracket_stack[bracket_stack_size].bracket = bracket;
  bracket_stack[bracket_stack_size].line = line;
  bracket_stack[bracket_stack_size].col = col;
  bracket_stack_size++;
}

BracketStackItem pop_bracket_stack() {
  if (bracket_stack_size == 0) {
    fprintf(stderr, "Stack underflow.\n");
    cleanup(EXIT_FAILURE);
  }
  return bracket_stack[--bracket_stack_size];
}