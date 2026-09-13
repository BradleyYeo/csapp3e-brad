# Stage 3: Exercise 07 - Process Virtual Memory Layout

A foundational guide explaining the virtual address space topography: Text, Initialized Data (.data), Uninitialized Data (.bss), Runtime Heap, and Stack downward growth for C beginners.

# Conceptual Foundations and Mental Model

## Virtual Memory Topography
- Modern operating systems give each process the illusion of a vast, contiguous, private address space (up to 48 or 57 bits on 64-bit platforms).
- The classic Unix process address space is organized from lowest to highest virtual address:
  - `.text` Segment: Executable machine code instructions. Marked read-only and executable (`r-x`) to prevent self-modifying code or accidental overwriting.
  - `.data` Segment: Global and static variables that have an explicit initial value at compile time (e.g. `static int count = 10;`). Marked read-write (`rw-`).
  - `.bss` Segment: Global and static variables that are uninitialized or initialized to zero (e.g. `static int buffer[1024];`). Takes zero space in the executable binary on disk; the OS zero-fills these pages upon demand. Marked read-write (`rw-`).
  - Runtime Heap: Grows upward from lower to higher memory addresses as memory is dynamically requested via `malloc`/`calloc`/`sbrk`.
  - Memory Mapping Area (`mmap`): Shared libraries and file-backed mappings reside between the heap and the stack.
  - Runtime Stack: Stores local variables, function return addresses, and register spill slots. On x86_64 and ARM64 systems, the stack grows downwards (from high memory addresses toward lower memory addresses).

## Why the Stack Grows Downward
- Placing the heap at the bottom growing upward and the stack at the top growing downward allows both dynamic regions to expand towards each other into the large unallocated middle area of virtual memory without pre-allocating fixed boundaries.

---

# Line-by-Line Code Breakdown

## Segment Sampling and Declarations

```c
static int g_initialized_data = 100;
static int g_uninitialized_bss;

static void dummy_function(void) {
}
```

### Static Storage Duration
- `g_initialized_data` has non-zero initial value `100`, placing it in `.data`.
- `g_uninitialized_bss` has no initial value, placing it in `.bss`.
- `&dummy_function` yields the memory address of the first instruction in `.text`.

---

## Function: sample_nested_stack

```c
static void sample_nested_stack(uintptr_t *out_addr) {
  int nested_local = 0;
  *out_addr = (uintptr_t)&nested_local;
}
```

### Stack Frame Sampling
- When `sample_nested_stack` is called, the CPU pushes a new activation frame onto the stack.
- The local variable `nested_local` is allocated in this new frame.
- Its address `&nested_local` is strictly less than variables in the parent frame, proving downward growth.

---

## Function: classify_segment

```c
static SegmentType classify_segment(uintptr_t addr, const MemoryBounds *bounds) {
  if (addr == 0 || bounds == nullptr) {
    return SEG_UNKNOWN;
  }

  if (addr < bounds->data_sample) {
    return SEG_TEXT;
  } else if (addr < bounds->bss_sample) {
    return SEG_DATA;
  } else if (addr < bounds->heap_sample) {
    return SEG_BSS;
  } else if (addr < bounds->stack_sample) {
    return SEG_HEAP;
  } else {
    return SEG_STACK;
  }
}
```

### Relative Range Classification
- Compares numeric address `addr` against runtime sample pointers.
- Because Unix virtual memory orders segments monotonically (`text < data < bss < heap < stack`), simple monotonic threshold checks accurately classify any sample address.

---

## Function: calculate_stack_growth_offset

```c
static ptrdiff_t calculate_stack_growth_offset(uintptr_t parent_frame) {
  if (parent_frame == 0) {
    return 0;
  }

  int child_local = 0;
  return (ptrdiff_t)(parent_frame - (uintptr_t)&child_local);
}
```

### Downward Growth Verification
- If stack grows downwards: `parent_frame > &child_local`, yielding a positive `ptrdiff_t`.
- On x86_64 / ARM64 macOS, this difference is typically between 32 and 64 bytes (the size of the call frame).

---

# Memory Layout Visualization

Standard Unix virtual address space layout:

```
High Memory: 0x7FFFFFFFFFFF
  +-----------------------------------+
  |               STACK               |   |  Stack grows DOWNWARDS (v)
  |  (local vars, frame pointers)     |   v
  +-----------------------------------+
  |                 |                 |
  |                 v                 |
  |                                   |
  |                 ^                 |
  |                 |                 |
  +-----------------------------------+
  |               HEAP                |   ^  Heap grows UPWARDS (^)
  |  (malloc, calloc, realloc)        |   |
  +-----------------------------------+
  |               .bss                |   Read-Write, Zero-Initialized
  |  (uninitialized static/globals)   |
  +-----------------------------------+
  |              .data                |   Read-Write, Initialized
  |  (initialized static/globals)     |
  +-----------------------------------+
  |              .text                |   Read-Only, Executable (Code)
  |  (machine instructions)           |
  +-----------------------------------+
Low Memory:  0x000000000000
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: Why does an uninitialized global array of 1,000,000 integers (`static int big[1000000];`) take almost 0 bytes in the compiled executable binary on disk?
- Question 2: What happens at the hardware/OS level if code tries to write to an address inside the `.text` segment?
- Question 3: How does Address Space Layout Randomization (ASLR) alter the absolute addresses of these segments across program executions while preserving their relative ordering?

## Drill: Hands-On Code Validation
- Open [07_process_memory_layout.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.c).
- Add a recursive function `verify_stack_depth(int depth, uintptr_t prev_addr)` that recurses 5 times, printing the difference `prev_addr - current_addr` on each recursion level.
- Observe whether stack frame spacing remains constant across recursive calls.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 07_process_memory_layout 07_process_memory_layout.c
  ./07_process_memory_layout
  ```

---

# Next Step in Curriculum
Proceed to [09_dynamic_memory_lifecycle.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.md) to master dynamic memory allocation (`malloc`, `calloc`, `realloc`, `free`), safe reallocation idioms, and zeroing pointers.
