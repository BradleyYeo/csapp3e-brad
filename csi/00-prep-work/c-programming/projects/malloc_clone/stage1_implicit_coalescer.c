/*
 * Stage 1: Implicit Free List & Boundary Tag Coalescer (CS:APP Baseline)
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o stage1_implicit stage1_implicit_coalescer.c && ./stage1_implicit
 *
 * Learning Objectives:
 * - Acquire page-aligned virtual memory slabs via anonymous mmap().
 * - Format a heap with Prologue (16B allocated) and Epilogue (0B allocated) sentinel blocks
 *   to define boundary-check edge cases out of existence (John Ousterhout).
 * - Pack block size and allocation status into an 8-byte header/footer using bit-stealing
 *   on the lower 4 bits (guaranteed zero by 16-byte alignment).
 * - Implement first-fit block search, block splitting, and immediate 4-case boundary-tag coalescing.
 * - Implement a heap invariant checker (mm_checkheap) to verify structural integrity.
 */
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
#include <sys/mman.h>

/* ============================================================================
 * Architectural Constants & Bitwise Invariants
 * ============================================================================
 *
 * Memory Layout of the Heap Slab:
 * +-------------+---------------+---------------+-------------------------------+---------------+
 * | Alignment   | Prologue Hdr  | Prologue Ftr  | Regular Blocks...             | Epilogue Hdr  |
 * | Pad (8 B)   | 16 | 0x1 (8B) | 16 | 0x1 (8B) | [Hdr][Payload/Pad][Ftr]       | 0 | 0x1 (8B)  |
 * +-------------+---------------+---------------+-------------------------------+---------------+
 *               ^               ^               ^
 *               prologue_hdr    heap_listp      First user payload pointer (16-byte aligned!)
 */
static const size_t WORD_SIZE = sizeof(size_t);          /* 8 bytes on 64-bit */
static const size_t ALIGNMENT = 16;                      /* 16-byte ABI alignment requirement */
static const size_t MIN_BLOCK_SIZE = 32;                 /* 8B Hdr + 16B Min Payload + 8B Ftr */
static const size_t ALLOC_MASK = 0x1;                    /* Bit 0: 1 = Allocated, 0 = Free */
static const size_t SIZE_MASK = ~(size_t)(ALIGNMENT - 1);/* Clears lower 4 bits */

/* Global Arena State */
typedef struct {
  uint8_t *slab_base;   /* Raw pointer returned by mmap() */
  size_t slab_capacity; /* Total bytes mapped from OS */
  uint8_t *heap_listp;  /* Points to payload of the Prologue block (slab_base + 16) */
} ImplicitArena;

static ImplicitArena g_arena = {0};

/* ============================================================================
 * Bitwise & Pointer Arithmetic Helpers (Inline Primitives)
 * ============================================================================ */

/* Pack a size and allocated bit into a single 8-byte word */
static inline size_t pack_word(size_t size, bool is_alloc) {
  return size | (is_alloc ? ALLOC_MASK : 0);
}

/* Read and write an 8-byte word at address p */
static inline size_t read_word(const void *p) {
  return *(const size_t *)p;
}

static inline void write_word(void *p, size_t val) {
  *(size_t *)p = val;
}

/* Extract block size and allocated status from a header/footer address */
static inline size_t get_size(const void *p) {
  return read_word(p) & SIZE_MASK;
}

static inline bool get_alloc(const void *p) {
  return (read_word(p) & ALLOC_MASK) != 0;
}

/* Given a block payload pointer bp, compute addresses of its header and footer */
static inline void *hdrp(const void *bp) {
  return (uint8_t *)bp - WORD_SIZE;
}

static inline void *ftrp(const void *bp) {
  return (uint8_t *)bp + get_size(hdrp(bp)) - (2 * WORD_SIZE);
}

/* Given a block payload pointer bp, compute payload pointers of next and previous blocks */
static inline void *next_blkp(const void *bp) {
  return (uint8_t *)bp + get_size((const uint8_t *)bp - WORD_SIZE);
}

static inline void *prev_blkp(const void *bp) {
  size_t prev_size = get_size((const uint8_t *)bp - (2 * WORD_SIZE));
  return (uint8_t *)bp - prev_size;
}

