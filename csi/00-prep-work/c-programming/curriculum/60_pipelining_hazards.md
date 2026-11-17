# Conceptual Foundations

## The Five-Stage Pipelined Architecture (CS:APP 4.5)
- Sequential execution (unpipelined) executes one instruction in its entirety before starting the next, forcing the clock cycle period to equal the sum of all execution stages combined.
- Pipelining divides instruction execution into five discrete stages separated by clocked pipeline registers:
  - Fetch (F): Reads the instruction byte stream from memory using the Program Counter (PC) and computes the incremented sequential address (`valP`).
  - Decode (D): Reads up to two operands (`valA`, `valB`) from the register file based on register specifiers (`srcA`, `srcB`).
  - Execute (E): Arithmetic Logic Unit (ALU) performs operations, evaluates branch conditions (Zero Flag, Sign Flag), or computes effective memory addresses.
  - Memory (M): Reads data from RAM (`valM`) for load operations or writes data to RAM for store operations.
  - Write-back (W): Commits results (`valE` from ALU or `valM` from memory) back to the register file destination registers (`dstE`, `dstM`).
- Throughput vs Latency:
  - Latency: The total elapsed time for a single instruction to traverse from Fetch to Write-back (5 clock cycles).
  - Throughput: In the steady state without hazards, one instruction completes every clock cycle (1 instruction per cycle, IPC = 1.0).

## Hardware Hazards Taxonomy
- Pipelining introduces hardware timing conflicts where the next instruction cannot execute in the following clock cycle:
  - Data Hazards (Read-After-Write / RAW): An instruction in Decode requires an operand value from a register that a preceding instruction in Execute, Memory, or Write-back has not yet committed to the register file.
  - Control Hazards: The Fetch stage must determine the next instruction address before a conditional branch instruction in Execute has resolved whether the branch is taken.
  - Structural Hazards: Multiple pipeline stages attempt to access the same physical hardware resource simultaneously (e.g. unified memory bus accessed by both Fetch and Memory stages).

## Hardware Resolution Strategies
- Inserting Bubbles (Pipeline Stalls):
  - The hardware Hazard Detection Unit freezes the Program Counter (PC) and the Fetch/Decode pipeline registers.
  - Dynamically injects a no-operation (bubble / `NOP`) into the Execute stage.
  - Adds dead clock cycles, increasing Cycles Per Instruction (CPI > 1.0) and degrading throughput.
- Data Forwarding (Bypassing):
  - Hardware routing paths connect intermediate stage outputs directly to the ALU input multiplexers in the Execute stage.
  - Forward from Execute ($E \to E$): Routes ALU output directly to the input of the next instruction entering Execute.
  - Forward from Memory ($M \to E$): Routes ALU result or memory read data from the Memory stage to the Execute stage.
  - Forward from Write-back ($W \to E$): Routes committed results from the Write-back stage to the Execute stage.
- The Load-Use Interlock:
  - For `Load` instructions, data is retrieved from RAM at the end of the Memory (M) stage.
  - If the immediately following instruction consumes the loaded register in its Execute (E) stage, forwarding alone cannot travel backward in time.
  - The hazard detection unit must inject 1 bubble stall, after which data is forwarded from the Memory stage to the Execute stage.

---

# Deliverable: Sum to n Assembly Loop Hazard Analysis

## Assembly Source and Instruction Encoding
- Consider the algorithmic `Sum to n` assembly program executed on the 8-bit architecture:
  ```asm
  ; Address 8 (Entry)
  I1: load r1, 1       ; r1 <- memory[1] (n)
  ; Address 11 (Loop Head)
  I2: beqz r1, 8       ; if r1 == 0 jump to address 19 (store r2, 0)
  I3: add r2, r1       ; r2 <- r2 + r1
  I4: subi r1, 1       ; r1 <- r1 - 1
  I5: jump 11          ; jump back to Address 11 (beqz r1, 8)
  ; Address 19 (Exit)
  I6: store r2, 0      ; memory[0] <- r2
  I7: halt             ; clean termination
  ```

## Register Dependency Analysis and RAW Hazard Pairs

### Pair 1: Entry Load-Use (I1 -> I2)
- Producer: `I1: load r1, 1` (writes `r1` in stage W).
- Consumer: `I2: beqz r1, 8` (reads `r1` in stage D).
- Instruction Distance: 1 (immediately adjacent).
- Conflict: `I2` enters Decode at cycle 3, but `I1` does not reach Write-back until cycle 5.
- Stalls without forwarding: 2 clock cycles (bubbles inserted at cycles 3 and 4).
- Stalls with forwarding: 1 clock cycle (Load-Use interlock; memory read completes at end of cycle 4 in stage M, forwarded to cycle 5 in stage E).

