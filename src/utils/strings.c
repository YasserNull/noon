// utils/strings.c
// This file contains miscellaneous utility functions for string and number
// manipulation.

#include "utils/strings.h"
#include "utils/log.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Checks if a string is NULL or contains only whitespace characters.
int is_nothing(const char *str) {
  debug_func("str: %s", str);
  if (str == NULL)
    return 1;
  while (*str) {
    if (!isspace((unsigned char)*str))
      return 0; // Found a non-whitespace character.
    str++;
  }
  return 1; // String is all whitespace.
}

// Counts the number of digits in an integer.
int number_count(int number) {
  debug_func("str: %d", number);
  if (number == 0)
    return 1;
  return (int)(log10(number) + 1);
}
