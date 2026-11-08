# Curriculum Context and Architectural Connections

## How Caching Connects Across the Workspace

- Pointer Stride Scaling: In [08_pointer_arithmetic_strides.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/08_pointer_arithmetic_strides.md), you established that advancing a pointer `T *p` by `n` elements scales the byte address by `n * sizeof(T)`. 2D row-major matrices map `(r, c)` to $\text{Base} + (r \times \text{cols} + c) \times \text{sizeof}(T)$. In this module, you study how hardware cache controllers physically react to those byte strides.
- Power-of-Two Bitwise Masks: In [11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md), you used `(addr + (a - 1)) & ~(a - 1)` to align addresses to power-of-two boundaries. Hardware caches partition 64-bit physical addresses using the exact same power-of-two bitmask algebra (`offset`, `set_index`, `tag`).
- Pipelining and Functional Unit Utilization: In [19_pipelining_hazards.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/19_pipelining_hazards.md) and [05-low-level-optimization](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization/README.md), you broke Read-After-Write (RAW) data dependency chains using loop unrolling and independent accumulators. However, superscalar arithmetic throughput is useless if the Load/Store Unit stalls for $200$ clock cycles waiting on a DRAM fetch.
- Dynamic Binary Instrumentation: In [06_valgrind_fundamentals.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_valgrind_fundamentals.md), you examined how Valgrind translates machine instructions into VEX IR. Valgrind's `cachegrind` tool intercepts every instruction fetch, data load, and data store across this VEX IR stream to simulate an L1/LL cache hierarchy.
- Cache Lab Integration: In [labs/05_cachelab/CONCEPTS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/CONCEPTS.md), you construct a trace-driven simulator and optimize matrix transpositions. The exercises in this directory build the foundational mental models, bitwise operators, and quantitative miss-prediction skills required for Cache Lab.

---

# Software Design Principles in Hardware Caching

## John Ousterhout: Deep Modules

- Transparent Hardware Interface: A CPU cache provides an interface with zero new instructions for normal user software. A standard `movq (%rdi), %rax` operates identically whether data resides in L1 SRAM or off-chip DRAM.
- Encapsulated Complexity: Behind that zero-surface interface, the cache controller manages address tag decoding, multi-way parallel comparator banks, dirty-bit write-back queues, Miss Status Holding Registers (MSHRs), and hardware stream prefetchers.

## Daniel Jackson: Concept Decomposition

- Concept 1 — Spatial Block Transfer (`CacheLine`): Memory is never transferred between DRAM and SRAM as individual bytes; it moves in fixed-size aligned blocks of $B = 2^b$ bytes (64 bytes on x86-64, 128 bytes on Apple Silicon).
- Concept 2 — Modulo Set Partitioning (`CacheSet`): The $S = 2^s$ sets partition the physical address space via middle bits so contiguous $B$-byte blocks distribute across independent SRAM rows.
- Concept 3 — Associative Way Lookup (`CacheWay`): Within a selected set, $E$ parallel lines allow up to $E$ distinct memory blocks with the same set index to coexist without mutual eviction.
- Concept 4 — Replacement Policy (`LRUState`): When all $E$ ways in a set are valid and a new block maps to that set, a deterministic policy (Least Recently Used) selects the victim line.

## Jimmy Koppel: Single Authority and Representation Invariants

- Single Authority for Block Identity: Because the set index $s$ and block offset $b$ are encoded in the physical location within the cache array (`sets[index]`), only the upper $t = m - (s + b)$ tag bits are stored in `line.tag`. The tuple `(set_index, line.tag)` is the single authority identifying which main memory block occupies the line.
- Validity Invariant: Uninitialized SRAM cells contain arbitrary voltage states (`tag == 0` can match physical address `0x0`). A line contains meaningful data if and only if `line.valid == true`.
- Dirty Invariant (Write-Back Caches): Main memory is stale relative to a cache line if and only if `line.valid == true && line.dirty == true`. Evicting a dirty line requires an explicit write-back transaction to lower memory before overwriting the line.

---

# The Memory Wall and Storage Hierarchy

## Latency and Capacity Tradeoffs Across Tiers

- CPU Registers: Capacity $\sim 1\text{ KiB}$, latency $0\text{ cycles}$ ($< 1\text{ ns}$).
- L1 Data & Instruction Cache: Capacity $32\text{--}64\text{ KiB}$ per core, latency $\sim 4\text{ cycles}$ ($\approx 1.2\text{ ns}$).
- L2 Unified Cache: Capacity $256\text{ KiB}\text{--}4\text{ MiB}$ per core, latency $\sim 12\text{ cycles}$ ($\approx 4\text{ ns}$).
- L3 Shared Last-Level Cache (LLC): Capacity $8\text{--}64\text{ MiB}$ shared, latency $\sim 40\text{--}50\text{ cycles}$ ($\approx 15\text{ ns}$).
- Main Memory (DRAM): Capacity $16\text{--}512\text{ GiB}$, latency $\sim 200+\text{ cycles}$ ($60\text{--}80\text{ ns}$).
- Persistent Storage (NVMe SSD): Capacity $1\text{--}8\text{ TiB}$, latency $\sim 100,000+\text{ cycles}$ ($30\text{--}100\ \mu\text{s}$).

## Physical Origin of the Latency Hierarchy

### Static RAM (SRAM — L1, L2, L3 Caches)

- Constructed from 6-transistor (6T) bistable flip-flop latches per bit.
- Retains state as long as power is maintained without refresh cycles.
- Fast switching speeds ($1\text{--}15\text{ ns}$), but low density and high silicon area cost.

### Dynamic RAM (DRAM — Main Memory)

- Constructed from 1 capacitor and 1 access transistor (1T1C) per bit.
- High density ($10\times\text{--}20\times$ denser than SRAM), but reading discharges the capacitor (requiring immediate rewrite) and charge leaks over time (requiring periodic refresh intervals).
- Physical wire distance and memory bus serialization impose a $\sim 200$-cycle latency wall.
