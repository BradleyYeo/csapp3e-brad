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
void fold_lines(const char *in, char *out, size_t out_cap, size_t maxcol) {
  (void)in;
  (void)maxcol;
  if (out_cap > 0) {
    out[0] = '\0';
  }
  /* TODO: implement */
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

  printf("[PASS] Exercise 11: fold lines verified.\n");
  return EXIT_SUCCESS;
}
