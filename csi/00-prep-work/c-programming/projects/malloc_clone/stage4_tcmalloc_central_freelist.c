/*
 * Stage 4: TCMalloc Middle-End — Segregated Size Classes & CentralFreeList
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o stage4_central stage4_tcmalloc_central_freelist.c && ./stage4_central
 *
 * Learning Objectives:
 * - Implement TCMalloc's segregated `SizeMap` mapping small byte requests (<= 4096B) into
 *   discrete size classes (`16, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024, 2048, 4096`).
 * - Carve an 8 KiB `Span` into an array of fixed-size objects with ZERO per-object header bytes!
 * - Maintain a `CentralFreeList` per size class that tracks `nonempty_spans` and `empty_spans`.
 * - Implement `tcm_free(ptr)` without boundary tags: look up `Span *s = pagemap_get(addr_to_page_id(ptr))`,
 *   read `s->size_class`, push `ptr` back onto `s->freelist`, and when `s->allocated_count == 0`,
 *   automatically return the entire `Span` back to `PageHeap`!
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
 * Size Class Table & CentralFreeList Data Structures
 * ============================================================================
 *
 * 8 KiB Span Carved into 16-Byte Size-Class Objects (512 Objects, ZERO Headers!):
 * +-----------------+-----------------+-----------------+-----+-----------------+
 * | Obj 0 (16 B)    | Obj 1 (16 B)    | Obj 2 (16 B)    | ... | Obj 511 (16 B)  |
 * | [next_ptr | --] | [next_ptr | --] | [nullptr  | --] |     | [User Payload]  |
 * +-----------------+-----------------+-----------------+-----+-----------------+
 * ^
 * Span Base (PageId * 8192) -> Registered in PageMap -> Span { size_class = 1 (16B) }
 */
enum {
  TCM_PAGE_SHIFT = 13,
  TCM_PAGE_SIZE  = (1 << TCM_PAGE_SHIFT), /* 8192 bytes */
  MAX_SMALL_SIZE = 4096,
  NUM_SIZE_CLASSES = 14                   /* Class 0 is reserved for Large (> 4096B); 1..13 are small classes */
};

/* Canonical Size Classes (16-byte aligned for ABI vector compliance) */
static const size_t kSizeClassBytes[NUM_SIZE_CLASSES] = {
  0,    /* Class 0: Large object (> 4096B), handled directly by PageHeap */
  16,   /* Class 1: 512 objects per 8 KiB Span */
  32,   /* Class 2: 256 objects per 8 KiB Span */
  48,   /* Class 3: 170 objects per 8 KiB Span */
  64,   /* Class 4: 128 objects per 8 KiB Span */
  96,   /* Class 5: 85 objects per 8 KiB Span */
  128,  /* Class 6: 64 objects per 8 KiB Span */
  192,  /* Class 7: 42 objects per 8 KiB Span */
  256,  /* Class 8: 32 objects per 8 KiB Span */
  384,  /* Class 9: 21 objects per 8 KiB Span */
  512,  /* Class 10: 16 objects per 8 KiB Span */
  1024, /* Class 11: 8 objects per 8 KiB Span */
  2048, /* Class 12: 4 objects per 8 KiB Span */
  4096  /* Class 13: 2 objects per 8 KiB Span */
};

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
  void *freelist;      /* Singly-linked intrusive freelist of objects inside this Span */
  struct Span *prev;
  struct Span *next;
} Span;

typedef struct {
  uint32_t size_class;
  size_t object_size;
  Span *nonempty_spans; /* Spans that currently have >= 1 free object in span->freelist */
  Span *empty_spans;    /* Spans where all objects are currently allocated (freelist == nullptr) */
} CentralFreeList;

/*
 * Back-End PageHeap & PageMap Interface (Bring over your Stage 3 implementation
 * or include it here so CentralFreeList can allocate/free Spans and query PageMap!)
 */
[[nodiscard]] Span *pagemap_get(PageId page);
[[nodiscard]] bool pagemap_set(PageId page, Span *span);
[[nodiscard]] bool pageheap_init(size_t num_initial_pages);
void pageheap_teardown(void);
[[nodiscard]] Span *pageheap_allocate_span(size_t num_pages);
void pageheap_free_span(Span *span);

/* ============================================================================
 * Core TCMalloc Middle-End Drills (Implement the TODOs below)
 * ============================================================================ */

/*
 * Drill 1: Size Class Mapping (`size_to_class` and `class_to_size`)
 *
 * Requirements:
 * - Given `size` in bytes:
 *   - If `size == 0`, return 0.
 *   - Find the smallest index `c` in `1 .. NUM_SIZE_CLASSES - 1` such that `kSizeClassBytes[c] >= size`.
 *   - If `size > MAX_SMALL_SIZE` (4096), return `0` (indicating a large multi-page Span request).
 * - `class_to_size(c)` returns `kSizeClassBytes[c]` for `c < NUM_SIZE_CLASSES`, or `0` otherwise.
 */
