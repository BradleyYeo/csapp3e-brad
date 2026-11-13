#include <stdio.h>

/* verify value of expression getchar() != EOF */
int main() {
  int c;
  while ((c = (getchar() != EOF))) {
    printf("Value is %d\n", c);
  }
  printf("Value is %d\n", c);

  return 0;
}
