# Computer Networks

## Overview and Curriculum Progression

This module covers networking from low-level packet bytes up to application-level HTTP proxies and routing protocols.

```
01-packet-capture-ctf ──► 02-dns-client ──► 03-http-reverse-proxy ──► 04-reliable-transport ──► 05-traceroute
(Raw PCAP, Ethernet,      (UDP sockets,     (TCP server, HTTP/1.1,    (Sliding window,          (ICMP, TTL,
 IP, TCP packet parsing)   DNS wire format)  concurrency, caching)     ACKs, retransmissions)    raw sockets)
```

## Exercise Breakdown

### 1. Packet Capture CTF (`01-packet-capture-ctf`)
- Directory: [01-packet-capture-ctf/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/01-packet-capture-ctf)
- Concepts: Libpcap file format, Ethernet II framing (14 bytes), IPv4 header unpacking (IHL, protocol, source/dest IP), TCP segment reassembly (sequence numbers, flags).
- Task: Parse raw packet captures from `net.cap` to reconstruct transmitted payload data.

### 2. DNS Client (`02-dns-client`)
- Directory: [02-dns-client/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/02-dns-client)
- Concepts: UDP socket programming, RFC 1035 wire format, DNS header flags, question encoding, label-length prefixing, response compression pointers.
- Task: Send UDP query to `8.8.8.8:53` and parse IPv4 `A` record answers.

### 3. HTTP Reverse Proxy (`03-http-reverse-proxy`)
- Directory: [03-http-reverse-proxy/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/03-http-reverse-proxy)
- Concepts: TCP server listening, concurrent connection handling, HTTP/1.1 request/response streaming, LRU response caching.
- Task: Build an HTTP reverse proxy that routes requests to upstream servers and caches responses. Verify with `proxy_tester.py`.

### 4. Reliable Transport Protocol (`04-reliable-transport`)
- Directory: [04-reliable-transport/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/04-reliable-transport)
- Concepts: UDP unreliable channel, packet drop simulation, sequence numbering, cumulative ACKs, timeouts, sliding window flow control.
- Task: Implement reliable transport over a lossy proxy (`unreliable_proxy`).

### 5. Traceroute and IP Routing (`05-traceroute`)
- Directory: [05-traceroute/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/02-computer-networks/05-traceroute)
- Concepts: IP Time-To-Live (TTL) field, ICMP `Time Exceeded` (Type 11) replies, raw sockets, round-trip time (RTT) calculation.
- Task: Increment TTL hop-by-hop to discover the router path to remote hosts.
