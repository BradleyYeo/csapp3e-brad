package bradvm

import (
	"fmt"
)

func execute(cpu *CPU, memory []byte, inst Instruction) (FlowDirective, error) {
	switch inst.Opcode {
	case Beqz:
		if cpu.Registers[inst.DestReg] != 0 {
			return FlowDirective{Kind: FlowNext}, nil
		}
		offset := int8(inst.Imm)
		target := cpu.PC + inst.Length + int(offset)
		return FlowDirective{
			Kind:   FlowJump,
			Target: target,
		}, nil
	case Jump:
		return FlowDirective{
			Kind:   FlowJump,
			Target: int(inst.Addr),
		}, nil
	case Addi:
		cpu.Registers[inst.DestReg] = cpu.Registers[inst.DestReg] + inst.Imm
		return FlowDirective{Kind: FlowNext}, nil
	case Subi:
		cpu.Registers[inst.DestReg] = cpu.Registers[inst.DestReg] - inst.Imm
		return FlowDirective{Kind: FlowNext}, nil
	case Add:
		cpu.Registers[inst.DestReg] = cpu.Registers[inst.DestReg] + cpu.Registers[inst.SrcReg]
		return FlowDirective{Kind: FlowNext}, nil
	case Sub:
		cpu.Registers[inst.DestReg] = cpu.Registers[inst.DestReg] - cpu.Registers[inst.SrcReg]
		return FlowDirective{Kind: FlowNext}, nil
	case Load:
		cpu.Registers[inst.DestReg] = memory[inst.Addr]
		return FlowDirective{Kind: FlowNext}, nil
	case Store:
		memory[inst.Addr] = cpu.Registers[inst.SrcReg]
		return FlowDirective{Kind: FlowNext}, nil
	case Halt:
		return FlowDirective{Kind: FlowHalt}, nil
	default:
		return FlowDirective{}, fmt.Errorf("unknown opcode: 0x%02x", inst.Opcode)
	}
}

func Run(memory []byte) {
	cpu := NewCPU()
	for {
		// 1 clock cycle
		inst, err := decode(memory, cpu.PC)
		if err != nil {
			panic(err)
		}
		flow, err := execute(cpu, memory, inst)
		switch flow.Kind {
		case FlowNext:
			cpu.PC += inst.Length
		case FlowJump:
			cpu.PC = flow.Target
		case FlowHalt:
			return
		}

	}
}
