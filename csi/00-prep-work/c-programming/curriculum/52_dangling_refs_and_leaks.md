# Exercise 52: Identifying, Fixing, and Preventing Dangling References & Memory Leaks

A hands-on workshop bridging heap fundamentals (`malloc`, `realloc`, `free`) to production C memory safety: diagnosing defects with ASan and leak detectors, repairing broken ownership patterns, and preventing them by design.

---

# 1. Identify: Taxonomy of Dangling References & Memory Leaks

## Dangling References (Pointer Outlives Target Storage)
- **Stack Frame Return**: Returning the address of a local array/variable. Once the function returns, the stack frame is popped and reused by subsequent calls.
- **Realloc Relocation Alias**: Holding an interior pointer `int *p = &vec->data[i]` across `realloc`. If `realloc` moves the buffer, the old block is freed and `p` becomes a dangling pointer.
- **Un-nulled Freed Pointer (UAF)**: Calling `free(p)` leaves `p` holding the freed address; dereferencing it reads/writes freed heap metadata or recycled objects.
- **Shallow-Copy Aliasing**: Copying a struct containing raw pointers by value (`Pair b = a;`) creates two owners for one heap block. Freeing `a` turns `b`'s pointers into dangling pointers (and freeing `b` triggers a double free).

```
Before realloc:
  vec.data (0x1000) --> [ 10 ][ 20 ]
                           ^
  alias -------------------+ (valid)

After realloc moves block to 0x5000:
  0x1000: [ FREED ]  <-- alias STILL points here (Dangling Reference!)
  0x5000: [ 10 ][ 20 ][ uninit ][ uninit ]  <-- vec.data
```

## Memory Leaks (Heap Block Loses Path to `free`)
- **Overwrite Leak**: Reassigning an owning pointer (`p = malloc(64)` or `p = realloc(p, sz)` when `realloc` returns `NULL`) drops the only address of the old block.
- **Early-Return / Partial-Init Leak**: Allocating resource A, then failing while allocating resource B and returning early without unwinding A.
- **Indirect / Shallow-Free Leak**: Freeing an outer container (`free(pair)`) without freeing heap pointers stored inside it (`pair->first`, `pair->second`), or dropping the head of a linked list.

## Diagnostic Commands
- **ASan + UBSan** (catches UAF, double-free, buffer overflows immediately at runtime):
  ```bash
  clang -std=c23 -Wall -Wextra -pedantic -Werror -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -o 52_dangling_refs_and_leaks 52_dangling_refs_and_leaks.c
  ./52_dangling_refs_and_leaks --trigger-dangling-uaf
  ./52_dangling_refs_and_leaks --trigger-dangling-realloc
  ./52_dangling_refs_and_leaks --trigger-dangling-alias
  ```
- **Leak Detection**:
  - macOS:
    ```bash
    leaks --atExit -- ./52_dangling_refs_and_leaks --trigger-leak-overwrite
    leaks --atExit -- ./52_dangling_refs_and_leaks --trigger-leak-early-return
    leaks --atExit -- ./52_dangling_refs_and_leaks --trigger-leak-indirect
    ```
  - Linux / WSL:
    ```bash
    valgrind --leak-check=full --show-leak-kinds=all ./52_dangling_refs_and_leaks --trigger-leak-indirect
    ```

---

# 2. Fix: Repair Patterns in C

## Pattern A: Heap-Allocate or Caller-Allocate Instead of Returning Stack Addresses
- Returning `char buf[64]` fails because automatic storage ends on `return`.
- Either return a caller-owned heap allocation (`malloc(needed + 1)`) or require the caller to pass `(char *out, size_t cap)`.

## Pattern B: Rebase by Index Offset Across `realloc`
- Indices (`size_t idx`) survive relocation; raw addresses (`int *`) do not.
- Record `idx` before `realloc` (into a temporary pointer), then recompute `vec->data + idx` afterward.

## Pattern C: Reverse-Order Unwind (`goto cleanup`) on Partial Init
- Initialize all owned pointer fields to `nullptr` immediately upon allocating the parent struct.
- On any failure, jump to one cleanup label (or call the struct's destructor) so partially constructed objects are always torn down cleanly.

## Pattern D: Allocate-Before-Free on Replacement
- When replacing `*slot` with a new heap string, allocate `copy` first. Only after `copy != nullptr` succeeds do you `free(*slot)` and assign `*slot = copy`.
- Failure leaves the original value valid and leak-free.

## Pattern E: Save `next` Before Freeing Linked Nodes
- Reading `curr->next` after `free(curr)` is Use-After-Free.
- Always store `ListNode *next = curr->next;` before freeing `curr`'s payload and `curr`.

---

# 3. Prevent: Architectural Ownership Invariants

## Jimmy Koppel — Define Dangling Pointers Out of Existence
- Pair every `free(p)` with `p = nullptr` via `SAFE_FREE(p)` or a `T **` destructor (`fixed_pair_destroy(&p)`).
- Avoid casting `(void **)&typed_ptr` because dereferencing `void **` on an `int *` object violates C strict-aliasing rules; prefer `SAFE_FREE(p)` or typed `T **` destructors.

## Single Exclusive Owner + Move Semantics
- Never duplicate a struct holding owning pointers with `dst = src`.
- Transfer ownership explicitly (`owned_vec_move(&dst, &src)`): clean up `dst`'s prior buffer, copy fields, and zero `src` so only one handle can ever `free` the memory.

## Daniel Jackson — Separate Owned Storage from Borrowed Views
- `IntVec` owns the heap block and controls `realloc`/`free`.
- `VecView` (`const int *items; size_t len;`) is a short-lived, read-only cursor that borrows access without managing lifetime. Recreate views after any mutating operation.

---

# 4. Active Recall Checklist
- Why does `SAFE_FREE(p)` prevent double-free, and why does `free(nullptr)` work?
- Why is `(void **)&my_int_ptr` a strict-aliasing hazard compared to `SAFE_FREE(my_int_ptr)`?
- When replacing an owned string `*slot`, why must `malloc`/`strdup` happen *before* `free(*slot)`?
- Why does saving `next` before `free(curr)` matter in linked-list destruction even if it "seems to work" without ASan?
