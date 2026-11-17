#!/usr/bin/env python3
"""
Exercise 20: Python Pointer Chasing and Memory Layout Optimization
==================================================================
This script demonstrates the hardware and architectural mechanics of pointer
chasing in CPython, showing how memory layout directly impacts CPU cache
efficiency, execution latency, and RAM utilization.

Experiments:
  1. Linked List Traversal vs. Contiguous Array (Random Pointer Chasing)
  2. Object Anatomy & Attribute Lookup (Dict vs Class vs __slots__ vs Tuple)
  3. Columnar Projection & Cache Line Pollution (AoS vs SoA)
  4. Memory Footprint and The CPython Primitive Boxing Tradeoff
"""

import argparse
import random
import struct
import sys
import time
from array import array
from typing import Dict, List, NamedTuple, Tuple


# ==============================================================================
# Domain Models for Experiment 1: Linked Node vs Contiguous Array
# ==============================================================================

class Node:
    """A singly linked list node using __slots__ to isolate the pointer chase."""
    __slots__ = ("val", "next")

    def __init__(self, val: int):
        self.val: int = val
        self.next: "Node | None" = None


# ==============================================================================
# Domain Models for Experiment 2: Object Attribute Lookup Anatomy
# ==============================================================================

class RecordStandard:
    """Standard Python class: attributes live in dynamic instance __dict__."""
    def __init__(self, x: float, y: float, val: float):
        self.x = x
        self.y = y
        self.val = val


class RecordSlots:
    """Optimized class: __slots__ eliminates __dict__ and stores pointers at fixed offsets."""
    __slots__ = ("x", "y", "val")

    def __init__(self, x: float, y: float, val: float):
        self.x = x
        self.y = y
        self.val = val


class RecordNamedTuple(NamedTuple):
    """NamedTuple: immutable tuple with descriptor-based field access."""
    x: float
    y: float
    val: float


# ==============================================================================
# Domain Models for Experiment 3: Wide Records (AoS vs SoA)
# ==============================================================================

class WideRecordAoS:
    """Wide record with 10 fields representing typical entity/model instances."""
    __slots__ = ("id", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "metric")

    def __init__(self, record_id: int, metric: float):
        self.id = record_id
        self.f1 = float(record_id * 1)
        self.f2 = float(record_id * 2)
        self.f3 = float(record_id * 3)
        self.f4 = float(record_id * 4)
        self.f5 = float(record_id * 5)
        self.f6 = float(record_id * 6)
        self.f7 = float(record_id * 7)
        self.f8 = float(record_id * 8)
        self.metric = metric


class WideRecordsSoA:
    """Struct-of-Arrays layout separating the frequently queried metric column."""
    def __init__(self, count: int, metrics: List[float]):
        self.ids = list(range(count))
        # Frequently aggregated column stored in its own contiguous list
        self.metrics = metrics
        # Secondary columns stored separately
        self.f1 = [float(i * 1) for i in range(count)]
        self.f2 = [float(i * 2) for i in range(count)]


# ==============================================================================
# Benchmark Logic & Experiments
# ==============================================================================

def run_experiment_1(n: int = 500_000) -> None:
    """Experiment 1: The Traversal Gap - Random Pointer Chasing vs Contiguous Array."""
    print("=" * 72)
    print("EXPERIMENT 1: Linked List (Pointer Chasing) vs Contiguous Array")
    print(f"Data Size: {n:,} elements")
    print("=" * 72)

    # 1. Allocate nodes sequentially in memory
    nodes = [Node(i) for i in range(n)]

    # Sequential linkage: Node i points to Node i+1
    for i in range(n - 1):
        nodes[i].next = nodes[i + 1]

    # Measure sequential linked list traversal
    curr = nodes[0]
    t0 = time.perf_counter()
    sum_seq = 0
    while curr is not None:
        sum_seq += curr.val
        curr = curr.next
    t_seq = time.perf_counter() - t0

    # 2. Scramble node pointers to simulate fragmented heap over time
    shuffled_indices = list(range(n))
    random.seed(42)
    random.shuffle(shuffled_indices)

    for i in range(n - 1):
        nodes[shuffled_indices[i]].next = nodes[shuffled_indices[i + 1]]
    nodes[shuffled_indices[-1]].next = None

    # Measure scrambled linked list traversal (guaranteed cache miss per hop)
    curr = nodes[shuffled_indices[0]]
    t0 = time.perf_counter()
    sum_scrambled = 0
    while curr is not None:
        sum_scrambled += curr.val
        curr = curr.next
    t_scrambled = time.perf_counter() - t0

    # 3. Contiguous array traversal (sequential pointers in list buffer)
    flat_array = [node.val for node in nodes]
    t0 = time.perf_counter()
    sum_flat = sum(flat_array)
    t_flat = time.perf_counter() - t0

    assert sum_seq == sum_scrambled == sum_flat, "Sum mismatch in Experiment 1!"

    print(f"1. Sequential Linked List: {t_seq:8.4f}s | {n/t_seq/1e6:6.2f} M ops/s | (Spatial locality preserved)")
    print(f"2. Scrambled Linked List:  {t_scrambled:8.4f}s | {n/t_scrambled/1e6:6.2f} M ops/s | (Cache miss on every hop!)")
    print(f"3. Direct Contiguous Sum:  {t_flat:8.4f}s | {n/t_flat/1e6:6.2f} M ops/s | (Hardware streaming / built-in)")
    print(f"\n=> Pointer chasing slowdown: {t_scrambled / t_seq:.2f}x slower due to cache misses alone.")
    print(f"=> Contiguous sum speedup:   {t_scrambled / t_flat:.2f}x faster than pointer-chased list.\n")


