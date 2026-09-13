# Stage 6: Exercise 06 - Zero-Copy File Search with mmap

A beginner's guide explaining inode metadata querying (`fstat`), memory-mapped I/O (`mmap`), page tables, demand paging, `SIGBUS` vs `SIGSEGV`, and bounded byte scanning (`memchr`).

# Conceptual Foundations and Mental Model

## Traditional read() vs Zero-Copy mmap
- Traditional `read(fd, buf, count)`:
  - Requires the OS kernel to read file blocks from disk into kernel page cache buffers.
  - Requires a second copy operation to copy bytes from kernel memory across the user/kernel boundary into your application's stack or heap buffer (`buf`).
  - High CPU memory bandwidth overhead and multiple context switches.
- Memory Mapping (`mmap`):
  - Does NOT copy file contents upfront.
  - The OS maps the physical disk pages directly into the process's virtual page table.
  - Your program receives a raw memory pointer (`const char *mapped`).
  - Reading `mapped[i]` accesses the file content directly in memory as if it were an array in RAM.

## Demand Paging and Page Faults
- When `mmap` returns, NO file bytes have actually been loaded into physical RAM yet.
- When your code first reads `mapped[0]`:
  - The CPU Memory Management Unit (MMU) looks up the virtual address in the page table.
  - The page table entry is marked "not present" (minor page fault).
  - The CPU pauses execution and triggers a page fault exception into the OS kernel.
  - The kernel reads the 4096-byte disk page into RAM, updates the page table entry to "present", and resumes your instruction.
  - Subsequent reads to that 4KB page execute at hardware RAM speeds with zero system call overhead.

## Bounded Slices vs Null-Terminated Strings
- Files on disk are arbitrary byte streams. They are NOT null-terminated strings!
- Never use `strlen()` or `strchr()` on an `mmap` buffer:
  - If the file does not contain a `\0` byte near the end, `strchr` will read past the mapped memory boundary.
  - Reading past the end of an `mmap` boundary triggers an immediate hardware `SIGSEGV` (Segmentation Fault).
  - Production tools like `grep` always use `memchr(buf, target, len)`, which enforces strict length bounds.

## SIGBUS vs SIGSEGV
- `SIGSEGV`: Accessing a virtual memory address that is completely unmapped or violates permissions (e.g. writing to `PROT_READ` memory).
- `SIGBUS`: Accessing an address that is within a valid page table mapping, but the underlying disk file was truncated or shrunk by another process while mapped, meaning no physical disk block exists to back the page.

---

# Line-by-Line Code Breakdown

## Function: get_file_len

```c
static bool get_file_len(int fd, size_t *len_out) {
  if (fd < 0 || len_out == nullptr) {
    return false;
  }

  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    return false;
  }

  if (!S_ISREG(sb.st_mode)) {
    return false;
  }

  *len_out = (size_t)sb.st_size;
  return true;
}
```

### Precondition Check: fd < 0 || len_out == nullptr
- In Unix, valid file descriptors are non-negative integers (`0, 1, 2, ...`). Negative values represent invalid descriptors or error returns.
- `len_out == nullptr` protects against dereferencing a null output pointer.

### Structure: struct stat sb
- Declares a stack structure defined in `<sys/stat.h>`.
- Contains metadata stored in the filesystem inode: file size (`st_size`), permissions and file type (`st_mode`), owner ID (`st_uid`), timestamps, and block counts.

### System Call: fstat(fd, &sb)
- Passes the open file descriptor `fd` and pointer `&sb` to the kernel.
- Returns `0` on success. Returns `-1` on error (e.g. bad descriptor, permission denied).
- Does not read any file data, making it extremely fast ($O(1)$ inode query).

### File Type Predicate: !S_ISREG(sb.st_mode)
- `sb.st_mode` contains bit flags encoding both permissions (read/write/execute) and the file kind.
- `S_ISREG()` is a POSIX macro that tests whether the descriptor is a regular disk file.
- Rejects directories, pipes, sockets, and character devices (such as `/dev/urandom`), which have no fixed size and cannot be mapped into virtual memory.

