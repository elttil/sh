#include <ast.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_command(struct AST *ast, int input_fd) {
  char *program = ast->val.string;
  struct AST *child = ast->children;
  char *argv[100];
  argv[0] = program;
  int i = 1;
  for (; child; i++, child = child->next) {
    argv[i] = child->val.string;
  }
  argv[i] = NULL;

  int in = input_fd;
  int out = STDOUT_FILENO;
  int slave_input = -1;
  if (ast->pipe_rhs) {
    int fds[2];
    pipe(fds);
    out = fds[1];
    slave_input = fds[0];
  }

  int pid = fork();
  if (0 == pid) {
    if (slave_input >= 0)
      close(slave_input);
    dup2(in, STDIN_FILENO);
    dup2(out, STDOUT_FILENO);
    execvp(program, argv);
    exit(1);
  }

  if (ast->pipe_rhs) {
    if (out >= 0)
      close(out);
    return execute_command(ast->pipe_rhs, slave_input);
  }
  int rc;
  waitpid(pid, &rc, 0);
  return rc;
}

void execute_ast(struct AST *ast) {
  int rc = -1;
  for (; ast;) {
    if (AST_COMMAND == ast->type) {
      rc = execute_command(ast, STDIN_FILENO);
      ast = ast->next;
      continue;
    } else if (AST_CONDITIONAL_AND == ast->type) {
      if (rc != 0) {
        ast = ast->next;
        if (!ast)
          break;
      }
      ast = ast->next;
      continue;
    } else if (AST_CONDITIONAL_NOT == ast->type) {
      if (rc == 0) {
        ast = ast->next;
        if (!ast)
          break;
      }
      ast = ast->next;
      continue;
    }
  }
}

int main(void) {
  struct TOKEN *h = lex("echo test | cat");
  struct AST *ast_h = generate_ast(h);
  execute_ast(ast_h);
  free_tokens(h);
  free_ast(ast_h);
  return 0;
}
