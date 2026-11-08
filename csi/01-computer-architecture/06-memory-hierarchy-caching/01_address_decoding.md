# Concepts: Cache Geometry and Address Decoding

## The $(S, E, B, m)$ Tuple

- $m$: Number of physical address bits ($m = 64$ on x86-64 and ARM64).
- $s$: Number of set index bits, defining $S = 2^s$ total cache sets.
- $E$: Associativity (number of cache lines per set).
  - $E = 1$: Direct-mapped cache.
  - $E > 1$: $E$-way set-associative cache.
  - $S = 1, E = C/B$: Fully associative cache.
- $b$: Number of block offset bits, defining $B = 2^b$ bytes per cache line.
- $t = m - (s + b)$: Number of tag bits stored in each line.
- Total data capacity:
  $$C = S \times E \times B = 2^{s+b} \times E \text{ bytes}$$
- Critical Stride (Set wrap-around period):
  $$\text{Critical Stride} = S \times B \text{ bytes}$$

## Address Bit Partitioning

```
63                      s + b               b                   0
+-----------------------+-------------------+-------------------+
|      Tag (t bits)     | Set Index (s bits)| Block Offset (b)  |
+-----------------------+-------------------+-------------------+
```

- Offset ($b$ bits): Selects the specific byte within the $B$-byte cache line.
- Set Index ($s$ bits): Determines which set the address maps to.
- Tag ($t$ bits): Distinguishes memory blocks mapping to the same set.

## Bitwise Extraction Algebra

```c
uint64_t offset    = addr & ((1ULL << b) - 1);
uint64_t set_index = (addr >> b) & ((1ULL << s) - 1);
uint64_t tag       = addr >> (s + b);
```

- `1ULL << b`: Creates a 64-bit unsigned integer with bit $b$ set.
- `(1ULL << b) - 1`: Produces a low-bit mask of $b$ ones.
- Using `1 << b` invokes 32-bit signed shift undefined behavior when $b \ge 31$.

## Why Set Index Bits Occupy the Middle

- In high-bit indexing, consecutive memory addresses share the same high bits. Scanning an array would concentrate all memory references into a single set while the other $S-1$ sets sit idle.
- In middle-bit indexing, consecutive $B$-byte memory blocks increment the set index by $1 \pmod S$. Contiguous arrays distribute across all $S$ sets without set collisions.

---

# Task: Implement Address Decoding Functions

- Target File: [01_address_decoding.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.c)
- Shared Types: Defined in [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)

## Functions to Complete

- [`compute_cache_geometry`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.c): Computes `num_sets`, `block_size`, `tag_bits`, `capacity_bytes`, and `critical_stride_bytes`.
- [`decode_cache_address`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.c): Extracts `offset`, `set_index`, and `tag` using bitwise masks.
- [`decode_flawed_high_bit_index`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/01_address_decoding.c): Extracts the top $s$ bits to demonstrate high-bit collision.

---

# Predict Before Running

- Address: `0x123456789ABCDEF5` on $(s=4, E=1, b=4)$:
  - Offset bits: Low 4 bits (hex digit `0x5`).
  - Set index bits: Bits 4..7 (hex digit `0xF`).
  - Tag bits: Bits 8..63 (`0x00123456789ABCDE`).
- Cache geometry for $(s=6, E=8, b=6)$:
  - Total capacity: $2^6 \times 8 \times 2^6 = 64 \times 8 \times 64 = 32,768\text{ bytes}$ ($32\text{ KiB}$).
  - Critical stride: $64 \times 64 = 4096\text{ bytes}$ ($4\text{ KiB}$).

---

# Active Recall Checkpoints

- Why must `1ULL` be used instead of `1` when building bitmasks for 64-bit addresses?
- If a cache has $m=64, s=5, b=5$, how many bits are used for the tag?
- What physical phenomenon occurs if two active memory buffers are separated by an exact multiple of the critical stride in a direct-mapped cache?

---

# Navigation Links

- Next Exercise: [02_hand_trace_drills.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/02_hand_trace_drills.md)
- Reference Solution: [reference-solution/01_address_decoding.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/01_address_decoding.c)
