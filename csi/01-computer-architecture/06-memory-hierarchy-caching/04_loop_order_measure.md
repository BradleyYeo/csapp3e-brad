# Concepts: Measuring Cache Behavior on Real Hardware

## Stride-1 vs Stride-4000 in Real Code

- File: [04_loop_order_measure.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c)
- Compares loop orientations over a $4000 \times 4000$ matrix ($16,000,000$ integers, $64\text{ MB}$):
  - `option_one`: Iterates $i$ then $j$ (`x[i][j] = i + j`). Stride-1 access across rows.
  - `option_two`: Iterates $i$ then $j$ (`x[j][i] = i + j`). Stride-4000 down columns.
  - `option_five`: Iterates over `x[4000][8]`. Narrow column traversal where each 64-byte line holds 2 rows.

## The Compiler Optimization Trap (-O1 Dead Code Elimination)

- When compiling with `gcc -O0`:
  - `option_one` runs in $\approx 51\text{ ms}$.
  - `option_two` runs in $\approx 126\text{ ms}$ ($2.5\times$ slower).
- When compiling with `gcc -O1`:
  - Both functions run in $\approx 0.7\text{ ms}$!
  - Inspecting disassembly reveals the compiler proved `static int x[4000][4000]` has no observable side-effects after the function returns. GCC replaced the $16,000,000$ memory stores with an empty decrementation loop (`sub edx, 1; jne .L2`).
- The Solution: Pass dynamically allocated memory via pointer (`option_three` vs `option_four`):
  ```c
  void option_three(int (*x)[4000]) { ... }
  void option_four(int (*x)[4000]) { ... }
  ```
  The compiler cannot prove pointer alias escape and is forced to emit actual memory instructions.

## Hardware Architecture: Intel 64-Byte Line vs Apple Silicon 128-Byte Line

- Query your host cache line size:
  ```bash
  sysctl -n hw.cachelinesize
  ```
- On Intel x86-64:
  - Cache line $B = 64\text{ bytes} = 16\text{ ints}$.
  - `option_one` misses: $16,000,000 / 16 = 1,000,000$ misses.
- On Apple Silicon (M1 Pro):
  - Cache line $B = 128\text{ bytes} = 32\text{ ints}$.
  - `option_one` misses: $16,000,000 / 32 = 500,000$ misses ($2\times$ fewer misses due to wider spatial line!).

---

# Hands-On Measurement Workflow

## Step 1: Predict Before Measuring
- Write down your predicted numbers on paper:
  - Predicted L1 write misses for `option_one` on 64B line:
  - Predicted L1 write misses for `option_two` on 64B line:
  - Predicted L1 write misses for `option_five` on 64B line:

## Step 2: Native Measurement on macOS
- Build and run:
  ```bash
  make 04
  ```

## Step 3: Linux Container Measurement with Valgrind Cachegrind
- If using Colima or Docker:
  ```bash
  colima start --arch x86_64
  docker run -it --rm -v "$(pwd)":/app -w /app gcc:latest bash
  apt-get update && apt-get install -y valgrind linux-tools-generic
  gcc -O0 -g 04_loop_order_measure.c -o loop-order
  valgrind --tool=cachegrind --cache-sim=yes ./loop-order
  cg_annotate cachegrind.out.<pid>
  ```
- Compare your output against recorded benchmarks in [reference-solution/measurements_intel_linux.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/measurements_intel_linux.md).

---

# Active Recall Checkpoints

- Why did `gcc -O1` eliminate the execution time difference between `option_one` and `option_two`?
- How does passing a heap buffer via pointer `int (*x)[4000]` prevent dead code elimination?
- In Cachegrind output for `option_five`, why is `D1 miss rate` $37.5\%$ while `LLd miss rate` is only $5.7\%$? (Hint: The entire $32,000$-int array is $128\text{ KiB}$, which fits inside the $512\text{ KiB}$ L2 and $3\text{ MiB}$ L3 cache!)

---

# Navigation Links

- Next Exercise: [05_lru_cache_sim.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/05_lru_cache_sim.md)
- Recorded Linux Benchmarks: [reference-solution/measurements_intel_linux.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/measurements_intel_linux.md)

