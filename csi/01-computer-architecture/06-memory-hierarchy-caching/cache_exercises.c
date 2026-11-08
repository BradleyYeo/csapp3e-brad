/*
 * Progressive Deliberate Practice: The Memory Hierarchy & Hardware Caching
 * (CS:APP3e Chapter 6, Sections 6.1 - 6.6)
 *
 * Build and run all stages:
 *   make test
 *
 * Run a specific stage (1 to 6):
 *   make test STAGE=1
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
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * Core Data Structures and Types
 * ============================================================================
 */

typedef struct {
  uint32_t s;                     /* Number of set index bits */
  uint32_t E;                     /* Associativity (lines per set) */
  uint32_t b;                     /* Number of block offset bits */
  uint32_t tag_bits;              /* t = 64 - (s + b) */
  uint64_t num_sets;              /* S = 2^s */
  uint64_t block_size;            /* B = 2^b */
  uint64_t capacity_bytes;        /* C = S * E * B */
  uint64_t critical_stride_bytes; /* S * B (set wrap-around period) */
} cache_geometry_t;

typedef struct {
  uint64_t tag;
  uint64_t set_index;
  uint64_t offset;
} decoded_addr_t;

typedef enum {
  LOOP_ROW_MAJOR = 0, /* Inner loop increments column j: x[i][j] */
  LOOP_COL_MAJOR = 1  /* Inner loop increments row j:    x[j][i] */
} loop_order_t;

typedef enum {
  WRITE_BACK_ALLOCATE = 0,     /* Allocate on miss, write to RAM only on dirty eviction */
  WRITE_THROUGH_NO_ALLOC = 1   /* Never allocate on write miss, every store writes RAM */
} write_policy_t;

typedef enum {
  MEM_LOAD = 0,
  MEM_STORE = 1
} mem_op_t;

typedef struct {
  bool valid;
  bool dirty;
  uint64_t tag;
  uint64_t lru_clock;
} sim_line_t;

typedef struct {
  cache_geometry_t geom;
  write_policy_t policy;
  sim_line_t *lines; /* Flat array of S * E lines: set i starts at i * E */
  uint64_t clock;
  uint64_t hits;
  uint64_t misses;
  uint64_t evictions;
  uint64_t dirty_writebacks;
  uint64_t main_memory_writes;
} sim_cache_t;

/*
 * Simulator Lifecycle Helpers (Provided)
 */
static void sim_cache_init(sim_cache_t *cache, uint32_t s, uint32_t E,
                           uint32_t b, write_policy_t policy) {
  memset(cache, 0, sizeof(*cache));
  cache->geom.s = s;
  cache->geom.E = E;
  cache->geom.b = b;
  cache->geom.tag_bits = 64 - (s + b);
  cache->geom.num_sets = 1ULL << s;
  cache->geom.block_size = 1ULL << b;
  cache->geom.capacity_bytes = cache->geom.num_sets * E * cache->geom.block_size;
  cache->geom.critical_stride_bytes = cache->geom.num_sets * cache->geom.block_size;
  cache->policy = policy;
  cache->lines = calloc(cache->geom.num_sets * E, sizeof(sim_line_t));
  assert(cache->lines != nullptr);
}

static void sim_cache_reset_stats(sim_cache_t *cache) {
  memset(cache->lines, 0,
         cache->geom.num_sets * cache->geom.E * sizeof(sim_line_t));
  cache->clock = 0;
  cache->hits = 0;
  cache->misses = 0;
  cache->evictions = 0;
  cache->dirty_writebacks = 0;
  cache->main_memory_writes = 0;
}

static void sim_cache_destroy(sim_cache_t *cache) {
  free(cache->lines);
  cache->lines = nullptr;
}

