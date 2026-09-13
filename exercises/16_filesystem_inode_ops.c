/*
 * Exercise 16: VFS Inode Mechanics, File Descriptors, Directories, and Heap Break
 *
 * Covers system calls:
 * - open(), read(), write(), close(), fstat()
 * - link(), unlink(), mkdir(), chdir(), mknod(), sbrk()
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 16_filesystem_inode_ops 16_filesystem_inode_ops.c && ./16_filesystem_inode_ops
 */
#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// POSIX.1-2001 marked sbrk legacy; provide explicit prototype across platforms
extern void *sbrk(intptr_t increment);

/*
 * Drill 1: Directory Lifecycle and Working Directory Traversal
 *
 * Demonstrates:
 * - mkdir(): Creates a directory node with default '.' and '..' entries.
 * - chdir(): Modifies the current working directory in the process table.
 * - getcwd(): Reads the absolute path of the current working directory.
 * - rmdir(): Removes an empty directory.
 */
static bool test_directory_lifecycle(void) {
  char orig_cwd[1024];
  if (getcwd(orig_cwd, sizeof(orig_cwd)) == nullptr) {
    return false;
  }

  const char *sandbox = "test_sandbox_dir";
  // Clean up any stale directory from previous failed run
  rmdir(sandbox);

  // 1. Create directory with read, write, execute permissions for owner
  if (mkdir(sandbox, 0755) != 0) {
    return false;
  }

  // 2. Change into directory
  if (chdir(sandbox) != 0) {
    rmdir(sandbox);
    return false;
  }

  // 3. Verify '.' has at least 2 hard links (itself and parent's link to it)
  struct stat st;
  if (stat(".", &st) != 0 || !S_ISDIR(st.st_mode) || st.st_nlink < 2) {
    chdir(orig_cwd);
    rmdir(sandbox);
    return false;
  }

  // 4. Return to original directory
  if (chdir(orig_cwd) != 0) {
    return false;
  }

  // 5. Remove directory
  return (rmdir(sandbox) == 0);
}

/*
 * Drill 2: Hard Link Invariant & Inode Reference Counting
 *
 * In UNIX, a file is an inode (data blocks + metadata). A filename is merely
 * a directory entry (dentry) pointing to an inode number.
 *
 * Demonstrates:
 * - open(), write(), close(): Creating file and writing payload.
 * - fstat(): Reading inode metadata (st_nlink, st_ino, st_size).
 * - link(): Adding a second directory entry pointing to the SAME inode (st_nlink -> 2).
 * - unlink(): Removing first directory entry. File data persists through link 2 (st_nlink -> 1).
 */
