# Systems Design

Real-world distributed systems design case studies focusing on capacity estimation, consistency trade-offs, storage partitioning, data flow architectures, and failure domain boundaries.

## Architecture & Concept Design Principles

Following Daniel Jackson's *Concept Design* and John Ousterhout's *A Philosophy of Software Design*:
- **Concept Independence**: Each subsystem (metadata storage, blob ingestion, counter aggregation, booking state machine) must own its invariants without leaking intermediate synchronization primitives across subsystem boundaries.
- **Deep Interfaces**: Modules present simple interfaces (`RecordMetric(name, ts, val)`, `ReserveSlot(listingId, timeRange)`) hiding complex internals such as LSM compaction, distributed write buffers, and optimistic lock managers.
- **Single Authority**: Critical state transitions (such as payment authorization and inventory deduction) must identify exactly one authoritative source of truth to avoid split-brain states under network partitions.

## Curriculum Case Studies

- [YouTube Dislikes & Counter Aggregator](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/youtube_dislikes.md)
  - Focus: High-throughput write-heavy counter pipeline, eventual consistency, lossy aggregation vs persistent accuracy, and sliding-window write buffers.
- [Real-Time Distributed Metrics Aggregation](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/metrics_system.md)
  - Focus: Time-series ingestion pipelines, downsampling retention tiers (raw, minute-rollup, daily-rollup), time-bucketed indexing, and fast range queries.
- [Code Snippet Hosting Platform](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/code_snippet_hosting_platform.md)
  - Focus: Multi-tenant content-addressed storage (CAS), metadata indexing vs blob storage separation, immutable snapshots, and CDN caching topologies.
- [Simple Airbnb Marketplace Engine](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/07-systems-design/simple_airbnb.md)
  - Focus: Double-booking prevention, distributed transactions, 2-phase reservations with expiration leases, geospatial range indexing (S2/H3/Geohash), and search relevance.

## Implementation Guide & Analysis Checklist

### Capacity & Workload Estimation
- Distinguish read-dominated (e.g. Code Snippet viewing, Listing search) from write-dominated (e.g. Telemetry metric points, Live counters) workloads.
- Calculate peak throughput (QPS), bandwidth requirements (ingress/egress), and cumulative storage growth across 1-, 3-, and 5-year retention windows.

### Data Modeling & Storage Selection
- Choose relational storage (PostgreSQL/MySQL) when ACID compliance and strict uniqueness constraints across relational entities are required.
- Choose LSM-tree key-value stores (e.g., RocksDB, Cassandra) when write throughput dominates and append performance is paramount.
- Choose object stores (e.g., S3-compatible) for immutable payload blobs, separating high-velocity metadata indices from bulk raw data.

### Fault Tolerance & High Availability
- Quantify SLOs and SLAs (Availability target, e.g. 99.99%).
- Define clear degradation modes: fail-open for telemetry aggregation vs fail-closed for financial transactions and booking reservations.
