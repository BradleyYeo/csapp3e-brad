/*
 * Exercise 07: Matrix Multiplication Loop Permutations
 *
 * Guide: 07_matmul_loop_orders.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 07
 */
#include "cache_sim.h"

/*
 * Drill 1: ijk Dot-Product Loop Permutation
 *
 * Requirements:
 * - for i = 0..n-1:
 *     for j = 0..n-1:
 *       sum = 0.0
 *       for k = 0..n-1:
 *         Record MEM_LOAD A[i][k], MEM_LOAD B[k][j] in cache
 *         sum += A[i][k] * B[k][j]
 *       C[i][j] = sum
 */
void sim_matmul_ijk(sim_cache_t *cache, double *C, const double *A,
                    const double *B, size_t n) {
  // TODO: Implement ijk loop order with simulated cache accesses
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

/*
 * Drill 2: jki Column-Update Loop Permutation
 *
 * Requirements:
 * - Zero array C first via memset.
 * - for j = 0..n-1:
 *     for k = 0..n-1:
 *       r = B[k][j]
 *       for i = 0..n-1:
 *         Record MEM_LOAD A[i][k], MEM_LOAD C[i][j], MEM_STORE C[i][j] in cache
 *         C[i][j] += A[i][k] * r
 */
void sim_matmul_jki(sim_cache_t *cache, double *C, const double *A,
                    const double *B, size_t n) {
  // TODO: Implement jki loop order with simulated cache accesses
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

/*
 * Drill 3: ikj Row-SAXPY Loop Permutation
 *
 * Requirements:
 * - Zero array C first via memset.
 * - for i = 0..n-1:
 *     for k = 0..n-1:
 *       r = A[i][k]
 *       for j = 0..n-1:
 *         Record MEM_LOAD B[k][j], MEM_LOAD C[i][j], MEM_STORE C[i][j] in cache
 *         C[i][j] += r * B[k][j]
 */
void sim_matmul_ikj(sim_cache_t *cache, double *C, const double *A,
                    const double *B, size_t n) {
  // TODO: Implement ikj loop order with simulated cache accesses
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
}

int main(void) {
  printf("Running Exercise 07: Matmul Loop Permutations...\n\n");

  const size_t n = 16;
  double *A = malloc(n * n * sizeof(double));
  double *B = malloc(n * n * sizeof(double));
  double *C_ijk = calloc(n * n, sizeof(double));
  double *C_jki = calloc(n * n, sizeof(double));
  double *C_ikj = calloc(n * n, sizeof(double));

  for (size_t i = 0; i < n * n; ++i) {
    A[i] = (double)(i % 13) * 0.5;
    B[i] = (double)((i * 7) % 11) * 0.25;
  }

  /* Simulated L1 cache: s=3 (8 sets), E=2 (2-way), b=5 (32B line = 4 doubles) -> 512B */
  sim_cache_t cache;
  sim_cache_init(&cache, 3, 2, 5, WRITE_BACK_ALLOCATE);

  sim_cache_reset_stats(&cache);
  sim_matmul_ijk(&cache, C_ijk, A, B, n);
  uint64_t misses_ijk = cache.misses;

  sim_cache_reset_stats(&cache);
  sim_matmul_jki(&cache, C_jki, A, B, n);
  uint64_t misses_jki = cache.misses;

  sim_cache_reset_stats(&cache);
  sim_matmul_ikj(&cache, C_ikj, A, B, n);
  uint64_t misses_ikj = cache.misses;

  for (size_t i = 0; i < n * n; ++i) {
    assert(fabs(C_ijk[i] - C_jki[i]) < 1e-9);
    assert(fabs(C_ijk[i] - C_ikj[i]) < 1e-9);
  }

  printf("  [N=16 Misses] jki: %llu | ijk: %llu | ikj: %llu\n",
         (unsigned long long)misses_jki, (unsigned long long)misses_ijk,
         (unsigned long long)misses_ikj);
  assert(misses_ikj > 0);
  assert(misses_ikj * 4 < misses_ijk);
  assert(misses_ijk < misses_jki);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ijk);
  free(C_jki);
  free(C_ikj);

  printf("Exercise 07 PASSED!\n");
  return EXIT_SUCCESS;
}
