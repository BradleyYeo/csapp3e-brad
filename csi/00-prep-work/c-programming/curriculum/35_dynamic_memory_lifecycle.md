# Exercise 35: Dynamic Memory Lifecycle and Safe Reallocation (C23)

Teaches ISO C23 dynamic storage fundamentals from first principles, then applies them to safe reallocation, lifetime ownership, and deep cloning in [35_dynamic_memory_lifecycle.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/00-prep-work/c-programming/curriculum/35_dynamic_memory_lifecycle.c) and `malloc_clone`.

---

# Fundamentals: ISO C23 Dynamic Storage Duration

## Storage Duration: Stack vs Heap (C23 §6.2.4)
- **Automatic storage duration (stack):** Objects live only until execution exits their enclosing block; returning a pointer to a local array creates a dangling pointer immediately.
- **Allocated storage duration (heap):** Memory returned by `malloc()`, `calloc()`, or `realloc()` persists across function calls until your code explicitly passes it to `free()` or a relocating `realloc()`.
- **Alignment guarantee:** Standard C23 allocators return pointers aligned to at least `alignof(max_align_t)`, making them safe for any fundamental object type.

## C23 Allocator Contracts (`<stdlib.h>`, §7.24.3)
- `malloc(size)`: Allocates `size` uninitialised bytes on the heap; reading those bytes before writing them is undefined behaviour.
- `calloc(nmemb, size)`: Computes `nmemb * size` with built-in overflow checking (returning `nullptr` if it overflows) and zero-initialises every byte.
- `realloc(ptr, new_size)`: Resizes an existing allocation by either extending in place or allocating a fresh block, copying `min(old_size, new_size)` bytes, and freeing `ptr`.
- `free(ptr)`: Releases allocated storage back to the runtime; calling `free(nullptr)` is a guaranteed safe no-op (§7.24.3.3).
- **New in C23 (`free_sized` and `memset_explicit`):**
  - `free_sized(ptr, size)` (§7.24.3.4) and `free_aligned_sized(ptr, align, size)` (§7.24.3.5) pass the allocation size back to the allocator so size-class allocators (such as `tcmalloc` in `malloc_clone`) can free in $O(1)$ without metadata lookups.
  - `memset_explicit(dst, val, n)` (§7.26.6.2) wipes sensitive buffers before `free()` without the compiler deleting the store as dead code.

## Critical C23 Rule: `realloc(ptr, 0)` Is Undefined Behaviour (§7.24.3.7)
- Pre-C23 standards left `realloc(non_null_ptr, 0)` implementation-defined (some libraries freed `ptr` and returned `NULL`, while others returned a minimal allocation).
- ISO C23 §7.24.3.7 explicitly classifies calling `realloc(non_null_ptr, 0)` as **undefined behaviour**.
- Never pass `0` as the new size to `realloc`; either call `free()` explicitly when empty, or clamp the requested capacity to at least `1` byte (required in Drill 2 `buffer_shrink_to_fit`).

## Pointer Lifetime, Indeterminate Pointers, and C23 `nullptr`
- After `free(p)` or a successful `realloc(p, n)`, the old pointer value `p` becomes **indeterminate** (C23 §6.2.4p2); even comparing or reading `p` without overwriting it invokes undefined behaviour.
- **`nullptr` (`nullptr_t`, C23 §6.4.1):** Converts only to pointers or `bool`, never to integers like `0`, catching null-vs-zero type bugs at compile time.
- **Empty-brace zero-initialisation (`{}`, C23 §6.7.10):** Writing `Buffer b = {};` zeroes every member and padding byte cleanly (`{0}` stays valid for older compiler fallbacks).
- **No cast on `malloc` / `realloc`:** In C (unlike C++), `void *` converts implicitly to any object pointer, so writing `uint8_t *p = malloc(n);` is cleaner and avoids hiding a missing `<stdlib.h>` include.

