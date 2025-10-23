// lexer/tokens/identifiers.c
// This file contains the logic for tokenizing identifiers. An identifier is a
// sequence of letters, digits, and underscores, starting with a letter or
// underscore. After tokenizing, it checks if the identifier is a reserved
// keyword.

#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Attempts to tokenize an identifier from the current input position.
bool tokenize_identifier(void) {
  debug_func("");
  char ch = safe_char(ctx->current_line, ctx->line_index);

  // An identifier must start with a letter or an underscore.
  if (isalpha((unsigned char)ch) || ch == '_') {
    size_t start = ctx->line_index;
    // Consume all subsequent alphanumeric characters and underscores.
    while (isalnum((unsigned char)safe_char(ctx->current_line, ctx->line_index)) || safe_char(ctx->current_line, ctx->line_index) == '_')
      ctx->line_index++;

    // Extract the identifier string.
    size_t length = ctx->line_index - start;
    char *word = safe_malloc(length + 1);
    memcpy(word, &ctx->current_line[start], length);
    word[length] = '\0';

    // Check if the identifier is a keyword.
    TokenType type = get_keyword_type(word);
    append_token(type, word, ctx->line_number, ctx->line_index);

    free(word);
    ctx->line_index--; // Decrement to re-evaluate the current character in the
                       // next loop.
    return true;
  }
  return false;
}
