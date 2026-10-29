/*
 * Stage 3: TCMalloc Back-End — PageHeap, Spans, and Radix-Tree PageMap
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o stage3_pageheap stage3_tcmalloc_pageheap.c && ./stage3_pageheap
 *
 * Learning Objectives:
 * - Replace in-band boundary tags with TCMalloc's out-of-band `Span` and `PageMap` architecture.
 * - Divide virtual memory into 8 KiB TCMalloc logical pages (`TCM_PAGE_SHIFT = 13`).
 * - Implement a 2-level Radix Tree `PageMap` mapping any 48-bit virtual address `ptr` to its
 *   owning `Span*` in O(1) via `PageId = (uintptr_t)ptr >> 13`.
 * - Manage free page runs in `PageHeap` using segregated page-length lists (`free_spans[k]`)
 *   and coalesce adjacent free `Span`s in O(1) using `PageMap` neighbor lookups!
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
 * TCMalloc Page, Span, and Radix-Tree PageMap Definitions
 * ============================================================================
 *
 * 48-Bit Virtual Address Decomposition (TCM_PAGE_SHIFT = 13, 8 KiB Page):
 * +-------------------------+-------------------------+-------------------+
 * | Bits 47..30 (18 bits)   | Bits 29..13 (17 bits)   | Bits 12..0 (13b)  |
 * | L1 Root Index           | L2 Leaf Index           | Page Byte Offset  |
 * +-------------------------+-------------------------+-------------------+
 *             |                         |
 *             v                         v
 *       root[L1_idx] -----------> leaf[L2_idx] ===> Span*
 */
enum {
  TCM_PAGE_SHIFT = 13,
  TCM_PAGE_SIZE  = (1 << TCM_PAGE_SHIFT), /* 8192 bytes (8 KiB) */
  PAGEMAP_L1_BITS = 18,
  PAGEMAP_L2_BITS = 17,
  PAGEMAP_L1_LENGTH = (1 << PAGEMAP_L1_BITS), /* 262,144 root entries */
  PAGEMAP_L2_LENGTH = (1 << PAGEMAP_L2_BITS), /* 131,072 leaf entries per L2 node */
  MAX_PAGEHEAP_CLASSES = 64                   /* Free lists for 1..63 pages, plus [0] for >= 64 pages */
};

typedef uintptr_t PageId;

typedef enum {
  SPAN_ON_PAGEHEAP = 0,
  SPAN_IN_USE_SMALL = 1,
  SPAN_IN_USE_LARGE = 2
} SpanState;

typedef struct Span {
  PageId start_page;          /* Starting 8 KiB PageId */
  size_t num_pages;           /* Number of contiguous 8 KiB pages in this Span */
  uint32_t size_class;        /* 0 for large allocations; >0 for small-object size classes */
  uint32_t allocated_count;   /* Number of live objects currently allocated out of this Span */
  uint32_t max_objects;       /* Total objects carved when state == SPAN_IN_USE_SMALL */
  SpanState state;            /* Single authority for Span ownership state */
  void *freelist;             /* Intrusive singly-linked list of free objects (Stage 4) */
  struct Span *prev;          /* Doubly-linked list pointer (for PageHeap or CentralFreeList) */
  struct Span *next;
} Span;

typedef struct {
  Span *values[PAGEMAP_L2_LENGTH];
} PageMapLeaf;

typedef struct {
  PageMapLeaf *root[PAGEMAP_L1_LENGTH];
} PageMap;

typedef struct {
  PageMap pagemap;
  /* Segregated doubly-linked lists of free Spans indexed by num_pages (1..63), and [0] for >= 64 */
  Span *free_lists[MAX_PAGEHEAP_CLASSES];
  /* Metadata arena for allocating Span structs and PageMapLeaf nodes without calling stdlib malloc */
  uint8_t *meta_base;
  size_t meta_capacity;
  size_t meta_offset;
  /* Backing virtual page slab for test determinism */
  uint8_t *BackingSlabBase;
  size_t BackingSlabBytes;
} PageHeap;

static PageHeap g_pageheap = {0};

/* ============================================================================
 * Address <-> PageId Conversion Helpers
 * ============================================================================ */

static inline PageId addr_to_page_id(const void *addr) {
  return (uintptr_t)addr >> TCM_PAGE_SHIFT;
}

static inline void *page_id_to_addr(PageId page_id) {
  return (void *)(page_id << TCM_PAGE_SHIFT);
}

/* Internal metadata bump allocator for Span and PageMapLeaf structs */
static void *meta_alloc(size_t size, size_t align) {
  uintptr_t curr = (uintptr_t)(g_pageheap.meta_base + g_pageheap.meta_offset);
  uintptr_t aligned = (curr + (align - 1)) & ~(uintptr_t)(align - 1);
  size_t pad = aligned - curr;
  if (g_pageheap.meta_offset + pad + size > g_pageheap.meta_capacity) {
    return nullptr;
  }
  g_pageheap.meta_offset += pad + size;
  void *res = (void *)aligned;
  memset(res, 0, size);
  return res;
}

