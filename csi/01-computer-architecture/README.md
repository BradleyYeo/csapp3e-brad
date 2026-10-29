# Computer Architecture and Systems Fundamentals

## Overview and Curriculum Progression

This module covers computer architecture, machine-level programming, memory hierarchies, and low-level systems programming. The exercises are structured in strict pedagogical sequence: each milestone builds directly upon the mental models established in the preceding exercises.

```
01-virtual-machine ──────► 02-binary-encodings ──────► 03-x86-64-assembly-intro
(Von Neumann CPU)          (Two's comp, floats)        (ABI, registers, loops)
                                                               │
                                                               ▼
06-memory-hierarchy-caching ◄─ 05-low-level-optimization ◄─ 04-x86-64-assembly-further
(Cache lines, strides)      (ILP, unrolling, accum)    (Arrays, floating point)
       │
       ▼
07-memory-hierarchy-data-layout ──► 08-exceptional-control-flow
(SoA, columnar arrays, Go)          (Processes, signals, Unix shell)
```

## Exercise Sequence and Learning Objectives

### 1. Virtual Machine and CPU Emulator
- Directory: [01-virtual-machine/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/01-virtual-machine)
- Core Concepts: Von Neumann architecture, Program Counter fetch-increment invariant, Fetch-Decode-Execute cycle, Deep Decoder, Single Authority Principle.
- Textbook: CS:APP3e Chapter 4 (Y86-64 SEQ processor).
- Milestone: Build an 8-bit CPU emulator executing variable-length bytecodes from scratch.

### 2. Binary Encodings of Data
- Directory: [02-binary-encodings/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings)
- Core Concepts: Hexadecimal, Two's complement integer arithmetic, carry overflow detection, Big-Endian vs. Little-Endian, IEEE 754 floating point, UTF-8 variable-width character encoding, binary headers (BMP, TCP).
- Textbook: CS:APP3e Chapter 2.

### 3. Introduction to x86-64 Assembly Programming
- Directory: [03-x86-64-assembly-intro/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/03-x86-64-assembly-intro)
- Core Concepts: System V AMD64 ABI calling convention, caller-saved vs. callee-saved registers, register zeroing idioms, polynomial `leaq` arithmetic, bitmask manipulation.
- Textbook: CS:APP3e Chapter 3 (Sections 3.1–3.7).
- Problems: `sum_to_n.asm`, `binary_convert.asm`, `pangram.asm`.

### 4. Further x86-64 Assembly Programming
- Directory: [04-x86-64-assembly-further/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/04-x86-64-assembly-further)
- Core Concepts: Multi-dimensional array row-major memory calculation, stack frame allocation, single-precision floating point with SSE2/AVX `%xmm` vector registers.
- Textbook: CS:APP3e Chapter 3 (Sections 3.8, 3.11).
- Problems: `fib.asm`, `matrix.asm`, `cone.asm`.

### 5. Low-Level Code Optimization
- Directory: [05-low-level-optimization/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/05-low-level-optimization)
- Core Concepts: Superscalar pipelining, instruction-level parallelism (ILP), loop unrolling, multiple accumulators breaking Read-After-Write (RAW) data dependency chains, profiling with `perf stat`.
- Textbook: CS:APP3e Chapter 5.
- Problems: `pagecount.c`, `vec/dotproduct.c`.

### 6. The Memory Hierarchy: Caching and Locality
- Directory: [06-memory-hierarchy-caching/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching)
- Core Concepts: Principle of Locality (spatial vs. temporal), cache lines, Stride-1 vs. Stride-$N$ memory access, loop order permutations ($ijk$, $jik$, $kij$), cache blocking / tiling, Valgrind Cachegrind profiling.
- Textbook: CS:APP3e Chapter 6 (Sections 6.1–6.6).
- Problems: `loop-order.c`, `matrix-multiply.c`.

### 7. The Memory Hierarchy: Data Layout
- Directory: [07-memory-hierarchy-data-layout/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/07-memory-hierarchy-data-layout)
- Core Concepts: Data-Oriented Design, Array of Structs (AoS) vs. Struct of Arrays (SoA), memory fragmentation, pointer chasing vs. contiguous streaming in Go.
- Problems: `metrics/metrics.go` benchmark optimization ($40\times$ speedup).

### 8. Exceptional Control Flow: Systems Programming and Unix Shell
- Directory: [08-exceptional-control-flow/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/08-exceptional-control-flow)
- Core Concepts: Interrupts, traps, system calls (`fork`, `execvp`, `waitpid`), asynchronous signal safety, zombie process reaping (`SIGCHLD`).
- Textbook: CS:APP3e Chapter 8.
- Problem: Implement a fully featured Unix Shell (`shell.c`).

## Toolchain and macOS / Linux Architecture Setup

### macOS Apple Silicon Setup (ARM64 host)
- The assembly exercises target x86-64 architecture (`nasm -f elf64` or `-f macho64`).
- **Option A (Rosetta 2)**:
  Install Rosetta to execute x86-64 Mach-O binaries natively:
  ```bash
  softwareupdate --install-rosetta
  ```
- **Option B (Docker / Colima x86-64 Container - Recommended for Linux Tools)**:
  To run Linux `elf64` binaries, `perf`, and Valgrind Cachegrind on macOS:
  ```bash
  # Start Colima with x86-64 emulation
  colima start --arch x86_64

  # Or run any exercise in a disposable GCC container
  docker run --rm -it -v "$(pwd):/workspace" -w /workspace gcc:latest bash
  ```

## Reference Solutions
- Each exercise folder includes a tucked-away `reference-solution/` subfolder containing completed reference code and writeups.
- Implement your solutions from scratch in the root of each exercise folder first, using tests for feedback, and consult reference solutions only during review.
