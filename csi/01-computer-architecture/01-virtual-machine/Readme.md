# Virtual Machine and CPU Emulator

## Mental Models and Theoretical Foundations

### The Von Neumann Architecture
- Unified memory holds both executable program instructions and application data in the same physical address space.
- The meaning of a byte in memory is contextual: when pointed to by the Program Counter (`PC`), the byte is latched and decoded as an instruction opcode; when addressed by a `Load` or `Store` operand, it is manipulated as numeric data.

### The Fetch-Decode-Execute Cycle
- **Fetch**: The CPU asserts the Program Counter value on the memory address bus and latches the instruction byte from RAM into the Instruction Register.
- **Decode**: The Control Unit identifies the instruction opcode, calculates variable instruction byte stride (1, 2, or 3 bytes), and reads necessary operands.
- **Execute**: The Arithmetic Logic Unit (ALU) performs computation or the Control Unit diverts the Program Counter.

### Software Architecture Invariants
- **Daniel Jackson's Concept Design**: Separate state representations into distinct orthogonal concepts: `CPU` (registers, flags, PC), `Instruction` (decoded syntax), and `Memory` (raw linear byte storage).
- **John Ousterhout's Deep Modules**: The instruction decoder `decode(memory, pc)` provides a deep interface, hiding byte slicing and length calculations behind a simple `Instruction` struct.
- **Jimmy Koppel's Single Authority Principle**: Avoid mutating the Program Counter in distributed execution switch cases. Let `execute()` return an explicit `FlowDirective` (`FlowNext`, `FlowJump`, `FlowHalt`), making the clock loop the sole authority updating `cpu.PC`.

## Textbook Reference
- **CS:APP3e**: Chapter 4 (*Processor Architecture*), Sections 4.1–4.3 (*Y86-64 Instruction Set Architecture and Sequential Execution*).

## Hardware Specifications
- **Memory**: 256 bytes (`[256]byte`), byte-addressable from `0x00` to `0xff`.
  - `0x00`: Reserved for program return value / output.
  - `0x01`: Input parameter X.
  - `0x02`: Input parameter Y.
  - `0x03` to `0x07`: Scratchpad storage.
  - `0x08` to `0xff`: Instruction segment (code entry point begins at `0x08`).
- **Registers**:
  - `PC` (Program Counter): Starts at `0x08`.
  - `R1`: 8-bit accumulator register.
  - `R2`: 8-bit accumulator register.
  - `FLAGS`: Status register (Zero, Sign, Carry, Overflow).

## Instruction Set Architecture (ISA)
- `Halt` (`0xff`, 1 byte): Halts execution.
- `Load` (`0x01 <reg> <addr>`, 3 bytes): `registers[reg] = memory[addr]`.
- `Store` (`0x02 <reg> <addr>`, 3 bytes): `memory[addr] = registers[reg]`.
- `Add` (`0x03 <dest> <src>`, 3 bytes): `registers[dest] += registers[src]`.
- `Sub` (`0x04 <dest> <src>`, 3 bytes): `registers[dest] -= registers[src]`.
- `Addi` (`0x05 <dest> <imm>`, 3 bytes): `registers[dest] += imm`.
- `Subi` (`0x06 <dest> <imm>`, 3 bytes): `registers[dest] -= imm`.
- `Jump` (`0x07 <target_addr>`, 2 bytes): `PC = target_addr`.
- `Beqz` (`0x08 <reg> <offset>`, 3 bytes): If `registers[reg] == 0`, `PC = (PC + 3) + offset`.

## Step-by-Step Implementation Guide
- **Phase 1: Basic Oscillator and Halt**
  - Implement a `for` loop in `compute(memory []byte)` starting `pc = 8`.
  - Check for `opcode == Halt` (`0xff`) and exit cleanly.
- **Phase 2: Memory Access and Registers**
  - Implement `Load` (`0x01`) and `Store` (`0x02`).
  - Advance `pc += 3` after non-branching instructions.
- **Phase 3: ALU Arithmetic**
  - Implement `Add` (`0x03`) and `Sub` (`0x04`). Verify 8-bit unsigned modulo 256 wrapping.
- **Phase 4: Immediate Addressing**
  - Implement `Addi` (`0x05`) and `Subi` (`0x06`).
- **Phase 5: Control Flow**
  - Implement `Jump` (`0x07`) and `Beqz` (`0x08`).
- **Phase 6: Structural Refactoring**
  - Decompose into `CPU`, `Instruction`, and `FlowDirective` types as demonstrated in the [bradvm/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/01-virtual-machine/bradvm) subproject.

## Verification
- Run tests in `bradvm/`:
  ```bash
  cd bradvm && go test -v ./...
  ```
- Run tests in `introduction-prework/`:
  ```bash
  cd introduction-prework && go test -v ./...
  ```

## Active Recall Checkpoint
- Why does `uint8(255) + uint8(1)` equal `0` in binary hardware without throwing an exception?
- How does the Program Counter maintain boundary alignment, and what happens if an off-by-one error occurs during instruction decode?
- What is the difference between relative branching (`Beqz`) and absolute jumping (`Jump`) in position-independent code?
