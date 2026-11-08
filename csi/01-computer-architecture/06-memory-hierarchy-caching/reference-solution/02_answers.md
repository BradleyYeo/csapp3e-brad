# Reference Answers: Paper Tracing Drills

## Drill 1: Bit Slicing Answers

Cache parameters: $s = 4, E = 2, b = 5$.
Offset: bits 0..4 (mask `0x1F`).
Set index: bits 5..8 (shift 5, mask `0xF`).
Tag: bits 9..63 (shift 9).

| Address (Hex) | Offset (Hex) | Set Index (Hex) | Tag (Hex) | Notes |
| :--- | :--- | :--- | :--- | :--- |
| `0x0000000000000000` | `0x00` | `0x0` | `0x0` | Set 0, byte 0 |
| `0x000000000000001F` | `0x1F` | `0x0` | `0x0` | Set 0, last byte of block 0 |
| `0x0000000000000020` | `0x00` | `0x1` | `0x0` | Set 1, byte 0 (next cache line!) |
| `0x0000000000000400` | `0x00` | `0x0` | `0x2` | Shift 5 gives `0x20`; lower 4 bits are 0 $\implies$ Set 0 |
| `0x0000000000001428` | `0x08` | `0x1` | `0xA` | Binary: `1 0100 0010 1000`. Offset `0x08`, Set `0x1`, Tag `0xA` |

---

## Drill 2: `yi.trace` Step-by-Step Table

Cache parameters: $s = 4, E = 1, b = 4$.
Address format: `0x[Tag][Set][Offset]`.

| Step | Operation | Address | Tag | Set | Hit/Miss | Eviction | Cache State After Step |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 1 | `L 10,1` | `0x010` | `0x0` | `0x1` | **Miss** | No | Set 1: [Valid, Tag 0] |
| 2 | `M 20,1` (L) | `0x020` | `0x0` | `0x2` | **Miss** | No | Set 2: [Valid, Tag 0] |
| 3 | `M 20,1` (S) | `0x020` | `0x0` | `0x2` | **Hit**  | No | Set 2: [Valid, Tag 0, Dirty] |
| 4 | `L 22,1` | `0x022` | `0x0` | `0x2` | **Hit**  | No | Set 2: [Valid, Tag 0] |
| 5 | `S 18,1` | `0x018` | `0x0` | `0x1` | **Hit**  | No | Set 1: [Valid, Tag 0, Dirty] |
| 6 | `L 110,1` | `0x110` | `0x1` | `0x1` | **Miss** | **Yes** | Set 1 replaced with Tag 1 (Dirty WB) |
| 7 | `L 210,1` | `0x210` | `0x2` | `0x1` | **Miss** | **Yes** | Set 1 replaced with Tag 2 |
| 8 | `M 12,1` (L) | `0x012` | `0x0` | `0x1` | **Miss** | **Yes** | Set 1 replaced with Tag 0 |
| 9 | `M 12,1` (S) | `0x012` | `0x0` | `0x1` | **Hit**  | No | Set 1: [Valid, Tag 0, Dirty] |

### Final Totals
- Hits: 4
- Misses: 5
- Evictions: 3

---

## Drill 3: 2-Way Set-Associative LRU Answers

Set 0 has 2 ways: `[Way 0, Way 1]`.

- Access 1 (`L 0x000`, Tag 0): Cold miss. Loaded into Way 0 (clk = 1).
- Access 2 (`L 0x040`, Tag 1): Cold miss. Loaded into Way 1 (clk = 2).
- Access 3 (`L 0x000`, Tag 0): Cache hit on Way 0. Timestamp refreshed (clk = 3). Way 1 now has older timestamp (clk = 2).
- Access 4 (`L 0x080`, Tag 2): Conflict miss. Way 1 (Tag 1, clk = 2) is older than Way 0 (Tag 0, clk = 3).
  - Victim: **Tag 1 is evicted** from Way 1.
  - Way 1 receives Tag 2 (clk = 4).
- Access 5 (`L 0x000`, Tag 0): **Cache Hit**! Tag 0 is still resident in Way 0 (clk = 5).
- Access 6 (`L 0x040`, Tag 1): **Cache Miss**! Tag 1 was evicted in Access 4.
