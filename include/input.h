// input.h
// Header file for input management. It defines the NoonInput struct, which
// holds all input-related state and command-line options. It also declares
// the portable_getline function for reading input.

#ifndef INPUT_H
#define INPUT_H

#include "config.h"
#include <stddef.h>
#include <stdio.h>
#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

/* Command history used only in REPL mode to store previous input lines */
typedef struct {
  char **items;
  size_t count;
} History;

// Struct to hold all input state and command-line options.
typedef struct {
  // Input source info
  const char *program_name;
  int is_repl;
  FILE *file;
  const char *input;
  // Command-line options
  int dump_tokens;
  int dump_ast;
  int check_syntax;
  int debug;
  // Repl history
  History history;
} NoonInput;

// Global pointer to the input state.
extern NoonInput *ni;

// initialize the global input state
void init_input(void);
// our custom getline version that works on all systems and supports REPL
// history
ssize_t portable_getline(char **lineptr, size_t *n, FILE *stream);

#endif