/*
 * ============================================================================
 * Stage 1: Cache Geometry & Bitwise Address Decoding (CS:APP 6.4.1)
 * ============================================================================
 *
 * Precondition:
 *   s + b < 64, E >= 1, out != nullptr.
 * Postcondition:
 *   Populates all fields of *out:
 *   - num_sets = 2^s
 *   - block_size = 2^b
 *   - tag_bits = 64 - (s + b)
 *   - capacity_bytes = S * E * B
 *   - critical_stride_bytes = S * B
 *
 * Mental Model Invariant:
 *   Always use 1ULL << s and 1ULL << b to avoid 32-bit signed shift UB.
 */
static void compute_cache_geometry(uint32_t s, uint32_t E, uint32_t b,
                                   cache_geometry_t *out) {
  // TODO (Stage 1.1): Compute cache geometry parameters
  (void)s;
  (void)E;
  (void)b;
  memset(out, 0, sizeof(*out));
}

/*
 * Precondition:
 *   s + b < 64.
 * Postcondition:
 *   Extracts offset (low b bits), set_index (middle s bits), and tag (upper bits).
 */
static decoded_addr_t decode_cache_address(uint64_t addr, uint32_t s,
                                           uint32_t b) {
  // TODO (Stage 1.2): Extract offset, set_index, and tag using 1ULL masks
  (void)addr;
  (void)s;
  (void)b;
  return (decoded_addr_t){0, 0, 0};
}

/*
 * Precondition:
 *   s > 0 && s < 64.
 * Postcondition:
 *   Extracts the highest s bits of the 64-bit address (bits [64-s .. 63]).
 *   Demonstrates why high-bit indexing fails for contiguous arrays.
 */
static uint64_t decode_flawed_high_bit_index(uint64_t addr, uint32_t s) {
  // TODO (Stage 1.3): Extract the top s bits of addr
  (void)addr;
  (void)s;
  return 0;
}

/*
 * ============================================================================
 * Stage 2: Analytical Miss Rate & 2D Loop Miss Predictor (CS:APP 6.2 - 6.4)
 * ============================================================================
 *
 * Precondition:
 *   stride_elems >= 1, elem_size >= 1, block_size >= elem_size.
 * Postcondition:
 *   Returns the theoretical miss rate in [0.0, 1.0] for scanning an array
 *   much larger than cache capacity with step size `stride_elems`:
 *     miss_rate = min(1.0, (stride_elems * elem_size) / (double)block_size)
 */
static double predict_stride_miss_rate(size_t stride_elems, size_t elem_size,
                                       size_t block_size) {
  // TODO (Stage 2.1): Compute theoretical miss rate for Stride-k traversal
  (void)stride_elems;
  (void)elem_size;
  (void)block_size;
  return 0.0;
}

/*
 * Precondition:
 *   rows >= 1, cols >= 1, elem_size >= 1, block_size >= elem_size.
 *   (rows * cols * elem_size) is a multiple of block_size.
 * Postcondition:
 *   Returns the predicted number of L1 write misses when writing all
 *   rows * cols elements of `x[rows][cols]`:
 *   - Case 1 (LOOP_ROW_MAJOR):
 *     Sequential Stride-1 scan across all rows * cols elements.
 *     Misses = (rows * cols * elem_size) / block_size.
 *   - Case 2 (LOOP_COL_MAJOR):
 *     Inner loop steps down a column (byte stride = cols * elem_size).
 *     - Subcase 2a (Working set fits in cache: rows * cols * elem_size <= cache_capacity):
 *       Lines loaded during column 0 remain in cache for columns 1..cols-1!
 *       Misses = (rows * cols * elem_size) / block_size (only compulsory misses).
 *     - Subcase 2b (Working set exceeds cache capacity):
 *       Each column step advances by `row_bytes = cols * elem_size`.
 *       If `row_bytes >= block_size`, every single access misses (rows * cols).
 *       If `row_bytes < block_size` (e.g. option_five x[4000][8] where row_bytes = 32B
 *       and block_size = 64B), `rows_per_line = block_size / row_bytes` consecutive
 *       inner-loop steps share the same cache line before eviction!
 *       Misses = (rows * cols) / (block_size / row_bytes).
 */
