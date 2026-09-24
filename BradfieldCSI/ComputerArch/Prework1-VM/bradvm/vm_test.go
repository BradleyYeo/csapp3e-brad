package bradvm

import (
	// "os"
	// "strconv"
	// "strings"
	"testing"
)

func TestHaltMinimal(t *testing.T) {
	memory := make([]byte, 256)
	memory[8] = 0xff // Halt opcode
	compute(memory)
}

func TestLoadStoreMinimal(t *testing.T) {
	memory := make([]byte, 256)
	memory[1] = 42 // Set input parameter in data segment memory[0x01]
	// Program: load r1 [1]; store r1 [0]; halt
	program := []byte{
		0x01, 0x01, 0x01, // load r1, 1
		0x02, 0x01, 0x00, // store r1, 0  Destination memory address (0x00, the designated output location).
		0xff, // halt
	}
	copy(memory[8:], program) // Flashes the machine code bytes into RAM starting at offset 8 (the code segment entry point).
	compute(memory)

	// address 0x00 is the designated return value/result destination.
	if memory[0] != 42 {
		t.Fatalf("expected memory[0] to be 42, got %d", memory[0])
	}
}

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
				op, 0x01, 0x02, // add/sub r1, r2
				0x02, 0x01, 0x00, // store r1, 0
				0xff, // halt
			}
			copy(memory[8:], program)
			compute(memory)

			if memory[0] != tc.expected {
				t.Fatalf("expected %d, got %d", tc.expected, memory[0])
			}
		})
	}
}

func TestImmediateArithmetic(t *testing.T) {
	memory := make([]byte, 256)
	memory[1] = 20 // input X

	program := []byte{
		0x01, 0x01, 0x01, // load r1, 1 (r1 = 20)
		0x05, 0x01, 0x03, // addi r1, 3 (r1 = 23)
		0x06, 0x01, 0x05, // subi r1, 5 (r1 = 18)
		0x02, 0x01, 0x00, // store r1, 0
		0xff, // halt
	}
	copy(memory[8:], program)

	compute(memory)

	if memory[0] != 18 {
		t.Fatalf("expected memory[0] to be 18, got %d", memory[0])
	}
}

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