/* Align an arbitrary size upwards to the nearest multiple of ALIGNMENT (16) */
static inline size_t align_size(size_t size) {
  return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

/* ============================================================================
 * Core Allocator Drills (Implement the TODOs below)
 * ============================================================================ */

/*
 * Drill 1: Immediate 4-Case Boundary-Tag Coalescing
 *
 * Given a newly freed block payload pointer `bp`:
 * - Inspect the allocation status of the previous block (`get_alloc(ftrp(prev_blkp(bp)))`)
 *   and next block (`get_alloc(hdrp(next_blkp(bp)))`).
 * - Handle the 4 cases:
 *   - Case 1 (prev_alloc && next_alloc): Return bp unchanged.
 *   - Case 2 (prev_alloc && !next_alloc): Merge current and next block; update current header & next footer.
 *   - Case 3 (!prev_alloc && next_alloc): Merge prev and current block; update prev header & current footer; set bp = prev_blkp(bp).
 *   - Case 4 (!prev_alloc && !next_alloc): Merge prev, current, and next block; update prev header & next footer; set bp = prev_blkp(bp).
 * - Return the payload pointer of the coalesced free block.
 */
static void *coalesce(void *bp) {
  (void)bp;
  /* TODO: Implement 4-case boundary-tag coalescing */
  return nullptr;
}

/*
 * Drill 2: Arena Initialization with Prologue and Epilogue Sentinels
 *
 * Requirements:
 * - Reject arena_size < 64 bytes. Round arena_size up to a multiple of 4096 (OS page size).
 * - Request anonymous virtual memory via:
 *   mmap(nullptr, aligned_cap, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
 * - Format the slab starting at slab_base:
 *   - Bytes [0..7]:   Alignment padding word (0)
 *   - Bytes [8..15]:  Prologue Header: pack_word(16, true)
 *   - Bytes [16..23]: Prologue Footer: pack_word(16, true)
 *   - Bytes [24..N-9]: Initial Free Block Header & Footer of size (aligned_cap - 32), marked unallocated (false)
 *   - Bytes [N-8..N-1]: Epilogue Header: pack_word(0, true)
 * - Set g_arena.heap_listp = slab_base + 16 (pointing to Prologue Footer, so next_blkp(heap_listp) is the first regular block).
 */
[[nodiscard]] bool mm_init(size_t arena_size) {
  (void)arena_size;
  (void)MIN_BLOCK_SIZE;
  (void)pack_word;
  (void)write_word;
  (void)get_alloc;
  (void)ftrp;
  (void)prev_blkp;
  (void)align_size;
  /* TODO: Implement mmap slab allocation and sentinel block initialization */
  return false;
}

/*
 * Drill 3: Teardown Arena
 *
 * Requirements:
 * - If g_arena.slab_base != nullptr, unmap the slab via munmap(g_arena.slab_base, g_arena.slab_capacity).
 * - Zero out g_arena fields.
 */
void mm_teardown(void) {
  if (g_arena.slab_base != nullptr) {
    munmap(g_arena.slab_base, g_arena.slab_capacity);
    g_arena.slab_base = nullptr;
    g_arena.slab_capacity = 0;
    g_arena.heap_listp = nullptr;
  }
}

/*
 * Drill 4: First-Fit Search & Block Splitting (`mm_malloc`)
 *
 * Requirements:
 * - If size == 0 or g_arena.heap_listp == nullptr, return nullptr.
 * - Compute total adjusted block size `asize`:
 *   Add overhead (2 * WORD_SIZE = 16 bytes for Hdr + Ftr) to `size`, then round up via `align_size`.
 *   Enforce `asize >= MIN_BLOCK_SIZE` (32 bytes).
 *   Guard against integer overflow when `size > SIZE_MAX - 32`.
 * - Traverse the implicit list starting at `bp = next_blkp(g_arena.heap_listp)` until reaching Epilogue (`get_size(hdrp(bp)) == 0`).
 * - When a free block with `csize = get_size(hdrp(bp)) >= asize` is found:
 *   - If `(csize - asize) >= MIN_BLOCK_SIZE`:
 *     Split the block! Write allocated header/footer of `asize` at `bp`, and write free header/footer
 *     of `(csize - asize)` at the newly created remainder block `next_blkp(bp)`.
 *   - Else:
 *     Allocate the entire `csize` block (absorbing remainder as internal padding).
 *   - Return `bp`.
 * - If no block fits, return nullptr.
 */
[[nodiscard]] void *mm_malloc(size_t size) {
  (void)size;
  /* TODO: Implement size adjustment, first-fit traversal, and block splitting */
  return nullptr;
}

/*
 * Drill 5: Constant-Time Free & Coalesce (`mm_free`)
 *
 * Requirements:
 * - Define `mm_free(nullptr)` as a safe no-op.
 * - Read `size = get_size(hdrp(ptr))`.
 * - Update both header (`hdrp(ptr)`) and footer (`ftrp(ptr)`) to `pack_word(size, false)`.
 * - Invoke `coalesce(ptr)` immediately to merge any adjacent free blocks.
 */
void mm_free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }
  (void)coalesce;
  /* TODO: Mark block unallocated and coalesce */
}

