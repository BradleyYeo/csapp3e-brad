# Remediation Curriculum & Execution Sequence

# Sequence Overview

- Execution Strategy: Foundations before abstractions. Master memory segments and pointer arithmetic before managing dynamic heap ownership or invoking operating system calls.
- Phase A: Memory Layout & Pointer Arithmetic
- Phase B: Dynamic Allocation & Lifetime Management
- Phase C: Low-Level POSIX System Calls
- Phase D: Architectural Systems Integration

# Phase A: Memory Layout & Pointer Arithmetic

## Stage: Pointer Decay & Stride Mechanics
- Target File: [Chapter5/pointers.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter5/pointers.c)
- Core Concept: Array decay, type-level pointer differences (`char *` vs `char (*)[N]`).
- Key Tasks:
  - Eliminate speculative comments (`// not actually a pointer`).
  - Add explicit assertions using `assert()` validating pointer byte distances.
  - Print and inspect pointer arithmetic strides for `arr + 1`, `&arr[0] + 1`, and `&arr + 1`.

## Stage: String Literal Immutability & Buffer Sizing
- Target File: [Chapter5/exercise_5-3.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter5/exercise_5-3.c)
- Core Concept: Virtual memory segments (`.rodata` vs stack), uninitialized variables.
- Key Tasks:
  - Remove `malloc` from `strcatcopy` to match standard in-place `strcat(char *s, const char *t)`.
  - Pass a stack-allocated buffer with surplus capacity from `main()` (`char s[128] = "...";`).
  - Fix undefined behavior in `getstrlen()` by initializing `int count = 0;`.
  - Mark read-only string parameter with `const char *t`.

## Stage: Bounded String Operations & Suffix Matching
- Target Files:
  - [Chapter5/5.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter5/5.c)
  - [Chapter5/exercise_5-4.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter5/exercise_5-4.c)
- Core Concept: Buffer boundaries, pure pointer arithmetic.
- Key Tasks:
  - Implement `strncpy`, `strncat`, and `strncmp` with strict length limits.
  - Implement `strend(s, t)` using end-of-string pointer arithmetic without array brackets `[]`.

# Phase B: Dynamic Allocation & Lifetime Management

## Stage: Stack Frame Lifetimes & Re-entrant Data Structures
- Target Files:
  - [diagnostic/2.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/diagnostic/2.c)
  - [Chapter4/stack.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter4/stack.c)
- Core Concept: Stack lifetime invalidation, encapsulation, caller-allocated patterns.
- Key Tasks:
  - Fix `diagnostic/2.c` by refactoring from returning a stack pointer to caller-allocated buffer management (`snprintf(out, cap, ...)`).
  - Encapsulate the global stack arrays in `Chapter4/stack.c` into an explicit heap-allocated `Stack` struct with `init`, `push`, `pop`, and `free`.

## Stage: Pointer Indirection & Dynamic Reallocation
- Target Files:
  - [diagnostic/3.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/diagnostic/3.c)
  - [Chapter5/7.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter5/7.c)
- Core Concept: Double indirection (`T **`), safe `realloc` error handling, geometric growth.
- Key Tasks:
  - Fix `diagnostic/3.c` to accept `int **buf` to propagate reallocated heap addresses to the caller.
  - Refactor `Chapter5/7.c` to replace static `allocbuf` with dynamic line allocations.
  - Dynamically resize the line pointer array using geometric capacity scaling.

# Phase C: Low-Level POSIX System Calls

## Stage: File Descriptors & Unbuffered I/O
- Target File: [Chapter8/1.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter8/1.c)
- Core Concept: File descriptors, system call error checking, argument synchronization.
- Key Tasks:
  - Invert reversed file descriptor arguments in `fcopy(1, fd)` to `fcopy(fd, 1)`.
  - Remove desynchronized `curFileIndex` counter and read filenames directly from `*argv`.
  - Loop over `write()` calls to handle partial write situations safely.

# Phase D: Architectural Systems Integration

## Stage: Clean Architecture POSIX ls Clone
- Target Files:
  - [projects/ls_clone.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c)
  - [projects/ls_clone_guide.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone_guide.md)
- Core Concept: Software design principles (Ousterhout, Jackson, Koppel) and modern C23 standards.
- Key Tasks:
  - Complete the 5 exercises in `ls_clone_guide.md`.
  - Replace static `FileInfo files[1024]` with dynamic vector.
  - Implement identity cache to define unmapped UID errors out of existence.
  - Eliminate 64-bit integer truncation in comparator.
  - Use `openat` and `fstatat` with `AT_SYMLINK_NOFOLLOW`.