## Unsigned Wraparound and `memcpy` Preconditions in C23
- `size_t` arithmetic wraps modulo $2^{64}$ (C23 §6.2.5p9); doubling `cap * 2` when `cap > SIZE_MAX / 2` silently wraps to a tiny value instead of failing safely.
- Under ISO C23 §7.26.1p2, passing `nullptr` to `memcpy(dst, src, 0)` is still **undefined behaviour** even when the byte count is `0`; guard pointers and check `len > 0` before calling `memcpy`.

---

# Concept 1: Explicit Ownership and Representation Invariants (`Buffer`)

## Separating `size` from `capacity`
- `size`: the count of valid payload bytes currently stored in the buffer.
- `capacity`: the total heap bytes owned before another reallocation is needed.
- **Representation invariant:** `0 <= size <= capacity`, and whenever `capacity > 0`, `data != nullptr` points to an owned heap block of `capacity` bytes.

## Defining Dangling Pointers Out of Existence (Jimmy Koppel / John Ousterhout)
- Calling `free(raw)` releases heap memory, but `raw` still holds the stale address (a dangling pointer) that causes silent Use-After-Free or Double-Free corruption if reused.
- Clearing the pointer to `nullptr` and zeroing its metadata right after `free()` ensures that a duplicate destroy hits `free(nullptr)` (a safe no-op) and an accidental dereference faults deterministically at `0x0`.

```c
free(raw);       // Release the heap allocation back to the runtime
raw = nullptr;   // Invalidate the handle so no dangling address survives
used = 0;        // Reset live byte count to match empty ownership
limit = 0;       // Reset owned capacity to maintain 0 <= used <= limit
```

---

# Concept 2: The Safe `realloc` Idiom and Geometric Growth (Drill 1)

## Why Direct Assignment Leaks Memory
- Writing `slot = realloc(slot, next_cap);` destroys your only handle to the existing heap block whenever `realloc` fails and returns `nullptr`.
- Because `realloc` leaves the original block untouched on failure, overwriting `slot` creates an unrecoverable memory leak and corrupts state.

## The Temporary-Pointer Commit Pattern
- Compute `next_cap` safely, store the `realloc` result in a temporary pointer `grown`, check `grown == nullptr`, and commit `slot = grown` only after success.

```c
if (limit > SIZE_MAX / 2) return ALLOC_OUT_OF_MEMORY; // Guard size_t wraparound
size_t next_cap = (limit == 0) ? 1 : limit * 2;       // Double capacity geometrically
uint8_t *grown = realloc(slot, next_cap);             // Keep slot intact on failure
if (grown == nullptr) return ALLOC_OUT_OF_MEMORY;     // Original data remains valid
```

## Amortised $O(1)$ Growth and Relocation Visualisation
- Doubling capacity ensures that $N$ appends copy $O(N)$ bytes in total ($1 + 2 + 4 + \dots + N/2 < N$), avoiding $O(N^2)$ behaviour.

```text
State 1: Buffer full (size == capacity == 2)
  Heap (0x2000): [ 0xAA ][ 0xBB ]
  buf.data: 0x2000, buf.size: 2, buf.capacity: 2

State 2: realloc(0x2000, 4) into temporary pointer
  Case A (In-place expansion):
    Heap (0x2000): [ 0xAA ][ 0xBB ][ uninit ][ uninit ]  (temp == 0x2000)

  Case B (Relocation to new heap block):
    Heap (0x2000): [ freed ]
    Heap (0x8000): [ 0xAA ][ 0xBB ][ uninit ][ uninit ]  (temp == 0x8000)

  Case C (Failure: temp == nullptr):
    Heap (0x2000): [ 0xAA ][ 0xBB ]                      (buf.data untouched!)

State 3: Commit temp, write 0xCC, increment size
    Heap (0x8000): [ 0xAA ][ 0xBB ][ 0xCC ][ uninit ]
    buf.size = 3, buf.capacity = 4
```

---

# Concept 3: Shrinking Safely Without C23 `realloc(ptr, 0)` UB (Drill 2)

