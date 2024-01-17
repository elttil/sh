#include <assert.h>
#include <ctype.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <util.h>

void free_tokens(struct TOKEN *token) {
  for (; token;) {
    struct TOKEN *old = token;
    token = token->next;
    free(old);
  }
}

int is_nonspecial_char(char c) {
  if (!isprint(c)) {
    return 0;
  }
  if (isspace(c)) {
    return 0;
  }
  if (isalnum(c)) {
    return 1;
  }
  return ('>' != c && '|' != c && '&' != c && '(' != c && ')' != c && '{' != c && '}' != c);
}

int parse_chars(const char **code_ptr, struct TOKEN *cur) {
  const char *code = *code_ptr;
  if (!is_nonspecial_char(*code)) {
    return 0;
  }
  cur->type = TOKEN_CHARS;
  int i = 0;
  for (; *code; code++, i++) {
    if (!is_nonspecial_char(*code)) {
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
#define TRY_PARSE_STRING(_s, _token)                                           \
  if (0 == strncmp(code, _s, strlen(_s))) {                                    \
    cur->type = _token;                                                        \
    strcpy(cur->string_rep, _s);                                               \
    code += strlen(_s);                                                        \
    goto complete_return;                                                      \
  }
  TRY_PARSE_STRING("&&", TOKEN_AND);
  TRY_PARSE_STRING("||", TOKEN_NOT);
  TRY_PARSE_STRING(">>", TOKEN_STREAM_APPEND);
  TRY_PARSE_STRING(">", TOKEN_STREAM);
  TRY_PARSE_STRING("|", TOKEN_PIPE);
  TRY_PARSE_STRING("(", TOKEN_OPEN_PAREN);
  TRY_PARSE_STRING(")", TOKEN_CLOSE_PAREN);
  TRY_PARSE_STRING("{", TOKEN_OPEN_BRACKET);
  TRY_PARSE_STRING("}", TOKEN_CLOSE_BRACKET);
  // TODO: &

  // Failed to parse
  return 0;

complete_return:
  *code_ptr = code;
  return 1;
}

void skip_whitespace(const char **code_ptr) {
  const char *code = *code_ptr;
  for (; isspace(*code); code++) {
    ;
  }
  *code_ptr = code;
}

int chars_to_token(const char **code_ptr, struct TOKEN *token) {
  if (parse_chars(code_ptr, token)) {
    return 1;
  }
  if (parse_operand(code_ptr, token)) {
    return 1;
  }
  return 0;
}

struct TOKEN *lex(const char *code) {
  struct TOKEN *head = NULL;
  struct TOKEN *prev = NULL;
  for (; *code;) {
    skip_whitespace(&code);
    if (!*code) {
      break;
    }
    struct TOKEN *cur = xzmalloc(sizeof(struct TOKEN));
    cur->next = NULL;
    if (prev) {
      prev->next = cur;
    }
    if (!chars_to_token(&code, cur)) {
      free(cur);
      printf("at: %s\n", code);
      assert(0 && "Unknown token");
    }
    if (!head) {
      head = cur;
    }
    prev = cur;
  }
  return head;
}
