#include <time.h>
#include <stdio.h>
#include "vec.h"

#define TEST_LOOPS 10000

/*
 * dotproduct computes the vector dot product:
 * sum = sum(u[i] * v[i]) for i in [0, len)
 *
 * Baseline Naive Implementation:
 * Uses abstract data type accessor get_vec_element and calls vec_length
 * on every single loop iteration.
 */
data_t dotproduct(vec_ptr u, vec_ptr v) {
   data_t sum = 0, u_val, v_val;

   for (long i = 0; i < vec_length(u); i++) {
      get_vec_element(u, i, &u_val);
      get_vec_element(v, i, &v_val);
      sum += u_val * v_val;
   }   
   return sum;
}

/*
 * dotproductfast: Your optimized implementation from scratch.
 *
 * Optimization Ladder to master (CS:APP Chapter 5):
 * 1. Hoist Loop Invariant:
 *    Call vec_length(u) once before loop begins instead of every iteration.
 *
 * 2. Eliminate Function Call Overhead:
 *    Access the underlying array pointers directly (e.g. data_t *u_data = get_vec_start(u)).
 *
 * 3. Loop Unrolling (2x, 4x, 8x):
 *    Process multiple elements per iteration to amortize loop counter and branch overhead.
 *    Remember to handle any remaining tail elements (i < len) after the unrolled loop!
 *
 * 4. Multiple Accumulators (2x2, 4x4):
 *    A single accumulator `sum += ...` creates a Read-After-Write (RAW) data dependency chain:
 *    the next addition cannot issue until the current addition finishes writeback.
 *    By maintaining multiple independent accumulators (sum0, sum1, sum2, sum3), you allow
 *    the CPU's superscalar execution units to issue multiple floating point multiplications
 *    and additions in parallel every cycle.
 */
data_t dotproductfast(vec_ptr u, vec_ptr v) {
   // TODO: Step 1 - Hoist loop length
   // TODO: Step 2 - Access vector data array pointers directly
   // TODO: Step 3 - Unroll loop (e.g. 4x or 8x)
   // TODO: Step 4 - Use multiple independent accumulators to break RAW dependency
   // TODO: Step 5 - Handle leftover elements in a cleanup loop

   // Starter fallback (identical to baseline):
   return dotproduct(u, v);
}
