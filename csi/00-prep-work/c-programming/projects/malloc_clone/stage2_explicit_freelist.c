/*
 * Stage 2: Intrusive Explicit Doubly-Linked Free List & Footer Elision
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o stage2_explicit stage2_explicit_freelist.c && ./stage2_explicit
 *
 * Learning Objectives:
 * - Reduce allocation search time from O(N_total_blocks) to O(N_free_blocks) by threading an
 *   intrusive doubly-linked list (`FreeNode`) directly inside the unused payload of free blocks.
 * - Eliminate the 8-byte footer on ALLOCATED blocks using bit 1 (`PREV_ALLOC_MASK = 0x2`) in
 *   the block header (Koppel's representation optimization / TCMalloc & dlmalloc technique).
 * - Support O(1) LIFO insertion on `mm_free` and O(1) node removal during coalescing.
 * - Implement in-place `mm_realloc` expansion when the next physical block is free.
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
 * Allocated Block (NO FOOTER! Minimum size = 32B so it can become a FreeNode when freed,
 * OR 16B if using compact 32-bit offsets; here we use 32B minimum block size with 8B header
 * and 24B usable payload!):
 * +-----------------------------------+---------------------------------------------------+
 * | Header: size | PREV_ALLOC | 0x1   | User Payload (up to size - 8 bytes!)              |
 * +-----------------------------------+---------------------------------------------------+
 *
 * Free Block (Minimum 32 Bytes: 8B Header + 8B prev + 8B next + 8B Footer):
 * +-------------------+-------------------+-------------------+-------------------+
 * | Header: size |PA|0| FreeNode *prev    | FreeNode *next    | Footer: size |PA|0|
 * +-------------------+-------------------+-------------------+-------------------+
 */
static const size_t WORD_SIZE = sizeof(size_t);          /* 8 bytes */
static const size_t ALIGNMENT = 16;                      /* 16-byte alignment */
static const size_t MIN_BLOCK_SIZE = 32;                 /* 8B Hdr + 16B FreeNode + 8B Ftr */
static const size_t ALLOC_MASK = 0x1;                    /* Bit 0: Current block allocated */
static const size_t PREV_ALLOC_MASK = 0x2;               /* Bit 1: Previous physical block allocated */
static const size_t SIZE_MASK = ~(size_t)(ALIGNMENT - 1);/* Clears lower 4 bits */

/* Intrusive Free List Node overlaid inside a Free Block's payload */
typedef struct FreeNode {
  struct FreeNode *prev;
  struct FreeNode *next;
} FreeNode;

typedef struct {
  uint8_t *slab_base;
  size_t slab_capacity;
  uint8_t *heap_listp;
  FreeNode *free_head;  /* Head of explicit LIFO doubly-linked free list */
} ExplicitArena;

static ExplicitArena g_arena = {0};

/* ============================================================================
 * Bitwise & Block Navigation Primitives
 * ============================================================================ */

static inline size_t pack_word(size_t size, bool prev_alloc, bool is_alloc) {
  return size | (prev_alloc ? PREV_ALLOC_MASK : 0) | (is_alloc ? ALLOC_MASK : 0);
}

static inline size_t read_word(const void *p) {
  return *(const size_t *)p;
}

static inline void write_word(void *p, size_t val) {
  *(size_t *)p = val;
}

static inline size_t get_size(const void *p) {
  return read_word(p) & SIZE_MASK;
}

static inline bool get_alloc(const void *p) {
  return (read_word(p) & ALLOC_MASK) != 0;
}

static inline bool get_prev_alloc(const void *p) {
  return (read_word(p) & PREV_ALLOC_MASK) != 0;
}

static inline void *hdrp(const void *bp) {
  return (uint8_t *)bp - WORD_SIZE;
}

/* Note: ftrp(bp) is ONLY valid on FREE blocks because allocated blocks have no footer! */
static inline void *ftrp(const void *bp) {
  return (uint8_t *)bp + get_size(hdrp(bp)) - (2 * WORD_SIZE);
}

