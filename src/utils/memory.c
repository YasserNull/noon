// utils/memory.c
// This file provides safe memory management wrappers (malloc, calloc, etc.)
// that automatically handle allocation failures by exiting the program.
// It also contains the central cleanup function to free all allocated
// resources.

#include "utils/memory.h"
#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/log.h"
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else

#include <termios.h>
#include <unistd.h>
#endif

#include "input.h"

// A safe wrapper for malloc that exits on failure.
void *safe_malloc(size_t n) {
  debug_func("n: %zu", n);
  void *p = malloc(n);
  if (!p) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  return p;
}

// A safe wrapper for calloc that exits on failure.
void *safe_calloc(size_t nmemb, size_t size) {
  debug_func("nmemb:%zu, size:%zu", nmemb, size);
  void *p = calloc(nmemb, size);
  if (!p) {
    perror("calloc failed");

    exit(EXIT_FAILURE);
  }
  return p;
}

// A safe wrapper for realloc that exits on failure.
void *safe_realloc(void *ptr, size_t new_size) {
  debug_func("ptr:%p, new_size:%zu", ptr, new_size);
  void *p = realloc(ptr, new_size);
  if (!p) {
    perror("realloc failed");

    exit(EXIT_FAILURE);
  }
  return p;
}

// A safe wrapper for strdup that uses safe_malloc.
char *safe_strdup(const char *s) {
  debug_func("s: %s", s);
  if (!s)
    return NULL;
  size_t n = strlen(s);
  char *p = safe_malloc(n + 1);
  memcpy(p, s, n);
  p[n] = '\0';
  return p;
}

// Safely reads a character from a string at a given position, handling
// out-of-bounds.
char safe_char(const char *s, size_t char_pos) {
  debug_func("s: %s, char_pos: %zu", s, char_pos);
  if (!s)
    return '\0';
  size_t i = 0;
  while (1) {
    unsigned char c = (unsigned char)s[i];
    if (i == char_pos)
      return c; // Safely return the character at the position.
    if (c == '\0')
      return '\0'; // End of string reached before the position.
    i++;
  }
}

// Central cleanup function to free all allocated resources before exiting.
void cleanup(void) {

  debug_func("");

  // Print summary only in file mode and if there are logs.
  if (!ni->is_repl && (ctx->total_errors || ctx->total_warnings || ctx->total_infos)) {
    print_summary();
  }

  // Free all stored lines of source code.
  if (ctx->lines) {
    for (size_t i = 0; i < ctx->line_number; i++) {
      if (ctx->lines[i])
        free(ctx->lines[i]);
      ctx->lines[i] = NULL;
    }
    free((void *)ctx->lines);
    ctx->lines = NULL;
    ctx->line_number = 0;
    ctx->lines_capacity = 0;
  }

  // Free the Abstract Syntax Tree.
  if (ctx->ast_root) {
    free_node(ctx->ast_root);
    ctx->ast_root = NULL;
  }

  // Free the bracket matching stack.
  if (ctx->bracket_stack) {
    free(ctx->bracket_stack);
    ctx->bracket_stack = NULL;
    ctx->bracket_stack_capacity = 0;
    ctx->bracket_stack_size = 0;
  }
  // Close the input file if it's not stdin.
  if (ni->file && ni->file != stdin) {
    fclose(ni->file);
    ni->file = NULL;
  }

  // Free all tokens.
  if (ctx->tokens) {
    for (size_t i = 0; i < ctx->tokens_count; ++i) {
      if (ctx->tokens[i].token_value)
        free(ctx->tokens[i].token_value);
    }
    free(ctx->tokens);
    ctx->tokens = NULL;
    ctx->tokens_capacity = 0;
    ctx->tokens_count = 0;
  }

  // Free the temporary string token buffer.
  if (ctx->string_token) {
    free(ctx->string_token);
    ctx->string_token = NULL;
    ctx->string_token_length = 0;
    ctx->string_token_capacity = 0;
  }

  // Free all saved log entries.
  if (ctx->logs) {
    for (size_t i = 0; i < ctx->logs_count; ++i) {
      if (ctx->logs[i].log_msg)
        free(ctx->logs[i].log_msg);
      if (ctx->logs[i].log_symbol)
        free(ctx->logs[i].log_symbol);
    }

    free(ctx->logs);
    ctx->logs = NULL;
    ctx->logs_capacity = 0;
    ctx->logs_count = 0;

    ctx->total_errors = 0;
    ctx->total_warnings = 0;
    ctx->total_infos = 0;
  }
  // Free the buffer for the current line being processed.
  if (ctx->current_line) {
    free(ctx->current_line);
    ctx->current_line = NULL;
    ctx->line_length = 0;
    ctx->bytes_read = 0;
  }

  if (ni->is_repl) {
    if (ni->history.count != 0) { // Only save history if there are entries

      const char *home = getenv("HOME"); // Get HOME directory (Unix)
      if (!home) {
        home = getenv("USERPROFILE"); // Fallback for Windows
      }
      if (!home) {
        return; // Cannot determine home directory, abort saving
      }

      char path[1024];
      snprintf(path, sizeof(path), "%s/.noon_history", home); // Build history file path

      FILE *fp = fopen(path, "w"); // Open file for writing (overwrite)
      if (!fp) {
        return; // Cannot open file, abort saving
      }

      // Write each history entry into the file
      for (size_t i = 0; i < ni->history.count; i++) {
        fprintf(fp, "%s\n", ni->history.items[i]);
      }

      fclose(fp); // Close the file
    }
    // Free REPL history when the entire program stops.

    if (ni->history.items) {
      for (size_t i = 0; i < ni->history.count; i++) {
        free(ni->history.items[i]);
      }
      free(ni->history.items);
      ni->history.items = NULL;
      ni->history.count = 0;
    }
  }
  // Free the main context struct.
  if (ctx) {
    free(ctx);
    ctx = NULL;
  }

  if (ni->is_repl) {
    // In REPL mode, don't exit the whole program on error, just reset.
    return;
  }
  // In file mode, free the input struct and exit.
  if (ni) {
    free(ni);
    ni = NULL;
  }
}
