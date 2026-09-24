# Von Neumann Architecture Fundamentals

## Stored-Program Concept

- The Von Neumann model unifies program instructions and application data within a single shared memory address space.
- Both instructions and data are encoded as raw binary numbers; the central processing unit (CPU) distinguishes between them strictly based on context (whether an address is referenced by the Program Counter or by a data load/store instruction).
- In [introduction-prework/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go), memory is an array of 256 bytes (`[]byte` of size 256).
- Address layout partition:
  - `0x00`: Output register/result storage.
  - `0x01`: Input parameter X.
  - `0x02`: Input parameter Y.
  - `0x03` to `0x07`: Reserved data memory.
  - `0x08` to `0xff`: Instruction segment (code space).
- Comparison with Harvard architecture:
  - Harvard architecture uses physically separated memories and buses for instructions and data.
  - Modern general-purpose processors (x86, ARM) use a modified Harvard architecture: unified physical memory space at the system bus level, but separate L1 instruction cache (I-cache) and L1 data cache (D-cache) inside the CPU core.

## Architectural State vs Storage Hierarchy

### Registers

- Small, high-speed storage locations residing directly on the CPU die.
- Zero-latency access (~0.5 ns in physical silicon) compared to main memory (~50-100 ns).
- The VM exposes three 8-bit registers stored in `registers [3]byte`:
  - `registers[0]` (`pc`): Program Counter.
  - `registers[1]` (`r1`): General-purpose register 1.
  - `registers[2]` (`r2`): General-purpose register 2.

### Program Counter (PC)

- Dedicated register holding the memory address of the instruction currently being executed or scheduled for fetch.
- Automatically initializes to `0x08` (the entry point of the instruction segment) in [vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L40).

### Main Memory (RAM)

- Linear byte-addressable array (`[256]byte`).
- Holds input values, computation results, and machine code instructions side by side.
- Read/write access is mediated via software array indexing, representing memory bus read/write cycles.

# Instruction Set Architecture and Encoding

## Instruction Set Architecture (ISA)

- An ISA defines the abstract model of a computer, acting as the boundary contract between hardware execution units and software programs.
- It specifies supported data types, register sets, addressing modes, and the machine code binary encoding format.

## Instruction Formats and Variable-Length Encoding

### Instruction Length Categories

- CISC (Complex Instruction Set Computer) architectures like x86 use variable-length instructions to pack operations densely.
- RISC (Reduced Instruction Set Computer) architectures like RISC-V and ARM use fixed-length instructions (typically 32 bits) to simplify hardware decoders.
- The VM in [vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go) implements a variable-length instruction set:
  - 1-byte instruction: `Halt` (`0xff`).
  - 2-byte instruction: `Jump` (`0x07 <target_address>`).
  - 3-byte instructions: `Load`, `Store`, `Add`, `Sub`, `Addi`, `Subi`, `Beqz`.

### Binary Encoding Layouts

- `Halt` (`0xff`):
  - Byte 0: Opcode (`0xff`).
- `Jump` (`0x07`):
  - Byte 0: Opcode (`0x07`).
  - Byte 1: Target address (8-bit literal memory index).
- Three-byte operations:
  - Byte 0: Opcode (`0x01` through `0x06`, `0x08`).
  - Byte 1: Destination or evaluation register identifier (`0x01` for `r1`, `0x02` for `r2`).
  - Byte 2: Second operand:
    - Memory address for `Load` and `Store`.
    - Source register identifier for `Add` and `Sub`.
    - Immediate constant integer for `Addi` and `Subi`.
    - Relative jump offset for `Beqz`.

## Addressing Modes

### Register Addressing

- Operands are retrieved directly from CPU registers.
- Example: `add r1 r2` (`0x03 0x01 0x02`).
- Both source and destination values reside in the register file; no memory bus access is generated.

### Immediate Addressing

- The operand value is embedded directly inside the instruction stream bytes.
- Example: `addi r1 3` (`0x05 0x01 0x03`).
- Eliminates the need for a separate load from memory to introduce constant values into computation.

