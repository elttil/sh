#include <lexer.h>

typedef enum {
  AST_VALUE_STRING,
} ast_value_type_t;

struct AST_VALUE {
  ast_value_type_t type;
  union {
    char *string;
  };
};

typedef enum {
  AST_COMMAND,
  AST_EXPRESSION,
  AST_CONDITIONAL_AND,
  AST_CONDITIONAL_NOT,
} ast_type_t;

struct AST {
  ast_type_t type;
  struct AST_VALUE val;
  struct AST *children;
  struct AST *pipe_rhs; // in "func1 | func2" func2 is the piped rhs
  struct AST *next;
};

void free_ast(struct AST *ast);
struct AST *generate_ast(struct TOKEN *token);
