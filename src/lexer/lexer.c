// lexer/lexer.c
// This is the main file for the lexer (tokenizer).
// It reads input line by line, processes characters through a state machine,
// and converts the character stream into a sequence of tokens for the parser.

#include "lexer/lexer.h"
#include "config.h"
#include "context.h"
#include "lexer/tokens.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "utils/log.h"
#include "utils/memory.h"
#include "utils/strings.h"
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initializes the lexer state and allocates memory for various buffers.
void init_lexer(void) {
  debug_func("");
  init_context(); // Prepare all fields in the global context `ctx`.

  // Allocate initial memory for storing source code lines.
  ctx->lines = safe_calloc(INITIAL_CAPACITY, sizeof(char *));
  ctx->lines_capacity = INITIAL_CAPACITY;

  // Allocate initial memory for the token list.
  ctx->tokens = safe_calloc(INITIAL_CAPACITY, sizeof(Token));
  ctx->tokens_capacity = INITIAL_CAPACITY;

  // Allocate initial memory for the log entries.
  ctx->logs = safe_calloc(INITIAL_CAPACITY, sizeof(LogEntry));
  ctx->logs_capacity = INITIAL_CAPACITY;

  // Allocate initial memory for the bracket matching stack.
  ctx->bracket_stack = safe_calloc(INITIAL_CAPACITY, sizeof(BracketStackItem));
  ctx->bracket_stack_capacity = INITIAL_CAPACITY;

  // Print the initial REPL prompt if in REPL mode.
  if (ni->is_repl) {
    // Print the initial primary prompt (>>>).
    printf(">>> ");
    fflush(stdout);
  }
}

