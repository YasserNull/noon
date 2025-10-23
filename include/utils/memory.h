// utils/memory.h
// Header file for memory utilities. It declares safe wrappers for standard
// memory allocation functions (malloc, calloc, etc.) and the central
// cleanup function for releasing all program resources.

#ifndef MOMERY_H
#define MOMERY_H

#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Safe memory allocation wrappers.
void *safe_malloc(size_t n);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_realloc(void *ptr, size_t new_size);
char *safe_strdup(const char *s);

// Safe character access function.
char safe_char(const char *s, size_t char_pos);

// Central cleanup and exit function.
void cleanup(void);

#endif
