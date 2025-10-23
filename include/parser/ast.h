// parser/ast.h
// Header file for the Abstract Syntax Tree (AST). It defines the node types
// and the main Node struct, which is used to build the tree representation
// of the parsed code. It also declares functions for creating and managing
// nodes.

#ifndef AST_H
#define AST_H

#include "lexer/tokens.h"
#include <stdbool.h>

// Enum of all possible AST node types.
typedef enum {
  NODE_NUMBER,
  NODE_CHAR,
  NODE_STRING,
  NODE_BOOLEAN,
  NODE_NULL,
  NODE_BINARY_OP,
  NODE_UNARY_OP,  // For prefix unary operators (e.g., ++x, -x)
  NODE_POSTFIX_OP // For postfix unary operators (e.g., x++, x--)
} NodeType;

// The main struct for an AST node.
typedef struct Node {
  NodeType node_type;
  Token token; // Stores the original token for location/value info.
  union {
    double number_value;
    char *string_value;
    bool boolean_value;

    // For binary operation nodes.
    struct {
      struct Node *left;
      struct Node *right;
      Token op;
    } binary;

    // For both prefix and postfix unary nodes.
    struct {
      struct Node *operand;
      Token op;
    } unary;
  };
} Node;

// Function declarations for creating different types of AST nodes.
Node *create_number_node(Token token, double value);
Node *create_char_node(Token token, const char *value);
Node *create_string_node(Token token, const char *value);
Node *create_boolean_node(Token token, bool value);
Node *create_null_node(Token token);
Node *create_binary_op_node(Token op, Node *left, Node *right);
Node *create_unary_op_node(Token op, Node *operand);
Node *create_postfix_op_node(Token op, Node *operand);

// Function declarations for managing the AST.
void free_node(Node *node);
void print_ast(Node *node);

#endif
