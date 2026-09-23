package main

import (
	"fmt"
)

const x int64 = 10 // wtf...

const (
	idKey   = "id"
	nameKey = "name"
)

const z = 20 * 10

func main() {
	const y = "hello"
	fmt.Println(x)
	fmt.Println(y)

	// cannot reassign value to const variables
	x = x + 1
	y = "bye"

	fmt.Println(x)
	fmt.Println(y)
}
