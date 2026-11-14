/* K&R 2-8: Write a function rightrot(x, n) that returns the value of the
 * integer x rotated to the right by n positions.
 */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int rightrot(unsigned int x, int n) {
  (void)x;
  (void)n;
  /* TODO: implement without UB on shift >= word width */
  return 0U;
}

int main(void) {
  constexpr int width = (int)(sizeof(unsigned int) * CHAR_BIT);

  assert(rightrot(0x12345678U, 0) == 0x12345678U);
  assert(rightrot(0x12345678U, width) == 0x12345678U);
  assert(rightrot(0x00000001U, 1) == (1U << (width - 1)));
  assert(rightrot(0x80000001U, 1) == ((1U << (width - 1)) | 0x40000000U));
  assert(rightrot(0x12345678U, 4) == 0x81234567U);

  printf("[PASS] Exercise 16: rightrot verified.\n");
  return EXIT_SUCCESS;
}