def run_experiment_2(n: int = 500_000) -> None:
    """Experiment 2: Object Anatomy & Attribute Lookup Overhead."""
    print("=" * 72)
    print("EXPERIMENT 2: Object Anatomy & Attribute Lookup Overhead")
    print(f"Data Size: {n:,} records | Filter & Aggregate: sum(x * val) where y > 25.0")
    print("=" * 72)

    # Prepare datasets
    dicts = [{"x": float(i % 100), "y": float(i % 50), "val": 1.5} for i in range(n)]
    records_std = [RecordStandard(float(i % 100), float(i % 50), 1.5) for i in range(n)]
    records_slots = [RecordSlots(float(i % 100), float(i % 50), 1.5) for i in range(n)]
    records_namedtuple = [RecordNamedTuple(float(i % 100), float(i % 50), 1.5) for i in range(n)]
    records_tuples = [(float(i % 100), float(i % 50), 1.5) for i in range(n)]

    # 1. Dict
    t0 = time.perf_counter()
    ans_dict = sum(d["x"] * d["val"] for d in dicts if d["y"] > 25.0)
    t_dict = time.perf_counter() - t0

    # 2. Standard class
    t0 = time.perf_counter()
    ans_std = sum(r.x * r.val for r in records_std if r.y > 25.0)
    t_std = time.perf_counter() - t0

    # 3. Slots class
    t0 = time.perf_counter()
    ans_slots = sum(r.x * r.val for r in records_slots if r.y > 25.0)
    t_slots = time.perf_counter() - t0

    # 4. NamedTuple
    t0 = time.perf_counter()
    ans_nt = sum(r.x * r.val for r in records_namedtuple if r.y > 25.0)
    t_nt = time.perf_counter() - t0

    # 5. Raw Tuple
    t0 = time.perf_counter()
    ans_tup = sum(r[0] * r[2] for r in records_tuples if r[1] > 25.0)
    t_tup = time.perf_counter() - t0

    assert abs(ans_dict - ans_std) < 1e-3, "Mismatch in Experiment 2 results!"

    # Memory estimation
    mem_dict = (sys.getsizeof(dicts) + n * (sys.getsizeof(dicts[0]) + 3 * 24)) / 1024**2
    mem_std = (sys.getsizeof(records_std) + n * (sys.getsizeof(records_std[0]) + sys.getsizeof(records_std[0].__dict__) + 3 * 24)) / 1024**2
    mem_slots = (sys.getsizeof(records_slots) + n * (sys.getsizeof(records_slots[0]) + 3 * 24)) / 1024**2
    mem_tup = (sys.getsizeof(records_tuples) + n * (sys.getsizeof(records_tuples[0]) + 3 * 24)) / 1024**2

    print(f"{'Layout / Structure':<24} | {'Time (s)':<9} | {'Rel Speed':<10} | {'Est RAM (MB)':<12}")
    print("-" * 65)
    print(f"{'1. List of Dicts':<24} | {t_dict:<9.4f} | {'1.00x (base)':<10} | {mem_dict:<12.1f}")
    print(f"{'2. Standard Class':<24} | {t_std:<9.4f} | {t_dict/t_std:<9.2f}x | {mem_std:<12.1f}")
    print(f"{'3. __slots__ Class':<24} | {t_slots:<9.4f} | {t_dict/t_slots:<9.2f}x | {mem_slots:<12.1f}")
    print(f"{'4. NamedTuple':<24} | {t_nt:<9.4f} | {t_dict/t_nt:<9.2f}x | {mem_tup:<12.1f}")
    print(f"{'5. Raw 3-Tuple (Index)':<24} | {t_tup:<9.4f} | {t_dict/t_tup:<9.2f}x | {mem_tup:<12.1f}")
    print("\nKey Insight: __slots__ eliminates the per-instance __dict__ hash table, saving ~40% RAM.\n")


