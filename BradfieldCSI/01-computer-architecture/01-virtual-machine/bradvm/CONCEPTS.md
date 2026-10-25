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

# Program Counter (PC) Mechanics and Invariants

## Definition and Physical Hardware Role
- The Program Counter (`PC`) is a dedicated processor register that holds the memory address of the next machine instruction to be fetched and executed.
- Across major architectures:
  - ARM and RISC-V: Designated as `PC`.
  - x86 (16-bit): Known as `IP` (Instruction Pointer).
  - x86-32: Known as `EIP` (Extended Instruction Pointer).
  - x86-64: Known as `RIP` (Register Instruction Pointer).

## The Fetch-Increment Invariant
- During sequential program execution, the PC progresses monotonically:
  $$\text{PC}_{\text{next}} = \text{PC}_{\text{current}} + \text{Instruction Length}$$
- In physical silicon, the CPU hardware increments the PC immediately during or directly following the instruction fetch phase.
- Instruction Boundary Alignment:
  - Machine instructions consist of an opcode byte followed by optional operand bytes.
  - If the PC is incremented incorrectly by an off-by-one offset, the CPU attempts to interpret operand data (register numbers, addresses, or constants) as opcodes, causing instruction desynchronization and illegal opcode faults.

## Control Flow Diversion
- Control flow instructions intentionally break the default sequential progression:
  - Absolute Jump (`Jump`): Overwrites the Program Counter directly with a target address ($\text{PC} \leftarrow \text{Target}$).
  - Relative Branch (`Beqz`, `Beq`, `Bne`): Adds a displacement to the already-advanced Program Counter ($\text{PC} \leftarrow \text{PC} + \text{Offset}$).
  - Subroutine Call (`Call`): Pushes the sequential continuation address ($\text{PC} + \text{Length}$) onto the call stack before redirecting $\text{PC} \leftarrow \text{Target}$.
  - Subroutine Return (`Ret`): Pops the saved return address from the call stack directly into the PC.

## Single Authority Principle in Software Simulation
- In software emulators, updating `pc` ad-hoc inside multiple individual `switch` cases creates distributed authority.
- When every instruction handles its own PC stepping, relative branch calculations and jump targets become temporally coupled to loop tail logic.
- Best practice: Instruction execution returns an explicit flow directive (`FlowNext`, `FlowJump`, `FlowHalt`), leaving the clock loop as the sole authority that applies mutations to `cpu.PC`.


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

# The Sequential (SEQ) Processor Model (CS:APP Chapter 4)

## The Six Canonical Execution Stages
- In CS:APP 4.3, sequential processor hardware processes instructions through six discrete stages in a single clock cycle:
  - Fetch (`F`): Reads instruction bytes from memory at address `PC`, extracts opcode and operand bytes, and computes sequential successor address `valP` ($\text{PC} + \text{Length}$).
  - Decode (`D`): Reads up to two operand values (`valA`, `valB`) from the register file.
  - Execute (`E`): The Arithmetic Logic Unit (ALU) performs mathematical operations, effective address calculations, or evaluates condition flags to produce branch decision `Cnd`.
  - Memory (`M`): Reads a byte from data memory (`valM`) or writes a byte to data memory.
  - Write-Back (`W`): Writes up to two results (`valE` from ALU, `valM` from memory) back to the register file.
  - PC Update (`P`): Sets the Program Counter to the next instruction address (`valP`, jump target, or branch target).

