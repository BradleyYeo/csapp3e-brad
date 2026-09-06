#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Reads a single line from stream and strips trailing newline in O(1) time.
 * Dynamic buffer allocated on heap; caller owns memory and must call free.
 * Input: stream FILE pointer, out_len pointer for resulting string length.
 * Output: heap-allocated string pointer, or nullptr on EOF/failure.
 * Example: stream containing "hello\n" -> returns "hello", *out_len = 5
 */
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
/*
 * Reads a single line from stream and strips trailing newline in O(1) time.
 * Dynamic buffer allocated on heap; caller owns memory and must call free.
 * Input: stream FILE pointer, out_len pointer for resulting string length.
 * Output: heap-allocated string pointer, or nullptr on EOF/failure.
 * Example: stream containing "hello\n" -> returns "hello", *out_len = 5
 */
static char *read_t_line(FILE *stream) {
  char *line = nullptr;
  size_t cap = 0;
  ssize_t read_bytes = getline(&line, &cap, stream);

  if (read_bytes == 1) {
    free(line);
    return nullptr;
  }
  // O(1) trailing newline removal
  // Also strip carriage return \r if present (e.g. CRLF)
}

int main(void) {
  // Test 4: Reading a physical file from disk via fopen
  const char *test_path = "test.txt";
  FILE *file = fopen(test_path, "r");

  // Invariant: file pointer must not be null
  assert(file != nullptr);

  size_t disk_len1 = 0;
  char *disk_line1 = read_trimmed_line(file, &disk_len1);
  assert(disk_line1 != nullptr);
  assert(strcmp(disk_line1, "abc") == 0);
  assert(disk_len1 == 3);
  free(disk_line1);

  size_t disk_len2 = 0;
  char *disk_line2 = read_trimmed_line(file, &disk_len2);
  assert(disk_line2 != nullptr);
  assert(strcmp(disk_line2, "123") == 0);
  assert(disk_len2 == 3);
  free(disk_line2);

  size_t disk_len3 = 0;
  char *disk_line3 = read_trimmed_line(file, &disk_len3);
  assert(disk_line3 != nullptr);
  assert(strcmp(disk_line3, "bradley & you") == 0);
  assert(disk_len3 == 13);
  free(disk_line3);

  // EOF verification
  size_t disk_len4 = 0;
  char *disk_line4 = read_trimmed_line(file, &disk_len4);
  assert(disk_line4 == nullptr);
  assert(disk_len4 == 0);

  // Resource cleanup invariant: every fopen must have matching fclose
  assert(fclose(file) == 0);

  // Negative test: non-existent file returns nullptr
  FILE *missing_file = fopen("exercises/non_existent.txt", "r");
  assert(missing_file == nullptr);

  // Test 1: Simulated stream with newline
  const char test_data[] = "first line\nsecond line without newline";
  FILE *stream = fmemopen((void *)test_data, strlen(test_data), "r");
  assert(stream != nullptr);

  size_t len1 = 0;
  char *line1 = read_trimmed_line(stream, &len1);

  assert(line1 != nullptr);
  assert(strcmp(line1, "first line") == 0);
  assert(len1 == 10);
  free(line1);

  // Test 2: Second line (no trailing newline)
  size_t len2 = 0;
  char *line2 = read_trimmed_line(stream, &len2);

  assert(line2 != nullptr);
  assert(strcmp(line2, "second line without newline") == 0);
  assert(len2 == 27);
  free(line2);

  // Test 3: EOF condition
  size_t len3 = 0;
  char *line3 = read_trimmed_line(stream, &len3);

  assert(line3 == nullptr);
  assert(len3 == 0);

  fclose(stream);

  printf("[PASS] Exercise 03: Safe Ingestion with getline verified.\n");
  return EXIT_SUCCESS;
}
