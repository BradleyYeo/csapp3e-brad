# Exercise 56: Memory Virtualization — Segmentation, Multi-Level Paging, TLB, and Clock

Prepares bit-field slicing, multi-level pointer-free table walks, and eviction state invariants for OSTEP Chapters 15–22 and `xv6/kernel/vm.c`.

---

# Hardware Segmentation with Growth Direction (OSTEP Ch. 16)

```
16-bit Virtual Address:
+---------+------------------------------------+
| seg (2) |            offset (14)             |
+---------+------------------------------------+
 00=Code, 01=Heap (+growth)   10=Stack (-growth)
```

- Positive-growing segments add `offset` to `base` and require `offset < bounds`.
- Negative-growing stacks measure size from the top of the 14-bit segment (`SEG_MAX_SIZE - offset`) and subtract from `base`.

---

# Two-Level Page Tables and Hardware A/D Bits (OSTEP Ch. 18–20)

- A flat linear page table wastes megabytes of contiguous RAM for sparse address spaces; a two-level page directory allocates leaf page tables only for active regions.
- On every successful translation, hardware sets `PTE_A` (Accessed) and, on stores, `PTE_D` (Dirty) so the OS page-replacement daemon knows which pages are hot or modified.

---

# ASID-Tagged TLBs and Clock Replacement (OSTEP Ch. 19–22)

- **ASIDs**: Tagging TLB entries with an Address Space Identifier lets processes share the TLB across context switches without flushing non-global entries.
- **Clock algorithm**: Scans circularly through frames; `ref_bit == true` grants a second chance (`ref_bit = false`), and the first frame found with `ref_bit == false` is evicted.

---

# Active Recall Questions
- Why does a negative-growing stack segment compute `SEG_MAX_SIZE - offset` instead of checking `offset < bounds`?
- What is the difference between `VM_ERR_BOUNDS` (unmapped PDE/PTE) and `VM_ERR_PROT` (mapped, but wrong `R`/`W`/`U` bit) during a page table walk?
- Why must kernel mappings (`PTE_G` / `global = true`) bypass ASID checks and survive `tlb_flush_non_global`?
