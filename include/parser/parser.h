// parser/parser.h
// Header file for the main parser logic. It declares the main `parse` function
// and the generic `parse_binary_operator` function which forms the core of the
// precedence climbing parser.

#ifndef PARSER_H
#define PARSER_H

#include "lexer/tokens.h"
#include "parser/ast.h"

// A function pointer type for parsing operands at a higher precedence level.
typedef Node *(*ParseOperandFunc)(void);

// Declaration for the generic binary operator parsing function.
Node *parse_binary_operator(int operator_count, const TokenType *operator_types, ParseOperandFunc parse_operand);

// Declaration for the main parser entry point.
Node *parse(void);

#endif
