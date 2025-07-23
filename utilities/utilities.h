#ifndef UTILITIES_H
#define UTILITIES_H
#include "../world_variables/world_variables.h"
#include <stddef.h>

void cleanup(int exit_code);
int is_empty(const char *str);
int number_count(int number);
void print_log(int type, const char *msg, size_t line, size_t col,
               const char *symbol_str);
void handle_sigint();
void push_bracket_stack(Bracket bracket, size_t line, size_t col);
BracketStackItem pop_bracket_stack();
#endif
