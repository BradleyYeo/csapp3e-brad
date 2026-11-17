# Exercise 58: Persistence — VSFS Bitmaps, Inodes, Directories, Checksums, and WAL Recovery

Bridges low-level C bitmasks, `memcpy` serialization, and invariant checks to OSTEP Part III (Chapters 39–44) and `xv6/kernel/{fs,log}.c`.

---

# On-Disk Allocation Bitmaps and `bmap` (OSTEP Ch. 40)

```
VsfsInode:
  direct[0..3]  -----> data block (direct access, 0 extra disk reads)
  indirect      -----> [ b0 | b1 | ... | b15 ] -----> data blocks 4..19
```

- Every bit `i` in `data_bitmap` sits at byte `i / 8` and bit offset `i % 8`.
- Storing raw `int32_t` block numbers inside `uint8_t blocks[][]` and copying them via `memcpy` avoids strict-aliasing undefined behavior when serializing disk metadata.

---

# Directory Entries (`dirlookup` / `dirlink`)

- Directories map human-readable names (`name[14]`) to inode numbers (`inum`, where `0` marks a free slot).
- `vsfs_dir_link` enforces uniqueness (no duplicate names in one directory) and bounds (`strlen(name) < sizeof(entries[0].name)`).

---

# Data Integrity and Write-Ahead Logging (OSTEP Ch. 42 & 44)

- **Fletcher-16**: Accumulates two running modulo-255 sums (`sum1`, `sum2`) so even transposed or single-bit-flipped bytes yield a mismatch.
- **WAL Recovery (`wal_recover`)**: During crash recovery, only transactions that have valid `WAL_MAGIC`, `committed == true`, a valid `target_block`, and a matching payload checksum are replayed onto home disk blocks.

---

# Active Recall Questions
- Why must indirect blocks initialize all `VSFS_NINDIRECT` entries to `-1` rather than `0` upon allocation?
- Why does `vsfs_bmap` copy indirect block entries with `memcpy` instead of casting `(int32_t *)disk->blocks[ino->indirect]`?
- Why does `wal_recover` refuse to replay a transaction whose `committed` flag is `true` if its Fletcher-16 checksum mismatches?
