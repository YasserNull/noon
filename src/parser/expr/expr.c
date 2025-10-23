// parser/expr/expr.c
// This file implements the parsing logic for different levels of operator
// precedence, following the standard order of operations. Each function handles
// one or more operators at a specific precedence level and calls the function
// for the next higher level to parse its operands.

#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/expression.h"

#include "parser/parser.h"
#include "utils/log.h"
#include "utils/memory.h"

// Parses assignment operators (e.g., =, +=, -=). Lowest precedence.
Node *parse_assignment(void) {
  debug_func("");
  Node *left_node = parse_logical_or();

  if (left_node && peek(0)) {
    const Token *tok = peek(0);
    TokenType op_type = tok->token_type;

    // Check if the token is an assignment operator.
    if (op_type == TOKEN_EQUAL || op_type == TOKEN_PLUSEQUAL || op_type == TOKEN_MINEQUAL || op_type == TOKEN_STAREQUAL || op_type == TOKEN_SLASHEQUAL || op_type == TOKEN_PERCENTEQUAL ||
        op_type == TOKEN_AMPERSANDEQUAL || op_type == TOKEN_PIPEEQUAL || op_type == TOKEN_CARETEQUAL || op_type == TOKEN_LEFTSHIFTEQUAL || op_type == TOKEN_RIGHTSHIFTEQUAL ||
        op_type == TOKEN_DOUBLESTAREQUAL || op_type == TOKEN_DOUBLEPERCENTEQUAL) {

      eat(op_type);

      // Assignment is right-associative, so we recursively call
      // parse_assignment.
      Node *right_node = parse_assignment();

      if (!right_node) {
        if (!ctx->has_syntax_error) {
          ctx->has_syntax_error = 1;
          print_log(LOG_ERROR, ERR_EXPECTED_VALUE_AFTER_OP, (LogPosition){tok->token_line, tok->token_index + 1}, tok->token_value, tok->token_value);
        }
        free_node(left_node);
        return NULL;
      }
      return create_binary_op_node(*tok, left_node, right_node);
    }
  }
  return left_node;
}

// Parses the logical OR operator (||).
Node *parse_logical_or(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_OR};
  return parse_binary_operator(1, operators, parse_logical_and);
}

// Parses the logical AND operator (&&).
Node *parse_logical_and(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_AND};
  return parse_binary_operator(1, operators, parse_or);
}

// Parses the bitwise OR operator (|).
Node *parse_or(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_PIPE};
  return parse_binary_operator(1, operators, parse_xor);
}

// Parses the bitwise XOR operator (^).
Node *parse_xor(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_CARET};
  return parse_binary_operator(1, operators, parse_and);
}

// Parses the bitwise AND operator (&).
Node *parse_and(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_AMPERSAND};
  return parse_binary_operator(1, operators, parse_equality);
}

// Parses equality operators (==, !=).
Node *parse_equality(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_EQEQUAL, TOKEN_NOTEQUAL};
  return parse_binary_operator(2, operators, parse_relational);
}

// Parses relational operators (<, <=, >, >=).
Node *parse_relational(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_LESS, TOKEN_LESSEQUAL, TOKEN_GREATER, TOKEN_GREATEREQUAL};
  return parse_binary_operator(4, operators, parse_shift);
}

// Parses bitwise shift operators (<<, >>).
Node *parse_shift(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_LEFTSHIFT, TOKEN_RIGHTSHIFT};
  return parse_binary_operator(2, operators, parse_expr);
}

// Parses additive operators (+, -).
Node *parse_expr(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_PLUS, TOKEN_MINUS};
  return parse_binary_operator(2, operators, parse_term);
}

// Parses multiplicative operators (*, /, //, %).
Node *parse_term(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_STAR, TOKEN_SLASH, TOKEN_DOUBLEPERCENT, TOKEN_PERCENT};
  return parse_binary_operator(4, operators, parse_power);
}

// Parses the exponentiation operator (**).
Node *parse_power(void) {
  debug_func("");
  TokenType operators[] = {TOKEN_POW};
  return parse_binary_operator(1, operators, parse_unary);
}
