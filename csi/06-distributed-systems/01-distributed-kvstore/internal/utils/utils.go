package utils

import (
	"fmt"
	"strings"
)

func ParseInput(input string) (operation, key, value string, error error) {
	operations := map[string]bool{"set": true, "get": true}

	cmd := strings.Split(input, " ")
	if len(cmd) != 2 {
		error = fmt.Errorf("Error: Invalid input -> %s", cmd)
		return "", "", "", error
	}

	operation = cmd[0]
	if _, ok := operations[operation]; !ok {
		error = fmt.Errorf("Error: Invalid instruction. Please make sure to use either 'get' or 'set'.")
		return "", "", "", error
	}

	split_keyvalue := strings.Split(cmd[1], "=")

	if len(split_keyvalue) == 2 {
		key, value = split_keyvalue[0], split_keyvalue[1]
		return operation, key, value, nil
	}
	key = split_keyvalue[0]
	return operation, key, "", nil
}
