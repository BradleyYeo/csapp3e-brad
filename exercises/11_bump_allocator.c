/*
 * Compile and run:
 *   clang -Wall -Wextra -pedantic -g -o 11_bump_allocator 11_bump_allocator.c && ./11_bump_allocator
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
 * Status codes for allocator initialization.
 * Eliminates magic return integers.
 */
typedef enum {
  BUMP_OK = 0,
  BUMP_ERR_INVALID_ARG = 1
} BumpStatus;

/*
 * BumpAllocator Data Structure
 *
 * Encapsulates a pre-allocated contiguous memory arena and tracks
 * the current allocation offset relative to the base address.
 *
 * Invariants:
 * - buffer is non-null when initialized.
 * - offset <= capacity at all times.
 */
typedef struct {
  uint8_t *buffer;    /* Pointer to start of backing memory arena */
  size_t capacity;    /* Total capacity of backing memory in bytes */
  size_t offset;      /* Allocation cursor: bytes allocated so far */
} BumpAllocator;

/*
 * BumpMarker Typedef
 *
 * Represents an opaque offset snapshot used to rollback allocations
 * to a prior checkpoint.
 */
typedef size_t BumpMarker;

/*
 * Composite test struct for alignment and stride validation.
 */
typedef struct {
  uint32_t id;
  char tag[12];
  uint64_t timestamp;
} EventRecord;

/*
 * Drill 1: Power of Two Validator
 *
 * Requirements:
 * - Return true if n is a non-zero power of two (1, 2, 4, 8, 16, ...).
 * - Return false if n == 0 or has more than one bit set in binary.
 * - Must use bitwise manipulation: (n & (n - 1)) clears the lowest set bit.
 */
/* TODO: Implement is_power_of_two */
static inline bool is_power_of_two(size_t n) {
  (void)n;
  // Type your implementation here.
  return false;
}

/*
 * Drill 2: Bitwise Forward Address Alignment
 *
 * Requirements:
 * - Precondition: align must be a non-zero power of two.
 * - Bitwise formula: (addr + (align - 1)) & ~(align - 1).
 * - Must round addr upwards to the next multiple of align.
 * - If addr is already aligned, return addr unchanged.
 */
/* TODO: Implement bump_align_forward */
static inline uintptr_t bump_align_forward(uintptr_t addr, size_t align) {
  (void)addr;
  (void)align;
  // Type your implementation here.
  return 0;
}

/*
 * Drill 3: Allocator Initialization
 *
 * Requirements:
 * - Return BUMP_ERR_INVALID_ARG if alloc is nullptr, memory_block is nullptr,
 *   or capacity_bytes == 0.
 * - Assign alloc->buffer = (uint8_t *)memory_block.
 * - Set alloc->capacity = capacity_bytes.
 * - Initialize alloc->offset = 0.
 * - Return BUMP_OK.
 */
/* TODO: Implement bump_init */
static BumpStatus bump_init(BumpAllocator *alloc, void *memory_block, size_t capacity_bytes) {
  (void)alloc;
  (void)memory_block;
  (void)capacity_bytes;
  // Type your implementation here.
  return BUMP_ERR_INVALID_ARG;
}

/*
 * Drill 4: Aligned Memory Allocation with Bounds and Overflow Defense
 *
 * Requirements:
 * - Return nullptr if alloc is nullptr or alloc->buffer is nullptr.
 * - If align == 0, default to alignof(max_align_t).
 * - Return nullptr if align is not a power of two.
 * - If size == 0, return current aligned address without advancing offset
 *   (or nullptr if current aligned offset exceeds capacity).
 * - Compute current physical address: alloc->buffer + alloc->offset.
 * - Compute aligned physical address via bump_align_forward.
 * - Compute padding bytes skipped: aligned_addr - current_addr.
 * - Guard against integer overflow before addition:
 *     Check if size > SIZE_MAX - padding or (padding + size) > SIZE_MAX - alloc->offset.
 * - Ensure alloc->offset + padding + size <= alloc->capacity.
 * - Increment alloc->offset by (padding + size).
 * - Return pointer to allocated memory block ((void *)aligned_addr).
 */
/* TODO: Implement bump_alloc */
static void *bump_alloc(BumpAllocator *alloc, size_t size, size_t align) {
  (void)alloc;
  (void)size;
  (void)align;
  // Type your implementation here.
  return nullptr;
}

/*
 * Drill 5: Bulk Deallocation (Reset)
 *
 * Requirements:
 * - If alloc is not nullptr, reset alloc->offset = 0.
 * - Reclaims all allocated blocks in O(1) time.
 */
