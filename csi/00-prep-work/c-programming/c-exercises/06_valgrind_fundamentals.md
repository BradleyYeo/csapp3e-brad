# 06: Valgrind Architecture, Memcheck Diagnostics, and Memory Profiling Guide

A diagnostic manual on Valgrind internals, Dynamic Binary Instrumentation (DBI), shadow memory tracking, uninitialized value propagation, leak classification, and containerized diagnostic workflows.

# Curriculum Reading Sequence

- Layer 01: [01: Systems C Fundamentals, Syntax, and Core Concepts](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_FUNDAMENTALS.md)
- Layer 02: [02: Hands-On Practice Exercises and Deliberate Practice Drills](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_README.md)
- Layer 03: [03: Fixed-Size Bump Allocator Architecture and Implementation Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_bump_allocator.md)
- Layer 04: [04: Memory Debugging, Sanitizers, and Defect Remediation Manual](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/04_MEMORY_DEBUGGING.md)
- Layer 05: [05: UNIX Pipes Ping-Pong Benchmark and IPC Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_pipe_pingpong.md)
- Layer 06: [06: Valgrind Architecture, Memcheck Diagnostics, and Memory Profiling Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_valgrind_fundamentals.md) (Current Document)
- Layer 07: [07: Hoare Logic, Loop Invariants, and Refactoring via the Hidden Layer of Logic](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_hoare_logic_refactoring.md)

---

# Dynamic Binary Instrumentation Architecture

## VEX Intermediate Representation and JIT Engine

- Valgrind operates as a virtual machine using Dynamic Binary Instrumentation (DBI).
- Unlike AddressSanitizer (ASan), Valgrind does not instrument code during compilation.
- Machine code translation pipeline:
  - Disassembly: Native machine code (x86_64, arm64) is decoded into an architecture-neutral Intermediate Representation called VEX IR.
  - Instrumentation: Analysis tools (such as Memcheck) inject tracking operations into the VEX IR instructions.
  - JIT Compilation: The augmented VEX IR is compiled just-in-time into native machine instructions and stored in an execution translation cache.
- Because instrumentation happens dynamically on raw binary instructions, Valgrind analyzes third-party shared libraries (`libc`, vendor `.so` files) without requiring their source code or special compiler rebuilds.
- Execution slowdown is roughly 20x to 30x compared to native execution, making it best suited for test suites rather than latency-sensitive benchmarks.

## Shadow State: A-Bits vs V-Bits

Memcheck maintains two distinct bitmasks for every piece of data in the system:

### Addressability Bits (A-bits)
- Resolution: 1 bit per byte in physical memory.
- Semantic Meaning: Indicates whether the program has the legal right to read or write this memory location.
- Lifecycle transitions:
  - Memory allocated via `malloc(size)` sets the A-bits of those `size` bytes to 1 (addressable).
  - Redzones surrounding allocations have A-bits set to 0 (non-addressable).
  - Deallocation via `free(ptr)` resets the A-bits of the block to 0.
- Violation: Accessing an address where A-bit is 0 triggers `Invalid read of size N` or `Invalid write of size N`.

### Validity Bits (V-bits)
- Resolution: 1 bit per bit of data (in both physical memory and CPU registers).
- Semantic Meaning: Indicates whether the data bit contains a defined, initialized value.
- Lifecycle transitions:
  - Newly allocated memory via `malloc(size)` has A-bits = 1, but V-bits = 0 (undefined/uninitialized).
  - Writing a defined constant or expression to a byte sets its V-bits to 1.
  - Arithmetic operations propagate V-bits: computing `c = a + b` sets V-bits of `c` based on the V-bits of `a` and `b`.
- Violation: Executing a decision or hardware effect on bits where V-bit is 0 triggers uninitialized value diagnostics.

## Valgrind Memcheck vs AddressSanitizer (ASan) Tradeoffs

### Instrumentation Point
- ASan: Compile-time instrumentation. Compiler inserts check instructions and redzone offsets directly into the output object code.
- Valgrind: Runtime JIT dynamic translation. Instruments machine code in memory on demand.

### Performance Overhead
- ASan: 1.5x to 2x execution slowdown; ~3x memory footprint increase.
- Valgrind: 20x to 30x execution slowdown; ~4x memory footprint increase.

### Uninitialized Memory Tracking
- ASan: Incapable of tracking bit-level uninitialized memory propagation in CPU registers without MemorySanitizer (MSan). MSan requires recompiling the entire dependency graph including standard libraries.
- Valgrind: Tracks uninitialized data bit-by-bit through memory and all CPU registers out of the box.

### Stack and Global Buffer Overflows
- ASan: Detects stack and global buffer overflows with near 100% reliability because the compiler positions redzones around stack variables and global symbols.
- Valgrind: Limited stack/global overflow detection because it cannot alter the stack layout chosen by the compiler without source-level redzone padding.

---

# Lazy Evaluation and Uninitialized Value Diagnostics

