/*
 * Reference Solution: Exercise 07 (Matrix Multiplication Loop Permutations)
 */
#include "../cache_sim.h"

void sim_matmul_ijk(sim_cache_t *cache, double *C, const double *A,
                    const double *B, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      double sum = 0.0;
      for (size_t k = 0; k < n; ++k) {
        sim_cache_access(cache, mat_addr(SIM_BASE_A, n, i, k), MEM_LOAD);
        sim_cache_access(cache, mat_addr(SIM_BASE_B, n, k, j), MEM_LOAD);
        sum += A[i * n + k] * B[k * n + j];
      }
      C[i * n + j] = sum;
    }
  }
}

void sim_matmul_jki(sim_cache_t *cache, double *C, const double *A,
                    const double *B, size_t n) {
  memset(C, 0, n * n * sizeof(double));
  for (size_t j = 0; j < n; ++j) {
    for (size_t k = 0; k < n; ++k) {
      double r = B[k * n + j];
      for (size_t i = 0; i < n; ++i) {
        sim_cache_access(cache, mat_addr(SIM_BASE_A, n, i, k), MEM_LOAD);
        sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_LOAD);
        sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_STORE);
        C[i * n + j] += A[i * n + k] * r;
      }
    }
  }
}

void sim_matmul_ikj(sim_cache_t *cache, double *C, const double *A,
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
  printf("Running Reference Solution: Exercise 07...\n\n");

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

  assert(misses_ikj > 0);
  assert(misses_ikj * 4 < misses_ijk);
  assert(misses_ijk < misses_jki);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ijk);
  free(C_jki);
  free(C_ikj);

  printf("Reference Solution 07 PASSED!\n");
  return EXIT_SUCCESS;
}