static uint64_t predict_2d_loop_write_misses(size_t rows, size_t cols,
                                             size_t elem_size,
                                             size_t block_size,
                                             size_t cache_capacity,
                                             loop_order_t order) {
  // TODO (Stage 2.2): Implement analytical 2D loop write miss predictor
  (void)rows;
  (void)cols;
  (void)elem_size;
  (void)block_size;
  (void)cache_capacity;
  (void)order;
  return 0;
}

/*
 * ============================================================================
 * Stage 3: LRU Cache & Write Policy State Machine (CS:APP 6.4.2 - 6.4.4)
 * ============================================================================
 *
 * Precondition:
 *   cache is an initialized sim_cache_t, op is MEM_LOAD or MEM_STORE.
 * Postcondition:
 *   Advances cache->clock++, decodes addr into set_index and tag, and updates:
 *   - On Hit (valid && line.tag == tag):
 *     - cache->hits++
 *     - line.lru_clock = cache->clock
 *     - If op == MEM_STORE:
 *       - If policy == WRITE_BACK_ALLOCATE: line.dirty = true
 *       - If policy == WRITE_THROUGH_NO_ALLOC: cache->main_memory_writes++
 *   - On Miss:
 *     - cache->misses++
 *     - If op == MEM_STORE && policy == WRITE_THROUGH_NO_ALLOC:
 *       - Do NOT allocate a line in cache; simply increment cache->main_memory_writes++
 *         and return false (miss).
 *     - Otherwise (MEM_LOAD, or MEM_STORE with WRITE_BACK_ALLOCATE):
 *       - Select target line: first invalid line (`!valid`) in set; if all E lines
 *         are valid, increment `cache->evictions++` and pick the victim line with
 *         the minimum `lru_clock`.
 *       - If evicting a valid victim with `victim.dirty == true`:
 *         increment `cache->dirty_writebacks++` and `cache->main_memory_writes++`.
 *       - Initialize target line: `valid = true`, `tag = decoded.tag`,
 *         `lru_clock = cache->clock`, `dirty = (op == MEM_STORE && policy == WRITE_BACK_ALLOCATE)`.
 *   Returns true on cache hit, false on cache miss.
 */
static bool sim_cache_access(sim_cache_t *cache, uint64_t addr, mem_op_t op) {
  // TODO (Stage 3): Implement LRU cache access and write policy state machine
  (void)cache;
  (void)addr;
  (void)op;
  return false;
}

/*
 * ============================================================================
 * Stage 4: Direct-Mapped Conflict Thrashing & Array Padding (CS:APP 6.4.2)
 * ============================================================================
 *
 * Precondition:
 *   geom points to a valid cache_geometry_t.
 * Postcondition:
 *   Returns true if `addr_a` and `addr_b` map to the exact same cache set index
 *   (i.e. their set_index bits are identical), false otherwise.
 */
static bool is_set_aliasing(uint64_t addr_a, uint64_t addr_b,
                            const cache_geometry_t *geom) {
  // TODO (Stage 4.1): Check if addr_a and addr_b map to the same cache set
  (void)addr_a;
  (void)addr_b;
  (void)geom;
  return false;
}

/*
 * Precondition:
 *   row_cols >= 1, elem_size >= 1, geom != nullptr.
 * Postcondition:
 *   If a row stride `row_cols * elem_size` is an exact multiple of
 *   `geom->critical_stride_bytes`, adjacent rows `A[i][j]` and `A[i+1][j]`
 *   map to the exact same cache set and thrash during column-wise access.
 *   Returns 0 if no critical-stride aliasing occurs; otherwise returns the
 *   number of padding elements occupying exactly 1 cache block (`block_size / elem_size`)
 *   to shift each subsequent row into the next cache set.
 */
static size_t compute_row_padding_elements(size_t row_cols, size_t elem_size,
                                           const cache_geometry_t *geom) {
  // TODO (Stage 4.2): Compute padding elements needed to break critical-stride aliasing
  (void)row_cols;
  (void)elem_size;
  (void)geom;
  return 0;
}

