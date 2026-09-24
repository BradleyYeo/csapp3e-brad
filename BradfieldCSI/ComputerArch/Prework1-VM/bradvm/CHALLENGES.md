# Curriculum Overview and Methodology

## Test-Driven Hardware Emulation

- Build the virtual machine one layer at a time using Test-Driven Development (TDD).
- For each challenge:
  - Write the test in [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go).
  - Run the test using `go test -v ./...` to verify it fails for the expected reason.
  - Implement the minimal code in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go) to pass the test.
  - Review the architectural concepts and active recall checkpoints before proceeding.

## Conceptual Layering Strategy

- Begin with raw machine code bytes (`[]byte`) rather than symbolic strings to master the underlying binary representation.
- Introduce arithmetic and data transfer before control flow.
- Address architectural flaws identified in [bradvm/CONCEPTS.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/CONCEPTS.md) (shallow decoders, coupled Program Counter updates) via structured refactoring milestones.

# Challenge 1: The Oscillator and Halt

## Concept: Hardware Clock and Execution Termination

- A physical CPU requires an oscillating clock signal to drive state transitions across clock cycles.
- In software emulation, an infinite `for` loop acts as the clock generator.
- The machine begins executing instructions starting at address `0x08` (the boundary between data and code space).
- The `Halt` instruction (`0xff`) signals the CPU to stop the clock and terminate execution cleanly.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestHaltMinimal(t *testing.T) {
      memory := make([]byte, 256)
      memory[8] = 0xff // HALT opcode at instruction entry point

      compute(memory)
      // Test passes if function halts and does not loop infinitely
  }
  ```

## Architectural Mechanics

- Memory layout at cycle 0:
  - `memory[0x00]` to `memory[0x07]`: All zero.
  - `memory[0x08]`: `0xff` (`Halt`).
- State variables required:
  - Program Counter: `pc = 8`.
- Fetch phase:
  - Read `opcode := memory[pc]`.
- Execute phase:
  - Detect `opcode == 0xff` and break the loop.

## Implementation Guidelines

- Define opcode constants in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go):
  ```go
  const (
      Halt = 0xff
  )
  ```
- Implement `compute(memory []byte)` with a loop that tracks `pc` starting at 8.
- Break when `opcode == Halt`.

## Active Recall Checkpoint

- What happens in hardware when a CPU receives an unhandled or invalid opcode?
- Why must `pc` initialize to 8 instead of 0 in this specific memory architecture?

# Challenge 2: Direct Memory Access and Register Files

## Concept: The Register File, Bus Transfers, and Instruction Stepping

- The CPU cannot operate on RAM directly without first loading data into on-chip registers.
- Registers provide temporary storage that the ALU and control unit can read and write within a single clock cycle.
- `Load` reads a byte from a memory address into a register.
- `Store` writes a byte from a register into a memory address.
- Both instructions are 3 bytes long:
  - Byte 0: Opcode (`0x01` for `Load`, `0x02` for `Store`).
  - Byte 1: Register identifier (`1` for `r1`, `2` for `r2`).
  - Byte 2: Linear memory address (`0` to `255`).
- The Program Counter must advance by 3 bytes after each non-branch instruction.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestLoadStoreMinimal(t *testing.T) {
      memory := make([]byte, 256)
      memory[1] = 42 // Set input parameter in data segment

      // Program: load r1 [1]; store r1 [0]; halt
      program := []byte{
          0x01, 0x01, 0x01, // load r1, 1
          0x02, 0x01, 0x00, // store r1, 0
          0xff,             // halt
      }
      copy(memory[8:], program)

      compute(memory)

      if memory[0] != 42 {
          t.Fatalf("expected memory[0] to be 42, got %d", memory[0])
      }
  }
  ```

## Architectural Mechanics

- Execution trace:
  - Cycle 1: `pc = 8`. Fetches `0x01` (`Load`). Decodes destination register `1` (`r1`), source address `1`. Executes: `registers[1] = memory[1]` (`42`). Advances: `pc = 8 + 3 = 11`.
  - Cycle 2: `pc = 11`. Fetches `0x02` (`Store`). Decodes source register `1` (`r1`), destination address `0`. Executes: `memory[0] = registers[1]` (`42`). Advances: `pc = 11 + 3 = 14`.
  - Cycle 3: `pc = 14`. Fetches `0xff` (`Halt`). Terminates.

## Implementation Guidelines

- Define constants:
  ```go
  const (
      Load  = 0x01
      Store = 0x02
      Halt  = 0xff
  )
  ```
- Maintain register file state inside `compute`:
  ```go
  var registers [3]byte // registers[1] is r1, registers[2] is r2
  ```
- For 3-byte instructions, fetch operands at `memory[pc+1]` and `memory[pc+2]`.
- Increment `pc += 3` after execution.

## Active Recall Checkpoint

- What are the tradeoffs between fixed-length 3-byte instructions and variable-length instructions?
- In physical silicon, what physical buses are energized during a `Load` vs a `Store`?

# Challenge 3: Arithmetic Logic Unit Operations and Two's Complement

## Concept: Integer Arithmetic, Overflows, and Register-to-Register Operations

