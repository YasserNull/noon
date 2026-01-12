#ifndef EVAL_H
#define EVAL_H

#include "parser/ast.h"
#include "interpreter/value.h"

Value eval(Node *root);

#endif