/* K&R 1-20: Write a function detab that replaces tabs in the input with the
 * proper number of blanks to space to the next tab stop. Assume a fixed set of
 * tab stops, every `tabstop` columns.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { DEFAULT_TABSTOP = 8 };

/*
 * detab: copies `in` to `out` (up to `out_cap` bytes, NUL-terminated),
 * replacing each '\t' with spaces up to the next `tabstop` column boundary.
 * '\n' resets the column counter to 0.
 */
static void append_char(char *out, size_t out_cap, size_t *pos, char ch) {
  if (*pos + 1 < out_cap) {
    out[*pos] = ch;
    (*pos)++;
  }
}

void detab(const char *in, char *out, size_t out_cap, int tabstop) {
  if (out == NULL || out_cap == 0) {
    return;
  }
  if (in == NULL) {
    out[0] = '\0';
    return;
  }
  if (tabstop <= 0) {
    tabstop = DEFAULT_TABSTOP;
  }
  int col = 0;
  size_t pos = 0;
  while (*in != '\0') {
    if (*in == '\t') {
      int spaces = tabstop - (col % tabstop);
      for (int i = 0; i < spaces; i++) {
        col++;
        append_char(out, out_cap, &pos, ' ');
      }
    } else if (*in == '\n') {
      append_char(out, out_cap, &pos, '\n');
      col = 0;
    } else {
      append_char(out, out_cap, &pos, *in);
      col++;
    }
    in++;
  }
  out[pos] = '\0';
}

int main(void) {
  char out[256];

  detab("\t", out, sizeof(out), DEFAULT_TABSTOP);
  printf("actual out=[%s]\n", out);
  assert(strcmp(out, "        ") == 0);

  detab("ab\tc", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "ab      c") == 0);

  detab("12345678\tx", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "12345678        x") == 0);

  detab("ab\ncd\te", out, sizeof(out), 4);
  assert(strcmp(out, "ab\ncd  e") == 0);

  detab("", out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "") == 0);

  char small[5];
  detab("\t", small, sizeof(small), DEFAULT_TABSTOP);
  assert(strcmp(small, "    ") == 0);

  detab("\t", NULL, 10, DEFAULT_TABSTOP);
  detab("\t", out, 0, DEFAULT_TABSTOP);

  detab(NULL, out, sizeof(out), DEFAULT_TABSTOP);
  assert(strcmp(out, "") == 0);

  detab("\t", out, sizeof(out), 0);
  assert(strcmp(out, "        ") == 0);

  printf("[PASS] Exercise 10: detab verified.\n");
  return EXIT_SUCCESS;
}
