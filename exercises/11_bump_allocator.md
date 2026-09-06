# 03: Fixed-Size Bump Allocator Architecture and Implementation Guide

A bump allocator (also called an arena allocator) manages a contiguous block of memory by maintaining a single cursor (or offset) that increments monotonically forward ("bumps") with each allocation.

## Curriculum Reading Sequence
- Layer 01: [01: Systems C Fundamentals, Syntax, and Core Concepts](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/FUNDAMENTALS.md)
- Layer 02: [02: Hands-On Practice Exercises and Deliberate Practice Drills](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/README.md)
- Layer 03: [03: Fixed-Size Bump Allocator Architecture and Implementation Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md) (Current Document)
- Layer 04: [04: Memory Debugging, Sanitizers, and Defect Remediation Manual](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md)

---

## Process Memory Segmentation Context

In the virtual memory map of a modern process, allocations traditionally take place in one of several segments:
- Stack: Extremely fast, LIFO allocation via CPU stack pointer decrement. Deallocation is automatic when a frame exits.
- Heap: Flexible dynamic memory via `malloc`/`calloc`. Supports arbitrary allocation and deallocation lifetimes at the expense of metadata tracking, fragmentation, and heap synchronization overhead.
- Static / BSS: Global or static variables with process-wide lifetimes.

A bump allocator provides stack-like allocation speed with heap-like sizing flexibility by imposing a lifetime constraint: individual allocations cannot be freed independently; instead, memory is reclaimed all at once in $O(1)$ time.

```
Initial State:
[------------------------- Fixed Capacity (e.g. 1024 Bytes) -------------------------]
^
buffer base
offset = 0

After Allocation 1 (size = 12 bytes, align = 4):
[ Alloc 1: 12 B ][------------------------- Free Space -----------------------------]
                 ^
                 offset = 12

After Allocation 2 (size = 8 bytes, align = 8):
[ Alloc 1: 12 B ][ Pad: 4 B ][ Alloc 2: 8 B ][------------- Free Space -------------]
                             ^
                             aligned_offset = 16
                                             ^
                                             offset = 24
```

## Architectural Comparison: Bump Allocator vs General-Purpose Heap

- Allocation Latency: General-purpose allocators search free lists, bins, or buddy trees ($O(\log N)$ or $O(N)$). Bump allocators perform one addition and bitmask ($O(1)$).
- Memory Overhead: General allocators prepend an 8-to-16 byte chunk header before every allocation to store size, flags, and list pointers. Bump allocators have zero per-allocation metadata overhead.
- Fragmentation: General allocators suffer from external fragmentation when interleaved allocations are freed. Bump allocators experience zero external fragmentation because memory is packed monotonically.
- Cache Locality: Because successive allocations are placed contiguously in virtual address space, traversal across sequentially allocated objects minimizes CPU L1/L2 cache misses.
- Deallocation Tradeoff: Bump allocators do not support individual `free(ptr)`. Deallocation is performed in bulk by resetting the cursor back to the start or rewinding to a saved checkpoint.

---

# Core Concepts in Pointers and Memory Allocation

## Pointer Types and Raw Byte Addressing

C provides multiple pointer representations with distinct operational semantics:
- `void*`: Represents a generic memory address. In standard ISO C, `sizeof(void)` is undefined, and arithmetic on `void*` is prohibited under strict `-pedantic` flags.
- `uint8_t*` (or `unsigned char*`): Guaranteed by standard C to have a size of exactly 1 byte. All pointer displacement and byte offset calculations must be performed using `uint8_t*` or `char*`.
- `uintptr_t`: An unsigned integer type from `<stdint.h>` guaranteed to be capable of holding any pointer address without truncation. Essential for bitwise alignment calculations.

```
Conversion Hierarchy:
Raw Address (void*)
       |
       v  (reinterpret as raw bytes)
Byte Cursor (uint8_t*)
       |
       v  (cast for bitwise arithmetic)
Integer Value (uintptr_t)
```

## Memory Alignment Mechanics

