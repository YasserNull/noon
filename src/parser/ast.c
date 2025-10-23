// parser/ast.c
// This file defines the structure of the Abstract Syntax Tree (AST) and
// provides functions to create, free, and print AST nodes. The AST is the
// hierarchical representation of the source code's structure.

#include "parser/ast.h"
#include "config.h"
#include "context.h"
#include "input.h"
#include "lexer/lexer.h"
#include "utils/log.h"
#include "utils/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Creates a number node for the AST.
Node *create_number_node(Token token, double value) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_NUMBER;
  node->token = token;
  node->number_value = value;
  return node;
}

// Creates a character literal node for the AST.
Node *create_char_node(Token token, const char *value) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_CHAR;
  node->token = token;
  node->string_value = safe_strdup(value);
  return node;
}

// Creates a string literal node for the AST.
Node *create_string_node(Token token, const char *value) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_STRING;
  node->token = token;
  node->string_value = safe_strdup(value);
  return node;
}

// Creates a boolean literal node for the AST.
Node *create_boolean_node(Token token, bool value) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_BOOLEAN;
  node->token = token;
  node->boolean_value = value;
  return node;
}

// Creates a null literal node for the AST.
Node *create_null_node(Token token) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_NULL;
  node->token = token;
  return node;
}

// Helper function to determine the type of a sub-expression for type checking.
static NodeType get_expression_type(Node *node) {
  debug_func("");
  if (!node) {
    return NODE_NULL;
  }

  switch (node->node_type) {
  case NODE_NUMBER: return NODE_NUMBER;
  case NODE_STRING:
  case NODE_CHAR: return NODE_STRING;
  case NODE_BOOLEAN: return NODE_BOOLEAN;
  case NODE_NULL: return NODE_NULL;
  case NODE_UNARY_OP:
  case NODE_POSTFIX_OP: return get_expression_type(node->unary.operand);
  case NODE_BINARY_OP: {
    // For arithmetic operations, the result type is the common type.
    NodeType left_type = get_expression_type(node->binary.left);
    NodeType right_type = get_expression_type(node->binary.right);

    if (left_type == NODE_NUMBER && right_type == NODE_NUMBER)
      return NODE_NUMBER;
    if ((left_type == NODE_STRING || left_type == NODE_CHAR) && (right_type == NODE_STRING || right_type == NODE_CHAR))
      return NODE_STRING;
    return NODE_NULL; // Incompatible types.
  }
  default: return node->node_type;
  }
}

// Creates a binary operation node and performs basic type checking.
Node *create_binary_op_node(Token op, Node *left, Node *right) {
  debug_func("");

  if (!left || !right) {
    if (left)
      free_node(left);
    if (right)
      free_node(right);
    return NULL;
  }

  NodeType left_type = get_expression_type(left);
  NodeType right_type = get_expression_type(right);

  bool is_left_numeric = (left_type == NODE_NUMBER);
  bool is_right_numeric = (right_type == NODE_NUMBER);

  bool is_left_stringy = (left_type == NODE_STRING || left_type == NODE_CHAR);
  bool is_right_stringy = (right_type == NODE_STRING || right_type == NODE_CHAR);

  bool is_valid = false;

  // Check if the operator is valid for the given operand types.
  switch (op.token_type) {
  case TOKEN_PLUS:
    // '+' is valid for number+number or string+string.
    if ((is_left_numeric && is_right_numeric) || (is_left_stringy && is_right_stringy)) {
      is_valid = true;
    }
    break;

  // These operators are only valid for numbers.
  case TOKEN_MINUS:
  case TOKEN_STAR:
  case TOKEN_SLASH:
  case TOKEN_DOUBLEPERCENT:
  case TOKEN_PERCENT:
  case TOKEN_POW:
  case TOKEN_LEFTSHIFT:
  case TOKEN_RIGHTSHIFT:
  case TOKEN_AMPERSAND:
  case TOKEN_CARET:
  case TOKEN_PIPE:
    if (is_left_numeric && is_right_numeric) {
      is_valid = true;
    }
    break;

  // Comparison operators are valid for number/number or string/string.
  case TOKEN_EQEQUAL:
  case TOKEN_NOTEQUAL:
  case TOKEN_LESS:
  case TOKEN_LESSEQUAL:
  case TOKEN_GREATER:
  case TOKEN_GREATEREQUAL:
    if ((is_left_numeric && is_right_numeric) || (is_left_stringy && is_right_stringy)) {
      is_valid = true;
    }
    break;

  // Logical operators.
  case TOKEN_AND:
  case TOKEN_OR:
    if (is_left_numeric && is_right_numeric) {
      is_valid = true;
    }
    break;

  // Assignment operators.
  case TOKEN_EQUAL:
  case TOKEN_PLUSEQUAL:
  case TOKEN_MINEQUAL:
  case TOKEN_STAREQUAL:
  case TOKEN_SLASHEQUAL:
  case TOKEN_PERCENTEQUAL:
  case TOKEN_AMPERSANDEQUAL:
  case TOKEN_PIPEEQUAL:
  case TOKEN_CARETEQUAL:
  case TOKEN_LEFTSHIFTEQUAL:
  case TOKEN_RIGHTSHIFTEQUAL:
  case TOKEN_DOUBLESTAREQUAL:
  case TOKEN_DOUBLEPERCENTEQUAL:
    if ((is_left_numeric && is_right_numeric) || (is_left_stringy && is_right_stringy)) {
      is_valid = true;
    }
    break;

  default: break;
  }

  // If the operation is not valid, report a type error.
  if (!is_valid) {
    if (!ctx->has_syntax_error) {
      ctx->has_syntax_error = 1;
      const char *left_str = token_type_to_string(left->token.token_type);
      const char *right_str = token_type_to_string(right->token.token_type);

      print_log(
          LOG_ERROR, ERR_TYPE_OP_NOT_SUPPORTED, (LogPosition){op.token_line, op.token_index + 1}, op.token_value, op.token_value, left_str ? left_str : "unknown", right_str ? right_str : "unknown");
    }

    return NULL;
  }

  // Create and return the new binary operation node.
  Node *node = safe_malloc(sizeof(Node));
  if (!node) {
    free_node(left);
    free_node(right);
    return NULL;
  }

  node->node_type = NODE_BINARY_OP;
  node->binary.left = left;
  node->binary.right = right;
  node->binary.op = op;

  return node;
}

