# Stage 3: Exercise 09 - Dynamic Memory Lifecycle and Safe Reallocation

A foundational guide explaining heap allocation (`malloc`, `calloc`), the safe `realloc` pattern, zeroing dangling pointers, and deep cloning for C beginners.

# Conceptual Foundations and Mental Model

## The Heap Memory Contract
- Unlike stack memory (which is allocated and popped automatically with function frames), heap memory persists until explicitly freed.
- The standard allocation functions:
  - `malloc(size)`: Allocates `size` uninitialized bytes. Memory contains whatever arbitrary garbage was left by previous users.
  - `calloc(count, size)`: Allocates `count * size` bytes and initializes every single byte to zero.
  - `free(ptr)`: Returns the allocated block back to the heap manager. Passing `nullptr` to `free` is a safe no-op.
  - `realloc(ptr, new_size)`: Resizes an existing heap allocation.

## The Fatal realloc Anti-Pattern
- The most common beginner bug in C memory management is:
  ```c
  // DANGEROUS ANTI-PATTERN:
  buf->data = realloc(buf->data, new_cap);
  ```
- Why is this fatal?
  - If the operating system cannot allocate `new_cap` bytes, `realloc` returns `NULL`.
  - The assignment immediately overwrites `buf->data` with `NULL`.
  - The original memory block is NOT freed by `realloc`, but you have now lost the only pointer pointing to it!
  - Result: An unrecoverable memory leak and corrupted buffer state.
- The Safe `realloc` Idiom:
  ```c
  void *temp = realloc(buf->data, new_cap);
  if (temp == nullptr) {
    // Allocation failed! buf->data still points to valid original data.
    return ALLOC_OUT_OF_MEMORY;
  }
  buf->data = temp;
  buf->capacity = new_cap;
  ```

## Defining Dangling Pointers Out of Existence (Jimmy Koppel's Principle)
- When you call `free(p)`, the memory at `p` is deallocated, but variable `p` still holds the old address (a "dangling pointer").
- If subsequent code accidentally dereferences `p`, it creates a Use-After-Free vulnerability.
- By immediately setting `p = nullptr` upon freeing, any subsequent dereference fails with an immediate, deterministic crash at address `0x0` rather than silent memory corruption.

---

# Line-by-Line Code Breakdown

## Structure: Buffer

```c
typedef struct {
  uint8_t *data;
  size_t size;
  size_t capacity;
} Buffer;
```

### Distinction Between Size and Capacity
- `size`: The count of actual payload bytes currently stored in the buffer.
- `capacity`: The total number of bytes allocated on the heap available before a resizing reallocation is necessary.
- Invariant: `0 <= size <= capacity`.

---

## Function: buffer_destroy

```c
static void buffer_destroy(Buffer *buf) {
  if (buf == nullptr) {
    return;
  }

  free(buf->data);
  buf->data = nullptr;
  buf->size = 0;
  buf->capacity = 0;
}
```

### Total Reset
- Frees heap storage.
- Immediately zeroes `buf->data`, `buf->size`, and `buf->capacity`, leaving the struct in a safe, inert state.

---

## Function: buffer_append

```c
static AllocStatus buffer_append(Buffer *buf, uint8_t byte) {
  if (buf == nullptr || buf->data == nullptr) {
    return ALLOC_INVALID_ARG;
  }

  if (buf->size == buf->capacity) {
    size_t new_cap = (buf->capacity == 0) ? 1 : buf->capacity * 2;
    uint8_t *temp = realloc(buf->data, new_cap * sizeof(uint8_t));
    if (temp == nullptr) {
      return ALLOC_OUT_OF_MEMORY;
    }
    buf->data = temp;
    buf->capacity = new_cap;
  }

  buf->data[buf->size] = byte;
  buf->size++;
  return ALLOC_SUCCESS;
}
```

### Amortized O(1) Growth Strategy
- Doubling capacity (`buf->capacity * 2`) ensures that appending $N$ elements takes $O(N)$ total time rather than $O(N^2)$.
- The temporary pointer `temp` protects the original data pointer in case reallocation fails.

---

## Function: buffer_clone

```c
static AllocStatus buffer_clone(Buffer *dest, const Buffer *src) {
  if (dest == nullptr || src == nullptr || src->data == nullptr) {
    return ALLOC_INVALID_ARG;
  }

  dest->data = malloc(src->capacity * sizeof(uint8_t));
  if (dest->data == nullptr) {
    dest->size = 0;
    dest->capacity = 0;
    return ALLOC_OUT_OF_MEMORY;
  }

  if (src->size > 0) {
    memcpy(dest->data, src->data, src->size);
  }
  dest->size = src->size;
  dest->capacity = src->capacity;
  return ALLOC_SUCCESS;
}
```

### Deep Copy vs Shallow Copy
- A shallow copy (`dest = *src`) would copy the pointer address, resulting in two structs referencing the exact same heap memory (leading to double-free bugs upon destruction).
- A deep copy allocates a distinct heap block (`malloc`) and copies the byte contents (`memcpy`), guaranteeing independent lifecycles.

---

# Memory Layout Visualization

Safe buffer reallocation and capacity doubling:

```
State 1: Buffer full (size == capacity == 2)
  Heap (0x2000): [ 0xAA ][ 0xBB ]
  buf.data: 0x2000, buf.size: 2, buf.capacity: 2

State 2: buffer_append triggers realloc(0x2000, 4)
  Case A (In-place expansion):
    Heap (0x2000): [ 0xAA ][ 0xBB ][ uninit ][ uninit ]
    buf.data remains 0x2000

  Case B (Relocation to new heap region):
    Heap (0x2000): [ freed ]
    Heap (0x8000): [ 0xAA ][ 0xBB ][ uninit ][ uninit ]
    buf.data becomes 0x8000

State 3: Writing byte and incrementing size
    Heap (0x8000): [ 0xAA ][ 0xBB ][ 0xCC ][ uninit ]
    buf.size = 3, buf.capacity = 4
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What happens to the original memory block if `p = realloc(p, 1000000000000ULL)` returns `NULL`?
- Question 2: Why is calling `free(p)` twice on the same pointer ("double free") dangerous? How does `p = nullptr` prevent it?
- Question 3: What is the semantic difference between `malloc(n * sizeof(T))` and `calloc(n, sizeof(T))`?

## Drill: Hands-On Code Validation
- Open [09_dynamic_memory_lifecycle.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/09_dynamic_memory_lifecycle.c).
- Implement a buffer truncation helper:
  ```c
  AllocStatus buffer_truncate(Buffer *buf, size_t new_size);
  ```
- Require that `new_size <= buf->size`. Reset unused trailing bytes to 0, and update `buf->size`.
- Add test assertions in `main()` verifying truncation behavior.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 09_dynamic_memory_lifecycle 09_dynamic_memory_lifecycle.c
  ./09_dynamic_memory_lifecycle
  ```

---

# Next Step in Curriculum
Proceed to [11_bump_allocator.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/11_bump_allocator.md) to build a custom monotonic arena allocator with bitwise alignment and bulk reset capabilities.
