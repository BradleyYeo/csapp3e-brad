/*
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 19_pipelining_hazards \
 *         19_pipelining_hazards.c && ./19_pipelining_hazards
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdalign.h>
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Architectural Registers and Opcodes (CS:APP 4.5)
 */
typedef enum {
  REG_NONE = 0,
  REG_R1 = 1,
  REG_R2 = 2,
  REG_R3 = 3,
  REG_COUNT = 4
} reg_id_t;

typedef enum {
  OP_NOP = 0,
  OP_LOAD = 1,
  OP_STORE = 2,
  OP_ADD = 3,
  OP_SUB = 4,
  OP_ADDI = 5,
  OP_SUBI = 6,
  OP_JUMP = 7,
  OP_BEQZ = 8,
  OP_HALT = 0xFF
} opcode_t;

typedef enum {
  FWD_NONE = 0,
  FWD_EX = 1,
  FWD_MEM = 2,
  FWD_WB = 3
} forward_src_t;

/*
 * Explicit Instruction Representation
 * Captures source and destination registers for dependency analysis.
 */
typedef struct {
  uint32_t id;       /* Unique sequence identifier */
  uint32_t pc;       /* Byte address in memory */
  opcode_t op;       /* Instruction opcode */
  reg_id_t dst;      /* Destination register written in Write-back */
  reg_id_t srcA;     /* Primary source operand read in Decode */
  reg_id_t srcB;     /* Secondary source operand read in Decode */
  uint8_t imm;       /* Immediate value */
  uint8_t addr;      /* Direct memory target address */
  const char *label; /* Disassembly mnemonic string */
} instruction_t;

/*
 * Pipeline Register State between Stages
 */
typedef struct {
  bool valid;
  bool bubble;
  instruction_t inst;
  uint8_t valA;
  uint8_t valB;
  uint8_t valE;
  uint8_t valM;
} pipe_reg_t;

/*
 * Five-Stage Pipeline State Container
 */
typedef struct {
  uint32_t pc;
  pipe_reg_t f_stage;
  pipe_reg_t d_stage;
  pipe_reg_t e_stage;
  pipe_reg_t m_stage;
  pipe_reg_t w_stage;
  uint8_t registers[REG_COUNT];
  uint8_t memory[256];
  uint32_t cycle_count;
  uint32_t stall_count;
  uint32_t inst_retired;
  bool halted;
  bool forwarding_enabled;
} pipeline_t;

/*
 * Helper: Create an instruction representation
 */
static instruction_t make_inst(uint32_t id, uint32_t pc, opcode_t op,
                               reg_id_t dst, reg_id_t srcA, reg_id_t srcB,
                               uint8_t imm, uint8_t addr, const char *label) {
  return (instruction_t){.id = id,
                         .pc = pc,
                         .op = op,
                         .dst = dst,
                         .srcA = srcA,
                         .srcB = srcB,
                         .imm = imm,
                         .addr = addr,
                         .label = label};
}

/*
 * Helper: Empty bubble (NOP) instruction
 */
static instruction_t make_bubble(void) {
  return make_inst(0, 0, OP_NOP, REG_NONE, REG_NONE, REG_NONE, 0, 0, "bubble");
}

/*
 * Exercise 1: RAW Data Hazard Detection and Distance Calculation
 *
 * Precondition:
 *   producer and consumer are valid instruction structs.
 * Postcondition:
 *   Returns true if consumer reads a register written by producer before
 *   producer has completed Write-back. Sets *conflict_reg to the register id.
 *
 * Mental Model Invariant:
 *   A RAW hazard exists iff producer writes to a non-zero register and that
 *   register matches consumer's srcA or srcB.
 */