- The Arithmetic Logic Unit (ALU) performs computation on register operands.
- `Add` (`0x03`) and `Sub` (`0x04`) are 3-byte register-to-register instructions:
  - Byte 0: Opcode.
  - Byte 1: Destination register (`r1` or `r2`).
  - Byte 2: Source register (`r1` or `r2`).
- Semantics:
  - `Add r1 r2` $\rightarrow$ `r1 = r1 + r2`.
  - `Sub r1 r2` $\rightarrow$ `r1 = r1 - r2`.
- Overflow behavior: 8-bit unsigned integers wrap modulo 256 without error conditions.
  - `254 + 1 = 255`
  - `255 + 1 = 0` (overflow)
  - `0 - 1 = 255` (underflow / borrow)

## Test Specification

- Add tests covering normal arithmetic and overflow boundaries to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestAddAndSubtract(t *testing.T) {
      cases := []struct {
          name     string
          x, y     byte
          isSub    bool
          expected byte
      }{
          {"AddNormal", 10, 20, false, 30},
          {"AddOverflow", 255, 1, false, 0},
          {"SubNormal", 20, 5, true, 15},
          {"SubUnderflow", 0, 1, true, 255},
      }

      for _, tc := range cases {
          t.Run(tc.name, func(t *testing.T) {
              memory := make([]byte, 256)
              memory[1] = tc.x
              memory[2] = tc.y

              op := byte(0x03) // Add
              if tc.isSub {
                  op = 0x04 // Sub
              }

              program := []byte{
                  0x01, 0x01, 0x01, // load r1, 1
                  0x01, 0x02, 0x02, // load r2, 2
                  op, 0x01, 0x02,   // add/sub r1, r2
                  0x02, 0x01, 0x00, // store r1, 0
                  0xff,             // halt
              }
              copy(memory[8:], program)

              compute(memory)

              if memory[0] != tc.expected {
                  t.Fatalf("expected %d, got %d", tc.expected, memory[0])
              }
          })
      }
  }
  ```

## Architectural Mechanics

- The ALU does not touch memory; it receives inputs strictly from the register file.
- The result of the ALU operation is written back to the destination register during the writeback phase of the clock cycle.

## Implementation Guidelines

- Define constants:
  ```go
  const (
      Add = 0x03
      Sub = 0x04
  )
  ```
- Implement dispatch cases for `Add` and `Sub` reading register indices from `memory[pc+1]` and `memory[pc+2]`.

## Active Recall Checkpoint

- Why does `uint8(255) + uint8(1)` wrap to `0` in Go without throwing an exception?
- How does hardware detect whether an arithmetic result overflowed (hint: status/flags register)?

# Challenge 4: Immediate Addressing Modes

## Concept: Constants in the Instruction Stream

- Register-to-register arithmetic requires both operands to be loaded into registers first.
- If an algorithm needs to add or subtract a known constant (e.g., decrementing a loop counter by 1), storing that constant in data memory and loading it waste memory bus cycles.
- Immediate instructions (`Addi = 0x05`, `Subi = 0x06`) embed the literal operand directly inside Byte 2 of the instruction:
  - Byte 0: Opcode.
  - Byte 1: Destination register.
  - Byte 2: Literal integer constant.
- Semantics:
  - `Addi r1 3` $\rightarrow$ `registers[r1] = registers[r1] + 3`.
  - `Subi r1 1` $\rightarrow$ `registers[r1] = registers[r1] - 1`.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestImmediateArithmetic(t *testing.T) {
      memory := make([]byte, 256)
      memory[1] = 20 // Input X

      program := []byte{
          0x01, 0x01, 0x01, // load r1, 1 (r1 = 20)
          0x05, 0x01, 0x03, // addi r1, 3 (r1 = 23)
          0x06, 0x01, 0x05, // subi r1, 5 (r1 = 18)
          0x02, 0x01, 0x00, // store r1, 0
          0xff,             // halt
      }
      copy(memory[8:], program)

      compute(memory)

      if memory[0] != 18 {
          t.Fatalf("expected memory[0] to be 18, got %d", memory[0])
      }
  }
  ```

## Architectural Mechanics

- The second operand is sourced directly from the Instruction Register / instruction buffer, bypassing the register read port.
- This cuts memory references by half compared to `load r2 [constant_addr]` followed by `add r1 r2`.

## Implementation Guidelines

- Add constants:
  ```go
  const (
      Addi = 0x05
      Subi = 0x06
  )
  ```
- In the execution switch, treat operand 2 as a literal byte value rather than a register index.

## Active Recall Checkpoint

- What is the maximum constant value that can be represented in an immediate field of 1 byte?
- How do 64-bit architectures like x86-64 handle large 64-bit immediate values?

# Challenge 5: Control Flow Diversion and Absolute Jumps

## Concept: The Program Counter as an Invariant State Register

- Linear sequential execution (`pc += length`) is insufficient for algorithms requiring branches or loops.
- An unconditional `Jump` (`0x07`) overrides the sequential increment by setting the Program Counter directly to a target memory address.
- `Jump` is a 2-byte instruction:
  - Byte 0: Opcode (`0x07`).
  - Byte 1: Target address (`0` to `255`).
