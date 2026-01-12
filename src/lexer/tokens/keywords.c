// lexer/tokens/keywords.c
// This file defines the language's keywords and provides helper functions
// to classify identifiers as either value-keywords (literals) or
// syntax-keywords (language constructs).

#include "lexer/tokens.h"
#include "utils/log.h"
#include <stdbool.h>
#include <string.h>

/* ============================================================
 * Syntax Keywords (language constructs, no intrinsic value)
 * ------------------------------------------------------------
 * These keywords affect parsing decisions. They must take
 * priority over identifiers.
 * ============================================================ */

typedef enum {
	KEYWORD_CONST,
	KEYWORD_VOID,
  KEYWORD_INT,
  KEYWORD_FLOAT,
  KEYWORD_BOOL,
  KEYWORD_CHAR,
  KEYWORD_STRING
} KeywordType;

typedef struct {
  const char *word;
  KeywordType type;
} SyntaxKeywordEntry;

static const SyntaxKeywordEntry SYNTAX_KEYWORDS[] = {
    {"const", KEYWORD_CONST},
    {"void", KEYWORD_VOID},
    {"int",   KEYWORD_INT},
    {"float",  KEYWORD_FLOAT},
    {"bool", KEYWORD_BOOL},
    {"char", KEYWORD_CHAR},
    {"string", KEYWORD_STRING},
};

static const int NUM_SYNTAX_KEYWORDS =
    (int)(sizeof(SYNTAX_KEYWORDS) / sizeof(SYNTAX_KEYWORDS[0]));

bool is_syntax_keyword(const char *word, KeywordType *out_type) {
  debug_func("%s", word);

  if (!word)
    return false;

  for (int i = 0; i < NUM_SYNTAX_KEYWORDS; i++) {
    if (strcmp(word, SYNTAX_KEYWORDS[i].word) == 0) {
      if (out_type)
        *out_type = SYNTAX_KEYWORDS[i].type;
      return true;
    }
  }

  return false;
}

/* ============================================================
 * Value Keywords (have intrinsic values)
 * ------------------------------------------------------------
 * These keywords behave like literals and are emitted directly
 * as their corresponding TokenType by the lexer.
 * ============================================================ */

typedef struct {
  const char *word;
  TokenType type;
} ValueKeywordEntry;

static const ValueKeywordEntry VALUE_KEYWORDS[] = {
    {"true",  TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"null",  TOKEN_NULL},
};

static const int NUM_VALUE_KEYWORDS =
    (int)(sizeof(VALUE_KEYWORDS) / sizeof(VALUE_KEYWORDS[0]));

/* ============================================================
 * Keyword Classification (lexer entry point)
 * ------------------------------------------------------------
 * Priority order:
 * 1) Syntax keyword  -> TOKEN_KEYWORD
 * 2) Value keyword   -> literal token
 * 3) Otherwise       -> TOKEN_IDENTIFIER
 * ============================================================ */

TokenType get_keyword_type(const char *word) {
  debug_func("%s", word);

  if (!word)
    return TOKEN_IDENTIFIER;

  // 1) Syntax keywords have highest priority
  if (is_syntax_keyword(word, NULL)) {
    return TOKEN_KEYWORD;
  }

  // 2) Value keywords (true, false, null)
  for (int i = 0; i < NUM_VALUE_KEYWORDS; i++) {
    if (strcmp(word, VALUE_KEYWORDS[i].word) == 0) {
      return VALUE_KEYWORDS[i].type;
    }
  }

  // 3) Regular identifier
  return TOKEN_IDENTIFIER;
}