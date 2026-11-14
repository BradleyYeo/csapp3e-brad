# Database Systems

## Overview and Curriculum Progression

This module covers the core architectural layers of relational database management systems (RDBMS): physical page storage, Volcano query execution, cost-based query optimization, and concurrency control.

```
01-query-executor ──────► 02-physical-storage ──────► 03-query-optimization ──────► 04-concurrency-lock-manager
(Volcano iterator model)   (Slotted pages, rows)      (Indexes, join reordering)     (Strict 2PL, deadlock detection)
```

## Exercise Breakdown

### 1. Query Execution (`01-query-executor`)
- Directory: [01-query-executor/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/01-query-executor)
- Concepts: Volcano iterator execution model (`Init()`, `Next()`, `Close()`), sequential scan, filter predicates, hash join, sort-merge join, streaming execution.

### 2. Physical Storage (`02-physical-storage`)
- Directory: [02-physical-storage/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/02-physical-storage)
- Concepts: Slotted-page architecture, fixed 4KB/8KB page layout, slot directory growing downward, row data growing upward, record serialization and variable-length strings.

### 3. Query Optimization (`03-query-optimization`)
- Directory: [03-query-optimization/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/03-query-optimization)
- Concepts: Relational algebra equivalences, index scan vs. sequential scan tradeoffs, predicate pushdown, join order optimization, analyzing `EXPLAIN` query plans.

### 4. Concurrency Control and Locking (`04-concurrency-lock-manager`)
- Directory: [04-concurrency-lock-manager/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/04-database-systems/04-concurrency-lock-manager)
- Concepts: ACID transaction properties, serializability, Strict Two-Phase Locking (SS2PL), Shared (S) vs. Exclusive (X) locks, lock escalation, deadlock detection via wait-for graphs.
