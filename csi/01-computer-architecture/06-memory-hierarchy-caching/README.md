# The Memory Hierarchy: Caching and Locality

## Curriculum Overview
- Module path: [06-memory-hierarchy-caching/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching)
- Textbook reference: CS:APP3e Chapter 6 (Sections 6.1–6.6)
- Prerequisites: [01-bits-and-bytes/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/01-bits-and-bytes) and C bitwise arithmetic
- Next module: [07-memory-hierarchy-data-layout/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/07-memory-hierarchy-data-layout)
- Estimated time: ~22 hours across 2 weeks

## Three-Layer Learning Methodology
- Layer 1 (Theory & Active Recall): Read the theory, study the memory hierarchy and invariants, close the notes, and solve hand-trace calculations on paper before writing any code.
- Layer 2 (Simulation & Software Instrumentation): Implement minimal, test-driven C simulation modules (`01`, `03`, `05`, `06`, `07`, `09`) to model cache behavior without hardware noise. The simulator written in `05` becomes the instrument for subsequent stages.
- Layer 3 (Hardware Measurement & Validation): Run hardware profiling benchmarks (`04`, `08`, `10`) on real hardware (`perf`, Valgrind `cachegrind`, and native timers) to validate analytical predictions against CPU realities (cache lines, hardware prefetchers, and latency plateaus).

---

# Practice Sequence & Roadmap

| Step | Topic | Mode | Target Files | Est. Time |
|:---|:---|:---|:---|:---|
| 00 | Overview & Design Principles | Theory | [00_overview.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/00_overview.md) | 1.0h |
| 01 | Cache Geometry & Address Decoding | Code | [01_address_decoding.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.md), [01_address_decoding.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.c) | 1.5h |
| 02 | Hand-Trace Drills | Paper | [02_hand_trace_drills.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/02_hand_trace_drills.md) | 1.5h |
| 03 | Analytical Miss Prediction | Code | [03_miss_prediction.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.md), [03_miss_prediction.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.c) | 1.5h |
| 04 | Loop Order Hardware Measurement | Measure | [04_loop_order_measure.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.md), [04_loop_order_measure.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c) | 2.0h |
| 05 | LRU Cache Simulation & Write Policies | Code | [05_lru_cache_sim.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/05_lru_cache_sim.md), [05_lru_cache_sim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/05_lru_cache_sim.c) | 3.0h |
| 06 | Conflict Misses & Cache Padding | Code | [06_conflict_misses.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/06_conflict_misses.md), [06_conflict_misses.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/06_conflict_misses.c) | 2.0h |
| 07 | Matrix Multiply Loop Permutations | Code | [07_matmul_loop_orders.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.md), [07_matmul_loop_orders.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.c) | 2.0h |
| 08 | Matrix Multiply Benchmark Ladder | Measure | [08_matmul_benchmark.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark/08_matmul_benchmark.md), [08_matmul_benchmark/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/08_matmul_benchmark) | 3.0h |
| 09 | Cache Tiling (Blocking) | Code | [09_tiling.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.md), [09_tiling.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.c) | 2.0h |
| 10 | Memory Mountain Throughput Sweep | Measure | [10_memory_mountain.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/10_memory_mountain.md), [10_memory_mountain.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/10_memory_mountain.c) | 2.0h |
| 11 | Cumulative Self-Test & Recall | Recall | [11_self_test.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/11_self_test.md) | 1.0h |

---

# Build and Execution Workflow

## Practice Stub Commands
- Build and run a single exercise:
  - `make 01` (Address decoding)
  - `make 03` (Miss rate prediction)
  - `make 04` (Loop order hardware timing)
  - `make 05` (LRU cache simulation)
  - `make 06` (Conflict misses & padding)
  - `make 07` (Matmul loop orders)
  - `make 08` (4-stage matrix multiplication benchmark)
  - `make 09` (Matrix multiplication tiling)
  - `make 10` (Memory mountain throughput sweep)
- Run all practice test suites:
  - `make test` (Compiles all stubs and runs their unit test suites)
- Clean all build binaries and debug artifacts:
  - `make clean`

## Reference Solution Verification
- Run a single reference solution:
  - `make solution-01`
  - `make solution-03`
  - `make solution-05`
  - `make solution-06`
  - `make solution-07`
  - `make solution-09`
  - `make solution-10`
- Run the full reference solution suite:
  - `make test-solution`
- Detailed written solutions:
  - [reference-solution/02_answers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/02_answers.md) (Hand-trace solutions)
  - [reference-solution/11_answers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/11_answers.md) (Self-test recall solutions)
  - [reference-solution/measurements_intel_linux.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/measurements_intel_linux.md) (Original Intel Linux hardware profiling numbers)

## Containerized Linux Measurement (Colima / Docker)
- macOS ARM64 does not support Linux performance counters (`perf`) or Valgrind `cachegrind`.
- Build and run the x86-64 container for hardware-counter measurements:
  ```bash
  docker build -t cache-perf .
  docker run --rm -it -v $(pwd):/workspace -w /workspace cache-perf
  valgrind --tool=cachegrind ./04_loop_order_measure
  perf stat ./04_loop_order_measure
  ```

---

# Architectural Concept Summary

## Shared Simulator Infrastructure
- Core declarations: [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)
- Structs:
  - [`cache_geometry_t`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h#L16-L23): Cache configuration tuple $(S, E, B, m, s, b)$.
  - [`decoded_addr_t`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h#L25-L29): Sliced tag, set index, and block offset.
  - [`cache_line_t`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h#L31-L37): Line state containing valid bit, dirty bit, tag, and LRU timestamp.
  - [`sim_cache_t`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h#L39-L51): Multi-set cache state, write policy flags, and access counters (hits, misses, evictions, writebacks).
- Linking:
  - [05_lru_cache_sim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/05_lru_cache_sim.c) wraps `main()` in `#ifndef NO_MAIN`.
  - Exercises `06`, `07`, and `09` compile and link against `05_lru_cache_sim.c -DNO_MAIN`.

## Key Invariants & Formulas
- Address partitioning:
  - $m = t + s + b$
  - Block offset mask: `(1ULL << b) - 1`
  - Set index mask: `(1ULL << s) - 1`
  - Tag shift: `s + b`
- Stride-$k$ miss rate:
  - Miss rate $= \min(1.0, \frac{k \cdot w}{B})$ where $w$ is word size in bytes and $B$ is cache line size in bytes.
- Critical stride:
  - $\Delta_{\text{crit}} = S \times B$ bytes. Accesses strided by multiples of $\Delta_{\text{crit}}$ map to the identical cache set, inducing conflict thrashing when the number of competing arrays exceeds associativity $E$.
- Matrix multiplication inner-loop miss profile ($B=64$, $w=8$, $\frac{B}{w} = 8$ elements/line):
  - `ijk`: $A[i][k]$ hits (register), $B[k][j]$ misses $0.125$, $C[i][j]$ misses $1.0$ (writes) $\to 1.125$ misses/iter.
  - `jki`: $A[i][k]$ misses $1.0$, $B[k][j]$ hits (register), $C[i][j]$ misses $1.0$ $\to 2.0$ misses/iter.
  - `ikj`: $A[i][k]$ hits (register), $B[k][j]$ misses $0.125$, $C[i][j]$ misses $0.125$ $\to 0.25$ misses/iter.
- Cache tiling working set bound:
  - $3 \times T^2 \times w \le C$ (where $C$ is cache capacity in bytes).
