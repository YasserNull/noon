// lexer/handlers/quotes.c
// This file contains the logic for tokenizing string (`"..."`) and
// character (`'...'`) literals. It handles escape sequences and ensures
// literals are properly terminated.

#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Processes characters while the lexer is in the STATE_QUOTE state.
bool tokenize_strings(char c) {
  debug_func("%c", c);
  // Handle escape sequences (e.g., \n, \t, \").
  if (c == '\\' && safe_char(ctx->current_line, ctx->line_index + 1) != '\0') {
    // Ensure buffer has space.
    if (ctx->string_token_length + 2 >= ctx->string_token_capacity) {
      size_t new_cap = ctx->string_token_capacity ? ctx->string_token_capacity * 2 : INITIAL_CAPACITY;
      ctx->string_token = safe_realloc(ctx->string_token, new_cap);
      ctx->string_token_capacity = new_cap;
    }
    // Add both the backslash and the escaped character to the buffer.
    ctx->string_token[ctx->string_token_length++] = c;
    ctx->string_token[ctx->string_token_length++] = safe_char(ctx->current_line, ctx->line_index + 1);
    ctx->string_token[ctx->string_token_length] = '\0';
    ctx->line_index++; // Skip the escaped character.
    return true;
  }

  if (c == '\0')
    return true;

  // Check for the closing quote.
  if (ctx->quote_char != '\0' && c == ctx->quote_char) {
    // Ensure buffer has space.
    if (ctx->string_token_length + 1 >= ctx->string_token_capacity) {
      size_t new_cap = ctx->string_token_capacity ? ctx->string_token_capacity * 2 : INITIAL_CAPACITY;
      ctx->string_token = safe_realloc(ctx->string_token, new_cap);
      ctx->string_token_capacity = new_cap;
    }
    ctx->string_token[ctx->string_token_length++] = c;
    ctx->string_token[ctx->string_token_length] = '\0';

    // Create the appropriate token (CHAR or STRING).
    if (ctx->quote_char == '\'') {
      size_t content = ctx->string_token_length - 2;
      if (content > 1) {
        // Warn if a character literal contains more than one character.
        print_log(LOG_WARNING, WRN_MULTICHAR_COMMENT, (LogPosition){ctx->quote_line, ctx->quote_index}, ctx->string_token);
      }
      append_token(TOKEN_CHAR, ctx->string_token, ctx->line_number, ctx->line_index);
    } else {
      append_token(TOKEN_STRING, ctx->string_token, ctx->line_number, ctx->line_index);
    }

    // Reset the string tokenizing state.
    ctx->string_token_length = 0;
    if (ctx->string_token)
      ctx->string_token[0] = '\0';
    ctx->quote_char = '\0';
    ctx->quote_line = 0;
    ctx->quote_index = 0;
    ctx->state = STATE_NORMAL;

  } else {
    // Append the character to the current string token buffer.
    if (ctx->string_token_length + 1 >= ctx->string_token_capacity) {
      size_t new_cap = ctx->string_token_capacity ? ctx->string_token_capacity * 2 : INITIAL_CAPACITY;
      ctx->string_token = safe_realloc(ctx->string_token, new_cap);
      ctx->string_token_capacity = new_cap;
    }
    ctx->string_token[ctx->string_token_length++] = c;
    ctx->string_token[ctx->string_token_length] = '\0';
  }
  return true;
}

// Detects the start of a string or character literal.
void handle_quotes(char c) {
  debug_func("%c", c);
  if (c == '"' || c == '\'') {
    // Enter the quote state.
    ctx->state = STATE_QUOTE;
    // Initialize the string buffer if it's the first time.
    if (!ctx->string_token) {
      ctx->string_token_capacity = INITIAL_CAPACITY;
      ctx->string_token = safe_malloc(ctx->string_token_capacity);
      ctx->string_token_length = 0;
      ctx->string_token[0] = '\0';
    }
    // Record the quote type and its position for error reporting.
    ctx->quote_char = c;
    ctx->quote_line = ctx->line_number;
    ctx->quote_index = ctx->line_index + 1;

    // Add the opening quote to the buffer.
    if (ctx->string_token_length + 1 >= ctx->string_token_capacity) {
      size_t new_cap = ctx->string_token_capacity ? ctx->string_token_capacity * 2 : INITIAL_CAPACITY;
      ctx->string_token = safe_realloc(ctx->string_token, new_cap);
      ctx->string_token_capacity = new_cap;
    }
    ctx->string_token[ctx->string_token_length++] = c;
    ctx->string_token[ctx->string_token_length] = '\0';
  }
}

// Checks for an unclosed string or character literal at the end of the input.
void check_unclosed_quote(void) {
  debug_func("");
  if (ctx->state == STATE_QUOTE) {
    const char *type = (ctx->quote_char == '\'') ? "char" : "string";
    const char tmp[2] = {ctx->quote_char ? ctx->quote_char : '"', '\0'};
    save_log(LOG_ERROR, ERR_UNCLOSED, (LogPosition){ctx->quote_line, ctx->quote_index}, tmp, type, tmp);
  }
}
