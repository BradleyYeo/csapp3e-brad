#include <stddef.h>
#include <stdint.h>

struct RecordA {
  uint8_t flag;
  uint64_t id;
  uint32_t count;
};

struct RecordB {
  uint64_t id;
  uint32_t count;
  uint8_t flag;
};

/*
Explain why sizeof(struct RecordA) and sizeof(struct RecordB) differ on an
x86-64 Linux system, explaining alignment restrictions and padding bytes.


Calculate the exact byte size of both struct RecordA and struct RecordB.


Explain how generic functions such as qsort use const void * parameters to
achieve polymorphism, and explain how to safely cast and dereference a const
void * back into a struct pointer without violating strict aliasing rules.
*/