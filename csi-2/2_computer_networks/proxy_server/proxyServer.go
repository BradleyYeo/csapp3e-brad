package main

import (
	"fmt"
	"net"
)

func main() {
	address := "localhost:8080"
	ln, err := net.Listen("tcp", address)

	if err != nil {
		panic(err)
	}

	defer ln.Close()

	host, port, err := net.SplitHostPort(ln.Addr().String())
	if err != nil {
		panic(err)
	}
	fmt.Printf("Listening on host: %s, port: %s\n", host, port)

	for {
		conn, err := ln.Accept()
		if err != nil {
			panic(err)
		}
		go func(conn net.Conn) {
			buf := make([]byte, 1024)

			for {
				len, err := conn.Read(buf)
				if err != nil {
					fmt.Printf("Error reading: %#v\n", err)
					break
				}

				// Need to check but conn.Read() returns index
				// at which the data starts on the TCP connection
				// buffer
				s := string(buf[:len])
				fmt.Printf("Message received: %s\n", s)

				conn.Write([]byte("Message received.\n"))
			}

			conn.Close()
		}(conn)
	}
}