[[nodiscard]] uint32_t size_to_class(size_t size) {
  (void)size;
  (void)kSizeClassBytes;
  /* TODO: Map byte size to canonical size class index 1..13 (or 0 if > 4096 or 0) */
  return 0;
}

[[nodiscard]] size_t class_to_size(uint32_t size_class) {
  if (size_class >= NUM_SIZE_CLASSES) {
    return 0;
  }
  return kSizeClassBytes[size_class];
}

/*
 * Drill 2: Carving an 8 KiB Span into Headerless Fixed-Size Objects (`span_populate_freelist`)
 *
 * When `CentralFreeList[c]` needs a new Span from `PageHeap`:
 * - Allocate a 1-page (8192-byte) `Span *s` from `PageHeap`.
 * - Set `s->state = SPAN_IN_USE_SMALL`, `s->size_class = c`, `s->allocated_count = 0`.
 * - Compute `obj_size = class_to_size(c)` and `num_objs = TCM_PAGE_SIZE / obj_size`.
 * - Set `s->max_objects = (uint32_t)num_objs`.
 * - Slice the 8192-byte page starting at `base = (uint8_t *)(s->start_page << TCM_PAGE_SHIFT)`
 *   into `num_objs` contiguous blocks of `obj_size` bytes.
 * - Thread an intrusive singly-linked list through all `num_objs` blocks:
 *   Store a pointer to block `i + 1` in the first 8 bytes of block `i` (`*(void **)curr = next`),
 *   with the final block storing `nullptr`.
 * - Set `s->freelist = base`.
 */
void span_populate_freelist(Span *s, uint32_t size_class) {
  (void)s;
  (void)size_class;
  /* TODO: Carve the 8 KiB Span page into a linked list of `8192 / obj_size` headerless objects */
}

/*
 * Drill 3: Batch Object Extraction from `CentralFreeList` (`central_remove_range`)
 *
 * Requirements:
 * - Given `size_class` and a caller array `void *batch_out[count]`:
 * - Pop up to `count` objects from `cfl->nonempty_spans`:
 *   - If `cfl->nonempty_spans == nullptr`, allocate a new 1-page `Span` from `PageHeap`,
 *     call `span_populate_freelist(s, size_class)`, and link `s` into `cfl->nonempty_spans`.
 *   - Pop objects from `s->freelist` (`obj = s->freelist; s->freelist = *(void **)obj;`),
 *     incrementing `s->allocated_count`.
 *   - If `s->freelist == nullptr` (all objects in `s` are now in use!), move `s` from
 *     `cfl->nonempty_spans` to `cfl->empty_spans`!
 * - Return the number of objects actually written into `batch_out`.
 */
[[nodiscard]] size_t central_remove_range(uint32_t size_class, void **batch_out, size_t count) {
  (void)size_class;
  (void)batch_out;
  (void)count;
  /* TODO: Extract up to `count` objects from CentralFreeList[size_class] */
  return 0;
}

/*
 * Drill 4: Batch Object Return & Automatic Empty-Span Reclamation (`central_insert_range`)
 *
 * Requirements:
 * - For each pointer `ptr` in `batch_in[0 .. count - 1]`:
 *   1. Compute `PageId pid = (uintptr_t)ptr >> TCM_PAGE_SHIFT`.
 *   2. Look up `Span *s = pagemap_get(pid)`.
 *   3. Check whether `s->freelist == nullptr` prior to insertion (meaning `s` is currently on `cfl->empty_spans`).
 *   4. Push `ptr` onto `s->freelist` (`*(void **)ptr = s->freelist; s->freelist = ptr;`)
 *      and decrement `s->allocated_count`.
 *   5. If `s` was on `cfl->empty_spans`, move `s` from `cfl->empty_spans` to `cfl->nonempty_spans`.
 *   6. Crucial TCMalloc Invariant: If `s->allocated_count == 0` (every single object in `s` has been freed!):
 *      Unlink `s` from `cfl->nonempty_spans`, clear `s->freelist = nullptr`, and return `s` to
 *      the back-end via `pageheap_free_span(s)` so the OS page can be reused by ANY size class or coalesced!
 */
void central_insert_range(uint32_t size_class, void **batch_in, size_t count) {
  (void)size_class;
  (void)batch_in;
  (void)count;
  /* TODO: Return objects to their owning Spans and reclaim spans whose allocated_count hits 0 */
}

/*
 * Drill 5: Direct Middle-End Allocator Entrypoints (`tcm_central_init`, `tcm_malloc`, `tcm_free`)
 *
 * - `tcm_malloc(size)`:
 *   - If `size == 0`, return `nullptr`.
 *   - Map `size` via `c = size_to_class(size)`.
 *   - If `c > 0` (small object): request 1 object via `central_remove_range(c, &obj, 1)` and return `obj`.
 *   - If `c == 0` (large object > 4096B): compute `num_pages = (size + TCM_PAGE_SIZE - 1) >> TCM_PAGE_SHIFT`,
 *     allocate a large `Span *s = pageheap_allocate_span(num_pages)`, and return `(void *)(s->start_page << TCM_PAGE_SHIFT)`.
 * - `tcm_free(ptr)`:
 *   - If `ptr == nullptr`, return.
 *   - Look up `Span *s = pagemap_get((uintptr_t)ptr >> TCM_PAGE_SHIFT)`.
 *   - If `s->state == SPAN_IN_USE_SMALL`: call `central_insert_range(s->size_class, &ptr, 1)`.
 *   - If `s->state == SPAN_IN_USE_LARGE`: call `pageheap_free_span(s)`.
 */
