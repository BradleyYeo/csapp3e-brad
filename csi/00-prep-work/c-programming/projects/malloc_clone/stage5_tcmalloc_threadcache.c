/*
 * Stage 5: TCMalloc Front-End — ThreadCache & Batch TransferCache
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -pthread -o stage5_threadcache stage5_tcmalloc_threadcache.c && ./stage5_threadcache
 *
 * Learning Objectives:
 * - Eliminate lock contention on the fast path by implementing a lock-free C23 `thread_local`
 *   front-end `ThreadCache` per thread.
 * - Amortize synchronization overhead across batches (`BATCH_SIZE = 32`) via a mutex-guarded
 *   `TransferCache` ring buffer sitting in front of `CentralFreeList`.
 * - Support cross-thread producer-consumer allocation/deallocation where Thread A allocates
 *   objects and Thread B calls `tcm_free(ptr)` without deadlocks or unbounded cache bloat.
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
#ifndef nullptr
#define nullptr NULL
#endif
#ifndef thread_local
#define thread_local _Thread_local
#endif
#endif

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

/* ============================================================================
 * TCMalloc 3-Tier Hierarchy Constants & Structures
 * ============================================================================
 *
 * Fast Path (`tcm_malloc` / `tcm_free` for small objects <= 4096B):
 *   Thread -> thread_local ThreadCache.bins[c] (ZERO LOCKS!)
 *     |
 *     +-- On Miss (length == 0) or Overflow (length == max_length):
 *           Move a batch of 32 pointers <-> TransferCache[c] (per-class mutex)
 *             |
 *             +-- On TransferCache empty/full:
 *                   Refill/Flush batch <-> CentralFreeList[c] <-> PageHeap (global pageheap mutex)
 */
enum {
  TCM_PAGE_SHIFT = 13,
  TCM_PAGE_SIZE  = (1 << TCM_PAGE_SHIFT), /* 8192 bytes */
  MAX_SMALL_SIZE = 4096,
  NUM_SIZE_CLASSES = 14,
  BATCH_SIZE = 32,                        /* Number of objects transferred per middle-end lock acquisition */
  BIN_MAX_LENGTH = 64,                    /* High-water mark in a ThreadCache bin before flushing a batch */
  TRANSFER_CACHE_CAPACITY = 512           /* Max cached pointers in TransferCache[c] ring/stack */
};

static const size_t kSizeClassBytes[NUM_SIZE_CLASSES] = {
  0, 16, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024, 2048, 4096
};

/* Tier 1: Front-End Thread-Local Cache */
typedef struct {
  void *head;           /* Intrusive singly-linked list of free objects (`*(void **)obj`) */
  uint32_t length;      /* Current count of cached objects in this bin */
  uint32_t max_length;  /* Maximum objects before flushing BATCH_SIZE to TransferCache */
} FreeListBin;

typedef struct {
  FreeListBin bins[NUM_SIZE_CLASSES];
  bool initialized;
  uint64_t fast_alloc_hits;
  uint64_t batch_refills;
  uint64_t batch_flushes;
} ThreadCache;

/* C23 Thread-Local Front-End Instance (Zero locks required to access!) */
static thread_local ThreadCache tls_cache = {0};

/* Tier 2: Middle-End Per-Size-Class TransferCache */
typedef struct {
  pthread_mutex_t lock;
  void *slots[TRANSFER_CACHE_CAPACITY];
  size_t count;         /* Number of object pointers currently buffered in slots[] */
} TransferCache;

static TransferCache g_transfer_cache[NUM_SIZE_CLASSES];

/*
 * Back-End (Stage 3) & Middle-End (Stage 4) Prototypes
 * Bring over your Stage 3 PageHeap/PageMap and Stage 4 CentralFreeList functions
 * to back the TransferCache and ThreadCache!
 */
typedef uintptr_t PageId;

typedef enum {
  SPAN_ON_PAGEHEAP = 0,
  SPAN_IN_USE_SMALL = 1,
  SPAN_IN_USE_LARGE = 2
} SpanState;

typedef struct Span {
  PageId start_page;
  size_t num_pages;
  uint32_t size_class;
  uint32_t allocated_count;
  uint32_t max_objects;
  SpanState state;
  void *freelist;
  struct Span *prev;
  struct Span *next;
} Span;

[[nodiscard]] uint32_t size_to_class(size_t size);
[[nodiscard]] size_t class_to_size(uint32_t size_class);
[[nodiscard]] size_t central_remove_range(uint32_t size_class, void **batch_out, size_t count);
void central_insert_range(uint32_t size_class, void **batch_in, size_t count);

/* ============================================================================
 * Core TCMalloc Front-End & Middle-End Drills (Implement the TODOs below)
 * ============================================================================ */

