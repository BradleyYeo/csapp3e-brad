/*
 * Exercise 54: Struct Alignment, Intrusive Containers & Function Pointers (CS:APP Ch. 3 / Systems C)
 *
 * Compile and run:
 *   make -C curriculum 54_struct_alignment_and_fn_ptrs && ./curriculum/54_struct_alignment_and_fn_ptrs
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdalign.h>
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint8_t flags;
  uint64_t seq;
  uint16_t port;
  uint32_t crc;
} BloatedPacket;

typedef struct {
  uint64_t seq;
  uint32_t crc;
  uint16_t port;
  uint8_t flags;
} PackedPacket;

typedef struct IntrusiveLink {
  struct IntrusiveLink *next;
} IntrusiveLink;

typedef struct {
  int id;
  int priority;
  IntrusiveLink link;
} TaskItem;

typedef int (*generic_cmp_fn)(const void *a, const void *b);
typedef int32_t (*alu_op_fn)(int32_t a, int32_t b);

/*
 * Drill 1: Struct Padding Computation (CS:APP 3.9.3 Alignment)
 *
 * Requirements:
 * - Return 0 if total_sizeof < sum_of_fields.
 * - Return total_sizeof - sum_of_fields (total internal + tail padding bytes).
 */
static size_t struct_padding_bytes(size_t total_sizeof, size_t sum_of_fields) {
  (void)total_sizeof;
  (void)sum_of_fields;
  return 0; /* TODO: implement */
}

/*
 * Drill 2: Intrusive Pointer Recovery (`container_of` idiom)
 *
 * Linux kernel and xv6 embed list links inside structs instead of wrapping
 * data in separate heap-allocated list nodes. Given a pointer to the embedded
 * `link` member, recover the base pointer to the enclosing `TaskItem`.
 *
 * Requirements:
 * - Return nullptr if link == nullptr.
 * - Compute enclosing TaskItem * by subtracting offsetof(TaskItem, link)
 *   from (uintptr_t)link.
 */
static TaskItem *task_from_link(const IntrusiveLink *link) {
  (void)link;
  return nullptr; /* TODO: implement */
}

/*
 * Drill 3: Overflow-Safe Three-Way Comparator (qsort / bsearch contract)
 *
 * Naive broken comparator: `return *(const int32_t *)a - *(const int32_t *)b;`
 * Fails on (INT32_MIN, 1) -> signed overflow UB and wrong sign!
 *
 * Requirements:
 * - Safely read `va` and `vb` as `int32_t`.
 * - Return -1 if va < vb, 0 if va == vb, +1 if va > vb (via `(va > vb) - (va < vb)`).
 */
static int cmp_i32_asc(const void *a, const void *b) {
  (void)a;
  (void)b;
  return 0; /* TODO: implement */
}

/*
 * Drill 4: Generic Binary Search (Lower-Bound First Match via Byte Strides)
 *
 * Requirements:
 * - Return nullptr if key == nullptr || base == nullptr || cmp == nullptr ||
 *   nmemb == 0 || size == 0.
 * - Perform binary search over [0, nmemb) using byte pointer arithmetic:
 *     `const unsigned char *elem = (const unsigned char *)base + (mid * size);`
 * - If duplicate keys exist, return the pointer to the FIRST matching element
 *   (lowest index where cmp(key, elem) == 0).
 * - Return nullptr if no element matches `key`.
 */
static const void *generic_bsearch_first(const void *key, const void *base,
                                         size_t nmemb, size_t size,
                                         generic_cmp_fn cmp) {
  (void)key;
  (void)base;
  (void)nmemb;
  (void)size;
  (void)cmp;
  return nullptr; /* TODO: implement */
}

/*
 * Drill 5: Function-Pointer Dispatch Table (Jump Table Pattern)
 *
 * Requirements:
 * - Define static helper functions (or table of `alu_op_fn`) for opcodes:
 *     0: ADD (compute via `(int32_t)((uint32_t)a + (uint32_t)b)` — no signed overflow UB)
 *     1: SUB (compute via `(int32_t)((uint32_t)a - (uint32_t)b)` — no signed overflow UB)
 *     2: BIT_AND (`(int32_t)((uint32_t)a & (uint32_t)b)`)
 *     3: BIT_XOR (`(int32_t)((uint32_t)a ^ (uint32_t)b)`)
 * - Return false if out == nullptr or opcode >= 4.
 * - Dispatch through the `alu_op_fn` table, store result in *out, and return true.
 */
