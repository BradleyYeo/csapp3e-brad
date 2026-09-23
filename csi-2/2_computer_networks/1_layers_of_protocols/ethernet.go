package main

import (
	"encoding/binary"
	"os"
)

type EthernetHeader struct {
	DestMAC   []byte
	SourceMAC []byte
	EtherType uint16
}

func readEthernetHeader(file *os.File) (*EthernetHeader, error) {
	headerData := make([]byte, EthernetHeaderSize)
	_, err := file.Read(headerData)
	if err != nil {
		return nil, err
	}
	return &EthernetHeader{
		DestMAC:   headerData[0:6],
		SourceMAC: headerData[6:12],
		EtherType: binary.BigEndian.Uint16(headerData[12:14]),
	}, nil
}
