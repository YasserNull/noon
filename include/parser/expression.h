// parser/expression.h
// Header file for expression parsing. It declares the functions for parsing
// expressions at each level of operator precedence, from assignment down to
// factors and unary operators.

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "parser/ast.h"

// Declarations for functions that parse expressions based on operator
// precedence.
Node *parse_or(void);
Node *parse_shift(void);
Node *parse_xor(void);
Node *parse_power(void);
Node *parse_term(void);
Node *parse_expr(void);
Node *parse_and(void);
Node *parse_assignment(void);
Node *parse_logical_or(void);
Node *parse_logical_and(void);
Node *parse_equality(void);
Node *parse_relational(void);

// Declarations for the highest precedence levels.
Node *parse_factor(void);
Node *parse_unary(void);

#endif
