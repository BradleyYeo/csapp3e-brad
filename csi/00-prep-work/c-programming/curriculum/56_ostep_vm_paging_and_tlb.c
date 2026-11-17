/*
 * Exercise 56: Memory Virtualization — Segmentation, Multi-Level Paging, TLB & Clock (OSTEP Ch. 15–22 / xv6)
 *
 * Compile and run:
 *   make -C curriculum 56_ostep_vm_paging_and_tlb && ./curriculum/56_ostep_vm_paging_and_tlb
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

enum {
  SEG_MAX_SIZE = 16384U, /* 14-bit segment offset space */
  VM_PAGE_BITS = 12U,
  VM_PAGE_SIZE = 1U << VM_PAGE_BITS,
  VM_PAGE_MASK = VM_PAGE_SIZE - 1U,
  VM_PT_ENTRIES = 32U,
  VM_VADDR_LIMIT = 1U << 22
};

enum {
  PTE_V = 1U << 0, /* Valid */
  PTE_R = 1U << 1, /* Readable */
  PTE_W = 1U << 2, /* Writable */
  PTE_X = 1U << 3, /* Executable */
  PTE_U = 1U << 4, /* User-accessible */
  PTE_A = 1U << 6, /* Accessed (hardware-set) */
  PTE_D = 1U << 7  /* Dirty (hardware-set on write) */
};

typedef enum {
  VM_OK = 0,
  VM_ERR_INVALID = 1,
  VM_ERR_BOUNDS = 2,
  VM_ERR_PROT = 3
} VmStatus;

typedef struct {
  uint32_t base;
  uint16_t bounds;
  bool grows_negative;
  bool can_read;
  bool can_write;
  bool can_exec;
} SegmentDesc;

typedef struct {
  uint32_t dir[VM_PT_ENTRIES];                    /* PDE: (leaf_tbl_idx << 12) | PTE_V */
  uint32_t tables[VM_PT_ENTRIES][VM_PT_ENTRIES]; /* Leaf PTE: (pfn << 12) | flags */
} TwoLevelPageTable;

typedef struct {
  bool valid;
  bool global;
  uint8_t asid;
  uint32_t vpn;
  uint32_t pfn;
} TlbEntry;

typedef struct {
  bool valid;
  bool ref_bit;
  uint32_t vpn;
} ClockFrame;

/*
 * Drill 1: Hardware Segmentation with Positive & Negative Growth (OSTEP Ch. 16)
 *
 * A 16-bit virtual address encodes:
 *   [ seg_idx (2 bits: 15..14) | offset (14 bits: 13..0) ]
 *
 * Positive growth (Code/Heap): physical = base + offset (valid iff offset < bounds).
 * Negative growth (Stack):     neg_size = SEG_MAX_SIZE - offset;
 *                              physical = base - neg_size (valid iff neg_size <= bounds && base >= neg_size).
 *
 * Requirements:
 * - Return VM_ERR_INVALID if segs == nullptr || paddr_out == nullptr.
 * - Extract `seg_idx` and `offset`.
 * - Check bounds first; return VM_ERR_BOUNDS on out-of-bounds access.
 * - Check permissions next:
 *     - if is_write && !seg.can_write -> VM_ERR_PROT
 *     - if is_exec && !seg.can_exec   -> VM_ERR_PROT
 *     - if !is_write && !is_exec && !seg.can_read -> VM_ERR_PROT
 * - Write translated physical address to `*paddr_out` and return VM_OK.
 */
static VmStatus segment_translate(const SegmentDesc segs[4], uint16_t vaddr,
                                  bool is_write, bool is_exec,
                                  uint32_t *paddr_out) {
  (void)segs;
  (void)vaddr;
  (void)is_write;
  (void)is_exec;
  (void)paddr_out;
  return VM_ERR_INVALID; /* TODO: implement */
}

/*
 * Drill 2: Two-Level Page Table Walk & Hardware A/D Bits (OSTEP Ch. 18–20 / xv6)
 *
 * 22-bit virtual address layout:
 *   [ vpn1 (5 bits: 21..17) | vpn0 (5 bits: 16..12) | offset (12 bits: 11..0) ]
 *
 * Requirements:
 * - Return VM_ERR_INVALID if pt == nullptr || paddr_out == nullptr || vaddr >= VM_VADDR_LIMIT.
 * - Look up `pde = pt->dir[vpn1]`. If `!(pde & PTE_V)`, return VM_ERR_BOUNDS.
 * - Extract leaf table index `tbl_idx = pde >> VM_PAGE_BITS`. If `tbl_idx >= VM_PT_ENTRIES`,
 *   return VM_ERR_BOUNDS.
 * - Look up leaf PTE `pte = pt->tables[tbl_idx][vpn0]`. If `!(pte & PTE_V)`, return VM_ERR_BOUNDS.
 * - Check protection:
 *     - if is_user && !(pte & PTE_U) -> VM_ERR_PROT
 *     - if is_write && !(pte & PTE_W) -> VM_ERR_PROT
 *     - if !is_write && !(pte & PTE_R) -> VM_ERR_PROT
 * - Update hardware tracking bits in `pt->tables[tbl_idx][vpn0]`:
 *     - Always set `PTE_A`.
 *     - If `is_write`, also set `PTE_D`.
 * - Compute `*paddr_out = ((pte >> VM_PAGE_BITS) << VM_PAGE_BITS) | offset` and return VM_OK.
 */
