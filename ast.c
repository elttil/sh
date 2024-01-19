#include <assert.h>
#include <ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

int tokens_to_ast(struct TOKEN **token_ptr, struct AST *cur);

void free_ast_command(struct AST *ast) {
  free_ast(ast->children);
  free_ast(ast->pipe_rhs);
}

void free_ast(struct AST *ast) {
  for (; ast;) {
    if (AST_COMMAND == ast->type) {
      free_ast_command(ast);
    }
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
  if (TOKEN_CHARS != token->type) {
    return 0;
  }
  cur->type = AST_COMMAND;
  cur->val.type = AST_VALUE_STRING;
  cur->val.string = token->string_rep;
  // Parse the arguments
  if (token_next_nonewhite(token) && TOKEN_CHARS == token_next_nonewhite(token)->type) {
    token = token_next_nonewhite(token);
    cur->children = allocate_ast();
    struct AST *child = cur->children;
    for (;;) {
      child->type = AST_EXPRESSION;
      child->val.type = AST_VALUE_STRING;
      child->val.string = token->string_rep;
      if (!token_next_nonewhite(token)) {
        break;
      }
      if (TOKEN_CHARS != token_next_nonewhite(token)->type) {
        break;
      }
      token = token_next_nonewhite(token);
      child->next = allocate_ast();
      child = child->next;
    }
  }
  token = token_next_nonewhite(token);
  // Parse the stream modifier "prog > file.txt"
  if (token &&
      (TOKEN_STREAM == token->type || TOKEN_STREAM_APPEND == token->type)) {
    cur->file_out_append = (TOKEN_STREAM_APPEND == token->type);
    // TODO: Allow it to be modified
    cur->file_out_fd_to_use = STDOUT_FILENO;
    token = token_next_nonewhite(token);
    cur->file_out = token->string_rep;
    token = token_next_nonewhite(token);
  }
  // Parse pipe '|'
  if (token && TOKEN_PIPE == token->type) {
    cur->pipe_rhs = allocate_ast();
    token = token_next_nonewhite(token);
    if (!parse_command(&token, cur->pipe_rhs)) {
      fprintf(stderr, "Expected command after |.");
      exit(1);
    }
  }
  *token_ptr = token;
  return 1;
}

#define EXPECT_TOKEN(_tok)                                                     \
  {                                                                            \
    fprintf(stderr, "Expected %s\n", _tok);                                    \
    exit(1);                                                                   \
  }

int parse_if_statement(struct TOKEN **token_ptr, struct AST *cur) {
  struct TOKEN *token = *token_ptr;
  if (TOKEN_CHARS != token->type) {
    return 0;
  }
  if (0 != strcmp("if", token->string_rep)) {
    return 0;
  }

  cur->type = AST_IF_STATEMENT;

  token = token_next_nonewhite(token);
  if (!token || TOKEN_OPEN_PAREN != token->type) {
    EXPECT_TOKEN("(");
  }
  token = token_next_nonewhite(token);

  cur->condition = allocate_ast();
  int rc = tokens_to_ast(&token, cur->condition);
  if (!rc) {
    fprintf(stderr, "Failed to parse condition in 'if' statement.\n");
    exit(1);
  }

  if (!token || TOKEN_CLOSE_PAREN != token->type) {
    EXPECT_TOKEN(")");
  }
  token = token_next_nonewhite(token);
  if (!token || TOKEN_OPEN_BRACKET != token->type) {
    EXPECT_TOKEN("{");
  }
  token = token_next_nonewhite(token);

  cur->children = NULL;
  struct AST *previous = NULL;
  // Loop until closing bracket
  for (;;) {

    if (!token) {
      fprintf(stderr, "Expected } before end of expression.\n");
      exit(1);
    }

    if (TOKEN_CLOSE_BRACKET == token->type) {
      token = token_next_nonewhite(token);
      break;
    }

    struct AST *a = allocate_ast();
    int rc = tokens_to_ast(&token, a);
    if (!rc) {
      fprintf(stderr, "Expected } before end of expression.\n");
      exit(1);
    }

    if (NULL == previous) {
      cur->children = a;
    } else {
      previous->next = a;
    }
    previous = a;
  }

  *token_ptr = token;
  return 1;
}

int tokens_to_ast(struct TOKEN **token_ptr, struct AST *cur) {
  struct TOKEN *token = *token_ptr;
  if (parse_if_statement(&token, cur)) {
    goto tokens_to_ast_success;
  }
  if (parse_command(&token, cur)) {
    goto tokens_to_ast_success;
  }
  if (TOKEN_AND == token->type) {
    cur->type = AST_CONDITIONAL_AND;
    token = token_next_nonewhite(token);
    goto tokens_to_ast_success;
  }
  if (TOKEN_NOT == token->type) {
    cur->type = AST_CONDITIONAL_NOT;
    token = token_next_nonewhite(token);
    goto tokens_to_ast_success;
  }
  // Semicolon and newlines are treated as a conditional such as && and
  // || except that it does not care about the return value. This means
  // it can use the same logic of seperating commands that && and ||
  // does.
  if (TOKEN_SEMICOLON == token->type || TOKEN_NEWLINE == token->type) {
    cur->type = AST_NOOP;
    token = token_next_nonewhite(token);
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
    if (prev) {
      prev->next = cur;
    }
    if (!tokens_to_ast(&token, cur)) {
      assert(0 && "Unexpected token");
    }
    if (!head) {
      head = cur;
    }
    prev = cur;
  }
  return head;
}