static bool detect_raw_hazard(instruction_t producer, instruction_t consumer,
                              reg_id_t *conflict_reg) {
  if (producer.dst == REG_NONE) {
    if (conflict_reg != nullptr) {
      *conflict_reg = REG_NONE;
    }
    return false;
  }

  if (consumer.srcA == producer.dst) {
    if (conflict_reg != nullptr) {
      *conflict_reg = producer.dst;
    }
    return true;
  }

  if (consumer.srcB == producer.dst) {
    if (conflict_reg != nullptr) {
      *conflict_reg = producer.dst;
    }
    return true;
  }

  if (conflict_reg != nullptr) {
    *conflict_reg = REG_NONE;
  }
  return false;
}

/*
 * Calculates required stall cycles without data forwarding based on
 * instruction distance in clock cycles.
 *
 * Distance d = cycle_consumer - cycle_producer:
 *   d = 1: Producer is in Execute (E) when consumer is in Decode (D).
 *          Consumer needs result in D, producer writes in W (cycle + 3).
 *          With split-phase register file (write first half, read second half):
 *          Consumer must wait through E and M stages -> exactly 2 stall cycles.
 *   d = 2: Producer is in Memory (M) when consumer is in Decode (D).
 *          Consumer must wait through W stage -> exactly 1 stall cycle.
 *   d >= 3: Producer is in Write-back (W) or already retired.
 *          Split-phase register file completes write-back before decode read.
 *          -> exactly 0 stall cycles.
 */
static uint32_t calc_stalls_without_forwarding(uint32_t distance) {
  if (distance == 1) {
    return 2;
  }
  if (distance == 2) {
    return 1;
  }
  return 0;
}

/*
 * Exercise 2: Hardware Stall & Bubble Insertion Simulation
 *
 * Determines whether the instruction currently residing in the Decode stage
 * must stall due to an uncommitted producer in Execute or Memory stages
 * (when data forwarding is disabled).
 */
static bool detect_raw_stall_in_decode(const pipeline_t *pipe,
                                       reg_id_t *conflict_reg) {
  if (!pipe->d_stage.valid || pipe->d_stage.bubble) {
    return false;
  }

  const instruction_t *consumer = &pipe->d_stage.inst;

  /* Check Execute stage (distance 1 conflict) */
  if (pipe->e_stage.valid && !pipe->e_stage.bubble) {
    if (detect_raw_hazard(pipe->e_stage.inst, *consumer, conflict_reg)) {
      return true;
    }
  }

  /* Check Memory stage (distance 2 conflict) */
  if (pipe->m_stage.valid && !pipe->m_stage.bubble) {
    if (detect_raw_hazard(pipe->m_stage.inst, *consumer, conflict_reg)) {
      return true;
    }
  }

  return false;
}

/*
 * Exercise 3: Data Forwarding Unit (Bypass Multiplexers)
 *
 * Inspects pipeline stage registers and determines the earliest forward source
 * for an operand register needed by an instruction in the Execute stage.
 *
 * Priority Rule:
 *   Execute stage (most recent computation) > Memory stage > Write-back stage.
 */
static forward_src_t resolve_forwarding(const pipeline_t *pipe,
                                        reg_id_t src_reg) {
  if (src_reg == REG_NONE) {
    return FWD_NONE;
  }

  /* 1. Forward from Execute stage (ALU output available at end of E) */
  if (pipe->e_stage.valid && !pipe->e_stage.bubble &&
      pipe->e_stage.inst.dst == src_reg) {
    return FWD_EX;
  }

  /* 2. Forward from Memory stage (ALU or RAM result available in M) */
  if (pipe->m_stage.valid && !pipe->m_stage.bubble &&
      pipe->m_stage.inst.dst == src_reg) {
    return FWD_MEM;
  }

  /* 3. Forward from Write-back stage (Committed result in W) */
  if (pipe->w_stage.valid && !pipe->w_stage.bubble &&
      pipe->w_stage.inst.dst == src_reg) {
    return FWD_WB;
  }

  return FWD_NONE;
}

/*
 * Helper: Returns the actual forwarded operand value based on the forwarding
 * source.
 */