static VmStatus pt_walk_2level(TwoLevelPageTable *pt, uint32_t vaddr,
                               bool is_write, bool is_user,
                               uint32_t *paddr_out) {
  (void)pt;
  (void)vaddr;
  (void)is_write;
  (void)is_user;
  (void)paddr_out;
  return VM_ERR_INVALID; /* TODO: implement */
}

/*
 * Drill 3: Software TLB Lookup with ASID & Context-Switch Flush (OSTEP Ch. 19)
 *
 * Address Space Identifiers (ASIDs) tag TLB entries so user mappings from
 * multiple processes coexist without flushing the entire TLB on context switch.
 * Global entries (`global == true`, e.g., kernel text) match regardless of ASID.
 *
 * Requirements for tlb_lookup:
 * - Return false if tlb == nullptr || pfn_out == nullptr.
 * - Scan `0..cap`: match entry where `valid && vpn == entry.vpn && (entry.global || entry.asid == current_asid)`.
 * - On match, set `*pfn_out = entry.pfn` and return true; else return false.
 *
 * Requirements for tlb_flush_non_global:
 * - If tlb == nullptr, return.
 * - For each entry in `0..cap`, if `!entry.global`, set `entry.valid = false`.
 */
static bool tlb_lookup(const TlbEntry *tlb, size_t cap, uint8_t current_asid,
                       uint32_t vpn, uint32_t *pfn_out) {
  (void)tlb;
  (void)cap;
  (void)current_asid;
  (void)vpn;
  (void)pfn_out;
  return false; /* TODO: implement */
}

static void tlb_flush_non_global(TlbEntry *tlb, size_t cap) {
  (void)tlb;
  (void)cap;
  /* TODO: implement */
}

/*
 * Drill 4: Clock (Second-Chance) Page Replacement Policy (OSTEP Ch. 21–22)
 *
 * Requirements:
 * - Return SIZE_MAX if frames == nullptr || num_frames == 0 || hand_inout == nullptr.
 * - Normalize `size_t h = *hand_inout % num_frames`.
 * - Loop circularly inspecting `frames[h]`:
 *     - If `!frames[h].valid`: install `new_vpn` (`valid = true`, `ref_bit = true`),
 *       advance `*hand_inout = (h + 1) % num_frames`, and return `h`.
 *     - If `frames[h].ref_bit`: give second chance by setting `frames[h].ref_bit = false`
 *       and advancing `h = (h + 1) % num_frames`.
 *     - Otherwise (`valid && !ref_bit`): evict this frame, install `new_vpn`
 *       (`valid = true`, `ref_bit = true`), advance `*hand_inout = (h + 1) % num_frames`,
 *       and return `h`.
 */
static size_t clock_replace_vpn(ClockFrame *frames, size_t num_frames,
                                size_t *hand_inout, uint32_t new_vpn) {
  (void)frames;
  (void)num_frames;
  (void)hand_inout;
  (void)new_vpn;
  return SIZE_MAX; /* TODO: implement */
}

