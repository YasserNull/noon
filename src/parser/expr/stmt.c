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

// parser/statement/var_decl.c

Node *parse_variable_declaration(void) {
  debug_func("");

  bool is_const = false;

  // optional: const
  if (peek(0) &&
      peek(0)->token_type == TOKEN_KEYWORD &&
      strcmp(peek(0)->token_value, "const") == 0) {
    eat(TOKEN_KEYWORD);
    is_const = true;
  }

  // type (must be keyword)
  if (!peek(0) || peek(0)->token_type != TOKEN_KEYWORD) {
    print_log(LOG_ERROR, ERR_INVALID_SYNTAX,
              (LogPosition){peek(0)->token_line, peek(0)->token_index},
              peek(0)->token_value);
    return NULL;
  }

  Token type_token = *peek(0);
  eat(TOKEN_KEYWORD);

  // variable name
  if (!peek(0) || peek(0)->token_type != TOKEN_IDENTIFIER) {
    print_log(LOG_ERROR, "",
              (LogPosition){peek(0)->token_line, peek(0)->token_index},
              peek(0)->token_value);
    return NULL;
  }

  Token name_token = *peek(0);
  eat(TOKEN_IDENTIFIER);

  // =
  if (!peek(0) || peek(0)->token_type != TOKEN_EQUAL) {
    print_log(LOG_ERROR, "",
              (LogPosition){peek(0)->token_line, peek(0)->token_index},
              "=");
    return NULL;
  }
  eat(TOKEN_EQUAL);

  // value expression
  Node *value = parse_assignment();
  if (!value)
    return NULL;

  // ✅ مؤقتًا: رجّع value أو AST فارغ
  // لاحقًا نبني NODE_VAR_DECL
  return value;
}

Node *parse_statement(void) {
  debug_func("");

  // const <type> <name> = ...
  if (peek(0) &&
      peek(0)->token_type == TOKEN_KEYWORD &&
      strcmp(peek(0)->token_value, "const") == 0) {
    return parse_variable_declaration();
  }

  // <type> <name> = ...
  if (peek(0) &&
      peek(0)->token_type == TOKEN_KEYWORD) {
    return parse_variable_declaration();
  }

  // otherwise: expression / assignment
  return parse_assignment();
}