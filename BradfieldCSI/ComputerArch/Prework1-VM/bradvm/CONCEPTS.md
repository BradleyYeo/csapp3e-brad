# Project Mission: What We Are Building in `bradvm`

## Architectural Objective
- You are building an emulator for an 8-bit central processing unit (CPU) based on the classic Von Neumann architecture from first principles in Go.
- The emulator simulates how physical electronic computers fetch instructions from memory, decode bit patterns into control signals, execute mathematical and data transfer operations, and advance the Program Counter.

## Hardware Specifications of `bradvm`
- Unified Memory (RAM): 256 bytes (`[256]byte`), byte-addressable from `0x00` to `0xff`.
- Address Space Partitioning:
  - `0x00`: Reserved for program output / return value.
  - `0x01`: Input parameter X.
  - `0x02`: Input parameter Y.
  - `0x03` to `0x07`: Reserved data scratchpad.
  - `0x08` to `0xff`: Instruction segment (code space).
- Register File:
  - `PC` (Program Counter): Points to the memory address of the current instruction (initializes to `0x08`).
  - `R1`: 8-bit general-purpose data accumulator.
  - `R2`: 8-bit general-purpose data accumulator.
  - `FLAGS`: Status register tracking ALU arithmetic condition codes ($Z$, $S$, $C$, $V$).
  - `SP` (Stack Pointer): Points to the active top of the call stack (initializes to `0xff`, growing downwards).
- Clock Oscillator:
  - Simulated via a synchronous loop driving discrete state transitions cycle by cycle.

## Project Progression Phases
- Prototyping Phase:
  - Writing an initial monolithic interpreter loop in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go) to verify raw instruction opcodes against unit tests.
- Architectural Refactoring Phase:
  - Decomposing the monolithic switch into decoupled concepts: an explicit `CPU` struct, a deep `Instruction` decoder, and a single-authority `FlowDirective` control loop.
- Advanced Systems Subsystems Phase:
  - Extending hardware capabilities with condition flags, a memory-backed call stack for subroutines, hardware exception traps, register-indirect pointers, and a disassembler.

# Von Neumann Architecture and Hardware Fundamentals

## The Stored-Program Concept
- Instructions and data reside together in the same physical memory space.
- A byte in RAM has no intrinsic meaning; its interpretation depends entirely on context:
  - If referenced by the Program Counter (`PC`), the byte is fetched and decoded as machine code.
  - If referenced by a `Load` or `Store` address operand, the byte is treated as application data.

## Von Neumann vs Harvard Architecture
- Von Neumann Architecture:
  - Employs a single unified bus for both instruction fetch and data read/write.
  - Advantages: Flexible memory allocation between code and data.
  - Tradeoffs: Von Neumann bottleneck, where instruction fetch and data access contend for the same memory bus.
- Harvard Architecture:
  - Employs physically separate memory chips and separate buses for instructions and data.
  - Can fetch an instruction and read/write data in the exact same clock cycle.
- Modern Processor Design:
  - Modern general-purpose processors (x86-64, ARM64) use a Modified Harvard Architecture: unified system RAM at the bus level, but split L1 Instruction Cache (I-cache) and L1 Data Cache (D-cache) inside the CPU core.

## The Memory and Storage Hierarchy
- Registers:
  - Located directly on the CPU die with sub-nanosecond access latency.
  - The ALU can only operate directly on data stored in registers.
- Main Memory (RAM):
  - Located across the memory bus with higher latency.
  - Requires explicit bus transactions (`Load` and `Store`) to transfer data to and from CPU registers.

# Instruction Set Architecture and Addressing Modes

## Instruction Set Architecture (ISA) Definition
- The ISA defines the boundary contract between hardware execution units and software programs.
- It specifies the instruction vocabulary, data word sizes, register definitions, addressing modes, and binary machine code formats.

## Variable-Length Instruction Formats
- 1-Byte Instructions:
  - `Halt` (`0xff`): Stop execution.
  - `Ret` (`0x0d`): Return from subroutine.
