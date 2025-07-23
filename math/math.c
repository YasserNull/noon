#include "math.h"
#include "../tinyexpr/tinyexpr.h"
#include "../utilities/utilities.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// دالة ل تحليل رياضيات
char *evaluate_math_expr(const char *string) {
  double value = te_interp(string, 0);
  if (!isnan(value)) {
    int size = snprintf(NULL, 0, "%g", value) + 1;
    char *result = (char *)malloc(size);
    if (!result) {
      fprintf(stderr, "Memory allocation failed!\n");
      cleanup(EXIT_FAILURE);
    }
    snprintf(result, size, "%g", value);
    return result;
  }
  return NULL;
}
