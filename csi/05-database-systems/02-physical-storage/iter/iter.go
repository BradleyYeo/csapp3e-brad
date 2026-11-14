package iter

import (
	"errors"
)

type Iterator interface {
	// Init()
	Next() (Tuple, error)
	// Close()
}

type Tuple struct {
	Columns []Column
}

type Column struct {
	Name  string
	Value string
}

var ERROR_EOF = errors.New("EOF")