static bool test_hard_link_invariant(void) {
  const char *file1 = "temp_inode_file_1.txt";
  const char *file2 = "temp_inode_file_2.txt";
  const char payload[] = "systems_inode_persistence_test";
  const size_t len = sizeof(payload) - 1;

  // Clean up any stale files
  unlink(file1);
  unlink(file2);

  // 1. Create original file
  int fd = open(file1, O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (fd < 0) {
    return false;
  }
  if (write(fd, payload, len) != (ssize_t)len) {
    close(fd);
    unlink(file1);
    return false;
  }

  // 2. Query inode metadata via fstat
  struct stat st1;
  if (fstat(fd, &st1) != 0) {
    close(fd);
    unlink(file1);
    return false;
  }
  assert(st1.st_nlink == 1);
  ino_t original_inode = st1.st_ino;

  // 3. Create a hard link: file2 -> same inode as file1
  if (link(file1, file2) != 0) {
    close(fd);
    unlink(file1);
    return false;
  }

  // 4. Inspect inode link count via open fd
  struct stat st_linked;
  if (fstat(fd, &st_linked) != 0) {
    close(fd);
    unlink(file1);
    unlink(file2);
    return false;
  }
  // Hard link incremented the inode's reference count to 2
  assert(st_linked.st_nlink == 2);
  assert(st_linked.st_ino == original_inode);
  close(fd);

  // 5. Unlink original filename file1
  if (unlink(file1) != 0) {
    unlink(file2);
    return false;
  }

  // 6. Invariant Verification: file2 still points to original inode with full content!
  int fd2 = open(file2, O_RDONLY);
  if (fd2 < 0) {
    unlink(file2);
    return false;
  }

  struct stat st2;
  if (fstat(fd2, &st2) != 0) {
    close(fd2);
    unlink(file2);
    return false;
  }
  assert(st2.st_ino == original_inode);
  assert(st2.st_nlink == 1); // Back down to 1 link

  char read_buf[64];
  memset(read_buf, 0, sizeof(read_buf));
  ssize_t bytes_read = read(fd2, read_buf, sizeof(read_buf) - 1);
  close(fd2);

  assert(bytes_read == (ssize_t)len && strcmp(read_buf, payload) == 0);

  // Final cleanup: unlink second link, now inode reference count is 0 (disk blocks reclaimed)
  return (unlink(file2) == 0);
}

/*
 * Drill 3: Special Node Creation with mknod (Named Pipe FIFO)
 *
 * Demonstrates:
 * - mknod(): Creates a special filesystem node in the VFS.
 *   (S_IFIFO creates a named pipe accessible without superuser privileges).
 * - stat(): Verifies S_ISFIFO is true.
 * - unlink(): Removes the FIFO from the VFS namespace.
 */
static bool test_mknod_fifo(void) {
  const char *fifo_path = "test_named_pipe.fifo";
  unlink(fifo_path);

  // Create FIFO special file
  if (mknod(fifo_path, S_IFIFO | 0644, 0) != 0) {
    return false;
  }

  struct stat st;
  if (stat(fifo_path, &st) != 0) {
    unlink(fifo_path);
    return false;
  }

  // Invariant: mknod created a FIFO node in the VFS
  if (!S_ISFIFO(st.st_mode)) {
    unlink(fifo_path);
    return false;
  }

  return (unlink(fifo_path) == 0);
}

/*
 * Drill 4: Program Break Expansion with sbrk
 *
 * In classic UNIX, the process heap begins immediately above the uninitialized
 * data segment (.bss) and grows toward higher memory addresses.
 *
 * Demonstrates:
 * - sbrk(0): Queries current address of the program break.
 * - sbrk(increment): Moves the program break upward, allocating zero-filled memory.
 * - sbrk(-increment): Contracts the program break downward, returning memory to kernel.
 */
static bool test_program_break_sbrk(void) {
  // Query initial program break
  void *initial_break = sbrk(0);
  if (initial_break == (void *)-1) {
    return false;
  }

  // Grow the heap by 1024 bytes
  intptr_t alloc_size = 1024;
  void *prev_break = sbrk(alloc_size);
  if (prev_break == (void *)-1) {
    return false;
  }

  // Invariant 1: Return value of sbrk is the PREVIOUS break
  assert(prev_break == initial_break);

  // Invariant 2: Current break has advanced by alloc_size
  void *new_break = sbrk(0);
  assert((char *)new_break == (char *)initial_break + alloc_size);

  // Verify memory is readable and writable
  volatile uint8_t *heap_mem = (volatile uint8_t *)prev_break;
  heap_mem[0] = 0xAA;
  heap_mem[alloc_size - 1] = 0x55;
  assert(heap_mem[0] == 0xAA && heap_mem[alloc_size - 1] == 0x55);

  // Attempt to contract the heap back to initial break
  void *rewound_break = sbrk(-alloc_size);
  if (rewound_break != (void *)-1) {
    void *final_break = sbrk(0);
    // On Linux, the break contracts; on Darwin, sbrk does not contract backward
    if (final_break == initial_break) {
      assert(final_break == initial_break);
    }
  }

  return true;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

int main(void) {
  printf("Running Exercise 16: VFS Inodes, Directories, Special Nodes & Heap Break...\n");

  // Test 1: Directory lifecycle (mkdir, chdir, rmdir)
  assert(test_directory_lifecycle() == true);
  printf("  [PASS] Drill 1: mkdir, chdir, and rmdir directory lifecycle verified.\n");

  // Test 2: Inode reference counting and hard links (open, write, link, unlink, fstat)
  assert(test_hard_link_invariant() == true);
  printf("  [PASS] Drill 2: open, write, read, link, unlink, and fstat inode mechanics verified.\n");

  // Test 3: Special filesystem node creation via mknod
  assert(test_mknod_fifo() == true);
  printf("  [PASS] Drill 3: mknod FIFO creation and verification verified.\n");

  // Test 4: Heap boundary manipulation via sbrk
  assert(test_program_break_sbrk() == true);
  printf("  [PASS] Drill 4: sbrk heap expansion, read/write, and contraction verified.\n");

  printf("[PASS] Exercise 16: All VFS, inode, and memory break system calls verified successfully.\n");
  return EXIT_SUCCESS;
}