/*
 * ============================================================================
 * Stage 5: Matrix Multiplication Loop Permutations (CS:APP 6.5.3)
 * ============================================================================
 *
 * In all three functions below:
 * - Matrices C, A, B are flat row-major arrays of `n * n` doubles.
 * - Base virtual addresses for simulation are:
 *     base_A = 0x100000ULL
 *     base_B = 0x200000ULL
 *     base_C = 0x300000ULL
 * - Element (r, c) of matrix M has simulated byte address:
 *     addr_M(r, c) = base_M + ((r * n + c) * sizeof(double))
 *
 * IMPORTANT: Record memory accesses in `cache` ONLY inside the innermost loop!
 */
static constexpr uint64_t SIM_BASE_A = 0x100000ULL;
static constexpr uint64_t SIM_BASE_B = 0x200000ULL;
static constexpr uint64_t SIM_BASE_C = 0x300000ULL;

static inline uint64_t mat_addr(uint64_t base, size_t n, size_t r, size_t c) {
  return base + (uint64_t)((r * n + c) * sizeof(double));
}

/*
 * Class 1 (`ijk` Dot-Product):
 *   for i = 0..n-1:
 *     for j = 0..n-1:
 *       sum = 0.0
 *       for k = 0..n-1:
 *         MEM_LOAD A[i][k], MEM_LOAD B[k][j]
 *         sum += A[i][k] * B[k][j]
 *       C[i][j] = sum
 */
