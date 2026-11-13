# Stage 3: Exercise 11 - Fixed-Size Bump (Arena) Allocator

A guide explaining arena memory management, power-of-two alignment arithmetic, padding calculations, checkpoint rollback (`save`/`restore`), and bulk $O(1)$ deallocation for C beginners.

# Conceptual Foundations and Mental Model

## What Is a Bump Allocator?
- A bump allocator (or arena allocator) manages a contiguous pre-allocated byte buffer by maintaining an integer cursor (`offset`).
- Allocation: Whenever memory is requested, the allocator checks if enough space remains, aligns the cursor forward to satisfy alignment requirements, and "bumps" the cursor forward:
  $$\text{offset}_{\text{new}} = \text{offset}_{\text{aligned}} + \text{size}$$
- Deallocation: Individual blocks cannot be freed independently. Instead, all allocated memory is reclaimed simultaneously in $O(1)$ time by resetting the offset back to 0 (`bump_reset`).

## Power-of-Two Alignment Algebra
- CPUs transfer data across memory buses most efficiently when addresses are natural multiples of the operand size (e.g. 4-byte integers at addresses divisible by 4; 8-byte pointers at addresses divisible by 8).
- Alignment Requirement: The alignment $A$ must always be a power of two ($A = 2^k$, such as 1, 2, 4, 8, 16, 64).
- Testing for Power of Two:
  ```c
  (n != 0) && ((n & (n - 1)) == 0)
  ```
  Subtracting 1 from a power of two flips all trailing zeros to ones and clears the single set bit. Bitwise ANDing them produces zero.
- Forward Alignment Formula:
  $$\text{aligned\_addr} = (\text{addr} + (A - 1)) \ \& \ \sim(A - 1)$$
  Adding $A - 1$ pushes any unaligned address into or past the next multiple of $A$. Inverting $A - 1$ produces a bitmask with $k$ trailing zeros, which clears the low bits.

## Scoped Checkpoints (Save and Restore)
- For temporary, scoped allocations (such as parsing an expression or processing a single network packet), an arena can save its current offset (`bump_save`).
- After completing the scoped task, calling `bump_restore` rewinds the offset back to the marker, instantly freeing all temporary allocations while preserving older allocations.

---

# Line-by-Line Code Breakdown

## Structure: BumpAllocator

```c
typedef struct {
  uint8_t *buffer;    /* Pointer to start of backing memory arena */
  size_t capacity;    /* Total capacity of backing memory in bytes */
  size_t offset;      /* Allocation cursor: bytes allocated so far */
} BumpAllocator;
```

### Invariants
- `buffer != nullptr`: Backing memory must exist.
- `offset <= capacity`: Offset can never exceed the total capacity.

---

## Function: bump_align_forward

```c
static inline uintptr_t bump_align_forward(uintptr_t addr, size_t align) {
  return (addr + (align - 1)) & ~(uintptr_t)(align - 1);
}
```

### Bitwise Operation Walkthrough
- Suppose `addr = 13` (`0b00001101`) and `align = 8` (`0b00001000`).
- `align - 1` = 7 (`0b00000111`).
- `~(align - 1)` = `0b11111000`.
- `addr + (align - 1)` = $13 + 7 = 20$ (`0b00010100`).
- `20 & 0b11111000` = 16 (`0b00010000`).
- The address is rounded up to 16, the next multiple of 8.

---

## Function: bump_alloc

```c
static void *bump_alloc(BumpAllocator *alloc, size_t size, size_t align) {
  if (alloc == nullptr || alloc->buffer == nullptr) {
    return nullptr;
  }
  if (align == 0) {
    align = alignof(max_align_t);
  }
  if (!is_power_of_two(align)) {
    return nullptr;
  }

  uintptr_t current_addr = (uintptr_t)(alloc->buffer + alloc->offset);
  uintptr_t aligned_addr = bump_align_forward(current_addr, align);
  size_t padding = (size_t)(aligned_addr - current_addr);

  if (size == 0) {
    if (alloc->offset + padding > alloc->capacity) {
      return nullptr;
    }
    return (void *)aligned_addr;
  }

  if (size > SIZE_MAX - padding || (padding + size) > SIZE_MAX - alloc->offset) {
    return nullptr;
  }

  if (alloc->offset + padding + size > alloc->capacity) {
    return nullptr;
  }

  alloc->offset += (padding + size);
  return (void *)aligned_addr;
}
```

### Guard Conditions and Defaults
- `align == 0`: Defaults to `alignof(max_align_t)` (typically 16 bytes).
- `!is_power_of_two(align)`: Rejects non-power-of-two alignments.

### Integer Overflow Defense
- Checks whether `size > SIZE_MAX - padding` or `(padding + size) > SIZE_MAX - alloc->offset` before performing additions. Prevents integer wrapping bugs.

### Capacity Check and Offset Advancement
- Ensures `alloc->offset + padding + size <= alloc->capacity`.
- Bumps offset by the exact sum of alignment padding plus payload size.

---

# Memory Layout Visualization

Allocation progression and alignment padding in an arena:

```
Initial Arena:
  [-------------------------- Capacity: 512 Bytes --------------------------]
  ^
  offset = 0

After Alloc 1 (size = 10, align = 1):
  [ Block 1: 10B ][------------------------ Free Space ----------------------]
                  ^
                  offset = 10

After Alloc 2 (size = 20, align = 1):
  [ Block 1: 10B ][ Block 2: 20B ][--------- Free Space ---------------------]
                                 ^
                                 offset = 30

After Alloc 3 (uint64_t, size = 8, align = 8):
  Current offset 30 is not divisible by 8! Next multiple is 32.
  Padding = 32 - 30 = 2 bytes.
  [ Block 1: 10B ][ Block 2: 20B ][ Pad: 2B ][ Block 3: 8B ][--- Free Space ---]
                                             ^              ^
                                             aligned addr   offset = 40
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: Why does `(n & (n - 1)) == 0` evaluate to true for power-of-two integers? What happens when $n = 0$?
- Question 2: Why are arena allocators immune to external memory fragmentation?
- Question 3: What is the computational complexity ($O$) of `bump_reset` compared to calling `free()` on 10,000 separate heap nodes?

## Drill: Hands-On Code Validation
- Open [11_bump_allocator.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.c).
- Implement an allocation helper:
  ```c
  void *bump_alloc_zeroed(BumpAllocator *alloc, size_t size, size_t align);
  ```
- Use `bump_alloc` followed by `memset` to zero the payload memory.
- Add test assertions in `main()` verifying that all allocated bytes are initialized to `0x00`.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 11_bump_allocator 11_bump_allocator.c
  ./11_bump_allocator
  ```

---

# Next Step in Curriculum
Proceed to [10_memory_safety_sanitizers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/10_memory_safety_sanitizers.md) (Stage 4) to master AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and memory bug diagnosis.
