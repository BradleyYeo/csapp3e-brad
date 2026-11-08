/*
 * Exercise 09: 2D Cache Blocking (Tiling) & Working Set Fitting
 *
 * Guide: 09_tiling.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 09
 */
#include "cache_sim.h"

/*
 * Drill 1: Compute Largest Power-of-Two Tile Size
 *
 * Precondition:
 *   l1_capacity_bytes >= 3 * elem_size, elem_size >= 1.
 * Postcondition:
 *   Returns largest power-of-two T (1, 2, 4, 8, 16, 32...) such that:
 *     3 * (T * T) * elem_size <= l1_capacity_bytes
 */
size_t compute_max_tile_size(size_t l1_capacity_bytes, size_t elem_size) {
  // TODO: Compute largest power-of-two tile size
  (void)l1_capacity_bytes;
  (void)elem_size;
  return 0;
}

/*
 * Drill 2: Tiled Matrix Multiplication Simulation
 *
 * Precondition:
 *   n is a multiple of tile_size, tile_size >= 1.
 * Postcondition:
 *   Computes C = A * B using 2D cache blocking with ikj inner order.
 *   Records memory accesses in cache:
 *     - Outer tile loops: ti, tj, tk by tile_size
 *     - Local tile loops: i from ti, k from tk
 *         Record MEM_LOAD A[i][k]
 *         for j from tj:
 *           Record MEM_LOAD B[k][j], MEM_LOAD C[i][j], MEM_STORE C[i][j]
 *           C[i][j] += A[i][k] * B[k][j]
 */
void sim_matmul_tiled(sim_cache_t *cache, double *C, const double *A,
                      const double *B, size_t n, size_t tile_size) {
  // TODO: Implement tiled matrix multiplication with simulated cache access tracking
  (void)cache;
  (void)C;
  (void)A;
  (void)B;
  (void)n;
  (void)tile_size;
}

/* Untiled reference helper */
static void sim_matmul_ikj_ref(sim_cache_t *cache, double *C, const double *A,
                               const double *B, size_t n) {
  memset(C, 0, n * n * sizeof(double));
  for (size_t i = 0; i < n; ++i) {
    for (size_t k = 0; k < n; ++k) {
      double r = A[i * n + k];
      for (size_t j = 0; j < n; ++j) {
        sim_cache_access(cache, mat_addr(SIM_BASE_B, n, k, j), MEM_LOAD);
        sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_LOAD);
        sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_STORE);
        C[i * n + j] += r * B[k * n + j];
      }
    }
  }
}

int main(void) {
  printf("Running Exercise 09: 2D Cache Blocking (Tiling)...\n\n");

  /* Test 1: Tile sizing */
  assert(compute_max_tile_size(32768, 8) == 32); /* 3 * 32^2 * 8 = 24,576 <= 32 KiB */
  assert(compute_max_tile_size(1024, 8) == 4);   /* 3 * 4^2 * 8 = 384 <= 1024 */

  /* Test 2: Tiling miss reduction on 1 KiB cache */
  const size_t n = 16;
  double *A = malloc(n * n * sizeof(double));
  double *B = malloc(n * n * sizeof(double));
  double *C_ikj = calloc(n * n, sizeof(double));
  double *C_tiled = calloc(n * n, sizeof(double));

  for (size_t i = 0; i < n * n; ++i) {
    A[i] = (double)((i + 3) % 17) * 0.5;
    B[i] = (double)((i * 5 + 1) % 19) * 0.25;
  }

  sim_cache_t cache;
  sim_cache_init(&cache, 3, 4, 5, WRITE_BACK_ALLOCATE);

  sim_cache_reset_stats(&cache);
  sim_matmul_ikj_ref(&cache, C_ikj, A, B, n);
  uint64_t misses_untiled = cache.misses;

  size_t T = compute_max_tile_size(cache.geom.capacity_bytes, sizeof(double));
  assert(T == 4);

  sim_cache_reset_stats(&cache);
  sim_matmul_tiled(&cache, C_tiled, A, B, n, T);
  uint64_t misses_tiled = cache.misses;

  for (size_t i = 0; i < n * n; ++i) {
    assert(fabs(C_ikj[i] - C_tiled[i]) < 1e-9);
  }

  printf("  [N=16 on 1KiB Cache] Untiled ikj misses: %llu | Tiled (T=4) misses: %llu\n",
         (unsigned long long)misses_untiled, (unsigned long long)misses_tiled);
  assert(misses_tiled > 0 && misses_tiled < misses_untiled);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ikj);
  free(C_tiled);

  printf("Exercise 09 PASSED!\n");
  return EXIT_SUCCESS;
}