/* ============================================================================
 * Core TCMalloc Back-End Drills (Implement the TODOs below)
 * ============================================================================ */

/*
 * Drill 1: Two-Level Radix Tree `PageMap` Lookup and Registration
 *
 * Given a 35-bit `PageId` (from a 48-bit virtual address shifted right by 13):
 * - Compute `l1_idx = page >> PAGEMAP_L2_BITS` and `l2_idx = page & (PAGEMAP_L2_LENGTH - 1)`.
 * - Guard that `l1_idx < PAGEMAP_L1_LENGTH`.
 * - In `pagemap_get(page)`:
 *   If `g_pageheap.pagemap.root[l1_idx] == nullptr`, return `nullptr`.
 *   Otherwise return `g_pageheap.pagemap.root[l1_idx]->values[l2_idx]`.
 * - In `pagemap_set(page, span)`:
 *   If `g_pageheap.pagemap.root[l1_idx] == nullptr`, allocate a `PageMapLeaf` via
 *   `meta_alloc(sizeof(PageMapLeaf), alignof(PageMapLeaf))`.
 *   Store `span` in `root[l1_idx]->values[l2_idx]` and return `true`.
 * - In `pagemap_register_span(span)`:
 *   For a FREE span (`span->state == SPAN_ON_PAGEHEAP`), at minimum its first and last page
 *   (`start_page` and `start_page + num_pages - 1`) must point to `span` in the PageMap so
 *   neighbor coalescing works in O(1); registering all pages `[start_page .. start_page + num_pages - 1]`
 *   makes `pagemap_get` work for any interior byte address! Register all pages in the span.
 */
[[nodiscard]] Span *pagemap_get(PageId page) {
  (void)page;
  /* TODO: Implement O(1) 2-level radix tree lookup */
  return nullptr;
}

[[nodiscard]] bool pagemap_set(PageId page, Span *span) {
  (void)page;
  (void)span;
  (void)meta_alloc;
  /* TODO: Implement 2-level radix tree insertion with on-demand leaf allocation */
  return false;
}

[[nodiscard]] bool pagemap_register_span(Span *span) {
  (void)span;
  /* TODO: Register every PageId in [span->start_page, span->start_page + span->num_pages) */
  return false;
}

/*
 * Drill 2: PageHeap Initialization & Teardown
 *
 * Requirements:
 * - Map a metadata arena (`meta_base`) of 16 MiB via anonymous `mmap` to hold `Span` and `PageMapLeaf` structs.
 * - Map an initial contiguous page-aligned heap region of `num_initial_pages * TCM_PAGE_SIZE` via `mmap`.
 *   (Ensure its base address is aligned to `TCM_PAGE_SIZE = 8192`; if `mmap` returns a 4KB-aligned address
 *   on Linux, over-allocate by `TCM_PAGE_SIZE` and align the start pointer forward to 8192!).
 * - Create an initial `Span` covering all `num_initial_pages`, set `state = SPAN_ON_PAGEHEAP`,
 *   register it in the `PageMap`, and insert it into the appropriate `g_pageheap.free_lists` bucket.
 */
[[nodiscard]] bool pageheap_init(size_t num_initial_pages) {
  (void)num_initial_pages;
  (void)addr_to_page_id;
  /* TODO: Map metadata arena and initial backing pages, create initial Span, register in PageMap */
  return false;
}

void pageheap_teardown(void) {
  if (g_pageheap.BackingSlabBase != nullptr) {
    munmap(g_pageheap.BackingSlabBase, g_pageheap.BackingSlabBytes);
  }
  if (g_pageheap.meta_base != nullptr) {
    munmap(g_pageheap.meta_base, g_pageheap.meta_capacity);
  }
  memset(&g_pageheap, 0, sizeof(g_pageheap));
}

/*
 * Drill 3: Span Allocation & Carving (`pageheap_allocate_span`)
 *
 * Requirements:
 * - Return `nullptr` if `num_pages == 0`.
 * - Search `g_pageheap.free_lists[k]` starting at `k = (num_pages < MAX_PAGEHEAP_CLASSES) ? num_pages : 0`
 *   up through `MAX_PAGEHEAP_CLASSES - 1`, and finally check bucket `[0]` (large spans >= 64 pages)
 *   for the first `Span *s` with `s->num_pages >= num_pages`.
 * - Unlink `s` from its free list.
 * - If `s->num_pages > num_pages`:
 *   Carve `s`! Allocate a new `Span *remainder` via `meta_alloc(sizeof(Span), alignof(Span))`
 *   with `start_page = s->start_page + num_pages`, `num_pages = s->num_pages - num_pages`,
 *   `state = SPAN_ON_PAGEHEAP`.
 *   Register `remainder` in `PageMap` and insert `remainder` into the free list for its page count!
 *   Shrink `s->num_pages = num_pages` and re-register `s` in `PageMap`.
 * - Set `s->state = SPAN_IN_USE_LARGE` and return `s`.
 */
