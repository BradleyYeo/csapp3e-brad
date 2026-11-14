/* K&R 1-20: Write a function detab that replaces tabs in the input with the
 * proper number of blanks to space to the next tab stop. Assume a fixed set of
 * tab stops, every `tabstop` columns.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr int DEFAULT_TABSTOP = 8;

/*
 * detab: copies `in` to `out` (up to `out_cap` bytes, NUL-terminated),
 * replacing each '\t' with spaces up to the next `tabstop` column boundary.
 * '\n' resets the column counter to 0.
 */
void detab(const char *in, char *out, size_t out_cap, int tabstop) {
  (void)in;
  (void)tabstop;
  if (out_cap > 0) {
    out[0] = '\0';
  }
  /* TODO: implement */
}

int main(void) {
  char out[256];

  detab("\t", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "        ") == 0);

  detab("ab\tc", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "ab      c") == 0);

  detab("12345678\tx", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "12345678        x") == 0);

  detab("ab\ncd\te", out, sizeof(out), 4);
  assert(strcmp(out, "ab\ncd  e") == 0);

  detab("", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "") == 0);

  printf("[PASS] Exercise 10: detab verified.\n");
  return EXIT_SUCCESS;
}
