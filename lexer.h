#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>

typedef enum {
  TOKEN_WHITESPACE,
  TOKEN_CHARS,
  TOKEN_AND,
  TOKEN_NOT,
  TOKEN_NOOP,
  TOKEN_PIPE,
  TOKEN_SEMICOLON,
  TOKEN_NEWLINE,
  TOKEN_STREAM,
  TOKEN_STREAM_APPEND,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACKET,
  TOKEN_CLOSE_BRACKET,
} token_type_t;

struct TOKEN {
  token_type_t type;
  char string_rep[256];
  struct TOKEN *next;
};

struct TOKEN *token_next_nonewhite(const struct TOKEN *token);
struct TOKEN *lex(const char *code);
struct AST *generate_ast(struct TOKEN *token);
void free_tokens(struct TOKEN *token);
#endif // LEXER_H
