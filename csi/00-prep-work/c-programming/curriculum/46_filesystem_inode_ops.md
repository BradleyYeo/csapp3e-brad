# Stage 6: Exercise 16 - VFS Inode Operations, Directories, and Heap Break

A guide explaining Virtual File System (VFS) architecture, the three-level descriptor-to-inode table hierarchy, hard links (`link`/`unlink`), directory lifecycles (`mkdir`/`chdir`), named pipes (`mknod`), and the runtime program break (`sbrk`) for C beginners.

# Conceptual Foundations and Mental Model

## Inodes vs Filenames: The Core UNIX Abstraction
- In UNIX and Linux, a file is NOT its name. A file is an inode (index node) identified by a numeric ID (`st_ino`).
- The inode stores all metadata: file size, ownership UID/GID, access permissions, timestamp dates, and pointers to the disk blocks storing data.
- What is a filename? A directory is simply a special file containing a list of directory entries (dentries). Each entry maps a human-readable string (filename) to an inode number.
- What is a hard link (`link`)? Creating a hard link creates a new directory entry pointing to the *exact same* inode number. It does NOT duplicate the file data.
- What does `unlink()` do? Decrements the inode's reference count (`st_nlink`). The operating system only frees the disk data blocks when `st_nlink` reaches 0 AND no running process has an open file descriptor pointing to it!

## The Three-Level Kernel Table Hierarchy
- When your program accesses files, the OS kernel coordinates three separate tables:
  - Per-Process File Descriptor Table: Each process has a private array of file descriptors (`fd = 0, 1, 2, ...`). Entries point to items in the Open File Description Table.
  - System-Wide Open File Description Table: Tracks shared file session state: the current read/write offset (in bytes), open flags (`O_RDONLY`, `O_APPEND`), and a pointer to the VFS inode.
  - System-Wide Vnode / Inode Table: Represents the actual physical file or device on disk. Contains physical locks, cached disk blocks, and file size.
- Duplicating descriptors (`dup2`) or `fork` creates multiple FD entries that point to the SAME Open File Description. Writing to one advances the offset for both.

## The Process Program Break: sbrk
- Before modern `mmap`-based dynamic memory allocators, processes grew their heap by adjusting the "program break" (the virtual address separating allocated heap memory from unmapped virtual space).
- Calling `sbrk(increment)`:
  - If `increment > 0`: Moves the break upward by `increment` bytes, expanding the process heap. Returns the previous break address (the start of the newly usable memory).
  - If `increment == 0`: Returns the current program break address without allocating memory.
- Modern `malloc` implementations use `sbrk` for small heap chunks and `mmap` for large memory blocks.

---

# Line-by-Line Code Breakdown

## Function: test_directory_lifecycle

```c
static bool test_directory_lifecycle(void) {
  char orig_cwd[1024];
  if (getcwd(orig_cwd, sizeof(orig_cwd)) == nullptr) {
    return false;
  }

  const char *sandbox = "test_sandbox_dir";
  rmdir(sandbox);

  if (mkdir(sandbox, 0755) != 0) {
    return false;
  }

  if (chdir(sandbox) != 0) {
    rmdir(sandbox);
    return false;
  }

  struct stat st;
  if (stat(".", &st) != 0 || !S_ISDIR(st.st_mode) || st.st_nlink < 2) {
    chdir(orig_cwd);
    rmdir(sandbox);
    return false;
  }

  if (chdir(orig_cwd) != 0) {
    return false;
  }

  return (rmdir(sandbox) == 0);
}
```

### System Call: mkdir(sandbox, 0755)
- Creates a new directory inode with octal permission `0755` (`rwxr-xr-x`).
- Automatically populates two entries inside the directory:
  - `.` (dot): Points to the directory's own inode.
  - `..` (dot-dot): Points to the parent directory's inode.

### Hard Link Invariant: st_nlink >= 2
- A newly created empty directory always has at least 2 hard links:
  - Its name in the parent directory.
  - The `.` entry inside itself.
- Every subdirectory created within it adds another hard link to the parent via `..`.

### System Call: chdir(sandbox)
- Modifies the process table's current working directory pointer without moving any files.

---

## Function: test_hard_link_invariant

