# x86-64 Assembly Programming: Fundamentals

## Mental Models and Theoretical Foundations

### The System V AMD64 ABI Calling Convention
- Functions receive up to 6 integer/pointer arguments in registers before spilling to the stack:
  - 1st argument: `%rdi`
  - 2nd argument: `%rsi`
  - 3rd argument: `%rdx`
  - 4th argument: `%rcx`
  - 5th argument: `%r8`
  - 6th argument: `%r9`
- Additional arguments beyond 6 are pushed onto the stack in reverse order.
- Function return values are placed in `%rax` (64-bit) or `%eax` (32-bit).

### Register Preservation Invariants
- **Caller-Saved Registers**: `%rax`, `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9`, `%r10`, `%r11`. The caller cannot assume their contents will be preserved across a `call` instruction.
- **Callee-Saved Registers**: `%rbx`, `%rbp`, `%r12`, `%r13`, `%r14`, `%r15`. If a function modifies any of these registers, it must push them to the stack upon entry and restore (`pop`) them prior to `ret`.
- **Stack Alignment Invariant**: The System V ABI requires the stack pointer `%rsp` to be 16-byte aligned before issuing any `call` instruction.

### Instruction Operands and Addressing Forms
- **Immediate**: `$Val` (e.g. `$42`).
- **Register**: `%rax`, `%edi`, `%al`. Writing to a 32-bit sub-register (e.g. `movl $1, %eax`) automatically zero-extends into the upper 32 bits of `%rax`.
- **Memory Dereference**: `Imm(Rb, Ri, S)` computes $\text{Address} = \text{Imm} + \text{Rb} + (\text{Ri} \times S)$, where scale $S \in \{1, 2, 4, 8\}$.
- **Leaq (Load Effective Address)**: `leaq (%rdi, %rsi, 4), %rax` computes the polynomial address without touching memory. Used for fast arithmetic.

## Textbook Reference
- **CS:APP3e**: Chapter 3 (*Machine-Level Representation of Programs*), Sections 3.1–3.7.
- Practice Problems: 3.1, 3.4, 3.6, 3.11, 3.32, 3.34 in [Exercises.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/Exercises.md).

## Problem Set and Specifications

### Exercise 1: `sum_to_n.asm`
- Signature: `int sum_to_n(int n);`
- Objective: Compute the sum $1 + 2 + \dots + n$ for integer $n$. Return 0 if $n \le 0$.
- Input: `%rdi` (or `%edi`)
- Output: `%rax` (or `%eax`)
- Key Instructions: `xor eax, eax`, `add rax, rdi`, `dec rdi`, `jg loop`, `ret`.

### Exercise 2: `binary_convert.asm`
- Signature: `int binary_convert(char *bits);`
- Objective: Parse a null-terminated ASCII string containing `'0'` and `'1'` characters and return the integer value.
- Algorithm:
  - Initialize accumulator `%rax = 0`.
  - Loop over characters: read byte `movzx edx, byte [rdi]`.
  - If byte is 0, exit loop.
  - Shift accumulator: `shl rax, 1`.
  - If char is `'1'`, `or rax, 1` (or `inc rax`).
  - Advance pointer `inc rdi`.
  - Return `%rax`.

### Exercise 3: `pangram.asm`
- Signature: `int pangram(char *phrase);`
- Objective: Determine if the phrase contains every letter from `'a'` to `'z'` case-insensitively. Return 1 (true) or 0 (false).
- Bitmask Technique:
  - Maintain a 32-bit register `%edx = 0`.
  - For each character: check if letter `'a'-'z'` or `'A'-'Z'`. Convert to lowercase index $0 \le i \le 25$.
  - Set bit: `bts edx, ecx` (or `mov ebx, 1; shl ebx, cl; or edx, ebx`).
  - After processing string, compare `%edx` against `0x03ffffff` (`(1 << 26) - 1`).

## Build and Verification

### macOS (Apple Silicon with Rosetta / x86-64 Toolchain)
```bash
make clean && make
```

### Linux (Docker or Colima)
```bash
# Using Colima / Docker container:
docker run --rm -v "$(pwd):/src" -w /src gcc:latest bash -c "apt-get update && apt-get install -y nasm && make clean && make"
```

## Active Recall Checkpoint
- Why do compilers emit `xorl %edx, %edx` instead of `movq $0, %rdx` to clear registers?
- How does the `callq` instruction mutate `%rsp` and memory, and how does `retq` reverse this?
- Why must callee-saved registers be preserved by subroutines while caller-saved registers can be overwritten freely?
