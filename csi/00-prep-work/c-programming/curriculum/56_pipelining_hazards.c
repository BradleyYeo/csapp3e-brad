/*
 * Exercise 19: Pipelining Hazards, Stalls, and Data Forwarding (CS:APP 4.5)
 *
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
 * Architectural Registers (CS:APP 4.5)
 */
typedef enum {
  REG_NONE = 0,
  REG_R1 = 1,
  REG_R2 = 2,
  REG_R3 = 3,
  REG_COUNT = 4
} reg_id_t;

/*
 * Opcodes modeled after BradVM / CS:APP
 */
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

/*
 * Data Forwarding Sources for ALU Input Multiplexers
 */
typedef enum {
  FWD_NONE = 0, /* Read value from register file */
  FWD_EX = 1,   /* Bypass from Execute stage (ALU output) */
  FWD_MEM = 2,  /* Bypass from Memory stage (ALU result or memory read) */
  FWD_WB = 3    /* Bypass from Write-back stage (Committed result) */
} forward_src_t;

/*
 * Instruction Representation
 */
typedef struct {
  uint32_t id;       /* Unique instruction identifier */
  uint32_t pc;       /* Memory address */
  opcode_t op;       /* Opcode */
  reg_id_t dst;      /* Destination register written in Write-back */
  reg_id_t srcA;     /* Primary source register read in Decode */
  reg_id_t srcB;     /* Secondary source register read in Decode */
  uint8_t imm;       /* Immediate operand */
  uint8_t addr;      /* Direct memory target address */
  const char *label; /* Disassembly string */
} instruction_t;

/*
 * Pipeline Register State between Stages (F, D, E, M, W)
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
 * Five-Stage Pipeline State Machine Container
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
 * Helper Constructors
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

static instruction_t make_bubble(void) {
  return make_inst(0, 0, OP_NOP, REG_NONE, REG_NONE, REG_NONE, 0, 0, "bubble");
}

[[maybe_unused]] static void pipeline_init(pipeline_t *pipe, bool enable_forwarding) {
  memset(pipe, 0, sizeof(*pipe));
  pipe->pc = 8;
  pipe->forwarding_enabled = enable_forwarding;
}

/*
 * ============================================================================
 * Exercise 1: RAW Data Hazard Detection & Distance Stalls
 * ============================================================================
 *
 * Precondition:
 *   producer and consumer are valid instruction structs.
 * Postcondition:
 *   Returns true if consumer reads a register written by producer before
 *   producer has completed Write-back.
 *   If conflict_reg is non-null, assigns *conflict_reg to the conflicting register id.
 *
 * Mental Model Invariant:
 *   A RAW hazard exists if producer writes to a non-zero register (dst != REG_NONE)
 *   and producer.dst matches consumer.srcA or consumer.srcB.
 */
[[maybe_unused]] static bool detect_raw_hazard(instruction_t producer, instruction_t consumer,
                              reg_id_t *conflict_reg) {
  // TODO: Implement RAW hazard predicate
  (void)producer;
  (void)consumer;
  if (conflict_reg != nullptr) {
    *conflict_reg = REG_NONE;
  }
  return false;
}

/*
 * Precondition:
 *   distance d = cycle_consumer - cycle_producer (where d >= 1).
 * Postcondition:
 *   Returns number of stall cycles required without data forwarding.
 *
 * Mental Model Invariant:
 *   - d = 1: Producer in E when consumer in D. Requires 2 stall bubbles.
 *   - d = 2: Producer in M when consumer in D. Requires 1 stall bubble.
 *   - d >= 3: Producer in W or retired. Split-phase register file allows
 *             write in first half of cycle and read in second half -> 0 stalls.
 */
[[maybe_unused]] static uint32_t calc_stalls_without_forwarding(uint32_t distance) {
  // TODO: Implement distance-to-stall mapping
  (void)distance;
  return 0;
}

/*
 * ============================================================================
 * Exercise 2: Pipeline Stalling & Bubble Injection Simulation
 * ============================================================================
 *
 * Precondition:
 *   pipe points to an active pipeline state where data forwarding is disabled.
 * Postcondition:
 *   Returns true if the instruction in Decode (d_stage) has a RAW dependency
 *   against an uncommitted producer in Execute (e_stage) or Memory (m_stage).
 */