### Storing Output: *len_out = (size_t)sb.st_size
- Extracts `st_size` (type `off_t`, a signed integer) and stores it in caller's `size_t` variable.

---

## Function: map_file_ro

```c
static const char *map_file_ro(int fd, size_t len) {
  if (fd < 0 || len == 0) {
    return nullptr;
  }

  void *addr = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    return nullptr;
  }

  #ifdef MADV_SEQUENTIAL
  madvise(addr, len, MADV_SEQUENTIAL);
  #endif

  return (const char *)addr;
}
```

### Parameters of mmap
- `nullptr`: Asks the kernel to choose any suitable unused virtual memory address.
- `len`: Exact number of bytes to map.
- `PROT_READ`: Memory protection flag; pages are read-only. Attempting to write triggers `SIGSEGV`.
- `MAP_PRIVATE`: Copy-on-write mapping; modifications are private to the process and never flushed back to disk.
- `fd`: Open file descriptor backing the mapping.
- `0`: Byte offset in the file where mapping starts (must be a multiple of page size).

### Error Return: MAP_FAILED
- `mmap` does NOT return `NULL` on error. It returns `MAP_FAILED` (which is `(void *)-1`).
- Comparing `addr == MAP_FAILED` is essential.

### Kernel Optimization: madvise(MADV_SEQUENTIAL)
- Hints to the kernel page cache that the process will scan pages linearly forward.
- The kernel aggressively pre-fetches upcoming disk pages in the background before they are accessed.

---

## Function: unmap_file

```c
static bool unmap_file(const char *addr, size_t len) {
  if (addr == nullptr || len == 0) {
    return false;
  }

  return munmap((void *)addr, len) == 0;
}
```

### Resource Deallocation
- `munmap` removes the page table entries and releases the virtual address space.
- Invariant: Every successful `mmap` must have a matching `munmap`.

---

# Memory Layout Visualization

Memory-mapped file page table interaction:

```
Process Virtual Memory Space:
  [ ... Heap ... ]
  Address: 0x70001000  [ Page 0: 4KB ] -----> Disk Page 0 (Cached in RAM)
  Address: 0x70002000  [ Page 1: 4KB ] -----> Not in RAM yet! (Page fault on first access)
  Address: 0x70003000  [ Page 2: 4KB ]
  [ ... Stack ... ]

Reading mapped[0]:
  1. MMU resolves 0x70001000 -> Hits RAM page cache directly -> instantaneous read!

Reading mapped[4096]:
  1. MMU resolves 0x70002000 -> Page fault exception.
  2. OS kernel loads disk block 1 into RAM.
  3. Page table updated.
  4. Program resumes transparently.
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: Why does `mmap` return `MAP_FAILED` (`(void *)-1`) instead of `NULL` on error?
- Question 2: Why will using `strchr` on an `mmap` buffer cause a segmentation fault on a binary file or file without null bytes?
- Question 3: What is the difference between `fstat` and `stat`? Why is `fstat` preferred when an open descriptor already exists?

## Drill: Hands-On Code Validation
- Open [06_mmap_search.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_mmap_search.c).
- Implement a substring search function:
  ```c
  const char *mmap_search_substr(const char *mapped, size_t len, const char *substr);
  ```
- Use `memchr` to find candidate occurrences of the first character, followed by `memcmp` to verify the full substring.
- Add assertions in `main()` verifying that searching for `"needle"` succeeds and returns the correct offset.
- Compile and run:
  ```bash
  clang -Wall -Wextra -pedantic -g -o 06_mmap_search 06_mmap_search.c
  ./06_mmap_search
  ```

---

# Next Step in Curriculum
Proceed to [16_filesystem_inode_ops.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/16_filesystem_inode_ops.md) to explore low-level file descriptors, VFS inodes, hard links (`link`/`unlink`), directories, and heap growth (`sbrk`).
