package vm

import (
	"fmt"
)

// Instruction Opcodes (1-byte to 3-byte instructions)
const (
	Load  = 0x01 // load r1, addr      (3 bytes: 0x01 <reg> <addr>)
	Store = 0x02 // store r1, addr     (3 bytes: 0x02 <reg> <addr>)
	Add   = 0x03 // add r1, r2         (3 bytes: 0x03 <dest> <src>)
	Sub   = 0x04 // sub r1, r2         (3 bytes: 0x04 <dest> <src>)
	Addi  = 0x05 // addi r1, imm       (3 bytes: 0x05 <dest> <imm>)
	Subi  = 0x06 // subi r1, imm       (3 bytes: 0x06 <dest> <imm>)
	Jump  = 0x07 // jump addr          (2 bytes: 0x07 <target_addr>)
	Beqz  = 0x08 // beqz r1, offset    (3 bytes: 0x08 <reg> <offset>)
	Halt  = 0xff // halt               (1 byte:  0xff)
)

// Register indices
const (
	pc = 0 // Program Counter (initializes to 8, entry point to code)
	r1 = 1 // General Purpose Register 1
	r2 = 2 // General Purpose Register 2
)

// compute executes the stored machine program in memory.
//
// Memory Architecture:
// - 256 bytes total (byte-addressable 0x00 to 0xff)
// - 0x00: Reserved for return value / output
// - 0x01: Input parameter X
// - 0x02: Input parameter Y
// - 0x03 to 0x07: Scratchpad
// - 0x08 to 0xff: Instruction space
func compute(memory []byte) {
	registers := [3]byte{8, 0, 0} // PC = 8, R1 = 0, R2 = 0

	for {
		// Fetch Phase: Fetch opcode byte at current PC
		opcode := memory[registers[pc]]

		// Execute Phase
		switch opcode {
		case Halt:
			return

		case Load:
			// TODO: Load byte from memory[addr] into destination register
			// Advance PC by 3 bytes

		case Store:
			// TODO: Store byte from source register into memory[addr]
			// Advance PC by 3 bytes

		case Add:
			// TODO: registers[dest] = registers[dest] + registers[src]
			// Advance PC by 3 bytes

		case Sub:
			// TODO: registers[dest] = registers[dest] - registers[src]
			// Advance PC by 3 bytes

		case Addi:
			// TODO: registers[dest] = registers[dest] + immediate
			// Advance PC by 3 bytes

		case Subi:
			// TODO: registers[dest] = registers[dest] - immediate
			// Advance PC by 3 bytes

		case Jump:
			// TODO: Overwrite PC directly with target address
			// Note: Do not advance PC further

		case Beqz:
			// TODO: If registers[reg] == 0, jump PC forward by offset
			// Remember to advance PC past this instruction before adding offset

		default:
			panic(fmt.Sprintf("unknown opcode: 0x%02x at pc=%d", opcode, registers[pc]))
		}
	}
}
