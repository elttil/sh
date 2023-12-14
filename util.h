#include <stddef.h>

// Returns a pointer to a allocated region that is zero initalized. The
// pointer can be passed to free().
void *xzmalloc(size_t size);
