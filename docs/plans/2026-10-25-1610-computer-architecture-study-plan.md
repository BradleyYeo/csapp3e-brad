# Computer Architecture and Systems Foundations Study Plan (Oct 2026 – Jan 2027)
Created: 2026-10-25

A 10-week curriculum schedule integrating CS:APP3e readings, Attack Lab milestones, and Bradfield CSI hands-on exercises in C, Go, and x86-64 assembly.

## Executive Context & Constraints

- **Timeframe**: 25 October 2026 to 4 January 2027 (10 weeks, 71 days).
- **Available Time**: Weekdays 1–2 hours/day (~7.5 hours/week), Weekends 4–6 hours/weekend (~5 hours/week). Total weekly capacity: ~12.5 hours/week (~125 hours cumulative).
- **Core Scope**:
  - CS:APP3e Chapters: 3 (Machine Programming / Attack Lab), 4 (Processor Architecture), 5 (Optimization), 6 (Memory Hierarchy), 8 (Exceptional Control Flow).
  - Target Folders: `BradfieldCSI/00-prep-work` and `BradfieldCSI/01-computer-architecture`.
- **Selected Preferences**:
  - Attack Lab: Complete Phases 1–3 (`ctarget` code injection) and Phase 4 (`rtarget` basic ROP gadgets); Phase 5 optional extension.
  - Prep Work: Comprehensive 2.5-week foundational review covering K&R C, POSIX `ls_clone.c`, Go idiom internals, and shell pipelines.

---

## Conceptual Architecture & Learning Principles

- **Ousterhout's Deep Modules**: Hardware abstractions (ALU, cache controllers, process managers) expose thin, simple interfaces while encapsulating deep operational complexity. Study each subsystem by first isolating its interface boundary from internal mechanisms.
- **Jackson's Concept Design**: Separate architectural concepts (e.g. instruction decoding, register renaming, hazard detection, cache indexing) into independent mental models before composing them into unified pipelined CPUs.
- **Koppel's Single Authority Principle**: Identify the single authority responsible for every state transition in hardware and operating systems — such as the Program Counter for control flow, the hazard detection unit for bubble stalls, and the kernel process table for PID lifecycles.
- **Active Recall & Deliberate Practice**: Implement all programming exercises from scratch using starter skeletons before consulting `reference-solution/` implementations.

---

## 10-Week Master Curriculum Schedule

### Phase 1: Systems Foundations, Attack Lab & Tooling (Weeks 1 – 2.5: Oct 26 – Nov 11)
Total Time: ~32 Hours | Focus: K&R C, Go Language, Unix Pipelines, Attack Lab Phases 3 & 4

#### Week 1: Oct 26 – Nov 1 | K&R C Foundations & Attack Lab Phase 3
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master pointer arithmetic, stack layout, and solve Attack Lab Phase 3 (`ctarget` `touch3`).
- Daily Breakdown:
  - Monday (1.5h): Review [exercises/01_pointer_traversal.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_pointer_traversal.md) and complete K&R Chapter 1 exercises in [00-prep-work/c-programming/Exercises/Chapter1/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/Exercises/Chapter1).
  - Tuesday (1.5h): Complete K&R Chapter 2 & 3 exercises (operators, bitwise logic, control statements).
  - Wednesday (1.5h): CS:APP Section 3.7 review (procedure call frames, stack pointer `%rsp`, return address mechanism).
  - Thursday (1.5h): Attack Lab Phase 3 (`ctarget` `touch3` disassembly). Inspect `touch3` and `hexmatch`. Trace hex string format expectations and stack pointer offset calculations.
  - Friday (1.5h): Craft byte-sequence exploit for Phase 3. Assemble machine code to move string address to `%rdi` and return to `touch3`.
  - Saturday (2.5h): Verify Phase 3 exploit with `hex2raw` inside Colima/Docker x86-64 container (`docker run --cap-add=SYS_PTRACE ...`).
  - Sunday (2.5h): Implement [00-prep-work/c-programming/projects/ls_clone.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c). Practice POSIX file APIs: `opendir`, `readdir`, `lstat`, and mode bitmask inspection (`S_ISDIR`, `S_IRUSR`).