[[nodiscard]] Span *pageheap_allocate_span(size_t num_pages) {
  (void)num_pages;
  /* TODO: Search segregated page lists, carve excess pages into a remainder Span, and return */
  return nullptr;
}

/*
 * Drill 4: Radix-Tree Neighbor Coalescing (`pageheap_free_span`)
 *
 * How does TCMalloc coalesce spans WITHOUT boundary tags inside user pages?
 * Via the Radix-Tree `PageMap`!
 * - Given `span` being freed:
 *   1. Check left neighbor page: `Span *prev = pagemap_get(span->start_page - 1)`.
 *      If `prev != nullptr && prev->state == SPAN_ON_PAGEHEAP`:
 *      Unlink `prev` from its `PageHeap` free list, set `span->start_page = prev->start_page`,
 *      and `span->num_pages += prev->num_pages`.
 *   2. Check right neighbor page: `Span *next = pagemap_get(span->start_page + span->num_pages)`.
 *      If `next != nullptr && next->state == SPAN_ON_PAGEHEAP`:
 *      Unlink `next` from its `PageHeap` free list, and `span->num_pages += next->num_pages`.
 *   3. Set `span->state = SPAN_ON_PAGEHEAP`, `span->size_class = 0`, `span->allocated_count = 0`.
 *   4. Call `pagemap_register_span(span)` so all pages of the newly coalesced span point to `span`.
 *   5. Insert `span` into the appropriate `g_pageheap.free_lists` bucket.
 */
void pageheap_free_span(Span *span) {
  if (span == nullptr) {
    return;
  }
  /* TODO: Coalesce left and right free neighbor spans via pagemap_get and re-insert into free_lists */
}

/* ============================================================================
 * Verification Harness
 * ============================================================================ */
int main(void) {
  printf("=== Stage 3: TCMalloc Back-End (PageHeap, Spans & Radix-Tree PageMap) ===\n");

  /* Initialize PageHeap with 128 pages (1 MiB of 8 KiB pages) */
  assert(pageheap_init(128) == true);

  /* Test 1: Allocate Spans and Verify O(1) PageMap Pointer Resolution */
  Span *s1 = pageheap_allocate_span(1);
  Span *s2 = pageheap_allocate_span(4);
  Span *s3 = pageheap_allocate_span(2);
  Span *s4_guard = pageheap_allocate_span(1); /* Guard span preventing s3 from coalescing with wilderness */
  assert(s1 != nullptr && s2 != nullptr && s3 != nullptr && s4_guard != nullptr);
  assert(s1->num_pages == 1 && s2->num_pages == 4 && s3->num_pages == 2);
  assert(s2->start_page == s1->start_page + 1);
  assert(s3->start_page == s2->start_page + 4);

  /* Any arbitrary byte offset inside s2's 4 pages (32 KiB) must resolve to s2 via PageMap! */
  uint8_t *s2_mem = (uint8_t *)page_id_to_addr(s2->start_page);
  for (size_t offset = 0; offset < 4 * TCM_PAGE_SIZE; offset += 511) {
    uint8_t *interior_ptr = s2_mem + offset;
    PageId pid = addr_to_page_id(interior_ptr);
    assert(pagemap_get(pid) == s2);
  }

  /* Verify memory is writable across the entire 4-page Span */
  memset(s2_mem, 0x5A, 4 * TCM_PAGE_SIZE);

  /* Test 2: Out-of-Band PageMap Coalescing (Left, Right, and Triple Merge) */
  PageId base_page = s1->start_page;
  pageheap_free_span(s1); /* Free left (1 page) */
  pageheap_free_span(s3); /* Free right (2 pages; blocked on right by s4_guard) */
  assert(pagemap_get(base_page)->state == SPAN_ON_PAGEHEAP);
  assert(pagemap_get(base_page)->num_pages == 1);

  /* Now free middle s2 (4 pages): must coalesce s1 (1) + s2 (4) + s3 (2) = 7-page free Span! */
  pageheap_free_span(s2);
  Span *merged = pagemap_get(base_page);
  assert(merged != nullptr);
  assert(merged->state == SPAN_ON_PAGEHEAP);
  assert(merged->start_page == base_page);
  assert(merged->num_pages == 7);
  /* Verify tail page of merged span also points to `merged` */
  assert(pagemap_get(base_page + 6) == merged);

  /* Test 3: Re-allocating 7 pages should reuse the coalesced Span at base_page */
  Span *s_reused = pageheap_allocate_span(7);
  assert(s_reused != nullptr);
  assert(s_reused->start_page == base_page);
  assert(s_reused->num_pages == 7);

  pageheap_free_span(s_reused);
  pageheap_free_span(s4_guard);
  pageheap_teardown();

  printf("[PASS] Stage 3: TCMalloc PageHeap, Spans, and Radix-Tree PageMap verified!\n");
  return EXIT_SUCCESS;
}
