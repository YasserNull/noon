// main.c
// This is the main entry point of the interpreter.
// It handles command-line argument parsing, sets up the program's initial
// state, manages input from files or the REPL, and starts the lexer.

#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* handle Ctrl+C signals for both Windows and Unix */
#ifdef _WIN32
// Signal handler for Ctrl+C on Windows.
static BOOL WINAPI handle_ctrl_c(DWORD type) {
  debug_func("type: %d", type);
  if (type == CTRL_C_EVENT) {
    return TRUE; // stop program cleanly when user presses Ctrl+C
  }
  return FALSE;
}
#else
// Signal handler for SIGINT (Ctrl+C) on Unix-like systems.
static void handle_sigint(int signum) {
  debug_func("signum: %d", signum);
  (void)signum;
} // ignore signal on Unix
#endif

// The main function of the program.
int main(int argc, char **argv) {
  debug_func("argc: %d, argv[]", argc);
  disable_colors_if_not_tty(); // disable ANSI colors if not in a TTY terminal
  atexit(cleanup);
// Set up the appropriate signal handler for Ctrl+C.
#ifdef _WIN32
  SetConsoleCtrlHandler(handle_ctrl_c, TRUE);
#else
  signal(SIGINT, handle_sigint);
#endif

  init_input(); // initialize NoonInput global structure
  ni->program_name = argv[0];

  /* parse command line arguments */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
      ni->debug = 1; // enable debug mode
      continue;
    } else if (strcmp(argv[i], "-pt") == 0 || strcmp(argv[i], "--print-tokens") == 0) {
      ni->dump_tokens = 1; // enable token printing
      continue;
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0) {
      // execute code directly from command string
      if (i + 1 >= argc) {
        fprintf(stderr, ERR_OPTION_REQUIRES_ARGUMENT, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, argv[i]);
        exit(EXIT_FAILURE);
      }
      ni->input = "<string>";
      const char *command = argv[i + 1];
      // Open a memory stream to read the command string as a file.
      ni->file = fmemopen((void *)command, strlen(command), "r");
      if (!ni->file) {
        fprintf(stderr, ERR_MEM_STREAM_OPEN, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD);
        exit(EXIT_FAILURE);
      }
      i++; // skip the code argument
      continue;
    } else if (strcmp(argv[i], "-rp") == 0 || strcmp(argv[i], "--repl") == 0) {
      ni->is_repl = 1; // enable REPL mode
      continue;
    } else if (strcmp(argv[i], "-pa") == 0 || strcmp(argv[i], "--print-ast") == 0) {
      ni->dump_ast = 1; // enable AST printing
      continue;
    } else if (strcmp(argv[i], "-cs") == 0 || strcmp(argv[i], "--check-syntax") == 0) {
      ni->check_syntax = 1; // check syntax
      continue;
    } else if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2]) {
      // Handle unrecognized long options (e.g., --invalidoption).
      fprintf(stderr, ERR_UNRECOGNIZED_OPTION, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, argv[i], ni->program_name);
      exit(EXIT_FAILURE);
    } else if (argv[i][0] == '-' && argv[i][1]) {
      // Handle invalid short options (e.g., -z).
      fprintf(stderr, ERR_INVALID_OPTION, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, argv[i] + 1 /*+1 to skip '-' */, ni->program_name);

      exit(EXIT_FAILURE);
    } else {
      // handle source file input
      if (ni->file != NULL) {
        fprintf(stderr, ERR_MULTIPLE_INPUT_FILES, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, argv[i]);
        fclose(ni->file);
        exit(EXIT_FAILURE);
      }
      ni->file = fopen(argv[i], "r");
      if (!ni->file) {
        fprintf(stderr, ERR_NO_FILE, COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, argv[i]);
        exit(EXIT_FAILURE);
      }
      ni->input = argv[i];
    }
  }

  /* if no file is provided, enter REPL mode */
  if (!ni->file) {
    ni->file = stdin;
    ni->is_repl = 1;
  }

  return lexer(); // start the lexer
}