- Critical invariant: When a `Jump` executes, the default `pc += length` step must NOT run; the PC must take on the target address immediately.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestJump(t *testing.T) {
      memory := make([]byte, 256)
      memory[1] = 42

      // Program layout:
      // Address 8:  load r1, 1       (3 bytes: 8, 9, 10)
      // Address 11: jump 17          (2 bytes: 11, 12)
      // Address 13: store r1, 0      (3 bytes: 13, 14, 15) -> SHOULD BE SKIPPED
      // Address 16: halt             (1 byte: 16)          -> Landing point
      program := []byte{
          0x01, 0x01, 0x01, // 8:  load r1, 1
          0x07, 0x10,       // 11: jump 16 (0x10)
          0x02, 0x01, 0x00, // 13: store r1, 0 (skipped)
          0xff,             // 16: halt
      }
      copy(memory[8:], program)

      compute(memory)

      // Since store was jumped over, memory[0] must remain 0
      if memory[0] != 0 {
          t.Fatalf("expected memory[0] to be 0 (skipped store), got %d", memory[0])
      }
  }
  ```

## Architectural Mechanics

- Target address calculation: Direct assignment `pc = targetAddress`.
- Because `Jump` is 2 bytes while other instructions are 3 bytes (or 1 byte for `Halt`), the VM architecture must handle variable instruction lengths.

## Implementation Guidelines

- Add constant:
  ```go
  const Jump = 0x07
  ```
- Guard the natural PC increment so that when `opcode == Jump`, the PC is not incremented further.

## Active Recall Checkpoint

- What is the difference between an absolute jump and a relative jump?
- Why can absolute jumps become problematic when code must be relocated to different base addresses in memory?

# Challenge 6: Conditional Branching and Relative Addressing

## Concept: Predicated Execution and PC-Relative Offsets

- Conditional control flow allows a program to make decisions based on runtime data.
- `Beqz` (`0x08` - Branch if Equal to Zero) evaluates a register; if the register holds zero, execution jumps by a relative offset.
- `Beqz` is a 3-byte instruction:
  - Byte 0: Opcode (`0x08`).
  - Byte 1: Evaluation register (`r1` or `r2`).
  - Byte 2: Forward offset (number of bytes to skip).
- Single Authority PC Principle:
  - In [introduction-prework/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L89-L93), `beqzProcedure` adds `offset` to `pc`, and then the outer loop *also* adds `maxOffset + 1 = 3`.
  - This dual authority creates temporal coupling where the offset must be written relative to the instruction end rather than the current instruction pointer.
  - In your implementation, ensure control flow updates are transparent and clearly defined.

## Test Specification

- Add tests for both taken and not-taken branch conditions to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestBeqz(t *testing.T) {
      cases := []struct {
          name     string
          r2Value  byte
          expected byte
      }{
          {"BranchTakenWhenZero", 0, 0},     // r2 is 0 -> branch over store -> memory[0] stays 0
          {"FallthroughWhenNonZero", 1, 42}, // r2 is 1 -> execute store -> memory[0] becomes 42
      }

      for _, tc := range cases {
          t.Run(tc.name, func(t *testing.T) {
              memory := make([]byte, 256)
              memory[1] = 42
              memory[2] = tc.r2Value

              // Layout:
              // 8:  load r1, 1       (3 bytes: 8..10)
              // 11: load r2, 2       (3 bytes: 11..13)
              // 14: beqz r2, 3       (3 bytes: 14..16) -> skip 3-byte store instruction
              // 17: store r1, 0      (3 bytes: 17..19)
              // 20: halt             (1 byte: 20)
              program := []byte{
                  0x01, 0x01, 0x01, // 8:  load r1, 1
                  0x01, 0x02, 0x02, // 11: load r2, 2
                  0x08, 0x02, 0x03, // 14: beqz r2, 3
                  0x02, 0x01, 0x00, // 17: store r1, 0
                  0xff,             // 20: halt
              }
              copy(memory[8:], program)

              compute(memory)

              if memory[0] != tc.expected {
                  t.Fatalf("expected memory[0] to be %d, got %d", tc.expected, memory[0])
              }
          })
      }
  }
  ```

## Architectural Mechanics

- When condition is true (`registers[r2] == 0`):
  - Next PC = Target after the skipped block.
- When condition is false (`registers[r2] != 0`):
  - Next PC = Normal sequential progression (`pc + 3`).

## Implementation Guidelines

- Add constant:
  ```go
  const Beqz = 0x08
  ```
- Implement the conditional evaluation:
  - If register value is zero, add the offset to the sequential progression.
  - If register value is non-zero, proceed sequentially.

## Active Recall Checkpoint

- Why do compilers prefer relative branches over absolute jumps for local loops and `if/else` statements?
- What happens if an offset wraps the 8-bit Program Counter past 255?

# Challenge 7: Deep Module Refactoring and State Invariant Preservation

## Concept: Applying Ousterhout, Koppel, and Jackson to `bradvm/vm.go`

- Your initial working implementation in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go) successfully passes all tests for Challenges 1 through 6.
- However, as the instruction set expands towards subroutines, flags, and indirect addressing, the current structure exhibits three architectural design flaws:

