#include <assert.h>
#include <ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

void free_ast_command(struct AST *ast) {
  free_ast(ast->children);
  free_ast(ast->pipe_rhs);
}

void free_ast(struct AST *ast) {
  for (; ast;) {
    if (AST_COMMAND == ast->type)
      free_ast_command(ast);
    struct AST *old = ast;
    ast = ast->next;
    free(old);
  }
}

struct AST *allocate_ast(void) {
  return xzmalloc(sizeof(struct AST));
}

int parse_command(struct TOKEN **token_ptr, struct AST *cur) {
  struct TOKEN *token = *token_ptr;
  if (TOKEN_CHARS != token->type)
    return 0;
  cur->type = AST_COMMAND;
  cur->val.type = AST_VALUE_STRING;
  cur->val.string = token->string_rep;
  // Parse the arguments
  if (token->next && TOKEN_CHARS == token->next->type) {
    token = token->next;
    cur->children = allocate_ast();
    struct AST *child = cur->children;
    for (;;) {
      child->type = AST_EXPRESSION;
      child->val.type = AST_VALUE_STRING;
      child->val.string = token->string_rep;
      if (!token->next)
        break;
      if (TOKEN_CHARS != token->next->type)
        break;
      token = token->next;
      child->next = allocate_ast();
      child = child->next;
    }
  }
  token = token->next;
  // Parse the stream modifier "prog > file.txt"
  if (token &&
      (TOKEN_STREAM == token->type || TOKEN_STREAM_APPEND == token->type)) {
    cur->file_out_append = (TOKEN_STREAM_APPEND == token->type);
    // TODO: Allow it to be modified
    cur->file_out_fd_to_use = STDOUT_FILENO;
    token = token->next;
    cur->file_out = token->string_rep;
    token = token->next;
  }
  // Parse pipe '|'
  if (token && TOKEN_PIPE == token->type) {
    cur->pipe_rhs = allocate_ast();
    token = token->next;
    if (!parse_command(&token, cur->pipe_rhs)) {
      fprintf(stderr, "Expected command after |.");
      exit(1);
    }
  }
  *token_ptr = token;
  return 1;
}

int tokens_to_ast(struct TOKEN **token_ptr, struct AST *cur) {
  struct TOKEN *token = *token_ptr;
  if (parse_command(&token, cur)) {
    goto tokens_to_ast_success;
  }
  if (TOKEN_AND == token->type) {
    cur->type = AST_CONDITIONAL_AND;
    token = token->next;
    goto tokens_to_ast_success;
  }
  if (TOKEN_NOT == token->type) {
    cur->type = AST_CONDITIONAL_NOT;
    token = token->next;
    goto tokens_to_ast_success;
  }
  return 0;
tokens_to_ast_success:
  *token_ptr = token;
  return 1;
}

struct AST *generate_ast(struct TOKEN *token) {
  struct AST *head = NULL;
  struct AST *prev = NULL;
  for (; token;) {
    struct AST *cur = allocate_ast();
    if (prev)
      prev->next = cur;
    if (!tokens_to_ast(&token, cur)) {
      assert(0 && "Unexpected token");
    }
    if (!head)
      head = cur;
    prev = cur;
  }
  return head;
}
