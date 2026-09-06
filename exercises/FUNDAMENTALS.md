# 01: Systems C Fundamentals, Syntax, and Core Concepts

Reading systems-level C code requires understanding pointer notation, standard fixed-width types, memory qualifiers, and modern C23 idioms.

## Curriculum Reading Sequence
- Layer 01: [01: Systems C Fundamentals, Syntax, and Core Concepts](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/FUNDAMENTALS.md) (Current Document)
- Layer 02: [02: Hands-On Practice Exercises and Deliberate Practice Drills](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/README.md)
- Layer 03: [03: Fixed-Size Bump Allocator Architecture and Implementation Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md)
- Layer 04: [04: Memory Debugging, Sanitizers, and Defect Remediation Manual](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md)

---

## Pointer Declarations and the Clockwise Spiral Rule

Pointers store memory addresses. The asterisk `*` indicates indirection, while `&` extracts the address of an object.

### The Right-to-Left Reading Convention
Read pointer declarations starting from the variable name and moving right, then left:
- `char *p`: `p` is a pointer to `char`.
- `const char *p`: `p` is a pointer to a `char` that is constant. The character value `*p` cannot be modified, but the pointer `p` can advance (`++p`).
- `char * const p`: `p` is a `const` pointer to `char`. The pointer address `p` cannot change, but the pointed-to character `*p` can be modified.
- `const char * const p`: Both the address in `p` and the character data `*p` are immutable.
- `const void *base`: Pointer to memory of arbitrary type. Dereferencing `*base` is forbidden because `sizeof(void)` is incomplete.

### Struct Member Access: Dot vs Arrow
- Dot Operator (`.`): Accesses a field on a struct instance directly:
  ```c
  BumpAllocator alloc;
  alloc.offset = 0;
  ```
- Arrow Operator (`->`): Dereferences a pointer to a struct and accesses the member (`p->m` is equivalent to `(*p).m`):
  ```c
  BumpAllocator *p = &alloc;
  p->offset = 0;
  ```

---

## Standard Fixed-Width and Pointer-Sized Types

Standard C provides integer types with guaranteed bit widths in `<stdint.h>` and memory sizing types in `<stddef.h>`:

### Fixed-Width Integer Types
- `uint8_t`: Unsigned 8-bit integer (0 to 255). Standard type for raw byte memory inspection.
- `int32_t`: Signed 32-bit integer (-2,147,483,648 to 2,147,483,647).
- `uint32_t`: Unsigned 32-bit integer (0 to 4,294,967,295).
- `uint64_t`: Unsigned 64-bit integer (0 to $2^{64}-1$).

### Memory and Address Integer Types
- `size_t`: Unsigned integer representing the size of any object in bytes (`sizeof` returns `size_t`). Guarantees representation of maximum addressable memory.
- `ptrdiff_t`: Signed integer representing the difference between two pointers into the same array.
- `uintptr_t`: Unsigned integer large enough to hold any memory address. Essential for bitwise alignment calculations where pointer types cannot be directly bitwise manipulated.

---

## Modern C23 Standard Features

The codebase targets `-std=c23`. Modern C introduces features that make intent explicit:

### Language Keywords and Types
- `nullptr`: Strongly-typed null pointer constant. Replaces the generic macro `NULL` (`(void*)0` or `0`), preventing ambiguous overload conversions.
- `bool`, `true`, `false`: First-class native types without requiring `<stdbool.h>` inclusion in C23.
- `alignof(T)`: Queries the memory alignment boundary requirement in bytes for type `T`.
- `max_align_t`: Defined in `<stddef.h>`, the scalar type with the strictest alignment requirement on the host architecture (typically 16 bytes on 64-bit platforms).

### Designated Initializers
Structs are initialized by naming fields explicitly, defining uninitialized fields to zero:
```c
EventRecord rec = {
  .id = 100,
  .tag = "Network",
  .timestamp = 1718000000
};
```

---

## Bitwise Operators in Systems Code

Low-level allocators and masks rely heavily on bitwise operations:
- Bitwise AND (`&`): Clears bits. Used for masks: `val & 0xFF`.
- Bitwise OR (`|`): Sets bits. Used to combine flags: `PROT_READ | PROT_WRITE`.
- Bitwise XOR (`^`): Toggles bits.
- Bitwise NOT (`~`): Inverts all bits. Crucial for alignment masks: `~(align - 1)`.
- Bitwise Left Shift (`<<`): Multiplies by powers of two: `1 << 3` equals $2^3 = 8$.
- Bitwise Right Shift (`>>`): Divides by powers of two: `val >> 3`.

