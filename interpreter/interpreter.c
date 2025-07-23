#include "interpreter.h"
#include "../math/math.h"
#include "../parentheses/parentheses.h"
#include "../tinyexpr/tinyexpr.h"
#include "../tuple/tuple.h"
#include "../utilities/utilities.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// محلل نص
char *evaluate_expression(char *string) {
  if (!string)
    return NULL;

  string = strdup(string);
  if (!string)
    return NULL;

  char *tmp = NULL;

  // إذا كان تعبير رياضي بدون tuple
  tmp = evaluate_math_expr(string);
  if (tmp && !is_tuple_syntax(string)) {
    free(string);
    string = tmp;
  }

  // إذا كان محاطًا بأقواس ()
  if (is_wrapped_expression(string)) {
    char *inner = unwrap_expression(string);
    tmp = strdup(inner);
    if (tmp) {
      free(string);
      string = tmp;
    }
  }

  // إذا كان tuple
  if (is_tuple_syntax(string)) {
    tmp = parse_tuple(string);
    if (tmp) {
      free(string);
      string = tmp;
    }
  }

  return string;
}

// المفسر
void interpreter(char *string) {
  if (!string)
    return;

  char *result = evaluate_expression(string);

  if (result) {
    printf("%s\n", result);
    free(result); // تحرير الذاكرة بعد الاستخدام
  }
}