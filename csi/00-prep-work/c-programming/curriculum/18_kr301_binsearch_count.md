# Exercise 18 (K&R 3-1): Binary Search C Tips and Performance Optimization

Connects K&R 3-1 to CS:APP Chapter 5 (Code Optimization), Chapter 6 (Memory Hierarchy), and Sergey Slotin's *Algorithms for Modern Hardware*.

---

# C Safety and Correctness Tips

## Prevent Midpoint Overflow
- `(low + high) / 2` triggers signed integer overflow undefined behavior when `low + high > INT_MAX`.
- Write `low + (high - low) / 2` instead (or use `size_t` indices so the CPU avoids 32-bit sign-extension instructions on 64-bit platforms).

## Heap-Allocate Large Datasets
- A `4,194,304`-element `int` array occupies `16 MiB`, which overflows the default `8 MiB` process stack and crashes with `SIGSEGV`.
- Allocate large arrays with `malloc`, verify non-null, and always `free` them before returning.

## Use `const`, `static`, and `int main(void)`
- Mark read-only array parameters `const int a[]` so the compiler knows loads cannot be aliased by writes through `a`.
- Mark file-local helpers `static` to allow aggressive inlining under `-O2`.

---

# Benchmarking Discipline

## Why Random Queries Matter
- Searching `0, 1, 2, ...` in order trains the CPU branch predictor and keeps the same cache lines hot, hiding real latency.
- Pre-generating `500,000` pseudo-random queries (`xorshift32`) makes every step a 50/50 coin flip and forces cold cache lookups across `16 MiB`.

## Why `volatile` Sink and `CLOCK_MONOTONIC` Matter
- At `-O2`, the compiler deletes loops whose return values are never used (dead-code elimination). Accumulating a checksum into a `volatile` sink forces every lookup to run.
- `clock_gettime(CLOCK_MONOTONIC, ...)` provides nanosecond wall-clock timing without the coarse resolution of `clock()`.

---

# Performance Optimization Ladder

```text
Level 1: Baseline (2 tests/iter)  --> 2 branches per step, frequent mispredictions
Level 2: K&R 3-1 (1 test/iter)    --> 1 branch per step, defers equality check to end
Level 3: Branchless (CMOV / CSEL) --> 0 unpredictable branches, ~15-20 cycles saved per miss
Level 4: Cache & Prefetching      --> __builtin_prefetch / Eytzinger layout hides DRAM latency
```

## Drill 2: Single Comparison Inside the Loop (K&R 3-1)
- Checking `a[mid] == x` at every step rarely succeeds early (half of all elements sit at the leaf level of the search tree).
- Narrowing with `if (x <= a[mid]) high = mid; else low = mid + 1;` halves the branches per iteration and checks equality once after the loop exits.

## Drill 3: Branchless Binary Search (`cmov` / `csel`)
- On unpredictable 50/50 comparisons, a branch misprediction flushes the CPU pipeline (~15–20 cycles lost per step).
- Computing `base += (base[half] <= x) ? half : 0;` has no control-flow jump; at `-O2`, `clang` emits a conditional move (`cmov` on x86-64, `csel` on ARM64).
- Always benchmark at `-O2` (never `-O0`) and inspect the assembly:
  `clang -std=c23 -O2 -S 18_kr301_binsearch_count.c -o - | grep -E "cmov|csel"`

## Beyond Drill 3: Memory Latency Dominance
- Once you eliminate branch mispredictions on a `16 MiB` dataset, L3/DRAM cache misses (~40–200 cycles) become the main bottleneck.
- **Software prefetching:** Issue `__builtin_prefetch(&base[half / 2])` and `__builtin_prefetch(&base[half + half / 2])` before evaluating `base[half] <= x` to overlap the next level's memory fetch.
- **Eytzinger (BFS) layout:** Packing the top levels of the search tree into contiguous cache lines keeps the first ~10 steps inside L1/L2 cache.

---

# Active Recall Questions
- Why does `int big[1 << 22];` inside `main` crash immediately with `SIGSEGV`, while `malloc` succeeds?
- Why does `binarySearch_branchless` show a dramatic speedup at `-O2` on random queries, yet can look slower at `-O0`?
- On a `16 MiB` array (`2^22` elements), why does the first equality check `a[mid] == x` almost never save time?
