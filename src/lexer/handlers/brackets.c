// lexer/handlers/brackets.c
// This file manages bracket matching using a stack. It ensures that all
// parentheses `()`, braces `{}`, and square brackets `[]` are properly
// opened and closed in the correct order.

#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Defines the types of brackets the lexer tracks.
const Bracket brackets[] = {{'(', ')', 0, 0}, {'{', '}', 0, 0}, {'[', ']', 0, 0}};
const size_t NUM_BRACKETS = sizeof(brackets) / sizeof(brackets[0]);

// Pushes an opening bracket onto the stack.
static void push_bracket_stack(Bracket bracket, size_t bracket_line, size_t bracket_index) {
  debug_func("%zu, %zu", bracket_line, bracket_index);
  // Resize the stack if it's full.
  if (ctx->bracket_stack_size >= ctx->bracket_stack_capacity) {
    size_t new_cap = ctx->bracket_stack_capacity ? ctx->bracket_stack_capacity * 2 : INITIAL_CAPACITY;
    BracketStackItem *tmp = safe_realloc(ctx->bracket_stack, new_cap * sizeof(BracketStackItem));
    ctx->bracket_stack = tmp;
    ctx->bracket_stack_capacity = new_cap;
  }

  // Add the new bracket item to the top of the stack.
  ctx->bracket_stack[ctx->bracket_stack_size].bracket = bracket;
  ctx->bracket_stack[ctx->bracket_stack_size].bracket_line = bracket_line;
  ctx->bracket_stack[ctx->bracket_stack_size].bracket_index = bracket_index;
  ctx->bracket_stack_size++;
}

// Pops a bracket from the stack.
static BracketStackItem pop_bracket_stack(void) {
  debug_func("");
  if (ctx->bracket_stack_size == 0) {
    // This case should be handled by handle_brackets, but is here for safety.
    save_log(LOG_ERROR, "stack underflow", (LogPosition){ctx->line_number, ctx->line_index}, "");
    BracketStackItem dummy = {{0, 0, 0, 0}, 0, 0};
    return dummy;
  }
  return ctx->bracket_stack[--ctx->bracket_stack_size];
}

// Processes a character to check if it's an opening or closing bracket.
bool handle_brackets(char c) {
  debug_func("%c", c);
  for (size_t b = 0; b < NUM_BRACKETS; b++) {
    Bracket br = brackets[b];
    // If it's an opening bracket, push it onto the stack.
    if (c == br.open) {
      push_bracket_stack(br, ctx->line_number, ctx->line_index + 1);
    } else if (c == br.close) {
      // If it's a closing bracket, check for a match on top of the stack.
      if (ctx->bracket_stack_size == 0 || ctx->bracket_stack[ctx->bracket_stack_size - 1].bracket.close != br.close) {
        // Error: Unmatched closing bracket.
        const char tmpc[2] = {br.close, '\0'};
        const char *type = (br.close == ')') ? "bracket" : (br.close == '}') ? "curly" : (br.close == ']') ? "square" : "unknown";
        print_log(LOG_ERROR, ERR_UNMATCHED, (LogPosition){ctx->line_number, ctx->line_index + 1}, tmpc, type, tmpc);
        // This is a fatal syntax error.
        ctx->has_syntax_error = 1;
        return true;
      } else {
        // Matched successfully, pop from the stack.
        pop_bracket_stack();
      }
    }
  }
  return false;
}

// Checks for any unclosed brackets at the end of the input.
void check_unclosed_brackets(void) {
  debug_func("");
  // Any items left on the stack are unclosed brackets.
  for (size_t i = 0; i < ctx->bracket_stack_size; i++) {
    BracketStackItem item = ctx->bracket_stack[i];
    const char tmp[2] = {item.bracket.open, '\0'};
    const char *type = (item.bracket.open == '(') ? "bracket" : (item.bracket.open == '{') ? "curly" : (item.bracket.open == '[') ? "square" : "unknown";
    save_log(LOG_ERROR, ERR_UNCLOSED, (LogPosition){item.bracket_line, item.bracket_index}, tmp, type, tmp);
  }
}
