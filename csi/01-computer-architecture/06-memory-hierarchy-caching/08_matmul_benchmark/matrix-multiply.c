/*
 * Matrix Multiplication Cache Optimization (CS:APP Section 6.2, 6.6)
 *
 * Compute: C = A * B
 * Matrix A: a_rows x a_cols
 * Matrix B: a_cols x b_cols
 * Matrix C: a_rows x b_cols
 */

#include <stdio.h>
#include <stdlib.h>
#include "matrix-multiply.h"

/*
 * Baseline naive matrix multiplication: i-j-k loop order.
 *
 * Cache Behavior Analysis:
 * - A[i][k]: Iterating k scans row i sequentially (Stride-1 access, excellent spatial locality).
 * - B[k][j]: Iterating k scans down column j (Stride-N access, poor spatial locality, high cache miss rate).
 * - C[i][j]: Held in register or single location across k loop.
 *
 * DO NOT MODIFY THIS FUNCTION (the benchmark compares against it).
 */
void matrix_multiply(double **C, double **A, double **B, int a_rows, int a_cols,
                     int b_cols) {
  for (int i = 0; i < a_rows; i++) {
    for (int j = 0; j < b_cols; j++) {
      C[i][j] = 0;
      for (int k = 0; k < a_cols; k++)
        C[i][j] += A[i][k] * B[k][j];
    }
  }
}

/*
 * fast_matrix_multiply: Your optimized implementation from scratch.
 *
 * Optimization Strategies to explore (CS:APP 6.6.2):
 *
 * 1. Loop Reordering:
 *    Analyze the 6 possible permutations of loops (ijk, jik, kij, ikj, jki, kji).
 *    Notice that in the i-k-j or k-i-j loop orders, the inner loop index is j:
 *    Both B[k][j] and C[i][j] are accessed with Stride-1 (contiguous in memory)!
 *    This converts column strides into row strides, dramatically reducing L1/L2 misses.
 *
 * 2. Matrix Transposition:
 *    Transpose matrix B so that B_T[j][k] stores elements row-wise.
 *    Then computing the dot product of A[i] and B_T[j] reads both matrices contiguously.
 *
 * 3. Cache Blocking / Tiling:
 *    Divide the large N x N matrices into smaller B x B sub-blocks that fit entirely
 *    within the L1 cache (e.g. B = 32 or 64). Iterate over blocks so all loaded data
 *    is reused before eviction.
 */
void fast_matrix_multiply(double **C, double **A, double **B, int a_rows,
                          int a_cols, int b_cols) {
  // TODO: Implement cache-friendly matrix multiplication
  //
  // Starter fallback: identical to naive
  matrix_multiply(C, A, B, a_rows, a_cols, b_cols);
}
