/* K&R 2-1: Determine the ranges of signed/unsigned char, short, int, and long
 * by direct computation (without signed-overflow UB) and verify against
 * <limits.h>.
 */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char computed_uchar_max(void) {
  /* TODO: compute using bitwise / unsigned arithmetic without <limits.h> */
  return 0;
}

signed char computed_schar_max(void) {
  /* TODO: compute without signed overflow UB */
  return 0;
}

signed char computed_schar_min(void) {
  /* TODO: compute without signed overflow UB */
  return 0;
}

unsigned int computed_uint_max(void) {
  /* TODO: compute using unsigned arithmetic without <limits.h> */
  return 0U;
}

int computed_int_max(void) {
  /* TODO: compute without signed overflow UB */
  return 0;
}

int computed_int_min(void) {
  /* TODO: compute without signed overflow UB */
  return 0;
}

int main(void) {
  assert(computed_uchar_max() == UCHAR_MAX);
  assert(computed_schar_max() == SCHAR_MAX);
  assert(computed_schar_min() == SCHAR_MIN);
  assert(computed_uint_max() == UINT_MAX);
  assert(computed_int_max() == INT_MAX);
  assert(computed_int_min() == INT_MIN);

  printf("[PASS] Exercise 13: type ranges verified.\n");
  return EXIT_SUCCESS;
}
