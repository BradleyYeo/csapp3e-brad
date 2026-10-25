package main

import (
	"bytes"
	"fmt"
	"os"
	"sort"
)

type TCPData struct {
	SeqNumber uint32
	Data      []byte
}

// List to store all the TCP data
var dataList []TCPData

// Adds TCP data to the dataList
func addTCPData(seqNum uint32, data []byte) {
	dataList = append(dataList, TCPData{SeqNumber: seqNum, Data: data})
	fmt.Println("Added TCP data. Total packets:", len(dataList))
	fmt.Printf("DataList length after adding: %d\n", len(dataList))
}

// Sorts and combines all TCP data
func getCombinedTCPData() []byte {
	sort.Slice(dataList, func(i, j int) bool {
		return dataList[i].SeqNumber < dataList[j].SeqNumber
	})

	var combinedData bytes.Buffer
	for _, data := range dataList {
		combinedData.Write(data.Data)
	}
	return combinedData.Bytes()
}

// Extracts and prints the HTTP header and saves the HTTP body as a .jpg file
func extractAndSaveHTTP(combinedData []byte) {
	// Splitting combined data into header and body
	fmt.Println(string(combinedData[:500]))
	httpParts := bytes.SplitN(combinedData, []byte("\r\n\r\n"), 2)
	if len(httpParts) != 2 {
		fmt.Println("Error: Could not separate HTTP header and body.")
		return
	}

	httpHeader := httpParts[0]
	httpBody := httpParts[1]

	// Printing the HTTP header
	fmt.Println("HTTP Header:")
	fmt.Println(string(httpHeader))
	fmt.Println("========================")

	// Writing the HTTP body to a .jpg file
	err := os.WriteFile("output.jpg", httpBody, 0644)
	if err != nil {
		fmt.Println("Error writing to output.jpg:", err)
		return
	}
	fmt.Println("HTTP body written to output.jpg")
}
