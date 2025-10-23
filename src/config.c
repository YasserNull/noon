// config.c
// This file defines global configuration values, such as ANSI color codes
// for formatted output. It also provides a function to disable colors
// when the output is not a terminal (e.g., when redirecting to a file).

#include "config.h"
#include "utils/log.h"
#include <stdio.h>
#include <unistd.h>

// Colors
const char *COLOR_RED = "\033[31m";
const char *COLOR_CYAN = "\033[36m";
const char *COLOR_GREEN = "\033[32m";
const char *COLOR_PURPLE = "\033[35m";
const char *COLOR_BOLD = "\033[1m";
const char *COLOR_RESET = "\033[0m";

// Disable colors if stderr is not a TTY (terminal).
void disable_colors_if_not_tty(void) {
  debug_func("");
  if (!isatty(fileno(stderr))) {
    COLOR_RED = "";
    COLOR_CYAN = "";
    COLOR_PURPLE = "";
    COLOR_GREEN = "";
    COLOR_BOLD = "";
    COLOR_RESET = "";
  }
}
