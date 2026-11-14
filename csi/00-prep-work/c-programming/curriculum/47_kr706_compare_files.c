/* K&R 7-6: Write a program to compare two files, printing the first line
 * where they differ.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr size_t MAXLINELENGTH = 256;

/*
 * compare_files: compares `fp1` and `fp2` line by line (up to `cap` bytes per
 * line).
 * Returns:
 *   0  if both files are identical,
 *   1  if a difference (or early EOF in one file) is found, writing the
 *      1-based line number to `*diff_line` and the differing lines (or "" on
 *      EOF) into `line1` and `line2`,
 *  -1  on stream error or invalid arguments.
 */
int compare_files(FILE *fp1, FILE *fp2, char *line1, char *line2, size_t cap,
                  size_t *diff_line) {
  (void)fp1;
  (void)fp2;
  (void)diff_line;
  if (cap > 0) {
    line1[0] = '\0';
    line2[0] = '\0';
  }
  /* TODO: implement */
  return -1;
}

static FILE *make_memfile(const char *content) {
  FILE *fp = tmpfile();
  assert(fp != nullptr);
  fputs(content, fp);
  rewind(fp);
  return fp;
}

int main(void) {
  char l1[MAXLINELENGTH];
  char l2[MAXLINELENGTH];
  size_t diff_line = 0;

  FILE *a = make_memfile("line one\nline two\n");
  FILE *b = make_memfile("line one\nline two\n");
  assert(compare_files(a, b, l1, l2, sizeof(l1), &diff_line) == 0);
  fclose(a);
  fclose(b);

  FILE *c = make_memfile("line one\nalpha\nline three\n");
  FILE *d = make_memfile("line one\nbeta\nline three\n");
  assert(compare_files(c, d, l1, l2, sizeof(l1), &diff_line) == 1);
  assert(diff_line == 2);
  assert(strcmp(l1, "alpha\n") == 0);
  assert(strcmp(l2, "beta\n") == 0);
  fclose(c);
  fclose(d);

  FILE *e = make_memfile("only one\n");
  FILE *f = make_memfile("only one\nextra line\n");
  assert(compare_files(e, f, l1, l2, sizeof(l1), &diff_line) == 1);
  assert(diff_line == 2);
  assert(strcmp(l1, "") == 0);
  assert(strcmp(l2, "extra line\n") == 0);
  fclose(e);
  fclose(f);

  printf("[PASS] Exercise 43: compare files verified.\n");
  return EXIT_SUCCESS;
}
