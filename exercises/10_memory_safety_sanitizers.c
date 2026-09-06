#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Defensive Memory Container
 * Ensures bounds and null pointer verification before any access.
 */
typedef struct {
  uint8_t *buffer;
  size_t length;
} SafeBlock;

/*
 * Allocates and initializes a safe memory container.
 */
static SafeBlock safe_block_create(size_t length) {
  if (length == 0) {
    return (SafeBlock){.buffer = nullptr, .length = 0};
  }

  uint8_t *buf = calloc(length, sizeof(uint8_t));
  return (SafeBlock){.buffer = buf, .length = (buf != nullptr) ? length : 0};
}

/*
 * Deallocates the block and explicitly zeroes pointer and length.
 * Prevents dangling pointers and double frees (Jimmy Koppel).
 */
static void safe_block_destroy(SafeBlock *block) {
  if (block == nullptr || block->buffer == nullptr) {
    return;
  }

  free(block->buffer);
  block->buffer = nullptr;
  block->length = 0;
}

/*
 * Drill 1: Bounded Safe Write
 * Implement:
 *   bool safe_block_write(SafeBlock *block, size_t index, uint8_t byte)
 *
 * Requirements:
 * - Return false if block is nullptr, block->buffer is nullptr, or index >= block->length.
 * - Write byte to block->buffer[index].
 * - Return true.
 * - This defines buffer overflows out of existence at the abstraction layer.
 */
/* TODO: Implement safe_block_write */
static bool safe_block_write(SafeBlock *block, size_t index, uint8_t byte) {
  if (block == nullptr || block->buffer == nullptr || index >= block->length) {
    return false;
  }
  (void)byte;

  // Type your implementation here.
  return true;
}

/*
 * Drill 2: Bounded Safe Read
 * Implement:
 *   bool safe_block_read(const SafeBlock *block, size_t index, uint8_t *out_byte)
 *
 * Requirements:
 * - Return false if block is nullptr, block->buffer is nullptr, out_byte is nullptr,
 *   or index >= block->length.
 * - Assign block->buffer[index] to *out_byte.
 * - Return true.
 */
/* TODO: Implement safe_block_read */
static bool safe_block_read(const SafeBlock *block, size_t index, uint8_t *out_byte) {
  if (block == nullptr || block->buffer == nullptr || out_byte == nullptr || index >= block->length) {
    return false;
  }

  // Type your implementation here.
  return true;
}

/*
 * Intentional Defect Triggers for Compiler Sanitizers (ASan / UBSan)
 * These functions are deliberately invoked via CLI flags to demonstrate
 * real sanitizer crash reports as described in MEMORY_DEBUGGING.md.
 */

static void trigger_use_after_free(void) {
  printf("[TRIGGER] Invoking Heap Use-After-Free...\n");
  volatile int *p = malloc(sizeof(int));
  assert(p != nullptr);
  *p = 42;
  free((void *)p);

  // Faulty access on deallocated memory (triggers ASan heap-use-after-free)
  printf("Attempting read after free: %d\n", *p);
}

static void trigger_double_free(void) {
  printf("[TRIGGER] Invoking Double Free...\n");
  void *p = malloc(32);
  assert(p != nullptr);
  free(p);

  // Faulty second deallocation (triggers ASan double-free)
  free(p);
}

static void trigger_heap_buffer_overflow(void) {
  printf("[TRIGGER] Invoking Heap Buffer Overflow...\n");
  volatile uint8_t *buf = malloc(8);
  assert(buf != nullptr);

  // Faulty out-of-bounds write (triggers ASan heap-buffer-overflow)
  volatile size_t bad_idx = 8;
  buf[bad_idx] = 0xFF;
  free((void *)buf);
}

static void trigger_stack_buffer_overflow(void) {
  printf("[TRIGGER] Invoking Stack Buffer Overflow...\n");
  volatile char stack_buf[8];

  // Faulty write past local array frame (triggers ASan stack-buffer-overflow)
  volatile size_t bad_idx = 12;
  stack_buf[bad_idx] = 'Z';
  printf("Value: %c\n", stack_buf[bad_idx]);
}

static void trigger_memory_leak(void) {
  printf("[TRIGGER] Invoking Memory Leak...\n");
  void *leak_ptr = malloc(64);
  assert(leak_ptr != nullptr);
  memset(leak_ptr, 0xAA, 64);
  printf("Allocated 64 bytes at %p without free. Check via leaks tool or LSan.\n", leak_ptr);
  // Exits without free(leak_ptr)
}

int main(int argc, char *argv[]) {
  // Check for defect trigger flags
  if (argc > 1) {
    if (strcmp(argv[1], "--trigger-uaf") == 0) {
      trigger_use_after_free();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-double-free") == 0) {
      trigger_double_free();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-overflow") == 0) {
      trigger_heap_buffer_overflow();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-stack-overflow") == 0) {
      trigger_stack_buffer_overflow();
      return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "--trigger-leak") == 0) {
      trigger_memory_leak();
      return EXIT_SUCCESS;
    }
  }

  // Default mode: Verify safe memory container hygiene
  SafeBlock block = safe_block_create(4);
  assert(block.buffer != nullptr && block.length == 4);

  // Drill 1 Verification: Bounded writes
  assert(safe_block_write(&block, 0, 0x10) == true);
  assert(safe_block_write(&block, 3, 0x40) == true);
  assert(safe_block_write(&block, 4, 0x50) == false); // Out-of-bounds caught!
  assert(safe_block_write(nullptr, 0, 0x00) == false);

  // Drill 2 Verification: Bounded reads
  uint8_t val = 0;
  assert(safe_block_read(&block, 0, &val) == true && val == 0x10);
  assert(safe_block_read(&block, 3, &val) == true && val == 0x40);
  assert(safe_block_read(&block, 4, &val) == false);  // Out-of-bounds caught!
  assert(safe_block_read(&block, 0, nullptr) == false);

  // Safe Destruction
  safe_block_destroy(&block);
  assert(block.buffer == nullptr && block.length == 0);

  // Multiple destructions are defined safe (no double free)
  safe_block_destroy(&block);

  printf("[PASS] Exercise 10: Memory Safety & Sanitizers verified.\n");
  return EXIT_SUCCESS;
}
