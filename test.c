#include <lexer.h>

int internal_shelltest(void) {
#define TOKEN_MATCH(_t)                                                        \
  {                                                                            \
    if (_t != token->type)                                                     \
      return 0;                                                                \
    token = token->next;                                                       \
  }
  struct TOKEN *token =
      lex("echo test>>testfile||echo foo > testfile | echo bar > zoo");
  struct TOKEN *head = token;
  TOKEN_MATCH(TOKEN_ALPHA);         // echo
  TOKEN_MATCH(TOKEN_ALPHA);         // test
  TOKEN_MATCH(TOKEN_STREAM_APPEND); // >>
  TOKEN_MATCH(TOKEN_ALPHA);         // testfile
  TOKEN_MATCH(TOKEN_NOT);           // ||
  TOKEN_MATCH(TOKEN_ALPHA);         // echo
  TOKEN_MATCH(TOKEN_ALPHA);         // foo
  TOKEN_MATCH(TOKEN_STREAM);        // >
  TOKEN_MATCH(TOKEN_ALPHA);         // testfile
  TOKEN_MATCH(TOKEN_PIPE);          // |
  TOKEN_MATCH(TOKEN_ALPHA);         // echo
  TOKEN_MATCH(TOKEN_ALPHA);         // bar
  TOKEN_MATCH(TOKEN_STREAM);        // >
  TOKEN_MATCH(TOKEN_ALPHA);         // zoo
  if (token)                        // End of tokens
    return 0;
  free_tokens(head);
  return 1;
}
