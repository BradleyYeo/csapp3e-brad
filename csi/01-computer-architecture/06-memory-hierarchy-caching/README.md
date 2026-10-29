# The Memory Hierarchy: Caching and Locality

## Mental Models and Theoretical Foundations

### The Principle of Locality
- Programs with good locality access the same set of memory locations repeatedly (temporal locality) or access memory locations near recently accessed addresses (spatial locality).
- Hardware caches exploit locality to provide the illusion of a massive memory system with the speed of static RAM (SRAM).

### Hardware Cache Organization
- A cache is characterized by a tuple $(S, E, B, m)$:
  - $m$: Number of physical address bits.
  - $S = 2^s$: Number of cache sets.
  - $E$: Number of cache lines per set (associativity: $E=1$ is direct-mapped, $E>1$ is set-associative).
  - $B = 2^b$: Cache line / block size in bytes (typically 64 bytes in modern x86 and ARM processors).
- Address division:
  $$\text{Address} = [\text{Tag } (t \text{ bits}) \mid \text{Set Index } (s \text{ bits}) \mid \text{Block Offset } (b \text{ bits})]$$
- When a 4-byte `int` is accessed, an entire 64-byte cache line (holding 16 integers) is loaded from RAM into L1 cache. Subsequent sequential accesses to the next 15 integers will be instantaneous cache hits (Stride-1 access).

### Stride-1 vs. Stride-N Memory Access Patterns
- In row-major languages (C, C++, Go):
  - Iterating across row columns $A[i][0], A[i][1], \dots, A[i][j]$ yields Stride-1 access: 1 miss every 16 elements (miss rate $\approx 6.25\%$).
  - Iterating down column rows $A[0][j], A[1][j], \dots, A[i][j]$ yields Stride-$N$ access: every element access loads a distinct 64-byte line, evicting prior lines (miss rate $\approx 100\%$).

## Textbook Reference
- **CS:APP3e**: Chapter 6 (*The Memory Hierarchy*), Sections 6.1–6.6.

## Problem Set and Specifications

### Exercise 1: `loop-order.c` (Row-Major vs. Column-Major Access)
- File: [loop-order.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching/loop-order.c)
- Compares two loop orientations over a $4000 \times 4000$ 2D array of integers:
  - `option_one`: Iterates $i$ then $j$ ($x[i][j] = i + j$). Stride-1 access.
  - `option_two`: Iterates $i$ then $j$ ($x[j][i] = i + j$). Stride-4000 access.
- Profiling Task:
  - Measure execution time difference with `perf stat`:
    ```bash
    gcc -O0 loop-order.c -o loop-order && ./loop-order
    ```
  - Analyze cache line misses using Valgrind Cachegrind:
    ```bash
    valgrind --tool=cachegrind --cache-sim=yes ./loop-order
    cg_annotate cachegrind.out.<pid>
    ```
  - Verify that `option_one` generates $\approx 1,000,000$ write misses ($\frac{16,000,000}{16}$) while `option_two` generates $\approx 16,000,000$ write misses.

### Exercise 2: `matrix-multiply.c` (Matrix Multiplication Caching)
- Objective: Compute matrix multiplication $C = A \times B$ for $N \times N$ matrices ($N = 1000$ doubles).
- Baseline Naive: $i-j-k$ loop order in [matrix-multiply.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching/matrix-multiply.c).
- Cache Analysis:
  - $A[i][k]$ has Stride-1 access.
  - $B[k][j]$ has Stride-$N$ access (column stride, causing constant cache thrashing).
- Optimization Strategies to implement in `fast_matrix_multiply`:
  1. **Loop Permutation (i-k-j)**: Swap inner loops $j$ and $k$. The inner loop iterates over $j$, making accesses to both $B[k][j]$ and $C[i][j]$ sequential Stride-1!
  2. **Matrix Transposition**: Transpose matrix $B$ before multiplication so that both arrays are scanned along rows.
  3. **Cache Tiling / Blocking**: Partition matrices into $B \times B$ sub-matrices (e.g. $B = 32$) that fit comfortably in L1 cache ($32 \text{ KiB}$).

## Build and Verification
- Compile and run benchmark:
  ```bash
  gcc -Wall -O2 matrix-multiply.c benchmark.c -o matmul && ./matmul 1000
  ```
- Compare runtime of Naive vs. Fast implementation. Target speedup: $> 1.5\times$ to $2\times$.

## Active Recall Checkpoint
- Given 64-byte cache lines and 8-byte `double` elements, what is the theoretical minimum miss rate for Stride-1 array traversal?
- Why does the $i-k-j$ loop order significantly outperform $i-j-k$ in matrix multiplication even though both perform the exact same $N^3$ arithmetic operations?
- What causes conflict misses in direct-mapped caches when traversing arrays with power-of-two dimensions?
