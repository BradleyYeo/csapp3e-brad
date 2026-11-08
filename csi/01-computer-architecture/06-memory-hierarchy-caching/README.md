# The Memory Hierarchy: Caching and Locality

## Curriculum Reading Sequence
- Layer 01: [CONCEPTS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/CONCEPTS.md) - Theory: geometry, address decoding, locality algebra, write policies, 3 Cs, matmul permutations, tiling
- Layer 02: [cache_exercises.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/cache_exercises.c) - 6 test-driven coding stages (no hardware needed)
- Layer 03: Hands-on measurement: [loop-order.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/loop-order.c) and [matrix-multiply.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/matrix-multiply.c) with `perf` / `cachegrind`
- Layer 04: Self-test (recall questions below)
- Next: [07-memory-hierarchy-data-layout](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/07-memory-hierarchy-data-layout/README.md)

## Textbook Reference
- CS:APP3e Chapter 6, Sections 6.1-6.6

## Workflow Per Stage
- Read the matching section of CONCEPTS.md.
- Close it. Write the answer on paper first (active recall).
- Implement the TODO in `cache_exercises.c`.
- Run `make test STAGE=n`. Each stage is independent.
- Only after the stage passes (or after 30 minutes stuck), compare with `reference-solution/`.

## Build and Run
```bash
make test              # all stages
make test STAGE=3      # one stage
make test-solution     # sanity check reference (spoilers)
make clean
```

---

# Part 1: Coding Stages (`cache_exercises.c`)

## Stage 1: Geometry and Address Decoding
- Functions: `compute_cache_geometry`, `decode_cache_address`, `decode_flawed_high_bit_index`
- Concepts: $(S,E,B,m)$, tag/set/offset bit slicing, `1ULL` shifts, why set bits sit in the middle
- Link: same bit-masking algebra as `bump_align_forward` in `exercises/11_bump_allocator`
- Cache Lab link: needed for `cache_access` in `csim.c`

## Stage 2: Analytical Miss Prediction
- Functions: `predict_stride_miss_rate`, `predict_2d_loop_write_misses`
- Concepts: Stride-k miss rate $\min(1, kw/B)$, working set vs capacity, narrow-row case (`option_five`)
- Check: reproduces the numbers in `reference-solution/solution.md` (1,000,000 / 16,000,000 / 16,000 misses)

## Stage 3: LRU Cache with Write Policies
- Function: `sim_cache_access`
- Concepts: hit/miss/eviction, LRU clock, dirty bit, write-back + write-allocate vs write-through + no-allocate
- Trace to hand-verify first: `yi.trace` in [CONCEPTS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/CONCEPTS.md) (hits 4, misses 5, evictions 3)
- Cache Lab link: this is `csim.c` plus a dirty bit

## Stage 4: Conflict Misses and Padding
- Functions: `is_set_aliasing`, `compute_row_padding_elements`
- Concepts: critical stride $S \times B$, power-of-two aliasing, associativity as the hardware fix, padding as the software fix
- Cache Lab link: why 64x64 transpose is adversarial

## Stage 5: Matmul Loop Permutations
- Functions: `sim_matmul_ijk`, `sim_matmul_jki`, `sim_matmul_ikj`
- Concepts: per-inner-iteration miss table (1.125 / 2.0 / 0.25), register-held invariants
- Predict first: write the expected miss ordering, then compare to printed counts

## Stage 6: Tiling
- Functions: `compute_max_tile_size`, `sim_matmul_tiled`
- Concepts: $3T^2 w \le C$, capacity misses vs tiling
- Stretch: change the test cache to `E=2` and explain why tiling then loses (tile rows alias into the same sets). Fix it with array padding from Stage 4.

---

# Part 2: Hands-On Measurement (Predict, Then Measure)

## Exercise A: `loop-order.c`
- Predict L1 write misses for `option_one`, `option_two`, `option_five` before running (formulas in CONCEPTS.md).
- Measure (Linux or Colima x86-64 container; macOS has no Valgrind):
  ```bash
  gcc -O0 -g loop-order.c -o loop-order
  valgrind --tool=cachegrind --cache-sim=yes ./loop-order
  perf stat ./loop-order
  ```
- Compile at `-O1` and read the assembly. Why do both options become empty loops? Reproduce the fix from `solution.md` (pass the array via pointer).
- Open question from your notes: why do `LLd` misses for `option_five` not match `D1` misses? Hint: the array is $128$ KB, which fits in the last-level cache, so repeated passes hit there.

## Exercise B: `matrix-multiply.c`
- Predict the D1 read-miss count for naive `ijk` at $N=512$ using the Stage 5 table.
- Implement `fast_matrix_multiply` in stages and record speedup after each:
  - `ikj` loop permutation (no transpose, no extra memory)
  - Transpose `B` then `ijk`
  - Tiling with `T` from Stage 6
  - Tiling + multiple accumulators (ties back to Chapter 5)
- Verify with a tolerance, not `!=`: floating-point addition is not associative.
- Report results in a table: variant, `-O0`, `-O2`, D1 misses, speedup.

## Exercise C: Memory Mountain (Stretch)
- Write a program that reads a buffer of size `size` with stride `stride` and records throughput (MB/s).
- Sweep `size` from 4 KB to 64 MB and `stride` from 1 to 64 elements.
- Identify L1, L2, L3, DRAM plateaus. Compare to `lscpu` cache sizes.
- Reference: CS:APP 6.6.1.

---

# Active Recall Checkpoints

## Geometry and Addressing
- A cache has $s=6$, $E=8$, $b=6$. What is its capacity, and what is its critical stride?
- Why are the set index bits in the middle and not the top of the address?
- Why does the cache store only the tag, not the full address?

## Locality and Miss Rates
- What is the minimum miss rate of Stride-1 traversal of `double`s with 64-byte lines?
- At what stride does the miss rate reach 100%?
- Why does `x[4000][8]` column traversal miss only 50% of the time while `x[4000][4000]` misses 100%?

## Policies
- Why does write-back pair naturally with write-allocate?
- When does a dirty eviction cost extra memory traffic?
- Why do loop counters and accumulators never reach DRAM?

## Conflicts and Capacity
- Name the 3 Cs. Which one does tiling fix? Which does associativity fix?
- Why are power-of-two matrix sizes pathological?
- Why can tiling make things worse when associativity is low?

## Matmul
- Why does `ikj` beat `ijk` with identical arithmetic?
- Why does `ikj` alone still degrade for large $N$?
- Why does `matrix_equal` fail after reordering floating-point additions?

---

# Mastery Checklist
- Decode any address by hand for a given $(s, E, b)$.
- Predict hit/miss for a short trace by hand (as you did for `yi.trace`).
- Predict the miss count of a 2D loop nest within ~10% before measuring.
- Explain from first principles why the 64x64 transpose needs $4 \times 4$ staging.
- Teach someone why tiling needs $3T^2 w \le C$.
