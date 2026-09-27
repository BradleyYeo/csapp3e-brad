# Workspace Organization and Textbook Source Reference

## Textbook Source Reference
- Book: *Computer Systems: A Programmer's Perspective (3rd Edition)* (CS:APP3e) by Randal E. Bryant and David R. O'Hallaron.
- Chapter: Chapter 3 (*Machine-Level Representation of Programs*).
- Practice problems (e.g., Problem 3.1) are integrated inline within chapter sections, with solutions in the chapter appendix.
- Homework problems (e.g., Problem 3.67, 3.71) are located at the end of Chapter 3.

## Workspace Directories and Solution Locations
- Chapter 3 solutions directory: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3)
- Clang compilation script: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/run.sh](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/run.sh)
- Attack Lab project directory: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab)
- Attack Lab PDF writeup: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/attacklab.pdf](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/attacklab.pdf)
- Attack Lab Phase 1 notes: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P1.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P1.md)
- Attack Lab Phase 2 notes: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P2.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P2.md)
- Attack Lab target binaries: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/target1](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/target1)

# Essential Practice Problems for Assembly Fundamentals

## Operand Specifiers and Addressing Modes
### Problem 3.1: Decoding Memory Reference Forms
- Location in textbook: CS:APP3e Section 3.4.1 (*Operand Specifiers*), page 181.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_1.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_1.txt)
- Setup values:
  - `%rax = 0x100`
  - `%rcx = 0x1`
  - `%rdx = 0x3`
  - Memory `[0x100] = 0xFF`
  - Memory `[0x104] = 0xAB`
  - Memory `[0x108] = 0x13`
  - Memory `[0x10C] = 0x11`
- Operands to evaluate:
  - `%rax`: Register value `0x100`
  - `0x104`: Absolute memory access `[0x104] = 0xAB`
  - `$0x108`: Immediate value `0x108`
  - `(%rax)`: Indirect memory access `[0x100] = 0xFF`
  - `4(%rax)`: Base + displacement `[0x100 + 4] = [0x104] = 0xAB`
  - `9(%rax,%rdx)`: Base + index + displacement `[0x100 + 0x3 + 9] = [0x10C] = 0x11`
  - `260(%rcx,%rdx)`: `[0x1 + 0x3 + 260] = [0x108] = 0x13`
  - `0xFC(,%rcx,4)`: Scaled index + displacement `[0xFC + 4] = [0x100] = 0xFF`
  - `(%rax,%rdx,4)`: Base + scaled index `[0x100 + (3 * 4)] = [0x10C] = 0x11`
- Mental model: Distinguish immediate constants (`$`), direct register values, and dereferenced memory addresses (`(...)`). Critical for interpreting GDB disassembly in Attack Lab.

### Problem 3.4: Data Movement and Sign Extension
- Location in textbook: CS:APP3e Section 3.4.2 (*Data Movement Instructions*), page 184.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_4.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_4.c)
- Focus: Determine instruction variants (`movb`, `movw`, `movl`, `movq`, `movsbl`, `movzwl`) for type conversions between `char`, `short`, `int`, and `long`.
- Mental model: Register slicing ($64$-bit `%rax` vs $32$-bit `%eax` vs $8$-bit `%al`) and how $32$-bit writes automatically zero-extend into the upper $32$ bits of a $64$-bit register.

### Problem 3.6 and Problem 3.7: Leaq as Pure Arithmetic
- Location in textbook: CS:APP3e Section 3.5.1 (*Load Effective Address*), pages 191–192.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_6.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_6.c)
- Focus: Compute linear expressions like $x + 4y$, $9x$, and $7x + 60$ using single `leaq` instructions.
- Mental model: `leaq` calculates addresses or polynomial math without reading from or writing to memory; contrast directly with `movq`.

## Register Conventions and Clearing Idioms
### Problem 3.11: Register Zeroing Idioms
- Location in textbook: CS:APP3e Section 3.5.2 (*Unary and Binary Operations*), page 195.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_11.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_11.txt)
- Focus: Analyze why compilers emit `xorl %edx, %edx` instead of `movq $0, %rdx`.
- Mental model: Instruction byte density (2 bytes vs 7 bytes) and CPU register renaming eliminating execution latency.

# Essential Problems for Procedure Linkage and Stack Mechanics

## Call and Return Protocol
### Problem 3.32: Instruction Pointer and Stack Pointer Execution Trace
- Location in textbook: CS:APP3e Section 3.7.1 (*Passing Control*), page 231.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_32.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_32.txt)
- Focus: Step-by-step trace of `%rip` and `%rsp` values during sequential execution, `callq`, and `retq`.
- Mental model: `callq` decrements `%rsp` by $8$ and pushes the address of the next sequential instruction; `retq` pops the $8$-byte return address from the top of the stack into `%rip` and increments `%rsp` by $8$.
- Direct Attack Lab link: The core mechanism of Phase 1 through Phase 5 depends entirely on manipulating what `retq` pops off the stack into `%rip`.

## Register Preservation and Argument Passing
### Problem 3.34: Caller-Saved vs Callee-Saved Invariants
- Location in textbook: CS:APP3e Section 3.7.5 (*Managing Local Storage on the Stack*), page 240.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_34.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_34.txt)
- Callee-saved registers: `%rbx`, `%rbp`, `%r12`–`%r15`. A procedure must restore their original values before returning.
- Caller-saved registers: `%rax` (return value), `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9` (first 6 function arguments), `%r10`, `%r11`.
- Direct Attack Lab link: Phase 2 and Phase 4 require passing a cookie value as the first argument to target functions (`touch2`), which means loading the value into `%rdi` prior to jumping or returning into the target.

