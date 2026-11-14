/*
 * Exercise 55: Cache Address Geometry, LRU Simulator & Tiled Blocking (CS:APP Ch. 5–6 / cachelab)
 *
 * Compile and run:
 *   make -C curriculum 55_cache_locality_and_blocking && ./curriculum/55_cache_locality_and_blocking
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
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

typedef struct {
  bool valid;
  uint64_t tag;
  uint64_t lru_clock;
} CacheLine;

typedef struct {
  uint32_t s;
  uint32_t E;
  uint32_t b;
  size_t num_sets;
  CacheLine *lines;
  uint64_t clock;
  size_t hits;
  size_t misses;
  size_t evictions;
} CacheSim;

/*
 * Drill 1: Address Bit Decomposition (tag | set_index | block_offset)
 *
 * For an address partitioned into `b` block-offset bits, `s` set-index bits,
 * and remaining `64 - s - b` tag bits:
 *
 * Requirements:
 * - Compute block_offset = lowest `b` bits of addr (0 if b == 0).
 * - Compute set_index    = next `s` bits `(addr >> b) & ((1ULL << s) - 1)` (0 if s == 0).
 * - Compute tag          = upper bits `addr >> (s + b)` (assume s + b < 64).
 * - Write results to non-null out pointers (tag_out, set_out, offset_out).
 */
static void cache_extract_bits(uint64_t addr, uint32_t s, uint32_t b,
                               uint64_t *tag_out, size_t *set_out,
                               size_t *offset_out) {
  (void)addr;
  (void)s;
  (void)b;
  if (tag_out != nullptr) *tag_out = 0;
  if (set_out != nullptr) *set_out = 0;
  if (offset_out != nullptr) *offset_out = 0;
  /* TODO: implement */
}

/*
 * Drill 2: Cache Simulator Lifecycle (`cachelab` Part A)
 *
 * Requirements for cache_sim_init:
 * - Return false if sim == nullptr || E == 0 || s >= 20 || (s + b) >= 64.
 * - Compute num_sets = (size_t)1U << s.
 * - Guard overflow on num_sets * E.
 * - Allocate `num_sets * E` zeroed CacheLine structs via calloc.
 * - Initialize s, E, b, num_sets, lines, clock = 0, hits = 0, misses = 0, evictions = 0.
 *
 * Requirements for cache_sim_destroy:
 * - If sim == nullptr, return.
 * - Free sim->lines, set sim->lines = nullptr, and zero metadata fields.
 */
static bool cache_sim_init(CacheSim *sim, uint32_t s, uint32_t E, uint32_t b) {
  (void)sim;
  (void)s;
  (void)E;
  (void)b;
  return false; /* TODO: implement */
}

static void cache_sim_destroy(CacheSim *sim) {
  (void)sim;
  /* TODO: implement */
}

/*
 * Drill 3: Set-Associative Cache Lookup with LRU Replacement
 *
 * Requirements:
 * - If sim == nullptr || sim->lines == nullptr, return.
 * - Extract tag and set_idx using cache_extract_bits.
 * - Inspect lines in set `set_idx` (`&sim->lines[set_idx * sim->E]`):
 *     1. Hit: if a line has `valid && tag == line.tag`:
 *          increment sim->hits, set `line.lru_clock = ++sim->clock`, and return.
 *     2. Miss: increment sim->misses.
 *          Find an empty line (`!valid`) if one exists;
 *          otherwise find the victim line with the smallest `lru_clock` and
 *          increment sim->evictions.
 *          Store `valid = true`, `tag = tag`, `lru_clock = ++sim->clock`.
 */
static void cache_sim_access(CacheSim *sim, uint64_t addr) {
  (void)sim;
  (void)addr;
  /* TODO: implement */
}

/*
 * Drill 4: Cache-Blocked (Tiled) Matrix Transpose (`cachelab` Part B)
 *
 * Naive transpose (`dst[j * n + i] = src[i * n + j]`) scans `src` row-wise
 * (spatial locality) while writing `dst` column-wise (stride = n * sizeof(int)),
 * thrashing cache sets before columns are reused.
 *
 * Requirements:
 * - Return immediately if src == nullptr || dst == nullptr || n == 0 || block_size == 0.
 * - Tile outer loops i0, j0 in steps of `block_size`.
 * - Clamp inner loop bounds (`i < n`, `j < n`) so non-divisible `n` works cleanly.
 * - For each element (i, j):
 *     If optional_sim != nullptr:
 *       cache_sim_access(optional_sim, (uint64_t)(uintptr_t)&src[i * n + j]);
 *       cache_sim_access(optional_sim, (uint64_t)(uintptr_t)&dst[j * n + i]);
 *     Assign dst[j * n + i] = src[i * n + j];
 */
