/* K&R 2-3: Write a function htoi(s) that converts a string of hexadecimal
 * digits (including an optional 0x or 0X prefix) into its equivalent integer
 * value.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * htoi: converts hexadecimal string `s` (optional "0x" / "0X" prefix,
 * digits '0'-'9', 'a'-'f', 'A'-'F') to its integer value.
 */
int htoi(const char *s) {
  (void)s;
  /* TODO: implement */
  return -1;
}

int main(void) {
  assert(htoi("AAF41") == 700225);
  assert(htoi("0x1f") == 31);
  assert(htoi("0XFF") == 255);
  assert(htoi("0") == 0);
  assert(htoi("deadBEEF") == (int)0xdeadbeef);

  printf("[PASS] Exercise 14: htoi verified.\n");
  return EXIT_SUCCESS;
}
