// lexer/tokens/symbols.c
// This file defines all single-character and multi-character symbols/operators
// and provides the logic to tokenize them from the input stream.

#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdbool.h>

// Attempts to tokenize a symbol from the current input position.
// It tries to match the longest possible symbol (e.g., `<<=` before `<<`).
bool tokenize_symbol(void) {
  debug_func("");
  // Peek up to 3 characters ahead for multi-character symbols.
  char c0 = safe_char(ctx->current_line, ctx->line_index);
  char c1 = safe_char(ctx->current_line, ctx->line_index + 1);
  char c2 = safe_char(ctx->current_line, ctx->line_index + 2);

  char three[4] = {c0, c1, c2, '\0'};
  char two[3] = {c0, c1, '\0'};
  char one[2] = {c0, '\0'};

  // Try matching 3-char, then 2-char, then 1-char symbols.
  for (int line_length_try = 3; line_length_try >= 1; line_length_try--) {
    const char *candidate = (line_length_try == 3) ? three : (line_length_try == 2) ? two : one;
    for (size_t i = 0; i < NUM_SYMBOLS; i++) {
      if (strcmp(symbols[i].symbol, candidate) == 0) {
        // Save start position before updating line_index.
        size_t start_index = ctx->line_index;

        // Advance the index by the length of the matched symbol.
        ctx->line_index += (size_t)(line_length_try - 1);

        // Use the start position when creating the token.
        append_token(symbols[i].type, symbols[i].symbol, ctx->line_number,
                     start_index + 1); // +1 because columns are 1-based.
        return true;
      }
    }
  }
  // If no symbol matches, but it's not whitespace, it's an unknown token.
  char ch = safe_char(ctx->current_line, ctx->line_index);
  if (!isspace((unsigned char)ch)) {
    const char tmp[2] = {ch, '\0'};
    append_token(TOKEN_UNKNOWN, tmp, ctx->line_number, ctx->line_index + 1);
    return true;
  }
  return false;
}

// The table of all symbols, ordered from longest to shortest for correct
// matching.
const SymbolMap symbols[] = {{"...", TOKEN_ELLIPSIS},
                             {"::", TOKEN_SCOPE},
                             {"++", TOKEN_INCREMENT},
                             {"--", TOKEN_DECREMENT},
                             {":=", TOKEN_COLONEQUAL},
                             {"<<=", TOKEN_LEFTSHIFTEQUAL},
                             {">>=", TOKEN_RIGHTSHIFTEQUAL},
                             {"**=", TOKEN_DOUBLESTAREQUAL},
                             {"%%=", TOKEN_DOUBLEPERCENTEQUAL},
                             {"==", TOKEN_EQEQUAL},
                             {"!=", TOKEN_NOTEQUAL},
                             {"<=", TOKEN_LESSEQUAL},
                             {">=", TOKEN_GREATEREQUAL},
                             {"<<", TOKEN_LEFTSHIFT},
                             {">>", TOKEN_RIGHTSHIFT},
                             {"&&", TOKEN_AND},
                             {"||", TOKEN_OR},
                             {"+=", TOKEN_PLUSEQUAL},
                             {"-=", TOKEN_MINEQUAL},
                             {"*=", TOKEN_STAREQUAL},
                             {"/=", TOKEN_SLASHEQUAL},
                             {"%=", TOKEN_PERCENTEQUAL},
                             {"&=", TOKEN_AMPERSANDEQUAL},
                             {"|=", TOKEN_PIPEEQUAL},
                             {"^=", TOKEN_CARETEQUAL},
                             {"->", TOKEN_ARROW},
                             {"%%", TOKEN_DOUBLEPERCENT},
                             {"**", TOKEN_POW},

                             {"(", TOKEN_LPAREN},
                             {")", TOKEN_RPAREN},
                             {"{", TOKEN_LBRACE},
                             {"}", TOKEN_RBRACE},
                             {"[", TOKEN_LBRACKET},
                             {"]", TOKEN_RBRACKET},
                             {",", TOKEN_COMMA},
                             {";", TOKEN_SEMICOLON},
                             {".", TOKEN_DOT},
                             {":", TOKEN_COLON},
                             {"+", TOKEN_PLUS},
                             {"-", TOKEN_MINUS},
                             {"*", TOKEN_STAR},
                             {"/", TOKEN_SLASH},
                             {"%", TOKEN_PERCENT},
                             {"=", TOKEN_EQUAL},
                             {"!", TOKEN_NOT},
                             {"<", TOKEN_LESS},
                             {">", TOKEN_GREATER},
                             {"&", TOKEN_AMPERSAND},
                             {"|", TOKEN_PIPE},
                             {"^", TOKEN_CARET},
                             {"~", TOKEN_TILDE},
                             {"?", TOKEN_QUESTION}};

// Helper function to check if a token type is a binary or assignment operator.
bool is_operator(TokenType type) {
  switch (type) {
  case TOKEN_PLUS:
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_PERCENT:
  case TOKEN_EQUAL:
  case TOKEN_NOT:
  case TOKEN_LESS:
  case TOKEN_GREATER:
  case TOKEN_EQEQUAL:
  case TOKEN_NOTEQUAL:
  case TOKEN_LESSEQUAL:
  case TOKEN_GREATEREQUAL:
  case TOKEN_AND:
  case TOKEN_OR:
  case TOKEN_AMPERSAND:
  case TOKEN_PIPE:
  case TOKEN_CARET:
  case TOKEN_TILDE:
  case TOKEN_LEFTSHIFT:
  case TOKEN_RIGHTSHIFT:
  case TOKEN_PLUSEQUAL:
  case TOKEN_MINEQUAL:
  case TOKEN_STAREQUAL:
  case TOKEN_SLASHEQUAL:
  case TOKEN_PERCENTEQUAL:
  case TOKEN_DOUBLESTAREQUAL:
  case TOKEN_DOUBLEPERCENTEQUAL:
  case TOKEN_AMPERSANDEQUAL:
  case TOKEN_PIPEEQUAL:
  case TOKEN_CARETEQUAL:
  case TOKEN_LEFTSHIFTEQUAL:
  case TOKEN_RIGHTSHIFTEQUAL:
  case TOKEN_DOUBLEPERCENT:
  case TOKEN_POW:
  case TOKEN_INCREMENT:
  case TOKEN_DECREMENT:
  case TOKEN_ARROW:
  case TOKEN_COLONEQUAL:
  case TOKEN_QUESTION: return true;
  default: return false;
  }
}

// Helper function to check if a token type can be a unary operator.
bool is_unary(TokenType type) {
  if (type == TOKEN_PLUS || type == TOKEN_MINUS || type == TOKEN_NOT || type == TOKEN_TILDE || type == TOKEN_INCREMENT || type == TOKEN_DECREMENT) {
    return true;
  }
  return false;
}

const size_t NUM_SYMBOLS = sizeof(symbols) / sizeof(symbols[0]);
