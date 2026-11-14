# Stage 4: Exercise 10 - Memory Safety Sanitizers (ASan & UBSan)

A guide explaining AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), shadow memory architecture, redzones, defect diagnosis, and defensive bounded data structures for C beginners.

# Conceptual Foundations and Mental Model

## Why Standard C Does Not Catch Memory Errors
- Standard C optimizes for raw hardware execution speed, omitting runtime bounds checks.
- When code reads past an allocated buffer or reads deallocated heap memory, the hardware often does not crash immediately. Instead, it reads stale bytes or corrupts unrelated variables silently.
- Silent memory corruption leads to insidious security vulnerabilities (arbitrary code execution, info leaks).

## How AddressSanitizer (ASan) Works
- Compile flag: `-fsanitize=address -g`.
- Shadow Memory: ASan maps every 8 bytes of application virtual memory to 1 byte of "shadow memory" ($1/8\text{th}$ overhead):
  $$\text{ShadowAddress} = (\text{AppAddress} \gg 3) + \text{Offset}$$
- Shadow Byte Encodings:
  - `0x00`: All 8 application bytes are valid and addressable.
  - `0x01` through `0x07`: Only the first $k$ bytes are addressable.
  - Negative values (high bit set): The entire 8-byte chunk is poisoned!
    - `0xFA`: Heap left redzone (guards before allocation).
    - `0xFB`: Heap right redzone (guards after allocation).
    - `0xFD`: Freed heap region (detects Use-After-Free).
    - `0xF1` / `0xF2` / `0xF3`: Stack left, mid, and right redzones.
- Redzones: ASan surrounds every stack and heap buffer with unaddressable "poisoned" redzones. If an index overshoots by even a single byte, the injected instrumentation detects the poisoned shadow byte and aborts with a diagnostic crash report.

## UndefinedBehaviorSanitizer (UBSan)
- Compile flag: `-fsanitize=undefined -g`.
- Injects runtime checks for non-memory undefined behaviors: signed integer overflow, division by zero, null pointer dereferences, unaligned pointer access, and out-of-range bit shifts.

---

# Line-by-Line Code Breakdown

## Structure: SafeBlock

```c
typedef struct {
  uint8_t *buffer;
  size_t length;
} SafeBlock;
```

### Encapsulating Size with Pointers
- The root cause of C buffer overflows is passing raw pointers without length bounds.
- Pairing `buffer` with `length` inside `SafeBlock` allows every read and write function to enforce runtime boundary invariants.

---

## Functions: safe_block_write and safe_block_read

```c
static bool safe_block_write(SafeBlock *block, size_t index, uint8_t byte) {
  if (block == nullptr || block->buffer == nullptr || index >= block->length) {
    return false;
  }
  block->buffer[index] = byte;
  return true;
}

static bool safe_block_read(const SafeBlock *block, size_t index, uint8_t *out_byte) {
  if (block == nullptr || block->buffer == nullptr || out_byte == nullptr || index >= block->length) {
    return false;
  }
  *out_byte = block->buffer[index];
  return true;
}
```

### Defining Overflows Out of Existence
- Guard `index >= block->length`: If caller attempts an out-of-bounds access, the function returns `false` rather than executing the invalid write.
- This design defines buffer overflows out of existence at the container boundary.

---

## Defect Trigger Modes in 10_memory_safety_sanitizers.c

The exercise provides CLI flags to deliberately trigger real sanitizer crashes for study:
- `--trigger-uaf`: Triggers `heap-use-after-free` by freeing a pointer and then reading `*p`.
- `--trigger-double-free`: Triggers `attempting double-free` by freeing `p` twice.
- `--trigger-overflow`: Triggers `heap-buffer-overflow` by writing to `buf[8]` on an 8-byte allocation.
- `--trigger-stack-overflow`: Triggers `stack-buffer-overflow` by writing past a local stack frame array.
- `--trigger-leak`: Allocates memory without freeing to demonstrate leak detection.

---

# Memory Layout Visualization

Shadow Memory and Redzones during heap buffer allocation:

```
Application Virtual Memory:
  [ Redzone (16B) ][ Payload Buffer (8B) ][ Redzone (16B) ]
  Address: 0x1000   Address: 0x1010       Address: 0x1018
  Status:  POISONED Status:  VALID        Status:  POISONED

Shadow Memory Representation:
  Address:  Shadow(0x1000)  Shadow(0x1010)  Shadow(0x1018)
  Byte:        [ 0xFA ]         [ 0x00 ]        [ 0xFB ]
               Poisoned          Valid          Poisoned

Access Checks:
  Read 0x1010 -> Shadow byte 0x00 -> ALLOWED (Valid payload)
  Read 0x1018 -> Shadow byte 0xFB -> ABORT: heap-buffer-overflow!
  free(0x1010)-> Shadow byte set to 0xFD (freed)
  Read 0x1010 -> Shadow byte 0xFD -> ABORT: heap-use-after-free!
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the ratio between Application Virtual Memory and Shadow Memory in AddressSanitizer?
- Question 2: What is the purpose of redzones around heap allocations?
- Question 3: How does ASan distinguish between a `heap-use-after-free` and a `heap-buffer-overflow` using shadow byte values?

## Drill: Hands-On Code Validation
- Compile [10_memory_safety_sanitizers.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/10_memory_safety_sanitizers.c) with ASan and UBSan enabled:
  ```bash
  clang -std=c23 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -o 10_memory_safety_sanitizers 10_memory_safety_sanitizers.c
  ```
- Trigger each intentional defect and inspect the stack traces:
  ```bash
  ./10_memory_safety_sanitizers --trigger-uaf
  ./10_memory_safety_sanitizers --trigger-double-free
  ./10_memory_safety_sanitizers --trigger-overflow
  ./10_memory_safety_sanitizers --trigger-stack-overflow
  ```
- Run the default verification suite to ensure normal operations pass cleanly:
  ```bash
  ./10_memory_safety_sanitizers
  ```

---

# Next Step in Curriculum
Proceed to [13_valgrind_fundamentals.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/13_valgrind_fundamentals.md) to explore Dynamic Binary Instrumentation (DBI), Valgrind Memcheck internals (A-bits vs V-bits), and uninitialized memory traps.
