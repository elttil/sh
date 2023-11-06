#include <ast.h>
#include <fcntl.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <test.h>
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

  int file_out_fd;
  if (ast->file_out) {
    file_out_fd =
        open(ast->file_out,
             O_WRONLY | O_CREAT | ((ast->file_out_append) ? O_APPEND : O_TRUNC),
             0666);
  }

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
    if (ast->file_out)
      dup2(file_out_fd, ast->file_out_fd_to_use);

    execvp(program, argv);
    exit(1);
  }
  if (ast->file_out)
    close(file_out_fd);

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
    } else if (AST_CONDITIONAL_AND == ast->type) {
      if (0 != rc) {
        ast = ast->next;
        if (!ast)
          break;
      }
    } else if (AST_CONDITIONAL_NOT == ast->type) {
      if (0 == rc) {
        ast = ast->next;
        if (!ast)
          break;
      }
    }
    ast = ast->next;
  }
}

int main(int argc, char **argv) {
  if (argc >= 1) {
    char *p = argv[0];
    for (; *p; p++)
      ;
    p--;
    for (; *p && *p != '/'; p--)
      ;
    if (!*p)
      return 1;
    p++;
    if (0 == strcmp("shelltest", p)) {
      printf("Running internal shell test\n");
      if (internal_shelltest()) {
        printf("Success\n");
        return 0;
      }
      printf("Failed\n");
      return 1;
    }
  }

  struct TOKEN *h = lex("echo test | cat");
  struct AST *ast_h = generate_ast(h);
  execute_ast(ast_h);
  free_tokens(h);
  free_ast(ast_h);
  return 0;
}
