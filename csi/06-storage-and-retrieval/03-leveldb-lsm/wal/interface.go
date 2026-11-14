package wal

type WriteAheadLog interface {
	// Write to the Write Ahead Log
	Write() error
}
