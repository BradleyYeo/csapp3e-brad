/* K&R 1-14: Count character frequencies (ASCII 0..127) and format a
 * horizontal histogram.
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * count_char_frequencies: counts occurrences of each ASCII character (0..127)
 * in the NUL-terminated string `input` and stores them in `counts`.
 * `counts` must be zero-initialized by this function.
 */
void count_char_frequencies(const char *input, int counts[static 128]) {
  (void)input;
  for (size_t i = 0; i < 128; i++) {
    counts[i] = 0;
  }
  /* TODO: implement */
}

/*
 * format_histogram: writes a horizontal bar chart into `out` (up to `out_cap`
 * bytes, always NUL-terminated) for characters with counts[i] > 0, formatted
 * as "<char>: <count asterisks>\n" in ascending ASCII order.
 */
void format_histogram(const int counts[static 128], char *out, size_t out_cap) {
  (void)counts;
  if (out_cap > 0) {
    out[0] = '\0';
  }
  /* TODO: implement */
}

int main(void) {
  int counts[128];
  count_char_frequencies("abcaab", counts);
  assert(counts[(unsigned char)'a'] == 3);
  assert(counts[(unsigned char)'b'] == 2);
  assert(counts[(unsigned char)'c'] == 1);
  assert(counts[(unsigned char)'z'] == 0);

  char buf[256];
  format_histogram(counts, buf, sizeof(buf));
  assert(strcmp(buf, "a: ***\nb: **\nc: *\n") == 0);

  count_char_frequencies("", counts);
  format_histogram(counts, buf, sizeof(buf));
  assert(strcmp(buf, "") == 0);

  printf("[PASS] Exercise 08: char histogram verified.\n");
  return EXIT_SUCCESS;
}