static bool detect_raw_stall_in_decode(const pipeline_t *pipe,
                                       reg_id_t *conflict_reg) {
  // TODO: Implement stall condition check for Decode stage
  (void)pipe;
  if (conflict_reg != nullptr) {
    *conflict_reg = REG_NONE;
  }
  return false;
}

/*
 * ============================================================================
 * Exercise 3: Data Forwarding Unit (Bypass Multiplexers)
 * ============================================================================
 *
 * Precondition:
 *   src_reg is a register required by an instruction in Decode/Execute.
 * Postcondition:
 *   Returns the earliest forwarding stage (FWD_EX, FWD_MEM, FWD_WB, or FWD_NONE).
 *
 * Mental Model Invariant:
 *   Priority order: Execute stage (most recent ALU output) > Memory stage > Write-back stage.
 */
static forward_src_t resolve_forwarding(const pipeline_t *pipe,
                                        reg_id_t src_reg) {
  // TODO: Implement priority forwarding multiplexer selection
  (void)pipe;
  (void)src_reg;
  return FWD_NONE;
}

/*
 * Precondition:
 *   fwd indicates forwarding source, reg_file_val is the raw register file value.
 * Postcondition:
 *   Returns the forwarded 8-bit value according to the bypass source.
 */
static uint8_t get_forwarded_value(const pipeline_t *pipe, forward_src_t fwd,
                                   uint8_t reg_file_val) {
  // TODO: Implement forwarded operand value extraction
  (void)pipe;
  (void)fwd;
  return reg_file_val;
}

/*
 * ============================================================================
 * Exercise 4: Load-Use Hazard Interlock
 * ============================================================================
 *
 * Precondition:
 *   pipe points to an active pipeline state with forwarding enabled.
 * Postcondition:
 *   Returns true if instruction in Execute is OP_LOAD and its destination register
 *   is read by the instruction in Decode (srcA or srcB).
 *
 * Mental Model Invariant:
 *   A Load retrieves RAM data only at the end of stage M. Even with forwarding,
 *   a consumer in Decode cannot proceed without 1 bubble injected into Execute.
 */
static bool detect_load_use_hazard(const pipeline_t *pipe,
                                   reg_id_t *conflict_reg) {
  // TODO: Implement load-use hazard interlock check
  (void)pipe;
  if (conflict_reg != nullptr) {
    *conflict_reg = REG_NONE;
  }
  return false;
}

/*
 * ============================================================================
 * Pipeline Cycle Step Simulation Engine
 * ============================================================================
 */
[[maybe_unused]] static void pipeline_step(pipeline_t *pipe, const instruction_t *prog,
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
    if (detect_load_use_hazard(pipe, &conflict_reg)) {
      stall = true;
    }
  } else {
    if (detect_raw_stall_in_decode(pipe, &conflict_reg)) {
      stall = true;
    }
  }

  pipe_reg_t next_e = {0};
  pipe_reg_t next_d = pipe->d_stage;

  if (branch_taken) {
    next_e.valid = false;
    next_e.bubble = true;
    next_d.valid = false;
    next_d.bubble = true;
    pipe->pc = branch_target;
  } else if (stall) {
    pipe->stall_count++;
    next_e.valid = true;
    next_e.bubble = true;
    next_e.inst = make_bubble();
  } else {
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

  pipe->w_stage = next_w;
  pipe->m_stage = next_m;
  pipe->e_stage = next_e;
  pipe->d_stage = next_d;
}

/*
 * ============================================================================
 * Exercise 5: Deliverable Simulation Benchmark
 * ============================================================================
 */
[[maybe_unused]] static void run_sum_to_n_benchmark(uint8_t n, bool enable_forwarding,
                                   uint32_t *out_cycles, uint32_t *out_stalls,
                                   uint8_t *out_result) {
  // TODO: Build the Sum to n assembly program and simulate until halt
  (void)n;
  (void)enable_forwarding;
  if (out_cycles != nullptr) *out_cycles = 0;
  if (out_stalls != nullptr) *out_stalls = 0;
  if (out_result != nullptr) *out_result = 0;
}

