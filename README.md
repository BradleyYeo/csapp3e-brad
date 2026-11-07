# 6 Month Curriculum

**1. First Principles: How CS:APP 3e Fits with Hardware Courses**
**CS:APP 3e (Computer Systems: A Programmer's Perspective)** lives strictly at the **Hardware/Software Interface** and **Systems Software** layers. Its fundamental question is: _"Given how modern hardware and operating systems work, how do I write fast, reliable, and secure C code?"_

Because CS:APP looks _upward_ toward the programmer, it treats two massive domains as semi-black boxes:

**Below the ISA (Microarchitecture & Digital Logic):** Chapter 4 of CS:APP builds a toy processor (Y86-64) to demystify pipelining, but it skips how real modern out-of-order cores, branch predictors, and DRAM controllers actually work in silicon.

**Inside the Kernel (OS Internals):** Chapters 8, 9, and 10 teach you how to _use_ system calls (fork, mmap, epoll) and virtual memory from user space, but they don't show you how the kernel actually implements page tables, context switches, or file systems in C and assembly.

NAND2Tetris Only do Part I (Projects 1–6), and timebox it to a single weekend or two. Skip Part II.

**What Nand2Tetris (Part I) does brilliantly:** It destroys the "magic" of hardware. In ~15–20 hours, you go from a single NAND gate to building an ALU, RAM, a Program Counter, and a working 16-bit CPU (Hack), finishing by writing an Assembler in a high-level language. It gives you the complete vertical mental model in record time.

**The Downside (Why it falls short for systems engineers):** The Hack architecture is a radical simplification. It has no pipelining, no caches, no byte-addressable memory, no interrupts, and no privilege rings (user vs. kernel mode). Furthermore, **Part II** (building a VM translator, compiler, and toy OS for a Java-like language) abstracts away the exact hardware realities—like pointer arithmetic, cache locality, and page tables—that you are trying to master in CS:APP.

**Take Nand2Tetris Part I** if you have never wired logic gates before and want a fast, tactile warmup before touching CS:APP Chapter 4.

**Replace Nand2Tetris with Onur Mutlu’s DDCA** (specifically the lectures + Harris & Harris textbook) if you want industrial-reality hardware design (pipelining, hazards, RISC-V/MIPS, real memory timing).

**3. The Integrated Systems & Architecture Curriculum**

Rather than taking courses sequentially in isolation, the best way to learn is **interleaving**—using hardware and OS courses to peel back the curtain on CS:APP chapters right as you read them.

**Phase 0: The Logic & Gate Warmup** _(Optional — 1 to 2 Weeks)_

**Goal:** Understand how electricity and boolean logic become a stateful machine controlled by a clock signal.

**Actionable Track:** **Nand2Tetris Part I** (Projects 1–5) _OR_ **Onur Mutlu DDCA Lectures 1–11**.

**Companion Text:** _Digital Design and Computer Architecture (RISC-V Edition)_ by Sarah Harris & David Harris (Chapters 1–5).

**Core Concepts:** Combinational logic, MUX/DEMUX, full adders, sequential logic, flip-flops, registers, clock skew, and Finite State Machines (FSMs).

**Phase 1: The ISA & Execution Engine** _(CS:APP Chapters 1–5)_

**Goal:** Master machine-level representation, assembly, and how a CPU executes instructions at high throughput.

**Primary Spine:** **CS:APP 3e Chapters 1, 2, 3, 4, and 5** (along with CMU 15-213 lectures).

**Mandatory Labs:**

datalab (bit manipulation & IEEE 754 floating point)

bomblab (reverse engineering x86-64 assembly with gdb)

attacklab (stack frames, buffer overflows, Return-Oriented Programming)

**Hardware Complement (Interleave after CS:APP Ch. 4 & 5):**

**Onur Mutlu DDCA (Lectures 12–18):** Single-cycle, multi-cycle, and pipelined microarchitectures, data/control hazards, forwarding, and branch prediction.

**Onur Mutlu Computer Architecture (Selected Lectures):** Out-of-Order (OoO) execution, Tomasulo’s Algorithm, Register Renaming, and Reorder Buffers (ROB).

_Why here?_ CS:APP Chapter 5 talks about Instruction-Level Parallelism (ILP), dataflow graphs, and latency bounds in C code; Mutlu's lectures show you the actual hardware structures executing your C loops out-of-order.

**Reference Text:** _Computer Organization and Design (Patterson & Hennessy)_ — Chapters 3 & 4.

**Phase 2: The Memory Hierarchy & Storage Engines** _(CS:APP Chapter 6)_

**Goal:** Understand the physical reality of the "Memory Wall," spatial/temporal locality, cache coherence, and DRAM internals.

**Primary Spine:** **CS:APP 3e Chapter 6** (The Memory Hierarchy).

**Mandatory Lab:** cachelab (building a C cache simulator + optimizing matrix transpose for cache line blocking).

**Hardware Complement (Deep Dive):**

**Onur Mutlu Computer Architecture (Memory Hierarchy & DRAM Lectures):**

Cache organization (VIPT vs. PIPT, write-buffers, MSHRs, hardware prefetchers).

**DRAM Internals:** Banks, rows, columns, row buffer hits/conflicts, refresh overhead, and memory controller scheduling.

**Multi-core Cache Coherence:** MESI/MOESI protocols and false sharing.

**Essential Short Read:** Ulrich Drepper’s classic paper, _"What Every Programmer Should Know About Memory"_ (Sections 1–6).

**Why this pairing works:** When you write a memory-bound storage engine or high-throughput data structure in C or Go, CS:APP teaches you cache-friendly layout (e.g., struct packing, stride-1 access), while Mutlu teaches you _why_ random access stalls on DRAM row buffer conflicts and TLB misses.

**Phase 3: Linking, Processes, Virtual Memory & The Kernel** _(CS:APP Chapters 7–10)_

**Goal:** Cross the boundary from user-space systems programming into actual Operating System kernel internals.

**Primary Spine:** **CS:APP 3e Chapters 7, 8, 9, and 10** (Linking, Exceptional Control Flow, Virtual Memory, System-Level I/O).

**Mandatory Labs:**

shlab (building a Unix shell with job control and signal handling)

malloclab (writing your own dynamic memory allocator using explicit/segregated free lists)

**Systems Complement 1 (Conceptual OS):** _Operating Systems: Three Easy Pieces (OSTEP)_ by Remzi & Andrea Arpaci-Dusseau.

Read **Part 1 (Virtualization)** alongside CS:APP Chapters 8 & 9. OSTEP’s explanations of multi-level page tables, TLBs, and page replacement policies are unmatched.

**Systems Complement 2 (Hands-on Kernel Engineering):** **MIT 6.1810 (formerly 6.828 / 6.S081): Operating System Engineering**.

Uses **xv6** (a clean, modern re-implementation of Sixth Edition Unix in ANSI C for RISC-V).

**Key Labs to complete:** syscall (tracing/adding a system call), pgtbl (inspecting and manipulating hardware page tables directly), traps (handling hardware interrupts), and cow (implementing Copy-on-Write fork()).

**Why this pairing works:** In CS:APP, fork() and mmap() look like magic C functions. In MIT 6.1810, you write the exact C and assembly code inside the kernel that swaps the satp page-table register, handles page faults, and saves trapframes.

**Phase 4: Concurrency, Networking & High-Performance I/O** _(CS:APP Chapters 11–12)_

**Goal:** Master multi-threaded synchronization, network sockets, event-driven I/O, and hardware-level memory ordering.

**Primary Spine:** **CS:APP 3e Chapters 11 and 12** (Network Programming, Concurrent Programming).

**Mandatory Lab:** proxylab (building a concurrent caching web proxy).

**Systems Complement:**

**OSTEP Part 2 (Concurrency):** Locks, condition variables, semaphores, and lock-free/event-based concurrency.

**Hardware Memory Models (Mutlu CompArch / C11 Atomics):** Learn why modern CPUs reorder loads and stores (Store Buffers, Invalidation Queues) and how hardware memory barriers (mfence, acquire/release semantics) work under the hood.

**Modern Linux I/O Extension:** CS:APP 3e covers select and basic multiplexing, which are dated for modern production systems. Supplement Chapter 11/12 by studying epoll (edge-triggered vs. level-triggered) and io_uring (shared ring buffers between user and kernel space to eliminate syscall overhead).

**Summary Roadmap Cheat-Sheet**

**Stage** **Core Reading (CS:APP 3e)** **CS:APP Lab** **Companion Course / Text** **Focus Area**

**0. Logic** _Pre-work_ — **Nand2Tetris Part I** _or_ **Harris & Harris Ch 1–5** Gates, ALU, Registers, Clocks

**1.** **CPU** Chapters 1–5 datalab, bomblab, attacklab **Mutlu DDCA** (Pipelining) + **CompArch** (OoO/Branch Pred.) Assembly, Stack, Pipelining, ILP

**2. Memory** Chapter 6 cachelab **Mutlu CompArch** (Caches, DRAM, Coherence) Spatial Locality, Cache Lines, DRAM

**3. Kernel/VM** Chapters 7–10 shlab, malloclab **OSTEP (Part 1)** + **MIT 6.1810 (xv6 labs)** Linking, ELF, Page Tables, Allocators

**4. Concurrency** Chapters 11–12 proxylab **OSTEP (Part 2)** + **Mutlu** (Memory Consistency) Threads, Atomics, Sockets, epoll

**How to Execute Without Getting Sidetracked**

The biggest trap in self-studying systems is **course collector syndrome**—watching 40 hours of video lectures without writing code.

Make **CS:APP 3e + its official Labs** your non-negotiable anchor. Never skip a CS:APP lab; that is where 80% of the neural wiring happens.

Treat **Onur Mutlu’s lectures** and **OSTEP chapters** as targeted deep-dives when you hit the corresponding chapter in CS:APP.

Save **MIT 6.1810 (xv6)** for right after you finish shlab and malloclab in Phase 3—at that point, your C pointer fluency and GDB skills will be sharp enough that reading kernel code will feel natural rather than overwhelming.