/*
 * Exercise 03: Analytical Miss Rate & 2D Loop Miss Predictor
 *
 * Guide: 03_miss_prediction.md
 * Shared definitions: cache_sim.h
 *
 * Build and run:
 *   make 03
 */
#include "cache_sim.h"

/*
 * Drill 1: Stride-k Miss Rate Formula
 *
 * Precondition:
 *   stride_elems >= 1, elem_size >= 1, block_size >= elem_size.
 * Postcondition:
 *   Returns theoretical miss rate in [0.0, 1.0]:
 *     miss_rate = min(1.0, (stride_elems * elem_size) / (double)block_size)
 */
double predict_stride_miss_rate(size_t stride_elems, size_t elem_size,
                                size_t block_size) {
  // TODO: Implement Stride-k miss rate formula
  (void)stride_elems;
  (void)elem_size;
  (void)block_size;
  return 0.0;
}

/*
 * Drill 2: 2D Loop Write Miss Predictor
 *
 * Precondition:
 *   rows >= 1, cols >= 1, elem_size >= 1, block_size >= elem_size.
 *   (rows * cols * elem_size) is a multiple of block_size.
 * Postcondition:
 *   Returns predicted L1 write misses:
 *   - LOOP_ROW_MAJOR:
 *       total_bytes / block_size
 *   - LOOP_COL_MAJOR:
 *       If total_bytes <= cache_capacity: total_bytes / block_size
 *       Else:
 *         row_bytes = cols * elem_size
 *         If row_bytes >= block_size: rows * cols
 *         Else: (rows * cols) / (block_size / row_bytes)
 */
uint64_t predict_2d_loop_write_misses(size_t rows, size_t cols,
                                      size_t elem_size,
                                      size_t block_size,
                                      size_t cache_capacity,
                                      loop_order_t order) {
  // TODO: Implement 2D loop write miss predictor
  (void)rows;
  (void)cols;
  (void)elem_size;
  (void)block_size;
  (void)cache_capacity;
  (void)order;
  return 0;
}

int main(void) {
  printf("Running Exercise 03: Analytical Miss Rate Predictor...\n\n");

  /* Test 1: Stride-k miss rates */
  assert(fabs(predict_stride_miss_rate(1, 4, 64) - 0.0625) < 1e-9);
  assert(fabs(predict_stride_miss_rate(2, 4, 64) - 0.125) < 1e-9);
  assert(fabs(predict_stride_miss_rate(16, 4, 64) - 1.0) < 1e-9);
  assert(fabs(predict_stride_miss_rate(4000, 4, 64) - 1.0) < 1e-9);

  /* Test 2: loop-order.c option_one (4000x4000 row-major, 64 KiB L1d, 64B line) */
  uint64_t misses_opt1 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_ROW_MAJOR);
  assert(misses_opt1 == 1000000ULL);

  /* Test 3: loop-order.c option_two (4000x4000 col-major, 64 KiB L1d, 64B line) */
  uint64_t misses_opt2 =
      predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt2 == 16000000ULL);

  /* Test 4: loop-order.c option_five (4000x8 col-major, 64 KiB L1d, 64B line) */
  uint64_t misses_opt5 =
      predict_2d_loop_write_misses(4000, 8, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_opt5 == 16000ULL);

  /* Test 5: Small matrix 64x64 inside 64 KiB L1d */
  uint64_t misses_small =
      predict_2d_loop_write_misses(64, 64, 4, 64, 65536, LOOP_COL_MAJOR);
  assert(misses_small == 256ULL);

  printf("Exercise 03 PASSED!\n");
  return EXIT_SUCCESS;
}
