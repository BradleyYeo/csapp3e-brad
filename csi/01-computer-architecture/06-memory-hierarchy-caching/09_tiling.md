# Concepts: 2D Cache Blocking (Tiling) & Working Set Geometry

## Why Untiled Algorithms Suffer Capacity Evictions

- In untiled `ikj`, for a single outer row `i`, the middle loop `k` iterates across all columns of matrix $B$, streaming through the entire $N \times N$ matrix ($8\text{ MiB}$ for $N = 1024$).
- By the time outer loop row `i` finishes and row `i+1` starts, all of matrix $B$ has been evicted from L1 and L2 caches and must be re-fetched from DRAM!

## The Working Set Constraint

To keep one active tile of $A$, one active tile of $B$, and one active tile of $C$ simultaneously resident in an L1 data cache of capacity $C_{\text{L1}}$:
$$3 \times T^2 \times \text{sizeof}(\text{double}) \le C_{\text{L1}}$$

- For standard L1d cache ($C_{\text{L1}} = 32\text{ KiB} = 32,768\text{ bytes}$):
  $$3 \times T^2 \times 8 \le 32,768 \implies T^2 \le 1365 \implies T = 32 \quad (3 \times 32^2 \times 8 = 24,576\text{ bytes} \le 32\text{ KiB})$$
- For a small 1 KiB cache ($C = 1024\text{ bytes}$):
  $$3 \times T^2 \times 8 \le 1024 \implies T^2 \le 42.6 \implies T = 4 \quad (3 \times 16 \times 8 = 384\text{ bytes} \le 1024\text{ bytes})$$

## The Tiled Matrix Multiplication Algorithm

```c
for (size_t ti = 0; ti < n; ti += tile_size) {
  for (size_t tj = 0; tj < n; tj += tile_size) {
    for (size_t tk = 0; tk < n; tk += tile_size) {
      for (size_t i = ti; i < min(ti + tile_size, n); ++i) {
        for (size_t k = tk; k < min(tk + tile_size, n); ++k) {
          double r = A[i * n + k];
          for (size_t j = tj; j < min(tj + tile_size, n); ++j) {
            C[i * n + j] += r * B[k * n + j];
          }
        }
      }
    }
  }
}
```

## The Low-Associativity Trap (Why Tiling Can Fail on $E=2$)

- If cache associativity is too low (e.g. $E = 2$ lines per set) and row stride is a multiple of half the critical stride, tile rows alias into the same sets.
- Because active tiles of $A$, $B$, and $C$ map to the exact same set index, 2 lines per set are insufficient to hold all 3 tiles simultaneously, causing continuous conflict thrashing.
- With $E \ge 4$, all 3 active tiles reside in the cache concurrently, reducing total misses from $1088$ to $636$!

---

# Task: Implement Tiling and Working Set Calculation

- Target File: [09_tiling.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.c)
- Shared Types: Defined in [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)
- Simulator Dependency: Links against `05_lru_cache_sim.c` for `sim_cache_access`.

## Functions to Complete

- [`compute_max_tile_size`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.c): Returns the largest power-of-two tile dimension $T$ satisfying $3 \times T^2 \times \text{elem\_size} \le C$.
- [`sim_matmul_tiled`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/09_tiling.c): Computes matrix multiplication using 2D cache blocking while recording accesses in the simulator.

---

# Predict Before Running

- Small 1 KiB cache ($s=3, E=4, b=5$):
  - Untiled `ikj` generates: $1088$ misses.
  - Tiled `ikj` with $T = 4$ generates: $636$ misses ($41\%$ miss reduction!).

---

# Active Recall Checkpoints

- Why must the working set calculation account for 3 tiles simultaneously ($A$, $B$, and $C$)?
- How does tiling convert capacity misses into cache hits?
- What happens if the tile dimension $T$ is chosen too large ($3 \times T^2 \times 8 > C_{\text{L1}}$)?

---

# Navigation Links

- Next Exercise: [10_memory_mountain.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/10_memory_mountain.md)
- Reference Solution: [reference-solution/09_tiling.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/09_tiling.c)