### Pair 2: Loop Counter Decrement to Branch (I4 -> I2 across Jump)
- Producer: `I4: subi r1, 1` (writes `r1` in stage W).
- Intervening: `I5: jump 11` (does not write general registers).
- Consumer: `I2: beqz r1, 8` in the subsequent loop iteration.
- Instruction Distance: 2 instructions (separated by `jump`).
- Static RAW Stall Requirement (without forwarding, zero-latency branch):
  - 1 clock cycle (bubble inserted while `I4` moves from M to W).
- Dynamic Hardware Interaction with Control Hazard Flushes:
  - If unconditional `jump` resolves in the Execute stage, the processor flushes the subsequent 2 fetched instructions.
  - These 2 control flush bubbles advance the clock by 2 cycles, allowing `I4` to reach Write-back before `I2` enters Decode.
  - Consequently, in a processor with branch resolution in Execute, the control flush naturally absorbs the data hazard.
- Stalls with forwarding: 0 clock cycles (forwarded directly from Memory stage pipeline register to Execute stage).

### Pair 3: Accumulator Dependencies (I3 -> I3 across Iterations)
- Producer: `I3: add r2, r1` (writes `r2` in stage W).
- Consumer: `I3: add r2, r1` in the subsequent iteration.
- Intervening Instructions: `I4` (`subi`), `I5` (`jump`), `I2` (`beqz`).
- Instruction Distance: 4 instructions apart.
- Conflict: By the time `I3` of iteration $k+1$ enters Decode, `I3` of iteration $k$ has already cleared Write-back (distance $\ge 3$).
- Stalls without forwarding: 0 clock cycles.
- Stalls with forwarding: 0 clock cycles.

### Pair 4: Adjacent Loop Arithmetic (I3 -> I4)
- First Instruction: `I3: add r2, r1` (reads `r1`, `r2`; writes `r2`).
- Second Instruction: `I4: subi r1, 1` (reads `r1`; writes `r1`).
- Register Overlap: Both access `r1`, but both are reading `r1` (Read-After-Read / RAR).
- Register `r2`: Written by `I3`, but not read by `I4`.
- Conflict: None.
- Stalls without forwarding: 0 clock cycles.
- Stalls with forwarding: 0 clock cycles.

### Pair 5: Loop Exit Store (I3 -> I6)
- Producer: Final iteration `I3: add r2, r1` (writes `r2`).
- Consumer: `I6: store r2, 0` (reads `r2`).
- Intervening Instructions: Final `I4` (`subi`), `I5` (`jump`), `I2` (`beqz` taken to exit).
- Instruction Distance: 4 instructions apart.
- Conflict: By the time `I6` reaches Decode, the final `I3` has already completed Write-back.
- Stalls without forwarding: 0 clock cycles.
- Stalls with forwarding: 0 clock cycles.

## Cycle-by-Cycle Pipeline Space-Time Diagram (No Forwarding)
- Space-time trace for iteration with $n = 1$:
  ```
  Cycle:  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
  -------------------------------------------------------------------------------
  I1:     F   D   E   M   W
  I2:         F   D   D   D   E   M   W
  Bubble:         -   -   E   M   W
  Bubble:             -   -   E   M   W
  I3:                     F   D   E   M   W
  I4:                         F   D   E   M   W
  I5:                             F   D   E   M   W
  Flush:                              -   -   -   -
  Flush:                                  -   -   -
  I2:                                     F   D   E   M   W
  I6:                                         F   D   E   M   W
  I7:                                             F   D   E   M   W
  ```
- Key observation:
  - `I1 -> I2` causes 2 data hazard bubbles in stage D.
  - The branch resolution of `I5` flushes instructions behind it, allowing `I4` to reach Write-back before `I2` enters Decode.

## Quantitative Stall Summary
- Static RAW Hazard Analysis (Isolated Instruction Pairs):
  - `I1 -> I2`: Distance 1 $\implies$ 2 stall cycles without data forwarding; 1 stall cycle with data forwarding (load-use).
  - `I4 -> I2`: Distance 2 $\implies$ 1 stall cycle without data forwarding; 0 stall cycles with data forwarding.
  - All other pairs: Distance $\ge 4$ or disjoint registers $\implies$ 0 stall cycles.
