# x86-64 Assembly Programming: Advanced Structures and Floating Point

## Mental Models and Theoretical Foundations

### Multi-Dimensional Arrays in Row-Major Order
- In C and x86-64, multi-dimensional arrays are flattened into a contiguous linear memory layout in row-major order:
  $$\text{Address}(A[i][j]) = x_A + L \times (i \times C + j)$$
  where $x_A$ is the base pointer, $L$ is element size in bytes ($4$ for `int`), $C$ is the total column count, $i$ is row index, and $j$ is column index.
- Efficient addressing combines integer multiplication (`imul`) for row strides with `lea` for scaling and base addition.

### Floating-Point Architecture and SIMD / SSE2 Registers
- Floating-point calculations do not use general-purpose integer registers (`%rax`, `%rbx`, etc.).
- x86-64 uses dedicated 128-bit vector registers (`%xmm0` through `%xmm15`):
  - Single-precision float arguments: passed in `%xmm0`, `%xmm1`, `%xmm2`, etc.
  - Double-precision float arguments: passed in `%xmm0`, `%xmm1`, etc.
  - Return values: placed in `%xmm0`.
- Instructions:
  - `movss`: Move scalar single-precision float.
  - `mulss`: Multiply scalar single-precision float.
  - `divss`: Divide scalar single-precision float.
  - `addss`: Add scalar single-precision float.
- Constants cannot be used as immediate operands in floating-point instructions; they must be stored in `.rodata` or `.data` and loaded via RIP-relative addressing (`[pi]`).

## Textbook Reference
- **CS:APP3e**: Chapter 3 (*Machine-Level Representation of Programs*), Section 3.8 (*Array Allocation and Access*) and Section 3.11 (*Floating-Point Code*).

## Problem Set and Specifications

### Exercise 1: `fib.asm`
- Signature: `int fib(int n);`
- Objective: Compute the $n$-th Fibonacci number ($F_0=0, F_1=1, F_2=1, F_3=2, \dots$).
- Calling Convention:
  - Input: $n$ passed in `%edi`
  - Output: result in `%eax`
- Challenge:
  - Implement iteratively in $O(n)$ time using registers `%eax`, `%edx`, `%ecx`.
  - Or implement recursively, maintaining 16-byte stack alignment invariants and preserving `%rbx`/`%r12` across recursive calls.

### Exercise 2: `matrix.asm`
- Signature: `int index(int *matrix, int rows, int cols, int rindex, int cindex);`
- Objective: Return the 32-bit integer at coordinates `(rindex, cindex)` from a 2D integer matrix array.
- Calling Convention:
  - `%rdi`: base pointer to `int` matrix
  - `%rsi`: number of rows
  - `%rdx`: number of columns ($C$)
  - `%rcx`: row index ($i$)
  - `%r8`: column index ($j$)
  - Output: 32-bit integer loaded into `%eax`
- Formula:
  $$\text{Address} = \text{matrix} + 4 \times (rindex \times cols + cindex)$$

### Exercise 3: `cone.asm`
- Signature: `float volume(float radius, float height);`
- Objective: Compute the volume of a right circular cone:
  $$V = \frac{\pi \times r^2 \times h}{3}$$
- Calling Convention:
  - `%xmm0`: `radius` (single-precision float)
  - `%xmm1`: `height` (single-precision float)
  - Output: `volume` in `%xmm0`
- Constants:
  - Define `pi: dd 3.14159` and `const_3: dd 3.0` in `.rodata`.
  - Use `default rel` to enable position-independent RIP-relative loads (`movss xmm2, [pi]`).

## Build and Verification
```bash
make clean && make
```
- Or run under Docker/Colima x86-64 container:
  ```bash
  docker run --rm -v "$(pwd):/src" -w /src gcc:latest bash -c "apt-get update && apt-get install -y nasm && make clean && make"
  ```

## Active Recall Checkpoint
- How does the row-major ordering formula change if elements are 8-byte `double` values instead of 4-byte `int`s?
- Why can immediate constants like `$3.14159` not be encoded directly into `mulss` instructions?
- What is RIP-relative addressing and why is it mandatory for modern 64-bit Position-Independent Executables (PIE)?
