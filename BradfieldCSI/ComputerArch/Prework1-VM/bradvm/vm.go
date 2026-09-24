package bradvm

const (
	Load  = 0x01
	Store = 0x02
	Add   = 0x03
	Sub   = 0x04
	Addi  = 0x05
	Subi  = 0x06
	Jump = 0x07
	Halt  = 0xff
)

func compute(memory []byte) {
	pc := 0x08
	var registers [3]byte // registers[1] is r1, registers[2] is r2
	for {
		// 1 clock cycle
		opcode := memory[pc]
		switch opcode {
		case Jump:
			target_addr := memory[pc+1]
			pc = int(target_addr)
		case Addi:
			reg := memory[pc+1]
			currentVal := registers[reg]
			constant := memory[pc+2]
			result := currentVal + constant
			registers[reg] = result
			pc += 3
		case Subi:
			reg := memory[pc+1]
			currentVal := registers[reg]
			constant := memory[pc+2]
			result := currentVal - constant
			registers[reg] = result
			pc += 3
		case Load:
			src_reg := memory[pc+1]
			addr := memory[pc+2]
			val := memory[addr]
			registers[src_reg] = val
			pc += 3
		case Store:
			src_reg := memory[pc+1]
			dest_addr := memory[pc+2]
			memory[dest_addr] = registers[src_reg]
			pc += 3
		case Add:
			dest_reg := memory[pc+1]
			src_reg := memory[pc+2]
			registers[dest_reg] = registers[dest_reg] + registers[src_reg]
			pc += 3
		case Sub:
			dest_reg := memory[pc+1]
			src_reg := memory[pc+2]
			registers[dest_reg] = registers[dest_reg] - registers[src_reg]
			pc += 3
		case Halt:
			return
		default:
			panic("Unknown opcode")
		}
	}
}
