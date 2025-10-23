// lexer/lexer.h
// Header file for the lexer. It declares the main lexer functions,
// state definitions, and handler functions for specific lexical constructs
// like brackets, comments, and quotes.

#ifndef LEXER_H
#define LEXER_H

#include "utils/memory.h"

#include "config.h"

#include <stdbool.h>
#include <stddef.h>

// Struct to define an open/close bracket pair.
typedef struct {
  char open;
  char close;
  size_t bracket_line;
  size_t bracket_index;
} Bracket;

// Struct for an item on the bracket matching stack.
typedef struct {
  Bracket bracket;
  size_t bracket_line;
  size_t bracket_index;
} BracketStackItem;

extern const Bracket brackets[];
extern const size_t NUM_BRACKETS;

// Handler function declarations.
bool handle_brackets(char c);
void check_unclosed_brackets(void);
void handle_multi_comment(char c);
bool handle_comments(char c);
void check_unclosed_comment(void);
bool tokenize_strings(char c);
void handle_quotes(char c);
void check_unclosed_quote(void);

// Defines the possible states of the lexer state machine.
typedef enum { STATE_NORMAL, STATE_QUOTE, STATE_MULTI_COMMENT } LexerState;

// Main lexer function declarations.
void init_lexer(void);
int lexer(void);

#endif