CPUs do not read memory byte-by-byte; they transfer data across memory buses in chunks of 2, 4, 8, 16, or 64 bytes.
- Natural Alignment: An object of size $S$ is naturally aligned when its memory address is a multiple of $S$ (`address % S == 0`).
- Unaligned Access Penalties: On x86_64, unaligned memory accesses require multiple bus transactions, decreasing throughput. On strict RISC architectures (ARM, SPARC), unaligned accesses trigger hardware bus errors (`SIGBUS`).
- Maximum Alignment (`max_align_t`): Defined in `<stddef.h>`, representing the largest alignment required by any scalar type in the architecture (typically 16 bytes on modern 64-bit systems).

### Bitwise Alignment Algebra

To round an address or offset `x` upwards to the next multiple of alignment `a`:
- Precondition: `a` must be a non-zero power of two ($a = 2^k$, such as 1, 2, 4, 8, 16, 64).
- The alignment formula:
  `aligned_x = (x + (a - 1)) & ~(a - 1)`

### Bitwise Operation Breakdown

Suppose `x = 13` (`0b00001101`) and alignment `a = 8` (`0b00001000`):
- Compute Alignment Mask `~(a - 1)`:
  - `a - 1` = $8 - 1 = 7$ (`0b00000111`).
  - `~(a - 1)` = bitwise NOT of 7 = `0b11111000`.
  - Notice that the lower 3 bits are 0. Any value ANDed with this mask will have its lower 3 bits cleared, forcing it to be a multiple of 8.
- Add `a - 1` to `x`:
  - `x + (a - 1)` = $13 + 7 = 20$ (`0b00010100`).
  - Adding $a - 1$ ensures that any value that is not already a multiple of $a$ gets pushed to or past the next multiple of $a$. If $x$ were already aligned (e.g. 16), $16 + 7 = 23$, which still stays within the multiple-of-8 bucket.
- Apply Mask:
  - `20 & ~(7)` = `0b00010100 & 0b11111000` = `0b00010000` = 16.
  - The result is 16, which is the smallest multiple of 8 that is $\ge 13$.

### Power-of-Two Invariant Verification

To ensure alignment `a` is a valid power of two without invoking expensive division or modulo operations:
- A non-zero number `a` is a power of two if and only if `(a & (a - 1)) == 0`.
- Example for $8$ (`0b1000`): $8 - 1 = 7$ (`0b0111`). $8 \ \& \ 7 = 0$.
- Example for $6$ (`0b0110`): $6 - 1 = 5$ (`0b0101`). $6 \ \& \ 5 = 4 \ne 0$.

## Bounds Validation and Integer Overflow Defense

Every allocation must verify that the requested size plus alignment padding does not exceed the remaining capacity.
- Arithmetic Overflow Hazard: A malicious or buggy caller might request `size = SIZE_MAX`. Naively calculating `current_offset + padding + size` wraps around zero, bypassing naive bounds checks (`wrapped_value < capacity`).
- Safe Validation Order:
  - Verify alignment is a valid power of two.
  - Calculate alignment padding: `padding = aligned_offset - current_offset`.
  - Check for integer overflow before addition: `size > SIZE_MAX - padding - alloc->offset`.
  - Check capacity: `alloc->offset + padding + size > alloc->capacity`.

---

# Software Engineering Principles

## John Ousterhout: Deep Module Design

A module is deep if it provides significant functionality through a simple, narrow interface:
- Shallow Module Anti-Pattern: Exposing the raw cursor, requiring callers to manually calculate padding, compute byte offsets, and verify bounds before writing.
- Deep Module Pattern: Callers interact solely with:
  - `bump_init(alloc, buffer, capacity)`
  - `bump_alloc(alloc, size, align)`
  - `bump_reset(alloc)`
- All pointer casting, bitwise alignment masking, overflow guarding, and cursor advancements are private implementation details.

## Daniel Jackson: Concept Design

Concept Design requires identifying independent, orthogonal concepts:
- Backing Store Concept: The raw bytes provided for storage. The allocator does not care whether this buffer originates from the stack (`uint8_t stack_buf[1024]`), static data (`static uint8_t arena[4096]`), or the heap (`malloc(10240)`).
- Cursor Sequencing Concept: The monotonic forward displacement and address calculation relative to the base pointer.
- Checkpoint / Lifetime Concept: Saving and restoring offsets (`bump_save` and `bump_restore`) to provide nested, stack-like allocation scopes without coupling to memory deallocation routines.

## Jimmy Koppel: Defining Illegal States Out of Existence