### Direct Memory Addressing

- The exact memory address is provided as a literal operand in the instruction.
- Example: `load r1 1` (`0x01 0x01 0x01`) and `store r1 0` (`0x02 0x01 0x00`).
- Generates a memory access cycle to the specified linear address.

### Relative Addressing

- The target address is specified as a displacement relative to the current Program Counter location.
- Example: `beqz r2 3` (`0x08 0x02 0x03`).
- Enables position-independent code (PIC), where blocks of machine code can be executed at arbitrary memory locations without rewriting address pointers.

# Fetch-Decode-Execute Cycle Mechanics

## Physical Machine Cycle vs Simulation Loop

- In physical hardware, the cycle is synchronized by an oscillating electronic clock signal.
- In [introduction-prework/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L43), the hardware clock is simulated by an infinite `for` loop.

## Fetch Phase

### Hardware Mechanics

- The CPU outputs the value of the Program Counter onto the address bus.
- The control bus asserts a memory read signal.
- The memory subsystem drives the byte stored at that address onto the data bus.
- The CPU latches the byte from the data bus into its internal Instruction Register (IR).

### Simulation Implementation

- Executed at [vm.go line 46](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L46):
  ```go
  opcode := memory[registers[pc]]
  ```
- Reads the single byte located at index `registers[pc]`.

## Decode Phase

### Hardware Mechanics

- Combinational logic circuits evaluate the bit patterns in the Instruction Register.
- Control lines are asserted to route data from register files, enable ALU operation modes, and configure bus drivers.

### Simulation Implementation

- The VM determines how many extra operand bytes need to be read based on the opcode:
  - `Halt`: `maxOffset = 0` (0 operand bytes).
  - `Jump`: `maxOffset = 1` (1 operand byte).
  - Others: `maxOffset = 2` (2 operand bytes).
- The operands are packed into a 16-bit integer via `decodeOperands`:
  ```go
  func decodeOperands(memory []byte, registers [3]byte, maxOffset byte) (operands uint16) {
      for offset := maxOffset; offset > 0x00; offset-- {
          operands |= (uint16(memory[registers[pc]+offset]) << (8 * (maxOffset - offset)))
      }
      return operands
  }
  ```

### Bitwise Packing and Unpacking Walkthrough

- Consider `load r1 1` stored at memory index 8:
  - `memory[8] = 0x01` (opcode: Load).
  - `memory[9] = 0x01` (operand 1: register `r1`).
  - `memory[10] = 0x01` (operand 2: memory address `1`).
- Inside `decodeOperands` with `maxOffset = 2`:
  - First iteration (`offset = 2`): reads `memory[10]` (`0x01`), shifted left by `8 * (2 - 2) = 0` bits. Accumulator `operands = 0x0001`.
  - Second iteration (`offset = 1`): reads `memory[9]` (`0x01`), shifted left by `8 * (2 - 1) = 8` bits (`0x0100`). Bitwise OR yields `operands = 0x0101`.
- Inside `loadProcedure`:
  - Unpack operand 2 (address): `address := byte(0x00 | operands)` extracts low byte (`0x01`).
  - Shift right: `operands >>= 8` leaves `0x0001`.
  - Unpack operand 1 (register): `register := byte(0x00 | operands)` extracts next byte (`0x01`).
  - Execute: `registers[1] = memory[1]`.

## Execute Phase

### Arithmetic Logic Unit (ALU) Operations

- Performed on registers using two's complement 8-bit arithmetic:
  - `Add`: `registers[r1] = registers[r1] + registers[r2]`.
  - `Sub`: `registers[r1] = registers[r1] - registers[r2]`.
  - `Addi`: `registers[r1] = registers[r1] + op1`.
  - `Subi`: `registers[r1] = registers[r1] - op1`.