## The Three Trigger Points for Undefined Values

- Memcheck employs lazy reporting: it does not raise an error merely because an uninitialized value was moved, copied, or used in mathematical calculation.
- Storing or computing with undefined bits only propagates zeroes across V-bits.
- An error is reported only when an undefined bit reaches a sink that directly influences observable execution:

### Trigger Point 1: Conditional Branching
- Code evaluates `if (flag)` or `switch (val)` where `flag` contains undefined V-bits.
- Diagnostic:
  ```
  ==12345== Conditional jump or move depends on uninitialised value(s)
  ==12345==    at 0x4012A4: trigger_uninit_branch (13_valgrind_fundamentals.c:236)
  ==12345==    by 0x4018B0: main (13_valgrind_fundamentals.c:380)
  ```

### Trigger Point 2: Memory Address Dereference
- Code uses an undefined value as a pointer or array offset: `array[uninit_index]`.
- Diagnostic:
  ```
  ==12345== Use of uninitialised value of size 8
  ==12345==    at 0x401310: deref_offset (13_valgrind_fundamentals.c:250)
  ```

### Trigger Point 3: System Call Parameter
- Code passes a buffer containing uninitialized bytes across the kernel boundary via a syscall (e.g. `write(fd, buf, count)`).
- Diagnostic:
  ```
  ==12345== Syscall param write(buf) points to uninitialised byte(s)
  ==12345==    at 0x4F24B40: write (in /lib/x86_64-linux-gnu/libc.so.6)
  ==12345==    by 0x401350: trigger_uninit_syscall (13_valgrind_fundamentals.c:255)
  ```

## Tracing Root Causes with --track-origins=yes

- Without origin tracking, Memcheck only shows the line where the branch or syscall occurred, leaving the developer to guess where the uninitialized memory was allocated.
- Passing `--track-origins=yes` instructs Memcheck to maintain an auxiliary table recording the origin of every uninitialized chunk (allocation call site or stack frame declaration).
- Sample Report with Origin Tracking:
  ```
  ==12345== Conditional jump or move depends on uninitialised value(s)
  ==12345==    at 0x4012A4: trigger_uninit_branch (13_valgrind_fundamentals.c:236)
  ==12345==  Uninitialised value was created by a heap allocation
  ==12345==    at 0x4C2DB8F: malloc (vg_replace_malloc.c:309)
  ==12345==    by 0x401280: trigger_uninit_branch (13_valgrind_fundamentals.c:229)
  ```

---

# Memory Leak Taxonomy and Reachability Graph

At process exit, Valgrind scans all memory roots (CPU registers, execution stacks, global/static variables) and traverses pointer graphs to classify every unreclaimed heap block into one of four categories:

```
Roots (Registers, Stack Frames, Global Variables)
   │
   ├──> [ Block A (Offset 0) ] ──> [ Block B (Offset 0) ]  ==> STILL REACHABLE
   │
   └──> [ Block C (Offset +16) ]                           ==> POSSIBLY LOST (Interior Pointer)

Orphaned in Heap:
   [ Block D (Unreachable) ] ──> [ Block E (Unreachable) ]
         │                              │
         ▼                              ▼
  DEFINITELY LOST                 INDIRECTLY LOST
```

## Definitely Lost
- Definition: No surviving pointer to the block exists anywhere in the program's address space.
- Root Cause: Pointer was overwritten (e.g. `ptr = nullptr` or reassigned) without calling `free(ptr)`.
- Action: Absolute defect; must be fixed by adding explicit deallocation.

## Indirectly Lost
- Definition: Pointers to the block still exist, but those pointers are stored exclusively inside other blocks that are themselves lost.
- Example: In a linked list or tree, dropping the root node pointer makes the root `definitely lost`, while all child nodes become `indirectly lost`.
- Action: Fixing the deallocation of the root container automatically resolves indirectly lost children.

## Possibly Lost (Interior Pointers)
- Definition: Pointers pointing into the block exist, but they point to an interior offset ($> 0$), not to the start of the block.
- Root Cause:
  - Custom allocators: Storing a pointer past an internal chunk header.
  - Slicing: Keeping a pointer to a sub-array (`char *cursor = buf + 16`).
  - True leak: Base pointer was lost while an interior cursor remained on the stack.
- Action: Inspect pointer arithmetic to verify if the base pointer was deliberately displaced or leaked.

## Still Reachable
- Definition: Valid pointers to the start of the block still exist at program termination.
- Root Cause: The memory was never freed before `exit(0)` or `return 0` from `main`.
- Analysis: Operating systems automatically reclaim process address spaces on termination. However, in long-running services, Daemons, or modular libraries, failing to free reachable data obscures true leaks and wastes memory over time.

---

# Production Diagnostic Flags and Workflows

## Essential Memcheck Flags

```bash
valgrind \
  --leak-check=full \
  --show-leak-kinds=all \
  --track-origins=yes \
  --show-reachable=yes \
  --error-exitcode=1 \
  ./13_valgrind_fundamentals
```

