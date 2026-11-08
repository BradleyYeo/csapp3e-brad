/*
 * Reference Solution: Exercise 06 (Conflict Misses, Power-of-Two Aliasing, and Padding)
 */
#include "../cache_sim.h"

bool is_set_aliasing(uint64_t addr_a, uint64_t addr_b,
                     const cache_geometry_t *geom) {
  uint64_t mask = geom->num_sets - 1;
  return ((addr_a >> geom->b) & mask) == ((addr_b >> geom->b) & mask);
}

size_t compute_row_padding_elements(size_t row_cols, size_t elem_size,
                                    const cache_geometry_t *geom) {
  uint64_t row_bytes = (uint64_t)row_cols * elem_size;
  if (row_bytes % geom->critical_stride_bytes != 0) return 0;
  return (size_t)(geom->block_size / elem_size);
}

int main(void) {
  printf("Running Reference Solution: Exercise 06...\n\n");

  cache_geometry_t geom;
  geom.s = 5;
  geom.E = 1;
  geom.b = 5;
  geom.num_sets = 32;
  geom.block_size = 32;
  geom.critical_stride_bytes = 1024;

  assert(is_set_aliasing(0x10000ULL, 0x10400ULL, &geom) == true);
  assert(is_set_aliasing(0x10000ULL, 0x10800ULL, &geom) == true);
  assert(is_set_aliasing(0x10000ULL, 0x10020ULL, &geom) == false);

  assert(compute_row_padding_elements(256, 4, &geom) == 8);
  assert(compute_row_padding_elements(64, 4, &geom) == 0);

  sim_cache_t dm_cache, sa_cache;
  sim_cache_init(&dm_cache, 5, 1, 5, WRITE_BACK_ALLOCATE);
  sim_cache_init(&sa_cache, 5, 2, 5, WRITE_BACK_ALLOCATE);

  for (int i = 0; i < 50; ++i) {
    sim_cache_access(&dm_cache, 0x10000ULL, MEM_LOAD);
    sim_cache_access(&dm_cache, 0x10400ULL, MEM_LOAD);
    sim_cache_access(&sa_cache, 0x10000ULL, MEM_LOAD);
    sim_cache_access(&sa_cache, 0x10400ULL, MEM_LOAD);
  }
  assert(dm_cache.misses == 100 && dm_cache.hits == 0);
  assert(sa_cache.misses == 2 && sa_cache.hits == 98);

  sim_cache_destroy(&dm_cache);
  sim_cache_destroy(&sa_cache);

  printf("Reference Solution 06 PASSED!\n");
  return EXIT_SUCCESS;
}