- Overflow and underflow behavior:
  - Go's unsigned `byte` (`uint8`) wraps around modulo 256.
  - `255 + 1 = 0` (`0xff + 0x01 = 0x00`).
  - `0 - 1 = 255` (`0x00 - 0x01 = 0xff`).
  - Verified by tests in [vm_test.go lines 49 and 62](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm_test.go#L49-L62).

### Memory Subsystem Operations

- `Load`: Reads from main memory address into CPU register (`registers[register] = memory[address]`).
- `Store`: Writes value from CPU register into main memory address (`memory[address] = registers[register]`).

### Control Flow Operations

- `Jump`: Overwrites Program Counter directly with an absolute target address.
- `Beqz`: Evaluates whether the condition register equals zero; if true, shifts Program Counter by a relative offset.
- `Halt`: Breaks out of execution loop, returning control to host environment.

# Control Flow and Program Counter Progression

## Absolute vs Relative Control Transfer

### Absolute Jumps

- Target address is hardcoded in the machine instruction.
- Pros: Simple decoding, direct jump to any memory location.
- Cons: Cannot relocate code in memory without rebinding addresses.

### Relative Branches

- Target address is computed as: $\text{Target} = \text{Current PC} + \text{Offset}$.
- Pros: Relocatable code blocks, smaller operand footprint for nearby jumps.
- Cons: Range-limited by offset bit width.

## Program Counter Advancement Coupling in Introduction Prework

### Implementation Mechanics

- In [vm.go lines 89-93](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L89-L93):
  ```go
  if opcode != Jump {
      registers[pc] += maxOffset + 1
  }
  ```
- The loop unconditionally increments `registers[pc]` by instruction width (`maxOffset + 1`) for every instruction except `Jump`.

### The Branch Offset Interaction

- In `beqzProcedure`:
  ```go
  if registers[r1] == 0x00 {
      registers[pc] += offset
  }
  ```
- Because the main loop still adds `maxOffset + 1` afterwards, the total advance when a branch is taken is:
  $$\Delta \text{PC} = \text{offset} + (\text{maxOffset} + 1) = \text{offset} + 3$$
- Example from [vm_test.go line 84](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm_test.go#L84):
  - `beqz r2 3` sits at PC 14 (length 3 bytes: indices 14, 15, 16).
  - The next instruction `store r1 0` sits at PC 17 (length 3 bytes: indices 17, 18, 19).
  - The target `halt` sits at PC 20.
  - When `r2 == 0`, `beqzProcedure` adds 3 (PC becomes 17). Then the loop adds `maxOffset + 1 = 3` (PC becomes 20).
  - The offset in assembly represents the distance to skip *beyond* the natural increment of the instruction.

### Hardware Comparison

- In real hardware pipelines, the PC is incremented immediately after the fetch stage (or continuously per byte fetched).
- Hardware branch instructions calculate their target as:
  $$\text{Target} = \text{PC}_{\text{next}} + \text{Offset}$$
- The implementation in `vm.go` introduces temporal coupling between subroutine execution and loop tail logic.

# Software Architecture and Concept Design Critique

## Concept Design Analysis (Daniel Jackson)

- Daniel Jackson's Concept Design requires separating state, invariants, and actions into coherent, independent concepts.

### Conflation of Architectural Roles

- In `vm.go`, `registers` is declared as `[3]byte{8, 0, 0}` where index 0 is `pc` and indices 1 and 2 are general registers.
- Problem: The Program Counter has strict sequencing and control invariants, while `r1` and `r2` are free-form data accumulators.
- Blurring them into a uniform array allows data operations to accidentally clobber control registers if bounds or indices mismatch.

### Concept Separation Recommendation

- Separate `CPU` state into:
  - `PC`: Instruction pointer with validated stepping and branching actions.
  - `Registers`: General-purpose register bank indexable by instruction operand fields.
  - `Memory`: Byte-addressable storage enforcing boundary checks between data and code.

## Deep Module Principles (John Ousterhout)

- John Ousterhout defines a deep module as one that provides powerful functionality behind a simple, clean interface. A shallow module has an interface that is complex relative to the small amount of work it performs.

### Analysis of `decodeOperands`

- Current signature:
  ```go
  func decodeOperands(memory []byte, registers [3]byte, maxOffset byte) (operands uint16)
  ```
- Shallow module symptoms:
  - Caller must determine `maxOffset` beforehand through a separate switch statement.
  - Returns a raw packed `uint16`, forcing caller functions (`loadProcedure`, `addProcedure`) to repeat identical bitwise unpacking logic (`byte(0x00 | operands)` and `operands >>= 8`).
  - High interface complexity with low information hiding.

### Deep Module Alternative

- Define an explicit `Instruction` struct:
  ```go
  type Instruction struct {
      Opcode   byte
      DestReg  byte
      SrcReg   byte
      Address  byte
      Imm      byte
      Length   byte
  }
  ```
- A single `Decode(memory []byte, pc byte) (Instruction, error)` function encapsulates all byte-reading, offset tracking, and field extraction behind one unified call.

## State Invariant Management (Jimmy Koppel)

- Jimmy Koppel emphasizes eliminating redundant state representations and preventing distributed mutations of core invariants.

### Dual Authority over Program Counter

- In `vm.go`, Program Counter progression is governed by two separate entities:
  - Individual procedure functions (`jumpProcedure`, `beqzProcedure`).
  - The outer loop tail (`registers[pc] += maxOffset + 1`).
- This distributed mutation pattern makes control flow non-local and increases cognitive overhead during debugging.

### Invariant Preservation Strategy

- Make the execution of an instruction a pure transition function returning the next state or an explicit control directive:
  ```go
  type FlowDirective interface{}
  type Next struct{ Step byte }
  type JumpTo struct{ Address byte }
  type HaltExecution struct{}
  ```
- Centralizing PC updates into one dedicated handler ensures that off-by-one errors and branch displacement errors are localized and provable.

# Deliberate Practice and Active Recall

## Conceptual Recall Questions

- Why does the VM reserve addresses `0x00` to `0x07` instead of starting code at `0x00`?
- How does the decode phase in a CISC variable-length processor differ from a fixed-length RISC processor?
- What physical hardware components correspond to `memory[registers[pc]]`?
- If `r1 = 0` and the CPU executes `subi r1 1`, what value resides in `r1`, and why?
- In `introduction-prework/vm.go`, why does `Jump` suppress the loop's `registers[pc] += maxOffset + 1` increment, while `Beqz` does not?
- How does storing `operands` as a big-endian packed `uint16` affect the order of unpacking via `operands >>= 8`?

## Architectural Edge Cases and Failure Modes

### Program Counter Wraparound

- Memory is 256 bytes, and `pc` is a Go `byte` (`uint8`).
- If execution reaches address `255` without a `Halt`, incrementing `pc` wraps to `0`, causing the CPU to begin executing data bytes at `0x00` as machine instructions.

### Self-Modifying Code

- Because memory is unified, a `Store` instruction can write to addresses $\ge 8$.
- An instruction can overwrite future instructions in the code segment, altering program behavior on the fly.
- In modern operating systems, memory pages are protected with permission flags: `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`. The W^X (Write XOR Execute) security invariant forbids pages from being both writable and executable simultaneously to prevent arbitrary code execution attacks.

### Misaligned or Truncated Instruction Sequences

- If an instruction at address 255 expects 2 operand bytes (e.g., `Load`), reading `memory[pc+1]` and `memory[pc+2]` causes a panic due to slice index out of range.

## Connections to Linux Systems and Compilers

### ELF Binary Segments

- The split between `0x00-0x07` (data) and `0x08-0xff` (instructions) mirrors ELF executable segments:
  - `.data` and `.bss`: Global writable state (equivalent to `0x00`-`0x07`).
  - `.text`: Read-only machine instructions (equivalent to `0x08`-`0xff`).

### Context Switching and Register Spilling

- When the Linux kernel context-switches a process, it serializes architectural registers (`rip`, `rsp`, general registers) into memory (task struct / kernel stack).
- When a compiler runs out of physical registers for variables, it generates `Store` and `Load` instructions to move values to the stack frame (known as register spilling).