// Creates a unary (prefix) operation node for the AST.
Node *create_unary_op_node(Token op, Node *operand) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_UNARY_OP;
  node->unary.op = op;
  node->unary.operand = operand;
  return node;
}

// Creates a postfix operation node for the AST.
Node *create_postfix_op_node(Token op, Node *operand) {
  debug_func("");
  Node *node = safe_malloc(sizeof(Node));
  node->node_type = NODE_POSTFIX_OP;
  node->unary.op = op;
  node->unary.operand = operand;
  return node;
}

// Recursively frees an AST node and all its children.
void free_node(Node *node) {
  if (!node)
    return;

  switch (node->node_type) {
  case NODE_BINARY_OP:
    // Free left and right children.
    if (node->binary.left) {
      free_node(node->binary.left);
      node->binary.left = NULL;
    }
    if (node->binary.right) {
      free_node(node->binary.right);
      node->binary.right = NULL;
    }
    break;
  case NODE_POSTFIX_OP:
  case NODE_UNARY_OP:
    // Free the single operand.
    if (node->unary.operand) {
      free_node(node->unary.operand);
      node->unary.operand = NULL;
    }
    break;
  default: break;
  }

  // Free string value for string/char nodes.
  if ((node->node_type == NODE_STRING || node->node_type == NODE_CHAR) && node->string_value) {
    free(node->string_value);
    node->string_value = NULL;
  }

  // Free the node itself.
  free(node);
}

// Forward declaration for the internal recursive printing function.
static void print_ast_recursive(Node *node, const char *prefix, bool is_last);