static void transpose_blocked(size_t n, const int *src, int *dst,
                              size_t block_size, CacheSim *optional_sim) {
  (void)n;
  (void)src;
  (void)dst;
  (void)block_size;
  (void)optional_sim;
  /* TODO: implement */
}

static void transpose_naive(size_t n, const int *src, int *dst,
                            CacheSim *optional_sim) {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      if (optional_sim != nullptr) {
        cache_sim_access(optional_sim, (uint64_t)(uintptr_t)&src[i * n + j]);
        cache_sim_access(optional_sim, (uint64_t)(uintptr_t)&dst[j * n + i]);
      }
      dst[j * n + i] = src[i * n + j];
    }
  }
}

int main(void) {
  /* Drill 1: Address bit extraction (s=4, b=5 -> 16 sets, 32-byte lines) */
  uint64_t tag = 0;
  size_t set_idx = 0;
  size_t offset = 0;
  cache_extract_bits(0x00001234ULL, 4, 5, &tag, &set_idx, &offset);
  assert(offset == (0x1234U & 0x1FU));
  assert(set_idx == ((0x1234U >> 5) & 0xFU));
  assert(tag == (0x1234U >> 9));

  /* Drill 2 & 3: Direct-mapped & 2-way LRU trace verification */
  CacheSim sim = {0};
  assert(cache_sim_init(nullptr, 2, 1, 4) == false);
  assert(cache_sim_init(&sim, 2, 0, 4) == false);
  assert(cache_sim_init(&sim, 2, 2, 4) == true); /* 4 sets, 2-way, 16B blocks */

  /* Accesses in set 0: tags 0x10, 0x20, 0x10 (hit!), 0x30 (evicts 0x20!), 0x20 (miss+evict!) */
  cache_sim_access(&sim, 0x10ULL << 6); /* miss */
  cache_sim_access(&sim, 0x20ULL << 6); /* miss */
  cache_sim_access(&sim, 0x10ULL << 6); /* hit, refreshes 0x10 LRU */
  cache_sim_access(&sim, 0x30ULL << 6); /* miss + evicts 0x20 */
  cache_sim_access(&sim, 0x10ULL << 6); /* hit! */
  cache_sim_access(&sim, 0x20ULL << 6); /* miss + evicts 0x30 */

  assert(sim.hits == 2);
  assert(sim.misses == 4);
  assert(sim.evictions == 2);
  cache_sim_destroy(&sim);
  assert(sim.lines == nullptr);

  /* Drill 4a: Non-divisible size correctness (13x13, block=4) */
  size_t n_odd = 13;
  int *src_odd = malloc(n_odd * n_odd * sizeof(int));
  int *dst_odd = calloc(n_odd * n_odd, sizeof(int));
  assert(src_odd != nullptr && dst_odd != nullptr);
  for (size_t i = 0; i < n_odd * n_odd; i++) {
    src_odd[i] = (int)(i + 1);
  }
  transpose_blocked(n_odd, src_odd, dst_odd, 4, nullptr);
  for (size_t i = 0; i < n_odd; i++) {
    for (size_t j = 0; j < n_odd; j++) {
      assert(dst_odd[j * n_odd + i] == src_odd[i * n_odd + j]);
    }
  }
  free(src_odd);
  free(dst_odd);

  /* Drill 4b: Cache miss reduction on 32x32 transpose (s=4, E=2, b=5: 1KB 2-way) */
  size_t n = 32;
  int *arena = calloc(2 * n * n, sizeof(int));
  assert(arena != nullptr);
  int *src = arena;
  int *dst = arena + n * n;
  for (size_t i = 0; i < n * n; i++) {
    src[i] = (int)i;
  }

  CacheSim naive_sim = {0};
  CacheSim blocked_sim = {0};
  assert(cache_sim_init(&naive_sim, 4, 2, 5) == true);
  assert(cache_sim_init(&blocked_sim, 4, 2, 5) == true);

  transpose_naive(n, src, dst, &naive_sim);
  memset(dst, 0, n * n * sizeof(int));
  transpose_blocked(n, src, dst, 8, &blocked_sim);

  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      assert(dst[j * n + i] == src[i * n + j]);
    }
  }
  assert(blocked_sim.misses * 2 < naive_sim.misses);

  cache_sim_destroy(&naive_sim);
  cache_sim_destroy(&blocked_sim);
  free(arena);

  printf("[PASS] Exercise 55: Cache Locality & Tiled Blocking verified.\n");
  return EXIT_SUCCESS;
}