/* TODO: Implement bump_reset */
static void bump_reset(BumpAllocator *alloc) {
  (void)alloc;
  // Type your implementation here.
}

/*
 * Drill 6: Checkpoint Marker Capture
 *
 * Requirements:
 * - Return current alloc->offset snapshot.
 * - Return 0 if alloc is nullptr.
 */
/* TODO: Implement bump_save */
static BumpMarker bump_save(const BumpAllocator *alloc) {
  (void)alloc;
  // Type your implementation here.
  return 0;
}

/*
 * Drill 7: Scoped Checkpoint Rollback
 *
 * Requirements:
 * - Invariant: marker must be <= alloc->offset to prevent expanding past cursor.
 * - Return false if alloc is nullptr or marker > alloc->offset.
 * - Set alloc->offset = marker.
 * - Return true.
 */
/* TODO: Implement bump_restore */
static bool bump_restore(BumpAllocator *alloc, BumpMarker marker) {
  (void)alloc;
  (void)marker;
  // Type your implementation here.
  return false;
}

/*
 * Drill 8: Query Remaining Capacity
 *
 * Requirements:
 * - Return remaining allocatable raw bytes (capacity - offset).
 * - Return 0 if alloc is nullptr or offset > capacity.
 */
/* TODO: Implement bump_available */
static size_t bump_available(const BumpAllocator *alloc) {
  (void)alloc;
  // Type your implementation here.
  return 0;
}

/*
 * Drill 9: Pointer Ownership Bounds Verification
 *
 * Requirements:
 * - Return false if alloc is nullptr, alloc->buffer is nullptr, or ptr is nullptr.
 * - Return true if (uintptr_t)ptr >= (uintptr_t)alloc->buffer
 *   AND (uintptr_t)ptr < ((uintptr_t)alloc->buffer + alloc->capacity).
 */
/* TODO: Implement bump_owns */
static bool bump_owns(const BumpAllocator *alloc, const void *ptr) {
  (void)alloc;
  (void)ptr;
  // Type your implementation here.
  return false;
}