- 2-Byte Instructions:
  - `Jump` (`0x07 <target_addr>`): Unconditional absolute jump.
  - `Beq` (`0x09 <offset>`): Branch if Zero flag set.
  - `Bne` (`0x0a <offset>`): Branch if Zero flag clear.
  - `Call` (`0x0c <target_addr>`): Subroutine call.
- 3-Byte Instructions:
  - `Load` (`0x01 <reg> <addr>`): Direct memory load into register.
  - `Store` (`0x02 <reg> <addr>`): Store register into direct memory address.
  - `Add` (`0x03 <dest> <src>`): Register-to-register addition.
  - `Sub` (`0x04 <dest> <src>`): Register-to-register subtraction.
  - `Addi` (`0x05 <dest> <imm>`): Add immediate constant to register.
  - `Subi` (`0x06 <dest> <imm>`): Subtract immediate constant from register.
  - `Beqz` (`0x08 <reg> <offset>`): Branch if register equals zero.
  - `Cmp` (`0x0b <r1> <r2>`): Compare registers, updating flags without writeback.
  - `LoadInd` (`0x0e <dest> <addr_reg>`): Load value at address pointed to by register.
  - `StoreInd` (`0x0f <src> <addr_reg>`): Store value into address pointed to by register.

## The Five Addressing Modes
- Register Addressing:
  - Operands reside directly in CPU registers (`add r1, r2`).
  - No memory bus access is generated.
- Immediate Addressing:
  - The constant operand is embedded directly inside the instruction byte stream (`addi r1, 3`).
  - Represented by the `Imm` field in decoded instructions.
- Direct Memory Addressing:
  - The exact linear memory address is hardcoded into the instruction (`load r1, 1`).
  - Represented by the `Addr` field in decoded instructions.
- Relative Addressing:
  - The target address is computed as a displacement from the updated Program Counter (`pc + offset`).
  - Enables position-independent code (PIC) that can run regardless of where it is loaded in RAM.
- Register-Indirect Addressing:
  - A general-purpose register holds the memory address to access (`loadind r1, r2`).
  - Essential for dynamic data structures, array traversal, and pointer dereferencing.

# The Hardware Clock and Fetch-Decode-Execute Cycle

## The Physical Machine Cycle
- Synchronized by an electronic oscillator.
- Each clock cycle drives one discrete state transition of the digital logic circuits.

## Fetch Phase Mechanics
- The CPU drives the value of the Program Counter (`PC`) onto the address bus.
- The memory controller returns the byte stored at that address across the data bus.
- The CPU latches this byte into its internal Instruction Register (`IR`).

## Decode Phase Mechanics
- The control unit evaluates the opcode bit pattern to determine:
  - Instruction byte length (1, 2, or 3 bytes).
  - Which internal data paths to open.
  - How many additional operand bytes to read from memory.
- In software, encapsulated into the `decode(memory, pc)` function which produces an `Instruction` struct.

## Execute Phase Mechanics
- Arithmetic Logic Unit (ALU):
  - Performs computation (`Add`, `Sub`, `Addi`, `Subi`).
  - Operates using 8-bit unsigned and two's complement integer arithmetic.
  - Integer overflow wraps modulo 256 (`255 + 1 = 0`, `0 - 1 = 255`).
  - Updates condition flags ($Z$, $S$, $C$, $V$).
- Memory Access:
  - Asserts address and data signals to transfer values between registers and RAM (`Load`, `Store`).
- Control Flow:
  - Evaluates branch conditions and updates the Program Counter.

## Single Authority Program Counter Progression
- In physical hardware, the PC advances past the current instruction during fetch and decode.
- When a branch occurs, the target address overrides the sequential increment.
- Maintaining a single authority in the clock loop prevents distributed mutations and eliminates subtle off-by-one errors.

# Advanced Hardware Subsystems