/*
 * ============================================================================
 * Test Harness: Author Your Tests Below
 * ============================================================================
 */
int main(void) {
  printf("Running Exercise 19: Pipelining Hazards Test Suite...\n\n");

  /*
   * --- Exercise 1: RAW Data Hazard Detection & Distance Stalls ---
   *
   * TODO: Author test cases to verify:
   * 1. detect_raw_hazard returns true when consumer.srcA or srcB matches producer.dst.
   * 2. detect_raw_hazard returns false when producer.dst is REG_NONE.
   * 3. detect_raw_hazard returns false for Read-After-Read (both reading same register).
   * 4. calc_stalls_without_forwarding returns 2 for dist 1, 1 for dist 2, 0 for dist >= 3.
   */
  printf("[TEST] Exercise 1: RAW Data Hazard Detection & Distance Stalls\n");
  {
    // Write your test cases with assert() here
    printf("  -> TODO: Write Exercise 1 tests.\n\n");
    instruction_t i_load = make_inst(1, 8, OP_LOAD, REG_R1, REG_NONE, REG_NONE,
                                     0, 1, "load r1, 1") instruction_t i_beqz =
        make_inst(2, 11, OP_BEQZ, REG_NONE, REG_R1, REG_NONE, reg_id_t srcB,
                  uint8_t imm, uint8_t addr, const char *label)
  }

  /*
   * --- Exercise 2: Pipeline Stalling Simulation (No Forwarding) ---
   *
   * TODO: Author test cases to verify:
   * 1. detect_raw_stall_in_decode triggers when e_stage or m_stage produces consumer register.
   * 2. Simulating adjacent instructions (e.g. load r1, 1 followed by subi r1, 1) inserts
   *    at least 2 stall bubbles and completes with correct memory result.
   */
  printf("[TEST] Exercise 2: Pipeline Stalling Simulation (No Forwarding)\n");
  {
    // Write your test cases with assert() here
    printf("  -> TODO: Write Exercise 2 tests.\n\n");
  }

  /*
   * --- Exercise 3: Data Forwarding Unit Priority Logic ---
   *
   * TODO: Author test cases to verify:
   * 1. If only Write-back writes r1, resolve_forwarding returns FWD_WB.
   * 2. If both Memory and Write-back write r1, Memory takes priority (FWD_MEM).
   * 3. If Execute, Memory, and Write-back all write r1, Execute takes highest priority (FWD_EX).
   * 4. get_forwarded_value extracts correct ALU valE or Memory valM.
   */
  printf("[TEST] Exercise 3: Data Forwarding Unit Priority Logic\n");
  {
    // Write your test cases with assert() here
    printf("  -> TODO: Write Exercise 3 tests.\n\n");
  }

  /*
   * --- Exercise 4: Load-Use Hazard Interlock ---
   *
   * TODO: Author test cases to verify:
   * 1. detect_load_use_hazard triggers only when e_stage.inst.op == OP_LOAD
   *    and d_stage.inst reads the loaded register.
   */
  printf("[TEST] Exercise 4: Load-Use Hazard Interlock\n");
  {
    // Write your test cases with assert() here
    printf("  -> TODO: Write Exercise 4 tests.\n\n");
  }

  /*
   * --- Exercise 5: Deliverable Analysis of Sum to n ---
   *
   * TODO: Author test cases to verify:
   * 1. run_sum_to_n_benchmark computes correct sum n*(n+1)/2 for n in {0, 1, 5, 10}.
   * 2. Compare stall counts: No-Forwarding stalls vs Forwarding stalls.
   */
  printf("[TEST] Exercise 5: Deliverable Analysis of Sum to n\n");
  {
    // Write your test cases with assert() here
    printf("  -> TODO: Write Exercise 5 tests.\n\n");
  }

  printf("[STATUS] Complete the TODOs to finish Exercise 19.\n");
  return EXIT_SUCCESS;
}
