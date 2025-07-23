#include "interpreter/interpreter.h"
#include "tinyexpr/tinyexpr.h"
#include "utilities/utilities.h"
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  signal(SIGINT, handle_sigint);
  ParserState state = STATE_NORMAL;
  char quote_char = 0;
  size_t comment_start_line = 0, comment_start_col = 0;
  size_t quote_start_line = 0, quote_start_col = 0;
  lines = (char **)malloc(lines_cap * sizeof(char *));
  if (!lines) {
    perror("Malloc failed for lines buffer");
    return EXIT_FAILURE;
  }

  error_logs = malloc(error_logs_capacity * sizeof(ErrorLog));
  if (!error_logs) {
    perror("Malloc failed for error_logs");
    cleanup(EXIT_FAILURE);
  }
  bracket_stack = (BracketStackItem *)malloc(bracket_stack_capacity *
                                             sizeof(BracketStackItem));
  if (!bracket_stack) {
    perror("Malloc failed for bracket_stack");
    cleanup(0);
    return EXIT_FAILURE;
  }

  if (argc > 1) {
    file = fopen(argv[1], "r");
    if (!file) {
      fprintf(stderr,
              "null+: \033[31;1merror: \033[0;1mno such file or directory: "
              "'%s'\033[0m\n",
              argv[1]);
      cleanup(0);
      return EXIT_FAILURE;
    }
    input = argv[1];
  } else {
    file = stdin;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file)) != -1) {
    if (line_count >= lines_cap) {
      size_t new_cap = lines_cap * 2;
      char **tmp = (char **)realloc(lines, new_cap * sizeof(char *));
      if (!tmp) {
        fprintf(stderr, "Failed to reallocate lines buffer.\n");
        free(line);
        cleanup(EXIT_FAILURE);
      }
      lines = tmp;
      lines_cap = new_cap;
    }
    lines[line_count] = strdup(line);
    if (!lines[line_count]) {
      perror("Failed to duplicate line");
      free(line);
      cleanup(EXIT_FAILURE);
    }
    lines[line_count][strcspn(lines[line_count], "\n")] = '\0';
    line_count++;

    if (is_empty(lines[line_count - 1])) {
      continue;
    }

    for (size_t i = 0; lines[line_count - 1][i]; i++) {

      // تخطي رمز \ مثل \"
      if (lines[line_count - 1][i] == '\\') {
        i++;
        continue;
      }
      switch (state) {
      case STATE_NORMAL:
        // التعليقات
        if ((lines[line_count - 1][i] == '/' &&
             lines[line_count - 1][i + 1] == '/') ||
            lines[line_count - 1][i] == '#') {
          line[i] = '\0';
          i = strlen(lines[line_count - 1]);
          // التعليقات مزدوجة
        } else if (lines[line_count - 1][i] == '/' &&
                   lines[line_count - 1][i + 1] == '*') {
          state = STATE_MULTI_COMMENT;
          comment_start_line = line_count;
          comment_start_col = i;
          // تخطي */
          line[i] = ' ';
          line[i + 1] = ' ';
          i++;

          // علامات اقتباس
        } else if (lines[line_count - 1][i] == '"' ||
                   lines[line_count - 1][i] == '\'') {
          state = STATE_QUOTE;
          quote_char = lines[line_count - 1][i];
          quote_start_line = line_count;
          quote_start_col = i;
        } else {
          // تعليق مزدوج مغلوق بدون ان يفتح
          if (state != STATE_MULTI_COMMENT && lines[line_count - 1][i] == '*' &&
              lines[line_count - 1][i + 1] == '/') {
            print_log(0, "unmatched comment", line_count, i + 1, "*/");
            total_errors++;
            goto exit;
          } else {
            // الأقواس
            for (size_t b = 0; b < sizeof(brackets) / sizeof(*brackets); b++) {
              if (lines[line_count - 1][i] == brackets[b].open) {
                push_bracket_stack(brackets[b], line_count, i + 1);
              } else if (lines[line_count - 1][i] == brackets[b].close) {
                if (bracket_stack_size == 0 ||
                    bracket_stack[bracket_stack_size - 1].bracket.close !=
                        brackets[b].close) {
                  print_log(0, "unmatched ", line_count, i + 1,
                            (char[]){brackets[b].close, '\0'});
                  total_errors++;
                  goto exit;
                } else {
                  pop_bracket_stack();
                }
              }
            }
          }
        }
        break;
      case STATE_QUOTE:
        if (lines[line_count - 1][i] == quote_char) {
          state = STATE_NORMAL;
        }
        break;

      case STATE_MULTI_COMMENT:
        line[i] = ' ';
        if (lines[line_count - 1][i] == '*' &&
            lines[line_count - 1][i + 1] == '/') {
          state = STATE_NORMAL;
          line[i + 1] = ' ';

          i++;
        }
        break;
      }
    }
    if (!total_errors) {
      const char *current_line = line;

      size_t code_size = strlen(current_line) + 2;
      if (state == STATE_NORMAL && bracket_stack_size == 0) {
        if (code != NULL) {
          code_size += strlen(code);
          code = realloc(code, code_size);
          if (code == NULL) {
            perror("Memory allocation failed");
            cleanup(0);
            return EXIT_FAILURE;
          }
          snprintf(code + strlen(code), code_size - strlen(code), "%s",
                   current_line);
        }
        if (code == NULL) {
          code = (char *)malloc(code_size);
          if (code == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
          }
          code[0] = '\0';
          snprintf(code + strlen(code), code_size - strlen(code), "%s",
                   current_line);
        }
        // ازالة سطر من نهاية كود
        char *p = strrchr(code, '\n');
        if (p)
          *p = '\0';
        // تفسير
        interpreter(code);
        if (code != NULL) {
          free(code);
          code = NULL;
        }
      } else {
        size_t current_length = code ? strlen(code) : 0;
        size_t new_length =
            current_length + strlen(current_line) + 1; // +1 لـ '\0'

        char *new_code = realloc(code, new_length);
        if (new_code == NULL) {
          perror("Memory allocation failed");
          cleanup(0);
          return EXIT_FAILURE;
        }
        code = new_code;
        code[current_length] = '\0';
        snprintf(code + current_length, new_length - current_length, "%s",
                 current_line);
      }
    }
  }

  if (state == STATE_QUOTE) {
    if (error_count >= error_logs_capacity) {
      error_logs_capacity *= 2;
      ErrorLog *tmp =
          realloc(error_logs, error_logs_capacity * sizeof(ErrorLog));
      if (!tmp) {
        perror("Realloc failed for error_logs");
        cleanup(EXIT_FAILURE);
      }
      error_logs = tmp;
    }
    error_logs[error_count].line = quote_start_line;
    error_logs[error_count].col = quote_start_col + 1;
    error_logs[error_count].msg = "unclosed quote";
    error_logs[error_count].symbol[0] = quote_char;
    error_logs[error_count].symbol[1] = '\0';
    error_count++;
  }
  if (state == STATE_MULTI_COMMENT) {
    if (error_count >= error_logs_capacity) {
      error_logs_capacity *= 2;
      ErrorLog *tmp =
          realloc(error_logs, error_logs_capacity * sizeof(ErrorLog));
      if (!tmp) {
        perror("Realloc failed for error_logs");
        cleanup(EXIT_FAILURE);
      }
      error_logs = tmp;
    }
    error_logs[error_count].line = comment_start_line;
    error_logs[error_count].col = comment_start_col + 1;
    error_logs[error_count].msg = "unclosed comment";
    error_logs[error_count].symbol[0] = '/';
    error_logs[error_count].symbol[1] = '*';
    error_logs[error_count].symbol[2] = '\0';
    error_count++;
  }
  for (size_t i = 0; i < bracket_stack_size; i++) {
    if (error_count >= error_logs_capacity) {
      error_logs_capacity *= 2;
      ErrorLog *tmp =
          realloc(error_logs, error_logs_capacity * sizeof(ErrorLog));
      if (!tmp) {
        perror("Realloc failed for error_logs");
        cleanup(EXIT_FAILURE);
      }
      error_logs = tmp;
    }
    BracketStackItem item = bracket_stack[i];
    error_logs[error_count].line = item.line;
    error_logs[error_count].col = item.col;
    error_logs[error_count].msg = "unclosed ";
    error_logs[error_count].symbol[0] = item.bracket.open;
    error_logs[error_count].symbol[1] = '\0';
    error_count++;
  }

  for (size_t i = 0; i < error_count; i++) {
    for (size_t j = i + 1; j < error_count; j++) {
      if (error_logs[j].line < error_logs[i].line ||
          (error_logs[j].line == error_logs[i].line &&
           error_logs[j].col < error_logs[i].col)) {
        ErrorLog temp = error_logs[i];
        error_logs[i] = error_logs[j];
        error_logs[j] = temp;
      }
    }
  }

  for (size_t i = 0; i < error_count; i++) {
    print_log(0, error_logs[i].msg, error_logs[i].line, error_logs[i].col,
              error_logs[i].symbol);
    total_errors++;
  }
exit:

  cleanup(0);
  free(line);
  if (total_errors) {
    fprintf(stderr, "%d error%c generated.\n", total_errors,
            total_errors > 1 ? 's' : '\0');
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