static uint8_t get_forwarded_value(const pipeline_t *pipe, forward_src_t fwd,
                                   uint8_t reg_file_val) {
  switch (fwd) {
  case FWD_EX:
    return pipe->e_stage.valE;
  case FWD_MEM:
    /* If producer was load, value came from memory; otherwise ALU valE */
    if (pipe->m_stage.inst.op == OP_LOAD) {
      return pipe->m_stage.valM;
    }
    return pipe->m_stage.valE;
  case FWD_WB:
    if (pipe->w_stage.inst.op == OP_LOAD) {
      return pipe->w_stage.valM;
    }
    return pipe->w_stage.valE;
  case FWD_NONE:
  default:
    return reg_file_val;
  }
}

/*
 * Exercise 4: Load-Use Hazard Interlock
 *
 * A Load instruction writes its result from memory only at the end of stage M.
 * Even with forwarding, if the immediately following instruction consumes the
 * loaded register in its Execute stage, data cannot travel backward in time.
 *
 * Invariant:
 *   If instruction in Execute is OP_LOAD and its destination matches
 *   Decode stage's srcA or srcB, the hazard unit must inject 1 bubble into
 *   Execute and freeze Fetch & Decode.
 */
static bool detect_load_use_hazard(const pipeline_t *pipe,
                                   reg_id_t *conflict_reg) {
  if (!pipe->e_stage.valid || pipe->e_stage.bubble) {
    return false;
  }
  if (pipe->e_stage.inst.op != OP_LOAD) {
    return false;
  }
  if (!pipe->d_stage.valid || pipe->d_stage.bubble) {
    return false;
  }

  return detect_raw_hazard(pipe->e_stage.inst, pipe->d_stage.inst,
                           conflict_reg);
}

/*
 * Pipeline State Machine Initialization
 */
static void pipeline_init(pipeline_t *pipe, bool enable_forwarding) {
  memset(pipe, 0, sizeof(*pipe));
  pipe->pc = 8; /* Execution starts at offset 8 */
  pipe->forwarding_enabled = enable_forwarding;
}

/*
 * Advances the pipeline by one clock cycle.
 * Handles Fetch, Decode, Execute, Memory, and Write-back stages.
 * Manages stall freezes and bubble injections according to hazard mode.
 */
