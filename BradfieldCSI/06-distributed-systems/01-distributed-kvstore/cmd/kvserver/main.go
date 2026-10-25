package main

import (
	"fmt"
	"net"
	"os"

	pb "github.com/omarfq/kvstore/api/v1"
	"github.com/omarfq/kvstore/internal/store"
	"google.golang.org/protobuf/proto"
)

const SOCKET_PATH = "/tmp/kvstore.sock"
const PATH = "data/kvstore.json"

func main() {
	if err := os.Remove(SOCKET_PATH); err != nil && !os.IsNotExist(err) {
		panic(err)
	}

	listener, err := net.Listen("unix", SOCKET_PATH)
	if err != nil {
		panic(err)
	}
	defer listener.Close()

	fmt.Println("Server running...")

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Printf("Failed to accept connection: %s\n", err)
			continue
		}
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()
	readData(conn)
}

func readData(conn net.Conn) {
	buf := make([]byte, 512)
	command := &pb.Command{}
	n, err := conn.Read(buf)
	if err != nil {
		fmt.Printf("Error reading data: %s\n", err)
		return
	}

	err = proto.Unmarshal(buf[:n], command)
	if err != nil {
		fmt.Printf("Error unmarshalling the incoming data buffer: %s\n", err)
		return
	}

	response, err := processInstruction(command.Operation, command.Key, command.Value)
	if err != nil {
		fmt.Fprintf(conn, "Error: %s\n", err)
		return
	}
	fmt.Fprintf(conn, "%s\n", response)
}

func processInstruction(operation, key, value string) (string, error) {
	kvstore, err := store.FileKVStoreInstance()
	data := &pb.Data{
		Key:   key,
		Value: value,
	}

	if err != nil {
		return "", fmt.Errorf("Unable to instantiate the FileKVStoreInstance: %s", err)
	}

	switch operation {
	case "set":
		if err := kvstore.Set(data); err != nil {
			return "", fmt.Errorf("Could not write to file: %s", err)
		}
		return fmt.Sprint("OK\n"), nil
	case "get":
		val, err := kvstore.Get(data)
		if err != nil {
			return "", fmt.Errorf("Could not read from file: %s", err)
		}
		return val, nil
	default:
		return "", fmt.Errorf("Unknown instruction")
	}
}
