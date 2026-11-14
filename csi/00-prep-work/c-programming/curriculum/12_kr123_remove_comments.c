/* K&R 1-23: Write a function to remove all comments from a C program.
 * Don't forget to handle quoted strings and character constants properly.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * remove_comments: copies C source from `in` into `out` (up to `out_cap`
 * bytes, NUL-terminated), stripping both block and line comments while
 * preserving string literals ("...") and character constants ('...'),
 * including escaped quotes.
 */
void remove_comments(const char *in, char *out, size_t out_cap) {
  (void)in;
  if (out_cap > 0) {
    out[0] = '\0';
  }
  /* TODO: implement */
}

int main(void) {
  char out[512];

  remove_comments("int x = 1; /* comment */\n", out, sizeof(out));
  assert(strcmp(out, "int x = 1; \n") == 0);

  remove_comments("int y = 2; // line comment\nint z = 3;\n", out, sizeof(out));
  assert(strcmp(out, "int y = 2; \nint z = 3;\n") == 0);

  remove_comments("const char *s = \"/* not a comment */ // still string\";\n",
                  out, sizeof(out));
  assert(strcmp(out, "const char *s = \"/* not a comment */ // still string\";\n") == 0);

  remove_comments("const char *q = \"escaped \\\" /* keep */\";\n", out,
                  sizeof(out));
  assert(strcmp(out, "const char *q = \"escaped \\\" /* keep */\";\n") == 0);

  remove_comments("char c = '/'; /* real comment */\n", out, sizeof(out));
  assert(strcmp(out, "char c = '/'; \n") == 0);

  printf("[PASS] Exercise 12: remove comments verified.\n");
  return EXIT_SUCCESS;
}
