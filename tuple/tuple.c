#include "tuple.h"
#include "../interpreter/interpreter.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *parse_tuple(const char *string) {
  int i = 0, start = 0;
  bool in_quotes = false;
  char quote_char = '\0';
  int bracket_depth = 0;
  size_t result_capacity = 128;
  size_t result_length = 0;
  char *result = malloc(result_capacity);
  if (!result)
    return NULL;
  result[0] = '\0';

  bool first = true;

  while (1) {
    char ch = string[i];

    bool end = (ch == '\0');
    bool is_comma = (ch == ',' && bracket_depth == 0 && !in_quotes);

    if (ch == '\\') {
      i += 2;
      continue;
    }

    if (!in_quotes && (ch == '"' || ch == '\'')) {
      in_quotes = true;
      quote_char = ch;
    } else if (in_quotes && ch == quote_char) {
      in_quotes = false;
    }

    if (!in_quotes) {
      if (ch == '(' || ch == '[' || ch == '{') {
        bracket_depth++;
      } else if (ch == ')' || ch == ']' || ch == '}') {
        bracket_depth--;
      }
    }

    if (is_comma || end) {
      int len = i - start;
      if (len >= 0) {
        char *part = strndup(string + start, len);
        if (!part)
          break;

        char *processed = evaluate_expression(part);
        free(part);

        size_t processed_len = strlen(processed);
        size_t extra = processed_len + 2;

        if (result_length + extra >= result_capacity) {
          result_capacity *= 2;
          char *new_result = realloc(result, result_capacity);
          if (!new_result) {
            free(processed);
            free(result);
            return NULL;
          }
          result = new_result;
        }

        if (!first) {
          result[result_length++] = ',';
          result[result_length] = '\0';
        } else {
          first = false;
        }

        strcat(result, processed);
        result_length += processed_len;

        free(processed);
      }
      start = i + 1;
    }

    if (end)
      break;
    i++;
  }
  size_t wrapped_len = result_length + 3;
  char *wrapped = malloc(wrapped_len);
  if (!wrapped) {
    free(result);
    return NULL;
  }
  snprintf(wrapped, wrapped_len, "(%s)", result);
  free(result);
  return wrapped;
}

int is_tuple_syntax(char *string) {
  int i;
  bool in_quotes = false;
  char quote_char = '\0';
  int bracket_depth = 0;
  for (i = 0; isspace(string[i]); i++)
    ;

  if (string[i] == ',') {
    return 0;
  }
  for (i = 0; string[i]; i++) {
    char ch = string[i];

    if (ch == '\\') {
      i++;
      continue;
    }

    if (!in_quotes && (ch == '"' || ch == '\'')) {
      in_quotes = true;
      quote_char = ch;
      continue;
    } else if (in_quotes && ch == quote_char) {
      in_quotes = false;
      continue;
    }

    if (in_quotes) {
      continue;
    }

    if (ch == '(' || ch == '[' || ch == '{') {
      bracket_depth++;
    } else if (ch == ')' || ch == ']' || ch == '}') {
      bracket_depth--;
      if (bracket_depth < 0)
        return 0;
    }
    if (ch == ',' && bracket_depth == 0) {
      return 1;
    } else if (ch == ',') {
      return 2;
    }
  }

  return 0;
}
