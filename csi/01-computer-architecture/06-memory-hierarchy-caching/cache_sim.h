#ifndef CACHE_SIM_H
#define CACHE_SIM_H

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
 * Cache Geometry Parameters
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

/*
 * Decoded Address Components
 */
typedef struct {
  uint64_t tag;
  uint64_t set_index;
  uint64_t offset;
} decoded_addr_t;

/*
 * Loop Traversals
 */
typedef enum {
  LOOP_ROW_MAJOR = 0, /* Inner loop increments column j: x[i][j] */
  LOOP_COL_MAJOR = 1  /* Inner loop increments row j:    x[j][i] */
} loop_order_t;

/*
 * Cache Write Policies
 */
typedef enum {
  WRITE_BACK_ALLOCATE = 0,     /* Allocate on miss, write to RAM only on dirty eviction */
  WRITE_THROUGH_NO_ALLOC = 1   /* Never allocate on write miss, every store writes RAM */
} write_policy_t;

/*
 * Memory Operation
 */
typedef enum {
  MEM_LOAD = 0,
  MEM_STORE = 1
} mem_op_t;

/*
 * Cache Line State
 */
typedef struct {
  bool valid;
  bool dirty;
  uint64_t tag;
  uint64_t lru_clock;
} sim_line_t;

/*
 * Cache State Machine
 */
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
 * Simulated Matrix Addresses
 */
static constexpr uint64_t SIM_BASE_A = 0x100000ULL;
static constexpr uint64_t SIM_BASE_B = 0x200000ULL;
static constexpr uint64_t SIM_BASE_C = 0x300000ULL;

static inline uint64_t mat_addr(uint64_t base, size_t n, size_t r, size_t c) {
  return base + (uint64_t)((r * n + c) * sizeof(double));
}

/*
 * Simulator Lifecycle Functions
 */
static inline void sim_cache_init(sim_cache_t *cache, uint32_t s, uint32_t E,
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

static inline void sim_cache_reset_stats(sim_cache_t *cache) {
  memset(cache->lines, 0,
         cache->geom.num_sets * cache->geom.E * sizeof(sim_line_t));
  cache->clock = 0;
  cache->hits = 0;
  cache->misses = 0;
  cache->evictions = 0;
  cache->dirty_writebacks = 0;
  cache->main_memory_writes = 0;
}

static inline void sim_cache_destroy(sim_cache_t *cache) {
  free(cache->lines);
  cache->lines = nullptr;
}

/*
 * Simulator Access Function (Defined in 05_lru_cache_sim.c)
 */
bool sim_cache_access(sim_cache_t *cache, uint64_t addr, mem_op_t op);

#endif /* CACHE_SIM_H */

