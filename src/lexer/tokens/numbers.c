// lexer/tokens/numbers.c
// This file contains the logic for tokenizing various numeric literals,
// including integers and floating-point numbers. It handles decimal points,
// exponents, and numeric separators ('_').

#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

// Attempts to tokenize a number from the current input position.
bool tokenize_number(void) {
  debug_func("");
  char ch = safe_char(ctx->current_line, ctx->line_index);
  size_t start = ctx->line_index;
  // A number must start with a digit, or a '.' followed by a digit.
  if (ch == '.')
    return false;
  if (!isdigit((unsigned char)ch) && !(ch == '.' && isdigit((unsigned char)safe_char(ctx->current_line, start + 1))))
    return false;

  TokenType type = TOKEN_INT;
  bool has_decimal_point = false;
  bool has_exponent = false;
  bool has_digit = false;
  bool prev_underscore = false;
  size_t end = 0;
  size_t len = 0;

  // Variable to track the specific error type.
  const char *specific_error_message = ERR_INVALID_DECIMAL_LITERAL;

  // Loop to consume all parts of the number literal.
  while (1) {
    char c = safe_char(ctx->current_line, ctx->line_index);

    if (isdigit((unsigned char)c)) {
      has_digit = true;
      prev_underscore = false;
      ctx->line_index++;
    } else if (c == '_') {
      // Check for consecutive underscores (e.g., 1__2).
      if (prev_underscore) {
        specific_error_message = ERR_CONSECUTIVE_NUMERIC_SEPARATOR;
        goto error;
      }
      prev_underscore = true;
      ctx->line_index++;
    } else if (c == '.' && !has_decimal_point && !has_exponent) {
      // A dot cannot be followed by an underscore.
      if (safe_char(ctx->current_line, ctx->line_index + 1) == '_')
        goto error;
      has_decimal_point = true;
      prev_underscore = false;
      ctx->line_index++;
    } else if ((c == 'e' || c == 'E') && !has_exponent) {
      has_exponent = true;
      prev_underscore = false;
      ctx->line_index++;
      char next = safe_char(ctx->current_line, ctx->line_index);
      if (next == '+' || next == '-') {
        ctx->line_index++;
      }
      // An exponent part cannot be followed by an underscore.
      if (safe_char(ctx->current_line, ctx->line_index) == '_')
        goto error;
    } else
      break;
  }

  // Check for a trailing underscore (e.g., 123_).
  if (prev_underscore) {
    specific_error_message = ERR_TRAILING_NUMERIC_SEPARATOR;
    goto error;
  }

  end = ctx->line_index;
  if (end == start)
    goto error;

  // Check for incomplete exponent part (e.g., 1e, 1e+).
  char last_char = safe_char(ctx->current_line, end - 1);
  if (last_char == 'e' || last_char == 'E' || last_char == '+' || last_char == '-')
    goto error;

  // If it has a decimal or exponent, it's a float.
  if (has_decimal_point || has_exponent)
    type = TOKEN_FLOAT;

  // Create the token with the number string.
  len = end - start;
  char *num = safe_malloc(len + 1);
  memcpy(num, &ctx->current_line[start], len);
  num[len] = '\0';

  ctx->line_index--;
  append_token(type, num, ctx->line_number, start);
  free(num);
  return true;

// Error handling for invalid number formats.
error:
  end = ctx->line_index;
  // Consume the rest of the invalid number literal for error reporting.
  while (isalnum(safe_char(ctx->current_line, end)) || safe_char(ctx->current_line, end) == '.' || safe_char(ctx->current_line, end) == '_' || safe_char(ctx->current_line, end) == 'e' ||
         safe_char(ctx->current_line, end) == 'E')
    end++;
  len = end - start;
  if (len == 0)
    len = 1;
  char *val = safe_malloc(len + 1);
  memcpy(val, &ctx->current_line[start], len);
  val[len] = '\0';

  // Use the specific error message determined earlier.
  print_log(LOG_ERROR, specific_error_message, (LogPosition){ctx->line_number, start}, val, val);
  free(val);
  ctx->has_syntax_error = 1;
  ctx->line_index = end - 1;
  return true;
}
