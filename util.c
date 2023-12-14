#include <stdio.h>
#include <stdlib.h>
#include <util.h>

void *xzmalloc(size_t size) {
  void *r = calloc(1, size);
  // NOTE: On linux and other modern operating systems a call to malloc
  // can not fail unless the size specified is too large to fit inside
  // the virtual memory region. But imo it is better to follow the
  // outcome expected from the standard if possible.
  if (NULL == r) {
    perror("calloc");
    exit(1);
  }
  return r;
}