- Reject invalid configurations at the boundary:
  - Non-power-of-two alignments return `nullptr` immediately.
  - Zero-capacity initialization returns explicit error status.
  - Out-of-memory and integer overflows return `nullptr` without corrupting internal offsets.
- Invariant: At all times, `alloc->offset <= alloc->capacity`.
- Checkpoint Invariant: A `BumpMarker` cannot exceed `alloc->capacity` or the current `alloc->offset`.

---

# Complete C23 Implementation with Detailed Explanations

Here is the complete reference implementation of a fixed-size bump allocator.

```c
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Status codes for allocator initialization.
 * Eliminates magic return values.
 */
typedef enum {
  BUMP_OK = 0,
  BUMP_ERR_INVALID_ARG = 1
} BumpStatus;

/*
 * BumpAllocator Struct
 *
 * Encapsulates a contiguous raw byte buffer and tracks
 * the current allocation offset relative to the base address.
 */
typedef struct {
  uint8_t *buffer;    /* Pointer to the start of the backing memory arena */
  size_t capacity;    /* Total size of the backing buffer in bytes */
  size_t offset;      /* Current allocation cursor (bytes allocated so far) */
} BumpAllocator;

/*
 * BumpMarker Typedef
 *
 * Represents a saved offset snapshot for scoped rollbacks.
 */
typedef size_t BumpMarker;

/*
 * Checks whether a number is a non-zero power of two.
 * Uses bitwise manipulation: (n & (n - 1)) clears the lowest set bit.
 * If the result is zero and n != 0, exactly one bit was set.
 */
static inline bool is_power_of_two(size_t n) {
  return (n != 0) && ((n & (n - 1)) == 0);
}

/*
 * Forward-aligns an integer address or offset to the next multiple of align.
 *
 * Preconditions:
 * - align must be a non-zero power of two.
 *
 * Bitwise Algebra:
 * - (align - 1) produces a mask with the lower k bits set.
 * - ~(align - 1) produces a mask with the lower k bits cleared.
 * - Adding (align - 1) pushes unaligned values to or past the next boundary.
 * - ANDing with ~(align - 1) clears the lower bits, achieving the multiple.
 */
static inline uintptr_t align_forward_address(uintptr_t addr, size_t align) {
  assert(is_power_of_two(align));
  return (addr + (align - 1)) & ~(uintptr_t)(align - 1);
}

/*
 * Initializes a BumpAllocator instance.
 *
 * Parameters:
 * - alloc: Pointer to the BumpAllocator structure to initialize.
 * - memory_block: Pointer to pre-allocated contiguous memory.
 * - capacity_bytes: Size of memory_block in bytes.
 *
 * Returns:
 * - BUMP_OK on success.
 * - BUMP_ERR_INVALID_ARG if any pointer is nullptr or capacity is 0.
 */
static BumpStatus bump_init(BumpAllocator *alloc, void *memory_block, size_t capacity_bytes) {
  if (alloc == nullptr || memory_block == nullptr || capacity_bytes == 0) {
    return BUMP_ERR_INVALID_ARG;
  }

  alloc->buffer = (uint8_t *)memory_block;
  alloc->capacity = capacity_bytes;
  alloc->offset = 0;

  return BUMP_OK;
}

/*
 * Allocates a contiguous block of memory with strict alignment.
 *
 * Parameters:
 * - alloc: Pointer to the initialized BumpAllocator.
 * - size: Number of bytes requested.
 * - align: Required alignment in bytes (must be a power of two).
 *
 * Invariants Enforced:
 * - alloc and alloc->buffer must be non-null.
 * - align must be a power of two (default to alignof(max_align_t) if 0).
 * - Returned pointer must satisfy: ((uintptr_t)ptr % align) == 0.
 * - Must guard against integer overflow during addition.
 * - alloc->offset + padding + size must not exceed alloc->capacity.
 *
 * Returns:
 * - Pointer to the allocated block, or nullptr if out of memory or invalid arguments.
 */
static void *bump_alloc(BumpAllocator *alloc, size_t size, size_t align) {
  /* Guard 1: Validate allocator instance */
  if (alloc == nullptr || alloc->buffer == nullptr) {
    return nullptr;
  }

  /* Guard 2: Default alignment if 0 is passed */
  if (align == 0) {
    align = alignof(max_align_t);
  }

  /* Guard 3: Alignment must be a power of two */
  if (!is_power_of_two(align)) {
    return nullptr;
  }

  /*
   * Handle zero-size allocation:
   * Return a valid aligned pointer at the current offset without consuming space.
   */
  if (size == 0) {
    uintptr_t current_addr = (uintptr_t)(alloc->buffer + alloc->offset);
    uintptr_t aligned_addr = align_forward_address(current_addr, align);
    size_t padding = aligned_addr - current_addr;
    if (alloc->offset + padding > alloc->capacity) {
      return nullptr;
    }
    return (void *)aligned_addr;
  }

  /*
   * Step 1: Calculate current physical address and next aligned address.
   */
  uintptr_t current_addr = (uintptr_t)(alloc->buffer + alloc->offset);
  uintptr_t aligned_addr = align_forward_address(current_addr, align);

  /*
   * Step 2: Compute padding bytes skipped to achieve alignment.
   */
  size_t padding = aligned_addr - current_addr;

  /*
   * Step 3: Integer overflow guard.
   * Verify that offset + padding + size will not overflow size_t.
   */
  if (size > SIZE_MAX - padding || (padding + size) > SIZE_MAX - alloc->offset) {
    return nullptr;
  }

  size_t total_needed = padding + size;

  /*
   * Step 4: Capacity check.
   * Ensure total required space fits within remaining arena capacity.
   */
  if (alloc->offset + total_needed > alloc->capacity) {
    return nullptr; /* Out of Memory */
  }

  /*
   * Step 5: Advance cursor ("bump" the offset).
   */
  alloc->offset += total_needed;

  /*
   * Step 6: Return the aligned memory pointer.
   */
  return (void *)aligned_addr;
}

/*
 * Resets the entire allocator, reclaiming all allocated memory in O(1).
 *
 * Parameters:
 * - alloc: Pointer to the BumpAllocator.
 */
static void bump_reset(BumpAllocator *alloc) {
  if (alloc != nullptr) {
    alloc->offset = 0;
  }
}

/*
 * Saves a checkpoint marker of the current allocation offset.
 * Used for nested or scoped rollbacks.
 */
static BumpMarker bump_save(const BumpAllocator *alloc) {
  if (alloc == nullptr) {
    return 0;
  }
  return alloc->offset;
}

/*
 * Rolls back the allocation offset to a previously saved marker.
 * Reclaims all memory allocated since bump_save was called.
 *
 * Invariant:
 * - marker must be <= alloc->offset to prevent expanding into unallocated space.
 */
static bool bump_restore(BumpAllocator *alloc, BumpMarker marker) {
  if (alloc == nullptr || marker > alloc->offset) {
    return false;
  }
  alloc->offset = marker;
  return true;
}

/*
 * Returns the number of raw bytes remaining in the allocator.
 * Note: Actual allocatable bytes may be slightly less due to alignment padding.
 */
static size_t bump_available(const BumpAllocator *alloc) {
  if (alloc == nullptr || alloc->offset > alloc->capacity) {
    return 0;
  }
  return alloc->capacity - alloc->offset;
}

/*
 * Verifies whether a given pointer belongs to this allocator's buffer range.
 */
static bool bump_owns(const BumpAllocator *alloc, const void *ptr) {
  if (alloc == nullptr || alloc->buffer == nullptr || ptr == nullptr) {
    return false;
  }
  uintptr_t p = (uintptr_t)ptr;
  uintptr_t start = (uintptr_t)alloc->buffer;
  uintptr_t end = start + alloc->capacity;

  return (p >= start) && (p < end);
}
```

---

# Active Recall and Mental Model Checks

Use these questions to verify your understanding before implementing the drills in `11_bump_allocator.c`:

- Why is `sizeof(void*)` not the same as `sizeof(void)`? Why must pointer arithmetic be performed with `uint8_t*` instead of `void*`?
- Given base address `0x1003` and required alignment of `8`, what are the values of `a - 1`, `~(a - 1)`, and `aligned_addr`?
- How does `(a & (a - 1)) == 0` identify whether `a` is a power of two in binary?
- Why can a bump allocator not support an arbitrary `bump_free(ptr)` operation without losing its performance characteristics?
- If `alloc->capacity` is 1024 and `alloc->offset` is 1020, why might an allocation request for 4 bytes fail?
- What potential security vulnerability occurs if an allocator adds `offset + padding + size` without checking for integer overflow?
