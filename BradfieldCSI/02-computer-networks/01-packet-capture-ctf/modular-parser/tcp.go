package main

import (
	"encoding/binary"
	"fmt"
	"os"
)

type TCPHeader struct {
	SourcePort      uint16
	DestinationPort uint16
	SequenceNumber  uint32
	HeaderLength    uint8
}

func parseTCPHeader(file *os.File) (*TCPHeader, []byte, error) {
	// Read the first 20 bytes which is the minimum size of the TCP header
	buffer := make([]byte, 20)
	_, err := file.Read(buffer)
	if err != nil {
		return nil, nil, fmt.Errorf("error reading TCP header: %v", err)
	}

	header := &TCPHeader{
		SourcePort:      binary.BigEndian.Uint16(buffer[0:2]),
		DestinationPort: binary.BigEndian.Uint16(buffer[2:4]),
		SequenceNumber:  binary.BigEndian.Uint32(buffer[4:8]),
		HeaderLength:    (buffer[12] >> 4) * 4, // Multiply by 4 to convert to bytes
	}

	// If the TCP header length indicates that it's longer than 20 bytes,
	// it means there are options. Read the additional data.
	if header.HeaderLength > 20 {
		additionalBytes := make([]byte, header.HeaderLength-20)
		_, err := file.Read(additionalBytes)
		if err != nil {
			return nil, nil, fmt.Errorf("error reading TCP header options: %v", err)
		}
		buffer = append(buffer, additionalBytes...)
	}

	return header, buffer, nil
}
