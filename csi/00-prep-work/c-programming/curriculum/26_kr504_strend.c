/* K&R 5-4: write the function strend(s,t), which returns 1 if the string
 * t occurs at the end of the string s, and zero otherwise.
 * Use pointer arithmetic only - no [] subscripting anywhere.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * strend(s, t): tests whether t is a suffix of s.
 * Input:  s, t - NUL-terminated strings (advanced via local pointers
 *         only; neither string is modified).
 * Output: 1 if t occurs at the very end of s, 0 otherwise; 0 when t is
 *         longer than s.
 * Example: strend("hello", "lo") -> 1, strend("hello", "he") -> 0
 */
int strend(char *s, char *t) {
  (void)s;
  (void)t;
  return 0; /* TODO: implement */
}

int main() {
  char s[] = "hello";
  char a[] = "a";
  char e[] = "";

  assert(strend(s, "lo") == 1);
  assert(strend(s, "o") == 1);
  assert(strend(s, "llo") == 1);
  assert(strend(s, "hello") == 1);
  assert(strend(s, "he") == 0);
  assert(strend(s, "xllo") == 0);
  assert(strend(s, "helllo") == 0);
  assert(strend(a, "a") == 1);
  assert(strend(a, "b") == 0);
  assert(strend(e, "x") == 0);

  printf("[PASS] Exercise 26: strend verified.\n");
  return EXIT_SUCCESS;
}
