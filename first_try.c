#include "tinyexpr/tinyexpr.h"
#include <ctype.h>
#include <glib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool logger = false;
int line = 0;
int input_type;
int exit_proc = 0;
bool check_brackets(char *string);
void handle_sigint() {}
bool check_brackets_1(char *string) {
  if (logger == true) {
    printf("check_brackets:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int open = 0;
  int close = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (string[i] == '(') {
        open++;
      } else if (string[i] == ')') {
        close--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }
  if (open == 0 && close == 0) {
    return true;
  }
  return false;
}
char *brackets_r(const char *string);
void removeNewlines(char *text) {
  char *src = text;
  char *dst = text;

  while (*src) {
    if (*src != '\n') {
      *dst++ = *src;
    }
    src++;
  }
  *dst = '\0';
}
/*bool is_space(char c) {
  if (c == '\0' || c == '\t' && c == '\n' || c == ' ') {
    return true;
  }
  return false;
}*/
int is_space(char c) {
  return (c == '\0' || c == '\t' || c == '\n' || c == ' ');
}
char *plus_processor(char *string);
char *syntax_int(char *string);
char *plus_fixer(char *string);
char *text_processor(char *string);
void remove_newline(char *str);
int doublequote(char *string);
int singlequote(char *string);
int only_whitespace(const char *string);
void remove_spaces(char *string);
void trim(char *str) {
  if (logger == true) {
    printf("trim:%s\n", str);
  }
  int start = 0;
  int end = strlen(str) - 1;
  int i;

  for (i = 0; str[i] == ' ' || str[i] == '\t'; i++)
    ;
  start = i;

  // Find the last non-space character from the end
  for (i = end; i >= 0 && (str[i] == ' ' || str[i] == '\t'); i--)
    ;
  end = i;

  // Shift the characters to the beginning of the string
  for (i = start; i <= end; i++) {
    str[i - start] = str[i];
  }

  // Null terminate the trimmed string
  str[i - start] = '\0';
}

char *plus(char *string, int ignore_plus) {
  if (logger == true) {
    printf("plus:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  int plus_number = 0;
  char *result = (char *)malloc(strlen(string) + 1);
  int j = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && string[i] == '+') {
        if (plus_number < ignore_plus) {
          plus_number++;
          continue;
        }
      }

      if (string[i] == '(') {
        brackets++;
      }

      if (string[i] == ')') {
        brackets--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
    if (plus_number == ignore_plus) {
      result[j++] = string[i];
    }
  }
  result[j] = '\0';
  char *result1 = (char *)malloc(strlen(result) + 1);
  int a = 0;
  for (int i = 0; result[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && result[i] == '+') {
        break;
      }

      if (result[i] == '(') {
        brackets++;
      }

      if (result[i] == ')') {
        brackets--;
      }
    }
    if (result[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && result[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && result[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
    result1[a++] = result[i];
  }
  result1[a] = '\0';
  free(result); // تحرير الذاكرة المخصصة للمتغير result
  return result1;
}

int plus_count(char *string) {
  if (logger == true) {
    printf("plus_count:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  int plus_num = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && string[i] == '+') {
        plus_num++;
      }

      if (string[i] == '(') {
        brackets++;
      }

      if (string[i] == ')') {
        brackets--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }
  return plus_num;
}
char *comma(char *string, int ignore_comma) {
  if (logger == true) {
    printf("comma:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  int comma_number = 0;
  char *result = (char *)malloc(strlen(string) + 1);
  int j = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && string[i] == ',') {
        if (comma_number < ignore_comma) {
          comma_number++;
          continue;
        }
      }

      if (string[i] == '(') {
        brackets++;
      }

      if (string[i] == ')') {
        brackets--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
    if (comma_number == ignore_comma) {
      result[j++] = string[i];
    }
  }
  result[j] = '\0';
  char *result1 = (char *)malloc(strlen(result) + 1);
  int a = 0;
  for (int i = 0; result[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && result[i] == ',') {
        break;
      }

      if (result[i] == '(') {
        brackets++;
      }

      if (result[i] == ')') {
        brackets--;
      }
    }
    if (result[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && result[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && result[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
    result1[a++] = result[i];
  }
  result1[a] = '\0';
  free(result); // تحرير الذاكرة المخصصة للمتغير result
  return result1;
}

int comma_count(char *string) {
  if (logger == true) {
    printf("comma_count:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  int comma_num = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0 && string[i] == ',') {
        comma_num++;
      }

      if (string[i] == '(') {
        brackets++;
      }

      if (string[i] == ')') {
        brackets--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }
  return comma_num;
}
int error(char *string) {
  if (logger == true) {
    printf("error:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;

  int end = 0;
  bool close = false;
  int opened = 0;
  int closed = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (close == true) {
      break;
    }
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (string[i] == '(') {
        opened++;
      }

      if (string[i] == ')') {
        closed++;
        if (opened == closed) {
          close = true;
          end = i;
        }
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }
  for (int i = end + 1; string[i] != '\0'; i++) {

    if (!in_escape && !in_single_quotes && !in_double_quotes &&
        string[i] != ' ' && string[i] != '\n' && string[i] != '\t') {

      return 1;
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }
    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }
    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }
  return 0;
}
void remove_spaces(char *string) {
  if (logger == true) {
    printf("remove_spaces:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int j = 0;

  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (string[i] == '\n' /*|| string[i] == ' ' || string[i] == '\t'*/) {
        continue;
      }
    }

    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }

    string[j++] = string[i];
  }

  string[j] = '\0';
}

char *plus_trim(char *string) {
  if (logger == true) {
    printf("plus_trim:%s\n", string);
  }

  char *result = NULL;
  size_t string_len = strlen(string);

  // تخصيص ذاكرة مبدئية للناتج النهائي
  result = malloc(string_len + 1); // مبدئيًا، تكون بنفس حجم النص الأصلي
  if (result == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(1);
  }

  int result_index = 0;
  for (size_t i = 0; i < string_len; i++) {
    // تجاهل المسافات البيضاء (المسافة، التبويب، والسطر الجديد)
    if (string[i] != ' ' && string[i] != '\t' && string[i] != '\n') {
      result[result_index++] = string[i];
    }
  }

  // إضافة علامة النهاية للنص الناتج
  result[result_index] = '\0';

  // إعادة تخصيص الذاكرة لتتناسب مع الحجم النهائي للنص
  char *temp = realloc(result, result_index + 1);
  if (temp == NULL) {
    fprintf(stderr, "Failed to reallocate memory\n");
    free(result);
    exit(1);
  }
  result = temp;

  // إرجاع النتيجة النهائية

  return result;
}
char *comma_trim(char *string) {
  if (logger == true) {
    printf("comma_trim:%s\n", string);
  }

  char *result = NULL;
  size_t string_len = strlen(string);

  // تخصيص ذاكرة مبدئية للناتج النهائي
  result = malloc(string_len + 1); // مبدئيًا، تكون بنفس حجم النص الأصلي
  if (result == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(1);
  }

  int result_index = 0;
  for (size_t i = 0; i < string_len; i++) {
    // تجاهل المسافات البيضاء (المسافة، التبويب، والسطر الجديد)
    if (string[i] != ' ' && string[i] != '\t' && string[i] != '\n') {
      result[result_index++] = string[i];
    }
  }

  // إضافة علامة النهاية للنص الناتج
  result[result_index] = '\0';

  // إعادة تخصيص الذاكرة لتتناسب مع الحجم النهائي للنص
  char *temp = realloc(result, result_index + 1);
  if (temp == NULL) {
    fprintf(stderr, "Failed to reallocate memory\n");
    free(result);
    exit(1);
  }
  result = temp;

  // إرجاع النتيجة النهائية

  return result;
}
bool check_plus(char *string) {
  string = plus_trim(string);
  if (logger == true) {
    printf("check_plus:%s\n", string);
  }
  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return false;
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0) {
        if (string[i] == '+' && string[i - 1] != '+' && string[i + 1] != '+' &&
            string[i - 1] != ',' && string[i + 1] != ',' &&
            !is_space(string[i + 1]) && !is_space(string[i - 1])) {

          return true;
        }
      }
      if (string[i] == '(') {
        brackets++;
      } else if (string[i] == ')') {
        brackets--;
      }
    }

    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }

  return false;
}
bool check_comma(char *string) {
  string = comma_trim(string);
  if (logger == true) {
    printf("check_comma:%s\n", string);
  }
  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return false;
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int brackets = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (brackets == 0) {
        if (string[i] == ',' && string[i - 1] != '+' && string[i + 1] != '+' &&
            string[i - 1] != ',' && string[i + 1] != ',' &&
            !is_space(string[i + 1]) && !is_space(string[i - 1])) {
          return true;
        }
      }

      if (string[i] == '(') {
        brackets++;
      } else if (string[i] == ')') {
        brackets--;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }

  return false;
}
bool check_brackets(char *string) {
  if (logger == true) {
    printf("check_brackets:%s\n", string);
  }
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int inside_brackets = false;

  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && !in_single_quotes && !in_double_quotes) {
      if (string[i] == '(') {
        inside_brackets++;
      } else if (string[i] == ')') {
        inside_brackets--;
      } else if (inside_brackets == 0 && string[i] != ' ' &&
                 string[i] != '\t' && string[i] != '\n') {
        return true;
      }
    }

    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }

    if (!in_escape && string[i] == '\"') {
      if (!in_single_quotes) {
        in_double_quotes = !in_double_quotes;
      }
    }

    if (!in_escape && string[i] == '\'') {
      if (!in_double_quotes) {
        in_single_quotes = !in_single_quotes;
      }
    }
  }

  return false;
}

char *brackets_r(const char *string) {
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return NULL;
  }
  if (logger == true) {
    printf("brackets:%s\n", string);
  }

  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int open_brackets = 0;
  int close_brackets = 0;
  int start_index = -1;
  int result_length = 0;

  for (int i = 0; string[i] != '\0'; i++) {
    if (in_escape) {
      // Reset the escape flag
      in_escape = false;
    } else if (string[i] == '\\') {
      // Set the escape flag when encountering a backslash
      in_escape = true;
      continue;
    } else if (string[i] == '\"' && !in_single_quotes) {
      // Toggle double quotes mode
      in_double_quotes = !in_double_quotes;
    } else if (string[i] == '\'' && !in_double_quotes) {
      // Toggle single quotes mode
      in_single_quotes = !in_single_quotes;
    } else if (!in_double_quotes && !in_single_quotes) {
      if (string[i] == '(') {
        open_brackets++;
        if (open_brackets == close_brackets + 1) {
          start_index = i + 1;
        }
      } else if (string[i] == ')') {
        close_brackets++;
        if (open_brackets == close_brackets && start_index != -1) {
          result_length = i - start_index;
          break;
        }
      }
    }
  }

  char *result = (char *)malloc((result_length + 1) * sizeof(char));
  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  if (start_index != -1 && result_length > 0) {
    int result_index = 0;
    for (int j = start_index; j < start_index + result_length; j++) {
      result[result_index++] = string[j];
    }
    result[result_index] = '\0';
  } else {
    // Handle case where no valid bracket pair was found
    result[0] = '\0';
  }

  return result;
}

struct Variable {
  char *name;
  int key;
  char *value;
};

struct Variable *variables = NULL;
int NVariables = 0;

void AddVariable(const char *name, int key, const char *value) {
  variables = realloc(variables, (NVariables + 1) * sizeof(struct Variable));
  if (variables) {
    variables[NVariables].name = strdup(name);
    variables[NVariables].key = key;
    variables[NVariables].value = strdup(value);
    NVariables++;
  } else {
    printf("Memory allocation failed.\n");
  }
}

const char *GetVariable(const char *name, int key) {
  for (int i = 0; i < NVariables; ++i) {
    if (strcmp(variables[i].name, name) == 0 && variables[i].key == key) {
      return variables[i].value;
    }
  }
  printf("Variable with name %s and key %d not found.\n", name, key);
  return "";
}

void SetVariable(const char *name, int key, const char *newValue) {
  for (int i = 0; i < NVariables; ++i) {
    if (strcmp(variables[i].name, name) == 0 && variables[i].key == key) {
      free(variables[i].value);
      variables[i].value = strdup(newValue);
      return;
    }
  }
  printf("Variable with name %s and key %d not found.\n", name, key);
}
void remove_double_quotes(char *string) {
  if (logger == true) {
    printf("remove_double_quotes:%s\n", string);
  }

  bool in_escape = false;
  int i, j = 0;

  for (i = 0; string[i] != '\0'; i++) {
    if (in_escape) {
      // Copy the current character and reset in_escape
      string[j++] = string[i];
      in_escape = false;
    } else if (string[i] == '\\') {
      // If a backslash is found, set in_escape to true and copy it
      in_escape = true;
      string[j++] = string[i];
    } else if (string[i] != '\"') {
      // Copy the character if it's not a double quote
      string[j++] = string[i];
    }
  }

  // Null-terminate the string
  string[j] = '\0';
}
void remove_single_quotes(char *string) {
  if (logger == true) {
    printf("remove_single_quotes:%s\n", string);
  }

  bool in_escape = false;
  int i, j = 0;

  for (i = 0; string[i] != '\0'; i++) {
    if (in_escape) {
      // Copy the current character and reset in_escape
      string[j++] = string[i];
      in_escape = false;
    } else if (string[i] == '\\') {
      // If a backslash is found, set in_escape to true and copy it
      in_escape = true;
      string[j++] = string[i];
    } else if (string[i] != '\'') {
      // Copy the character if it's not a double quote
      string[j++] = string[i];
    }
  }

  // Null-terminate the string
  string[j] = '\0';
}

void syntax(char *string);

int text_type(char *string) {
  if (logger == true) {
    printf("text_type:%s\n", string);
  }
  if (strcmp(string, "") == 0) {
    return 0;
  }
  trim(string);
  remove_newline(string);
  if (check_brackets(string) == 0) {

    strcpy(string, brackets_r(string));
    return text_type(string);
  }

  if (!isnan(te_interp(string, 0))) {
    sprintf(string, "%g", te_interp(string, 0));
    return 1;
  } else if (singlequote(string) == 0 && doublequote(string) == 0) {
  } else if (singlequote(string) == 0) {

    return 1;
  } else if (doublequote(string) == 0) {

    return 1;

  } else {
    return 0;
  }

  return 0;
}
int syntax_processor(char *string, char *word) {
  if (logger == true) {
    printf("syntax_processor:%s\n", string);
  }
  int word_length = strlen(word);
  bool in_escape = false;
  bool in_double_quotes = false;
  bool in_single_quotes = false;
  int open_brackets = 0;
  int close_brackets = 0;
  if (strncmp(string, word, word_length) != 0 ||
      (string[word_length] != ' ' && string[word_length] != '\t' &&
       string[word_length] != '(' && string[word_length] != '\0')) {
    return 0; // لا تطابق
  }
  if (strcmp(string, word) == 0) {

    return 2; // "print" is not at the beginning
  }

  int i = word_length; // Start checking after "print"

  // Skip any spaces after "print"
  while (string[i] == ' ' || string[i] == '\t' || string[i] == '\n') {
    i++;
  }

  // Check if there is an opening bracket or end of string
  if (string[i] == '(' || string[i] == '\0') {
    // Traverse the string after "print"
    for (; string[i] != '\0'; i++) {
      if (string[i] == '(') {
        open_brackets++;
      } else if (string[i] == ')') {
        close_brackets++;
      } else if (string[i] == '\\') {
        in_escape = !in_escape;
      } else {
        in_escape = false;
      }

      if (!in_escape && string[i] == '\"') {
        if (!in_single_quotes) {
          in_double_quotes = !in_double_quotes;
        }
      }

      if (!in_escape && string[i] == '\'') {
        if (!in_double_quotes) {
          in_single_quotes = !in_single_quotes;
        }
      }

      // Check if parentheses are balanced
      if (open_brackets > 0 && open_brackets == close_brackets) {
        if (string[i + 1] == '\0') {
          return 2; // Word "print" matched and parentheses balanced
        } else {
          return 0; // Word "print" matched but parentheses unbalanced
        }
      }
    }
  }
  return 0; // No match or incorrect format
}
void remove_newline(char *str) {
  if (logger == true) {
    printf("remove_newline:%s\n", str);
  }
  int len = strlen(str);
  if (len > 0 && str[len - 1] == '\n') {
    str[len - 1] = '\0';
  }
}
int text__type(char *string) {
  if (logger == true) {
    printf("text__type:%s\n", string);
  }
  if (check_brackets(string) == 0) {

    strcpy(string, brackets_r(string));
    return text__type(string);
  }
  if (!isnan(te_interp(string, 0))) {
    sprintf(string, "%g", te_interp(string, 0));
    return 1;
  } else if (singlequote(string) == 0 && doublequote(string) == 0) {
    return 2;
  } else if (singlequote(string) == 0) {
    return 2;
  } else if (doublequote(string) == 0) {
    return 2;
  } else {
    return 3;
  }
  return 3;
}

void plus_error(char *string) {
  if (logger == true) {
    printf("plus_error:%s\n", string);
  }
  int syntax_ = 0;
  if (check_plus(string) && isnan(te_interp(string, 0))) {
    for (int i = 0; i <= plus_count(string); i++) {

      if (text__type(plus(string, i)) == 1) {
        if (syntax_ == 2) {
          printf("%d %s: unsupported operand type(s) for +: 'int' and "
                 "'string'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else if (syntax_ == 3) {
          printf("%d %s: unsupported operand type(s) for +: 'int' and "
                 "'syntax_'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else {
          syntax_ = 1;
        }
      } else if (text__type(plus(string, i)) == 2) {
        if (syntax_ == 1) {
          printf("%d %s: unsupported operand type(s) for +: 'string' and "
                 "'int'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else if (syntax_ == 3) {
          printf("%d %s: unsupported operand type(s) for +: 'string' and "
                 "'syntax_'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else {
          syntax_ = 2;
        }
      } else {
        if (syntax_ == 2) {
          printf("%d %s: unsupported operand type(s) for +: 'syntax_' and "
                 "'string'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else if (syntax_ == 1) {
          printf("%d %s: unsupported operand type(s) for +: 'syntax_' and "
                 "'int'\n",
                 line, string);
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        } else {
          syntax_ = 3;
        }
      }
    }
  }
  return;
}
int syntax_type(char *string) {
  if (logger == true) {
    printf("syntax_type:%s\n", string);
  }
  trim(string);
  remove_newline(string);
  if (only_whitespace(string) == 1) {
    return 0;
  }

  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return 0;
  }

  if (syntax_processor(string, "print") == 2) {
    return 1;
  } else if (syntax_processor(string, "atoi") == 2) {
    return 2;
  } else {
    if (!check_brackets(string)) {
      return 1;
    } else if (check_plus(string) && check_comma(string)) {
      for (int i = 0; i <= comma_count(string); i++) {
        if (text_type(comma(string, i)) != 0) {

        } else if (syntax_type(comma(string, i)) == 1) {

          return 1;
        } else if (syntax_type(comma(string, i)) == 2) {

          return 2;
        }
      }

    } else if (check_plus(string)) {
      plus_error(string);
      for (int i = 0; i <= plus_count(string); i++) {
        if (text_type(plus(string, i)) == 0) {

          if (syntax_type(plus(string, i)) == 1) {
            return 1;

          } else if (syntax_type(plus(string, i)) == 2) {
            return 2;
          }
        }
      }
    } else if (check_comma(string)) {
      for (int i = 0; i <= comma_count(string); i++) {
        if (text_type(comma(string, i)) == 0) {

          if (syntax_type(comma(string, i)) == 1) {
            return 1;

          } else if (syntax_type(comma(string, i)) == 2) {
            return 2;
          }
        }
      }
    } else {
      printf("%d Error : %s\n", line, string);
      if (input_type == 1) {
        exit(1);
      } else {
        exit_proc = 1;
      }
    }
  }
  return 0; // يجب إضافة هذا السطر لتفادي الأخطاء
}
void type_dec(char *string, bool intger, bool str, bool syntax_) {
  if (logger == true) {
    printf("type_dec:%s\n", string);
  }
  if (text__type(string) == 1 && intger == false) {
    printf("error : %d %s , int not supported\n", line, string);
    if (input_type == 1) {
      exit(1);
    } else {
      exit_proc = 1;
    }
  } else if (text__type(string) == 2 && str == false) {
    printf("error : %d %s , string not supported\n", line, string);
    if (input_type == 1) {
      exit(1);
    } else {
      exit_proc = 1;
    }
  } else if (text__type(string) == 3 && syntax_ == false) {
    printf("error : %d %s , syntax not supported\n", line, string);
    if (input_type == 1) {
      exit(1);
    } else {
      exit_proc = 1;
    }
  }
}
char *plus_processor1(char *string) {
  if (logger == true) {
    printf("plus_processor:%s\n", string);
  }
  char *result = NULL;
  size_t result_len = 0;
  char *processed_text = "";
  for (int i = 0; i <= plus_count(string); i++) {
    if (text_type(plus(string, i)) != 0) {
      processed_text = plus(string, i);
    } else if (syntax_type(plus(string, i)) == 2) {
      processed_text = syntax_int(plus(string, i));
    }

    // حساب الطول الجديد للنص الديناميكي
    size_t processed_len = strlen(processed_text);
    size_t new_len = result_len + processed_len + 1;

    // إعادة تخصيص الذاكرة
    char *temp = realloc(result, new_len);
    if (temp == NULL) {
      fprintf(stderr, "Failed to reallocate memory\n");
      free(result);

      exit(1);
    }
    result = temp;
    // نسخ النص المعالج إلى المتغير الديناميكي
    strcpy(result + result_len, processed_text);
    result_len += processed_len;
  }

  // إرجاع النتيجة النهائية
  return result;
}
char *syntax_int(char *string) {
  if (logger == true) {
    printf("syntax_int:%s\n", string);
  }
  trim(string);
  remove_newline(string);
  if (only_whitespace(string) == 1) {
    return "";
  }

  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return "";
  }
  if (syntax_processor(string, "atoi") == 2) {

    if (check_brackets_1(string)) {
      return "";
    }
    string = brackets_r(string);
    if (string[0] == '\0') {
      return "";
    }
    if (check_comma(string)) {
      printf("error %d : connect to use , >> %s \n", line, string);
      if (input_type == 1) {
        exit(1);
      } else {
        exit_proc = 1;
        return "";
      }
    }
    if (only_whitespace(string) == 1) {
      return "";
    }
    while (check_brackets(string) == 0) {
      if (check_brackets(string)) {
        break;
      }
      if (only_whitespace(string) == 1 || string[0] == '\0') {
        break;
      }
      string = brackets_r(string);
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      return "";
    }

    if (check_plus(string) && isnan(te_interp(string, 0))) {

      string = plus_fixer(string);
      if (only_whitespace(string) == 1 || string[0] == '\0') {
        return "";
      }
      plus_error(string);
      for (int i = 0; i <= plus_count(string); i++) {
        if (text_type(plus(string, i)) != 0 ||
            syntax_type(plus(string, i)) == 2) {
          type_dec(plus_processor1(string), false, true, false);
          return plus_processor1(string);

          break;
        } else if (syntax_type(plus(string, i)) == 1) {
          syntax(plus(string, i));
        }
      }
    } else if (text_type(string) != 0) {

      return string;

    } else {
      if (syntax_type(string) == 0) {
        return "";
      } else if (syntax_type(string) == 1) {
        syntax(string);
      } else if (syntax_type(string) == 2) {
        return text_processor(syntax_int(string));
      }
    }
  } else {
    printf("Error : %d %s\n", line, string);
    if (input_type == 1) {
      exit(1);
    } else {
      exit_proc = 1;
      return "";
    }
  }
  return "";
}
char *plus_fixer(char *string) {
  if (logger == true) {
    printf("plus_fixer: %s\n", string);
  }

  size_t buffer_size = strlen(string) * 2; // حجم مبدئي أكبر
  char *result = malloc(buffer_size + 1);
  if (result == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    exit(1);
  }
  result[0] = '\0';

  int plus_total_count = plus_count(string);
  size_t result_len = 0;

  for (int i = 0; i <= plus_total_count; i++) {
    char *tmp = strdup(plus(string, i));
    if (tmp == NULL) {
      free(result);
      return NULL;
    }

    if (check_brackets(tmp) == 0) {
      tmp = plus_fixer(brackets_r(tmp));
    }
    if (check_comma(tmp)) {
      printf("error tuple %s", tmp);
      exit(1);
    }
    if (only_whitespace(tmp) == 1 || tmp[0] == '\0') {
      continue;
    }

    if (check_plus(tmp) && isnan(te_interp(tmp, 0))) {
      size_t test_size = strlen(tmp) + strlen(string) + 2;
      char *test = malloc(test_size);
      if (test == NULL) {
        free(tmp);
        free(result);
        return NULL;
      }
      test[0] = '\0';

      for (int j = 0; j <= plus_total_count; j++) {
        if (i == j) {
          strcat(test, tmp);

        } else if (j == plus_count(string)) {

          strcat(test, plus(string, j));

        } else {

          strcat(test, plus(string, j));
          strcat(test, "+");
        }
      }

      free(tmp);
      free(result);
      return test;
    }

    char *processed_text = NULL;
    if (text_type(tmp) != 0) {
      processed_text = tmp;
    } else if (syntax_type(tmp) == 1 || syntax_type(tmp) == 3) {
      processed_text = tmp;
    } else if (syntax_type(tmp) == 2 || syntax_type(tmp) == 4) {
      processed_text = syntax_int(tmp);
    }

    if (processed_text == NULL) {
      free(tmp);
      continue;
    }

    size_t new_len =
        result_len + strlen(processed_text) + (i < plus_total_count ? 1 : 0);
    if (new_len > buffer_size) {
      buffer_size = new_len * 2;
      char *temp = realloc(result, buffer_size + 1);
      if (temp == NULL) {
        free(tmp);
        free(result);
        fprintf(stderr, "Failed to reallocate memory\n");
        exit(1);
      }
      result = temp;
    }

    strcat(result, processed_text);
    if (i < plus_total_count) {
      strcat(result, "+");
    }

    result_len = new_len;

    free(tmp);
  }

  return result;
}

char *plus_processor(char *string) {
  if (logger == true) {
    printf("plus_processor:%s\n", string);
  }
  char *result = NULL;
  size_t result_len = 0;
  char *processed_text = "";
  for (int i = 0; i <= plus_count(string); i++) {
    if (text_type(plus(string, i)) != 0) {
      processed_text = text_processor(plus(string, i));
    } else if (syntax_type(plus(string, i)) == 2) {
      processed_text = text_processor(syntax_int(plus(string, i)));
    }

    // حساب الطول الجديد للنص الديناميكي
    size_t processed_len = strlen(processed_text);
    size_t new_len = result_len + processed_len + 1;

    // إعادة تخصيص الذاكرة
    char *temp = realloc(result, new_len);
    if (temp == NULL) {
      fprintf(stderr, "Failed to reallocate memory\n");
      free(result);

      exit(1);
    }
    result = temp;
    // نسخ النص المعالج إلى المتغير الديناميكي
    strcpy(result + result_len, processed_text);
    result_len += processed_len;
  }

  // إرجاع النتيجة النهائية
  return result;
}
char *comma_processor(char *string) {
  size_t buffer_size = strlen(string) * 2;
  char *result = malloc(buffer_size + 1);
  if (result == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");

    exit(1);
  }
  result[0] = '\0'; // تأكد من أن النتيجة تبدأ بسلسلة فارغة

  size_t result_len = 0;

  for (int i = 0; i <= comma_count(string); i++) {
    char *processed_text = ""; // أعد تهيئة النص المعالج لكل عملية

    if (text_type(comma(string, i)) != 0) {
      processed_text = text_processor(comma(string, i));
    } else if (syntax_type(comma(string, i)) == 2) {
      processed_text = text_processor(syntax_int(comma(string, i)));
    }

    // تحقق من أن هناك مساحة كافية لإضافة النص المعالج مع الفراغ
    size_t processed_len = strlen(processed_text) + 1; // +1 للفراغ
    size_t new_len = result_len + processed_len;

    if (new_len > buffer_size) {
      buffer_size =
          new_len * 2; // زد حجم المخزن بشكل مضاعف لتجنب إعادة التخصيص المتكرر
      char *temp = realloc(result, buffer_size);
      if (temp == NULL) {
        fprintf(stderr, "Failed to reallocate memory\n");
        free(result);
        exit(1);
      }
      result = temp;
    }

    strcat(result + result_len, processed_text); // إضافة النص المعالج
    result_len += strlen(processed_text);

    strcat(result + result_len, " "); // إضافة الفراغ بعد النص
    result_len += 1;
  }

  return result;
}
void syntax(char *string) {
  remove_newline(string);
  if (logger == true) {
    printf("syntax:%s\n", string);
  }
  if (only_whitespace(string) == 1) {
    return;
  }

  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return;
  }
  if (syntax_processor(string, "print") == 2) {
    if (check_brackets_1(string)) {

      return;
    }
    string = brackets_r(string);
    if (string[0] == '\0') {
      printf("\n");
      return;
    }

    if (only_whitespace(string) == 1) {
      return;
    }
    while (check_brackets(string) == 0) {
      if (check_brackets(string)) {
        break;
      }
      if (only_whitespace(string) == 1 || string[0] == '\0') {
        break;
      }
      string = brackets_r(string);
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      return;
    }

    if (check_plus(string) && isnan(te_interp(string, 0)) &&
        check_comma(string)) {
      //      exit(1);
      char *tmp_ = "";
      for (int i = 0; i <= comma_count(string); i++) {
        tmp_ = comma(string, i);
        while (check_brackets(tmp_) == 0) {
          if (check_brackets(tmp_)) {
            break;
          }
          if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
            break;
          }
          tmp_ = brackets_r(tmp_);
        }
        if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
          return;
        }
        if (check_plus(tmp_) && isnan(te_interp(tmp_, 0)) &&
            !check_comma(tmp_)) {
          tmp_ = plus_fixer(tmp_);

          if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
            return;
          }
          plus_error(tmp_);

          if (check_plus(tmp_) && isnan(te_interp(tmp_, 0))) {
            for (int j = 0; j <= plus_count(tmp_); j++) {
              if (check_comma(plus(tmp_, j))) {
                printf("error tuple %s\n", plus(tmp_, j));
                exit(1);
              }

              if (text_type(plus(tmp_, j)) != 0 ||
                  syntax_type(plus(tmp_, j)) == 2) {
                printf("%s", plus_processor(tmp_));

                break;
              } else if (syntax_type(plus(tmp_, j)) == 1) {
                syntax(plus(tmp_, j));
              }
            }
          } else {

            if (text_type(tmp_) != 0) {

              printf("%s", text_processor(tmp_));

            } else if (syntax_type(tmp_) == 1) {
              syntax(tmp_);
            } else if (syntax_type(tmp_) == 2) {

              printf("%s", text_processor(syntax_int(tmp_)));
            }
          }

        } else {
          if (text_type(tmp_) != 0) {
            printf("%s ", text_processor(tmp_));

          } else if (syntax_type(tmp_) == 1) {
            syntax(tmp_);
          } else if (syntax_type(tmp_) == 2) {

            printf("%s ", text_processor(syntax_int(tmp_)));
          }
        }
      }

      printf("\n");

      free(tmp_);
    } else if (check_plus(string) && isnan(te_interp(string, 0))) {

      string = plus_fixer(string);
      if (only_whitespace(string) == 1 || string[0] == '\0') {
        return;
      }
      plus_error(string);
      for (int i = 0; i <= plus_count(string); i++) {
        if (text_type(plus(string, i)) != 0 ||
            syntax_type(plus(string, i)) == 2) {
          printf("%s\n", plus_processor(string));

          break;
        } else if (syntax_type(plus(string, i)) == 1) {
          syntax(plus(string, i));
        }
      }

    } else if (check_comma(string)) {
      for (int i = 0; i <= comma_count(string); i++) {
        if (text_type(comma(string, i)) != 0) {

          printf("%s ", text_processor(comma(string, i)));

        } else if (syntax_type(comma(string, i)) == 1) {
          syntax(comma(string, i));
        } else if (syntax_type(comma(string, i)) == 2) {

          printf("%s ", text_processor(syntax_int(comma(string, i))));
        }
      }

      printf("\n");

    } else if (text_type(string) != 0) {

      printf("%s\n", text_processor(string));

    } else {
      if (syntax_type(string) == 0) {
        return;
      } else if (syntax_type(string) == 1) {
        syntax(string);
      } else if (syntax_type(string) == 2) {
        printf("%s\n", text_processor(syntax_int(string)));
      }
    }
  } else {
    printf("Error : %d %s\n", line, string);
    if (input_type == 1) {
      exit(1);
    } else {
      exit_proc = 1;
    }
  }
}

int only_whitespace(const char *string) {
  if (logger == true) {
    printf("only_whitespace:%s\n", string);
  }
  while (*string != '\0') {
    if (!isspace(*string)) {
      return 0;
    }
    string++;
  }
  return 1;
}
int singlequote(char *string) {
  if (logger == true) {
    printf("singlequote:%s\n", string);
  }
  bool in_escape = false;
  bool quotes = false;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && string[i] == '\'') {
      quotes = !quotes;
    } else if (!in_escape && !quotes && string[i] != ' ' && string[i] != '\n' &&
               string[i] != '\t' && string[i] != '\r') {
      return 1;
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }
  }
  return 0;
}
int doublequote(char *string) {
  if (logger == true) {
    printf("doublequote:%s\n", string);
  }
  bool in_escape = false;
  bool quotes = false;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!in_escape && string[i] == '\"') {
      quotes = !quotes;
    } else if (!in_escape && !quotes && string[i] != ' ' && string[i] != '\n' &&
               string[i] != '\t' && string[i] != '\r') {
      return 1;
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }
  }
  return 0;
}
char *text_processor(char *string) {

  if (logger == true) {
    printf("text_processor:%s\n", string);
  }
  if (strcmp(string, "") == 0) {
    return "";
  }
  trim(string);
  while (check_brackets(string) == 0) {
    if (check_brackets(string)) {
      break;
    }
    if (only_whitespace(string) == 1 || string[0] == '\0') {
      break;
    }
    string = brackets_r(string);
  }
  if (only_whitespace(string) == 1 || string[0] == '\0') {
    return "";
  }

  if (!isnan(te_interp(string, 0))) {
    sprintf(string, "%g", te_interp(string, 0));
    return string;
  } else if (singlequote(string) == 0 && doublequote(string) == 0) {
  } else if (singlequote(string) == 0) {
    remove_single_quotes(string);
    return string;
  } else if (doublequote(string) == 0) {
    remove_double_quotes(string);

    return g_strcompress(string);
  } else {
    return NULL;
  }
  return NULL;
}

