# Curriculum Overview and Methodology

## Test-Driven Hardware Emulation

- Build the virtual machine one layer at a time using Test-Driven Development (TDD).
- For each challenge:
  - Write the test in [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go).
  - Run the test using `go test -v ./...` to verify it fails for the expected reason.
  - Implement the minimal code in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go) to pass the test.
  - Review the architectural concepts and active recall checkpoints before proceeding.

## Conceptual Layering Strategy

- Begin with raw machine code bytes (`[]byte`) rather than symbolic strings to master the underlying binary representation.
- Introduce arithmetic and data transfer before control flow.
- Address architectural flaws identified in [bradvm/CONCEPTS.md](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/CONCEPTS.md) (shallow decoders, coupled Program Counter updates) via structured refactoring milestones.

# Challenge 1: The Oscillator and Halt

## Concept: Hardware Clock and Execution Termination

- A physical CPU requires an oscillating clock signal to drive state transitions across clock cycles.
- In software emulation, an infinite `for` loop acts as the clock generator.
- The machine begins executing instructions starting at address `0x08` (the boundary between data and code space).
- The `Halt` instruction (`0xff`) signals the CPU to stop the clock and terminate execution cleanly.

## Test Specification

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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

- Define opcode constants in [bradvm/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm.go):
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

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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

- Add tests covering normal arithmetic and overflow boundaries to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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

- Add the following test to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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
  - In [introduction-prework/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go#L89-L93), `beqzProcedure` adds `offset` to `pc`, and then the outer loop *also* adds `maxOffset + 1 = 3`.
  - This dual authority creates temporal coupling where the offset must be written relative to the instruction end rather than the current instruction pointer.
  - In your implementation, ensure control flow updates are transparent and clearly defined.

## Test Specification

- Add tests for both taken and not-taken branch conditions to [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
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

## Concept: Ousterhout's Deep Modules and Koppel's Single Authority Principle

- In [introduction-prework/vm.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/introduction-prework/vm.go), decoding and execution suffer from two key architectural flaws:
  - Shallow Module: `decodeOperands` returns a raw packed `uint16` and requires the caller to know `maxOffset`, leading to repeated bit-shifting logic across every procedure.
  - Dual Authority over PC: Both `beqzProcedure`/`jumpProcedure` and the outer loop modify `registers[pc]`.
- Refactoring goals:
  - John Ousterhout: Hide instruction format complexity behind a clean `Instruction` abstraction.
  - Jimmy Koppel: Eliminate temporal coupling by having instruction execution return an explicit control flow directive.

## Architectural Refactoring Requirements

### 1. The Instruction Model

- Create a structured representation that encapsulates decoded fields:
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

### 2. The Deep Decoder

- Implement a decoder that takes the memory slice and current PC, returning a complete `Instruction` without requiring caller hints:
  ```go
  func decode(memory []byte, pc byte) (Instruction, error)
  ```
- This completely encapsulates length calculations, operand offsets, and boundary checks.

### 3. Explicit Control Flow Directive

- Model the outcome of instruction execution as an explicit state transition directive:
  ```go
  type FlowDirective int

  const (
      FlowNext FlowDirective = iota
      FlowJump
      FlowHalt
  )

  type ExecutionResult struct {
      Flow       FlowDirective
      NextPC     byte
  }
  ```
- The execution phase never mutates `pc` directly. It returns `ExecutionResult`, making the outer clock loop the sole authority that updates the Program Counter.

## Verification Strategy

- Run `go test -v ./...` in `bradvm`.
- Every test from Challenges 1 through 6 must pass without modification after the refactor.

## Active Recall Checkpoint

- Why does centralizing state updates into a single authority reduce subtle concurrency and off-by-one errors?
- How does the `Instruction` struct reduce the cognitive load of adding a new instruction to the VM?

# Challenge 8: Assembler Construction and Algorithmic Execution

## Concept: Symbolic Assembly Translation and Looping Constructs

- Writing raw byte slices is prone to human error and obscures algorithmic intent.
- An assembler translates symbolic human-readable assembly lines into machine code bytes.
- The `Sum to n` algorithm tests all architectural concepts working in harmony:
  - Input reading (`Load`).
  - Loop termination condition (`Beqz`).
  - Accumulation (`Add`).
  - Counter decrement (`Subi`).
  - Unconditional backward loop jump (`Jump`).
  - Result writeback (`Store`).
  - Termination (`Halt`).

## Test Specification

- Implement `assemble(asm string) []byte` and test the `Sum to n` program in [bradvm/vm_test.go](file:///Users/bradleyyeo/Documents/learn/c-learn/chettriyuvraj/BradfieldCSI/ComputerArch/Prework1-VM/bradvm/vm_test.go):
  ```go
  func TestSumToN(t *testing.T) {
      asm := `
  load r1 1
  beqz r1 8
  add r2 r1
  subi r1 1
  jump 11
  store r2 0
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

- Algorithm trace for $n = 3$:
  - `load r1 1`: `r1 = 3`.
  - Loop iteration 1: `r1 != 0` (no branch). `r2 = 0 + 3 = 3`. `r1 = 3 - 1 = 2`. `jump 11`.
  - Loop iteration 2: `r1 != 0`. `r2 = 3 + 2 = 5`. `r1 = 2 - 1 = 1`. `jump 11`.
  - Loop iteration 3: `r1 != 0`. `r2 = 5 + 1 = 6`. `r1 = 1 - 1 = 0`. `jump 11`.
  - Loop iteration 4: `r1 == 0`. `beqz r1 8` takes branch, jumping forward by 8 bytes over `add`, `subi`, `jump`.
  - Land at `store r2 0`: `memory[0] = 6`.
  - `halt`: Exit.

## Implementation Guidelines

- Build `assemble(asm string) []byte`:
  - Tokenize lines and whitespace.
  - Map instruction names to opcodes.
  - Map `"r1"` $\rightarrow 1$, `"r2"` $\rightarrow 2$.
  - Parse numerical literals using `strconv.Atoi`.

## Active Recall Checkpoint

- What is the difference between an assembler and a compiler?
- Trace how `jump 11` targets the `beqz` instruction: why address 11? (Hint: calculate the byte sizes of the preceding instructions).