## Preserving the Non-Null Invariant on Empty Buffers
- If `size == capacity`, the buffer is already tight; return `ALLOC_SUCCESS` without calling `realloc`.
- If `size == 0`, set the target size to `1` byte (`size_t target = (buf->size == 0) ? 1 : buf->size;`) so C23 `realloc(ptr, 0)` undefined behaviour never occurs and `buf->data` remains a valid owned pointer.
- Set `buf->capacity = target` (not `0` when `size == 0`!) so the struct accurately reflects the 1-byte allocation.
- Keep the temporary-pointer check on shrink too: a custom or instrumented allocator can fail even on shrink, and failing must preserve the existing buffer.

---

# Concept 4: Deep Copy vs Shallow Copy (Drill 3)

## Why Struct Assignment Causes Double-Free Bugs
- Struct assignment (`b2 = b1`) copies the pointer value `data`, leaving two structs owning the exact same heap address.

```text
Shallow copy (b2 = b1):                    Deep copy (independent allocation):
  b1.data ──┐                                b1.data ──> Heap (0x2000): [AA][BB]
            ▼
       Heap (0x2000): [AA][BB]               b2.data ──> Heap (0x9000): [AA][BB]
            ▲
  b2.data ──┘  --> destroy(&b1) frees;       Each struct frees its own block safely!
                   destroy(&b2) DOUBLE-FREE!
```

## Clean Rollback on Allocation Failure
- Allocate a separate block sized to `src->capacity`, check for `nullptr` (zeroing `dest->size` and `dest->capacity` on failure so `dest` never stays half-initialised), and copy `src->size` bytes via `memcpy` only when `src->size > 0`.

---

# Pitfalls to Watch For in `35_dynamic_memory_lifecycle.c`

- **Invalid `initial_cap == 0` in `buffer_create`:** Return `ALLOC_INVALID_ARG` when `initial_cap == 0` or `buf == nullptr` (verified on line 136).
- **Forgotten capacity sync on empty shrink:** When `buffer_shrink_to_fit` clamps `size == 0` to `1` byte, update `buf->capacity = 1` so future `buffer_append` calls do not write out of bounds.
- **Passing `0` to `memcpy` or `realloc`:** Guard `src->size > 0` before `memcpy` and clamp `target >= 1` before `realloc` to obey strict C23 + UBSan checks.
- **Verifying in WSL:** Run both sanitizer and Valgrind lanes:
  - `make one T=35_dynamic_memory_lifecycle`
  - `valgrind --leak-check=full --show-leak-kinds=all ./35_dynamic_memory_lifecycle`

---

# Active Recall Questions

- What changed in ISO C23 §7.24.3.7 regarding `realloc(non_null_ptr, 0)`, and how does `buffer_shrink_to_fit` avoid it?
- What happens to the original heap block if `p = realloc(p, SIZE_MAX)` returns `nullptr`, and why is `p` unrecoverable afterward?
- Why is reading a pointer variable `p` *after* `free(p)` undefined behaviour in C23 even if you do not dereference `*p`?
- How does `calloc(n, sizeof(T))` protect against integer overflow in a way that `malloc(n * sizeof(T))` does not?
- Why must you still guard `len > 0` before calling `memcpy(dst, src, len)` under C23 §7.26.1p2?
- Why does `buffer_append` check `buf->capacity > SIZE_MAX / 2` before computing `buf->capacity * 2`?
- What failure occurs if two `Buffer` instances share one `data` pointer via shallow copy when both are destroyed?
- How does C23's `free_sized(ptr, size)` enable $O(1)$ deallocation in size-class allocators like `malloc_clone`?

---

# Next Step in Curriculum
Proceed to [36_memory_safety_sanitizers.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/00-prep-work/c-programming/curriculum/36_memory_safety_sanitizers.md), [38_dangling_refs_and_leaks.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/00-prep-work/c-programming/curriculum/38_dangling_refs_and_leaks.md), and [39_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/00-prep-work/c-programming/curriculum/39_bump_allocator.md).
