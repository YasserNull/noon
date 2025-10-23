// lexer/tokens.c
// This file handles the creation, management, and utility functions for tokens.
// It includes functions for converting token types to strings, appending tokens
// to a dynamic array, and helpers for the parser to consume tokens.

#include "lexer/tokens.h"
#include "config.h"
#include "context.h"
#include "input.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Converts a TokenType enum to its string representation for
// debugging/printing.
const char *token_type_to_string(TokenType type) {
  debug_func("type %d", type);
  switch (type) {
  case TOKEN_INT: return "integer";
  case TOKEN_FLOAT: return "float";
  case TOKEN_BINARY: return "binary";
  case TOKEN_OCTAL: return "octal";
  case TOKEN_HEX: return "hexadecimal";
  case TOKEN_CHAR: return "char";
  case TOKEN_STRING: return "string";
  case TOKEN_IDENTIFIER: return "identifier";
  case TOKEN_BOOLEAN: return "boolean";
  case TOKEN_NULL: return "null";
  case TOKEN_TRUE: return "true";
  case TOKEN_FALSE: return "false";
  case TOKEN_KEYWORD: return "keyword";
  case TOKEN_COMMA: return "comma";
  case TOKEN_SEMICOLON: return "semicolon";
  case TOKEN_DOT: return "dot";
  case TOKEN_COLON: return "colon";
  case TOKEN_LPAREN: return "left parenthesis";
  case TOKEN_RPAREN: return "right parenthesis";
  case TOKEN_LBRACE: return "left brace";
  case TOKEN_RBRACE: return "right brace";
  case TOKEN_LBRACKET: return "left bracket";
  case TOKEN_RBRACKET: return "right bracket";
  case TOKEN_PLUS: return "plus";
  case TOKEN_MINUS: return "minus";
  case TOKEN_STAR: return "multiply";
  case TOKEN_SLASH: return "divide";
  case TOKEN_PERCENT: return "percent";
  case TOKEN_EQUAL: return "equal";
  case TOKEN_NOT: return "not";
  case TOKEN_LESS: return "less than";
  case TOKEN_GREATER: return "greater than";
  case TOKEN_EQEQUAL: return "equals";
  case TOKEN_NOTEQUAL: return "not equal";
  case TOKEN_LESSEQUAL: return "less or equal";
  case TOKEN_GREATEREQUAL: return "greater or equal";
  case TOKEN_AND: return "and";
  case TOKEN_OR: return "or";
  case TOKEN_AMPERSAND: return "ampersand";
  case TOKEN_PIPE: return "pipe";
  case TOKEN_CARET: return "caret";
  case TOKEN_TILDE: return "tilde";
  case TOKEN_LEFTSHIFT: return "left shift";
  case TOKEN_RIGHTSHIFT: return "right shift";
  case TOKEN_PLUSEQUAL: return "plus equal";
  case TOKEN_MINEQUAL: return "minus equal";
  case TOKEN_STAREQUAL: return "multiply equal";
  case TOKEN_SLASHEQUAL: return "divide equal";
  case TOKEN_PERCENTEQUAL: return "percent equal";
  case TOKEN_DOUBLESTAREQUAL: return "power equal";
  case TOKEN_DOUBLEPERCENTEQUAL: return "floor divide equal";
  case TOKEN_AMPERSANDEQUAL: return "and equal";
  case TOKEN_PIPEEQUAL: return "or equal";
  case TOKEN_CARETEQUAL: return "caret equal";
  case TOKEN_LEFTSHIFTEQUAL: return "left shift equal";
  case TOKEN_RIGHTSHIFTEQUAL: return "right shift equal";
  case TOKEN_DOUBLEPERCENT: return "floor divide";
  case TOKEN_POW: return "power";
  case TOKEN_ARROW: return "arrow";
  case TOKEN_COLONEQUAL: return "colon equal";
  case TOKEN_ELLIPSIS: return "ellipsis";
  case TOKEN_INCREMENT: return "increment";
  case TOKEN_DECREMENT: return "decrement";
  case TOKEN_SCOPE: return "scope";
  case TOKEN_QUESTION: return "ternary operator";
  case TOKEN_UNKNOWN:
  default: return "unknown";
  }
}

// Helper function to create and initialize a single Token struct.
static Token create_token(TokenType token_type, const char *token_value, size_t token_line, size_t token_index) {
  debug_func("token_type: %d, token_value: %s,token_line: %zu, token_index: %zu", token_type, token_value, token_line, token_index);
  Token token;
  token.token_type = token_type;
  token.token_line = token_line;
  token.token_index = token_index;
  if (!token_value)
    token_value = "";
  // Allocate memory for the token's value and copy it.
  size_t n = strlen(token_value);
  token.token_value = safe_malloc(n + 1);
  memcpy(token.token_value, token_value, n);
  token.token_value[n] = '\0';
  return token;
}

// Ensures the global tokens array has enough capacity for a new token.
static void ensure_tokens_capacity(void) {
  debug_func("");
  if (ctx->tokens_count >= ctx->tokens_capacity) {
    size_t new_capacity = ctx->tokens_capacity ? ctx->tokens_capacity * 2 : INITIAL_CAPACITY;
    Token *tmp = safe_realloc(ctx->tokens, sizeof(Token) * new_capacity);
    ctx->tokens = tmp;
    ctx->tokens_capacity = new_capacity;
  }
}

// Appends a new token to the global tokens array.
void append_token(TokenType token_type, const char *token_value, size_t token_line, size_t token_index) {
  debug_func("token_type: %d, token_value: %s,token_line: %zu, token_index: %zu", token_type, token_value, token_line, token_index);
  ensure_tokens_capacity();
  ctx->tokens[ctx->tokens_count++] = create_token(token_type, token_value, token_line, token_index);
}

// Prints all collected tokens if the dump_tokens flag is enabled.
void print_tokens(void) {
  debug_func("");
  if (ni->dump_tokens) {
    for (size_t i = 0; i < ctx->tokens_count; ++i) {
      printf("[%zu] %s: %s\n", i, token_type_to_string(ctx->tokens[i].token_type), ctx->tokens[i].token_value);
    }
    putchar('\n');
  }
}

// Frees the memory used by the global tokens array and resets its state.
void free_tokens(void) {
  debug_func("");
  if (!ctx->tokens)
    return;
  // Free the value string for each token.
  for (size_t i = 0; i < ctx->tokens_count; ++i) {
    if (ctx->tokens[i].token_value) {
      free(ctx->tokens[i].token_value);
      ctx->tokens[i].token_value = NULL;
    }
  }
  // Free the array itself.
  free(ctx->tokens);
  ctx->tokens = NULL;
  ctx->tokens_capacity = 0;
  ctx->tokens_position = 0;
  ctx->tokens_count = 0;
}

// Parser helper: looks at a token in the stream without consuming it.
Token *peek(size_t token_position) {
  debug_func("token_position: %zu", token_position);
  if (ctx->tokens_position < ctx->tokens_count && token_position < ctx->tokens_count)
    return &ctx->tokens[ctx->tokens_position + token_position];
  return NULL;
}

// Parser helper: consumes the current token if it matches the expected type.
Token *eat(TokenType type) {
  debug_func("type: %d", type);
  Token *tok = peek(0);
  if (tok && tok->token_type == type) {
    ctx->tokens_position++;
    return tok;
  }
  return NULL;
}

// Parser helper: returns the token that was just consumed.
const Token *previous_token(void) {
  debug_func("");
  if (ctx->tokens_position > 0)
    return &ctx->tokens[ctx->tokens_position - 1];
  return NULL;
}
