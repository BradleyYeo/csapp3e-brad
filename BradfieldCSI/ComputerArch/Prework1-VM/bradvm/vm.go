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

type CPU struct {
	PC        int
	Registers [3]byte
}

type FlowDirective int

const (
	FlowNext FlowDirective = iota
	FlowJump
	FlowHalt
)

type ExecutionResult struct {
	Flow FlowDirective
	NextPC byte
}

func NewCPU() *CPU {
	return &CPU{
		PC: 0x08,
	}
}

func decode(memory []byte, pc int) (Instruction, error) {
	opcode := memory[pc]
	switch opcode {
	case Jump:
		return Instruction{
			Opcode:  opcode,
			Addr: memory[pc+1],
			Length:  2,
		}, nil
	case Addi:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Imm:  memory[pc+2],
			Length:  3,
		}, nil
	case Subi:
		return Instruction{
			Opcode:  opcode,
			DestReg: memory[pc+1],
			Imm:  memory[pc+2],
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

func compute(memory []byte) {
	pc := 0x08
	var registers [3]byte // registers[1] is r1, registers[2] is r2
	for {
		// 1 clock cycle
		instruct, err := decode(memory, pc)
		if err != nil {
			panic(err)
		}
		switch instruct.Opcode {
		case Jump:
			target_addr := instruct.Addr
			pc = int(target_addr)
		case Addi:
			registers[instruct.DestReg] = registers[instruct.DestReg] + instruct.Imm
			pc += instruct.Length
		case Subi:
			registers[instruct.DestReg] = registers[instruct.DestReg] - instruct.Imm
			pc += instruct.Length
		case Add:
			registers[instruct.DestReg] = registers[instruct.DestReg] + registers[instruct.SrcReg]
			pc += instruct.Length
		case Sub:
			registers[instruct.DestReg] = registers[instruct.DestReg] - registers[instruct.SrcReg]
			pc += instruct.Length
		case Load:
			registers[instruct.DestReg] = memory[instruct.Addr]
			pc += instruct.Length
		case Store:
			memory[instruct.Addr] = registers[instruct.SrcReg]
			pc += instruct.Length
		case Beqz:
			reg := memory[pc+1]
			cc := registers[reg]
			relative_offset := int8(memory[pc+2])
			pc += 3
			if cc == 0 {
				pc += int(relative_offset)
			}
		case Halt:
			return
		default:
			panic("Unknown opcode")
		}
	}
}
