#include "interpreter/interpreter.h"
#include "interpreter/value.h"
#include "interpreter/eval.h"
#include "parser/parser.h"
#include "config.h"
#include "context.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/expression.h"
#include "interpreter/interpreter.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdio.h>

void print_result(Value result) {
    if (!ni->is_repl)
        return;

    switch (result.type) {
    case VALUE_NUMBER:
        printf("%g\n", result.number);
        break;
    case VALUE_BOOLEAN:
        printf("%s\n", result.boolean ? "true" : "false");
        break;
    case VALUE_CHAR:
        printf("%c\n", result.char_);
        break;
    case VALUE_STRING:
        printf("%s\n", result.string);
        break;
    default:
        break;
    }
}

void interpreter(Node *node) {
    Value eval_result = eval(node);

    switch (eval_result.type) {
    case VALUE_NUMBER:
    case VALUE_STRING:
    case VALUE_CHAR:
    case VALUE_BOOLEAN:
        print_result(eval_result);
        break;
    default:
        break;
    }
}