static inline void *next_blkp(const void *bp) {
  return (uint8_t *)bp + get_size(hdrp(bp));
}

/* Note: prev_blkp(bp) is ONLY valid when !get_prev_alloc(hdrp(bp))! */
static inline void *prev_blkp(const void *bp) {
  size_t prev_size = get_size((const uint8_t *)bp - (2 * WORD_SIZE));
  return (uint8_t *)bp - prev_size;
}

static inline void set_next_prev_alloc(const void *bp, bool is_alloc) {
  void *next_hdr = hdrp(next_blkp(bp));
  size_t word = read_word(next_hdr);
  if (is_alloc) {
    word |= PREV_ALLOC_MASK;
  } else {
    word &= ~PREV_ALLOC_MASK;
  }
  write_word(next_hdr, word);
}

/* ============================================================================
 * Core Allocator Drills (Implement the TODOs below)
 * ============================================================================ */

/*
 * Drill 1: Intrusive Doubly-Linked List Insertion (LIFO Head) & Removal
 *
 * Requirements:
 * - `freelist_insert_head`: Insert `node` at `g_arena.free_head` in O(1).
 *   Set `node->prev = nullptr`, `node->next = *head`. Update old head's `prev` if non-null.
 * - `freelist_remove`: Splice `node` out of `*head` in O(1).
 *   Update `node->prev->next` (or `*head` if `node->prev == nullptr`) and `node->next->prev`.
 *   Null out `node->prev` and `node->next` to define dangling links out of existence.
 */
void freelist_insert_head(FreeNode **head, FreeNode *node) {
  (void)head;
  (void)node;
  /* TODO: Implement O(1) LIFO head insertion */
}

void freelist_remove(FreeNode **head, FreeNode *node) {
  (void)head;
  (void)node;
  /* TODO: Implement O(1) doubly-linked node removal */
}

/*
 * Drill 2: Coalescing with Intrusive Free List Maintenance & PREV_ALLOC Propagation
 *
 * Given a newly freed block `bp`:
 * - Read `prev_alloc = get_prev_alloc(hdrp(bp))` and `next_alloc = get_alloc(hdrp(next_blkp(bp)))`.
 * - If next block is free: remove `(FreeNode *)next_blkp(bp)` from `g_arena.free_head` and add its size.
 * - If prev block is free (`!prev_alloc`): remove `(FreeNode *)prev_blkp(bp)` from `g_arena.free_head`,
 *   add its size, and move `bp = prev_blkp(bp)`.
 * - Write updated header and footer on the merged free block (preserving `get_prev_alloc(hdrp(bp))`).
 * - Clear the `PREV_ALLOC` bit on the physical block following the merged block (`set_next_prev_alloc(bp, false)`).
 * - Insert `(FreeNode *)bp` at `g_arena.free_head` and return `bp`.
 */
static void *coalesce(void *bp) {
  (void)bp;
  (void)ftrp;
  (void)prev_blkp;
  (void)set_next_prev_alloc;
  /* TODO: Implement coalescing with freelist splicing and PREV_ALLOC bit updates */
  return nullptr;
}

/*
 * Drill 3: Arena Initialization
 *
 * Format the mmap slab with Prologue (16B, allocated), initial free block (inserted into
 * `g_arena.free_head` with `prev_alloc = true`), and Epilogue (0B, allocated, `prev_alloc = false`).
 */
[[nodiscard]] bool mm_init(size_t arena_size) {
  (void)arena_size;
  (void)MIN_BLOCK_SIZE;
  (void)pack_word;
  (void)get_alloc;
  (void)get_prev_alloc;
  /* TODO: Initialize mmap slab, sentinel blocks, and initial FreeNode in g_arena.free_head */
  return false;
}

