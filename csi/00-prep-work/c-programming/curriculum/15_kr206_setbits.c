#include <stdio.h>

unsigned setbits(unsigned x, int p, int n, unsigned y) {
  return x & ~(((1 << n) - 1) << (p + 1 - n)) |
         ((y & ((1 << n) - 1)) << (p + 1 - n));
}

void print_binary(unsigned n) {
  for (int i = sizeof(n) * 8 - 1; i >= 0; i--) {
    putchar((n & (1 << i)) ? '1' : '0');
  }
  putchar('\n');
}

int main() {
  unsigned x = 202; // 1110 in binary
  int p = 4;
  int n = 3;
  unsigned y = 179; // 1001 in binary

  unsigned result = setbits(x, p, n, y);

  printf("x in binary: ");
  print_binary(x);

  printf("y in binary: ");
  print_binary(y);

  printf("Result in binary: ");
  print_binary(result);

  printf("Result in decimal: %u\n", result);

  return 0;
}
