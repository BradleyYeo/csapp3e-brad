# The Memory Hierarchy: Data Layout and Cache Locality

## Mental Models and Theoretical Foundations

### Data-Oriented Design vs. Object-Oriented Structures
- In object-oriented programming, data is organized around entities (Array of Structs / AoS):
  ```go
  type User struct {
      id       UserId       // 8 bytes
      name     string       // 16 bytes (ptr + len)
      age      int          // 8 bytes
      address  Address      // 32 bytes
      payments []Payment    // 24 bytes
  }
  ```
- Each `User` object occupies $\ge 88$ bytes in memory.
- If an analytics algorithm only needs to compute `AverageAge`, iterating through `[]*User` loads the entire 88-byte struct (and follows heap pointers) across multiple 64-byte cache lines, wasting $\approx 90\%$ of memory bandwidth on unused address and name fields.

### Struct of Arrays (SoA) and Cache Line Packing
- Reorganizing memory layout into columnar arrays (Struct of Arrays / SoA):
  ```go
  type UserData struct {
      ages     []uint8      // 1 byte per user
      payments []uint32     // 4 bytes per payment (in cents)
  }
  ```
- In this columnar layout, a single 64-byte cache line holds the ages of **64 consecutive users**!
- Memory access is 100% efficient Stride-1 sequential streaming without pointer chasing or hash table lookups.

### Eliminating Integer Divisions in Hot Loops
- Integer division (`/`) is expensive on modern hardware ($10$–$40$ clock cycles latency).
- Pre-converting monetary amounts to integer cents (or scaling by multiplying by the reciprocal $0.01$) eliminates millions of division instructions in aggregation loops.

## Textbook Reference
- **CS:APP3e**: Chapter 6 (*The Memory Hierarchy*), Section 6.4 (*Cache Memories*) and Section 6.5 (*Writing Cache-Friendly Code*).

## Problem Set and Specifications

### Exercise: `metrics/` Optimization
- Directory: [metrics/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/07-memory-hierarchy-data-layout/metrics)
- Data files:
  - `users.csv`: Contains 100,000 user records (name, age, address, etc.).
  - `payments.csv`: Contains millions of monetary transaction records.
- Benchmark targets (`go test -bench=.`):
  1. `AverageAge(users)`: Computes mean user age.
  2. `AveragePaymentAmount(users)`: Computes mean transaction amount.
  3. `StdDevPaymentAmount(users)`: Computes standard deviation of transaction amounts:
     $$\sigma = \sqrt{E[X^2] - (E[X])^2}$$

### Optimization Milestones
- **Baseline (AoS Map)**:
  - Storing users in `map[UserId]*User` and chasing pointers.
  - Runtime: $\approx 1.2 \text{ ms}$ for age, $\approx 13.6 \text{ ms}$ for payments.
- **Milestone 1: Flat Slices**:
  - Extract `ages` into `[]int` and payments into flat slices. Eliminates hash map lookups.
  - Runtime drops by $2.5\times$.
- **Milestone 2: Struct of Arrays (`UserData`)**:
  - Replace large struct definitions with columnar `UserData{ages []uint8, amounts []uint32}`.
  - 64 user ages fit per cache line.
- **Milestone 3: Division Hoisting**:
  - Perform arithmetic accumulation in integer cents; divide once by total count at the end.
- **Milestone 4: Multiple Accumulators**:
  - Use 4 parallel accumulators (`sum0, sum1, sum2, sum3`) to break loop-carried RAW dependencies.
  - Final runtime: $\approx 25 \mu s$ for age ($47\times$ speedup), $\approx 244 \mu s$ for payments ($55\times$ speedup).

## Build and Verification
```bash
cd metrics
go test -v -bench=. -benchmem
```

## Active Recall Checkpoint
- How does pointer indirection (e.g. `[]*User` vs `[]User`) degrade CPU hardware prefetcher efficiency?
- Why does packing 1-byte `uint8` ages instead of 8-byte `int`s increase cache hit rates by $8\times$?
- How does the variance formula $\text{Var}[X] = E[X^2] - (E[X])^2$ enable single-pass standard deviation calculation compared to the two-pass $\sum (x_i - \mu)^2$ formula?
