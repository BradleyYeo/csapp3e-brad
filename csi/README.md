# Bradfield Computer Science Intensive Curriculum

## Master Learning Sequence

This repository is organized into a sequential curriculum designed for deliberate practice and deep understanding of computer systems from first principles.

```
00-prep-work ──► 01-computer-architecture ──► 02-computer-networks ──► 03-languages-and-compilers
(C, Go, Unix      (Von Neumann, assembly,      (Packets, DNS, TCP,      (Lexer, parser, AST,
 shell pipelines)  caching, processes)          proxy, traceroute)       threads, concurrency)
                                                                                  │
                                                                                  ▼
07-systems-design ◄── 06-distributed-systems ◄── 05-storage-and-retrieval ◄── 04-database-systems
(High-scale case      (Raft consensus,           (LSM-Tree, WAL, MemTable,    (Volcano iterator,
 studies & capacity)   replication, partitions)   SSTable, Bloom filters)      slotted pages, 2PL)
```

## Curriculum Modules

### Module 00: Prep Phase
- Directory: [00-prep-work/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work)
- Exercises:
  - [shell-scripting/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/shell-scripting): Unix stream pipelines (`curl`, `grep -Po`, `sed`, `wiki.sh`).
  - [c-programming/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming): K&R exercises and [projects/ls_clone.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c).
  - [go-programming/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/go-programming): The Go Programming Language exercises, [xkcd/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/go-programming/xkcd), and [learning-go/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/go-programming/learning-go).

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
- Exercises:
  - [01-packet-capture-ctf/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf): Binary PCAP parsing, HTTP reassembly CTF, and [modular-parser/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser).
  - [02-dns-client/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/02-dns-client): RFC 1035 UDP DNS query encoder and response parser.
  - [03-http-reverse-proxy/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/03-http-reverse-proxy): Concurrent HTTP proxy with LRU cache in Go and C.
  - [04-reliable-transport/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/04-reliable-transport): Sliding-window reliable transport over UDP.
  - [05-traceroute/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/05-traceroute): Raw sockets, IP TTL incrementation, and ICMP Time Exceeded parsing.

### Module 03: Languages and Compilers
- Directory: [03-languages-and-compilers/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers)
- Exercises:
  - [01-ast-eval-and-compiler/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/01-ast-eval-and-compiler): Recursive AST evaluation, bytecode compiler, stack VM.
  - [02-lexical-scanning/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/02-lexical-scanning): Lox language tokenizer.
  - [03-parsing/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/03-parsing): Recursive descent parser for Lox grammar.
  - [04-threads-and-shared-memory/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/04-threads-and-shared-memory): POSIX threads, race conditions, mutex locking.
  - [05-concurrency-patterns/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns): Go concurrency patterns and [counter-service/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/03-languages-and-compilers/05-concurrency-patterns/counter-service) contention benchmark.

### Module 04: Database Systems
- Directory: [04-database-systems/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems)
- Exercises:
  - [01-query-executor/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/01-query-executor): Volcano iterator model execution engine.
  - [02-physical-storage/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/02-physical-storage): Slotted page binary format and storage layout.
  - [03-query-optimization/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/03-query-optimization): Relational algebra predicate pushdown, join order optimization.
  - [04-concurrency-lock-manager/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/04-concurrency-lock-manager): Strict 2-Phase Locking (2PL) lock manager with deadlock prevention.

### Module 05: Data Structures for Storage and Retrieval
- Directory: [05-storage-and-retrieval/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval)
- Exercises:
  - [01-in-memory-kv/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/01-in-memory-kv): In-memory key-value store with range iteration.
  - [02-linked-list-kv/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/02-linked-list-kv): Ordered linked list storage engine.
  - [03-leveldb-lsm/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/03-leveldb-lsm): LSM-Tree engine with Write-Ahead Log (WAL), SkipList MemTable, immutable SSTables, and Leveled Compaction.
  - [04-leveldb-test-suite/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/04-leveldb-test-suite): Comprehensive verification test harness.

### Module 06: Distributed Systems
- Directory: [06-distributed-systems/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/06-distributed-systems)
- Exercises:
  - [01-distributed-kvstore/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/06-distributed-systems/01-distributed-kvstore): Distributed replicated key-value store over gRPC and protocol buffers.

### Module 07: Systems Design
- Directory: [07-systems-design/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design)
- Exercises:
  - [youtube_dislikes.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/youtube_dislikes.md): High-throughput counter pipelines and lossy aggregation.
  - [metrics_system.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/metrics_system.md): Time-series ingestion pipelines, downsampling tiers, and range queries.
  - [code_snippet_hosting_platform.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/code_snippet_hosting_platform.md): Content-addressed storage (CAS), metadata vs blob separation, and caching topologies.
  - [simple_airbnb.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/simple_airbnb.md): Double-booking prevention, geospatial indexing, and 2-phase reservation state machines.

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
  colima start --arch x86_64
  docker run --rm -it -v "$(pwd):/workspace" -w /workspace gcc:latest bash
  ```
