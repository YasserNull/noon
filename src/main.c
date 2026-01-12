// main.c
// Fixed argv parsing and safer handling of -c / --command input.

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
#include <errno.h>

/* handle Ctrl+C signals for both Windows and Unix */
#ifdef _WIN32
static BOOL WINAPI handle_ctrl_c(DWORD type) {
  debug_func("type: %d", type);
  if (type == CTRL_C_EVENT) {
    return TRUE;
  }
  return FALSE;
}
#else
static void handle_sigint(int signum) {
  debug_func("signum: %d", signum);
  (void)signum;
}
#endif

// create a FILE* stream containing `command` text using tmpfile().
// tmpfile() is portable and avoids issues with fmemopen and read-only string literals.
static FILE *open_command_stream(const char *command) {
  if (!command) return NULL;
  FILE *f = tmpfile();
  if (!f) return NULL;
  size_t len = strlen(command);
  if (len > 0) {
    size_t wrote = fwrite(command, 1, len, f);
    if (wrote != len) {
      fclose(f);
      return NULL;
    }
  }
  rewind(f);
  return f;
}

int main(int argc, char **argv) {
  debug_func("argc: %d", argc);
  disable_colors_if_not_tty(); // disable ANSI colors if not in a TTY terminal

  // Initialize input/ni **before** registering cleanup with atexit.
  init_input(); // initialize NoonInput global structure
  atexit(cleanup); // now safe: cleanup can rely on ni being initialized
  ni->program_name = argv[0];

#ifdef _WIN32
  SetConsoleCtrlHandler(handle_ctrl_c, TRUE);
#else
  signal(SIGINT, handle_sigint);
#endif

  /* parse command line arguments */
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];

    if (strcmp(arg, "-d") == 0 || strcmp(arg, "--debug") == 0) {
      ni->debug = 1;
      continue;
    }

    if (strcmp(arg, "-pt") == 0 || strcmp(arg, "--print-tokens") == 0) {
      ni->dump_tokens = 1;
      continue;
    }

    if (strcmp(arg, "-c") == 0 || strcmp(arg, "--command") == 0) {
      // next argv must exist
      if (i + 1 >= argc) {
        fprintf(stderr, ERR_OPTION_REQUIRES_ARGUMENT,
                COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, arg);
        exit(EXIT_FAILURE);
      }
      const char *command = argv[i + 1];
      ni->input = "<string>";
      ni->file = open_command_stream(command);
      if (!ni->file) {
        fprintf(stderr, ERR_MEM_STREAM_OPEN,
                COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD);
        exit(EXIT_FAILURE);
      }
      i++; // skip the command argument
      continue;
    }

    if (strcmp(arg, "-rp") == 0 || strcmp(arg, "--repl") == 0) {
      ni->is_repl = 1;
      continue;
    }

    if (strcmp(arg, "-pa") == 0 || strcmp(arg, "--print-ast") == 0) {
      ni->dump_ast = 1;
      continue;
    }

    if (strcmp(arg, "-cs") == 0 || strcmp(arg, "--check-syntax") == 0) {
      ni->check_syntax = 1;
      continue;
    }

    // unrecognized long option like --something
    if (arg[0] == '-' && arg[1] == '-' && arg[2]) {
      fprintf(stderr, ERR_UNRECOGNIZED_OPTION,
              COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, arg, ni->program_name);
      exit(EXIT_FAILURE);
    }

    // invalid short option (e.g., -z or grouped -abc)
    if (arg[0] == '-' && arg[1]) {
      fprintf(stderr, ERR_INVALID_OPTION,
              COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, arg + 1, ni->program_name);
      exit(EXIT_FAILURE);
    }

    // Otherwise treat as a source filename
    if (ni->file != NULL) {
      if (ni->file != stdin) {
        fclose(ni->file);
      }
      fprintf(stderr, ERR_MULTIPLE_INPUT_FILES,
              COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, arg);
      exit(EXIT_FAILURE);
    }

    ni->file = fopen(arg, "r");
    if (!ni->file) {
      fprintf(stderr, ERR_NO_FILE,
              COLOR_BOLD, ni->program_name, COLOR_RED, COLOR_RESET, COLOR_BOLD, arg);
      exit(EXIT_FAILURE);
    }
    ni->input = arg;
  }

  /* if no file is provided, enter REPL mode */
  if (!ni->file) {
    ni->file = stdin;
    ni->is_repl = 1;
  }

  return lexer(); // start the lexer
}