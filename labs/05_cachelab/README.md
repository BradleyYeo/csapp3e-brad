# Environment Setup and Prerequisites

## System Architecture Requirements

- Architecture: 64-bit x86-64 Linux environment is required.
  - [csim-ref](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/csim-ref) and [test-csim](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/test-csim) are precompiled 64-bit ELF binaries.
  - [test-trans.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/test-trans.c#L71) requires Valgrind with the `lackey` tool (`valgrind --tool=lackey --trace-mem=yes`) to generate memory traces.
- C Compiler: GCC supporting the C23 standard (`-std=c23`) specified in [Makefile](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/Makefile#L6) (GCC 14 or newer).
- Python Runtime: Python 3 (or Python 2) for executing [driver.py](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/driver.py).

## Container Setup on macOS (Colima / Docker)

### Starting the x86-64 VM via Colima

```bash
colima start --arch x86_64 --cpu 2 --memory 4
```

### Building the Cache Lab Container Image

- Build using the local [Dockerfile](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/Dockerfile):
  ```bash
  cd /Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab
  docker build -t cachelab-env .
  ```

### Launching the Build and Test Container

```bash
docker run -it --rm \
  -v "$(pwd)/cachelab-handout":/app \
  cachelab-env bash
```

---

# Build Instructions

## Building All Targets

- Change directory to `cachelab-handout`:
  ```bash
  cd cachelab-handout
  ```
- Compile the cache simulator, transpose test harness, and trace generator:
  ```bash
  make clean && make
  ```

## Output Binaries

- `csim`: The cache simulator compiled from [csim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/csim.c).
- `test-trans`: The transpose correctness and performance testing binary compiled from [test-trans.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/test-trans.c) and [trans.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/trans.c).
- `tracegen`: Memory trace generation binary driven by Valgrind lackey.
- `${USER}-handin.tar`: Submission tar archive containing `csim.c` and `trans.c`.

---

# Testing and Verification Workflows

## Part A: Cache Simulator Testing (`csim.c`)

### Progressive Unit Tests (Run Natively on macOS)

- Run each stage once you finish the matching TODO in `csim.c`. Each stage also re-runs all earlier stages:

| Command | TODO covered | What it checks |
| :--- | :--- | :--- |
| `make test STAGE=1` | Structs + constants | Field names, `tag` is `uint64_t`, `CACHE_MISS`/`CACHE_EVICT` are distinct bits |
| `make test STAGE=2` | `cache_init` | Geometry stored, `2^s` sets × `E` lines allocated (ASan catches undersized mallocs), all lines invalid |
| `make test STAGE=3` | `cache_destroy` | No double or invalid frees across 1000 init/destroy cycles |
| `make test STAGE=4` | `cache_access` | Set/tag decoding, offset ignored, cold miss vs eviction, filling every way before evicting, LRU order, hits refreshing recency |
| `make test STAGE=5` | `main` loop | All 8 graded traces, `I` lines ignored, `M` = 2 accesses, `-v` output format |

- Leak check on native macOS (using `/usr/bin/leaks`):
  ```bash
  make test-leaks STAGE=3
  ```
- Leak check via Linux container (Valgrind Memcheck):
  ```bash
  # From labs/05_cachelab directory:
  docker run --rm -v "$(pwd)/cachelab-handout":/app cachelab-env make test-memcheck STAGE=3

  # Or inside the interactive container shell:
  make test-memcheck STAGE=3
  ```
- Test sources: [tests/test_csim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/tests/test_csim.c) and [tests/run_traces.sh](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/tests/run_traces.sh).

### Manual Trace Verification Against Reference Simulator

- Run reference simulator on `traces/yi.trace`:
  ```bash
  ./csim-ref -v -s 4 -E 1 -b 4 -t traces/yi.trace
  ```
- Run student simulator on `traces/yi.trace`:
  ```bash
  ./csim -v -s 4 -E 1 -b 4 -t traces/yi.trace
  ```
- Verify outputs match bit-for-bit:
  ```bash
  diff <(./csim -v -s 4 -E 1 -b 4 -t traces/yi.trace) \
       <(./csim-ref -v -s 4 -E 1 -b 4 -t traces/yi.trace)
  ```

### Automated Part A Correctness Suite

- Run the autograder test suite across all reference traces:
  ```bash
  ./test-csim
  ```
- Expected output:
  ```
  Points: 27/27
  TEST_CSIM_RESULTS=27
  ```

### Memory Leak and Address Sanitization

- Verify zero memory leaks and clean destruction with Valgrind Memcheck:
  ```bash
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
    ./csim -s 4 -E 2 -b 4 -t traces/yi.trace
  ```

---

## Part B: Matrix Transpose Testing (`trans.c`)

### Individual Matrix Evaluations

- Evaluate $32 \times 32$ matrix transpose:
  ```bash
  ./test-trans -M 32 -N 32
  ```
- Evaluate $64 \times 64$ matrix transpose:
  ```bash
  ./test-trans -M 64 -N 64
  ```
- Evaluate $61 \times 67$ matrix transpose:
  ```bash
  ./test-trans -M 61 -N 67
  ```

### Transpose Trace Debugging Workflow

- If a matrix size exceeds miss thresholds, trace the exact memory references:
  ```bash
  ./test-trans -M 32 -N 32
  ./csim-ref -v -s 5 -E 1 -b 5 -t trace.f0 | head -n 50
  ```
- `trace.f0` contains the memory operations generated by the submitted transpose function (`func 0`).

---

## Full End-to-End Evaluation (`driver.py`)

### Running the Complete Driver

```bash
./driver.py
```

- If `./driver.py` complains about Python path, invoke explicitly via Python 2:
  ```bash
  python2 driver.py
  ```

### Target Scoring Rubric

- Maximum total score: 53.0 points.
- Summary targets:

| Benchmark | Max Points | Target Miss Threshold | Full Credit Condition |
| :--- | :--- | :--- | :--- |
| `csim` correctness | 27.0 | N/A | Passes all 8 reference traces |
| $32 \times 32$ transpose | 8.0 | $< 300$ misses | $< 300$ misses (0 pts if $\ge 600$) |
| $64 \times 64$ transpose | 8.0 | $< 1,300$ misses | $< 1,300$ misses (0 pts if $\ge 2,000$) |
| $61 \times 67$ transpose | 10.0 | $< 2,000$ misses | $< 2,000$ misses (0 pts if $\ge 3,000$) |

### Verified Results

```
Cache Lab summary:
                        Points   Max pts      Misses
Csim correctness          27.0        27
Trans perf 32x32           8.0         8         287
Trans perf 64x64           8.0         8        1139
Trans perf 61x67          10.0        10        1992
          Total points    53.0        53
```

---

# Matrix Transpose Optimization Analysis (64x64)

## Direct-Mapped Cache Aliasing Constraints

- Cache configuration: $(s = 5, E = 1, b = 5) \implies 32$ sets, direct-mapped, 32 bytes per block.
- Capacity: $1024$ bytes ($256$ integers).
- In a $64 \times 64$ matrix of 4-byte integers:
  - 1 row $= 64 \times 4 = 256$ bytes $= 8$ cache lines.
  - The cache holds $32 / 8 = 4$ rows total.
  - Row $i$ and row $i + 4$ map to the exact same cache sets.

## Step-by-Step Quadrant Transpose Walkthrough

### Step 0: Initial State

```
                                      Block A
                            +-------------------------+
                            | 00 01 02 03 04 05 06 07 |
                            | 10 11 12 13 14 15 16 17 |
                            | 20 21 22 23 24 25 26 27 |
                            | 30 31 32 33 34 35 36 37 |
                            +-------------------------+
                            | 40 41 42 43 44 45 46 47 |
                            | 50 51 52 53 54 55 56 57 |
                            | 60 61 62 63 64 65 66 67 |
                            | 70 71 72 73 74 75 76 77 |
                            +-------------------------+
+-------------------------+
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
+-------------------------+
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
+-------------------------+
          Block B
```

### Step 1: Staging Top-Right of A into Top-Right of B

- Load `A[0, 0:8]` into registers.
- Store `A[0, 0:4]` into `B[0:4, 0]` and `A[0, 4:8]` into `B[0:4, 4]`.

```
                                      Block A
                            +-------------------------+
                            | -- -- -- -- -- -- -- -- |
                            | 10 11 12 13 14 15 16 17 |
                            | 20 21 22 23 24 25 26 27 |
                            | 30 31 32 33 34 35 36 37 |
                            +-------------------------+
                            | 40 41 42 43 44 45 46 47 |
                            | 50 51 52 53 54 55 56 57 |
                            | 60 61 62 63 64 65 66 67 |
                            | 70 71 72 73 74 75 76 77 |
                            +-------------------------+
+-------------------------+
| 00 xx xx xx 04 xx xx xx |
| 01 xx xx xx 05 xx xx xx |
| 02 xx xx xx 06 xx xx xx |
| 03 xx xx xx 07 xx xx xx |
+-------------------------+
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
+-------------------------+
          Block B
```

### Step 2: Processing Remaining Top Half Rows of A

- Repeat Step 1 on rows 1 through 3 of block A.

```
                                      Block A
                            +-------------------------+
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            +-------------------------+
                            | 40 41 42 43 44 45 46 47 |
                            | 50 51 52 53 54 55 56 57 |
                            | 60 61 62 63 64 65 66 67 |
                            | 70 71 72 73 74 75 76 77 |
                            +-------------------------+
+-------------------------+
| 00 10 20 30 04 14 24 34 |
| 01 11 21 31 05 15 25 35 |
| 02 12 22 32 06 16 26 36 |
| 03 13 23 33 07 17 27 37 |
+-------------------------+
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
+-------------------------+
          Block B
```

### Step 3: Columnar Register Handoff

- Load column `A[4:8, 0]` into registers.
- Read parked row `B[0, 4:8]` into registers.
- Overwrite `B[0, 4:8]` with `A[4:8, 0]`.
- Load `A[4:8, 4]` into registers.
- Store original `B[0, 4:8]` into `B[4, 0:4]` and `A[4:8, 4]` into `B[4, 4:8]`.

```
                                      Block A
                            +-------------------------+
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            +-------------------------+
                            | -- 41 42 43 -- 45 46 47 |
                            | -- 51 52 53 -- 55 56 57 |
                            | -- 61 62 63 -- 65 66 67 |
                            | -- 71 72 73 -- 75 76 77 |
                            +-------------------------+
+-------------------------+
| 00 10 20 30 40 50 60 70 |
| 01 11 21 31 05 15 25 35 |
| 02 12 22 32 06 16 26 36 |
| 03 13 23 33 07 17 27 37 |
+-------------------------+
| 04 14 24 34 44 54 64 74 |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
| xx xx xx xx xx xx xx xx |
+-------------------------+
          Block B
```

### Step 4: Completing Lower Quadrants

- Repeat Step 3 across the remaining columns `j = 1, 2, 3`.

```
                                      Block A
                            +-------------------------+
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            +-------------------------+
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            | -- -- -- -- -- -- -- -- |
                            +-------------------------+
+-------------------------+
| 00 10 20 30 40 50 60 70 |
| 01 11 21 31 41 51 61 71 |
| 02 12 22 32 42 52 62 72 |
| 03 13 23 33 43 53 63 73 |
+-------------------------+
| 04 14 24 34 44 54 64 74 |
| 05 15 25 35 45 55 65 75 |
| 06 16 26 36 46 56 66 76 |
| 07 17 27 37 47 57 67 77 |
+-------------------------+
          Block B
```
