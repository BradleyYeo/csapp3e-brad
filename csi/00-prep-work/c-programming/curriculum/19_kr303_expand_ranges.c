/* K&R 3-3: Write a function expand(s1,s2) that expands shorthand
 * notations like a-z in the string s1 into the equivalent complete list
 * stored in s2. Allow for letters of either case and digits, and be
 * prepared to handle cases like a-b-c and a-z0-9 and -a. Arrange that a
 * leading or trailing '-' is taken literally.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 1000

/*
 * expand(s1, s2): expands every ascending range x-y in s1 into the
 * characters x..y, copying everything else verbatim into s2.
 * Input:  s1 - shorthand pattern, e.g. "a-z0-9".
 * Output: none; s2 holds the expansion, NUL-terminated. The caller
 *         sizes s2 large enough (MAX_LENGTH covers the drills below).
 * Example: expand("a-c1-3", s2) -> s2 == "abc123"
 */
void expand(char s1[], char s2[]) {
  (void)s1;
  s2[0] = '\0'; /* TODO: implement */
}

int main() {
  char s2[MAX_LENGTH];

  expand("a-z0-9", s2);
  assert(strcmp(s2, "abcdefghijklmnopqrstuvwxyz0123456789") == 0);

  expand("0-9", s2);
  assert(strcmp(s2, "0123456789") == 0);

  expand("A-F", s2);
  assert(strcmp(s2, "ABCDEF") == 0);

  expand("a-c1-3", s2);
  assert(strcmp(s2, "abc123") == 0);

  expand("hello", s2);
  assert(strcmp(s2, "hello") == 0);

  expand("a", s2);
  assert(strcmp(s2, "a") == 0);

  expand("", s2);
  assert(strcmp(s2, "") == 0);

  printf("[PASS] Exercise 18: expand ranges verified.\n");
  return EXIT_SUCCESS;
}
