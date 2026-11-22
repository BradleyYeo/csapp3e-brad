/*
 * Exercise 18 (K&R 3-1): Binary Search, Comparison Counting & Performance
 * Optimization
 *
 * Concepts covered:
 * - Overflow-safe midpoint calculation (`low + (high - low) / 2`)
 * - Heap allocation (`malloc`/`free`) for datasets that exceed stack & L2/L3
 * cache
 * - Defeating dead-code elimination (`volatile` sink) and branch-predictor bias
 * (PRNG queries)
 * - Drill 1 (Baseline): Classic 2-comparison binary search
 * - Drill 2 (K&R 3-1): Single-comparison loop + comparison counter
 * - Drill 3 (AMH / Systems Perf): Branchless binary search (`cmov` / `csel`)
 *
 * Build & benchmark in WSL:
 *   make 18_kr301_binsearch_count && ./18_kr301_binsearch_count
 *   clang -std=c23 -O2 -Wall -Wextra -o 18_opt 18_kr301_binsearch_count.c &&
 * ./18_opt
 */
#include <stddef.h>
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
  BENCH_N = 1 << 22, /* 4,194,304 ints (~16 MiB: spills out of L1/L2 cache) */
  BENCH_QUERIES = 500000, /* 500k random lookups to defeat branch prediction */
  TODO_SENTINEL = -2      /* Returned by unimplemented optimization drills */
};

/*
 * Drill 1 (Baseline — Implemented): Classic Two-Test Binary Search
 *
 * C Tips:
 * - `const int a[]` promises read-only access and lets the compiler optimize
 * loads.
 * - `low + (high - low) / 2` avoids signed integer overflow UB when `n >
 * INT_MAX / 2`.
 * - `static` gives internal linkage, allowing `-O2` to inline cleanly.
 */