## Processor Status Register (FLAGS)
- Decouples computation evaluation from branching decisions.
- The ALU automatically calculates condition flags on every arithmetic operation:
  - Zero Flag ($Z$): Set to 1 if result equals 0.
  - Sign Flag ($S$): Set to 1 if the most significant bit (bit 7) is 1.
  - Carry Flag ($C$): Set to 1 if unsigned addition overflows past 255 or unsigned subtraction borrows below 0.
  - Overflow Flag ($V$): Set to 1 if signed two's complement addition/subtraction produces an invalid sign.
- Conditional branches (`Beq`, `Bne`) evaluate condition codes against `FLAGS` rather than evaluating general registers directly.

## The Call Stack and Subroutines
- Enables reusable functions and modular code execution.
- Hardware mechanics:
  - Stack Pointer (`SP`): Initialized to `0xff` (top of memory) and grows downwards towards `0x00`.
  - `Call <target>`: Pushes the return address (`PC + 2`) to `memory[SP]`, decrements `SP--`, and sets `PC = target`.
  - `Ret`: Increments `SP++`, and pops the return address from `memory[SP]` into `PC`.
- Preserves the caller's execution context across function invocations.

## CPU Traps and Exception Handling
- Physical processors do not crash when encountering invalid operations.
- Hardware traps cleanly halt execution or divert control to an exception vector:
  - Illegal opcode encountered.
  - Memory access outside valid bounds.
  - Invalid register referenced.
- Distinguishes between fatal machine faults and normal termination (`Halt`).

## Memory-Mapped I/O (MMIO)
- Peripherals (keyboards, serial consoles, displays) are mapped into reserved memory addresses.
- Writing to a designated memory location (e.g. `0x07` as `UART_TX`) does not store data in RAM; it transmits the byte to an external device.

# Software Architecture Principles for Hardware Emulation

## Deep Modules (John Ousterhout)
- A deep module provides rich functionality behind a clean, simple interface.
- A shallow module has an interface that is complex relative to the minimal work it performs.
- In `bradvm`:
  - Shallow approach: Inlining `memory[pc+1]` and `memory[pc+2]` slicing inside every switch case, forcing each case to manage byte strides and bounds checks.
  - Deep approach: A single `decode(memory []byte, pc int) (Instruction, error)` function that hides byte extraction, length calculation, and boundary validation behind a simple `Instruction` struct.

## Concept Design (Daniel Jackson)
- Concept design requires separating distinct architectural roles into independent, coherent concepts.
- Conflating registers, program counter, and memory into raw arrays creates accidental coupling and potential state corruption.
- In `bradvm`:
  - `CPU`: Owns physical processor registers (`PC`, `Registers`, `Flags`, `SP`) and their state transitions.
  - `Instruction`: Owns the decoded representation of machine code.
  - `Memory`: Owns the linear byte array and enforces access boundaries.

## Single Authority Principle (Jimmy Koppel)
- Core state invariants must be mutated by exactly one authoritative component.
- Mutating `pc` directly inside multiple switch cases causes distributed authority and temporal coupling.
- In `bradvm`:
  - Instructions return an explicit `FlowDirective` (`FlowNext`, `FlowJump`, `FlowHalt`).
  - The central clock loop acts as the sole authority updating `cpu.PC`.

# Operating System and Linux Systems Connections

## Executable Segments (ELF Layout)
- The memory partitioning in `bradvm` directly mirrors Linux Executable and Linkable Format (ELF) binary segments:
  - `.text`: Read-only machine instructions (`0x08` to `0xef`).
  - `.data` / `.bss`: Global read-write variables (`0x00` to `0x07`).
  - `stack`: Dynamic call frames growing downwards (`0xf0` to `0xff`).

## Memory Protection and W^X (Write XOR Execute)
- Modern operating systems enforce page permissions using the hardware Memory Management Unit (MMU):
  - `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`.
  - W^X Invariant: Memory pages must never be simultaneously writable and executable.
  - Prevents security exploits where user input written to a buffer is executed as machine code.

## Context Switching and Register Spilling
- When the Linux kernel performs a context switch, it saves the processor's register state into the process control block (task struct).
- When a compiler runs out of physical registers for variables, it generates `Store` and `Load` instructions to spill register values into stack memory.
