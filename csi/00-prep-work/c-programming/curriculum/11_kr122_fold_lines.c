/* K&R 1-22: Write a program to "fold" long input lines into two or more
 * shorter lines after the last non-blank character that occurs before the
 * n-th column of input. Handle very long lines and no-blank runs gracefully.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * fold_lines: wraps `in` at `maxcol` columns into `out` (up to `out_cap`
 * bytes, NUL-terminated). Breaks after the last blank before `maxcol` when one
 * exists; otherwise hard-wraps at `maxcol`.
 */
static void append(char *out, size_t out_cap, size_t *pos, char ch){
  if (*pos + 1 < out_cap){
    out[*pos] = ch;
    (*pos)++;
  }
}

static void copy_all(const char *in, char *out, size_t out_cap){
   for (size_t i = 0; in[i] != '\0'; i++){
     append(out, out_cap, &in, in[i]);
   }
}

void fold_lines(const char *in, char *out, size_t out_cap, size_t maxcol) {
  if (out_cap > 0) {
    out[0] = '\0';
  }

  if (in == NULL){
    return;
  }

  if (out_cap == 0) {
    return;
  }

  size_t pos = 0;
  size_t col_count = 0;
  size_t last_blank = 0;
  for (size_t i = 0; in[i] != '\0'; i++) {
    if (in[i] == ' '){
      last_blank = i;
    }

    append(out, out_cap, &pos, '\n');
    
  }
  append(out, out_cap, &pos, '\n');
}

int main(void) {
  char out[256];

  fold_lines("short\n", out, sizeof(out), 10);
  assert(strcmp(out, "short\n") == 0);

  fold_lines("hello world!\n", out, sizeof(out), 10);
  assert(strcmp(out, "hello\nworld!\n") == 0);

  fold_lines("abcdefghijkl\n", out, sizeof(out), 5);
  assert(strcmp(out, "abcde\nfghij\nkl\n") == 0);

  fold_lines("", out, sizeof(out), 10);
  assert(strcmp(out, "") == 0);

  char small[6];
  fold_lines("hello world!\n", small, sizeof(small), 10);
  assert(strcmp(small, "hello") == 0);

  fold_lines("hello\n", NULL, 10, 10);
  fold_lines("hello\n", out, 0, 10);

  fold_lines(NULL, out, sizeof(out), 10);
  assert(strcmp(out, "") == 0);

  fold_lines("hello\n", out, sizeof(out), 0);
  assert(strcmp(out, "hello\n") == 0);

  printf("[PASS] Exercise 11: fold lines verified.\n");
  return EXIT_SUCCESS;
}