- Active Recall Checkpoint:
  - Why must the string pointer passed to `touch3` point to the injected string on the stack rather than a constant address?
  - How does `hexmatch` allocate its own stack frame without overwriting the injected payload?

#### Week 2: Nov 2 – Nov 8 | Advanced C, Shell Pipelines & Attack Lab Phase 4 (ROP)
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master function pointers, Unix stream automation, and Return-Oriented Programming (ROP).
- Daily Breakdown:
  - Monday (1.5h): K&R Chapter 4 (functions, external variables, header file modularity).
  - Tuesday (1.5h): K&R Chapter 5 (pointers to pointers, function pointer dispatch tables).
  - Wednesday (1.5h): Unix stream tooling: run and inspect [00-prep-work/shell-scripting/wiki.sh](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/shell-scripting/wiki.sh). Practice regex lookarounds (`grep -Po`) and `sed` stream parsing.
  - Thursday (1.5h): CS:APP Section 3.12 (Buffer Overflow Protection: ASLR, stack canaries, non-executable stack $W \oplus X$). Introduction to ROP on `rtarget`.
  - Friday (1.5h): Disassemble `rtarget`. Search for gadgets in `start_farm` and `mid_farm`. Identify byte offsets for `popq %rax` (`58 c3`), `movq %rax, %rdi` (`48 89 c7 c3`).
  - Saturday (2.5h): Construct Phase 4 ROP chain: stack padding (40 bytes), Gadget 1 address, cookie value, Gadget 2 address, `touch2` entry address.
  - Sunday (2.5h): Execute and validate Phase 4 exploit inside GDB. Review Phase 5 (ROP string generation) as an optional architectural puzzle.
- Active Recall Checkpoint:
  - Why does the $W \oplus X$ (NX/DEP) memory protection prevent Phase 1–3 attacks but fail to stop ROP attacks?
  - What determines where the CPU reads the next instruction address after executing `ret` (`c3`) in a gadget?

#### Week 2.5: Nov 9 – Nov 11 | Go Language Systems Internals
- Duration: 3 Days (~4.5h total).
- Goals: Build fluency in Go language runtime mechanics for CPU emulator and system tooling.
- Daily Breakdown:
  - Monday (1.5h): Study [00-prep-work/go-programming/learning-go/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/go-programming/learning-go): slice header internals (`Data`, `Len`, `Cap`), pointer mechanics, and type assertions.
  - Tuesday (1.5h): Go closures, defer stacks, and error handling patterns.
  - Wednesday (1.5h): Build [00-prep-work/go-programming/xkcd/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/go-programming/xkcd) CLI to practice JSON streaming decoding and file persistence.
- Active Recall Checkpoint:
  - How does passing a slice to a function differ from passing an array in terms of memory layout and copying overhead?

---

### Phase 2: CPU Micro-Architecture, ISAs & Emulation (Weeks 3 – 4: Nov 12 – Nov 25)
Total Time: ~25 Hours | Focus: CS:APP Chapter 4, Von Neumann Architecture, CPU Emulator Implementation, Hardware Hazards

