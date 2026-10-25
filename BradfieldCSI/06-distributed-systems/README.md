# Distributed Systems

## Overview and Curriculum Progression

This module covers distributed systems fundamentals: replication, consistency models, failure detectors, consensus algorithms (Raft), and fault-tolerant state machine replication.

## Exercise Breakdown

### 1. Replicated Key-Value Store (`01-distributed-kvstore`)
- Directory: [01-distributed-kvstore/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/06-distributed-systems/01-distributed-kvstore)
- Concepts:
  - Client-server RPC architecture.
  - Replication across multiple cluster nodes.
  - Quorum reads and writes.
  - Leader election, term numbers, and log replication using the Raft consensus protocol.
  - Network partition tolerance and split-brain prevention.