[[nodiscard]] bool tcm_central_init(size_t num_initial_pages) {
  (void)num_initial_pages;
  /* TODO: Initialize PageHeap and CentralFreeList array */
  return false;
}

void tcm_central_teardown(void) {
  /* TODO: Teardown PageHeap and reset CentralFreeList state */
}

[[nodiscard]] void *tcm_malloc(size_t size) {
  (void)size;
  /* TODO: Dispatch small requests to CentralFreeList and large requests to PageHeap */
  return nullptr;
}

void tcm_free(void *ptr) {
  if (ptr == nullptr) {
    return;
  }
  /* TODO: Resolve Span via PageMap and free to CentralFreeList or PageHeap */
}

/* Helper to inspect Span owning a pointer for verification tests */
[[nodiscard]] Span *tcm_debug_get_span(const void *ptr) {
  (void)ptr;
  /* TODO: Return pagemap_get((uintptr_t)ptr >> TCM_PAGE_SHIFT) */
  return nullptr;
}

/* ============================================================================
 * Verification Harness
 * ============================================================================ */
int main(void) {
  printf("=== Stage 4: TCMalloc Middle-End (Size Classes & CentralFreeList) ===\n");

  assert(tcm_central_init(128) == true);

  /* Test 1: Size Class Mapping */
  assert(size_to_class(0) == 0);
  assert(size_to_class(1) == 1 && class_to_size(1) == 16);
  assert(size_to_class(16) == 1);
  assert(size_to_class(17) == 2 && class_to_size(2) == 32);
  assert(size_to_class(33) == 3 && class_to_size(3) == 48);
  assert(size_to_class(4096) == 13 && class_to_size(13) == 4096);
  assert(size_to_class(4097) == 0); /* Large object */

  /* Test 2: Zero-Header Packing Density Proof!
   * Allocate 512 objects of 16 bytes.
   * Because TCMalloc stores ZERO header bytes inside the page (size_class is stored in the Span!),
   * all 512 objects (512 * 16 = 8192 bytes) MUST fit inside a SINGLE 8 KiB TCMalloc page!
   */
  void *objs[512] = {0};
  for (int i = 0; i < 512; i++) {
    objs[i] = tcm_malloc(16);
    assert(objs[i] != nullptr);
    assert(((uintptr_t)objs[i] % 16) == 0);
    memset(objs[i], 0x3C, 16);
  }

  PageId first_page = (uintptr_t)objs[0] >> TCM_PAGE_SHIFT;
  Span *owning_span = tcm_debug_get_span(objs[0]);
  assert(owning_span != nullptr);
  assert(owning_span->state == SPAN_IN_USE_SMALL);
  assert(owning_span->size_class == 1);
  assert(owning_span->allocated_count == 512);
  assert(owning_span->freelist == nullptr); /* Span is completely full! */

  for (int i = 1; i < 512; i++) {
    PageId pid = (uintptr_t)objs[i] >> TCM_PAGE_SHIFT;
    assert(pid == first_page); /* Every single 16B object is on the exact same 8 KiB page! */
  }

  /* Test 3: Automatic Full-Span Reclamation to PageHeap
   * Free all 512 objects. As soon as the 512th object is freed, `allocated_count` hits 0,
   * and CentralFreeList must return the Span to PageHeap (`state == SPAN_ON_PAGEHEAP`),
   * allowing that page to be repurposed for a 64-byte size class!
   */
  for (int i = 0; i < 512; i++) {
    tcm_free(objs[i]);
  }
  assert(owning_span->allocated_count == 0);
  assert(owning_span->state == SPAN_ON_PAGEHEAP);

  /* Now request a 64-byte object: it can reuse that exact same page for size class 4 (64B)! */
  void *p64 = tcm_malloc(64);
  assert(p64 != nullptr);
  Span *reused_span = tcm_debug_get_span(p64);
  assert(reused_span->size_class == size_to_class(64));
  tcm_free(p64);

  /* Test 4: Large Object (> 4096B) Direct PageHeap Routing */
  void *large = tcm_malloc(20000); /* Needs ceil(20000 / 8192) = 3 pages */
  assert(large != nullptr);
  Span *large_span = tcm_debug_get_span(large);
  assert(large_span != nullptr);
  assert(large_span->state == SPAN_IN_USE_LARGE);
  assert(large_span->num_pages == 3);
  memset(large, 0x7F, 20000);
  tcm_free(large);
  assert(large_span->state == SPAN_ON_PAGEHEAP);

  tcm_central_teardown();
  printf("[PASS] Stage 4: TCMalloc Size Classes & CentralFreeList verified!\n");
  return EXIT_SUCCESS;
}
