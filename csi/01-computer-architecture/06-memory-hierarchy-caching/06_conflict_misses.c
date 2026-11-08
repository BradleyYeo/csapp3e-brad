/*
 * Exercise 06: Conflict Misses, Power-of-Two Aliasing, and Padding
 *
 * Guide: 06_conflict_misses.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 06
 */
#include "cache_sim.h"

/*
 * Drill 1: Set Aliasing Predicate
 *
 * Precondition:
 *   geom points to a valid cache_geometry_t.
 * Postcondition:
 *   Returns true if addr_a and addr_b map to the exact same cache set index:
 *     ((addr_a >> b) & (num_sets - 1)) == ((addr_b >> b) & (num_sets - 1))
 */
bool is_set_aliasing(uint64_t addr_a, uint64_t addr_b,
                     const cache_geometry_t *geom) {
  // TODO: Implement set aliasing check
  (void)addr_a;
  (void)addr_b;
  (void)geom;
  return false;
}

/*
 * Drill 2: Row Padding Element Calculator
 *
 * Precondition:
 *   row_cols >= 1, elem_size >= 1, geom != nullptr.
 * Postcondition:
 *   If row_bytes = row_cols * elem_size is an exact multiple of geom->critical_stride_bytes,
 *   returns geom->block_size / elem_size to shift each row into the next cache set.
 *   Otherwise returns 0 (no padding required).
 */
size_t compute_row_padding_elements(size_t row_cols, size_t elem_size,
                                    const cache_geometry_t *geom) {
  // TODO: Calculate required padding elements
  (void)row_cols;
  (void)elem_size;
  (void)geom;
  return 0;
}

int main(void) {
  printf("Running Exercise 06: Conflict Misses & Array Padding...\n\n");

  cache_geometry_t geom;
  /* s=5, E=1, b=5 -> S=32 sets, B=32B, Critical Stride = 1024B */
  geom.s = 5;
  geom.E = 1;
  geom.b = 5;
  geom.num_sets = 32;
  geom.block_size = 32;
  geom.critical_stride_bytes = 1024;

  /* Test 1: Set aliasing predicate */
  assert(is_set_aliasing(0x10000ULL, 0x10400ULL, &geom) == true);  /* +1024B collides! */
  assert(is_set_aliasing(0x10000ULL, 0x10800ULL, &geom) == true);  /* +2048B collides! */
  assert(is_set_aliasing(0x10000ULL, 0x10020ULL, &geom) == false); /* +32B is in next set */

  /* Test 2: Row padding calculation */
  /* 256 ints * 4B = 1024B (critical stride multiple) -> pad by 32B / 4B = 8 ints */
  assert(compute_row_padding_elements(256, 4, &geom) == 8);
  /* 64 ints * 4B = 256B (not a multiple of 1024B) -> 0 padding */
  assert(compute_row_padding_elements(64, 4, &geom) == 0);

  /* Test 3: Simulation demonstrating 100% conflict thrashing on E=1 vs 0 evictions on E=2 */
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

  printf("Exercise 06 PASSED!\n");
  return EXIT_SUCCESS;
}
