#include "stdio.h"

int main(void) {
  int x = 30000000000;
  printf("%zu", sizeof(x));
  printf("%d", x);
}