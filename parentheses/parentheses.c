#include "parentheses.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// التحقق من إذا ما كان النص ملفوفًا بأقواس ()
int is_wrapped_expression(const char *string) {
  int i, bracket_depth = 0;
  bool in_quotes = false;
  char quote_char = '\0';
  char stack[256];
  int stack_top = -1;

  for (i = 0; isspace(string[i]); i++)
    ;

  if (string[i] != '(')
    return 0;

  char open = '(';
  char close = ')';

  for (; string[i]; i++) {
    char ch = string[i];

    if (ch == '\\') {
      i++;
      continue;
    }

    if (!in_quotes && (ch == '"' || ch == '\'')) {
      in_quotes = true;
      quote_char = ch;
    } else if (in_quotes && ch == quote_char) {
      in_quotes = false;
    } else if (!in_quotes) {
      if (ch == '(' || ch == '[' || ch == '{') {
        stack[++stack_top] = ch;
        if (ch == '(')
          bracket_depth++;
      } else if (ch == ')' || ch == ']' || ch == '}') {
        if (stack_top < 0)
          return 0;
        char last = stack[stack_top--];
        if (!((last == '(' && ch == ')') || (last == '[' && ch == ']') ||
              (last == '{' && ch == '}')))
          return 0;
        if (ch == ')')
          bracket_depth--;
        if (bracket_depth == 0 && string[i + 1] != '\0') {
          while (isspace(string[i + 1]))
            i++;
          if (string[i + 1] != '\0')
            return 0;
        }
      }
    }
  }
  return bracket_depth == 0 && stack_top == -1;
}

// إزالة القوسين الخارجيين إذا كانا بدون فائدة ()
char *unwrap_expression(char *string) {
  int i, start = 0, end;

  // تخطي الفراغات
  for (i = 0; isspace(string[i]); i++)
    ;
  start = i;

  // تحديد نهاية السلسلة
  for (end = strlen(string) - 1; end > start && isspace(string[end]); end--)
    ;

  // تحقق من وجود قوسين متطابقين ()
  if (string[start] == '(' && string[end] == ')') {
    string[end] = '\0'; // إزالة القوس الأخير
    if (is_wrapped_expression(&string[start + 1])) {
      return unwrap_expression(&string[start + 1]);
    } else {
      return &string[start + 1]; // تجاوز القوس الأول
    }
  }
  return string;
}
