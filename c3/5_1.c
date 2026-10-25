#include <stdio.h>
#include <stdlib.h>
/* Swap value x at xp with value y at yp */
void swap(long *xp, long *yp) {
   *xp = *xp + *yp; /* x+y *xp = 10*/
   *yp = *xp - *yp; /* x+y-y = x  yp = 10-10 = 0*/
   *xp = *xp - *yp; /* x+y-x = y xp = 0 = 0-0 */
}

static void test_swap(long a, long b) {
  printf("Input:  a = %ld, b = %ld\n", a, b);
  swap(&a, &b);
  printf("Output: a = %ld, b = %ld\n\n", a, b);
}

int main(void) {
  /* Pointer aliasing test case */
  long x = 5;
  printf("Input:  x = %ld (aliased: &x passed to both params)\n", x);
  swap(&x, &x);
  printf("Output: x = %ld\n", x);

  return 0;
}
