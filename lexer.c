#include <assert.h>
#include <ctype.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void free_tokens(struct TOKEN *token) {
  for (; token;) {
    struct TOKEN *old = token;
    token = token->next;
    free(old);
  }
}

int parse_alphastring(const char **code_ptr, struct TOKEN *cur) {
  const char *code = *code_ptr;
  if (!isalpha(*code))
    return 0;
  cur->type = TOKEN_ALPHA;
  int i = 0;
  for (; *code; code++, i++) {
    if (!isalpha(*code)) {
      break;
    }
    assert(i < 256);
    cur->string_rep[i] = *code;
  }
  cur->string_rep[i] = '\0';
  *code_ptr = code;
  return 1;
}

// Operands such as: &, &&, |, || etc
// Is operands the right word?
int parse_operand(const char **code_ptr, struct TOKEN *cur) {
  const char *code = *code_ptr;
  if (0 == strncmp(code, "&&", 2)) {
    cur->type = TOKEN_AND;
    strcpy(cur->string_rep, "&&");
    code += 2;
    goto complete_return;
  }
  if (0 == strncmp(code, "||", 2)) {
    cur->type = TOKEN_NOT;
    strcpy(cur->string_rep, "||");
    code += 2;
    goto complete_return;
  }
  if (0 == strncmp(code, "|", 1)) {
    cur->type = TOKEN_PIPE;
    strcpy(cur->string_rep, "|");
    code++;
    goto complete_return;
  }
  if (0 == strncmp(code, "&", 1)) {
    assert(0 && "TODO");
    code++;
    goto complete_return;
  }
  // Failed to parse
  return 0;

complete_return:
  *code_ptr = code;
  return 1;
}

void skip_whitespace(const char **code_ptr) {
  const char *code = *code_ptr;
  for (; isspace(*code); code++)
    ;
  *code_ptr = code;
}

struct TOKEN *lex(const char *code) {
  struct TOKEN *head = malloc(sizeof(struct TOKEN));
  struct TOKEN *cur = head;
  struct TOKEN *prev = NULL;
  cur->next = NULL;
  cur->type = TOKEN_NOOP;
  for (; *code;) {
    skip_whitespace(&code);
    if (parse_alphastring(&code, cur)) {
    } else if (parse_operand(&code, cur)) {
    } else {
      assert(0 && "Unknown token");
    }
    prev = cur;
    cur->next = malloc(sizeof(struct TOKEN));
    cur = cur->next;
    cur->type = TOKEN_NOOP;
    cur->next = NULL;
  }
  free(prev->next);
  prev->next = NULL;
  return head;
}