int main(void) {
  /* Drill 1: Segmentation translation (Code=0, Heap=1, Stack=2) */
  SegmentDesc segs[4] = {
      [0] = {.base = 32768U, .bounds = 2048U, .grows_negative = false, .can_read = true, .can_write = false, .can_exec = true},
      [1] = {.base = 34816U, .bounds = 3072U, .grows_negative = false, .can_read = true, .can_write = true, .can_exec = false},
      [2] = {.base = 28672U, .bounds = 2048U, .grows_negative = true, .can_read = true, .can_write = true, .can_exec = false},
      [3] = {0},
  };
  uint32_t paddr = 0;

  /* Code read/exec inside bounds */
  assert(segment_translate(segs, (0U << 14) | 100U, false, true, &paddr) == VM_OK);
  assert(paddr == 32868U);
  /* Write to read-only code segment */
  assert(segment_translate(segs, (0U << 14) | 100U, true, false, &paddr) == VM_ERR_PROT);
  /* Heap out of bounds */
  assert(segment_translate(segs, (1U << 14) | 3072U, false, false, &paddr) == VM_ERR_BOUNDS);
  /* Stack negative growth: offset 16380 -> neg_size = 4 -> base - 4 = 28668 */
  assert(segment_translate(segs, (uint16_t)((2U << 14) | 16380U), true, false, &paddr) == VM_OK);
  assert(paddr == 28668U);
  /* Stack overflow beyond 2KB bounds */
  assert(segment_translate(segs, (uint16_t)((2U << 14) | 14000U), true, false, &paddr) == VM_ERR_BOUNDS);

  /* Drill 2: Two-level page table walk & A/D bits */
  TwoLevelPageTable *pt = calloc(1, sizeof(*pt));
  assert(pt != nullptr);
  /* Map VPN1=3, VPN0=5 -> PFN=0x2A (user R/W); VPN1=3, VPN0=6 -> PFN=0x2B (kernel-only R) */
  pt->dir[3] = (1U << VM_PAGE_BITS) | PTE_V;
  pt->tables[1][5] = (0x2AU << VM_PAGE_BITS) | PTE_V | PTE_R | PTE_W | PTE_U;
  pt->tables[1][6] = (0x2BU << VM_PAGE_BITS) | PTE_V | PTE_R;

  uint32_t va_rw = (3U << 17) | (5U << 12) | 0x345U;
  assert(pt_walk_2level(pt, va_rw, false, true, &paddr) == VM_OK);
  assert(paddr == ((0x2AU << 12) | 0x345U));
  assert((pt->tables[1][5] & PTE_A) != 0);
  assert((pt->tables[1][5] & PTE_D) == 0);

  assert(pt_walk_2level(pt, va_rw, true, true, &paddr) == VM_OK);
  assert((pt->tables[1][5] & (PTE_A | PTE_D)) == (PTE_A | PTE_D));

  uint32_t va_kern = (3U << 17) | (6U << 12) | 0x10U;
  assert(pt_walk_2level(pt, va_kern, false, true, &paddr) == VM_ERR_PROT);
  assert(pt_walk_2level(pt, va_kern, false, false, &paddr) == VM_OK);
  assert(pt_walk_2level(pt, (4U << 17), false, true, &paddr) == VM_ERR_BOUNDS);
  assert(pt_walk_2level(pt, VM_VADDR_LIMIT, false, true, &paddr) == VM_ERR_INVALID);
  free(pt);

  /* Drill 3: Software TLB with ASID & non-global flush */
  TlbEntry tlb[4] = {
      {.valid = true, .global = false, .asid = 1, .vpn = 0x10, .pfn = 0xA0},
      {.valid = true, .global = false, .asid = 2, .vpn = 0x10, .pfn = 0xB0},
      {.valid = true, .global = true, .asid = 0, .vpn = 0xFF, .pfn = 0x01},
      {.valid = false, .global = false, .asid = 0, .vpn = 0, .pfn = 0},
  };
  uint32_t pfn = 0;
  assert(tlb_lookup(tlb, 4, 1, 0x10, &pfn) == true && pfn == 0xA0);
  assert(tlb_lookup(tlb, 4, 2, 0x10, &pfn) == true && pfn == 0xB0);
  assert(tlb_lookup(tlb, 4, 3, 0x10, &pfn) == false);
  assert(tlb_lookup(tlb, 4, 3, 0xFF, &pfn) == true && pfn == 0x01);

  tlb_flush_non_global(tlb, 4);
  assert(tlb_lookup(tlb, 4, 1, 0x10, &pfn) == false);
  assert(tlb_lookup(tlb, 4, 3, 0xFF, &pfn) == true && pfn == 0x01);

  /* Drill 4: Clock replacement */
  ClockFrame frames[3] = {
      {.valid = true, .ref_bit = true, .vpn = 1},
      {.valid = true, .ref_bit = false, .vpn = 2},
      {.valid = true, .ref_bit = true, .vpn = 3},
  };
  size_t hand = 0;
  /* Frame 0 gets second chance (ref_bit cleared); frame 1 is evicted! */
  size_t victim = clock_replace_vpn(frames, 3, &hand, 99);
  assert(victim == 1);
  assert(frames[0].ref_bit == false);
  assert(frames[1].vpn == 99 && frames[1].ref_bit == true);
  assert(hand == 2);

  /* Next miss at hand=2 clears frame 2, then evicts frame 0 (whose ref_bit was cleared) */
  victim = clock_replace_vpn(frames, 3, &hand, 100);
  assert(victim == 0);
  assert(frames[2].ref_bit == false);
  assert(frames[0].vpn == 100 && frames[0].ref_bit == true);
  assert(hand == 1);

  printf("[PASS] Exercise 56: OSTEP VM, Paging, TLB & Clock verified.\n");
  return EXIT_SUCCESS;
}