## Mapping CS:APP SEQ Stages to `bradvm`
- Fetch and Decode Stages:
  - Encapsulated by [decode](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/constructs.go#L28)`(memory, cpu.PC) (Instruction, error)`.
  - Slices opcode and operand bytes from RAM, computes `inst.Length`, and validates memory boundaries.
- Execute, Memory, and Write-Back Stages:
  - Encapsulated by [execute](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go#L7)`(cpu, memory, inst) (FlowDirective, error)`.
  - Performs register math, modifies RAM bytes on `Store`, mutates `cpu.Registers` on write-back, and generates flow directives.
- PC Update Stage:
  - Encapsulated by the clock oscillator loop in [Run](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go#L49).
  - Evaluates [FlowDirective](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/constructs.go#L114) and commits the single authoritative mutation to `cpu.PC`.

# Assembler Architecture and Symbol Resolution (CS:APP 3.2, 7.5)

## The Assembler Translation Model
- Cardinality:
  - An assembler performs a 1:1 isomorphic mapping between human-readable mnemonics and hardware machine code bytes.
  - Unlike a compiler (which maps 1 high-level statement to many machine instructions), an assembler does not perform register allocation, high-level control-flow lowering, or algorithmic restructuring.
- Assembly Grammar for `bradvm`:
  - 1-byte instructions: `halt`.
  - 2-byte instructions: `jump <addr>`, `call <addr>`, `ret`.
  - 3-byte instructions: `load <reg>, <addr>`, `store <reg>, <addr>`, `add <dest>, <src>`, `sub <dest>, <src>`, `addi <dest>, <imm>`, `subi <dest>, <imm>`, `beqz <reg>, <offset>`.

## Two-Pass Assembler Design
- The Forward Reference Problem:
  - In assembly code with backward loops and forward branches, branch targets frequently refer to labels that have not yet been encountered in the text.
- Pass 1: Symbol Definition Table Construction:
  - Scans assembly text line by line while maintaining an instruction pointer offset (beginning at reset vector `0x08`).
  - When encountering a label definition (e.g. `loop:` or `done:`), records the label name and current byte offset into a symbol table (`map[string]int`).
  - Does not emit machine code bytes; calculates instruction lengths based on mnemonics to track address progression.
- Pass 2: Machine Code Generation and Target Resolution:
  - Re-scans the assembly text and emits binary machine bytes.
  - Replaces symbolic references with concrete numerical values:
    - Absolute jumps (`jump loop`): looks up `loop` in the symbol table and emits absolute target address byte.
    - Relative branches (`beqz r1, done`): calculates displacement $\text{offset} = \text{target} - (\text{PC} + \text{Length})$ and emits signed offset byte.

# Processor Status Codes and Exceptional Control Flow (CS:APP 4.1.4, 8.1)

## Processor Status Codes (`Stat`)
- In CS:APP 4.1.4, physical processors track operational integrity through a status code register (`Stat`):
  - `AOK` (`0x00`): Normal operating status; clock oscillator continues running.
  - `HLT` (`0x01`): `Halt` instruction executed; processor clock terminates cleanly.
  - `INS` (`0x02`): Illegal or undefined instruction opcode encountered; triggers hardware exception.
  - `ADR` (`0x04`): Address violation; instruction fetch or data memory access outside valid RAM boundaries.
  - `REG` (`0x03`): Invalid register identifier referenced in instruction operand bytes.

## Hardware Traps vs Host Runtime Panics
- Real silicon cannot invoke host language panics (`panic()` in Go or aborting the simulator).
- Exceptional conditions latch fault status codes into processor registers and halt execution or transfer execution to a pre-configured hardware exception vector.
- Distinguishes between normal termination (`HLT`) and abnormal termination (`INS`, `ADR`).

## Operating System Exception Classes (CS:APP 8.1)
- Interrupts:
  - Asynchronous events triggered by external hardware pins (e.g. timer tick, network packet arrival).
  - Always returns control to next instruction.
- Traps:
  - Synchronous intentional exceptions invoked by software instructions (e.g. `syscall`, software breakpoints).
  - Returns control to next instruction after kernel service execution.
- Faults:
  - Synchronous unintentional error conditions that may be recoverable (e.g. page fault, floating point divide-by-zero).
  - Re-executes the faulting instruction if kernel resolves condition, or aborts process.
- Aborts:
  - Synchronous unrecoverable hardware failures (e.g. parity error, machine check exception).
  - Terminates the application immediately.

# ALU Condition Codes and Hardware Predication (CS:APP 3.6.1, 4.3.2)

## Condition Code Bitmasks (`FLAGS`)
- In CS:APP 3.6.1, the CPU maintains a set of 1-bit status flags updated automatically by arithmetic logic units:
  - Zero Flag ($ZF$, bit 0, mask `0x01`): Set if computation result equals 0 ($\text{result} == 0$).
  - Sign Flag ($SF$, bit 1, mask `0x02`): Set if most significant bit (bit 7) is 1 ($\text{result} < 0$ in two's complement).
  - Carry Flag ($CF$, bit 2, mask `0x04`): Set if unsigned addition overflows past 255 or unsigned subtraction requires a borrow below 0.
  - Overflow Flag ($OF$, bit 3, mask `0x08`): Set if signed two's complement arithmetic produces an erroneous sign bit.

## Mathematical Formulas for 8-Bit Overflow
- Unsigned Carry Detection:
  - Addition: $\text{Carry} = (\text{uint16}(a) + \text{uint16}(b)) > 255$.
  - Subtraction: $\text{Borrow} = a < b$.
- Signed Two's Complement Overflow Detection:
  - Adding two positive numbers yields a negative result: $(a_{7} == 0 \land b_{7} == 0 \land \text{res}_{7} == 1)$.
  - Adding two negative numbers yields a positive result: $(a_{7} == 1 \land b_{7} == 1 \land \text{res}_{7} == 0)$.
  - Compact bitwise formula:
    $$\text{Overflow} = ((a \oplus \text{res}) \land (b \oplus \text{res}) \land 0\text{x}80) \neq 0$$

## Decoupled Predication: Compare vs Branch
- In early virtual machines, branching logic bundles condition testing directly with jump execution (e.g. `Beqz r1, offset`).
- Advanced ISAs decouple condition generation from condition testing:
  - Comparison instruction (`Cmp r1, r2`): Computes $r1 - r2$, updates $ZF, SF, CF, OF$, and disables register writeback.
  - Predicated branches (`Beq`, `Bne`, `Blt`, `Bge`): Inspect condition code bitmasks directly without reading general data registers:
    - Equal / Zero (`Beq`): Tests $ZF == 1$.
    - Not Equal / Non-Zero (`Bne`): Tests $ZF == 0$.
    - Less Than (signed): Tests $(SF \oplus OF) == 1$.
    - Less Than or Equal (unsigned): Tests $(CF == 1 \lor ZF == 1)$.

# The Runtime Call Stack and Subroutine Linkage (CS:APP 3.7, 4.3.5)

## Subroutine Linkage Mechanics
- Subroutines require dynamic call-return linkage so that shared utility procedures can return execution back to variable call sites across a program.
- Call Stack Register (`SP`):
  - Dedicated hardware register holding the linear address of the current stack top.
  - Initialized to `0xff` (top of memory) and grows downwards toward `0x00`.
- Calling Procedure (`Call target`):
  - Computes sequential continuation address: $\text{retAddr} = \text{PC} + \text{Length}$.
  - Pushes return address to stack: $\text{memory}[\text{SP}] \leftarrow \text{retAddr}$.
  - Decrements stack pointer: $\text{SP} \leftarrow \text{SP} - 1$.
  - Diverts control: $\text{PC} \leftarrow \text{target}$.
- Returning from Procedure (`Ret`):
  - Increments stack pointer: $\text{SP} \leftarrow \text{SP} + 1$.
  - Pops return address: $\text{PC} \leftarrow \text{memory}[\text{SP}]$.

## Calling Conventions and Activation Records
- Register Preservation Conventions (CS:APP 3.7.3):
  - Caller-Saved Registers: The caller must store these registers in memory prior to `Call` if their values are needed after return.
  - Callee-Saved Registers: The subroutine must preserve these registers on the stack and restore them before `Ret`.
- Stack Frame Invariants:
  - Stack Underflow: Occurs when popping from an empty stack ($\text{SP} > 0\text{xff}$).
  - Stack Overflow: Occurs when pushing data into reserved code or data segments ($\text{SP} < 0\text{x}08$).

# Register-Indirect Addressing and Pointer Mechanics (CS:APP 3.8)

## Hardware Memory Bus Multiplexing
- Direct Memory Addressing (`Load`, `Store`):
  - Memory address bus multiplexer selects target address directly from the instruction byte stream register (`inst.Addr`).
  - Address is static and hardwired at assembly time.
- Register-Indirect Addressing (`LoadInd`, `StoreInd`):
  - Memory address bus multiplexer selects target address from the register file data output (`cpu.Registers[rAddr]`).
  - Address is dynamic and computed at runtime.

## Lowering High-Level Pointer and Array Operations
- C Pointer Dereferencing:
  - `*ptr = val` lowers directly to `StoreInd val, ptr`.
  - `val = *ptr` lowers directly to `LoadInd val, ptr`.
- Array Traversal:
  - Combining register-indirect addressing with immediate addition (`Addi ptr, elementSize`) provides hardware support for pointer arithmetic, buffer scans, and dynamically allocated collections.

# Pipelining Principles and Hardware Hazards (CS:APP 4.5)

## Computational Throughput and Latency
- Sequential Execution (Non-Pipelined):
  - Clock period must accommodate the total propagation delay of all six execution stages combined.
  - Throughput: 1 instruction every $N$ nanoseconds.
- Pipelined Execution:
  - Divides instruction execution across pipeline stages separated by clocked pipeline registers.
  - Multiple instructions overlap across distinct stages concurrently.
  - Throughput: Approaches 1 instruction completed per clock cycle, despite individual instruction latency remaining constant or slightly increasing.

## The Three Hardware Hazards
- Data Hazards (Read-After-Write / RAW):
  - Occurs when an instruction in the Decode stage requires an operand register that a preceding instruction in the Execute or Memory stage has not yet committed to the register file.
  - Solutions:
    - Pipeline Stalls (Bubbles): Pausing dependent pipeline stages until write-back completes.
    - Data Forwarding (Bypassing): Routing intermediate ALU outputs directly across hardware bypass paths to the inputs of dependent ALU stages without waiting for register write-back.
- Control Hazards:
  - Occurs when the Fetch stage must fetch the next instruction before a conditional branch in the Execute stage determines whether the branch is taken.
  - Solutions:
    - Speculative execution with branch prediction.
    - Pipeline flush upon detecting mispredicted branch.
- Structural Hazards:
  - Occurs when multiple pipeline stages contend for the same physical hardware component simultaneously (e.g. instruction fetch and data memory access contending for a unified Von Neumann memory bus).