- Dynamic Pipeline Execution (Sum to n Program):
  - Without forwarding: 2 data hazard stalls (from `I1 -> I2`), plus control hazard flushes.
  - With forwarding: 1 data hazard stall (from `I1 -> I2` load-use interlock), plus control hazard flushes.

---

# Exercise 1: RAW Data Hazard Detection and Distance Calculation

## Mental Model and Register Conflict Invariant
- Goal: Inspect two instructions $I_A$ and $I_B$ and determine whether a Read-After-Write (RAW) data dependency exists.
- Invariant: A RAW hazard occurs if and only if:
  $$\text{producer.dst} \neq \text{REG\_NONE} \land (\text{producer.dst} == \text{consumer.srcA} \lor \text{producer.dst} == \text{consumer.srcB})$$
- Distance Metric $d$: The number of clock cycles between producer fetch and consumer fetch ($d = \text{cycle}_B - \text{cycle}_A$).
- Stall Cycles Calculation (Standard 5-stage pipeline with split-phase register file):
  - If $d = 1$ (adjacent): Producer is in Execute when consumer enters Decode. Requires 2 stall bubbles.
  - If $d = 2$: Producer is in Memory when consumer enters Decode. Requires 1 stall bubble.
  - If $d \ge 3$: Producer is in Write-back or has committed. Requires 0 stall bubbles.

## Active Recall Checkpoints
- Why does a Write-After-Read (WAR) conflict not cause hazards in an in-order 5-stage pipeline?
- How does a split-phase register file (write on clock rising edge, read on falling edge) eliminate stalls at distance $d = 3$?

---

# Exercise 2: Hardware Stall and Bubble Injection Simulation

## Mental Model and Pipeline Register Control
- Goal: Implement the control signals to stall earlier stages while allowing later stages to drain.
- Invariant 1 (Freeze PC and Fetch/Decode):
  - When a stall condition is detected in Decode, the PC and the Decode pipeline register must retain their current contents (`stall = true`).
- Invariant 2 (Bubble Injection into Execute):
  - The Execute pipeline register must be cleared to a no-operation (`OP_NOP`) to prevent stale or duplicate execution (`bubble = true`).
- Invariant 3 (Monotonic Stage Advance):
  - Stages Execute, Memory, and Write-back advance normally, draining downstream instructions toward retirement.

## Active Recall Checkpoints
- What is the physical difference between an instruction `NOP` fetched from memory and a hardware pipeline `bubble`?
- What occurs if the hazard detection unit injects a bubble into Decode instead of Execute?

---

# Exercise 3: Data Forwarding Unit (Bypass Multiplexers)

## Mental Model and Priority Bypassing
- Goal: Route operand values directly from pipeline registers ($E/M$, $M/W$) to ALU inputs, bypassing the register file.
- Priority Invariant:
  - If both Execute and Memory stages produce the same destination register, the younger instruction (Execute stage) takes precedence:
    $$\text{Forward}_{\text{src}} = \begin{cases} \text{FWD\_EX} & \text{if } E.\text{dst} == \text{src} \land E.\text{dst} \neq 0 \\ \text{FWD\_MEM} & \text{if } M.\text{dst} == \text{src} \land M.\text{dst} \neq 0 \\ \text{FWD\_WB} & \text{if } W.\text{dst} == \text{src} \land W.\text{dst} \neq 0 \\ \text{FWD\_NONE} & \text{otherwise} \end{cases}$$

## Active Recall Checkpoints
- Why must forwarding priority favor the Execute stage over the Memory stage when both write to register `r1`?
- Under what conditions does forwarding fail to eliminate all stalls between two instructions?

---

# Exercise 4: Load-Use Hazard Interlock

## Mental Model and Temporal Boundary Violation
- Goal: Detect when an instruction depends on a memory read that has not yet arrived from RAM.
- Condition for Load-Use Stall:
  $$\text{Decode}.\text{is\_consumer\_of}(E.\text{dst}) \land (E.\text{op} == \text{OP\_LOAD})$$
- Interlock Action:
  - Stall Fetch and Decode for 1 clock cycle.
  - Inject 1 bubble into Execute.
  - On the subsequent clock cycle, forward data from Memory to Execute ($M \to E$).

## Active Recall Checkpoints
- Can compiler instruction scheduling (reordering independent instructions) eliminate load-use stalls without hardware changes?
- Why can an ALU-to-ALU dependency be forwarded with 0 stalls, while a Load-Use dependency requires 1 stall?
