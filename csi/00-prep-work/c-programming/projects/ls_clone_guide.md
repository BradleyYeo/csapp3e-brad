# System Design & Implementation Guide: POSIX Directory Lister (`ls`)

## Overview

Documentation and architectural guide for [ls_clone.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c), deconstructing the existing implementation, identifying bugs and design debt, and establishing an exercise curriculum based on:
- John Ousterhout (*A Philosophy of Software Design*): Deep modules, information hiding, defining errors out of existence.
- Daniel Jackson (*The Essence of Software / Concept Design*): Independent concepts, concept state, operational principles.
- Jimmy Koppel (*Advanced Software Design*): Representation independence, design for change, explicit invariants.
- Modern C23 Standard: `nullptr`, native `bool`, `constexpr`, `auto`, `[[nodiscard]]`, zero initialization.
- Linux & POSIX Internals: Inode mechanics, directory streams, directory-relative syscalls (`fstatat`), NSS caching, terminal geometry.

## Current Implementation Analysis: `ls_clone.c`

### Failure Modes & Vulnerabilities

- **Buffer Overflow & Stack Allocation (`FileInfo files[1024]`)**:
  - In [ls_clone.c:L82](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c#L82), a static stack array of size 1024 is allocated.
  - Directories containing more than 1024 entries trigger out-of-bounds writes on `files[count]`, corrupting memory.
  - Fixed-size path buffers `char name[256]` and `char full_path[512]` violate POSIX `PATH_MAX` (4096 bytes) and can truncate or overflow.

- **Integer Overflow in Sorting Comparator**:
  - In [ls_clone.c:L18-L22](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c#L18-L22):
    ```c
    int sort_by_size(const void *a, const void *b) {
      FileInfo *f1 = (FileInfo *)a;
      FileInfo *f2 = (FileInfo *)b;
      return f2->size - f1->size;
    }
    ```
  - `size` is an `off_t` (signed 64-bit integer on Linux x86_64).
  - Truncating a 64-bit subtraction result into a 32-bit `int` causes integer overflow for files larger than 2 GiB, producing reversed or unstable ordering.

- **Dereferencing `NULL` on Unmapped UID/GID**:
  - In [ls_clone.c:L40-L43](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c#L40-L43):
    ```c
    printf(" %s", getpwuid(st->st_uid)->pw_name);
    printf(" %s", getgrgid(st->st_gid)->gr_name);
    ```
  - When a file is owned by an orphaned UID or GID (common in containers or after user deletion), `getpwuid` and `getgrgid` return `NULL`.
  - Immediate dereference (`->pw_name`) causes a segmentation fault (`SIGSEGV`).

- **Symlink Dereference via `stat`**:
  - In [ls_clone.c:L92](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c#L92), `stat` is invoked rather than `lstat`.
  - If a symlink points to a non-existent file (dangling symlink), `stat` returns `-1`, leaving `files[count].st` completely uninitialized.

- **N+1 System Call Performance Bottleneck**:
  - For every file listed in long format, `getpwuid` and `getgrgid` are invoked. These functions query NSS databases (`/etc/passwd`, LDAP, or SSSD), introducing disk I/O and IPC per entry without caching.

### Design Debt

- **Shallow Module Structure (Ousterhout)**:
  - `main` in [ls_clone.c:L57-L111](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/00-prep-work/c-programming/projects/ls_clone.c#L57-L111) handles flag parsing, path concatenation, directory traversal, syscall invocation, sorting, and formatting.
  - No abstraction hides the directory stream mechanics or formatting logic.

- **Tangled Concepts (Jackson)**:
  - Entry discovery, metadata retrieval, sorting, and terminal presentation are tightly coupled in a single loop.
  - Adding a new flag (such as `-t` for time sort or `-r` for reverse) requires mutating multiple branches throughout `main`.

- **Lack of Representation Independence (Koppel)**:
  - Storage representation (`FileInfo` struct with embedded raw `struct stat`) is exposed directly to presentation code, preventing deferred metadata fetching or alternative memory layouts.

---

## Conceptual Architecture

### Concept Decomposition (Daniel Jackson)

- **Concept 1: DirectoryScanner**:
  - *Purpose*: Read directory entries matching inclusion criteria.
  - *State*: Path, directory handle (`DIR *`), list of entry names and types.
  - *Actions*: `scan_directory(path, filter_pred) -> vector<Entry>`.

- **Concept 2: MetadataResolver**:
  - *Purpose*: Fetch file attributes via `fstatat` or `lstat`.
  - *State*: File descriptor, entry list.
  - *Actions*: `resolve_metadata(entry) -> EntryMetadata`.

- **Concept 3: IdentityRegistry**:
  - *Purpose*: Map numeric UID/GID to strings with cache backing.
  - *State*: Hash table / lookup array of cached UID/GID mappings.
  - *Actions*: `resolve_uid(uid) -> string_view`, `resolve_gid(gid) -> string_view`.

- **Concept 4: EntryOrdering**:
  - *Purpose*: Provide total ordering over entry collections.
  - *State*: Sort key selector (name, size, mtime) and polarity (forward, reverse).
  - *Actions*: `sort_entries(entries, comparator)`.

- **Concept 5: PresentationLayout**:
  - *Purpose*: Format entries for terminal or stream output.
  - *State*: Layout mode (single-column, long list, multi-column grid), terminal width.
  - *Actions*: `render(entries, layout_options, stream)`.

### Deep Interface Principles (John Ousterhout)

- **Define Errors Out of Existence**:
  - Identity lookup: If a UID cannot be mapped to a username, the `IdentityRegistry` formats the raw numeric UID into a string buffer rather than returning an error or `nullptr`. The caller never handles "user not found" exceptions.
  - Path joining: Use directory file descriptors with `fstatat` (`AT_SYMLINK_NOFOLLOW`) rather than manual string path concatenation, eliminating buffer overflow risks.

- **Pull Complexity Downward**:
  - Terminal column calculation: The caller asks to `render_grid(entries)`. The presentation module queries terminal window size (`ioctl` with `TIOCGWINSZ`), computes maximum string lengths, determines optimal column count, and formats padding without caller involvement.

### Invariants & Representation Independence (Jimmy Koppel)

- **Dynamic Vector Invariant**:
  - An entry collection satisfies `count <= capacity`. Growth is handled uniformly with geometric resizing, guaranteeing $O(1)$ amortized insertion and memory safety.

- **Orthogonal Transformations**:
  - Sorting, filtering, and presentation are independent pipeline stages. A sort flag alters the comparator function pointer passed to `sort_entries` without changing traversal or rendering logic.

---

## Modern C23 Standards Integration

- **Pointer Literals**: Use `nullptr` exclusively instead of `NULL` or `0`.
- **Booleans**: Use native `bool`, `true`, and `false` keywords.
- **Attributes**:
  - `[[nodiscard]]` on all functions returning dynamically allocated memory, error codes, or critical state.
  - `[[maybe_unused]]` on comparator parameters or optional arguments.
- **Compile-time Constants**: Use `constexpr` instead of preprocessor `#define` for sizing, initial capacities, and flags.
- **Type Deduction**: Use `auto` for iterators and complex pointer types where the RHS is explicit.
- **Initialization**: Uniform zero-initialization using `{}` syntax.

---

## Curriculum: Incremental Exercises

### Exercise 1: Bounded Growth & Safe String Management
- *Goal*: Eliminate static buffer limits (`FileInfo files[1024]`, `name[256]`, `full_path[512]`).
- *Concepts*: Dynamic memory, invariant preservation, geometric growth.
- *Tasks*:
  - Define an opaque or semi-opaque `EntryVector` type managing dynamically allocated entries.
  - Enforce `[[nodiscard]] bool entry_vector_push(EntryVector *vec, Entry entry)`.
  - Store dynamically allocated file names (`strdup` or heap allocation) rather than fixed 256-byte arrays.
  - Write a cleanup function `void entry_vector_destroy(EntryVector *vec)` that leaves no memory leaks.

### Exercise 2: Robust Metadata & Identity Cache
- *Goal*: Eliminate null-dereference crashes on unmapped UID/GID and replace `stat` with `lstat`.
- *Concepts*: Defining errors out of existence, caching, POSIX file modes.
- *Tasks*:
  - Replace `stat` with `lstat` or `fstatat(dirfd, name, &st, AT_SYMLINK_NOFOLLOW)`.
  - Implement an `IdentityCache` module caching up to 64 UID and GID lookups.
  - Implement fallback: If `getpwuid` returns `nullptr`, format the numeric ID into a numeric string (e.g., `"1001"`).
  - Extract permission formatting into a pure helper `void format_mode(mode_t mode, char out[11])` handling regular files, directories, symlinks, block devices, and character devices.

### Exercise 3: Overflow-Safe Generic Sorting
- *Goal*: Eliminate 64-bit integer truncation bugs in `qsort`.
- *Concepts*: Arithmetic invariants, comparator abstraction, orthogonal configuration.
- *Tasks*:
  - Implement a three-way comparison utility: `int cmp_int64(int64_t a, int64_t b)`.
  - Write dedicated comparators for size (`-S`), modification time (`-t`), and alphabetical name (`default`).
  - Implement a higher-order comparator wrapper or flag inverter to support reverse sorting (`-r`) without rewriting individual comparators.

### Exercise 4: Multi-Column Terminal Grid Formatting
- *Goal*: Replicate standard `ls` behavior when stdout is a TTY without `-l`.
- *Concepts*: Linux ioctl terminal geometry, modular formatting.
- *Tasks*:
  - Query terminal width using `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)`. Fall back to 80 columns if stdout is not a TTY (`isatty`).
  - Determine the maximum filename length in the entry vector.
  - Compute the number of rows and columns, printing vertically or horizontally aligned columns with uniform padding.

### Exercise 5: Symlink Target Resolution & Formatting
- *Goal*: Implement complete POSIX long-listing output for symbolic links (`name -> target`).
- *Concepts*: System calls (`readlink` / `readlinkat`), buffer sizing.
- *Tasks*:
  - Detect symbolic links using `S_ISLNK(st.st_mode)`.
  - Use `readlinkat` to read the symlink destination into a dynamically sized buffer without null-termination pitfalls.
  - Format symlink entries in the long format view as `link_name -> target_path`.
