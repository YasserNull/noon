// context.h
// Header file for the global context. It defines the NoonContext struct,
// which holds the entire state of the interpreter (lexer, parser, logs, etc.),
// making it accessible throughout the program.

#ifndef CONTEXT_H
#define CONTEXT_H

#include "config.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "utils/log.h"
#include <stdbool.h>
#include <stddef.h>

// The main context struct holding all interpreter state.
typedef struct {
  /* Lexer */
  // Read lines
  ssize_t bytes_read;
  char *current_line;
  size_t line_length;
  size_t line_index;
  size_t line_number;
  char **lines;
  size_t lines_capacity;
  // Lexer state
  LexerState state;
  // Quotes
  char quote_char;
  size_t quote_line;
  size_t quote_index;
  // Comments
  size_t multi_comment_line;
  size_t multi_comment_index;
  // Brackets
  BracketStackItem *bracket_stack;
  size_t bracket_stack_size;
  size_t bracket_stack_capacity;

  /* Tokens */
  Token *tokens;
  size_t tokens_capacity;
  size_t tokens_count;
  size_t tokens_position;
  char *string_token;
  size_t string_token_length;
  size_t string_token_capacity;
  /* Ast */
  Node *ast_root;
  bool has_syntax_error;
  /* Logs */
  LogEntry *logs;
  size_t logs_count;
  size_t logs_capacity;
  int total_errors;
  int total_warnings;
  int total_infos;
} NoonContext;

// Global pointer to the context.
extern NoonContext *ctx;

// Function to initialize the context.
void init_context(void);

#endif
