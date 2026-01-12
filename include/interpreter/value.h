#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include "parser/ast.h"

typedef enum {
  VALUE_NUMBER,
  VALUE_BOOLEAN,
  VALUE_CHAR,
  VALUE_STRING,
  VALUE_NULL
} ValueType;

typedef struct {
  ValueType type;
  union {
    double number;
    bool boolean;
    char char_;
    char *string;
  };
} Value;

Value make_number(double v);
Value make_boolean(double b);
Value make_char(const char *src);
Value make_string_from_buf_take(char *buf);
Value make_string(const char *s);
double as_number(Value v);
int truthy(Value v);
Value repeat_string(const char *s, long long n);


#endif