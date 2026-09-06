#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Drill 1: File Size Query via fstat
 *
 * mmap requires the exact byte count to map into the page table.
 * fstat retrieves file metadata from the inode without reading file content.
 *
 * Expected input: open file descriptor, size output pointer.
 * Expected output: true if regular file with valid size, false otherwise.
 */
static bool get_file_len(int fd, size_t *len_out) {
  if (fd < 0 || len_out == nullptr) {
    return false;
  }

  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    return false;
  }

  // Only regular files can be mapped; sockets, pipes, and FIFOs cannot
  if (!S_ISREG(sb.st_mode)) {
    return false;
  }

  *len_out = (size_t)sb.st_size;
  return true;
}

/*
 * Drill 2: Memory Mapping via mmap
 *
 * Maps file blocks directly into process virtual memory.
 * Bypasses user-space stdio buffers (zero-copy read).
 *
 * Flags:
 * - PROT_READ: Pages are read-only.
 * - MAP_PRIVATE: Copy-on-write mapping; modifications do not write to disk.
 *
 * Expected input: file descriptor, byte length.
 * Expected output: pointer to mapped memory, or nullptr on failure.
 */
static const char *map_file_ro(int fd, size_t len) {
  if (fd < 0 || len == 0) {
    return nullptr;
  }

  void *addr = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    return nullptr;
  }

  // Hint to Linux kernel readahead engine for sequential streaming
  #ifdef MADV_SEQUENTIAL
  madvise(addr, len, MADV_SEQUENTIAL);
  #endif

  return (const char *)addr;
}

/*
 * Drill 3: Bounded In-Memory Scanning (Avoiding strchr on mmap)
 *
 * Mapped files are byte slices bounded by size, NOT null-terminated strings.
 * Using strchr or strlen on an mmap buffer risks reading past the mapping,
 * triggering a SIGSEGV / segmentation fault.
 *
 * Production grep uses memchr to guarantee scans never exceed mapped length.
 *
 * Expected input: mapped buffer pointer, buffer length, target byte.
 * Expected output: pointer to matching byte, or nullptr if not found.
 */
static const char *find_byte_bounded(const char *buf, size_t len, char target) {
  if (buf == nullptr || len == 0) {
    return nullptr;
  }

  return (const char *)memchr(buf, target, len);
}

/*
 * Drill 4: Unmapping and Cleanup Invariant
 *
 * Unmaps memory pages from the process page table and releases virtual address space.
 *
 * Expected input: mapped memory address, byte length.
 * Expected output: true on successful unmapping, false otherwise.
 */
static bool unmap_file(const char *addr, size_t len) {
  if (addr == nullptr || len == 0) {
    return false;
  }

  return munmap((void *)addr, len) == 0;
}

/*
 * Helper: Creates a temporary file on disk with given content for testing.
 */
static int create_temp_file(const char *content, size_t len, char *path_template) {
  int fd = mkstemp(path_template);
  if (fd < 0) {
    return -1;
  }

  if (write(fd, content, len) != (ssize_t)len) {
    close(fd);
    unlink(path_template);
    return -1;
  }

  return fd;
}

int main(void) {
  char temp_path[] = "/tmp/mmap_test_XXXXXX";
  const char test_data[] = "0123456789\nneedle in haystack\nfinal_line";
  const size_t test_len = sizeof(test_data) - 1;

  int fd = create_temp_file(test_data, test_len, temp_path);
  assert(fd >= 0);

  // Test 1: File size query via fstat
  size_t file_len = 0;
  assert(get_file_len(fd, &file_len) == true);
  assert(file_len == test_len);

  // Test 2: Memory map read-only
  const char *mapped = map_file_ro(fd, file_len);
  assert(mapped != nullptr);

  // Test 3: Zero-copy direct memory reads
  assert(mapped[0] == '0');
  assert(mapped[9] == '9');

  // Test 4: Bounded scanning with memchr (safe without null terminator)
  const char *needle = find_byte_bounded(mapped, file_len, 'n');
  assert(needle != nullptr);
  assert(strncmp(needle, "needle", 6) == 0);

  const char *missing = find_byte_bounded(mapped, file_len, 'z');
  assert(missing == nullptr);

  // Test 5: Clean unmap and file descriptor release
  assert(unmap_file(mapped, file_len) == true);
  assert(close(fd) == 0);
  assert(unlink(temp_path) == 0);

  printf("[PASS] Exercise 06: Memory Mapping (mmap) verified.\n");
  return EXIT_SUCCESS;
}