static void sim_matmul_ijk(sim_cache_t *cache, double *C, const double *A,
                           const double *B, size_t n) {
  // TODO (Stage 5.1): Implement ijk loop order and record inner-loop loads in cache
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

/*
 * Class 2 (`jki` Column-Scale — Worst Spatial Locality):
 *   Zero C first.
 *   for j = 0..n-1:
 *     for k = 0..n-1:
 *       r = B[k][j]
 *       for i = 0..n-1:
 *         MEM_LOAD A[i][k], MEM_LOAD C[i][j], MEM_STORE C[i][j]
 *         C[i][j] += A[i][k] * r
 */
static void sim_matmul_jki(sim_cache_t *cache, double *C, const double *A,
                           const double *B, size_t n) {
  // TODO (Stage 5.2): Implement jki loop order and record inner-loop accesses in cache
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

/*
 * Class 3 (`ikj` Row-SAXPY — Optimal Stride-1 Inner Loop):
 *   Zero C first.
 *   for i = 0..n-1:
 *     for k = 0..n-1:
 *       r = A[i][k]
 *       for j = 0..n-1:
 *         MEM_LOAD B[k][j], MEM_LOAD C[i][j], MEM_STORE C[i][j]
 *         C[i][j] += r * B[k][j]
 */
static void sim_matmul_ikj(sim_cache_t *cache, double *C, const double *A,
                           const double *B, size_t n) {
  // TODO (Stage 5.3): Implement ikj loop order and record inner-loop accesses in cache
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

/*
 * ============================================================================
 * Stage 6: 2D Cache Blocking / Tiling & Working Set Fitting (CS:APP 6.5.3)
 * ============================================================================
 *
 * Precondition:
 *   l1_capacity_bytes >= 3 * elem_size, elem_size >= 1.
 * Postcondition:
 *   Returns the largest power-of-two tile dimension T (e.g. 1, 2, 4, 8, 16, 32, 64...)
 *   such that 3 full tiles (one for A, one for B, one for C) fit simultaneously
 *   inside the L1 cache:
 *     3 * (T * T) * elem_size <= l1_capacity_bytes
 */
static size_t compute_max_tile_size(size_t l1_capacity_bytes,
                                    size_t elem_size) {
  // TODO (Stage 6.1): Compute largest power-of-two tile dimension T
  (void)l1_capacity_bytes;
  (void)elem_size;
  return 0;
}

/*
 * Precondition:
 *   n is a multiple of tile_size, tile_size >= 1.
 * Postcondition:
 *   Computes C = A * B using 2D cache blocking with `ikj` order inside each tile:
 *   for ti = 0..n (step tile_size):
 *     for tj = 0..n (step tile_size):
 *       for tk = 0..n (step tile_size):
 *         for i = ti .. min(ti + tile_size, n):
 *           for k = tk .. min(tk + tile_size, n):
 *             r = A[i][k] (record MEM_LOAD A[i][k] here!)
 *             for j = tj .. min(tj + tile_size, n):
 *               record MEM_LOAD B[k][j], MEM_LOAD C[i][j], MEM_STORE C[i][j]
 *               C[i][j] += r * B[k][j]
 */
static void sim_matmul_tiled(sim_cache_t *cache, double *C, const double *A,
                             const double *B, size_t n, size_t tile_size) {
  // TODO (Stage 6.2): Implement tiled matrix multiplication with cache access tracking
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
  (void)tile_size;
}

/*
 * ============================================================================
 * Verification Suite
 * ============================================================================
 */
static void test_stage_1(void) {
  printf("[Stage 1] Testing Cache Geometry & Bitwise Address Decoding...\n");

  cache_geometry_t geom;
  /* Typical L1d cache: s=6 (64 sets), E=8 (8-way), b=6 (64B lines) -> 32 KiB */
  compute_cache_geometry(6, 8, 6, &geom);
  assert(geom.num_sets == 64);
  assert(geom.block_size == 64);
  assert(geom.tag_bits == 52);
  assert(geom.capacity_bytes == 32768);
  assert(geom.critical_stride_bytes == 4096);

  /* Cache Lab Part B geometry: s=5 (32 sets), E=1 (direct-mapped), b=5 (32B lines) -> 1 KiB */
  compute_cache_geometry(5, 1, 5, &geom);
  assert(geom.num_sets == 32);
  assert(geom.block_size == 32);
  assert(geom.tag_bits == 54);
  assert(geom.capacity_bytes == 1024);
  assert(geom.critical_stride_bytes == 1024);

  /* Address decoding on (s=4, b=4): addr = 0x123456789ABCDEF5 */
  decoded_addr_t dec = decode_cache_address(0x123456789ABCDEF5ULL, 4, 4);
  assert(dec.offset == 0x5ULL);
  assert(dec.set_index == 0xFULL);
  assert(dec.tag == 0x00123456789ABCDEULL);

  /* Verify Middle-Bit vs High-Bit Indexing across 16 contiguous 64B blocks */
  for (uint64_t block = 0; block < 16; ++block) {
    uint64_t addr = 0x400000ULL + block * 64ULL;
    decoded_addr_t mid = decode_cache_address(addr, 4, 6);
    uint64_t high = decode_flawed_high_bit_index(addr, 4);
    assert(mid.set_index == block); /* Middle bits cycle cleanly 0..15 */
    assert(high == 0ULL);           /* High bits all collide in Set 0! */
  }

  printf("  -> Stage 1 PASSED!\n\n");
}

static void test_stage_2(void) {
  printf("[Stage 2] Testing Analytical Miss Rate & 2D Loop Miss Predictor...\n");

  /* Stride-1 on 4B ints with 64B cache lines -> 1/16 = 0.0625 */
  assert(fabs(predict_stride_miss_rate(1, 4, 64) - 0.0625) < 1e-9);
  /* Stride-2 on 4B ints with 64B cache lines -> 2/16 = 0.125 */
  assert(fabs(predict_stride_miss_rate(2, 4, 64) - 0.125) < 1e-9);
  /* Stride-16 or Stride-4000 on 4B ints with 64B cache lines -> 1.0 (100%) */
  assert(fabs(predict_stride_miss_rate(16, 4, 64) - 1.0) < 1e-9);
  assert(fabs(predict_stride_miss_rate(4000, 4, 64) - 1.0) < 1e-9);

  /* loop-order.c option_one: x[4000][4000] row-major, 64 KiB L1d, 64B line */
  uint64_t misses_opt1 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_ROW_MAJOR);
  assert(misses_opt1 == 1000000ULL);

  /* loop-order.c option_two: x[4000][4000] col-major, 64 KiB L1d, 64B line */
  uint64_t misses_opt2 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt2 == 16000000ULL);

  /* loop-order.c option_five: x[4000][8] col-major, 64 KiB L1d, 64B line -> 16,000 misses */
  uint64_t misses_opt5 =
      predict_2d_loop_write_misses(4000, 8, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt5 == 16000ULL);

  /* Small matrix x[64][64] (16 KiB) inside 64 KiB L1d: col-major only has cold misses (256) */
  uint64_t misses_small_col =
      predict_2d_loop_write_misses(64, 64, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_small_col == 256ULL);

  printf("  -> Stage 2 PASSED!\n\n");
}

static void test_stage_3(void) {
  printf("[Stage 3] Testing LRU Cache & Write Policy State Machine...\n");

  /* 1. Direct-mapped Write-Back + Write-Allocate (s=4, E=1, b=4) on yi.trace sequence */
  sim_cache_t wb_cache;
  sim_cache_init(&wb_cache, 4, 1, 4, WRITE_BACK_ALLOCATE);

  assert(sim_cache_access(&wb_cache, 0x010, MEM_LOAD) == false);  /* Miss (Set 1, Tag 0) */
  assert(sim_cache_access(&wb_cache, 0x020, MEM_LOAD) == false);  /* Miss (Set 2, Tag 0) */
  assert(sim_cache_access(&wb_cache, 0x020, MEM_STORE) == true);  /* Hit  (Set 2 becomes dirty) */
  assert(sim_cache_access(&wb_cache, 0x022, MEM_LOAD) == true);   /* Hit  (Set 2, Tag 0) */
  assert(sim_cache_access(&wb_cache, 0x018, MEM_STORE) == true);  /* Hit  (Set 1 becomes dirty) */
  assert(sim_cache_access(&wb_cache, 0x110, MEM_LOAD) == false);  /* Miss + Evict dirty Set 1! */
  assert(sim_cache_access(&wb_cache, 0x210, MEM_LOAD) == false);  /* Miss + Evict clean Set 1! */
  assert(sim_cache_access(&wb_cache, 0x012, MEM_LOAD) == false);  /* Miss + Evict clean Set 1! */
  assert(sim_cache_access(&wb_cache, 0x012, MEM_STORE) == true);  /* Hit  (Set 1 becomes dirty) */

  assert(wb_cache.hits == 4);
  assert(wb_cache.misses == 5);
  assert(wb_cache.evictions == 3);
  assert(wb_cache.dirty_writebacks == 1);
  assert(wb_cache.main_memory_writes == 1);
  sim_cache_destroy(&wb_cache);

  /* 2. 2-Way Set Associative LRU Ordering Check (s=2, E=2, b=4) */
  sim_cache_t lru_cache;
  sim_cache_init(&lru_cache, 2, 2, 4, WRITE_BACK_ALLOCATE);
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == false); /* Set 0, Tag 0 (clk 1) */
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false); /* Set 0, Tag 1 (clk 2) */
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);  /* Refresh Tag 0 (clk 3) */
  assert(sim_cache_access(&lru_cache, 0x080, MEM_LOAD) == false); /* Evicts Tag 1 (clk 2)! */
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);  /* Tag 0 still resident! */
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false); /* Tag 1 was evicted! */
  sim_cache_destroy(&lru_cache);

  /* 3. Write-Through + No-Write-Allocate Check */
  sim_cache_t wt_cache;
  sim_cache_init(&wt_cache, 2, 1, 4, WRITE_THROUGH_NO_ALLOC);
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == false); /* Store miss: no alloc, writes RAM */
  assert(sim_cache_access(&wt_cache, 0x100, MEM_LOAD) == false);  /* Load still misses! Now allocates */
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == true);  /* Store hit: writes RAM immediately */
  assert(wt_cache.hits == 1);
  assert(wt_cache.misses == 2);
  assert(wt_cache.dirty_writebacks == 0);
  assert(wt_cache.main_memory_writes == 2);
  sim_cache_destroy(&wt_cache);

  printf("  -> Stage 3 PASSED!\n\n");
}