---

# Conceptual Foundations Across All Exercises

The exercises systematically construct mental models for how memory, pointers, and the operating system interact.

## Exercises 01 and 02: Cursor Scanning vs Position Predicates

Source: [01_pointer_traversal.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c) and [02_anchored_matching.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c)

### Indexing vs Pointer Arithmetic
- Indexing `str[i]` recalculates the absolute memory address `*(str + i)` relative to `str` on every iteration.
- Pointer cursor `const char *p` moves a single register pointer forward via `++p`, checking `*p != '\0'` directly.

### Daniel Jackson Concept Design: Decoupling Predicate from Scanner
- Position Predicate: Tests whether a pattern matches starting at a single exact memory address (`match_here`).
- Cursor Scanner: Iterates through the input string, passing the current cursor to the predicate (`find_pattern`).
- Decoupling ensures adding new pattern syntax requires modifying only the predicate, leaving search traversal untouched.

---

## Exercise 03: Dynamic Buffer Ingestion and Ownership

Source: [03_getline_ingestion.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_getline_ingestion.c)

### Stack Buffers vs Heap Ingestion
- Fixed stack buffers (`char buf[1024]`) fail on arbitrary line lengths, leading to truncation or stack overflow bugs.
- `getline(&line, &cap, stream)` dynamically allocates or expands heap storage via `realloc` as needed.

### Resource Ownership Lifecycle
- The caller of `getline` assumes ownership of the allocated buffer.
- The buffer must be explicitly freed with `free(line)` on all exit paths to prevent memory leaks.

---

## Exercise 04: State Modeling with Enums

Source: [04_enum_state_machine.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/04_enum_state_machine.c)

### Defining Errors Out of Existence (Jimmy Koppel)
- Magic integers (-1, 0, 1) or abrupt calls to `exit(1)` obscure failure modes and prevent callers from recovering.
- Explicit enumerations (`MatchResult`: `MATCH_FOUND`, `MATCH_NOT_FOUND`, `MATCH_SYNTAX_ERROR`) turn errors into typed, compiler-checked return values.

---

## Exercise 05: Character Groups, Slicing, and String Invariants

Source: [05_char_group_slicing.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_char_group_slicing.c)

### Pointer Distance Calculation
- In C, subtracting two pointers that point into the same array evaluates to the number of elements between them:
  `ptrdiff_t len = end - start;`
- Subtraction is element-scaled: `(uintptr_t)end - (uintptr_t)start` divided by `sizeof(*start)`.

### Null-Termination Invariant
- Functions like `memcpy(dest, start, len)` copy exactly `len` bytes without appending a terminating null character.
- Calling standard string functions (`printf`, `strlen`, `strcmp`) on a buffer missing `dest[len] = '\0'` reads out-of-bounds memory.

---

## Exercise 06: Zero-Copy Memory Mapping (`mmap`)

Source: [06_mmap_search.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_mmap_search.c)

### Virtual Memory Zero-Copy Architecture
- Standard `read(2)` copies storage pages from disk into the kernel Page Cache, then copies them a second time into user-space heap/stack buffers.
- `mmap(2)` manipulates process page tables so virtual addresses map directly to kernel Page Cache pages, eliminating user-space memory copies.

### Bounded Scanning vs Null-Terminator Assumption
- Disk files do not terminate with a null byte `'\0'`.
- Using `strlen` or `strchr` on memory-mapped regions causes reading off the end of the mapping, triggering a segmentation fault (`SIGSEGV`).
- Production systems use bounded scanners (`memchr(buf, c, length)`) that strictly respect file size.

---

## Exercise 07: Process Memory Space and Segment Topology

Source: [07_process_memory_layout.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.c)

### Segment Hierarchy in 64-Bit Virtual Memory
- Text Segment (`.text`): Read-only executable machine instructions and string literals.
- Initialized Data Segment (`.data`): Global and static variables initialized before runtime.
- Uninitialized Data Segment (`.bss`): Global and static variables without explicit initialization; zeroed by the kernel.
- Heap Segment: Dynamically allocated memory (`malloc`, `calloc`); grows upwards toward higher addresses.
- Stack Segment: Local variables and function call frames; grows downwards toward lower addresses.

