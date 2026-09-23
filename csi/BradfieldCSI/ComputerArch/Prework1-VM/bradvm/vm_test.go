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
