# Paper Practice: Manual Address Decoding & Cache Tracing

## Objective

Before writing C simulation code, you must be able to trace cache lookups on paper without a computer. Completing these paper drills ensures you can spot address decoding and LRU bugs in your simulator immediately.

---

# Drill 1: Manual Bit Slicing

## Cache Geometry
- Address width: $m = 64$ bits
- Set index bits: $s = 4 \implies S = 16$ sets (Sets `0x0` through `0xF`)
- Associativity: $E = 2$ lines per set
- Block offset bits: $b = 5 \implies B = 32$ bytes per cache line
- Tag bits: $t = 64 - (4 + 5) = 55$ bits

## Addresses to Decode
For each 64-bit hexadecimal address, determine the Block Offset (hex), Set Index (hex), and Tag (hex):
- `0x0000000000000000`
- `0x000000000000001F`
- `0x0000000000000020`
- `0x0000000000000400`
- `0x0000000000001428`

---

# Drill 2: Trace `yi.trace` on Direct-Mapped Cache

## Cache Geometry
- $s = 4 \implies S = 16$ sets
- $E = 1 \implies$ Direct-mapped
- $b = 4 \implies B = 16$ bytes per line
- Address interpretation: For hex `0x...TSO`, `O` is 4-bit offset, `S` is 4-bit set index, `T` is tag.

## Trace Operations
Fill out the table row by row on paper:

| Step | Operation | Address | Tag | Set | Hit or Miss? | Eviction? | Cache State Changes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | `L 10,1` | `0x010` | | | | | |
| 2 | `M 20,1` (Load) | `0x020` | | | | | |
| 3 | `M 20,1` (Store)| `0x020` | | | | | |
| 4 | `L 22,1` | `0x022` | | | | | |
| 5 | `S 18,1` | `0x018` | | | | | |
| 6 | `L 110,1` | `0x110` | | | | | |
| 7 | `L 210,1` | `0x210` | | | | | |
| 8 | `M 12,1` (Load) | `0x012` | | | | | |
| 9 | `M 12,1` (Store)| `0x012` | | | | | |

Record your final totals:
- Hits:
- Misses:
- Evictions:

---

# Drill 3: 2-Way Set-Associative LRU Replacement

## Cache Geometry
- $s = 2 \implies S = 4$ sets (Sets `0` through `3`)
- $E = 2 \implies 2$ lines per set (`Way 0`, `Way 1`)
- $b = 4 \implies B = 16$ bytes per line
- Set index mask: bits 4..5 (`(addr >> 4) & 0x3`)
- Tag: bits 6..63 (`addr >> 6`)

## Sequence of Operations
Trace each access into Set 0:
- Access 1: `L 0x000` (Tag 0, Set 0)
- Access 2: `L 0x040` (Tag 1, Set 0)
- Access 3: `L 0x000` (Tag 0, Set 0)
- Access 4: `L 0x080` (Tag 2, Set 0)
- Access 5: `L 0x000` (Tag 0, Set 0)
- Access 6: `L 0x040` (Tag 1, Set 0)

## Questions to Answer on Paper
- At Access 4, which tag was evicted from Set 0? Why?
- Did Access 5 hit or miss?
- Did Access 6 hit or miss?

---

# Verification and Next Steps

- Check your answers against [reference-solution/02_answers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/reference-solution/02_answers.md).
- Proceed to coding: [03_miss_prediction.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/01-computer-architecture/06-memory-hierarchy-caching/03_miss_prediction.md).