static void test_stage_4(void) {
  printf("[Stage 4] Testing Conflict Thrashing & Array Padding...\n");

  cache_geometry_t geom;
  /* s=5, E=1, b=5 -> S=32 sets, B=32B, Critical Stride = 1024B */
  compute_cache_geometry(5, 1, 5, &geom);

  assert(is_set_aliasing(0x10000ULL, 0x10400ULL, &geom) == true);  /* +1024B collides! */
  assert(is_set_aliasing(0x10000ULL, 0x10800ULL, &geom) == true);  /* +2048B collides! */
  assert(is_set_aliasing(0x10000ULL, 0x10020ULL, &geom) == false); /* +32B is in next set */

  /* 256 ints * 4B = 1024B (exact multiple of critical stride) -> pad by 32B / 4B = 8 ints */
  assert(compute_row_padding_elements(256, 4, &geom) == 8);
  /* 64 ints * 4B = 256B (not a multiple of 1024B) -> 0 padding needed for adjacent row */
  assert(compute_row_padding_elements(64, 4, &geom) == 0);

  /* Demonstrate 100% conflict thrashing on E=1 vs 0 evictions on E=2 */
  sim_cache_t dm_cache, sa_cache;
  sim_cache_init(&dm_cache, 5, 1, 5, WRITE_BACK_ALLOCATE);
  sim_cache_init(&sa_cache, 5, 2, 5, WRITE_BACK_ALLOCATE);

  for (int i = 0; i < 50; ++i) {
    sim_cache_access(&dm_cache, 0x10000ULL, MEM_LOAD);
    sim_cache_access(&dm_cache, 0x10400ULL, MEM_LOAD);
    sim_cache_access(&sa_cache, 0x10000ULL, MEM_LOAD);
    sim_cache_access(&sa_cache, 0x10400ULL, MEM_LOAD);
  }
  assert(dm_cache.misses == 100 && dm_cache.hits == 0); /* 100% conflict thrashing! */
  assert(sa_cache.misses == 2 && sa_cache.hits == 98);  /* E=2 eliminates conflict! */

  sim_cache_destroy(&dm_cache);
  sim_cache_destroy(&sa_cache);

  printf("  -> Stage 4 PASSED!\n\n");
}

