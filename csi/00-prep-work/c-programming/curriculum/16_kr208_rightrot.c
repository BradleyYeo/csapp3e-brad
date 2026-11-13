#include <stdio.h>

unsigned rightrot(unsigned x, int n);
int wordlength(void);
void print_binary(unsigned n);

int main() {
  int x = 55;
  int n = 10;
  print_binary(x);
  printf("\n");
  unsigned num = rightrot(x, n);
  print_binary(num);
}

unsigned rightrot(unsigned x, int n) {
  int wordlength();
  int rbit;

  while (n-- > 0) {
    rbit = (x & 1) << (wordlength() - 1);
    x = x >> 1;
    x = x | rbit;
  }
  return x;
}

void print_binary(unsigned n) {
  if (n > 1) {
    print_binary(n / 2);
  }
  printf("%d", n % 2);
}

int wordlength(void) {
  int i;
  unsigned v = (unsigned)~0;

  for (i = 1; (v = v >> 1) > 0; i++)
    ;
  return i;
}