/*
 * Drill 6: Heap Invariant Verifier (`mm_checkheap`)
 *
 * Verify all structural invariants of the implicit free list:
 * 1. Prologue header and footer are size 16 and marked allocated.
 * 2. Every block payload pointer `bp` is 16-byte aligned (`((uintptr_t)bp % 16) == 0`).
 * 3. Every block's size is at least `MIN_BLOCK_SIZE` (32) and a multiple of 16.
 * 4. Every block's header word exactly matches its footer word (`read_word(hdrp(bp)) == read_word(ftrp(bp))`).
 * 5. Coalescing invariant: No two consecutive blocks in the heap are both free (`!prev_free || !curr_free`).
 * 6. The loop terminates at the exact Epilogue header address (`hdrp(bp) == g_arena.slab_base + g_arena.slab_capacity - WORD_SIZE`)
 *    where `get_size(hdrp(bp)) == 0` and `get_alloc(hdrp(bp)) == true`.
 */
[[nodiscard]] bool mm_checkheap(void) {
  if (g_arena.heap_listp == nullptr) {
    return false;
  }
  /* TODO: Implement structural invariant checks across the heap */
  (void)next_blkp;
  return false;
}

/* ============================================================================
 * Verification Harness
 * ============================================================================ */
int main(void) {
  printf("=== Stage 1: Implicit Free List & Boundary-Tag Coalescer ===\n");

  /* Test 1: Arena Initialization & Sentinel Verification */
  assert(mm_init(65536) == true);
  assert(mm_checkheap() == true);

  /* Test 2: 16-Byte Alignment Across Arbitrary Request Sizes */
  size_t req_sizes[] = {1, 7, 8, 13, 16, 24, 31, 48, 64, 127};
  void *ptrs[10] = {0};
  for (size_t i = 0; i < 10; i++) {
    ptrs[i] = mm_malloc(req_sizes[i]);
    assert(ptrs[i] != nullptr);
    assert(((uintptr_t)ptrs[i] % ALIGNMENT) == 0);
    /* Fill payload to verify no header/footer overwrite */
    memset(ptrs[i], (int)(0xA0 + i), req_sizes[i]);
    assert(mm_checkheap() == true);
  }

  for (size_t i = 0; i < 10; i++) {
    mm_free(ptrs[i]);
    assert(mm_checkheap() == true);
  }

  /* Test 3: 4-Case Coalescing State Machine Verification */
  void *a = mm_malloc(64);
  void *b = mm_malloc(64);
  void *c = mm_malloc(64);
  void *d = mm_malloc(64); /* Guard block so c doesn't merge with wilderness */
  assert(a != nullptr && b != nullptr && c != nullptr && d != nullptr);

  /* Case 1: Free A (neighbors Prologue [alloc] and B [alloc]) */
  mm_free(a);
  assert(mm_checkheap() == true);

  /* Case 1: Free C (neighbors B [alloc] and D [alloc]) */
  mm_free(c);
  assert(mm_checkheap() == true);

  /* Case 4: Free B (neighbors A [free] and C [free]) -> Triple merge into single free block! */
  mm_free(b);
  assert(mm_checkheap() == true);

  /* Requesting 192 bytes must fit inside the merged A+B+C block starting at `a` */
  void *merged = mm_malloc(192);
  assert(merged == a);
  assert(mm_checkheap() == true);

  mm_free(merged);
  mm_free(d);
  assert(mm_checkheap() == true);

  /* Test 4: Overflow & Edge Case Defense */
  assert(mm_malloc(0) == nullptr);
  assert(mm_malloc(SIZE_MAX) == nullptr);
  assert(mm_malloc(SIZE_MAX - 16) == nullptr);
  mm_free(nullptr); /* Must be a safe no-op */

  mm_teardown();
  printf("[PASS] Stage 1: Implicit Free List & Boundary-Tag Coalescer verified!\n");
  return EXIT_SUCCESS;
}
