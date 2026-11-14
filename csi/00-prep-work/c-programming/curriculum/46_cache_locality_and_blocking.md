# Exercise 55: Cache Address Geometry, LRU Simulation, and Tiled Blocking

Bridges C array access patterns to CS:APP Chapters 5–6 and `cachelab`: virtual/physical address decomposition, set-associative cache eviction, and loop tiling for spatial and temporal locality.

---

# Hardware Cache Geometry (`(S, E, B)`)

```
64-bit Address:
+------------------------------+--------------------+--------------------+
|          tag                 |   set index (s)    |  block offset (b)  |
+------------------------------+--------------------+--------------------+
```

## Parameters
- $B = 2^b$: cache block size in bytes (contiguous bytes loaded together on every miss).
- $S = 2^s$: number of cache sets.
- $E$: associativity (number of cache lines per set).
- Total data capacity: $C = S \times E \times B$.

## LRU Replacement Policy
- On a memory access, `set_index` selects one set of $E$ lines.
- If any line has `valid && line.tag == tag`, it is a **hit** and updates `lru_clock`.
- On a **miss**, an unused (`!valid`) line is filled first; if all $E$ lines are valid, the line with the smallest `lru_clock` is evicted (`evictions++`).

---

# Loop Blocking (Tiling) for Matrix Transpose

## Why Naive Transpose Thrashes the Cache
- Reading `src[i * n + j]` steps sequentially across a row (high spatial locality: 8 consecutive 4-byte `int`s share one 32-byte cache block).
- Writing `dst[j * n + i]` jumps by `n * sizeof(int)` bytes on each inner iteration. When `n * sizeof(int)` exceeds the cache working set, every column line is evicted before the next row accesses the second element in that block.

## Why `block_size x block_size` Tiling Fixes It
- Processing an $8 \times 8$ tile keeps both the source tile and destination tile resident inside the cache simultaneously.
- Each loaded cache block in `dst` is reused for all 8 rows of the tile before eviction, converting $O(N^2)$ misses on `dst` into $O(N^2 / (B / \text{sizeof(int)}))$ compulsory misses.

---

# Active Recall Questions
- Given `s = 5`, `E = 1`, `b = 5`, what is the total cache size in bytes and how many `int`s fit in one cache line?
- Why must bit masks in `cache_extract_bits` guard against shifting by `>= 64` bits?
- Why does `transpose_blocked` require clamping inner loop upper bounds with `min(i0 + block_size, n)`?
