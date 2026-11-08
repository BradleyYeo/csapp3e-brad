# Cumulative Self-Test: Caching and the Memory Hierarchy

Answer each question on paper from memory before checking your responses against [reference-solution/11_answers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/11_answers.md).

---

# Section 1: Address Decoding and Geometry

## Question 1.1: Bit Slicing
A microprocessor features 64-bit physical addresses and a cache with $s = 6$, $E = 8$, $b = 6$.
- How many total sets $S$ are in the cache?
- What is the cache block size $B$ in bytes?
- What is the total data capacity $C$ of the cache?
- How many tag bits $t$ are stored per cache line?
- What is the critical stride in bytes?

## Question 1.2: Middle-Bit Indexing
Why does hardware place the set index in the middle bits of an address rather than in the highest bits?

## Question 1.3: Tag Storage
Why does a cache line store only the tag bits rather than the entire 64-bit physical address?

---

# Section 2: Principle of Locality and Miss Rates

## Question 2.1: Stride-k Formula
Given 64-byte cache lines, what is the theoretical miss rate when sequentially traversing an array of 8-byte `double` elements with:
- Stride-1?
- Stride-4?
- Stride-8?
- Stride-16?

## Question 2.2: The Narrow-Row Case
In `04_loop_order_measure.c`, why did `option_five` (column-major access on `int x[4000][8]`) produce a $50\%$ miss rate rather than $100\%$ on a 64-byte line architecture?

## Question 2.3: Temporal vs Spatial Locality
Identify which type of locality (temporal or spatial) is primarily exploited by:
- Storing a loop accumulator in a CPU register.
- Reading through a contiguous string using a cursor pointer.
- Loop unrolling with multiple accumulators.

---

# Section 3: Write Policies and Dirty-Bit Lifecycle

## Question 3.1: Policy Pairings
Why does Write-Back pair with Write-Allocate, while Write-Through pairs with No-Write-Allocate?

## Question 3.2: DRAM Bus Traffic
In a write-back cache, under what exact conditions does evicting a cache line generate bus traffic to main memory?

## Question 3.3: Repeated Stores
Why does an algorithm performing $1,000,000$ consecutive stores to the same variable generate near-zero off-chip memory traffic in a write-back cache?

---

# Section 4: Conflict Thrashing and Cache Lab Insights

## Question 4.1: The 3 Cs
Explain the difference between a Capacity miss and a Conflict miss. Which one can be resolved by increasing associativity $E$? Which one can be resolved by loop tiling?

## Question 4.2: Critical Stride Aliasing
In a direct-mapped cache with $S = 32$ sets and $B = 32\text{ bytes}$, why do addresses `0x10000` and `0x10400` thrash each other continuously?

## Question 4.3: Cache Lab Transpose Diagonal Collision
In Cache Lab Part B ($s=5, E=1, b=5$), why does transposing a diagonal $8 \times 8$ block ($bi == bj$) suffer severe conflict misses unless row elements are staged into local scalar registers?

---

# Section 5: Matrix Multiplication and Tiling

## Question 5.1: Inner Loop Ordering
Why does the `ikj` loop order generate $\approx 4\times$ fewer cache misses than `ijk` for matrix multiplication, despite performing the exact same arithmetic operations?

## Question 5.2: Tiling Working Set Constraint
Derive the formula for the maximum square tile dimension $T$ that can fit in an L1 data cache of capacity $C_{\text{L1}}$ during matrix multiplication.

## Question 5.3: IEEE 754 Non-Associativity
Why does matrix verification fail with exact `!=` after reordering matrix loops or introducing multiple accumulators?

---

# Mastery Checklist

Before concluding this module, verify that you can:
- Derive tag, set index, and block offset by hand for any 64-bit address.
- Predict the L1 miss count of any 2D loop nest within $10\%$ before running a profiler.
- Trace an access stream through a multi-way set-associative LRU cache on paper.
- Explain the physical difference between SRAM (L1/L2/L3) and DRAM (Main Memory).
- Implement cache blocking (tiling) and calculate optimal tile size from hardware parameters.
- Diagnose and eliminate conflict thrashing using row padding or associativity.
