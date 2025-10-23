// context.c
// This file defines and initializes the global NoonContext struct.
// This struct acts as a central container for the entire state of the
// interpreter, including lexer state, token lists, AST, and logs.

#include "context.h"
#include "lexer/lexer.h"
#include "parser/ast.h"
#include "utils/log.h"
#include "utils/memory.h"

// Global pointer to the context.
NoonContext *ctx = NULL;

// Allocates and initializes the global context struct with default values.
void init_context(void) {
  debug_func("");
  ctx = safe_malloc(sizeof(NoonContext));
  /* Lexer */
  // Read lines
  ctx->bytes_read = 0;
  ctx->current_line = NULL;
  ctx->line_length = 0;
  ctx->line_index = 0;
  ctx->line_number = 0;
  ctx->lines = NULL;
  ctx->lines_capacity = 0;
  // Lexer state
  ctx->state = STATE_NORMAL;
  // Quotes
  ctx->quote_char = 0;
  ctx->quote_line = 0;
  ctx->quote_index = 0;
  // Comments
  ctx->multi_comment_line = 0;
  ctx->multi_comment_index = 0;
  // Brackets
  ctx->bracket_stack = NULL;
  ctx->bracket_stack_size = 0;
  ctx->bracket_stack_capacity = 0;

  /* Tokens */
  ctx->tokens = NULL;
  ctx->tokens_capacity = 0;
  ctx->tokens_count = 0;
  ctx->tokens_position = 0;
  ctx->string_token = NULL;
  ctx->string_token_length = 0;
  ctx->string_token_capacity = 0;
  /* Ast */
  ctx->ast_root = NULL;
  ctx->has_syntax_error = false;
  /* Logs */
  ctx->logs = NULL;
  ctx->logs_count = 0;
  ctx->logs_capacity = 0;
  ctx->total_errors = 0;
  ctx->total_warnings = 0;
  ctx->total_infos = 0;
}
