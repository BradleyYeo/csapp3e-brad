/*
 * Exercise 05: LRU Cache State Machine & Write Policies
 *
 * Guide: 05_lru_cache_sim.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 05
 */
#include "cache_sim.h"

/*
 * Cache Access State Machine
 *
 * Precondition:
 *   cache is an initialized sim_cache_t, op is MEM_LOAD or MEM_STORE.
 * Postcondition:
 *   Advances cache->clock++, decodes addr into set_index and tag.
 *   - On Hit:
 *     Increments cache->hits++, sets line.lru_clock = cache->clock.
 *     If op == MEM_STORE:
 *       If policy == WRITE_BACK_ALLOCATE: line.dirty = true.
 *       If policy == WRITE_THROUGH_NO_ALLOC: cache->main_memory_writes++.
 *     Returns true.
 *   - On Miss:
 *     Increments cache->misses++.
 *     If op == MEM_STORE && policy == WRITE_THROUGH_NO_ALLOC:
 *       Increments cache->main_memory_writes++ and returns false without allocating line.
 *     Otherwise:
 *       Finds empty line or evicts line with minimum lru_clock (incrementing cache->evictions++).
 *       If evicted victim was dirty: increments cache->dirty_writebacks++ and cache->main_memory_writes++.
 *       Sets target line: valid=true, tag=tag, lru_clock=clock, dirty=(STORE && WB).
 *     Returns false.
 */
bool sim_cache_access(sim_cache_t *cache, uint64_t addr, mem_op_t op) {
  // TODO: Implement LRU cache access state machine
  (void)cache;
  (void)addr;
  (void)op;
  return false;
}

#ifndef NO_MAIN
int main(void) {
  printf("Running Exercise 05: LRU Cache State Machine...\n\n");

  /* Test 1: Direct-mapped Write-Back + Write-Allocate on yi.trace */
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

  /* Test 2: 2-Way Set-Associative LRU Ordering Check (s=2, E=2, b=4) */
  sim_cache_t lru_cache;
  sim_cache_init(&lru_cache, 2, 2, 4, WRITE_BACK_ALLOCATE);
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == false); /* Set 0, Tag 0 (clk 1) */
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false); /* Set 0, Tag 1 (clk 2) */
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);  /* Refresh Tag 0 (clk 3) */
  assert(sim_cache_access(&lru_cache, 0x080, MEM_LOAD) == false); /* Evicts Tag 1 (clk 2)! */
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);  /* Tag 0 still resident! */
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false); /* Tag 1 was evicted! */
  sim_cache_destroy(&lru_cache);

  /* Test 3: Write-Through + No-Write-Allocate Check */
  sim_cache_t wt_cache;
  sim_cache_init(&wt_cache, 2, 1, 4, WRITE_THROUGH_NO_ALLOC);
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == false); /* Store miss: no alloc, writes RAM */
  assert(sim_cache_access(&wt_cache, 0x100, MEM_LOAD) == false);  /* Load miss: allocates clean line */
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == true);  /* Store hit: writes RAM immediately */
  assert(wt_cache.hits == 1);
  assert(wt_cache.misses == 2);
  assert(wt_cache.dirty_writebacks == 0);
  assert(wt_cache.main_memory_writes == 2);
  sim_cache_destroy(&wt_cache);

  printf("Exercise 05 PASSED!\n");
  return EXIT_SUCCESS;
}
#endif