static int binarySearch(int x, int n, const int a[]) {
  int low = 0;
  int high = n - 1;
  while (low <= high) {
    size_t mid = low + (high - low) / 2;
    if (a[mid] == x) {
      return mid;
    } else if (a[mid] < x) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  return -1;
}

/*
 * Drill 2 (K&R 3-1): One-Test-Inside-Loop Binary Search with Comparison Counter
 *
 * Optimization concept:
 * - The baseline tests `a[mid] == x` AND `a[mid] < x` on every iteration (2
 * branches).
 * - Narrow the half-open/closed interval using ONLY ONE comparison inside the
 * loop: while (low < high): mid = low + (high - low) / 2 increment
 * `*comparisons_out` (if non-null) if (x <= a[mid]) high = mid; else low = mid
 * + 1;
 * - After the loop, check once outside: if `n > 0 && a[low] == x`, return
 * `low`, else `-1`.
 *
 * Requirements:
 * - Return -1 immediately if `n <= 0 || a == nullptr` (setting
 * `*comparisons_out = 0` if non-null).
 * - Count every element comparison (`x <= a[mid]` inside loop + final equality
 * check) in `*comparisons_out`.
 * - Return the index of `x` (lowest index if duplicates exist), or `-1` if not
 * found.
 */
static int binarySearch_one_test(int x, int n, const int a[],
                                 size_t *comparisons_out) {
  size_t cmps = 0;
  if (comparisons_out != nullptr) {
    *comparisons_out = 0;
  }
  if (n <= 0 || a == nullptr) {
    return -1;
  }
  size_t low = 0;
  size_t high = (size_t)n - 1;
  while (low < high) {
    size_t mid = low + (high - low) / 2;
    cmps++;
    if (x <= a[mid]) {
      high = mid;
    } else {
      low = mid + 1;
    }
  }
  cmps++;
  if (a[low] == x) {
    return (int)low;
  }
  if (comparisons_out != nullptr) {
    *comparisons_out = cmps;
  }
  return -1;
}

/*
 * Drill 3 (Performance Optimization — Branchless Search / CMOV):
 *
 * Optimization concept (Slotin AMH / CS:APP Ch. 5):
 * - On random queries, each binary search comparison is a 50/50 coin flip.
 * - Modern CPUs suffer a ~15-20 cycle pipeline flush on every branch
 * misprediction.
 * - By maintaining a `const int *base = a` and `size_t len = (size_t)n` and
 * updating: size_t half = len / 2; base += (base[half] <= x) ? half : 0; len -=
 * half; `clang -O2` emits a branchless conditional move (`cmov` on x86-64 /
 * `csel` on ARM64).
 *
 * Requirements:
 * - Return -1 if `n <= 0 || a == nullptr`.
 * - Loop while `len > 1`, halving `len` and conditionally advancing `base`.
 * - After the loop, return `(int)(base - a)` if `*base == x`, else `-1`.
 * - Verify emitted assembly with: `clang -std=c23 -O2 -S
 * 18_kr301_binsearch_count.c -o -`
 */
static int binarySearch_branchless(int x, int n, const int a[]) {
  (void)x;
  (void)n;
  (void)a;
  return TODO_SENTINEL; /* TODO: implement branchless cmov search */
}

/* ── Benchmark & Verification Infrastructure ─────────────────────────────── */

static uint32_t xorshift32(uint32_t *state) {
  uint32_t s = *state;
  s ^= s << 13;
  s ^= s >> 17;
  s ^= s << 5;
  *state = s;
  return s;
}

static double elapsed_ms(struct timespec start, struct timespec end) {
  double sec = (double)(end.tv_sec - start.tv_sec);
  double nsec = (double)(end.tv_nsec - start.tv_nsec);
  return (sec * 1e3) + (nsec / 1e6);
}

static void verify_basic_cases(int (*fn)(int, int, const int *),
                               const char *name) {
  const int a[5] = {1, 2, 3, 4, 5};
  const int evens[4] = {2, 4, 6, 8};
  const int single[1] = {42};

  assert(fn(4, 5, a) == 3); /* catches returning 0 instead of mid */
  assert(fn(1, 5, a) == 0); /* first element */
  assert(fn(5, 5, a) ==
         4); /* last element: catches low < high & low = mid hang */
  assert(fn(0, 5, a) == -1);      /* missing below min */
  assert(fn(6, 5, a) == -1);      /* missing above max */
  assert(fn(5, 4, evens) == -1);  /* missing in middle */
  assert(fn(42, 1, single) == 0); /* single-element hit: catches low < high */
  assert(fn(7, 1, single) == -1); /* single-element miss */
  assert(fn(4, 0, a) == -1);      /* empty array */

  printf("[PASS] %s: core & boundary assertions passed.\n", name);
}

static int one_test_adapter(int x, int n, const int a[]) {
  size_t cmp = 0;
  return binarySearch_one_test(x, n, a, &cmp);
}

int main(void) {
  /* 1. Core & boundary unit tests on baseline */
  verify_basic_cases(binarySearch, "binarySearch (baseline)");

  /* 2. Allocate large sorted dataset on the HEAP (16 MiB would overflow stack!)
   */
  int *big = malloc((size_t)BENCH_N * sizeof(int));
  int *queries = malloc((size_t)BENCH_QUERIES * sizeof(int));
  assert(big != nullptr && queries != nullptr);

  /* Store even numbers [0, 2, 4, ..., 2*(N-1)] so even queries hit and odd
   * queries miss */
  for (int i = 0; i < BENCH_N; i++) {
    big[i] = i * 2;
  }

  /* Pre-generate unpredictable random queries so the branch predictor cannot
   * cheat */
  uint32_t rng = 0xDEADBEEFU;
  for (int i = 0; i < BENCH_QUERIES; i++) {
    queries[i] = (int)(xorshift32(&rng) % (uint32_t)(BENCH_N * 2));
  }

  /* Verify baseline on large dataset spot checks */
  assert(binarySearch(0, BENCH_N, big) == 0);
  assert(binarySearch((BENCH_N - 1) * 2, BENCH_N, big) == BENCH_N - 1);
  assert(binarySearch(123456 * 2, BENCH_N, big) == 123456);
  assert(binarySearch(123456 * 2 + 1, BENCH_N, big) == -1);

  /* 3. Benchmark Baseline */
  struct timespec t0 = {0}, t1 = {0};
  int64_t baseline_checksum = 0;
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (int i = 0; i < BENCH_QUERIES; i++) {
    baseline_checksum += binarySearch(queries[i], BENCH_N, big);
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double base_ms = elapsed_ms(t0, t1);

  /* volatile sink prevents -O2 from deleting the timed benchmark loop */
  volatile int64_t sink = baseline_checksum;
  (void)sink;

  printf("\nBenchmark: N = %d ints (%.1f MiB), %d random queries\n", BENCH_N,
         (double)((size_t)BENCH_N * sizeof(int)) / (1024.0 * 1024.0),
         BENCH_QUERIES);
  printf("  baseline (2 tests/iter): %8.2f ms  (%5.1f ns/query)\n", base_ms,
         (base_ms * 1e6) / (double)BENCH_QUERIES);

  /* 4. Drill 2: K&R 3-1 Single-Test Loop (auto-verified once implemented) */
  size_t sample_cmps = 0;
  if (binarySearch_one_test(4, 5, (const int[]){1, 2, 3, 4, 5}, &sample_cmps) !=
      TODO_SENTINEL) {
    verify_basic_cases(one_test_adapter, "binarySearch_one_test (K&R 3-1)");
    assert(sample_cmps > 0 && sample_cmps <= 4);

    int64_t one_checksum = 0;
    size_t total_cmps = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < BENCH_QUERIES; i++) {
      size_t cmps = 0;
      one_checksum += binarySearch_one_test(queries[i], BENCH_N, big, &cmps);
      total_cmps += cmps;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double one_ms = elapsed_ms(t0, t1);
    assert(one_checksum == baseline_checksum);
    printf("  one_test (K&R 3-1):      %8.2f ms  (%5.1f ns/query, %.1f "
           "cmps/query, %.2fx)\n",
           one_ms, (one_ms * 1e6) / (double)BENCH_QUERIES,
           (double)total_cmps / (double)BENCH_QUERIES, base_ms / one_ms);
  } else {
    printf("  one_test (K&R 3-1):      [TODO] implement binarySearch_one_test "
           "to benchmark\n");
  }

  /* 5. Drill 3: Branchless CMOV Search (auto-verified once implemented) */
  if (binarySearch_branchless(4, 5, (const int[]){1, 2, 3, 4, 5}) !=
      TODO_SENTINEL) {
    verify_basic_cases(binarySearch_branchless,
                       "binarySearch_branchless (CMOV)");

    int64_t bl_checksum = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < BENCH_QUERIES; i++) {
      bl_checksum += binarySearch_branchless(queries[i], BENCH_N, big);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double bl_ms = elapsed_ms(t0, t1);
    assert(bl_checksum == baseline_checksum);
    printf("  branchless (CMOV):       %8.2f ms  (%5.1f ns/query, %.2fx)\n",
           bl_ms, (bl_ms * 1e6) / (double)BENCH_QUERIES, base_ms / bl_ms);
  } else {
    printf("  branchless (CMOV):       [TODO] implement "
           "binarySearch_branchless to benchmark\n");
  }

  free(queries);
  free(big);
  return EXIT_SUCCESS;
}