static void test_stage_5(void) {
  printf("[Stage 5] Testing Matrix Multiplication Loop Permutations (ijk vs jki vs ikj)...\n");

  const size_t n = 16;
  double *A = malloc(n * n * sizeof(double));
  double *B = malloc(n * n * sizeof(double));
  double *C_ijk = calloc(n * n, sizeof(double));
  double *C_jki = calloc(n * n, sizeof(double));
  double *C_ikj = calloc(n * n, sizeof(double));

  for (size_t i = 0; i < n * n; ++i) {
    A[i] = (double)(i % 13) * 0.5;
    B[i] = (double)((i * 7) % 11) * 0.25;
  }

  /* Simulated L1 cache: s=3 (8 sets), E=2 (2-way), b=5 (32B line = 4 doubles) -> 512B */
  sim_cache_t cache;
  sim_cache_init(&cache, 3, 2, 5, WRITE_BACK_ALLOCATE);

  sim_cache_reset_stats(&cache);
  sim_matmul_ijk(&cache, C_ijk, A, B, n);
  uint64_t misses_ijk = cache.misses;

  sim_cache_reset_stats(&cache);
  sim_matmul_jki(&cache, C_jki, A, B, n);
  uint64_t misses_jki = cache.misses;

  sim_cache_reset_stats(&cache);
  sim_matmul_ikj(&cache, C_ikj, A, B, n);
  uint64_t misses_ikj = cache.misses;

  for (size_t i = 0; i < n * n; ++i) {
    assert(fabs(C_ijk[i] - C_jki[i]) < 1e-9);
    assert(fabs(C_ijk[i] - C_ikj[i]) < 1e-9);
  }

  printf("    [N=16 Misses] jki: %llu | ijk: %llu | ikj: %llu\n",
         (unsigned long long)misses_jki, (unsigned long long)misses_ijk,
         (unsigned long long)misses_ikj);
  assert(misses_ikj > 0);
  assert(misses_ikj * 4 < misses_ijk);
  assert(misses_ijk < misses_jki);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ijk);
  free(C_jki);
  free(C_ikj);

  printf("  -> Stage 5 PASSED!\n\n");
}

