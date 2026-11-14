/* K&R 2-6: Write a function setbits(x, p, n, y) that returns x with the n bits
 * that begin at position p set to the rightmost n bits of y, leaving the other
 * bits unchanged. (Position 0 is the least-significant bit.)
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int setbits(unsigned int x, int p, int n, unsigned int y) {
  (void)x;
  (void)p;
  (void)n;
  (void)y;
  /* TODO: implement using unsigned bit shifts */
  return 0U;
}

int main(void) {
  assert(setbits(0xCAU, 4, 3, 0xB3U) == 0xCEU); /* 11001010 -> 11001110 */
  assert(setbits(0xFFU, 3, 4, 0x05U) == 0xF5U);
  assert(setbits(0x00U, 7, 4, 0x0AU) == 0xA0U);
  assert(setbits(0x1234U, 0, 1, 0x01U) == 0x1235U);

  printf("[PASS] Exercise 15: setbits verified.\n");
  return EXIT_SUCCESS;
}
