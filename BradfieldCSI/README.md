# Bradfield Computer Science Intensive Curriculum

## Master Learning Sequence

This repository is organized into a sequential curriculum designed for deliberate practice and deep understanding of computer systems from first principles.

```
01-computer-architecture ──► 02-computer-networks ──► 03-languages-and-compilers
(Von Neumann, assembly,      (Packets, DNS, TCP,      (Lexer, parser, AST,
 caching, processes)          proxy, traceroute)       threads, concurrency)
                                                              │
                                                              ▼
06-distributed-systems ◄── 05-storage-and-retrieval ◄── 04-database-systems
(Raft consensus,           (LSM-Tree, WAL, MemTable,    (Volcano iterator,
 replication, partitions)   SSTable, Bloom filters)      slotted pages, 2PL)
```

## Curriculum Modules

### Module 01: Computer Architecture and Systems Fundamentals
- Directory: [01-computer-architecture/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture)
- Exercises:
  - [01-virtual-machine/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/01-virtual-machine): Build a Von Neumann 8-bit CPU emulator in Go.
  - [02-binary-encodings/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings): Two's complement, floats, byte ordering, BMP/TCP parsing.
  - [03-x86-64-assembly-intro/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/03-x86-64-assembly-intro): System V AMD64 ABI, registers, `sum_to_n`, `binary_convert`, `pangram`.
  - [04-x86-64-assembly-further/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/04-x86-64-assembly-further): Row-major matrix indexing, recursion, SSE2 float cone volume.
  - [05-low-level-optimization/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/05-low-level-optimization): Instruction-level parallelism, loop unrolling, multiple accumulators.
  - [06-memory-hierarchy-caching/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching): Cache lines, row vs column strides, matrix multiplication tiling.
  - [07-memory-hierarchy-data-layout/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/07-memory-hierarchy-data-layout): Struct of Arrays (SoA) vs Array of Structs (AoS) cache optimization in Go.
  - [08-exceptional-control-flow/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/08-exceptional-control-flow): Processes, system calls (`fork`/`exec`), signal safety, Unix shell in C.

### Module 02: Computer Networks
- Directory: [02-computer-networks/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks)
- Exercises: Packet capture parsing, DNS client, HTTP reverse proxy, sliding-window reliable transport, ICMP traceroute.

### Module 03: Languages and Compilers
- Directory: [03-languages-and-compilers/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers)
- Exercises: AST expression evaluation, bytecode compiler, lexical scanner, recursive descent parser, POSIX threads, synchronization patterns.

### Module 04: Database Systems
- Directory: [04-database-systems/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems)
- Exercises: Volcano iterator query executor, slotted page physical storage, query plan optimization, 2PL lock manager.

### Module 05: Data Structures for Storage and Retrieval
- Directory: [05-storage-and-retrieval/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval)
- Exercises: In-memory KV store, LevelDB LSM-Tree engine (WAL, SkipList MemTable, immutable SSTables, Bloom filters, Leveled Compaction).

### Module 06: Distributed Systems
- Directory: [06-distributed-systems/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/06-distributed-systems)
- Exercises: Replicated key-value store, Raft consensus protocol, fault tolerance under network partitions.

### Module 00: Prep Phase
- Directory: [00-prep-work/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work)
- Exercises: C programming fundamentals (pointers, manual memory management), Go systems programming.

## Learning Methodology

### Deliberate Practice and Implementing from Scratch
- Every exercise folder contains clean starter code skeletons and automated test harnesses.
- Complete solutions from reference implementations have been organized into isolated `reference-solution/` subdirectories. Avoid viewing them until you have attempted your own implementation.

### Mental Models and Active Recall
- Every exercise includes a comprehensive `README.md` defining the hardware and systems mental models, architectural principles (Ousterhout's Deep Modules, Jackson's Concept Design, Koppel's Single Authority Principle), and active recall checkpoints.
- Review the active recall questions before and after completing each exercise to consolidate your mental models.

## macOS and Linux Environment Setup

### Assembly and Linux Tooling on Apple Silicon (ARM64 host)
- The x86-64 assembly exercises (`03-x86-64-assembly-intro`, `04-x86-64-assembly-further`) and profiling tools (`perf`, Valgrind `cachegrind`) target x86-64 Linux.
- **Option 1: Rosetta 2 (macOS Mach-O)**
  ```bash
  softwareupdate --install-rosetta
  ```
- **Option 2: Linux Container via Colima / Docker (Recommended)**
  ```bash
  # Launch an x86-64 Linux environment
  colima start --arch x86_64
  # Or run a disposable build container
  docker run --rm -it -v "$(pwd):/workspace" -w /workspace gcc:latest bash
  ```
