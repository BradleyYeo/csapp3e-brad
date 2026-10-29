# Modular Packet Parser

A modular network protocol decoder demonstrating protocol encapsulation across OSI Layers 2, 3, 4, and 7.

## Architecture

Unlike monolithic packet parsers that inspect byte offsets within a single procedural function, this implementation separates each networking layer into dedicated parsing units:
- [constants.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/constants.go): Protocol IDs (EtherType, IP protocol numbers, standard ports).
- [packet.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/packet.go): PCAP global and per-packet header decoding.
- [ethernet.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/ethernet.go): Data link layer (Ethernet II framing, MAC source/destination, EtherType dispatch).
- [ip.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/ip.go): Network layer (IPv4 header, IHL, Total Length, Protocol field dispatch).
- [tcp.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/tcp.go): Transport layer (TCP ports, sequence/acknowledgement numbers, data offset calculation).
- [http.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/http.go): Application layer (HTTP request/response extraction and payload reassembly).
- [main.go](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/main.go): Pipeline driver processing [net.cap](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf/modular-parser/net.cap).

## Execution

### Run Parser
- Run directly using the Go toolchain:
  ```bash
  go run .
  ```