static bool alu_dispatch(uint8_t opcode, int32_t a, int32_t b, int32_t *out) {
  (void)opcode;
  (void)a;
  (void)b;
  (void)out;
  return false; /* TODO: implement */
}

int main(void) {
  /* Drill 1: Alignment & packing */
  size_t raw_sum = sizeof(uint8_t) + sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint32_t);
  assert(raw_sum == 15);
  assert(alignof(BloatedPacket) == 8);
  assert(sizeof(BloatedPacket) == 24);
  assert(sizeof(PackedPacket) == 16);
  assert(struct_padding_bytes(sizeof(BloatedPacket), raw_sum) == 9);
  assert(struct_padding_bytes(sizeof(PackedPacket), raw_sum) == 1);
  assert(struct_padding_bytes(4, 8) == 0);

  /* Drill 2: Intrusive list traversal + container_of recovery */
  TaskItem t1 = {.id = 101, .priority = 1, .link = {.next = nullptr}};
  TaskItem t2 = {.id = 202, .priority = 5, .link = {.next = nullptr}};
  t1.link.next = &t2.link;

  assert(task_from_link(nullptr) == nullptr);
  assert(task_from_link(&t1.link) == &t1);
  assert(task_from_link(t1.link.next) == &t2);
  assert(task_from_link(t1.link.next)->id == 202);

  /* Drill 3: Overflow-safe comparator */
  int32_t min_v = INT32_MIN;
  int32_t max_v = INT32_MAX;
  int32_t one_v = 1;
  assert(cmp_i32_asc(&min_v, &max_v) < 0);
  assert(cmp_i32_asc(&max_v, &min_v) > 0);
  assert(cmp_i32_asc(&min_v, &one_v) < 0);
  assert(cmp_i32_asc(&one_v, &one_v) == 0);

  /* Drill 4: Generic binary search (including duplicates) */
  int32_t arr[] = {INT32_MIN, -5, 10, 10, 10, 42, INT32_MAX};
  size_t n = sizeof(arr) / sizeof(arr[0]);

  int32_t k1 = 10;
  const void *found1 = generic_bsearch_first(&k1, arr, n, sizeof(arr[0]), cmp_i32_asc);
  assert(found1 == &arr[2]);

  int32_t k2 = INT32_MIN;
  assert(generic_bsearch_first(&k2, arr, n, sizeof(arr[0]), cmp_i32_asc) == &arr[0]);

  int32_t k3 = INT32_MAX;
  assert(generic_bsearch_first(&k3, arr, n, sizeof(arr[0]), cmp_i32_asc) == &arr[6]);

  int32_t missing = 11;
  assert(generic_bsearch_first(&missing, arr, n, sizeof(arr[0]), cmp_i32_asc) == nullptr);
  assert(generic_bsearch_first(&k1, arr, 0, sizeof(arr[0]), cmp_i32_asc) == nullptr);
  assert(generic_bsearch_first(nullptr, arr, n, sizeof(arr[0]), cmp_i32_asc) == nullptr);

  /* Drill 5: Function-pointer dispatch table */
  int32_t res = 0;
  assert(alu_dispatch(0, 15, 27, &res) == true && res == 42);
  assert(alu_dispatch(1, 10, 25, &res) == true && res == -15);
  assert(alu_dispatch(2, 0x0F, 0x36, &res) == true && res == 0x06);
  assert(alu_dispatch(3, 0x0F, 0x05, &res) == true && res == 0x0A);
  assert(alu_dispatch(0, INT32_MAX, 1, &res) == true && res == INT32_MIN);
  assert(alu_dispatch(4, 1, 2, &res) == false);
  assert(alu_dispatch(0, 1, 2, nullptr) == false);

  printf("[PASS] Exercise 54: Struct Alignment & Function Pointers verified.\n");
  return EXIT_SUCCESS;
}
