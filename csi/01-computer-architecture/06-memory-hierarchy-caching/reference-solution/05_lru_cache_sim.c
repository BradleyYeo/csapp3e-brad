/*
 * Reference Solution: Exercise 05 (LRU Cache State Machine & Write Policies)
 */
#include "../cache_sim.h"

bool sim_cache_access(sim_cache_t *cache, uint64_t addr, mem_op_t op) {
  cache->clock++;
  uint64_t set_index = (addr >> cache->geom.b) & (cache->geom.num_sets - 1);
  uint64_t tag = addr >> (cache->geom.s + cache->geom.b);
  sim_line_t *set = &cache->lines[set_index * cache->geom.E];

  for (uint32_t i = 0; i < cache->geom.E; ++i) {
    if (set[i].valid && set[i].tag == tag) {
      cache->hits++;
      set[i].lru_clock = cache->clock;
      if (op == MEM_STORE) {
        if (cache->policy == WRITE_BACK_ALLOCATE) {
          set[i].dirty = true;
        } else {
          cache->main_memory_writes++;
        }
      }
      return true;
    }
  }

  cache->misses++;
  if (op == MEM_STORE && cache->policy == WRITE_THROUGH_NO_ALLOC) {
    cache->main_memory_writes++;
    return false;
  }

  sim_line_t *victim = nullptr;
  for (uint32_t i = 0; i < cache->geom.E; ++i) {
    if (!set[i].valid) {
      victim = &set[i];
      break;
    }
  }
  if (victim == nullptr) {
    cache->evictions++;
    victim = &set[0];
    for (uint32_t i = 1; i < cache->geom.E; ++i) {
      if (set[i].lru_clock < victim->lru_clock) {
        victim = &set[i];
      }
    }
    if (victim->dirty) {
      cache->dirty_writebacks++;
      cache->main_memory_writes++;
    }
  }

  victim->valid = true;
  victim->tag = tag;
  victim->lru_clock = cache->clock;
  victim->dirty = (op == MEM_STORE && cache->policy == WRITE_BACK_ALLOCATE);
  return false;
}

#ifndef NO_MAIN
int main(void) {
  printf("Running Reference Solution: Exercise 05...\n\n");

  sim_cache_t wb_cache;
  sim_cache_init(&wb_cache, 4, 1, 4, WRITE_BACK_ALLOCATE);

  assert(sim_cache_access(&wb_cache, 0x010, MEM_LOAD) == false);
  assert(sim_cache_access(&wb_cache, 0x020, MEM_LOAD) == false);
  assert(sim_cache_access(&wb_cache, 0x020, MEM_STORE) == true);
  assert(sim_cache_access(&wb_cache, 0x022, MEM_LOAD) == true);
  assert(sim_cache_access(&wb_cache, 0x018, MEM_STORE) == true);
  assert(sim_cache_access(&wb_cache, 0x110, MEM_LOAD) == false);
  assert(sim_cache_access(&wb_cache, 0x210, MEM_LOAD) == false);
  assert(sim_cache_access(&wb_cache, 0x012, MEM_LOAD) == false);
  assert(sim_cache_access(&wb_cache, 0x012, MEM_STORE) == true);

  assert(wb_cache.hits == 4);
  assert(wb_cache.misses == 5);
  assert(wb_cache.evictions == 3);
  assert(wb_cache.dirty_writebacks == 1);
  assert(wb_cache.main_memory_writes == 1);
  sim_cache_destroy(&wb_cache);

  sim_cache_t lru_cache;
  sim_cache_init(&lru_cache, 2, 2, 4, WRITE_BACK_ALLOCATE);
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == false);
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false);
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);
  assert(sim_cache_access(&lru_cache, 0x080, MEM_LOAD) == false);
  assert(sim_cache_access(&lru_cache, 0x000, MEM_LOAD) == true);
  assert(sim_cache_access(&lru_cache, 0x040, MEM_LOAD) == false);
  sim_cache_destroy(&lru_cache);

  sim_cache_t wt_cache;
  sim_cache_init(&wt_cache, 2, 1, 4, WRITE_THROUGH_NO_ALLOC);
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == false);
  assert(sim_cache_access(&wt_cache, 0x100, MEM_LOAD) == false);
  assert(sim_cache_access(&wt_cache, 0x100, MEM_STORE) == true);
  assert(wt_cache.hits == 1);
  assert(wt_cache.misses == 2);
  assert(wt_cache.dirty_writebacks == 0);
  assert(wt_cache.main_memory_writes == 2);
  sim_cache_destroy(&wt_cache);

  printf("Reference Solution 05 PASSED!\n");
  return EXIT_SUCCESS;
}
#endif