static void test_stage_6(void) {
  printf("[Stage 6] Testing 2D Cache Blocking (Tiling) & Working Set Fitting...\n");

  /* For 32 KiB L1d and 8B doubles: 3 * 32^2 * 8 = 24,576 <= 32,768 -> T = 32 */
  assert(compute_max_tile_size(32768, 8) == 32);
  /* For 1 KiB cache and 8B doubles: 3 * 4^2 * 8 = 384 <= 1024 (8^2 * 24 = 1536 > 1024) -> T = 4 */
  assert(compute_max_tile_size(1024, 8) == 4);

  const size_t n = 16;
  double *A = malloc(n * n * sizeof(double));
  double *B = malloc(n * n * sizeof(double));
  double *C_ikj = calloc(n * n, sizeof(double));
  double *C_tiled = calloc(n * n, sizeof(double));

  for (size_t i = 0; i < n * n; ++i) {
    A[i] = (double)((i + 3) % 17) * 0.5;
    B[i] = (double)((i * 5 + 1) % 19) * 0.25;
  }

  /*
   * Small cache: s=3 (8 sets), E=2 (2-way), b=5 (32B line) -> 512B capacity.
   * Full 16x16 matrix = 2048B (4x larger than cache), so untiled ikj evicts B on every row i.
   * Tile size T = 4: 3 * 4 * 4 * 8B = 384B <= 512B, so tiles stay resident in cache!
   */
  sim_cache_t cache;
  sim_cache_init(&cache, 3, 2, 5, WRITE_BACK_ALLOCATE);

  sim_cache_reset_stats(&cache);
  sim_matmul_ikj(&cache, C_ikj, A, B, n);
  uint64_t misses_untiled = cache.misses;

  size_t T = compute_max_tile_size(cache.geom.capacity_bytes, sizeof(double));
  assert(T == 4);

  sim_cache_reset_stats(&cache);
  sim_matmul_tiled(&cache, C_tiled, A, B, n, T);
  uint64_t misses_tiled = cache.misses;

  for (size_t i = 0; i < n * n; ++i) {
    assert(fabs(C_ikj[i] - C_tiled[i]) < 1e-9);
  }

  printf("    [N=16 on 512B Cache] Untiled ikj misses: %llu | Tiled (T=4) misses: %llu\n",
         (unsigned long long)misses_untiled, (unsigned long long)misses_tiled);
  assert(misses_tiled > 0 && misses_tiled < misses_untiled);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ikj);
  free(C_tiled);

  printf("  -> Stage 6 PASSED!\n\n");
}

int main(int argc, char *argv[]) {
  int stage = 0;
  if (argc > 1) {
    stage = atoi(argv[1]);
  }

  printf("================================================================\n");
  printf("  Memory Hierarchy & Caching Deliberate Practice Suite\n");
  printf("================================================================\n\n");

  if (stage == 0 || stage == 1) test_stage_1();
  if (stage == 0 || stage == 2) test_stage_2();
  if (stage == 0 || stage == 3) test_stage_3();
  if (stage == 0 || stage == 4) test_stage_4();
  if (stage == 0 || stage == 5) test_stage_5();
  if (stage == 0 || stage == 6) test_stage_6();

  printf("All requested stages completed successfully!\n");
  return EXIT_SUCCESS;
}