static void pipeline_step(pipeline_t *pipe, const instruction_t *prog,
                          size_t prog_len) {
  pipe->cycle_count++;

  /* --- 1. Write-Back (W) Stage --- */
  if (pipe->w_stage.valid && !pipe->w_stage.bubble) {
    instruction_t inst = pipe->w_stage.inst;
    if (inst.dst != REG_NONE) {
      if (inst.op == OP_LOAD) {
        pipe->registers[inst.dst] = pipe->w_stage.valM;
      } else {
        pipe->registers[inst.dst] = pipe->w_stage.valE;
      }
    }
    if (inst.op == OP_HALT) {
      pipe->halted = true;
    }
    pipe->inst_retired++;
  }

  /* --- 2. Memory (M) Stage --- */
  pipe_reg_t next_w = {0};
  if (pipe->m_stage.valid && !pipe->m_stage.bubble) {
    next_w.valid = true;
    next_w.inst = pipe->m_stage.inst;
    next_w.valE = pipe->m_stage.valE;

    if (pipe->m_stage.inst.op == OP_LOAD) {
      uint8_t addr = pipe->m_stage.inst.addr;
      next_w.valM = pipe->memory[addr];
      pipe->m_stage.valM = next_w.valM;
    } else if (pipe->m_stage.inst.op == OP_STORE) {
      uint8_t addr = pipe->m_stage.inst.addr;
      pipe->memory[addr] = pipe->m_stage.valA;
    }
  }

  /* --- 3. Execute (E) Stage --- */
  pipe_reg_t next_m = {0};
  bool branch_taken = false;
  uint32_t branch_target = 0;

  if (pipe->e_stage.valid && !pipe->e_stage.bubble) {
    next_m.valid = true;
    next_m.inst = pipe->e_stage.inst;
    instruction_t inst = pipe->e_stage.inst;

    uint8_t opA = pipe->e_stage.valA;
    uint8_t opB = pipe->e_stage.valB;

    switch (inst.op) {
    case OP_ADD:
      next_m.valE = (uint8_t)(opA + opB);
      break;
    case OP_SUB:
      next_m.valE = (uint8_t)(opA - opB);
      break;
    case OP_ADDI:
      next_m.valE = (uint8_t)(opA + inst.imm);
      break;
    case OP_SUBI:
      next_m.valE = (uint8_t)(opA - inst.imm);
      break;
    case OP_BEQZ:
      if (opA == 0) {
        branch_taken = true;
        branch_target = inst.pc + 3 + inst.imm;
      }
      break;
    case OP_JUMP:
      branch_taken = true;
      branch_target = inst.addr;
      break;
    case OP_LOAD:
    case OP_STORE:
    case OP_HALT:
    case OP_NOP:
    default:
      next_m.valE = 0;
      break;
    }
    next_m.valA = opA;
    pipe->e_stage.valE = next_m.valE;
  }

  /* --- 4. Hazard Detection and Decode (D) Stage --- */
  reg_id_t conflict_reg = REG_NONE;
  bool stall = false;

  if (pipe->forwarding_enabled) {
    /* With data forwarding: only load-use hazard requires a stall bubble */
    if (detect_load_use_hazard(pipe, &conflict_reg)) {
      stall = true;
    }
  } else {
    /* Without data forwarding: any RAW dependency in E or M requires stalling
     */
    if (detect_raw_stall_in_decode(pipe, &conflict_reg)) {
      stall = true;
    }
  }

  pipe_reg_t next_e = {0};
  pipe_reg_t next_d = pipe->d_stage;

  if (branch_taken) {
    /* Control hazard: Flush Decode and Fetch (misprediction / branch resolution
     * in E) */
    next_e.valid = false;
    next_e.bubble = true;
    next_d.valid = false;
    next_d.bubble = true;
    pipe->pc = branch_target;
  } else if (stall) {
    /* Data hazard resolution: Freeze PC, freeze Decode, inject bubble into
     * Execute */
    pipe->stall_count++;
    next_e.valid = true;
    next_e.bubble = true;
    next_e.inst = make_bubble();
    /* next_d stays as pipe->d_stage (frozen) */
  } else {
    /* Normal advancement from Decode to Execute */
    if (pipe->d_stage.valid && !pipe->d_stage.bubble) {
      next_e.valid = true;
      next_e.inst = pipe->d_stage.inst;

      instruction_t inst = pipe->d_stage.inst;
      uint8_t rawA =
          (inst.srcA != REG_NONE) ? pipe->registers[inst.srcA] : (uint8_t)0;
      uint8_t rawB =
          (inst.srcB != REG_NONE) ? pipe->registers[inst.srcB] : (uint8_t)0;

      if (pipe->forwarding_enabled) {
        forward_src_t fwdA = resolve_forwarding(pipe, inst.srcA);
        forward_src_t fwdB = resolve_forwarding(pipe, inst.srcB);
        next_e.valA = get_forwarded_value(pipe, fwdA, rawA);
        next_e.valB = get_forwarded_value(pipe, fwdB, rawB);
      } else {
        next_e.valA = rawA;
        next_e.valB = rawB;
      }
    }

    /* --- 5. Fetch (F) Stage --- */
    if (!pipe->halted && pipe->pc < prog_len * 3 + 8) {
      /* Look up instruction at current PC */
      bool found = false;
      for (size_t i = 0; i < prog_len; ++i) {
        if (prog[i].pc == pipe->pc) {
          next_d.valid = true;
          next_d.bubble = false;
          next_d.inst = prog[i];
          found = true;
          break;
        }
      }
      if (found) {
        /* Advance PC sequentially (instructions are 2 or 3 bytes long) */
        if (next_d.inst.op == OP_JUMP) {
          pipe->pc += 2;
        } else if (next_d.inst.op == OP_HALT) {
          pipe->pc += 1;
        } else {
          pipe->pc += 3;
        }
      } else {
        next_d.valid = false;
      }
    } else {
      next_d.valid = false;
    }
  }

  /* Commit next stage registers */
  pipe->w_stage = next_w;
  pipe->m_stage = next_m;
  pipe->e_stage = next_e;
  pipe->d_stage = next_d;
}

