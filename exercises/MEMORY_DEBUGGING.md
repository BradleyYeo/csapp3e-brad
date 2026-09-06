# Memory Debugging Guide: Sanitizers, Leaks, and Defect Remediation

A practical diagnostic manual for identifying, reading, and fixing memory corruption defects, leaks, and undefined behavior using compiler sanitizers and system diagnostics.

---

# Compiler Sanitizers Overview

## AddressSanitizer (ASan) Architecture

- ASan instruments memory accesses with compiler-generated checks and intercepts standard allocator functions (`malloc`, `calloc`, `realloc`, `free`).
- Virtual address space is divided into two parts:
  - Main Application Memory: Normal process memory space.
  - Shadow Memory: A dedicated range where every 8 bytes of application memory maps to 1 byte of shadow memory.
- Address calculation: `ShadowAddr = (AppAddr >> 3) + Offset`.
- Allocations are padded with poisoned "redzones" before and after the buffer.
- Poisoned memory access instantly triggers a crash report before memory corruption spreads.

### Shadow Memory Byte Encodings

```
Value   State
00      All 8 bytes in corresponding application chunk are addressable.
01-07   First k bytes addressable, remaining (8 - k) bytes poisoned.
fa      Address belongs to Heap Left Redzone.
fb      Address belongs to Heap Right Redzone.
fd      Address belongs to Freed Heap Memory (Use-After-Free trigger).
bb      Address belongs to Stack Left Redzone.
f1      Address belongs to Stack Left Redzone.
f2      Address belongs to Stack Mid Redzone.
f3      Address belongs to Stack Right Redzone.
f5      Address belongs to Global Redzone.
```

---

# Dissecting Sanitizer Crash Reports

## Heap Use-After-Free (UAF)

### Sample ASan Report

```
=================================================================
==84210==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010 at pc 0x000104f4a320 bp 0x7ffee3a9b1c0 sp 0x7ffee3a9b1b8
READ of size 4 at 0x602000000010 thread T0
    #0 0x104f4a31f in inspect_node 10_memory_safety_sanitizers.c:42
    #1 0x104f4a6e8 in main 10_memory_safety_sanitizers.c:118
    #2 0x7fff20355f3c in start (libdyld.dylib:x86_64+0x15f3c)

0x602000000010 is located 0 bytes inside of 16-byte region [0x602000000010,0x602000000020)
freed by thread T0 here:
    #0 0x10526019b in wrap_free (libclang_rt.asan_osx_dynamic.dylib:x86_64+0x4a19b)
    #1 0x104f4a2da in free_node 10_memory_safety_sanitizers.c:31
    #2 0x104f4a6cf in main 10_memory_safety_sanitizers.c:114

previously allocated by thread T0 here:
    #0 0x10525ff0b in wrap_malloc (libclang_rt.asan_osx_dynamic.dylib:x86_64+0x49f0b)
    #1 0x104f4a2a2 in create_node 10_memory_safety_sanitizers.c:20
    #2 0x104f4a6b2 in main 10_memory_safety_sanitizers.c:110

Shadow bytes around the buggy address:
  0x1c0400000000: fa fa fd fd fa fa fa fa fa fa fa fa fa fa fa fa
=>0x1c0400000010:[fd]fd fa fa fa fa fa fa fa fa fa fa fa fa fa fa
```

### Report Breakdown

- `ERROR: AddressSanitizer: heap-use-after-free`: Defect type classification.
- `READ of size 4 at 0x602000000010`: Program attempted to read a 4-byte value (`int` or pointer) from a deallocated address.
- `freed by thread T0 here`: Exact stack trace where the memory was previously deallocated.
- `previously allocated by thread T0 here`: Exact stack trace where the memory was originally created.
- `Shadow bytes: [fd]`: The shadow byte corresponding to the target address contains `fd`, confirming access to freed memory.

---

## Heap Buffer Overflow

### Sample ASan Report

```
=================================================================
==84299==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000030 at pc 0x0001021bc340 bp 0x7ffee5a9b1c0 sp 0x7ffee5a9b1b8
WRITE of size 1 at 0x602000000030 thread T0
    #0 0x1021bc33f in write_payload 10_memory_safety_sanitizers.c:56
    #1 0x1021bc710 in main 10_memory_safety_sanitizers.c:135

0x602000000030 is located 0 bytes to the right of 16-byte region [0x602000000020,0x602000000030)
allocated by thread T0 here:
    #0 0x10246ef0b in wrap_malloc (libclang_rt.asan_osx_dynamic.dylib:x86_64+0x49f0b)
    #1 0x1021bc302 in create_buffer 10_memory_safety_sanitizers.c:48

Shadow bytes around the buggy address:
  0x1c0400000000: fa fa 00 00 fa fa fa fa fa fa fa fa fa fa fa fa
=>0x1c0400000020: 00 00[fa]fa fa fa fa fa fa fa fa fa fa fa fa fa
```

### Report Breakdown

- `0 bytes to the right of 16-byte region`: An off-by-one or out-of-bounds access immediately adjacent to the allocation boundary.
- `Shadow bytes: [fa]`: The shadow byte contains `fa`, indicating the write struck the heap right redzone.

---

## Stack Buffer Overflow

### Sample ASan Report