void mm_teardown(void) {
  if (g_arena.slab_base != nullptr) {
    munmap(g_arena.slab_base, g_arena.slab_capacity);
    g_arena = (ExplicitArena){0};
  }
}

/*
 * Drill 4: Explicit Free List Allocation with Footer Elision (`mm_malloc`)
 *
 * Requirements:
 * - Because allocated blocks have NO footer, payload overhead is only `WORD_SIZE` (8 bytes)!
 *   Compute `asize = align_up(size + WORD_SIZE, 16)`, clamped to at least `MIN_BLOCK_SIZE` (32 bytes).
 *   (Thus a 24-byte user request needs 24 + 8 = 32 bytes total, fitting in a 32-byte block!)
 * - Traverse ONLY `g_arena.free_head` (not the entire heap!).
 * - When a suitable `FreeNode *curr` with `csize >= asize` is found:
 *   - Remove `curr` from `g_arena.free_head`.
 *   - If `(csize - asize) >= MIN_BLOCK_SIZE`:
 *     Split! Mark first `asize` bytes as allocated (header only, preserving `prev_alloc`),
 *     create remainder free block of `(csize - asize)` with `prev_alloc = true` (both header and footer),
 *     and insert remainder into `g_arena.free_head`.
 *   - Else:
 *     Mark entire `csize` block as allocated and call `set_next_prev_alloc(curr, true)`.
 *   - Return `(void *)curr`.
 */
[[nodiscard]] void *mm_malloc(size_t size) {
  (void)size;
  /* TODO: Traverse g_arena.free_head, unlink, split if remainder >= 32B, and update PREV_ALLOC */
  return nullptr;
}

/*
 * Drill 5: Free with Footer Restoration (`mm_free`)
 */
void mm_free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }
  (void)coalesce;
  /* TODO: Write free header and footer (preserving prev_alloc), then call coalesce(ptr) */
}

/*
 * Drill 6: In-Place Reallocation (`mm_realloc`)
 *
 * Requirements:
 * - If `ptr == nullptr`, equivalent to `mm_malloc(new_size)`.
 * - If `new_size == 0`, call `mm_free(ptr)` and return `nullptr`.
 * - If current block size `csize >= asize`, return `ptr` in-place.
 * - If next physical block (`next_blkp(ptr)`) is free and `csize + next_size >= asize`:
 *   Splice `next_blkp(ptr)` out of `g_arena.free_head`, combine sizes in-place without `memcpy`,
 *   split trailing remainder if `>= MIN_BLOCK_SIZE`, and return `ptr`!
 * - Otherwise, allocate new block via `mm_malloc(new_size)`, `memcpy` old payload (`csize - WORD_SIZE`),
 *   `mm_free(ptr)`, and return new block.
 */
[[nodiscard]] void *mm_realloc(void *ptr, size_t new_size) {
  (void)ptr;
  (void)new_size;
  /* TODO: Implement in-place expansion when next block is free, with fallback to malloc+memcpy+free */
  return nullptr;
}

/*
 * Drill 7: Explicit Free List & Boundary Invariant Checker (`mm_checkheap`)
 *
 * Verify:
 * 1. Every node in `g_arena.free_head` has consistent `prev`/`next` pointers (no cycles or broken links)
 *    and is marked unallocated (`!get_alloc(hdrp(node))`).
 * 2. Every physical block's `PREV_ALLOC` bit matches the actual `ALLOC` bit of the preceding physical block.
 * 3. Every free block in the physical heap walk is present in `g_arena.free_head` (count of free blocks
 *    in physical walk == count of nodes in `g_arena.free_head`).
 */
[[nodiscard]] bool mm_checkheap(void) {
  if (g_arena.heap_listp == nullptr) {
    return false;
  }
  /* TODO: Verify both physical block walk and explicit free list invariants */
  return false;
}

/* ============================================================================
 * Verification Harness
 * ============================================================================ */
