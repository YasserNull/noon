#ifndef WORLD_VARIABLES_H
#define WORLD_VARIABLES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#define INITIAL_CAPACITY 16
#define LINE_SIZE 1024

typedef enum { STATE_NORMAL, STATE_QUOTE, STATE_MULTI_COMMENT } ParserState;

typedef struct {
  char open;
  char close;
  size_t line;
  size_t col;
} Bracket;

typedef struct {
  Bracket bracket;
  size_t line;
  size_t col;
} BracketStackItem;

typedef struct {
  size_t line;
  size_t col;
  const char *msg;
  char symbol[3];
} ErrorLog;

// extern declarations
extern char **lines;
extern size_t lines_cap;
extern size_t line_count;
extern const char *input;
extern FILE *file;
extern char *code;
extern int total_errors;
extern const Bracket brackets[3];

extern BracketStackItem *bracket_stack;
extern size_t bracket_stack_size;
extern size_t bracket_stack_capacity;

extern ErrorLog *error_logs;
extern size_t error_logs_capacity;
extern size_t error_count;

#endif // WORLD_VARIABLES_H