// The main lexer function. It loops through input and produces tokens.
int lexer(void) {
  debug_func("");
  init_lexer();

  // Main loop: read one line at a time.
  while ((ctx->bytes_read = portable_getline(&ctx->current_line, &ctx->line_length, ni->file)) != -1) {

    // Expand the lines buffer if necessary.
    if (ctx->line_number >= ctx->lines_capacity) {
      size_t new_cap = ctx->lines_capacity * 2;
      ctx->lines = safe_realloc(ctx->lines, new_cap * sizeof(char *));
      ctx->lines_capacity = new_cap;
    }

    // Store a copy of the current line for error reporting.
    ctx->lines[ctx->line_number] = safe_strdup(ctx->current_line);
    if (ctx->lines[ctx->line_number]) {
      // Remove trailing newline character.
      ctx->lines[ctx->line_number][strcspn(ctx->lines[ctx->line_number], "\n")] = '\0';
    }
    ctx->line_number++;

    if (ni->is_repl && ni->file != stdin) {
      // just for tests
      printf("%s\n", ctx->current_line);
    }
    // If the line is empty or just whitespace, skip to the next line.
    if (is_nothing(ctx->current_line)) {
      if (ni->is_repl) {
        // Print the appropriate REPL prompt.
        if (ctx->state == STATE_NORMAL && ctx->bracket_stack_size == 0) {
          // if state is normal and opened brackets is 0
          // This condition is met if the line is empty and the current statement is complete (ready for a new command).
          printf(">>> ");
          fflush(stdout);
        } else {
          // if state is not normal and opened brackets is not 0
          // examples:/*
          // here the if statement is well happen
          //*/
          // or (
          // 1+1 #here the if statement is well happen
          // )
          // This condition is met if the line is empty but a multi-line structure is still open (e.g., unclosed brackets or comment).
          printf("... ");
          fflush(stdout);
        }
      }
      continue;
    }

    // Loop through each character of the current line.
    for (ctx->line_index = 0; ctx->bytes_read > 0 && ctx->line_index < (size_t)ctx->bytes_read && ctx->current_line[ctx->line_index] != '\0'; ctx->line_index++) {

      char c = ctx->current_line[ctx->line_index];

      // Skip whitespace characters.
      if (isspace((unsigned char)c))
        continue;

      // Handle characters based on the current lexer state.
      if (ctx->state == STATE_QUOTE) {
        tokenize_strings(c);
        continue;
      }
      if (ctx->state == STATE_MULTI_COMMENT) {
        handle_multi_comment(c);

        if (is_nothing(ctx->current_line) && ctx->current_line[ctx->line_index + 1] == '\0' && ni->is_repl) {

          if (ctx->state != STATE_NORMAL || ctx->bracket_stack_size != 0) {
            // // if line is empty and state is multi comment like
            //                                                                     /*
            //  this line the if statement is It will happen */
            // If the current line ends, and the state is still STATE_MULTI_COMMENT, prompt for continuation.
            printf("... "), fflush(stdout);
          } else {
            // example 1/**/ if line is multi line comment like /**/
            // If the multi-line comment closed on this line, reset the prompt to '>>>'.
            printf(">>> "), fflush(stdout);
          }
        }
        continue;
      }
      if (ctx->state == STATE_NORMAL) {
        handle_quotes(c);
        if ((handle_comments(c) || handle_brackets(c)) && ni->is_repl) {
          // A fatal, interactive error occurred.
          break;
        }
        if (is_nothing(ctx->current_line) && ctx->current_line[ctx->line_index + 1] == '\0' && ni->is_repl && ctx->state == STATE_MULTI_COMMENT) {
          // if line is empty and state is multi comment and this is last char
          // example `/*`
          // If a multi-line comment (/*) was started right before the end of the line, prompt for continuation.
          printf("... "), fflush(stdout);
        }
      }
      // In normal state, try to tokenize numbers, identifiers, or symbols.
      if (ctx->state == STATE_NORMAL) {
        if (tokenize_number())
          continue;
        if (tokenize_identifier())
          continue;
        if (tokenize_symbol())
          continue;
      }
    }
    if (!ni->check_syntax && !ni->is_repl && ctx->has_syntax_error) {
      exit(1);
    }

    if (is_nothing(ctx->current_line)) {

      continue;
    }
    // If a complete statement is formed, parse it.
    if (ctx->state == STATE_NORMAL && ctx->bracket_stack_size == 0) {
      print_tokens();
      ctx->ast_root = parse();

      // If parsing was successful, print the AST.
      if (ctx->ast_root) {
        print_ast(ctx->ast_root);
        free_node(ctx->ast_root);
        ctx->ast_root = NULL;
      }

      // Clear tokens for the next statement.
      free_tokens();

      if (!ni->check_syntax && !ni->is_repl && ctx->has_syntax_error) {
        exit(EXIT_FAILURE);
      }
      if (ni->is_repl && ni->file == stdin) {
        // this statement is well happen if line is not empty and state is normal and brackets size is 0
        // If a non-empty line resulted in a complete statement, print the primary prompt (>>>).
        printf(">>> ");
        fflush(stdout);
      }

    } else if (ni->is_repl) {
      // this statement is well happen if line is not empty and state is not normal and brackets size is not 0
      // example `(,{,[` or `",'` or `/*,*/`
      // note if is multi comment, only if line not empty like `1/*` or `*/2`
      // If the line was non-empty but the statement is incomplete (e.g., unclosed brackets or multi-line state), print the secondary prompt (...).
      printf("... ");
      fflush(stdout);
    }
    ctx->has_syntax_error = 0;
  }

  // Reset logs on repl
  if (ni && ni->is_repl) {
    reset_logs();
  }
  // After reaching EOF, check for any unclosed constructs.
  check_unclosed_quote();
  check_unclosed_comment();
  check_unclosed_brackets();

  // Sort and print all collected logs.
  sort_logs();
  if (ni && ni->is_repl)
    putchar('\n');
  print_logs();

  if (ctx->total_errors || ctx->total_warnings || ctx->total_infos) {
    if (ni && ni->is_repl) {
      print_summary();
    }

    // In REPL mode, reset state after printing logs.
    if (ni->is_repl) {
      return EXIT_SUCCESS;
    }

    // In file mode, exit with failure after printing summary.
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