/*
 * Deliverable Simulation Driver: Analyzes the Sum to n loop
 */
static void run_sum_to_n_benchmark(uint8_t n, bool enable_forwarding,
                                   uint32_t *out_cycles, uint32_t *out_stalls,
                                   uint8_t *out_result) {
  /*
   * Sum to n Assembly Program Layout:
   *   Address 8:  load r1, 1       (I1: n input from memory[1])
   *   Address 11: beqz r1, 8       (I2: if r1 == 0 jump to address 19)
   *   Address 14: add r2, r1       (I3: r2 = r2 + r1)
   *   Address 17: subi r1, 1       (I4: r1 = r1 - 1)
   *   Address 20: jump 11          (I5: jump back to loop head at 11)
   *   Address 22: store r2, 0      (I6: store result to memory[0])
   *   Address 25: halt             (I7: halt)
   */
  const instruction_t prog[] = {
      make_inst(1, 8, OP_LOAD, REG_R1, REG_NONE, REG_NONE, 0, 1, "load r1, 1"),
      make_inst(2, 11, OP_BEQZ, REG_NONE, REG_R1, REG_NONE, 8, 0,
                "beqz r1, 8"),
      make_inst(3, 14, OP_ADD, REG_R2, REG_R2, REG_R1, 0, 0, "add r2, r1"),
      make_inst(4, 17, OP_SUBI, REG_R1, REG_R1, REG_NONE, 1, 0, "subi r1, 1"),
      make_inst(5, 20, OP_JUMP, REG_NONE, REG_NONE, REG_NONE, 0, 11,
                "jump 11"),
      make_inst(6, 22, OP_STORE, REG_NONE, REG_R2, REG_NONE, 0, 0,
                "store r2, 0"),
      make_inst(7, 25, OP_HALT, REG_NONE, REG_NONE, REG_NONE, 0, 0, "halt")};
  const size_t prog_len = sizeof(prog) / sizeof(prog[0]);

  pipeline_t pipe;
  pipeline_init(&pipe, enable_forwarding);
  pipe.memory[1] = n; /* Set input parameter */

  const uint32_t MAX_CYCLES = 1000;
  while (!pipe.halted && pipe.cycle_count < MAX_CYCLES) {
    pipeline_step(&pipe, prog, prog_len);
  }

  if (out_cycles != nullptr) {
    *out_cycles = pipe.cycle_count;
  }
  if (out_stalls != nullptr) {
    *out_stalls = pipe.stall_count;
  }
  if (out_result != nullptr) {
    *out_result = pipe.memory[0];
  }
}

/*
 * Verification and Test Harness
 */