- `--leak-check=full`: Emits detailed call stacks for every leaked block rather than a simple summary count.
- `--show-leak-kinds=all`: Displays all 4 categories (`definite`, `indirect`, `possible`, `reachable`).
- `--track-origins=yes`: Records the birth site of uninitialized memory.
- `--error-exitcode=1`: Forces Valgrind to exit with return code `1` if any memory defect or leak is detected, failing automated CI pipelines.

## Suppression Files and False-Positive Management

System libraries (such as `libdl`, `libc`, or third-party drivers) may leave memory allocated for caches or global tables. Valgrind allows suppressing known non-defects.

### Generating Suppressions Automatically
```bash
valgrind --leak-check=full --gen-suppressions=all ./program 2>&1 | tee valgrind.log
```

### Format of a Suppression Block
```
{
   dlopen_reachable_cache
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:calloc
   fun:_dl_check_map_versions
   fun:dl_open_worker
}
```

### Running with Suppressions
```bash
valgrind --suppressions=default.supp --error-exitcode=1 ./program
```

## Interactive Post-Mortem Debugging with VGDB

Valgrind embeds a GDB server (VGDB), allowing live step-debugging directly at the instant a memory error occurs.

### Step 1: Launch Valgrind Paused on Error
```bash
valgrind --vgdb=yes --vgdb-error=0 ./13_valgrind_fundamentals --trigger-invalid-read
```

### Step 2: Connect GDB in Another Terminal
```bash
gdb ./13_valgrind_fundamentals
(gdb) target remote | vgdb
(gdb) continue
```
Execution halts on the offending instruction before memory corruption propagates.

---

# Custom Allocator Instrumentation (Client Requests API)

## Memory Poisoning Macros

When implementing custom memory allocators (such as the Fixed-Size Bump Allocator from Layer 03), Valgrind treats the entire arena as a single giant block allocated by `malloc`. Out-of-bounds accesses *between* arena sub-allocations go unnoticed unless explicitly marked.

Valgrind provides client request macros via `<valgrind/memcheck.h>`:

- `VALGRIND_MAKE_MEM_NOACCESS(addr, len)`: Sets A-bits = 0. Any read or write by the program will trigger an immediate `Invalid read` or `Invalid write`.
- `VALGRIND_MAKE_MEM_UNDEFINED(addr, len)`: Sets A-bits = 1, V-bits = 0. Addressable, but marked uninitialized.
- `VALGRIND_MAKE_MEM_DEFINED(addr, len)`: Sets A-bits = 1, V-bits = 1. Fully initialized and addressable.

## Integrating Valgrind with Bump Allocators

```c
#if defined(__has_include)
#if __has_include(<valgrind/memcheck.h>)
#include <valgrind/memcheck.h>
#endif
#endif

void *bump_alloc(BumpAllocator *alloc, size_t size, size_t align) {
  /* Calculate aligned offset */
  size_t current = alloc->offset;
  size_t aligned = (current + (align - 1)) & ~(align - 1);
  if (aligned + size > alloc->capacity) return nullptr;

  void *ptr = alloc->buffer + aligned;
  alloc->offset = aligned + size;

  /* Inform Valgrind: newly handed-out slice is now addressable but undefined */
  VALGRIND_MAKE_MEM_UNDEFINED(ptr, size);

  return ptr;
}

void bump_reset(BumpAllocator *alloc) {
  alloc->offset = 0;
  /* Inform Valgrind: entire arena capacity is now poisoned / unaddressable */
  VALGRIND_MAKE_MEM_NOACCESS(alloc->buffer, alloc->capacity);
}
```

---

# Software Design and Memory Hygiene Principles

## Defining Leaks Out of Existence (Jimmy Koppel)

- Symmetrical Destruction: Every structure constructor (`foo_create`) must have a corresponding destructor (`foo_destroy`) that frees all nested members before freeing the container.
- Zeroing Pointers Post-Free:
  ```c
  free(entry->key);
  entry->key = nullptr;
  ```
  Subsequent dereferences crash deterministically at `0x0` instead of reading stale heap data or corrupting shadow bits.

## Ownership and Concept Separation (Daniel Jackson)

- Distinguish Owners from Borrowers:
  - `ScopeEnvironment`: Owns the lifetime of `SymbolEntry` records.
  - `ScopeCursor`: Borrows a `const SymbolEntry*` for inspection. Never calls `free()` on borrowed references.
- Slicing vs Copying:
  - When returning sub-strings, clarify whether the consumer borrows an interior pointer or receives an independent heap clone.

## Encapsulating Memory Lifecycles (John Ousterhout)

- Deep Modules: The caller should interact with the symbol table exclusively through high-level verbs (`scope_env_insert`, `scope_env_lookup`, `scope_env_destroy`).
- Callers must never be required to manage internal linked list pointers (`entry->next`) or calculate string allocation sizes manually.
