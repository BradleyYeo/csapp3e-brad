package main

import (
	"encoding/binary"
	"os"
)

type PacketHeader struct {
	CapturedLen uint32
	OriginalLen uint32
}

func skipGlobalHeader(file *os.File) error {
	_, err := file.Seek(GlobalHeaderSize, 0)
	return err
}

func readPacketHeader(file *os.File) (*PacketHeader, error) {
	headerData := make([]byte, PerPacketHeaderSize)
	_, err := file.Read(headerData)
	if err != nil {
		return nil, err
	}
	return &PacketHeader{
		CapturedLen: binary.LittleEndian.Uint32(headerData[8:12]),
		OriginalLen: binary.LittleEndian.Uint32(headerData[12:16]),
	}, nil
}
