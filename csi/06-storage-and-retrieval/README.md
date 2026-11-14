# Data Structures for Storage and Retrieval

## Overview and Curriculum Progression

This module covers storage engine internals modeled after Google's LevelDB and modern Log-Structured Merge-tree (LSM) architectures.

```
01-in-memory-kv ──► 02-linked-list-kv ──► 03-leveldb-lsm (WAL, MemTable, SSTables, Bloom Filters, Compaction)
(Interface def)     (Sorted array)        (Write-Ahead Logging, SkipLists, immutable SSTables, Leveled Compaction)
```

## Exercise Breakdown

### 1. In-Memory Key-Value Store (`01-in-memory-kv`)
- Directory: [01-in-memory-kv/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/01-in-memory-kv)
- Concepts: Key-value storage interface (`Get`, `Put`, `Delete`), iterators, snapshot isolation semantics.

### 2. Sorted Linked-List Key-Value Store (`02-linked-list-kv`)
- Directory: [02-linked-list-kv/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/02-linked-list-kv)
- Concepts: Sorted in-memory structures, ordered range scanning, range iterators.

### 3. LevelDB LSM-Tree Engine (`03-leveldb-lsm`)
- Directory: [03-leveldb-lsm/](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/05-storage-and-retrieval/03-leveldb-lsm)
- Components:
  - `wal/`: Write-Ahead Log providing crash-resilience and durability via append-only disk logging.
  - `skiplist/`: Concurrent lock-free / probabilistic skip list acting as the active in-memory `MemTable`.
  - `sstable/`: Sorted String Table disk format containing data blocks, index blocks, and block footers.
  - `iterator/`: Two-level merging iterators over in-memory MemTables and on-disk SSTables.
  - Leveled Compaction: Merging Level $L$ SSTables into Level $L+1$ to reclaim space and prune tombstones.
  - Bloom Filters: Probabilistic data structures to eliminate disk reads for non-existent keys.
