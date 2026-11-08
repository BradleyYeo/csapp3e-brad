# Concepts: Quantitative Miss Rate Prediction

## Stride-k Miss Rate Algebra

- Let element size be $w = \text{sizeof}(T)$ bytes, cache block size be $B$ bytes, and loop step be $\text{Stride} = k$ elements ($k \times w$ bytes per step).
- Number of elements that fit in a single cache line:
  $$L = \frac{B}{w}$$
- For an array much larger than cache capacity, the theoretical miss rate is:
  $$\text{Miss Rate}(\text{Stride } k) = \min\left(1.0, \frac{k \times w}{B}\right) = \min\left(1.0, \frac{k}{L}\right)$$
- When $k \times w \ge B$, every access loads a distinct cache line, yielding a $100\%$ miss rate.

## 2D Matrix Access Analysis

### Case 1: Row-Major Traversal (`LOOP_ROW_MAJOR`)
- Loop structure:
  ```c
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      x[i][j] = i + j;
    }
  }
  ```
- Memory step between `x[i][j]` and `x[i][j+1]` is $+w$ bytes (Stride-1).
- Misses are strictly compulsory:
  $$\text{Misses} = \frac{\text{rows} \times \text{cols} \times w}{B}$$

### Case 2: Column-Major Traversal (`LOOP_COL_MAJOR`)
- Loop structure:
  ```c
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      x[j][i] = i + j;
    }
  }
  ```
- Memory step between `x[j][i]` and `x[j+1][i]` is $+(\text{cols} \times w)$ bytes.
- Subcase A (Matrix fits in cache: $\text{rows} \times \text{cols} \times w \le C$):
  - All lines loaded during column 0 remain in cache for columns $1 \dots \text{cols}-1$.
  - Only compulsory misses occur: $\text{Misses} = \frac{\text{rows} \times \text{cols} \times w}{B}$.
- Subcase B (Matrix exceeds cache capacity, row width $\ge B$):
  - Every column step jumps by at least one cache line and evicts prior lines before reuse.
  - Misses: $\text{rows} \times \text{cols}$ ($100\%$ miss rate).
- Subcase C (Matrix exceeds cache capacity, row width $< B$):
  - Example: `x[4000][8]` of 4-byte integers on a 64-byte cache line.
  - Row size is $8 \times 4 = 32\text{ bytes}$.
  - A 64-byte line holds $64 / 32 = 2$ consecutive rows (`Row 2k` and `Row 2k+1`).
  - Advancing from `Row 2k` to `Row 2k+1` in the inner loop hits the same line!
  - Misses: $\frac{\text{rows} \times \text{cols}}{B / (\text{cols} \times w)} = \frac{32,000}{2} = 16,000$ ($50\%$ miss rate).

---

# Task: Implement Analytical Predictors

- Target File: [03_miss_prediction.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.c)
- Shared Types: Defined in [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)

## Functions to Complete

- [`predict_stride_miss_rate`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.c): Computes theoretical miss rate for Stride-$k$ traversal.
- [`predict_2d_loop_write_misses`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.c): Predicts exact L1 write miss count for row-major and column-major loops.

---

# Predict Before Running

- `predict_stride_miss_rate(1, 4, 64)` $\to 0.0625$
- `predict_stride_miss_rate(2, 4, 64)` $\to 0.125$
- `predict_stride_miss_rate(16, 4, 64)` $\to 1.0$
- `predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_ROW_MAJOR)` $\to 1,000,000$
- `predict_2d_loop_write_misses(4000, 4000, 4, 64, 65536, LOOP_COL_MAJOR)` $\to 16,000,000$
- `predict_2d_loop_write_misses(4000, 8, 4, 64, 65536, LOOP_COL_MAJOR)` $\to 16,000$
- `predict_2d_loop_write_misses(64, 64, 4, 64, 65536, LOOP_COL_MAJOR)` $\to 256$ (working set fits in 64 KiB!)

---

# Active Recall Checkpoints

- Why does column-major traversal of `x[64][64]` not suffer a 100% miss rate on a 64 KiB cache?
- In `loop-order.c`, why did `option_five` (`x[4000][8]`) produce ~16,000 misses instead of 32,000?
- What is the difference between compulsory misses and capacity misses in large matrix loops?

---

# Navigation Links

- Next Exercise: [04_loop_order_measure.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.md)
- Reference Solution: [reference-solution/03_miss_prediction.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/03_miss_prediction.c)
