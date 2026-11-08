# Concepts: The Memory Mountain (CS:APP Section 6.6.1)

## What Is the Memory Mountain?

The Memory Mountain is a comprehensive characterization of a computer system's memory hierarchy. It measures read throughput (in MB/s) as a function of two independent variables:
- **Temporal Locality Axis (Working Set Size)**: Sweeps buffer sizes from $4\text{ KiB}$ to $64\text{ MiB}$.
- **Spatial Locality Axis (Stride)**: Sweeps step intervals from Stride-1 to Stride-32.

```
Throughput (MB/s)
  ^
  |  [ L1 Ridge ] (~64 KiB)
  |      \
  |       [ L2 Ridge ] (~4 MiB)
  |           \
  |            [ Main Memory Plateau ] (>4 MiB)
  +----------------------------------------------------> Working Set Size
```

## Interpreting the Ridges and Slopes

### The Working Set Ridges (Temporal Locality)
- L1 Ridge: When the buffer fits entirely in the L1 data cache ($\le 64\text{ KiB}$ on Apple M1 Pro), throughput reaches maximum speed ($> 50,000\text{ MB/s}$).
- L2 Ridge: When the working set exceeds L1 but fits in L2 ($64\text{ KiB} \dots 4\text{ MiB}$), throughput drops to L2 speeds.
- Main Memory Plateau: When the working set exceeds L2 ($> 4\text{ MiB}$), reads miss in all SRAM caches and stall on off-chip DRAM.

### The Stride Slopes (Spatial Locality)
- At Stride-1, every 128-byte cache line loaded from memory supplies 16 consecutive 8-byte elements ($100\%$ spatial utilization). Hardware stream prefetchers detect sequential access and preload subsequent lines before the CPU requests them.
- As stride increases to 16 or 32, fewer elements are used per loaded cache line. At Stride-16 ($128$ bytes), every single access touches a distinct cache line, wasting memory bus bandwidth and dropping throughput.

---

# Task: Implement and Run the Memory Mountain

- Target File: [10_memory_mountain.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/10_memory_mountain.c)

## Implementation Details

- Implement the read kernel:
  ```c
  uint64_t run_mountain_pass(uint64_t *data, size_t elems, size_t stride);
  ```
- Use high-precision monotonic timing (`clock_gettime(CLOCK_MONOTONIC)`) to calculate throughput:
  $$\text{Throughput} = \frac{\text{Bytes Read}}{\text{Elapsed Seconds} \times 10^6} \text{ MB/s}$$
- Execute the sweep across sizes $4\text{ KiB} \dots 64\text{ MiB}$ and strides $1 \dots 32$.

---

# Predict Before Running

- Host: Apple M1 Pro (`sysctl -n hw.l1dcachesize hw.l2cachesize` $\implies 64\text{ KiB}$, $4\text{ MiB}$).
  - Sizes $4\text{ KB} \dots 64\text{ KB}$: L1 peak throughput ($> 40,000\text{ MB/s}$ at Stride-1).
  - Sizes $128\text{ KB} \dots 4\text{ MB}$: L2 throughput ($\approx 20,000\text{--}30,000\text{ MB/s}$).
  - Sizes $8\text{ MB} \dots 64\text{ MB}$: DRAM plateau ($< 15,000\text{ MB/s}$).

---

# Active Recall Checkpoints

- Why does throughput decrease as stride increases from 1 to 16, even when the buffer size is held constant?
- At what exact working set size does read throughput drop off the L1 ridge on your machine?
- What role does the CPU hardware stream prefetcher play along the Stride-1 ridge?

---

# Navigation Links

- Next Exercise: [11_self_test.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/11_self_test.md)
- Reference Solution: [reference-solution/10_memory_mountain.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/10_memory_mountain.c)
