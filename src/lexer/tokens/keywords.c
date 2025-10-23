// lexer/tokens/keywords.c
// This file defines the language's keywords and provides a function
// to check if a given identifier string is a keyword.

#include "lexer/tokens.h"
#include "utils/log.h"
#include <stdbool.h>
#include <string.h>

// A struct to map keyword strings to their token types.
typedef struct {
  const char *word;
  TokenType type;
} KeywordEntry;

// The table of all keywords in the language.
static const KeywordEntry KEYWORDS[] = {{"bool", TOKEN_BOOLEAN}, {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"null", TOKEN_NULL}};

static const int NUM_KEYWORDS = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

// Looks up a string in the keyword table.
// Returns the corresponding TokenType if found, otherwise returns
// TOKEN_IDENTIFIER.
TokenType get_keyword_type(const char *word) {
  debug_func("%s", word);
  if (!word)
    return TOKEN_IDENTIFIER;

  for (int i = 0; i < NUM_KEYWORDS; i++) {
    if (strcmp(word, KEYWORDS[i].word) == 0) {
      return KEYWORDS[i].type;
    }
  }
  return TOKEN_IDENTIFIER; // default if not a keyword
}
