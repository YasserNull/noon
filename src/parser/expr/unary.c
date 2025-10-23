// parser/expr/unary.c
// This file handles the parsing of unary operators. It distinguishes between
// prefix operators (e.g., -x, ++x) and postfix operators (e.g., x++).

#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/expression.h"
#include "utils/log.h"

// Parses prefix unary operators (e.g., +, -, !, ~, ++, --).
static Node *parse_prefix_unary(void) {
  debug_func("");
  const Token *tok = peek(0);

  // Check if the current token is a prefix operator.
  if (tok && (tok->token_type == TOKEN_PLUS || tok->token_type == TOKEN_MINUS || tok->token_type == TOKEN_NOT || tok->token_type == TOKEN_TILDE || tok->token_type == TOKEN_INCREMENT ||
              tok->token_type == TOKEN_DECREMENT)) {
    Token op_token = *tok;
    if (!eat(op_token.token_type))
      return NULL;

    const Token *next_tok = peek(0);
    if (!next_tok) {
      // Error if an operator is not followed by an expression.
      if (!ctx->has_syntax_error) {
        ctx->has_syntax_error = 1;
        print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){op_token.token_line, op_token.token_index}, op_token.token_value);
      }
      return NULL;
    }

    // Recursively call to handle multiple prefix operators like --++x.
    Node *operand_node = parse_prefix_unary();
    if (!operand_node)
      return NULL;

    return create_unary_op_node(op_token, operand_node);
  }

  // If there's no prefix operator, parse the next higher precedence level (a
  // factor).
  return parse_factor();
}

// Main function to parse unary expressions, handling both prefix and postfix
// operators.
Node *parse_unary(void) {
  debug_func("");
  // First, parse any prefix operators or the base factor.
  Node *node = parse_prefix_unary();
  if (!node) {
    return NULL;
  }

  // Now, loop to check for any postfix operators (e.g., ++, --).
  while (true) {
    const Token *tok = peek(0);
    if (tok && (tok->token_type == TOKEN_INCREMENT || tok->token_type == TOKEN_DECREMENT)) {
      Token op_token = *tok;
      eat(op_token.token_type);

      // Create a new postfix node, with the previous node as its operand.
      node = create_postfix_op_node(op_token, node);
      if (!node) {
        return NULL;
      }
    } else {
      break; // No more postfix operators.
    }
  }

  return node;
}
