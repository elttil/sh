#include <lexer.h>
#include <stdio.h>

int internal_shelltest(void) {
#define TOKEN_MATCH(_t)                                                        \
  {                                                                            \
    if (_t != token->type) {                                                   \
      fprintf(stderr, "Test failed at check: %d\n", i);                        \
      return 0;                                                                \
    }                                                                          \
    token = token->next;                                                       \
    i++;                                                                       \
  }
  struct TOKEN *token =
      lex("echo test>>testfile||echo foo > testfile | echo bar > zoo");
  int i = 0;
  struct TOKEN *head = token;
  TOKEN_MATCH(TOKEN_CHARS);         // echo
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // test
  TOKEN_MATCH(TOKEN_STREAM_APPEND); // >>
  TOKEN_MATCH(TOKEN_CHARS);         // testfile
  TOKEN_MATCH(TOKEN_NOT);           // ||
  TOKEN_MATCH(TOKEN_CHARS);         // echo
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // foo
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_STREAM);        // >
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // testfile
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_PIPE);          // |
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // echo
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // bar
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_STREAM);        // >
  TOKEN_MATCH(TOKEN_WHITESPACE);    // 
  TOKEN_MATCH(TOKEN_CHARS);         // zoo
  if (token)                        // End of tokens
    return 0;
  free_tokens(head);
  return 1;
}