### Architectural Flaws in Current `bradvm/vm.go`

- Entangled Decoding and Execution (John Ousterhout's Shallow Module):
  - In [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go#L20-L70), the `switch opcode` block performs fetch, operand decoding, computation, and memory access in a single inlined routine.
  - Every case directly slices `memory[pc+1]` and `memory[pc+2]`.
  - Knowledge of instruction byte lengths and operand offsets is duplicated across every case. Adding a new instruction requires calculating raw memory indices ad-hoc.
- Distributed Multi-Authority Program Counter Mutations (Jimmy Koppel's Single Authority Principle):
  - Program Counter updates are scattered across multiple cases:
    - Sequential increment `pc += 3` is duplicated across 6 distinct switch cases (`Addi`, `Subi`, `Load`, `Store`, `Add`, `Sub`).
    - `Jump` directly sets `pc = int(target_addr)`.
    - `Beqz` mutates `pc` in two consecutive steps: `pc += 3` followed by `if cc == 0 { pc += int(relative_offset) }`.
  - Because multiple blocks mutate `pc` directly, there is no single authoritative gatekeeper ensuring that the PC remains aligned to valid instruction boundaries.
- Unencapsulated Architectural State (Daniel Jackson's Concept Design):
  - State variables (`pc := 0x08`, `var registers [3]byte`) are declared as bare local variables inside `compute`.
  - Registers, memory, and control flow are conflated within a single function scope, making it impossible to inspect, snapshot, or test the CPU state independently of execution.

## Architectural Refactoring Requirements

### The CPU Concept

- Encapsulate physical processor state into a dedicated `CPU` struct:
  ```go
  type CPU struct {
      PC        int
      Registers [3]byte
  }

  func NewCPU() *CPU {
      return &CPU{
          PC: 0x08,
      }
  }
  ```

### The Instruction Model

- Create a structured representation that encapsulates decoded instruction fields:
  ```go
  type Instruction struct {
      Opcode  byte
      DestReg byte
      SrcReg  byte
      Addr    byte
      Imm     byte
      Length  int
  }
  ```

### The Deep Decoder

- Implement a decoder that extracts and validates an instruction behind a clean interface:
  ```go
  func decode(memory []byte, pc int) (Instruction, error)
  ```
- This function encapsulates:
  - Determining instruction length from opcode (`Halt`: 1 byte, `Jump`: 2 bytes, others: 3 bytes).
  - Validating that `pc + length <= len(memory)` before dereferencing operands.
  - Extracting registers, memory addresses, and immediate constants into typed fields.

### Explicit Control Flow Directives

- Eliminate distributed `pc` mutations by modeling execution outcomes as explicit control flow directives:
  ```go
  type FlowKind int

  const (
      FlowNext FlowKind = iota
      FlowJump
      FlowHalt
  )

  type FlowDirective struct {
      Kind   FlowKind
      Target int
  }
  ```
- The execution phase never mutates `cpu.PC` directly. It returns a `FlowDirective`:
  ```go
  func execute(cpu *CPU, memory []byte, inst Instruction) (FlowDirective, error)
  ```
- The central clock loop becomes the sole authority over Program Counter advancement:
  ```go
  flow, err := execute(cpu, memory, inst)
  if err != nil {
      panic(err)
  }
  switch flow.Kind {
  case FlowNext:
      cpu.PC += inst.Length
  case FlowJump:
      cpu.PC = flow.Target
  case FlowHalt:
      return
  }
  ```

## Verification Strategy

- Run `go test -v ./...` in [bradvm/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm).
- Every test from Challenges 1 through 6 (`TestHaltMinimal`, `TestLoadStoreMinimal`, `TestAddAndSubtract`, `TestImmediateArithmetic`, `TestJump`, `TestBeqz`) must pass without modification after the refactor.

## Active Recall Checkpoint

- How does centralizing Program Counter updates into a single clock loop eliminate off-by-one errors in relative branches?
- In terms of John Ousterhout's deep modules, why is `decode(memory, pc)` considered deep while inlined `memory[pc+1]` access is shallow?

# Challenge 8: Assembler Construction and Algorithmic Execution

## Concept: Symbolic Assembly Translation and Looping Constructs

- Writing machine code as raw byte slices is error-prone and obscures algorithmic structure.
- An assembler parses human-readable assembly instructions and emits valid machine code bytes.
- The `Sum to n` program tests all architectural components working in harmony:
  - Parameter loading (`Load`).
  - Loop termination condition (`Beqz`).
  - Accumulation (`Add`).
  - Loop counter decrement (`Subi`).
  - Unconditional backward loop jump (`Jump`).
  - Writeback to memory address `0x00` (`Store`).
  - Clean termination (`Halt`).

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestSumToN(t *testing.T) {
      asm := `
  load r1, 1
  beqz r1, 8
  add r2, r1
  subi r1, 1
  jump 11
  store r2, 0
  halt`

      testCases := []struct {
          n        byte
          expected byte
      }{
          {0, 0},   // sum(0) = 0
          {1, 1},   // sum(1) = 1
          {5, 15},  // sum(5) = 1 + 2 + 3 + 4 + 5 = 15
          {10, 55}, // sum(10) = 55
      }

      bytecode := assemble(asm)

      for _, tc := range testCases {
          memory := make([]byte, 256)
          memory[1] = tc.n
          copy(memory[8:], bytecode)

          compute(memory)

          if memory[0] != tc.expected {
              t.Fatalf("for n=%d, expected %d, got %d", tc.n, tc.expected, memory[0])
          }
      }
  }
  ```

## Architectural Mechanics

- Execution trace for $n = 3$:
  - `load r1, 1`: `r1 = 3`.
  - Loop iteration 1: `r1 != 0` (branch not taken). `r2 = 0 + 3 = 3`. `r1 = 3 - 1 = 2`. `jump 11`.
  - Loop iteration 2: `r1 != 0`. `r2 = 3 + 2 = 5`. `r1 = 2 - 1 = 1`. `jump 11`.
  - Loop iteration 3: `r1 != 0`. `r2 = 5 + 1 = 6`. `r1 = 1 - 1 = 0`. `jump 11`.
  - Loop iteration 4: `r1 == 0`. `beqz r1, 8` takes branch, skipping forward 8 bytes past `add`, `subi`, `jump`.
  - Land at `store r2, 0`: `memory[0] = 6`.
  - `halt`: Execution halts.

## Implementation Guidelines

- Implement `assemble(asm string) []byte` in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go) (or a dedicated `assembler.go` in `bradvm`):
  - Split text by newlines and trim whitespace.
  - Map instruction mnemonics to opcode constants (`Load`, `Store`, `Add`, etc.).
  - Parse register strings (`"r1"` $\rightarrow 1$, `"r2"` $\rightarrow 2$).
  - Parse integers using `strconv.Atoi`.

## Active Recall Checkpoint

- What is the difference between an assembler and a compiler?
- Trace how `jump 11` targets the `beqz` instruction: why address 11? (Hint: calculate the byte sizes of the preceding instructions).

# Challenge 9: Traps, Fault Handling, and Hardware Exception Vectors

## Concept: CPU Traps, Faults, and Abrupt State Termination

- In physical silicon, illegal instructions or invalid memory accesses trigger hardware exceptions rather than crashing the emulator host environment.
- In your refactored architecture, `decode` and `execute` can report errors as structured hardware traps rather than invoking Go's `panic()`.
- Define trap codes:
  - `TrapNone`: Normal execution status.
  - `TrapHalt`: Normal termination via `Halt` (`0xff`).
  - `TrapIllegalOpcode`: Unrecognized opcode encountered at current PC.
  - `TrapInvalidRegister`: Register identifier outside valid bounds (`1` or `2`).
  - `TrapMemoryOutOfBounds`: Instruction fetch or operand address exceeds RAM bounds ($> 255$).

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestTrapHandling(t *testing.T) {
      cases := []struct {
          name         string
          program      []byte
          expectedTrap byte
      }{
          {
              name: "IllegalOpcode",
              program: []byte{
                  0xee, // Undefined opcode
              },
              expectedTrap: 0x02, // TrapIllegalOpcode
          },
          {
              name: "InvalidRegister",
              program: []byte{
                  0x01, 0x05, 0x01, // load r5, 1 (r5 does not exist)
              },
              expectedTrap: 0x03, // TrapInvalidRegister
          },
          {
              name: "MemoryOutOfBounds",
              program: []byte{
                  0x07, 0xfe, // jump 254 (points to instruction requiring 3 bytes at EOF)
              },
              expectedTrap: 0x04, // TrapMemoryOutOfBounds
          },
      }

      for _, tc := range cases {
          t.Run(tc.name, func(t *testing.T) {
              memory := make([]byte, 256)
              copy(memory[8:], tc.program)

              trap := computeWithTrap(memory)
              if trap != tc.expectedTrap {
                  t.Fatalf("expected trap 0x%02x, got 0x%02x", tc.expectedTrap, trap)
              }
          })
      }
  }
  ```

## Architectural Mechanics

- The deep `decode(memory, pc)` function validates memory bounds before reading operands:
  - If `pc >= len(memory)` or `pc + inst.Length > len(memory)`, it returns `TrapMemoryOutOfBounds`.
  - If opcode is unknown, it returns `TrapIllegalOpcode`.
- The `execute(cpu, memory, inst)` function validates register bounds:
  - If `inst.DestReg > 2` or `inst.SrcReg > 2`, it returns `TrapInvalidRegister`.
- The clock loop catches the trap, halts execution cleanly, and records the trap code.

## Implementation Guidelines

- Define trap constants in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go):
  ```go
  const (
      TrapNone              byte = 0x00
      TrapHalt              byte = 0x01
      TrapIllegalOpcode     byte = 0x02
      TrapInvalidRegister   byte = 0x03
      TrapMemoryOutOfBounds byte = 0x04
  )
  ```
- Implement `computeWithTrap(memory []byte) byte` returning the final trap code.
- Keep `compute(memory []byte)` as a wrapper that calls `computeWithTrap`.

## Active Recall Checkpoint

- How does the x86 processor trigger `#UD` (Undefined Opcode exception) and how does Linux handle it via `SIGILL`?
- What is the difference between a trap, a fault, and an interrupt in CPU architecture?

# Challenge 10: Processor Status Register (FLAGS) and Condition Codes

## Concept: ALU Condition Codes and Decoupled Predication

- In `Beqz`, zero checking is coupled directly to the branch instruction.
- Real CPUs separate the calculation of status flags from control flow decisions:
  - Arithmetic operations update a dedicated Status Register (`FLAGS`).
  - Standard flags:
    - Zero Flag ($Z$, bit 0): Set if computation result equals 0.
    - Sign Flag ($S$, bit 1): Set if bit 7 of the result is 1 (negative in two's complement).
    - Carry Flag ($C$, bit 2): Set if unsigned addition wraps past 255 or unsigned subtraction borrows below 0.
    - Overflow Flag ($V$, bit 3): Set if signed addition/subtraction produces an erroneous sign bit.
- Control flow instructions evaluate these flags:
  - `Beq` (`0x09`): Branch if $Z = 1$ (equal / zero).
  - `Bne` (`0x0a`): Branch if $Z = 0$ (not equal / non-zero).
- A comparison instruction `Cmp` (`0x0b`): Computes `r1 - r2`, updates flags, and discards the result without modifying general-purpose registers.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestStatusFlagsAndConditionalBranch(t *testing.T) {
      cases := []struct {
          name         string
          r1Val, r2Val byte
          expectBranch bool
      }{
          {"ValuesEqual", 42, 42, true},
          {"ValuesNotEqual", 42, 99, false},
      }

      for _, tc := range cases {
          t.Run(tc.name, func(t *testing.T) {
              memory := make([]byte, 256)
              memory[1] = tc.r1Val
              memory[2] = tc.r2Val

              // Layout:
              // 8:  load r1, 1      (3 bytes)
              // 11: load r2, 2      (3 bytes)
              // 14: cmp r1, r2      (3 bytes: 14..16) -> sets Zero flag
              // 17: beq 3           (2 bytes: 17..18) -> if Z=1, skip 3-byte store
              // 19: store r1, 0     (3 bytes: 19..21)
              // 22: halt            (1 byte)
              program := []byte{
                  0x01, 0x01, 0x01, // load r1, 1
                  0x01, 0x02, 0x02, // load r2, 2
                  0x0b, 0x01, 0x02, // cmp r1, r2
                  0x09, 0x03,       // beq +3
                  0x02, 0x01, 0x00, // store r1, 0
                  0xff,             // halt
              }
              copy(memory[8:], program)

              compute(memory)

              if tc.expectBranch && memory[0] != 0 {
                  t.Fatalf("expected branch taken (memory[0] == 0), got %d", memory[0])
              }
              if !tc.expectBranch && memory[0] != tc.r1Val {
                  t.Fatalf("expected fallthrough (memory[0] == %d), got %d", tc.r1Val, memory[0])
              }
          })
      }
  }
  ```

## Architectural Mechanics

- The `CPU` struct adds `Flags byte`.
- During `execute`, arithmetic operations (`Add`, `Sub`, `Addi`, `Subi`, `Cmp`) update `cpu.Flags`.
- `Cmp` disables register writeback while enabling flags update.
- Branch instructions evaluate bitwise predicates against `cpu.Flags` and return `FlowJump(target)` or `FlowNext`.

## Implementation Guidelines

- Define flag bitmasks:
  ```go
  const (
      FlagZ byte = 1 << 0 // Zero
      FlagS byte = 1 << 1 // Sign
      FlagC byte = 1 << 2 // Carry
      FlagV byte = 1 << 3 // Overflow
  )
  ```
- Update `CPU` struct:
  ```go
  type CPU struct {
      PC        int
      Registers [3]byte
      Flags     byte
  }
  ```
- Implement flag setting in ALU execution and evaluate in `Beq`/`Bne`.

## Active Recall Checkpoint

- How does the ALU calculate the signed overflow flag ($V$) for an 8-bit addition?
- Why do modern ISAs like x86 use condition flags while RISC-V avoids a flags register in favor of compare-and-branch instructions?

# Challenge 11: Call Stack, Subroutines, and Activation Frames

## Concept: The Call Stack, Return Addresses, and Stack Pointer

- Subroutines allow reusable code execution from multiple callers.
- A call stack stores return addresses dynamically so subroutines know where to return.
- Stack mechanics:
  - Add Stack Pointer (`SP`) to the `CPU` struct, initialized to `0xff`.
  - The stack grows downwards from `0xff` towards `0x00`.
  - `Call <addr>` (`0x0c <addr>`):
    - Pushes the return address (`cpu.PC + 2`) to `memory[cpu.SP]`.
    - Decrements `cpu.SP--`.
    - Sets `cpu.PC = target_addr`.
  - `Ret` (`0x0d`):
    - Increments `cpu.SP++`.
    - Pops return address from `memory[cpu.SP]` into `cpu.PC`.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestSubroutineCallAndReturn(t *testing.T) {
      memory := make([]byte, 256)
      memory[1] = 5 // Input X

      // Program layout:
      // 8:  load r1, 1       (3 bytes: 8..10)  -> r1 = 5
      // 11: call 20          (2 bytes: 11..12) -> return address is 13
      // 13: store r1, 0      (3 bytes: 13..15) -> store result
      // 16: halt             (1 byte: 16)
      //
      // Subroutine: DoubleValue at address 20 (0x14)
      // 20: add r1, r1       (3 bytes: 20..22) -> r1 = r1 + r1
      // 23: ret              (1 byte: 23)      -> returns to 13
      program := []byte{
          0x01, 0x01, 0x01, // 8:  load r1, 1
          0x0c, 0x14,       // 11: call 20
          0x02, 0x01, 0x00, // 13: store r1, 0
          0xff,             // 16: halt
      }
      copy(memory[8:], program)

      subroutine := []byte{
          0x03, 0x01, 0x01, // 20: add r1, r1
          0x0d,             // 23: ret
      }
      copy(memory[20:], subroutine)

      compute(memory)

      if memory[0] != 10 {
          t.Fatalf("expected memory[0] to be 10, got %d", memory[0])
      }
  }
  ```

## Architectural Mechanics

- At initialization: `cpu.SP = 0xff`.
- When `Call` executes at PC 11:
  - Return address is `11 + 2 = 13`.
  - Byte `13` is written to `memory[0xff]`.
  - `cpu.SP` decrements to `0xfe`.
  - `cpu.PC` is diverted to `20`.
- When `Ret` executes at PC 23:
  - `cpu.SP` increments to `0xff`.
  - Byte `memory[0xff]` (`13`) is read into `cpu.PC`.
  - Sequential execution resumes at PC 13.

## Implementation Guidelines

- Define constants:
  ```go
  const (
      Call = 0x0c
      Ret  = 0x0d
  )
  ```
- Add `SP byte` to `CPU` struct, initialized to `0xff`.
- In `execute`, `Call` and `Ret` return `FlowCall` and `FlowRet` directives to the central clock loop.

## Active Recall Checkpoint

- What happens in physical hardware when recursive subroutines run without a base termination condition?
- How do x86 `CALL`/`RET` instructions use the `RSP` and `RIP` registers?

# Challenge 12: Register-Indirect Addressing and Pointer Dereferencing

## Concept: Pointers and Dynamically Computed Addresses

- In direct addressing (`Load` and `Store`), memory addresses are fixed at compile time in the instruction bytes.
- Dynamic data structures (arrays, buffers, linked nodes) require computing memory addresses at runtime.
- Register-indirect addressing uses a register as a pointer:
  - `LoadInd rDest, rAddr` (`0x0e rDest rAddr`): Reads `memory[cpu.Registers[rAddr]]` into `cpu.Registers[rDest]`.
  - `StoreInd rSrc, rAddr` (`0x0f rSrc rAddr`): Writes `cpu.Registers[rSrc]` into `memory[cpu.Registers[rAddr]]`.
- Together with `Addi rAddr, 1`, indirect addressing enables pointer loops and buffer scanning.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestIndirectAddressingArraySum(t *testing.T) {
      memory := make([]byte, 256)
      // Array data stored at addresses 0x20..0x22: [10, 20, 30]
      memory[0x20] = 10
      memory[0x21] = 20
      memory[0x22] = 30

      // Pointer base in memory[1] = 0x20, count in memory[2] = 3
      memory[1] = 0x20
      memory[2] = 3

      // Algorithm:
      // r1 = accumulator (init 0)
      // r2 = pointer
      program := []byte{
          0x01, 0x02, 0x01, // 8:  load r2, 1       (r2 = 0x20)
          0x0e, 0x01, 0x02, // 11: loadind r1, r2   (r1 = memory[r2] = 10)
          0x05, 0x02, 0x01, // 14: addi r2, 1       (r2 = 0x21)
          0x0e, 0x02, 0x02, // 17: loadind r2, r2   (r2 = memory[r2] = 20)
          0x03, 0x01, 0x02, // 20: add r1, r2       (r1 = 10 + 20 = 30)
          0x02, 0x01, 0x00, // 23: store r1, 0      (save 30)
          0xff,             // 26: halt
      }
      copy(memory[8:], program)

      compute(memory)

      if memory[0] != 30 {
          t.Fatalf("expected memory[0] to be 30, got %d", memory[0])
      }
  }
  ```

## Architectural Mechanics

- The memory address bus multiplexer selects the address from the register file output instead of the instruction stream register.
- Allows arbitrary memory dereferencing, enabling heap structures, lookup tables, and stack frame pointer offsets.

## Implementation Guidelines

- Define constants:
  ```go
  const (
      LoadInd  = 0x0e
      StoreInd = 0x0f
  )
  ```
- Implement decode and execute handlers:
  - `LoadInd`: `cpu.Registers[dest] = memory[cpu.Registers[addrReg]]`.
  - `StoreInd`: `memory[cpu.Registers[addrReg]] = cpu.Registers[srcReg]`.

## Active Recall Checkpoint

- How does register-indirect addressing map to C pointer dereferencing (`*ptr`) and array indexing (`arr[i]`)?
- What hardware faults can occur during an indirect memory access in virtual memory systems?

# Challenge 13: Execution Tracing and Disassembler Construction

## Concept: Machine Introspection, Disassembly, and Cycle Accounting

- Inspecting raw byte arrays during VM failures introduces high cognitive overhead.
- A disassembler translates binary bytecode back into symbolic human-readable assembly lines.
- Because your architecture has a deep `decode()` module from Challenge 7, building a disassembler requires zero duplicated instruction length logic.
- An execution tracer records the complete architectural state (`PC`, `Opcode`, `Registers`, `Flags`) on every clock cycle.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestDisassembler(t *testing.T) {
      bytecode := []byte{
          0x01, 0x01, 0x01, // load r1, 1
          0x05, 0x01, 0x03, // addi r1, 3
          0x07, 0x10,       // jump 16
          0xff,             // halt
      }

      expected := []string{
          "load r1, 1",
          "addi r1, 3",
          "jump 16",
          "halt",
      }

      disassembled, err := disassemble(bytecode)
      if err != nil {
          t.Fatalf("disassembly failed: %v", err)
      }

      if len(disassembled) != len(expected) {
          t.Fatalf("expected %d lines, got %d", len(expected), len(disassembled))
      }

      for i := range expected {
          if disassembled[i] != expected[i] {
              t.Errorf("line %d: expected %q, got %q", i, expected[i], disassembled[i])
          }
      }
  }
  ```

## Architectural Mechanics

- The disassembler functions as a non-mutating fetch-decode pipeline:
  - Iterates through the bytecode slice.
  - Calls `decode(bytecode, pc)`.
  - Formats symbolic mnemonic from `Instruction`.
  - Advances `pc += inst.Length`.

## Implementation Guidelines

- Implement `disassemble(bytecode []byte) ([]string, error)` in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go).
- Return error on unrecognized opcodes or truncated operand streams.

## Active Recall Checkpoint

- Why is disassembling variable-length instruction architectures (x86) more complex than fixed-length architectures (ARM/RISC-V)?
- What is the difference between static disassembly and dynamic execution tracing?

# Extension Exercises: Systems Engineering & Mental Model Expansion

## Extension Exercise 1: Two-Pass Assembler with Symbolic Labels

### The Problem with Hardcoded Offsets

- In raw assembly, jumps and branches require manual calculation of byte targets (e.g., `jump 11` or `beqz r1, 8`).
- Adding or removing a single instruction shifts all subsequent memory addresses, requiring error-prone recalculation of all branch targets.

### Two-Pass Architecture

- Pass 1 (Symbol Definition):
  - Scan the assembly source line by line.
  - Track current byte offset starting at 8.
  - Detect label declarations (e.g., `loop:` or `exit:`).
  - Record the label name and byte address in a symbol table (`map[string]byte`).
- Pass 2 (Code Emission):
  - Translate symbolic instructions to machine code.
  - Replace symbolic target references (`jump loop`) with resolved absolute addresses or relative offsets computed from the symbol table.

### Deliverable

- Implement `assembleWithLabels(source string) ([]byte, error)`.
- Test the `Sum to n` program written cleanly with `loop:` and `done:` labels without numeric branch targets.

## Extension Exercise 2: W^X Memory Protection Unit

### Architectural Invariant

- Modern operating systems enforce the W^X (Write XOR Execute) security invariant:
  - Memory pages can be writable, or executable, but never both simultaneously.
  - Prevents arbitrary code injection attacks where user input is executed as machine code.

### Segmentation Rules

- Divide the 256-byte address space into three permission segments:
  - `0x00` to `0x07` (Data Segment): Read-Write, No-Execute (`RW-`).
  - `0x08` to `0xef` (Code Segment): Read-Only, Executable (`R-X`).
  - `0xf0` to `0xff` (Stack Segment): Read-Write, No-Execute (`RW-`).

### Deliverable

- Add permission checks to memory read, write, and execute operations.
- Emit `TrapMemoryProtectionViolation` if:
  - The PC attempts to fetch instructions from the Data segment (`< 0x08`) or Stack segment (`>= 0xf0`).
  - A `Store` or `StoreInd` instruction attempts to write into the Code segment (`0x08` to `0xef`).

## Extension Exercise 3: Memory-Mapped I/O (MMIO) Virtual Teletype

### Concept: Peripheral Communication via Memory Bus

- Physical CPUs communicate with external peripherals (keyboards, serial consoles, network adapters) by mapping device control registers into the memory address space.
- A write to a specific memory address does not store data in RAM; it transmits the data across an I/O bus to a peripheral controller.

### Architectural Mapping

- Designate memory address `0x07` as the `UART_TX` (Serial Teletype Output) data register.
- When the CPU executes `Store` or `StoreInd` targeting address `0x07`:
  - Intercept the write operation before writing to RAM.
  - Transmit the byte value as an ASCII character to an attached `io.Writer`.

### Deliverable

- Add an `io.Writer` interface field to the VM.
- Write an assembly program that prints a null-terminated string (`"HELLO\n"`) stored in data memory to the teletype using a pointer loop.