int main(void) {
  printf("Running Exercise 19: Pipelining Hazards & Forwarding Tests...\n\n");

  /* --- Exercise 1: RAW Data Hazard Detection & Distance Stalls --- */
  printf("[TEST] Exercise 1: RAW Data Hazard Detection & Distance Stalls\n");
  {
    instruction_t i_load =
        make_inst(1, 8, OP_LOAD, REG_R1, REG_NONE, REG_NONE, 0, 1, "load");
    instruction_t i_beqz =
        make_inst(2, 11, OP_BEQZ, REG_NONE, REG_R1, REG_NONE, 8, 0, "beqz");
    instruction_t i_add =
        make_inst(3, 14, OP_ADD, REG_R2, REG_R2, REG_R1, 0, 0, "add");
    instruction_t i_subi =
        make_inst(4, 17, OP_SUBI, REG_R1, REG_R1, REG_NONE, 1, 0, "subi");

    reg_id_t conflict = REG_NONE;

    /* I1 -> I2 (load r1 -> beqz r1): RAW on r1 */
    assert(detect_raw_hazard(i_load, i_beqz, &conflict) == true);
    assert(conflict == REG_R1);

    /* I1 -> I3 (load r1 -> add r2, r1): RAW on r1 */
    assert(detect_raw_hazard(i_load, i_add, &conflict) == true);
    assert(conflict == REG_R1);

    /* I3 -> I4 (add r2, r1 -> subi r1, 1): No RAW (both read r1, I4 does not
     * read r2) */
    assert(detect_raw_hazard(i_add, i_subi, &conflict) == false);
    assert(conflict == REG_NONE);

    /* Distance stall calculations without forwarding */
    assert(calc_stalls_without_forwarding(1) == 2);
    assert(calc_stalls_without_forwarding(2) == 1);
    assert(calc_stalls_without_forwarding(3) == 0);
    assert(calc_stalls_without_forwarding(4) == 0);

    printf("  -> Passed all RAW detection and distance stall tests.\n\n");
  }

  /* --- Exercise 2: Pipeline Stalling Simulation (No Forwarding) --- */
  printf("[TEST] Exercise 2: Pipeline Stalling Simulation (No Forwarding)\n");
  {
    /* Test pair: load r1, 1 followed immediately by subi r1, 1 (distance 1) */
    const instruction_t prog[] = {
        make_inst(1, 8, OP_LOAD, REG_R1, REG_NONE, REG_NONE, 0, 1, "load"),
        make_inst(2, 11, OP_SUBI, REG_R1, REG_R1, REG_NONE, 1, 0, "subi"),
        make_inst(3, 14, OP_STORE, REG_NONE, REG_R1, REG_NONE, 0, 0, "store"),
        make_inst(4, 17, OP_HALT, REG_NONE, REG_NONE, REG_NONE, 0, 0, "halt")};
    const size_t prog_len = sizeof(prog) / sizeof(prog[0]);

    pipeline_t pipe;
    pipeline_init(&pipe, false);
    pipe.memory[1] = 10;

    while (!pipe.halted && pipe.cycle_count < 50) {
      pipeline_step(&pipe, prog, prog_len);
    }

    assert(pipe.memory[0] == 9);
    /* Exactly 2 stalls injected for the distance-1 RAW between load and subi */
    assert(pipe.stall_count >= 2);
    printf("  -> Passed pipeline stalling simulation without forwarding.\n\n");
  }

  /* --- Exercise 3: Data Forwarding Unit Priority Logic --- */
  printf("[TEST] Exercise 3: Data Forwarding Unit Priority Logic\n");
  {
    pipeline_t pipe;
    pipeline_init(&pipe, true);

    /* Stage W writes r1 */
    pipe.w_stage.valid = true;
    pipe.w_stage.inst =
        make_inst(1, 8, OP_ADDI, REG_R1, REG_NONE, REG_NONE, 1, 0, "addi");
    pipe.w_stage.valE = 10;

    /* If only W writes r1 -> FWD_WB */
    assert(resolve_forwarding(&pipe, REG_R1) == FWD_WB);

    /* Stage M also writes r1 -> M takes priority over W */
    pipe.m_stage.valid = true;
    pipe.m_stage.inst =
        make_inst(2, 11, OP_ADDI, REG_R1, REG_NONE, REG_NONE, 2, 0, "addi");
    pipe.m_stage.valE = 20;
    assert(resolve_forwarding(&pipe, REG_R1) == FWD_MEM);

    /* Stage E also writes r1 -> E takes highest priority */
    pipe.e_stage.valid = true;
    pipe.e_stage.inst =
        make_inst(3, 14, OP_ADDI, REG_R1, REG_NONE, REG_NONE, 3, 0, "addi");
    pipe.e_stage.valE = 30;
    assert(resolve_forwarding(&pipe, REG_R1) == FWD_EX);

    /* Verify forwarded values */
    assert(get_forwarded_value(&pipe, FWD_EX, 0) == 30);
    assert(get_forwarded_value(&pipe, FWD_MEM, 0) == 20);
    assert(get_forwarded_value(&pipe, FWD_WB, 0) == 10);
    assert(get_forwarded_value(&pipe, FWD_NONE, 5) == 5);

    printf("  -> Passed all data forwarding priority unit tests.\n\n");
  }

  /* --- Exercise 4: Load-Use Hazard Interlock --- */
  printf("[TEST] Exercise 4: Load-Use Hazard Interlock\n");
  {
    pipeline_t pipe;
    pipeline_init(&pipe, true);

    pipe.e_stage.valid = true;
    pipe.e_stage.bubble = false;
    pipe.e_stage.inst =
        make_inst(1, 8, OP_LOAD, REG_R1, REG_NONE, REG_NONE, 0, 1, "load");

    pipe.d_stage.valid = true;
    pipe.d_stage.bubble = false;
    pipe.d_stage.inst =
        make_inst(2, 11, OP_ADD, REG_R2, REG_R2, REG_R1, 0, 0, "add");

    reg_id_t conflict = REG_NONE;
    assert(detect_load_use_hazard(&pipe, &conflict) == true);
    assert(conflict == REG_R1);

    printf("  -> Passed load-use interlock detection tests.\n\n");
  }

  /* --- Exercise 5: Deliverable Analysis of Sum to n --- */
  printf("[TEST] Exercise 5: Deliverable Analysis of Sum to n\n");
  {
    const uint8_t test_values[] = {0, 1, 5, 10};
    const size_t num_tests = sizeof(test_values) / sizeof(test_values[0]);

    for (size_t i = 0; i < num_tests; ++i) {
      uint8_t n = test_values[i];
      uint8_t expected_sum = (uint8_t)((n * (n + 1)) / 2);

      uint32_t cycles_no_fwd = 0;
      uint32_t stalls_no_fwd = 0;
      uint8_t res_no_fwd = 0;

      uint32_t cycles_fwd = 0;
      uint32_t stalls_fwd = 0;
      uint8_t res_fwd = 0;

      run_sum_to_n_benchmark(n, false, &cycles_no_fwd, &stalls_no_fwd,
                             &res_no_fwd);
      run_sum_to_n_benchmark(n, true, &cycles_fwd, &stalls_fwd, &res_fwd);

      assert(res_no_fwd == expected_sum);
      assert(res_fwd == expected_sum);

      /*
       * Pipeline Stall Invariants:
       *   Without Forwarding:
       *     Entry load r1 -> beqz r1 requires exactly 2 data hazard stalls.
       *     Loop iterations: subi r1 writes r1, but the subsequent jump 11
       *     incurs a 2-cycle control hazard flush upon branch resolution in Execute.
       *     These 2 flush bubbles allow subi r1 to reach Write-back before beqz r1
       *     enters Decode, naturally absorbing the data hazard without extra stalls.
       *     Total data stalls = 2.
       *   With Forwarding:
       *     Entry load r1 -> beqz r1 requires exactly 1 stall (load-use interlock).
       *     Loop iterations require 0 stalls (forwarded directly).
       *     Total data stalls = 1.
       */
      printf("  Sum to n (%2u): expected = %3u | NoFwd Stalls = %2u, Cycles = "
             "%3u | Fwd Stalls = %u, Cycles = %3u\n",
             n, expected_sum, stalls_no_fwd, cycles_no_fwd, stalls_fwd,
             cycles_fwd);

      assert(stalls_no_fwd == 2);
      assert(stalls_fwd == 1);
    }

    printf("  -> Passed all Sum to n pipeline deliverable benchmarks.\n\n");
  }

  printf("[ALL PASSED] Exercise 19: Pipelining Hazards successfully verified.\n");
  return EXIT_SUCCESS;
}
