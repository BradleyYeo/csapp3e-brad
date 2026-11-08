# Concepts: Matrix Multiplication Loop Permutations

## The 6 Loop Orders (CS:APP Section 6.5.3)

Computing matrix product $C = A \times B$ requires three nested loops: $i$, $j$, and $k$. The order of these loops determines which matrices are traversed along rows (Stride-1, spatial locality), which along columns (Stride-$N$, cache thrashing), and which remain constant across the innermost loop (held in a CPU register).

```
Matrix dimensions: A is N x N, B is N x N, C is N x N (sizeof(double) = 8 bytes, line B = 64 bytes).
Elements per cache line: L = 64 / 8 = 8 doubles.
```

### Miss Rate Breakdown Per Inner Iteration

| Class | Loop Orders | Inner Index | $A$ Access | $B$ Access | $C$ Access | Misses / Inner Iter |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| Class 1 | `ijk`, `jik` | $k$ (Dot Product) | Stride-1 ($0.125$) | Stride-$N$ ($1.000$) | Register ($0.0$) | **$1.125$** |
| Class 2 | `jki`, `kji` | $i$ (Column Scale) | Stride-$N$ ($1.000$) | Register ($0.0$) | Stride-$N$ ($1.000$) | **$2.000$** |
| Class 3 | `kij`, `ikj` | $j$ (Row-SAXPY) | Register ($0.0$) | Stride-1 ($0.125$) | Stride-1 ($0.125$) | **$0.250$** |

### Why `ikj` Outperforms `ijk` by $4\times\text{--}5\times$
- In `ijk`, the inner loop iterates over $k$: `sum += A[i][k] * B[k][j]`. Scanning down column $j$ of $B$ misses on every step.
- In `ikj`, the inner loop iterates over $j$: `C[i][j] += r * B[k][j]`. Both $B[k][j]$ and $C[i][j]$ are accessed along contiguous rows (Stride-1). A single cache line load satisfies 8 consecutive iterations for both $B$ and $C$!

---

# Task: Simulate the Three Representative Loop Orders

- Target File: [07_matmul_loop_orders.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.c)
- Shared Types: Defined in [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)
- Simulator Dependency: Links against `05_lru_cache_sim.c` for `sim_cache_access`.

## Functions to Complete

- [`sim_matmul_ijk`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.c): Simulates Class 1 dot product. Record `MEM_LOAD A[i][k]` and `MEM_LOAD B[k][j]` in the inner loop.
- [`sim_matmul_jki`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.c): Simulates Class 2 column scaling. Record `MEM_LOAD A[i][k]`, `MEM_LOAD C[i][j]`, and `MEM_STORE C[i][j]` in the inner loop.
- [`sim_matmul_ikj`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.c): Simulates Class 3 row-SAXPY. Record `MEM_LOAD B[k][j]`, `MEM_LOAD C[i][j]`, and `MEM_STORE C[i][j]` in the inner loop.

---

# Predict Before Running

- Run on $N = 16$ with small cache ($s=3, E=2, b=5$):
  - Predict miss ordering: `misses_ikj < misses_ijk < misses_jki`
  - Expected `ikj` misses should be $\approx 4\times$ lower than `ijk`.

---

# Active Recall Checkpoints

- Why does swapping loop indices $j$ and $k$ convert column memory strides into row memory strides?
- In `ikj`, why is `A[i][k]` accessed with $0.0$ misses per inner iteration?
- Does changing loop order alter the final computed numerical result of matrix multiplication?

---

# Navigation Links

- Next Exercise: [08_matmul_benchmark/08_matmul_benchmark.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark/08_matmul_benchmark.md)
- Reference Solution: [reference-solution/07_matmul_loop_orders.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/07_matmul_loop_orders.c)

