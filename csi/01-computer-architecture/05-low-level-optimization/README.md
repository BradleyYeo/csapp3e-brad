# Low-Level Code Optimization and Processor Architecture

## Mental Models and Theoretical Foundations

### Superscalar Out-of-Order Execution
- Modern microprocessors execute instructions using superscalar pipelined execution units capable of retiring multiple instructions per clock cycle.
- Execution bottlenecks:
  - **Latency**: The time required to execute one instruction from inputs to outputs (e.g. 3 cycles for floating-point add, 5 cycles for multiply).
  - **Issue Time / Throughput**: Minimum cycles between consecutive independent operations of the same type (typically 1 cycle when execution units are pipelined).
  - **Read-After-Write (RAW) Dependencies**: A sequential chain where instruction $k+1$ depends on the result of instruction $k$ cannot take advantage of multiple execution units, serializing execution to latency rather than throughput.

### The Optimization Ladder
- **Tier 1: High-Level Algorithmic and Invariant Hoisting**
  - Code motion: Move calculations that yield identical values outside loops (e.g., hoisting `vec_length(u)` outside the loop boundary).
  - Eliminating procedure calls in inner loops: Direct pointer access eliminates prologue/epilogue overhead and enables compiler register allocation.
- **Tier 2: Loop Unrolling**
  - Unrolling loops ($2 \times 1$, $4 \times 1$, $8 \times 1$) reduces branch instruction count, loop counter adjustments, and pipeline stalls.
- **Tier 3: Multiple Accumulators (Breaking Dependency Chains)**
  - Unrolling with a single accumulator (`sum += ...`) still suffers from a strict sequential RAW latency chain.
  - Using multiple parallel accumulators (`sum0`, `sum1`, `sum2`, `sum3`) breaks the dependency chain into independent data streams. The processor can dispatch additions to independent functional units concurrently.
- **Tier 4: Re-association Transformations**
  - Parenthesizing operations `(u[i] * v[i]) + (u[i+1] * v[i+1])` allows both multiplications to execute in parallel before accumulating.

### Profiling with Hardware Performance Counters (`perf`)
- Key CPU metrics:
  - `cycles`: Total processor clock cycles elapsed.
  - `instructions`: Total instructions retired.
  - `IPC (Instructions Per Cycle)`: $\text{IPC} = \frac{\text{instructions}}{\text{cycles}}$. Values $> 1.0$ indicate superscalar execution; values $< 0.5$ indicate pipeline stalls.
  - `branch-misses`: Penalized by 15–20 wasted cycles flushing the speculative pipeline.
  - `page-faults`: Memory management unit traps bringing pages into physical RAM.

## Textbook Reference
- **CS:APP3e**: Chapter 5 (*Optimizing Program Performance*), Sections 5.1–5.14.

## Problem Set and Specifications

### Exercise 1: `pagecount.c` (Division vs. Bitshift)
- Objective: Calculate the number of memory pages of size `pageSize` (e.g. 4096 bytes) needed to store `memorySize` bytes:
  $$\text{pages} = \frac{\text{memorySize} + \text{pageSize} - 1}{\text{pageSize}}$$
- Tasks:
  - Implement naive integer division `/ pageSize`.
  - Implement bitshift `>> 12` and compare execution speed over 10,000,000 iterations.
  - Benchmark across compiler optimization levels (`-O0`, `-O1`, `-O2`, `-O3`).
  - Observe how GCC automatically replaces division by powers of 2 with right shifts under `-O2`.
  - Profile using Linux `perf stat ./a.out`.

### Exercise 2: `vec/dotproduct.c` (Vector Optimization)
- Objective: Optimize computation of the dot product of two vectors of length $N = 10,000$:
  $$\text{dotproduct}(u, v) = \sum_{i=0}^{N-1} u_i \times v_i$$
- Baseline: Located in [vec/dotproduct.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/05-low-level-optimization/vec/dotproduct.c). Uses `get_vec_element()` and calls `vec_length()` every loop iteration.
- Optimization Goals:
  - Step 1: Hoist `vec_length` out of the loop condition.
  - Step 2: Access `data_t *u_data = get_vec_start(u)` directly.
  - Step 3: Unroll loop $4 \times 1$ and $8 \times 1$.
  - Step 4: Implement $4 \times 4$ multiple accumulators (`sum0, sum1, sum2, sum3`).
  - Measure execution time reduction and observe nano-seconds per operation drop by $5\times$ to $10\times$.

## Build and Verification

### Benchmark Vector Dot Product
```bash
cd vec
make clean && make
```

### Benchmark Pagecount
```bash
gcc -O2 pagecount.c -o pagecount && ./pagecount
```

## Active Recall Checkpoint
- Why does a single accumulator prevent a superscalar CPU with 2 addition units from achieving an IPC > 1.0?
- What are the tradeoffs of loop unrolling (e.g. code size, instruction cache pressure)?
- Why cannot compilers automatically re-associate floating-point operations $(a + b) + c \rightarrow a + (b + c)$ under strict IEEE 754 compliance?
