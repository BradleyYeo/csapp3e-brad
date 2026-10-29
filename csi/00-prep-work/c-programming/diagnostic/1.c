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
6 (size of 5 characters + null terminator) and 8(size of pointer is 8 bytes in
64 bit address space to an array of characters on the stack)?


Explain what happens at runtime
    if ptr[0] = 'H' is executed,
       referencing the specific virtual memory segments where arr and ptr
reside. Answer ("ptr immediately replaces hello with H"): Incorrect.
Executing ptr[0] = 'H' does not modify the string; it crashes the program
immediately with a Segmentation fault (SIGSEGV). Mechanical Breakdown char arr[]
= "hello";: Allocates 6 bytes in the current stack frame. Copies the string data
from binary storage into the stack memory at runtime. The stack memory segment
is mapped with Read-Write permissions (PROT_READ | PROT_WRITE). Executing arr[0]
= 'H' succeeds because writes to the stack frame are valid. char *ptr =
"hello";: Allocates an 8-byte pointer on the stack frame. Initializes the
pointer with the address of the literal "hello". String literals are placed by
the compiler and linker into the .rodata (read-only data) ELF section / text
segment. The operating system kernel maps the .rodata page in the process page
tables with Read-Only permissions (PROT_READ). Executing ptr[0] = 'H' causes the
CPU Memory Management Unit (MMU) to trigger a hardware page protection fault.
The Linux kernel traps the fault, checks VMA permissions, detects an illegal
write to a read-only page, and sends SIGSEGV to the process.


Explain the type and memory address difference between arr, &arr, and &arr[0]
All point to first byte of array on stack.
arr is an array with 6 bytes allocated on stack frame. which containts ac

*/
