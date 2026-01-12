#include "interpreter/value.h"
#include "utils/memory.h"
#include <string.h>
#include <stdlib.h>

/* ========== Constructors ========== */

Value make_number(double v) {
    Value value;
    value.type = VALUE_NUMBER;
    value.number = v;
    return value;
}

Value make_boolean(double b) {
    Value value;
    value.type = VALUE_BOOLEAN;
    value.boolean = (b == 1.0);
    return value;
}

Value make_char(const char *src) {
    Value value;
    value.type = VALUE_CHAR;
    value.char_ = (src && *src) ? *src : ' ';
    return value;
}

Value make_string_from_buf_take(char *buf) {
    Value v;
    v.type = VALUE_STRING;
    v.string = buf;
    return v;
}

Value make_string(const char *s) {
    Value value;
    value.type = VALUE_STRING;
    value.string = safe_strdup(s ? s : "");
    return value;
}

/* ========== Helpers ========== */

double as_number(Value v) {
    if (v.type == VALUE_NUMBER)
        return v.number;
    return 0.0;
}

int truthy(Value v) {
    if (v.type == VALUE_STRING)
        return v.string && v.string[0] != '\0';
    if (v.type == VALUE_BOOLEAN)
        return v.boolean ? 1 : 0;
    if (v.type == VALUE_CHAR)
        return v.char_ != '\0';
    return as_number(v) != 0.0;
}

Value repeat_string(const char *s, long long n) {
    if (!s || n <= 0)
        return make_string("");

    size_t len = strlen(s);
    size_t total = len * (size_t)n;

    char *buf = safe_malloc(total + 1);
    char *p = buf;

    for (long long i = 0; i < n; ++i) {
        memcpy(p, s, len);
        p += len;
    }
    buf[total] = '\0';

    return make_string_from_buf_take(buf);
}