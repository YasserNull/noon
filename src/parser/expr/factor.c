// parser/expr/factor.c
// This file parses the most basic elements of an expression, known as
// "factors". Factors are the highest-precedence elements and include literals
// (numbers, strings), and expressions grouped by parentheses.

#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/expression.h"
#include "utils/log.h"

// Parses a factor.
Node *parse_factor(void) {
  debug_func("");
  const Token *tok = peek(0);
  if (!tok) {
    // Error if the expression ends unexpectedly.
    if (!ctx->has_syntax_error) {
      ctx->has_syntax_error = 1;
      print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){ctx->line_number, ctx->line_index}, "");
    }
    return NULL;
  }

  switch (tok->token_type) {
  // Handle numeric literals.
  case TOKEN_INT:
  case TOKEN_FLOAT:
  case TOKEN_HEX:
  case TOKEN_BINARY:
  case TOKEN_OCTAL: eat(tok->token_type); return create_number_node(*tok, strtod(tok->token_value, NULL));
  // Handle string literal.
  case TOKEN_STRING: eat(TOKEN_STRING); return create_string_node(*tok, tok->token_value);
  // Handle character literal.
  case TOKEN_CHAR: eat(TOKEN_CHAR); return create_char_node(*tok, tok->token_value);
  // Handle boolean literals.
  case TOKEN_TRUE: eat(TOKEN_TRUE); return create_boolean_node(*tok, true);
  case TOKEN_FALSE: eat(TOKEN_FALSE); return create_boolean_node(*tok, false);
  // Handle null literal.
  case TOKEN_NULL:
    eat(TOKEN_NULL);
    return create_null_node(*tok);
    // Handle grouped expressions, fixing fallthrough and bracket matching logic.
  case TOKEN_LPAREN:
  case TOKEN_LBRACE:
  case TOKEN_LBRACKET:

  {
    TokenType open_bracket_type = tok->token_type;
    TokenType close_bracket_type;
    if (open_bracket_type == TOKEN_LPAREN)
      close_bracket_type = TOKEN_RPAREN;
    else if (open_bracket_type == TOKEN_LBRACE)
      close_bracket_type = TOKEN_RBRACE;
    else
      close_bracket_type = TOKEN_RBRACKET; /* TOKEN_LBRACKET */

    eat(open_bracket_type); // Consume the opening bracket.

    // Check for empty brackets e.g., `()`.
    if (peek(0) && peek(0)->token_type == close_bracket_type) {
      eat(close_bracket_type);
      return NULL;
    }

    // Recursively parse the expression inside the parentheses.
    Node *node = parse_assignment();
    if (!node)
      return NULL;
    // Expect a closing parenthesis.
    if (!eat(close_bracket_type)) {
      free_node(node);
      return NULL;
    }
    return node;
  }
  default:
    // If no factor matches, it's a syntax error.
    if (!ctx->has_syntax_error) {
      ctx->has_syntax_error = 1;
      print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){tok->token_line, tok->token_index}, "");
    }
    return NULL;
  }
}