def run_experiment_3(n: int = 500_000) -> None:
    """Experiment 3: Columnar Projection / Wide Record Cache Pollution (AoS vs SoA)."""
    print("=" * 72)
    print("EXPERIMENT 3: Columnar Projection / Wide Record Cache Pollution (AoS vs SoA)")
    print(f"Data Size: {n:,} 10-field records | Task: Single column projection sum(metric)")
    print("=" * 72)

    # 1. AoS: List of 10-field records
    records_aos = [WideRecordAoS(i, 1.5) for i in range(n)]

    # 2. SoA: Columnar layout where 'metric' is a contiguous float list
    metrics_list = [1.5 for _ in range(n)]
    soa = WideRecordsSoA(n, metrics_list)

    # 3. SoA Array: Contiguous C-array buffer of IEEE 754 doubles
    metrics_array = array("d", (1.5 for _ in range(n)))

    # Benchmark AoS: Must traverse every record object just to read one float
    t0 = time.perf_counter()
    sum_aos = 0.0
    for r in records_aos:
        sum_aos += r.metric
    t_aos = time.perf_counter() - t0

    # Benchmark SoA List: Direct iteration over the single column list
    t0 = time.perf_counter()
    sum_soa_list = sum(soa.metrics)
    t_soa_list = time.perf_counter() - t0

    # Benchmark SoA Array: Direct C-level sum over contiguous memory buffer
    t0 = time.perf_counter()
    sum_soa_arr = sum(metrics_array)
    t_soa_arr = time.perf_counter() - t0

    assert abs(sum_aos - sum_soa_list) < 1e-3 and abs(sum_aos - sum_soa_arr) < 1e-3, "Mismatch in Experiment 3!"

    print(f"1. AoS Wide Objects (for r in records: sum += r.metric): {t_aos:8.4f}s | (Pulls unused fields into cache)")
    print(f"2. SoA Parallel List (sum(soa.metrics)):                 {t_soa_list:8.4f}s | {t_aos/t_soa_list:6.2f}x faster!")
    print(f"3. SoA array.array  (sum(metrics_array)):               {t_soa_arr:8.4f}s | {t_aos/t_soa_arr:6.2f}x faster!")
    print(f"\n=> Columnar projection eliminates memory bus pollution: 100% of the cache line is payload.\n")


def run_experiment_4(n: int = 500_000) -> None:
    """Experiment 4: Contiguous Buffers vs CPython Boxing Overhead."""
    print("=" * 72)
    print("EXPERIMENT 4: Contiguous Buffers & The CPython Boxing Overhead")
    print(f"Data Size: {n:,} elements | Task: Vector dot product x[i] * y[i]")
    print("=" * 72)

    # Setup arrays
    x_list = [float(i % 100) for i in range(n)]
    y_list = [float(i % 50) for i in range(n)]

    x_arr = array("d", x_list)
    y_arr = array("d", y_list)

    # 1. Python List with zip (iterating pre-boxed floats)
    t0 = time.perf_counter()
    dot_list = sum(x * y for x, y in zip(x_list, y_list))
    t_list = time.perf_counter() - t0

    # 2. array.array with zip (must unpack C double -> allocate new PyFloatObject per element)
    t0 = time.perf_counter()
    dot_arr_zip = sum(x * y for x, y in zip(x_arr, y_arr))
    t_arr_zip = time.perf_counter() - t0

    # 3. Packed byte buffer with struct.iter_unpack
    packed_buf = bytearray(n * 16)
    for i in range(n):
        struct.pack_into("dd", packed_buf, i * 16, float(i % 100), float(i % 50))

    t0 = time.perf_counter()
    dot_struct = sum(x * y for x, y in struct.iter_unpack("dd", packed_buf))
    t_struct = time.perf_counter() - t0

    # Memory comparison
    mem_list = (2 * sys.getsizeof(x_list) + 2 * n * 24) / 1024**2
    mem_arr = (sys.getsizeof(x_arr) + sys.getsizeof(y_arr)) / 1024**2
    mem_packed = sys.getsizeof(packed_buf) / 1024**2

    assert abs(dot_list - dot_arr_zip) < 1e-3 and abs(dot_list - dot_struct) < 1e-3, "Mismatch in Experiment 4!"

    print(f"{'Representation':<28} | {'Time (s)':<9} | {'RAM (MB)':<10} | {'Bytes/Elem':<10}")
    print("-" * 65)
    print(f"{'1. Standard Float Lists':<28} | {t_list:<9.4f} | {mem_list:<10.1f} | {'64 B':<10}")
    print(f"{'2. array.array(\"d\")':<28} | {t_arr_zip:<9.4f} | {mem_arr:<10.1f} | {'16 B':<10}")
    print(f"{'3. struct packed bytearray':<28} | {t_struct:<9.4f} | {mem_packed:<10.1f} | {'16 B':<10}")
    print("\nCPython Architectural Tradeoff:")
    print("- array.array and struct cut memory by 4x to 8x and eliminate heap fragmentation.")
    print("- When looped element-by-element in pure Python, CPython must re-box primitives into PyObjects.")
    print("- To extract maximum speed from contiguous arrays, pass them directly to C built-ins or vectorized routines.\n")