int main(void) {
  printf("=== Stage 2: Intrusive Explicit Free List & Footer Elision ===\n");

  assert(mm_init(65536) == true);
  assert(mm_checkheap() == true);

  /* Test 1: Footer Elision Payload Density Verification
   * A 24-byte request + 8-byte header = 32 bytes total!
   * In Stage 1 (with 8B footer), 24B + 16B = 40B -> rounded to 48B.
   * Verify that consecutive 24-byte allocations are spaced exactly 32 bytes apart!
   */
  void *p1 = mm_malloc(24);
  void *p2 = mm_malloc(24);
  assert(p1 != nullptr && p2 != nullptr);
  assert((uintptr_t)p2 - (uintptr_t)p1 == 32);
  /* Write all 24 bytes of p1 and verify p2's header was not corrupted */
  memset(p1, 0xAA, 24);
  memset(p2, 0xBB, 24);
  assert(mm_checkheap() == true);

  mm_free(p1);
  mm_free(p2);
  assert(mm_checkheap() == true);

  /* Test 2: Intrusive Free List Stress & Pattern Integrity */
  enum { NUM_BLOCKS = 64 };
  uint8_t *blocks[NUM_BLOCKS] = {0};
  for (int i = 0; i < NUM_BLOCKS; i++) {
    blocks[i] = mm_malloc(40);
    assert(blocks[i] != nullptr);
    memset(blocks[i], (uint8_t)(i + 1), 40);
  }
  assert(mm_checkheap() == true);

  /* Free all even-indexed blocks (creates 32 non-coalescable free blocks in explicit list) */
  for (int i = 0; i < NUM_BLOCKS; i += 2) {
    mm_free(blocks[i]);
    blocks[i] = nullptr;
  }
  assert(mm_checkheap() == true);

  /* Verify odd-indexed blocks still hold their exact byte patterns */
  for (int i = 1; i < NUM_BLOCKS; i += 2) {
    for (int b = 0; b < 40; b++) {
      assert(blocks[i][b] == (uint8_t)(i + 1));
    }
  }

  /* Free odd-indexed blocks -> triggers Case 4 coalescing across all 64 blocks! */
  for (int i = 1; i < NUM_BLOCKS; i += 2) {
    mm_free(blocks[i]);
    blocks[i] = nullptr;
  }
  assert(mm_checkheap() == true);

  /* Test 3: In-Place Realloc Absorption of Free Next Neighbor */
  uint8_t *r1 = mm_malloc(24);
  uint8_t *r2 = mm_malloc(64);
  uint8_t *r3 = mm_malloc(24); /* Guard block */
  assert(r1 != nullptr && r2 != nullptr && r3 != nullptr);
  for (int i = 0; i < 24; i++) {
    r1[i] = (uint8_t)(0x10 + i);
  }

  /* Free r2 so that the block immediately following r1 is free */
  mm_free(r2);
  assert(mm_checkheap() == true);

  /* Expand r1 from 24 bytes to 72 bytes: must absorb r2 in-place without moving r1! */
  uint8_t *r1_expanded = mm_realloc(r1, 72);
  assert(r1_expanded == r1);
  for (int i = 0; i < 24; i++) {
    assert(r1_expanded[i] == (uint8_t)(0x10 + i));
  }
  assert(mm_checkheap() == true);

  /* Also verify mm_realloc contract edge cases: shrink in-place, nullptr alloc, and 0-size free */
  uint8_t *r1_shrunk = mm_realloc(r1_expanded, 16);
  assert(r1_shrunk == r1_expanded);
  void *r_from_null = mm_realloc(nullptr, 32);
  assert(r_from_null != nullptr);
  assert(mm_realloc(r_from_null, 0) == nullptr);

  mm_free(r1_shrunk);
  mm_free(r3);
  assert(mm_checkheap() == true);

  mm_teardown();
  printf("[PASS] Stage 2: Intrusive Explicit Free List & Footer Elision verified!\n");
  return EXIT_SUCCESS;
}
