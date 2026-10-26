#include <stdlib.h>

void expand_buffer(int *buf, size_t *capacity) {
  size_t new_cap = (*capacity) * 2;
  int *temp = realloc(buf, new_cap * sizeof(int));
  if (temp != NULL) {
    buf = temp;
    *capacity = new_cap;
  }
}

/*
Explain why the caller's pointer to the buffer remains unchanged after calling
expand_buffer. Explain how the function signature must be changed to update the
caller's pointer correctly. Explain why directly assigning buf = realloc(buf,
new_cap) without a temporary pointer introduces a memory leak vulnerability.
*/