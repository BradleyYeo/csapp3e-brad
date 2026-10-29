package bradvm

import (
	"fmt"
)

const (
	Load  = 0x01
	Store = 0x02
	Add   = 0x03
	Sub   = 0x04
	Addi  = 0x05
	Subi  = 0x06
	Jump  = 0x07
	Beqz  = 0x08
	Halt  = 0xff
)

type Instruction struct {
	Opcode  byte
	DestReg byte
	SrcReg  byte
	Addr    byte
	Imm     byte // immediate value a constant
	Length  int
}

func decode(memory []byte, pc int) (Instruction, error) {
	opcode := memory[pc]
	switch opcode {
	case Beqz:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Imm: memory[pc+2],
			Length:  3,
		}, nil
	case Jump:
		return Instruction{
			Opcode: opcode,
			Addr:   memory[pc+1],
			Length: 2,
		}, nil
	case Addi:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Imm:     memory[pc+2],
			Length:  3,
		}, nil
	case Subi:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Imm:     memory[pc+2],
			Length:  3,
		}, nil
	case Add:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			SrcReg:  memory[pc+2],
			Length:  3,
		}, nil
	case Sub:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			SrcReg:  memory[pc+2],
			Length:  3,
		}, nil
	case Load:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Addr:    memory[pc+2],
			Length:  3,
		}, nil
	case Store:
		return Instruction{
			Opcode: opcode,
			SrcReg: memory[pc+1],
			Addr:   memory[pc+2],
			Length: 3,
		}, nil
	case Halt:
		return Instruction{
			Opcode: opcode,
			Length: 1}, nil
	default:
		return Instruction{}, fmt.Errorf("unknown opcode: 0x%02x", opcode)
	}
}

type CPU struct {
	PC        int
	Registers [3]byte
}

func NewCPU() *CPU {
	return &CPU{
		PC: 0x08,
	}
}

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

