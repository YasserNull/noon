// lexer/handlers/comments.c
// This file contains the logic for handling and ignoring single-line (`//` or
// `#`) and multi-line (`/* ... */`) comments during tokenization.

#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "utils/log.h"
#include "utils/memory.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Processes characters while the lexer is in the STATE_MULTI_COMMENT state.
void handle_multi_comment(char c) {
  debug_func("%c", c);
  // Check for the end of the multi-line comment.
  if (c == '*' && safe_char(ctx->current_line, ctx->line_index + 1) == '/') {
    ctx->state = STATE_NORMAL;
    ctx->multi_comment_line = 0;
    ctx->multi_comment_index = 0;
    // Replace the '*/' with spaces to ignore them.
    if (ctx->line_index < ctx->line_length)
      ctx->current_line[ctx->line_index] = ' ';
    if (ctx->line_index + 1 < ctx->line_length)
      ctx->current_line[ctx->line_index + 1] = ' ';
    ctx->line_index++;
  } else {
    // Replace every character inside the comment with a space.
    if (ctx->current_line[ctx->line_index] != '\0')
      ctx->current_line[ctx->line_index] = ' ';
  }
}

// Detects the start of single-line or multi-line comments.
bool handle_comments(char c) {
  debug_func("%c", c);
  // Handle single-line comments.
  if (c == '#' || (c == '/' && safe_char(ctx->current_line, ctx->line_index + 1) == '/')) {
    // Terminate the line at the comment start to ignore the rest.
    ctx->current_line[ctx->line_index] = '\0';
    return true; // Indicates the rest of the line should be skipped.
  } else if (c == '/' && safe_char(ctx->current_line, ctx->line_index + 1) == '*') {
    // Handle the start of a multi-line comment.
    ctx->state = STATE_MULTI_COMMENT;
    ctx->multi_comment_line = ctx->line_number;
    ctx->multi_comment_index = ctx->line_index + 1;
    // Replace '/*' with spaces.
    if (ctx->line_index < ctx->line_length)
      ctx->current_line[ctx->line_index] = ' ';
    if (ctx->line_index + 1 < ctx->line_length)
      ctx->current_line[ctx->line_index + 1] = ' ';

    ctx->line_index++;
    return false;
  } else if (c == '*' && safe_char(ctx->current_line, ctx->line_index + 1) == '/') {
    // Error: Found a closing comment tag without an opening one.
    print_log(LOG_ERROR, ERR_UNMATCHED, (LogPosition){ctx->line_number, ctx->line_index + 1}, "*/", "comment", "*/");
    ctx->has_syntax_error = 1;
    return true; // Indicates a fatal error.
  }
  return false; // No comment detected.
}

// Checks for an unclosed multi-line comment at the end of the input.
void check_unclosed_comment(void) {
  debug_func("");
  if (ctx->state == STATE_MULTI_COMMENT) {
    save_log(LOG_ERROR, ERR_UNCLOSED, (LogPosition){ctx->multi_comment_line, ctx->multi_comment_index}, "/*", "comment", "/*");
  }
}
