include<stdio.h>

    void test(char *str) {
  printf("%c", str);
}

int main() {
  char *first = "Ethan";
  char *second = "Godt";
  char third[6] = {'h', 'e', 'l', 'l', 'o', '\0'};
  char *strings[3] = {first, second, third};

  printf("first: %p\n", first);   // not actually a pointer
  printf("&first: %p\n", &first); // not actually a pointer

  printf("strings address: %p\n", strings);    // not actually a pointer
  printf("strings address 2: %p\n", &strings); // what address is being returned

  printf("first address: %p\n", strings[0]);
  printf("first pointer address: %p\n", &strings[0]);

  printf("second address: %p\n", strings[1]);
  printf("second pointer address: %p\n", &strings[1]);

  printf("third address: %p\n", strings[2]);
  printf("third pointer address: %p\n", &strings[2]);

  printf("%p\n", strings[2]);

  strings[1] = third;
  strings[2] = second;
}
