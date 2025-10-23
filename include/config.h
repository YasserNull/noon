// config.h
// This file contains global configuration constants for the interpreter,
// including buffer sizes, ANSI color codes, and all error/warning message
// format strings.

#ifndef CONFIG_H
#define CONFIG_H

#define INITIAL_CAPACITY 16
#define LINE_SIZE 1024

/* Colors */
extern const char *COLOR_RED;
extern const char *COLOR_CYAN;
extern const char *COLOR_PURPLE;
extern const char *COLOR_BOLD;
extern const char *COLOR_GREEN;
extern const char *COLOR_RESET;

/* Errors messages */

// Input and command-line argument errors.
#define ERR_NO_FILE "%s%s: %serror: %s%sno such file or directory: '%s'\n"
#define ERR_MEM_STREAM_OPEN "%s%s: %serror: %s%scannot open memory stream\n"
#define ERR_MULTIPLE_INPUT_FILES                                                                                                                                                                       \
  "%s%s: %serror: %s%scannot open '%s': another input file already "                                                                                                                                   \
  "specified\n"
#define ERR_OPTION_REQUIRES_ARGUMENT "%s%s: %serror: %s%s%s: option requires an argument\n"
#define ERR_INVALID_OPTION                                                                                                                                                                             \
  "%s%s: %serror: %s%sinvalid option -- '%s'\nTry '%s --help' for more "                                                                                                                               \
  "information\n"
#define ERR_UNRECOGNIZED_OPTION                                                                                                                                                                        \
  "%s%s: %serror: %s%sunrecognized option '%s'\nTry '%s --help' for more "                                                                                                                             \
  "information\n"

#define TOTAL_ERRORS "%d error%s generated.\n"

// Lexer warnings and errors.
#define WRN_MULTICHAR_COMMENT "multi-character character constant"
#define ERR_UNCLOSED "unclosed %s `%s`"
#define ERR_UNMATCHED "unmatched %s `%s`"

// Parser errors.
#define ERR_EXPECTED_VALUE_BEFORE_OP "expected value before operator `%s`"
#define ERR_EXPECTED_VALUE_AFTER_OP "expected value after operator `%s`"
#define ERR_EXPECTED_EXPR_AFTER_UNARY "expected expression after unary operator `%s`"
#define ERR_EXPECTED_EXPR_IN_PARENS "expected expression inside parentheses"
#define ERR_EXPECTED_RPAREN "expected ')' after expression"
#define ERR_EXPECTED_EXPRESSION "expected expression"
#define ERR_EXPECTED_VALUE "expected value"
#define ERR_TYPE_OP_NOT_SUPPORTED "operator `%s` not supported between %s and %s"
#define ERR_INVALID_SYNTAX "invalid syntax `%s`"

// Numeric literal errors.
#define ERR_INVALID_DECIMAL_LITERAL "invalid decimal literal `%s`"
#define ERR_INVALID_HEX_LITERAL "invalid hexadecimal literal"
#define ERR_INVALID_BINARY_LITERAL "invalid binary literal"
#define ERR_INVALID_OCTAL_LITERAL "invalid octal literal"
#define ERR_CONSECUTIVE_NUMERIC_SEPARATOR "consecutive underscore in numeric literal `%s`"
#define ERR_TRAILING_NUMERIC_SEPARATOR "trailing underscore in numeric literal `%s`"

// Function to disable colors if not in a TTY.
void disable_colors_if_not_tty(void);
#endif