int main(void) {
  // Test 1: Power-of-two validation
  assert(is_power_of_two(1) == true);
  assert(is_power_of_two(2) == true);
  assert(is_power_of_two(4) == true);
  assert(is_power_of_two(8) == true);
  assert(is_power_of_two(16) == true);
  assert(is_power_of_two(64) == true);
  assert(is_power_of_two(0) == false);
  assert(is_power_of_two(3) == false);
  assert(is_power_of_two(5) == false);
  assert(is_power_of_two(6) == false);
  assert(is_power_of_two(7) == false);
  assert(is_power_of_two(9) == false);

  // Test 2: Bitwise forward alignment arithmetic
  assert(bump_align_forward(0, 4) == 0);
  assert(bump_align_forward(1, 4) == 4);
  assert(bump_align_forward(4, 4) == 4);
  assert(bump_align_forward(5, 4) == 8);
  assert(bump_align_forward(13, 8) == 16);
  assert(bump_align_forward(16, 8) == 16);
  assert(bump_align_forward(17, 8) == 24);
  assert(bump_align_forward(31, 16) == 32);
  assert(bump_align_forward(32, 16) == 32);

  // Test 3: Allocator initialization
  BumpAllocator alloc = {0};
  uint8_t memory_arena[512] = {0};

  assert(bump_init(&alloc, memory_arena, sizeof(memory_arena)) == BUMP_OK);
  assert(alloc.buffer == memory_arena);
  assert(alloc.capacity == 512);
  assert(alloc.offset == 0);
  assert(bump_available(&alloc) == 512);

  // Invalid initialization parameters
  assert(bump_init(nullptr, memory_arena, 512) == BUMP_ERR_INVALID_ARG);
  assert(bump_init(&alloc, nullptr, 512) == BUMP_ERR_INVALID_ARG);
  assert(bump_init(&alloc, memory_arena, 0) == BUMP_ERR_INVALID_ARG);

  // Test 4: Basic monotonic allocation and contiguous packing
  void *p1 = bump_alloc(&alloc, 10, 1);
  assert(p1 != nullptr);
  assert(p1 == (void *)memory_arena);
  assert(alloc.offset == 10);
  assert(bump_available(&alloc) == 502);

  void *p2 = bump_alloc(&alloc, 20, 1);
  assert(p2 != nullptr);
  assert(p2 == (void *)(memory_arena + 10));
  assert(alloc.offset == 30);
  assert(bump_available(&alloc) == 482);

  // Test 5: Strict alignment guarantees across mixed types
  // Currently at offset 30 (not a multiple of 8).
  // Requesting 8-byte alignment should inject 2 bytes of padding to reach offset 32.
  uint64_t *p_u64 = bump_alloc(&alloc, sizeof(uint64_t), alignof(uint64_t));
  assert(p_u64 != nullptr);
  assert(((uintptr_t)p_u64 % alignof(uint64_t)) == 0);
  assert((void *)p_u64 == (void *)(memory_arena + 32));
  assert(alloc.offset == 32 + sizeof(uint64_t)); // 40

  // Allocate 1 byte, bringing offset to 41 (unaligned for 16-byte boundary)
  void *p_byte = bump_alloc(&alloc, 1, 1);
  assert(p_byte != nullptr);
  assert(alloc.offset == 41);

  // Request 16-byte alignment: offset 41 + 7 bytes padding -> aligned to 48
  void *p_simd = bump_alloc(&alloc, 32, 16);
  assert(p_simd != nullptr);
  assert(((uintptr_t)p_simd % 16) == 0);
  assert(p_simd == (void *)(memory_arena + 48));
  assert(alloc.offset == 48 + 32); // 80

  // Allocate composite struct type
  EventRecord *rec = bump_alloc(&alloc, sizeof(EventRecord), alignof(EventRecord));
  assert(rec != nullptr);
  assert(((uintptr_t)rec % alignof(EventRecord)) == 0);
  rec->id = 42;
  strncpy(rec->tag, "Sensors", sizeof(rec->tag) - 1);
  rec->timestamp = 999999;
  assert(rec->id == 42 && strcmp(rec->tag, "Sensors") == 0);

  // Test 6: Memory ownership verification
  assert(bump_owns(&alloc, p1) == true);
  assert(bump_owns(&alloc, p_u64) == true);
  assert(bump_owns(&alloc, rec) == true);
  uint8_t external_stack_var = 0;
  assert(bump_owns(&alloc, &external_stack_var) == false);
  assert(bump_owns(&alloc, nullptr) == false);
  assert(bump_owns(nullptr, p1) == false);

  // Test 7: Out-of-memory boundary detection
  size_t remaining = bump_available(&alloc);
  // Requesting remaining + 1 bytes must fail cleanly
  void *p_oom = bump_alloc(&alloc, remaining + 1, 1);
  assert(p_oom == nullptr);

  // Requesting with alignment padding exceeding remaining capacity must fail cleanly
  size_t current_offset_before_fail = alloc.offset;
  void *p_oom_pad = bump_alloc(&alloc, remaining, 64);
  // If remaining capacity cannot accommodate 64-byte padding + size, returns nullptr
  if (bump_align_forward((uintptr_t)(alloc.buffer + alloc.offset), 64) - (uintptr_t)(alloc.buffer + alloc.offset) > 0) {
    assert(p_oom_pad == nullptr);
    assert(alloc.offset == current_offset_before_fail); // Invariant: offset unchanged on failure
  }

  // Test 8: Integer overflow protection
  assert(bump_alloc(&alloc, SIZE_MAX, 8) == nullptr);
  assert(bump_alloc(&alloc, SIZE_MAX - 10, 8) == nullptr);
  assert(alloc.offset == current_offset_before_fail);

  // Test 9: Scoped checkpointing (save & restore)
  BumpMarker checkpoint = bump_save(&alloc);
  assert(checkpoint == alloc.offset);

  void *temp1 = bump_alloc(&alloc, 24, 8);
  void *temp2 = bump_alloc(&alloc, 32, 8);
  assert(temp1 != nullptr && temp2 != nullptr);
  assert(alloc.offset > checkpoint);

  // Rollback to checkpoint
  assert(bump_restore(&alloc, checkpoint) == true);
  assert(alloc.offset == checkpoint);

  // Invariant: restoring past current cursor must be rejected
  assert(bump_restore(&alloc, alloc.capacity + 10) == false);

  // Re-allocating after restore reuses the exact same memory range
  void *reused = bump_alloc(&alloc, 24, 8);
  assert(reused == temp1);

  // Test 10: Reset functionality (bulk deallocation)
  bump_reset(&alloc);
  assert(alloc.offset == 0);
  assert(bump_available(&alloc) == alloc.capacity);

  // Re-allocating from base after reset
  void *fresh = bump_alloc(&alloc, 16, 8);
  assert(fresh == (void *)memory_arena);
  assert(alloc.offset == 16);

  printf("[PASS] Exercise 11: Fixed-Size Bump Allocator verified.\n");
  return EXIT_SUCCESS;
}
