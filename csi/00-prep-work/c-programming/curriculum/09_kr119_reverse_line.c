/* K&R 1-19: Write a function reverse(s) that reverses the character string s
 * in place.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * reverse: reverses the NUL-terminated string `s` in place.
 */
void reverse(char s[]) {
  (void)s;
  /* TODO: implement */
}

int main(void) {
  char s1[] = "hello";
  reverse(s1);
  assert(strcmp(s1, "olleh") == 0);

  char s2[] = "abcd";
  reverse(s2);
  assert(strcmp(s2, "dcba") == 0);

  char s3[] = "a";
  reverse(s3);
  assert(strcmp(s3, "a") == 0);

  char s4[] = "";
  reverse(s4);
  assert(strcmp(s4, "") == 0);

  char s5[] = "race car";
  reverse(s5);
  assert(strcmp(s5, "rac ecar") == 0);

  printf("[PASS] Exercise 09: reverse line verified.\n");
  return EXIT_SUCCESS;
}
