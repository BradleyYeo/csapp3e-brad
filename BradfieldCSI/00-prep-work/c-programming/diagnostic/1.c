#include <stdio.h>

void inspect(char arr[]) { printf("inside inspect: %zu\n", sizeof(arr)); }

int main(void) {
  char arr[] = "hello";
  char *ptr = "hello";

  printf("inside main: %zu\n", sizeof(arr));
  inspect(arr);

  arr[0] = 'H';
  // ptr[0] = 'H';

  return 0;
}
/*
Explain what sizeof(arr) outputs inside main versus inside inspect, and explain
the mechanism causing the difference.
6 (size of 5 characters + null terminator) and 4(size of pointer to an array of characters on the
stack)?


Explain what happens at runtime
    if ptr[0] = 'H' is executed,
       referencing the specific virtual memory segments where arr and ptr
reside. ptr immediately replaces hello with H


Explain the type and memory address difference between arr, &arr, and &arr[0]
arr is an array which containts ac

*/
