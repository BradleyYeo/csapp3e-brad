package main

import (
	"errors"
	"fmt"
	"io"
	"os"
)

func main() {
	file, err := os.Open("/Users/omarflores/Documents/BradfieldCSI/omarfq/2_computer_networks/1_layers_of_protocols/net.cap")
	if err != nil {
		fmt.Println("Error opening file:", err)
		return
	}
	defer file.Close()

	err = skipGlobalHeader(file)
	if err != nil {
		fmt.Println("Error skipping global header:", err)
		return
	}

	packetCount := 0

	for {
		pHeader, err := readPacketHeader(file)
		if errors.Is(err, io.EOF) {
			break
		}
		if err != nil {
			fmt.Println("Error reading Packet header:", err)
			return
		}

		eHeader, err := readEthernetHeader(file)
		if err != nil {
			fmt.Println("Error reading Ethernet header:", err)
			return
		}

		ipHeader, err := readIPHeader(file)
		if err != nil {
			fmt.Println("Error reading IP header:", err)
			return
		}

		var tcpHeader *TCPHeader

		if ipHeader.Protocol == 6 { // 6 indicates TCP
			tcpHeaderRes, tcpData, err := parseTCPHeader(file)
			tcpHeader = tcpHeaderRes
			if err != nil {
				fmt.Println("Error parsing TCP header:", err)
				return
			}
			fmt.Println("Processing TCP packet...")
			httpData := tcpData[tcpHeader.HeaderLength:]
			addTCPData(tcpHeader.SequenceNumber, httpData)
		}

		printPacketInfo(pHeader, eHeader, ipHeader, tcpHeader)

		// Skip remaining packet data
		remainingData := int64(pHeader.CapturedLen) - EthernetHeaderSize - int64(ipHeader.IHL*4) - int64(tcpHeader.HeaderLength)

		_, err = file.Seek(remainingData, 1)
		if err != nil {
			fmt.Println("Error while skipping remaining data:", err)
			return
		}
		packetCount += 1
	}

	fmt.Printf("DataList length before combining: %d\n", len(dataList))
	combinedData := getCombinedTCPData()

	extractAndSaveHTTP(combinedData)

	fmt.Printf("Number of packets: %d\n", packetCount)
}

func printPacketInfo(pHeader *PacketHeader, eHeader *EthernetHeader, ipHeader *IPHeader, tcpHeader *TCPHeader) {
	// Packet Header Info
	fmt.Printf("Captured Length: %d, Original Length: %d\n", pHeader.CapturedLen, pHeader.OriginalLen)

	// Ethernet Header Info
	fmt.Printf("Destination MAC: %x:%x:%x:%x:%x:%x\n", eHeader.DestMAC[0], eHeader.DestMAC[1], eHeader.DestMAC[2], eHeader.DestMAC[3], eHeader.DestMAC[4], eHeader.DestMAC[5])
	fmt.Printf("Source MAC: %x:%x:%x:%x:%x:%x\n", eHeader.SourceMAC[0], eHeader.SourceMAC[1], eHeader.SourceMAC[2], eHeader.SourceMAC[3], eHeader.SourceMAC[4], eHeader.SourceMAC[5])
	switch eHeader.EtherType {
	case IPv4EtherType:
		fmt.Println("Payload is IPv4")
	case IPv6EtherType:
		fmt.Println("Payload is IPv6")
	default:
		fmt.Printf("Unknown EtherType: 0x%x\n", eHeader.EtherType)
	}

	// IP Header info
	fmt.Printf("Source IP: %v.%v.%v.%v\n", ipHeader.SourceIP[0], ipHeader.SourceIP[1], ipHeader.SourceIP[2], ipHeader.SourceIP[3])
	fmt.Printf("Destination IP: %v.%v.%v.%v\n", ipHeader.DestinationIP[0], ipHeader.DestinationIP[1], ipHeader.DestinationIP[2], ipHeader.DestinationIP[3])
	fmt.Printf("Protocol: %v\n", ipHeader.Protocol)
	// Compute payload length by subtracting header length from total length
	payloadLength := ipHeader.TotalLength - (uint16(ipHeader.IHL) * 4)
	fmt.Printf("Payload Length: %d bytes\n", payloadLength)

	// TCP Header Info
	if tcpHeader != nil {
		fmt.Printf("Source Port: %d\n", tcpHeader.SourcePort)
		fmt.Printf("Destination Port: %d\n", tcpHeader.DestinationPort)
		fmt.Printf("Sequence Number: %d\n", tcpHeader.SequenceNumber)
	}

	fmt.Println("========================")
}
