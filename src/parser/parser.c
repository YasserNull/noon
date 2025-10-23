// parser/parser.c
// This is the core of the parser. It implements a precedence climbing algorithm
// for parsing binary expressions and orchestrates the overall parsing process
// by calling functions for different precedence levels.

#include "parser/parser.h"
#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/expression.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// A generic function for parsing left-associative binary operators.
// It takes a set of operators and a function to parse the next higher
// precedence level (operand).
Node *parse_binary_operator(int operator_count, const TokenType *operator_types, ParseOperandFunc parse_operand) {
  debug_func("%d", operator_count);
  const Token *current = peek(0);
  const Token *next = peek(1);
  const Token *next_next = peek(2);

  /* Check for invalid starting operators, e.g., `* 5` or `5 + * 3` */
  if (current) {
    // Case: two consecutive binary operators, e.g., `a + * b`
    if (is_operator(current->token_type) && next && is_operator(next->token_type)) {
      if (!ctx->has_syntax_error) {
        ctx->has_syntax_error = 1;
        print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){current->token_line, current->token_index}, current->token_value);
      }
      return NULL;
    }
    if (next && is_operator(next->token_type) && next_next && is_operator(next_next->token_type)) {
      if (!ctx->has_syntax_error) {
        ctx->has_syntax_error = 1;
        print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){next->token_line, next->token_index}, next->token_value);
      }
      return NULL;
    }
    // Case: expression starts with a binary (non-unary) operator, e.g., `* 5`
    if (is_operator(current->token_type) && !is_unary(current->token_type)) {
      if (next && next->token_type != current->token_type) {
        if (!ctx->has_syntax_error) {
          ctx->has_syntax_error = 1;
          print_log(LOG_ERROR, ERR_EXPECTED_VALUE_BEFORE_OP, (LogPosition){current->token_line, current->token_index}, current->token_value, current->token_value);
        }
        return NULL;
      } else {
        if (!ctx->has_syntax_error) {
          ctx->has_syntax_error = 1;
          print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){current->token_line, current->token_index}, current->token_value);
        }
        return NULL;
      }
    } else if (next && is_operator(next->token_type)) {
      // Case: empty parentheses with an operator, e.g., `()*`
      if (next_next && (current->token_type == TOKEN_LPAREN && next_next->token_type == TOKEN_RPAREN)) {
        if (!ctx->has_syntax_error) {
          ctx->has_syntax_error = 1;
          print_log(LOG_ERROR, ERR_EXPECTED_EXPRESSION, (LogPosition){next->token_line, next->token_index}, next->token_value);
        }
        return NULL;
      }
    }
  }

  /* Check for an operator followed by nothing, e.g. "a + " */
  if (next) {
    for (int i = 0; i < operator_count; i++) {
      if ((!next_next && next->token_type == operator_types[i]) || (next->token_type == operator_types[i] && next_next && next_next->token_type == TOKEN_RPAREN)) {
        if (!ctx->has_syntax_error) {
          ctx->has_syntax_error = 1;
          print_log(LOG_ERROR, ERR_EXPECTED_VALUE_AFTER_OP, (LogPosition){next->token_line, next->token_index}, next->token_value, next->token_value);
        }
        return NULL;
      }
    }
  }

  // Parse the left-hand side operand.
  Node *left_node = parse_operand();
  if (!left_node)
    return NULL;

  // Loop to consume a sequence of operators at the current precedence level.
  while (1) {
    const Token *tok = peek(0);
    if (!tok)
      break;

    // Check if the current token is one of the operators for this level.
    int found_operator = 0;
    for (int i = 0; i < operator_count; i++) {
      if (tok->token_type == operator_types[i]) {
        found_operator = 1;
        break;
      }
    }
    if (!found_operator)
      break;

    // Consume the operator token.
    Token op_token = *tok;
    if (!eat(op_token.token_type)) {
      free_node(left_node);
      return NULL;
    }

    // Parse the right-hand side operand.
    Node *right_node = parse_operand();
    if (!right_node) {
      if (!ctx->has_syntax_error) {
        ctx->has_syntax_error = 1;
        print_log(LOG_ERROR, ERR_EXPECTED_VALUE_AFTER_OP, (LogPosition){op_token.token_line, op_token.token_index + 1}, op_token.token_value, op_token.token_value);
      }
      free_node(left_node);
      return NULL;
    }

    // Create a new binary operation node and make it the new left node.
    Node *new_node = create_binary_op_node(op_token, left_node, right_node);
    if (!new_node) {
      free_node(left_node);
      free_node(right_node);
      return NULL;
    }
    left_node = new_node;
  }

  return left_node;
}

// The main entry point for the parser.
Node *parse(void) {
  debug_func("");
  // Start parsing from the lowest precedence level (assignment).
  ctx->ast_root = parse_assignment();
  if (!ctx->ast_root) {
    return NULL;
  }

  // After parsing, check if there are any leftover tokens.
  if (peek(0) != NULL) {
    Token *extra = peek(0);
    // Leftover tokens indicate a syntax error.
    if (!ctx->has_syntax_error) {
      ctx->has_syntax_error = 1;
      print_log(LOG_ERROR, ERR_INVALID_SYNTAX, (LogPosition){extra->token_line, extra->token_index}, extra->token_value, extra->token_value);
    }
    free_node(ctx->ast_root);
    ctx->ast_root = NULL;
    return NULL;
  }
  return ctx->ast_root;
}