```
=================================================================
==84350==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffee3a9b188 at pc 0x000104f4a410 bp 0x7ffee3a9b140 sp 0x7ffee3a9b138
WRITE of size 1 at 0x7ffee3a9b188 thread T0
    #0 0x104f4a40f in corrupt_stack 10_memory_safety_sanitizers.c:75
    #1 0x104f4a745 in main 10_memory_safety_sanitizers.c:148

Address 0x7ffee3a9b188 is located in stack of thread T0 at offset 40 in frame
    #0 0x104f4a36f in corrupt_stack 10_memory_safety_sanitizers.c:68

  This frame has 1 object(s):
    [32, 40) 'small_buf' <== Memory access at offset 40 overflows this variable
```

### Report Breakdown

- `[32, 40) 'small_buf'`: Stack frame layout computed by ASan. The local array occupies bytes 32 through 39 (size 8).
- `offset 40 overflows this variable`: Write targeted byte 40 (1 byte past end of array), corrupting stack metadata.

---

# Memory Leak Detection

## Linux (LeakSanitizer / LSan)

- LeakSanitizer is integrated directly into AddressSanitizer on GCC and Clang on Linux.
- Execution:
  ```bash
  ASAN_OPTIONS=detect_leaks=1 ./exercises/09_dynamic_memory_lifecycle
  ```
- Sample LSan Output:
  ```
  =================================================================
  ==91024==ERROR: LeakSanitizer: detected memory leaks

  Direct leak of 32 byte(s) in 1 object(s) allocated from:
      #0 0x7f8a31e84808 in __interceptor_malloc (/lib/x86_64-linux-gnu/libasan.so.5+0x10d808)
      #1 0x55d78a101210 in buffer_create exercises/09_dynamic_memory_lifecycle.c:28
      #2 0x55d78a1014a0 in main exercises/09_dynamic_memory_lifecycle.c:95

  SUMMARY: AddressSanitizer: 32 byte(s) leaked in 1 allocation(s).
  ```

## macOS (Darwin Native Diagnostics)

Apple Clang on macOS does not bundle standalone LSan into ASan. Use macOS native diagnostic tooling:

### Method 1: Using `/usr/bin/leaks` at Process Exit

```bash
leaks --atExit -- ./exercises/09_dynamic_memory_lifecycle
```

Sample Report:
```
Process 84520: 1 leak for 32 total leaked bytes.
Leak: 0x600000a28020  size=32  zone=DefaultMallocZone_0x104868000
    0x68 0x65 0x6c 0x6c 0x6f 0x00 0x00 0x00  hello...
```

### Method 2: Enabling MallocStackLogging for Detailed Traces

```bash
MallocStackLogging=1 ./exercises/09_dynamic_memory_lifecycle &
PID=$!
leaks $PID
```
- Pinpoints the exact allocation call stack corresponding to the leaked address.

---

# UndefinedBehaviorSanitizer (UBSan)

UBSan detects non-fatal undefined behavior that does not immediately crash the application but introduces silent data corruption or compiler misoptimizations.

- Flag: `-fsanitize=undefined`

## Common UBSan Detections

### 1. Pointer Misalignment
```
runtime error: member access within misaligned address 0x600000a28021 for type 'struct Node', which requires 8 byte alignment
```
- Occurs when casting arbitrary byte buffers (`char*`) to typed structs without ensuring hardware alignment.

### 2. Null Pointer Arithmetic
```
runtime error: applying non-zero offset 4 to null pointer
```
- In C23, evaluating `nullptr + offset` is undefined behavior even if never dereferenced.

### 3. Signed Integer Overflow
```
runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
```
- Hardware arithmetic wraps, but in standard C, signed overflow permits the compiler to eliminate entire condition blocks as dead code.

---

# Software Design & Memory Safety Principles

## 1. Defining Errors Out of Existence (Jimmy Koppel)

- Zero Pointers Upon Deallocation:
  ```c
  free(ptr);
  ptr = nullptr;
  ```
  - Defect defined out of existence: Subsequent access crashes immediately at address `0x0` (predictable `SIGSEGV`) instead of silently reading corrupted heap memory.

- Defensive Reallocation:
  ```c
  // WRONG: If realloc fails, original pointer is overwritten with nullptr and memory leaks.
  buf = realloc(buf, new_size);

  // CORRECT:
  void *temp = realloc(buf, new_size);
  if (temp == nullptr) {
    // Preserve buf, handle out-of-memory gracefully
    return ALLOC_OUT_OF_MEMORY;
  }
  buf = temp;
  ```

- Precondition Verification:
  - Check for `nullptr` and zero sizes at API boundaries before executing pointer arithmetic.

## 2. Deep Module Architecture (John Ousterhout)

- Encapsulate Memory Lifecycle:
  - Do not scatter raw `malloc` and `free` across operational scanning logic.
  - Define deep abstract data types (e.g. `Buffer`, `Arena`) where creation, resizing, and cleanup are encapsulated behind short, clear interfaces.
  - Callers never manage raw buffer pointers or recalculate allocation capacities directly.

## 3. Concept Design & Ownership (Daniel Jackson)

- Concept Separation:
  - Separate the concept of **Buffer Storage** (heap allocation, capacity tracking) from **Text Cursor Scanning** (read-only position predicates).
  - Cursors borrow references (`const char*`) and never manage lifecycle.
  - Owners hold mutable pointers (`char*` / `void*`) and have exclusive responsibility for executing deallocation.
