# Reference Answers: Cumulative Self-Test

## Section 1: Address Decoding and Geometry

### Question 1.1 Answers
- Total sets: $S = 2^6 = 64$ sets.
- Block size: $B = 2^6 = 64$ bytes per line.
- Capacity: $C = S \times E \times B = 64 \times 8 \times 64 = 32,768\text{ bytes}$ ($32\text{ KiB}$).
- Tag bits: $t = 64 - (6 + 6) = 52$ bits.
- Critical stride: $S \times B = 64 \times 64 = 4096\text{ bytes}$ ($4\text{ KiB}$).

### Question 1.2 Answer
Middle-bit indexing ensures that contiguous memory addresses cycle sequentially through sets $0, 1, 2 \dots S-1$. High-bit indexing would map all consecutive memory blocks within a $2^{64-s}$ byte range to the exact same cache set, leaving the rest of the cache completely unutilized.

### Question 1.3 Answer
The set index and block offset are implicitly determined by the physical row within the cache array (`sets[index]`) and byte position within the line. Storing set index or offset in the line header would be redundant. Only the upper tag bits are necessary to disambiguate blocks sharing the same set index.

---

## Section 2: Principle of Locality and Miss Rates

### Question 2.1 Answers
Elements per line: $L = 64 / 8 = 8$ doubles.
- Stride-1: $1 / 8 = 0.125$ ($12.5\%$ miss rate).
- Stride-4: $4 / 8 = 0.500$ ($50.0\%$ miss rate).
- Stride-8: $8 / 8 = 1.000$ ($100.0\%$ miss rate).
- Stride-16: $\min(1.0, 16 / 8) = 1.000$ ($100.0\%$ miss rate).

### Question 2.2 Answer
In `x[4000][8]`, each row occupies $8 \times 4 = 32\text{ bytes}$. On a 64-byte line architecture, every cache line holds 2 consecutive rows (`Row 2k` and `Row 2k+1`). In column-major traversal, the inner loop advances by 32 bytes from `Row 2k` to `Row 2k+1`, hitting the second half of the cache line already loaded for `Row 2k`. Therefore, only 1 miss occurs every 2 inner iterations ($50\%$).

### Question 2.3 Answers
- Loop accumulator in register: **Temporal Locality** (same location referenced repeatedly).
- String cursor traversal: **Spatial Locality** (neighboring bytes loaded in the same cache block).
- Multiple accumulators: **Instruction-Level Parallelism** (breaks RAW dependency chains; registers hold temporal state).

---

## Section 3: Write Policies and Dirty-Bit Lifecycle

### Question 3.1 Answer
Write-Back buffers writes in cache SRAM, so it needs Write-Allocate to pull the target block into cache on a write miss so future writes hit SRAM. Write-Through updates main memory on every write, so allocating a cache line on a write miss provides little advantage and would evict other useful lines.

### Question 3.2 Answer
Only when the evicted line has `valid == true` and `dirty == true`. Clean lines (`dirty == false`) are discarded silently with zero DRAM traffic because main memory already contains identical data.

### Question 3.3 Answer
The first store incurs a write miss and allocates a cache line, setting `dirty = true`. The remaining $999,999$ stores hit the L1 SRAM cache line, updating data at CPU core speeds. No memory bus transactions occur until that line is evicted from L1.

---

## Section 4: Conflict Thrashing and Cache Lab Insights

### Question 4.1 Answer
- Capacity Miss: The working set size exceeds total cache capacity $C$. Resolved by **loop tiling**, which shrinks the active working set to fit inside $C$.
- Conflict Miss: The working set size is smaller than $C$, but multiple active variables map to the same set index. Resolved by **increasing associativity $E$** or adding array padding.

### Question 4.2 Answer
Critical stride is $32 \times 32 = 1024 = \text{0x400}$ bytes. The distance between `0x10000` and `0x10400` is exactly $1024$ bytes. Their set index bits `(addr >> 5) & 0x1F` are identical. In a direct-mapped cache ($E = 1$), loading one line immediately evicts the other.

### Question 4.3 Answer
In Cache Lab, arrays $A$ and $B$ are separated by an exact multiple of 1024 bytes, meaning $A[i][j]$ and $B[i][j]$ map to the exact same cache set. On a diagonal block ($bi == bj$), reading $A[i][i]$ and writing $B[i][i]$ collide in the same set, evicting $A$'s cache line before the inner loop can read the next column element $A[i][i+1]$. Staging row elements into 8 scalar registers reads all of $A$'s line at once before writing $B$.

---

## Section 5: Matrix Multiplication and Tiling

### Question 5.1 Answer
In `ijk`, the inner loop iterates over $k$, scanning down columns of matrix $B$ (Stride-$N$, $100\%$ misses). In `ikj`, the inner loop iterates over $j$, scanning along rows of both $B$ and $C$ with Stride-1 ($12.5\%$ misses).

### Question 5.2 Answer
To hold one active tile of $A$, one active tile of $B$, and one active tile of $C$ in L1 cache simultaneously:
$$3 \times T^2 \times \text{sizeof}(\text{double}) \le C_{\text{L1}}$$
$$T \le \sqrt{\frac{C_{\text{L1}}}{3 \times \text{sizeof}(\text{double})}}$$

### Question 5.3 Answer
IEEE 754 floating-point addition is commutative but not associative: $(a + b) + c \neq a + (b + c)$. Intermediate rounding differences alter the lowest mantissa bits when summation order changes. Verification must use a bounded tolerance ($|A - B| < 10^{-9}$).
