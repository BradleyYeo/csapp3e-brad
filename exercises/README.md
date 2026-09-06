# 02: Hands-On Practice Exercises and Deliberate Practice Drills

Hands-on exercises demonstrating modern C23 idioms, memory layout, pointer traversal, and software design principles applied to regex engines and memory allocators.

## Curriculum Reading Sequence
- Layer 01: [01: Systems C Fundamentals, Syntax, and Core Concepts](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/FUNDAMENTALS.md)
- Layer 02: [02: Hands-On Practice Exercises and Deliberate Practice Drills](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/README.md) (Current Document)
- Layer 03: [03: Fixed-Size Bump Allocator Architecture and Implementation Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md)
- Layer 04: [04: Memory Debugging, Sanitizers, and Defect Remediation Manual](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md)

---

## Build and Execution
WSL `sudo apt update && sudo apt install -y clang make build-essential`
`make -C exercises 11_bump_allocator && ./exercises/11_bump_allocator`

### Prerequisites
- Clang or GCC supporting C23 (`-std=c23`).
- Make.

### Running All Tests
```bash
make -C exercises test
```

### Running Individual Exercises
```bash
make -C exercises 01_pointer_traversal && ./exercises/01_pointer_traversal
make -C exercises 02_anchored_matching && ./exercises/02_anchored_matching
make -C exercises 03_getline_ingestion && ./exercises/03_getline_ingestion
make -C exercises 04_enum_state_machine && ./exercises/04_enum_state_machine
make -C exercises 05_char_group_slicing && ./exercises/05_char_group_slicing
make -C exercises 06_mmap_search && ./exercises/06_mmap_search
make -C exercises 07_process_memory_layout && ./exercises/07_process_memory_layout
make -C exercises 08_pointer_arithmetic_strides && ./exercises/08_pointer_arithmetic_strides
make -C exercises 09_dynamic_memory_lifecycle && ./exercises/09_dynamic_memory_lifecycle
make -C exercises 10_memory_safety_sanitizers && ./exercises/10_memory_safety_sanitizers
make -C exercises 11_bump_allocator && ./exercises/11_bump_allocator
```

### Running Under Sanitizers (AddressSanitizer & UndefinedBehaviorSanitizer)
```bash
make -C exercises test-sanitizers
```

See [FUNDAMENTALS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/FUNDAMENTALS.md) for a complete primer on C syntax, pointer concepts, and memory profiling with ASan, Valgrind, Massif, and perf.
See [MEMORY_DEBUGGING.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md) for detailed diagnostics, crash report breakdowns, and leak detection commands.

### Cleaning Build Artifacts
```bash
make -C exercises clean
```

---

# Exercise Guide and Mental Models

> [!TIP]
> If you are new to pointer arithmetic, memory alignment bitwise algebra, or standard types, start by reading [FUNDAMENTALS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/FUNDAMENTALS.md) before implementing the exercises below.

## Exercise 01: Pointer Traversal and Cursor Arithmetic

Source: [01_pointer_traversal.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c)

### Memory Layout
- A string in C is a sequence of characters terminated by null byte `'\0'`.
- Indexing `str[i]` recalculates address `*(str + i)` relative to base pointer `str`.
- Cursor traversal `const char *p` moves a single pointer forward byte-by-byte via `++p`.

```
Byte Array: ['c', 'a', 't', '9', '\0']
Address:     0x20 0x21 0x22 0x23 0x24
Cursor p:     ^
              *p == 'c'
                           ^
                           *p == '9' (isdigit evaluates true)
```

