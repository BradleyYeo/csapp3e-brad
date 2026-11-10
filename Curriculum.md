# Operating Rhythm & Time Commitment

## Schedule Architecture

- Morning Theory Block (Monday to Friday, 6:30 AM – 7:30 AM, 1.0 hour):
  - Dedicated to conceptual textbook reading and lecture videos.
  - Textbooks & Literature: CS:APP 3e, OSTEP (*Operating Systems: Three Easy Pieces*), Harris & Harris DDCA, Hennessy & Patterson (H&P/P&H) *Computer Architecture: A Quantitative Approach* (7th Edition, with Kozyrakis), Sergey Slotin's [Algorithms for Modern Hardware (AMH)](https://en.algorithmica.org/hpc/), and [UC Berkeley CS 61C: Number Representation Notes](https://notes.cs61c.org/content/number-rep/).
  - Lecture Video Tracks:
    - [Onur Mutlu: Digital Design & Computer Architecture Playlist](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd)
    - [CMU 15-213: Introduction to Computer Systems Playlist](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj)
    - [Wei Yang: CMU 15-213 Lectures & Lab Recitations Channel](https://www.youtube.com/@weiyang2613/videos)
    - [MIT 6.S081 / 6.1810: Operating System Engineering Playlist](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK)
    - [UC Berkeley CS 162: Operating Systems and Systems Programming Playlist](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC)
    - [xv6 OS Kernel Walkthrough Playlist](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs)
- Evening Laboratory Block (Monday to Friday, 8:00 PM – 9:30 PM, 1.5 hours):
  - Dedicated to hands-on coding, debugging, and lab execution.
  - Workspaces & Benchmark Repositories:
    - Colima / Docker x86-64 containers for CS:APP labs.
    - [sslotin/amh-code](https://github.com/sslotin/amh-code) (hardware micro-benchmarks for cache latency, bandwidth, associativity, and branch prediction).
    - Native / QEMU for RISC-V xv6.
    - CodeCrafters CLI.
- Weekend Deep Work Blocks (Saturday and Sunday):
  - Afternoon Block (2:00 PM – 5:00 PM, 3.0 hours): Lab architecture, systems implementations, and CodeCrafters challenges.
  - Evening Block (8:00 PM – 9:30 PM, 1.5 hours): Code review, memory leak profiling with Valgrind, and active recall synthesis.
- Total Time Allocation:
  - Weekday: 2.5 hours/day $\times$ 5 days = 12.5 hours/week.
  - Weekend: 4.5 hours/day $\times$ 2 days = 9.0 hours/week.
  - Cumulative: 21.5 hours/week $\times$ 26 weeks = 559 hours total.

## Deliberate Practice Rules

- Active Recall Over Passive Viewing:
  - Never watch lectures or read textbooks without an active terminal or notebook.
  - Summarize each chapter using mental models, invariants, and failure modes before writing code.
- Zero-Copy Implementation:
  - Implement all starter skeletons and lab deliverables from scratch.
  - Consult reference solutions only when a bug persists past two full debugging sessions.
- Memory and ABI Discipline:
  - Every C allocation must have a deterministic deallocation site verified by Valgrind (`lackey`, `memcheck`).
  - Every assembly function must strictly preserve callee-saved registers and maintain 16-byte stack alignment.

---

# Strategic Remediation: Bomb Lab & Attack Lab

## Diagnostic: Why Bomb Lab and Attack Lab Were Difficult

- Assembly Reading vs Writing Deficit:
  - Reading optimized compiler assembly without first authoring assembly creates cognitive overload.
  - Control flow patterns (jump tables, recursive stack frames, struct indexing) look arbitrary until implemented by hand.
- Stack Topography Uncertainty:
  - Ambiguity regarding `%rsp` movement, return addresses, and buffer boundaries leads to blind guessing in exploit payloads.
- GDB Inspection Hesitation:
  - Insufficient fluency with raw GDB memory dereferencing (`x/gx`, `x/s`, `x/i`, `$rip`, `$rsp`) hinders reverse engineering.

## Remediation Sequence

- Immediate Priority: Complete [labs/05_cachelab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab) since memory hierarchy concepts and C data structure mechanics are active.
- Ground-Up Assembly Foundation:
  - Review [UC Berkeley CS 61C: Number Representation Notes](https://notes.cs61c.org/content/number-rep/) to master two's complement arithmetic, sign extension, and signed vs unsigned jump semantics (`jg`/`jl` vs `ja`/`jb`).
  - Author x86-64 assembly in [csi/01-computer-architecture/03-x86-64-assembly-intro](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro) ([sum_to_n.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/sum_to_n.asm), [binary_convert.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/binary_convert.asm), [pangram.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/pangram.asm)).
  - Author advanced assembly in [csi/01-computer-architecture/04-x86-64-assembly-further](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further) ([fib.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/fib.asm), [matrix.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/matrix.asm), [cone.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/cone.asm)).
- Lecture Track Integration:
  - Watch [Wei Yang: Machine Level Programming I–IV](https://www.youtube.com/@weiyang2613/videos) and [CMU 15-213 Machine Programming Lectures](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) (Lectures 05, 06, 07, 08).
- Defeat Bomb Lab:
  - Apply newly mastered assembly models to systematically disassemble and defuse [labs/02_bomblab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/02_bomblab) Phases 1 through 6 plus the secret phase.
- Defeat Attack Lab:
  - Systematically map buffer overflows, code injection, and gadget-based Return-Oriented Programming (ROP) in [labs/03_attacklab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/03_attacklab) Phases 1 through 4.

---

# Six-Month Curriculum Progression

## Phase 1: Memory Hierarchy & Cache Lab Completion (Weeks 1–3: Nov 9 – Nov 29, 2026)

### Focus & Objectives
- Master the $(S, E, B, m)$ hardware cache geometry, set indexing arithmetic, and LRU replacement policies.
- Complete [labs/05_cachelab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab) Part A (Cache Simulator) and Part B (Matrix Transposition Optimization).
- Analyze row-major vs column-major memory access strides, cache blocking, and on-disk page structures.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapter 6.
  - Hennessy & Patterson (H&P 7th ed) Appendix B (Review of Memory Hierarchy: cache geometry, 3Cs miss classification, write policies) and Chapter 2 (§§2.1–2.3 Advanced Cache Optimizations, AMAT, and DRAM organization).
  - [Sergey Slotin: Algorithms for Modern Hardware - CPU Caches: Latency](https://en.algorithmica.org/hpc/cpu-cache/latency/) (Pointer chasing & defeating prefetchers).
  - [Sergey Slotin: Algorithms for Modern Hardware - Associativity & Cache Conflicts](https://en.algorithmica.org/hpc/cpu-cache/associativity/) (Conflict misses & set thrashing).
  - [CMU 15-213: Lecture 12 & 13 (The Memory Hierarchy & Cache Memories)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - [Onur Mutlu DDCA: Memory Hierarchy & Caches](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd).
  - Ulrich Drepper's *What Every Programmer Should Know About Memory* (Sections 1–4).
- Hands-on Lab Targets:
  - [labs/05_cachelab/cachelab-handout/csim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/csim.c)
  - [labs/05_cachelab/cachelab-handout/trans.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/trans.c)
  - [csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c)
  - [exercises/20_python_pointer_chasing.py](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/20_python_pointer_chasing.py) (Pointer chasing & memory layout optimization)
  - [sslotin/amh-code: cpu-cache benchmarks](https://github.com/sslotin/amh-code) (`latency.cc`, `associativity.cc`, `matrix-transpose.cc`)
- Systems Practice Extension:
  - [CodeCrafters: Build your own SQLite](https://codecrafters.io/challenges/sqlite) (Stage 1 to Stage 3: Reading DB header, page sizes, and parsing table records from raw disk pages).

---

## Phase 2: Assembly Fluency & Conquering Bomb + Attack Lab (Weeks 4–7: Nov 30 – Dec 27, 2026)

### Focus & Objectives
- Build authoring fluency in x86-64 assembly conforming to System V AMD64 ABI.
- Master GDB dynamic analysis (`stepi`, `nexti`, register dump, memory format strings).
- Solve all phases of Bomb Lab and Attack Lab.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapter 3 (Machine-Level Representation of Programs).
  - [UC Berkeley CS 61C: Number Representation Notes](https://notes.cs61c.org/content/number-rep/) (Sign-magnitude, two's complement, bias encoding, sign extension, and integer overflow conditions).
  - [CMU 15-213: Lectures 05–08 (Machine Programming Basics, Control, Procedures, Data)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - [Wei Yang: Machine Level Programming Lectures & Recitations](https://www.youtube.com/@weiyang2613/videos) (C Review, Assembly Basics, Stack Framing).
- Hands-on Lab Targets:
  - [csi/01-computer-architecture/03-x86-64-assembly-intro/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro) ([sum_to_n.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/sum_to_n.asm), [binary_convert.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/binary_convert.asm), [pangram.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/pangram.asm))
  - [csi/01-computer-architecture/04-x86-64-assembly-further/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further) ([fib.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/fib.asm), [matrix.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/matrix.asm), [cone.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/cone.asm))
  - [labs/02_bomblab/bomb](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/02_bomblab/bomb) (Phases 1–6 + Secret Phase)
  - [labs/03_attacklab/target/ctarget](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/03_attacklab/target/ctarget) (Phases 1–3) and [labs/03_attacklab/target/rtarget](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/03_attacklab/target/rtarget) (Phase 4 ROP)

---

## Phase 3: Processor Architecture, Pipelining & Optimization (Weeks 8–11: Dec 28, 2026 – Jan 24, 2027)

### Focus & Objectives
- Understand digital logic, sequential execution, pipelining, and hazard resolution.
- Build a Von Neumann 8-bit CPU emulator in Go.
- Explore Out-of-Order execution, branch prediction, loop unrolling, and multiple accumulators.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapters 4 & 5.
  - Hennessy & Patterson (H&P 7th ed) Appendix C (Pipelining: Basic and Intermediate Concepts: datapath, pipeline registers, hazard equations) and Chapter 3 (Instruction-Level Parallelism: dynamic scheduling, Tomasulo's algorithm, hardware speculation, ROB).
  - Hennessy & Patterson (H&P 7th ed) Chapter 4 (Data-Level Parallelism in Vector, SIMD, and GPU Architectures).
  - [Sergey Slotin: Algorithms for Modern Hardware - Pipelining, Branching & Superscalar Execution](https://en.algorithmica.org/hpc/pipelining/).
  - [Onur Mutlu DDCA: Lectures 12–18 (Single-Cycle, Multi-Cycle, Pipelined Microarchitectures, Hazards & Branch Prediction)](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd).
  - [CMU 15-213: Lecture 09 & 10 (Code Optimization)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - Mutlu CompArch (Tomasulo's Algorithm, Register Renaming, Reorder Buffer).
- Hands-on Lab Targets:
  - [csi/01-computer-architecture/01-virtual-machine/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/01-virtual-machine) (Von Neumann emulator)
  - [exercises/19_pipelining_hazards.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/19_pipelining_hazards.md) (Hardware hazard analysis)
  - [sslotin/amh-code: pipelining & branching benchmarks](https://github.com/sslotin/amh-code) (`branching.cc`, `superscalar.cc`)
  - [csi/01-computer-architecture/05-low-level-optimization/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization) ([pagecount.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization/pagecount.c), [dotproduct.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization/vec/dotproduct.c))
  - [csi/01-computer-architecture/07-memory-hierarchy-data-layout/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/07-memory-hierarchy-data-layout) (SoA vs AoS cache optimization)

---

## Phase 4: Linking & Exceptional Control Flow (Weeks 12–14: Jan 25 – Feb 14, 2027)

### Focus & Objectives
- Understand object files, symbol resolution, relocation, dynamic linking, and ELF binaries.
- Master Unix process lifecycles (`fork`, `execve`, `waitpid`), signal handling, and signal safety.
- Implement a Unix command shell with complete job control and foreground/background handling.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapters 7 & 8.
  - [CMU 15-213: Lecture 14 & 15 (Exceptional Control Flow: Exceptions, Processes, Signals)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - [Wei Yang: Lecture 14 (Exceptional Control Flow & Processes)](https://www.youtube.com/@weiyang2613/videos).
  - [UC Berkeley CS 162: Lectures 02 & 03 (Four Fundamental OS Abstractions, Dual-Mode Operation & Process Control)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - OSTEP Chapters 4–6 (Processes, Process API, Limited Direct Execution).
- Hands-on Lab Targets:
  - [labs/07_shlab/shlab-handout/tsh.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/07_shlab/shlab-handout/tsh.c) (CS:APP Shell Lab)
  - [csi/01-computer-architecture/08-exceptional-control-flow/shell.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/08-exceptional-control-flow/shell.c)
- Systems Practice Extension:
  - [CodeCrafters: Build your own Shell](https://codecrafters.io/challenges/shell) (Complete full track: REPL, builtins, PATH search, redirection `>`, `>>`, pipelines `|`).

---

## Phase 5: Virtual Memory & Dynamic Storage Allocators (Weeks 15–18: Feb 15 – Mar 14, 2027)

### Focus & Objectives
- Master multi-level page tables, TLBs, page faults, address translation math, and `mmap`.
- Implement a high-performance dynamic memory allocator (`malloc`, `free`, `realloc`) using explicit and segregated free lists.
- Eliminate external and internal memory fragmentation.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapter 9.
  - Hennessy & Patterson (H&P 7th ed) Chapter 2 (§2.4 Virtual Memory, Protection, and Fast Address Translation: TLBs, page table organization, and VIPT vs PIPT cache design).
  - [CMU 15-213: Lectures 17–19 (Virtual Memory Systems & Dynamic Memory Allocation)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - [Wei Yang: Lecture 19 (Dynamic Memory Allocation Basic Concepts & Free Lists)](https://www.youtube.com/@weiyang2613/videos).
  - [UC Berkeley CS 162: Lectures 10–12 (Address Translation, Multi-Level Paging, TLBs & Demand Paging)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - OSTEP Chapters 13–22 (Virtual Memory, Address Translation, Paging, TLB, Page Replacement).
- Hands-on Lab Targets:
  - [exercises/07_process_memory_layout.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.md)
  - [exercises/09_dynamic_memory_lifecycle.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.md)
  - [exercises/11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md)
  - [labs/06_malloclab/malloclab-handout/mm.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/06_malloclab/malloclab-handout/mm.c) (Explicit and Segregated Free Lists)
- Systems Practice Extension:
  - [CodeCrafters: Build your own SQLite](https://codecrafters.io/challenges/sqlite) (Stage 4 to Stage 7: B-Tree interior nodes, table scans, and index searches).

---

## Phase 6: OS Kernel Engineering with xv6 (Weeks 19–22: Mar 15 – Apr 11, 2027)

### Focus & Objectives
- Step inside the kernel boundary to inspect how OS primitives are implemented on physical hardware.
- Add system calls, manipulate hardware page tables, handle user/kernel trap transitions, and build Copy-on-Write fork.

### Core Resources
- Theory & Lectures:
  - [MIT 6.S081 / 6.1810: Operating System Engineering Playlist](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK) (Profs. Robert Morris & Frans Kaashoek; official MIT xv6 lectures).
  - [xv6 OS Kernel Walkthrough Playlist](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs) (xv6 #1–#10: Architecture, Traps, Syscalls, Page Tables, Memory Management).
  - Textbook: *xv6: a simple, Unix-like teaching operating system* by Cox, Kaashoek, Morris.
- Hands-on Lab Targets:
  - [xv6-riscv/kernel/sysproc.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/sysproc.c) (System calls: `trace`, `sysinfo`)
  - [xv6-riscv/kernel/vm.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/vm.c) (Page table inspection, virtual memory mapping)
  - [xv6-riscv/kernel/trap.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/trap.c) (Hardware interrupts, user traps, timer alarm)
  - [xv6-riscv/kernel/kalloc.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/kalloc.c) (Copy-on-Write reference counting allocator)

---

## Phase 7: Concurrency, Network I/O & High-Performance Systems (Weeks 23–26: Apr 12 – May 9, 2027)

### Focus & Objectives
- Master POSIX threads, synchronization primitives (mutexes, semaphores), race conditions, and deadlocks.
- Build a concurrent caching web proxy.
- Learn modern high-throughput Linux I/O (`epoll`, non-blocking sockets), build an in-memory database, and construct an append-only message broker.

### Core Resources
- Theory & Lectures:
  - CS:APP 3e Chapters 10, 11, 12.
  - Hennessy & Patterson (H&P 7th ed) Chapter 5 (Thread-Level Parallelism: symmetric shared-memory architectures, cache coherence protocols [MSI/MESI/MOESI], false sharing, memory consistency models [Sequential Consistency, Total Store Order], and hardware synchronization primitives).
  - [CMU 15-213: Lectures 20–25 (Network Programming, Sockets, Concurrency, Synchronization)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj).
  - [Wei Yang: Recitation 13 (Proxy Lab) & Recitation 14 (Synchronization)](https://www.youtube.com/@weiyang2613/videos).
  - [UC Berkeley CS 162: Lectures 04–07 (Concurrency, Synchronization, Monitors, Semaphores, Deadlock)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - OSTEP Chapters 26–32 (Concurrency, Locks, Condition Variables, Semaphores).
  - Mutlu Computer Architecture lectures on Memory Consistency & Cache Coherence.
- Hands-on Lab Targets:
  - [labs/08_proxylab/proxylab-handout/proxy.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/08_proxylab/proxylab-handout/proxy.c)
  - [csi/02-computer-networks/03-http-reverse-proxy/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/02-computer-networks/03-http-reverse-proxy)
  - [csi/03-languages-and-compilers/05-concurrency-patterns/counter-service/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/03-languages-and-compilers/05-concurrency-patterns/counter-service)
- Systems Practice Extension:
  - [CodeCrafters: Build your own Redis](https://codecrafters.io/challenges/redis) (TCP server, RESP protocol parser, concurrent client connections, TTL expiration, RDB persistence).
  - [CodeCrafters: Build your own Kafka](https://codecrafters.io/challenges/kafka) (Commit log storage format, binary wire protocol serialization, partition fetch streaming).

---

# Detailed Weekly Execution Plan

## Week 1: Nov 9 – Nov 15, 2026 | Cache Geometry & csim.c
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 6.1–6.4 (Locality, SRAM vs DRAM, $(S, E, B)$ geometry), H&P 7th ed Appendix B (Review of Memory Hierarchy: cache line mapping, tags/indices, 3Cs miss classification), and [Sergey Slotin AMH: CPU Caches: Latency](https://en.algorithmica.org/hpc/cpu-cache/latency/) (Measuring cache latency via cyclic permutation pointer chasing; hardware prefetcher defeat).
  - Thu–Fri: [CMU 15-213: Lecture 12 (Cache Memories)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) and [Onur Mutlu DDCA: Caches](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Parse Valgrind `lackey` trace lines in [labs/05_cachelab/cachelab-handout/csim.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/csim.c).
  - Wed–Fri: Implement cache set allocation, line tag comparison, and hit/miss/eviction accounting.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement LRU replacement policy using dynamic counter tracking in `csim.c`. Benchmark [sslotin/amh-code: latency.cc](https://github.com/sslotin/amh-code) in QEMU / Colima container to observe physical L1/L2/L3/RAM latency plateaus.
  - Sun: Verify `csim.c` against `csim-ref` across all test traces (`yi.trace`, `dave.trace`, `long.trace`). Achieve 27/27 points on Part A. Run [exercises/20_python_pointer_chasing.py](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/20_python_pointer_chasing.py) to inspect pointer chasing slowdown in high-level runtimes.
- Active Recall:
  - Why does pointer chasing across random cyclic permutations defeat CPU stream prefetchers and allow direct measurement of true memory latency?
  - Why are Set Index bits placed in the middle of the address rather than the high-order bits?

## Week 2: Nov 16 – Nov 22, 2026 | Matrix Transpose Blocking ($32 \times 32$ & $64 \times 64$)
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Section 6.5–6.6 (Cache impact on program performance, memory mountain), H&P 7th ed Chapter 2 (§§2.1–2.2 Advanced Cache Optimizations: non-blocking caches, critical word first, compiler blocking), and [Sergey Slotin AMH: Associativity & Cache Conflicts](https://en.algorithmica.org/hpc/cpu-cache/associativity/).
  - Thu–Fri: Conflict miss analysis, diagonal self-eviction in matrix transposition, and [Sergey Slotin AMH: Cache-Oblivious Algorithms](https://en.algorithmica.org/hpc/cpu-cache/oblivious/).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Benchmark baseline transpose in [labs/05_cachelab/cachelab-handout/trans.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/05_cachelab/cachelab-handout/trans.c). Trace 1KB direct-mapped cache collisions ($s=5, E=1, b=5$) and compare with [sslotin/amh-code: associativity.cc](https://github.com/sslotin/amh-code) power-of-two stride collisions.
  - Wed–Fri: Implement $8 \times 8$ blocking for $32 \times 32$ matrix. Use local register variables to avoid diagonal thrashing. Target $< 300$ misses.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement $4 \times 4$ sub-block register staging for $64 \times 64$ matrix. Target $< 1300$ misses.
  - Sun: Optimize $61 \times 67$ irregular matrix with rectangular blocking ($16 \times 16$ or $17 \times 17$). Target $< 2000$ misses.
- Active Recall:
  - Why does an $8 \times 8$ block fail for a $64 \times 64$ matrix on a 1KB direct-mapped cache?
  - In H&P's quantitative cache model, what is the formula for Average Memory Access Time (AMAT), and how do Miss Status Holding Registers (MSHRs) enable non-blocking caches to overlap miss latency?

## Week 3: Nov 23 – Nov 29, 2026 | Cache Synthesis & CodeCrafters SQLite Page Formats
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: Ulrich Drepper's *What Every Programmer Should Know About Memory* (CPU Caches, Virtual Memory, NUMA).
  - Thu–Fri: Database storage layouts: slotted pages, B-Tree leaf pages, cell pointers.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Profile loop order cache efficiency in [csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/04_loop_order_measure.c) using Valgrind `cachegrind`.
  - Wed–Fri: Run automated driver on `cachelab` (`./driver.py`). Verify full score (53/53 points).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Begin [CodeCrafters: Build your own SQLite](https://codecrafters.io/challenges/sqlite). Parse 100-byte SQLite database header, page size, and decode B-Tree leaf page cell pointers.
- Active Recall:
  - How does cache tiling transform matrix multiplication miss complexity from $O(N^3)$ to $O(N^3 / B)$?

## Week 4: Nov 30 – Dec 6, 2026 | x86-64 Assembly Authoring Foundations
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 3.1–3.6 (Registers, operand specifiers, data movement, arithmetic/logical operations) and [UC Berkeley CS 61C: Number Representation Notes](https://notes.cs61c.org/content/number-rep/) (Sign-magnitude vs 2's complement, sign extension, and signed vs unsigned integer overflow).
  - Thu–Fri: [CMU 15-213: Lecture 05 & 06 (Machine Programming I & II)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) and [Wei Yang: C Review & Assembly Basics](https://www.youtube.com/@weiyang2613/videos).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Implement [csi/01-computer-architecture/03-x86-64-assembly-intro/sum_to_n.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/sum_to_n.asm) using arithmetic loop and conditional jumps.
  - Wed–Fri: Implement [csi/01-computer-architecture/03-x86-64-assembly-intro/binary_convert.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/binary_convert.asm) with ASCII parsing, two's complement accumulation, and bit shifts.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement [csi/01-computer-architecture/03-x86-64-assembly-intro/pangram.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/03-x86-64-assembly-intro/pangram.asm) with 26-bit bitmask tracking (`bts` / bitwise shift).
  - Sun: Verify all intro assembly tests via `make test`. Practice GDB register inspection (`info registers`, `p/x $rax`).
- Active Recall:
  - Why must a callee function preserve `%rbx`, `%rbp`, and `%r12-%r15` before overwriting them?
  - How does a CPU hardware adder detect two's complement signed overflow versus unsigned carry during an addition operation?

## Week 5: Dec 7 – Dec 13, 2026 | Advanced Assembly: Stack Frames, Recursion & SIB Addressing
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 3.7–3.9 (Stack frame allocation, arrays, multi-dimensional pointers, nested structures).
  - Thu–Fri: [CMU 15-213: Lecture 07 & 08 (Procedures & Structured Data)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) and [Wei Yang: Machine Level Programming IV (Data)](https://www.youtube.com/@weiyang2613/videos).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Implement recursive Fibonacci in [csi/01-computer-architecture/04-x86-64-assembly-further/fib.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/fib.asm). Enforce stack alignment and register preservation.
  - Wed–Fri: Implement 2D row-major matrix lookup in [csi/01-computer-architecture/04-x86-64-assembly-further/matrix.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/matrix.asm) using `imul` and SIB addressing.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement floating-point cone volume in [csi/01-computer-architecture/04-x86-64-assembly-further/cone.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/04-x86-64-assembly-further/cone.asm) using SSE2 scalar instructions (`mulsd`, `divsd`, `%xmm0`).
  - Sun: Disassemble compiler output (`gcc -O2 -S`) for recursive C functions and compare against hand-written assembly.
- Active Recall:
  - What happens if the stack pointer `%rsp` is not aligned to a 16-byte boundary prior to executing a `call` instruction?

## Week 6: Dec 14 – Dec 20, 2026 | Conquering Bomb Lab
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: Reverse engineering control structures: switches with jump tables, while loops, recursion in assembly.
  - Thu–Fri: GDB commands for binary analysis: `disas`, `x/Nuf <addr>`, `layout asm`, `layout regs`, `break`, `si`, `ni`.
- Evening Exercises (8:00–9:30 PM):
  - Mon: Disassemble and defuse Phase 1 (string comparison) and Phase 2 (loop sequence) in [labs/02_bomblab/bomb](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/02_bomblab/bomb).
  - Tue: Defuse Phase 3 (switch-case jump table disassembly).
  - Wed: Defuse Phase 4 (recursive call stack analysis).
  - Thu: Defuse Phase 5 (character indexing and array masking).
  - Fri: Disassemble Phase 6 (linked list node traversal and pointer sorting).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Defuse Phase 6 by reconstructing node pointer structs and order logic.
  - Sun: Find and defuse the Secret Phase (binary tree traversal). Complete full [labs/02_bomblab/answer.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/02_bomblab/answer.txt).
- Active Recall:
  - How do you locate a jump table in memory and calculate the target jump address from the index register?

## Week 7: Dec 21 – Dec 27, 2026 | Conquering Attack Lab (Phases 1–4)
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Section 3.12 (Buffer overflow, stack layout, code injection, stack protector canaries, $W \oplus X$ / NX).
  - Thu–Fri: Return-Oriented Programming (ROP): gadget taxonomy (`pop %rdi; ret`), gadget farms, stack pivoting.
- Evening Exercises (8:00–9:30 PM):
  - Mon: Execute Phase 1 (`ctarget` `touch1` return address redirection).
  - Tue: Execute Phase 2 (`touch2` passing integer argument by value in `%rdi`).
  - Wed: Execute Phase 3 (`touch3` passing pointer to cookie string; avoid stack collision with `hexmatch`).
  - Thu: Review Phase 4 (`rtarget` ROP). Disassemble `start_farm` and `mid_farm`.
  - Fri: Search and catalog ROP gadgets (`popq %rax`, `movq %rax, %rdi`).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Construct Phase 4 exploit chain. Overwrite return address with gadget addresses and cookie payload.
  - Sun: Validate Phase 4 exploit. Document complete attack payloads in [labs/03_attacklab/P3.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/03_attacklab/P3.md). (Phase 5 optional bonus).
- Active Recall:
  - Why does `touch3` fail if the cookie string is placed inside `getbuf`'s stack frame?

## Week 8: Dec 28, 2026 – Jan 3, 2027 | CPU Microarchitecture & Von Neumann Emulator
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 4.1–4.2 (Y86-64 instruction set, logic gates, ALU, hardware control languages), H&P 7th ed Appendix C (Datapath structure, instruction execution phases), and review [UC Berkeley CS 61C: Number Representation](https://notes.cs61c.org/content/number-rep/) (ALU condition flags: zero, sign, carry, and overflow).
  - Thu–Fri: [Onur Mutlu DDCA: Instruction Set Architectures & Von Neumann Model](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Inspect [csi/01-computer-architecture/01-virtual-machine/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/01-virtual-machine). Define 256-byte RAM, registers (`r1`, `r2`), Program Counter (`PC`).
  - Wed–Fri: Implement opcode extraction, memory load/store, and ALU addition/subtraction in Go emulator.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement control transfer instructions (`jump`, `beqz`). Run unit test suite (`go test -v`).
  - Sun: Implement immediate instructions (`addi`, `subi`). Assemble and run `sum_to_n` assembly on your emulator.
- Active Recall:
  - What constitutes the minimal architectural state of a CPU that must be preserved across clock cycles?

## Week 9: Jan 4 – Jan 10, 2027 | Pipelining, Hazards & Data Forwarding
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 4.4–4.5 (Pipelining principles, throughput vs latency, pipeline registers) and H&P 7th ed Appendix C (§C.2–§C.3 Data hazards, forwarding condition logic equations, load-use interlocks, branch stalls).
  - Thu–Fri: [Onur Mutlu DDCA: Lectures 14–16 (Pipelined Microarchitecture, Hazards, Stalls, and Data Forwarding)](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Trace 5-stage pipeline (Fetch, Decode, Execute, Memory, Write-back). Analyze Read-After-Write (RAW) data hazards.
  - Wed–Fri: Deepen with [exercises/19_pipelining_hazards.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/19_pipelining_hazards.md). Trace hardware forwarding paths ($E \to E$, $M \to E$, $W \to E$).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Analyze the Load-Use hazard: prove why forwarding alone cannot resolve a load consumed immediately by the next instruction; diagram the 1-cycle bubble injection.
  - Sun: Analyze branch prediction and branch misprediction penalty (2-bubble cancellation). Hand-trace execution cycle tables for a loop.
- Active Recall:
  - Why can data not be forwarded backward in time during a load-use hazard?

## Week 10: Jan 11 – Jan 17, 2027 | Out-of-Order Execution & Modern Microarchitecture
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: H&P 7th ed Chapter 3 (§§3.1–3.6 Instruction-Level Parallelism: Dynamic scheduling via Tomasulo's algorithm, reservation stations, Common Data Bus [CDB]), [Onur Mutlu DDCA: Branch Prediction & Out-of-Order Execution](https://www.youtube.com/playlist?list=PL5Q2soXY2Zi-yo9kK-BKrq11ykNKkVEpd), and [Sergey Slotin AMH: Branch Prediction](https://en.algorithmica.org/hpc/pipelining/branching/).
  - Thu–Fri: H&P 7th ed Chapter 3 (§§3.7–3.9 Hardware-based speculation, Reorder Buffer [ROB], in-order commit, precise exceptions, and register renaming).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Construct dependency graph of an assembly basic block. Identify true dependencies (RAW) vs false dependencies (WAR, WAW).
  - Wed–Fri: Trace register renaming table transitions across multiple loop iterations.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Study branch predictors: 2-bit saturating counters, 2-level adaptive predictors, gshare, TAGE branch predictors. Benchmark branch misprediction penalties using [sslotin/amh-code: branching.cc](https://github.com/sslotin/amh-code).
  - Sun: Synthesize CPU execution pipeline mental model: Front-End (Fetch, Decode, Rename) vs Back-End (Issue, Execute, Writeback, Commit).
- Active Recall:
  - How does register renaming eliminate Write-After-Read (WAR) and Write-After-Write (WAW) false hazards?
  - In Tomasulo's algorithm with a Reorder Buffer (ROB), why must instruction commit occur in-order even though issue and execution occur out-of-order?

## Week 11: Jan 18 – Jan 24, 2027 | Program Optimization & Instruction-Level Parallelism
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Chapter 5 (Optimizing Program Performance: compiler capabilities/limits, eliminating loop inefficiencies), H&P 7th ed Chapter 1 (CPU performance equation: $\text{CPU Time} = \text{IC} \times \text{CPI} \times \text{Cycle Time}$), and [Sergey Slotin AMH: Instruction-Level Parallelism](https://en.algorithmica.org/hpc/pipelining/tables/).
  - Thu–Fri: H&P 7th ed Chapter 4 (Data-Level Parallelism in SIMD and Vector Architectures) and [CMU 15-213: Lecture 09 & 10 (Code Optimization)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj). Latency bound vs throughput bound; functional unit issue time.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Implement [csi/01-computer-architecture/05-low-level-optimization/pagecount.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization/pagecount.c): replace 64-bit integer division with bitwise shifting.
  - Wed–Fri: Implement loop unrolling and multiple accumulators in [csi/01-computer-architecture/05-low-level-optimization/vec/dotproduct.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/05-low-level-optimization/vec/dotproduct.c).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Benchmark Cycles Per Element (CPE) for $1 \times 1, 2 \times 2, 4 \times 4$ unrolling. Compare with [sslotin/amh-code: superscalar.cc](https://github.com/sslotin/amh-code) to observe port saturation and register pressure.
  - Sun: Benchmark Struct of Arrays (SoA) vs Array of Structs (AoS) in [csi/01-computer-architecture/07-memory-hierarchy-data-layout/metrics/metrics.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/07-memory-hierarchy-data-layout/metrics/metrics.go).
- Active Recall:
  - Why does simple $k \times 1$ loop unrolling fail to break the latency bound, while $k \times k$ unrolling with multiple accumulators breaks it?

## Week 12: Jan 25 – Jan 31, 2027 | Linking & The ELF Object File Format
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Chapter 7 (Linking: compiler drivers, static linking, object files, relocatable ELF format).
  - Thu–Fri: [CMU 15-213: Lecture 13 (Linking)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj). Symbol resolution (global, external, local; strong vs weak symbols).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Inspect ELF headers, section tables (`.text`, `.data`, `.bss`, `.rodata`), and symbol tables (`.symtab`) using `readelf -a` and `objdump -t`.
  - Wed–Fri: Write multiple C files exhibiting duplicate strong symbol errors, uninitialized weak symbol resolution, and hidden static symbols.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Trace relocation entries (`.rela.text`, `.rela.data`). Calculate relocated addresses by hand for PC-relative and absolute references.
  - Sun: Study Position-Independent Code (PIC), Global Offset Table (GOT), and Procedure Linkage Table (PLT) in shared dynamic libraries (`.so`).
- Active Recall:
  - How does the dynamic linker resolve a function call on first invocation using the PLT and GOT?

## Week 13: Feb 1 – Feb 7, 2027 | Exceptional Control Flow & Process Lifecycle
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 8.1–8.4 (Exceptions: asynchronous interrupts, synchronous traps/faults/aborts; process abstraction, dual-mode execution).
  - Thu–Fri: [CMU 15-213: Lecture 14 (ECF: Exceptions & Processes)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj), [Wei Yang: Lecture 14 (Exceptional Control Flow)](https://www.youtube.com/@weiyang2613/videos), and [UC Berkeley CS 162: Lectures 02 & 03 (Four Fundamental OS Abstractions, Dual-Mode Operation & Process Control)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Trace hardware trap entry: privilege switch from User to Kernel ring, saving user registers onto kernel stack, loading exception vector table.
  - Wed–Fri: Practice process management C code: `fork()` process hierarchies, reaping child processes with `waitpid()`, and observing zombie processes.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Implement basic command parsing and foreground job execution in [csi/01-computer-architecture/08-exceptional-control-flow/shell.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/08-exceptional-control-flow/shell.c).
- Active Recall:
  - What is the difference between a trap, an interrupt, a fault, and an abort?

## Week 14: Feb 8 – Feb 14, 2027 | Signals, Shell Lab & CodeCrafters Shell
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Section 8.5 (Signals, pending and blocked vectors, async-signal safety).
  - Thu–Fri: [CMU 15-213: Lecture 15 (ECF: Signals & Nonlocal Jumps)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj). Race conditions and atomic signal masking (`sigprocmask`).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Build signal handlers for `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+Z), and `SIGCHLD` in [labs/07_shlab/shlab-handout/tsh.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/07_shlab/shlab-handout/tsh.c).
  - Wed–Fri: Resolve the fork/SIGCHLD race condition and verify all 16 traces (`trace01.txt` through `trace16.txt`).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Complete [CodeCrafters: Build your own Shell](https://codecrafters.io/challenges/shell). Implement REPL, command execution, path searching, builtins (`exit`, `echo`, `type`, `pwd`, `cd`), redirection (`1>`, `2>`, `>>`), and pipelines (`|`).
- Active Recall:
  - Why is calling `printf` or `malloc` inside a signal handler dangerous?

## Week 15: Feb 15 – Feb 21, 2027 | Virtual Memory Topography & Address Translation
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Sections 9.1–9.5 (Virtual address space, physical addressing, pages, page tables, Page Faults, Translation Lookaside Buffer) and H&P 7th ed Chapter 2 (§2.4 Fast address translation with TLBs, page table walks, and Virtually Indexed Physically Tagged [VIPT] caches).
  - Thu–Fri: [CMU 15-213: Lectures 16 & 17 (Virtual Memory: Concepts & Systems)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj), [UC Berkeley CS 162: Lectures 10 & 11 (Address Translation, Multi-Level Paging, TLBs)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC), and OSTEP Chapters 13–19 (Address Spaces, Paging, TLBs).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Review [exercises/07_process_memory_layout.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.md). Verify `.text`, `.data`, `.bss`, heap, memory-mapped region, and stack addresses in C.
  - Wed–Fri: Calculate multi-level page table math: 4-level x86-64 page tables (PGD, P4D, PUD, PMD, PTE) for 48-bit virtual address spaces with 4KB pages.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Trace complete address translation: Virtual Address $\to$ TLB Hit/Miss $\to$ Multi-Level Page Table Walk $\to$ Physical Address $\to$ L1 Cache Tag/Index.
  - Sun: Experiment with `mmap` anonymous private mappings and shared file mappings in C.
- Active Recall:
  - Why does a multi-level page table drastically reduce memory footprint compared to a single-level page table for sparse address spaces?

## Week 16: Feb 22 – Feb 28, 2027 | Dynamic Memory Allocation & Implicit Free Lists
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Section 9.9 (Dynamic Memory Allocation: `malloc`/`free`, throughput vs utilization, internal/external fragmentation) and [UC Berkeley CS 162: Lecture 12 (Demand Paging & Page Replacement)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - Thu–Fri: [CMU 15-213: Lecture 18 (Dynamic Memory Allocation: Basic Concepts)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) and [Wei Yang: Lecture 19 (Dynamic Memory Allocation)](https://www.youtube.com/@weiyang2613/videos).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Review [exercises/09_dynamic_memory_lifecycle.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.md) and implement [exercises/11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md) in C.
  - Wed–Fri: Implement an implicit free list memory allocator in [labs/06_malloclab/malloclab-handout/mm.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/06_malloclab/malloclab-handout/mm.c) with first-fit search.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement immediate boundary-tag coalescing on `free` (four cases: previous/next block allocated/free).
  - Sun: Benchmark implicit free list on `malloclab` traces. Analyze throughput bottlenecks ($O(N)$ free list scan).
- Active Recall:
  - How do boundary tags (footer tags) allow $O(1)$ constant-time coalescing with the preceding block in memory?

## Week 17: Mar 1 – Mar 7, 2027 | Explicit Free Lists & Pointer Arithmetic in C
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: Explicit free list design: embedding `next` and `prev` pointers directly inside the payload of freed memory blocks.
  - Thu–Fri: [CMU 15-213: Lecture 19 (Dynamic Memory Allocation: Advanced Concepts)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj). Free list ordering policies: LIFO vs Address-Ordered.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Re-architect `mm.c` block layout: add `prev` and `next` pointer offsets inside free blocks without increasing minimum block size.
  - Wed–Fri: Implement doubly linked list insertion and deletion (`list_remove`, `list_insert`).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Connect explicit free list to `mm_malloc`, `mm_free`, and coalescing logic.
  - Sun: Benchmark explicit free list on trace files. Validate that allocation search time scales with $O(\text{free\_blocks})$ instead of $O(\text{total\_blocks})$.
- Active Recall:
  - Why does an explicit free list require zero extra memory overhead for pointers compared to an implicit list?

## Week 18: Mar 8 – Mar 14, 2027 | Segregated Free Lists & CodeCrafters SQLite B-Trees
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: Segregated free list architecture: size classes (powers of two: 32, 64, 128, 256, ...), segregated fits.
  - Thu–Fri: In-place block extension on reallocation; memory utilization optimization.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Implement array of segregated free list root pointers in `mm.c`.
  - Wed–Fri: Implement size class bucket selection using bitwise operations or `clz` (count leading zeros). Tune allocator to pass `malloclab` with score $> 85/100$.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Advance [CodeCrafters: Build your own SQLite](https://codecrafters.io/challenges/sqlite). Implement B-Tree interior page traversal and binary search on index records.
- Active Recall:
  - Why does a segregated free list approximate best-fit memory utilization while preserving constant-time first-fit speed?

## Week 19: Mar 15 – Mar 21, 2027 | xv6 Kernel Architecture & System Calls
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: [xv6 OS Kernel Walkthrough Playlist (Videos #1 & #2: Introduction & Overview, System Calls)](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs) and [MIT 6.S081: Lecture 1 (Operating System Interfaces) & Lecture 3 (OS Organization & System Calls)](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK).
  - Thu–Fri: xv6 Book Chapter 1 & 2; Dual-mode execution on RISC-V: User mode (U-mode) vs Supervisor mode (S-mode); `ecall` and `sret` instructions.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Build and boot [xv6-riscv](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv) in QEMU (`make qemu`). Inspect kernel boot sequence in `kernel/main.c`.
  - Wed–Fri: Implement MIT Lab 2 (System calls): implement `trace` system call in [xv6-riscv/kernel/sysproc.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/sysproc.c).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement `sysinfo` system call to collect free memory bytes and process count.
  - Sun: Verify user-space system call stub dispatch via `user/usys.S` and kernel dispatch table in `kernel/syscall.c`.
- Active Recall:
  - What happens to the CPU registers and program counter when a user program executes the RISC-V `ecall` instruction?

## Week 20: Mar 22 – Mar 28, 2027 | xv6 Hardware Page Tables & Kernel Paging
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: [xv6 OS Kernel Walkthrough Playlist (Videos #3 & #4: Page Tables & Memory Management)](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs) and [MIT 6.S081: Lecture 4 (Page Tables)](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK).
  - Thu–Fri: xv6 Book Chapter 3 (Page Tables); RISC-V Sv39 paging architecture (3-level page tables, 39-bit virtual addresses, 56-bit physical addresses); `satp` register.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Trace page table walking in [xv6-riscv/kernel/vm.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/vm.c) (`walk()` function).
  - Wed–Fri: Implement MIT Lab 3 (Page tables): print page table hierarchy (`vmprint`) showing PTE flags (`V`, `R`, `W`, `X`, `U`).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement shared user/kernel read-only page to accelerate `getpid()` without context switching to the kernel.
  - Sun: Verify virtual memory isolation: attempt to access kernel physical memory from user space and observe page fault exception.
- Active Recall:
  - What are the physical steps taken by the RISC-V MMU when traversing a Sv39 3-level page table upon a TLB miss?

## Week 21: Mar 29 – Apr 4, 2027 | xv6 Traps, Interrupts & Device Drivers
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: [xv6 OS Kernel Walkthrough Playlist (Videos #5 & #6: Traps, Interrupts, & Exception Handling)](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs) and [MIT 6.S081: Lecture 5 (Calling Conventions & Stack Frames RISC-V) & Lecture 9 (Interrupts)](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK).
  - Thu–Fri: xv6 Book Chapter 4 (Traps and System Calls); Trapframe struct, `stvec`, `sstatus`, `sepc`, `scause`; Trampoline page at `MAXVA`.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Trace trap lifecycle in [xv6-riscv/kernel/trap.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/trap.c): `uservec` $\to$ `usertrap` $\to$ `syscall` $\to$ `usertrapret` $\to$ `userret`.
  - Wed–Fri: Implement MIT Lab 4 (Traps): implement stack frame backtrace using saved frame pointer (`%s0`/`%fp`).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Implement user alarm system call (`sigalarm` / `sigreturn`): periodically interrupt execution and execute a user handler function.
  - Sun: Verify trapframe register restoration in `sigreturn`. Ensure clean test passage on `alarmtest`.
- Active Recall:
  - Why must the trampoline page be mapped at the exact same virtual address in both user page tables and the kernel page table?

## Week 22: Apr 5 – Apr 11, 2027 | xv6 Copy-on-Write (COW) Fork
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: [xv6 OS Kernel Walkthrough Playlist (Video #7 & #8: Lazy Allocation & Copy-on-Write)](https://www.youtube.com/watch?v=xieHeMeu1HQ&list=PLbtzT1TYeoMj_YvECDl0YqipVZqucZHXs) and [MIT 6.S081: Lecture 8 (Page Faults) & Lecture 16 (Virtual Memory Primitives)](https://www.youtube.com/watch?v=J3LCzufEYt0&list=PLTsf9UeqkReZHXWY9yJvTwLJWYYPcKEqK).
  - Thu–Fri: Copy-on-Write fork architecture: clearing `PTE_W`, setting COW software bit in PTE flags, physical page reference counting.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Implement physical page reference counting table in [xv6-riscv/kernel/kalloc.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/xv6-riscv/kernel/kalloc.c).
  - Wed–Fri: Modify `uvmcopy()` in `kernel/vm.c` to map child pages to parent physical pages read-only instead of allocating new physical pages.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Handle page fault in `usertrap()` (`scause == 15` store page fault): allocate new physical page, copy data, map page writable, decrement old page reference count.
  - Sun: Test and debug `cowtest`. Ensure memory reclamation when processes terminate. Pass all COW tests.
- Active Recall:
  - What race condition occurs if reference count increment/decrement operations in a COW kernel allocator are not protected by locks?

## Week 23: Apr 12 – Apr 18, 2027 | System-Level I/O & Network Sockets
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Chapters 10 & 11 (Unix I/O, file descriptors, open file table, v-node table, standard I/O vs robust I/O).
  - Thu–Fri: [CMU 15-213: Lectures 20 & 21 (System-Level I/O & Network Programming)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj). TCP/IP sockets API (`socket`, `bind`, `listen`, `accept`, `connect`).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Trace file descriptor duplication using `dup2()`. Experiment with redirecting stdout/stdin in C.
  - Wed–Fri: Build an echo client and iterative echo server using the CS:APP `Rio` (Robust I/O) package.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat: Inspect network packet captures from [csi/02-computer-networks/01-packet-capture-ctf](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/02-computer-networks/01-packet-capture-ctf).
  - Sun: Implement basic HTTP/1.0 request parser and response generator in [labs/08_proxylab/proxylab-handout/proxy.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/08_proxylab/proxylab-handout/proxy.c).
- Active Recall:
  - What is the difference between the open file table entry and the file descriptor table entry when a file is opened twice versus when `dup2` is called?

## Week 24: Apr 19 – Apr 25, 2027 | Concurrent Programming & CodeCrafters Redis (Part 1)
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: CS:APP Chapter 12 (Concurrent Programming: process-based, I/O multiplexing, thread-based) and [UC Berkeley CS 162: Lectures 04 & 05 (Concurrency, Synchronization, Locks, Semaphores)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - Thu–Fri: [CMU 15-213: Lectures 22 & 23 (Concurrent Programming & Synchronization Basics)](https://www.youtube.com/playlist?list=PLtv4PQFH2yMas_SCMTEaE4eUIWWWiwISj) and [Wei Yang: Recitation 14 (Synchronization)](https://www.youtube.com/@weiyang2613/videos).
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: POSIX threads (`pthread_create`, `pthread_join`, `pthread_detach`). Trace shared stack memory and race conditions on global counters.
  - Wed–Fri: Refactor [labs/08_proxylab/proxylab-handout/proxy.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/08_proxylab/proxylab-handout/proxy.c) to spawn a detached worker thread per client connection.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Start [CodeCrafters: Build your own Redis](https://codecrafters.io/challenges/redis). Implement TCP server binding, RESP protocol parsing (`PING`, `ECHO`), and concurrent connection handling with threads or goroutines.
- Active Recall:
  - Why is a detached thread required when threads do not need to be explicitly joined by the main parent thread?

## Week 25: Apr 26 – May 2, 2027 | Thread Synchronization, Caching & CodeCrafters Redis (Part 2)
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: H&P 7th ed Chapter 5 (§§5.1–5.3 Shared-memory multiprocessors, cache coherence problem, snooping bus protocols [MSI/MESI], write-invalidate vs write-update, and false sharing), [Wei Yang: Recitation 13 (Proxy Lab Caching)](https://www.youtube.com/@weiyang2613/videos), and [UC Berkeley CS 162: Lecture 06 (Monitors, Condition Variables, Readers-Writers)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC). Readers-Writer Problem (favoring readers vs favoring writers); coarse vs fine-grained locking.
  - Thu–Fri: OSTEP Chapter 32 (Common Concurrency Bugs) and [UC Berkeley CS 162: Lecture 07 (Deadlock & Priority Inversion)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC). Deadlock conditions, hold-and-wait, circular wait prevention.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Design an in-memory thread-safe LRU cache structure in [labs/08_proxylab/proxylab-handout/proxy.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/labs/08_proxylab/proxylab-handout/proxy.c) with maximum object size (100KB) and total cache size (1MB).
  - Wed–Fri: Implement Readers-Writer lock synchronization protecting cache lookups and cache insertions. Run `./driver.sh`.
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Complete [CodeCrafters: Build your own Redis](https://codecrafters.io/challenges/redis). Implement `SET` with expiration (`PX`), `GET`, in-memory key-value eviction, and RDB snapshot parsing.
- Active Recall:
  - What are the four Coffman conditions necessary for deadlock to occur?
  - In the MESI protocol, why does an attempt to write to a cache line in the Shared (S) state generate an invalidation request on the bus rather than writing directly?

## Week 26: May 3 – May 9, 2027 | High-Throughput Linux I/O & CodeCrafters Kafka
- Morning Theory (6:30–7:30 AM):
  - Mon–Wed: Modern Linux Event Demultiplexing: `select` vs `poll` vs `epoll` ($O(1)$ readiness notification, edge-triggered vs level-triggered) and [UC Berkeley CS 162: Lecture 16 (Networking, Sockets, and I/O Multiplexing)](https://www.youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC).
  - Thu–Fri: H&P 7th ed Chapter 5 (§§5.6–5.7 Memory Consistency Models: Sequential Consistency vs Total Store Order [TSO], store buffering with forwarding, and memory barriers), Linux `io_uring`, and C11 acquire-release atomics.
- Evening Exercises (8:00–9:30 PM):
  - Mon–Tue: Build a non-blocking TCP server using Linux `epoll` (`epoll_create1`, `epoll_ctl`, `epoll_wait`).
  - Wed–Fri: Benchmark atomic counters vs mutexes vs Go channels in [csi/03-languages-and-compilers/05-concurrency-patterns/counter-service/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/03-languages-and-compilers/05-concurrency-patterns/counter-service).
- Weekend Deep Work (2:00–5:00 PM & 8:00–9:30 PM):
  - Sat–Sun: Capstone project: [CodeCrafters: Build your own Kafka](https://codecrafters.io/challenges/kafka). Implement binary Kafka protocol serialization, commit log file writes to disk, and message batching.
- Active Recall:
  - Why does `epoll` scale to tens of thousands of idle connections ($O(1)$) while `select` degrades linearly ($O(N)$)?
  - Under x86-64's Total Store Order (TSO) memory model, why can a store operation be delayed past subsequent loads to different memory locations without an explicit memory fence?

---

# Verification & Milestone Tracking

## Phase Milestones
- Milestone 1 (End of Week 3): Cache Lab completed (Part A `csim` and Part B `trans` passing all autograder tests with full 53/53 score); CodeCrafters SQLite header parsing verified.
- Milestone 2 (End of Week 7): Bomb Lab fully defused (Phases 1–6 + secret phase) and Attack Lab completed (Phases 1–4 ROP chain).
- Milestone 3 (End of Week 11): 8-bit Von Neumann CPU emulator implemented; instruction-level parallelism and loop unrolling verified.
- Milestone 4 (End of Week 14): Shell Lab (`tsh.c`) fully implemented with complete signal masking and job control; CodeCrafters Shell passed.
- Milestone 5 (End of Week 18): Malloc Lab (`mm.c`) segregated free list memory allocator exceeding 85/100 performance index; CodeCrafters SQLite B-Tree search complete.
- Milestone 6 (End of Week 22): MIT 6.1810 / xv6 kernel labs passed (`syscall`, `pgtbl`, `traps`, `cow`).
- Milestone 7 (End of Week 26): Proxy Lab (`proxy.c`) multi-threaded caching proxy passed; CodeCrafters Redis and Kafka completed.
