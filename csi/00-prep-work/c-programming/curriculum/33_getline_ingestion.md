# Stage 2: Exercise 03 - Stream Ingestion and Dynamic Line Reading

A foundational guide explaining POSIX `getline(3)`, stream buffering, dynamic heap reallocation in libc, EOF semantics, and $O(1)$ newline trimming for C beginners.

# Conceptual Foundations and Mental Model

## The Danger of fgets and Fixed Buffers
- In traditional C, `fgets(buf, size, stream)` reads at most `size - 1` bytes into a static or stack buffer.
- Problems:
  - If a line exceeds `size`, `fgets` leaves the remainder of the line in the stream, causing subsequent reads to desynchronize.
  - Buffer sizing is an arbitrary guess: either wasting stack memory or truncating lines.

## How POSIX getline(3) Works
- POSIX introduced `getline(char **lineptr, size_t *n, FILE *stream)` to solve buffer sizing once and for all:
  - If `*lineptr == nullptr` and `*n == 0`, `getline` automatically allocates a buffer on the heap using `malloc`.
  - If the line being read exceeds `*n`, `getline` automatically grows the buffer using `realloc` and updates `*n` to the new capacity.
  - Returns `ssize_t`: the exact number of bytes read (including the newline `\n`), or `-1` on EOF or allocation error.
- Crucial invariant: The memory allocated by `getline` belongs to the caller! The caller must explicitly call `free(*lineptr)`.

## Stream Invariants and fmemopen
- The C standard library wraps low-level file descriptors in a `FILE *` structure containing read/write buffers, current file position, and status flags (EOF and error).
- POSIX `fmemopen(buf, size, mode)` creates a `FILE *` stream backed directly by a fixed memory buffer instead of a file on disk. This enables fast, hermetic unit testing of stream functions without touching the disk filesystem.

---

# Line-by-Line Code Breakdown

## Function: read_trimmed_line

```c
static char *read_trimmed_line(FILE *stream, size_t *out_len) {
  char *line = nullptr;
  size_t cap = 0;
  ssize_t read_bytes = getline(&line, &cap, stream);

  if (read_bytes == -1) {
    free(line);
    return nullptr;
  }

  // O(1) trailing newline removal
  if (read_bytes > 0 && line[read_bytes - 1] == '\n') {
    line[read_bytes - 1] = '\0';
    read_bytes--;
  }

  // Also strip carriage return \r if present (e.g. CRLF)
  if (read_bytes > 0 && line[read_bytes - 1] == '\r') {
    line[read_bytes - 1] = '\0';
    read_bytes--;
  }

  if (out_len != nullptr) {
    *out_len = (size_t)read_bytes;
  }

  return line;
}
```

### Initializing getline Parameters: line = nullptr, cap = 0
- Passing `&line` (where `line == nullptr`) and `&cap` (where `cap == 0`) signals to `getline` to allocate its initial heap buffer dynamically.

### Error and EOF Handling: read_bytes == -1
- When `getline` reaches the end of the file (EOF) or encounters an error, it returns `-1`.
- If memory was allocated during a partial read or attempt, `getline` may leave a buffer allocated.
- Calling `free(line)` before returning `nullptr` ensures zero memory leaks on EOF.

### O(1) In-Place Trimming
- Rather than scanning the entire string with `strlen()` (an $O(N)$ operation), `read_bytes` tells us the exact index of the last character: `read_bytes - 1`.
- Direct indexing `line[read_bytes - 1] = '\0'` overwrites `\n` in $O(1)$ time.
- Repeating for `\r` handles Windows-style CRLF line endings transparently.

---

# Memory Layout Visualization

Dynamic buffer growth inside `getline(&line, &cap, stream)`:

```
Step 1: Before call
  line: nullptr
  cap:  0

Step 2: Inside getline, allocating heap buffer
  Heap: [ 'h' ][ 'e' ][ 'l' ][ 'l' ][ 'o' ][ '\n' ][ '\0' ][ ? ][ ? ] ... (120 bytes)
  line: 0x5000 (points to Heap)
  cap:  120

Step 3: After O(1) trimming in read_trimmed_line
  line[5] = '\0'
  Heap: [ 'h' ][ 'e' ][ 'l' ][ 'l' ][ 'o' ][ '\0' ][ '\0' ][ ? ][ ? ] ...
  read_bytes = 5
  Caller receives heap pointer 0x5000 and is responsible for calling free(0x5000).
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the type difference between `size_t` and `ssize_t`, and why does `getline` return `ssize_t`?
- Question 2: Why must you call `free(line)` even when `getline` returns `-1`?
- Question 3: How does `fmemopen` differ from `fopen`, and what makes it ideal for unit testing?

## Drill: Hands-On Code Validation
- Open [03_getline_ingestion.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_getline_ingestion.c).
- Implement a line counter helper:
  ```c
  size_t count_stream_lines(FILE *stream);
  ```
- Use a `while ((line = read_trimmed_line(stream, &len)) != nullptr)` loop, freeing each line in the loop body.
- Verify on `test.txt` that it correctly counts 3 lines.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 03_getline_ingestion 03_getline_ingestion.c
  ./03_getline_ingestion
  ```

---

# Next Step in Curriculum
Proceed to [07_process_memory_layout.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/07_process_memory_layout.md) (Stage 3) to explore the process virtual address space: Text, Data, BSS, Heap, and Stack.
