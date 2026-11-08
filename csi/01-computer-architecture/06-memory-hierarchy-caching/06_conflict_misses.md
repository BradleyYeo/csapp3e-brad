# Concepts: Conflict Misses, Power-of-Two Aliasing, and Padding

## The Critical Stride Formula

- In any cache with $S = 2^s$ sets and block size $B = 2^b$ bytes, the set index repeats every:
  $$\text{Critical Stride} = S \times B \text{ bytes}$$
- Any two addresses $A_1$ and $A_2$ whose difference is an exact multiple of the critical stride:
  $$(A_1 - A_2) \equiv 0 \pmod{S \times B}$$
  possess identical middle bits $[b \dots s+b-1]$ and map to the **exact same cache set**.

## Pathological Conflict Thrashing in Direct-Mapped Caches ($E = 1$)

- Consider $S = 32$ sets, $B = 32\text{ bytes}$ ($\text{Critical Stride} = 1024\text{ bytes}$).
- If an algorithm alternates access between two variables `A` and `B` located $1024$ bytes apart:
  - Load `A` $\implies$ Evicts `B`.
  - Load `B` $\implies$ Evicts `A`.
  - Result: $100\%$ cache miss rate, even though the working set is only 2 cache lines ($64$ bytes) and the cache has $1024$ bytes of space!

## The Two Remedies for Conflict Thrashing

### Hardware Remedy: Associativity ($E > 1$)
- Adding parallel ways per set allows multiple addresses with the same set index to reside in the set simultaneously.
- In a 2-way set-associative cache ($E = 2$), `A` and `B` occupy `Way 0` and `Way 1` of the same set with zero evictions.

### Software Remedy: Array Padding
- When an array row width $W = \text{cols} \times \text{sizeof}(T)$ is a multiple of the critical stride, row $i$ and row $i+1$ map to the exact same set index during column-wise traversal.
- By padding each row with $P = B / \text{sizeof}(T)$ elements (1 cache block), each row's starting address shifts by $+B$ bytes, shifting its set index by $+1 \pmod S$.

---

# Task: Implement Aliasing and Padding Utilities

- Target File: [06_conflict_misses.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/06_conflict_misses.c)
- Shared Types: Defined in [cache_sim.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_sim.h)
- Simulator Dependency: Links against `05_lru_cache_sim.c` for `sim_cache_access`.

## Functions to Complete

- [`is_set_aliasing`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/06_conflict_misses.c): Determines whether two arbitrary 64-bit addresses map to the same set index.
- [`compute_row_padding_elements`](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/06_conflict_misses.c): Computes the number of padding elements required to break critical-stride row alignment.

---

# Predict Before Running

- Geometry $(s=5, E=1, b=5) \implies S=32, B=32\text{ B}, \text{Critical Stride} = 1024\text{ B}$:
  - `is_set_aliasing(0x10000, 0x10400, &geom)` $\to$ `true` ($+1024\text{ B}$)
  - `is_set_aliasing(0x10000, 0x10800, &geom)` $\to$ `true` ($+2048\text{ B}$)
  - `is_set_aliasing(0x10000, 0x10020, &geom)` $\to$ `false` ($+32\text{ B}$)
- 50 alternating accesses on $E=1$: $100$ misses, $0$ hits.
- 50 alternating accesses on $E=2$: $2$ misses, $98$ hits.

---

# Active Recall Checkpoints

- Why does Cache Lab's $64 \times 64$ matrix transpose experience severe conflict thrashing with an $8 \times 8$ blocking factor?
- What is the mathematical definition of critical stride?
- If row width in bytes is not divisible by critical stride, why is row padding unnecessary?

---

# Navigation Links

- Next Exercise: [07_matmul_loop_orders.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/07_matmul_loop_orders.md)
- Reference Solution: [reference-solution/06_conflict_misses.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/06_conflict_misses.c)
