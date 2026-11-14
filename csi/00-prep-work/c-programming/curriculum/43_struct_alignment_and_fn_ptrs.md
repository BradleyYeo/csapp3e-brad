# Exercise 54: Struct Alignment, Intrusive Containers, and Function Pointers

Bridges C memory layout and polymorphism to CS:APP Chapter 3, `bomblab`, `attacklab`, and kernel/allocator codebases (`xv6`, `malloc_clone`).

---

# Struct Alignment and Field Packing

## Alignment Invariants (`CS:APP 3.9.3`)
- A primitive of size $K$ must be stored at an address divisible by $K$ (its `alignof`).
- The alignment of a `struct` equals the maximum `alignof` among its members, and `sizeof(struct)` is padded to a multiple of that alignment so arrays of structs preserve every element's alignment.
- Ordering fields from largest alignment to smallest minimizes wasted internal and tail padding (`BloatedPacket` 24 bytes vs `PackedPacket` 16 bytes for the exact same 15 bytes of payload).

```
BloatedPacket (24 bytes):
[ flags:1 ][ pad:7 ][ seq:8 ][ port:2 ][ pad:2 ][ crc:4 ]

PackedPacket (16 bytes):
[ seq:8 ][ crc:4 ][ port:2 ][ flags:1 ][ pad:1 ]
```

---

# Intrusive Containers (`container_of`)

## Byte-Offset Recovery
- Standard library lists often allocate wrapper nodes holding a `void *data` pointer, doubling allocations and hurting cache locality.
- Intrusive containers embed `IntrusiveLink` directly inside `TaskItem`.
- `offsetof(TaskItem, link)` yields the compile-time byte offset of `link` from the start of `TaskItem`; subtracting it from `(uintptr_t)link` recovers the owning `TaskItem *`.

---

# Safe Generic Comparators and Function-Pointer Dispatch

## Eliminating Comparator Subtraction Overflow
- `return *a - *b;` overflows on extreme values such as `INT32_MIN - 1`, triggering signed overflow UB and returning a positive number instead of negative.
- `(va > vb) - (va < vb)` produces `-1`, `0`, or `+1` with zero overflow risk.

## Jump Tables via Function Pointers
- `alu_op_fn table[]` maps integer opcodes directly to handler functions, mirroring how compilers compile dense `switch` jump tables (`CS:APP 3.6.8`) and kernel syscall tables.
- Generic algorithms (`generic_bsearch_first`) combine `const unsigned char *` byte strides (`size`) with a `generic_cmp_fn` callback to operate polymorphically on any element type.

---

# Active Recall Questions
- Why does reordering fields in `BloatedPacket` shrink `sizeof` from 24 bytes to 16 bytes without changing any member type?
- Why must `(uintptr_t)link - offsetof(TaskItem, link)` operate on `uintptr_t` or `unsigned char *` rather than `IntrusiveLink *`?
- Why does `*a - *b` fail as a `qsort`/`bsearch` comparator for `int32_t`?