// Helper function to print the string representation of a single node's value.
// This function avoids printing any tree-formatting characters or newlines.
static void print_node_value(Node *node) {
  if (!node)
    return;

  switch (node->node_type) {
  case NODE_NUMBER: printf("%g", node->number_value); break;
  case NODE_CHAR:
  case NODE_STRING: printf("%s", node->string_value); break;
  case NODE_BOOLEAN: printf("%s", node->boolean_value ? "true" : "false"); break;
  case NODE_NULL: printf("null"); break;
  case NODE_BINARY_OP:
    switch (node->binary.op.token_type) {
    case TOKEN_PLUS: printf("+"); break;
    case TOKEN_MINUS: printf("-"); break;
    case TOKEN_STAR: printf("*"); break;
    case TOKEN_SLASH: printf("/"); break;
    case TOKEN_DOUBLEPERCENT: printf("%%"); break;
    case TOKEN_PERCENT: printf("%%"); break;
    case TOKEN_POW: printf("**"); break;
    case TOKEN_LEFTSHIFT: printf("<<"); break;
    case TOKEN_RIGHTSHIFT: printf(">>"); break;
    case TOKEN_AMPERSAND: printf("&"); break;
    case TOKEN_CARET: printf("^"); break;
    case TOKEN_PIPE: printf("|"); break;
    case TOKEN_EQEQUAL: printf("=="); break;
    case TOKEN_NOTEQUAL: printf("!="); break;
    case TOKEN_LESSEQUAL: printf("<="); break;
    case TOKEN_GREATEREQUAL: printf(">="); break;
    case TOKEN_LESS: printf("<"); break;
    case TOKEN_GREATER: printf(">"); break;
    case TOKEN_AND: printf("&&"); break;
    case TOKEN_OR: printf("||"); break;
    case TOKEN_EQUAL: printf("="); break;
    case TOKEN_PLUSEQUAL: printf("+="); break;
    case TOKEN_MINEQUAL: printf("-="); break;
    case TOKEN_STAREQUAL: printf("*="); break;
    case TOKEN_SLASHEQUAL: printf("/="); break;
    case TOKEN_PERCENTEQUAL: printf("%%="); break;
    case TOKEN_AMPERSANDEQUAL: printf("&=\n"); break;
    case TOKEN_PIPEEQUAL: printf("|="); break;
    case TOKEN_CARETEQUAL: printf("^="); break;
    case TOKEN_LEFTSHIFTEQUAL: printf("<<="); break;
    case TOKEN_RIGHTSHIFTEQUAL: printf(">>="); break;
    case TOKEN_DOUBLESTAREQUAL: printf("**="); break;
    case TOKEN_DOUBLEPERCENTEQUAL: printf("%%="); break;
    default: printf("?"); break;
    }
    break;
  case NODE_UNARY_OP:
    switch (node->unary.op.token_type) {
    case TOKEN_PLUS: printf("+ (unary)"); break;
    case TOKEN_MINUS: printf("- (unary)"); break;
    case TOKEN_NOT: printf("! (unary)"); break;
    case TOKEN_TILDE: printf("~ (unary)"); break;
    case TOKEN_INCREMENT: printf("++ (prefix)"); break;
    case TOKEN_DECREMENT: printf("-- (prefix)"); break;
    default: printf("? (unary)"); break;
    }
    break;
  case NODE_POSTFIX_OP:
    switch (node->unary.op.token_type) {
    case TOKEN_INCREMENT: printf("++ (postfix)"); break;
    case TOKEN_DECREMENT: printf("-- (postfix)"); break;
    default: printf("? (postfix)"); break;
    }
    break;
  default: printf("Unknown node"); break;
  }
}

// Prints a formatted, tree-like representation of the AST for debugging.
// This is the main entry point for printing the tree, which then calls a
// recursive helper to handle the branches.
void print_ast(Node *node) {
  if (!ni->dump_ast)
    return;
  debug_func("");
  if (!node)
    return;

  // Print the root node's value first, as it has no prefix.
  print_node_value(node);
  printf("\n");

  // Collect direct children of the root into a temporary array.
  // This is needed to know which child is the last one for proper formatting.
  Node *children[2] = {NULL, NULL};
  int num_children = 0;

  switch (node->node_type) {
  case NODE_BINARY_OP:
    if (node->binary.left)
      children[num_children++] = node->binary.left;
    if (node->binary.right)
      children[num_children++] = node->binary.right;
    break;
  case NODE_UNARY_OP:
  case NODE_POSTFIX_OP:
    if (node->unary.operand)
      children[num_children++] = node->unary.operand;
    break;
  // Leaf nodes (like numbers, strings) have no children.
  default: break;
  }

  // Start the recursion for the children.
  for (int i = 0; i < num_children; i++) {
    bool is_last = (i == num_children - 1);
    // The initial prefix for the root's children is an empty string.
    print_ast_recursive(children[i], "", is_last);
  }
}

// Internal recursive function to print a node and traverse its children.
// This handles the core logic of building the tree structure with prefixes.
static void print_ast_recursive(Node *node, const char *prefix, bool is_last) {
  if (!node)
    return;

  // Print the prefix and branch connector for the current line.
  printf("%s", prefix);
  printf("%s", is_last ? "└── " : "├── ");
  print_node_value(node);
  printf("\n");

  // Prepare the prefix for the next level of children.
  // A new string is dynamically allocated to append the next segment.
  char *child_prefix = safe_malloc(strlen(prefix) + 5); // "│   " is 4 chars + null terminator.
  strcpy(child_prefix, prefix);

  // If the current node is the last in its list, its children's prefix
  // should have empty space instead of a vertical bar.
  strcat(child_prefix, is_last ? "    " : "│   ");

  // Collect children of the current node.
  Node *children[2] = {NULL, NULL};
  int num_children = 0;

  switch (node->node_type) {
  case NODE_BINARY_OP:
    if (node->binary.left)
      children[num_children++] = node->binary.left;
    if (node->binary.right)
      children[num_children++] = node->binary.right;
    break;
  case NODE_UNARY_OP:
  case NODE_POSTFIX_OP:
    if (node->unary.operand)
      children[num_children++] = node->unary.operand;
    break;
  default: break; // Leaf node.
  }

  // Recurse for each child.
  for (int i = 0; i < num_children; i++) {
    print_ast_recursive(children[i], child_prefix, (i == num_children - 1));
  }

  // Free the dynamically allocated prefix to prevent memory leaks.
  free(child_prefix);
}
