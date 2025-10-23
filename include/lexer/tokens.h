// lexer/tokens.h
// Header file for tokens. It defines the TokenType enum for all possible
// token types, the Token struct itself, and declares functions for
// token management and parsing helpers.

#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>
#include <stddef.h>

// Enum of all possible token types in the language.
typedef enum {
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_BOOLEAN,
  TOKEN_NULL,

  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_BINARY,
  TOKEN_OCTAL,
  TOKEN_HEX,
  TOKEN_TRUE,
  TOKEN_FALSE,

  TOKEN_IDENTIFIER,
  TOKEN_KEYWORD,

  TOKEN_COMMA,
  TOKEN_SEMICOLON,
  TOKEN_DOT,
  TOKEN_COLON,

  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,

  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_EQUAL,
  TOKEN_NOT,
  TOKEN_LESS,
  TOKEN_GREATER,

  TOKEN_EQEQUAL,
  TOKEN_NOTEQUAL,
  TOKEN_LESSEQUAL,
  TOKEN_GREATEREQUAL,

  TOKEN_AND,
  TOKEN_OR,

  TOKEN_AMPERSAND,
  TOKEN_PIPE,
  TOKEN_CARET,
  TOKEN_TILDE,

  TOKEN_LEFTSHIFT,
  TOKEN_RIGHTSHIFT,

  TOKEN_PLUSEQUAL,
  TOKEN_MINEQUAL,
  TOKEN_STAREQUAL,
  TOKEN_SLASHEQUAL,
  TOKEN_PERCENTEQUAL,
  TOKEN_DOUBLESTAREQUAL,
  TOKEN_DOUBLEPERCENTEQUAL, // "//="
  TOKEN_AMPERSANDEQUAL,
  TOKEN_PIPEEQUAL,
  TOKEN_CARETEQUAL,
  TOKEN_LEFTSHIFTEQUAL,
  TOKEN_RIGHTSHIFTEQUAL,

  TOKEN_DOUBLEPERCENT, // "//"
  TOKEN_POW,           // "**"

  TOKEN_ARROW,
  TOKEN_COLONEQUAL,
  TOKEN_ELLIPSIS,

  TOKEN_INCREMENT, // "++"
  TOKEN_DECREMENT, // "--"
  TOKEN_SCOPE,     // "::"
  TOKEN_QUESTION,  // "?"

  TOKEN_UNKNOWN
} TokenType;

// Struct representing a single token.
typedef struct {
  TokenType token_type;
  char *token_value;
  size_t token_line;
  size_t token_index;
} Token;

// Parser helper function declarations.
Token *peek(size_t token_position);
Token *eat(TokenType type);
const Token *previous_token(void);

// Token utility function declarations.
const char *token_type_to_string(TokenType type);
bool is_unary(TokenType type);
bool is_operator(TokenType type);
void append_token(TokenType token_type, const char *token_value, size_t token_line, size_t token_col);
void print_tokens(void);
void free_tokens(void);

// Tokenizer function declarations.
bool tokenize_identifier(void);
bool tokenize_symbol(void);
TokenType get_keyword_type(const char *word);
bool tokenize_number(void);

// Struct for mapping symbol strings to token types.
typedef struct {
  const char *symbol;
  TokenType type;
} SymbolMap;

extern const SymbolMap symbols[];
extern const size_t NUM_SYMBOLS;
#endif