void comments(char *string) {
  if (logger == true) {
    printf("comments:%s\n", string);
  }
  bool in_escape = false;
  int j = 0;
  bool doublequotes = false, singlequotes = false;

  for (int i = 0; string[i] != '\0'; i++) {
    if (!singlequotes && !in_escape && string[i] == '\"') {
      if (!doublequotes) {
        doublequotes = true;
      } else if (string[i - 1] != '\\') {
        doublequotes = false;
      }
    }
    if (!doublequotes && !in_escape && string[i] == '\'') {
      if (!singlequotes) {
        singlequotes = true;
      } else if (string[i - 1] != '\\') {
        singlequotes = false;
      }
    }
    if (!in_escape && !doublequotes && !singlequotes) {
      if (string[i] == '#' || (string[i] == '/' && string[i + 1] == '/')) {
        string[i] = '\0';
        break;
      }
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }
    string[j++] = string[i];
  }
  string[j] = '\0';
}

void multi_comments(char *string) {
  if (logger == true) {
    printf("multi_comments:%s\n", string);
  }
  bool in_escape = false;
  bool multi_comment = false;
  int j = 0;
  bool doublequotes = false, singlequotes = false;
  for (int i = 0; string[i] != '\0'; i++) {
    if (!multi_comment && !singlequotes && !in_escape && string[i] == '\"') {
      if (!doublequotes) {
        doublequotes = true;
      } else {
        doublequotes = false;
      }
    }
    if (!multi_comment && !doublequotes && !in_escape && string[i] == '\'') {
      if (!singlequotes) {
        singlequotes = true;
      } else {
        singlequotes = false;
      }
    }
    if (!multi_comment && !in_escape && !doublequotes && !singlequotes &&
        string[i] == '/' && string[i + 1] == '*') {
      multi_comment = true;
      i += 1;
    } else if (multi_comment && string[i] == '*' && string[i + 1] == '/') {
      multi_comment = false;
      i += 1;
    } else if (!multi_comment) {
      string[j++] = string[i];
    }
    if (string[i] == '\\') {
      in_escape = !in_escape;
    } else {
      in_escape = false;
    }
  }
  string[j] = '\0';
}