### Core Functions
- [count_digits](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Iterates text with a pointer cursor and counts numeric digits.
- [count_alpha](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Counts alphabetic characters using `isalpha((unsigned char)*p)`.
- [find_first_digit](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Returns the memory address of the first digit found, or `nullptr`.
- [find_last_digit](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Scans backward from null terminator with lower-bound protection (`p > text`).
- [match_prefix](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Lockstep two-pointer prefix matching with `*prefix != '\0'` runaway prevention.
- [find_substring](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.c): Sliding cursor scanner that delegates to `match_prefix` at each byte.

### Deliberate Practice Drills Completed
- Reverse pointer traversal with strict bounds checking.
- Lockstep multi-pointer traversal without `strlen` or index recalculation.
- Modular composition: building `find_substring` on top of `match_prefix`.

---

## Exercise 02: Anchored Matching vs Cursor Scanning

Source: [02_anchored_matching.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c)

### Concept Design (Daniel Jackson)
- Coupling token checking with a full-string search loop leads to code duplication across every regex feature.
- Deconstruct the engine into two independent concepts:
  - Position Predicate: Tests if a pattern matches starting *at this exact byte*.
  - Cursor Scanner: Slides the text cursor forward across the input line.

```
Input: "hello_123"
Pattern: "\\w"

Step 1: match_here("hello_123", "\\w") -> 'h' is word char -> true (Stop)
```

### Core Functions
- [is_word_char](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c): Inspects character with `isalnum((unsigned char)c) || c == '_'`.
- [match_here](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c): Evaluates literal characters and `\w` without looping over input.
- [find_pattern](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c): Drives the search cursor and delegates matching at each byte.

### Deliberate Practice Task
- Add support for digit token `\d` to [match_here](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c).
- Notice how [find_pattern](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_anchored_matching.c) requires zero modifications.
- Connect to [src/main.c](file:///Users/bradleyyeo/Documents/learn/c-learn/codecrafters-grep-c/src/main.c) to replace duplicate `match_digit` and `match_word` scanning loops with a unified `match_here` + `find_pattern` engine.

---

## Exercise 03: Dynamic Buffer Ingestion with `getline`

Source: [03_getline_ingestion.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_getline_ingestion.c)

### Resource Management and Ownership
- Fixed stack buffers (`char buf[1024]`) truncate lines larger than 1023 bytes.
- `getline(&line, &cap, stream)` allocates memory on the heap dynamically, expanding capacity as needed.
- Ownership rule: The caller owns `line` and must invoke `free(line)` on all exit branches.

### O(1) Trailing Line Separator Stripping
- Standard `strcspn(line, "\n")` scans from byte 0 ($O(N)$).
- Inspecting `line[len - 1]` utilizes the length already returned by `getline`, running in $O(1)$ time.

```
Buffer: ['h', 'e', 'l', 'l', 'o', '\n', '\0']
Length: 6
Action: line[5] = '\0'
Result: ['h', 'e', 'l', 'l', 'o', '\0', '\0']
Length: 5
```

### Deliberate Practice Task
- Read lines from a file on disk rather than `fmemopen`.
- Verify absence of memory leaks using Valgrind or compiler AddressSanitizer (`-fsanitize=address`).

---

## Exercise 04: State Modeling with Enums

Source: [04_enum_state_machine.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/04_enum_state_machine.c)

### Defining Errors Out of Existence (Jimmy Koppel)
- In [src/main.c](file:///Users/bradleyyeo/Documents/learn/c-learn/codecrafters-grep-c/src/main.c), unsupported patterns invoke `exit(1)` abruptly.
- Explicit status enums turn potential crashes into recoverable return values.

### Enums vs Magic Integers
- `MatchResult`: `MATCH_FOUND`, `MATCH_NOT_FOUND`, `MATCH_SYNTAX_ERROR`.
- Callers switch on explicit status identifiers with compiler exhaustiveness checking.

```
Pattern String: "\\d"
       |
       v
classify_token() --> PATTERN_DIGIT
       |
       v
match_string()   --> MATCH_FOUND / MATCH_NOT_FOUND
```

### Deliberate Practice Task
- Add a new pattern token `PATTERN_WHITESPACE` for `\s` (`isspace`).
- Update `classify_token` and `test_char` to handle it.

---

## Exercise 05: Character Groups, Slicing, and Set Scanning

Source: [05_char_group_slicing.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_char_group_slicing.c)

### Memory Layout and Pointer Distance
- In C, subtracting two pointers into the same array computes the element distance: `len = end - start`.
- Advancing past `[` with `pattern + 1` sets `start` to the first character of the group.
- Using `strchr(start, ']')` locates the closing delimiter `end`.

```
Index:    0    1    2    3    4    5
Byte:   ['[', 'a', 'b', 'c', ']', '\0']
Address: 0x10 0x11 0x12 0x13 0x14 0x15
Pointers: ^    ^              ^
       pattern start         end
Distance: end - start = 0x14 - 0x11 = 3 bytes
```

### Buffer Slicing and the Null-Termination Invariant
- `memcpy(dest, start, len)` copies exactly `len` bytes; it does NOT append a null terminator.
- Omitting `dest[len] = '\0'` violates the C string contract and causes out-of-bounds reads during later string calls.
- Always check `len < capacity` before copying to define buffer overflows out of existence.

```
Dest buffer: ['a', 'b', 'c', '\0', '?', '?', ...]
Offset:        0    1    2     3     4    5
```

### Set Searching with strpbrk
- `strchr(s, c)` matches a single character.
- `strpbrk(s, accept)` searches `s` for any character in `accept`.
- Acts as a vectorized candidate scanner in production grep, skipping unmatching byte runs in hardware.

### Recursive Descent Pattern
- Base Case: `text == nullptr || *text == '\0'` returns `false`.
- Search Step: `candidate = strpbrk(text, charset)`.
- If `candidate == nullptr`, terminate with `false`.
- Recursive Extension: Recurse on `candidate + 1` when evaluating following pattern tokens.

### Deliberate Practice Tasks
- Modify [05_char_group_slicing.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_char_group_slicing.c) to handle negative character groups `[^abc]`.
- Verify bounds when pattern contains empty brackets `[]`.

---

## Exercise 06: Zero-Copy File Search with mmap

Source: [06_mmap_search.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_mmap_search.c)

### Virtual Memory Mapping Architecture
- Standard `read(2)` copies storage pages from kernel Page Cache to user-space buffers.
- `mmap(2)` maps kernel storage pages directly into process virtual memory addresses (`pte`).
- Avoids memory allocations and intermediate user-space buffer copies.

```
Standard read(2):
[Disk] --DMA--> [Kernel Page Cache] --copy--> [User Buffer]

Memory-Mapped mmap(2):
[Disk] --DMA--> [Kernel Page Cache] <== Page Table Virtual Map (Zero Copy)
```

### Bounded Scanning vs. The Null-Terminator Assumption
- Disk files do NOT end with a null terminator `\0`.
- Running `strchr` or `strlen` on an `mmap` buffer reads past the allocated file mapping, causing a segmentation fault (`SIGSEGV`).
- Production grep uses `memchr(buf, c, len)` which strictly enforces buffer length bounds.

### Resource Lifecycle Management
- Query length via `fstat(fd, &sb)` before mapping.
- Map read-only with `mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0)`.
- Issue kernel readahead hint via `madvise(addr, len, MADV_SEQUENTIAL)`.
- Release pages via `munmap(addr, len)` and close `fd`.

### Deliberate Practice Tasks
- Add sequential substring search across the mapped buffer.
- Benchmark search throughput between `mmap` and `read(2)` on files exceeding 1 MB.

---

## Exercise 07: Process Memory Space and Segment Topology

Source: [07_process_memory_layout.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.c)

### Virtual Memory Segment Map

- Modern OS virtual memory maps each process into an isolated 64-bit address space.
- Segments have distinct permissions and growth behaviors:
  - Text (`.text`): Executable machine instructions, read-only string literals.
  - Initialized Data (`.data`): Global and static variables initialized before runtime.
  - BSS (`.bss`): Uninitialized global and static variables, zero-initialized by kernel page faults.
  - Heap: Dynamically requested memory via `malloc`/`calloc`/`realloc` (grows upwards toward higher addresses).
  - Stack: Automatic function call frames, parameters, and local variables (grows downwards toward lower addresses).

```
High Addresses (0x7FFF... / 0xFFFF...)
+------------------------------------+
| Stack Segment                      |  | Grows downwards toward heap
| - local variables, call frames     |  v (&frame1 > &frame2)
+------------------------------------+
|                 |                  |
|                 v                  |
|          Unallocated Space         |
|                 ^                  |
|                 |                  |
+------------------------------------+
| Heap Segment                       |  ^ Grows upwards toward stack
| - malloc / calloc / realloc blocks |  | (brk / mmap allocations)
+------------------------------------+
| BSS Segment (.bss)                 |  <- Uninitialized globals (zeroed)
+------------------------------------+
| Data Segment (.data)               |  <- Initialized globals & statics
+------------------------------------+
| Text / Code Segment (.text)        |  <- Machine code & literals (RX)
+------------------------------------+
| Reserved Page (0x0)                |  <- NULL pointer dereference traps
Low Addresses (0x0000000000000000)
```

### Core Invariants
- `&func < &data < &bss < heap_ptr << &stack_local`.
- Nested/recursive function call frames allocate locals at strictly decreasing stack addresses.

### Deliberate Practice Tasks
- Classify runtime pointers into their host memory segment using captured boundary heuristic addresses.
- Verify downward stack growth by calculating frame offset difference between parent and nested child function calls.

---

## Exercise 08: Pointer Arithmetic, Strides, and Address Offsets

Source: [08_pointer_arithmetic_strides.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/08_pointer_arithmetic_strides.c)

### Pointer Stride Mechanics
- Adding an integer `n` to a pointer `T *p` does NOT add `n` bytes; it scales by element size:
  `Address(p + n) = Address(p) + (n * sizeof(T))`
- Subtracting two pointers `p2 - p1` into the same array computes element count as signed integer `ptrdiff_t`:
  `p2 - p1 = (ByteDistance) / sizeof(T)`

```
Type: int32_t (4 bytes)
Base: p = 0x1000

p + 0: [ 0x1000 .. 0x1003 ] (int 0)
p + 1: [ 0x1004 .. 0x1007 ] (int 1) -> +4 bytes
p + 2: [ 0x1008 .. 0x100B ] (int 2) -> +8 bytes

Difference: (p + 2) - p = 2 elements (type: ptrdiff_t)
Byte Distance: (uintptr_t)(p + 2) - (uintptr_t)p = 8 bytes
```

### Void Pointer Arithmetic Hazards
- In standard ISO C, `void*` has no size (`sizeof(void)` is undefined).
- Performing arithmetic on `void*` (`ptr++`) is a non-standard GNU extension forbidden under strict `-pedantic`.
- Idiom: Cast to `const unsigned char *` or `uint8_t *` before computing byte offsets.

### Deliberate Practice Tasks
- Implement a generic array walker using raw byte strides without bracket indexing.
- Implement a bounded slice validator that verifies pointer `p` lies strictly within `[start, end)`.

---

## Exercise 09: Dynamic Memory Allocation Lifecycle & Safe Realloc

Source: [09_dynamic_memory_lifecycle.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.c)

### Memory Lifecycle & Ownership
- Dynamic allocations reside on the heap and require explicit deallocation via `free`.
- `malloc(size)`: Allocates raw bytes; contents are uninitialized garbage.
- `calloc(count, size)`: Allocates zeroed bytes with built-in multiplication overflow detection.
- `realloc(ptr, new_size)`: Resizes existing block or migrates contents to a new block.

### Safe Reallocation Pattern
- Naive reallocation (`buf = realloc(buf, new_cap)`) leaks the original allocation if reallocation fails (returning `nullptr`).
- Safe idiom: Store in temporary pointer and check before reassignment:
  ```c
  void *temp = realloc(buf->data, new_cap);
  if (temp == nullptr) {
    return ALLOC_OUT_OF_MEMORY; // buf->data preserved!
  }
  buf->data = temp;
  buf->capacity = new_cap;
  ```

### Defining Dangling Pointers Out of Existence (Jimmy Koppel)
- Always nullify pointers immediately after freeing:
  ```c
  free(buf->data);
  buf->data = nullptr;
  buf->size = 0;
  buf->capacity = 0;
  ```

### Deliberate Practice Tasks
- Implement dynamic buffer growth and shrink-to-fit resizing.
- Implement buffer cloning with independent heap lifecycle and zero shared references.

---

## Exercise 10: Memory Safety Sanitization & Defect Verification

Source: [10_memory_safety_sanitizers.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/10_memory_safety_sanitizers.c)

### Compiler Sanitizers
- AddressSanitizer (ASan): Injects redzones and shadow memory tracking to trap:
  - Heap Use-After-Free (UAF)
  - Heap Buffer Overflow / Underflow
  - Stack Buffer Overflow
  - Global Buffer Overflow
- UndefinedBehaviorSanitizer (UBSan): Traps misaligned pointers, null dereferences, and signed overflows.

### Defect Triggers vs. Defensive Hygiene
- Default execution validates safe defensive patterns (bounds checking, nullification, clean free).
- CLI flags (`--trigger-uaf`, `--trigger-overflow`, `--trigger-double-free`, `--trigger-leak`) allow deliberate defect execution to inspect real sanitizer crash reports.

### Deliberate Practice Tasks
- Run exercise under `make test-sanitizers` and verify clean execution in default mode.
- Trigger `--trigger-uaf` and trace the allocation and free call sites in the ASan error log using [MEMORY_DEBUGGING.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/MEMORY_DEBUGGING.md).

---

## Exercise 11: Fixed-Size Bump Allocator (Arena Allocator)

Source: [11_bump_allocator.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.c)
Guide & Implementation Reference: [11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md)

### Memory Model and Arena Topography
- Backed by a contiguous array of raw bytes (`uint8_t *buffer`) with fixed `capacity`.
- Monotonic pointer advancement: each allocation increments an internal `offset` cursor forward.
- Zero per-allocation metadata overhead: no chunk headers or free-list nodes.
- Bulk reclamation: `bump_reset` rewinds `offset = 0` in $O(1)$ time, reclaiming all blocks simultaneously.

```
[------------------------- Fixed Capacity (e.g. 512 Bytes) -------------------------]
[ Alloc 1: 10 B ][ Alloc 2: 20 B ][ Pad: 2 B ][ Alloc 3: 8 B (aligned) ][ Free Space ]
                                                                        ^
                                                                        offset = 40
```

### Memory Alignment Formula
- Alignment requirement $a$ must be a power of two ($2^k$).
- Address forward-alignment bitwise equation:
  `aligned_addr = (addr + (align - 1)) & ~(align - 1)`
- Padding calculation:
  `padding = aligned_addr - current_addr`

### Invariants and Defensive Hygiene (Jimmy Koppel & John Ousterhout)
- Reject non-power-of-two alignments immediately.
- Prevent integer overflow by verifying `size <= SIZE_MAX - padding - alloc->offset`.
- Ensure `alloc->offset + padding + size <= alloc->capacity`.
- On allocation failure, leave `alloc->offset` unmodified.
- Scoped rollback: `bump_save` captures current offset, and `bump_restore` rewinds cursor to checkpoint without metadata corruption.

### Deliberate Practice Tasks
- Complete the function stubs in [11_bump_allocator.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.c) using the specification comments.
- Verify memory alignment across 1, 2, 4, 8, and 16-byte boundaries.
- Verify clean execution against all 10 unit tests in `main()`.
- Reference [11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md) for the complete annotated reference implementation.

