package main

import (
	"encoding/binary"
	"os"
)

type IPHeader struct {
	Version       uint8
	IHL           uint8
	TotalLength   uint16
	Protocol      uint8
	SourceIP      [4]byte
	DestinationIP [4]byte
	PayloadLength int
}

const (
	TCPProtocol = 0x06
	UDPProtocol = 0x11
)

func readIPHeader(file *os.File) (*IPHeader, error) {
	headerData := make([]byte, 20) // Minimum IP header size
	_, err := file.Read(headerData)
	if err != nil {
		return nil, err
	}

	// Extract fields from the IP header
	versionAndIHL := headerData[0]
	version := versionAndIHL >> 4
	ihl := uint8(versionAndIHL & 0x0F)
	totalLength := binary.BigEndian.Uint16(headerData[2:4])
	protocol := headerData[9]
	sourceIP := [4]byte{headerData[12], headerData[13], headerData[14], headerData[15]}
	destIP := [4]byte{headerData[16], headerData[17], headerData[18], headerData[19]}
	payloadLength := int(totalLength) - int(ihl*4)

	return &IPHeader{
		Version:       version,
		IHL:           ihl,
		TotalLength:   totalLength,
		Protocol:      protocol,
		SourceIP:      sourceIP,
		DestinationIP: destIP,
		PayloadLength: payloadLength,
	}, nil
}