```
High Addresses (0x7FFF...)
+------------------------------------+
| Stack Segment                      |  | Grows downward
|   &frame1 > &frame2                |  v
+------------------------------------+
|                 |                  |
|                 v                  |
|          Unallocated Space         |
|                 ^                  |
|                 |                  |
+------------------------------------+
| Heap Segment                       |  ^ Grows upward
|   malloc / calloc blocks           |  |
+------------------------------------+
| BSS Segment (.bss)                 |  Zero-filled globals
+------------------------------------+
| Data Segment (.data)               |  Initialized globals
+------------------------------------+
| Text Segment (.text)               |  Machine code & literals
+------------------------------------+
| Reserved Null Page (0x0)           |  Traps null pointer access
Low Addresses (0x0000...)
```

---

## Exercise 08: Pointer Arithmetic, Strides, and Address Offsets

Source: [08_pointer_arithmetic_strides.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/08_pointer_arithmetic_strides.c)

### Type Scaling in Pointer Arithmetic
- When adding an integer `n` to a pointer `T *p`, the compiler scales the address by `sizeof(T)`:
  `Address(p + n) = Address(p) + (n * sizeof(T))`
- Adding 1 to an `int32_t *` advances the physical address by 4 bytes.
- Adding 1 to an 8-byte pointer advances the physical address by 8 bytes.

### Void Pointer Arithmetic Prohibition
- ISO C does not define `sizeof(void)`. Arithmetic on `void*` is non-standard.
- To compute raw byte offsets generically, cast the pointer to `const uint8_t *` or `const unsigned char *` before performing additions.

---

## Exercise 09: Dynamic Memory Allocation Lifecycle and Safe Realloc

Source: [09_dynamic_memory_lifecycle.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.c)

### The Safe Reallocation Pattern
- Naive reallocation overwrites the existing pointer: `buf = realloc(buf, new_cap)`.
- If reallocation fails, `realloc` returns `nullptr`, but the original buffer remains allocated in memory, creating a permanent memory leak.
- Idiomatic safe pattern:
  ```c
  void *temp = realloc(buf->data, new_cap);
  if (temp == nullptr) {
    return ALLOC_OUT_OF_MEMORY; /* buf->data remains valid and accessible */
  }
  buf->data = temp;
  buf->capacity = new_cap;
  ```

### Defining Dangling Pointers Out of Existence
- When freeing memory, the pointer retains the numeric address of the deallocated block.
- Subsequent dereferences trigger a Heap Use-After-Free.
- Always nullify pointers immediately after deallocation:
  ```c
  free(buf->data);
  buf->data = nullptr;
  buf->size = 0;
  buf->capacity = 0;
  ```

---

## Exercise 10: Compiler Sanitizers and Memory Hygiene

Source: [10_memory_safety_sanitizers.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/10_memory_safety_sanitizers.c)
Manual: [MEMORY_DEBUGGING.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md)

### Sanitizer Types
- AddressSanitizer (ASan): Injects redzones around allocations and checks every read and write against shadow memory bytes to trap buffer overflows and use-after-free defects.
- UndefinedBehaviorSanitizer (UBSan): Traps misaligned pointers, null dereferences, and signed integer overflows.

---

## Exercise 11: Fixed-Size Bump Allocators (Arena Allocators)

Source: [11_bump_allocator.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.c)
Reference: [11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md)

### Arena Allocation Model
- Allocates from a contiguous buffer using a monotonic forward offset.
- Zero per-allocation headers or metadata.
- Entire arena reclaimed simultaneously in $O(1)$ time via `bump_reset`.

### Memory Alignment Bitwise Algebra
- Alignment requirement $a$ must be a power of two ($a = 2^k$).
- Forward alignment equation:
  `aligned_addr = (addr + (align - 1)) & ~(align - 1)`
- Verification of power of two:
  `(a != 0) && ((a & (a - 1)) == 0)`

---

# Profiling and Identifying Memory Issues

Systems software must be analyzed for memory leaks, invalid accesses, and excessive consumption using specialized profiling tools.

## AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan)

Compiler sanitizers provide zero-overhead runtime defect detection during development.

### Compilation Flags
Compile with frame pointers and sanitizers enabled:
```bash
clang -std=c23 -Wall -Wextra -pedantic -g -fsanitize=address,undefined -fno-omit-frame-pointer -o program program.c
```