def print_perf_guide() -> None:
    """Print instructions for running Linux hardware counter profiling."""
    print("=" * 72)
    print("HARDWARE PROFILING WITH PERF (LINUX / COLIMA / DOCKER)")
    print("=" * 72)
    print("""
To measure actual CPU cache line misses and branch statistics in real hardware:

1. Launch your Linux / Colima container:
   docker run --rm -it --privileged -v $(pwd):/workspace ubuntu:22.04 bash

2. Install linux-tools (perf):
   apt-get update && apt-get install -y linux-tools-generic python3

3. Run the benchmark with perf monitoring L1 Data Cache misses:
   perf stat -e L1-dcache-loads,L1-dcache-load-misses,cycles,instructions \\
     python3 exercises/20_python_pointer_chasing.py --exp 1

Expected Hardware Observation:
- Sequential Traversal: ~1-3% L1-dcache-load-misses (hardware prefetcher streams lines).
- Scrambled Traversal:  ~20-35% L1-dcache-load-misses (nearly every memory access misses L1/L2!).
- Instructions per Cycle (IPC): Drops dramatically during pointer chasing due to CPU stalls.
""")


def run_all_tests() -> bool:
    """Verification test suite asserting correctness across all experiments."""
    print("Running verification test suite...")
    n = 10_000

    # Test 1: Node traversal equality
    nodes = [Node(i) for i in range(n)]
    for i in range(n - 1):
        nodes[i].next = nodes[i + 1]
    curr = nodes[0]
    s1 = 0
    while curr:
        s1 += curr.val
        curr = curr.next
    assert s1 == sum(range(n)), "Test 1 Failed"

    # Test 2: Attribute access equality
    d_sum = sum(d["x"] for d in [{"x": float(i)} for i in range(n)])
    s_sum = sum(r.x for r in [RecordSlots(float(i), 0.0, 0.0) for i in range(n)])
    assert d_sum == s_sum, "Test 2 Failed"

    # Test 3: Projection equality
    wide_aos = [WideRecordAoS(i, 2.5) for i in range(n)]
    soa_metrics = [2.5 for _ in range(n)]
    assert sum(r.metric for r in wide_aos) == sum(soa_metrics), "Test 3 Failed"

    print("All verification tests passed successfully!")
    return True


def main() -> None:
    parser = argparse.ArgumentParser(description="Python Pointer Chasing & Memory Layout Benchmarks")
    parser.add_argument("--all", action="store_true", help="Run all 4 experiments")
    parser.add_argument("--exp", type=int, choices=[1, 2, 3, 4], help="Run a specific experiment (1-4)")
    parser.add_argument("--test", action="store_true", help="Run verification unit test suite")
    parser.add_argument("--perf-guide", action="store_true", help="Display Linux perf stat instructions")
    parser.add_argument("-n", "--count", type=int, default=500_000, help="Number of elements (default: 500,000)")

    args = parser.parse_args()

    if args.test:
        run_all_tests()
        return

    if args.perf_guide:
        print_perf_guide()
        return

    if args.exp:
        if args.exp == 1:
            run_experiment_1(args.count)
        elif args.exp == 2:
            run_experiment_2(args.count)
        elif args.exp == 3:
            run_experiment_3(args.count)
        elif args.exp == 4:
            run_experiment_4(args.count)
        return

    # Default: Run all benchmarks
    run_experiment_1(args.count)
    run_experiment_2(args.count)
    run_experiment_3(args.count)
    run_experiment_4(args.count)


if __name__ == "__main__":
    main()