#### Week 3: Nov 12 – Nov 18 | Von Neumann Architecture & Emulator Implementation
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Understand instruction fetch-decode-execute cycles by building an 8-bit CPU emulator from scratch in Go.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Sections 4.1 & 4.2 (Instruction Set Architecture, Y86-64 instructions, logic gates, and hardware control languages).
  - Friday (1.5h): Inspect [01-computer-architecture/01-virtual-machine/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/01-virtual-machine). Define memory layout (256-byte RAM), registers (`r1`, `r2`), Program Counter (`PC`), and instruction opcodes (`halt`, `load`, `store`, `add`, `sub`).
  - Saturday (2.5h): Implement core CPU execution loop: opcode extraction, operand decoding, memory bus read/write operations, ALU additions and subtractions with overflow detection.
  - Sunday (2.5h): Implement control instructions (`jump`, `beqz` conditional branch). Execute test suites with `go test -v`.
  - Monday (1.5h): Implement immediate instructions (`addi`, `subi`). Assemble and run a `sum_to_n` machine code program on your emulator.
  - Tuesday (1.5h): Review [01-computer-architecture/02-binary-encodings/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/02-binary-encodings) alongside CS:APP Chapter 2 (two's complement wrap-around, IEEE 754 float sign-exponent-fraction fields).
  - Wednesday (1.5h): Parse binary BMP image headers and TCP packet header bit fields.
- Active Recall Checkpoint:
  - What constitutes the minimal architectural state of a Von Neumann CPU between consecutive clock cycles?
  - Why is the Program Counter incremented during the Fetch stage rather than during the Execute stage?

#### Week 4: Nov 19 – Nov 25 | Pipelined Processor Design, Hazards & Forwarding
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master pipelined execution stages, data hazards (RAW), control hazards, pipeline stalls, and data forwarding.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Section 4.3 (Sequential Y86-64 Implementation SEQ - stage-by-stage data paths).
  - Friday (1.5h): CS:APP Section 4.4 (General Principles of Pipelining: throughput, latency, clock cycle limits, pipeline registers).
  - Saturday (2.5h): CS:APP Section 4.5 (Pipelined Implementation PIPE: dividing execution into Fetch, Decode, Execute, Memory, Write-back). Deepen with [exercises/19_pipelining_hazards.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/19_pipelining_hazards.md).
  - Sunday (2.5h): Data Hazards & Forwarding: Trace hardware routing paths ($E \to E$, $M \to E$, $W \to E$). Diagram how forwarding avoids bubble stalls for ALU dependencies.
  - Monday (1.5h): The Load-Use Hazard: Analyze why forwarding alone cannot resolve a load immediately followed by an ALU operation using that value. Trace the 1-cycle stall bubble injection.
  - Tuesday (1.5h): Control Hazards: Branch prediction strategies (predict taken vs predict not taken) and branch penalty (2 bubbles to cancel mispredicted instructions).
  - Wednesday (1.5h): Pipeline hazard simulation: Hand-trace clock cycle tables for the `sum_to_n` assembly loop showing stages, stalls, and forwarded values.
- Active Recall Checkpoint:
  - Why can data not be forwarded backward in time during a load-use hazard?
  - What are the hardware mechanisms required to insert a bubble into a pipeline stage?

---

### Phase 3: Assembly Mastery, Binary Encoding & Micro-Optimization (Weeks 5 – 7: Nov 26 – Dec 16)
Total Time: ~38 Hours | Focus: System V AMD64 ABI, Advanced x86-64, SSE SIMD, CS:APP Chapter 5 Optimization

#### Week 5: Nov 26 – Dec 2 | System V AMD64 ABI & Assembly Intro
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Author handwritten x86-64 assembly conforming strictly to the 64-bit calling convention.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Sections 3.7 & 3.8 review. Register conventions: argument registers (`%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`), return register (`%rax`), caller-saved vs callee-saved registers.
  - Friday (1.5h): Implement [03-x86-64-assembly-intro/sum_to_n.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/03-x86-64-assembly-intro/sum_to_n.asm) using arithmetic loops and conditional jumps. Verify via `make test`.
  - Saturday (2.5h): Implement [03-x86-64-assembly-intro/binary_convert.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/03-x86-64-assembly-intro/binary_convert.asm): ASCII string parsing, bitwise shifting (`shlq`), and accumulator accumulation.
  - Sunday (2.5h): Implement [03-x86-64-assembly-intro/pangram.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/03-x86-64-assembly-intro/pangram.asm): 26-bit bitmask tracking with `bts` (bit test and set) or bitwise shifts, case conversion (`or $0x20`), and return flag verification.
  - Monday (1.5h): Code review and optimization of assembly solutions. Compare with compiler outputs (`gcc -O2 -S`).
  - Tuesday (1.5h): Callee-saved register discipline drills: proper preservation of `%rbx`, `%rbp`, `%r12-%r15` across function boundaries.
  - Wednesday (1.5h): ABI stack alignment: verify 16-byte stack boundary alignment before `call` instructions.
- Active Recall Checkpoint:
  - Why does the System V AMD64 ABI mandate 16-byte stack alignment immediately prior to executing a `call` instruction?
  - What is the performance and correctness consequence of omitting register preservation for a callee-saved register?

#### Week 6: Dec 3 – Dec 9 | Advanced Assembly: Recursion, Matrices & Floating Point SSE
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master recursive stack framing, 2D array row-major memory strides, and SSE vector floating point registers.
- Daily Breakdown:
  - Thursday (1.5h): Implement [04-x86-64-assembly-further/fib.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/04-x86-64-assembly-further/fib.asm): recursive Fibonacci implementing base cases ($n \le 1$), recursive calls, stack frame allocation, and callee-saved register restoration.
  - Friday (1.5h): Implement [04-x86-64-assembly-further/matrix.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/04-x86-64-assembly-further/matrix.asm): 2D row-major addressing calculation: $\text{addr} = \text{base} + 4 \times (r \times \text{cols} + c)$.
  - Saturday (2.5h): CS:APP Section 3.11 (Floating Point Architecture: XMM registers `%xmm0-%xmm15`, scalar operations).
  - Sunday (2.5h): Implement [04-x86-64-assembly-further/cone.asm](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/04-x86-64-assembly-further/cone.asm): calculate volume of a cone using SSE2 instructions (`mulsd`, `divsd`, loading $\pi$ constant, returning result in `%xmm0`). Run `make test`.
  - Monday (1.5h): Disassemble and trace matrix addressing using complex SIB addressing modes: `movl (%rdi, %rcx, 4), %eax`.
  - Tuesday (1.5h): Compare recursive `fib` stack depth and memory consumption against iterative accumulator implementations.
  - Wednesday (1.5h): Floating point comparison and branching: examine `ucomisd` and condition flags (`ZF`, `PF`, `CF`).
- Active Recall Checkpoint:
  - In a 2D matrix stored in row-major order, why does iterating by column stride cause significant cache misses compared to row stride?
  - How are floating-point arguments and return values passed in System V AMD64 compared to integer arguments?

#### Week 7: Dec 10 – Dec 16 | Instruction-Level Parallelism & Program Optimization
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master modern superscalar processor micro-architectures, out-of-order execution, loop unrolling, and multiple accumulators.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Sections 5.1–5.6 (Capabilities and Limitations of Optimizing Compilers, Program Example, Eliminating Loop Inefficiencies).
  - Friday (1.5h): CS:APP Sections 5.7–5.8 (Understanding Modern Processors: Instruction Control Unit, Execution Unit, functional units, latency vs issue time).
  - Saturday (2.5h): Implement [05-low-level-optimization/pagecount.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/05-low-level-optimization/pagecount.c): replace expensive 64-bit integer division with power-of-two bitwise shifts and masks. Measure throughput gains.
  - Sunday (2.5h): CS:APP Section 5.9–5.11 (Loop Unrolling, Enhancing Parallelism with Multiple Accumulators and Reassociation).
  - Monday (1.5h): Implement [05-low-level-optimization/vec/dotproduct.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/05-low-level-optimization/vec/dotproduct.c): implement $2 \times 2$ and $4 \times 4$ unrolling with independent accumulation registers.
  - Tuesday (1.5h): Benchmark cycles per element (CPE) for vector dot product. Observe how independent accumulators break loop-carried data dependency chains.
  - Wednesday (1.5h): CS:APP Section 5.12 (Branch Prediction and Misprediction Penalties). Study conditional move instructions (`cmov`) vs conditional branches.
- Active Recall Checkpoint:
  - Why does simple $k \times 1$ loop unrolling fail to exceed the latency bound of the functional unit, whereas $k \times k$ unrolling with multiple accumulators breaks through it?
  - How does out-of-order execution utilize register renaming to eliminate false dependencies (Write-After-Read, Write-After-Write)?

---

### Phase 4: Memory Hierarchy, Cache Tuning & Exceptional Control Flow (Weeks 8 – 10: Dec 17 – Jan 4)
Total Time: ~38 Hours | Focus: CS:APP Chapters 6 & 8, Cache Organization, Matrix Tiling, Struct Layouts, Unix Shell

#### Week 8: Dec 17 – Dec 23 | The Memory Hierarchy & Hardware Caching
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Understand cache memories, set-associative address mapping (Tag, Set Index, Block Offset), and cache blocking.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Sections 6.1–6.3 (Storage Technologies, Locality: temporal vs spatial locality).
  - Friday (1.5h): CS:APP Sections 6.4–6.5 (Cache Memories: direct-mapped, set-associative, fully associative caches; write hit/miss policies).
  - Saturday (2.5h): Cache Address Decoding: Deconstruct a 64-bit address into Tag bits ($t$), Set Index bits ($s$), and Block Offset bits ($b$). Calculate total cache capacity and miss penalties.
  - Sunday (2.5h): Run and profile [06-memory-hierarchy-caching/loop-order.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching/loop-order.c). Compare `ij`, `ji` loop nesting orders. Measure L1-D cache miss rates using Valgrind `cachegrind`.
  - Monday (1.5h): CS:APP Section 6.6 (Putting It Together: The Impact of Caches on Program Performance, The Memory Mountain).
  - Tuesday (1.5h): Implement [06-memory-hierarchy-caching/matrix-multiply.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/06-memory-hierarchy-caching/matrix-multiply.c): standard $O(N^3)$ matrix multiplication. Identify cache thrashing on matrix dimensions exceeding cache capacity.
  - Wednesday (1.5h): Implement blocked (tiled) matrix multiplication in `matrix-multiply.c`. Select optimal block size $B$ matching L1 cache capacity.
- Active Recall Checkpoint:
  - Why is the Set Index placed in the middle bits of an address rather than the high-order bits?
  - How does cache tiling / blocking transform an algorithm's cache miss rate from $O(N^3)$ to $O(N^3 / B)$?

#### Week 9: Dec 24 – Dec 30 | Memory Layout, Cache Line Pre-fetching & Data Layout
- Weekly Allocation: Weekdays 7.5h, Weekend 5h (~12.5h total).
- Goals: Master cache-conscious data structures, Array of Structs (AoS) vs Struct of Arrays (SoA), and false sharing.
- Daily Breakdown:
  - Thursday (1.5h): Memory alignment and padding in C and Go structs: field ordering to minimize struct alignment padding.
  - Friday (1.5h): Inspect [01-computer-architecture/07-memory-hierarchy-data-layout/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/07-memory-hierarchy-data-layout). Compare Array of Structs (AoS) vs Struct of Arrays (SoA).
  - Saturday (2.5h): Benchmark [metrics/metrics.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/07-memory-hierarchy-data-layout/metrics/metrics.go) using `go test -bench=. -benchmem`. Measure nanoseconds per operation between AoS and SoA.
  - Sunday (2.5h): Spatial Prefetching Analysis: trace how hardware stream prefetchers detect contiguous cache line accesses and pre-load adjacent 64-byte lines into L1.
  - Monday (1.5h): Multi-core cache coherence: MSI/MESI protocol fundamentals and false sharing between threads on the same cache line.
  - Tuesday (1.5h): False sharing mitigations: struct cache line padding (`64-byte` alignment).
  - Wednesday (1.5h): Synthesis review of memory hierarchy: Register $\to$ L1 $\to$ L2 $\to$ L3 $\to$ DRAM latency and bandwidth hierarchies.
- Active Recall Checkpoint:
  - When scanning only one field of a multi-field record, why does Struct of Arrays (SoA) provide a massive throughput improvement over Array of Structs (AoS)?
  - What is false sharing and how does it degrade multi-threaded throughput?

#### Week 10: Dec 31 – Jan 4 | Exceptional Control Flow & Systems Capstone (Unix Shell)
- Weekly Allocation: 5 Days (~12.5h total).
- Goals: Master OS exceptions, system calls, process state machines, signal safety, and complete a working Unix Shell.
- Daily Breakdown:
  - Thursday (1.5h): CS:APP Sections 8.1–8.2 (Exceptions: synchronous traps/faults/aborts vs asynchronous interrupts; system call mechanism).
  - Friday (1.5h): CS:APP Sections 8.3–8.4 (Processes, Process Control: `fork`, `execve`, `waitpid`, zombie processes, process states).
  - Saturday (2.5h): CS:APP Section 8.5 (Signals and Nonlocal Jumps: signal reception, pending and blocked bit vectors, async-signal-safety).
  - Sunday (2.5h): Implement [01-computer-architecture/08-exceptional-control-flow/shell.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/08-exceptional-control-flow/shell.c): command loop, parsing arguments, builtin command execution (`exit`, `cd`). Foreground job execution via `fork` and `execvp`.
  - Monday (2.5h): Background jobs (`&`) and asynchronous child reaping: implement `SIGCHLD` signal handler with `waitpid(-1, &status, WNOHANG | WUNTRACED)`. Mask signals (`sigprocmask`) around job list modifications to prevent race conditions.
  - Tuesday Jan 4 (2.0h): Final Capstone Verification: test shell with foreground commands, background commands, and pipeline signals (`Ctrl+C`, `Ctrl+Z`). Complete retrospective and active recall review.
- Active Recall Checkpoint:
  - Why is calling `printf` or `malloc` inside a Unix signal handler unsafe?
  - What race condition occurs if `SIGCHLD` is not blocked before `fork` and unblocked after adding the child to the job list?

---

## Tracking & Progress Milestones

- **Milestone 1 (Nov 1, End of Week 1)**: Attack Lab Phase 3 passed (`touch3`); K&R Chapters 1–3 complete; `ls_clone.c` working.
- **Milestone 2 (Nov 8, End of Week 2)**: Attack Lab Phase 4 passed (`rtarget` ROP); `wiki.sh` pipeline complete; K&R Chapters 4–5 complete.
- **Milestone 3 (Nov 18, End of Week 3)**: 8-bit Von Neumann CPU emulator implemented and passing all tests in `bradvm`.
- **Milestone 4 (Nov 25, End of Week 4)**: 5-stage pipeline hazard analysis complete; load-use and branch stalls mastered.
- **Milestone 5 (Dec 2, End of Week 5)**: System V AMD64 assembly exercises (`sum_to_n`, `binary_convert`, `pangram`) fully verified.
- **Milestone 6 (Dec 9, End of Week 6)**: Advanced assembly (`fib`, `matrix`, `cone`) complete; SSE2 floating point understood.
- **Milestone 7 (Dec 16, End of Week 7)**: Program optimization complete; ILP and multiple accumulators verified in `pagecount.c` and `dotproduct.c`.
- **Milestone 8 (Dec 23, End of Week 8)**: Cache hierarchy benchmarked in `loop-order.c`; tiled matrix multiplication implemented.
- **Milestone 9 (Dec 30, End of Week 9)**: SoA vs AoS data layout benchmarked in Go `metrics.go`.
- **Milestone 10 (Jan 4, End of Week 10)**: Capstone Unix shell complete with process handling, signal safety, and job control.