### Reading an ASan Report
When ASan detects an illegal memory operation, it prints:
- Defect Type: E.g., `heap-use-after-free`, `heap-buffer-overflow`, `stack-buffer-overflow`.
- Access Size and Address: Size of invalid read/write and memory location.
- Allocation Call Stack: The exact function trace where the memory was originally allocated.
- Deallocation Call Stack: For use-after-free, where the memory was freed.
- Shadow Memory Dump: Displays byte state around the crashing address (`[fd]` = freed memory, `[fa]` = left redzone, `[fb]` = right redzone).

---

## LeakSanitizer (LSan)

LeakSanitizer checks the heap at process exit and reports allocations that were never freed.

### Running with Leak Detection
On Linux and WSL:
```bash
ASAN_OPTIONS=detect_leaks=1 ./program
```

### Interpreting Leak Reports
- Direct Leaks: Blocks of memory allocated with `malloc`/`calloc` where no active pointer in the program references the block.
- Indirect Leaks: Blocks referenced only by memory blocks that are themselves leaked.

---

## Valgrind Memcheck

Valgrind executes programs inside an emulated CPU environment to detect uninitialized memory reads, invalid memory accesses, and memory leaks without requiring recompilation with sanitizer flags.

### Running Memcheck
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./program
```

### Common Valgrind Diagnostics
- `Conditional jump or move depends on uninitialised value(s)`: Code branched on an uninitialized variable (`if (x == 0)` where `x` was never assigned).
- `Use of uninitialised value of size 8`: Reading memory that has not been initialized.
- `Invalid read of size 4` / `Invalid write of size 4`: Reading or writing outside the bounds of an allocated heap block or stack variable.
- `definitely lost`: Memory leaked with zero surviving pointers.
- `indirectly lost`: Leaked memory reachable only from other leaked structures.

---

## Profiling Heap Usage Over Time with Valgrind Massif

Massif measures heap memory usage over the program's lifecycle, identifying peak memory consumption (high-water mark) and memory allocation hot spots.

### Profiling Heap Allocation
```bash
valgrind --tool=massif --pages-as-heap=yes ./program
```
This produces a data file named `massif.out.<pid>`.

### Generating Visual ASCII Heap Graphs
Analyze the data file using `ms_print`:
```bash
ms_print massif.out.* | head -n 40
```

### Sample Massif Output Graph
```
    KB
30.00^                                                  #
     |                                                  #
     |                                                  #
     |                                       @          #
     |                                  :::@:@:::::     #
     |                            :::::::@:@:@:::::     #
     |                      :::::::@:::::@:@:@:::::     #
     |                :::::::@:::::@:::::@:@:@:::::     #
     |          :::::::@:::::@:::::@:::::@:@:@:::::     #
   0 +------------------------------------------------->Ki
     0                                                 120
```
- The graph shows total heap memory (vertical axis) plotted against instructions or allocations executed (horizontal axis).
- Detailed call trees below the graph show which functions were responsible for peak memory allocations.

---

## CPU and Memory Hotspot Profiling with Linux `perf`

`perf` samples hardware performance counters and kernel events to locate performance bottlenecks.

### Recording Performance Profiles
```bash
perf record -g ./program
```
The `-g` flag captures call graph backtraces.

### Inspecting Hotspots
```bash
perf report
```
- Identifies functions consuming the highest percentage of CPU cycles.
- Detects excessive cache misses and page faults caused by unaligned memory access or poor cache locality.

---

## Systematic Memory Debugging Workflow

Follow this systematic sequence when encountering memory corruption, unexpected crashes, or leaks:

### Step 1: Compile Under AddressSanitizer and UBSan
- Recompile with `-fsanitize=address,undefined -g -fno-omit-frame-pointer`.
- Reproduce the crash and inspect the stack trace and shadow bytes.

### Step 2: Validate Pointers and Invariants
- Verify that every pointer returned from an allocator is non-null before dereferencing.
- Check alignment boundaries using `(uintptr_t)ptr % align == 0`.
- Verify half-open bounds: `(uintptr_t)ptr >= start && (uintptr_t)ptr < end`.

### Step 3: Check Null-Termination and Bounded Copying
- Replace unbound string functions (`strcpy`, `strcat`, `strlen`) with bounded alternatives (`strncpy`, `memcpy`, `memchr`).
- Ensure every buffer slice explicitly appends `'\0'` at `dest[len]`.

### Step 4: Trace Allocation Ownership and Free Paths
- Ensure every allocation has a designated owner responsible for freeing it.
- Immediately assign `ptr = nullptr;` after calling `free(ptr)`.
- Use Massif or LSan to confirm memory is reclaimed before process shutdown.
