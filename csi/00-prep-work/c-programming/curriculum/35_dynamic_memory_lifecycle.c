/*
 * Compile and run:
 *   clang -Wall -Wextra -pedantic -g -o 09_dynamic_memory_lifecycle 09_dynamic_memory_lifecycle.c && ./09_dynamic_memory_lifecycle
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
#include <string.h>

/*
 * Allocation Status Enum
 * Defines allocation and resizing results explicitly without magic numbers.
 */
typedef enum {
  ALLOC_SUCCESS = 0,
  ALLOC_OUT_OF_MEMORY,
  ALLOC_INVALID_ARG
} AllocStatus;

/*
 * Dynamic Byte Buffer
 * Encapsulates dynamic memory allocation, tracking size and capacity.
 */
typedef struct {
  uint8_t *data;
  size_t size;
  size_t capacity;
} Buffer;

/*
 * Creates a dynamic buffer with specified initial capacity.
 * Uses calloc to zero-initialize the memory block.
 */
static AllocStatus buffer_create(Buffer *buf, size_t initial_cap) {
  (void)buf;
  (void)initial_cap;
  /* TODO: implement — ALLOC_INVALID_ARG on bad args, calloc(initial_cap,
   * sizeof(uint8_t)) into buf->data, ALLOC_OUT_OF_MEMORY when calloc fails.
   * Stub allocates nothing so no later assert can free an unowned pointer. */
  return ALLOC_INVALID_ARG;
}

/*
 * Releases dynamic buffer storage and immediately nullifies pointers.
 * Defining dangling pointers out of existence (Jimmy Koppel).
 */
static void buffer_destroy(Buffer *buf) {
  (void)buf;
  /* TODO: implement — free(buf->data), then set data = nullptr, size = 0,
   * capacity = 0. Stub frees nothing, so a stubbed buffer_create can never
   * produce a double free before the first assert runs. */
}

/*
 * Drill 1: Safe Buffer Append with Capacity Growth
 * Implement:
 *   AllocStatus buffer_append(Buffer *buf, uint8_t byte)
 *
 * Requirements:
 * - Return ALLOC_INVALID_ARG if buf is nullptr or buf->data is nullptr.
 * - If buf->size == buf->capacity:
 *     Calculate new_cap = buf->capacity * 2.
 *     Reallocate using safe realloc pattern:
 *       void *temp = realloc(buf->data, new_cap);
 *       if (temp == nullptr) return ALLOC_OUT_OF_MEMORY; // Original data preserved!
 *       buf->data = temp;
 *       buf->capacity = new_cap;
 * - Write byte to buf->data[buf->size].
 * - Increment buf->size.
 * - Return ALLOC_SUCCESS.
 */
static AllocStatus buffer_append(Buffer *buf, uint8_t byte) {
  (void)buf;
  (void)byte;
  return ALLOC_INVALID_ARG; /* TODO: implement */
}

/*
 * Drill 2: Shrink to Fit
 * Implement:
 *   AllocStatus buffer_shrink_to_fit(Buffer *buf)
 *
 * Requirements:
 * - Return ALLOC_INVALID_ARG if buf is nullptr or buf->data is nullptr.
 * - If buf->size == buf->capacity, nothing to shrink; return ALLOC_SUCCESS.
 * - If buf->size == 0, keep minimum allocation of 1 byte to avoid implementation-defined realloc(ptr, 0).
 * - Reallocate buffer->data to new capacity equal to buf->size (or 1 if size is 0).
 * - If realloc succeeds, update buf->data and buf->capacity = buf->size.
 * - Return ALLOC_SUCCESS or ALLOC_OUT_OF_MEMORY.
 */
static AllocStatus buffer_shrink_to_fit(Buffer *buf) {
  (void)buf;
  return ALLOC_INVALID_ARG; /* TODO: implement */
}

/*
 * Drill 3: Buffer Deep Clone (Independent Lifecycle)
 * Implement:
 *   AllocStatus buffer_clone(Buffer *dest, const Buffer *src)
 *
 * Requirements:
 * - Return ALLOC_INVALID_ARG if dest or src is nullptr, or src->data is nullptr.
 * - Allocate dest->data with malloc/calloc to match src->capacity.
 * - Copy src->size bytes from src->data to dest->data using memcpy.
 * - Set dest->size = src->size and dest->capacity = src->capacity.
 * - Return ALLOC_SUCCESS or ALLOC_OUT_OF_MEMORY.
 */
static AllocStatus buffer_clone(Buffer *dest, const Buffer *src) {
  (void)dest;
  (void)src;
  /* TODO: implement — leave dest untouched on the stub path so it can never
   * alias (and double free) src's storage. */
  return ALLOC_INVALID_ARG;
}

int main(void) {
  Buffer buf = {0};

  // Test 1: Buffer creation
  assert(buffer_create(&buf, 2) == ALLOC_SUCCESS);
  assert(buf.data != nullptr);
  assert(buf.size == 0);
  assert(buf.capacity == 2);
  assert(buffer_create(nullptr, 10) == ALLOC_INVALID_ARG);
  assert(buffer_create(&buf, 0) == ALLOC_INVALID_ARG);

  // Drill 1 Verification: Appending and capacity growth
  assert(buffer_append(&buf, 0xAA) == ALLOC_SUCCESS);
  assert(buf.size == 1 && buf.data[0] == 0xAA);
  assert(buf.capacity == 2);

  assert(buffer_append(&buf, 0xBB) == ALLOC_SUCCESS);
  assert(buf.size == 2 && buf.data[1] == 0xBB);
  assert(buf.capacity == 2);

  // Exceeds capacity -> triggers realloc growth to 4
  assert(buffer_append(&buf, 0xCC) == ALLOC_SUCCESS);
  assert(buf.size == 3 && buf.data[2] == 0xCC);
  assert(buf.capacity == 4);

  // Drill 3 Verification: Cloning
  Buffer clone = {0};
  assert(buffer_clone(&clone, &buf) == ALLOC_SUCCESS);
  assert(clone.data != buf.data); // Independent heap pointers!
  assert(clone.size == buf.size);
  assert(clone.capacity == buf.capacity);
  assert(memcmp(clone.data, buf.data, buf.size) == 0);

  // Drill 2 Verification: Shrink to fit
  assert(buffer_shrink_to_fit(&buf) == ALLOC_SUCCESS);
  assert(buf.size == 3);
  assert(buf.capacity == 3);
  assert(buf.data[0] == 0xAA && buf.data[1] == 0xBB && buf.data[2] == 0xCC);

  // Cleanup
  buffer_destroy(&buf);
  assert(buf.data == nullptr && buf.size == 0 && buf.capacity == 0);

  buffer_destroy(&clone);
  assert(clone.data == nullptr);

  printf("[PASS] Exercise 09: Dynamic Memory Lifecycle verified.\n");
  return EXIT_SUCCESS;
}