char *int_to_stringing(int number) {
  if (logger == true) {
    printf("int_to_stringing:%d\n", number);
  }
  int digits = snprintf(NULL, 0, "%d", number);
  char *string = (char *)malloc(digits + 1);
  if (string == NULL) {
    return NULL;
  }
  snprintf(string, digits + 1, "%d", number);

  return string;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    /* fprintf(stderr,
             "\033[31mError \033[0m\033[1m: invalid option -- "
             "'%s'\nTry 'Null+ --help' for more information.\n",
             argv[1]);
     return 1;*/
    printf("Null+\n");
    input_type = 0;
  }

  FILE *file = NULL;
  if (argc >= 2) {
    file = fopen(argv[1], "r");
    input_type = 1;
    if (file == NULL) {
      fprintf(stderr,
              "\033[31mError \033[0m\033[1m: no such file or "
              "directory : '%s' :(\n",
              argv[1]);
      return 2;
    }
  }
  char *string = NULL;
  char *code = NULL;
  code = malloc(1);
  if (code == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    fclose(file);
    return 3;
  }
  code[0] = '\0';
  size_t len = 0;
  ssize_t read;
  size_t code_size = 0;
  bool multi_comment = false;
  int multi_comment_line = 0;
  int multi_comment_col = 0;
  // Quotes
  bool in_escape = false;
  int quotestype = 0;
  bool doublequotes = false, singlequotes = false;
  int doublequotes_col = 0, singlequotes_col = 0;
  int doublequotes_line = 0, singlequotes_line = 0;
  //
  int open_brackets = 0;
  int close_brackets = 0;
  int brackets = 0;
  int open_braces = 0;
  int close_braces = 0;
  int braces = 0;
  int open_curly = 0;
  int close_curly = 0;
  int curly = 0;

  int brackets_type = 0;
  while (1) {

    if (exit_proc == 1) {
      multi_comment = false;
      multi_comment_line = 0;
      multi_comment_col = 0;
      in_escape = false;
      quotestype = 0;
      doublequotes = false, singlequotes = false;
      doublequotes_col = 0, singlequotes_col = 0;
      doublequotes_line = 0, singlequotes_line = 0;
      open_brackets = 0;
      close_brackets = 0;
      brackets = 0;
      open_braces = 0;
      close_braces = 0;
      braces = 0;
      open_curly = 0;
      close_curly = 0;
      curly = 0;
      brackets_type = 0;
      line = 0;
      exit_proc = 0;
    }
    if (input_type == 0) {
      if (!brackets && !curly && !braces && !multi_comment && !doublequotes &&
          !singlequotes) {
        printf(">>> ");
      } else {
        printf("... ");
      }
    }
    signal(SIGINT, handle_sigint);
    if (file == NULL) {
      read = getline(&string, &len, stdin);
    }
    if (file != NULL) {
      read = getline(&string, &len, file);
    }

    if (read == -1) {
      break;
    }
    line++;
    if (string[0] == '\n') {
      continue;
    }
    comments(string);
    if (only_whitespace(string) == 1) {
      continue;
    }
    size_t string_len = strlen(string);

    AddVariable("string", line, string);
    for (int i = 0; string[i] != '\0'; i++) {
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == '(') {
        brackets_type = 1;
        open_brackets++;
        brackets++;
        AddVariable("open_brackets_line", brackets, "");
        SetVariable("open_brackets_line", brackets, int_to_stringing(line));
        AddVariable("open_brackets_col", brackets, "");
        SetVariable("open_brackets_col", brackets, int_to_stringing(i));
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == '{') {
        open_curly++;
        brackets_type = 2;
        curly++;
        AddVariable("open_curly_line", curly, "");
        SetVariable("open_curly_line", curly, int_to_stringing(line));
        AddVariable("open_curly_col", curly, "");
        SetVariable("open_curly_col", curly, int_to_stringing(i));
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == '[') {
        brackets_type = 3;
        open_braces++;
        braces++;
        AddVariable("open_braces_line", braces, "");
        SetVariable("open_braces_line", braces, int_to_stringing(line));
        AddVariable("open_braces_col", braces, "");
        SetVariable("open_braces_col", braces, int_to_stringing(i));
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == ')') {
        close_brackets++;
        brackets--;
        if (open_brackets < close_brackets) {
          printf("\033[0m\033[1mFile : %s, Line : %d"
                 "\033[32m\n\033[0m%s\033[1m\033[32m",
                 argv[1], line, GetVariable("string", line));
          for (int ii = 0; ii < i; ii++) {
            printf(" ");
          }
          printf("^\n\033[0m\033[1m\033[31m\033[1mError\033[0m\033[1m : "
                 "unmatched ')'\n");
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        }
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == ']') {
        close_braces++;
        braces--;
        if (open_braces < close_braces) {
          printf("\033[0m\033[1mFile : %s, Line : %d"
                 "\033[32m\n\033[0m%s\033[1m\033[32m",
                 argv[1], line, GetVariable("string", line));
          for (int ii = 0; ii < i; ii++) {
            printf(" ");
          }
          printf("^\n\033[0m\033[1m\033[31m\033[1mError\033[0m\033[1m : "
                 "unmatched ']'\n");
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        }
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == '}') {
        close_curly++;
        curly--;
        if (open_curly < close_curly) {
          printf("\033[0m\033[1mFile : %s, Line : %d"
                 "\033[32m\n\033[0m%s\033[1m\033[32m",
                 argv[1], line, GetVariable("string", line));
          for (int ii = 0; ii < i; ii++) {
            printf(" ");
          }
          printf("^\n\033[0m\033[1m\033[31m\033[1mError\033[0m\033[1m : "
                 "unmatched '}'\n");
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        }
      }
      if (!multi_comment && !doublequotes && !singlequotes && !in_escape &&
          string[i] == '/' && string[i + 1] == '*') {
        multi_comment_line = line;
        multi_comment_col = i;
        multi_comment = true;
      }
      if (!doublequotes && !singlequotes && !in_escape && string[i] == '*' &&
          string[i + 1] == '/') {
        if (multi_comment) {
          multi_comment = false;
        } else if (!multi_comment) {
          printf("\033[0m\033[1mFile : %s, Line : %d"
                 "\033[32m\n\033[0m%s\033[1m\033[32m",
                 argv[1], line, GetVariable("string", line));
          for (int ii = 0; ii < i; ii++) {
            printf(" ");
          }
          printf("^^\n\033[0m\033[1m\033[31m\033[1mError\033[0m\033[1m : "
                 "unterminated comment\n");
          if (input_type == 1) {
            exit(1);
          } else {
            exit_proc = 1;
          }
        }
      }
      if (!multi_comment && !singlequotes && !in_escape && string[i] == '\"') {
        if (!doublequotes) {
          doublequotes = true;
          doublequotes_line = line;
          quotestype = 1;
        } else {
          doublequotes = false;
        }
        doublequotes_col = i;
      }
      if (!multi_comment && !doublequotes && !in_escape && string[i] == '\'') {
        if (!singlequotes) {
          singlequotes = true;
          singlequotes_line = line;
          quotestype = 2;
        } else {
          singlequotes = false;
        }
        singlequotes_col = i;
      }
      if (string[i] == '\\') {
        in_escape = !in_escape;
      } else {
        in_escape = false;
      }
    }

    if (!brackets && !curly && !braces && !multi_comment && !doublequotes &&
        !singlequotes) {
      if (only_whitespace(code) == 1) {

        multi_comments(string);
        if (only_whitespace(string) == 1) {
          continue;
        }
        while (check_brackets(string) == 0) {
          if (check_brackets(string)) {
            break;
          }
          if (only_whitespace(string) == 1 || string[0] == '\0') {
            break;
          }
          string = brackets_r(string);
        }

        if (only_whitespace(string) == 1 || string[0] == '\0') {
          continue;
        }
        if (check_plus(string) && isnan(te_interp(string, 0)) &&
            check_comma(string)) {
          //      exit(1);
          char *tmp_ = "";
          for (int i = 0; i <= comma_count(string); i++) {
            tmp_ = comma(string, i);

            while (check_brackets(tmp_) == 0) {
              if (check_brackets(tmp_)) {
                break;
              }
              if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
                break;
              }
              tmp_ = brackets_r(tmp_);
            }

            if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
              continue;
            }
            if (check_plus(tmp_) && isnan(te_interp(tmp_, 0)) &&
                !check_comma(tmp_)) {
              tmp_ = plus_fixer(tmp_);

              if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
                continue;
              }
              plus_error(tmp_);

              if (check_plus(tmp_) && isnan(te_interp(tmp_, 0))) {
                for (int j = 0; j <= plus_count(tmp_); j++) {
                  if (check_comma(plus(tmp_, j))) {
                    printf("error tuple %s\n", plus(tmp_, j));
                    exit(1);
                  }

                  if (text_type(plus(tmp_, j)) != 0 ||
                      syntax_type(plus(tmp_, j)) == 2) {
                    if (input_type == 0) {
                      printf("%s", plus_processor(tmp_));
                    } else {
                      plus_processor(tmp_);
                    }
                    break;
                  } else if (syntax_type(plus(tmp_, j)) == 1) {
                    syntax(plus(tmp_, j));
                  }
                }
              } else {

                if (text_type(tmp_) != 0) {
                  if (input_type == 0) {
                    printf("%s", text_processor(tmp_));
                  } else {
                    text_processor(tmp_);
                  }
                } else if (syntax_type(tmp_) == 1) {
                  syntax(tmp_);
                } else if (syntax_type(tmp_) == 2) {
                  if (input_type == 0) {
                    printf("%s", text_processor(syntax_int(tmp_)));
                  } else {
                    text_processor(syntax_int(tmp_));
                  }
                }
              }

            } else {
              if (text_type(tmp_) != 0) {
                if (input_type == 0) {
                  printf("%s ", text_processor(tmp_));
                } else {
                  text_processor(tmp_);
                }
              } else if (syntax_type(tmp_) == 1) {
                syntax(tmp_);
              } else if (syntax_type(tmp_) == 2) {
                if (input_type == 0) {
                  printf("%s ", text_processor(syntax_int(tmp_)));
                } else {
                  text_processor(syntax_int(tmp_));
                }
              }
            }
          }
          if (input_type == 0) {
            printf("\n");
          }
          free(tmp_);
        } else if (check_plus(string) && isnan(te_interp(string, 0))) {

          string = plus_fixer(string);
          if (only_whitespace(string) == 1 || string[0] == '\0') {
            continue;
          }
          plus_error(string);
          for (int i = 0; i <= plus_count(string); i++) {
            if (text_type(plus(string, i)) != 0 ||
                syntax_type(plus(string, i)) == 2) {
              if (input_type == 0) {
                printf("%s", plus_processor(string));
                printf("\n");
              } else {
                plus_processor(string);
              }
              break;
            } else if (syntax_type(plus(string, i)) == 1) {
              syntax(plus(string, i));
            }
          }

        } else if (check_comma(string)) {
          for (int i = 0; i <= comma_count(string); i++) {
            if (text_type(comma(string, i)) != 0) {

              if (input_type == 0) {
                printf("%s ", text_processor(comma(string, i)));
              } else {
                text_processor(comma(string, i));
              }

            } else if (syntax_type(comma(string, i)) == 1) {
              syntax(comma(string, i));
            } else if (syntax_type(comma(string, i)) == 2) {

              if (input_type == 0) {
                printf("%s ", text_processor(syntax_int(comma(string, i))));
              } else {
                text_processor(syntax_int(comma(string, i)));
              }
            }
          }
          if (input_type == 0) {
            printf("\n");
          }
        } else if (text_type(string) != 0) {

          if (input_type == 0) {
            printf("%s\n", text_processor(string));
          } else {
            text_processor(string);
          }
        } else {
          if (syntax_type(string) == 0) {
            continue;
          } else if (syntax_type(string) == 1) {
            syntax(string);
          } else if (syntax_type(string) == 2) {

            if (input_type == 0) {
              printf("%s\n", text_processor(syntax_int(string)));
            } else {
              text_processor(syntax_int(string));
            }
          }
        }

      } else {

        char *new_code = realloc(code, code_size + string_len + 1);
        if (new_code == NULL) {
          fprintf(stderr, "Memory allocation failed.\n");
          free(code);
          fclose(file);
          free(string);
          return 3;
        }
        code = new_code;
        strcat(code, string);
        code_size += string_len;
        removeNewlines(code);
        multi_comments(code);
        remove_newline(code);

        if (only_whitespace(code) == 1) {
          continue;
        }
        while (check_brackets(code) == 0) {
          if (check_brackets(code)) {
            break;
          }
          if (only_whitespace(code) == 1 || code[0] == '\0') {
            break;
          }
          code = brackets_r(code);
        }
        if (only_whitespace(code) == 1 || code[0] == '\0') {
          continue;
        }
        if (check_plus(code) && isnan(te_interp(code, 0)) &&
            check_comma(code)) {
          //      exit(1);
          char *tmp_ = "";
          for (int i = 0; i <= comma_count(code); i++) {
            tmp_ = comma(code, i);

            while (check_brackets(tmp_) == 0) {
              if (check_brackets(tmp_)) {
                break;
              }
              if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
                break;
              }
              tmp_ = brackets_r(tmp_);
            }
            if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
              continue;
            }
            if (check_plus(tmp_) && isnan(te_interp(tmp_, 0)) &&
                !check_comma(tmp_)) {
              tmp_ = plus_fixer(tmp_);

              if (only_whitespace(tmp_) == 1 || tmp_[0] == '\0') {
                continue;
              }
              plus_error(tmp_);

              if (check_plus(tmp_) && isnan(te_interp(tmp_, 0))) {
                for (int j = 0; j <= plus_count(tmp_); j++) {
                  if (check_comma(plus(tmp_, j))) {
                    printf("error tuple %s\n", plus(tmp_, j));
                    exit(1);
                  }

                  if (text_type(plus(tmp_, j)) != 0 ||
                      syntax_type(plus(tmp_, j)) == 2) {
                    if (input_type == 0) {
                      printf("%s", plus_processor(tmp_));
                    } else {
                      plus_processor(tmp_);
                    }
                    break;
                  } else if (syntax_type(plus(tmp_, j)) == 1) {
                    syntax(plus(tmp_, j));
                  }
                }
              } else {

                if (text_type(tmp_) != 0) {
                  if (input_type == 0) {
                    printf("%s", text_processor(tmp_));
                  } else {
                    text_processor(tmp_);
                  }
                } else if (syntax_type(tmp_) == 1) {
                  syntax(tmp_);
                } else if (syntax_type(tmp_) == 2) {
                  if (input_type == 0) {
                    printf("%s", text_processor(syntax_int(tmp_)));
                  } else {
                    text_processor(syntax_int(tmp_));
                  }
                }
              }

            } else {
              if (text_type(tmp_) != 0) {
                if (input_type == 0) {
                  printf("%s ", text_processor(tmp_));
                } else {
                  text_processor(tmp_);
                }
              } else if (syntax_type(tmp_) == 1) {
                syntax(tmp_);
              } else if (syntax_type(tmp_) == 2) {
                if (input_type == 0) {
                  printf("%s ", text_processor(syntax_int(tmp_)));
                } else {
                  text_processor(syntax_int(tmp_));
                }
              }
            }
          }
          if (input_type == 0) {
            printf("\n");
          }
          free(tmp_);
        } else if (check_plus(code) && isnan(te_interp(code, 0))) {

          code = plus_fixer(code);
          if (only_whitespace(code) == 1 || code[0] == '\0') {
            continue;
          }
          plus_error(code);
          for (int i = 0; i <= plus_count(code); i++) {
            if (text_type(plus(code, i)) != 0 ||
                syntax_type(plus(code, i)) == 2) {
              if (input_type == 0) {
                printf("%s", plus_processor(code));
                printf("\n");
              } else {
                plus_processor(code);
              }
              break;
            } else if (syntax_type(plus(code, i)) == 1) {
              syntax(plus(code, i));
            }
          }

        } else if (check_comma(code)) {
          for (int i = 0; i <= comma_count(code); i++) {
            if (text_type(comma(code, i)) != 0) {

              if (input_type == 0) {
                printf("%s ", text_processor(comma(code, i)));
              } else {
                text_processor(comma(code, i));
              }

            } else if (syntax_type(comma(code, i)) == 1) {
              syntax(comma(code, i));
            } else if (syntax_type(comma(code, i)) == 2) {

              if (input_type == 0) {
                printf("%s ", text_processor(syntax_int(comma(code, i))));
              } else {
                text_processor(syntax_int(comma(code, i)));
              }
            }
          }
          if (input_type == 0) {
            printf("\n");
          }
        } else if (text_type(code) != 0) {

          if (input_type == 0) {
            printf("%s\n", text_processor(code));
          } else {
            text_processor(code);
          }
        } else {
          if (syntax_type(code) == 0) {
            continue;
          } else if (syntax_type(code) == 1) {
            syntax(code);
          } else if (syntax_type(code) == 2) {

            if (input_type == 0) {
              printf("%s\n", text_processor(syntax_int(code)));
            } else {
              text_processor(syntax_int(code));
            }
          }
        }
        // char *code = NULL;
        code = malloc(1);
        if (code == NULL) {
          fprintf(stderr, "Memory allocation failed.\n");
          fclose(file);
          return 3;
        }
        code_size = 0;
        new_code[0] = '\0';
        code[0] = '\0';
      }
    } else {
      char *new_code = realloc(code, code_size + string_len + 1);
      if (new_code == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(code);
        fclose(file);
        free(string);
        return 3;
      }
      code = new_code;
      strcat(code, string);
      code_size += string_len;
    }
  }

  if (brackets_type == 1) {
    if (brackets > 0) {
      printf("\033[0m\033[1mFile : %s, Line : %d"
             "\033[32m\n\033[0m%s\033[1m\033[32m",
             argv[1], atoi(GetVariable("open_brackets_line", brackets)),
             GetVariable("string",
                         atoi(GetVariable("open_brackets_line", brackets))));
      for (int i = 0; i < atoi(GetVariable("open_brackets_col", brackets));
           i++) {
        printf(" ");
      }
      printf("^\n\033[0m\033[1m\033[31m\033[1mSyntaxError\033[0m\033[1m : "
             "'(' was never closed\n");
      if (input_type == 1) {
        exit(1);
      }
    }
  } else if (brackets_type == 2) {
    if (curly > 0) {
      printf(
          "\033[0m\033[1mFile : %s, Line : %d"
          "\033[32m\n\033[0m%s\033[1m\033[32m",
          argv[1], atoi(GetVariable("open_curly_line", curly)),
          GetVariable("string", atoi(GetVariable("open_curly_line", curly))));
      for (int i = 0; i < atoi(GetVariable("open_curly_col", curly)); i++) {
        printf(" ");
      }
      printf("^\n\033[0m\033[1m\033[31m\033[1mSyntaxError\033[0m\033[1m : "
             "'{' was never closed\n");
      if (input_type == 1) {
        exit(1);
      }
    }
  } else if (brackets_type == 3) {
    if (braces > 0) {
      printf(
          "\033[0m\033[1mFile : %s, Line : %d"
          "\033[32m\n\033[0m%s\033[1m\033[32m",
          argv[1], atoi(GetVariable("open_braces_line", braces)),
          GetVariable("string", atoi(GetVariable("open_braces_line", braces))));
      for (int i = 0; i < atoi(GetVariable("open_braces_col", braces)); i++) {
        printf(" ");
      }
      printf("^\n\033[0m\033[1m\033[31m\033[1mSyntaxError\033[0m\033[1m : "
             "'[' was never closed\n");
      if (input_type == 1) {
        exit(1);
      }
    }
  }
  if (multi_comment) {
    printf("\033[0m\033[1mFile : %s, Line : %d"
           "\033[32m\n\033[0m%s\033[1m\033[32m",
           argv[1], multi_comment_line,
           GetVariable("string", multi_comment_line));
    for (int i = 0; i < multi_comment_col; i++) {
      printf(" ");
    }
    printf("^^\n\033[0m\033[1m\033[31m\033[1mError\033[0m\033[1m : "
           "unterminated comment\n");
    if (input_type == 1) {
      exit(1);
    }
  }
  if (quotestype == 1) {
    if (doublequotes) {
      printf("\033[0m\033[1mFile : %s, Line : %d"
             "\033[32m\n\033[0m%s\033[1m\033[32m",
             argv[1], doublequotes_line,
             GetVariable("string", doublequotes_line));
      for (int i = 0; i < doublequotes_col; i++) {
        printf(" ");
      }
      printf("^\n\033[0m\033[1m\033[31m\033[1mSyntaxError\033[0m\033[1m : "
             "unterminated string literal\n");
      if (input_type == 1) {
        exit(1);
      }
    }
  } else if (quotestype == 2) {
    if (singlequotes) {
      printf("\033[0m\033[1mFile : %s, Line : %d"
             "\033[32m\n\033[0m%s\033[1m\033[32m",
             argv[1], singlequotes_line,
             GetVariable("string", singlequotes_line));
      for (int i = 0; i < singlequotes_col; i++) {
        printf(" ");
      }
      printf("^\n\033[0m\033[1m\033[31m\033[1mSyntaxError\033[0m\033[1m : "
             "unterminated string literal\n");
      if (input_type == 1) {
        exit(1);
      }
    }
  }
  for (int i = 0; i < NVariables; ++i) {
    free(variables[i].name);
    free(variables[i].value);
  }
  free(variables);
  free(string);
  free(code);
  if (file != NULL) {
    fclose(file);
  }
  return 0;
}
