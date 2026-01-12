#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "interpreter/value.h"
#include "parser/ast.h"

void print_result(Value result);
void interpreter(Node *node);

#endif