```c
static bool test_hard_link_invariant(void) {
  const char *file1 = "test_link_orig.tmp";
  const char *file2 = "test_link_hard.tmp";
  unlink(file1);
  unlink(file2);

  int fd = open(file1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  write(fd, "payload", 7);
  close(fd);

  struct stat st1;
  stat(file1, &st1);
  assert(st1.st_nlink == 1);

  // Create hard link: file2 points to SAME inode as file1
  link(file1, file2);

  struct stat st2;
  stat(file2, &st2);
  assert(st2.st_ino == st1.st_ino);   // Identical inode numbers!
  assert(st2.st_nlink == 2);           // Reference count doubled!

  // Unlink original filename: data remains intact through link 2
  unlink(file1);

  char buf[8] = {0};
  int fd2 = open(file2, O_RDONLY);
  read(fd2, buf, 7);
  close(fd2);
  assert(strcmp(buf, "payload") == 0);

  unlink(file2); // Inode reference count hits 0; blocks reclaimed by OS
  return true;
}
```

### Inode Identity: st2.st_ino == st1.st_ino
- Proves that `link` does not duplicate data; both filenames resolve to the exact same filesystem inode.
- After `unlink(file1)`, reading `file2` retrieves the complete data because the inode reference count is still 1.

---

## Function: test_sbrk_heap_growth

```c
static bool test_sbrk_heap_growth(void) {
  void *initial_brk = sbrk(0);
  intptr_t alloc_bytes = 4096;

  void *allocated_block = sbrk(alloc_bytes);
  void *new_brk = sbrk(0);

  assert(allocated_block == initial_brk);
  assert((uintptr_t)new_brk - (uintptr_t)initial_brk == (uintptr_t)alloc_bytes);

  // Verify memory block is writable
  uint8_t *byte_ptr = (uint8_t *)allocated_block;
  byte_ptr[0] = 0xAA;
  byte_ptr[alloc_bytes - 1] = 0xBB;
  assert(byte_ptr[0] == 0xAA);
  assert(byte_ptr[alloc_bytes - 1] == 0xBB);

  return true;
}
```

### Expanding the Heap
- `sbrk(0)` queries the current break without changes.
- `sbrk(4096)` advances the break by 4096 bytes, returning the starting address of the newly allocated memory page.

---

# Memory Layout Visualization

The Three-Level Table Architecture in the UNIX Kernel:

```
Process Descriptor Table:          Open File Description Table:        VFS Inode Table:
  +---------------+                  +--------------------------+        +----------------------+
  | fd 0 (stdin)  |                  | Offset: 0, Flags: O_RD   |        | Inode 101            |
  +---------------+                  +--------------------------+        | File: "data.txt"     |
  | fd 3 (file1)  | -------------->  | Offset: 7, Flags: O_RDWR | -----> | Size: 7 Bytes        |
  +---------------+             /    +--------------------------+        | Links (st_nlink): 2  |
  | fd 4 (file2)  | -----------/                                         +----------------------+
  +---------------+
```

Hard link creation and reference counting:

```
Directory "exercises":
  [ "test_link_orig.tmp" ] ----\
                                +---> Inode #849204 [ Size: 7, st_nlink: 2, Blocks: [D1] ]
  [ "test_link_hard.tmp" ] ----/

After unlink("test_link_orig.tmp"):
  Directory "exercises":
  [ "test_link_hard.tmp" ] ---------> Inode #849204 [ Size: 7, st_nlink: 1, Blocks: [D1] ]
  (Data blocks D1 remain allocated and accessible)
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is the difference between an inode and a directory entry (dentry)?
- Question 2: Why does an empty directory created via `mkdir` have an `st_nlink` value of 2 instead of 1?
- Question 3: Under what exact conditions will the UNIX kernel free a file's physical disk blocks?

## Drill: Hands-On Code Validation
- Open [16_filesystem_inode_ops.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/16_filesystem_inode_ops.c).
- Implement a named pipe (FIFO) reader and writer using `mknod(path, S_IFIFO | 0666, 0)`.
- Write `"hello fifo"` from a child process and read it in the parent process.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 16_filesystem_inode_ops 16_filesystem_inode_ops.c
  ./16_filesystem_inode_ops
  ```

---

# Next Step in Curriculum
Proceed to [15_process_pipeline_exec.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/15_process_pipeline_exec.md) (Stage 7) to learn process lifecycles (`fork`, `exit`, `waitpid`, `kill`), pipelines (`dup2`, `pipe`), and program execution (`execvp`).