/*
 * Drill 1: Initialize and Teardown the Full 3-Tier TCMalloc Subsystem
 *
 * Requirements:
 * - Initialize `PageHeap` (from Stage 3) and `CentralFreeList` (from Stage 4), protected by
 *   a back-end mutex `g_backend_lock` when accessed from `CentralFreeList` or large allocations.
 * - Initialize `pthread_mutex_init(&g_transfer_cache[c].lock, nullptr)` for all `c` in `1..NUM_SIZE_CLASSES-1`.
 */
[[nodiscard]] bool tcm_runtime_init(size_t num_initial_pages) {
  (void)num_initial_pages;
  (void)kSizeClassBytes;
  (void)g_transfer_cache;
  /* TODO: Initialize PageHeap, CentralFreeList, and TransferCache mutexes */
  return false;
}

void tcm_runtime_teardown(void) {
  /* TODO: Destroy mutexes and unmap PageHeap slabs */
}

/*
 * Drill 2: TransferCache Batch Refill & Flush (`transfer_cache_remove` / `transfer_cache_insert`)
 *
 * Requirements:
 * - `transfer_cache_remove(c, batch_out, count)`:
 *   1. Lock `g_transfer_cache[c].lock`.
 *   2. If `g_transfer_cache[c].count >= count`, pop `count` pointers directly from `slots[]`,
 *      unlock, and return `count` (fast batch handoff between threads without touching Spans!).
 *   3. Otherwise, unlock `g_transfer_cache[c].lock`, acquire the back-end lock, and fetch
 *      `count` objects from `central_remove_range(c, batch_out, count)`.
 * - `transfer_cache_insert(c, batch_in, count)`:
 *   1. Lock `g_transfer_cache[c].lock`.
 *   2. If `g_transfer_cache[c].count + count <= TRANSFER_CACHE_CAPACITY`, copy `batch_in` into
 *      `slots[]`, unlock, and return.
 *   3. Otherwise, unlock `g_transfer_cache[c].lock`, acquire the back-end lock, and return
 *      the batch to `central_insert_range(c, batch_in, count)`.
 */
[[nodiscard]] size_t transfer_cache_remove(uint32_t size_class, void **batch_out, size_t count) {
  (void)size_class;
  (void)batch_out;
  (void)count;
  /* TODO: Pop a batch from TransferCache[size_class], falling back to CentralFreeList */
  return 0;
}

void transfer_cache_insert(uint32_t size_class, void **batch_in, size_t count) {
  (void)size_class;
  (void)batch_in;
  (void)count;
  /* TODO: Push a batch into TransferCache[size_class], overflowing to CentralFreeList */
}

/*
 * Drill 3: Lock-Free Front-End Fast Path (`tcm_malloc` & `tcm_free`)
 *
 * Requirements:
 * - Lazy ThreadCache initialization: if `!tls_cache.initialized`, set `bins[c].max_length = BIN_MAX_LENGTH`
 *   for all `c` and `tls_cache.initialized = true`.
 * - In `tcm_malloc(size)`:
 *   1. If `size == 0`, return `nullptr`.
 *   2. Compute `c = size_to_class(size)`.
 *   3. If `c == 0` (large object > 4096B): lock back-end, allocate multi-page Span from `PageHeap`, unlock, return.
 *   4. Check `FreeListBin *bin = &tls_cache.bins[c]`:
 *      - Fast Hit (`bin->head != nullptr`):
 *        Pop `obj = bin->head`, advance `bin->head = *(void **)obj`, decrement `bin->length`,
 *        increment `tls_cache.fast_alloc_hits`, and return `obj`! (ZERO locks!)
 *      - Slow Miss (`bin->head == nullptr`):
 *        Call `n = transfer_cache_remove(c, batch, BATCH_SIZE)`.
 *        Increment `tls_cache.batch_refills`.
 *        Return `batch[0]` to the caller, and push `batch[1 .. n - 1]` into `bin->head`!
 * - In `tcm_free(ptr)`:
 *   1. If `ptr == nullptr`, return.
 *   2. Look up `Span *s = pagemap_get((uintptr_t)ptr >> TCM_PAGE_SHIFT)`.
 *      (Note: `PageMap` reads for already-allocated pages are lock-free!)
 *   3. If `s->state == SPAN_IN_USE_LARGE`: lock back-end, call `pageheap_free_span(s)`, unlock, return.
 *   4. Otherwise, read `c = s->size_class` and push `ptr` onto `tls_cache.bins[c].head` (`bin->length++`).
 *   5. If `bin->length >= bin->max_length`:
 *      Pop `BATCH_SIZE` objects from `bin->head` into a temporary array `batch[BATCH_SIZE]`,
 *      increment `tls_cache.batch_flushes`, and call `transfer_cache_insert(c, batch, BATCH_SIZE)`.
 */
[[nodiscard]] void *tcm_malloc(size_t size) {
  (void)size;
  (void)tls_cache;
  /* TODO: Implement lock-free ThreadCache fast-path allocation with batch refill */
  return nullptr;
}

