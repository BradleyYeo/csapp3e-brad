package wal

import (
	"bytes"
	"encoding/binary"
	"os"
	"path/filepath"
)

const (
	OpPut    = 0x01
	OpDelete = 0x02
)

type WAL struct {
	File *os.File
}

func NewWAL(path string) (*WAL, error) {
	dir := filepath.Dir(path)
	if _, err := os.Stat(dir); os.IsNotExist(err) {
		os.MkdirAll(dir, 0755)
	}

	f, err := os.OpenFile(path, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
	if err != nil {
		return nil, err
	}

	return &WAL{
		File: f,
	}, nil
}

func (wal *WAL) Write(op byte, key, value []byte) error {
	f, err := os.OpenFile("logs/leveldb.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)

	if err != nil {
		return err
	}

	defer f.Close()

	buf := new(bytes.Buffer)
	buf.WriteByte(op)
	buf.Write(encodeUint32(uint32(len(key))))
	if op == OpPut {
		buf.Write(encodeUint32(uint32(len(value))))
	}
	buf.Write(key)
	if op == OpPut {
		buf.Write(value)
	}

	_, err = f.Write(buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

func encodeUint32(n uint32) []byte {
	buf := make([]byte, 4)
	binary.BigEndian.PutUint32(buf, n)
	return buf
}
