#include "interpreter/eval.h"
#include "interpreter/value.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Forward */
static Value eval_node(Node *node);

/* ========== Unary Op ========== */
static Value eval_unary(TokenType op, Value v) {
    double x = as_number(v);

    switch (op) {
    case TOKEN_PLUS: return make_number(+x);
    case TOKEN_MINUS: return make_number(-x);
    case TOKEN_NOT: return make_boolean(!truthy(v));
    case TOKEN_TILDE: return make_number((double)(~(long long)x));
    case TOKEN_INCREMENT: return make_number(x + 1);
    case TOKEN_DECREMENT: return make_number(x - 1);
    default:
        fprintf(stderr, "[runtime] unsupported unary operator (%d)\n", op);
        return make_number(0.0);
    }
}

/* ========== Postfix Op ========== */
static Value eval_postfix(TokenType op, Value v) {
    double x = as_number(v);
    if (op == TOKEN_INCREMENT || op == TOKEN_DECREMENT)
        return make_number(x);

    fprintf(stderr, "[runtime] unsupported postfix operator (%d)\n", op);
    return make_number(0.0);
}

/* ========== Binary Op ========== */
static Value eval_binary(TokenType op, Value l, Value r) {

    /* char == char */
    if ((op == TOKEN_EQEQUAL || op == TOKEN_NOTEQUAL) &&
        l.type == VALUE_CHAR && r.type == VALUE_CHAR) {

        int eq = (l.char_ == r.char_);
        return make_boolean(op == TOKEN_EQEQUAL ? eq : !eq);
    }

    /* char + char */
    if (op == TOKEN_PLUS &&
        l.type == VALUE_CHAR && r.type == VALUE_CHAR) {

        char tmp[3] = { l.char_, r.char_, '\0' };
        return make_string(tmp);
    }

    
    if (op == TOKEN_PLUS &&
        l.type == VALUE_CHAR && r.type == VALUE_STRING) {

        size_t rlen = strlen(r.string);
        char *buf = safe_malloc(rlen + 2);
        buf[0] = l.char_;
        memcpy(buf + 1, r.string, rlen);
        buf[rlen + 1] = '\0';
        return make_string_from_buf_take(buf);
    }

    if (op == TOKEN_PLUS &&
        l.type == VALUE_STRING && r.type == VALUE_CHAR) {

        size_t llen = strlen(l.string);
        char *buf = safe_malloc(llen + 2);
        memcpy(buf, l.string, llen);
        buf[llen] = r.char_;
        buf[llen + 1] = '\0';
        return make_string_from_buf_take(buf);
    }

    
    if (op == TOKEN_STAR &&
        l.type == VALUE_CHAR && r.type == VALUE_NUMBER) {

        long long n = (long long)r.number;
        if (n <= 0) return make_string("");
        char *buf = safe_malloc(n + 1);
        for (long long i = 0; i < n; i++) buf[i] = l.char_;
        buf[n] = '\0';
        return make_string_from_buf_take(buf);
    }

    /* number * char */
    if (op == TOKEN_STAR &&
        l.type == VALUE_NUMBER && r.type == VALUE_CHAR) {

        long long n = (long long)l.number;
        if (n <= 0) return make_string("");
        char *buf = safe_malloc(n + 1);
        for (long long i = 0; i < n; i++) buf[i] = r.char_;
        buf[n] = '\0';
        return make_string_from_buf_take(buf);
    }

    /* string + string */
    if (op == TOKEN_PLUS &&
        l.type == VALUE_STRING && r.type == VALUE_STRING) {

        size_t len = strlen(l.string) + strlen(r.string) + 1;
        char *buf = safe_malloc(len);
        strcpy(buf, l.string);
        strcat(buf, r.string);
        return make_string_from_buf_take(buf);
    }

    /* string * number */
    if (op == TOKEN_STAR &&
        l.type == VALUE_STRING && r.type == VALUE_NUMBER)
        return repeat_string(l.string, (long long)r.number);

    /* number * string */
    if (op == TOKEN_STAR &&
        l.type == VALUE_NUMBER && r.type == VALUE_STRING)
        return repeat_string(r.string, (long long)l.number);

    /* string comparisons */
    if ((op == TOKEN_EQEQUAL || op == TOKEN_NOTEQUAL) &&
        l.type == VALUE_STRING && r.type == VALUE_STRING) {

        int eq = strcmp(l.string, r.string) == 0;
        return make_boolean(op == TOKEN_EQEQUAL ? eq : !eq);
    }

    if (l.type == VALUE_STRING || r.type == VALUE_STRING) {
        fprintf(stderr, "[runtime] unsupported string operation (%d)\n", op);
        return make_number(0.0);
    }

if (l.type == VALUE_BOOLEAN && r.type == VALUE_NUMBER) {
    l.type = VALUE_NUMBER;
    l.number = l.boolean ? 1 : 0;
}

if (l.type == VALUE_NUMBER && r.type == VALUE_BOOLEAN) {
    r.type = VALUE_NUMBER;
    r.number = r.boolean ? 1 : 0;
}

if (l.type == VALUE_BOOLEAN && r.type == VALUE_BOOLEAN) {
    
    l.type = VALUE_NUMBER;
    r.type = VALUE_NUMBER;
    l.number = l.boolean ? 1 : 0;
    r.number = r.boolean ? 1 : 0;
}

    double a = as_number(l);
    double b = as_number(r);

    switch (op) {
    case TOKEN_PLUS: return make_number(a + b);
    case TOKEN_MINUS: return make_number(a - b);
    case TOKEN_STAR: return make_number(a * b);
    case TOKEN_SLASH:
        if (b == 0.0) {
            fprintf(stderr, "[runtime] division by zero\n");
            return make_number(0.0);
        }
        return make_number(a / b);

    case TOKEN_PERCENT:
        return make_number((double)((long long)a % (long long)b));

    case TOKEN_DOUBLEPERCENT:
        return make_number((double)((long long)a / (long long)b));

    case TOKEN_POW:
        return make_number(pow(a, b));

    case TOKEN_LEFTSHIFT:
        return make_number((double)((long long)a << (long long)b));

    case TOKEN_RIGHTSHIFT:
        return make_number((double)((long long)a >> (long long)b));

    case TOKEN_AMPERSAND:
        return make_number((double)((long long)a & (long long)b));

    case TOKEN_PIPE:
        return make_number((double)((long long)a | (long long)b));

    case TOKEN_CARET:
        return make_number((double)((long long)a ^ (long long)b));

    case TOKEN_EQEQUAL: return make_boolean(a == b);
    case TOKEN_NOTEQUAL: return make_boolean(a != b);
    case TOKEN_LESS: return make_boolean(a < b);
    case TOKEN_LESSEQUAL: return make_boolean(a <= b);
    case TOKEN_GREATER: return make_boolean(a > b);
    case TOKEN_GREATEREQUAL: return make_boolean(a >= b);

    case TOKEN_AND: return make_boolean(truthy(l) && truthy(r));
    case TOKEN_OR: return make_boolean(truthy(l) || truthy(r));

    default:
        fprintf(stderr, "[runtime] unsupported binary operator (%d)\n", op);
        return make_number(0.0);
    }
}

/* ========== eval_node ========== */
static Value eval_node(Node *node) {
    if (!node)
        return make_number(0.0);

    switch (node->node_type) {
    case NODE_NUMBER: return make_number(node->number_value);
    case NODE_BOOLEAN: return make_boolean(node->boolean_value ? 1 : 0);
    case NODE_NULL: return make_number(0.0);
    case NODE_STRING: return make_string(node->string_value);
    case NODE_CHAR: return make_char(node->char_value);

    case NODE_UNARY_OP:
        return eval_unary(node->unary.op.token_type,
                          eval_node(node->unary.operand));

    case NODE_POSTFIX_OP:
        return eval_postfix(node->unary.op.token_type,
                            eval_node(node->unary.operand));

    case NODE_BINARY_OP:
        return eval_binary(node->binary.op.token_type,
                           eval_node(node->binary.left),
                           eval_node(node->binary.right));
    }

    fprintf(stderr, "[runtime] unknown node type (%d)\n", node->node_type);
    return make_number(0.0);
}

/* ========== Public API ========== */
Value eval(Node *root) {
    return eval_node(root);
}