void tcm_free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }
  /* TODO: Implement lock-free ThreadCache fast-path deallocation with batch flush */
}

[[nodiscard]] void *tcm_calloc(size_t count, size_t size) {
  if (count == 0 || size == 0 || count > SIZE_MAX / size) {
    return nullptr;
  }
  size_t total = count * size;
  void *p = tcm_malloc(total);
  if (p != nullptr) {
    memset(p, 0, total);
  }
  return p;
}

[[nodiscard]] void *tcm_realloc(void *ptr, size_t new_size) {
  (void)ptr;
  (void)new_size;
  /* TODO: Look up current object size via PageMap->Span, return ptr if same size class, else malloc+memcpy+free */
  return nullptr;
}

/* ============================================================================
 * Multi-Threaded Verification Harness
 * ============================================================================ */

typedef struct {
  int thread_id;
  size_t iterations;
} WorkerArgs;

static void *concurrent_worker(void *arg) {
  WorkerArgs *w = (WorkerArgs *)arg;
  void *local_ring[64] = {0};

  for (size_t i = 0; i < w->iterations; i++) {
    size_t slot = i % 64;
    if (local_ring[slot] != nullptr) {
      tcm_free(local_ring[slot]);
      local_ring[slot] = nullptr;
    }

    size_t req_size = ((i * 37 + (size_t)w->thread_id * 13) % 512) + 1;
    uint8_t *p = tcm_malloc(req_size);
    assert(p != nullptr);
    assert(((uintptr_t)p % 16) == 0);
    p[0] = (uint8_t)w->thread_id;
    p[req_size - 1] = (uint8_t)(w->thread_id ^ 0xFF);
    local_ring[slot] = p;
  }

  for (size_t slot = 0; slot < 64; slot++) {
    if (local_ring[slot] != nullptr) {
      tcm_free(local_ring[slot]);
    }
  }

  /* Verify that >90% of allocations hit the lock-free ThreadCache fast path! */
  assert(tls_cache.fast_alloc_hits > (w->iterations * 9) / 10);
  return nullptr;
}

int main(void) {
  printf("=== Stage 5: TCMalloc Front-End (ThreadCache & TransferCache) ===\n");

  /* Initialize with 1024 pages (8 MiB) */
  assert(tcm_runtime_init(1024) == true);

  /* Test 1: Single-Thread Batch Refill, Lock-Free Fast-Path Hits, and Batch Flush */
  void *batch_ptrs[BIN_MAX_LENGTH] = {0};
  for (int i = 0; i < BIN_MAX_LENGTH; i++) {
    batch_ptrs[i] = tcm_malloc(32);
    assert(batch_ptrs[i] != nullptr);
  }
  /* 64 allocations of 32B triggered 2 batch refills of 32 objects; remaining 62 were fast-path hits! */
  assert(tls_cache.batch_refills == 2);
  assert(tls_cache.fast_alloc_hits == BIN_MAX_LENGTH - 2);

  for (int i = 0; i < BIN_MAX_LENGTH; i++) {
    tcm_free(batch_ptrs[i]);
  }
  /* Freeing all 64 objects hits BIN_MAX_LENGTH (64) and triggers 1 batch flush of 32 objects! */
  assert(tls_cache.batch_flushes == 1);

  /* Verify tcm_calloc zero-initialization and tcm_realloc migration */
  uint64_t *zeroed = tcm_calloc(8, sizeof(uint64_t));
  assert(zeroed != nullptr);
  for (int i = 0; i < 8; i++) {
    assert(zeroed[i] == 0);
    zeroed[i] = (uint64_t)(i + 100);
  }
  uint64_t *grown = tcm_realloc(zeroed, 16 * sizeof(uint64_t));
  assert(grown != nullptr);
  for (int i = 0; i < 8; i++) {
    assert(grown[i] == (uint64_t)(i + 100));
  }
  tcm_free(grown);

  /* Test 2: Multi-Threaded Contention & Lock-Free Fast-Path Verification */
  enum { NUM_THREADS = 8, ITERS_PER_THREAD = 20000 };
  pthread_t threads[NUM_THREADS];
  WorkerArgs args[NUM_THREADS];

  for (int t = 0; t < NUM_THREADS; t++) {
    args[t].thread_id = t + 1;
    args[t].iterations = ITERS_PER_THREAD;
    int rc = pthread_create(&threads[t], nullptr, concurrent_worker, &args[t]);
    assert(rc == 0);
  }

  for (int t = 0; t < NUM_THREADS; t++) {
    pthread_join(threads[t], nullptr);
  }

  tcm_runtime_teardown();
  printf("[PASS] Stage 5: Multi-Threaded TCMalloc ThreadCache & TransferCache verified!\n");
  return EXIT_SUCCESS;
}
