/*
 * Reference Solution: Exercise 03 (Analytical Miss Rate Predictor)
 */
#include "../cache_sim.h"

double predict_stride_miss_rate(size_t stride_elems, size_t elem_size,
                                size_t block_size) {
  double r = ((double)stride_elems * (double)elem_size) / (double)block_size;
  return r > 1.0 ? 1.0 : r;
}

uint64_t predict_2d_loop_write_misses(size_t rows, size_t cols,
                                      size_t elem_size,
                                      size_t block_size,
                                      size_t cache_capacity,
                                      loop_order_t order) {
  uint64_t total_bytes = (uint64_t)rows * cols * elem_size;
  uint64_t compulsory = total_bytes / block_size;
  if (order == LOOP_ROW_MAJOR) return compulsory;
  if (total_bytes <= cache_capacity) return compulsory;
  uint64_t row_bytes = (uint64_t)cols * elem_size;
  if (row_bytes >= block_size) return (uint64_t)rows * cols;
  uint64_t rows_per_line = block_size / row_bytes;
  return ((uint64_t)rows * cols) / rows_per_line;
}

int main(void) {
  printf("Running Reference Solution: Exercise 03...\n\n");

  assert(fabs(predict_stride_miss_rate(1, 4, 64) - 0.0625) < 1e-9);
  assert(fabs(predict_stride_miss_rate(2, 4, 64) - 0.125) < 1e-9);
  assert(fabs(predict_stride_miss_rate(16, 4, 64) - 1.0) < 1e-9);
  assert(fabs(predict_stride_miss_rate(4000, 4, 64) - 1.0) < 1e-9);

  uint64_t misses_opt1 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_ROW_MAJOR);
  assert(misses_opt1 == 1000000ULL);

  uint64_t misses_opt2 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt2 == 16000000ULL);

  uint64_t misses_opt5 =
      predict_2d_loop_write_misses(4000, 8, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt5 == 16000ULL);

  uint64_t misses_small =
      predict_2d_loop_write_misses(64, 64, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_small == 256ULL);

  printf("Reference Solution 03 PASSED!\n");
  return EXIT_SUCCESS;
}
