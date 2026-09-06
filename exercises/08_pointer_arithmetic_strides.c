/*
 * Compile and run:
 *   clang -Wall -Wextra -pedantic -g -o 08_pointer_arithmetic_strides 08_pointer_arithmetic_strides.c && ./08_pointer_arithmetic_strides
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Composite Record structure to demonstrate stride scaling
 * across non-primitive data types.
 */
typedef struct {
  uint32_t id;
  char name[12];
  uint64_t timestamp;
} Record;

/*
 * Drill 1: Generic Byte Advancement
 * Implement:
 *   const void *advance_bytes(const void *base, size_t byte_offset)
 *
 * Requirements:
 * - In ISO C, arithmetic on `void*` is undefined (sizeof(void) is incomplete).
 * - Cast base pointer to `const unsigned char *` before adding byte_offset.
 * - Return nullptr if base is nullptr.
 * - Return advanced pointer address.
 */
/* TODO: Implement advance_bytes */
static const void *advance_bytes(const void *base, size_t byte_offset) {
  if (base == nullptr) {
    return nullptr;
  }
  (void)byte_offset;

  // Type your implementation here.
  return nullptr;
}

/*
 * Drill 2: Element Lookup via Stride Calculation
 * Implement:
 *   const void *get_element_at(const void *base, size_t index, size_t elem_size)
 *
 * Requirements:
 * - Return nullptr if base is nullptr or elem_size == 0.
 * - Compute address: base + (index * elem_size).
 * - Must use byte stride calculation (advance_bytes), no array bracket indexing.
 */
/* TODO: Implement get_element_at */
static const void *get_element_at(const void *base, size_t index, size_t elem_size) {
  if (base == nullptr || elem_size == 0) {
    return nullptr;
  }
  (void)index;

  // Type your implementation here.
  return nullptr;
}

/*
 * Drill 3: Bounded Slice Pointer Invariant Validator
 * Implement:
 *   bool is_pointer_within_bounds(const void *ptr, const void *start, const void *end)
 *
 * Requirements:
 * - Verify that `ptr` falls within half-open range `[start, end)`.
 * - Return false if any pointer argument is nullptr.
 * - Return false if start >= end.
 * - Return true if (uintptr_t)ptr >= (uintptr_t)start AND (uintptr_t)ptr < (uintptr_t)end.
 */
/* TODO: Implement is_pointer_within_bounds */
static bool is_pointer_within_bounds(const void *ptr, const void *start, const void *end) {
  if (ptr == nullptr || start == nullptr || end == nullptr) {
    return false;
  }

  // Type your implementation here.
  return false;
}

int main(void) {
  // Test 1: Stride scaling across integer primitives
  const int32_t int_array[] = {10, 20, 30, 40, 50};
  const int32_t *p_int = int_array;

  // Pointer addition advances by sizeof(int32_t) == 4 bytes
  assert((uintptr_t)(p_int + 1) - (uintptr_t)p_int == sizeof(int32_t));
  assert((uintptr_t)(p_int + 3) - (uintptr_t)p_int == 3 * sizeof(int32_t));

  // Pointer difference computes element count as ptrdiff_t
  const int32_t *p_int_end = int_array + 5;
  ptrdiff_t int_count = p_int_end - p_int;
  assert(int_count == 5);

  // Test 2: Stride scaling across composite struct types
  const Record records[3] = {
    {.id = 1, .name = "Alpha", .timestamp = 1000},
    {.id = 2, .name = "Beta",  .timestamp = 2000},
    {.id = 3, .name = "Gamma", .timestamp = 3000}
  };
  const Record *p_rec = records;

  assert((uintptr_t)(p_rec + 1) - (uintptr_t)p_rec == sizeof(Record));
  assert((p_rec + 2) - p_rec == 2);

  // Drill 1 Verification: advance_bytes
  const void *byte_advanced = advance_bytes(int_array, 2 * sizeof(int32_t));
  assert(byte_advanced == &int_array[2]);
  assert(advance_bytes(nullptr, 10) == nullptr);

  // Drill 2 Verification: get_element_at
  const void *elem1 = get_element_at(int_array, 3, sizeof(int32_t));
  assert(elem1 == &int_array[3]);
  assert(*(const int32_t *)elem1 == 40);

  const void *rec2 = get_element_at(records, 1, sizeof(Record));
  assert(rec2 == &records[1]);
  assert(((const Record *)rec2)->id == 2);

  assert(get_element_at(nullptr, 0, sizeof(int)) == nullptr);
  assert(get_element_at(int_array, 0, 0) == nullptr);

  // Drill 3 Verification: is_pointer_within_bounds
  const char buffer[] = "abcdefghij";
  const char *start = buffer;
  const char *end = buffer + 10;

  assert(is_pointer_within_bounds(start, start, end) == true);
  assert(is_pointer_within_bounds(start + 5, start, end) == true);
  assert(is_pointer_within_bounds(end - 1, start, end) == true);
  assert(is_pointer_within_bounds(end, start, end) == false);
  assert(is_pointer_within_bounds(end + 1, start, end) == false);
  assert(is_pointer_within_bounds(start - 1, start, end) == false);
  assert(is_pointer_within_bounds(nullptr, start, end) == false);
  assert(is_pointer_within_bounds(start, end, start) == false);

  printf("[PASS] Exercise 08: Pointer Arithmetic & Strides verified.\n");
  return EXIT_SUCCESS;
}
