package main

import (
	"fmt"
	"net"

	"github.com/chzyer/readline"
	pb "github.com/omarfq/kvstore/api/v1"
	"github.com/omarfq/kvstore/internal/utils"
	"google.golang.org/protobuf/proto"
)

const SOCKET_PATH = "/tmp/kvstore.sock"

func main() {
	rl, err := readline.New("Commands: get [key] | set [key]=[value] > ")
	if err != nil {
		panic(err)
	}
	defer rl.Close()

	for {
		line, err := rl.Readline()
		if err != nil {
			fmt.Println("Error reading line:", err)
			break
		}

		operation, key, value, err := utils.ParseInput(line)
		if err != nil {
			fmt.Printf("Failed to parse input: %s", err)
			continue
		}

		// Create protobuf object
		command := &pb.Command{
			Operation: operation,
			Key:       key,
			Value:     value,
		}

		// The out variable is the encoded wire format of `command`
		out, err := proto.Marshal(command)
		if err != nil {
			fmt.Printf("An error occurred while marshalling the input: %s", err)
			continue
		}

		// Connect to the server via UNIX socket
		conn, err := net.Dial("unix", SOCKET_PATH)
		if err != nil {
			fmt.Printf("Failed to connect to server: %s\n", err)
			continue
		}

		// Send the instruction and value to the server
		_, err = conn.Write(out)
		if err != nil {
			fmt.Printf("Failed to send data: %s\n", err)
			conn.Close()
			continue
		}

		// Read response from server
		response := make([]byte, 1024)
		n, err := conn.Read(response)
		if err != nil {
			fmt.Printf("Failed to read response: %s\n", err)
			conn.Close()
			continue
		}

		fmt.Println(string(response[:n]))

		// Close the connection after the interaction
		conn.Close()
	}
}
