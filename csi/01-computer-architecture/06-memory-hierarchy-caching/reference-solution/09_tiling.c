/*
 * Reference Solution: Exercise 09 (2D Cache Blocking / Tiling)
 */
#include "../cache_sim.h"

size_t compute_max_tile_size(size_t l1_capacity_bytes, size_t elem_size) {
  size_t t = 1;
  while (3 * (t * 2) * (t * 2) * elem_size <= l1_capacity_bytes) {
    t *= 2;
  }
  return t;
}

void sim_matmul_tiled(sim_cache_t *cache, double *C, const double *A,
                      const double *B, size_t n, size_t tile_size) {
  memset(C, 0, n * n * sizeof(double));
  for (size_t ti = 0; ti < n; ti += tile_size) {
    for (size_t tj = 0; tj < n; tj += tile_size) {
      for (size_t tk = 0; tk < n; tk += tile_size) {
        for (size_t i = ti; i < ti + tile_size && i < n; ++i) {
          for (size_t k = tk; k < tk + tile_size && k < n; ++k) {
            double r = A[i * n + k];
            sim_cache_access(cache, mat_addr(SIM_BASE_A, n, i, k), MEM_LOAD);
            for (size_t j = tj; j < tj + tile_size && j < n; ++j) {
              sim_cache_access(cache, mat_addr(SIM_BASE_B, n, k, j), MEM_LOAD);
              sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_LOAD);
              sim_cache_access(cache, mat_addr(SIM_BASE_C, n, i, j), MEM_STORE);
              C[i * n + j] += r * B[k * n + j];
            }
          }
        }
      }
    }
  }
}

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
  printf("Running Reference Solution: Exercise 09...\n\n");

  assert(compute_max_tile_size(32768, 8) == 32);
  assert(compute_max_tile_size(1024, 8) == 4);

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

  assert(misses_tiled > 0 && misses_tiled < misses_untiled);

  sim_cache_destroy(&cache);
  free(A);
  free(B);
  free(C_ikj);
  free(C_tiled);

  printf("Reference Solution 09 PASSED!\n");
  return EXIT_SUCCESS;
}