# Essential Problems for Buffer Overflow and Exploit Mechanics

## Buffer Layout and Overwrite Mechanics
### Problem 3.46: Canonical Buffer Overflow Stack Reconstruction
- Location in textbook: CS:APP3e Section 3.10.3 (*Out-of-Bounds Memory References and Buffer Overflow*), pages 262–264.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_46.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_46.txt)
- Focus: Reconstruct the stack memory layout byte-by-byte for a function executing an unsafe `gets(buf)` call.
- Trace how string inputs exceeding buffer boundaries sequentially overwrite:
  - Local padding and compiler alignment bytes.
  - Saved base pointer (`%rbp`) if a frame pointer is present.
  - Saved return address (`%rip`).
  - Caller stack frame storage.
- Direct Attack Lab link: Directly mirrors Phase 1 ([attacklab/P1.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P1.md)). Solves the exact offset calculation required to overwrite the return address without corrupting unintended caller state.

## Memory Exploit Defenses and Mitigations
### Problem 3.47: Address Space Layout Randomization (ASLR)
- Location in textbook: CS:APP3e Section 3.10.4 (*Thwarting Buffer Overflow Attacks*), pages 268–269.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_47.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_47.txt)
- Focus: Calculate the entropy and address range randomization of stack allocations across program executions.
- Mental model: Stack randomization prevents hardcoded code-injection targets by shifting the stack base on every invocation; motivates the transition from code injection (`ctarget`) to Return-Oriented Programming (`rtarget`).

### Problem 3.48: Stack Protector and Canary Mechanics
- Location in textbook: CS:APP3e Section 3.10.4 (*Thwarting Buffer Overflow Attacks*), pages 270–271.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_48.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_48.txt)
- Focus: Analyze GCC stack canaries: canary placement between local buffers and saved return addresses.
- Mental model: Canary validation happens immediately before `retq`; any linear contiguous buffer overflow that reaches the return address corrupts the canary first and triggers `__stack_chk_fail`.

### Problem 3.49: Dynamic Stack Allocation and Alignment
- Location in textbook: CS:APP3e Section 3.10.5 (*Variable-Size Stack Frames*), pages 274–276.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_49.txt](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_49.txt)
- Focus: Analyze variable-sized stack frames allocated via `alloca` or variable-length arrays (VLAs).
- Mental model: How compilers maintain 16-byte stack alignment invariants (`andq $-16, %rsp`) and use `%rbp` as an anchor frame pointer.

# Selected Homework Problems for Deep Mastery

## Homework Problem 3.67: Passing Structures on the Stack
- Location in textbook: CS:APP3e End of Chapter 3 Homework Problems, pages 294–295.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_67.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_67.c)
- Focus: Reverse-engineer assembly passing aggregate structures by value and reference.
- Reinforces how arguments beyond the first 6 registers are staged on the stack at positive offsets from `%rsp`.

## Homework Problem 3.71: Writing Safe Buffer Handlers
- Location in textbook: CS:APP3e End of Chapter 3 Homework Problems, page 297.
- Target solution file: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_71.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/c3/3_71.c)
- Focus: Implement `good_echo` to safely read arbitrary-length lines using `fgets` instead of unsafe `gets`.
- Reinforces defensive programming against buffer overflows in user-space code.

# Attack Lab Phase Alignment Guide

## Target 1: Code Injection (`ctarget`)
### Phase 1: Overwriting Return Address (`touch1`)
- Focus: Practice Problem 3.46.
- Working notes: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P1.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P1.md)
- Objective: Calculate byte distance from `buf` start to the saved return address; pad with dummy bytes and append the little-endian entry address of `touch1`.

### Phase 2: Injecting Bytecode into Stack (`touch2`)
- Focus: Practice Problems 3.4, 3.11, 3.32, 3.34.
- Working notes: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P2.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab/P2.md)
- Objective: Write x86-64 assembly instructions to set register `%rdi` to your cookie value and transfer control to `touch2`.
- Assembly instructions needed:
  - `movq $COOKIE, %rdi`
  - `pushq $TOUCH2_ADDR; ret` (or `jmp *%rax`)
- Tooling: Assemble with `gcc -c` and extract raw hex opcodes with `objdump -d`.

### Phase 3: String Pointer Passing (`touch3`)
- Focus: Practice Problems 3.1, 3.32, 3.46.
- Working directory: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab)
- Objective: Pass the address of an ASCII hex string of your cookie to `touch3`. Requires calculating the exact stack address where your payload string resides and loading that address into `%rdi`.

## Target 2: Return-Oriented Programming (`rtarget`)
### Phase 4: Basic Gadget Chaining (`touch2`)
- Focus: Practice Problems 3.32, 3.34, 3.47.
- Working directory: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab)
- Objective: Stack is non-executable ($W \oplus X$ / NX enabled) and randomized. Construct an exploit without injecting executable code by chaining existing instruction sequences (gadgets) ending in `ret` (`c3`) from `farm.c`.
- Gadget pattern: Pop cookie from stack into a register (`popq %rax; ret`), then copy into first argument register (`movq %rax, %rdi; ret`), followed by the address of `touch2`.

### Phase 5: Dynamic Address Computation Gadgets (`touch3`)
- Focus: Practice Problems 3.6, 3.32, 3.49.
- Working directory: [/Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/attacklab)
- Objective: Compute the runtime address of your cookie string on the randomized stack using relative arithmetic gadgets (`movq %rsp, ...`, `add`, `leaq`), stage it in `%rdi`, and return to `touch3`.