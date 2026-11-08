# Concepts: Matrix Multiplication Hardware Optimization

## The 4-Stage Optimization Ladder

- Benchmark Directory: [08_matmul_benchmark/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark)
- Source Files:
  - [matrix-multiply.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark/matrix-multiply.c)
  - [benchmark.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark/benchmark.c)
  - [matrix-multiply.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark/matrix-multiply.h)

### Step 1: Loop Reordering (`ikj` Row-SAXPY)
- Swap loops $j$ and $k$ in `fast_matrix_multiply`.
- Requires zero extra memory and zero transpositions.
- Converts column traversal of $B$ into row traversal (Stride-1), immediately yielding $> 1.5\times$ speedup on $N = 1000$.

### Step 2: Matrix Transposition
- Explicitly transpose matrix $B$ into temporary buffer $B^T$:
  $$B^T[j][k] = B[k][j]$$
- The dot product between row $i$ of $A$ and row $j$ of $B^T$ reads both buffers contiguously.

### Step 3: Cache Blocking / Tiling
- Divide the large $N \times N$ matrices into $T \times T$ sub-matrices (e.g. $T = 32$) that fit comfortably in L1 cache ($32\text{ KiB}$).
- Prevents matrix $B$ from being evicted from L1/L2 cache when the outer loop advances to row $i+1$.

### Step 4: Tiling + Multiple Accumulators (Synthesis with Chapter 5)
- Unroll the inner loop $4 \times 1$ and maintain 4 independent accumulator variables:
  `acc1, acc2, acc3, acc4`
- Breaks the sequential floating-point addition latency chain (3 clock cycles per add) into independent instruction streams, allowing the superscalar CPU to execute multiple additions per clock cycle while streaming from L1 cache!
- Target speedup: $> 3\times$ over naive baseline.

---

# Verification and Floating-Point Invariants

## The IEEE 754 Non-Associativity Trap

- In mathematics, real number addition is associative: $(a + b) + c = a + (b + c)$.
- In computer hardware (IEEE 754 floating point), intermediate rounding occurs after every operation. Changing the summation order (as done by multiple accumulators or loop reordering) changes the least significant bits:
  $$(a + b) + c \neq a + (b + c)$$
- Comparing matrices with exact inequality `A[i][j] != B[i][j]` will falsely report failure!
- Invariant: Always compare floating-point matrices using an epsilon tolerance:
  ```c
  if (fabs(A[i][j] - B[i][j]) > 1e-9) return false;
  ```

---

# Build and Benchmark Commands

```bash
cd 08_matmul_benchmark
gcc -Wall -O2 matrix-multiply.c benchmark.c -o matmul -lm
./matmul 1000
```

---

# Active Recall Checkpoints

- Why does combining cache blocking with multiple accumulators produce dramatically higher throughput than multiple accumulators alone on an untiled matrix?
- Why is floating-point addition not associative in IEEE 754?
- What is the difference between latency bound and throughput bound in superscalar execution?

---

# Navigation Links

- Next Exercise: [09_tiling.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.md)
- Reference Implementation: [reference-solution/08_matrix-multiply.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/08_matrix-multiply.c)

